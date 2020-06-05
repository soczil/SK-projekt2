#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>
#include <chrono>
#include <csignal>
#include <cstring>
#include "radio-client.h"
#include "message.h"
#include "err.h"

bool finishTelnet = false;
bool endConnection = false;
bool isServer = false;

std::vector<Server> servers;
Server currentServer;

const char *KEY_UP = "\033[A";
const char *KEY_DOWN = "\033[B";
const char *ENTER = "\r\0";

static unsigned parseToUnsigned(char *numberPtr) {
    char *end;
    unsigned number = strtoul(numberPtr, &end, 10);

    if ((errno != 0) || (end == numberPtr) || (*end != '\0')) {
        return 0;
    }

    return number;
}

RadioClient::RadioClient(int argc, char **argv) {
    int opt;
    bool H, P, p;

    H = P = p = false;
    while ((opt = getopt(argc, argv, "H:P:p:T:")) != -1) {
        switch (opt) {
            case 'H':
                H = true;// Na adres rozgłoszeniowy.
                this->host = optarg;
                break;
            case 'P':
                P = true;
                this->udpPort = parseToUnsigned(optarg);
                break;
            case 'p':
                p = true;
                this->tcpPort = parseToUnsigned(optarg);
                break;
            case 'T':
                this->timeout = parseToUnsigned(optarg);
                if (this->timeout == 0) {
                    fatal("T option");
                }
                break;
            default:
                fatal("unknown option");
        }
    }

    if (!H || !P || !p) {
        fatal("obligatory arguments were not provided");
    }
}

void RadioClient::sendDiscover(struct sockaddr *address) {
    int sock = broadcastSocket.getSockNumber();
    struct message message {};
    size_t length;

    message.type = htons(1);
    message.length = htons(0);

    length = sizeof(message);
    if (sendto(sock, &message, length, 0,
            address, sizeof(*address)) != (ssize_t) length) {
        syserr("partial / failed sendto");
    }
}

int RadioClient::serverLookup(struct sockaddr *seekedAddress) {
    auto *seekedIn = (struct sockaddr_in *) seekedAddress;
    struct sockaddr_in *clientIn;

    for (size_t i = 0; i < servers.size(); i++) {
        clientIn = (struct sockaddr_in *) servers[i].getPtrToAddress();
        if (seekedIn->sin_addr.s_addr == clientIn->sin_addr.s_addr
            && seekedIn->sin_port == clientIn->sin_port) {
            return i;
        }
    }

    return -1;
}

void RadioClient::updateOptions() {
    size_t position = telnetScreen.getPosition();
    size_t options = telnetScreen.getOptions();

    if (position == options - 1) {
        // Wskazujemy na 'Koniec'.
        telnetScreen.setPosition(position + 1);
    }

    telnetScreen.setOptions(options + 1);
}

void RadioClient::handleIam(struct sockaddr *address, socklen_t addressSize,
                             struct message *message) {
    std::string name(message->buffer);
    int position = serverLookup(address);

    if (position == -1) {
        Server server(address, addressSize, name);
        protector.lock();
        servers.push_back(server);
        updateOptions();
        telnetScreen.render(telnetSock, servers);
        protector.unlock();
    }
}

void RadioClient::sendKeepAlive() {
    int sock = broadcastSocket.getSockNumber();
    struct sockaddr *address = currentServer.getPtrToAddress();
    socklen_t addressSize = currentServer.getAddressSize();
    struct message message {};
    size_t length;


    message.type = htons(3);
    message.length = htons(0);

    length = sizeof(message);
    while (!endConnection) {
        std::this_thread::sleep_for(std::chrono::milliseconds(3500));
        // TODO: check if sendto is non-blocking
        if (sendto(sock, &message, length, 0,
                address, addressSize) != (ssize_t) length) {
            syserr("partial / failed sendto");
        }
    }
}

void RadioClient::receiveData() {
    int sock = broadcastSocket.getSockNumber();
    ssize_t recvLength;
    struct sockaddr address;
    socklen_t addressSize;
    struct message message {};

    while (receive) {
        std::memset(&address, 0, sizeof(struct sockaddr));
        addressSize = sizeof(address);
        recvLength = recvfrom(sock, &message, sizeof(message),
                    0, &address, &addressSize);
        if (recvLength < 0) {
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                continue;
            }
            syserr("recvfrom");
        }

        message.type = ntohs(message.type);
        message.length = ntohs(message.length);

        if (message.type == 2) {
            std::cout << inet_ntoa(((struct sockaddr_in *) &address)->sin_addr) << std::endl;
            handleIam(&address, addressSize, &message);
        } else if (sameAddresses(&address, currentServer.getPtrToAddress())) {
            if (message.type == 4) {
                if (write(1, message.buffer, message.length) != message.length) {
                    syserr("write");
                }
            } else if (message.type == 6) {
                if (write(2, message.buffer, message.length) != message.length) {
                    syserr("write");
                }
            }
        }
    }
}

void RadioClient::keyUpClicked() {
    size_t position = telnetScreen.getPosition();

    if (position != 0) {
        telnetScreen.setPosition(position - 1);
    }
}

void RadioClient::keyDownClicked() {
    size_t position = telnetScreen.getPosition();
    size_t options = telnetScreen.getOptions();

    if (position != options - 1) {
        telnetScreen.setPosition(position + 1);
    }
}

void RadioClient::enterClicked(std::thread &keepAlive) {
    size_t position = telnetScreen.getPosition();
    size_t options = telnetScreen.getOptions();

    if (position == 0) {
        // Szukaj pośredników.
        sendDiscover((struct sockaddr *) broadcastSocket.getSockaddrPtr());
    } else if (position == options - 1) {
        // Koniec.
        finishTelnet = true;
        endConnection = true;
        receive = false;
    } else {
        // Wybrano radio.
        if (isServer) {
            // Jakieś radio gra, więc trzeba je zatrzymać.
            endConnection = true;
            keepAlive.join();
            memset(currentServer.getPtrToAddress(), 0, sizeof(struct sockaddr));
        }

        isServer = true;
        currentServer = servers[position - 1];
        sendDiscover(currentServer.getPtrToAddress());
        keepAlive = std::thread(&RadioClient::sendKeepAlive, this);
    }
}

void RadioClient::menageTelnet(int sock) {
    char buffer[64];

    if ((this->telnetSock = accept(sock, (struct sockaddr *) 0,
                (socklen_t *) 0)) < 0) {
        syserr("accept");
    }

    telnetScreen.setPosition(0);
    telnetScreen.setOptions(2);
    telnetScreen.prepare(telnetSock);

    std::thread receivingThread(&RadioClient::receiveData, this);
    std::thread keepAliveThread;

    while (!finishTelnet) {
        protector.lock();
        telnetScreen.render(telnetSock, servers);
        protector.unlock();

        memset(buffer, 0, 64);
        if (read(telnetSock, buffer, 64) < 0) {
            syserr("read");
        }

        protector.lock();
        if (strcmp(buffer, KEY_UP) == 0) {
            keyUpClicked();
        } else if (strcmp(buffer, KEY_DOWN) == 0) {
            keyDownClicked();
        } else if (strcmp(buffer, ENTER) == 0) {
            enterClicked(keepAliveThread);
        }
        protector.unlock();
    }

    receive = false;
    keepAliveThread.join();
    receivingThread.join();
    servers.clear();
}

bool RadioClient::sameAddresses(struct sockaddr *x, struct sockaddr *y) {
    auto xIn = (struct sockaddr_in *) x;
    auto yIn = (struct sockaddr_in *) y;

    return ((xIn->sin_addr.s_addr == yIn->sin_addr.s_addr)
            && (xIn->sin_port == yIn->sin_port));
}

void RadioClient::start() {
    broadcastSocket.openSocket(udpPort, host);
    tcpSocket.openSocketForTelnet(tcpPort);
    menageTelnet(tcpSocket.getSockNumber());
    servers.clear();
    broadcastSocket.closeSocket();
}

static void handleSignal(__attribute__((unused)) int signal) {
    finishTelnet = true;
    // TODO: close sockets itp
    exit(0);
}

int main(int argc, char **argv) {
    std::signal(SIGINT, handleSignal);
    RadioClient radioClient(argc, argv);
    radioClient.start();
}

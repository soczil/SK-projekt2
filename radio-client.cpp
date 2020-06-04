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

static int finish = false;
std::vector<Server> servers;
Server currentServer;
bool isServer = false;

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
                H = true;
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

void RadioClient::sendDiscover() {
    int sock = broadcastSocket.getSockNumber();
    struct message message {};
    size_t length;
    struct sockaddr *address;

    address = (struct sockaddr *) broadcastSocket.getSockaddrPtr();

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

void RadioClient::receiveIam(struct sockaddr *address, socklen_t addressSize,
                             struct message *message) {
    std::string name(message->buffer);
    int position = serverLookup(address);

    if (position == -1) {
        Server server(address, addressSize, name);
        servers.push_back(server);
        position = servers.size() - 1;
    }
    // TODO: sprawdzic czy mamy serwer
    if (!isServer) {
        currentServer = servers[position];
        std::thread thread(&RadioClient::sendKeepAlive, this);
        thread.detach();
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
    while (!finish) {
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

    while (!finish) {
        std::memset(&address, 0, sizeof(struct sockaddr));
        addressSize = sizeof(address);
        recvLength = recvfrom(sock, &message, sizeof(message),
                    0, &address, &addressSize);
        if (recvLength < 0) {
            syserr("recvfrom");
        }

        message.type = ntohs(message.type);
        message.length = ntohs(message.length);

        if (message.type == 2) {
            receiveIam(&address, addressSize, &message);
        } else if (message.type == 4) {
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

bool finishTelnet = false;
const char *KEY_UP = "\033[A";
const char *KEY_DOWN = "\033[B";
const char *ENTER = "\n";
void RadioClient::menageTelnet(int sock) {
    int telnetSock;
    char buffer[128];
    ssize_t recvLength;
    bool up, down, enter;
    up = down = enter = false;

    if ((telnetSock = accept(sock, (struct sockaddr *) 0, (socklen_t *) 0)) < 0) {
        syserr("accept");
    }

    if (write(telnetSock,"\377\375\042\377\373\001",6) != 6) {
        syserr("write");
    }

    if (write(telnetSock, "\u001B[2J", 4) != 4) {
        syserr("write");
    }

    while (!finishTelnet) {

        memset(buffer, 0, 128);
        if ((recvLength = read(telnetSock, buffer, 128)) < 0) {
            syserr("read");
        }

        if (strcmp(buffer, KEY_UP) == 0) {
            std::cout << "JOOOOL" << std::endl;
        }

        if (strcmp(buffer, KEY_DOWN) == 0) {
            std::cout << "JOOOOL" << std::endl;
        }

        if (strcmp(buffer, ENTER) == 0) {
            std::cout << "ENTER" << std::endl;
        }

        write(1, buffer, recvLength);
    }
}

void RadioClient::start() {
    broadcastSocket.openSocket(udpPort, host);
    tcpSocket.openSocketForTelnet(tcpPort);
    menageTelnet(tcpSocket.getSockNumber());
//    sendDiscover();
//    receiveData();
//    servers.clear();
    broadcastSocket.closeSocket();
}

static void handleSignal(__attribute__((unused)) int signal) {
    finish = true;
    finishTelnet = true;
    // TODO: close sockets itp
    exit(0);
}

int main(int argc, char **argv) {
    std::signal(SIGINT, handleSignal);
    RadioClient radioClient(argc, argv);
    radioClient.start();
}

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
                this->tcpPort = optarg;
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

bool RadioClient::receiveIam() {
    int sock = broadcastSocket.getSockNumber();
    struct message message {};
    struct sockaddr address;
    socklen_t addressSize;
    ssize_t recvLength;

    // TODO: jakas lista dostepnych i opcja wybierania
    std::memset(&address, 0, sizeof(struct sockaddr));
    addressSize = sizeof(address);
    recvLength = recvfrom(sock, &message, sizeof(message),
                0, &address, &addressSize);
    if (recvLength < 0) {
        syserr("recvfrom");
    }

    message.type = ntohs(message.type);
    message.length = ntohs(message.length);
    std::cout << message.type << std::endl;
    if (message.type == 2) {
        std::cout << "JOL" << std::endl;
        Server server(&address, addressSize, "jol", 3);
        servers.push_back(server);
        this->currentServer = 0;
        return true;
    }

    return false;
}

void RadioClient::sendKeepAlive() {
    int sock = broadcastSocket.getSockNumber();
    struct sockaddr *address = servers[currentServer].getPtrToAddress();
    socklen_t addressSize = servers[currentServer].getAddressSize();
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
    struct sockaddr *address = servers[currentServer].getPtrToAddress();
    socklen_t addressSize = servers[currentServer].getAddressSize();
    struct message message {};

    while (!finish) {
        recvLength = recvfrom(sock, &message, sizeof(message),
                    0, address, &addressSize);
        if (recvLength < 0) {
            syserr("recvfrom");
        }

        message.type = ntohs(message.type);
        message.length = ntohs(message.length);

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

void RadioClient::start() {
    broadcastSocket.openSocket(udpPort, host);
    sendDiscover();
    if (receiveIam()) {
        std::thread thread(&RadioClient::sendKeepAlive, this);
        thread.detach(); // TODO: ??
        receiveData();
    }

    servers.clear();
    broadcastSocket.closeSocket();
}

static void handleSignal(__attribute__((unused)) int signal) {
    finish = true;
    // TODO: close sockets itp
    exit(0);
}

int main(int argc, char **argv) {
    std::signal(SIGINT, handleSignal);
    RadioClient radioClient(argc, argv);
    radioClient.start();
}

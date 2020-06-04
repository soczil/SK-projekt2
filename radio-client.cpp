#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>
#include <chrono>
#include "radio-client.h"
#include "message.h"
#include "err.h"

const int finish = false;

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

void RadioClient::sendKeepAlive() {
    int sock = broadcastSocket.getSockNumber();
    struct message message {};
    size_t length;

    message.type = htons(3);
    message.length = htons(0);

    length = sizeof(message);
    while (!finish) {
        std::this_thread::sleep_for(std::chrono::milliseconds(3500));
        if (sendto(sock, &message, length, 0,
                &proxyAddress, proxyAddressSize) != length) {
            syserr("partial / failed sendto");
        }
    }
}

void RadioClient::start() {
    broadcastSocket.openSocket(udpPort, host);
}

int main(int argc, char **argv) {
    RadioClient radioClient(argc, argv);
    std::cout << "JOL" << std::endl;
}

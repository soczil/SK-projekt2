#include <iostream>
#include <unistd.h>
#include "radio-client.h"
#include "err.h"

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
                this->udpPort = optarg;
                break;
            case 'p':
                p = true;
                this->tcpPort = optarg;
                break;
            case 'T':
                char *end;
                this->timeout = strtoul(optarg, &end, 10);
                if ((errno != 0) || (end == optarg) || (*end != '\0')) {
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

int main(int argc, char **argv) {
    RadioClient radioClient(argc, argv);
    std::cout << "JOL" << std::endl;
}

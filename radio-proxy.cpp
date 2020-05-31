#include <iostream>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include "radio-proxy.h"
#include "err.h"

RadioProxy::RadioProxy(int argc, char *argv[]) {
    int opt;
    bool h, r, p;

    h = r = p = false;
    while ((opt = getopt(argc, argv, "h:r:p:m:t:")) != -1) {
        switch (opt) {
            case 'h':
                h = true;
                this->host = optarg;
                break;
            case 'r':
                r = true;
                this->resource = optarg;
                break;
            case 'p':
                p = true;
                this->port = optarg;
                break;
            case 'm':
                if (strcmp(optarg, "yes") == 0) {
                    this->metadata = true;
                } else if (strcmp(optarg, "no") == 0) {
                    this->metadata = false;
                } else {
                    fatal("metadata option");
                }
                break;
            case 't':
                char *end;
                this->timeout = strtoul(optarg, &end, 10);
                if ((errno != 0) || (end == optarg) || (*end != '\0')) {
                    fatal("timeout option");
                }
                break;
            default:
                fatal("unknown option");
        }
    }

    if (!h || !r || !p) {
        fatal("obligatory arguments were not provided");
    }
}

void RadioProxy::printRadioProxy() {
    std::cout << "host: " << this->host << std::endl;
    std::cout << "resource: " << this->resource << std::endl;
    std::cout << "port: " << this->port << std::endl;
    std::cout << "metadata: " << this->metadata << std::endl;
    std::cout << "timeout: " << this->timeout << std::endl;
}

void RadioProxy::connect() {
    socket.openSocket(host, port);
}

void RadioProxy::disconnect() {
    socket.closeSocket();
}

void RadioProxy::sendRequest() {
    std::ostringstream request;

    request << "GET " << resource << " HTTP/1.0\r\n";
    request << "Host: " << host << "\r\n";
    request << "User-Agent: Casty\r\n";
    request << "Icy-MetaData: " << (metadata ? "1" : "0") << "\r\n\r\n";

    std::cout << request.str().c_str();
    socket.writeToSocket(request.str());
}

void RadioProxy::readResponse() {
    socket.readFromSocket();
}

int main(int argc, char *argv[]) {
    std::cout << "JOL" << std::endl;
    RadioProxy radioClient(argc, argv);
    radioClient.printRadioProxy();
    radioClient.connect();
    radioClient.sendRequest();
    radioClient.readResponse();
    radioClient.disconnect();
}

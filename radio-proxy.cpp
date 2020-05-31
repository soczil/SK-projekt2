#include <iostream>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include <regex>
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
    ssize_t recvLength = 0;
    char *line = nullptr;
    size_t size = 0;
    int metaint = 0, sum = 0, difference = 0, metaLength = 0;
    std::cmatch cm;

    const std::regex correctStatus("(?:ICY 200 OK\r\n)|(?:HTTP\\/1\\.[0-1] "
                                   "200 OK\r\n)|(?:HTTP 200 OK\r\n)");
    const std::regex metaintRegex("(?:icy-metaint:([0-9]+)\r\n)",
            std::regex_constants::icase);

    FILE *fd = fdopen(socket.getSockNumber(), "r");

    recvLength = getline(&line, &size, fd);
    if (!std::regex_match(line, correctStatus)) {
        std::cout << line << std::endl;
        free(line);
        return;
    }

    while (((recvLength = getline(&line, &size, fd)) != -1)
            && (strcmp(line, "\r\n") != 0)) {
        if (std::regex_match(line, cm, metaintRegex)) {
            metaint = std::stoi(cm[1].str());
        }
    }

    while ((recvLength = getline(&line, &size, fd)) != -1) {
        if (sum + recvLength <= metaint) {
            sum += recvLength;
        } else {
            // std::cout << "sum1: " << sum << std::endl;
            difference = metaint - sum;
            metaLength = ((int) *(line + difference)) * 16;
            std::cout << metaLength << std::endl;
            if (metaLength > 0) {
                std::cout << (line + difference + 1) << std::endl;
            }
            sum = (int)recvLength - difference - metaLength - 1;
            // std::cout << "recvLength: " << recvLength << std::endl;
            // std::cout << "sum: " << sum << std::endl;
            //  std::cout << "difference: " << difference << std::endl;
        }
        //std::cout << "JOL" << std::endl;
    }

    free(line);
    if (fclose(fd) < 0) {
        syserr("fclose");
    }
}

int main(int argc, char *argv[]) {
    std::cout << "JOL" << std::endl;
    RadioProxy radioClient(argc, argv);
    radioClient.printRadioProxy();
    radioClient.connect();
    radioClient.sendRequest();
    radioClient.readResponse();
    //radioClient.disconnect();
}

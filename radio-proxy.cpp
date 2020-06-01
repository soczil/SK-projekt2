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
    char buffer[8192];
    ssize_t recvLength = 0;
    char *line = nullptr;
    size_t size = 0;
    int metaint = 0, sum = 0, difference = 0, metaLength = -1, position = 0;
    std::cmatch cm;
    int sock = socket.getSockNumber();

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
            std::cout << "METAINT" << metaint << std::endl;
        }
        std::cout << line << std::endl;
    }

    int allData = 0;
    int dataLeft = metaint;
    int metadataLeft = 0;
    bool readMetadata = false;

    while (true) {
        if (allData == 0) {
            //memset(buffer, 0, sizeof(buffer));
            //recvLength = read(sock, buffer, sizeof(buffer) - 1);
            recvLength = getline(&line, &size, fd);
            //std::cout << line << std::endl;
            if (recvLength == 0) {
                break;
            } else if (recvLength < 0) {
                syserr("read");
            }
            //std::cout << recvLength << std::endl;
            allData = recvLength;
        }

        if (!readMetadata) {
            if (dataLeft >= allData) {
                if (dataLeft == allData) {
                    readMetadata = true;
                }
                dataLeft -= allData;
                allData = 0;
            } else {
                allData -= dataLeft;
                dataLeft = 0;
                readMetadata = true;
            }
        } else {
            position = (int)recvLength - allData;
            if (metaLength == -1) {
                //std::cout << "ZNAK " << *(line + position) << std::endl;
                //std::cout << line << std::endl;
                metaLength = ((int) *(line + position)) * 16;
                //std::cout << "META LENGTH: " << metaLength << std::endl;
                metadataLeft = metaLength;
                allData--;
//                if (allData == 0) {
//                    continue;
//                }
//                position++;
//                std::cout << "ALL DATA: " << allData << std::endl;
//                std::cout << "DATA LEFT: " << dataLeft << std::endl;
//                std::cout << "METADATA READ: " << readMetadata << std::endl;
//                std::cout << "position " << position << std::endl;
                continue;
            }

            if (metadataLeft >= allData) {
                if (metadataLeft == allData) {
                    readMetadata = false;
                    metaLength = -1;
                    dataLeft = metaint;
                }
                metadataLeft -= allData;
                allData = 0;
                if (metaLength > 0) {
                    std::cout << (line + position) << std::endl;
                }
            } else {
                char sign = *(line + position + metadataLeft);
                *(line + position + metadataLeft) = '\0';
                if (metaLength > 0) {
                    std::cout << (line + position) << std::endl;
                }
                *(line + position + metadataLeft) = sign;
                allData -= metadataLeft;
                metadataLeft = 0;
                readMetadata = false;
                metaLength = -1;
                dataLeft = metaint;
            }
//            std::cout << "ALL DATA: " << allData << std::endl;
//            std::cout << "DATA LEFT: " << dataLeft << std::endl;
//            std::cout << "METADATA LEFT: " << metadataLeft << std::endl;
//            std::cout << "METADATA READ: " << readMetadata << std::endl;
        }
//        std::cout << "ALL DATA: " << allData << std::endl;
//        std::cout << "DATA LEFT: " << dataLeft << std::endl;
//        std::cout << "METADATA READ: " << readMetadata << std::endl;
    }

//    while ((recvLength = getline(&line, &size, fd)) != -1) {
//        if (sum + recvLength <= metaint) {
//            sum += recvLength;
//        } else {
//            do {
//                //std::cout << "sum1: " << sum << std::endl;
//                difference = metaint - sum;
//                metaLength = ((int) *(line + difference)) * 16;
//                std::cout << metaLength << std::endl;
//                if (metaLength > 0) {
//                    std::cout << (line + difference + 1) << std::endl;
//                }
//                sum = (int)recvLength - difference - metaLength - 1;
//                //std::cout << "recvLength: " << recvLength << std::endl;
//                //std::cout << "sum: " << sum << std::endl;
//                //std::cout << "difference: " << difference << std::endl;
//            } while (sum > metaint);
//        }
//        //std::cout << "JOL" << std::endl;
//    }

    free(line);
    if (fclose(fd) < 0) {
        syserr("fclose");
    }
}

const int HEADER_SIZE = 8192;

void RadioProxy::checkHeader(char *header, int *metaInt) {
    const std::regex correctStatus("(?:ICY 200 OK\r)|(?:HTTP\\/1\\.[0-1] 200 OK\r)|(?:HTTP 200 OK\r)");
    const std::regex metaIntRegex("\r\n(?:icy-metaint:([0-9]+))\r\n",
            std::regex_constants::icase);
    char *newLine = nullptr;
    std::cmatch cm;

    newLine = strchr(header, '\n');
    *newLine = '\0';
    if (!std::regex_match(header, correctStatus)) {
        std::cout << header << std::endl;
        return;
    }
    *newLine = '\n';

    if (std::regex_search(header, cm, metaIntRegex)) {
        *metaInt = std::stoi(cm[1].str());
    }
}

void RadioProxy::readResponse2() {
    int sock = socket.getSockNumber(), position = 0;
    char buffer[HEADER_SIZE + 1];
    char *headerBuffer = nullptr, *headerEnd = nullptr;
    size_t headerSize = 0, headerBufferSize = 0;
    ssize_t recvLength = 0;
    int metaInt = -1, allData, dataLeft, metadataLeft, metaLength;
    bool readMetadata;

    do {
        memset(buffer, 0, sizeof(buffer));
        recvLength = read(sock, buffer, sizeof(buffer) - 1);
        if (recvLength == 0) {
            fatal("could not get correct header");
        } else if (recvLength < 0) {
            syserr("read");
        }

        headerSize += recvLength;
        if (headerBufferSize < (headerSize + 1)) {
            headerBufferSize = (2 * headerSize) + 1;
            headerBuffer = (char *) realloc(headerBuffer, headerBufferSize * sizeof(char));
        }

        if (headerBuffer == nullptr) {
            syserr("realloc");
        }

        headerBuffer[position] = '\0';
        strcat(headerBuffer, buffer);
        position = headerSize;
    } while ((headerEnd = strstr(buffer, "\r\n\r\n")) == nullptr);
    checkHeader(headerBuffer, &metaInt);
    free(headerBuffer);
    headerEnd += 4;

    allData = recvLength - (headerEnd - buffer);
    dataLeft = metaInt;
    metadataLeft = 0;
    readMetadata = false;
    metaLength = -1;
    while (true) {
        if (allData == 0) {
            memset(buffer, 0, sizeof(buffer));
            recvLength = read(sock, buffer, sizeof(buffer) - 1);
            if (recvLength == 0) {
                break;
            } else if (recvLength < 0) {
                syserr("read");
            }
            allData = recvLength;
        }

        position = (int) recvLength - allData;
        if (!readMetadata) {
            if (dataLeft >= allData) {
                if (dataLeft == allData) {
                    readMetadata = true;
                }
                dataLeft -= allData;
                allData = 0;
            } else {
                allData -= dataLeft;
                dataLeft = 0;
                readMetadata = true;
            }
        } else {
            if (metaLength == -1) {
                metaLength = ((int) *(buffer + position)) * 16;
                metadataLeft = metaLength;
                allData--;
                continue;
            }

            if (metadataLeft >= allData) {
                if (metadataLeft == allData) {
                    readMetadata = false;
                    metaLength = -1;
                    dataLeft = metaInt;
                }
                metadataLeft -= allData;
                allData = 0;
                if (metaLength > 0) {
                    std::cout << (buffer + position) << std::endl;
                }
            } else {
                char sign = *(buffer + position + metadataLeft);
                *(buffer + position + metadataLeft) = '\0';
                if (metaLength > 0) {
                    std::cout << (buffer + position) << std::endl;
                }
                *(buffer + position + metadataLeft) = sign;
                allData -= metadataLeft;
                metadataLeft = 0;
                readMetadata = false;
                metaLength = -1;
                dataLeft = metaInt;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    std::cout << "JOL" << std::endl;
    RadioProxy radioClient(argc, argv);
    radioClient.printRadioProxy();
    radioClient.connect();
    radioClient.sendRequest();
    radioClient.readResponse2();
    radioClient.disconnect();
}

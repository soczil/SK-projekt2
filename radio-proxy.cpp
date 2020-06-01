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

    socket.writeToSocket(request.str());
}

const int HEADER_SIZE = 100;
const int BUFFER_SIZE = 100;

bool RadioProxy::correctHeader(char *header, int *metaInt) {
    const std::regex correctStatus("(?:ICY 200 OK\r)|(?:HTTP\\/1\\.[0-1] 200 OK\r)|(?:HTTP 200 OK\r)");
    const std::regex metaIntRegex("\r\n(?:icy-metaint:([0-9]+))\r\n",
            std::regex_constants::icase);
    char *newLine = nullptr;
    std::cmatch cm;

    newLine = strchr(header, '\n');
    *newLine = '\0';
    if (!std::regex_match(header, correctStatus)) {
        std::cout << header << std::endl;
        return false;
    }
    *newLine = '\n';

    if (std::regex_search(header, cm, metaIntRegex)) {
        *metaInt = std::stoi(cm[1].str());
    }

    return true;
}

bool RadioProxy::readHeader(char *buffer, int *metaInt, std::pair<int, int>& content) {
    bool status;
    int sock = socket.getSockNumber(), headerEnd = 0;
    char *header = nullptr, *contentStart;
    size_t headerSize = 0, sum = 0;
    ssize_t recvLength = 0;

    do {
        memset(buffer, 0, BUFFER_SIZE);
        recvLength = read(sock, buffer, BUFFER_SIZE - 1);
        if (recvLength == 0) {
            fatal("could not get correct header");
        } else if (recvLength < 0) {
            syserr("read");
        }

        sum += recvLength;
        if (headerSize < (sum + 1)) {
            headerSize = (2 * sum) + 1;
            header = (char *) realloc(header, headerSize * sizeof(char));
        }

        if (header == nullptr) {
            syserr("realloc");
        }

        header[headerEnd] = '\0';
        strcat(header, buffer);
        headerEnd = sum;
    } while (strstr(header, "\r\n\r\n") == nullptr);

    status = correctHeader(header, metaInt);
    //std::cout << header << std::endl; // TODO: remove
    free(header);

    if ((contentStart = strstr(buffer, "\r\n\r\n")) != nullptr) {
        contentStart += 4;
    } else if ((contentStart = strstr(buffer, "\r\n")) != nullptr) {
        contentStart += 2;
    } else {
        contentStart += 1;
    }
    content.first = contentStart - buffer;
    content.second = recvLength - content.first;

    return status;
}

static void writeToStdout(char *buffer, size_t size) {
    if (write(1, buffer, size) != (ssize_t) size) {
        syserr("partial / failed write");
    }
}

void RadioProxy::readWithoutMetadata(char *buffer, std::pair<int, int>&
        content) {
    int sock = socket.getSockNumber();
    ssize_t recvLength = 0;

    writeToStdout(buffer + content.first, content.second);
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        recvLength = read(sock, buffer, BUFFER_SIZE);
        if (recvLength == 0) {
            break;
        } else if (recvLength < 0) {
            syserr("read");
        }
        writeToStdout(buffer, recvLength);
    }
}

void RadioProxy::readResponse() {
    int metaInt = -1;
    char buffer[BUFFER_SIZE];
    std::pair<int, int> content(std::make_pair(0, 0));

    if (!readHeader(buffer, &metaInt, content)) {
        return;
    }

    if (!metadata && (metaInt != -1)) {
        fatal("server tries to send metadata");
    }

    if (metaInt == -1) {
        readWithoutMetadata(buffer, content);
    }
}

//void RadioProxy::readResponse() {
//    int sock = socket.getSockNumber(), position = 0;
//    char buffer[HEADER_SIZE + 1];
//    char *headerBuffer = nullptr, *headerEnd = nullptr;
//    size_t headerSize = 0, headerBufferSize = 0;
//    ssize_t recvLength = 0;
//    int metaInt = -1, allData, dataLeft, metadataLeft, metaLength;
//    bool readMetadata;
//
//    do {
//        memset(buffer, 0, sizeof(buffer));
//        recvLength = read(sock, buffer, sizeof(buffer) - 1);
//        if (recvLength == 0) {
//            fatal("could not get correct header");
//        } else if (recvLength < 0) {
//            syserr("read");
//        }
//
//        headerSize += recvLength;
//        if (headerBufferSize < (headerSize + 1)) {
//            headerBufferSize = (2 * headerSize) + 1;
//            headerBuffer = (char *) realloc(headerBuffer, headerBufferSize * sizeof(char));
//        }
//
//        if (headerBuffer == nullptr) {
//            syserr("realloc");
//        }
//
//        headerBuffer[position] = '\0';
//        strcat(headerBuffer, buffer);
//        position = headerSize;
//    } while ((headerEnd = strstr(buffer, "\r\n\r\n")) == nullptr);
//    checkHeader(headerBuffer, &metaInt);
//    free(headerBuffer);
//    headerEnd += 4;
//
//    if ((metaInt != -1) && !metadata) {
//        fatal("server tries to send metadata");
//    } else if (metaInt == -1) {
//        // Print the rest of the buffer that contains message.
//        size_t size = 100 - (headerEnd - buffer);
//        write(1, buffer + (headerEnd - buffer), size);
//        //fprintf(stdout, "%s", buffer + (headerEnd - buffer));
//        readWithoutMetadata();
//        return;
//    }
//
//    allData = recvLength - (headerEnd - buffer);
//    dataLeft = metaInt;
//    metadataLeft = 0;
//    readMetadata = false;
//    metaLength = -1;
//    while (true) {
//        if (allData == 0) {
//            memset(buffer, 0, sizeof(buffer));
//            recvLength = read(sock, buffer, sizeof(buffer) - 1);
//            if (recvLength == 0) {
//                break;
//            } else if (recvLength < 0) {
//                syserr("read");
//            }
//            allData = recvLength;
//        }
//
//        position = (int) recvLength - allData;
//        if (!readMetadata) {
//            if (dataLeft >= allData) {
//                if (dataLeft == allData) {
//                    readMetadata = true;
//                }
//                write(1, buffer + position, allData);
//                dataLeft -= allData;
//                allData = 0;
//                //fprintf(stdout, "%s", buffer + position);
//                //std::cout << (buffer + position) << std::endl;
//            } else {
////                char sign = *(buffer + position + dataLeft);
////                *(buffer + position + dataLeft) = '\0';
////                fprintf(stdout, "%s", buffer + position);
//                write(1, buffer + position, dataLeft);
//                //std::cout << (buffer + position) << std::endl;
//                //*(buffer + position + dataLeft) = sign;
//                allData -= dataLeft;
//                dataLeft = 0;
//                readMetadata = true;
//            }
//        } else {
//            if (metaLength == -1) {
//                metaLength = ((int) *(buffer + position)) * 16;
//                metadataLeft = metaLength;
//                allData--;
//                continue;
//            }
//
//            if (metadataLeft >= allData) {
//                if (metadataLeft == allData) {
//                    readMetadata = false;
//                    metaLength = -1;
//                    dataLeft = metaInt;
//                }
//                metadataLeft -= allData;
//                allData = 0;
//                if (metaLength > 0) {
//                    //std::cout << (buffer + position) << std::endl;
//                    fprintf(stderr, "%s\n", buffer + position);
//                }
//            } else {
//                char sign = *(buffer + position + metadataLeft);
//                *(buffer + position + metadataLeft) = '\0';
//                if (metaLength > 0) {
//                    //std::cout << (buffer + position) << std::endl;
//                    fprintf(stderr, "%s\n", buffer + position);
//                }
//                *(buffer + position + metadataLeft) = sign;
//                allData -= metadataLeft;
//                metadataLeft = 0;
//                readMetadata = false;
//                metaLength = -1;
//                dataLeft = metaInt;
//            }
//        }
//    }
//}

//void RadioProxy::readWithoutMetadata() {
//    int sock = socket.getSockNumber();
//    char buffer[HEADER_SIZE];
//    ssize_t recvLength;
//
//    while (true) {
//        memset(buffer, 0, sizeof(buffer));
//        recvLength = read(sock, buffer, sizeof(buffer) - 1);
//        if (recvLength == 0) {
//            break;
//        } else if (recvLength < 0) {
//            syserr("read");
//        }
//        //fprintf(stdout, "%s", buffer);
//        write(1, buffer, recvLength);
//    }
//}

int main(int argc, char *argv[]) {
    RadioProxy radioClient(argc, argv);
    radioClient.connect();
    radioClient.sendRequest();
    radioClient.readResponse();
    radioClient.disconnect();
}

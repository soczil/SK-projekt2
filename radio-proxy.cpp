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

//bool RadioProxy::correctHeader(char *header, int *metaInt) {
//    const std::regex correctStatus("(?:ICY 200 OK\r)|(?:HTTP\\/1\\.[0-1] 200 OK\r)|(?:HTTP 200 OK\r)");
//    const std::regex metaIntRegex("\r\n(?:icy-metaint:([0-9]+))\r\n",
//            std::regex_constants::icase);
//    char *newLine = nullptr;
//    std::cmatch cm;
//
//    newLine = strchr(header, '\n');
//    *newLine = '\0';
//    if (!std::regex_match(header, correctStatus)) {
//        std::cout << header << std::endl;
//        return false;
//    }
//    *newLine = '\n';
//
//    if (std::regex_search(header, cm, metaIntRegex)) {
//        *metaInt = std::stoi(cm[1].str());
//    }
//
//    return true;
//}

//bool RadioProxy::readHeader(char *buffer, int *metaInt, std::pair<int, int>& content) {
//    bool status;
//    int sock = socket.getSockNumber(), headerEnd = 0;
//    char *header = nullptr, *contentStart;
//    size_t headerSize = 0, sum = 0;
//    ssize_t recvLength = 0;
//
//    do {
//        memset(buffer, 0, BUFFER_SIZE);
//        recvLength = read(sock, buffer, BUFFER_SIZE - 1);
//        if (recvLength == 0) {
//            fatal("could not get correct header");
//        } else if (recvLength < 0) {
//            syserr("read");
//        }
//
//        sum += recvLength;
//        if (headerSize < (sum + 1)) {
//            headerSize = (2 * sum) + 1;
//            header = (char *) realloc(header, headerSize * sizeof(char));
//        }
//
//        if (header == nullptr) {
//            syserr("realloc");
//        }
//
//        header[headerEnd] = '\0';
//        strcat(header, buffer);
//        headerEnd = sum;
//    } while (strstr(header, "\r\n\r\n") == nullptr);
//
//    status = correctHeader(header, metaInt);
//    //std::cout << header << std::endl; // TODO: remove
//    free(header);
//
//    if ((contentStart = strstr(buffer, "\r\n\r\n")) != nullptr) {
//        contentStart += 4;
//    } else if ((contentStart = strstr(buffer, "\r\n")) != nullptr) {
//        contentStart += 2;
//    } else {
//        contentStart += 1;
//    }
//    content.first = contentStart - buffer;
//    content.second = recvLength - content.first;
//
//    return status;
//}

static void writeToStdout(char *buffer, size_t size) {
    if (write(1, buffer, size) != (ssize_t) size) {
        syserr("partial / failed write to stdout");
    }
}

static void writeToStderr(char *buffer, size_t size) {
    if (write(2, buffer, size) != (ssize_t) size) {
        syserr("partial / failed write to stderr");
    }
}

//void RadioProxy::readWithoutMetadata(char *buffer, std::pair<int, int>&
//        content) {
//    int sock = socket.getSockNumber();
//    ssize_t recvLength = 0;
//
//    writeToStdout(buffer + content.first, content.second);
//    while (true) {
//        memset(buffer, 0, BUFFER_SIZE);
//        recvLength = read(sock, buffer, BUFFER_SIZE);
//        if (recvLength == 0) {
//            break;
//        } else if (recvLength < 0) {
//            syserr("read");
//        }
//        writeToStdout(buffer, recvLength);
//    }
//}
//
//void RadioProxy::readWithMetadata(char *buffer, int metaInt, std::pair<int,
//        int>& content) {
//    int sock = socket.getSockNumber();
//    int allData, dataLeft, metadataLeft = 0, metaLength = -1, position;
//    bool readMetadata = false;
//    ssize_t recvLength = 0;
//
//    allData = content.second;
//    dataLeft = metaInt;
//    position = content.first;
//    while (true) {
//        if (allData == 0) {
//            memset(buffer, 0, BUFFER_SIZE);
//            recvLength = read(sock, buffer, BUFFER_SIZE);
//            if (recvLength == 0) {
//                break;
//            } else if (recvLength < 0) {
//                syserr("read");
//            }
//            allData = recvLength;
//            position = 0;
//        }
//
//        if (!readMetadata) {
//            if (dataLeft >= allData) {
//                if (dataLeft == allData) {
//                    readMetadata = true;
//                }
//                writeToStdout(buffer + position, allData);
//                dataLeft -= allData;
//                allData = 0;
//            } else {
//                writeToStdout(buffer + position, dataLeft);
//                position += dataLeft;
//                allData -= dataLeft;
//                dataLeft = 0;
//                readMetadata = true;
//            }
//        } else {
//            if (metaLength == -1) {
//                metaLength = ((int) *(buffer + position)) * 16;
//                std::cerr << metaLength << std::endl;
//                metadataLeft = metaLength;
//                allData--;
//                position++;
//            }
//
//            if (metadataLeft >= allData) {
//                if (metaLength > 0) {
//                    writeToStderr(buffer + position, allData);
//                }
//                if (metadataLeft == allData) {
//                    readMetadata = false;
//                    metaLength = -1;
//                    dataLeft = metaInt;
//                }
//                metadataLeft -= allData;
//                allData = 0;
//            } else {
//                if (metaLength > 0) {
//                    writeToStderr(buffer + position, metadataLeft);
//                }
//                position += metadataLeft;
//                allData -= metadataLeft;
//                metadataLeft = 0;
//                readMetadata = false;
//                metaLength = -1;
//                dataLeft = metaInt;
//            }
//        }
//    }
//}

//void RadioProxy::readResponse() {
//    int metaInt = -1;
//    char buffer[BUFFER_SIZE];
//    std::pair<int, int> content(std::make_pair(0, 0));
//
//    if (!readHeader(buffer, &metaInt, content)) {
//        return;
//    }
//
//    if (!metadata && (metaInt != -1)) {
//        fatal("server tries to send metadata");
//    }
//
//    if (metaInt == -1) {
//        readWithoutMetadata(buffer, content);
//    } else {
//        readWithMetadata(buffer, metaInt, content);
//    }
//}

bool RadioProxy::correctHeader(std::vector<char> &header, int &metaInt) {
    const std::regex correctStatus("(?:ICY 200 OK\r)|(?:HTTP\\/1\\.[0-1] 200 OK\r)|(?:HTTP 200 OK\r)");
    const std::regex metaIntRegex("\r\n(?:icy-metaint:([0-9]+))\r\n", std::regex_constants::icase);
    auto newLine = std::find(header.begin(), header.end(), '\n');

    if (!std::regex_match(header.begin(), newLine, correctStatus)) {
        std::string statusLine(header.begin(), newLine);
        std::cout << statusLine << std::endl;
        return false;
    }

    std::smatch sm;
    std::string headerString(header.begin(), header.end());
    if (std::regex_search(headerString, sm, metaIntRegex)) {
        metaInt = std::stoi(sm[1].str());
    }

    return true;
}

bool RadioProxy::readHeader(char *buffer, int &metaInt, std::pair<int, int> &restOfContent) {
    int sock = socket.getSockNumber();
    const char *crlf = "\r\n\r\n", *contentStart = nullptr;
    std::vector<char> header;
    ssize_t recvLength = 0;

    do {
        memset(buffer, 0, BUFFER_SIZE);
        recvLength = read(sock, buffer, BUFFER_SIZE);
        if (recvLength == 0) {
            fatal("could not get correct header");
        } else if (recvLength < 0) {
            syserr("read");
        }

        header.insert(header.end(), buffer, buffer + recvLength);
    } while (std::search(header.begin(), header.end(), crlf, crlf + 4) == header.end());

    if ((contentStart = strstr(buffer, "\r\n\r\n")) != nullptr) {
        contentStart += 4;
    } else if ((contentStart = strstr(buffer, "\r\n")) != nullptr) {
        contentStart += 2;
    } else {
        contentStart += 1;
    }

    restOfContent.first = contentStart - buffer;
    restOfContent.second = recvLength - restOfContent.first;

    return correctHeader(header, metaInt);;
}

void RadioProxy::readWithoutMetadata(char *buffer, std::pair<int, int> &restOfContent) {
    int sock = socket.getSockNumber();
    ssize_t recvLength = 0;

    writeToStdout(buffer + restOfContent.first, restOfContent.second);
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

void RadioProxy::readWithMetadata(char *buffer, int metaInt, std::pair<int, int> &restOfContent) {
    int sock = socket.getSockNumber(), i;
    int dataPosition, metadataPosition, position = restOfContent.first,
    metaLength;
    char dataBuffer[BUFFER_SIZE], metadataBuffer[BUFFER_SIZE];
    ssize_t recvLength = restOfContent.first + restOfContent.second;

    while (true) {
        dataPosition = 0;
        for (i = 0; i < metaInt; i++) {
            if (position == recvLength) {
                memset(buffer, 0, BUFFER_SIZE);
                recvLength = read(sock, buffer, BUFFER_SIZE);
                if (recvLength == 0) {
                    return;
                } else if (recvLength < 0) {
                    syserr("read");
                }
                position = 0;
            }

            if (dataPosition == BUFFER_SIZE) {
                writeToStdout(dataBuffer, dataPosition);
                dataPosition = 0;
            }

            dataBuffer[dataPosition] = buffer[position];
            dataPosition++;
            position++;
        }
        writeToStdout(dataBuffer, dataPosition);

        if (position == recvLength) {
            memset(buffer, 0, BUFFER_SIZE);
            recvLength = read(sock, buffer, BUFFER_SIZE);
            if (recvLength == 0) {
                return;
            } else if (recvLength < 0) {
                syserr("read");
            }
            position = 0;
        }
        metaLength = (int) (buffer[position] * 16);
        std::cerr << metaLength << std::endl;
        position++;

        metadataPosition = 0;
        for (i = 0; i < metaLength; i++) {
            if (position == recvLength) {
                memset(buffer, 0, BUFFER_SIZE);
                recvLength = read(sock, buffer, BUFFER_SIZE);
                if (recvLength == 0) {
                    return;
                } else if (recvLength < 0) {
                    syserr("read");
                }
                position = 0;
            }

            if (metadataPosition == BUFFER_SIZE) {
                writeToStderr(metadataBuffer, metadataPosition);
                metadataPosition = 0;
            }

            metadataBuffer[metadataPosition] = buffer[position];
            metadataPosition++;
            position++;
        }
        if (metaLength > 0) {
            writeToStderr(metadataBuffer, metadataPosition);
        }
    }
}

void RadioProxy::readResponse() {
    int metaInt = -1;
    char buffer[BUFFER_SIZE];
    std::pair<int, int> restOfContent(std::make_pair(0, 0));

    if (!readHeader(buffer, metaInt, restOfContent)) {
        return;
    }

    if (metaInt == -1) {
        readWithoutMetadata(buffer, restOfContent);
    } else {
        readWithMetadata(buffer, metaInt, restOfContent);
    }

}

int main(int argc, char *argv[]) {
    RadioProxy radioClient(argc, argv);
    radioClient.connect();
    radioClient.sendRequest();
    radioClient.readResponse();
    radioClient.disconnect();
}

#include <iostream>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include <regex>
#include <arpa/inet.h>
#include "radio-proxy.h"
#include "err.h"

static unsigned parseToUnsigned(char *numberPtr) {
    char *end;
    unsigned number = strtoul(numberPtr, &end, 10);

    if ((errno != 0) || (end == numberPtr) || (*end != '\0')) {
        return 0;
    }

    return number;
}

RadioProxy::RadioProxy(int argc, char *argv[]) {
    int opt;
    bool h, r, p;

    h = r = p = false;
    while ((opt = getopt(argc, argv, "h:r:p:m:t:P:B:T:")) != -1) {
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
                this->timeout = parseToUnsigned(optarg);
                if (this->timeout == 0) {
                    fatal("t option");
                }
                break;
            case 'P':
                this->proxy = true;
                this->udpPort = parseToUnsigned(optarg);
                break;
            case 'B':
                this->multiAddress = optarg;
                break;
            case 'T':
                this->clientsTimeout = parseToUnsigned(optarg);
                if (this->clientsTimeout == 0) {
                    fatal("T option");
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
    tcpSocket.openSocket(host, port);

    if (proxy) {
        std::cout << "OTWIERAM GNIAZDO UDP" << std::endl;
        udpSocket.openSocket(udpPort, multiAddress);
    }
}

void RadioProxy::disconnect() {
    tcpSocket.closeSocket();
}

void RadioProxy::sendRequest() {
    std::ostringstream request;

    request << "GET " << resource << " HTTP/1.0\r\n";
    request << "Host: " << host << "\r\n";
    request << "User-Agent: Casty\r\n";
    request << "Icy-MetaData: " << (metadata ? "1" : "0") << "\r\n\r\n";

    tcpSocket.writeToSocket(request.str());
}

const int BUFFER_SIZE = 100;

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
    const char *crlf = "\r\n\r\n", *contentStart = nullptr;
    std::vector<char> header;
    ssize_t recvLength = 0;

    do {
        recvLength = tcpSocket.readFromSocket(buffer, BUFFER_SIZE);
        if (recvLength == 0) {
            fatal("could not get correct header");
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
    ssize_t recvLength = 0;

    writeToStdout(buffer + restOfContent.first, restOfContent.second);
    while (true) {
        recvLength = tcpSocket.readFromSocket(buffer, BUFFER_SIZE);
        if (recvLength == 0) {
            break;
        }
        writeToStdout(buffer, recvLength);
    }
}

bool RadioProxy::readBlock(int limit, int &position, char *buffer,
                           ssize_t &recvLength, bool regularData) {
    int dataPosition = 0;
    char dataBuffer[BUFFER_SIZE];

    for (int i = 0; i < limit; i++) {
        if (position == recvLength) {
            recvLength = tcpSocket.readFromSocket(buffer, BUFFER_SIZE);
            if (recvLength == 0) {
                return false;
            }
            position = 0;
        }

        if (dataPosition == BUFFER_SIZE) {
            if (regularData) {
                writeToStdout(dataBuffer, dataPosition);
            } else {
                writeToStderr(dataBuffer, dataPosition);
            }
            dataPosition = 0;
        }

        dataBuffer[dataPosition] = buffer[position];
        dataPosition++;
        position++;
    }

    if (dataPosition > 0) {
        if (regularData) {
            writeToStdout(dataBuffer, dataPosition);
        } else {
            writeToStderr(dataBuffer, dataPosition);
        }
    }

    return true;
}

void RadioProxy::readWithMetadata(char *buffer, int metaInt,
                                  std::pair<int, int> &restOfContent) {
    int position = restOfContent.first, metaLength;
    ssize_t recvLength = restOfContent.first + restOfContent.second;

    while (true) {
        if (!readBlock(metaInt, position, buffer, recvLength, true)) {
            return;
        }

        if (position == recvLength) {
            recvLength = tcpSocket.readFromSocket(buffer, BUFFER_SIZE);
            if (recvLength == 0) {
                return;
            }
            position = 0;
        }
        metaLength = (int) (buffer[position] * 16);
        position++;

        if (!readBlock(metaLength, position, buffer, recvLength, false)) {
            return;
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

    if ((metaInt != -1) && !metadata) {
        fatal("server tries to send metadata");
    }

    if (metaInt == -1) {
        readWithoutMetadata(buffer, restOfContent);
    } else {
        readWithMetadata(buffer, metaInt, restOfContent);
    }

}

int RadioProxy::clientLookup(struct sockaddr *seekedAddress) {
    // TODO: mutex
    auto *seekedIn = (struct sockaddr_in *) seekedAddress;
    struct sockaddr_in *clientIn;

    for (size_t i = 0; i < clients.size(); i++) {
        clientIn = (struct sockaddr_in *) clients[i].getPtrToAddress();
        if (seekedIn->sin_addr.s_addr == clientIn->sin_addr.s_addr
            && seekedIn->sin_port == clientIn->sin_port) {
            std::cout << "JOOOL" << std::endl;
            return i;
        }
    }

    return -1;
}

void RadioProxy::addNewClient(struct sockaddr *clientAddress) {
    Client newClient(clientAddress);
    clients.push_back(newClient);
}

void RadioProxy::discoverMessage(struct sockaddr *clientAddress,
                                 socklen_t addressSize) {
    int sock = udpSocket.getSockNumber();
    struct message message {};
    ssize_t sendLength;
    int position = clientLookup(clientAddress);

    if (position == -1) {
        addNewClient(clientAddress);
        position = clients.size() - 1;
    }

    message.type = htons(2);
    message.length = htons(0);

    sendLength = sendto(sock, &message, sizeof(message), 0,
                    clientAddress, addressSize);
    if (sendLength != (ssize_t) sizeof(message)) {
        syserr("partial / failed write");
    }

    clients[position].setLastVisit(time(nullptr));
}

void RadioProxy::keepaliveMessage(struct sockaddr *clientAddress) {
    int position = clientLookup(clientAddress);

    if (position == -1) {
        return;
    }

    clients[position].setLastVisit(time(nullptr));
}

void RadioProxy::handleClients() {
    int sock = udpSocket.getSockNumber();
    struct message message {};
    struct sockaddr clientAddress {};
    socklen_t addressSize = sizeof(clientAddress);
    ssize_t recvLength;

    std::memset(&message, 0, sizeof(struct message));
    std::memset(&clientAddress, 0, sizeof(struct sockaddr));

    while (true) {
        recvLength = recvfrom(sock, &message,  sizeof(struct message), 0,
                        &clientAddress, &addressSize);
        if (recvLength < 0) {
            syserr("recvfrom");
            break;
        }

        message.type = ntohs(message.type);
        message.length = ntohs(message.length);
        if (message.length != 0) {
            continue;
        }

        if (message.type == 1) { // DISCOVER
            std::cout << "DISCOVER" << std::endl;
            discoverMessage(&clientAddress, addressSize);
        } else if (message.type == 3) { // KEEPALIVE
            std::cout << "KEEPALIVE" << std::endl;
            keepaliveMessage(&clientAddress);
        } else {
            std::cout << "POMIJAM" << std::endl; // TODO: remove
            continue;
        }

        std::cout << inet_ntoa(((struct sockaddr_in *) &clientAddress)->sin_addr) << std::endl;
        std::cout << message.type << std::endl;
        std::cout << message.length << std::endl;

        for (auto &client : clients) {
            std::cout << "CLIENT:" << std::endl;
            std::cout << inet_ntoa(((struct sockaddr_in *) client.getPtrToAddress())->sin_addr) << std::endl;
        }
    }
}

int main(int argc, char *argv[]) {
    RadioProxy radioProxy(argc, argv);
    radioProxy.connect();
    radioProxy.handleClients();
    radioProxy.sendRequest();
    radioProxy.readResponse();
    radioProxy.disconnect();
}

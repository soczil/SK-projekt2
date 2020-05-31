#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include "socket.h"
#include "err.h"

#include <iostream> // TODO: remove

Socket::Socket() {
    memset(&(this->addrHints), 0, sizeof(struct addrinfo));
    this->addrHints.ai_family = AF_INET;
    this->addrHints.ai_socktype = SOCK_STREAM;
    this->addrHints.ai_protocol = IPPROTO_TCP;
}

void Socket::openSocket(char *host, char *port) {
    int err;

    err = getaddrinfo(host, port, &addrHints, &addrResult);
    if (err != 0) {
        syserr("getaddrinfo: %s", gai_strerror(err));
    }

    this->sock = socket(addrResult->ai_family, addrResult->ai_socktype,
                        addrResult->ai_protocol);
    if (sock < 0) {
        syserr("socket");
    }

    err = connect(sock, addrResult->ai_addr, addrResult->ai_addrlen);
    if (err < 0) {
        syserr("connect");
    }

    freeaddrinfo(addrResult);
}

void Socket::closeSocket() {
    if (close(sock) < 0) {
        syserr("close");
    }
}

void Socket::writeToSocket(std::string buffer) {
    ssize_t sendLength = buffer.length();

    if (write(sock, buffer.c_str(), sendLength) != sendLength) {
        syserr("partial / failed write");
    }
}

void Socket::readFromSocket() {
    ssize_t recvLength = 0;
    char buffer[1000];

    memset(buffer, 0, sizeof(buffer));
    while ((recvLength = read(sock, buffer, sizeof(buffer) - 1)) > 0) {
        printf("%s", buffer);
    }

    if (recvLength < 0) {
        syserr("read");
    }
}

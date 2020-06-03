#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include "socket.h"
#include "err.h"

#include <iostream> // TODO: remove

Socket::Socket() {
    this->sock = 0;
}

void Socket::closeSocket() {
    if (close(sock) < 0) {
        syserr("close");
    }
}

int Socket::getSockNumber() {
    return sock;
}

void Socket::setSockNumber(int sockNumber) {
    this->sock = sockNumber;
}

TCPSocket::TCPSocket() {
    this->setSockNumber(0);
    this->addrResult = nullptr;

    std::memset(&(this->addrHints), 0, sizeof(struct addrinfo));
    this->addrHints.ai_family = AF_INET;
    this->addrHints.ai_socktype = SOCK_STREAM;
    this->addrHints.ai_protocol = IPPROTO_TCP;
}

void TCPSocket::openSocket(char *host, char *port) {
    int err, sock;

    err = getaddrinfo(host, port, &addrHints, &addrResult);
    if (err != 0) {
        syserr("getaddrinfo: %s", gai_strerror(err));
    }

    sock = socket(addrResult->ai_family, addrResult->ai_socktype,
                        addrResult->ai_protocol);
    if (sock < 0) {
        syserr("socket");
    }

    err = connect(sock, addrResult->ai_addr, addrResult->ai_addrlen);
    if (err < 0) {
        syserr("connect");
    }

    this->setSockNumber(sock);

    freeaddrinfo(addrResult);
}

void TCPSocket::writeToSocket(const std::string& buffer) {
    ssize_t sendLength = buffer.length();

    if (write(this->getSockNumber(), buffer.c_str(), sendLength) != sendLength) {
        syserr("partial / failed write");
    }
}

ssize_t TCPSocket::readFromSocket(char *buffer, size_t size) {
    ssize_t recvLength = 0;

    memset(buffer, 0, size);
    recvLength = read(this->getSockNumber(), buffer, size);
    if (recvLength < 0) {
        syserr("read");
    }

    return recvLength;
}

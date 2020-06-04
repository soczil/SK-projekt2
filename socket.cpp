#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>
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

UDPSocket::UDPSocket() = default;

void UDPSocket::openSocket(in_port_t port, char *multiAddress) {
    int sock;
    struct timeval tv;

    tv.tv_sec = 2;
    tv.tv_usec = 0;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        syserr("socket");
    }

    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        syserr("socketopt");
    }

    if (multiAddress != nullptr) {
        std::cout << "MULTI JOL" << std::endl;
        ipMreq.imr_interface.s_addr = htonl(INADDR_ANY);
        if (inet_aton(multiAddress, &ipMreq.imr_multiaddr) == 0) {
            syserr("inet_aton - invalid multicast address");
        }

        if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                (void *) &ipMreq, sizeof(ipMreq)) < 0) {
            syserr("setsockopt");
        }
    }

    sockaddrIn.sin_family = AF_INET;
    sockaddrIn.sin_addr.s_addr = htonl(INADDR_ANY);
    sockaddrIn.sin_port = htons(port);

    if (bind(sock, (struct sockaddr *) &sockaddrIn,
            sizeof(sockaddrIn)) < 0) {
        syserr("bind");
    }

    this->setSockNumber(sock);
}

BroadcastSocket::BroadcastSocket() = default;

void BroadcastSocket::openSocket(in_port_t port, char *address) {
    int sock, optval;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        syserr("socket");
    }

    // Uaktywnienie rozgłaszania.
    optval = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST,
            (void *) &optval, sizeof(optval)) < 0) {
        syserr("setsockopt broadcast");
    }

    // Ustawienie TTL dla datagramów rozsyłanych do grupy.
    optval = TTL_VALUE;
    if (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL,
            (void *) &optval, sizeof(optval)) < 0) {
        syserr("setsockopt multicast ttl");
    }

    sockaddrIn.sin_family = AF_INET;
    sockaddrIn.sin_port = htons(port);
    if (inet_aton(address, &sockaddrIn.sin_addr) == 0) {
        syserr("inet_aton - invalid multicast address");
    }

    this->setSockNumber(sock);
}

struct sockaddr_in *BroadcastSocket::getSockaddrPtr() {
    return &sockaddrIn;
}

#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include "socket.h"
#include "err.h"

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

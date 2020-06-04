#ifndef _SOCKET_
#define _SOCKET_

#include <netdb.h>
#include <string>

class Socket {
private:
    int sock;

public:
    Socket();
    void closeSocket();
    int getSockNumber();
    void setSockNumber(int);
};

class TCPSocket : public Socket {
private:
    struct addrinfo addrHints;
    struct addrinfo *addrResult;

public:
    TCPSocket();
    void openSocket(char *, char *);
    void writeToSocket(const std::string&);
    ssize_t readFromSocket(char *, size_t size);
};

class UDPSocket : public Socket {
private:
    struct sockaddr_in sockaddrIn;
    struct ip_mreq ipMreq;

public:
    UDPSocket();
    void openSocket(in_port_t, char *);
};

class BroadcastSocket : public Socket {
private:
    const static int TTL_VALUE = 4;
    struct sockaddr_in sockaddrIn;

public:
    BroadcastSocket();
    void openSocket(in_port_t, char *);
    struct sockaddr_in *getSockaddrPtr();
};

#endif // _SOCKET_

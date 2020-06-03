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
public:
    void openSocket();

};

#endif // _SOCKET_

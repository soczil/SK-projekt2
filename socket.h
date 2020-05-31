#ifndef _SOCKET_
#define _SOCKET_

#include <netdb.h>
#include <string>

class Socket {
private:
    int sock;
    struct addrinfo addrHints;
    struct addrinfo *addrResult;

public:
    Socket();
    void openSocket(char *, char *);
    void closeSocket();
    void writeToSocket(std::string);
    int getSockNumber();
};

#endif // _SOCKET_

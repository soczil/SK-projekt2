#ifndef _SOCKET_
#define _SOCKET_

#include <netdb.h>

class Socket {
private:
    int sock;
    struct addrinfo addrHints;
    struct addrinfo *addrResult;

public:
    Socket();
    void openSocket(char *, char *);
    void closeSocket();
};

#endif // _SOCKET_

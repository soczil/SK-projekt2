#ifndef _SERVER_
#define _SERVER_

#include <sys/socket.h>

class Server {
private:
    struct sockaddr address;
    socklen_t addressSize;
    char *name;
    u_int16_t nameSize;

public:
    Server(struct sockaddr *, socklen_t, char *, u_int16_t);
    struct sockaddr *getPtrToAddress();
    socklen_t getAddressSize();
};

#endif // _SERVER_

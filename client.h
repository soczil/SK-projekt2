#ifndef _CLIENT_
#define _CLIENT_

#include <ctime>
#include <netinet/in.h>

class Client {
    struct sockaddr address;
    time_t lastVisit;
};

#endif // _CLIENT_

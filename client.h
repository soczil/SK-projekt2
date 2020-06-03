#ifndef _CLIENT_
#define _CLIENT_

#include <ctime>
#include <netinet/in.h>

const int MAX_CLIENTS = 20;
class Client {
    struct sockaddr_in address;
    time_t lastVisit;
};

#endif // _CLIENT_

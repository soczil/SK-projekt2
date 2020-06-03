#ifndef _CLIENT_
#define _CLIENT_

#include <ctime>
#include <netinet/in.h>

class Client {
private:
    struct sockaddr address {};
    time_t lastVisit;
    bool remove = false;

public:
    Client(struct sockaddr);
    struct sockaddr *getPtrToAddress();
    void setLastVisit(time_t);
};

#endif // _CLIENT_

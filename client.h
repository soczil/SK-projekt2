#ifndef _CLIENT_
#define _CLIENT_

#include <ctime>
#include <netinet/in.h>

class Client {
private:
    struct sockaddr address;
    time_t lastVisit;

public:
    explicit Client(struct sockaddr *);
    struct sockaddr *getPtrToAddress();
    void setLastVisit(time_t);
    unsigned getTimeDifference();
};

#endif // _CLIENT_

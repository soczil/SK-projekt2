#ifndef _SERVER_
#define _SERVER_

#include <ctime>
#include <sys/socket.h>
#include <string>

class Server {
private:
    struct sockaddr address;
    socklen_t addressSize;
    std::string name;
    time_t lastMessageTime;

public:
    Server();
    Server(struct sockaddr *, socklen_t, std::string);
    Server(const Server &server);
    struct sockaddr *getPtrToAddress();
    socklen_t getAddressSize();
    std::string getName();
    void updateTime(time_t);
    unsigned getTimeDifference();
};

#endif // _SERVER_

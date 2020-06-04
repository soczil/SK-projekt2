#ifndef _RADIO_CLIENT_
#define _RADIO_CLIENT_

#include <vector>
#include "server.h"
#include "socket.h"

class RadioClient {
private:
    char *host;
    in_port_t udpPort;
    char *tcpPort;
    unsigned timeout;
    BroadcastSocket broadcastSocket;
    size_t currentServer;
    std::vector<Server> servers;

    void sendDiscover();
    bool receiveIam();
    void sendKeepAlive();
    void receiveData();

public:
    RadioClient(int, char **);
    void start();
};

#endif // _RADIO_CLIENT_

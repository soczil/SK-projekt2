#ifndef _RADIO_CLIENT_
#define _RADIO_CLIENT_

#include <vector>
#include "server.h"
#include "socket.h"

class RadioClient {
private:
    char *host;
    in_port_t udpPort;
    in_port_t tcpPort;
    unsigned timeout;
    BroadcastSocket broadcastSocket;
    TCPSocket tcpSocket;

    void sendDiscover();
    void receiveIam(struct sockaddr *, socklen_t, struct message *);
    void sendKeepAlive();
    void receiveData();
    int serverLookup(struct sockaddr *);
    void menageTelnet(int);

public:
    RadioClient(int, char **);
    void start();
};

#endif // _RADIO_CLIENT_

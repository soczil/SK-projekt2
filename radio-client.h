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

    void sendDiscover();
    void receiveIam(struct sockaddr *, socklen_t, struct message *);
    void sendKeepAlive();
    void receiveData();
    int serverLookup(struct sockaddr *);

public:
    RadioClient(int, char **);
    void start();
};

#endif // _RADIO_CLIENT_

#ifndef _RADIO_CLIENT_
#define _RADIO_CLIENT_

#include "socket.h"

class RadioClient {
private:
    char *host;
    in_port_t udpPort;
    char *tcpPort;
    unsigned timeout;
    struct sockaddr proxyAddress;
    size_t proxyAddressSize;
    BroadcastSocket broadcastSocket;

    void sendKeepAlive();

public:
    RadioClient(int, char **);
    void start();
};

#endif // _RADIO_CLIENT_

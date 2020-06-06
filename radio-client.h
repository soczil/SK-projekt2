#ifndef _RADIO_CLIENT_
#define _RADIO_CLIENT_

#include <vector>
#include "server.h"
#include "socket.h"
#include "telnet-screen.h"

class RadioClient {
private:
    char *host;
    in_port_t udpPort;
    in_port_t tcpPort;
    unsigned timeout = 5;
    BroadcastSocket broadcastSocket;
    TCPSocket tcpSocket;
    int telnetSock;
    TelnetScreen telnetScreen;
    std::mutex protector;

    void sendDiscover(struct sockaddr *);
    void handleIam(struct sockaddr *, socklen_t, struct message *);
    void sendKeepAlive();
    void receiveData();
    int serverLookup(struct sockaddr *);
    void menageTelnet(int);
    bool sameAddresses(struct sockaddr *, struct sockaddr *);
    void keyUpClicked();
    void keyDownClicked();
    void enterClicked(std::thread &);
    void updateOptions();
    void controlTimeout(std::thread &);
    void removeServer();

public:
    RadioClient(int, char **);
    void start();
};

#endif // _RADIO_CLIENT_

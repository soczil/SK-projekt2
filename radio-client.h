#ifndef _RADIO_CLIENT_
#define _RADIO_CLIENT_

class RadioClient {
private:
    char *host;
    char *udpPort;
    char *tcpPort;
    unsigned timeout;

public:
    RadioClient(int, char **);
};

#endif // _RADIO_CLIENT_

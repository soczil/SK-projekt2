#ifndef _RADIO_PROXY_
#define _RADIO_PROXY_

#include "socket.h"

class RadioProxy {
private:
    char *host;
    char  *resource;
    char *port;
    bool metadata = false;
    unsigned timeout = 5;
    Socket socket;

public:
    RadioProxy(int, char **);
    void printRadioProxy();
    void connect();
    void disconnect();
    void sendRequest();
    void readResponse();
};

#endif // _RADIO_PROXY_

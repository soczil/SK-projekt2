#ifndef _RADIO_PROXY_
#define _RADIO_PROXY_

#include <iostream>

class RadioClient {
private:
    char *host;
    char  *resource;
    char *port;
    bool metadata = false;
    unsigned timeout = 5;

public:
    RadioClient(int argc, char *argv[]);
    void printClient();
};

#endif // _RADIO_PROXY_

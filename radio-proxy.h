#ifndef _RADIO_PROXY_
#define _RADIO_PROXY_

#include <vector>
#include <utility>
#include "socket.h"

class RadioProxy {
private:
    char *host;
    char  *resource;
    char *port;
    bool metadata = false;
    unsigned timeout = 5;
    TCPSocket tcpSocket;

    bool readHeader(char *, int &, std::pair<int, int> &);
    static bool correctHeader(std::vector<char> &, int &);
    void readWithoutMetadata(char *, std::pair<int, int>&);
    void readWithMetadata(char *, int, std::pair<int, int>&);
    bool readBlock(int, int &, char *, ssize_t &, bool);

public:
    RadioProxy(int, char **);
    void connect();
    void disconnect();
    void sendRequest();
    void readResponse();
};

#endif // _RADIO_PROXY_

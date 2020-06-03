#ifndef _RADIO_PROXY_
#define _RADIO_PROXY_

#include <vector>
#include <utility>
#include "socket.h"
#include "client.h"
#include "message.h"

class RadioProxy {
private:
    char *host = nullptr;
    char *resource = nullptr;
    char *port = nullptr;
    bool metadata = false;
    unsigned timeout = 5;
    bool proxy = false;
    in_port_t udpPort = 0;
    char *multiAddress = nullptr;
    unsigned clientsTimeout = 5;
    TCPSocket tcpSocket;
    UDPSocket udpSocket;
    std::vector<Client> clients;
    std::string radioName;

    bool readHeader(char *, int &, std::pair<int, int> &);
    static bool correctHeader(std::vector<char> &, int &);
    void readWithoutMetadata(char *, std::pair<int, int>&);
    void readWithMetadata(char *, int, std::pair<int, int>&);
    bool readBlock(int, int &, char *, ssize_t &, bool);
    void discoverMessage(struct sockaddr *, socklen_t);
    void keepaliveMessage(struct sockaddr *);
    int clientLookup(struct sockaddr *);
    void addNewClient(struct sockaddr);

public:
    RadioProxy(int, char **);
    void connect();
    void disconnect();
    void sendRequest();
    void readResponse();
    void handleClients();
};

#endif // _RADIO_PROXY_

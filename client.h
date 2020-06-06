#ifndef _CLIENT_
#define _CLIENT_

#include <ctime>
#include <netinet/in.h>

class Client {
private:
    struct sockaddr address;
    time_t lastVisit;

public:
    /**
     * Tworzy klienta na podstawie jego adresu.
     */
    explicit Client(struct sockaddr *);

    /**
     * Daje w wyniku wskaźnik do struktury z adresem klienta.
     */
    struct sockaddr *getPtrToAddress();

    /**
     * Aktualizuje czas ostatniej wiadomości od klienta.
     */
    void setLastVisit(time_t);

    /**
     * Daje w wyniku liczbę sekund od ostatniej wiadomości od klienta.
     */
    unsigned getTimeDifference();
};

#endif // _CLIENT_

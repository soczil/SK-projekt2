#ifndef _SERVER_
#define _SERVER_

#include <ctime>
#include <sys/socket.h>
#include <string>

class Server {
private:
    struct sockaddr address;
    socklen_t addressSize;
    std::string name;
    time_t lastMessageTime;

public:
    /**
     * Konstruktor domyślny zeruje strukturę przechowującą adres.
     */
    Server();

    /**
     * Podstawowy konstruktor tworzący serwer.
     */
    Server(struct sockaddr *, socklen_t, std::string);

    /**
     * Przeciąża operator przypisania.
     */
    Server& operator=(const Server &server);

    /**
     * Przeciąża operator porównania.
     */
    bool operator==(const Server &server);

    /**
     * Daje w wyniku wskaźnik na strukturę zawierającą adres serwera.
     */
    struct sockaddr *getPtrToAddress();

    /**
     * Daje w wyniku rozmiar struktury z adresem serwera.
     */
    socklen_t getAddressSize();

    /**
     * Daje w wyniku nazwę serwera.
     */
    std::string getName();

    /**
     * Aktualizuje czas ostatniej wiadomości z danymi od serwera.
     */
    void updateTime(time_t);

    /**
     * Daje w wyniku liczbę sekund, które upłynęły
     * od ostatniej wiadomości od serwera.
     */
    unsigned getTimeDifference();
};

#endif // _SERVER_

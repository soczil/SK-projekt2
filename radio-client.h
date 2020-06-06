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

    /**
     * Wysyła wiadomość DISCOVER na wskazany adres.
     */
    void sendDiscover(struct sockaddr *);

    /**
     * Obsługuje otrzymanie wiadomości IAM.
     */
    void handleIam(struct sockaddr *, socklen_t, struct message *);

    /**
     * Funkcja uruchamiana w nowym wątku - wysyła wiadomość KEEPALIVE co 3.5s.
     */
    void sendKeepAlive();

    /**
     * Funkcja uruchamiana w nowym wątku - odbiera wiadomości od serwerów.
     */
    void receiveData();

    /**
     * Wyszukuje serwer po podanym adresie.
     */
    int serverLookup(struct sockaddr *);

    /**
     * Obsługuje połączenie z telnetem.
     */
    void menageTelnet(int);

    /**
     * Sprawdza, czy podane adresy są takie same.
     */
    bool sameAddresses(struct sockaddr *, struct sockaddr *);

    /**
     * Obsługuje kliknięcie strzałki w górę w telnecie.
     */
    void keyUpClicked();

    /**
     * Obsługuje kliknięcie strzałki w dół w telnecie.
     */
    void keyDownClicked();

    /**
     * Obsługuje kliknięcie enter w telnecie.
     */
    void enterClicked(std::thread &);

    /**
     * Aktualizuje pozycję podświetlenia opcji w telnecie.
     */
    void updateOptions();

    /**
     * Sprawdza, czy wystąpił timeout w połączeniu z serwerem, jeśli
     * tak to wywołuje funkcję removeServer i obsługuje wyłączanie radia.
     */
    void controlTimeout(std::thread &);

    /**
     * Usuwa serwer z wektora serwerów i aktualizuje pozycje opcji w telnecie.
     */
    void removeServer();

public:
    /**
     * Konstruktor obsługuje sprawdzanie flag z jakimi został uruchomiony program.
     */
    RadioClient(int, char **);

    /**
     * Rozpoczyna działanie RadioClient.
     */
    void start();
};

#endif // _RADIO_CLIENT_

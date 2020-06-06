#ifndef _RADIO_PROXY_
#define _RADIO_PROXY_

#include <vector>
#include <utility>
#include <mutex>
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
    std::mutex mutex;

    /**
     * Wczytuje header odpowiedzi od serwera.
     */
    bool readHeader(char *, int &, std::pair<int, int> &);

    /**
     * Sprawdza, czy header jest poprawny i wyciąga z niego potrzebne dane.
     */
    bool correctHeader(std::vector<char> &, int &);

    /**
     * Czyta content odpowiedzi bez metadanych.
     */
    void readWithoutMetadata(char *, std::pair<int, int>&);

    /**
     * Czyta content odpowiedzi z metadanymi.
     */
    void readWithMetadata(char *, int, std::pair<int, int>&);

    /**
     * Wczytuje blok danych lub metadanych w zależności od podanych parametrów.
     */
    bool readBlock(int, int &, char *, ssize_t &, bool, char *);

    /**
     * Obsługuje odebranie wiadomości DISCOVERY.
     */
    void discoverMessage(struct sockaddr *, socklen_t);

    /**
     * Obsługuje odebranie wiadomości KEEPALIVE.
     */
    void keepaliveMessage(struct sockaddr *);

    /**
     * Wyszukuje klienta na podstawie podanego adresu.
     */
    int clientLookup(struct sockaddr *);

    /**
     * Dodaje nowego klienta do obsługiwania.
     */
    void addNewClient(struct sockaddr *);

    /**
     * Pisze dane na stdout lub w przypadku pośrednika wysyła je klientom.
     */
    void writeData(char *, size_t);

    /**
     * Pisze metadane na stderr lub w przypadku pośrednika wysyła je klientom.
     */
    void writeMetadata(char *, size_t);

    /**
     * Wywołuje funkcje otwierające gniazda i nawiązujące połączenia.
     */
    void connect();

    /**
     * Wywołuje funkcje zamykające gniazda i rozłączające połączenia.
     */
    void disconnect();

    /**
     * Wysyła zapytanie GET do serwera HTTP.
     */
    void sendRequest();

    /**
     * Główna funkcja odpowiedzialna za odbieranie danych od serwera.
     */
    void readResponse();

    /**
     * Funkcja uruchamiana w nowym wątku obsługująca wiadomości od klientów.
     */
    void handleClients();

    /**
     * Funkcja wysyłająca dane do klientów.
     */
    void writeToClients(int, char *, size_t);

public:
    /**
     * Konstruktor sprawdzający flagi.
     */
    RadioProxy(int, char **);

    /**
     * Rozpoczyna działanie RadioProxy.
     */
    void start();
};

#endif // _RADIO_PROXY_

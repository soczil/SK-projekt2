#ifndef _TELNET_SCREEN_
#define _TELNET_SCREEN_

#include <mutex>

class TelnetScreen {
private:
    const char *MODE = "\377\375\042\377\373\001";
    const char *RESET = "\u001b[0m";
    const char *COLOR = "\033[31;7m";
    const char *CLEAR_SCREEN = "\u001B[2J";
    const char *SEARCH = "Szukaj pośrednika\r\n";
    const char *END = "Koniec\r\n";
    size_t options;
    size_t position;
    size_t playing;

public:
    /**
     * Ustawia wartosc position na 0 i options na 2, ponieważ początkowo jest
     * tylko opcja 'Szukaj pośredników' i 'Koniec'.
     */
    TelnetScreen();

    /**
     * Gettery.
     */
    size_t getOptions();
    size_t getPosition();
    size_t getPlaying();

    /**
     * Settery.
     */
    void setOptions(size_t);
    void setPosition(size_t);
    void setPlaying(size_t);

    /**
     * Wysyła do telneta komendę, dzięki której telnet wysyła nam kody
     * naciskanych przycisków.
     */
    void prepare(int);

    /**
     * Odświeża ekran telneta.
     */
    void render(int, std::vector<Server> &, std::vector<std::string> &);
};

#endif // _TELNET_SCREEN_

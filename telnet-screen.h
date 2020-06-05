#ifndef _TELNET_SCREEN_
#define _TELNET_SCREEN_

class TelnetScreen{
private:
    const char *MODE = "\377\375\042\377\373\001";
    const char *RESET = "\u001b[0m";
    const char *COLOR = "\033[31;7m";
    const char *CLEAR_SCREEN = "\u001B[2J";
    const char *SEARCH = "Szukaj pośrednika\r\n";
    const char *END = "Koniec\r\n";
    int options;
    int position;

public:
    TelnetScreen();
    int getOptions();
    int getPosition();
    void setOptions(int);
    void setPosition(int);
    void prepare(int);
    void render(int);
};

#endif // _TELNET_SCREEN_
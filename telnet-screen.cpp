#include <iostream>
#include <sstream>
#include <unistd.h>
#include <vector>
#include "server.h"
#include "err.h"
#include "telnet-screen.h"

TelnetScreen::TelnetScreen() {
    this->options = 2;
    this->position = 0;
}

int TelnetScreen::getOptions() {
    return options;
}

int TelnetScreen::getPosition() {
    return position;
}

void TelnetScreen::setOptions(size_t value) {
    this->options = value;
}

void TelnetScreen::setPosition(size_t value) {
    this->position = value;
}

void TelnetScreen::prepare(int sock) {
    if (write(sock, MODE, 6) != 6) {
        syserr("write");
    }
}

void TelnetScreen::render(int sock, std::vector<Server> &servers) {
    std::string buffer;
    std::ostringstream os;

    os << CLEAR_SCREEN;
    if (position == 0) {
        os << COLOR << SEARCH << RESET;
    } else {
        os << SEARCH;
    }

    for (size_t i = 0; i < servers.size(); i++) {
        if (position == i + 1) {
            os << COLOR << servers[i].getName() << "\r\n" << RESET;
        } else {
            os << servers[i].getName() << "\r\n";
        }
    }

    if (position == (options - 1)) {
        os << COLOR << END << RESET;
    } else {
        os << END;
    }

    buffer = os.str();

    if (write(sock, buffer.c_str(), buffer.length()) != (ssize_t) buffer.length()) {
        syserr("write");
    }
}

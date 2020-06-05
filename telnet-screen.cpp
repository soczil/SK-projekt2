#include <iostream>
#include <sstream>
#include <unistd.h>
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

void TelnetScreen::setOptions(int value) {
    this->options = value;
}

void TelnetScreen::setPosition(int value) {
    this->position = value;
}

void TelnetScreen::prepare(int sock) {
    if (write(sock, MODE, 6) != 6) {
        syserr("write");
    }
}

void TelnetScreen::render(int sock) {
    std::string buffer;
    std::ostringstream ss;

    ss << CLEAR_SCREEN;
    if (position == 0) {
        ss << COLOR << SEARCH << RESET;
    } else {
        ss << SEARCH;
    }

    if (position == (options - 1)) {
        ss << COLOR << END << RESET;
    } else {
        ss << END;
    }

    buffer = ss.str();
    if (write(sock, buffer.c_str(), buffer.length()) != (ssize_t) buffer.length()) {
        syserr("write");
    }
}

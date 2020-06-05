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

size_t TelnetScreen::getOptions() {
    return options;
}

size_t TelnetScreen::getPosition() {
    return position;
}

size_t TelnetScreen::getPlaying() {
    return playing;
}

void TelnetScreen::setOptions(size_t value) {
    this->options = value;
}

void TelnetScreen::setPosition(size_t value) {
    this->position = value;
}

void TelnetScreen::setPlaying(size_t value) {
    this->playing = value;
}

void TelnetScreen::prepare(int sock) {
    if (write(sock, MODE, 6) != 6) {
        syserr("write");
    }
}

void TelnetScreen::render(int sock, std::vector<Server> &servers) {
    std::string buffer;
    std::ostringstream os;
    std::string server;

    os << CLEAR_SCREEN;
    if (position == 0) {
        os << COLOR << SEARCH << RESET;
    } else {
        os << SEARCH;
    }

    for (size_t i = 0; i < servers.size(); i++) {
        if (i == playing) {
            server = servers[i].getName() + " *";
        } else {
            server = servers[i].getName();
        }

        if (position == i + 1) {
            os << COLOR << server << "\r\n" << RESET;
        } else {
            os << server << "\r\n";
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

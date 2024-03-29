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

void TelnetScreen::render(int sock, std::vector<Server> &servers,
                          std::vector<std::string> &metadata) {
    std::string buffer;
    std::ostringstream os;
    std::string server;

    // Opcja 'Szukaj pośredników'.
    os << CLEAR_SCREEN;
    if (position == 0) {
        os << COLOR << SEARCH << RESET;
    } else {
        os << SEARCH;
    }

    // Dostępni pośrednicy.
    for (size_t i = 0; i < servers.size(); i++) {
        if (i == playing) {
            server = "Pośrednik " + servers[i].getName() + " *";
        } else {
            server = "Pośrednik " + servers[i].getName();
        }

        if (position == i + 1) {
            os << COLOR << server << "\r\n" << RESET;
        } else {
            os << server << "\r\n";
        }
    }

    // Opcja 'Koniec'.
    if (position == (options - 1)) {
        os << COLOR << END << RESET;
    } else {
        os << END;
    }

    // Metadane.
    for (auto &meta : metadata) {
        os << meta << "\r\n";
    }

    buffer = os.str();

    if (write(sock, buffer.c_str(), buffer.length()) != (ssize_t) buffer.length()) {
        syserr("write");
    }
}

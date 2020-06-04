#include <cstring>
#include "server.h"

Server::Server(struct sockaddr *address, socklen_t addressSize,
        char *name, u_int16_t nameSize) {
    std::memcpy(&(this->address), address, addressSize);
    this->addressSize = addressSize;
    this->name = name;
    this->nameSize = nameSize;
}

struct sockaddr *Server::getPtrToAddress() {
    return &address;
}

socklen_t Server::getAddressSize() {
    return addressSize;
}

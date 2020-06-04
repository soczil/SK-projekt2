#include <cstring>
#include "server.h"

Server::Server(struct sockaddr *address, socklen_t addressSize, std::string name) {
    std::memcpy(&(this->address), address, addressSize);
    this->addressSize = addressSize;
    this->name = name;
}

Server::Server(const Server &server) {
    std::memcpy(&(this->address), &(server.address), server.addressSize);
    this->addressSize = server.addressSize;
    this->name = server.name;
    this->lastMessage = server.lastMessage;
}

Server::Server() = default;

struct sockaddr *Server::getPtrToAddress() {
    return &address;
}

socklen_t Server::getAddressSize() {
    return addressSize;
}

std::string Server::getName() {
    return name;
}

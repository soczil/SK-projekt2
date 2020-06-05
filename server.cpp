#include <cstring>
#include <netinet/in.h>
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
    this->lastMessageTime = server.lastMessageTime;
}

Server::Server() {
    std::memset(&address, 0, sizeof(address));
};

struct sockaddr *Server::getPtrToAddress() {
    return &address;
}

socklen_t Server::getAddressSize() {
    return addressSize;
}

std::string Server::getName() {
    return name;
}

void Server::updateTime(time_t value) {
    this->lastMessageTime = value;
}

unsigned Server::getTimeDifference() {
    return (time(nullptr) - lastMessageTime);
}

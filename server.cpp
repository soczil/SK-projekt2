#include <cstring>
#include <netinet/in.h>
#include "server.h"

Server::Server(struct sockaddr *address, socklen_t addressSize, std::string name) {
    std::memcpy(&(this->address), address, addressSize);
    this->addressSize = addressSize;
    this->name = name;
}

Server& Server::operator=(const Server &server) {
    std::memcpy(&(this->address), &(server.address), server.addressSize);
    this->addressSize = server.addressSize;
    this->name = server.name;
    this->lastMessageTime = server.lastMessageTime;
    return *this;
}

Server::Server() {
    std::memset(&address, 0, sizeof(address));
    this->lastMessageTime = time(nullptr);
    this->addressSize = sizeof(address);
    this->name = "";
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

bool Server::operator==(const Server &server) {
    auto myAddress = (struct sockaddr_in *) &address;
    auto otherAddress = (struct sockaddr_in *) &(server.address);

    return (myAddress->sin_addr.s_addr == otherAddress->sin_addr.s_addr)
           && (myAddress->sin_port == otherAddress->sin_port);
}

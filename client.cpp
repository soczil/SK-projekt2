#include "client.h"

Client::Client(struct sockaddr newAddress) {
    address = newAddress;
    lastVisit = time(nullptr);
}

struct sockaddr *Client::getPtrToAddress() {
    return &address;
}

void Client::setLastVisit(time_t newTime) {
    this->lastVisit = newTime;
}

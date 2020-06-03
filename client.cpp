#include <cstring>
#include "client.h"

Client::Client(struct sockaddr *newAddress) {
    memcpy(&address, newAddress, sizeof(struct sockaddr));
    lastVisit = time(nullptr);
}

struct sockaddr *Client::getPtrToAddress() {
    return &address;
}

void Client::setLastVisit(time_t newTime) {
    this->lastVisit = newTime;
}

time_t Client::getLastVisit() {
    return lastVisit;
}

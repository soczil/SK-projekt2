#ifndef _MESSAGE_
#define _MESSAGE_

#include <cstdint>

struct message {
    uint16_t type;
    uint16_t length;
    char buffer[100];
};

#endif // _MESSAGE_
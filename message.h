#ifndef _MESSAGE_
#define _MESSAGE_

#include <cstdint>

/**
 * Struktura przesyłana w konwersacji między radio-proxy i radio-client.
 */
struct message {
    uint16_t type;
    uint16_t length;
    char buffer[256];
};

#endif // _MESSAGE_

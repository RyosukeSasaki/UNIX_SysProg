#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

struct _msg {
    uint8_t type;
    uint8_t code;
    uint16_t ttl;
    uint32_t ipaddr;
    uint32_t netmask;
};

typedef union _dhcp_message {
    struct _msg message;
    char data[12];
} dhcp_message_t;

#endif
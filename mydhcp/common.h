#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define UNKNOWN_MSG -2
#define MESSAGE_TIMEOUT 10

typedef enum _dhcp_type {
    DHCP_TYPE_DISCOVER=1,
    DHCP_TYPE_OFFER,
    DHCP_TYPE_REQUEST,
    DHCP_TYPE_ACK,
    DHCP_TYPE_RELEASE
} dhcp_type_t;

struct _type_str {
    char *str;
} dhcp_type_str[] = {
    "",
    "DISCOVER",
    "OFFER",
    "REQUEST",
    "ACK",
    "RELEASE",
};

typedef enum _dhcp_code {
    DHCP_CODE_OK=0,
    DHCP_CODE_ERR_NOIP,
    DHCP_CODE_REQ_ALLOC,
    DHCP_CODE_REQ_EXT,
    DHCP_CODE_ERR_REQ
} dhcp_code_t;

struct _code_str {
    char *str;
} dhcp_code_str[] = {
    "OK",
    "NO IP",
    "ALLOCATION",
    "EXTENSION",
    "ERROR REQUEST"
};

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
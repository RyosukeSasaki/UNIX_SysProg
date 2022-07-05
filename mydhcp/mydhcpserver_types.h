#ifndef MYDHCPSERVER_TYPES_H
#define MYDHCPSERVER_TYPES_H

#include <netinet/in.h>

#define BUF_LEN 1024

typedef enum _stat {
    init=1,
    wait_req1,
    in_use,
    term,
    wait_req2
} state_t;

typedef enum _event {
    discover=1,
    request_alloc_ok,
    request_alloc_ng,
    request_ext_ok,
    request_ext_ng,
    release_ok,
    release_ng,
    req_timeout,
    ttl_timeout,
    unknown_msg
} event_t;

typedef struct _client {
    struct _client *fp;
    struct _client *bp;
    int stat;
    int ttlcounter;
    struct in_addr id;
    struct in_addr addr;
    struct in_addr netmask;
    in_port_t port;
    uint16_t ttl;
} client_t;


#endif
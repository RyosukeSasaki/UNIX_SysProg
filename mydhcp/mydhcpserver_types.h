#ifndef MYDHCPSERVER_TYPES_H
#define MYDHCPSERVER_TYPES_H

#include <netinet/in.h>

#define BUF_LEN 1024

typedef enum _stat {
    init=1,
    wait_req1,
    wait_req2,
    in_use,
    term,
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

typedef struct _addr_pool {
    struct _addr_pool *fp;
    struct _addr_pool *bp;
    struct in_addr addr;
    struct in_addr netmask;
} addr_pool_t;

typedef struct _client {
    struct _client *fp;
    struct _client *bp;
    int stat;
    uint16_t ttlcounter; // initial value of ttl
    int tout;
    struct in_addr id; //stored in network byte order
    addr_pool_t *addr; //stored in network byte order
    in_port_t port; //stored in network byte order
    uint16_t ttl;
} client_t;


#endif
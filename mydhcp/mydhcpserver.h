#ifndef MYDHCPSERVER_H
#define MYDHCPSERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include "common.h"

typedef enum _stat {
    init=1,
    wait_req1,
    wait_req2,
    in_use,
    term
} state_t;

typedef enum _event {
    discover=1,
    request_alloc_ok,
    request_alloc_ng,
    request_ext_ok,
    request_ext_ng,
    release_ok,
    release_ng,
    timeout,
    ttl_timeout
} event_t;

typedef struct _client {
    struct client *fp;
    struct client *bp;
    int stat;
    int ttlcounter;
    struct in_addr id;
    struct in_addr addr;
    struct in_addr netmask;
    in_port_t port;
    uint16_t ttl;
} client_t;

void fact1(client_t *, int);
void fact2(client_t *, int);
void fact3(client_t *, int);
void fact4(client_t *, int);
void fact5(client_t *, int);
void fact6(client_t *, int);
void fact7(client_t *, int);
void fact8(client_t *, int);

struct proctable {
    int stat;
    int event;
    void (*func)(client_t *, int);
} pstab[] = {
    {init, discover, fact1},
    {wait_req1, request_alloc_ng, fact2},
    {in_use, request_ext_ok, fact3},
    {in_use, release_ok, fact4},
    {in_use, ttl_timeout, fact4},
    {0, 0, NULL}
};

int create_client();
int wait_event(int, struct sockaddr_in *, socklen_t *);
client_t client_h;
int stop;

#endif
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
#include <arpa/inet.h>
#include "common.h"
#include "mydhcpserver_types.h"

void send_offer(client_t *, int);
void send_ack_ok(client_t *, int);
void reset_ttl(client_t *, int);
void release_client(client_t *, int);
void resend_offer(client_t *, int);
void send_ack_ng(client_t *, int);

typedef struct _proctable {
    int stat;
    int event;
    void (*func)(client_t *, int);
} proctable_t;
proctable_t pstab[] = {
    {init, discover, send_offer},
    {wait_req1, request_alloc_ok, send_ack_ok},
    {in_use, request_ext_ok, reset_ttl},
    {in_use, release_ok, release_client},
    {in_use, ttl_timeout, release_client},
    {wait_req1, req_timeout, resend_offer},
    {wait_req2, request_alloc_ok, send_ack_ok},
    {in_use, release_ng, NULL},
    {wait_req1, request_alloc_ng, send_ack_ng},
    {wait_req1, unknown_msg, release_client},
    {in_use, request_ext_ng, send_ack_ng},
    {in_use, unknown_msg, release_client},
    {wait_req2, request_alloc_ng, send_ack_ng},
    {wait_req2, unknown_msg, release_client},
    {wait_req2, req_timeout, release_client},
    {0, 0, NULL},
};

int parse_discover(dhcp_message_t *);
int parse_request(dhcp_message_t *);
int parse_relase(dhcp_message_t *);

typedef struct _parse_message {
    int type;
    int (*func)(dhcp_message_t *)
} parse_message_t;
parse_message_t pmsgtab[] = {
    {DHCP_TYPE_DISCOVER, parse_discover},
    {DHCP_TYPE_REQUEST, parse_request},
    {DHCP_TYPE_RELEASE, parse_relase},
};

int create_client();
int get_event();
int get_msg(int, struct sockaddr_in *, socklen_t *, dhcp_message_t *);
client_t *client_h;
int stop;

#endif
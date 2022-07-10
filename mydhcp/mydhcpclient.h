#ifndef MYDHCPCLIENT_H
#define MYDHCPCLIENT_H

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
#include <netdb.h>
#include "common.h"
#include "mydhcpclient_types.h"

#define BUF_LEN 1024
#define PORT "51230"

void change_state(int);
int send_discover();
int send_request_alloc();
int send_request_ext();
int send_release();
int start_using();

typedef struct _proctable {
    int stat;
    int event;
    int (*func)(int *);
} proctable_t;
proctable_t pstab[] = {
    {wait_offer1, offer_ok, send_request_alloc},
    {wait_ack1, ack_ok, start_using},
    {in_use, half_ttl, send_request_ext},
    {wait_ext_ack, ack_ok, start_using},
    {in_use, sighup, send_release},
    {wait_ext_ack, sighup, send_release}
};

int recv_offer(dhcp_message_t *);
int recv_ack(dhcp_message_t *);

typedef struct _parse_message {
    int type;
    int (*func)(dhcp_message_t *);
} parse_message_t;
parse_message_t pmsgtab[] = {
    {DHCP_TYPE_OFFER, recv_offer},
    {DHCP_TYPE_ACK, recv_ack}
};

void socket_conf(struct addrinfo **, char *, char *);
void signal_conf();
void sighup_handler();
int send_dhcp(dhcp_message_t *);
int get_msg(struct sockaddr_in *, socklen_t *, dhcp_message_t *);
int get_event(dhcp_message_t *);


uint16_t ttlcounter; // intialvalue of ttl
uint16_t ttl;
struct in_addr addr; //stored in network byte order
struct in_addr netmask; //stored in network byte order
struct addrinfo *res;
int stop_flag;
int sighup_flag;
int alarm_flag;
int stat;
int sfd;

struct statstr {
    char *str;
} ststr[] = {
    "NULL",
    "INIT",
    "WAIT_OFFER1",
    "WAIT_OFFER2",
    "WAIT_ACK1",
    "WAIT_ACK2",
    "IN_USE",
    "WAIT_EXT_ACK",
    "TERMINATE"
};

#endif
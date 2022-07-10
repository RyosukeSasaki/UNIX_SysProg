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
#include <sys/time.h>
#include "common.h"
#include "mydhcpclient_types.h"

#define BUF_LEN 1024
#define PORT "51230"


int send_discover();
int send_request_alloc();
int send_request_ext();
int send_release();
int start_using();

int resend_discover();
int resend_request_alloc();
int terminate();

typedef struct _proctable {
    int stat;
    int event;
    int (*func)();
} proctable_t;
proctable_t pstab[] = {
    {wait_offer1, offer_ok, send_request_alloc},
    {wait_offer1, offer_ng, terminate},
    {wait_offer1, timeout, resend_discover},
    {wait_offer1, unknown_msg, terminate},
    {wait_offer2, offer_ok, send_request_alloc},
    {wait_offer2, offer_ng, terminate},
    {wait_offer2, timeout, terminate},
    {wait_offer2, unknown_msg, terminate},
    {wait_ack1, ack_ok, start_using},
    {wait_ack1, ack_ng, terminate},
    {wait_ack1, timeout, resend_request_alloc},
    {wait_ack1, unknown_msg, terminate},
    {wait_ack2, ack_ok, start_using},
    {wait_ack2, ack_ng, terminate},
    {wait_ack2, timeout, terminate},
    {wait_ack2, unknown_msg, terminate},
    {in_use, half_ttl, send_request_ext},
    {in_use, sighup, send_release},
    {in_use, ttl_timeout, terminate},
    {wait_ext_ack, ack_ok, start_using},
    {wait_ext_ack, sighup, send_release},
    {wait_ext_ack, ttl_timeout, terminate},
    {wait_ext_ack, timeout, terminate},
    {wait_ext_ack, ack_ng, terminate},
    {wait_ext_ack, unknown_msg, terminate},
    {0, 0, NULL}
};

int recv_offer(dhcp_message_t *);
int recv_ack(dhcp_message_t *);

typedef struct _parse_message {
    int type;
    int (*func)(dhcp_message_t *);
} parse_message_t;
parse_message_t pmsgtab[] = {
    {DHCP_TYPE_OFFER, recv_offer},
    {DHCP_TYPE_ACK, recv_ack},
    {0, NULL}
};

void socket_conf(struct addrinfo **, char *, char *);
void signal_conf();
void sighup_handler();
void sigalrm_handler();
void sigint_handler();
void alarm_conf();
void change_state(int);

int FSM_func(int);
int send_dhcp(dhcp_message_t *);
int get_msg(struct sockaddr_in *, socklen_t *, dhcp_message_t *);
int get_event(dhcp_message_t *);
void decrement_ttl();
void decrement_tout();


uint16_t ttlcounter; // intialvalue of ttl
int ttl;
int tout;
struct in_addr addr; //stored in network byte order
struct in_addr netmask; //stored in network byte order
struct addrinfo *res;
int stop_flag;
int sighup_flag;
int sigint_flag;
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
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
#include <netdb.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include "common.h"
#include "mydhcpserver_types.h"

#define path_max 4096
#define ADDR_POOL_SIZE 256
#define PORT 51230

int send_offer_ng(int *, struct sockaddr_in *);
int send_offer_ok(int *, client_t *);
int send_ack_ok(int *, client_t *);
int send_ack_ng(int *, client_t *);
int reset_ttl(int *, client_t *);
int release_client(int *, client_t *);
int resend_offer(int *, client_t *);

typedef struct _proctable {
    int stat;
    int event;
    int (*func)(int *, client_t *);
} proctable_t;
proctable_t pstab[] = {
    {init, discover, send_offer_ok},
    {wait_req1, request_alloc_ok, send_ack_ok},
    {wait_req1, request_alloc_ng, send_ack_ng},
    {wait_req1, req_timeout, resend_offer},
    {wait_req1, unknown_msg, release_client},
    {in_use, request_ext_ok, reset_ttl},
    {in_use, release_ok, release_client},
    {in_use, ttl_timeout, release_client},
    {in_use, release_ng, NULL},
    {in_use, request_ext_ng, send_ack_ng},
    {in_use, unknown_msg, release_client},
    {wait_req2, request_alloc_ok, send_ack_ok},
    {wait_req2, request_alloc_ng, send_ack_ng},
    {wait_req2, unknown_msg, release_client},
    {wait_req2, req_timeout, release_client},
    {0, 0, NULL},
};

int recv_discover(dhcp_message_t *, client_t *);
int recv_request(dhcp_message_t *, client_t *);
int recv_release(dhcp_message_t *, client_t *);

typedef struct _parse_message {
    int type;
    int (*func)(dhcp_message_t *, client_t *);
} parse_message_t;
parse_message_t pmsgtab[] = {
    {DHCP_TYPE_DISCOVER, recv_discover},
    {DHCP_TYPE_REQUEST, recv_request},
    {DHCP_TYPE_RELEASE, recv_release},
    {0, NULL}
};


void socket_conf(int *, in_port_t);
void signal_conf();
void alarm_handler();
void sighup_handler();

void read_config(char *);
void show_addr_pool();
void getargs(int *, char *[], char *);
char *normalize_path(char *);

void initialize_addr_pool();
void insert_addr_pool(addr_pool_t *);
addr_pool_t *fetch_addr_pool();
int is_addr_pool_empty();

void initialize_client_buffer();
void insert_client(client_t *);
void remove_client(client_t *);
client_t *create_client(struct sockaddr_in *);

int get_event(dhcp_message_t *, client_t *);
int get_msg(int *, struct sockaddr_in *, socklen_t *, dhcp_message_t *);
int send_dhcp(int *, dhcp_message_t *, in_addr_t, in_port_t);
void change_state(client_t *, int);

client_t client_h;
addr_pool_t addr_pool_h;
addr_pool_t addr_pool[ADDR_POOL_SIZE];
uint16_t ttl_max;
int stop_flag;
int alarm_flag;

struct statstr {
    char *str;
} ststr[] = {
    "NULL",
    "INIT",
    "WAIT_REQUEST1",
    "WAIT_REQUEST2",
    "IN_USE",
    "TERMINATE"
};

#endif
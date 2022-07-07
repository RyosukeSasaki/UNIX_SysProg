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

void socket_conf(int *, struct addrinfo *, char *, char *);

int send_discover(int *, struct addrinfo *);
int send_request_alloc(int *, struct addrinfo *);
int send_request_ext(int *, struct addrinfo *);
int send_release(int *, struct addrinfo *);

uint16_t ttl_max;
struct in_addr addr;
struct in_addr netmask;

#endif
#include "mydhcpclient.h"

void socket_conf(int *sfd)
{}

int main()
{
    dhcp_message_t msg;
    msg.message.type = 0x12;
    msg.message.code = 0x34;
    msg.message.ttl = htons(0x5678);
    msg.message.ipaddr = htonl(0xAAAAbbbb);
    msg.message.netmask = htonl(0x22220000);

    //in_port_t port = 49152;
    //struct sockaddr_in addr;
    //memset(&addr, 0, sizeof addr);
    //addr.sin_family = AF_INET;
    //addr.sin_port = htons(port);
    //addr.sin_addr.s_addr = inet_addr("ipaddrstr");
    int err;
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof hints);
    hints.ai_socktype = SOCK_DGRAM;
    if ((err = getaddrinfo("localhost", "49152", &hints, &res)) < 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
        exit(1);
    }

    int sfd;
    if ((sfd=socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
        perror("socket");
        exit(-1);
    }
    int cnt;
    if ((cnt=sendto(sfd, msg.data, sizeof msg.data, 0, res->ai_addr, res->ai_addrlen)) < 0) {
        perror("sendto");
        exit(1);
    }

    freeaddrinfo(res);
}
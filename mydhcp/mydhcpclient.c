#include "mydhcpclient.h"

void socket_conf(int *sfd)
{}

int main()
{
    dhcp_message_t msg;
    msg.message.type = DHCP_TYPE_DISCOVER;
    msg.message.code = DHCP_CODE_OK;
    msg.message.ttl = htons(0x1122);
    msg.message.ipaddr = htonl(0x33445566);
    msg.message.netmask = htonl(0x778899aa);

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
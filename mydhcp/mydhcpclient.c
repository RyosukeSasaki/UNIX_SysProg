#include "mydhcpclient.h"

int main()
{
    dhcp_message_t msg;
    msg.message.type = DHCP_TYPE_DISCOVER;
    msg.message.code = DHCP_CODE_OK;
    msg.message.ttl = htons(0x1122);
    msg.message.ipaddr = htonl(0x33445566);
    msg.message.netmask = htonl(0x778899aa);
    int sfd;
    struct addrinfo *res;

    socket_conf(&sfd, res, "192.168.0.1", "49152");
    /**
    if ((cnt=sendto(sfd, msg.data, sizeof msg.data, 0, res->ai_addr, res->ai_addrlen)) < 0) {
        perror("sendto");
        exit(1);
    }
    **/

    freeaddrinfo(res);
}

void socket_conf(int *sfd, struct addrinfo *res, char *addrstr, char *portstr)
{
    int err;
    struct addrinfo hints;
    memset(&hints, 0, sizeof hints);
    hints.ai_socktype = SOCK_DGRAM;
    if ((err = getaddrinfo(addrstr, portstr, &hints, &res)) < 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
        exit(-1);
    }

    if ((sfd=socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
        perror("socket");
        exit(-1);
    }
}

int send_discover(int *sfd, struct addrinfo *res)
{
    int cnt;
    dhcp_message_t msg;
    memset(&msg, 0, sizeof msg.data);
    msg.message.type = DHCP_TYPE_DISCOVER;

    if ((cnt=sendto(*sfd, msg.data, sizeof msg.data, 0, res->ai_addr, res->ai_addrlen)) < 0) {
        perror("sendto");
        return -1;
    }
    return cnt;
}

int send_request_alloc(int *sfd, struct addrinfo *res)
{
    int cnt;
    dhcp_message_t msg;
    memset(&msg, 0, sizeof msg.data);
    msg.message.type = DHCP_TYPE_REQUEST;
    msg.message.code = DHCP_CODE_REQ_ALLOC;
    msg.message.ttl = ttl_max;
    msg.message.ipaddr = addr.s_addr;
    msg.message.netmask = netmask.s_addr;

    if ((cnt=sendto(*sfd, msg.data, sizeof msg.data, 0, res->ai_addr, res->ai_addrlen)) < 0) {
        perror("sendto");
        return -1;
    }
    return cnt;
}

int send_request_ext(int *sfd, struct addrinfo *res)
{
    int cnt;
    dhcp_message_t msg;
    memset(&msg, 0, sizeof msg.data);
    msg.message.type = DHCP_TYPE_REQUEST;
    msg.message.code = DHCP_CODE_REQ_EXT;
    msg.message.ttl = ttl_max;
    msg.message.ipaddr = addr.s_addr;
    msg.message.netmask = netmask.s_addr;

    if ((cnt=sendto(*sfd, msg.data, sizeof msg.data, 0, res->ai_addr, res->ai_addrlen)) < 0) {
        perror("sendto");
        return -1;
    }
    return cnt;
}

int send_release(int *sfd, struct addrinfo *res) {
    int cnt;
    dhcp_message_t msg;
    memset(&msg, 0, sizeof msg.data);
    msg.message.type = DHCP_TYPE_RELEASE;
    msg.message.ipaddr = addr.s_addr;

    if ((cnt=sendto(*sfd, msg.data, sizeof msg.data, 0, res->ai_addr, res->ai_addrlen)) < 0) {
        perror("sendto");
        return -1;
    }
    return cnt;
}
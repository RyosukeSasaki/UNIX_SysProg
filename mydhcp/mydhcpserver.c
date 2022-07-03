#include "mydhcpserver.h"

void fact1(client_t *client, int event) {};
void fact2(client_t *client, int event) {};
void fact3(client_t *client, int event) {};
void fact4(client_t *client, int event) {};
void fact5(client_t *client, int event) {};
void fact6(client_t *client, int event) {};
void fact7(client_t *client, int event) {};
void fact8(client_t *client, int event) {};
int create_client() {};

int wait_event(int sfd, struct sockaddr_in *fromaddr, socklen_t *addrsize) {
    uint8_t buf[1024];
    int cnt;
    dhcp_message_t msg;

    memset(buf, 0, sizeof buf);
    if ((cnt = recvfrom(sfd, buf, sizeof buf, 0, (struct sockaddr *)fromaddr, addrsize)) < 0) {
        perror("recvfrom");
        return cnt;
    }
    if (cnt == 0) return -1;

    #ifdef DEBUG
    fprintf(stderr, "%d byte received:\n", cnt);
    for (int i=0; i<cnt; i++) fprintf(stderr, "%"PRIx8" ", buf[i]);
    fprintf(stderr, "\n");
    #endif
    if (cnt == 12) {
        memcpy(msg.data, buf, cnt);
        msg.message.ttl = ntohs(msg.message.ttl);
        msg.message.ipaddr = ntohl(msg.message.ipaddr);
        msg.message.netmask = ntohl(msg.message.netmask);
        #ifdef DEBUG
        fprintf(stderr, "message received\n");
        fprintf(stderr, "type: %"PRIx8"\n", msg.message.type);
        fprintf(stderr, "code: %"PRIx8"\n", msg.message.code);
        fprintf(stderr, "ttl : %"PRIx16"\n", msg.message.ttl);
        fprintf(stderr, "ipad: %"PRIx32"\n", msg.message.ipaddr);
        fprintf(stderr, "netm: %"PRIx32"\n", msg.message.netmask);
        #endif
    }

    return cnt;
}

void sighup_handler() {stop = 1;}

void signal_conf()
{
    struct sigaction sa;
    sa.sa_handler = sighup_handler;
    if (sigaction(SIGHUP, &sa, NULL) < 0) {
        perror("sigaction");
        exit(0);
    }
}

void socket_conf(int *sfd)
{
    struct sockaddr_in addr;
    in_port_t port = 49152;
    if ((*sfd=socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(-1);
    }
    memset(&addr, 0, sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(*sfd, (struct sockaddr *)&addr, sizeof addr) < 0) {
        perror("bind");
        exit(-1);
    }
}

int main()
{
    event_t event;
    struct proctable *pt;
    int sfd;
    struct sockaddr_in  fromaddr;
    socklen_t addrsize;
    
    stop = 0;
    signal_conf();
    socket_conf(&sfd);

    while(!stop) {
        event = wait_event(sfd, &fromaddr, &addrsize);
        if (event == discover) {
            if ((create_client()) < 0) {}
        }
        for (pt=pstab; pt->stat; pt++) {
        }
    }
    fprintf(stderr, "bye\r\n");
    close(sfd);
};
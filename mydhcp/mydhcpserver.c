#include "mydhcpserver.h"

int main()
{
    int sfd;
    struct sockaddr_in fromaddr;
    socklen_t addrsize;
    dhcp_message_t msg;
    client_t *client_ptr;
    proctable_t *proc_ptr;
    event_t event;
    
    stop = 0;
    signal_conf();
    socket_conf(&sfd);
    addrsize = sizeof(fromaddr);

    while(!stop) {
        if (get_msg(sfd, &fromaddr, &addrsize, &msg) < 0) continue;
        // if timer interrupt occured
        for (client_ptr=client_h->fp; client_ptr!=client_h; client_ptr=client_ptr->fp) {
            if (client_ptr->addr.s_addr == fromaddr.sin_addr.s_addr) break;
        }
        if (client_ptr == client_h) {
            // client doesn't exist
            if (create_client() < 0) {
                fprintf(stderr, "failed to create new client\r\n");
            }
            continue;
        }
        // client_ptr->stat is the state of client
        event = get_event();
        for (proc_ptr=pstab; proc_ptr->stat; proc_ptr++) {
            if (client_ptr->stat == proc_ptr->stat && event == proc_ptr->event) {
                (*proc_ptr->func)(client_ptr, sfd);
            }
        }
    }
    fprintf(stderr, "bye\r\n");
    close(sfd);
};

void send_offer(client_t *client, int event) {};
void send_ack_ok(client_t *client, int event) {};
void send_ack_ng(client_t *client, int event) {};
void reset_ttl(client_t *client, int event) {};
void release_client(client_t *client, int event) {};
void resend_offer(client_t *client, int event) {};
int create_client() {};

int get_event() {};
int get_msg(int sfd, struct sockaddr_in *fromaddr, socklen_t *addrsize, dhcp_message_t *msg) 
{
    uint8_t buf[BUF_LEN];
    char fromaddrstr[16];
    int cnt;

    memset(buf, 0, sizeof buf);
    memset(msg, 0, sizeof msg->data);
    if ((cnt = recvfrom(sfd, buf, sizeof buf, 0, (struct sockaddr *)fromaddr, addrsize)) < 0) {
        perror("recvfrom");
        return cnt;
    }
    if (cnt == 0) return -1;
    if (inet_ntop(AF_INET, &fromaddr->sin_addr, fromaddrstr, sizeof fromaddrstr) < 0) {
        perror("inet_ntop");
        return -1;
    }
    fprintf(stderr, "## %d byte received from %s:%d ##\n", cnt,
        fromaddrstr, ntohs(fromaddr->sin_port));
    for (int i=0; i<cnt; i++) fprintf(stderr, "%#x ", buf[i]);
    fprintf(stderr, "\n");
    if (cnt == 12) {
        memcpy(msg->data, buf, cnt);
        msg->message.ttl = ntohs(msg->message.ttl);
        msg->message.ipaddr = ntohl(msg->message.ipaddr);
        msg->message.netmask = ntohl(msg->message.netmask);
        fprintf(stderr, "## dhcp message received ##\n");
        fprintf(stderr, "\ttype:      %#0x\n", msg->message.type);
        fprintf(stderr, "\tcode:      %#0x\n", msg->message.code);
        fprintf(stderr, "\tttl :      %#0x\n", msg->message.ttl);
        fprintf(stderr, "\tipaddr:    %#0x\n", msg->message.ipaddr);
        fprintf(stderr, "\tnetmask:   %#0x\n", msg->message.netmask);
    } else {
        fprintf(stderr, "received data doesn't match dhcp message format.\n");
        return -1;
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
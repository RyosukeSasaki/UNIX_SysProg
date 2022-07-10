#include "mydhcpclient.h"

int main(int argc, char *argv[])
{
    int ret;
    dhcp_message_t msg;
    struct sockaddr_in fromaddr;
    socklen_t addrsize = sizeof(fromaddr);
    event_t event;

    if (argc != 2) {
        fprintf(stderr, "Usage: mydhcpc server-ip\n");
        exit(1);
    }

    stop_flag = 0;
    sighup_flag = 0;
    sigint_flag = 0;
    alarm_flag = 0;
    tout = MESSAGE_TIMEOUT;
    alarm_conf();
    signal_conf();
    socket_conf(&res, argv[1], PORT);

    change_state(init);
    send_discover();

    while(!stop_flag) {
        event = -1;
        ret = get_msg(&fromaddr, &addrsize, &msg);

        if (alarm_flag > 0) {
            // if timer interrupt occured
            switch (stat) {
                case in_use:
                case wait_ext_ack:
                    decrement_ttl();
                    break;
                case wait_offer1:
                case wait_offer2:
                case wait_ack1:
                case wait_ack2:
                    decrement_tout();
                    break;
            }
            alarm_flag--;
        }

        if (ret > 0) {
            event = get_event(&msg);
        } else if (ret == UNKNOWN_MSG) {
            event = unknown_msg;
        } else if (sighup_flag) {
            event = sighup;
            sighup_flag = 0;
        } else if (sigint_flag) {
            fprintf(stderr, "Terminate without sending RELEASE\n");
            stop_flag = 1;
        } else {
            if (ttl <= ttlcounter/2 && 0 < ttl) {
                event = half_ttl;
            } else if (tout <= 0) {
                fprintf(stderr, "## message timeout ##\n");
                event = timeout;
            } else if (ttl <= 0) {
                event = ttl_timeout;
            }
        }
        FSM_func(event);
    }
    fprintf(stderr, "bye\n");
    close(sfd);
    freeaddrinfo(res);
}

void decrement_ttl() {
    ttl--;
    fprintf(stderr, "-- Time to Live: %d --\n", ttl);
}

void decrement_tout() {
    tout--;
    fprintf(stderr, "-- waiting message, remaining time: %d --\n", tout);
}

int FSM_func(int event)
{
    for (proctable_t *proc_ptr=pstab; proc_ptr->stat; proc_ptr++) {
        if (stat == proc_ptr->stat && event == proc_ptr->event) {
            return (*proc_ptr->func)();
        }
    }
}

void sighup_handler() {sighup_flag=1;}
void sigalrm_handler() {alarm_flag++;}
void sigint_handler() {sigint_flag=1;}
void signal_conf()
{
    struct sigaction sa;
    sa.sa_handler = sighup_handler;
    if (sigaction(SIGHUP, &sa, NULL) < 0) {
        perror("sigaction");
        exit(-1);
    }
    sa.sa_handler = sigalrm_handler;
    if (sigaction(SIGALRM, &sa, NULL) < 0) {
        perror("sigaction");
        exit(-1);
    }
    sa.sa_handler = sigint_handler;
    if (sigaction(SIGINT, &sa, NULL) < 0) {
        perror("sigaction");
        exit(-1);
    }
}

void alarm_conf()
{
    struct itimerval itimer;
    itimer.it_interval.tv_sec = 1;
    itimer.it_interval.tv_usec = 0;
    itimer.it_value = itimer.it_interval;
    if (setitimer(ITIMER_REAL, &itimer, NULL) < 0) {
        perror("setitimer");
        exit(-1);
    }
}

void socket_conf(struct addrinfo **res, char *addrstr, char *portstr)
{
    int err;
    struct addrinfo hints;
    memset(&hints, 0, sizeof hints);
    hints.ai_socktype = SOCK_DGRAM;
    if ((err = getaddrinfo(addrstr, portstr, &hints, res)) < 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
        exit(-1);
    }

    if ((sfd=socket((*res)->ai_family, (*res)->ai_socktype, (*res)->ai_protocol)) < 0) {
        perror("socket");
        exit(-1);
    }
}

int get_msg(struct sockaddr_in *fromaddr, socklen_t *addrsize, dhcp_message_t *msg) 
{
    uint8_t buf[BUF_LEN];
    struct in_addr addrbuf;
    char fromaddrstr[16];
    int cnt;

    memset(buf, 0, sizeof buf);
    memset(msg, 0, sizeof msg->data);
    if ((cnt = recvfrom(sfd, buf, sizeof buf, 0, (struct sockaddr *)fromaddr, addrsize)) < 0) {
        //perror("recvfrom");
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
        // msg->message.ipaddr = ntohl(msg->message.ipaddr);
        // msg->message.netmask = ntohl(msg->message.netmask);
        fprintf(stderr, "## dhcp message received ##\n");
        fprintf(stderr, "\ttype:      %s\n", dhcp_type_str[msg->message.type].str);
        fprintf(stderr, "\tcode:      %s\n", dhcp_code_str[msg->message.code].str);
        fprintf(stderr, "\tttl :      %d\n", msg->message.ttl);
        addrbuf.s_addr = msg->message.ipaddr;
        fprintf(stderr, "\tipaddr:    %s\n", inet_ntoa(addrbuf));
        addrbuf.s_addr = msg->message.netmask;
        fprintf(stderr, "\tnetmask:   %s\n", inet_ntoa(addrbuf));
    } else {
        fprintf(stderr, "received data doesn't match dhcp message format.\n");
        return UNKNOWN_MSG;
    }

    return cnt;
}

// network byte order
int send_dhcp(dhcp_message_t *msg)
{
    int cnt;
    struct sockaddr_in sendaddr;

    if ((cnt=sendto(sfd, msg->data, sizeof(msg->data), 0, res->ai_addr, res->ai_addrlen)) < 0) {
        perror("sendto");
        return -1;
    }
    return cnt;
}

void change_state(int _stat)
{
    int oldstat = stat;
    stat = _stat;
    tout = MESSAGE_TIMEOUT;
    fprintf(stderr, "## change state: %s to %s ##\n", ststr[oldstat].str, ststr[stat].str);
    if (_stat == term) {
        stop_flag = 1;
    }
}

int send_discover()
{
    int ret;
    dhcp_message_t msg;
    memset(&msg, 0, sizeof(msg.data));
    msg.message.type = DHCP_TYPE_DISCOVER;

    fprintf(stderr, "## send DISCOVER ##\n");
    ret = send_dhcp(&msg);
    change_state(wait_offer1);
    return ret;
}

int send_request_alloc()
{
    int ret;
    dhcp_message_t msg;
    memset(&msg, 0, sizeof(msg.data));
    msg.message.type = DHCP_TYPE_REQUEST;
    msg.message.code = DHCP_CODE_REQ_ALLOC;
    msg.message.ttl = htons(ttlcounter);
    msg.message.ipaddr = addr.s_addr;
    msg.message.netmask = netmask.s_addr;

    ret = send_dhcp(&msg);
    fprintf(stderr, "## send REQUEST ALLOCATION ##\n");
    change_state(wait_ack1);
    return ret;
}

int send_request_ext()
{
    int ret;
    dhcp_message_t msg;
    memset(&msg, 0, sizeof(msg.data));
    msg.message.type = DHCP_TYPE_REQUEST;
    msg.message.code = DHCP_CODE_REQ_EXT;
    msg.message.ttl = htons(ttlcounter);
    msg.message.ipaddr = addr.s_addr;
    msg.message.netmask = netmask.s_addr;

    fprintf(stderr, "## send REQUEST EXTENSION ##\n");
    ret = send_dhcp(&msg);
    change_state(wait_ext_ack);
    return ret;
}

int send_release()
{
    int ret;
    dhcp_message_t msg;
    memset(&msg, 0, sizeof(msg.data));
    msg.message.type = DHCP_TYPE_RELEASE;
    msg.message.ipaddr = addr.s_addr;

    fprintf(stderr, "## send RELEASE ##\n");
    ret = send_dhcp(&msg);
    change_state(term);
    return ret;
}

int start_using()
{
    ttl = ttlcounter;
    change_state(in_use);
    return 0;
}

int resend_discover()
{
    int ret;
    dhcp_message_t msg;
    memset(&msg, 0, sizeof(msg.data));
    msg.message.type = DHCP_TYPE_DISCOVER;

    fprintf(stderr, "## REsend DISCOVER ##\n");
    ret = send_dhcp(&msg);
    change_state(wait_offer2);
    return ret;
}

int resend_request_alloc()
{
    int ret;
    dhcp_message_t msg;
    memset(&msg, 0, sizeof(msg.data));
    msg.message.type = DHCP_TYPE_REQUEST;
    msg.message.code = DHCP_CODE_REQ_ALLOC;
    msg.message.ttl = htons(ttlcounter);
    msg.message.ipaddr = addr.s_addr;
    msg.message.netmask = netmask.s_addr;

    ret = send_dhcp(&msg);
    fprintf(stderr, "## REsend REQUEST ALLOCATION ##\n");
    change_state(wait_ack2);
    return ret;
}

int terminate()
{
    change_state(term);
    return 0;
}

int recv_offer(dhcp_message_t *msg)
{
    int ok=offer_ok, err=offer_ng;
    if (msg->message.code == DHCP_CODE_ERR_NOIP) {
        fprintf(stderr, "## OFFER arrived, server's IP is depleted ##\n");
        stop_flag = 1;
        return err;
    }
    ttlcounter = msg->message.ttl;
    addr.s_addr = msg->message.ipaddr;
    netmask.s_addr = msg->message.netmask;
    fprintf(stderr, "## OFFER arrived, ip allocated ##\n");
    fprintf(stderr, "\tipaddr:  %s\n", inet_ntoa(addr));
    fprintf(stderr, "\tnetmask: %s\n", inet_ntoa(netmask));
    return ok;
}

int recv_ack(dhcp_message_t *msg)
{
    int ok=ack_ok, err=ack_ng;
    int ret;
    if (msg->message.code == DHCP_CODE_ERR_REQ) {
        fprintf(stderr, "## ACK arrived, REQUEST message is invalid ##\n");
        return err;
    }
    ttlcounter = msg->message.ttl;
    addr.s_addr = msg->message.ipaddr;
    netmask.s_addr = msg->message.netmask;
    fprintf(stderr, "## ACK arrived ##\n");
    return ok;
}

int get_event(dhcp_message_t *msg)
{
    parse_message_t *pmsg_ptr;
    for (pmsg_ptr=pmsgtab; pmsg_ptr->type; pmsg_ptr++) {
        if (pmsg_ptr->type == msg->message.type) return (*pmsg_ptr->func)(msg);
    }
    if (pmsg_ptr->type == 0) return -1;
}
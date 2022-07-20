#include "mydhcpserver.h"

int main(int argc, char *argv[])
{
    int sfd, addr_pool_size=1, ret;
    struct sockaddr_in fromaddr;
    socklen_t addrsize = sizeof(fromaddr);
    dhcp_message_t msg;
    client_t *client_ptr;
    event_t event;
    struct addrinfo *res;
    char *filepath;
    stop_flag = 0;
    alarm_flag = 0;
    alarm_conf();
    signal_conf();
    socket_conf(&sfd, PORT);
    initialize_client_buffer();
    
    if (argc != 2) {
        fprintf(stderr, "Usage: mydhcpd config-file\n");
        exit(-1);
    }
    filepath = normalize_path(argv[1]);
    fprintf(stderr, "## config file: %s ##\n", filepath);
    read_config(filepath);
    free(filepath);

    while(!stop_flag) {
        event = -1;
        ret = get_msg(&sfd, &fromaddr, &addrsize, &msg);

        if (alarm_flag > 0) {
            // if timer interrupt occured
            for (client_t *ptr=client_h.fp; ptr!=&client_h; ptr=ptr->fp) {
                if (ptr->stat == in_use) decrement_ttl(ptr);
                if (ptr->stat == wait_req1) decrement_tout(ptr);
                if (ptr->stat == wait_req2) decrement_tout(ptr);
            }
            alarm_flag--;
        }

        if (ret > 0) {
            for (client_ptr=client_h.fp; client_ptr!=&client_h; client_ptr=client_ptr->fp) {
                if (client_ptr->id.s_addr == fromaddr.sin_addr.s_addr &&
                client_ptr->port == fromaddr.sin_port) break;
            }
            if (client_ptr == &client_h) {
                if (msg.message.type == DHCP_TYPE_DISCOVER) {
                    if ((client_ptr=create_client(&fromaddr)) == NULL) {
                        fprintf(stderr, "## failed to create new client ##\n");
                        send_offer_ng(&sfd, &fromaddr);
                        continue;
                    }
                }
            }
            event = get_event(&msg, client_ptr);
        } else if (ret == UNKNOWN_MSG) {
            event = unknown_msg;
        } else {
            for (client_t *ptr=client_h.fp; ptr!=&client_h; ptr=ptr->fp) {
                if (ptr->ttl <= 0) {
                    event = ttl_timeout;
                    if (FSM_func(&sfd, ptr, event) >= 0) {
                        ptr=&client_h;
                    }
                }
            }
            for (client_t *ptr=client_h.fp; ptr!=&client_h; ptr=ptr->fp) {
                if (ptr->tout <= 0) {
                    event = req_timeout;
                    fprintf(stderr, "## client %s, message timeout ##\n", inet_ntoa(ptr->addr->addr));
                    if (FSM_func(&sfd, ptr, event) >= 0) {
                        ptr=&client_h;
                    }
                }
            }
            continue;
        }
        FSM_func(&sfd, client_ptr, event);
    }
    for (client_t *ptr=client_h.fp; ptr!=&client_h; ptr=ptr->fp) {
        fprintf(stderr, "delete %s\n", inet_ntoa(ptr->addr->addr));
        if (release_client(&sfd, ptr) >= 0) ptr=&client_h;
    }
    close(sfd);
    fprintf(stderr, "bye\r\n");
}

int FSM_func(int *sfd, client_t *client_ptr, int event)
{
    for (proctable_t *proc_ptr=pstab; proc_ptr->stat; proc_ptr++) {
        if (client_ptr->stat == proc_ptr->stat && event == proc_ptr->event) {
            return (*proc_ptr->func)(sfd, client_ptr);
        }
    }
    return -1;
}

void decrement_ttl(client_t *client) {
    client->ttl--;
    fprintf(stderr, "-- client %s, ttl: %d --\n", inet_ntoa(client->addr->addr), client->ttl);
}

void decrement_tout(client_t *client) {
    client->tout--;
    fprintf(stderr, "-- client %s, waiting message, remaining time: %d --\n", inet_ntoa(client->addr->addr), client->tout);
}

int get_event(dhcp_message_t *msg, client_t *client)
{
    parse_message_t *pmsg_ptr;
    for (pmsg_ptr=pmsgtab; pmsg_ptr->type; pmsg_ptr++) {
        if (pmsg_ptr->type == msg->message.type) return (*pmsg_ptr->func)(msg, client);
    }
    if (pmsg_ptr->type == 0) return -1;
}

int get_msg(int *sfd, struct sockaddr_in *fromaddr, socklen_t *addrsize, dhcp_message_t *msg) 
{
    uint8_t buf[BUF_LEN];
    struct in_addr addrbuf;
    char fromaddrstr[16];
    int cnt;

    memset(buf, 0, sizeof(buf));
    memset(msg, 0, sizeof(msg->data));
    if ((cnt = recvfrom(*sfd, buf, sizeof(buf), 0, (struct sockaddr *)fromaddr, addrsize)) < 0) {
        //perror("recvfrom");
        return cnt;
    }
    if (cnt == 0) return -1;
    if (inet_ntop(AF_INET, &fromaddr->sin_addr, fromaddrstr, sizeof(fromaddrstr)) < 0) {
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
        //msg->message.ipaddr = ntohl(msg->message.ipaddr);
        //msg->message.netmask = ntohl(msg->message.netmask);
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


void sigalrm_handler() {alarm_flag++;}
void sigint_handler() {stop_flag=1;}
void sighup_handler() {stop_flag=1;}
void signal_conf()
{
    struct sigaction sa;
    sa.sa_handler = sighup_handler;
    if (sigaction(SIGHUP, &sa, NULL) < 0) {
        perror("sigaction");
        exit(-1);
    }
    sa.sa_handler = sigint_handler;
    if (sigaction(SIGINT, &sa, NULL) < 0) {
        perror("sigaction");
        exit(-1);
    }
    sa.sa_handler = sigalrm_handler;
    if (sigaction(SIGALRM, &sa, NULL) < 0) {
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

void socket_conf(int *sfd, in_port_t port)
{
    struct sockaddr_in addr;
    if ((*sfd=socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(-1);
    }
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(*sfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(-1);
    }
}

void read_config(char *path)
{
    FILE *fp;
    char lbuf[1024];
    char *argv[3];
    int ttl, argc, n=0;

    if ((fp = fopen(path, "r")) == NULL) {
        perror("fopen");
        exit(-1);
    }
    if (fgets(lbuf, sizeof(lbuf), fp) != NULL) {
        ttl = atoi(lbuf);
        if (ttl > UINT16_MAX && ttl < 1) {
            fprintf(stderr, "TTL must be in 1 and 65535");
            fclose(fp);
            exit(-1);
        }
        ttl_max = ttl;
        fprintf(stderr, "Time to Live: %d\n", ttl_max);
    } else {
        fprintf(stderr, "TTL is not given\n");
    }
    fprintf(stderr, "## initialize address pool ##\n");
    initialize_addr_pool();
    while (fgets(lbuf, sizeof(lbuf), fp) != NULL) {
        // create addr pool
        getargs(&argc, argv, lbuf);
        if (argc != 2) {
            fprintf(stderr, "config file is invalid\n");
            exit(-1);
        }
        if (inet_aton(argv[0], &addr_pool[n].addr) < 0) {
            fprintf(stderr, "failed to load IP value %s\n", argv[0]);
        }
        if (inet_aton(argv[1], &addr_pool[n].netmask) < 0) {
            fprintf(stderr, "failed to netmask value %s\n", argv[1]);
        }
        insert_addr_pool(&addr_pool[n]);
        n++;
    }
    show_addr_pool();

    fclose(fp);
}

void show_addr_pool()
{
    fprintf(stderr, "** address pool **\n");
    fprintf(stderr, "\tip address / netmask\n");
    for (addr_pool_t *ptr=addr_pool_h.bp; ptr!=&addr_pool_h; ptr=ptr->bp) {
        fprintf(stderr, "\t%s / ", inet_ntoa(ptr->addr));
        fprintf(stderr, "\t%s\n", inet_ntoa(ptr->netmask));
    }
}

// free after use
char *normalize_path(char *arg)
{
    char relpath[path_max];
    char *fullpath;
    char *home;
    
    if (arg[0] == '~') {
        if ((home=getenv("HOME")) == NULL) return NULL;
        if (strlen(home)+strlen(arg) > path_max -1) {
            fprintf(stderr, "path string is too long\r\n");
            return NULL;
        }
        memset(relpath, 0, sizeof(relpath));
        strcat(relpath, home);
        strcat(relpath, &arg[1]);
        if ((fullpath = realpath(relpath, NULL))==NULL) {
            perror("realpath");
            return NULL;
        }
    } else {
        if ((fullpath = realpath(arg, NULL))==NULL) {
            perror("realpath");
            return NULL;
        }
    }

    return fullpath;
}

void getargs(int *argc, char *argv[], char *lbuf)
{
    *argc = 0;
    argv[0] = NULL;

    while(1) {
        while(isblank(*lbuf)) lbuf++;
        if(*lbuf == '\0') return;
        argv[(*argc)++] = lbuf;

        while(*lbuf && !isblank(*lbuf)) lbuf++;
        if(*argc > 2) return;
        if(*lbuf == '\0') return;
        *lbuf++ = '\0';
    }
}

void initialize_addr_pool()
{
    addr_pool_h.fp = &addr_pool_h;
    addr_pool_h.bp = &addr_pool_h;
}

void insert_addr_pool(addr_pool_t *new)
{
    addr_pool_h.bp->fp = new;
    new->bp = addr_pool_h.bp;
    addr_pool_h.bp = new;
    new->fp = &addr_pool_h;
}

addr_pool_t *fetch_addr_pool()
{
    addr_pool_t *ret;
    ret = addr_pool_h.fp;
    addr_pool_h.fp->fp->bp = &addr_pool_h;
    addr_pool_h.fp = addr_pool_h.fp->fp;
    return ret;
}

int is_addr_pool_empty()
{
    int num = 0;
    addr_pool_t *ptr = addr_pool_h.fp;
    while(ptr != &addr_pool_h) {
        num++;
        ptr = ptr->fp;
    }
    return num;
}

void initialize_client_buffer() {
    client_h.fp = &client_h;
    client_h.bp = &client_h;
}

client_t *create_client(struct sockaddr_in *fromaddr)
{
    client_t *new;
    if (is_addr_pool_empty()) {
        new = (client_t *)malloc(sizeof(client_t));
        memset(new, 0, sizeof(client_t));
        new->id = fromaddr->sin_addr;
        new->port = fromaddr->sin_port;
        new->addr = fetch_addr_pool();
        new->tout = MESSAGE_TIMEOUT;
        change_state(new, init);
        insert_client(new);
        fprintf(stderr, "## new client %s created ##\n", inet_ntoa(new->addr->addr));
        fprintf(stderr, "\tid  :    %s\n", inet_ntoa(new->id));
        fprintf(stderr, "\tport:    %d\n", ntohs(new->port));
        return new;
    }
    return NULL;

};

void insert_client(client_t *new)
{
    new->bp = &client_h;
    new->fp = client_h.fp;
    client_h.fp->bp = new;
    client_h.fp = new;
}

void remove_client(client_t *ptr)
{
    ptr->bp->fp = ptr->fp;
    ptr->fp->bp = ptr->bp;
    free(ptr);
}

// send function (change state)
void change_state(client_t *client, int stat)
{
    int oldstat = client->stat;
    client->stat = stat;
    client->tout = MESSAGE_TIMEOUT;
    fprintf(stderr, "## client %s change its state: %s to %s ##\n", inet_ntoa(client->addr->addr),
    ststr[oldstat].str, ststr[stat].str);
    if (stat == term) {
        insert_addr_pool(client->addr);
        fprintf(stderr, "## ip %s returned to addr pool ##\n", inet_ntoa(client->addr->addr));
        show_addr_pool();
        remove_client(client);
        fprintf(stderr, "## client deleted ##\n");
    }
}

// network byte order
int send_dhcp(int *sfd, dhcp_message_t *msg, in_addr_t addr, in_port_t port)
{
    int cnt;
    struct sockaddr_in sendaddr;

    sendaddr.sin_family = AF_INET;
    sendaddr.sin_port = port;
    sendaddr.sin_addr.s_addr = addr;

    if ((cnt=sendto(*sfd, msg->data, sizeof(msg)->data, 0, (struct sockaddr *)&sendaddr, sizeof(sendaddr))) < 0) {
        perror("sendto");
        return -1;
    }
    return cnt;
}

int send_offer_ok(int *sfd, client_t *client)
{
    int ret;
    dhcp_message_t msg;
    memset(&msg, 0, sizeof(msg.data));
    msg.message.type = DHCP_TYPE_OFFER;
    msg.message.code = DHCP_CODE_OK;
    msg.message.ttl = htons(ttl_max);
    msg.message.ipaddr = client->addr->addr.s_addr;
    msg.message.netmask = client->addr->netmask.s_addr;

    fprintf(stderr, "## send OFFER OK to %s:%d ##\n", inet_ntoa(client->id), htons(client->port));
    ret = send_dhcp(sfd, &msg, client->id.s_addr, client->port);
    change_state(client, wait_req1);
    return ret;
}

int send_offer_ng(int *sfd, struct sockaddr_in *fromaddr)
{
    dhcp_message_t msg;
    memset(&msg, 0, sizeof(msg.data));
    msg.message.type = DHCP_TYPE_OFFER;
    msg.message.code = DHCP_CODE_ERR_NOIP;

    fprintf(stderr, "## send OFFER NG to %s:%d ##\n", inet_ntoa(fromaddr->sin_addr), htons(fromaddr->sin_port));
    return send_dhcp(sfd, &msg, fromaddr->sin_addr.s_addr, fromaddr->sin_port);
}

int send_ack_ok(int *sfd, client_t *client)
{
    int ret;
    dhcp_message_t msg;
    memset(&msg, 0, sizeof(msg.data));
    msg.message.type = DHCP_TYPE_ACK;
    msg.message.code = DHCP_CODE_OK;
    msg.message.ttl = htons(client->ttlcounter);
    msg.message.ipaddr = client->addr->addr.s_addr;
    msg.message.netmask = client->addr->netmask.s_addr;
    client->ttl = client->ttlcounter;

    fprintf(stderr, "## send ACK OK to %s:%d ##\n", inet_ntoa(client->id), htons(client->port));
    ret = send_dhcp(sfd, &msg, client->id.s_addr, client->port);
    change_state(client, in_use);
    return ret;
}

int send_ack_ng(int *sfd, client_t *client)
{
    int ret;
    dhcp_message_t msg;
    memset(&msg, 0, sizeof(msg.data));
    msg.message.type = DHCP_TYPE_ACK;
    msg.message.code = DHCP_CODE_ERR_REQ;

    fprintf(stderr, "## send ACK NG to %s:%d ##\n", inet_ntoa(client->id), htons(client->port));
    ret = send_dhcp(sfd, &msg, client->id.s_addr, client->port);
    change_state(client, term);
    return ret;
}

// receive function (generate event)
int recv_discover(dhcp_message_t *msg, client_t *client)
{
    if (msg->message.type != DHCP_TYPE_DISCOVER) return unknown_msg;
    if (msg->message.code != 0) return unknown_msg;
    if (msg->message.ttl != 0) return unknown_msg;
    if (msg->message.ipaddr != 0) return unknown_msg;
    if (msg->message.netmask != 0) return unknown_msg;
    fprintf(stderr, "## DISCOVER arrived ##\n");
    return discover;
}

int recv_request(dhcp_message_t *msg, client_t *client)
{
    int ok, err;
    int ret;
    if (msg->message.code == DHCP_CODE_REQ_ALLOC) {
        ok = request_alloc_ok; err = request_alloc_ng;
    } else if (msg->message.code == DHCP_CODE_REQ_EXT) {
        ok = request_ext_ok; err = request_ext_ng;
    } else {
        return unknown_msg;
    }
    ret = ok;
    fprintf(stderr, "## REQUEST arrived ##\n");
    if (msg->message.ttl > ttl_max) ret = err;
    client->ttlcounter = msg->message.ttl;
    if (msg->message.ipaddr != client->addr->addr.s_addr) ret = err;
    if (msg->message.netmask != client->addr->netmask.s_addr) ret = err;
    if (ret == err) {
        fprintf(stderr, "## REQUEST message invalid ##\n");
    }
    return ret;
}

int recv_release(dhcp_message_t *msg, client_t *client) 
{
    int ok=release_ok, err=release_ng;
    int ret=ok;
    fprintf(stderr, "## RELEASE arrived ##\n");
    if (msg->message.code != 0) ret = err;
    if (msg->message.ttl != 0) ret = err;
    if (msg->message.netmask != 0) ret = err;
    if (msg->message.ipaddr != client->addr->addr.s_addr) ret = err;
    if (ret == err) {
        fprintf(stderr, "## RELEASE message invalid, continue ##\n");
    }
    return ret;
}

int reset_ttl(int *sfd, client_t *client)
{
    client->ttl = client->ttlcounter;
    fprintf(stderr, "## TTL reseted ##\n");
    fprintf(stderr, "-- client %s, ttl: %d --\n", inet_ntoa(client->addr->addr), client->ttl);
    return send_ack_ok(sfd, client);
}

int resend_offer(int *sfd, client_t *client)
{
    int ret;
    dhcp_message_t msg;
    memset(&msg, 0, sizeof(msg.data));
    msg.message.type = DHCP_TYPE_OFFER;
    msg.message.code = DHCP_CODE_OK;
    msg.message.ttl = htons(ttl_max);
    msg.message.ipaddr = client->addr->addr.s_addr;
    msg.message.netmask = client->addr->netmask.s_addr;

    fprintf(stderr, "## REsend OFFER OK to %s:%d ##\n", inet_ntoa(client->id), htons(client->port));
    ret = send_dhcp(sfd, &msg, client->id.s_addr, client->port);
    change_state(client, wait_req2);
    return ret;
}

int release_client(int *sfd, client_t *client)
{
    fprintf(stderr, "## TERMINATE client %s ##\n", inet_ntoa(client->addr->addr));
    change_state(client, term);
    return 0;
}
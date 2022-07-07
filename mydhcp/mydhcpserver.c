#include "mydhcpserver.h"

int main(int argc, char *argv[])
{
    int sfd, addr_pool_size=1, ret;
    struct sockaddr_in fromaddr;
    socklen_t addrsize = sizeof(fromaddr);
    dhcp_message_t msg;
    client_t *client_ptr;
    proctable_t *proc_ptr;
    event_t event;
    struct addrinfo *res;
    char *filepath;
    
    if (argc != 2) {
        fprintf(stderr, "Usage: mydhcpd config-file\n");
        exit(-1);
    }
    filepath = normalize_path(argv[1]);
    fprintf(stderr, "## config file: %s ##\n", filepath);
    read_config(filepath);
    free(filepath);

    stop_flag = 0;
    signal_conf();
    socket_conf(&sfd, 49152);

    while(!stop_flag) {
        ret = get_msg(sfd, &fromaddr, &addrsize, &msg);
        if (alarm_flag > 0) {
            // if timer interrupt occured
            alarm_flag--;
            continue;
        }
        for (client_ptr=client_h.fp; client_ptr!=&client_h; client_ptr=client_ptr->fp) {
            if (client_ptr->addr->addr.s_addr == fromaddr.sin_addr.s_addr) break;
        }
        if (client_ptr == &client_h) {
            if (recv_discover(&msg) == discover) {
                if (create_client(&fromaddr) < 0) fprintf(stderr, "failed to create new client\n");
            } else {
                fprintf(stderr, "got unknown message\n");
            }
            continue;
        }
        if (ret > 0) {
            event = get_event(&msg, client_ptr);
        } else { event = unknown_msg; }

        for (proc_ptr=pstab; proc_ptr->stat; proc_ptr++) {
            if (client_ptr->stat == proc_ptr->stat && event == proc_ptr->event) {
                (*proc_ptr->func)(&sfd, client_ptr);
            }
        }
    }
    fprintf(stderr, "bye\r\n");
    close(sfd);
};


int get_event(dhcp_message_t *msg, client_t *client)
{
    parse_message_t *pmsg_ptr;
    for (pmsg_ptr=pmsgtab; pmsg_ptr->type; pmsg_ptr++) {
        if (pmsg_ptr->type == msg->message.type) return (*pmsg_ptr->func)(msg, client);
    }
    if (pmsg_ptr->type == 0) return -1;
};

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


void alarm_handler() {}
void sighup_handler() {stop_flag = 1;}
void signal_conf()
{
    struct sigaction sa;
    sa.sa_handler = sighup_handler;
    if (sigaction(SIGHUP, &sa, NULL) < 0) {
        perror("sigaction");
        exit(0);
    }
}

void socket_conf(int *sfd, in_port_t port)
{
    struct sockaddr_in addr;
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

int recv_discover(dhcp_message_t *msg)
{
    if (msg->message.type != DHCP_TYPE_DISCOVER) return unknown_msg;
    if (msg->message.code != 0) return unknown_msg;
    if (msg->message.ttl != 0) return unknown_msg;
    if (msg->message.ipaddr != 0) return unknown_msg;
    if (msg->message.netmask != 0) return unknown_msg;
    return discover;
}

int recv_request(dhcp_message_t *msg, client_t *client)
{
    int ok, err;
    if (msg->message.code == DHCP_CODE_REQ_ALLOC) {
        ok = request_alloc_ok; err = request_alloc_ng;
    } else if (msg->message.code == DHCP_CODE_REQ_EXT) {
        ok = request_ext_ok; err = request_ext_ng;
    } else {
        return unknown_msg;
    }
    if (msg->message.ttl > ttl_max) return err;
    if (msg->message.ipaddr != client->addr->addr.s_addr) return err;
    if (msg->message.netmask != client->addr->netmask.s_addr) return err;
    return ok;
}

int recv_relase(dhcp_message_t *msg, client_t *client) 
{
    int ok=release_ok, err=release_ng;
    if (msg->message.code != 0) return err;
    if (msg->message.ttl != 0) return err;
    if (msg->message.netmask != 0) return err;
    if (msg->message.ipaddr != client->addr->addr.s_addr) return err;
    return ok;
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
    if (fgets(lbuf, sizeof lbuf, fp) != NULL) {
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
    while (fgets(lbuf, sizeof lbuf, fp) != NULL) {
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
    fprintf(stderr, "## address pool ##\n");
    fprintf(stderr, "ip address / netmask\n");
    for (addr_pool_t *ptr=addr_pool_h.bp; ptr!=&addr_pool_h; ptr=ptr->bp) {
        fprintf(stderr, "%s / ", inet_ntoa(ptr->addr));
        fprintf(stderr, "%s\n", inet_ntoa(ptr->netmask));
    }

    fclose(fp);
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

void initalize_client_buffer() {
    client_h.fp = &client_h;
    client_h.bp = &client_h;
}

int create_client(struct sockaddr_in *fromaddr)
{
    client_t *new;
    if (is_addr_pool_empty()) {
        new = (client_t *)malloc(sizeof(client_t));
        new->id = fromaddr->sin_addr;
        new->port = fromaddr->sin_port;
        new->addr = fetch_addr_pool();
        new->stat = init;
        insert_client(new);
        return 0;
    }
    return -1;

};

void insert_client(client_t *new)
{
    client_h.bp->fp = new;
    new->bp = client_h.bp;
    client_h.bp = new;
    new->fp = &client_h;
}

void remove_client(client_t *ptr)
{
    ptr->bp = ptr->fp;
    ptr->fp = ptr->bp;
    free(ptr);
}

void send_offer(int *sfd, client_t *client) {};
void send_ack_ok(int *sfd, client_t *client) {};
void send_ack_ng(int *sfd, client_t *client) {};
void reset_ttl(int *sfd, client_t *client) {};
void release_client(int *sfd, client_t *client) {};
void resend_offer(int *sfd, client_t *client) {};

#include "myftpc.h"

int main(int argc, char *argv[])
{
    fd_set rdfds;
    char buf[1024];
    if (argc != 2) {
        fprintf(stderr, "usage: myftpc ADDR\n");
        exit(1);
    }
    if (sock_conf(argv[1]) < 0) exit(1);
    FD_ZERO(&rdfds);
    FD_SET(STDIN_FILENO, &rdfds);
    FD_SET(sfd, &rdfds);

    while(1) {
        fprintf(stderr, "\x1b[34mmyFTP%%\x1b[0m ");
        if (select(sfd+1, &rdfds, NULL, NULL, NULL) < 0) {
            perror("select");
        }
        if (FD_ISSET(STDIN_FILENO, &rdfds)) exec_command();
        if (FD_ISSET(sfd, &rdfds)) {
            if (recv_msg(NULL, 1) < 0) exit(1);
        }
    }
}

int recv_msg(void *msg, int len)
{
    int cnt=0, ret;
    struct timeval tv;
    tv.tv_sec=TIMEOUT;
    tv.tv_usec=0;
    fd_set rdfds;
    FD_ZERO(&rdfds);
    FD_SET(sfd, &rdfds);

    while (cnt < len) {
        if ((ret=select(sfd+1, &rdfds, NULL, NULL, &tv)) < 0) {
            perror("select");
            return -1;
        } else if (ret == 0) {
            fprintf(stderr, "## timeout ##\n");
            return -1;
        }
        if ((ret=recv(sfd, msg, len-cnt, 0)) <= 0) {
            if (errno != 0) perror("recv");
            return -1;
        }
        cnt += ret;
    }
    return 0;
}

int exec_command()
{
    char lbuf[BUF_LEN], *av[NARGS];
    int ac;
    struct command_table_t* p;
    if(fgets(lbuf, sizeof lbuf, stdin) == NULL) {
        fprintf(stderr, "\n");
        return 0;
    }
    lbuf[strlen(lbuf) - 1] = '\0';
    getargs(&ac, av, lbuf);
    if(ac < 1) return 0;

    for(p = cmd_tbl; p->cmd; p++) {
        if(strcmp(av[0], p->cmd) == 0) {
            (p->func)(&ac, av);
            break;
        }
    }
    if(p->cmd == NULL) {
        fprintf(stderr, "%s: Unknown command. Please Try help.\r\n", av[0]);
        return -1;
    }
}

int getargs(int *argc, char *argv[], char *lbuf)
{
    *argc = 0;
    argv[0] = NULL;

    while(1) {
        while(isblank(*lbuf)) lbuf++;
        if(*lbuf == '\0') return 0;
        argv[(*argc)++] = lbuf;

        if (*argc > NARGS) return -1;
        while(*lbuf && !isblank(*lbuf)) lbuf++;
        if(*lbuf == '\0') {
            argv[(*argc)] = NULL;
            return 0;
        }
        *lbuf++ = '\0';
    }
}



static inline void show_descr(struct command_table_t *p) { printf("\x1b[1m%s\n\x1b[0m%s", p->cmd, p->descr); }
int help(int *argc, char *argv[])
{
    struct command_table_t* p;
    if(*argc == 1) {
        for(p = cmd_tbl; p->cmd; p++) {
            show_descr(p);
        }
    } else if(*argc == 2) { 
        for(p = cmd_tbl; p->cmd; p++) {
            if(strcmp(argv[1], p->cmd) == 0) {
                show_descr(p);
                return 0;
            }
        }
        if(p->cmd == NULL) {
            fprintf(stderr, "%s: Unknown command.\r\n", argv[1]);
        }
    } else {
        fprintf(stderr, "Too many args for help.\r\n");
    }
    return -1;
}

int quit(int *argc, char *argv[]) {
    ftp_message_t msg;
    msg.type = TYPE_QUIT;
    msg.code = CMD_OK;
    msg.length = 0;
    if (send(sfd, &msg, HEADER_SIZE, 0) < 0) {
        perror("send");
    }
    close(sfd);
    exit(0);
}

int pwd(int *argc, char *argv[])
{
    ftp_message_t *msg;
    ftp_message_t *reply;
    msg->type = TYPE_PWD;
    msg->code = CMD_OK;
    msg->length = 0;
    if (send(sfd, msg, HEADER_SIZE, 0) < 0) {
        perror("send");
        return -1;
    }
    if (recv_msg(reply, HEADER_SIZE) < 0) return -1;
    reply->length = ntohs(reply->length);
    debug_msg(reply);
    if (reply->type==TYPE_OK && reply->code==CMD_OK) {
        uint8_t *data = (uint8_t *)malloc(sizeof(uint8_t) * (reply->length+1));
        recv_msg(data, reply->length);
        data[reply->length]='\0';
        fprintf(stderr, "%s\n", data);
        free(data);
    } else {
    }
    return 0;
}

int cd(int *argc, char *argv[])
{
    ftp_message_t *msg;
    ftp_message_t *reply;

    if (*argc != 2) {
        fprintf(stderr, "cd requires exact 1 argument\n");
        return -1;
    }
    size_t pathlen = strlen(argv[1]);

    msg = (ftp_message_t *)malloc(sizeof(uint8_t) * (HEADER_SIZE+pathlen));
    msg->type = TYPE_CWD;
    msg->code = CMD_OK;
    msg->length = htons(pathlen);
    memcpy(msg->data, argv[1], pathlen);
    if (send(sfd, msg, HEADER_SIZE+pathlen, 0) < 0) {
        perror("send");
        return -1;
    }
    if (recv_msg(reply, HEADER_SIZE) < 0) return -1;
    reply->length = ntohs(reply->length);
    debug_msg(reply);
    if (reply->type==TYPE_OK && reply->code==CMD_OK) {
        fprintf(stderr, "## cd succeeded ##\n");
    } else {
    }
    free(msg);
    return 0;
}

int dir(int *argc, char *argv[])
{}

int lpwd(int *argc, char *argv[])
{
    char fullpath[PATH_MAX];
    if (getcwd(fullpath, sizeof(fullpath)) == NULL) {
        perror("getcwd");
        return -1;
    }
    fprintf(stderr, "%s\n", fullpath);
    return 0;
}

int lcd(int *argc, char *argv[])
{
    if (*argc != 2) {
        fprintf(stderr, "lcd requires exact 1 argument\n");
        return -1;
    }

    char *fullpath;
    if ((fullpath = normalize_path(argv[1])) != NULL) {
        if (chdir(fullpath) < 0) {
            perror("chdir");
        }
    }
    free(fullpath);
    return 0;
}

void show_file(struct stat *st, char *name)
{
    char timestr[BUF_LEN];
    struct tm *tm_ptr;
    tm_ptr = localtime(&st->st_ctime);
    strftime(timestr, BUF_LEN, "%Y %b %d %H:%M", tm_ptr);
    fprintf(stderr, "%12zd %s %s\n", st->st_size, timestr, name);
};

int ldir(int *argc, char *argv[])
{
    DIR *dir;
    struct dirent *entry;
    struct stat st;

    if (*argc > 2) {
        fprintf(stderr, "Too many args given\n");
        return -1;
    }
    if (*argc == 1) {
        argv[1] = ".";
        argv[2] = NULL;
    }

    char *fullpath;
    char name_full[PATH_MAX];
    if ((fullpath = normalize_path(argv[1])) == NULL) {
        return -1;        
    }
    if (stat(fullpath, &st) < 0) {
        perror("stat");
        free(fullpath);
        return -1;
    }
    if (S_ISDIR(st.st_mode)) {
        if ((dir = opendir(fullpath)) == NULL) {
            perror("opendir");
            free(fullpath);
            return -1;
        }
        while ((entry = readdir(dir)) != NULL) {
            memset(name_full, 0, PATH_MAX);
            strcat(name_full, fullpath);
            strcat(name_full, "/");
            strcat(name_full, entry->d_name);
            if (stat(name_full, &st) < 0) {
                perror("stat");
                free(fullpath);
                return -1;
            }
            show_file(&st, entry->d_name);
        }
        closedir(dir);
    } else {
        show_file(&st, fullpath);
    }
    free(fullpath);
    return 0;
}

int ftpget(int *argc, char *argv[])
{}
int ftpput(int *argc, char *argv[])
{}

int sock_conf(char *addrstr)
{
    int ret;
    char portstr[10];
    struct addrinfo hints, *addr;
    sprintf(portstr, "%d", MYFTP_PORT);
    memset(&hints, 0, sizeof hints);
    hints.ai_socktype = SOCK_STREAM;
    if ((ret=getaddrinfo(addrstr, portstr, &hints, &addr)) < 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ret));
        return -1;
    }
    if ((sfd=socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol)) < 0) {
        perror("socket");
        return -1;
    }
    int yes = 1;
    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (const char *)&yes, sizeof(yes)) < 0) {
        perror("setsockopt");
        return -1;
    }
    if (connect(sfd, addr->ai_addr, addr->ai_addrlen) < 0) {
        perror("connect");
        return -1;
    }
    freeaddrinfo(addr);
}
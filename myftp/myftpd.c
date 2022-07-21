#include "myftpd.h"

static int stop, sigint_flag, sigterm_flag;
int first_data;

int main(int *argc, char *argv[])
{
    pid_t pid;
    int ret;
    stop = 0;
    sigint_flag = 0;
    sigterm_flag = 0;
    first_data = 1;
    if (signal_conf() < 0) exit(1);
    if (sock_conf() < 0) exit(1);
    
    while (!stop) {
        if (sigint_flag) {
            // kill all the children
            kill_children();
            stop = 1;
            continue;
        } else {
            ret = wait_client();
            if (ret < 0) continue;

            if ((pid=fork()) < 0) {
                perror("fork");
                exit(1);
            }
            if (pid==0) {
                signal_conf_child();
                client_handler();
                close(sfd);
                close(sfd_client);
                fprintf(stderr, "## child proc %d exit ##\n", getpid());
                exit(0);
            } else {
                close(sfd_client);
            }
        }
    }
    fprintf(stderr, "## close socket ##\n");
    close(sfd);
    fprintf(stderr, "## This is proc %d, Bye ##\n", getpid());
    exit(0);
}

void kill_children()
{
    struct child_proc_list *p, *fp;
    pid_t pid;
    kill(0, SIGTERM);
    return;
}

int recv_msg(void *msg, int len)
{
    int cnt=0, ret;
    while (cnt < len) {
        if ((ret=recv(sfd_client, msg, len-cnt, 0)) <= 0) {
            if (errno != 0) perror("recv");
            return -1;
        }
        cnt += ret;
    }
    return 0;
}

int client_handler()
{
    int ret, i;
    uint8_t *data;
    ftp_message_t msg;
    fprintf(stderr, "## proc %d will handle new client ##\n", getpid());
    while (!stop) {
        if (sigterm_flag == 1) break;
        memset(&msg, 0, sizeof(msg));
        if((ret=recv_msg(&msg, HEADER_SIZE)) < 0) {
            stop = 1;
            continue;
        }
        msg.length = ntohs(msg.length);
        debug_msg(&msg);
        for (i=0; valid_commands[i].type; i++) {
            if (msg.type==valid_commands[i].type && msg.code==valid_commands[i].code) {
                (*valid_commands[i].func)(&msg);
                break;
            }
        }
        if (!valid_commands[i].type) {
            fprintf(stderr, "## undefined command, ignoring ##\n");
        }
    }
}

int wait_client()
{
    client_len = sizeof(client);
    if ((sfd_client=accept(sfd, (struct sockaddr *)&client, &client_len)) < 0) {
        perror("accept");
        return -1;
    }
    fprintf(stderr, "## new client accepted ##\n");
    fprintf(stderr, "\tip:  %s\n", inet_ntoa(client.sin_addr));
    fprintf(stderr, "\tport:%d\n", ntohs(client.sin_port));
    return 0;
}

int sock_conf()
{
    struct sockaddr_in addr;
    if ((sfd=socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return -1;
    }
    int yes = 1;
    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (const char *)&yes, sizeof(yes)) < 0) {
        perror("setsockopt");
        return -1;
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons(MYFTP_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return -1;
    }
    if (listen(sfd, 5) < 0) {
        perror("listen");
        return -1;
    }
    return 0;
}

void sigchld_handler(int sig) {if (wait(NULL) < 0) perror("wait");}
void sigint_handler(int sig) {sigint_flag = 1;}
void sigterm_handler(int sig) {sigterm_flag = 1;}

int signal_conf()
{
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    if (sigaction(SIGTTOU, &sa, NULL) < 0) {
        perror("sigaction");
        return -1;
    }
    if (sigaction(SIGTERM, &sa, NULL) < 0) {
        perror("sigaction");
        return -1;
    }
    sa.sa_handler = sigint_handler;
    if (sigaction(SIGINT, &sa, NULL) < 0) {
        perror("sigaction");
        return -1;
    }
    sa.sa_handler = sigchld_handler;
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) < 0) {
        perror("sigaction");
        return -1;
    }
    return 0;
}

int signal_conf_child()
{
    struct sigaction sa;
    sa.sa_handler = sigterm_handler;
    if (sigaction(SIGTERM, &sa, NULL) < 0) {
        perror("sigaction");
        return -1;
    }
}

int pwd(ftp_message_t *msg)
{
    char path[PATH_MAX];

    fprintf(stderr, "## pwd command ##\n");

    if (msg->length != 0) {
        recv_msg(NULL, msg->length) ;
        return -1;
    }

    memset(path, 0, PATH_MAX);
    if (getcwd(path, sizeof(path)) == NULL) {
        perror("getcwd");
        return -1;
    }
    if (send_msg(&sfd_client, TYPE_OK, CMD_OK, strlen(path), path) < 0) return -1;
    return 0;
}

int cd(ftp_message_t *msg)
{
    char path[PATH_MAX];
    fprintf(stderr, "## cd command ##\n");
    
    if (msg->length == 0) {
        if (send_msg(&sfd_client, TYPE_ERR_CMD, CMD_ERR_SYNTAX, 0, NULL) < 0) return -1;
        return 0;
    }
    if (recv_msg(path, msg->length) < 0) return -1;
    path[msg->length] = '\0';
    if (change_dir(path) < 0) {
        if (file_err(errno) < 0) return -1;
        return -1;
    }
    if (send_msg(&sfd_client, TYPE_OK, CMD_OK, 0, NULL) < 0) return -1;
    
    return 0;
}

int file_err(int err)
{
    int code, type;
    type = TYPE_ERR_FILE;
    switch (err) {
        case EACCES:
            code = CMD_ERR_PERMISSION;
            break;
        case ENOENT:
            code = CMD_ERR_NEGATION;
            break;
        default:
            type = TYPE_ERR_UNKWN;
            code = CMD_ERR_UNKWN;
            break;
    }
    if (send_msg(&sfd_client, type, code, 0, NULL) < 0) return -1;
    return 0;
}

int print_dir(struct stat *st, char *name)
{
    char timestr[BUF_LEN];
    char str_buf[BUF_LEN];
    struct tm *tm_ptr;
    tm_ptr = localtime(&st->st_ctime);
    strftime(timestr, BUF_LEN, "%Y %b %d %H:%M", tm_ptr);
    if (snprintf(str_buf, BUF_LEN, "%12zd %s %s", st->st_size, timestr, name) > BUF_LEN) {
        return -1;
    }
    //fprintf(stderr, "%s", str_buf);
    if (send_data(strlen(str_buf), str_buf) < 0) return -1;
    return 0;
}

int send_data(uint16_t length, uint8_t *data)
{
    if (first_data) {
        send_msg(&sfd_client, TYPE_OK, CMD_OK_RETR, 0, NULL);
        first_data = 0;
    }
    return send_msg(&sfd_client, TYPE_DATA, CMD_DATA, length, data);
}


int dir(ftp_message_t *msg)
{
    int ret;
    char search_path[PATH_MAX];
    if (msg->length == 0) {
        sprintf(search_path, ".");
    } else {
        if (recv_msg(search_path, msg->length) < 0) {
            return -1;
        }
        search_path[msg->length] = '\0';
    }
    ret = list_dir(search_path);
    send_msg(&sfd_client, TYPE_DATA, CMD_DATA_LAST, 0, NULL);
    return ret;
}

int ftpget(ftp_message_t *msg) {}
int ftpput(ftp_message_t *msg) {}
int quit(ftp_message_t *msg) {sigterm_flag = 1;}
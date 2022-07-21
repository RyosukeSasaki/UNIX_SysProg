#include "myftpd.h"

int main(int *argc, char *argv[])
{
    pid_t pid;
    int ret;
    stop = 0;
    sigint_flag = 0;
    sigterm_flag = 0;
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
    ftp_message_t *reply;
    char path[PATH_MAX];

    fprintf(stderr, "## pwd command ##\n");

    memset(path, 0, PATH_MAX);
    if (getcwd(path, sizeof(path)) == NULL) {
        perror("getcwd");
        return -1;
    }
    reply = (ftp_message_t *)malloc(sizeof(uint8_t)*(HEADER_SIZE + strlen(path)));
    reply->type = TYPE_OK;
    reply->code = CMD_OK;
    reply->length = htons(strlen(path));
    memcpy(reply->data, path, strlen(path));
    fprintf(stderr, "pwd %s\n", path);
    if (send(sfd_client, reply, HEADER_SIZE+strlen(path), 0) < 0) {
        perror("send");
        free(reply);
        return -1;
    }
    free(reply);
    return 0;
}

int cd(ftp_message_t *msg)
{
    ftp_message_t *reply;
    int ret=0, err=0;
    char *path;
    char *fullpath;

    fprintf(stderr, "## cd command ##\n");
    
    reply = (ftp_message_t *)malloc(sizeof(uint8_t)*HEADER_SIZE);
    if (msg->length == 0) {
        reply->type = TYPE_ERR_CMD;
        reply->code = CMD_ERR_SYNTAX;
        reply->length = 0;
        if (send(sfd, reply, HEADER_SIZE, 0) < 0) {
            perror("send");
            free(reply);
            ret = -1;
        }
        free(reply);
        return ret;
    }

    path = (uint8_t *)malloc(sizeof(uint8_t) * (msg->length+1));
    if (recv_msg(path, msg->length) < 0) {
        free(reply);
        free(path);
        return -1;
    }
    path[msg->length] = '\0';
    if ((fullpath = normalize_path(path)) != NULL) {
        if ((ret=chdir(fullpath)) < 0) {
            perror("chdir");
            err = errno;
        }
    }
    free(fullpath);
    free(path);

    if (ret == 0) {
        reply->type = TYPE_OK;
        reply->code = CMD_OK;
        reply->length = 0;
    } else {
        reply->type = TYPE_ERR_FILE;
        reply->length = 0;
        switch (err) {
            case EACCES:
                reply->code = CMD_ERR_PERMISSION;
                break;
            default:
                reply->code = CMD_ERR_NEGATION;
                break;
        }
    }
    ret = 0;
    if (send(sfd_client, reply, HEADER_SIZE, 0) < 0) {
        perror("send");
        ret = -1;
    }
    free(reply);
    return ret;
}

void file_str(struct stat *st, char *name)
{
    char timestr[BUF_LEN];
    struct tm *tm_ptr;
    tm_ptr = localtime(&st->st_ctime);
    strftime(timestr, BUF_LEN, "%Y %b %d %H:%M", tm_ptr);
    fprintf(stderr, "%12zd %s %s\n", st->st_size, timestr, name);
};

int dir(ftp_message_t *msg) {
    DIR *dir;
    struct dirent *entry;
    struct stat st;

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
            file_str(&st, entry->d_name);
        }
        closedir(dir);
    } else {
        file_str(&st, fullpath);
    }
    free(fullpath);
    return 0;
}

int ftpget(ftp_message_t *msg) {}
int ftpput(ftp_message_t *msg) {}
int quit(ftp_message_t *msg) {sigterm_flag = 1;}
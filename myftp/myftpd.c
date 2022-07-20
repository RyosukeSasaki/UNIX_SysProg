#include "myftpd.h"

int sfd, sfd_client;
int stop, sigchld_flag, sigint_flag;
struct child_proc_list child_head;
struct sockaddr_in client;
socklen_t client_len;

int main(int *argc, char *argv[])
{
    pid_t pid;
    stop = 0;
    init_list();
    if (sock_conf() < 0) exit(1);
    
    while (!stop) {
        wait_client();
        if (sigchld_flag) {
            if (wait(NULL) < 0) perror("wait");
            sigchld_flag = 0;
            continue;
        }
        if (sigint_flag) {
            // kill all the children
            stop = 1;
            continue;
        }

        if ((pid=fork()) < 0) {
            perror("fork");
            exit(1);
        }
        if (pid==0) {
            close(sfd);
            client_handler();
        } else {
            close(sfd_client);
            // add child list
            struct child_proc_list *p;
            p = (struct child_proc_list *)malloc(sizeof(struct child_proc_list));
            p->pid = pid;
            insert_child_list(p);
        }
    }
    close(sfd);
}

int client_handler()
{
}

int wait_client()
{
    client_len = sizeof(client);
    sfd_client=accept(sfd, (struct sockaddr *)&client, &client_len);
}

int sock_conf()
{
    struct sockaddr_in addr;
    if ((sfd=socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
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

void sigchld_handler() {sigchld_flag = 1;}
void sigint_handler() {sigint_flag = 1;}

void init_list()
{
    child_head.fp = &child_head;
    child_head.bp = &child_head;
}

void insert_child_list(struct child_proc_list *p)
{
    child_head.fp->bp = p;
    p->bp = &child_head;
    p->fp = child_head.fp;
    child_head.fp = p;
}

void remove_child_list(struct child_proc_list *p)
{
    p->bp->fp = p->fp;
    p->fp->bp = p->bp;
}
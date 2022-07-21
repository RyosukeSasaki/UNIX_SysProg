#include "myftpd.h"

int main(int *argc, char *argv[])
{
    pid_t pid;
    int ret;
    stop = 0;
    sigint_flag = 0;
    sigterm_flag = 0;
    init_list();
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
                // add child list
                struct child_proc_list *p;
                p = (struct child_proc_list *)malloc(sizeof(struct child_proc_list));
                p->pid = pid;
                insert_child_list(p);
            }
        }
    }
    fprintf(stderr, "## This is proc %d, Bye ##\n", getpid());
}

void kill_children()
{
    struct child_proc_list *p, *fp;
    pid_t pid;
    if ((p=child_head.fp) == & child_head) return;
    kill(0, SIGTERM);
    return;
    while (1) {
        pid = p->pid;
        fp = p->fp;
        remove_child_list(p);
        free(p);
        p = fp;
        if (kill(pid, 0) == 0) {
            fprintf(stderr, "## kill child proc %d ##\n", pid);
            if (kill(pid, SIGTERM) < 0) perror("kill");
        }
        if (child_head.fp == &child_head) break;
    }
}

int client_handler()
{
    int ret;
    uint8_t buf[1024];
    fprintf(stderr, "## proc %d will handle new client ##\n", getpid());
    while (1) {
        if (sigterm_flag == 1) break;
        if ((ret=recv(sfd_client, buf, 1024, 0)) <= 0) {
            if (errno != 0) perror("recv");
            break;
        } else if (ret>0) {
            fprintf(stderr, "%s\n", buf);
        }
    }
}

int wait_client()
{
    client_len = sizeof(client);
    if ((sfd_client=accept(sfd, (struct sockaddr *)&client, &client_len)) < 0) {
        if (errno == EINTR) return -2;
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

void sigchld_handler() {if (wait(NULL) < 0) perror("wait");}
void sigint_handler() {sigint_flag = 1;}
void sigterm_handler() {sigterm_flag = 1;}

int signal_conf()
{
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
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
    if (sigaction(SIGCHLD, &sa, NULL) < 0) {
        perror("sigaction");
        return -1;
    }
    return 0;
}

int signal_conf_child()
{
    struct sigaction sa;
    /**sa.sa_handler = SIG_IGN;
    if (sigaction(SIGINT, &sa, NULL) < 0) {
        perror("sigaction");
        return -1;
    }**/
    sa.sa_handler = sigterm_handler;
    if (sigaction(SIGTERM, &sa, NULL) < 0) {
        perror("sigaction");
        return -1;
    }
}

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
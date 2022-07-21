#ifndef MYFTPD_H
#define MYFTPD_H

#define MAX_CLIENT 32

#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include "common.h"

struct child_proc_list {
    pid_t pid;
    struct child_proc_list *fp;
    struct child_proc_list *bp;
};

int sock_conf();
int signal_conf();
int signal_conf_child();
int wait_client();
int client_handler();
void sigchld_handler();
void sigint_handler();
void sigterm_handler();
void kill_children();
void init_list();
void insert_child_list(struct child_proc_list *);
void remove_child_list(struct child_proc_list *);
int pwd(int*, char *[]);
int cd(int*, char *[]);
int dir(int*, char *[]);
int ftpget(int*, char *[]);
int ftpput(int*, char *[]);


int sfd, sfd_client;
int stop, sigint_flag, sigterm_flag;
struct child_proc_list child_head;
struct sockaddr_in client;
socklen_t client_len;

#endif
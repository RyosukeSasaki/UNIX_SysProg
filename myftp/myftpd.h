#ifndef MYFTPD_H
#define MYFTPD_H

#define MAX_CLIENT 32

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <unistd.h>
#include "common.h"

struct child_proc_list {
    pid_t pid;
    struct child_proc_list *fp;
    struct child_proc_list *bp;
};

int sock_conf();
int signal_conf();
int wait_client();
int client_handler();
void sigchld_handler();
void sigint_handler();
void init_list();
void insert_child_list(struct child_proc_list *);
void remove_child_list(struct child_proc_list *);
int pwd(int*, char *[]);
int cd(int*, char *[]);
int dir(int*, char *[]);
int ftpget(int*, char *[]);
int ftpput(int*, char *[]);

#endif
#ifndef MYFTPD_H
#define MYFTPD_H

#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <time.h>
#include "common.h"

int sock_conf();
int signal_conf();
int signal_conf_child();
int wait_client();
int client_handler();
void sigchld_handler(int);
void sigint_handler(int);
void sigterm_handler(int);
void kill_children();
int pwd(ftp_message_t *);
int cd(ftp_message_t *);
int dir(ftp_message_t *);
int ftpget(ftp_message_t *);
int ftpput(ftp_message_t *);
int quit(ftp_message_t *);

typedef struct _vmsg {
    int type;
    int code;
    int (* func)(ftp_message_t *);
} valid_message_t;

valid_message_t valid_commands[] = {
    {TYPE_QUIT, CMD_OK, quit},
    {TYPE_PWD, CMD_OK, pwd},
    {TYPE_CWD, CMD_OK, cd},
    {TYPE_LIST, CMD_OK, dir},
    {TYPE_RETR, CMD_OK, ftpget},
    {TYPE_STOR, CMD_OK, ftpput},
    {0, 0}
};

int sfd, sfd_client;
struct sockaddr_in client;
socklen_t client_len;

#endif
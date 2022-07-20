#ifndef MYFTPC_H
#define MYFTPC_H

#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>
#include "common.h"

#define NARGS 64
#define BUF_LEN 256

int quit(int*, char *[]);
int pwd(int*, char *[]);
int cd(int*, char *[]);
int dir(int*, char *[]);
int lpwd(int*, char *[]);
int lcd(int*, char *[]);
int ldir(int*, char *[]);
int ftpget(int*, char *[]);
int ftpput(int*, char *[]);
int help(int*, char *[]);
int getargs(int*, char *[], char*);
int exec_command();
void show_file(struct stat *, char *);

struct command_table_t {
    char *cmd;
    int (*func)(int*, char* []);
    char* descr;
} cmd_tbl[] = {
    {"quit", quit, "\tQuit the program\n"},
    {"exit", quit, "\tQuit the program\n"},
    {"pwd", pwd, "\tShow current directory of the remote\n"},
    {"cd", cd, "\tusage: cd DIRECTORY\n\tChange the remote directory\n"},
    {"dir", dir, "\tusage: dir [PATH]\n\tShow infomation of remote directory\n"},
    {"lpwd", lpwd, "\tShow current directory of the local\n"},
    {"lcd", lcd, "\tusage: lcd DIRECTORY\n\tChange the local directory\n"},
    {"ldir", ldir, "\tusage: ldir [PATH]\n\tShow infomation of local directory\n"},
    {"get", ftpget, "\tusage: get SRC [DST]\n\tGet remote file (SRC) to local (DST)\n"},
    {"put", ftpput, "\tusage: put SRC [DST]\n\tSend local file (SRC) to remote (DST)\n"},
    {"help", help, "\tShow command description\n"},
    {NULL, NULL, NULL}
};

#endif
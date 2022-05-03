#ifndef COMMAND_H
#define COMMAND_H

#define BUF_LEN 256
#define NARGS 16
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "getblk.h"


struct command_table {
    char* cmd;
    void (*func)(int, char* []);
} cmd_tbl[] = {
    {"getblk", getblk},
    {NULL, NULL}
};

void getargs(int *argc, char *argv[], char *lbuf)
{
    *argc = 0;
    argv[0] = NULL;

    while(1) {
        while(isblank(*lbuf)) lbuf++;
        if(*lbuf == '\0') return;
        argv[(*argc)++] = lbuf;

        while(*lbuf && !isblank(*lbuf)) lbuf++;
        if(*lbuf == '\0') return;
        *lbuf++ = '\0';
    }
}

int parse_command()
{
    char lbuf[BUF_LEN], *argv[NARGS];
    int argc, i;
    struct command_table* p;

    fprintf(stderr, "$ ");
    if(fgets(lbuf, sizeof lbuf, stdin) == NULL) {
        fprintf(stderr, "\r\n");
        return 0;
    }
    lbuf[strlen(lbuf) - 1] = '\0';
    getargs(&argc, argv, lbuf);

    for(p = cmd_tbl; p->cmd; p++) {
        if(strcmp(argv[0], p->cmd) == 0) {
            (p->func)(argc, argv);
            break;
        }
    }
    if(p->cmd == NULL) {
        fprintf(stderr, "%s: unknown command", argv[0]);
        return -1;
    }
    return 0;
}

#endif
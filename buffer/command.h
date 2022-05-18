#ifndef COMMAND_H
#define COMMAND_H

#define BUF_LEN 256
#define NARGS 16
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include "blockfunc.h"

void init(int*, char *[]);
void help(int*, char *[]);
void buf(int*, char *[]);
void hash(int*, char *[]);
void free_func(int*, char *[]);
void getblk(int*, char *[]);
void brelse(int*, char *[]);
void set(int*, char *[]);
void reset(int*, char *[]);
void quit(int*, char *[]);
void getargs(int*, char *[], char*);
int parse_command();

struct command_table {
    char* cmd;
    void (*func)(int*, char* []);
    char* descr;
};

#endif
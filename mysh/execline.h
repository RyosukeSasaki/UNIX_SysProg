#pragma once
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include "mysh_types.h"

struct builtin_table {
    char *cmd;
    void (*func)(int*, char *[]);
};

void exit_mysh(int *, char *[]);
int execute(struct line *);
int exec_extra(struct line *, int);
void child_proc(char *[]);
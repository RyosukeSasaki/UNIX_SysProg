#pragma once
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include "mysh_types.h"

struct builtin_table {
    char *cmd;
    void (*func)(int*, char *[]);
};

void exit_mysh(int *, char *[]);
void cd(int *, char *[]);
void pwd(int *, char *[]);
void pid(int *, char *[]);
int execute(struct line *);
int exec_extra(struct line *);
int exec_recursive(struct line *, int);
void child_proc(int *, char *[]);
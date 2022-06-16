#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include "mysh_types.h"
#include "execline.h"

int read_command(struct line *);
int parseline(struct line *);
int gettoken(char*, int*, int);
void getargs(int*, char *[], char*);
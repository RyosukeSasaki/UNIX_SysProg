#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include "mysh_types.h"
#include "execline.h"

void read_command();
int gettoken(char*, int*, int);
void getargs(int*, char *[], char*);
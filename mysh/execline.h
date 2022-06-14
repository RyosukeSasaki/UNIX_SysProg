#pragma once
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "mysh_types.h"

int execute(struct line *);
int child_proc(char *[]);
#include <stdio.h>
#include "execline.h"

int execute(struct line *line)
{
    int status;
    pid_t pid, wpid;
    if ((pid = fork()) < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        // child process
        child_proc(NULL);
    } else {
        // parent process
        wait(&status);
    }
}

extern char **environ;

int child_proc(char *argv[])
{
    char *path = getenv("PATH");
    char *p = path;
    //printf("%s\r\n", path);
    //printf("%s\r\n", p);
    if (execve(argv[0], argv, environ) < 0) {
        perror("execve");
        return 1;
    }
    return 0;
}
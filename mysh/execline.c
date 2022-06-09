#include "execline.h"

int execute() {
    pid_t pid;
    pid = fork();

    if (pid == 0) {
        // child process
    } else if (pid < 0) {
        fprintf(stderr, "")
    }
}
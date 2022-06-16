#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include "parseline.h"
#include "mysh_types.h"
#include "execline.h"

void main()
{
    struct sigaction sa_sigint;
    struct line command_line;
    sa_sigint.sa_handler = SIG_IGN;
    if (sigaction(SIGINT, &sa_sigint, NULL) < 0) {
        perror("sigaction");
        exit(0);
    }
    
    /*
    pid_t sid, pid;
    if ((sid=setsid()) < 0) {
        perror("setsid");
        exit(0);
    }
    sid=getsid(pid=getpid());
    fprintf(stderr, "sid %d pid %d\r\n", sid, pid);
    fprintf(stderr, "Welcome to mysh.\r\n");
    */
    while (1) {
        fprintf(stderr, "mysh $ ");
        if (read_command(&command_line) < 0) {
            fprintf(stderr, "syntax error\r\n");
        } else {
            if (execute(&command_line) < 0) fprintf(stderr, "execution error\r\n");
        }
    }
    exit(0);
}

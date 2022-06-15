#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include "parseline.h"

void main()
{
    struct sigaction sa_sigint;
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
    read_command();
    exit(0);
}

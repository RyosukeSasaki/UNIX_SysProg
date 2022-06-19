#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include "parseline.h"
#include "mysh_types.h"
#include "execline.h"

void signal_conf()
{
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    if (sigaction(SIGINT, &sa, NULL) < 0) {
        perror("sigaction");
        exit(0);
    }
    if (sigaction(SIGTTOU, &sa, NULL) < 0) {
        perror("sigaction");
        exit(0);
    }
    //sa.sa_handler = sigchld_handler;
    //if (sigaction(SIGCHLD, &sa, NULL) < 0) {
    //    perror("sigaction");
    //    //return MYSH_EXEC_ERR;
    //    exit(0);
    //}
}

void main_loop()
{
    struct line command_line;
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

void main()
{
    signal_conf();
    fprintf(stderr, "This is mysh. pid=%d\r\n", getpid());
    main_loop();
}

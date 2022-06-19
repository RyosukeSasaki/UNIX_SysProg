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
    fprintf(stderr, "Welcome to mysh.\r\n");
    main_loop();
}

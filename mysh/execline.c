#include "execline.h"

struct builtin_table cmd_tbl[] = {
    {"exit", exit_mysh},
    {NULL, NULL}
};

void exit_mysh(int *argc, char *argv[])
{
    fprintf(stderr, "Bye\r\n");
    exit(0);
}

int execute(struct line *line)
{
    struct builtin_table *p;
    if (line->blocks[0].argc == 0) {
        fprintf(stderr, "first token is too short\r\n");
        return -1;
    }
    for(p=cmd_tbl; p->cmd; p++) {
        if(strcmp(line->blocks[0].argv[0], p->cmd) == 0) {
            (p->func)(&line->blocks[0].argc, line->blocks[0].argv);
            break;
        }
    }

    line->blocks[0].redir = 0;
    for (int i=1; i<line->nblock-1; i++) {
        switch (line->blocks[i].type) {
            case TKN_REDIR_IN:
            case TKN_REDIR_OUT:
            case TKN_REDIR_APPEND:
                if ((line->blocks[i-1].type == TKN_NORMAL || line->blocks[i-1].type == TKN_DIR) && 
                line->blocks[i+1].argc == 1) {
                    line->blocks[i-1].redir = line->blocks[i].type;
                    line->blocks[i-1].dir = line->blocks[i+1].argv[0];
                    line->blocks[i+1].type = TKN_DIR;
                } else {
                    fprintf(stderr, "Invalid use of redirection\r\n");
                }
                break;
            default:
                line->blocks[i].redir = 0;
                break;
        }
    }

    // print
    debug(stderr, "nblock: %d\r\n", line->nblock);
    for (int j=0; j<line->nblock; j++) {
        debug(stderr, "token %d type %d include %d args: \r\n", j, line->blocks[j].type,
        line->blocks[j].argc);
        for (int k=0; k<line->blocks[j].argc; k++) {
            debug(stderr, "\targ %d: %s\r\n", k, line->blocks[j].argv[k]);
        }
    }

    if (p->cmd == NULL) {
        return exec_extra(line, line->nblock);
    }
    return 0;
}

int exec_extra(struct line *line, int pos)
{
    int status;
    pid_t pid, wpid;
    struct token_block *b;
    
    if (pos < 0) return 0;
    //b=&line->blocks[pos];
    b=&line->blocks[0];
    switch (b->type) {
        case TKN_ERR:
            return -1;
        case TKN_NORMAL:
            break;
        case TKN_PIPE:
            break;
        case TKN_BG:
            break;
        case TKN_EOL:
        case TKN_EOF:
            return exec_extra(line, pos-1);
        default:
            return -1;
        
    }

    if ((pid = fork()) < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        // child process
        struct sigaction sa_sigint;
        sa_sigint.sa_handler = SIG_DFL;
        if (sigaction(SIGINT, &sa_sigint, NULL) < 0) {
            perror("sigaction");
            exit(0);
        }
        child_proc(b->argv);
    } else {
        // parent process
        wait(&status);
        fprintf(stderr, "child process return with %d\r\n", status);
    }
}

extern char **environ;

void child_proc(char *argv[])
{
    char *path = getenv("PATH");
    char *p = path;
    int stat;
    //printf("%s\r\n", path);
    //printf("%s\r\n", p);
    /*
    if ((stat=execve(argv[0], argv, environ)) < 0) {
        perror("execve");
        return 1;
    }
    */
    /*
    if ((stat=execvp(argv[0], argv)) < 0) {
        perror("execvp");
        exit(1);
    }
    */
    exit(0);
}
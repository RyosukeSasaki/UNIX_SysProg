#include "execline.h"

struct builtin_table cmd_tbl[] = {
    {"exit", exit_mysh},
    {"cd", cd},
    {"pwd", pwd},
    {"pid", pid},
    {NULL, NULL}
};

void exit_mysh(int *argc, char *argv[])
{
    fprintf(stderr, "Bye\r\n");
    exit(MYSH_OK);
}

void pid(int *argc, char *argv[])
{
    fprintf(stderr, "My pid: %d\r\n", getpid());
    return;
}

void cd(int *argc, char *argv[])
{
    char relpath[PATH_LEN];
    char fullpath[PATH_LEN];
    char *home;
    if (*argc != 2) {
        fprintf(stderr, "cd requires exact 1 argument\r\n");
        return;
    }
    
    if (argv[1][0] == '~') {
        home = getenv("HOME");
        if (strlen(home)+strlen(argv[1]) > PATH_LEN -1) {
            fprintf(stderr, "path string is too long\r\n");
            return;
        }
        memset(relpath, 0, sizeof(relpath));
        strcat(relpath, home);
        strcat(relpath, &argv[1][1]);
        if (realpath(relpath, fullpath)==NULL) {
            perror("realpath");
            return;
        }
    } else {
        if (realpath(argv[1], fullpath)==NULL) {
            perror("realpath");
            return;
        }
    }

    if (chdir(fullpath) < 0) {
        perror("chdir");
        return;
    }
    return;
}

void pwd(int *argc, char *argv[])
{
    char fullpath[PATH_LEN];
    if (getcwd(fullpath, sizeof(fullpath)) < 0) {
        perror("getcwd");
        return;
    }
    fprintf(stderr, "%s\r\n", fullpath);
    return;
}

int execute(struct line *line)
{
    // debug
    debug(stderr, "nblock: %d\r\n", line->nblock);
    for (int j=0; j<line->nblock; j++) {
        debug(stderr, "token %d type %d include %d args: \r\n", j, line->blocks[j].type,
        line->blocks[j].argc);
        for (int k=0; k<line->blocks[j].argc; k++) {
            debug(stderr, "\targ %d: %s\r\n", k, line->blocks[j].argv[k]);
        }
    }
    if (line->bg) debug(stderr, "this line execute as bg\r\n");

    struct builtin_table *p;
    for(p=cmd_tbl; p->cmd; p++) {
        if(strcmp(line->blocks[0].argv[0], p->cmd) == 0) {
            (p->func)(&line->blocks[0].argc, line->blocks[0].argv);
            break;
        }
    }

    if (p->cmd == NULL) return exec_extra(line);
    return MYSH_OK;
}

int exec_extra(struct line *line)
{
    int status, ret, fd;
    pid_t pid;
    struct sigaction sa_sigint;    
    
    if ((pid = fork()) < 0) {
        perror("fork");
        return MYSH_EXEC_ERR;
    }
    if (pid == 0) {
        // child process
        sa_sigint.sa_handler = SIG_DFL;
        if ((ret = sigaction(SIGINT, &sa_sigint, NULL)) < 0) {
            perror("sigaction");
            exit(ret);
        }
        sa_sigint.sa_handler = SIG_IGN;
        if ((ret = sigaction(SIGTTOU, &sa_sigint, NULL)) < 0) {
            perror("sigaction");
            exit(ret);
        }
        if (setpgid(0, 0) < 0) {
            perror("setpgid");
            exit(MYSH_EXEC_ERR);
        }
        //if ((fd=open("/dev/tty", O_RDWR)) < 0) {
        //if ((fd=open(ttyname(STDOUT_FILENO), O_RDWR)) < 0) {
        //    perror("open");
        //    exit(MYSH_EXEC_ERR);
        //}
        //close(fd);
        if (tcsetpgrp(STDOUT_FILENO, getpid()) < 0) {
            perror("tcsetpgrp");
            exit(MYSH_EXEC_ERR);
        }
        if (tcgetpgrp(STDOUT_FILENO) < 0) {
            perror("tcgetpgrp");
            exit(MYSH_EXEC_ERR);
        }
        exit(exec_recursive(line, line->nblock-1));
    } else {
        // parent process
        waitpid(pid, &status, WUNTRACED);
        if (tcsetpgrp(STDOUT_FILENO, getpid()) < 0) {
            perror("tcsetpgrp");
            exit(MYSH_EXEC_ERR);
        }
        if (tcgetpgrp(STDOUT_FILENO) < 0) {
            perror("tcgetpgrp");
            exit(MYSH_EXEC_ERR);
        }
        debug(stderr, "child process %d return with %d, returned to main\r\n", pid, status);
        return status;
    }
    return MYSH_OK;
}

int exec_recursive(struct line *line, int pos)
{
    int status, pfd[2], fd=0, ret;
    pid_t pid[TOKEN_MAX];
    struct token_block *b;

    if (pos < 0) return MYSH_OK;
    if (pipe(pfd) < 0) {
        perror("pipe");
        exit(MYSH_EXEC_ERR);
    }

    b=&line->blocks[pos];
    while (b->type!=TKN_NORMAL && pos>=0) {
        switch (b->type) {
            case TKN_REDIR_IN:
                if ((fd=open(b->dir, O_RDONLY)) < 0) {
                    perror("open");
                    return MYSH_FILE_ERR;
                }
                dup2(fd, 0);
                break;
            case TKN_REDIR_OUT:
                if ((fd=open(b->dir, O_WRONLY|O_CREAT|O_TRUNC, 0644)) < 0) {
                    perror("open");
                    return MYSH_FILE_ERR;
                }
                dup2(fd, 1);
                break;
            case TKN_REDIR_APPEND:
                if ((fd=open(b->dir, O_WRONLY|O_CREAT|O_APPEND, 0644)) < 0) {
                    perror("open");
                    return MYSH_FILE_ERR;
                }
                dup2(fd, 1);
                break;
        }
        if (fd) close(fd);
        b=&line->blocks[--pos];
    }
    b=&line->blocks[pos];

    if ((pid[pos] = fork()) < 0) {
        perror("fork");
        if (close(pfd[1]) < 0) {
            perror("close");
            exit(MYSH_EXEC_ERR);
        }
        if (close(pfd[0]) < 0) {
            perror("close");
            exit(MYSH_EXEC_ERR);
        }
        return MYSH_EXEC_ERR;
    }

    if (pid[pos]==0) {
        // child process
        debug(stderr, "im child %d will exec %s\r\n", getpid(), b->argv[0]);
        if (pos == 0) {
            child_proc(&b->argc, b->argv);
        }
        if (dup2(pfd[0], 0) < 0) {
            perror("dup2");
            exit(MYSH_EXEC_ERR);
        }
        if (close(pfd[0]) < 0) {
            perror("close");
            exit(MYSH_EXEC_ERR);
        }
        if (close(pfd[1]) < 0) {
            perror("close");
            exit(MYSH_EXEC_ERR);
        }
        child_proc(&b->argc, b->argv);
    } else {
        // parent proc
        if (dup2(pfd[1], 1) < 0) {
            perror("dup2");
            exit(MYSH_EXEC_ERR);
        }
        if (close(pfd[1]) < 0) {
            perror("close");
            exit(MYSH_EXEC_ERR);
        }
        if (close(pfd[0]) < 0) {
            perror("close");
            exit(MYSH_EXEC_ERR);
        }
        debug(stderr, "im pos %d, my child proc %d will execute %s\r\n", pos, pid[pos], b->argv[0]);
        ret = exec_recursive(line, pos-1);
        waitpid(pid[pos], NULL, 0);
        debug(stderr, "%d is died\r\n", pid[pos]);
        return ret;
    }

    return MYSH_OK;
}

extern char **environ;

void child_proc(int *argc, char *argv[])
{
    argv[*argc] = NULL;
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
    if ((stat=execvp(argv[0], argv)) < 0) {
        perror("execvp");
        exit(MYSH_EXEC_ERR);
    }
    exit(MYSH_OK);
}
#include "execline.h"

#ifdef PATH_MAX
  int path_max = PATH_MAX;
#else
  int path_max = pathconf(path, _PC_PATH_MAX);
  if (path_max <= 0) path_max = 4096;
#endif
volatile sig_atomic_t fpid;

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
    char relpath[path_max];
    char *fullpath;
    if (*argc != 2) {
        fprintf(stderr, "cd requires exact 1 argument\r\n");
        return;
    }

    if ((fullpath = normalize_path(argv[1])) != NULL) {
        if (chdir(fullpath) < 0) {
            perror("chdir");
        }
    }
    free(fullpath);
    return;
}

void pwd(int *argc, char *argv[])
{
    char fullpath[path_max];
    if (getcwd(fullpath, sizeof(fullpath)) < 0) {
        perror("getcwd");
        return;
    }
    fprintf(stderr, "%s\r\n", fullpath);
    return;
}

void sigchld_handler(int signum) {
    int status;
    pid_t pid;
    struct sigaction sa;

    getpid();
    if ((pid=waitpid(fpid, NULL, WUNTRACED | WNOHANG)) <= 0) {
    } else {
        sa.sa_handler = SIG_DFL;
        if (sigaction(SIGCHLD, &sa, NULL) < 0) {
            perror("sigaction");
        }
    }
    return;
}

int execute(struct line *line)
{
    // debug
    parse_debug(stderr, "nblock: %d\r\n", line->nblock);
    for (int j=0; j<line->nblock; j++) {
        parse_debug(stderr, "token %d type %d include %d args: \r\n", j, line->blocks[j].type,
        line->blocks[j].argc);
        for (int k=0; k<line->blocks[j].argc; k++) {
            parse_debug(stderr, "\targ %d: %s\r\n", k, line->blocks[j].argv[k]);
        }
    }
    if (line->bg) parse_debug(stderr, "this line execute as bg\r\n");

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
    pid_t pid, wpid;
    struct sigaction sa;
    
    //if (line->bg) {
    sa.sa_handler = sigchld_handler;
    sa.sa_flags = SA_NODEFER;
    if (sigaction(SIGCHLD, &sa, NULL) < 0) {
        perror("sigaction");
        return MYSH_EXEC_ERR;
    }
    //}
    
    if ((pid = fork()) < 0) {
        perror("fork");
        return MYSH_EXEC_ERR;
    }
    if (pid == 0) {
        // child process
        debug(stderr, "im the first child %d\r\n", getpid());
        /**
        if ((ret = sigprocmask(SIG_UNBLOCK)) < 0) {
            perror("sigprocmask");
            exit(ret);
        }
        if ((ret=sigemptyset(&sa.sa_mask)) < 0) {
            perror("sigemptyset");
            exit(ret);
        }
        if ((ret=sigaddset(&sa.sa_mask, SIGCHLD)) < 0) {
            perror("sigaddset");
            exit(ret);
        }
        **/
       sa.sa_flags = 0;
        sa.sa_handler = SIG_DFL;
        if ((ret = sigaction(SIGINT, &sa, NULL)) < 0) {
            perror("sigaction");
            exit(ret);
        }
        if (sigaction(SIGCHLD, &sa, NULL) < 0) {
            perror("sigaction");
            return MYSH_EXEC_ERR;
        }
        sa.sa_handler = SIG_IGN;
        if ((ret = sigaction(SIGTTOU, &sa, NULL)) < 0) {
            perror("sigaction");
            exit(ret);
        }
        if (setpgid(0, 0) < 0) { 
            perror("setpgid");
            exit(MYSH_EXEC_ERR);
        }
        if (!line->bg) {
            if (tcsetpgrp(STDOUT_FILENO, getpid()) < 0) {
                perror("tcsetpgrp");
                exit(MYSH_EXEC_ERR);
            }
        }
        if (tcgetpgrp(STDOUT_FILENO) < 0) {
            perror("tcgetpgrp");
            exit(MYSH_EXEC_ERR);
        }
        ret = exec_recursive(line, line->nblock-1);
        getpid();
        debug(stderr, "im %d going to die with status %d\r\n", getpid(), ret);
        exit(ret);
    } else {
        // parent process
        if (!line->bg) {
            debug(stderr, "im parent %d waiting %d\r\n", getpid(), pid);
            if ((wpid=waitpid(pid, &status, WUNTRACED)) < 0) {
                perror("waitpid");
            }
            if (tcsetpgrp(STDOUT_FILENO, getpid()) < 0) {
                perror("tcsetpgrp");
                exit(MYSH_EXEC_ERR);
            }
            if (tcgetpgrp(STDOUT_FILENO) < 0) {
                perror("tcgetpgrp");
                exit(MYSH_EXEC_ERR);
            }
            debug(stderr, "child process %d return with %d, returned to main\r\n", wpid, status);
            return status;
        } else {
            fpid = pid;
            debug(stderr, "fpid is set to %d\r\n", fpid);
        }
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
        debug(stderr, "im %d, my child proc %d will execute %s\r\n", getpid(), pid[pos], b->argv[0]);
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
    char *path;
    char fullpath[path_max], *p;
    argv[*argc] = NULL;

    if ((p=search_path(argv[0])) == NULL) {
        fprintf(stderr, "file not found\r\n");
        exit(MYSH_EXEC_ERR);
    }
    if (strlen(p) > path_max-1) {
        fprintf(stderr, "execute path is too long (>4096)\r\n");
        exit(MYSH_EXEC_ERR);
    }
    memcpy(fullpath, p, strlen(p));
    free(p);
    if (execve(fullpath, argv, environ) < 0) {
        perror("execve");
        exit(MYSH_EXEC_ERR);
    }
    exit(MYSH_OK);
}

// free after use
char *search_path(char *arg)
{
    char *path;
    char *p, *b, *buf=NULL;
    char *fullpath=NULL;
    struct stat st;
    
    if (strchr(arg, '/') != NULL) {
        // execute directly
        return normalize_path(arg);
    } else {
        // search in PATH
        if ((path=getenv("PATH")) == NULL) return NULL;
        if ((fullpath=malloc(sizeof(char)*path_max)) != NULL) {
            if ((p=buf=malloc(sizeof(char)*(strlen(path)+1))) != NULL) {
                memcpy(p, path, strlen(path));
                b = p;
                while (1) {
                    if ((b = strchr(p, ':')) != NULL) *b = '\0';
                    memset(fullpath, 0, sizeof(fullpath));
                    strcat(fullpath, p);
                    strcat(fullpath, "/");
                    strcat(fullpath, arg);
                    if (stat(fullpath, &st) == 0) {
                        break;
                    }
                    if (b==NULL) break;
                    p=++b;
                }
            } else {
                free(fullpath);
                return NULL;
            }
        }
        if (buf != NULL) free(buf);
        return b ? fullpath : NULL;
    }
}

// free after use
char *normalize_path(char *arg)
{
    char relpath[path_max];
    char *fullpath;
    char *home;
    
    if (arg[0] == '~') {
        if ((home=getenv("HOME")) == NULL) return NULL;
        if (strlen(home)+strlen(arg) > path_max -1) {
            fprintf(stderr, "path string is too long\r\n");
            return NULL;
        }
        memset(relpath, 0, sizeof(relpath));
        strcat(relpath, home);
        strcat(relpath, &arg[1]);
        if ((fullpath = realpath(relpath, NULL))==NULL) {
            perror("realpath");
            return NULL;
        }
    } else {
        if ((fullpath = realpath(arg, NULL))==NULL) {
            perror("realpath");
            return NULL;
        }
    }

    return fullpath;
}
#define BUF_LEN 256
#define NARGS 16
#include "command.h"

struct command_table  cmd_tbl[] = {
    {"help", help, "usage: help command(optional)\r\nShow command description. If command option is given, show description of command.\r\n"},
    {"init", init, "Initialize buffers.\r\n"},
    {"quit", quit, "Quit the Program.\r\n"},
    {"buf", buf, "usage: buf [n ...]\r\nShow all buffers' status. If n is given, show the status of buffer of number n.\r\n"},
    {"hash", hash, "usage: hash [n ...]\r\nShow all hash lists. If n is given, show the hash list of hash number n.\r\n"},
    {"free", free_func, "Show the free list.\r\n"},
    {"getblk", getblk, "usage: getblk n\r\nExcute getblk(n).\r\n"},
    {"brelse", brelse, "usage: brelse n\r\nExcute brelse(bp) where bp is pointer to buffer which blkno is n.\r\n"},
    {"set", set, "usage: set n stat [stat ...]\r\nstat: L(Locked), V(Valid), D(Delayed write), K(Kernel read/write), W(Waited), O(Old)\r\nSet stat on the buffer which blkno is n.\r\n"},
    {"reset", reset, "usage: reset n stat [stat ...]\r\nstat: L(Locked), V(Valid), D(Delayed write), K(Kernel read/write), W(Waited), O(Old)\r\nUnset stat on the buffer which blkno is n.\r\n"},
    {NULL, NULL}
};

struct stat_func_table stat_tbl[] = {
    {"L", set_locked},
    {"V", set_valid},
    {"D", set_dwr},
    {"K", set_krdwr},
    {"W", set_waited},
    {"O", set_old},
    {NULL, NULL}
};

void init(int *argc, char *argv[]) { init_buf(); }
void quit(int *argc, char *argv[]){ exit(0); }
void buf(int *argc, char *argv[])
{
    if(!initialized) {
        fprintf(stderr, "Buffers haven't initialized, please run init first.\r\n");
        return;
    }
    if(*argc == 1) {
        for(int i=0; i<BUF_SIZE; i++) { show_buffer(i); printf("\r\n"); }
    } else {
        long ln;
        int n;
        char *end_ptr;
        for(int i=1; i<*argc; i++) {
            ln = strtol(argv[i], &end_ptr, 10);
            n = (int)ln;
            if(end_ptr == argv[i] || ln > INT_MAX || ln < INT_MIN) {
                fprintf(stderr, "Conversion error of argument %s\r\n", argv[i]);
            } else if(n > BUF_SIZE || n < 0) {
                printf("Buffer %d does not exist\r\n", n);
            } else {
                show_buffer(n);
                printf("\r\n");
            }
        }
    }
}

void hash(int *argc, char *argv[])
{
    if(!initialized) {
        fprintf(stderr, "Buffers haven't initialized, please run init first.\r\n");
        return;
    }
    if(*argc == 1) {
        for(int i=0; i<HASH_SIZE; i++) {
            show_hash(i);
        }
    } else {
        long ln;
        int n;
        char *end_ptr;
        for(int i=1; i<*argc; i++) {
            ln = strtol(argv[i], &end_ptr, 10);
            n = (int)ln;
            if(end_ptr == argv[i] || ln > INT_MAX || ln < INT_MIN) {
                fprintf(stderr, "Conversion error of argument %s\r\n", argv[i]);
            } else if(n > HASH_SIZE || n < 0) {
                printf("Buffer %d does not exist\r\n", n);
            } else {
                show_hash(n);
            }
        }
    }
}

void free_func(int *argc, char *argv[])
{
    if(!initialized) {
        fprintf(stderr, "Buffers haven't initialized, please run init first.\r\n");
        return;
    }
    show_free();
}

void getblk(int *argc, char *argv[])
{
    buf_header *ret;
    if(!initialized) {
        fprintf(stderr, "Buffers haven't initialized, please run init first.\r\n");
        return;
    }
    if(*argc != 2) {
        printf("getblk needs exact 1 argument.\r\n");
    } else {
        long ln;
        int n;
        char *end_ptr;
        ln = strtol(argv[1], &end_ptr, 10);
        n = (int)ln;
        if(end_ptr == argv[1] || ln > INT_MAX || ln < INT_MIN) {
            fprintf(stderr, "Conversion error of argument %s\r\n", argv[1]);
        } else if(n < 0) {
            fprintf(stderr, "Blkno must be positive\r\n");
        } else {
            ret = _getblk(n);
        }
    }
}

void brelse(int *argc, char *argv[])
{
    if(!initialized) {
        fprintf(stderr, "Buffers haven't initialized, please run init first.\r\n");
        return;
    }
    if(*argc != 2) {
        printf("brelse needs exact 1 argument.\r\n");
    } else {
        long ln;
        int n;
        char *end_ptr;
        ln = strtol(argv[1], &end_ptr, 10);
        n = (int)ln;
        if(end_ptr == argv[1] || ln > INT_MAX || ln < INT_MIN) {
            fprintf(stderr, "Conversion error of argument %s\r\n", argv[1]);
        } else if(n < 0) {
            fprintf(stderr, "Blkno must be positive\r\n");
        } else {
            _brelse(n);
        }
    }   
}

void set(int *argc, char *argv[])
{
    struct stat_func_table *p;
    if(!initialized) {
        fprintf(stderr, "Buffers haven't initialized, please run init first.\r\n");
        return;
    }
    if(*argc < 3) {
        printf("set needs at least 2 arguments.\r\n");
    } else {
        long ln;
        int n;
        char *end_ptr;
        ln = strtol(argv[1], &end_ptr, 10);
        n = (int)ln;
        if(end_ptr == argv[1] || ln > INT_MAX || ln < INT_MIN) {
            fprintf(stderr, "Conversion error of argument %s\r\n", argv[1]);
            return;
        } else if(n < 0) {
            fprintf(stderr, "Blkno must be positive\r\n");
            return;
        }
        for(int i=2; i<*argc; i++) {
            for(p = stat_tbl; p->cmd; p++) {
                if(strcmp(argv[i], p->cmd) == 0) {
                    _set_stat(n, p->func);
                    break;
                }
            }
            if(p->cmd == NULL) {
                fprintf(stderr, "Stat %s is not defined.\r\n", argv[i]);
                return;
            }
        }
    }
}

void reset(int *argc, char *argv[])
{
    struct stat_func_table *p;
    if(!initialized) {
        fprintf(stderr, "Buffers haven't initialized, please run init first.\r\n");
        return;
    }
    if(*argc < 3) {
        printf("set needs at least 2 arguments.\r\n");
    } else {
        long ln;
        int n;
        char *end_ptr;
        ln = strtol(argv[1], &end_ptr, 10);
        n = (int)ln;
        if(end_ptr == argv[1] || ln > INT_MAX || ln < INT_MIN) {
            fprintf(stderr, "Conversion error of argument %s\r\n", argv[1]);
            return;
        } else if(n < 0) {
            fprintf(stderr, "Blkno must be positive\r\n");
            return;
        }
        for(int i=2; i<*argc; i++) {
            for(p = stat_tbl; p->cmd; p++) {
                if(strcmp(argv[i], p->cmd) == 0) {
                    _reset_stat(n, p->func);
                    break;
                }
            }
            if(p->cmd == NULL) {
                fprintf(stderr, "Stat %s is not defined.\r\n", argv[i]);
                return;
            }
        }
    }
}

static inline void show_descr(struct command_table *p) { printf("\x1b[1m%s\r\n\x1b[0m%s\r\n", p->cmd, p->descr); }
void help(int *argc, char *argv[])
{
    struct command_table* p;
    if(*argc == 1) {
        for(p = cmd_tbl; p->cmd; p++) {
            show_descr(p);
        }
    } else if(*argc == 2) { 
        for(p = cmd_tbl; p->cmd; p++) {
            if(strcmp(argv[1], p->cmd) == 0) {
                show_descr(p);
                return;
            }
        }
        if(p->cmd == NULL) {
            fprintf(stderr, "%s: Unknown command.\r\n", argv[2]);
        }
    } else {
        fprintf(stderr, "Too many args for help.\r\n");
    }
    return;
}

void getargs(int *argc, char *argv[], char *lbuf)
{
    *argc = 0;
    argv[0] = NULL;

    while(1) {
        while(isblank(*lbuf)) lbuf++;
        if(*lbuf == '\0') return;
        argv[(*argc)++] = lbuf;

        while(*lbuf && !isblank(*lbuf)) lbuf++;
        if(*lbuf == '\0') {
            return;
        }
        *lbuf++ = '\0';
    }
}

int parse_command()
{
    char lbuf[BUF_LEN], *argv[NARGS];
    int argc, i;
    struct command_table* p;

    fprintf(stderr, "$ ");
    if(fgets(lbuf, sizeof lbuf, stdin) == NULL) {
        fprintf(stderr, "\r\n");
        return 0;
    }
    lbuf[strlen(lbuf) - 1] = '\0';
    getargs(&argc, argv, lbuf);
    if(argc < 1) return 0;

    for(p = cmd_tbl; p->cmd; p++) {
        if(strcmp(argv[0], p->cmd) == 0) {
            (p->func)(&argc, argv);
            break;
        }
    }
    if(p->cmd == NULL) {
        fprintf(stderr, "%s: Unknown command. Please Try help.\r\n", argv[0]);
        return -1;
    }
    return 0;
}
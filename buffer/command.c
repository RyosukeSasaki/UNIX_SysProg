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
    {"set", set, "usage: set n stat [stat ...]\r\nSet stat on the buffer which blkno is n.\r\n"},
    {"reset", reset, "usage: reset n stat [stat ...]\r\nUnset stat on the buffer which blkno is n.\r\n"},
    {NULL, NULL}
};

void init(int *argc, char *argv[]){}
void buf(int *argc, char *argv[]){}
void hash(int *argc, char *argv[]){}
void free_func(int *argc, char *argv[]){}
void getblk(int *argc, char *argv[]){}
void brelse(int *argc, char *argv[]){}
void set(int *argc, char *argv[]){}
void reset(int *argc, char *argv[]){}
void quit(int *argc, char *argv[])
{
    exit(0);
}

static inline void show_descr(struct command_table *p)
{
    printf("\x1b[1m%s\r\n\x1b[0m%s\r\n", p->cmd, p->descr);
}

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
        if(*lbuf == '\0') return;
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

    for(p = cmd_tbl; p->cmd; p++) {
        if(strcmp(argv[0], p->cmd) == 0) {
            (p->func)(&argc, argv);
            break;
        }
    }
    if(p->cmd == NULL) {
        fprintf(stderr, "%s: unknown command", argv[0]);
        return -1;
    }
    return 0;
}
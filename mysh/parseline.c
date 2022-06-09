#include "parseline.h"

struct token_table tkn_tbl[] = {
    {'<', TKN_REDIR_IN},
    {'>', TKN_REDIR_OUT},
    {'|', TKN_PIPE},
    {'&', TKN_BG},
    {EOF, TKN_EOF},
    {'\n', TKN_EOL},
    {'\r', TKN_EOL},
    {0, 0}
};

int gettoken(char *tkn, int *len)
{
    *len = 0;
    int ret = ' ';
    struct token_table *p;

    while (isblank(ret)) ret=getc(stdin);
    if (ret == '>') {
        if ((ret=getc(stdin)) == '>') return TKN_REDIR_APPEND;
        if (ungetc(ret, stdin) == EOF) return TKN_ERR;
        return TKN_REDIR_OUT;
    }
    for (p=tkn_tbl; p->token; p++) {
        if (ret == p->token) return p->type;
    }
    if (ungetc(ret, stdin) == EOF) return TKN_ERR;
    while (1) {
        ret = getc(stdin);
        for (p=tkn_tbl; p->token; p++) {
            if (p->token == ret) {
                if (ungetc(ret, stdin) == EOF) return TKN_ERR;
                return TKN_NORMAL;
            }
        }
        *tkn++ = ret;
        (*len)++;
    }
    return TKN_NORMAL;
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
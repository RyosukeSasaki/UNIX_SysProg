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

void read_command()
{
    int len, i=0;
    int *argc, *type;
    char **argv;
    char *line_buf;
    struct line command_line;
    while (1) {
        fprintf(stderr, "mysh $ ");
        while (1) {
            if (i == TOKEN_MAX) {
                fprintf(stderr, "Too many tokens given.\r\n");
                i = 0;
                break;
            }
            argc = &command_line.blocks[i].argc;
            argv = command_line.blocks[i].argv;
            type = &command_line.blocks[i].type;
            line_buf = command_line.blocks[i].buf;
            //type[i] = gettoken(token[i], &len, TOKEN_LEN);
            //token[i][len] = '\0';
            //getargs(&argc[i], argv[i], token[i]);
            *type = gettoken(line_buf, &len, TOKEN_LEN);
            line_buf[len] = '\0';
            if (*type >= TKN_EOL) {
                break;
            } else if (*type == TKN_ERR) {
                fprintf(stderr, "Error on parsing line.\r\n");
                break;
            }
            getargs(argc, argv, line_buf);
            command_line.nblock = ++i;
        }

        // print
        for (int j=0; j<=command_line.nblock; j++) {
            printf("token %d type %d include %d args: \r\n", j, command_line.blocks[j].type,
            command_line.blocks[j].argc);
            for (int k=0; k<command_line.blocks[j].argc; k++) {
                printf("\targ %d: %s\r\n", k, command_line.blocks[j].argv[k]);
            }
        }

        // if line end
        type = &command_line.blocks[i-1].type;
        printf("last type %d\r\n", *type);
        if (*type < TKN_REDIR_IN || TKN_PIPE < *type) {
            i=0;
            if (execute(&command_line) < 0) fprintf(stderr, "Execution Error\r\n");
        }
    }
}

int gettoken(char *tkn, int *len, int max)
{
    *len = 0;
    int ret = ' ';
    struct token_table *p;

    while (isblank(ret)) ret=getc(stdin);
    if (ret == '>') {
        if ((ret=getc(stdin)) == '>') return TKN_REDIR_APPEND;
        if (ungetc(ret, stdin) == EOF) {
            perror("ungetc");
            return TKN_ERR;
        }
        return TKN_REDIR_OUT;
    }
    for (p=tkn_tbl; p->token; p++) {
        if (ret == p->token) return p->type;
    }
    if (ungetc(ret, stdin) == EOF) {
        perror("ungetc");
        return TKN_ERR;
    }
    while (1) {
        if (*len >= max) {
            fprintf(stderr, "Error: token is too long\r\n");
            return TKN_ERR;
        }
        ret = getc(stdin);
        for (p=tkn_tbl; p->token; p++) {
            if (p->token == ret) {
                if (ungetc(ret, stdin) == EOF) {
                    perror("ungetc");
                    return TKN_ERR;
                }
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
            //argv[(*argc)+1] = NULL;
            return;
        }
        *lbuf++ = '\0';
    }
}
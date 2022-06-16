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

int read_command(struct line *command_line)
{
    int len, i=0;
    int *argc, *type;
    char **argv;
    char *line_buf;
    //struct line command_line;

    do {
        while (1) {
            if (i == TOKEN_MAX) {
                fprintf(stderr, "Too many tokens given.\r\n");
                i = 0;
                break;
            }
            argc = &command_line->blocks[i].argc;
            argv = command_line->blocks[i].argv;
            type = &command_line->blocks[i].type;
            line_buf = command_line->blocks[i].buf;
            *type = gettoken(line_buf, &len, TOKEN_LEN);
            line_buf[len] = '\0';
            getargs(argc, argv, line_buf);
            command_line->nblock = ++i;
            if (*type >= TKN_EOL) {
                break;
            } else if (*type == TKN_ERR) {
                fprintf(stderr, "Error on parsing line.\r\n");
                break;
            }
        }
        type = &command_line->blocks[--i-1].type;

    } while (TKN_REDIR_IN <= *type && *type <= TKN_PIPE);
    i=0;
    return parseline(command_line);
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

int parseline(struct line *line)
{
    if (line->blocks[0].argc == 0) {
        fprintf(stderr, "first token is too short\r\n");
        return MYSH_PARSE_ERR;
    }
    line->bg = 0;

    for (int i=1; i<line->nblock-1; i++) {
        switch (line->blocks[i].type) {
            case TKN_REDIR_IN:
            case TKN_REDIR_OUT:
            case TKN_REDIR_APPEND:
                if ((line->blocks[i-1].type == TKN_NORMAL || line->blocks[i-1].type == TKN_DIR) &&
                line->blocks[i+1].type == TKN_NORMAL) {
                    line->blocks[i+1].type = TKN_DIR;
                    line->blocks[i].dir = line->blocks[i+1].argv[0];
                } else {
                    fprintf(stderr, "Invalid use of redirection\r\n");
                    return MYSH_PARSE_ERR;
                }
                break;
            case TKN_BG:
                if (i == line->nblock-2) {
                    line->bg = 1;
                } else {
                    fprintf(stderr, "'&' must be at the end of line\r\n");
                    return MYSH_PARSE_ERR;
                }
                break;
            default:
                break;
        }
    }
    /**
    for (int i=0;i<line->nblock; i++) {
        line->blocks[i].redir_in = 0;
        line->blocks[i].redir_out = 0;
        line->blocks[i].append = 0;
        line->blocks[i].pipe = NOPIPE;
    }
    for (int i=1; i<line->nblock-1; i++) {
        switch (line->blocks[i].type) {
            case TKN_REDIR_IN:
            case TKN_REDIR_OUT:
            case TKN_REDIR_APPEND:
                if (line->blocks[i-1].type == TKN_NORMAL && line->blocks[i+1].type == TKN_NORMAL) {
                    line->blocks[i-1].redir_out = 1;
                    line->blocks[i-1].dir_out = line->blocks[i+1].argv[0];
                    line->blocks[i+1].type = TKN_DIR;
                    if (line->blocks[i].type == TKN_REDIR_APPEND) 
                } else if ( line->blocks[i-3].type == TKN_NORMAL && 
                            line->blocks[i-1].type == TKN_DIR &&
                            line->blocks[i+1].type == TKN_NORMAL) {
                    line->blocks[i-3].redir_out = 1;
                    line->blocks[i-1].dir_out = line->blocks[i+1].argv[0];
                    line->blocks[i+1].type = TKN_DIR;
                } else {
                    fprintf(stderr, "Invalid use of redirection\r\n");
                    return MYSH_PARSE_ERR;
                }
                break;
            case TKN_PIPE:
                if (line->blocks[i-1].type == TKN_NORMAL && line->blocks[i+1].type == TKN_NORMAL) {
                    line->blocks[i-1].pipe += PIPE_OUT;
                    line->blocks[i+1].pipe += PIPE_IN;
                } else if ( line->blocks[i-3].type == TKN_NORMAL && 
                            line->blocks[i-1].type == TKN_DIR &&
                            line->blocks[i+1].type == TKN_NORMAL) {
                    line->blocks[i-3].pipe += PIPE_OUT;
                    line->blocks[i+1].pipe += PIPE_IN;
                } else {
                    fprintf(stderr, "Invalid use of pipe\r\n");
                    return MYSH_PARSE_ERR;
                }
                break;
            case TKN_BG:
                if (i == line->nblock-2) {
                    line->bg = 1;
                } else {
                    fprintf(stderr, "'&' must be at the end of line\r\n");
                    return MYSH_PARSE_ERR;
                }
                break;
            default:
                break;
        }
    }
    **/
    return MYSH_OK;
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
            argv[(*argc)] = NULL;
            return;
        }
        *lbuf++ = '\0';
    }
}
#pragma once
#define TOKEN_LEN 256
#define TOKEN_MAX 256
#define NARGS 64

/*
 * uncomment a line below to show debug message
 */
#define DEBUG
#ifdef DEBUG
#define debug(...) { fprintf(__VA_ARGS__); }
#else
#define debug 1 ? (void) 0 : fprintf
#endif

enum TKN_TYPES {
    TKN_ERR=0,
    TKN_NORMAL=1,
    TKN_DIR,
    TKN_REDIR_IN,
    TKN_REDIR_OUT,
    TKN_REDIR_APPEND,
    TKN_PIPE,
    TKN_BG,
    TKN_EOL,
    TKN_EOF
};

struct token_table {
    int token;
    enum TKN_TYPES type;
};

struct token_block {
    int type;
    int argc;
    int redir;
    char buf[TOKEN_LEN];
    char *argv[NARGS];
    char *dir;
};

struct line {
    int nblock;
    struct token_block blocks[TOKEN_MAX];
};
#pragma once
//#define PATH_LEN 256
#define TOKEN_LEN 256
#define TOKEN_MAX 256
#define NARGS 64

/*
 * uncomment a line below to show debug message
 */
//#define DEBUG
#ifdef DEBUG
#define debug(...) { fprintf(__VA_ARGS__); }
#else
#define debug 1 ? (void) 0 : fprintf
#endif
/*
 * for parser debug
 */
//#define PARSER_DEBUG
#ifdef PARSER_DEBUG
#define parse_debug(...) { fprintf(__VA_ARGS__); }
#else
#define parse_debug 1 ? (void) 0 : fprintf
#endif

enum MYSH_ERR_T {
    MYSH_OK=0,
    MYSH_PARSE_ERR=-1,
    MYSH_EXEC_ERR=-2,
    MYSH_ALLOC_ERR=-3,
    MYSH_FILE_ERR=-4,
    MYSH_ERR=-5
};

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

enum PIPE_DIR {
    NOPIPE=0,
    PIPE_IN,
    PIPE_OUT,
    PIPE_INOUT
};

struct token_table {
    int token;
    enum TKN_TYPES type;
};

struct token_block {
    int type;
    int argc;
    char *dir;
    char buf[TOKEN_LEN];
    char *argv[NARGS];
};

struct line {
    int nblock;
    int bg;
    struct token_block blocks[TOKEN_MAX];
};
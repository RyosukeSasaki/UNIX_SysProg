#pragma once

enum TKN_TYPES
{
    TKN_ERR=0,
    TKN_NORMAL=1,
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
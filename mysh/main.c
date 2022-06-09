#include <stdio.h>
#include "parseline.h"
#include "mysh_types.h"

int main()
{
    int len, type[TOKEN_MAX], argc[TOKEN_MAX];
    int i=0;
    char token[TOKEN_MAX][TOKEN_LEN];
    char *argv[TOKEN_MAX][NARGS];
    fprintf(stderr, "Welcome to mysh.\r\n");
    while (1) {
        fprintf(stderr, "mysh $ ");
        while (1) {
            if (i == TOKEN_MAX) {
                fprintf(stderr, "Too many tokens given.\r\n");
                i = 0;
                break;
            }
            type[i] = gettoken(token[i], &len, TOKEN_LEN);
            token[i][len] = '\0';
            if (type[i] >= TKN_EOL) {
                break;
            } else if (type[i] == TKN_ERR) {
                fprintf(stderr, "Error on parsing line.\r\n");
                break;
            }
            getargs(&argc[i], argv[i], token[i]);
            i++;
        }

        // print
        for (int j=0; j<i; j++) {
            printf("token %d type %d include %d args: \r\n", j, type[j], argc[j]);
            for (int k=0; k<argc[j]; k++) {
                printf("\targ %d: %s\r\n", k, argv[j][k]);
            }
        }

        // if line end
        if (type[i] < TKN_REDIR_IN && TKN_PIPE < type[i]) {
            i=0;
            printf("line ended, exec and return\r\n");
        }
    }
}
#include <stdio.h>
#include "parseline.h"
#define TOKEN_MAX 256
#define NARGS 64

int main()
{
    int len, type[TOKEN_MAX], argc[TOKEN_MAX];
    char token[TOKEN_MAX];
    char *argv[TOKEN_MAX][NARGS];
    while (1) {
        /**
        for (int i=0; i<TOKEN_MAX; i++) {
            type[i] = gettoken(token, &len);
            token[len] = '\0';
            getargs(&argc[i], argv[i], token);
            printf("type %d: \r\n", type[i]);
            for (int j=0; j<argc[i]; j++) {
                printf("\targ %d: %s\r\n", j, argv[i][j]);
            }
        }
        **/
    }
}
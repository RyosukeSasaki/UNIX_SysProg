#include <stdio.h>
#include <string.h>
#include <ctype.h>
#define BUF_LEN 256
#define NARGS 16

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

int main() 
{
    char lbuf[BUF_LEN], *argv[NARGS];
    int argc, i;

    fprintf(stderr, "input line: ");
    if(fgets(lbuf, sizeof lbuf, stdin) == NULL) {
        printf("\r\n");
        return 0;
    }
    lbuf[strlen(lbuf) - 1] = '\0';
    //if(*lbuf == '\0') continue;

    getargs(&argc, argv, lbuf);

    printf("argc: %d\r\n", argc);
    for(i = 0; i < argc; i++) {
        printf("argv[%d] = %s (len=%zu)\r\n", i, argv[i], strlen(argv[i]));
    }

}
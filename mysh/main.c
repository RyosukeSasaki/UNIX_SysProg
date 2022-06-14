#include <stdio.h>
#include "parseline.h"

int main()
{
    fprintf(stderr, "Welcome to mysh.\r\n");
    read_command();
    return 0;
}
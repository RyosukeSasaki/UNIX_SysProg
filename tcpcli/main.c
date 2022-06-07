#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int main() {
    struct addinfo hints, *res;
    char *host="131.113.108.54";
    char *serv="49152";
    int err;

    memset(&hints, 0, sizeof hints);
}

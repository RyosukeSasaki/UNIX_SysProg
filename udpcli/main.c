#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main()
{
    int s, count;
    struct sockaddr_in skt;
    char *sbuf = "testdata";
    char *ipaddrstr = "131.113.110.162";
    in_port_t port = 49152;
    struct in_addr ipaddr;
    char buf[512];
    socklen_t sktlen;

    if((s=socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    memset(&skt, 0, sizeof skt);
    skt.sin_family = AF_INET;
    skt.sin_port = htons(port);
    skt.sin_addr.s_addr = inet_addr(ipaddrstr);
    if ((count=sendto(s,sbuf,sizeof sbuf,0,(struct sockaddr *)&skt, sizeof skt)) < 0) {
        perror("sendto");
        exit(1);
    }

    close(s);
}
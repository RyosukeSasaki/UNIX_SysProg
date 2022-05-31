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
    int s, count, datalen;
    struct sockaddr_in skt;
    char *sbuf = "testdata";
    in_port_t port = 49152;
    struct in_addr ipaddr;

    if((s=socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    memset(&skt, 0, sizeof skt);
    inet_aton("131.113.110.100", &ipaddr);
    skt.sin_family = AF_INET;
    skt.sin_port = htons(port);
    skt.sin_addr.s_addr = htonl(ipaddr.s_addr);
    if ((count=sendto(s,sbuf,sizeof sbuf,0,(struct sockaddr *)&skt, sizeof skt)) < 0) {
        perror("sendto");
        exit(1);
    }
    close(s);
}
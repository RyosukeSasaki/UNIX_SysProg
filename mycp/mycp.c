#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#define READBLK 1

int main(int argc, char *argv[])
{
    if(argc != 3) {
        fprintf(stderr, "usage: mycp src dst\r\n");
        return -1;
    }
    int pos = 0;
    int readlen;
    int check = 1;
    int fd1, fd2;
    char rbuf[READBLK];
    char lbuf[256];
    struct stat sb1;
    if((fd1 = open(argv[1], O_RDONLY)) < 0) { perror("open"); return -1; }
    if((fd2 = open(argv[2], O_WRONLY | O_CREAT | O_EXCL, 0644)) < 0) {
        if(errno != EEXIST) { perror("open"); return -1; }
        while (check) {
            fprintf(stderr, "File exist. Overwrite? y/n: ");
            if(fgets(lbuf, sizeof lbuf, stdin) != NULL) {
                switch (lbuf[0]) {
                    case 'y':
                        check = 0;
                        break;
                    case 'n':
                        close(fd1);
                        return -1;
                    default:
                        break;
                }
            }
        }
        if((fd2 = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0) {
            perror("open");
            return -1;
        }
    }
    if(fstat(fd1, &sb1) < 0) { close(fd1); close(fd2); perror("fstat"); return -1;}
    while(pos < sb1.st_size) {
        readlen = read(fd1, rbuf, sizeof rbuf);
        if(write(fd2, rbuf, readlen) < 0) { close(fd1); close(fd2); perror("write"); return -1;}
        pos += readlen;
    }
    close(fd1); close(fd2);
    return 0;
}
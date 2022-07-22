#include "common.h"

char *normalize_path(char *arg)
{
    char relpath[PATH_MAX];
    char *fullpath;
    char *home;
    
    if (arg[0] == '~') {
        if ((home=getenv("HOME")) == NULL) return NULL;
        if (strlen(home)+strlen(arg) > PATH_MAX -1) {
            fprintf(stderr, "## path string is too long ##\n");
            errno = -1;
            return NULL;
        }
        memset(relpath, 0, sizeof(relpath));
        strcat(relpath, home);
        strcat(relpath, &arg[1]);
        if ((fullpath = realpath(relpath, NULL))==NULL) {
            perror("realpath");
            return NULL;
        }
    } else {
        if ((fullpath = realpath(arg, NULL))==NULL) {
            perror("realpath");
            return NULL;
        }
    }

    return fullpath;
}

void debug_msg(ftp_message_t *msg)
{
    debug(stderr, "\ttype     : %d\n", msg->type);
    debug(stderr, "\tcode     : %d\n", msg->code);
    debug(stderr, "\tlength   : %d\n", msg->length);
}

int send_msg(int *sfd, uint8_t type, uint8_t code, uint16_t length, uint8_t *data)
{
    int ret=0;
    ftp_message_t *msg = 
        (ftp_message_t *)malloc(sizeof(uint8_t)*(HEADER_SIZE+length));
    if (type!=TYPE_DATA||(type==TYPE_DATA&&code==CMD_DATA_LAST)) first_data = 1;
    msg->type = type;
    msg->code = code;
    msg->length = length;
    debug(stderr, "-- send message --\n");
    debug_msg(msg);
    debug(stderr, "------------------\n");
    msg->length = htons(length);
    if (data != NULL) {
        memcpy(msg->data, data, length);
    } else if (length > 0) {
        memset(msg->data, 0, length);
    }
    if (send(*sfd, msg, HEADER_SIZE+length, 0) < 0) {
        perror("send");
        ret = -1;
    }
    free(msg);

    return ret;
}

int change_dir(char *path)
{
    int ret;
    char *fullpath;
    if ((fullpath = normalize_path(path)) != NULL) {
        if ((ret=chdir(fullpath)) < 0) perror("chdir");
    } else {
        return -1;
    }
    free(fullpath);
    return ret;
}

int list_dir(char *search_path)
{
    int ret;
    DIR *dir;
    struct dirent *entry;
    struct stat st;

    char *fullpath;
    char name_full[PATH_MAX];
    if ((fullpath = normalize_path(search_path)) == NULL) {
        file_err(errno);
        return -1;
    }
    if (stat(fullpath, &st) < 0) {
        perror("stat");
        file_err(errno);
        free(fullpath);
        return -1;
    }
    if (S_ISDIR(st.st_mode)) {
        if ((dir = opendir(fullpath)) == NULL) {
            perror("opendir");
            file_err(errno);
            free(fullpath);
            return -1;
        }
        while ((entry = readdir(dir)) != NULL) {
            sprintf(name_full, "%s/%s", fullpath, entry->d_name);
            if (stat(name_full, &st) < 0) {
                perror("stat");
                file_err(errno);
                free(fullpath);
                return -1;
            }
            if ((ret=print_dir(&st, entry->d_name)) < 0) break;
        }
        closedir(dir);
    } else {
        ret = print_dir(&st, fullpath);
    }
    free(fullpath);
    return ret;
}

int send_file(int *sfd, int *fd)
{
    int pos=0, readlen=0;
    char buf[READBLK];
    struct stat st;
    if (fstat(*fd, &st) < 0) {
        perror("fstat");
        return -1;
    }
    while(pos < st.st_size) {
        if ((readlen=read(*fd, buf, sizeof(buf))) < 0) {
            perror("read");
            return -1;
        }
        if (send_data(readlen, buf) < 0) return -1;
        pos += readlen;
    }
    return 0;
}

int recv_file(int *sfd, int *fd)
{
    ftp_message_t reply;
    char buf[READBLK];
    int remain, ret, readlen;

    while (1) {
        if (recv_msg(&reply, HEADER_SIZE) < 0) return -1;
        remain = reply.length = ntohs(reply.length);
        if (reply.type == TYPE_DATA) {
            if (reply.length > 0) {
                while (remain > 0) {
                    readlen = (remain > READBLK)? READBLK:remain;
                    if ((ret=recv_msg(buf, readlen)) < 0) return -1;
                    if (write(*fd, buf, ret) < 0) return -1;
                    remain -= ret;
                }
            }
            if (reply.code == CMD_DATA_LAST) break;
        } else {
            recv_err(&reply);
            return -1;
        }
    }
    return 0;
}
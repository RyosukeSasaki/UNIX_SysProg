#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <linux/limits.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MYFTP_PORT 50021
#define HEADER_SIZE 4
#define TIMEOUT 1
#define BUF_LEN 256

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define CMD_OK 0x00
#define CMD_ERR_NEGATION 0x00
#define CMD_DATA_RETR 0x01
#define CMD_ERR_SYNTAX 0x01
#define CMD_ERR_PERMISSION 0x01
#define CMD_DATA_STOR 0x02
#define CMD_ERR_UNDEF 0x02
#define CMD_ERR_PRTCL 0x03
#define CMD_ERR_UNKWN 0x05

enum ftp_type {
    TYPE_QUIT = 0x01,
    TYPE_PWD,
    TYPE_CWD,
    TYPE_LIST,
    TYPE_RETR,
    TYPE_STOR,
    TYPE_OK = 0x10,
    TYPE_ERR_CMD,
    TYPE_ERR_FILE,
    TYPE_ERR_UNKWN,
    TYPE_DATA = 0x20
};

typedef struct _ftp_message {
    uint8_t type;
    uint8_t code;
    uint16_t length;
    uint8_t data[0];
} ftp_message_t;

char *normalize_path(char *arg);
void debug_msg(ftp_message_t *);

#endif
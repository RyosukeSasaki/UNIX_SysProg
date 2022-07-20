#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <linux/limits.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MYFTP_PORT 50021
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

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

enum ftp_code {
    CMD_OK = 0x00,
    CMD_DATA_RETR,
    CMD_DATA_STOR,
    CMD_ERR_PRTCL,
    CMD_ERR_UNKWN = 0x05
};

typedef struct _ftp_message {
    uint8_t type;
    uint8_t code;
    uint16_t length;
    char data[0];
} ftp_message_t;

char *normalize_path(char *arg);

#endif
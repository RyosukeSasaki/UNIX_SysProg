#ifndef BUFFER_H
#define BUFFER_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

/*
#define STAT_LOCKED     0b1
#define STAT_VALID      0b10
#define STAT_DWR        0b100
#define STAT_KRDWR      0b1000
#define STAT_WAITED     0b10000
#define STAT_OLD        0b100000
*/

#define HASH_SIZE 4

struct _buf_state{
    unsigned locked:    1;
    unsigned valid:     1;
    unsigned dwr:       1;
    unsigned krdwr:     1;
    unsigned waited:    1;
    unsigned old:       1;
};

typedef union _state_t{
    struct _buf_state bits;
    unsigned int s;
} state_t;

typedef struct BUF_HEADER{
    int blkno;
    char* cache_data;
    struct BUF_HEADER* hash_fp;
    struct BUF_HEADER* hash_bp;
    state_t stat;
    struct BUF_HEADER* free_fp;
    struct BUF_HEADER* free_bp;
} buf_header;


buf_header hash_head[HASH_SIZE];
buf_header freelist;


inline int hash(int blkno) { return blkno % HASH_SIZE; }
buf_header* search_hash(int);
void insert_head(buf_header*, buf_header*);
void insert_tail(buf_header*, buf_header*);
void remove_from_hash(buf_header*);
void insert_freelist_head(buf_header*, buf_header*);
void insert_freelist_taik(buf_header*, buf_header*);
void remove_from_freelist(buf_header*);
bool freelist_is_empty();
buf_header* get_oldest_buffer();

bool is_locked(buf_header*);
bool is_valid(buf_header*);
bool is_dwr(buf_header*);
bool is_krdwr(buf_header*);
bool is_waited(buf_header*);
bool is_old(buf_header*);
void set_locked(buf_header*, bool);
void set_valid(buf_header*, bool);
void set_dwr(buf_header*, bool);
void set_krdwr(buf_header*, bool);
void set_waited(buf_header*, bool);
void set_old(buf_header*, bool);

#endif
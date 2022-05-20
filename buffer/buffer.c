#include "buffer.h"

buf_header hash_head[HASH_SIZE];
buf_header freelist;
buf_header buffer[BUF_SIZE];
const int init_blkno[BUF_SIZE] = {28,4,64,17,5,97,98,50,10,3,35,99};
const int init_free[INIT_FREE] = {3,5,4,28,97,10};
static inline int hash(int blkno) { return blkno % HASH_SIZE; }
bool initialized = false;

void init_buf()
{
    initialized = true;
    for(int i=0; i<HASH_SIZE; i++) {
        hash_head[i].hash_fp = hash_head[i].hash_bp = &hash_head[i];
    }
    freelist.free_fp = freelist.free_bp = &freelist;
    for(int i=0; i<BUF_SIZE; i++) {
        add_buf_to_hashlist(init_blkno[i], &buffer[i]);
        set_stat(&buffer[i], 0);
        set_valid(&buffer[i], true);
        set_locked(&buffer[i], true);
        buffer[i].buf_number = i;
    }
    for(int i=0; i<INIT_FREE; i++) {
        insert_freelist_tail(search_hash(init_free[i]));
    }
}

buf_header* search_hash(int blkno)
{
    int h;
    buf_header *p;
    h = hash(blkno);
    for(p=hash_head[h].hash_fp; p!=&hash_head[h]; p=p->hash_fp) if(p->blkno == blkno) return p;
    return NULL;
}

void insert_head(buf_header* h, buf_header* new)
{
    new->hash_bp = h;
    new->hash_fp = h->hash_fp;
    h->hash_fp->hash_bp = new;
    h->hash_fp = new;
}

void insert_tail(buf_header* h, buf_header* new)
{
    new->hash_bp = h->hash_bp;
    new->hash_fp = h;
    h->hash_bp->hash_fp = new;
    h->hash_bp = new;
}

void add_buf_to_hashlist(int blkno, buf_header* new)
{
    new->blkno = blkno;
    insert_tail(&hash_head[hash(blkno)], new);
}

void remove_from_hash(buf_header *p)
{
    p->hash_bp->hash_fp = p->hash_fp;
    p->hash_fp->hash_bp = p->hash_bp;
    p->hash_fp = NULL;
    p->hash_bp = NULL;
}

void insert_freelist_head(buf_header* new)
{
    set_locked(new, false);
    new->free_bp = &freelist;
    new->free_bp = freelist.free_fp;
    freelist.free_fp->free_bp = new;
    freelist.free_fp = new;
}

void insert_freelist_tail(buf_header* new)
{
    set_locked(new, false);
    new->free_bp = freelist.free_bp;
    new->free_fp = &freelist;
    freelist.free_bp->free_fp = new;
    freelist.free_bp = new;
}

void remove_from_freelist(buf_header *p)
{
    set_locked(p, true);
    p->free_bp->free_fp = p->free_fp;
    p->free_fp->free_bp = p->free_bp;
    p->free_fp = NULL;
    p->free_bp = NULL;
}

buf_header* get_oldest_buffer()
{
    return freelist.free_fp;
}

bool is_freelist_empty()
{
    if(freelist.free_fp == &freelist) return true;
    return false;
}

int is_locked(buf_header *p) { return p->stat & STAT_LOCKED; }
int is_valid(buf_header *p) { return p->stat & STAT_VALID; }
int is_dwr(buf_header *p) { return p->stat & STAT_DWR; }
int is_krdwr(buf_header *p) { return p->stat & STAT_KRDWR; }
int is_waited(buf_header *p) { return p->stat & STAT_WAITED; }
int is_old(buf_header *p) { return p->stat & STAT_OLD; }
void set_locked(buf_header *p, bool val) { p->stat = val ? p->stat | STAT_LOCKED : p->stat & ~STAT_LOCKED; }
void set_valid(buf_header *p, bool val) { p->stat = val ? p->stat | STAT_VALID : p->stat & ~STAT_VALID; }
void set_dwr(buf_header *p, bool val) { p->stat = val ? p->stat | STAT_DWR : p->stat & ~STAT_DWR; }
void set_krdwr(buf_header *p, bool val) { p->stat = val ? p->stat | STAT_KRDWR : p->stat & ~STAT_KRDWR; }
void set_waited(buf_header *p, bool val) { p->stat = val ? p->stat | STAT_WAITED : p->stat & ~STAT_WAITED; }
void set_old(buf_header *p, bool val) { p->stat = val ? p->stat | STAT_OLD : p->stat & ~STAT_OLD; }
void set_stat(buf_header *p, uint8_t val) { p->stat = val; }

void buf_stat(buf_header *p, char *stat_str)
{
    snprintf(stat_str, sizeof stat_str, "%c%c%c%c%c%c",
    is_old(p)       ? 'O':'-',
    is_waited(p)    ? 'W':'-',
    is_krdwr(p)     ? 'K':'-',
    is_dwr(p)       ? 'D':'-',
    is_valid(p)     ? 'V':'-',
    is_locked(p)    ? 'L':'-');
}

void show_buffer(int buf_number)
{
    char stat_str[7];
    buf_header *p = &buffer[buf_number];
    buf_stat(p, stat_str);
    printf("[ %2d: %2d %s]", p->buf_number, p->blkno, stat_str);
}

void show_hash(int hash_number)
{
    buf_header *p;
    printf("%d: ", hash_number);
    for(p=hash_head[hash_number].hash_fp; p!=&hash_head[hash_number]; p=p->hash_fp) {
        show_buffer(p->buf_number);
        printf(" ");
    }
    printf("\r\n");
}

void show_free()
{
    buf_header *p;
    for(p=freelist.free_fp; p!=&freelist; p=p->free_fp) {
        show_buffer(p->buf_number);
        printf(" ");
    }
    printf("\r\n");
}
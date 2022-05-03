#include "buffer.h"

buf_header hash_head[HASH_SIZE];
buf_header freelist;
static inline int hash(int blkno) { return blkno % HASH_SIZE; }

buf_header* search_hash(int blkno)
{
    int h;
    buf_header* p;
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
    insert_tail(&hash_head[hash(blkno)], new);
}

void remove_from_hash(buf_header* p)
{
    p->hash_bp->hash_fp = p->hash_fp;
    p->hash_fp->hash_bp = p->hash_bp;
    p->hash_fp = NULL;
    p->hash_bp = NULL;
}

void insert_freelist_head(buf_header* h, buf_header* new)
{
    new->free_bp = h;
    new->free_bp = h->free_fp;
    h->free_fp->free_bp = new;
    h->free_fp = new;
}

void insert_freelist_tail(buf_header* h, buf_header* new)
{
    new->free_bp = h->free_bp;
    new->free_fp = h;
    h->free_bp->free_fp = new;
    h->free_bp = new;
}

void remove_from_freelist(buf_header* p)
{
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

int is_locked(buf_header* p) { return p->stat & STAT_LOCKED; }
int is_valid(buf_header* p) { return p->stat & STAT_VALID; }
int is_dwr(buf_header* p) { return p->stat & STAT_DWR; }
int is_krdwr(buf_header* p) { return p->stat & STAT_KRDWR; }
int is_waited(buf_header* p) { return p->stat & STAT_WAITED; }
int is_old(buf_header* p) { return p->stat & STAT_OLD; }
void set_locked(buf_header* p, bool val) { p->stat = val ? p->stat | STAT_LOCKED : p->stat & ~STAT_LOCKED; }
void set_valid(buf_header* p, bool val) { p->stat = val ? p->stat | STAT_VALID : p->stat & ~STAT_VALID; }
void set_dwr(buf_header* p, bool val) { p->stat = val ? p->stat | STAT_DWR : p->stat & ~STAT_DWR; }
void set_krdwr(buf_header* p, bool val) { p->stat = val ? p->stat | STAT_KRDWR : p->stat & ~STAT_KRDWR; }
void set_waited(buf_header* p, bool val) { p->stat = val ? p->stat | STAT_WAITED : p->stat & ~STAT_WAITED; }
void set_old(buf_header* p, bool val) { p->stat = val ? p->stat | STAT_OLD : p->stat & ~STAT_OLD; }
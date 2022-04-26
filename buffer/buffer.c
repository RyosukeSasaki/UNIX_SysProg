#include "buffer.h"

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

bool freelist_is_empty()
{
    if(freelist.free_fp == &freelist) return true;
    return false;
}

bool is_locked(buf_header* p) { return p->stat.bits.locked; }
void set_locked(buf_header* p, bool val) { p->stat.bits.locked = val; }
#include "blockfunc.h"

buf_header* _getblk(int blkno)
{
    buf_header* p;
    while(1) {
        p = search_hash(blkno);
        if(p != NULL) {
            // if blkno in hash queue
            if(is_locked(p)) {
                // scenario 5
                printf("Scenario 5: Block %d is in queue but locked.\r\n", blkno);
                printf("Wait to be unlocked.\r\n");
                set_waited(p, true);
                // sleep()
                printf("Process goes to sleep\r\n");
                // continue;
                return NULL;
            }
            printf("Scenario 1: Block %d is in queue and available.\r\n", blkno);
            set_locked(p, true);
            remove_from_freelist(p);
            return p;
        } else {
            if(is_freelist_empty()) {
                // scenario 4
                printf("Scenario 4: Block %d is not in queue, need to be acquired.\r\n", blkno);
                printf("However, free list is empty, wait any buffer to be released.\r\n");
                // sleep()
                printf("Process goes to sleep\r\n");
                // continue;
                return NULL;
            }
            p = get_oldest_buffer();
            set_locked(p, true);
            remove_from_freelist(p);
            if(is_dwr(p)) {
                printf("Scenario 3: Oldest buffer(blkno: %d) is delayed for write.\r\n", p->blkno);
                printf("Write buffer to the disk.\r\n");
                //set_old(p, true);
                //set_dwr(p, false);
                continue;
            }
            printf("Scenario 2: buffer replacement has occured. Block %d will replaced by block %d.\r\n", p->blkno, blkno);
            remove_from_hash(p);
            add_buf_to_hashlist(blkno, p);
            set_valid(p, false);
            return p;
        }
    }
}

void _brelse(int blkno)
{
    buf_header *buf;
    buf = search_hash(blkno);
    if(buf == NULL) {
        fprintf(stderr, "Buffer %d is not in queue.\r\n", blkno);
        return;
    }
    if(!is_locked(buf)) {
        fprintf(stderr, "Buffer %d is not locked. Nothing done.\r\n", blkno);
        return;
    }
    if(is_freelist_empty()) {
        // wakeup()
        printf("Scenario4: Wakeup Processes waiting for any buffer\r\n");
    }
    if(is_waited(buf)) {
        // wakeup()
        printf("Scenario5: Wakeup processes waiting for the buffer of blkno %d\r\n", blkno);
    }
    // raise_cpu_level();
    if(is_valid(buf) && !is_old(buf)) {
        insert_freelist_tail(buf);
    } else if(is_valid(buf) && is_old(buf)) {
        insert_freelist_head(buf);
        set_old(buf, false);
    } else {
        fprintf(stderr, "Buffer state is invalid.\r\n");
        return;
    }
    // lower_cpu_level();
    set_locked(buf, false);
}

void _set_stat(int blkno, void (*func)(buf_header*, bool))
{
    buf_header *buf;
    buf = search_hash(blkno);
    if(buf == NULL) {
        fprintf(stderr, "Buffer %d is not in queue.\r\n", blkno);
        return;
    }
    (*func)(buf, true);
}

void _reset_stat(int blkno, void (*func)(buf_header*, bool))
{
    buf_header *buf;
    buf = search_hash(blkno);
    if(buf == NULL) {
        fprintf(stderr, "Buffer %d is not in queue.\r\n", blkno);
        return;
    }
    (*func)(buf, false);
}
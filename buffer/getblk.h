#ifndef _GETBLK_H
#define _GETBLK_H

#include "buffer.h"
#include <stdbool.h>

buf_header* getblk(int blkno)
{
    buf_header* p;
    while(1) {
        /*
        p = search_hash(blkno);
        if(p != NULL) {
            // if blkno in hash queue
            if(is_locked(p)) {
                // scenario 5
                printf("scenario 5: block %d is in queue but locked.\r\n", blkno);
                printf("wait to be unlocked.\r\n");
                // sleep()
                printf("Process goes to sleep\r\n");
                return NULL;
                // continue;
            }
            printf("scenario 1: block %d is in queue and available.\r\n", blkno);
            set_locked(p, true);
            remove_from_freelist(p);
            return p;
        } else {
            if(freelist_is_empty()) {
                // scenario 4
                printf("scenario 4: block %d is not in queue, need to be acquired.\r\n", blkno);
                printf("However, free list is empty, wait any buffer to be released\r\n");
                // sleep()
                printf("Process goes to sleep\r\n");
                return NULL;
                // continue;
            }
            p = get_oldest_buffer();
        }
    */
    }
}

#endif
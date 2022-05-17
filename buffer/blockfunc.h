#ifndef _BLOCKFUNC_H
#define _BLOCKFUNC_H

#include "buffer.h"
#include <stdbool.h>

buf_header* _getblk(int);
void _brelse(buf_header *);

#endif
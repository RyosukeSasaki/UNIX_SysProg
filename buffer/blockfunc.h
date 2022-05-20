#ifndef _BLOCKFUNC_H
#define _BLOCKFUNC_H

#include "buffer.h"
#include <stdbool.h>

buf_header* _getblk(int);
void _brelse(int);
void _set_stat(int, void (*)(buf_header*, bool));
void _reset_stat(int, void (*)(buf_header*, bool));

#endif
#include "buffer.h"

#define CHAR_BIT 8
void printb(unsigned int v) {
  unsigned int mask = (int)1 << (sizeof(v) * CHAR_BIT - 1);
  do putchar(mask & v ? '1' : '0');
  while (mask >>= 1);
}
void putb(unsigned int v) {
  putchar('0'), putchar('b'), printb(v), putchar('\n');
}

buf_header bb;
int main()
{
    putb(bb.stat);
    set_locked(&bb, true);
    putb(bb.stat);
    set_valid(&bb, true);
    putb(bb.stat);
    set_dwr(&bb, true);
    putb(bb.stat);
    set_krdwr(&bb, true);
    putb(bb.stat);
    set_waited(&bb, true);
    putb(bb.stat);
    set_old(&bb, true);
    putb(bb.stat);
    set_locked(&bb, false);
    putb(bb.stat);
    set_valid(&bb, false);
    putb(bb.stat);
    set_dwr(&bb, false);
    putb(bb.stat);
    set_krdwr(&bb, false);
    putb(bb.stat);
    set_waited(&bb, false);
    putb(bb.stat);
    set_old(&bb, false);
    putb(bb.stat);
    return 0;
}
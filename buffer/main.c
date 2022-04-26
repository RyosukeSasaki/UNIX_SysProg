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

int main()
{
    /*
    state_t st;
    st.bits.locked = true;
    putb(st.s);
    st.bits.valid = true;
    putb(st.s);
    st.bits.dwr = true;
    putb(st.s);
    st.bits.krdwr = true;
    putb(st.s);
    st.bits.waited = true;
    putb(st.s);
    st.bits.old = true;
    putb(st.s);
    st.bits.locked = false;
    putb(st.s);
    st.bits.valid = false;
    putb(st.s);
    st.bits.dwr = false;
    putb(st.s);
    st.bits.krdwr = false;
    putb(st.s);
    st.bits.waited = false;
    putb(st.s);
    st.bits.old = false;
    putb(st.s);
    /*
    return 0;
}
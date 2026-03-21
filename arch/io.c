#include "io.h"
unsigned char portByteIn(unsigned short port) {
    unsigned char result;
    __asm__ volatile("in %%dx,%%al" : "=a" (result) : "d" (port));
    return result;
}
unsigned char portByteOut(unsigned short port,unsigned char data) {
    __asm__ volatile("outb %%al,%%dx" : : "a" (data), "d" (port));
}

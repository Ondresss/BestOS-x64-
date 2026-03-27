#include "io.h"
unsigned char portByteIn(unsigned short port) {
    unsigned char result;
    __asm__ volatile("in %%dx,%%al" : "=a" (result) : "d" (port));
    return result;
}
unsigned char portByteOut(unsigned short port,unsigned char data) {
    __asm__ volatile("outb %%al,%%dx" : : "a" (data), "d" (port));
}
unsigned short portWordIn(unsigned short  port) {
    unsigned short  result;
    __asm__ volatile("inw %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}
void portWordOut(unsigned short  port, unsigned short  data) {
    __asm__ volatile("outw %0, %1" : : "a"(data), "Nd"(port));
}
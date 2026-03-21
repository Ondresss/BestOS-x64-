#include "./serial.h"

void serial_print(const char *str) {
    int len = stringLength(str);
    for (int i = 0; i < len; ++i) {
        serial_putchar(str[i]);
    }
}


void serial_putchar(char c) {
    while ((portByteIn(0x3F8 + 5) & 0x20) == 0);

    portByteOut(0x3F8, c);
}

char serial_getchar() {
    while ((portByteIn(0x3F8 + 5) & 1) == 0);

    return portByteIn(0x3F8);
}


void serial_init() {
    portByteOut(COM1 + 1, 0x00);

    portByteOut(COM1 + 3, 0x80);

    portByteOut(COM1 + 0, 0x03);
    portByteOut(COM1 + 1, 0x00);

    portByteOut(COM1 + 3, 0x03);

    portByteOut(COM1 + 2, 0xC7);

    portByteOut(COM1 + 4, 0x0B);

    serial_print("Serial line intialized\n");
}
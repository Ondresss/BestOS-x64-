#include "keyboard.h"
void serial_print_hex8(unsigned char n) {
    char const hex_chars[] = "0123456789ABCDEF";
    serial_putchar('0');
    serial_putchar('x');
    serial_putchar(hex_chars[(n >> 4) & 0x0F]);
    serial_putchar(hex_chars[n & 0x0F]);
    serial_putchar(' ');
}

char keyboard_getchar() {
    unsigned char keyTable[128] = {
        0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
        '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
        0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
        '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ',
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        '-', 0, 0, 0, '+', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    while (1) {
        if (portByteIn(0x64) & 0x01) {
            unsigned char scancode = portByteIn(0x60);
            if (!(scancode & 0x80)) {
                if (scancode < 128 && keyTable[scancode] != 0) {
                    return keyTable[scancode];
                }
            }
        }
    }
}
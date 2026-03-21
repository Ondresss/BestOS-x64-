#pragma once
#include "../arch/io.h"
#include "../utils/strings.h"
#define COM1     0x3F8

void serial_init();
void serial_putchar(char c);
char serial_getchar();
void serial_print(const char *str);
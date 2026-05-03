#pragma once
#include "../arch/io.h"
#include "./serial.h"
#include <stdint.h>
#include "../arch/idt.h"
#include  "../arch/isr.h"
typedef enum {
    EOF_EV,
    NONE_EV
}Event;

char keyboard_getchar();
Event keyboard_getEvent();
uint32_t keyboard_callback(struct registers *regs);
void init_keyboard();
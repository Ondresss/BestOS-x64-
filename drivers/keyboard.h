#pragma once
#include "../arch/io.h"
#include "./serial.h"

typedef enum {
    EOF_EV,
    NONE_EV
}Event;

char keyboard_getchar();
Event keyboard_getEvent();
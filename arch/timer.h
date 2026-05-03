#pragma once
#include "io.h"
#include "idt.h"
#include "isr.h"
#include <stdint.h>
#include "../scheduler/scheduler.h"
#include  "../drivers/vga.h"
#include  "../utils/strings.h"
void timer_init(uint32_t frequency);
uint32_t timer_callback(struct registers *regs);
void timer_uptime();
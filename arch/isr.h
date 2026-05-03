#pragma once
#include  "../drivers/vga.h"
#include  "timer.h"
#include  "idt.h"
#include  "../scheduler/scheduler.h"
typedef uint32_t (*isr_t)(struct registers*);
uint32_t generic_isr_handler(struct registers *regs);
void register_interrupt_handler(uint8_t n, isr_t handler);
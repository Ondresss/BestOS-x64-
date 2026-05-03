#include "idt.h"
#include  "../drivers/serial.h"
extern void common_isr_handler();
extern void isr13();
extern void isr32();
extern void isr33();
extern void isr48();
extern void isr46();
extern void isr47();
struct idt_entry idt[256];
struct idt_ptr idtp;

void idt_set_gate(int n, uint32_t handler) {
    idt[n].base_low = (handler & 0xFFFF);
    idt[n].base_high = (handler >> 16) & 0xFFFF;
    idt[n].sel = 0x08;
    idt[n].always0 = 0;
    idt[n].flags = 0x8E;
}

void idt_load() {
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base = (uint32_t)&idt;

    __asm__ volatile("lidt %0" : : "m"(idtp));
}


void idt_init_test() {
    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, (uint32_t)isr48);
    }

    idt_set_gate(32, (uint32_t)isr32);
    idt_set_gate(33, (uint32_t)isr33);
    idt_set_gate(48, (uint32_t)isr48);
    idt_set_gate(46, (uint32_t)isr46);
    idt_set_gate(47, (uint32_t)isr47);

    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base = (uint32_t)&idt;

    __asm__ volatile("lidt %0" : : "m"(idtp));
}
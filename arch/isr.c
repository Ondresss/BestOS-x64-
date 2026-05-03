#include "isr.h"

#include "../drivers/serial.h"

isr_t interrupt_handlers[256] = {0};

void register_interrupt_handler(uint8_t n, isr_t handler) {
    interrupt_handlers[n] = handler;
}

uint32_t generic_isr_handler(struct registers *regs) {
    uint32_t final_esp = (uint32_t)regs;
    if (interrupt_handlers[regs->int_no] != 0) {
        interrupt_handlers[regs->int_no](regs);
    }

    if (regs->int_no == 32) {

        isr_t timerCallback = interrupt_handlers[regs->int_no];
        final_esp = timerCallback(regs);
        portByteOut(0x20, 0x20);
    }
    else if (regs->int_no > 32 && regs->int_no <= 47) {
        if (regs->int_no >= 40) portByteOut(0xA0, 0x20);
        portByteOut(0x20, 0x20);
    }

    return final_esp;
}
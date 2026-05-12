#include "timer.h"

#include "../drivers/serial.h"

void timer_init(uint32_t frequency) {
    uint32_t divisor = 1193182 / frequency;

    portByteOut(0x43, 0x36);

    uint8_t low = (uint8_t)(divisor & 0xFF);
    uint8_t high = (uint8_t)((divisor >> 8) & 0xFF);

    portByteOut(0x40, low);
    portByteOut(0x40, high);

    register_interrupt_handler(32, timer_callback);
}


unsigned int total_ticks = 0;
uint32_t timer_callback(struct registers *regs) {
    uint32_t final_esp = (uint32_t)regs;
    static int ticks_elapsed = 0;
    ticks_elapsed++;
    total_ticks++;
    char tickStr[12] = {0};
    unsignedIntToString(tickStr, total_ticks);
    serial_print("TICK: ");
    serial_print(tickStr);
    serial_print("\n");
    if (ticks_elapsed >= 12) {
        ticks_elapsed = 0;
        final_esp = schedule_tick((uint32_t)regs);
        gt_current->noSwitches++;
    } else {
        gt_current->totalTime++;
    }
    return final_esp;

}

void formatTwoDigits(char* buf, int number) {
    buf[0] = (number / 10) + '0';
    buf[1] = (number % 10) + '0';
    buf[2] = '\0';
}

void timer_uptime() {
    while(1) {
        int total_seconds = total_ticks / 100;
        int s = total_seconds % 60;
        int m = (total_seconds / 60) % 60;
        int h = (total_seconds / 3600);

        char tmp[11];

        displayStringAt("Uptime: ", GREEN_ON_BLACK, 55, 0);

        formatTwoDigits(tmp,h);
        displayStringAt(tmp, LIGHT_RED, 63, 0);
        displayStringAt(":", LIGHT_RED, 65, 0);
        formatTwoDigits(tmp,m);
        displayStringAt(tmp, LIGHT_RED, 66, 0);
        displayStringAt(":", LIGHT_RED, 68, 0);

        formatTwoDigits(tmp,s);
        displayStringAt(tmp, LIGHT_RED, 69, 0);

        for(volatile int i = 0; i < 1000000; i++);
    }
}
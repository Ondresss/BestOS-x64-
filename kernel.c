#include "./drivers/vga.h"
#include "./drivers/serial.h"
#include "./drivers/keyboard.h"
#include "./drivers/ide.h"
#include "./cli/cli.h"
#include  "./diskAccess/diskAccess.h"
#include  "./arch/idt.h"
#include "./arch/pic.h"
#include  "./arch/timer.h"
#include  "./scheduler/scheduler.h"
#include  "./scheduler/memory.h"
void main() {
    serial_init();
    clearScreen(0x07);

    idt_init_test();
    pic_remap();
    memory_init();
    scheduler_init();
    initFileSystem();
    const char* line1 = " \\______   \\ ____   _______/  |_\\_____  \\  /   _____/ ";
    const char* line2 = "  |    |  _// __ \\ /  ___/\\   __\\/   |   \\ \\_____  \\  ";
    const char* line3 = "  |    |   \\  ___/ \\___ \\  |  | /    |    \\/        \\ ";
    const char* line4 = "  |______  /\\___  >____  > |__| \\_______  /_______  / ";
    const char* line5 = "         \\/     \\/     \\/               \\/        \\/  ";
    clearScreen(0x07);
    const char* ascii_art[5] = {line1, line2, line3, line4, line5};
    setCursor(10 * 80);
    for(int i = 0; i < 5; i++) {
        displayString(ascii_art[i], GREEN_ON_BLACK);
        displayString("\n", 0);
    }
    displayString("#INFO: <Testing VGA output>\n",0x0f);
    displayString("#INFO <Testing SERIAL output>\n",0x0f);
    serial_print("#INFO <Testing SERIAL output>\n");
    displayString("#INFO <Keyboard initialized>\n",0x0f);
    displayString("#INFO <File system initialized>\n",0x0f);

    schedule_create(cliLoop);
    schedule_create(timer_uptime);
    schedule_create(cliPrintCPUVendor);

    timer_init(100);
    init_keyboard();
    register_interrupt_handler(46, disk_callback);
    register_interrupt_handler(47, disk_callback);

    serial_print("Starting Multitasking...\n");

    asm volatile("sti");

    while(1) {
        asm volatile("hlt");
    }
}
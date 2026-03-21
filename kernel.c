#include "./drivers/vga.h"
#include "./drivers/serial.h"
#include "./drivers/keyboard.h"
void displayChar(int x,int y,char c,unsigned char color) ;
void getCPUVendor(char* vendor);

void main() {
    serial_init();
    char vendor[13] = {0};
    unsigned char* video_mem = (unsigned char*)0xB8000;
    getCPUVendor(vendor);
    const char* line1 = " \\______   \\ ____   _______/  |_\\_____  \\  /   _____/ ";
    const char* line2 = "  |    |  _// __ \\ /  ___/\\   __\\/   |   \\ \\_____  \\  ";
    const char* line3 = "  |    |   \\  ___/ \\___ \\  |  | /    |    \\/        \\ ";
    const char* line4 = "  |______  /\\___  >____  > |__| \\_______  /_______  / ";
    const char* line5 = "         \\/     \\/     \\/               \\/        \\/  ";
    clearScreen(0x07);
    const char* ascii_art[5] = {line1, line2, line3, line4, line5};
    displayString("Your CPU Vendor is: ", GREEN_ON_BLACK);

    displayString(vendor, 0x0c);
    setCursor(10 * 80);
    for(int i = 0; i < 5; i++) {
        displayString(ascii_art[i], GREEN_ON_BLACK);
        displayString("\n", 0);
    }
    while (1) {
        char c = keyboard_getchar();
        if (c != 0) {
            serial_putchar(c);
            //displayString(&c, 0x0F);
        }
    }
}
void displayChar(int x,int y,char c,unsigned char color) {
    unsigned char* video_mem = (unsigned char*)0xB8000;
    video_mem[(y*80+x)*2] = c;
    video_mem[(y*80+x)*2+1] = color;
}

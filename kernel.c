#include "./drivers/vga.h"
#include "./drivers/serial.h"
#include "./drivers/keyboard.h"
#include "./drivers/ide.h"
#include "./cli/cli.h"
#include  "./diskAccess/diskAccess.h"
void getCPUVendor(char* vendor);
void main() {
    serial_init();
    initFileSystem();
    char vendor[13] = {0};
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
    displayString("<Testing VGA output>\n",0x0f);
    displayString("<Testing SERIAL output>\n",0x0f);
    serial_print("<Testing SERIAL output>\n");
    displayString("<Testing ATA/IDE Reading sector 0...>\n",0x0f);
    cliLoop();

}

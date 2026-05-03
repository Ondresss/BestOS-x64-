#include "pic.h"
#include "io.h"

void pic_remap() {
    // ICW1: Inicializace - začínáme sekvenci inicializace
    portByteOut(PIC1_COMMAND, 0x11);
    portByteOut(PIC2_COMMAND, 0x11);

    // ICW2: Offsety - kam se IRQ namapují v IDT
    portByteOut(PIC1_DATA, 0x20); // Master: IRQ 0-7 -> 0x20-0x27 (32-39)
    portByteOut(PIC2_DATA, 0x28); // Slave:  IRQ 8-15 -> 0x28-0x2F (40-47)

    // ICW3: Propojení - Master/Slave kaskáda
    portByteOut(PIC1_DATA, 4);    // Master má Slave na IRQ 2
    portByteOut(PIC2_DATA, 2);    // Slave je připojen na IRQ 2 Mastera

    // ICW4: Režim - 8086 režim
    portByteOut(PIC1_DATA, 0x01);
    portByteOut(PIC2_DATA, 0x01);

    /*
     * MASKY PŘERUŠENÍ
     * Bity: 76543210
     * 0 = Povoleno, 1 = Zakázáno
     */

    // Master: Povolíme IRQ0 (Timer) a IRQ1 (Keyboard)
    // 0xFC = 11111100b
    portByteOut(PIC2_DATA, 0x3F);

    // Master musíš taky povolit IRQ2 (cascade) jinak Slave vůbec neprojde!
    // 0xF8 = 11111000b → IRQ0(timer), IRQ1(kbd), IRQ2(cascade) povoleny
    portByteOut(PIC1_DATA, 0xF8);
}
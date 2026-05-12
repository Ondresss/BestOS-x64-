#include "keyboard.h"

#include <sys/types.h>
#define KBD_BUFFER_SIZE 256
static uint8_t kbd_buffer[KBD_BUFFER_SIZE];
static uint32_t kbd_head = 0;
static uint32_t kbd_tail = 0;
static unsigned char keyTable[] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
};

static unsigned char keyTableShift[] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0,
    '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' '
};
Event currentEvent = NONE_EV;

void serial_print_hex8(unsigned char n) {
    char const hex_chars[] = "0123456789ABCDEF";
    serial_putchar('0');
    serial_putchar('x');
    serial_putchar(hex_chars[(n >> 4) & 0x0F]);
    serial_putchar(hex_chars[n & 0x0F]);
    serial_putchar(' ');
}

Event keyboard_getEvent() {
    return currentEvent;
}


static int is_shift_pressed = 0;
static int is_ctrl_pressed = 0;
void kbd_enqueue(uint8_t scancode) {
    uint32_t next = (kbd_head + 1) % KBD_BUFFER_SIZE;
    if (next != kbd_tail) {
        kbd_buffer[kbd_head] = scancode;
        kbd_head = next;
    }
}

int kbd_dequeue() {
    if (kbd_head == kbd_tail) return -1; // Prázdno
    uint8_t scancode = kbd_buffer[kbd_tail];
    kbd_tail = (kbd_tail + 1) % KBD_BUFFER_SIZE;
    return (int)scancode;
}

uint32_t keyboard_callback(struct registers *regs) {
    uint8_t scancode = portByteIn(0x60);

    kbd_enqueue(scancode);

    return (uint32_t)regs;
}

char keyboard_getchar() {
    while (1) {
        int scancode = kbd_dequeue();

        if (scancode == -1) {
            asm volatile("hlt");
            continue;
        }

        currentEvent = NONE_EV;

        if (scancode == 0x1D) { is_ctrl_pressed = 1; continue; }
        if (scancode == 0x9D) { is_ctrl_pressed = 0; continue; }
        if (scancode == 0x2A || scancode == 0x36) { is_shift_pressed = 1; continue; }
        if (scancode == 0xAA || scancode == 0xB6) { is_shift_pressed = 0; continue; }

        if (scancode & 0x80) continue;

        if (scancode < sizeof(keyTable)) {
            char c = is_shift_pressed ? keyTableShift[scancode] : keyTable[scancode];
            if (is_ctrl_pressed && c == 'd') currentEvent = EOF_EV;
            if (c != 0) return c;
        }
    }
}

void init_keyboard() {
    register_interrupt_handler(33, keyboard_callback);
}
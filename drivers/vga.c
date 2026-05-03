#include "vga.h"
void checkForScrolling(int *offset) {
    int totalBytes = 25 * 160;
    if (*offset >= totalBytes) {
        unsigned char* video_mem = (unsigned char*)0xB8000;

        memoryCopy(video_mem, video_mem + 160, 3840);

        for (int i = 3840; i < 4000; i += 2) {
            video_mem[i] = ' ';
            video_mem[i + 1] = 0x07;
        }
        *offset = 3840;
    }
}

int getCursor() {
    portByteOut(VGA_CTRL_REGISTER,VGA_CUR_HIGH_REG);
    int offset = portByteIn(VGA_DATA_REGISTER) << 8;
    portByteOut(VGA_CTRL_REGISTER,VGA_CUR_LOW_REG);
    offset |= portByteIn(VGA_DATA_REGISTER);
    return offset * 2;
}
void setCharAtVideoMem(char c,int offset,unsigned char color) {
    unsigned char* mem = (unsigned char*)VIDEO_ADDRESS;
    mem[offset] = c;
    mem[offset+1] = color;
}
void displayString(char* str,unsigned char color) {

    int offset = getCursor();
    int i = 0;
    while (str[i] != '\0') {
        if (str[i] == '\n') {
            int row = offset / 160;
            offset = (row + 1) * 160;
            ++i;
            continue;
        }
        setCharAtVideoMem(str[i],offset,color);
        offset+=2;
        ++i;
    }
    checkForScrolling(&offset);
    setCursor(offset / 2);
}
void setCursor(int offset) {
    portByteOut(VGA_CTRL_REGISTER,VGA_CUR_HIGH_REG);
    portByteOut(VGA_DATA_REGISTER,(offset >> 8) & 0xFF);
    portByteOut(VGA_CTRL_REGISTER,VGA_CUR_LOW_REG);
    portByteOut(VGA_DATA_REGISTER,(offset & 0xFF));
}
void clearScreen(unsigned char color) {
    unsigned char* video_mem = (unsigned char*)0xB8000;

    for (int i = 0; i < 80 * 25; i++) {
        video_mem[i * 2] = ' ';
        video_mem[i * 2 + 1] = 0x07;
    }

    setCursor(0);
}
void clearLastCharacter(unsigned char color) {
    int currentOffset = getCursor();
    currentOffset-= 2;
    setCharAtVideoMem(' ',currentOffset,color);
    setCursor(currentOffset / 2);
}

void console_write(const char *str, unsigned int len) {
    int offset = getCursor();
    int i = 0;
    while (str[i] != '\0' && i < len) {
        if (str[i] == '\n') {
            int row = offset / 160;
            offset = (row + 1) * 160;
            ++i;
            continue;
        }
        setCharAtVideoMem(str[i],offset,WHITE_ON_BLACK);
        offset+=2;
        ++i;
    }
    checkForScrolling(&offset);
    setCursor(offset / 2);
}

void console_write_color(const char *str, unsigned int len,unsigned char color) {
    int offset = getCursor();
    int i = 0;
    while (str[i] != '\0' && i < len) {
        if (str[i] == '\n') {
            int row = offset / 160;
            offset = (row + 1) * 160;
            ++i;
            continue;
        }
        setCharAtVideoMem(str[i],offset,color);
        offset+=2;
        ++i;
    }
    checkForScrolling(&offset);
    setCursor(offset / 2);
}

void displayStringAt(char* str,unsigned char color,int x,int y) {
    int offset = (y * 80 + x) * 2;
    int i = 0;
    while (str[i] != '\0') {
        if (offset >= 4000) break;
        setCharAtVideoMem(str[i],offset,color);
        offset+=2;
        ++i;
    }
}
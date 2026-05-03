#pragma once
#include "../utils/strings.h"
#include "../arch/io.h"
#define VIDEO_ADDRESS 0xb8000
#define WHITE_ON_BLACK 0x0F
#define GREEN_ON_BLACK 0x0A
#define VGA_CTRL_REGISTER 0x3d4
#define VGA_DATA_REGISTER 0x3d5
#define VGA_CUR_LOW_REG 0x0f
#define VGA_CUR_HIGH_REG 0x0e
#define DARK_GREY 0x08
#define LIGHT_RED 0x0C
#define BLACK           0x00
#define BLUE            0x01
#define GREEN           0x02
#define CYAN            0x03
#define RED             0x04
#define MAGENTA         0x05
#define BROWN           0x06
#define LIGHT_GREY      0x07
#define DARK_GREY       0x08
#define LIGHT_BLUE      0x09
#define LIGHT_GREEN     0x0A
#define LIGHT_CYAN      0x0B
#define LIGHT_RED       0x0C
#define LIGHT_MAGENTA   0x0D
#define YELLOW          0x0E
#define WHITE           0x0F



int getCursor();
void setCharAtVideoMem(char c,int offset,unsigned char color);
void displayString(char* str,unsigned char color);
void setCursor(int offset);
void clearScreen(unsigned char color);
void clearLastCharacter(unsigned char color);
void displayStringAt(char* str,unsigned char color,int x,int y);
void console_write(const char *str, unsigned int len);
void console_write_color(const char *str, unsigned int len,unsigned char color);
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
int getCursor();
void setCharAtVideoMem(char c,int offset,unsigned char color);
void displayString(char* str,unsigned char color);
void setCursor(int offset);
void clearScreen(unsigned char color);
void clearLastCharacter(unsigned char color);
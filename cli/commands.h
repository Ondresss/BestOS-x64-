#pragma once
#include "../utils/strings.h"
#include "../drivers/vga.h"
#include "../drivers/keyboard.h"
#include "../drivers/ide.h"
#include "../drivers/serial.h"
typedef struct {
    char name[64];
    char params[16][64];
    int noParams;
}Command;

typedef struct {
    char buffer[513];
}CLIContext;

extern CLIContext cliContext;
void readCommand(Command* cmd);
void writeCommand(Command* cmd);
void helpCommand(Command* cmd);
void clearCommand(Command* cmd);
void loadCommand(Command* cmd);
void hexDumpCommand(Command* cmd);
void runCommand(Command* cmd);

static void printAddrInHex(unsigned int addr);
static void printHexByte(unsigned char b);
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
int cliReadline(char* buffer, int max_len);
int cliParse(char* line,Command* cmd);
void cliLoop();
void cliPrintCommand(Command* cmd);
void cliExecuteCommand(Command* cmd);
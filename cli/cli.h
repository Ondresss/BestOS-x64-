#pragma once
#include "commands.h"

int cliReadline(char* buffer, int max_len);
int cliParse(char* line,Command* cmd);
void cliLoop();
void cliPrintCommand(Command* cmd);
void cliExecuteCommand(Command* cmd);
#include "cli.h"

#include <strings.h>
CLIContext cliContext;
int cliReadline(char* buffer, int max_len) {
    int lineIndex = 0;
    char currentChar = 0;
    while (currentChar != '\n') {
        if (lineIndex >= max_len) {
            buffer[lineIndex-1] = '\0';
            return lineIndex;
        }
        currentChar = keyboard_getchar();
        if (currentChar != 0) {
            if (currentChar == '\b') {
                if (lineIndex > 0) {
                    clearLastCharacter(0x07);
                    lineIndex--;
                    buffer[lineIndex] = '\0';
                }
                continue;
            }
            char str[2] = {currentChar, '\0'};
            displayString(str, 0x0F);
            buffer[lineIndex++] = currentChar;
        }
    }
    buffer[lineIndex-1] = '\0';
    return lineIndex;
}

void cliLoop() {
    char BUFFER[128] = {0};
    Command currentCommand = {0};
    while (1) {
        memZero((char*)&currentCommand,sizeof(Command));
        memZero(BUFFER,1024);
        char currentDirBuf[256] = {0};
        getCurrentDir(currentDirBuf);
        displayString(currentDirBuf,GREEN_ON_BLACK);
        displayString("$ ",GREEN_ON_BLACK);
        cliReadline(BUFFER, 77);
        cliParse(BUFFER,&currentCommand);
        cliPrintCommand(&currentCommand);
        cliExecuteCommand(&currentCommand);

    }
}
int cliParse(char* line, Command* cmd) {
    char BUFFER[64] = {0};
    memZero((char*)cmd, sizeof(Command));
    int tokenInfo = stringSplit(BUFFER, line, ' ');
    if (tokenInfo == -1) return -1;
    stringCat(cmd->name, BUFFER);
    int paramIndex = 0;
    memZero(BUFFER,64);
    tokenInfo = stringSplit(BUFFER, line, ' ');
    while (tokenInfo != -1) {
        stringCat(cmd->params[paramIndex++], BUFFER);
        memZero(BUFFER,64);
        tokenInfo = stringSplit(BUFFER, line, ' ');
    }
    cmd->noParams = paramIndex;
    return paramIndex;
}

void cliPrintCommand(Command* cmd) {
    serial_print("Command name: ");
    serial_print(cmd->name);
    serial_print("\n");
    serial_print("Parameters: ");
    for (int i = 0; i < cmd->noParams; i++) {
        serial_print(cmd->params[i]);
        serial_print(",");
    }
    serial_print("\n");
    serial_print("No parameters: ");
    char NUM[124] = {0};
    unsignedIntToString(NUM,cmd->noParams);
    serial_print("\n");
    serial_print(NUM);
}

void cliExecuteCommand(Command* cmd) {
    if (!stringCompare(cmd->name,"")) return;
    if (!stringCompare(cmd->name,"clear")) {
        clearCommand(cmd);
    }
    else if (!stringCompare(cmd->name,"read")) {
        readCommand(cmd);
    } else if (!stringCompare(cmd->name,"wr")) {
        writeCommand(cmd);
    } else if (!stringCompare(cmd->name,"load")) {
        loadCommand(cmd);
    } else if (!stringCompare(cmd->name,"hexdump")) {
        hexDumpCommand(cmd);
    }else if (!stringCompare(cmd->name,"run")) {
        runCommand(cmd);
    } else if (!stringCompare(cmd->name,"help")) {
        helpCommand(cmd);
    }
    else if (!stringCompare(cmd->name,"cd")) {
        cdCommand(cmd);
    }
    else if (!stringCompare(cmd->name,"cat")) {
        catCommand(cmd);
    }else if (!stringCompare(cmd->name,"ls")) {
        lsCommand(cmd);
    }else if (!stringCompare(cmd->name,"tree")) {
        treeCommand(cmd);
    }else if (!stringCompare(cmd->name,"stat")) {
        statCommand(cmd);
    }else if (!stringCompare(cmd->name,"exec")) {
        execCommand(cmd);
    } else if (!stringCompare(cmd->name,"rm")) {
        rmCommand(cmd);
    } else if (!stringCompare(cmd->name,"write")) {
        writeFileCommand(cmd);
    } else if (!stringCompare(cmd->name,"touch")) {
        touchCommand(cmd);
    }
    else {
        displayString("UNRECOGNIZED COMMAND!\n",LIGHT_RED);
    }
}

void cliPrintCPUVendor() {
    while (true) {
        char BUFFER[128] = {0};
        getCPUVendor(BUFFER);
        displayStringAt("CPU Vendor: ",GREEN_ON_BLACK,0,0);
        displayStringAt(BUFFER,LIGHT_RED,15,0);
    }
}
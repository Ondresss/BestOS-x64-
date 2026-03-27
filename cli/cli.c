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
    char BUFFER[1024] = {0};
    Command currentCommand = {0};
    while (1) {
        memZero((char*)&currentCommand,sizeof(Command));
        memZero(BUFFER,1024);
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
        clearScreen(0x07);
    }
    else if (!stringCompare(cmd->name,"read")) {
        memZero((char*)&cliContext,sizeof(CLIContext));
        int lba = -1;
        if (stringFindChar(cmd->params[0],'x',true,0) != -1) {
            lba = stringToHex(cmd->params[0]);
        } else {
            lba = stringToInt(cmd->params[0]);
        }
        ideReadSector(lba, cliContext.buffer);
        displayString("DISK READ OK\n", WHITE_ON_BLACK);
        displayString("READ CONTENT: \n",WHITE_ON_BLACK);
        displayString(cliContext.buffer,DARK_GREY);
        displayString("\n",WHITE_ON_BLACK);
    } else if (!stringCompare(cmd->name,"write")) {
        int lba = -1;
        if (stringFindChar(cmd->params[0],'x',true,0) != -1) {
            lba = stringToHex(cmd->params[0]);
        } else {
            lba = stringToInt(cmd->params[0]);
        }
        ideWriteSector(lba, cliContext.buffer);
        displayString("WROTE TO THE SECTOR ", WHITE_ON_BLACK);
        displayString(cmd->params[0],WHITE_ON_BLACK);
        displayString("\n",WHITE_ON_BLACK);

    } else if (!stringCompare(cmd->name,"load")) {
        if (cmd->noParams < 2) {
            displayString("USAGE: load <lba> <address>\n", DARK_GREY);
            return;
        }
        unsigned int lba = stringToInt(cmd->params[0]);
        unsigned int raw_addr = stringToHex(cmd->params[1]);
        void* target_ptr = (void*)raw_addr;

        if (ideReadSector(lba, target_ptr) == 0) {
            displayString("Sector load to address: ", WHITE_ON_BLACK);
            displayString(cmd->params[1], 0x0A);
            displayString("\n", 0);
        } else {
            displayString("Error while reading from the disk!\n", 0x0C);
        }
    }
    else {
        displayString("UNRECOGNIZED COMMAND\n", LIGHT_RED);
    }

}
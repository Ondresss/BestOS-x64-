#include "commands.h"

void readCommand(Command* cmd) {
    memZero((char*)&cliContext,sizeof(CLIContext));
    int lba = -1;
    if (stringFindChar(cmd->params[0],'x',true,0) != -1) {
        lba = stringToHex(cmd->params[0]);
    } else {
        lba = stringToInt(cmd->params[0]);
    }
    ataReadSector(lba, cliContext.buffer);
    displayString("DISK READ OK\n", WHITE_ON_BLACK);
    displayString("READ CONTENT: \n",WHITE_ON_BLACK);
    displayString(cliContext.buffer,DARK_GREY);
    displayString("\n",WHITE_ON_BLACK);
}
void writeCommand(Command* cmd) {
    int lba = -1;
    if (stringFindChar(cmd->params[0],'x',true,0) != -1) {
        lba = stringToHex(cmd->params[0]);
    } else {
        lba = stringToInt(cmd->params[0]);
    }
    ataWriteSector(lba, cliContext.buffer);
    displayString("WROTE TO THE SECTOR ", WHITE_ON_BLACK);
    displayString(cmd->params[0],WHITE_ON_BLACK);
    displayString("\n",WHITE_ON_BLACK);
}
void helpCommand(Command* cmd);
void clearCommand(Command* cmd) {
    clearScreen(0x07);
}
void loadCommand(Command* cmd)  {
    if (cmd->noParams < 2) {
        displayString("USAGE: load <lba> <address>\n", DARK_GREY);
        return;
    }
    unsigned int lba = stringToInt(cmd->params[0]);
    unsigned int raw_addr = stringToHex(cmd->params[1]);
    void* target_ptr = (void*)raw_addr;

    if (ataReadSector(lba, target_ptr) == 0) {
        displayString("Sector load to address: ", WHITE_ON_BLACK);
        displayString(cmd->params[1], 0x0A);
        displayString("\n", 0);
    } else {
        displayString("Error while reading from the disk!\n", 0x0C);
    }
}

void printAddrInHex(unsigned int addr) {
    const char chars[] = "0123456789ABCDEF";
    for (int i = 7; i >= 0; i--) {
        char str[] = {chars[(addr >> (i * 4)) & 0x0F], '\0'};
        displayString(str, WHITE_ON_BLACK);
    }
}

void printHexByte(unsigned char b) {
    const char hex_chars[] = "010456789ABCDEF";
    char s[3];
    s[0] = hex_chars[(b >> 4) & 0x0F];
    s[1] = hex_chars[b & 0x0F];
    s[2] = '\0';
    displayString(s, 0x07);
}
void hexDumpCommand(Command* cmd) {
    if (cmd->noParams < 2) {
        displayString("Usage: hexdump <addr> <len>\n", 0x0C);
        return;
    }

    unsigned int addr = stringToHex(cmd->params[0]);
    unsigned int len = stringToInt(cmd->params[1]);
    unsigned char* ptr = (unsigned char*)addr;

    for (unsigned int i = 0; i < len; i += 16) {
        printAddrInHex(addr + i);
        displayString("  ", 0);

        for (int j = 0; j < 16; j++) {
            if (i + j < len) {
                printHexByte(ptr[i + j]);
                displayString(" ", 0);
            } else {
                displayString("   ", 0);
            }
            if (j == 7) displayString(" ", 0);
        }

        displayString(" |", 0x08);

        for (int j = 0; j < 16; j++) {
            if (i + j < len) {
                unsigned char c = ptr[i + j];
                if (c >= 32 && c <= 126) {
                    char s[2] = {c, '\0'};
                    displayString(s, 0x0A);
                } else {
                    displayString(".", 0x08);
                }
            }
        }
        displayString("|\n", 0x08);
    }
}

void runCommand(Command* cmd) {
    void (*f)(void);
    unsigned int addr = stringToHex(cmd->params[0]);
    f = (void (*)(void))addr;
    f();
}
void helpCommand(Command* cmd) {
    displayString("Commands: help, clear, read, write, load, run, hexdump\n", 0x07);
}

void catCommand(Command* cmd) {
    read_(cmd->params[0],0);
    displayString("\n",WHITE_ON_BLACK);
}
void cdCommand(Command* cmd) {
    changeDirAbsolute_(cmd->params[0]);
}

void lsCommand(Command* cmd) {
    list_();
    displayString("\n",WHITE_ON_BLACK);
}

void treeCommand(Command* cmd) {
    printTree();
}
void statCommand(Command* cmd) {
    stat_(cmd->params[0]);
}

void execCommand(Command* cmd) {
	Fat16Entry entry = findEntryInCurrentDir(cmd->params[0]);
    if (entry.filename[0] == 0 || (entry.attributes & 0x10)) {
        console_write_color("Error: Not a runnable file.\n", 28, 0x0C);
        return;
    }
    char* load_address = (char*)0x20000;
    read_(cmd->params[0], load_address);
    void (*program_entry)() = (void (*)())load_address;
    console_write_color("Executing... \n", 14, 0x0A);
    program_entry();
    console_write_color("\nProgram finished.\n", 18, 0x0A);
}

void rmCommand(Command* cmd) {
	delete_(cmd->params[0]);
	console_write("File deleted: ",13 );
	console_write(cmd->params[0],stringLength(cmd->params[0]));
	console_write("\n",1);
}

void writeFileCommand(Command* cmd) {
    write_(cmd->params[0],0,0);
}
void touchCommand(Command* cmd) {
    touch_(cmd->params[0]);
}
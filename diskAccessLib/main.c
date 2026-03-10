#include "diskAccess.h"

int main(void) {
    initFileSystem("./sd.img");
    changeDir("ADR1");
    //write_("AHOJ.txt");
    delete_("KOCKA.JPG");
    read_("KOREN.TXT");
    printTree();
    return 0;
}
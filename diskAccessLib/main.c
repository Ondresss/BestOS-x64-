#include "diskAccess.h"

int main(void) {
    initFileSystem("./sd.img");
    changeDir("ADR1");
    //write_("AHOJ.txt");
    //delete_("KOCKA.JPG");
    read_("KOCKA.JPG");
    printTree();
    return 0;
}
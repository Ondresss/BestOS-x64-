#include "diskAccess.h"

int main(void) {
    initFileSystem("./sd.img");
    //write_("HEJ.txt");
    printTree();
    read_("HEJ.txt");
    return 0;
}
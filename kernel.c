

void displayChar(char* mem,int x,int y,char c,char color) ;
void main() {
    char* video_mem = (char*)0xB8000;
    const char* line1 = " \\______   \\ ____   _______/  |_\\_____  \\  /   _____/ ";
    const char* line2 = "  |    |  _// __ \\ /  ___/\\   __\\/   |   \\ \\_____  \\  ";
    const char* line3 = "  |    |   \\  ___/ \\___ \\  |  | /    |    \\/        \\ ";
    const char* line4 = "  |______  /\\___  >____  > |__| \\_______  /_______  / ";
    const char* line5 = "         \\/     \\/     \\/               \\/        \\/  ";

    const char* ascii_art[5] = {line1, line2, line3, line4, line5};

    int i = 0;
    int j = 0;
    while (i < 5) {
        while (ascii_art[i][j] != '\0') {
            displayChar(video_mem,j,i+10,ascii_art[i][j],0x0f);
            ++j;
        }
        j = 0;
        ++i;
    }

    while (1);
}
void displayChar(char* mem,int x,int y,char c,char color) {
    mem[(y*80+x)*2] = c;
    mem[(y*80+x)*2+1] = color;
}
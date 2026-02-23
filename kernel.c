

void displayChar(char* mem,int x,int y,char c,char color) ;
void getCPUVendor(char* vendor);
void main() {
    char* video_mem = (char*)0xB8000;
    char vendor[13] = {0};
    getCPUVendor(vendor);
    const char* line1 = " \\______   \\ ____   _______/  |_\\_____  \\  /   _____/ ";
    const char* line2 = "  |    |  _// __ \\ /  ___/\\   __\\/   |   \\ \\_____  \\  ";
    const char* line3 = "  |    |   \\  ___/ \\___ \\  |  | /    |    \\/        \\ ";
    const char* line4 = "  |______  /\\___  >____  > |__| \\_______  /_______  / ";
    const char* line5 = "         \\/     \\/     \\/               \\/        \\/  ";

    const char* ascii_art[5] = {line1, line2, line3, line4, line5};
    const char* cpuMSG = "Your CPU Vendor is: ";
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
    i = 0;

    while (cpuMSG[i] != '\0') {
        displayChar(video_mem,i,20,cpuMSG[i],0x0c);
        ++i;
    }


    i = 0;
    while (vendor[i] != '\0') {
        displayChar(video_mem,i + 20,20,vendor[i],0x0f);
        ++i;
    }

    while (1);
}
void displayChar(char* mem,int x,int y,char c,char color) {
    mem[(y*80+x)*2] = c;
    mem[(y*80+x)*2+1] = color;
}
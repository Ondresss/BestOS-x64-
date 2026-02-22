

void main() {
    char* video_mem = (char*)0xB8000;
    const char* logo = "WELCOME TO BEST OS!";
    int i = 0;
    int memIndex = 0;
    while (logo[i] != '\0') {
        video_mem[20 + memIndex] = logo[i];
        video_mem[20 + memIndex+1] = 0x0f;
        ++i;
        memIndex+=2;
    }
    while (1);
}
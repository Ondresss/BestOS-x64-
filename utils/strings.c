#include "strings.h"

int stringLength(const char* str) {
    int i = 0;
    while (str[i] != '\0'){
        i++;
    }
    return i;
}


void memZero(char* dest,int len) {
    for (int i = 0; i < len; ++i) {
        dest[i] = 0;
    }
}



void formatFat16FileName(unsigned char* dest,const char* src) {
    int i = 0;
    while (src[i] != '.' && src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    int index = i;
    index++;
    while (i <= 7) {
        dest[i++] = ' ';
    }
    while (src[index] != '\0') {
        dest[i++] = src[index++];
    }
    while (i <= 11) {
        dest[i++] = ' ';
    }
}



void stringCat(char* str1,const char* str2) {
    int len = stringLength(str1);
    int len2 = stringLength(str2);
    int i = 0;
    for (i = 0; i < len2; ++i) {
        str1[i + len] = str2[i];
    }
    str1[i + len] = 0;
}

int unsignedIntToString(char* buf,unsigned int number){
    unsigned int cur = number;
    unsigned int i = 0;
    while (cur > 0) {
        buf[i++] = (cur % 10) + '0';
        cur = cur / 10;
    }
    for (int j = 0; j < i / 2; ++j) {
        char tmp = buf[j];
        buf[j] = buf[i - j - 1];
        buf[i - j - 1] = tmp;
    }
    buf[i] = 0;
    return i;
}

unsigned char stringCompare(const char* str1,const char* str2) {
    for (int i = 0; i <= stringLength(str1); ++i) {
        if (str1[i] != str2[i]) {
            return 1;
        }
    }
    return 0;
}

void stringFat16Format(char* buf,const unsigned char* filename,const unsigned char* ext) {
    int i = 0;
    int index = 0;
    while (i < 8) {
        if (filename[i] == ' ') {
            i++;
            continue;
        }
        buf[index++] = filename[i];
        i++;
    }
    if (ext[0] == ' ') {
        buf[index] = 0;
        return;
    }
    buf[index++] = '.';
    i = 0;
    while (i < 3) {
        if (ext[i] == ' ') {
            i++;
            continue;
        }
        buf[index++] = ext[i];
        i++;
    }
    buf[index] = 0;
}
int stringFindChar(const char* str,char c,bool direction,int skip) {
    int currentSkip = skip;
    if (direction) {
        for (int i = stringLength(str) - 1; i >= 0; --i) {
            if (str[i] == c) {
                if (currentSkip > 0 ) {
                    currentSkip--;
                    continue;
                } else {
                    return i;
                }
            }
        }
    } else {
        for (int i = 0; i < stringLength(str); ++i) {
            if (str[i] == c) {
                if (currentSkip > 0 ) {
                    currentSkip--;
                    continue;
                } else {
                    return i;
                }
            }
        }
    }
    return -1;
}

void memoryCopy(void *dest, const void *src, size_t n) {
    char *d = (char *)dest;
    const char *s = (const char *)src;

    for (size_t i = 0; i < n; i++) {
        d[i] = s[i];
    }

}

void stringParseFilename(char* BUFFER, const char* path) {
    int slashIndex = stringFindChar(path, '/', true, 0);

    if (slashIndex != -1 && path[slashIndex + 1] == '\0') {
        slashIndex = stringFindChar(path, '/', true, 1);
    }
    int i = (slashIndex == -1) ? 0 : slashIndex + 1;
    int bufferIndex = 0;

    while (path[i] != '\0' && path[i] != '/') {
        BUFFER[bufferIndex++] = path[i];
        i++;
    }
    BUFFER[bufferIndex] = '\0';
}

int stringSplit(char* dest, char* src, char sep) {
    static int nextTokenIndex = 0;
    int destIndex = 0;

    if (nextTokenIndex >= stringLength(src)) {
        nextTokenIndex = 0;
        return -1;
    }

    int i = nextTokenIndex;
    while (src[i] != '\0' && src[i] != sep) {
        dest[destIndex++] = src[i++];
    }

    dest[destIndex] = '\0';

    if (src[i] == '\0') {
        nextTokenIndex = i;
    } else {
        nextTokenIndex = i + 1;
    }

    return (destIndex > 0) ? nextTokenIndex : -1;
}

int stringToInt(char* buf) {
    int num = 0;
    int len = stringLength(buf);
    for (int i = 0; i < len; ++i) {
        if (buf[i] >= '0' && buf[i] <= '9') {
            num*=10;
            num+= buf[i] - '0';
        }
    }
    return num;
}

unsigned int stringToHex(char* str) {
    unsigned int val = 0;
    int i = 0;
    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) i = 2;

    while (str[i] != '\0') {
        unsigned int byte = str[i];
        if (byte >= '0' && byte <= '9') byte = byte - '0';
        else if (byte >= 'a' && byte <= 'f') byte = byte - 'a' + 10;
        else if (byte >= 'A' && byte <= 'F') byte = byte - 'A' + 10;
        else break;

        val = (val << 4) | (byte & 0xF);
        i++;
    }
    return val;
}

void *memset(void *ptr, int value, size_t num) {
    unsigned char *p = (unsigned char *)ptr;
    while (num--) {
        *p++ = (unsigned char)value;
    }
    return ptr;
}
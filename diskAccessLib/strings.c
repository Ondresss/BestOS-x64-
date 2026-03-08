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



void stringCat(char* str1,char* str2) {
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
    int i = 0;
    while (str1[i] != '\0' && str2[i] != '\0') {
        if (str1[i] != str2[i]) {
            return 1;
        }
        i++;
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
#pragma once
#include <stdio.h>
unsigned char stringCompare(const char* str1,const char* str2);
void stringFat16Format(char* buf,const unsigned char* filename,const unsigned char* ext);
int unsignedIntToString(char* buf,unsigned int number);
int stringLength(const char* str);
void stringCat(char* str1,char* str2);
void formatFat16FileName(unsigned char* dest,const char* src);
void memZero(char* dest,int len);
int stringFindChar(const char* str,char c,bool direction,int skip);

void memoryCopy(void *dest, const void *src, size_t n);
#include "fat.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
unsigned char stringCompare(const char* str1,const char* str2);
void stringFat16Format(char* buf,const unsigned char* filename,const unsigned char* ext);

void read_(const char* filename_);
void list_(const char* filename_);

static void readFat1Table(int fd, uint16_t* table, const Fat16BootSector* bs, uint32_t pos);
static int ataReadSector(int fd, uint32_t lba, uint8_t *buffer);
static void readFileContent(int fd,const uint16_t* fatTable, const Fat16Entry* entry, const Fat16BootSector* bs,unsigned  int dataAreaLBA);

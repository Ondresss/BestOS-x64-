#pragma once
#include "strings.h"
#include "fat.h"
#include <stdint.h>
#include <unistd.h>
typedef struct
{
    unsigned int day;
    unsigned int month;
    unsigned int year;
} Date;

typedef struct {
    char filename[256];
    unsigned int dirLBA;
}CurrentDir;

typedef struct {
    PartitionTable pt[4];
    Fat16BootSector bs;
    unsigned int rootDirLBA;
    unsigned int rootDirSectors;
    uint16_t fatTable[128 * 1024];
    int fd;
} FileSystem;

extern FileSystem fileSystem;
extern CurrentDir currentDir;

void console_write(const char *buf, uint32_t len);

int changeDirRecursive(const char* dirName,const Fat16Entry* dirEntry,int dataLBA);
Date parseDate(uint16_t date);
void readFat1Table(int fd, uint16_t* table, const Fat16BootSector* bs, uint32_t pos);
int ataReadSector(int fd, uint32_t lba, uint8_t *buffer);
void ataWriteSector(uint32_t lba, uint8_t *buffer);
void readFileContent(const Fat16Entry* entry, unsigned  int dataAreaLBA,char* BUFFER);
void printDirRecursive(const Fat16Entry* dirEntry,char* padding,int dataLBA);
int findFreeCluster();
void writeCluster(int clusterNumber,uint8_t* buffer);
void updateFatTable();
Fat16Entry findEntryAndErase(const char* fileName);
Fat16Entry findEntryAndEraseRecursive(const char* fileName,const Fat16Entry* entry);
void changeDirIterative(const char* targetName);
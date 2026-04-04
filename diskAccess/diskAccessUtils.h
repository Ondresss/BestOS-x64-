#pragma once
#include "../utils/strings.h"
#include "../drivers/ide.h"
#include  "../drivers/vga.h"
#include "fat.h"
#include  "../drivers/serial.h"

typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned char uint8_t;
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
} FileSystem;

extern FileSystem fileSystem;
extern CurrentDir currentDir;



int changeDirRecursive(const char* dirName,const Fat16Entry* dirEntry,int dataLBA);
Date parseDate(uint16_t date);
void readFat1Table(uint16_t* table, const Fat16BootSector* bs, uint32_t pos);
void readFileContent(const Fat16Entry* entry, unsigned  int dataAreaLBA,char* BUFFER);
void printDirRecursive(const Fat16Entry* dirEntry,char* padding,int dataLBA);
int findFreeCluster();
void writeCluster(int clusterNumber,uint8_t* buffer);
void updateFatTable();
Fat16Entry findEntryAndErase(const char* fileName);
Fat16Entry findEntryAndEraseRecursive(const char* fileName,const Fat16Entry* entry);
void changeDirIterative(const char* targetName);
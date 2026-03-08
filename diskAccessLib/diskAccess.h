#include "fat.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include "strings.h"
#pragma once

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


static FileSystem fileSystem;
static CurrentDir currentDir;

void console_write(const char *buf, uint32_t len);

void initFileSystem(const char* filename_);

void printTree();
void read_(const char* filename_);
void list_(const char* filename_);
void changeDir(const char* dirName);
void write_(const char* filename_);

static void changeDirRecursive(const char* dirName,const Fat16Entry* dirEntry,int dataLBA);
static Date parseDate(uint16_t date);
static void readFat1Table(int fd, uint16_t* table, const Fat16BootSector* bs, uint32_t pos);
static int ataReadSector(int fd, uint32_t lba, uint8_t *buffer);
static void ataWriteSector(uint32_t lba, uint8_t *buffer);
static void readFileContent(const Fat16Entry* entry, unsigned  int dataAreaLBA);
static void printDirRecursive(const Fat16Entry* dirEntry,char* padding,int dataLBA);
static int findFreeCluster();
static void writeCluster(int clusterNumber,uint8_t* buffer);
static void updateFatTable();
static void findFirstFreeEntry(const char* fileName, int startingCluster, uint32_t fileSize);
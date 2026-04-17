#include "fat.h"
#include "../utils/strings.h"
#include "diskAccessUtils.h"
#include "../drivers/keyboard.h"
#pragma once

void initFileSystem();

void printTree();
void read_(const char* filename_,char* BUFFER);
void list_();
void stat_(const char* filename_);
void changeDir(const char* dirName);
void write_(const char* filename_,const char* BUFF,size_t size);
void delete_(const char* filename_);
void changeDirAbsolute_(const char* absolutePath);
void create_(const char* filename_);
void rmCurrentDir_(const char* dirName_);
void touch_(const char* filename_);


int getCurrentEntries(Fat16Entry* arr);
Fat16Entry findEntryInCurrentDir(const char* path);
void getParentPath(const char *path, char *parent);
void findFirstFreeEntry(const char* fileName, int startingCluster, uint32_t fileSize);
Fat16Entry* findFirstFreeEntryInCurrentDir();
void getCurrentDir(char * buffer);




#include "diskAccessUtils.h"

void console_write(const char *buf, uint32_t len) {
    write(1,buf,len);
}
int changeDirRecursive(const char* dirName,const Fat16Entry* dirEntry,int dataLBA) {
    int startingDirCluster = dirEntry->starting_cluster;
    if (startingDirCluster == 0) return 0;

    uint8_t sectorBuffer[512];
    unsigned int clusterStart = dataLBA + (startingDirCluster - 2) * fileSystem.bs.sectors_per_cluster;

    for (int i = 0; i < fileSystem.bs.sectors_per_cluster; ++i) {
        ataReadSector(fileSystem.fd, clusterStart + i, sectorBuffer);

        for (int j = 0; j < 16; j++) {
            Fat16Entry* entryTmp = (Fat16Entry*)(sectorBuffer + j * sizeof(Fat16Entry));

            if (entryTmp->filename[0] == 0x00) continue;
            if (entryTmp->filename[0] == 0xE5) continue;

            if (entryTmp->filename[0] == '.') continue;

            char BUF[13] = {0};
            stringFat16Format(BUF, entryTmp->filename, entryTmp->ext);

            if (entryTmp->attributes & 0x10) {
                if (!stringCompare(BUF,dirName)) {
                    stringCat(currentDir.filename,BUF);
                    currentDir.dirLBA = dataLBA + (entryTmp->starting_cluster - 2) * fileSystem.bs.sectors_per_cluster;
                    return 1;
                } else {
                    if (changeDirRecursive(dirName,entryTmp,dataLBA) == 1) return 1;
                }

            }
        }
    }
    return 0;
}

Date parseDate(uint16_t date) {
    Date date_ = {0};
    date_.day = date & 0x001F;
    date_.month = (date >> 5) & 0x0F;
    date_.year = 1980 + ((date >> 9) & 0x7F);

    return date_;
}

void readFat1Table(int fd, uint16_t* table, const Fat16BootSector* bs, uint32_t pos) {
    uint8_t sectorBuffer[512];
    uint32_t fat1Start = pos + bs->reserved_sectors;

    uint8_t* tableBytePtr = (uint8_t*)table;

    for (uint32_t i = 0; i < bs->fat_size_sectors; ++i) {
        if (ataReadSector(fd, fat1Start + i, sectorBuffer) == 0) {
            for (int j = 0; j < 512; j++) {
                tableBytePtr[(i * 512) + j] = sectorBuffer[j];
            }
        }
    }
}


int ataReadSector(int fd, uint32_t lba, uint8_t *buffer) {
    unsigned long long offset = (unsigned long long)lba * 512;

    if (lseek(fd, offset, SEEK_SET) == -1) {
        return -1;
    }

    if (read(fd, buffer, 512) != 512) {
        return -1;
    }

    return 0;
}

void ataWriteSector(uint32_t lba, uint8_t *buffer) {
    lseek(fileSystem.fd, lba * 512, SEEK_SET);
    write(fileSystem.fd,buffer,512);
}


void readFileContent(const Fat16Entry* entry, unsigned int dataAreaLBA,char* BUFFER) {
    uint16_t currentCluster = entry->starting_cluster;
    uint8_t sectorBuffer[512] = {0};
    uint32_t remainingBytes = entry->file_size;

    while (currentCluster < 0xFFF8 && remainingBytes > 0) {
        unsigned int clusterStart = dataAreaLBA + (currentCluster - 2) * fileSystem.bs.sectors_per_cluster;

        for (int i = 0; i < fileSystem.bs.sectors_per_cluster && remainingBytes > 0; ++i) {
            ataReadSector(fileSystem.fd, clusterStart + i, sectorBuffer);
            uint32_t bytesToWrite = (remainingBytes > 512) ? 512 : remainingBytes;
            if(BUFFER) {
                memoryCopy(BUFFER,sectorBuffer,bytesToWrite);
                BUFFER+=bytesToWrite;
            } else
            {
                write(1,sectorBuffer,bytesToWrite);
            }

            remainingBytes -= bytesToWrite;
        }

        currentCluster = fileSystem.fatTable[currentCluster];
    }
}


void printDirRecursive(const Fat16Entry* dirEntry, char* padding, int dataLBA) {
    int startingDirCluster = dirEntry->starting_cluster;
    if (startingDirCluster == 0) return;

    uint8_t sectorBuffer[512];
    unsigned int clusterStart = dataLBA + (startingDirCluster - 2) * fileSystem.bs.sectors_per_cluster;

    for (int i = 0; i < fileSystem.bs.sectors_per_cluster; ++i) {
        ataReadSector(fileSystem.fd, clusterStart + i, sectorBuffer);

        for (int j = 0; j < 16; j++) {
            Fat16Entry* entryTmp = (Fat16Entry*)(sectorBuffer + j * sizeof(Fat16Entry));

            if (entryTmp->filename[0] == 0x00) return;
            if (entryTmp->filename[0] == 0xE5) continue;

            if (entryTmp->filename[0] == '.') continue;

            char BUF[13] = {0};
            stringFat16Format(BUF, entryTmp->filename, entryTmp->ext);

            console_write(padding, stringLength(padding));
            console_write("|-- ", 4);
            console_write(BUF, stringLength(BUF));

            if (entryTmp->attributes & 0x10) {
                console_write(" <DIR>\n", 7);

                char nextPadding[256];
                stringCat(nextPadding, "    ");

                printDirRecursive(entryTmp, nextPadding, dataLBA);
            } else {
                char filesizeStr[20];
                int fileLen = unsignedIntToString(filesizeStr, entryTmp->file_size);
                console_write(" (", 2);
                console_write(filesizeStr, fileLen);
                console_write(" bytes)\n", 8);
            }
        }
    }
}

int findFreeCluster() {
    int maxFatIndex = (fileSystem.bs.fat_size_sectors * 512) / 2;
    for (int i = 2; i < maxFatIndex; ++i) {
        if (fileSystem.fatTable[i] == 0) return i;
    }
    return -1;
}

void writeCluster(int clusterNumber, uint8_t* buffer) {
    int dataLBA = fileSystem.rootDirLBA + fileSystem.rootDirSectors;
    int startToWriteCluster = dataLBA + (clusterNumber - 2) * fileSystem.bs.sectors_per_cluster;
    ataWriteSector(startToWriteCluster, buffer);
}

void updateFatTable() {
    uint32_t fat1Start = fileSystem.pt[0].start_sector + fileSystem.bs.reserved_sectors;
    uint8_t* fatPtr = (uint8_t*)fileSystem.fatTable;

    for (int i = 0; i < fileSystem.bs.fat_size_sectors; ++i) {
        ataWriteSector(fat1Start + i, fatPtr + (i * 512));
        uint32_t fat2Start = fat1Start + fileSystem.bs.fat_size_sectors;
        ataWriteSector(fat2Start + i, fatPtr + (i * 512));
    }
}

Fat16Entry findEntryAndErase(const char* fileName) {
    uint8_t sectorBuffer[512];
    Fat16Entry entryCopy;
    memZero((char*)&entryCopy,sizeof(entryCopy));

    if (currentDir.dirLBA != -1) {

        for (int i = 0; i < fileSystem.bs.sectors_per_cluster; ++i) {
            ataReadSector(fileSystem.fd, currentDir.dirLBA + i, sectorBuffer);

            for (int j = 0; j < 16; j++) {
                Fat16Entry* entryTmp = (Fat16Entry*)(sectorBuffer + j * sizeof(Fat16Entry));

                if (entryTmp->filename[0] == 0x00) return entryCopy ;
                if (entryTmp->filename[0] == 0xE5) continue;

                if (entryTmp->filename[0] == '.') continue;

                char BUF[13] = {0};
                stringFat16Format(BUF, entryTmp->filename, entryTmp->ext);
                if (!stringCompare(BUF, fileName)) {
                    entryCopy = *entryTmp;
                    entryTmp->filename[0] = 0xE5;
                    ataWriteSector(currentDir.dirLBA + i, sectorBuffer);
                    return entryCopy;
                }

            }
        }

        return entryCopy;
    }


    for (unsigned int s = 0; s < fileSystem.rootDirSectors; s++) {
        ataReadSector(fileSystem.fd,fileSystem.rootDirLBA + s, sectorBuffer);
        int noOfEntries = 512 / sizeof(Fat16Entry);

        for (int i = 0; i < noOfEntries; i++) {
            Fat16Entry* entry = (Fat16Entry*)(sectorBuffer + i * sizeof(Fat16Entry));
            if (entry->filename[0] == 0x00 || (uint8_t)entry->filename[0] == 0xE5) continue;
            if (entry->attributes & 0x10) {
                entryCopy = findEntryAndEraseRecursive(fileName, entry);
                if (entryCopy.filename[0] != 0) return entryCopy;
            }
            char FILENAME[13] = {0};
            stringFat16Format(FILENAME,entry->filename,entry->ext);
            if (!stringCompare(FILENAME, fileName)) {
                entryCopy = *entry;
                entry->filename[0] = 0xE5;
                ataWriteSector(fileSystem.rootDirLBA + s, sectorBuffer);
                return entryCopy;
            }
        }
    }
    return entryCopy;
}

Fat16Entry findEntryAndEraseRecursive(const char* fileName,const Fat16Entry* entry) {
    int startingDirCluster = entry->starting_cluster;
    uint8_t sectorBuffer[512];
    unsigned int dataLBA = fileSystem.rootDirLBA + fileSystem.rootDirSectors;
    unsigned int clusterStart = dataLBA + (startingDirCluster - 2) * fileSystem.bs.sectors_per_cluster;
    Fat16Entry entryCopy;
    memZero((char*)&entryCopy,sizeof(entryCopy));
    for (int i = 0; i < fileSystem.bs.sectors_per_cluster; ++i) {
        ataReadSector(fileSystem.fd, clusterStart + i, sectorBuffer);

        for (int j = 0; j < 16; j++) {
            Fat16Entry* entryTmp = (Fat16Entry*)(sectorBuffer + j * sizeof(Fat16Entry));

            if (entryTmp->filename[0] == 0x00) return entryCopy ;
            if (entryTmp->filename[0] == 0xE5) continue;

            if (entryTmp->filename[0] == '.') continue;

            char BUF[13] = {0};
            stringFat16Format(BUF, entryTmp->filename, entryTmp->ext);
            if (entryTmp->attributes & 0x10) {
                entryCopy = findEntryAndEraseRecursive(fileName, entryTmp);
                if (entryCopy.filename[0] != 0) {
                    return entryCopy;
                };
            }
            if (!stringCompare(BUF, fileName)) {
                entryCopy = *entryTmp;
                entryTmp->filename[0] = 0xE5;
                ataWriteSector(clusterStart + i, sectorBuffer);
                return entryCopy;
            }

        }
    }
    return entryCopy;
}

void changeDirIterative(const char* targetName) {
    uint8_t sectorBuffer[512];
    uint32_t startLBA;
    uint32_t sectorsToRead;

    if (currentDir.dirLBA == -1) {
        startLBA = fileSystem.rootDirLBA;
        sectorsToRead = fileSystem.rootDirSectors;
    } else {
        startLBA = currentDir.dirLBA;
        sectorsToRead = fileSystem.bs.sectors_per_cluster;
    }

    for (uint32_t s = 0; s < sectorsToRead; s++) {
        ataReadSector(fileSystem.fd, startLBA + s, sectorBuffer);
        for (int i = 0; i < 16; i++) {
            Fat16Entry* entry = (Fat16Entry*)(sectorBuffer + i * sizeof(Fat16Entry));

            if (entry->filename[0] == 0x00) return;
            if ((uint8_t)entry->filename[0] == 0xE5) continue;

            char currentName[13];
            stringFat16Format(currentName, entry->filename, entry->ext);

            if (!stringCompare(currentName, targetName) && (entry->attributes & 0x10)) {
                unsigned int dataLBA = fileSystem.rootDirLBA + fileSystem.rootDirSectors;
                currentDir.dirLBA = dataLBA + (entry->starting_cluster - 2) * fileSystem.bs.sectors_per_cluster;
                stringCat(currentDir.filename, targetName);
                stringCat(currentDir.filename, "/");
                return;
            }
        }
    }
}
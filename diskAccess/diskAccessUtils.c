#include "diskAccessUtils.h"


int changeDirRecursive(const char* dirName,const Fat16Entry* dirEntry,int dataLBA) {
    int startingDirCluster = dirEntry->starting_cluster;
    if (startingDirCluster == 0) return 0;

    uint8_t sectorBuffer[512];
    unsigned int clusterStart = dataLBA + (startingDirCluster - 2) * fileSystem.bs.sectors_per_cluster;

    for (int i = 0; i < fileSystem.bs.sectors_per_cluster; ++i) {
        ataReadSector(clusterStart + i, sectorBuffer);

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

void readFat1Table(uint16_t* table, const Fat16BootSector* bs, uint32_t pos) {
    uint8_t sectorBuffer[512];
    uint32_t fat1Start = pos + bs->reserved_sectors;

    uint8_t* tableBytePtr = (uint8_t*)table;

    for (uint32_t i = 0; i < bs->fat_size_sectors; ++i) {
        if (ataReadSector(fat1Start + i, sectorBuffer) == 0) {
            for (int j = 0; j < 512; j++) {
                tableBytePtr[(i * 512) + j] = sectorBuffer[j];
            }
        }
    }
}


void readFileContent(const Fat16Entry* entry, unsigned int dataAreaLBA,char* BUFFER) {
    uint16_t currentCluster = entry->starting_cluster;
    char NAME[124] = {0};
    stringFat16Format(NAME,entry->filename,entry->ext);
    serial_print(NAME);
    uint8_t sectorBuffer[512] = {0};
    uint32_t remainingBytes = entry->file_size;
    while (currentCluster < 0xFFF8 && remainingBytes > 0) {
        unsigned int clusterStart = dataAreaLBA + (currentCluster - 2) * fileSystem.bs.sectors_per_cluster;

        for (int i = 0; i < fileSystem.bs.sectors_per_cluster && remainingBytes > 0; ++i) {
            ataReadSector(clusterStart + i, sectorBuffer);
            uint32_t bytesToWrite = (remainingBytes > 512) ? 512 : remainingBytes;
            if(BUFFER) {
                memoryCopy(BUFFER,sectorBuffer,bytesToWrite);
                BUFFER+=bytesToWrite;
            } else
            {
                console_write((char*)sectorBuffer,bytesToWrite);
            }

            remainingBytes -= bytesToWrite;
        }

        currentCluster = fileSystem.fatTable[currentCluster];
    }
}


void printDirRecursive(const Fat16Entry* dirEntry, char* padding, int dataLBA) {
    uint16_t startingDirCluster = dirEntry->starting_cluster;
    if (startingDirCluster < 2) return;

    uint8_t sectorBuffer[512];
    unsigned int clusterStart = dataLBA + (startingDirCluster - 2) * fileSystem.bs.sectors_per_cluster;

    for (int i = 0; i < fileSystem.bs.sectors_per_cluster; ++i) {
        if (ataReadSector(clusterStart + i, sectorBuffer) != 0) break;

        for (int j = 0; j < 16; j++) {
            Fat16Entry* entryTmp = (Fat16Entry*)(sectorBuffer + j * sizeof(Fat16Entry));

            if (entryTmp->filename[0] == 0x00) return;
            if ((uint8_t)entryTmp->filename[0] == 0xE5) continue;
            if (entryTmp->filename[0] == '.') continue;

            char BUF[13] = {0};
            stringFat16Format(BUF, entryTmp->filename, entryTmp->ext);

            console_write_color(padding, stringLength(padding), 0x08);
            console_write_color("|-- ", 4, 0x08);

            if (entryTmp->attributes & 0x10) {
                console_write_color("<DIR> ", 6, 0x0E);
                console_write_color(BUF, stringLength(BUF), 0x0B);
                console_write_color("\n", 1, 0x0F);

                char nextPadding[64] = {0};
                stringCat(nextPadding, padding);
                stringCat(nextPadding, "|   ");

                printDirRecursive(entryTmp, nextPadding, dataLBA);
            } else {
                console_write_color(BUF, stringLength(BUF), 0x0F);

                char filesizeStr[20] = {0};
                int fileLen = unsignedIntToString(filesizeStr, entryTmp->file_size);
                console_write_color(" (", 2, 0x07);
                console_write_color(filesizeStr, fileLen, 0x0A);
                console_write_color(" B)\n", 4, 0x07);
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
            ataReadSector(currentDir.dirLBA + i, sectorBuffer);

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
        ataReadSector(fileSystem.rootDirLBA + s, sectorBuffer);
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
        ataReadSector(clusterStart + i, sectorBuffer);

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
        ataReadSector(startLBA + s, sectorBuffer);
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
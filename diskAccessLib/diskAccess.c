#include "diskAccess.h"

extern CurrentDir currentDir;
extern FileSystem fileSystem;


void console_write(const char *buf, uint32_t len) {
  write(1,buf,len);
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

void readFileContent(const Fat16Entry* entry, unsigned  int dataAreaLBA) {
  uint16_t currentCluster = entry->starting_cluster;
  uint8_t sectorBuffer[512] = {0};
  while (currentCluster < 0xFFF8) {
    unsigned int clusterStart = dataAreaLBA + (currentCluster - 2) * fileSystem.bs.sectors_per_cluster;
    for (int i = 0; i < fileSystem.bs.sectors_per_cluster; ++i) {
        ataReadSector(fileSystem.fd,clusterStart + i, sectorBuffer);
        write(1,sectorBuffer,512);
    }
    currentCluster = fileSystem.fatTable[currentCluster];
  }
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

void initFileSystem(const char* filename_) {
  uint16_t fatTable[128 * 1024] = {0};
  const int fd = open(filename_,O_RDWR);
  fileSystem.fd = fd;
  uint8_t sectorBuffer[512] = {0};
  ataReadSector(fileSystem.fd,0,sectorBuffer);
  PartitionTable* ptTmp = (PartitionTable*)(sectorBuffer + 0x1BE);
  fileSystem.pt[0] = *ptTmp;
  ataReadSector(fileSystem.fd,fileSystem.pt[0].start_sector,sectorBuffer);
  Fat16BootSector* bsTmp = (Fat16BootSector*)sectorBuffer;
  fileSystem.bs = *bsTmp;
  readFat1Table(fileSystem.fd,fatTable,&fileSystem.bs,fileSystem.pt[0].start_sector);
  unsigned int rootDirLBA = (fileSystem.pt[0].start_sector + fileSystem.bs.reserved_sectors) + fileSystem.bs.number_of_fats * fileSystem.bs.fat_size_sectors;
  unsigned int rootDirSectors = (fileSystem.bs.root_dir_entries * sizeof(Fat16Entry)) / 512;
  fileSystem.rootDirLBA = rootDirLBA;
  fileSystem.rootDirSectors = rootDirSectors;
  readFat1Table(fileSystem.fd,fileSystem.fatTable,&fileSystem.bs,fileSystem.pt[0].start_sector);
}

void changeDirRecursive(const char* dirName,const Fat16Entry* dirEntry,int dataLBA) {
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

      if (entryTmp->attributes & 0x10) {
          if (!stringCompare(BUF,dirName)) {
              currentDir.dirLBA = clusterStart;
              stringCat(currentDir.filename,BUF);
          } else {
              changeDirRecursive(dirName,entryTmp,dataLBA);
          }

      }
    }
  }
}

void changeDir(const char* dirName) {
  uint8_t sectorBuffer[512] = {0};
  for (unsigned int s = 0; s < fileSystem.rootDirSectors; s++) {
    ataReadSector(fileSystem.fd, fileSystem.rootDirLBA + s, sectorBuffer);

    int noOfEntries = 512 / sizeof(Fat16Entry);

    for (int i = 0; i < noOfEntries; i++) {
      Fat16Entry* entryTmp = (Fat16Entry*)(sectorBuffer + i * sizeof(Fat16Entry));

      if (entryTmp->filename[0] == 0x00) return;
      if (entryTmp->filename[0] == 0xE5) continue;

      char BUF[255] = {0};
      stringFat16Format(BUF,entryTmp->filename,entryTmp->ext);

      if (entryTmp->attributes & 0x10) {
          int dataLBA = fileSystem.rootDirLBA + fileSystem.rootDirSectors;
          changeDirRecursive(dirName,entryTmp,dataLBA);
      }

    }
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

void printTree() {
  uint8_t sectorBuffer[512] = {0};
  for (unsigned int s = 0; s < fileSystem.rootDirSectors; s++) {
    ataReadSector(fileSystem.fd, fileSystem.rootDirLBA + s, sectorBuffer);

    int noOfEntries = 512 / sizeof(Fat16Entry);

    for (int i = 0; i < noOfEntries; i++) {
      Fat16Entry* entryTmp = (Fat16Entry*)(sectorBuffer + i * sizeof(Fat16Entry));

      if (entryTmp->filename[0] == 0x00) return;
      if (entryTmp->filename[0] == 0xE5) continue;

      char BUF[255] = {0};
      stringFat16Format(BUF,entryTmp->filename,entryTmp->ext);
      Date date = parseDate(entryTmp->modify_date);
      char day[10], month[10], year[10], filesize[20];
      char space = ' ';
      char dot = '.';
      char zero = '0';

      if (date.day < 10) console_write(&zero, 1);
      int dayLen = unsignedIntToString(day, date.day);
      console_write(day, dayLen);
      console_write(&dot, 1);

      if (date.month < 10) console_write(&zero, 1);
      int monthLen = unsignedIntToString(month, date.month);
      console_write(month, monthLen);
      console_write(&dot, 1);

      int yearLen = unsignedIntToString(year, date.year);
      console_write(year, yearLen);
      console_write("  ", 2);

      if (entryTmp->attributes & 0x10) {
        console_write("<DIR>          ", 15);
        char PADDING[1024 * 128] = {0};
        stringCat(PADDING,"                                    ");
        int dataStartLBA = fileSystem.rootDirLBA + fileSystem.rootDirSectors;
        char filesize[200] = {0};
        unsigned int fileLen = unsignedIntToString(filesize, entryTmp->file_size);
        console_write(filesize, fileLen);
        console_write("  ", 2);
        console_write(BUF, stringLength(BUF));
        console_write("\n", 1);
        printDirRecursive(entryTmp,PADDING,dataStartLBA);
      } else {
        int fileLen = unsignedIntToString(filesize, entryTmp->file_size);
        int padding = 15 - fileLen;
        for (int p = 0; p < padding; p++) console_write(&space, 1);
        console_write(filesize, fileLen);
        console_write("  ", 2);
        console_write(BUF, stringLength(BUF));
        console_write("\n", 1);
      }

    }
  }

}
void list_(const char* filename_) {
  uint8_t sectorBuffer[512] = {0};
  for (unsigned int s = 0; s < fileSystem.rootDirSectors; s++) {
    ataReadSector(fileSystem.fd, fileSystem.rootDirLBA + s, sectorBuffer);

    int noOfEntries = 512 / sizeof(Fat16Entry);

    for (int i = 0; i < noOfEntries; i++) {
      Fat16Entry* entryTmp = (Fat16Entry*)(sectorBuffer + i * sizeof(Fat16Entry));

      if (entryTmp->filename[0] == 0x00) return;
      if (entryTmp->filename[0] == 0xE5) continue;

      char BUF[255] = {0};
      stringFat16Format(BUF,entryTmp->filename,entryTmp->ext);
      Date date = parseDate(entryTmp->modify_date);
      char day[10], month[10], year[10], filesize[20];
      char space = ' ';
      char dot = '.';
      char zero = '0';

      if (date.day < 10) console_write(&zero, 1);
      int dayLen = unsignedIntToString(day, date.day);
      console_write(day, dayLen);
      console_write(&dot, 1);

      if (date.month < 10) console_write(&zero, 1);
      int monthLen = unsignedIntToString(month, date.month);
      console_write(month, monthLen);
      console_write(&dot, 1);

      int yearLen = unsignedIntToString(year, date.year);
      console_write(year, yearLen);
      console_write("  ", 2);

      if (entryTmp->attributes & 0x10) {
        console_write("<DIR>          ", 15);
      } else {
        int fileLen = unsignedIntToString(filesize, entryTmp->file_size);
        int padding = 15 - fileLen;
        for (int p = 0; p < padding; p++) console_write(&space, 1);
        console_write(filesize, fileLen);
      }

      console_write("  ", 2);
      console_write(BUF, stringLength(BUF));
      console_write("\n", 1);
    }
  }
}

Date parseDate(uint16_t date) {
  Date date_ = {0};
  date_.day = date & 0x001F;
  date_.month = (date >> 5) & 0x0F;
  date_.year = 1980 + ((date >> 9) & 0x7F);

  return date_;
}


void ataWriteSector(uint32_t lba, uint8_t *buffer) {
  lseek(fileSystem.fd, lba * 512, SEEK_SET);
  write(fileSystem.fd,buffer,512);
}


void writeCluster(int clusterNumber, uint8_t* buffer) {
  int dataLBA = fileSystem.rootDirLBA + fileSystem.rootDirSectors;
  int startToWriteCluster = dataLBA + (clusterNumber - 2) * fileSystem.bs.sectors_per_cluster;
  ataWriteSector(startToWriteCluster, buffer);
}


int findFreeCluster() {
  int maxFatIndex = (fileSystem.bs.fat_size_sectors * 512) / 2;
  for (int i = 2; i < maxFatIndex; ++i) {
    if (fileSystem.fatTable[i] == 0) return i;
  }
  return -1;
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


void findFirstFreeEntry(const char* fileName, int startingCluster, uint32_t fileSize) {
  uint8_t sectorBuffer[512];

  for (unsigned int s = 0; s < fileSystem.rootDirSectors; s++) {
    ataReadSector(fileSystem.fd,fileSystem.rootDirLBA + s, sectorBuffer);
    int noOfEntries = 512 / sizeof(Fat16Entry);

    for (int i = 0; i < noOfEntries; i++) {
      Fat16Entry* entry = (Fat16Entry*)(sectorBuffer + i * sizeof(Fat16Entry));
      if (entry->filename[0] == 0x00 || (uint8_t)entry->filename[0] == 0xE5) {

        formatFat16FileName(entry->filename,fileName);

        entry->attributes = 0x20;
        entry->starting_cluster = (uint16_t)startingCluster;
        entry->file_size = fileSize;

        ataWriteSector(fileSystem.rootDirLBA + s, sectorBuffer);

        return;
      }
    }
  }
}



void write_(const char* filename_) {
  uint8_t BUFFER[512];
  int prevClusterIndex = -1;
  int startingCluster = -1;
  uint32_t totalBytesRead = 0;

  while (1) {
    memZero((char*)BUFFER,512);
    int bytesRead = (int)read(0, BUFFER, 512);
    if (bytesRead <= 0) break;

    int current = findFreeCluster();
    if (current == -1) {
      console_write("Disk full\n", 10);
      break;
    }
    fileSystem.fatTable[current] = 0xFFFF;

    if (startingCluster == -1) {
      startingCluster = current;
    }

    if (prevClusterIndex != -1) {
      fileSystem.fatTable[prevClusterIndex] = (uint16_t)current;
    }

    writeCluster(current, BUFFER);
    prevClusterIndex = current;
    totalBytesRead += bytesRead;
  }

  if (startingCluster != -1) {
    updateFatTable();
    findFirstFreeEntry(filename_, startingCluster, totalBytesRead);
  }
}

void read_(const char* filename_) {
  uint8_t sectorBuffer[512] = {0};
  for (int s = 0; s < fileSystem.rootDirSectors; s++) {
    ataReadSector(fileSystem.fd, fileSystem.rootDirLBA + s, sectorBuffer);

    int noOfEntries = 512 / sizeof(Fat16Entry);

    for (int i = 0; i < noOfEntries; i++) {
      Fat16Entry* entryTmp = (Fat16Entry*)(sectorBuffer + i * sizeof(Fat16Entry));

      if (entryTmp->filename[0] == 0x00) return;
      if (entryTmp->filename[0] == 0xE5) continue;

      char BUF[255] = {0};
      stringFat16Format(BUF,entryTmp->filename,entryTmp->ext);

      if (!stringCompare(BUF, filename_)) {
         int dataStartLBA = fileSystem.rootDirLBA + fileSystem.rootDirSectors;
         readFileContent(entryTmp,dataStartLBA);
      }
    }
  }
}

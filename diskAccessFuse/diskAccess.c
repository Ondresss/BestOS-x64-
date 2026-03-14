#include "diskAccess.h"

#include <string.h>

CurrentDir currentDir;
FileSystem fileSystem;



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

  currentDir.dirLBA = -1;
  memZero(currentDir.filename,256);
}


void changeDir(const char* dirName) {
  if (!stringCompare(dirName,"/")) {
      currentDir.dirLBA = -1;
      char root[] = "/";
      stringCat(currentDir.filename,root);
      return;
  }
  uint8_t sectorBuffer[512] = {0};
  for (unsigned int s = 0; s < fileSystem.rootDirSectors; s++) {
    ataReadSector(fileSystem.fd, fileSystem.rootDirLBA + s, sectorBuffer);

    int noOfEntries = 512 / sizeof(Fat16Entry);

    for (int i = 0; i < noOfEntries; i++) {
      Fat16Entry* entryTmp = (Fat16Entry*)(sectorBuffer + i * sizeof(Fat16Entry));

      if (entryTmp->filename[0] == 0x00) continue;
      if (entryTmp->filename[0] == 0xE5) continue;

      char BUF[255] = {0};
      stringFat16Format(BUF,entryTmp->filename,entryTmp->ext);
      unsigned int dataLBA = fileSystem.rootDirLBA + fileSystem.rootDirSectors;
      if (entryTmp->attributes & 0x10) {
          if (!stringCompare(BUF,dirName)) {
            stringCat(currentDir.filename,BUF);
            currentDir.dirLBA = dataLBA + (entryTmp->starting_cluster - 2) * fileSystem.bs.sectors_per_cluster;
            return;
          } else {
            if (changeDirRecursive(dirName,entryTmp,dataLBA) == 1) return;
          }
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

void write_(const char* filename_,const char* BUFF,size_t size) {
  uint8_t BUFFER[512];
  int prevClusterIndex = -1;
  int startingCluster = -1;
  uint32_t totalBytesRead = 0;
  uint32_t remainingBytes = BUFF ? size : 0;
  while (1) {
    memZero((char*)BUFFER,512);
    int bytesRead = 0;
    if (BUFF) {
      if (remainingBytes <= 0) break;
      bytesRead = remainingBytes > 512 ? 512 : remainingBytes;
      memcpy(BUFFER,BUFF,bytesRead);
      BUFF += bytesRead;
      remainingBytes -= bytesRead;
    } else
    {
      bytesRead = (int)read(0, BUFFER, 512);
    }
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

void read_(const char* filename_,char* BUFFER) {
  uint8_t sectorBuffer[512] = {0};
  if (currentDir.dirLBA != -1) {
    for (int i = 0; i < fileSystem.bs.sectors_per_cluster; ++i) {
      ataReadSector(fileSystem.fd, currentDir.dirLBA + i, sectorBuffer);

      for (int j = 0; j < 16; j++) {
        Fat16Entry* entry = (Fat16Entry*)(sectorBuffer + j * sizeof(Fat16Entry));
        if (entry->filename[0] == 0x00) continue;
        if (entry->filename[0] == 0xE5) continue;

        char BUF[255] = {0};
        stringFat16Format(BUF,entry->filename,entry->ext);

        if (!stringCompare(BUF, filename_)) {
          int dataStartLBA = fileSystem.rootDirLBA + fileSystem.rootDirSectors;
          readFileContent(entry,dataStartLBA,BUFFER);
        }
      }
    }
    return;
  }
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
         readFileContent(entryTmp,dataStartLBA,BUFFER);
      }
    }
  }
}


void findFirstFreeEntry(const char* fileName, int startingCluster, uint32_t fileSize) {
  delete_(fileName);
  uint8_t sectorBuffer[512] = {0};
  uint32_t startLBA = 0;
  uint32_t sectorsToRead = 0;

  if (currentDir.dirLBA != -1) {
    startLBA = currentDir.dirLBA;
    sectorsToRead = fileSystem.bs.sectors_per_cluster;
  } else {
    startLBA = fileSystem.rootDirLBA;
    sectorsToRead = fileSystem.rootDirSectors;
  }

  for (uint32_t s = 0; s < sectorsToRead; s++) {
    if (ataReadSector(fileSystem.fd, startLBA + s, sectorBuffer) != 0) continue;

    for (int j = 0; j < 16; j++) {
      Fat16Entry* entry = (Fat16Entry*)(sectorBuffer + j * sizeof(Fat16Entry));

      if (entry->filename[0] == 0x00 || (uint8_t)entry->filename[0] == 0xE5) {
        formatFat16FileName(entry->filename, fileName);
        entry->attributes = 0x20;
        entry->starting_cluster = (uint16_t)startingCluster;
        entry->file_size = fileSize;

        ataWriteSector(startLBA + s, sectorBuffer);
        return;
      }
    }
  }

  console_write("Error: No free directory slot found!\n", 37);
}

void delete_(const char* filename_) {
  Fat16Entry entry = findEntryAndErase(filename_);
  if (entry.filename[0] == 0x00 || (uint8_t)entry.filename[0] == 0xE5) {
    return;
  }

  uint32_t currentCluster = entry.starting_cluster;
  while (currentCluster >= 2 && currentCluster < 0xFFF8) {
    uint32_t next = fileSystem.fatTable[currentCluster];
    fileSystem.fatTable[currentCluster] = 0x00;
    currentCluster = next;
  }
  updateFatTable();
}


void changeDirAbsolute_(const char* absolutePath) {
  char currentDirBuf[256] = {0};
  int currentDirIndex = 0;
  unsigned int len = stringLength(absolutePath);

  currentDir.dirLBA = -1;
  memZero(currentDir.filename, 256);
  stringCat(currentDir.filename, "/");

  if (len <= 1) return;

  for (int i = 1; i < len; ++i) {
    if (absolutePath[i] != '/') {
      currentDirBuf[currentDirIndex++] = absolutePath[i];
    }
    if (absolutePath[i] == '/' || i == len - 1) {
      if (currentDirIndex > 0) {
        currentDirBuf[currentDirIndex] = '\0';

        changeDirIterative(currentDirBuf);

        currentDirIndex = 0;
        memZero(currentDirBuf, 256);
      }
    }
  }
}
int getCurrentEntries(Fat16Entry* arr) {
  uint8_t sectorBuffer[512] = {0};
  int entryIndex = 0;
  if (currentDir.dirLBA == -1) {
    for (unsigned int s = 0; s < fileSystem.rootDirSectors; s++) {
      ataReadSector(fileSystem.fd,fileSystem.rootDirLBA + s, sectorBuffer);
      int noOfEntries = 512 / sizeof(Fat16Entry);

      for (int i = 0; i < noOfEntries; i++) {
        Fat16Entry* entry = (Fat16Entry*)(sectorBuffer + i * sizeof(Fat16Entry));
        if (entry->filename[0] == 0x00) break;
        if (entry->filename[0] == 0xE5) continue;
        arr[entryIndex++] = *entry;
      }
    }

    return entryIndex;
  }
  for (int i = 0; i < fileSystem.bs.sectors_per_cluster; ++i) {
    ataReadSector(fileSystem.fd, currentDir.dirLBA + i, sectorBuffer);

    for (int j = 0; j < 16; j++) {
      Fat16Entry* entryTmp = (Fat16Entry*)(sectorBuffer + j * sizeof(Fat16Entry));

      if (entryTmp->filename[0] == 0x00) break;
      if (entryTmp->filename[0] == 0xE5) continue;

      arr[entryIndex++] = *entryTmp;

    }
  }
  return entryIndex;
}

Fat16Entry findEntryInCurrentDir(const char* path) {
  Fat16Entry entries[16] = {0};
  Fat16Entry empty = {0};

  const char* lastPart = path;
  for (int i = stringLength(path) - 1; i >= 0; i--) {
    if (path[i] == '/') {
      lastPart = &path[i + 1];
      break;
    }
  }

  if (stringLength(lastPart) == 0) return empty;

  int noEntries = getCurrentEntries(entries);

  for (int i = 0; i < noEntries; i++) {
    char fileName[13] = {0};
    stringFat16Format(fileName, entries[i].filename, entries[i].ext);

    if (!stringCompare(fileName, (char*)lastPart)) {
      return entries[i];
    }
  }

  return empty;
}

void getParentPath(const char *path, char *parent) {
  int len = stringLength(path);
  if (len <= 1) {
    parent[0] = '/';
    parent[1] = '\0';
    return;
  }
  stringCat(parent, path);

  int slashIndex = stringFindChar(path,'/',true,0);

  if (slashIndex == 0) {
    parent[1] = '\0';
  } else if (slashIndex > 0) {
    parent[slashIndex] = '\0';
  } else {
    parent[0] = '/';
    parent[1] = '\0';
  }
}
void create_(const char* filename_) {
  int current = findFreeCluster();
  fileSystem.fatTable[current] = 0xFFFF;
  updateFatTable();
  findFirstFreeEntry(filename_,current,0);
}

void rmCurrentDir_(const char* dirName_) {
  Fat16Entry entries[512] = {0};
  int noEntries = getCurrentEntries(entries);

  for (int i = 0; i < noEntries; i++) {
    if (entries[i].filename[0] == '.'  ) {
      continue;
    }
    char filename[32] = {0};
    stringFat16Format(filename,entries[i].filename,entries[i].ext);
    delete_(filename);
  }
  changeDir("/");
  delete_(dirName_);
}
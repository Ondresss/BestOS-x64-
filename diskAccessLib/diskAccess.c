#include "diskAccess.h"

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

void readFileContent(int fd,const uint16_t* fatTable, const Fat16Entry* entry, const Fat16BootSector* bs,unsigned  int dataAreaLBA) {
  uint16_t currentCluster = entry->starting_cluster;
  uint8_t sectorBuffer[512] = {0};
  while (currentCluster < 0xFFF8) {
    unsigned int clusterStart = dataAreaLBA + (currentCluster - 2) * bs->sectors_per_cluster;
    for (int i = 0; i < bs->sectors_per_cluster; ++i) {
        ataReadSector(fd,clusterStart + i, sectorBuffer);
        write(1,sectorBuffer,512);
    }
    currentCluster = fatTable[currentCluster];
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


void list_(const char* filename_) {
  uint16_t fatTable[128 * 1024] = {0};
  const int fd = open("./sd.img",O_RDONLY);
  PartitionTable pt[4] = {0};
  Fat16BootSector bs = {0};

  uint8_t sectorBuffer[512] = {0};

  ataReadSector(fd,0,sectorBuffer);
  PartitionTable* ptTmp = (PartitionTable*)(sectorBuffer + 0x1BE);
  pt[0] = *ptTmp;

  ataReadSector(fd,pt[0].start_sector,sectorBuffer);
  Fat16BootSector* bsTmp = (Fat16BootSector*)sectorBuffer;
  bs = *bsTmp;

  readFat1Table(fd,fatTable,&bs,pt[0].start_sector);

  unsigned int rootDirLBA = (pt[0].start_sector + bs.reserved_sectors) + bs.number_of_fats * bs.fat_size_sectors;

  unsigned int rootDirSectors = (bs.root_dir_entries * sizeof(Fat16Entry)) / 512;

  for (int s = 0; s < rootDirSectors; s++) {
    ataReadSector(fd, rootDirLBA + s, sectorBuffer);

    int noOfEntries = 512 / sizeof(Fat16Entry);

    for (int i = 0; i < noOfEntries; i++) {
      Fat16Entry* entryTmp = (Fat16Entry*)(sectorBuffer + i * sizeof(Fat16Entry));

      if (entryTmp->filename[0] == 0x00) return;
      if (entryTmp->filename[0] == 0xE5) continue;

      char BUF[255] = {0};
      stringFat16Format(BUF,entryTmp->filename,entryTmp->ext);
      if (!stringCompare(filename_,"/")) {
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
        continue;
      }
      if (!stringCompare(BUF, filename_)) {
        char numBuf[20];
        int numLen;

        if (entryTmp->attributes & 0x10) {
          console_write("Found DIRECTORY: ", 17);
          console_write(BUF, stringLength(BUF));
          console_write("\n", 1);

          console_write("Start cluster: ", 15);
          numLen = unsignedIntToString(numBuf, entryTmp->starting_cluster);
          console_write(numBuf, numLen);
          console_write("\n", 1);
        }
        else {
          console_write("FILE: ", 6);
          console_write(BUF, stringLength(BUF));
          console_write("\n", 1);

          console_write("Size: ", 6);
          numLen = unsignedIntToString(numBuf, entryTmp->file_size);
          console_write(numBuf, numLen);
          console_write(" bytes\n", 7);

          console_write("Start cluster: ", 15);
          numLen = unsignedIntToString(numBuf, entryTmp->starting_cluster);
          console_write(numBuf, numLen);
          console_write("\n", 1);
        }
        return;
      }
    }
  }
  close(fd);
}

Date parseDate(uint16_t date) {
  Date date_ = {0};
  date_.day = date & 0x001F;
  date_.month = (date >> 5) & 0x0F;
  date_.year = 1980 + ((date >> 9) & 0x7F);

  return date_;
}



void read_(const char* filename_) {
  uint16_t fatTable[128 * 1024] = {0};
  const int fd = open("./sd.img",O_RDONLY);
  PartitionTable pt[4] = {0};
  Fat16BootSector bs = {0};

  uint8_t sectorBuffer[512] = {0};

  ataReadSector(fd,0,sectorBuffer);
  PartitionTable* ptTmp = (PartitionTable*)(sectorBuffer + 0x1BE);
  pt[0] = *ptTmp;

  ataReadSector(fd,pt[0].start_sector,sectorBuffer);
  Fat16BootSector* bsTmp = (Fat16BootSector*)sectorBuffer;
  bs = *bsTmp;

  readFat1Table(fd,fatTable,&bs,pt[0].start_sector);

  unsigned int rootDirLBA = (pt[0].start_sector + bs.reserved_sectors) + bs.number_of_fats * bs.fat_size_sectors;

  unsigned int rootDirSectors = (bs.root_dir_entries * sizeof(Fat16Entry)) / 512;

  for (int s = 0; s < rootDirSectors; s++) {
    ataReadSector(fd, rootDirLBA + s, sectorBuffer);

    int noOfEntries = 512 / sizeof(Fat16Entry);

    for (int i = 0; i < noOfEntries; i++) {
      Fat16Entry* entryTmp = (Fat16Entry*)(sectorBuffer + i * sizeof(Fat16Entry));

      if (entryTmp->filename[0] == 0x00) return;
      if (entryTmp->filename[0] == 0xE5) continue;

      char BUF[255] = {0};
      stringFat16Format(BUF,entryTmp->filename,entryTmp->ext);

      if (!stringCompare(BUF, filename_)) {
         int dataStartLBA = rootDirLBA + rootDirSectors;
         readFileContent(fd,fatTable,entryTmp,&bs,dataStartLBA);
      }
    }
  }
  close(fd);
}

#include "diskAccess.h"

unsigned char stringCompare(const char* str1,const char* str2) {
    int i = 0;
    while (str1[i] != '\0' && str2[i] != '\0') {
       if (str1[i] != str2[i]) {
          return 1;
       }
       i++;
    }
    return 0;
}

void stringFat16Format(char* buf,const unsigned char* filename,const unsigned char* ext) {
    int i = 0;
    int index = 0;
    while (i < 8) {
      if (filename[i] == ' ') {
        i++;
        continue;
      }
      buf[index++] = filename[i];
      i++;
    }
    if (ext[0] == ' ') {
      buf[index] = 0;
      return;
    }
    buf[index++] = '.';
    i = 0;
    while (i < 3) {
      if (ext[i] == ' ') {
        i++;
        continue;
      }
      buf[index++] = ext[i];
      i++;
    }
    buf[index] = 0;
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

void readFile(const char* filename_) {
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

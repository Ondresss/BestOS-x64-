#pragma once
#include "../arch/io.h"
#define ATA_DATA        0x1F0
#define ATA_SECTOR_CNT  0x1F2
#define ATA_LBA_LOW     0x1F3
#define ATA_LBA_MID     0x1F4
#define ATA_LBA_HIGH    0x1F5
#define ATA_DRIVE_SEL   0x1F6
#define ATA_COMMAND     0x1F7
#define ATA_STATUS      0x1F7
int ideReadSector(unsigned int lba, void *buffer);
int ideWriteSector(unsigned int lba, void *buffer);
void ideIsReady();

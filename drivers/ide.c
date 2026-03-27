#include "ide.h"


void ideIsReady() {
    while ((portByteIn(ATA_STATUS) & 0x80));
    while (!(portByteIn(ATA_STATUS) & 0x08));
}

void ideSelectDrive(unsigned int lba) {
    portByteOut(ATA_DRIVE_SEL, 0xE0 | ((lba >> 24) & 0x0F));
}

void ideSetAddress(unsigned int lba) {
    portByteOut(ATA_LBA_LOW,  (unsigned char)lba);
    portByteOut(ATA_LBA_MID,  (unsigned char)(lba >> 8));
    portByteOut(ATA_LBA_HIGH, (unsigned char)(lba >> 16));
}
int ideReadSector(unsigned int lba, void *buffer) {
    portByteOut(ATA_DRIVE_SEL, 0xE0 | ((lba >> 24) & 0x0F));

    for(int i=0; i<4; i++) portByteIn(ATA_STATUS);

    portByteOut(ATA_SECTOR_CNT, 1);
    portByteOut(ATA_LBA_LOW,  (unsigned char)lba);
    portByteOut(ATA_LBA_MID,  (unsigned char)(lba >> 8));
    portByteOut(ATA_LBA_HIGH, (unsigned char)(lba >> 16));

    portByteOut(ATA_COMMAND, 0x20);

    while (1) {
        unsigned char status = portByteIn(ATA_STATUS);
        if (!(status & 0x80) && (status & 0x08)) break;
        if (status & 0x01) return -1;
    }

    unsigned short *ptr = (unsigned short *)buffer;
    for (int i = 0; i < 256; i++) {
        ptr[i] = portWordIn(ATA_DATA);
    }

    return 0;
}
int ideWriteSector(unsigned int lba, void *buffer) {
    portByteOut(ATA_DRIVE_SEL, 0xE0 | ((lba >> 24) & 0x0F));
    for(int i = 0; i < 4; i++) portByteIn(ATA_STATUS);

    portByteOut(ATA_SECTOR_CNT, 1);
    portByteOut(ATA_LBA_LOW,  (unsigned char)lba);
    portByteOut(ATA_LBA_MID,  (unsigned char)(lba >> 8));
    portByteOut(ATA_LBA_HIGH, (unsigned char)(lba >> 16));

    portByteOut(ATA_COMMAND, 0x30);

    while (1) {
        unsigned char status = portByteIn(ATA_STATUS);
        if (!(status & 0x80) && (status & 0x08)) break;
        if (status & 0x01) return -1;
    }

    unsigned short *ptr = (unsigned short *)buffer;
    for (int i = 0; i < 256; i++) {
        portWordOut(ATA_DATA, ptr[i]);
    }

    portByteOut(ATA_COMMAND, 0xE7);

    while (portByteIn(ATA_STATUS) & 0x80);

    return 0;
}
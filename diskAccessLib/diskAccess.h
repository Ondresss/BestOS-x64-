#include "fat.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include "strings.h"

typedef struct {
    unsigned int day;
    unsigned int month;
    unsigned int year;
}Date;


void console_write(const char *buf, uint32_t len);



void read_(const char* filename_);
void list_(const char* filename_);

static Date parseDate(uint16_t date);
static void readFat1Table(int fd, uint16_t* table, const Fat16BootSector* bs, uint32_t pos);
static int ataReadSector(int fd, uint32_t lba, uint8_t *buffer);
static void readFileContent(int fd,const uint16_t* fatTable, const Fat16Entry* entry, const Fat16BootSector* bs,unsigned  int dataAreaLBA);

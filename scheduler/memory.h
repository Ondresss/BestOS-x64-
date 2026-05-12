#pragma once


#include <stdint.h>

#define PROC_BASE_START  0x00100000
#define PROC_SLOT_SIZE   0x00010000
#define MAX_PROCESSES    8

#define GET_PROC_BASE(i) (PROC_BASE_START + ((i) * PROC_SLOT_SIZE))
#define GET_PROC_STACK_TOP(i) (GET_PROC_BASE(i) + PROC_SLOT_SIZE - 4)

struct partition {
    uint32_t base;
    uint32_t size;
    int used;
    int thread_id;
};

void memory_init();
struct partition* memory_find_free_partition();


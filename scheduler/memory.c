#include "memory.h"

struct partition partition_table[MAX_PROCESSES];

void memory_init() {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        partition_table[i].base = GET_PROC_BASE(i);
        partition_table[i].size = PROC_SLOT_SIZE;
        partition_table[i].used = 0;
        partition_table[i].thread_id = -1;
    }
}

struct partition* memory_find_free_partition() {

    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (!partition_table[i].used) {
            return partition_table + i;
        }
    }

    return 0;
}

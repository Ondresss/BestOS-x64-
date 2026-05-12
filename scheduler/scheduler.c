#include "scheduler.h"

static unsigned char stacks[MAX_PROCESSES][STACK_SIZE] __attribute__((aligned(16)));
struct gt *gt_current;
struct gt gt_table[MAX_PROCESSES];


struct gt* scheduler_find_empty_pos () {
    struct gt *p = &gt_table[MaxGThreads];
    for (int i = 0; i < MaxGThreads; i++) {
        if (gt_table[i].state == Unused) {
            p = &gt_table[i];
            return p;
            break;
        }
    }
    return 0;
}


int schedule_create(void (*f)(void)) {
    int thread_idx = -1;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (gt_table[i].state == Unused) {
            thread_idx = i;
            break;
        }
    }

    if (thread_idx == -1) return -1;

    struct partition* part = memory_find_free_partition();
    if (part == 0) return -1;

    part->used = 1;
    struct gt *p = &gt_table[thread_idx];
    p->base = part->base;
    uint32_t *stack = (uint32_t *)(p->base + PROC_SLOT_SIZE - 4);

    *(--stack) = 0x0202;
    *(--stack) = 0x08;
    *(--stack) = (uint32_t)f;

    *(--stack) = 0;
    *(--stack) = 0;

    *(--stack) = 0; // EAX
    *(--stack) = 0; // ECX
    *(--stack) = 0; // EDX
    *(--stack) = 0; // EBX
    *(--stack) = 0; // ESP
    *(--stack) = 0; // EBP
    *(--stack) = 0; // ESI
    *(--stack) = 0; // EDI

    *(--stack) = 0x10;

    p->esp = (uint32_t)stack;
    p->state = Ready;

    return thread_idx;
}

uint32_t schedule_tick(uint32_t current_esp) {
    gt_current->esp = current_esp;

    if (gt_current->state == Running) {
        gt_current->state = Ready;
    }

    int start_idx = (gt_current - gt_table);
    int next_idx = start_idx;

    for (int i = 0; i < MAX_PROCESSES; i++) {
        next_idx = (next_idx + 1) % MAX_PROCESSES;
        if (gt_table[next_idx].state == Ready) {
            gt_current = &gt_table[next_idx];
            gt_current->state = Running;
            return gt_current->esp;
        }
    }

    if (gt_current->state == Unused) {
        gt_current = &gt_table[0];
    }

    gt_current->state = Running;
    return gt_current->esp;
}

void scheduler_init() {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        gt_table[i].state = Unused;
    }
    gt_table[0].state = Running;
    gt_current = &gt_table[0];
}
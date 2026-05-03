#include "scheduler.h"

static unsigned char stacks[MaxGThreads][STACK_SIZE] __attribute__((aligned(16)));
struct gt *gt_current;
struct gt gt_table[];
int schedule_create(void (*f)(void)) {
    struct gt *p;
    int idx = -1;

    for (int i = 0; i < MaxGThreads; i++) {
        if (gt_table[i].state == Unused) {
            p = &gt_table[i];
            idx = i;
            break;
        }
    }

    if (idx == -1) return -1;

    uint32_t *stack = (uint32_t *)&stacks[idx][STACK_SIZE];

    *(--stack) = 0x0202;
    *(--stack) = 0x08;
    *(--stack) = (uint32_t)f;

    *(--stack) = 32;
    *(--stack) = 0;
    *(--stack) = 0; // EAX
    *(--stack) = 0; // ECX
    *(--stack) = 0; // EDX
    *(--stack) = 0; // EBX
    *(--stack) = 0; // ESP (dummy)
    *(--stack) = 0; // EBP
    *(--stack) = 0; // ESI
    *(--stack) = 0; // EDI

    *(--stack) = 0x10; // DS
    p->esp = (uint32_t)stack;
    p->state = Ready;

    p->totalTime = 0;
    p->noSwitches = 0;

    return 0;
}

uint32_t schedule_tick(uint32_t current_esp) {
    gt_current->esp = current_esp;

    struct gt *p = gt_current;

    do {
        if (++p == &gt_table[MaxGThreads])
            p = &gt_table[0];
    } while (p->state != Ready && p->state != Running);

    if (gt_current->state == Running)
        gt_current->state = Ready;

    p->state = Running;
    gt_current = p;

    return p->esp;
}

    void __attribute__((noreturn)) schedule_return(int ret) {
    asm volatile("cli");

    if (gt_current != &gt_table[0]) {
        gt_current->state = Unused;


        asm volatile("sti");
        asm volatile("int $0x20");

        while(1) { asm volatile("hlt"); }
    }

    while (1) {
        asm volatile("hlt");
    }
}

void scheduler_init() {
    for (int i = 0; i < MaxGThreads; i++) {
        gt_table[i].state = Unused;
    }
    gt_table[0].state = Running;
    gt_current = &gt_table[0];
}
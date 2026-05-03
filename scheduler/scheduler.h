#pragma once
#include <stdint.h>
#define STACK_SIZE 4096
#define MAX_PRIORITY_QUEUES 40
#define MAX_THREADS_PER_QUEUE 20

struct gt {
    uint32_t esp;
    enum {
        Unused,
        Running,
        Ready,
     } state;
    long long totalTime;
    long long currentWaitTime;
    long long totalWaitTime;
    long long currentMinWaitTime;
    long long currentMaxWaitTime;
    long long noSwitches;
    int priority;
    int tickets;
};
enum {
    MaxGThreads = 5,
    StackSize = 0x400000,
};
typedef struct {
    struct gt* threads[MAX_THREADS_PER_QUEUE];
    int count;
} PriorityQueue;
extern struct gt *gt_current;
extern struct gt gt_table[];

int schedule_create(void (*f)(void));
uint32_t schedule_tick(uint32_t current_esp);
void __attribute__((noreturn)) schedule_return(int ret);
void scheduler_init();
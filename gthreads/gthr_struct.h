#pragma once
#include "gthr.h"
#define STACK_SIZE 4096
#define MAX_PRIORITY_QUEUES 40
#define MAX_THREADS_PER_QUEUE 20

extern struct gt gt_table[MaxGThreads];
extern struct gt *gt_current;
unsigned char stacks[MaxGThreads][STACK_SIZE] __attribute__((aligned(16)));

typedef struct {
    struct gt* threads[MAX_THREADS_PER_QUEUE];
    int count;
} PriorityQueue;

PriorityQueue priorityQueues[MAX_PRIORITY_QUEUES];
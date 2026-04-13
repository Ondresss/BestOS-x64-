// gthread control structures
#define STACK_SIZE 4096
struct gt gt_table[MaxGThreads];                                                // statically allocated table for thread control
struct gt *gt_current;
unsigned char stacks[MaxGThreads][STACK_SIZE] __attribute__((aligned(16)));

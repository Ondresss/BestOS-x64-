#pragma once
#define AGING_THRESHOLD_US 2000000
#include <sys/time.h>
#include <string.h>
#include <stdint.h>
enum {
	MaxGThreads = 5,
	StackSize = 0x400000,
};


typedef enum {
	RR,
	RP,
	LOT
}SchedulingAlg;

struct gt {
	struct gt_context {
		uint64_t rsp;
		uint64_t r15;
		uint64_t r14;
		uint64_t r13;
		uint64_t r12;
		uint64_t rbx;
		uint64_t rbp;
	}
	ctx;
	enum {
		Unused,
		Running,
		Ready,
	}
	state;

	long long totalTime;
	long long currentWaitTime;
	long long totalWaitTime;
	long long currentMinWaitTime;
	long long currentMaxWaitTime;
	long long noSwitches;
	struct timeval last_start;

	int priority;
	int tickets;
};


void gt_init(void);
void gt_return(int ret);
void gt_switch(struct gt_context * old, struct gt_context * new);
bool gt_schedule(void);
void gt_stop(void);
int gt_create(void( * f)(void));
void gt_reset_sig(int sig);
void gt_alarm_handle(int sig);
int gt_uninterruptible_nanosleep(time_t sec, long nanosec);

void calc_total_time();
void calc_statistics(struct gt* p);

struct gt* priority_schedule();
int priority_create(void( * f)(void),int priority);
void priority_age();
void priority_remove(struct gt* p);

int lottery_create(void( * f)(void),int ticketSum);
struct gt*  lottery_schedule();

void setSchedulingAlg(SchedulingAlg alg);
SchedulingAlg getSchedulingAlg(void);
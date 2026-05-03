#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#include "gthr.h"
#include "gthr_struct.h"


SchedulingAlg schedulingAlg = RR;


void setSchedulingAlg(SchedulingAlg alg) {
	schedulingAlg = alg;
}

SchedulingAlg getSchedulingAlg(void) {
	return schedulingAlg;
}

void gt_alarm_handle(int sig) {
	gt_schedule();
}


void gt_statistics_handler(int sig) {
	printf("\n--- THREAD STATISTICS ---\n");
	printf("----------------------------------------------------\n");
	for (int i = 0; i < MaxGThreads; i++) {
		if (gt_table[i].state != Unused) {
			long long avgWaitTime = 0;
			if (gt_table[i].noSwitches > 0) {
				avgWaitTime = gt_table[i].totalWaitTime / gt_table[i].noSwitches;
			}
			printf("Thread #%d | Total runtime:  %lld us | Total wait time: %lld us | Min wait time: %lld us | Max wait time: %lld us ! | Avg wait time: %lld us\n",
				   i,
				   gt_table[i].totalTime,
				   gt_table[i].totalWaitTime,
				   gt_table[i].currentMinWaitTime,
				   gt_table[i].currentMaxWaitTime,
				   avgWaitTime
				   );
		}
	}
	printf("----------------------------------------------------\n");

	exit(EXIT_SUCCESS);
}

void gt_init(void) {
	memset(gt_table,0,sizeof(gt_table));
	gt_current = & gt_table[0];
	gt_current -> state = Running;
	gettimeofday(&gt_current->last_start, NULL);
	signal(SIGALRM, gt_alarm_handle);
	signal(SIGINT, gt_statistics_handler);
}


void __attribute__((noreturn)) gt_return(int ret) {
	if (gt_current != & gt_table[0]) {
		gt_current -> state = Unused;
		gt_schedule();
		assert(!"reachable");
	}
	while (gt_schedule());
	exit(ret);
}

bool gt_schedule(void) {
	struct gt * p;
	struct gt_context * old, * new;
	struct timeval now;
	gettimeofday(&now, NULL);

	gt_reset_sig(SIGALRM);
	calc_total_time();

	p = gt_current;
	if (schedulingAlg == RR) {
		while (p -> state != Ready) {
			if (++p == & gt_table[MaxGThreads])
				p = & gt_table[0];
			if (p == gt_current)
				return false;
		}
	}
	if (schedulingAlg == RP) {
		priority_age();
		p = priority_schedule();
		if (p == gt_current) {
			printf("Could not found new thread in priority queues\n");
			return false;
		}
	}

	if (schedulingAlg == LOT) {
		p = lottery_schedule();
	}

	calc_statistics(p);

	if (gt_current -> state != Unused)
		gt_current -> state = Ready;
	p -> state = Running;
	old = & gt_current -> ctx;
	new = & p -> ctx;
	gt_current = p;
	gt_switch(old, new);
	return true;
}

void calc_statistics(struct gt* p) {
	struct timeval now;
	gettimeofday(&now, NULL);
	if (p->last_start.tv_sec > 0) {
		long long seconds = now.tv_sec - p->last_start.tv_sec;
		long long microseconds = now.tv_usec - p->last_start.tv_usec;
		long long wait_us = (seconds * 1000000LL) + microseconds;
		if (wait_us < 0) wait_us = 0;
		p->currentWaitTime = wait_us;
		p->totalWaitTime += wait_us;

	}
	if (p->currentWaitTime <= p->currentMinWaitTime || p->currentMinWaitTime == 0) {
		p->currentMinWaitTime = p->currentWaitTime;
	}
	if (p->currentWaitTime >= p->currentMaxWaitTime){
		p->currentMaxWaitTime = p->currentWaitTime;
	}

	p->noSwitches++;
	p->last_start = now;
}

void calc_total_time() {
	struct timeval now;
	gettimeofday(&now, NULL);
	if (gt_current->state == Running && gt_current->last_start.tv_sec > 0) {
		long long seconds = now.tv_sec - gt_current->last_start.tv_sec;
		long long microseconds = now.tv_usec - gt_current->last_start.tv_usec;

		long long elapsed_us = (seconds * 1000000LL) + microseconds;

		gt_current->totalTime += elapsed_us;
	}
}



void gt_stop(void) {
	gt_return(0);
}

int gt_create(void( * f)(void)) {
	struct gt * p;

	for (p = & gt_table[0];; p++)
		if (p == & gt_table[MaxGThreads])
			return -1;
		else if (p -> state == Unused)
			break;

	int idx = p - &gt_table[0];

	unsigned char *stack = stacks[idx];
	*(uint64_t * ) & stack[STACK_SIZE - 8] = (uint64_t) gt_stop;
	*(uint64_t * ) & stack[STACK_SIZE - 16] = (uint64_t) f;
	p -> ctx.rsp = (uint64_t) & stack[STACK_SIZE - 16];
	p -> state = Ready;

	return 0;
}


int priority_create(void( * f)(void),int priority) {
	struct gt * p;

	for (p = & gt_table[0];; p++)
		if (p == & gt_table[MaxGThreads])
			return -1;
		else if (p -> state == Unused)
			break;

	int idx = p - &gt_table[0];

	unsigned char *stack = stacks[idx];
	*(uint64_t * ) & stack[STACK_SIZE - 8] = (uint64_t) gt_stop;
	*(uint64_t * ) & stack[STACK_SIZE - 16] = (uint64_t) f;
	p -> ctx.rsp = (uint64_t) & stack[STACK_SIZE - 16];
	p -> state = Ready;
	int i = priorityQueues[priority + 20].count++;
	priorityQueues[priority + 20].threads[i] = p;

	return 0;
}


void gt_reset_sig(int sig) {
	if (sig == SIGALRM) {
		alarm(0);
	}

	sigset_t set;
	sigemptyset( & set);
	sigaddset( & set, sig);
	sigprocmask(SIG_UNBLOCK, & set, NULL);

	if (sig == SIGALRM) {
		ualarm(500, 500);
	}
}

int gt_uninterruptible_nanosleep(time_t sec, long nanosec) {
	struct timespec req;
	req.tv_sec = sec;
	req.tv_nsec = nanosec;

	do {
		if (0 != nanosleep( & req, & req)) {
			if (errno != EINTR)
				return -1;
		} else {
			break;
		}
	} while (req.tv_sec > 0 || req.tv_nsec > 0);
	return 0;
}

struct gt* priority_schedule() {
	int i = 0;
	while (i < MAX_PRIORITY_QUEUES) {
		int noThreadsInQueue = priorityQueues[i].count;
		if (noThreadsInQueue == 0) {
			++i;
			continue;
		}
		struct gt* foundThread = 0;
		for (int t = 0; t < noThreadsInQueue; ++t) {
			if (priorityQueues[i].threads[t]->state == Ready) {
				foundThread = priorityQueues[i].threads[t];
			}
		}

		if (foundThread) {
			struct gt* firstT = priorityQueues[i].threads[0];
			for (int t = 0; t < noThreadsInQueue-1; ++t) {
				priorityQueues[i].threads[t] = priorityQueues[i].threads[t+1];
			}
			priorityQueues[i].threads[noThreadsInQueue-1] = firstT;

			return foundThread;
		}

		++i;
	}
	return gt_current;
}


void priority_remove(struct gt* p) {
	PriorityQueue* queue = &priorityQueues[p->priority + 20];
	int noThreadsInQueue = queue->count;
	int index = -1;
	for (int i = 0; i < noThreadsInQueue; ++i) {
		if (queue->threads[i] == p) {
			queue->threads[i] = 0;
			index = i;
		}
		if (index >= 0 && i < noThreadsInQueue-1) {
			queue->threads[i] = queue->threads[i+1];
		}
	}
	queue->count--;
}

void priority_age() {
	struct gt * p;
	struct timeval now;
	gettimeofday(&now, NULL);

	for (int i = 0; i < MaxGThreads; i++) {
		p = &gt_table[i];

		if (p->state == Ready && p != gt_current) {

			long long seconds = now.tv_sec - p->last_start.tv_sec;
			long long microseconds = now.tv_usec - p->last_start.tv_usec;
			long long wait_us = (seconds * 1000000LL) + microseconds;

			if (wait_us >= AGING_THRESHOLD_US && p->priority > -20) {
				priority_remove(p);
				p->priority--;

				PriorityQueue* newQ = &priorityQueues[p->priority + 20];
				newQ->threads[newQ->count++] = p;

				p->last_start = now;
				printf("Aging: Thread %d promoted to priority %d\n", i, p->priority);
			}
		}
	}
}

int lottery_create(void( * f)(void),int ticketSum) {
	struct gt * p;

	for (p = & gt_table[0];; p++)
		if (p == & gt_table[MaxGThreads])
			return -1;
		else if (p -> state == Unused)
			break;

	int idx = p - &gt_table[0];

	unsigned char *stack = stacks[idx];
	*(uint64_t * ) & stack[STACK_SIZE - 8] = (uint64_t) gt_stop;
	*(uint64_t * ) & stack[STACK_SIZE - 16] = (uint64_t) f;
	p -> ctx.rsp = (uint64_t) & stack[STACK_SIZE - 16];
	p -> state = Ready;
	p->tickets = ticketSum;
	return 0;
}

struct gt*  lottery_schedule() {
	int totalSum = 0;
	struct gt * p;
	for (p = & gt_table[0];; p++) {
		if (p == & gt_table[MaxGThreads])
			break;
		if (p->state == Ready) {
			totalSum += p->tickets;
		}
	}
	int randTicket = rand() % totalSum;
	int currentSum = 0;
	for (p = & gt_table[0];; p++) {
		if (p == & gt_table[MaxGThreads])
			return gt_current;
		if (p->state == Ready) {
			currentSum += p->tickets;
			if (currentSum > randTicket) return p;
		}
	}
	return gt_current;
}



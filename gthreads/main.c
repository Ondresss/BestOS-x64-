// Based on https://c9x.me/articles/gthreads/code0.html
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

// Dummy function to simulate some thread work
void f(void) {
	static int x;
	int i = 0, id;

	id = ++x;
	while (true) {

		printf("F Thread id = %d, val = %d BEGINNING\n", id, ++i);
		gt_uninterruptible_nanosleep(0, 50000000);
		printf("F Thread id = %d, val = %d END\n", id, ++i);
		gt_uninterruptible_nanosleep(0, 50000000);
	}
}

// Dummy function to simulate some thread work
void g(void) {
	static int x;
	int i = 0, id;

	id = ++x;
	while (true) {
		printf("G Thread id = %d, val = %d BEGINNING\n", id, ++i);
		gt_uninterruptible_nanosleep(0, 50000000);
		printf("G Thread id = %d, val = %d END\n", id, ++i);
		gt_uninterruptible_nanosleep(0, 50000000);
	}
}

int main(int argc,const char** argv) {

	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i],"-RR")) setSchedulingAlg(RR);
		if (!strcmp(argv[i],"-PRI")) setSchedulingAlg(RP);
		if (!strcmp(argv[i],"-LS")) setSchedulingAlg(RR);
	}
	gt_init();
	if (getSchedulingAlg() == RP) {
		priority_create(f,-20);
		priority_create(f,-5);
		priority_create(g,2);
		priority_create(g,8);
		gt_return(1);
	}
	if (getSchedulingAlg() == LOT) {
		lottery_create(f,100);
		lottery_create(f,200);
		lottery_create(g,400);
		lottery_create(g,800);
		gt_return(1);
	}
	gt_create(f);
	gt_create(f);
	gt_create(g);
	gt_create(g);
	gt_return(1);
}

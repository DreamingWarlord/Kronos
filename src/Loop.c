#include "Common.h"


static void (*loops[256]) (double, double) = { NULL };
static uint64 loopc = 0;


void LoopRegister(void (*update) (double delta, double time))
{
	loops[loopc++] = update;
}

void Loop()
{
	double time = 0.0;
	double last_time = 0.0;
	double delta = 0.0;
	struct timespec tp;
	struct timespec tp_wanted;
	clock_gettime(CLOCK_MONOTONIC_RAW, &tp);
	time = tp.tv_sec + ((double) tp.tv_nsec / 1000000000.0);
	last_time = time - 0.01;

	while(TRUE) {
		clock_gettime(CLOCK_MONOTONIC_RAW, &tp);
		time = tp.tv_sec + ((double) tp.tv_nsec / 1000000000.0);
		delta = time - last_time;
		last_time = time;

		for(uint64 i = 0; i < loopc; i++)
			loops[i](delta, time);

		clock_gettime(CLOCK_MONOTONIC_RAW, &tp_wanted);
		tp_wanted.tv_sec -= tp.tv_sec;
		tp_wanted.tv_nsec -= tp.tv_nsec;

		if(tp_wanted.tv_sec == 0 && tp_wanted.tv_nsec < 20000000L)
			tp_wanted = (struct timespec) { 0, 20000000ULL - tp_wanted.tv_nsec };

		nanosleep(&tp_wanted, NULL);
	};
}

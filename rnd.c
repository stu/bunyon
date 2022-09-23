
#include <time.h>
#include <stdint.h>

#include "rnd.h"

int32_t rnd_seed;

void reset_rnd_seed(void)
{
	set_rnd_seed((time(NULL)%(RNG_M-1))+1);
}

void set_rnd_seed(int32_t seedval)
{
	rnd_seed=seedval;
}

int32_t get_rnd_seed(void)
{
	return(rnd_seed);
}


int32_t rnd(void)
{
	int32_t low, high, test;

	high = rnd_seed / RNG_Q;
	low = rnd_seed % RNG_Q;
	test = RNG_A * low - RNG_R * high;
	if(test>0)
		rnd_seed = test;
	else
		rnd_seed = test + RNG_M;

	return rnd_seed;
}

int32_t xrnd(int32_t maxrnd)
{
	if(maxrnd==0L)
		return rnd();
	else
		return((rnd())%maxrnd);
}

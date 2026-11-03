/*
 * threadchurn - start and join short threads forever. Each thread touches
 * new stack pages, so it takes page faults and then exits. Used by
 * tests/regress.sh to check that the lifetime fault sum never goes down
 * while threads move their counts into signal->min_flt at exit.
 */
#include <pthread.h>
#include <string.h>

#define NTHREADS 32

static void *worker(void *arg)
{
	volatile char buf[64 << 10];

	(void)arg;
	memset((char *)buf, 1, sizeof(buf));
	return NULL;
}

int main(void)
{
	pthread_t t[NTHREADS];

	for (;;) {
		for (int i = 0; i < NTHREADS; i++)
			pthread_create(&t[i], NULL, worker, NULL);
		for (int i = 0; i < NTHREADS; i++)
			pthread_join(t[i], NULL);
	}
}

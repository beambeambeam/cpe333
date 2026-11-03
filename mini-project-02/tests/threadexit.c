/*
 * threadexit - the main thread calls pthread_exit(), then NWORKERS threads
 * leave a barrier and exit at the same time. Used by tests/regress.sh to check
 * that exactly one thread takes the exit snapshot.
 */
#include <pthread.h>
#include <unistd.h>

#define NWORKERS 8

static pthread_barrier_t barrier;

static void *worker(void *arg)
{
	(void)arg;
	pthread_barrier_wait(&barrier);
	return NULL;
}

int main(void)
{
	pthread_t t[NWORKERS];

	usleep(50000);	/* time for the caller to select this PID */
	pthread_barrier_init(&barrier, NULL, NWORKERS);
	for (int i = 0; i < NWORKERS; i++)
		pthread_create(&t[i], NULL, worker, NULL);
	pthread_exit(NULL);
}

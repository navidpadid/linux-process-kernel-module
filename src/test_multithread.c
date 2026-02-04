// SPDX-License-Identifier: (GPL-2.0 OR BSD-2-Clause)
// Simple multi-threaded test program for E2E testing
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_THREADS 4

void *worker_thread(void *arg)
{
	long thread_id = (long)arg;
	long i;

	printf("Thread %ld started\n", thread_id);

	/* Do some work to show CPU usage and keep thread alive */
	for (i = 0; i < 1000000; i++) {
		/* Light CPU work */
		volatile long tmp = i * i;
		(void)tmp;

		/* Sleep occasionally to avoid 100% CPU */
		if (i % 10000 == 0) {
			usleep(100000); /* Sleep 100ms */
		}
	}

	printf("Thread %ld finished\n", thread_id);
	return NULL;
}

int main(void)
{
	pthread_t threads[NUM_THREADS];
	long i;

	printf("Multi-threaded test application\n");
	printf("Main PID: %d\n", getpid());
	printf("Creating %d threads...\n\n", NUM_THREADS);

	/* Create worker threads */
	for (i = 0; i < NUM_THREADS; i++) {
		int rc =
		    pthread_create(&threads[i], NULL, worker_thread, (void *)i);
		if (rc) {
			fprintf(stderr, "Error creating thread %ld: %d\n", i,
				rc);
			exit(1);
		}
	}

	/* Wait for threads to complete */
	for (i = 0; i < NUM_THREADS; i++) {
		pthread_join(threads[i], NULL);
	}

	printf("\nAll threads completed\n");
	printf("Total threads (main + workers): %d\n", NUM_THREADS + 1);

	return 0;
}

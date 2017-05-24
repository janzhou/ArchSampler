#ifndef PCM_H_
#define PCM_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>

#include "app.h"

#define PCM_NUM_BANKS		4
//#define PCM_ROWS_PER_BANK	(1024 * 32)
#define PCM_ROWS_PER_BANK	(1024 * 32)
#define PCM_ROW_SIZE		(1024 * 8)
#define PCM_BANK_SIZE		(PCM_ROWS_PER_BANK * PCM_ROW_SIZE)
#define PCM_NUM_ROWS		(PCM_NUM_BANKS * PCM_ROWS_PER_BANK)
#define PCM_SIZE		(PCM_NUM_ROWS * PCM_ROW_SIZE)

struct pcm_thread {
	pthread_t pthread;
	unsigned long (* count_fn)(void *row);
	unsigned long count;
	void (* fn)(void *row);
	int num_rows;
	void * rows[PCM_ROWS_PER_BANK];
};


inline void pcm_threads_map_fn(
		struct pcm_thread pcm_threads[], int num_threads,
		void (* fn)(void *row)
		) {
	int i;
	for(i = 0; i < num_threads; i++) {
		pcm_threads[i].fn = fn;
	}
}

inline void pcm_threads_map_count_fn(
		struct pcm_thread pcm_threads[], int num_threads,
		unsigned long (* count_fn)(void *row)
		) {
	int i;
	for(i = 0; i < num_threads; i++) {
		pcm_threads[i].count_fn = count_fn;
	}
}

inline void pcm_threads_reduce_count_fn(
		struct pcm_thread pcm_threads[], int num_threads,
		void (* count_reduce)(unsigned long count)
		){
	int i;
	for(i = 0; i < num_threads; i++) {
		count_reduce(pcm_threads[i].count);
	}
}

void pcm_threads_spawn(struct pcm_thread pcm_threads[], int num_threads);
void pcm_threads_join(struct pcm_thread pcm_threads[], int num_threads);

#endif /* PCM_H_ */

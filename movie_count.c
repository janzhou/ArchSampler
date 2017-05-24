#include "pcm.h"
#include "movie.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

int main(int argc, char* argv[]) {
	char* buf;
	struct pcm_thread pcm_threads[PCM_NUM_BANKS];

	struct timeval t1, t2;
	float execution_time;

	int r, b;

	if (argc != 2) {
		printf("Usage: pcm <input_file>\n");
		return 0;
	}

	buf = (char *) calloc(1, PCM_SIZE);
	if (!buf) {
		perror("Failed to allocate the memory:");
		return errno;
	}

	printf("Total memory allocated = %d\n", PCM_SIZE);

	for(r = 0; r < PCM_NUM_ROWS; r++) {
		int bank = PCM_R2B(r);
        pcm_thread_add_row(pcm_threads + bank, buf, r);
	}

	gettimeofday(&t1, NULL);

	pcm_threads_map_count_fn(pcm_threads, PCM_NUM_BANKS, pcm_movie_db_cnt_local);

	if (pcm_movie_db_init(buf, argv[1]))
		return errno;

	pcm_threads_spawn(pcm_threads, PCM_NUM_BANKS);
	pcm_threads_join(pcm_threads, PCM_NUM_BANKS);
	pcm_threads_reduce_count_fn(pcm_threads, PCM_NUM_BANKS, pcm_movie_db_cnt_global);

	gettimeofday(&t2, NULL);
	execution_time = ((t2.tv_sec * 1000000 + t2.tv_usec) - (t1.tv_sec * 1000000 + t1.tv_usec)) / (float) 1000;

	printf("Movie db element count: %lu\n", pcm_movie_db_get_global_cnt());
	printf("Time taken: %.2f ms\n", execution_time);

	free(buf);
}

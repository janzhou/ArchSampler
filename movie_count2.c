#include "pcm.h"
#include "movie.h"
#include "arielapi.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

int main(int argc, char* argv[]) {
	char* buf;
	struct pcm_thread pcm_threads[PCM_NUM_BANKS];

	pcm_param(argc, argv);

	memset(pcm_threads, 0, sizeof(pcm_threads));

	struct timeval t1, t2;
	float execution_time;

	buf = (char *) calloc(1, PCM_SIZE);
	if (!buf) {
		perror("Failed to allocate the memory:");
		return errno;
	}

	int rows[PCM_NUM_ROWS], r;
	for(r = 0; r < PCM_NUM_ROWS; r++) {
		rows[r] = r;
	}

	pcm_rows_shuffle(rows, PCM_NUM_ROWS);

	pcm_r2t_even_split(pcm_threads, PCM_NUM_BANKS, rows, PCM_NUM_ROWS, buf);

	pcm_threads_map_count_fn(pcm_threads, PCM_NUM_BANKS, pcm_movie_db_cnt_local);

	if (pcm_movie_db_init(buf, "data/ml-20m/ratings.csv"))
		return errno;

	gettimeofday(&t1, NULL);
	ariel_enable();

	pcm_threads_run(pcm_threads, PCM_NUM_BANKS);
	pcm_threads_reduce_count_fn(pcm_threads, PCM_NUM_BANKS, pcm_movie_db_cnt_global);

	gettimeofday(&t2, NULL);
	execution_time = ((t2.tv_sec * 1000000 + t2.tv_usec) - (t1.tv_sec * 1000000 + t1.tv_usec)) / (float) 1000;

	printf("Movie db element count: %lu\n", pcm_movie_db_get_global_cnt());
	printf("Time taken: %.2f ms\n", execution_time);

	free(buf);
}

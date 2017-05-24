#include "pcm.h"
#include "arielapi.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

void write(void* row){
	int i = 0;
	int *buf = (int *) row;
	for(i = 0; i < PCM_ROW_SIZE/sizeof(int); i++){
		buf[i] = i;
	}
}

void read(void* row){
	int i = 0;
	int *buf = (int *) row;
	int sum = 0;
	for(i = 0; i < PCM_ROW_SIZE/sizeof(int); i++){
		sum += buf[i];
	}

	//printf("%d\n", sum);
}

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
		int b = r % PCM_NUM_BANKS;
		struct pcm_thread * pth = pcm_threads + b;
		pth->rows[pth->num_rows] = buf + r * PCM_ROW_SIZE;
		pth->num_rows++;
	}

	gettimeofday(&t1, NULL);

	pcm_threads_map_count_fn(pcm_threads, PCM_NUM_BANKS, pcm_movie_db_cnt_local);

	if (pcm_movie_db_init(buf, argv[1]))
		return errno;

	ariel_enable();

	pcm_threads_spawn(pcm_threads, PCM_NUM_BANKS);
	pcm_threads_join(pcm_threads, PCM_NUM_BANKS);
	pcm_threads_reduce_count_fn(pcm_threads, PCM_NUM_BANKS, pcm_movie_db_cnt_global);

	gettimeofday(&t2, NULL);
	execution_time = ((t2.tv_sec * 1000000 + t2.tv_usec) - (t1.tv_sec * 1000000 + t1.tv_usec)) / (float) 1000;

	printf("Movie db element count: %lu\n", pcm_movie_db_get_global_cnt());
	printf("Time taken: %.2f ms\n", execution_time);

	free(buf);
}

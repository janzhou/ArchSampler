#include "pcm.h"
#include "arielapi.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

void read_fn(void* row){
	int i = 0;
	int *buf = (int *) row;
	int sum = 0;
	for(i = 0; i < PCM_ROW_SIZE/sizeof(int); i++){
		sum += buf[i];
	}

	//printf("%d\n", sum);
}

int main(int argc, char* argv[]) {
	pcm_param(argc, argv);

	char *buf;
	buf = calloc(1, PCM_SIZE);
	if (!buf) {
		perror("Unable to allocate memory");
		return errno;
	}

	struct pcm_thread *pcm_threads;
	pcm_threads = (struct pcm_thread *) calloc(PCM_NUM_BANKS, sizeof (*pcm_threads));
	if (!pcm_threads) {
		perror("Failed to allocate thread memory");
		return errno;
	}

	struct timeval t1, t2;
	float execution_time;

	int r, b;
	for(r = 0; r < PCM_NUM_ROWS; r++) {
		int bank = PCM_R2B(r);
        pcm_thread_add_row(pcm_threads + bank, buf, r);
	}

	for(b = 0; b < PCM_NUM_BANKS; b++) {
		struct pcm_thread * pth = pcm_threads + b;
		pth->fn = read_fn;
	}

	gettimeofday(&t1, NULL);
	ariel_enable();

	pcm_threads_run(pcm_threads, PCM_NUM_BANKS);

	gettimeofday(&t2, NULL);
	execution_time = ((t2.tv_sec * 1000000 + t2.tv_usec) - (t1.tv_sec * 1000000 + t1.tv_usec)) / (float) 1000;
	printf("Time taken: %.2f ms\n", execution_time);

	free(buf);
}

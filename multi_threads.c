#include "pcm.h"

#include <stdio.h>
#include <stdlib.h>

void fn(void* row){
}

int main(int argc, char* argv[]) {
	struct pcm_thread pcm_threads[PCM_NUM_BANKS];

	char* buf = (char *) malloc(sizeof(char) * PCM_SIZE);

	int r, b;
	for(r = 0; r < PCM_NUM_ROWS; r++) {
		int b = r % PCM_NUM_BANKS;
		struct pcm_thread * pth = pcm_threads + b;
		pth->rows[pth->num_rows] = buf + r;
		pth->num_rows++;
	}

	for(b = 0; b < PCM_NUM_BANKS; b++) {
		struct pcm_thread * pth = pcm_threads + b;
		pth->fn = fn;
	}

	pcm_threads_spawn(pcm_threads, PCM_NUM_BANKS);
	free(buf);
}

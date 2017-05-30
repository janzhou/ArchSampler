#include "pcm.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

int main(int argc, char* argv[]) {
	pcm_param(argc, argv);

	int rows[PCM_NUM_ROWS], r;
	for(r = 0; r < PCM_NUM_ROWS; r++) {
		rows[r] = r;
	}

	pcm_rows_shuffle(rows, PCM_NUM_ROWS);
	//pcm_rows_bank_aware_shuffle(rows, PCM_NUM_ROWS);
	for(r = 0; r < PCM_NUM_ROWS; r++) {
		printf("%d\n", rows[r]);
	}
}

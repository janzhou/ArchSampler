#include "define.h"
#include "pcm.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

int main(int argc, char* argv[]) {
	pcm_param(argc, argv, "-s <sample>\n-a bank aware shuffle\n");

	int option;
	int sample = 0;
	int bank_aware_shuffle = 0;

	while ((option = getopt(argc, argv,"s:a:")) != -1) {
		switch (option) {
			case 's' : sample = atoi(optarg);
				   break;
			case 'a' : bank_aware_shuffle = atoi(optarg);
				   break;
			case '?' :
			case 0 : break;
		}
	}

	int rows[PCM_NUM_ROWS], r, banks[PCM_NUM_BANKS], b, max, sum;
	for(r = 0; r < PCM_NUM_ROWS; r++) {
		rows[r] = r;
	}
	for(b = 0; b < PCM_NUM_BANKS; b++) {
		banks[b] = 0;
	}

	switch (bank_aware_shuffle) {
		case 4: pcm_rows_shuffle_random(rows, PCM_NUM_ROWS, PCM_NUM_ROWS - sample);
			printf("Random Shuffle-2\n");
			break;
		case 3:
			break;
		case 2: pcm_rows_bank_aware_shuffle2(rows, PCM_NUM_ROWS);
			printf("Bank-aware Shuffle-2\n");
			break;

		case 1: pcm_rows_bank_aware_shuffle(rows, PCM_NUM_ROWS);
			printf("Bank-aware Shuffle-1\n");
			break;

		case 0:
		default: pcm_rows_shuffle(rows, PCM_NUM_ROWS, PCM_NUM_ROWS - sample);
			 printf("Random Shuffle\n");
			 break;
	}

#ifdef PCM_DEBUG
	{
		int size = sample;
		int start = PCM_NUM_ROWS - size;
		pcm_print_row_shuffle(rows + start, size);
	}
#endif

	for(r = PCM_NUM_ROWS - sample; r < PCM_NUM_ROWS; r++) {
		int row = rows[r];
		banks[PCM_R2B(row)]++;
	}

	for(b = 0, max = 0, sum = 0; b < PCM_NUM_BANKS; b++) {
		if(max < banks[b]) max = banks[b];
		sum += banks[b];
	}

	printf("sampling:\t%f\n", (float)sample/PCM_NUM_ROWS);
	printf("sum/max/avg(%d/%d/%d):\t%f\n", sum, max, sample/PCM_NUM_BANKS, (float)max/(sample/PCM_NUM_BANKS));
}

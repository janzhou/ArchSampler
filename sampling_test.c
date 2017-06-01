#include "pcm.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

int main(int argc, char* argv[]) {
	pcm_param(argc, argv, "-s <sample>\n-a bank aware shuffle\n");

	int option;
	int sample = 0;
	int bank_aware_shuffle = 0;

	while ((option = getopt(argc, argv,"s:a")) != -1) {
		switch (option) {
			case 's' : sample = atoi(optarg);
				   break;
			case 'a' : bank_aware_shuffle = 1;
				   break;
			case '?' :
			case 0 : break;
		}
	}

	int rows[PCM_NUM_ROWS], r, banks[PCM_NUM_BANKS], b, max;
	for(r = 0; r < PCM_NUM_ROWS; r++) {
		rows[r] = r;
	}
	for(b = 0; b < PCM_NUM_BANKS; b++) {
		banks[b] = 0;
	}

	if(bank_aware_shuffle == 0) {
		pcm_rows_shuffle(rows, PCM_NUM_ROWS);
	} else {
		pcm_rows_bank_aware_shuffle(rows, PCM_NUM_ROWS);
	}

	for(r = 0; r < sample; r++) {
		int row = rows[r];
		banks[PCM_R2B(row)]++;
	}

	for(b = 0, max = 0; b < PCM_NUM_BANKS; b++) {
		if(max < banks[b]) max = banks[b];
	}

	printf("sampling:\t%f\n", (float)sample/PCM_NUM_ROWS);
	printf("max/avg(%d/%d):\t%f\n", max, sample/PCM_NUM_BANKS, (float)max/(sample/PCM_NUM_BANKS));
}

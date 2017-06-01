#include "pcm.h"
#include "amazon_movies.h"
#include "arielapi.h"

int main(int argc, char *argv[])
{
	pcm_param(argc, argv);

	int option;
	int sample = PCM_NUM_ROWS;
	int bank_aware_shuffle = 0;
	int contention_free_r2t = 0;

	while ((option = getopt(argc, argv,"s:a")) != -1) {
		switch (option) {
			case 's' : sample = atoi(optarg);
				   break;
			case 'a' : bank_aware_shuffle = 1;
				   break;
			case 'c' : contention_free_r2t = 1;
				   break;
			case '?' : break;
		}
	}

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

	int rows[PCM_NUM_ROWS], r;
	for(r = 0; r < PCM_NUM_ROWS; r++) {
		rows[r] = r;
	}

	if(bank_aware_shuffle == 0) {
		pcm_rows_shuffle(rows, PCM_NUM_ROWS);
	} else {
		pcm_rows_bank_aware_shuffle(rows, PCM_NUM_ROWS);
	}

	if(contention_free_r2t == 0) {
		pcm_r2t_even_split(pcm_threads, PCM_NUM_BANKS, rows, PCM_NUM_ROWS, buf);
	} else {
		pcm_r2t_contention_free(pcm_threads, PCM_NUM_BANKS, rows, sample, buf);
	}

	pcm_threads_map_count_fn(pcm_threads, PCM_NUM_BANKS, amazon_movies_cnt_local);	

	if (amazon_movies_init_mem(buf, "data/movies.txt"))
		return errno;

	ariel_enable();

	pcm_threads_run(pcm_threads, PCM_NUM_BANKS);
	pcm_threads_reduce_count_fn(pcm_threads, PCM_NUM_BANKS, amazon_movies_cnt_global);

	printf("Amazon reviews count: %lu\n", amazon_movies_get_global_cnt());

	free(buf);

	return 0;
}

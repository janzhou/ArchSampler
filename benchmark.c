#include "pcm.h"
#include "amazon_movies.h"
#include "movie.h"
#include "arielapi.h"
#include <assert.h>

unsigned long int read_fn(void* row){
	unsigned long i = 0;
	int *buf = (int *) row;
	int sum = 0;
	for(i = 0; i < PCM_ROW_SIZE/sizeof(int); i++){
		sum += buf[i];
	}

	//printf("%d\n", sum);
	return i;
}

unsigned long write_fn(void* row) {
	unsigned long i = 0;
	int *buf = (int *) row;
	for(i = 0; i < PCM_ROW_SIZE/sizeof(int); i++){
		buf[i] = i;
	}
	return i;
}

int main(int argc, char *argv[])
{
	pcm_param(argc, argv, "-p <repeat>\n-s <sample>\n-a bank aware shuffle\n-c contention free threads\n-w <workload> 1: amazon_movie; 2: movielens; 3: write; 4: read;\n");

	int option;
	int sample = PCM_NUM_ROWS;
	int repeat = 1;
	int bank_aware_shuffle = 0;
	int contention_free_r2t = 0;

	int workload = 0;
	int (* init_mem)(char *mem) = NULL;
	unsigned long (* count_map)(void *row) = NULL;
	void (* count_reduce)(unsigned long local_cnt) = NULL;
	void (* count_reset)() = NULL;
	unsigned long (* count_get)() = NULL;

	while ((option = getopt(argc, argv,"acs:p:w:")) != -1) {
		switch (option) {
			case 'p' : repeat = atoi(optarg);
				   break;
			case 's' : sample = atoi(optarg);
				   break;
			case 'a' : bank_aware_shuffle = 1;
				   break;
			case 'c' : contention_free_r2t = 1;
				   break;
			case 'w' : workload = atoi(optarg);
			case '?' :
			case 0 : break;
		}
	}

	switch(workload) {
		default:
		case 1:
			init_mem = amazon_movies_init_mem;
			count_get = amazon_movies_get_global_cnt;
			count_reduce = amazon_movies_cnt_global;
			count_reset = amazon_movies_reset_global_cnt;
			count_map = amazon_movies_cnt_local;
			break;
		case 2:
			init_mem = pcm_movie_db_init;
			count_map = pcm_movie_db_cnt_local;
			count_reduce = pcm_movie_db_cnt_global;
			count_get = pcm_movie_db_get_global_cnt;
			count_reset = pcm_movie_db_reset_global_cnt;
			break;
		case 3:
			count_map = write_fn;
			break;
		case 4:
			count_map = read_fn;
			break;
		case 5: init_mem = amazon_movies_init_mem;
			count_map = amazon_movies_capitalize_review;
	}

	char *buf;
	buf = calloc(1, PCM_SIZE);
	if (!buf) {
		perror("Unable to allocate memory");
		return errno;
	}

	//struct pcm_thread pcm_threads[PCM_NUM_BANKS];
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

	printf("loading data...\n");
	if(init_mem != NULL) {
		if ((* init_mem)(buf))
			return errno;
	}

	if(bank_aware_shuffle == 0) {
		pcm_rows_shuffle(rows, PCM_NUM_ROWS);
	} else {
		pcm_rows_bank_aware_shuffle(rows, PCM_NUM_ROWS);
	}

	assert(sample * repeat <= PCM_NUM_ROWS);

#ifdef PCM_DEBUG
	pcm_print_row_shuffle(rows);
#endif

	ariel_enable();

	for(; repeat > 0; repeat--){
		int skip = sample * ( repeat - 1 );
		if(contention_free_r2t == 0) {
			pcm_r2t_even_split(pcm_threads, PCM_NUM_BANKS, rows + skip, sample, buf);
		} else {
			pcm_r2t_contention_free(pcm_threads, PCM_NUM_BANKS, rows + skip, sample, buf);
		}

#ifdef PCM_DEBUG
		pcm_print_t2r(sample, rows);
#endif

		int banks[PCM_NUM_BANKS], b, max;
		for(b = 0; b < PCM_NUM_BANKS; b++) {
			banks[b] = 0;
		}
		for(r = 0; r < sample; r++) {
			int row = rows[skip + r];
			banks[PCM_R2B(row)]++;
		}
		for(b = 0, max = 0; b < PCM_NUM_BANKS; b++) {
			if(max < banks[b]) max = banks[b];
		}

		printf("sampling ratio: %f; load max/avg(%d/%d): %f;\n", (float)sample/PCM_NUM_ROWS, max, sample/PCM_NUM_BANKS, (float)max/(sample/PCM_NUM_BANKS));

		if(count_map != NULL) {
			pcm_threads_map_count_fn(pcm_threads, PCM_NUM_BANKS, count_map);	
		}
		pcm_threads_run(pcm_threads, PCM_NUM_BANKS);

		if(count_reset != NULL) {
			(* count_reset)();
		}

		if(count_reduce != NULL) {
			pcm_threads_reduce_count_fn(pcm_threads, PCM_NUM_BANKS, count_reduce);
		}

		if(count_get != NULL) {
			printf("count: %lu\n", (*count_get)());
		}
	}

	free(buf);

	return 0;
}

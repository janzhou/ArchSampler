#include "pcm.h"
#include "amazon_movies.h"
#include "amazon_movies_trim.h"
#include "movie.h"
#include "arielapi.h"
#include <limits.h>
#include <assert.h>

struct pcm_thread pcm_threads[PCM_NUM_BANKS_MAX];

void read_fn(void* row){
	unsigned long i = 0;
	int *buf = (int *) row;
	int sum = 0;
	for(i = 0; i < PCM_ROW_SIZE/sizeof(int); i++){
		sum += buf[i];
	}

	//printf("%d\n", sum);
}

void write_fn(void* row) {
	unsigned long i = 0;
	int *buf = (int *) row;
	for(i = 0; i < PCM_ROW_SIZE/sizeof(int); i++){
		buf[i] = i;
	}
}

void benchmark_sort(int num_threads)
{
	int i;
	int n = PCM_NUM_ROWS * PCM_ROW_SIZE / sizeof(struct amazon_movie_review_trim);

	// Treat odd-even as one phase
	n = n / 2;

	for (i = 0; i < n; i++) {
		pcm_threads_map(pcm_threads, num_threads, sort_even, amazon_movies_trim_merge);
		pcm_threads_reset_func(pcm_threads, num_threads, sort_even);

		pcm_threads_map(pcm_threads, num_threads - 1, sort_odd, amazon_movies_trim_merge);
		pcm_threads_reset_func(pcm_threads, num_threads, sort_odd);
	}
}

int main(int argc, char *argv[])
{
	pcm_param(argc, argv,
	"-p <repeat>\n"
	"-s <sample>\n"
	"-t <num_threads>\n"
	"-W <word_to_count> only work for -w1\n"
	"-a <shuffle pattern> (0: Random shuffle, 1: Bank-aware shuffle-1, 2: Bank-aware shuffle-2, 3: No shuffle)\n"
	"-c contention free threads\n"
	"-w <workload> 1: amazon_movie; 2: movielens; 3: write; 4: read; 5: amazon_movies_capitalize; 6: amazon_movies_sort\n");

	int option;
	int sample = PCM_NUM_ROWS;
	int repeat = 1;
	int bank_aware_shuffle = 0;
	int contention_free_r2t = 0;
	unsigned int num_threads = 0;

	int workload = 0;
	int (* init_mem)(char *mem) = NULL;
	unsigned long (* count_map)(void *row) = NULL;
	void (* fn_map)(void *row) = NULL;
	void (* count_reduce)(unsigned long local_cnt) = NULL;
	void (* count_reset)() = NULL;
	unsigned long (* count_get)() = NULL;
	char * word_to_count = NULL;

	while ((option = getopt(argc, argv,"a:cs:p:w:W:")) != -1) {
		switch (option) {
			case 'p' : repeat = atoi(optarg);
				   break;
			case 's' : sample = atoi(optarg);
				   break;
			case 'a' : bank_aware_shuffle = atoi(optarg);
				   break;
			case 'c' : contention_free_r2t = 1;
				   break;
			case 'w' : workload = atoi(optarg);
				   break;
			case 'W' : word_to_count = optarg;
				   break;
			case 't' : num_threads = atoi(optarg);
				   break;
			case '?' :
			case 0 : break;
		}
	}

	if(num_threads == 0) num_threads = PCM_NUM_BANKS;
	assert(num_threads <= PCM_NUM_BANKS_MAX);

	switch(workload) {
		default:
		case 1:
			init_mem = amazon_movies_init_mem;
			count_get = amazon_movies_get_global_cnt;
			count_reduce = amazon_movies_cnt_global;
			count_reset = amazon_movies_reset_global_cnt;
			count_map = amazon_movies_cnt_local;
			amazon_movies_cnt_word(word_to_count);
			break;
		case 2:
			init_mem = pcm_movie_db_init;
			count_map = pcm_movie_db_cnt_local;
			count_reduce = pcm_movie_db_cnt_global;
			count_get = pcm_movie_db_get_global_cnt;
			count_reset = pcm_movie_db_reset_global_cnt;
			break;
		case 3:
			fn_map = write_fn;
			break;
		case 4:
			fn_map = read_fn;
			break;
		case 5: init_mem = amazon_movies_init_mem;
			count_map = amazon_movies_capitalize_review;
			break;
		case 6: init_mem = amazon_movies_trim_init_mem;
			fn_map = amazon_movies_trim_sort_local;
	}

	char *buf;
	buf = calloc(1, PCM_SIZE);
	if (!buf) {
		perror("Unable to allocate memory");
		return errno;
	}

#ifdef PCM_DEBUG
	printf("buf: %p - %p\n", buf, buf + PCM_SIZE);
#endif

	posix_memalign((void **) &buf, PCM_ROW_SIZE, PCM_SIZE);

#ifdef PCM_DEBUG
	printf("aligned_buf: %p - %p\n", buf, buf + PCM_SIZE);
#endif

//	struct pcm_thread *pcm_threads;
//	pcm_threads = (struct pcm_thread *) calloc(PCM_NUM_BANKS, sizeof (*pcm_threads));
//	if (!pcm_threads) {
//		perror("Failed to allocate thread memory");
//		return errno;
//	}

	int rows[PCM_NUM_ROWS], r;
	for(r = 0; r < PCM_NUM_ROWS; r++) {
		rows[r] = r;
	}

	printf("loading data...\n");
	if(init_mem != NULL) {
		if ((* init_mem)(buf))
			return errno;
	}

	switch (bank_aware_shuffle) {
		case 3:
			break;
		case 2: pcm_rows_bank_aware_shuffle2(rows, PCM_NUM_ROWS);
			printf("Bank-aware Shuffle-2\n");
			break;

		case 1: pcm_rows_bank_aware_shuffle(rows, PCM_NUM_ROWS);
			printf("Bank-aware Shuffle-1\n");
			break;

		case 0:
		default: pcm_rows_shuffle(rows, PCM_NUM_ROWS);
			printf("Random Shuffle\n");
			break;
	}

	assert(sample * repeat <= PCM_NUM_ROWS);

#ifdef PCM_DEBUG
	pcm_print_row_shuffle(rows);
#endif

	ariel_enable();

	unsigned int results[repeat], repeat_loop, result_sum = 0, result_avg, result_min = UINT_MAX, result_max = 0;
	for(repeat_loop = 0; repeat_loop < repeat; repeat_loop++){
		int skip = sample * repeat_loop;
		if(contention_free_r2t == 0) {
			pcm_r2t_even_split(pcm_threads, num_threads, rows + skip, sample, buf);
		} else {
			pcm_r2t_contention_free(pcm_threads, num_threads, rows + skip, sample, buf);
		}

#ifdef PCM_DEBUG
		pcm_thread_print(pcm_threads, num_threads, buf);

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
#endif

		if(count_map != NULL) {
			pcm_threads_map(pcm_threads, num_threads, count_fn, count_map);	
//			pcm_threads_map(pcm_threads, num_threads, fn, amazon_movies_sort_reviews);
		}

		if (fn_map != NULL) {
			pcm_threads_map(pcm_threads, num_threads, fn, fn_map);
		}

		// Sorting
		if (workload == 6) {
			benchmark_sort(num_threads);

			// Debug
			int j;
			struct amazon_movie_review_trim *review;

			for (j = 0; j < PCM_NUM_ROWS; j++) {
				review = (struct amazon_movie_review_trim *) (buf + j * PCM_ROW_SIZE);
				amazon_movies_trim_print(review);
			}
		}

		if(count_reset != NULL) {
			(* count_reset)();
		}

		if(count_reduce != NULL) {
			pcm_threads_reduce(pcm_threads, num_threads, count, count_reduce);
		}

		if(count_get != NULL) {
			unsigned int result = (*count_get)();
			result_sum += result;
			result_min = result_min <= result ? result_min : result;
			result_max = result_max >= result ? result_max : result;
			results[repeat_loop] = result;
			printf("count: %u\n", result);
		}
	}

	if(repeat > 1 && count_get != NULL) {
		result_avg = result_sum/repeat;
		int ratio = PCM_NUM_ROWS / sample;
		printf("result avg/min/max: %u/%u/%u\n", result_avg, result_min, result_max);
		printf("approx avg/min/max: %u/%u/%u\n", result_avg * ratio, result_min * ratio, result_max * ratio);
	}

	free(buf);

	return 0;
}

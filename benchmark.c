#include "define.h"
#include "pcm.h"
#include "amazon_movies.h"
#include "amazon_movies_trim.h"
#include "movie.h"
#include "arielapi.h"
#include "keycnt.h"
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

#define smallest(t,arr, k, n) ({ \
 int pos=k,i; t small=arr[k]; \
 for(i=k+1;i<n;i++) \
 { \
  if(arr[i]<small) \
  { \
   small=arr[i]; \
   pos=i; \
  } \
 } \
 pos; \
})


#define sort(t,arr, n) { \
 int k,pos; t temp; \
 for(k=0 ; k < n ; k++) \
  { \
   pos = smallest(t,arr,k,n); \
   temp=arr[k]; \
   arr[k]=arr[pos]; \
   arr[pos]=temp; \
  } \
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
	"-w <workload> 1: amazon_movie; 2: movielens; 3: write; 4: read; 5: amazon_movies_capitalize;"
	" 6: amazon_movies_sort; 7: amazon_movies_average_ratings; 8: amazon_movies_with_most_reviews\n");

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
	unsigned long (* count_float_fn)(void *row, float *count_float) = NULL;
	void (*cnt_map_fn)(void *row, void **cnt_map_head) = NULL;
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
            if(word_to_count == NULL) {
                init_mem = amazon_movies_init_mem;
            } else {
                init_mem = amazon_movies_init_mem_raw;
            }
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
			break;
		case 7:
			init_mem = amazon_movies_trim_init_mem;
			count_float_fn = amazon_movies_trim_avg_rating_local;
			break;
		case 8:
			init_mem = amazon_movies_trim_init_mem;
			cnt_map_fn = amazon_movies_trim_movie_cnt_map;
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

	assert(sample * repeat <= PCM_NUM_ROWS);

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
		default: pcm_rows_shuffle(rows, PCM_NUM_ROWS, PCM_NUM_ROWS - sample * repeat);
			printf("Random Shuffle\n");
			break;
	}

#ifdef PCM_DEBUG
	pcm_print_row_shuffle(rows);
#endif

	ariel_enable();

	unsigned int results[repeat], repeat_loop, result_sum = 0, result_avg, result_min = UINT_MAX, result_max = 0;
	float avg[repeat], avg_sum = 0, avg_avg, avg_min, avg_max;
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

		if (cnt_map_fn != NULL) {
			pcm_threads_map(pcm_threads, num_threads, cnt_map_fn, cnt_map_fn);
		}

		if(count_map != NULL) {
			pcm_threads_map(pcm_threads, num_threads, count_fn, count_map);	
		}

		if (count_float_fn != NULL) {
			pcm_threads_map(pcm_threads, num_threads, count_float_fn, count_float_fn);
		}

		if (fn_map != NULL) {
			pcm_threads_map(pcm_threads, num_threads, fn, fn_map);
		}

		// Sorting
		if (workload == 6) {
			int sorted = 0;
			while(!sorted) {
				sorted = 1;
				pcm_threads_map(pcm_threads, num_threads, merge_sort, amazon_movies_trim_merge);
				pcm_threads_reduce_opt(pcm_threads, num_threads, sorted, &&, sorted);
			}
#ifdef PCM_DEBUG
			// Debug
			int i, j;
			struct amazon_movie_review_trim *review;

			for (i = 0; i < num_threads; i++) {
				for (j = 0; j < pcm_threads[i].num_rows; j++) {
					review = (struct amazon_movie_review_trim *) pcm_threads[i].rows[j];
					amazon_movies_trim_print(review);
				}
			}
#endif
		}

		// Average
		else if (workload == 7) {
			unsigned long n_elements = 0;
			float ratings_sum = 0;

			pcm_threads_reduce_opt(pcm_threads, num_threads, n_elements, +, count);
			pcm_threads_reduce_opt(pcm_threads, num_threads, ratings_sum, +, count_float);
			avg[repeat_loop] = ratings_sum / n_elements;
			avg_sum += avg[repeat_loop];
			printf("Average rating: %.2f\n", avg[repeat_loop]);
		}

		// Most movie reviews
		else if (workload == 8) {
			struct keycnt_node *max_review_node, *reduced_list;

			reduced_list = keycnt_pcm_threads_reduce(pcm_threads, num_threads);
			max_review_node = keycnt_most_reviews(reduced_list);

			printf("%s: %d\n", max_review_node->key, max_review_node->cnt);
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
			results[repeat_loop] = result;
			printf("count: %u\n", result);
		}
	}

	if(repeat > 1 && count_get != NULL) {
		sort(int, results, repeat);
		result_avg = result_sum/repeat;
		result_min = results[repeat * 2 / 10];
		result_max = results[repeat * 8 / 10];
		int ratio = PCM_NUM_ROWS / sample;
		printf("result min/avg/max: %u %u %u\n", result_min, result_avg, result_max);
		printf("approx min/avg/max: %u %u %u\n", result_min * ratio, result_avg * ratio, result_max * ratio);
	} else if ( repeat > 1 && workload == 7) {
		sort(float, avg, repeat);
		avg_avg = avg_sum/repeat;
		avg_min = avg[repeat * 2 / 10];
		avg_max = avg[repeat * 8 / 10];
		printf("approx min/avg/max: %f %f %f\n", avg_min, avg_avg, avg_max);
	
	}

	free(buf);

	return 0;
}

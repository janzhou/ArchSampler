#include "pcm.h"
#include <time.h>
#include <stdlib.h>

int PCM_NUM_BANKS = PCM_NUM_BANKS_MAX;
int PCM_ROWS_PER_BANK = PCM_ROWS_PER_BANK_MAX;
int PCM_ENABLE_OPENMP = 0;

void pcm_param(int argc, char* argv[]) {
	if (argc >= 3) {
		int banks = atoi(argv[1]);
		int rows = atoi(argv[2]);
		if(banks > PCM_NUM_BANKS_MAX || rows > PCM_ROWS_PER_BANK_MAX) {
			printf("PCM_NUM_BANKS_MAX: %d ; PCM_ROWS_PER_BANK_MAX: %lu\n", PCM_NUM_BANKS_MAX, PCM_ROWS_PER_BANK_MAX);
			exit(-1);
		}
		PCM_NUM_BANKS = banks;
		PCM_ROWS_PER_BANK = rows;
	} else if (argc != 1) {
		printf("Usage: %s <banks> <rows_per_bank>\n", argv[0]);
		exit(-1);
	}

	if (argc >= 4) {
		PCM_ENABLE_OPENMP = atoi(argv[3]);
	}

	printf("PCM_NUM_BANKS: %d;\nPCM_ROWS_PER_BANK: %d;\nPCM_SIZE: %luMB;\n", PCM_NUM_BANKS, PCM_ROWS_PER_BANK, PCM_SIZE/(1024*1024));
}

void *pcm_thread_func(void *data)
{
	struct pcm_thread *pcm_thread = (struct pcm_thread *) data;

	int i;
	for(i = 0; i < pcm_thread->num_rows; i++){
		if(pcm_thread->count_fn != NULL){
			pcm_thread->count += pcm_thread->count_fn(pcm_thread->rows[i]);
		} else if(pcm_thread->fn != NULL){
			pcm_thread->fn(pcm_thread->rows[i]);
		}
	}

	if(!PCM_ENABLE_OPENMP) {
		pthread_exit(0);
	}
}

void pcm_threads_run(struct pcm_thread * pcm_threads, int num_threads){
	int i;
	if(PCM_ENABLE_OPENMP) {
#pragma omp parallel for num_threads(num_threads)
		for(i = 0; i < num_threads; i++){
			pcm_thread_func(pcm_threads + i);
		}
	} else {
		for(i = 0; i < num_threads; i++){
			struct pcm_thread * pth = pcm_threads + i;
			pthread_create(&pth->pthread, NULL, pcm_thread_func, pth);
		}
		for (i = 0; i < num_threads; i++) {
			struct pcm_thread * pth = pcm_threads + i;
			pthread_join(pth->pthread, NULL);
		}
	}
}

void pcm_thread_add_row(struct pcm_thread * pth, void * base, int row) {
    pth->rows[pth->num_rows] = PCM_R2P(base, row);
    pth->num_rows++;
}

void pcm_threads_map_fn(
		struct pcm_thread pcm_threads[], int num_threads,
		void (* fn)(void *row)
		) {
	int i;
	for(i = 0; i < num_threads; i++) {
		pcm_threads[i].fn = fn;
	}
}

void pcm_threads_map_count_fn(
		struct pcm_thread pcm_threads[], int num_threads,
		unsigned long (* count_fn)(void *row)
		) {
	int i;
	for(i = 0; i < num_threads; i++) {
		pcm_threads[i].count_fn = count_fn;
	}
}

void pcm_threads_reduce_count_fn(
		struct pcm_thread pcm_threads[], int num_threads,
		void (* count_reduce)(unsigned long count)
		){
	int i;
	for(i = 0; i < num_threads; i++) {
		count_reduce(pcm_threads[i].count);
	}
}

void pcm_rows_shuffle(int rows[], int num_rows) {
	int r;
	srand(time(NULL));
	for(r = 0; r < num_rows; r++) {
		int swap = rand() % num_rows;
		int tmp = rows[r];
		rows[r] = rows[swap];
		rows[swap] = tmp;
	}
}

void pcm_r2t_contention_free(struct pcm_thread pths[], int num_threads, int rows[], int num_rows, void * buf) {
	int r;
	for(r = 0; r < num_rows; r++) {
		int bank = PCM_R2B(r);
		int t = bank % num_threads;
		pcm_thread_add_row(pths + t, buf, rows[r]);
	}
}

void pcm_r2t_even_split(struct pcm_thread pths[], int num_threads, int rows[], int num_rows, void * buf) {
	int rows_in_thread = num_rows / num_threads;
	int left_rows = num_rows % num_threads;

	int r, t;
	for(t = 0; t < num_threads; t++) {
		for(r = 0; r < rows_in_thread; r++) {
			pcm_thread_add_row(pths + t, buf, rows[t * rows_in_thread + r]);
		}
	}
	for(r = 0; r < left_rows; r++) {
		pcm_thread_add_row(pths + r, buf, rows[num_threads * rows_in_thread + r]);
	}
}

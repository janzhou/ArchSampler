#include "pcm.h"

int PCM_NUM_BANKS = PCM_NUM_BANKS_MAX;
int PCM_ROWS_PER_BANK = PCM_ROWS_PER_BANK_MAX;

void pcm_param(int argc, char* argv[]) {
	if (argc == 3) {
		int banks = atoi(argv[1]);
		int rows = atoi(argv[2]);
		if(banks > PCM_NUM_BANKS_MAX && rows > PCM_ROWS_PER_BANK_MAX) {
			printf("PCM_NUM_BANKS_MAX: %d ; PCM_ROWS_PER_BANK_MAX: %d\n", PCM_NUM_BANKS_MAX, PCM_ROWS_PER_BANK_MAX);
			exit(-1);
		}
		PCM_NUM_BANKS = banks;
		PCM_ROWS_PER_BANK = rows;
	} else if (argc != 1) {
		printf("Usage: %s <banks> <rows_per_bank>\n", argv[0]);
		exit(-1);
	}

	printf("PCM_NUM_BANKS: %d;\nPCM_ROWS_PER_BANK: %d;\nPCM_SIZE: %dMB;\n", PCM_NUM_BANKS, PCM_ROWS_PER_BANK, PCM_SIZE/(1024*1024));
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

	pthread_exit(0);
}

void pcm_threads_spawn(struct pcm_thread * pcm_threads, int num_threads){
	int i;
	for(i = 0; i < num_threads; i++){
		struct pcm_thread * pth = pcm_threads + i;
		pthread_create(&pth->pthread, NULL, pcm_thread_func, pth);
	}
}

void pcm_threads_join(struct pcm_thread pcm_threads[], int num_threads)
{
	int i;

	for (i = 0; i < num_threads; i++) {
		struct pcm_thread * pth = pcm_threads + i;
		pthread_join(pth->pthread, NULL);
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



#ifndef PCM_H_
#define PCM_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>

extern int PCM_NUM_BANKS;
extern int PCM_ROWS_PER_BANK;

#define PCM_NUM_BANKS_MAX	32
#define PCM_ROWS_PER_BANK_MAX	((unsigned long) (1024 * 256))
#define PCM_ROW_SIZE		((unsigned long) (1024 * 8))
#define PCM_BANK_SIZE		((unsigned long) PCM_ROWS_PER_BANK * PCM_ROW_SIZE)
#define PCM_NUM_ROWS		((unsigned long) PCM_NUM_BANKS * PCM_ROWS_PER_BANK)
#define PCM_SIZE			((unsigned long) PCM_NUM_ROWS * PCM_ROW_SIZE)

#define PCM_OFF(base, p)    ((char*)p - (char *)base)
#define PCM_P2R(base, p)    (PCM_OFF(base, p) / PCM_ROW_SIZE)
#define PCM_P2B(base, p)    PCM_R2B(PCM_P2R(base, p))
#define PCM_R2B(r)          (r % PCM_NUM_BANKS)
#define PCM_R2P(base, r)    ((char *)base + (r * PCM_ROW_SIZE))

//#define PCM_DEBUG

struct pcm_thread {
	int thread_id;
	int num_threads;
	pthread_t pthread;
	int (* merge_sort)(void *left, void *right);
	int sorted;
	unsigned long (* count_fn)(void *row);
	unsigned long count;
	unsigned long (* count_float_fn)(void *row, float *count_float);
	float count_float;
	void (* fn)(void *row);
	void (*cnt_map_fn)(void *row, void **cnt_map_head);
	void *cnt_map_head;
	unsigned long long num_rows;
	void * rows[PCM_ROWS_PER_BANK_MAX];
};

void pcm_thread_print(struct pcm_thread pcm_threads[], int num_threads, char* base);

void pcm_param(int argc, char* argv[], char* usage);

void pcm_thread_add_row(struct pcm_thread * pth, void * base, int row);

#define pcm_threads_map(pcm_threads, num_threads, call_fn, call_ptr) \
{ \
	int i; \
	for(i = 0; i < num_threads; i++) { \
		pcm_threads[i].thread_id = i; \
		pcm_threads[i].num_threads = num_threads; \
		pcm_threads[i].merge_sort = NULL; \
		pcm_threads[i].sorted = 1; \
		pcm_threads[i].count_fn = NULL; \
		pcm_threads[i].count_fn = NULL; \
		pcm_threads[i].count = 0; \
		pcm_threads[i].fn = NULL; \
		pcm_threads[i].call_fn = call_ptr; \
	} \
	pcm_threads_run(pcm_threads, num_threads); \
}

#define pcm_threads_reduce(pcm_threads, num_threads, call_val, call_ptr) \
{ \
	int i; \
	for(i = 0; i < num_threads; i++) { \
		(call_ptr)(pcm_threads[i].call_val); \
	} \
}

#define pcm_threads_reduce_opt(pcm_threads, num_threads, reduce_val, opt, opt_val) \
{ \
	int i; \
	for(i = 0; i < num_threads; i++) { \
		reduce_val = reduce_val opt pcm_threads[i].opt_val; \
	} \
}

void pcm_threads_run(struct pcm_thread pcm_threads[], int num_threads);
void pcm_rows_shuffle(int rows[], int num_rows);
void pcm_rows_bank_aware_shuffle(int rows[], int num_rows);
void pcm_rows_bank_aware_shuffle2(int rows[], int num_rows);

/* assign rows to threads */
void pcm_r2t_contention_free(struct pcm_thread pths[], int num_threads, int rows[], int num_rows, void * buf);
void pcm_r2t_even_split(struct pcm_thread pths[], int num_threads, int rows[], int num_rows, void * buf);

void pcm_print_row_shuffle(int rows[]);


#endif /* PCM_H_ */

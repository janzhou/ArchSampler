#include "pcm.h"

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
}

void pcm_threads_spawn(struct pcm_thread * pcm_threads, int num_threads)
{
	struct pcm_threads *pth;

	omp_set_num_threads(num_threads);

#pragma omp parallel private(pth)
	{
		pth = (struct pcm_threads *) (pcm_threads + omp_get_thread_num());
		pcm_thread_func(pth);
	}
}

void pcm_threads_join(struct pcm_thread pcm_threads[], int num_threads)
{

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



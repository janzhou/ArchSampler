#include "pcm.h"

void *pcm_thread_func(void *data)
{
	struct pcm_thread *pcm_thread = (struct pcm_thread *) data;

	int i;
	for(i = 0; i < pcm_thread->num_rows; i++){
		pcm_thread->fn(pcm_thread->rows[i]);
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

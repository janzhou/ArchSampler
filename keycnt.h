#ifndef KEYVAL_H_
#define KEYVAL_H_

#include "pcm.h"

struct keycnt_node {
	char key[16];
	int cnt;

	struct keycnt_node *next;
};

void keycnt_add(struct keycnt_node **head, char *key, int cnt);
struct keycnt_node *keycnt_pcm_threads_reduce(struct pcm_thread *pcm_threads, int num_threads);
struct keycnt_node *keycnt_most_reviews(struct keycnt_node *head);
void keycnt_print(struct keycnt_node *head);

#endif /* KEYVAL_H_ */

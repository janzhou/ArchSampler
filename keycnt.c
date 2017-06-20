#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "keycnt.h"

static struct keycnt_node *keycnt_new(char *key, int cnt)
{
	struct keycnt_node *new_node;

	new_node = calloc(1, sizeof(*new_node));
	if (!new_node)
		return;

	new_node->next = NULL;
	new_node->cnt = cnt;
	strcpy(new_node->key, key);

	return new_node;
}

void keycnt_add(struct keycnt_node **head, char *key, int cnt)
{
	struct keycnt_node *prev;
	struct keycnt_node *cur = *head;

	// List not yet formed
	if (!cur) {
		*head = keycnt_new(key, cnt);
		return;
	}

	while (cur) {
		if (!strcmp(key, cur->key)) {
			cur->cnt += cnt;
			return;
		}

		prev = cur;
		cur = cur->next;
	}

	prev->next = keycnt_new(key, cnt);
}

struct keycnt_node *keycnt_pcm_threads_reduce(struct pcm_thread *pcm_threads, int num_threads)
{
	int i;
	struct keycnt_node *cur;
	struct keycnt_node *merger_list = pcm_threads[0].cnt_map_head;

	// Merge all the lists into a single list
	for (i = 1; i < num_threads; i++) {
		cur = pcm_threads[i].cnt_map_head;

		while (cur) {
			keycnt_add(&merger_list, cur->key, cur->cnt);
			cur = cur->next;

			// TODO:: Free the list as and when you pass
		}
	}

	return merger_list;
}

struct keycnt_node *keycnt_most_reviews(struct keycnt_node *head)
{
	struct keycnt_node *most_reviews = head;
	struct keycnt_node *cur = head;

	while (cur) {
		if (cur->cnt > most_reviews->cnt)
			most_reviews = cur;

		cur = cur->next;
	}

	return most_reviews;
}

void keycnt_print(struct keycnt_node *head)
{
	while (head) {
		printf("%s: %d\n", head->key, head->cnt);
		head = head->next;
	}
}

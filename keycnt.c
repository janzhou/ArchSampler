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

void keycnt_print(struct keycnt_node *head)
{
	while (head) {
		printf("%s: %d\n", head->key, head->cnt);
		head = head->next;
	}
}

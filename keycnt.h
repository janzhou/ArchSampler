#ifndef KEYVAL_H_
#define KEYVAL_H_

struct keycnt_node {
	char key[16];
	int cnt;

	struct keycnt_node *next;
};

void keycnt_add(struct keycnt_node **head, char *key, int cnt);
void keycnt_print(struct keycnt_node *head);

#endif /* KEYVAL_H_ */

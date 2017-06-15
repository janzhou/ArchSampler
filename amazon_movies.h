#ifndef AMAZON_MOVIES_H_
#define AMAZON_MOVIES_H_

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

struct amazon_movie_review
{
	char product_id[16];
	char user_id[32];
	char profile_name[64];
	char helpfulness[8];
	float score;
	unsigned long time;
	char summary[256];
	char text[3704];
};

int amazon_movies_init_mem(char *mem);
void amazon_movies_cnt_word(char *word_to_count);
unsigned long amazon_movies_cnt_local(void *row);
void amazon_movies_cnt_global(unsigned long local_cnt);
unsigned long amazon_movies_get_global_cnt();
void amazon_movies_reset_global_cnt();
unsigned long amazon_movies_capitalize_review(void *row);

#endif /* AMAZON_MOVIES_H_ */

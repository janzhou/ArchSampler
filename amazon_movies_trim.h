#ifndef AMAZON_MOVIES_TRIM_H_
#define AMAZON_MOVIES_TRIM_H_

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

struct amazon_movie_review_trim
{
	char product_id[16];
	char user_id[32];
	char profile_name[64];
	char helpfulness[8];
	float score;
	unsigned long time;
};

int amazon_movies_trim_init_mem(char *mem);

#endif /* AMAZON_MOVIES_TRIM_H_ */

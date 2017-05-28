#include "pcm.h"
#include "amazon_movies.h"

sem_t n_ele_lock;
unsigned long n_elements;

//#define AMAZON_MOVIES_DEBUG

int amazon_movies_init_mem(char *mem, char *file)
{
	int i, j;	
	char *buff = NULL;
	size_t len;
	struct amazon_movie_review *review;
	int n_reviews_per_row = PCM_ROW_SIZE / sizeof(struct amazon_movie_review);

	if (!mem)
		return -ENOMEM;

	FILE *fp = fopen(file, "r");
	if (!fp) {
		perror("Unable to open input file");
		return errno;
	}

	sem_init(&n_ele_lock, 0, 1);

	for (j = 0; j < PCM_NUM_ROWS; j++) {

		review = (struct amazon_movie_review *) (mem + j * PCM_ROW_SIZE);
	
		for (i = 0; i < n_reviews_per_row; i++) {

			getline(&buff, &len, fp);
			sscanf(buff, "product/productId: %[^\n]s", review->product_id);

			getline(&buff, &len, fp);
			sscanf(buff, "review/userId: %[^\n]s", review->user_id);

			getline(&buff, &len, fp);
			sscanf(buff, "review/profileName: %[^\n]s", review->profile_name);

			getline(&buff, &len, fp);
			sscanf(buff, "review/helpfulness: %[^\n]s", review->helpfulness);

			getline(&buff, &len, fp);
			sscanf(buff, "review/score: %f\n", &review->score);

			getline(&buff, &len, fp);
			sscanf(buff, "review/time: %lu\n", &review->time);

			getline(&buff, &len, fp);
			sscanf(buff, "review/summary: %[^\n]s", review->summary);

			getline(&buff, &len, fp);
			sscanf(buff, "review/text: %[^\n]s", review->text);

			getline(&buff, &len, fp);

			if (feof(fp))
				break;

			review++;
		}
	}

	free (buff);
	fclose(fp);

#ifdef AMAZON_MOVIES_DEBUG
	for (j = 0; j < PCM_NUM_ROWS; j++) {

		review = (struct amazon_movie_review *) (mem + j * PCM_ROW_SIZE);

		for (i = 0; i < n_reviews_per_row; i++) {

			if (!strcmp(review->product_id, ""))
				return 0;

			printf("%s::%s::%s::%s::%.1f::%lu::%s::%s\n\n\n",
					review->product_id, review->user_id, review->profile_name,
					review->helpfulness, review->score, review->time,
					review->summary, review->text);

			review++;
		}
	}
#endif	

	return 0;
}

void amazon_movies_cnt_global(unsigned long local_cnt)
{
	sem_wait(&n_ele_lock);
	n_elements += local_cnt;
	sem_post(&n_ele_lock);
}

unsigned long amazon_movies_get_global_cnt()
{
	unsigned long cnt;

	sem_wait(&n_ele_lock);
	cnt = n_elements;
	sem_post(&n_ele_lock);

	return cnt;
}

unsigned long amazon_movies_cnt_local(void *row)
{
	unsigned long i;
	struct amazon_movie_review *review = (struct amazon_movie_review *) row;
	int n_reviews_per_row = PCM_ROW_SIZE / sizeof(struct amazon_movie_review);

	for (i = 0; i < n_reviews_per_row; i++) {

		if (!strcmp(review->product_id, ""))
			break;

		review++;
	}

	return i;
}

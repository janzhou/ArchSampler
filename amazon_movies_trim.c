#include "pcm.h"
#include "amazon_movies_trim.h"

//#define AMAZON_MOVIES_TRIM_DEBUG

int amazon_movies_trim_init_mem(char *mem)
{
	int i, j;
	char *buff = NULL;
	char *file = "data/movies.txt";
	size_t len;
	struct amazon_movie_review_trim *review;
	int n_reviews_per_row = PCM_ROW_SIZE / sizeof(*review);

	if (!mem)
		return -ENOMEM;

	FILE *fp = fopen(file, "r");
	if (!fp) {
		perror("Unable to open input file");
		return errno;
	}

	for (j = 0; j < PCM_NUM_ROWS; j++) {

		review = (struct amazon_movie_review_trim *) (mem + j * PCM_ROW_SIZE);

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

			/* Ignore the summary, review text and the new line from the record */
			getline(&buff, &len, fp);
			getline(&buff, &len, fp);
			getline(&buff, &len, fp);

			if (feof(fp))
				break;

			review++;
		}
	}

	free (buff);
	fclose(fp);

#ifdef AMAZON_MOVIES_TRIM_DEBUG
	for (j = 0; j < PCM_NUM_ROWS; j++) {

		review = (struct amazon_movie_review_trim *) (mem + j * PCM_ROW_SIZE);

		for (i = 0; i < n_reviews_per_row; i++) {

			if (!strcmp(review->product_id, ""))
				return 0;

			printf("%s::%s::%s::%s::%.1f::%lu\n\n\n",
					review->product_id, review->user_id, review->profile_name,
					review->helpfulness, review->score, review->time);

			review++;
		}
	}
#endif

	return 0;
}

/* Sorting application */
void amazon_movies_trim_swap(
		struct amazon_movie_review_trim *review1,
		struct amazon_movie_review_trim *review2)
{
	struct amazon_movie_review_trim temp_review;

	memset(&temp_review, 0, sizeof(temp_review));

	memcpy(&temp_review, review1, sizeof(*review1));
	memcpy(review1, review2, sizeof(*review1));
	memcpy(review2, &temp_review, sizeof(*review1));
}

int partition(
		struct amazon_movie_review_trim *review, int l, int h)
{
	int i, j;
	unsigned long pivot = review[h].time;

	for (j = l; j <= h - 1; j++) {
		if (review[j].time <= pivot) {
			i++;
			amazon_movies_trim_swap(&review[i], &review[j]);
		}
	}

	amazon_movies_trim_swap(&review[i], &review[j]);
	return i + 1;
}

void amazon_movies_trim_quick_sort(
		struct amazon_movie_review_trim *review, int l, int r)
{
	int j;

	if (l < r) {
		j = partition(review, l, r);
		amazon_movies_trim_quick_sort(review, l, j - 1);
		amazon_movies_trim_quick_sort(review, j + 1, r);
	}
}

void amazon_movies_trim_sort_local(void *row)
{
	struct amazon_movie_review_trim *review;
	int n = PCM_ROW_SIZE / sizeof(*review);

	amazon_movies_trim_quick_sort(review, 0, n - 1);
}

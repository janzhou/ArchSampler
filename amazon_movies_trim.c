#include "pcm.h"
#include "amazon_movies_trim.h"
#include "keycnt.h"

//#define AMAZON_MOVIES_TRIM_DEBUG

void amazon_movies_trim_print(struct amazon_movie_review_trim *review)
{
	int i;
	int n_reviews_per_row = PCM_ROW_SIZE / sizeof(*review);

	for (i = 0; i < n_reviews_per_row; i++) {

		// if (!strcmp(review->product_id, "")) {
		// 	printf("review->product_id == \"\"\n");
		// 	return;
		// }

		printf("%lu\n", review->time);

		review++;
	}
}

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
		amazon_movies_trim_print(review);
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

	temp_review = *review1;
	*review1 = *review2;
	*review2 = temp_review;
}

int partition(
		struct amazon_movie_review_trim *review, int l, int h)
{
	int j, i = l - 1;
	unsigned long pivot = review[h].time;

	for (j = l; j <= h - 1; j++) {
		if (review[j].time <= pivot) {
			i++;
			amazon_movies_trim_swap(&review[i], &review[j]);
		}
	}

	amazon_movies_trim_swap(&review[i + 1], &review[h]);
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
	struct amazon_movie_review_trim *review = row;
	int n = PCM_ROW_SIZE / sizeof(*review);

	amazon_movies_trim_quick_sort(review, 0, n - 1);
}

int amazon_movies_trim_merge(void *left, void *right)
{
	struct amazon_movie_review_trim *review_l = left;
	struct amazon_movie_review_trim *review_r = right;

	int i, sorted = 1;
	int n = PCM_ROW_SIZE / sizeof(*review_l);

	if (!(left && right))
		return sorted;

	for (i = n - 1; i >= 0; i--) {
		int j;
		unsigned long last = review_l[n - 1].time;

		for (j = n - 2; j >= 0 && review_l[j].time > review_r[i].time; j--) {
			review_l[j + 1] = review_l[j];
		}

		if (j != n - 2 || last > review_r[i].time) {
			review_l[j + 1] = review_r[i];
			review_r[i].time = last;
			sorted = 0;
		}
	}

	return sorted;
}

unsigned long amazon_movies_trim_avg_rating_local(void *row, float *rating_sum_out)
{
	int i;
	struct amazon_movie_review_trim *review = row;
	unsigned int n_reviews_per_row = PCM_ROW_SIZE / sizeof(*review);
	unsigned long num_reviews = 0;

	*rating_sum_out = 0;

	for (i = 0; i < n_reviews_per_row; i++) {
		if (!strcmp(review->product_id, ""))
			return;

		*rating_sum_out += review->score;
		num_reviews++;

		review++;
	}

	return num_reviews;
}

void amazon_movies_trim_movie_cnt_map(void *row, void **keycnt_head)
{
	int i;
	struct amazon_movie_review_trim *review = row;
	unsigned int n_reviews_per_row = PCM_ROW_SIZE / sizeof(*review);

	for (i = 0; i < n_reviews_per_row; i++, review++) {
		if (!strcmp(review->product_id, ""))
			continue;
		keycnt_add((struct keycnt_node **) keycnt_head, review->product_id, 1);
	}
}

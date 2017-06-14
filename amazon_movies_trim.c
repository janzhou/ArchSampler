#include "pcm.h"
#include "amazon_movies_trim.h"

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

#ifdef AMAZON_MOVIES_DEBUG
	for (j = 0; j < PCM_NUM_ROWS; j++) {

		review = (struct amazon_movie_review *) (mem + j * PCM_ROW_SIZE);

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

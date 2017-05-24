#include "pcm.h"

//#define MOVIE_DB_LOAD_DEBUG

int pcm_movie_db_init(char *buf, char *file)
{
	struct movie_db db;
	unsigned int i, j;
	unsigned int n_db_ele_per_row = PCM_ROW_SIZE / sizeof(db);
	char line[512];
	struct movie_db *location;

	FILE *fp = fopen(file, "r");
	if (!fp)
		return errno;

	// No use of recording the header
	fgets(line, sizeof(line), fp);

	for (i = 0; i < PCM_NUM_ROWS; i++) {

		location = (struct movie_db *) (buf + i * PCM_ROW_SIZE);

		for (j = 0; j < n_db_ele_per_row; j++) {

			if (fgets(line, sizeof(line), fp) == NULL)
				goto out;

			sscanf(line, "%u,%u,%f,%ld", &db.user_id, &db.movie_id, &db.rating, &db.timestamp);

			memcpy(location, &db, sizeof(db));

			location++;
		}
	}

out:

#ifdef MOVIE_DB_LOAD_DEBUG
	for (i = 0; i < PCM_NUM_ROWS; i++) {

		location = (struct movie_db *) (buf + i * PCM_ROW_SIZE);

		for (j = 0; j < n_db_ele_per_row; j++) {

			struct movie_db *db = location;

			if (db->user_id == 0 || db->movie_id == 0)
				return 0;

			printf("id: %u, mid: %u, rating: %.2f, time: %ld\n", db->user_id, db->movie_id, db->rating, db->timestamp);

			location++;
		}
	}
#endif

	return 0;
}

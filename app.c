#include "pcm.h"

sem_t n_ele_lock;
unsigned long n_elements;

//#define MOVIE_DB_LOAD_DEBUG

int pcm_movie_db_init(char *buf, char *file)
{
	struct movie_db db;
	unsigned int i, j;
	unsigned int n_db_ele_per_row = PCM_ROW_SIZE / sizeof(db);
	char line[512];
	struct movie_db *location;

	sem_init(&n_ele_lock, 0, 1);

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

			sscanf(line, "%u,%u,%f,%lu", &db.user_id, &db.movie_id, &db.rating, &db.timestamp);

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

			printf("id: %u, mid: %u, rating: %.2f, time: %lu\n", db->user_id, db->movie_id, db->rating, db->timestamp);

			location++;
		}
	}
#endif

	return 0;
}

void pcm_movie_db_cnt_global(unsigned long local_cnt)
{
	sem_wait(&n_ele_lock);
	n_elements += local_cnt;
	sem_post(&n_ele_lock);
}

unsigned long pcm_movie_db_get_global_cnt()
{
	unsigned long cnt;

	sem_wait(&n_ele_lock);
	cnt = n_elements;
	sem_post(&n_ele_lock);

	return cnt;
}

unsigned long pcm_movie_db_cnt_local(void *row)
{
	struct movie_db *db = (struct movie_db *) row;
	unsigned int n_db_ele_per_row = PCM_ROW_SIZE / sizeof(*db);
	unsigned long i;

	for (i = 0; i < n_db_ele_per_row; i++) {
		if (db->user_id == 0 || db->movie_id == 0)
			break;

		db++;
	}

	return i;
}

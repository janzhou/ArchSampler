#ifndef APP_H_
#define APP_H_

struct movie_db {
	unsigned int user_id;
	unsigned int movie_id;
	float rating;
	unsigned long timestamp;
};

int pcm_movie_db_init(char *buf);
unsigned long pcm_movie_db_cnt_local(void *row);
void pcm_movie_db_cnt_global(unsigned long local_cnt);
unsigned long pcm_movie_db_get_global_cnt();
void pcm_movie_db_reset_global_cnt();

#endif /* APP_H_ */

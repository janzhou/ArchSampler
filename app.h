#ifndef APP_H_
#define APP_H_

struct movie_db {
	unsigned int user_id;
	unsigned int movie_id;
	float rating;
	unsigned long timestamp;
};

int pcm_movie_db_init(char *buf, char *file);

#endif /* APP_H_ */

#ifndef __COMMON__
#define __COMMON__

#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>

extern char *message_buf;
extern pthread_mutex_t message_pthread_lock;
extern pthread_cond_t message_pthread_cond;

#define message_buf_lock_timeout(timeout_sec)				\
	do {								\
		struct timeval now;					\
		struct timespec timeout;				\
									\
		gettimeofday(&now, NULL);				\
		timeout.tv_sec = now.tv_sec + timeout_sec;		\
		timeout.tv_nsec = now.tv_usec * 1000;			\
									\
		pthread_mutex_lock(&message_pthread_lock);		\
		if (message_buf == NULL)				\
			pthread_cond_timedwait(&message_pthread_cond,	\
					       &message_pthread_lock,	\
					       &timeout);		\
		if (message_buf == NULL) {				\
			pthread_mutex_unlock(&message_pthread_lock);	\
			CU_ASSERT_TRUE_FATAL(false);			\
		}							\
	} while (0)

#define message_buf_unlock()						\
	do {								\
		free(message_buf);					\
		message_buf = NULL;					\
		pthread_mutex_unlock(&message_pthread_lock);		\
	} while (0)

#define message_buf_clear()						\
	do {								\
		pthread_mutex_lock(&message_pthread_lock);		\
		free(message_buf);					\
		message_buf = NULL;					\
		pthread_mutex_unlock(&message_pthread_lock);		\
	} while (0)

#define message_buf_wait_status(s, sec)					\
	do {								\
		struct timeval time0, time1;				\
		gettimeofday(&time0, NULL);				\
									\
		while (1) {						\
			gettimeofday(&time1, NULL);			\
			if ((time1.tv_sec - time0.tv_sec) * 1000000 +	\
			    time1.tv_usec - time0.tv_usec >		\
			    sec * 1000000) {				\
				CU_ASSERT_TRUE_FATAL(false);		\
				break;					\
			}						\
									\
			message_buf_lock_timeout(sec);			\
			if (strstr(message_buf, s)) {			\
				message_buf_unlock();			\
				break;					\
			}						\
			message_buf_unlock();				\
		}							\
	} while (0)

int stop_wifi_mode(void);
int stop_wifi_mode_and_clear(void);

#endif	/* __COMMON__ */

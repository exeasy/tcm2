#ifndef _MY_PTHREAD_H
#define _MY_PTHREAD_H
#include <pthread.h>

int     value;
int     wakeup;

void    consumer_wait( int num );
void    producer_wait();
void    producer_notify_consumer( int num );
void    consumer_notify_producer();
pthread_mutex_t mutex_pro;
pthread_cond_t cond;
pthread_cond_t cond_pro;
pthread_mutex_t mutex;

#endif

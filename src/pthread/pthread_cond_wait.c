#include "my_pthread.h"
#include "public.h"
#include "public_include.h"

void	producer_notify_consumer( int num )
{
	int	dosignal = 0;

	pthread_mutex_lock( &mutex );
	value++;

	if( num == value )
	{
		dosignal = 1;
	}

	if( 1 == dosignal )
	{
		pthread_cond_signal( &cond );
	}
	
	pthread_cond_wait(&cond_pro, &mutex);
	pthread_mutex_unlock( &mutex );
}

void	consumer_notify_producer()
{
	pthread_mutex_lock( &mutex );
	pthread_cond_broadcast( &cond_pro );
	pthread_mutex_unlock( &mutex );

	return ;
}

void	consumer_wait( int num )
{
	pthread_mutex_lock( &mutex );

	while( value < num )
	{
		WriteLog( DEBUG, "[%s][%d]tid=[%x]consumer: waiting...\n", __FILE__, __LINE__, pthread_self() );
		pthread_cond_wait( &cond, &mutex );
	}

	WriteLog( DEBUG, "[%s][%d]tid=[%x]consumer:get the product now\n", __FILE__, __LINE__, pthread_self() );
	WriteLog( DEBUG, "[%s][%d]tid=[%x]consumer:value=[%d]\n", __FILE__, __LINE__, pthread_self(), value );
	value = value - num;
	WriteLog( DEBUG, "[%s][%d]tid=[%x]consumer:value=[%d]\n", __FILE__, __LINE__, pthread_self(), value );

	pthread_mutex_unlock( &mutex );

	return ;
}

void	producer_wait()
{
	WriteLog( NORMT, "[%s][%d]tid=[%x]produer:I am waiting now\n", __FILE__, __LINE__, pthread_self() );

	pthread_mutex_lock( &mutex_pro );
	pthread_cond_wait( &cond_pro, &mutex_pro );
	pthread_mutex_unlock( &mutex_pro );

	WriteLog( NORMT, "[%s][%d]tid=[%x]produer:I am starting now\n", __FILE__, __LINE__, pthread_self() );

	return;
}

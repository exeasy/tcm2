/*
 *	queue.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>

#include "queue.h"

/**
 * 初始化一个队列
 * @param input: 
	int typesize - 数据类型大小,单位字节
	int len -  循环队列数组大小
 * @param output:
	none.
 * @return 
	成功时返回MyQueue类型的数据
	失败时返回NULL
 **/

MyQueue *QueueCreate( int typesize, int len )
{
	MyQueue *p = NULL;
	int length = 0;

	if( typesize == 0 || len == 0 )
	{
		printf( "[%s][%d]input param error\n", __FILE__, __LINE__ );
		return NULL;
	}

	p = (MyQueue *)malloc( sizeof(MyQueue) );
	if( p == NULL )
	{
		printf( "[%s][%d]malloc error\n", __FILE__, __LINE__ );
		return NULL;
	}

	length = typesize*len;
	p->queue = (char *)malloc( length );
	if( p->queue == NULL )
	{
		printf( "[%s][%d]malloc error\n", __FILE__, __LINE__ );
		return NULL;
	}
	memset( p->queue, 0x00, sizeof(length) );

	p->front = p->rear = 0;
	p->length = 0;
	p->maxsize = len;
	p->typesize = typesize;
	
	return p;
}

/**
 * 销毁一个已经存在的队列
 * @param input: 
	MyQueue *queue	:已经创建好的循环队列
 * @param output:
	none.
 * @return 
	成功时返回MY_SUCCESS
 **/

int	QueueDestory( MyQueue *queue )
{
	//判断队列是否初始化
	if( !isQueueInit(queue) )
	{
		printf( "[%s][%d]queue is not init\n", __FILE__, __LINE__ );
		return MY_ERR;
	}
	
	free( queue->queue );
	free( queue );

	return MY_SUCCESS;
}

/**
 * 判断队列是否被初始化
 * @param input: 
	MyQueue *queue - 已经创建好的循环队列
 * @param output:
	none.
 * @return 
	成功时返回TRUE
	失败时返回FAIL	
 **/

int isQueueInit( MyQueue *queue )
{
	int flag = FAIL;
	if( queue == NULL )
	{
		printf( "[%s][%d]input param error\n", __FILE__, __LINE__ );
		return flag;
	}

	if( queue->queue != NULL ) 
	{
		flag = TRUE;
	}

	return flag;
}

/**
 * 判断队列是否为空
 * @param input: 
	MyQueue *queue - 已经创建好的循环队列
 * @param output:
	none.
 * @return 
	成功时返回TRUE
	失败时返回FAIL	
 **/

int isQueueEmpty( MyQueue *p )
{
	int	flag;
	flag = FAIL;

	if( !isQueueInit(p) )
	{
		printf( "[%s][%d]queue do not init\n", __FILE__, __LINE__ );
		return flag;
	}

	if( p->front == p->rear )
	{
		flag = TRUE;
	}

	return flag;
}

/**
 * 判断队列是否满
 * @param input: 
	MyQueue *queue - 已经创建好的循环队列
 * @param output:
	none.
 * @return 
	成功时返回TRUE
	失败时返回FAIL	
 */

int isQueueFull( MyQueue *p )
{
	int	flag = FAIL;
	int	typesize = 0; 
	int	maxsize = 0;
	int	offset = 0;

	if( !isQueueInit(p) )
	{
		printf( "[%s][%d]the queue donot init\n", __FILE__, __LINE__ );
		return flag;
	}

	typesize = p->typesize;
	maxsize = p->maxsize*typesize;
	offset = typesize * 1;

	if( (p->rear + offset)%(maxsize) == p->front )
	{
		flag = TRUE;
	}

	return flag;
}

/**
 * 得到队列元素个数
 * @param input: 
	MyQueue *queue - 已经创建好的循环队列
 * @param output:
	none.
 * @return 
	成功时返回元素个数
	失败时返回0
 */

int GetQueueNum( MyQueue *p )
{
	if( !isQueueInit(p) )
	{
		printf( "[%s][%d]the queue donot init\n", __FILE__, __LINE__ );
		return 0;
	}

	return p->length;
}

/**
 * 入队操作函数
 * @param input: 
	MyQueue *queue - 已经创建好的循环队列
 * @param output:
	none.
 * @return 
	入队成功时返回TRUE
	入队失败时返回FAIL	
 */

int	enQueue( MyQueue *p, char *element )
{
	int	typesize = 0;
	int	maxsize = 0;
	int	offset = 0;

	if( !isQueueInit(p) || element == NULL )
	{
		printf( "[%s][%d]the queue donot init\n", __FILE__, __LINE__ );
		return MY_ERR;
	}

	if( isQueueFull(p) )
	{
		printf( "[%s][%d]queue is full\n", __FILE__, __LINE__ );
		return MY_ERR_Q_FULL;
	}

	//pthread_mutex_lock( &(p->mutex) );

	typesize = p->typesize;
	maxsize = p->maxsize * typesize;
	offset = typesize;

	//printf( "[%s][%d]elem=[%d]\n", __FILE__, __LINE__, *((int *)element) );
	memcpy( p->queue+p->rear, element, typesize );
	p->rear = (p->rear+offset)%(maxsize);
	
	p->length++;

	//pthread_mutex_unlock( &(p->mutex) );

	return MY_SUCCESS;
}

/**
 * 出队操作函数
 * @param input: 
	MyQueue *queue - 已经创建好的循环队列
 * @param output:
	none.
 * @return 
	成功时返回队首元素
	失败时返回NULL
 */

char	*DeQueue( MyQueue *p )
{
	int	typesize = 0;
	int	maxsize = 0;
	int	offset = 0;
	char	*element = NULL;

	if( !isQueueInit(p) )
	{
		printf( "[%s][%d]the queue donot init\n", __FILE__, __LINE__ );
		return NULL;
	}

	if( isQueueEmpty(p) )
	{
		printf( "[%s][%d]the queue is empty\n", __FILE__, __LINE__ );
		return NULL;
	}

//	pthread_mutex_lock( &(p->mutex) );

	typesize = p->typesize;
	maxsize = p->maxsize*typesize;
	offset = typesize;

	element = p->queue+p->front;
	p->front = (p->front+offset)%(maxsize);
	p->length--;
//	pthread_mutex_unlock( &(p->mutex) );

	return element;
}

/**
 * 取队首元素操作函数
 * @param input: 
	MyQueue *queue - 已经创建好的循环队列
 * @param output:
	none.
 * @return 
	成功时返回队首元素
	失败时返回NULL
 */

char	*GetQueueFront( MyQueue *p )
{
	char	*element = NULL;

	if( !isQueueInit(p) )
	{
		printf( "[%s][%d]the queue donot init\n", __FILE__, __LINE__ );
		return NULL;
	}

	if( isQueueEmpty(p) )
	{
		printf( "[%s][%d]the queue is empty\n", __FILE__, __LINE__ );
		return NULL;
	}

	element = p->queue + p->front;

	return element;
}

/**
 * 取队尾元素操作函数
 * @param input: 
	MyQueue *queue - 已经创建好的循环队列
 * @param output:
	none.
 * @return 
	成功时返回队首元素
	失败时返回NULL
 */

char	*GetQueueLast( MyQueue *p )
{
	int	queue_num = 0;
	char	*element = NULL;

	queue_num = GetQueueNum( p );

	element = GetQueueElem( p, queue_num );

	return element;
}

/**
 * 取队列第i个元素操作函数
 * @param input: 
	MyQueue *queue - 已经创建好的循环队列
 * @param output:
	none.
 * @return 
	成功时返回队首元素
	失败时返回NULL
 */

char	*GetQueueElem( MyQueue *p, int i )
{
	char	*element = NULL;
	int	typesize = 0; 
	int	maxsize = 0;
	int	index = 0;

	if( !isQueueInit(p) )
	{
		printf( "[%s][%d]do not init\n", __FILE__, __LINE__ );
		return NULL;
	}

	if( isQueueEmpty(p) )
	{
		printf( "[%s][%d]queue is empty\n", __FILE__, __LINE__ );
		return NULL;
	}

	if( i > p->length || i <= 0 )
	{
		printf( "[%s][%d]i is wrong number\n", __FILE__, __LINE__ );
		return NULL;
	}

	typesize = p->typesize;
	maxsize = p->maxsize*typesize;

	index = (p->front + (i-1)*typesize) % maxsize;
	element = p->queue + index;

	return element;
}

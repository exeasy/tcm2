#ifndef _MY_QUEUE_
#define _MY_QUEUE_

#define	FAIL 0
#define	TRUE 1
#define MY_SUCCESS 0
#define MY_ERR 1
#define MY_ERR_NULL 2
#define MY_ERR_CALL 3
#define MY_ERR_NO_INIT 4
#define MY_ERR_Q_FULL 11
#define MY_ERR_Q_EMPTY 12


typedef struct _my_queue
{
	char	*queue;		//队列存储空间
	int	front;		//队列头 - front实际上存放的是字符数组的下标
	int	rear;		//队列尾 - rear实际上存放的是字符数组的下标
	int	length;		//当前队列长度
	int	maxsize;	//循环队列最大长度
	int	typesize;	//某种数据类型长度
	pthread_mutex_t	mutex;
}MyQueue;

/**
 * 创建并初始化一个队列
 * @param input: 
	int typesize	:typesize是数据类型大小,单位为字节
			:(数据类型即队列中存放什么类型的数据)
	int len		:循环队列大小
 * @param output:
	none.
 * @return 
	成功时返回MyQueue类型的数据,表明队列创建成功
	失败时返回NULL,表明创建队列失败
 * @for example:
	MyQueue *p = QueueCreate( sizeof(int), 100 );
	if( p == NULL )
	{
		printf( "create queue error\n" );
	}
 * @for example End
 **/

MyQueue *QueueCreate( int typesize, int len );

/**
 * 销毁一个已经存在的队列
 * @param input: 
	MyQueue *queue	:已经创建好的循环队列
 * @param output:
	none.
 * @return 
	成功时返回MY_SUCCESS
 * @for example:
	QueueDestory( queue );	//queue is created by QueueCreate function
 * @for example End
 **/

int	QueueDestory( MyQueue *queue );

/**
 * 判断队列是否被初始化
 * @param input: 
	MyQueue *queue	:已经创建好的循环队列指针
 * @param output:
	none.
 * @return 
	队列初始化时返回TRUE
	队列未初始化时返回FAIL	
 * @for example:
	//queue is created by QueueCreate function
	if( !isQueueInit(queue) )
	{
		printf( "queue is not init\n" );
	}
	else
	{
		printf( "queue is init\n" );
	}
 * @for example End
 **/

 int isQueueInit( MyQueue *queue );

/**
 * 判断队列是否为空
 * @param input: 
	MyQueue *queue	:已经创建好的循环队列
 * @param output:
	none.
 * @return 
	队列初始化时返回TRUE
	队列未初始化时返回FAIL	
 * @for example:
	It's similar to the function of isQueueInit();
 * @for example End
 **/

 int isQueueEmpty( MyQueue *p );

/**
 * 判断队列是否满
 * @param input: 
	MyQueue *queue	:已经创建好的循环队列
 * @param output:
	none.
 * @return 
	队列初始化时返回TRUE
	队列未初始化时返回FAIL	
 * @for example:
	It's similar to the function of isQueueInit();
 * @for example End
 */

 int isQueueFull( MyQueue *p );

/**
 * 得到队列元素个数
 * @param input: 
	MyQueue *queue - 已经创建好的循环队列
 * @param output:
	none.
 * @return 
	成功时返回元素个数
	失败时返回0
 * @for example:
	int num = GetQueueNum( queue );	
 * @for example End
 */

int GetQueueNum( MyQueue *p );

/**
 * 入队操作函数
 * @param input: 
	MyQueue *queue	:已经创建好的循环队列
 * @param output:
	none.
 * @return 
	入队成功时返回TRUE
	入队失败时返回FAIL	
 * @for example:
	int a = 6;
	MyQueue	*p = QueueCreate( sizeof(int), 100 );
	if( p == NULL )
	{
		printf( "QueueCreate error\n" );
		return;
	}
	
	if( enQueue( p, (char *)&a ) == MY_SUCCESS )
	{
		printf( "enQueue error\n" );
		return ;
	}
 * @for example End
 */

int     enQueue( MyQueue *p, char *element );

/**
 * 出队操作函数
 * @param input: 
	MyQueue *queue	:已经创建好的循环队列
 * @param output:
	none.
 * @return 
	成功时返回队首元素
	失败时返回NULL
 * @for example:
	int a = 6;
	int b = 0;
	MyQueue	*p = QueueCreate( sizeof(int), 100 );
	if( p == NULL )
	{
		printf( "QueueCreate error\n" );
		return;
	}
	
	//push
	if( enQueue( p, (char *)&a ) == MY_SUCCESS )
	{
		printf( "enQueue error\n" );
		return ;
	}

	//pop
	b = *( (int *)DeQueue(p) );
	if( b == NULL )
	{
		printf( "DeQueue error\n" );
	}
 * @for example End
 */

char    *DeQueue( MyQueue *p );

/**
 * 取队首元素操作函数
 * @param input: 
	MyQueue *queue - 已经创建好的循环队列
 * @param output:
	none.
 * @return 
	成功时返回队首元素
	失败时返回NULL
 * @for example:
	int *front = GetQueueFront( queue );	
	if( front == NULL )
	{
		printf( "GetQueueFront error\n" );
		return -1
	}
	printf( "elem=[%d]\n", *((int *)front) );
 * @for example End
 */

char	*GetQueueFront( MyQueue *p );

/**
 * 取队尾元素操作函数
 * @param input: 
	MyQueue *queue - 已经创建好的循环队列
 * @param output:
	none.
 * @return 
	成功时返回队首元素
	失败时返回NULL
 * @for example:
	It's similar to the function of GetQueueFront();
 * @for example End
 */

char	*GetQueueLast( MyQueue *p );

/**
 * 取队列第i个元素操作函数
 * @param input: 
	MyQueue *queue - 已经创建好的循环队列
 * @param output:
	none.
 * @return 
	成功时返回队首元素
	失败时返回NULL
 * @for example:
 * @for example End

	int *elem = GetQueueElem( queue, 1 );
 */

char	*GetQueueElem( MyQueue *p, int i );

#endif

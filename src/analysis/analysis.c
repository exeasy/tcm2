#include "public_include.h"
#include "packet.h"
#include "ip_list.h"
#include "interface.h"

static int decide_sampled_packets( int if_index );
int	get_sampled_index( float out_sampled_rate, int if_index );
int	get_sampled_rate( int sampled_index, int if_index );
int	get_sampled_interval_time( int sampled_index, int if_index );
int	decide_sample_or_not( int out_sampled_rate );
int	get_interval_time( int if_index );
int	cal_interface_sampled_rate( int if_index );
int	AR_compute( TYPE current_value, struct _interface_queue *queue_array, int if_index );
int	regression_analysis( MyQueue *queue, TYPE sum_x, TYPE sum_y );

int	analysis_module( const u_char *packet_content, int if_index )
{
	struct ip		*iph = NULL;
	struct ip6_hdr		*iph6 = NULL;
	char			ip_addr[INET6_ADDRSTRLEN+1];
	struct _ip_func		*p = NULL;
	unsigned int 		version = 0;
	char			*flow = NULL;
	struct	list_head	*p_flow = NULL;
	int			iret = 0;

	//2011-09-09 start

	interface_sampled_struct[if_index].total_packets++;

	//判断是否抽样
	iret = decide_sampled_packets( if_index );
	//WriteLog( NORMT, "[%s][%d]probability iret=[%d]\n", __FILE__, __LINE__, iret );
	if( 0 == iret )
	{
		return 0;
	}
	interface_sampled_struct[if_index].sampled_packets++;

	//2011-09-09 end

	iph = (struct ip*)(packet_content+sizeof(struct ether_header) );

	version = iph->ip_v;
	WriteLog( DEBUG, "[%s][%d]version=[%d]\n", __FILE__, __LINE__, version );

	//this is similar to c++'s the factory mode
	p = get_operation_func( version );
	if( p == NULL )
	{
		WriteLog( ERR, "[%s][%d]get_operation_func error\n", __FILE__, __LINE__ );
		return -1;
	}

	//get flow control function
	//ipv4's flow control function is different from ipv6's
	flow = p->operate( packet_content, if_index );

	//for test start
	p->print_data( flow );
	struct timeval start;
	gettimeofday( &start, NULL );
	WriteLog( DEBUG, "[%s][%d]%s\n", __FILE__, __LINE__, ctime((const time_t*) &start.tv_sec) );
	//WriteLog( DEBUG, "[%s][%d]start time: %ld secs %ld micros\n", __FILE__, __LINE__, start.tv_sec, start.tv_usec );


	//for test end

	(p_flow = p->search(flow, if_index));
	if( p_flow == NULL )
	{
		WriteLog( DEBUG, "\n\n\n[%s][%d]null\n", __FILE__, __LINE__ );	
		p->add( flow, if_index );
	}
	else
	{
		WriteLog( DEBUG, "\n\n\n[%s][%d]not null\n", __FILE__, __LINE__ );	
		p->update( p_flow, flow );
	}

	memset( &start, 0x00, sizeof(struct timeval) );
	gettimeofday( &start, NULL );
	WriteLog( DEBUG, "[%s][%d]%s\n", __FILE__, __LINE__, ctime((const time_t*) &start.tv_sec) );

	return TC_SUCCESS;
}

/*
//判断是否抽样
	1.N++
	2.根据上一次抽样率得到抽样索引
	3.根据抽样索引得到实际抽样率
	6.n++
*/

static int decide_sampled_packets( int if_index )
{
	int	 iret = 0;
	//int	sampled_index = 0;
	struct _sampled_struct *p = NULL;
	int	out_sampled_rate = 0;

	p = &interface_sampled_struct[if_index];

	if( p->out_sampled_rate == 1 )
	{
		iret = 1;
	}
	else
	{
		if( p->sampled_index == 0 )
		{
			//得到抽样索引号
			p->sampled_index = get_sampled_index( p->out_sampled_rate, if_index );
		}

		//得到抽样率
		out_sampled_rate = get_sampled_rate( p->sampled_index, if_index );	

		//判断是否抽样
		iret = decide_sample_or_not( out_sampled_rate );

		WriteLog( DEBUG, "[%s][%d]out_sampled_rate=[%.2f][%d],index=[%d]\n", __FILE__, __LINE__, p->out_sampled_rate, out_sampled_rate, p->sampled_index );
	}

	return iret;
}

//得到抽样索引号
int	get_sampled_index( float out_sampled_rate, int if_index )
{
	float	a = 0;	
	int	b = 0;

	a = out_sampled_rate * 10;

	b = a;

	WriteLog( DEBUG, "[%s][%d]b=[%d]\n", __FILE__, __LINE__, b );

	return b;
}

//根据抽样索引号得到抽样率
int	get_sampled_rate( int sampled_index, int if_index )
{
	return if_sampled_array[if_index].sys_sampled_param[sampled_index].out_sampled_rate;
}

//根据抽样索引号得到抽样间隔时间
int	get_sampled_interval_time( int sampled_index, int if_index )
{
	return if_sampled_array[if_index].sys_sampled_param[sampled_index].interval_time*60000000;
}

//判断是否需要抽样
//????
int	decide_sample_or_not( int out_sampled_rate )
{
	return ( (rand()%10) < out_sampled_rate );
}

//得到抽样的间隔时间
int	get_interval_time( int if_index )
{
	struct _sampled_struct *p = NULL;
	float	total_packets = 0;
	float	sampled_packets = 0;
	int	sampled_index = 0;
	int	out_sampled_rate = 0;

	p = &interface_sampled_struct[if_index];
	out_sampled_rate = p->out_sampled_rate;

	WriteLog( DEBUG, "[%s][%d]out_sampled_rate=[%.2f]\n", __FILE__, __LINE__, p->out_sampled_rate );
	sampled_index = get_sampled_index( p->out_sampled_rate, if_index );

	return get_sampled_interval_time( sampled_index, if_index );
}

//计算网卡的抽样率
int	cal_interface_sampled_rate( int if_index )
{
	struct _sampled_struct *p = NULL;
	TYPE total_packets = 0;
	float sampled_packets = 0;
	TYPE Estimated_value = 0;

	p = &interface_sampled_struct[if_index];
	
	total_packets = p->total_packets;
	WriteLog( NORMT, "[%s][%d]total_packets=[%d]\n", __FILE__, __LINE__, p->total_packets );

	Estimated_value = AR_compute( total_packets, queue_array, if_index );
	if( 0 == Estimated_value 
		|| -1 == Estimated_value 
		|| 0 == system_best_sampled_n
		|| system_best_sampled_n >= Estimated_value )
	{
		p->out_sampled_rate = 1;	
	}
	else
	{
		p->out_sampled_rate = system_best_sampled_n / Estimated_value;
		//p->sampled_packets = 1 / Estimated_value;
	}
	
	WriteLog( NORMT,  "[%s][%d]out_sampled_rate=[%.2f]\n", __FILE__, __LINE__, p->out_sampled_rate );
	WriteLog( NORMT, "[%s][%d]out_sampled_rate=[%.2f]\n", __FILE__, __LINE__, p->out_sampled_rate );

	return 0;

/********************* deleted by gaowg 2012-02-24 (old sampled rate 's function)
	sampled_packets = p->sampled_packets;
	if( sampled_packets > MAX_FLOW_NUM )
	{
		sampled_packets = MAX_FLOW_NUM;
	}
	if( total_packets != 0 )
	{
		p->out_sampled_rate = sampled_packets / total_packets;
	}
	else
	{
		p->out_sampled_rate = 0;
	}

	WriteLog( NORMT, "[%s][%d]total_packets=[%d]\n", __FILE__, __LINE__, p->total_packets );
	WriteLog( NORMT, "[%s][%d]total_packets=[%d]\n", __FILE__, __LINE__, p->sampled_packets );
	WriteLog( NORMT, "[%s][%d]out_sampled_rate=[%.2f]\n", __FILE__, __LINE__, p->out_sampled_rate );
	return 0;
***********************/
}

//回归分析计算
int	AR_compute( TYPE current_value, struct _interface_queue *queue_array, int if_index )
{
	int	queue_num = 0;
	TYPE	*front = NULL;
	TYPE	*last = NULL;
	TYPE	*second = NULL;
	TYPE	sum_x = 0;
	TYPE	sum_y = 0;
	MyQueue	*queue = NULL;
	double next_m;		//预测的下一个block时间段内m的值

	WriteLog( NORMT, "[%s][%d]total_packets=[%d]\n", __FILE__, __LINE__, current_value );
	WriteLog( NORMT, "[%s][%d]total_packets=[%.0f]\n", __FILE__, __LINE__, current_value );
	if( queue_array == NULL )
	{
		WriteLog( ERR, "[%s][%d]queue_array is NULL\n", __FILE__, __LINE__ );
		return -1;
	}

	queue = queue_array[if_index].queue;
	if( queue == NULL )
	{
		WriteLog( ERR, "[%s][%d]queue is NULL\n", __FILE__, __LINE__ );
		return -1;
	}
	sum_x = queue_array[if_index].sum_x;
	sum_y = queue_array[if_index].sum_y;
	WriteLog( NORMT, "[%s][%d]sum_x=[%d]\n", __FILE__, __LINE__, sum_x );
	WriteLog( NORMT, "[%s][%d]sum_y=[%d]\n", __FILE__, __LINE__, sum_y );
	
	WriteLog( NORMT, "[%s][%d]value=[%d]\n", __FILE__, __LINE__, current_value );

	if( !isQueueFull(queue) )
	{
		if( !isQueueEmpty(queue) )
		{
			last = (TYPE *)GetQueueLast(queue);
			if( last == NULL )
			{
				WriteLog( "[%s][%d]GetQueueLast error\n", __FILE__, __LINE__ );
				return -1;
			}
			sum_x += *((TYPE *)last);

			sum_y += current_value;

			WriteLog( NORMT, "[%s][%d]sum_x=[%d]\n", __FILE__, __LINE__, sum_x );
			WriteLog( NORMT, "[%s][%d]sum_y=[%d]\n", __FILE__, __LINE__, sum_y );
		}
	}
	else
	{
		front = (TYPE *)DeQueue( queue );
		if( front == NULL )
		{
			WriteLog( ERR, "[%s][%d] DeQueue error\n", __FILE__, __LINE__ );
			return -1;
		}
		last = (TYPE *)GetQueueLast( queue );
		if( last == NULL )
		{
			WriteLog( ERR, "[%s][%d] GetQueueLast error\n", __FILE__, __LINE__ );
			return -1;
		}

		second = (TYPE *)GetQueueElem( queue, 1 );
		if( second == NULL )
		{
			WriteLog( ERR, "[%s][%d] GetQueueElem error\n", __FILE__, __LINE__ );
			return -1;
		}

		sum_x = sum_x + *((TYPE *)last) - *((TYPE *)front);
		sum_y = sum_y + current_value - *((TYPE *)second);	


		WriteLog( NORMT, "[%s][%d]sum_x=[%d]\n", __FILE__, __LINE__, sum_x );
		WriteLog( NORMT, "[%s][%d]sum_y=[%d]\n", __FILE__, __LINE__, sum_y );
	}
	queue_array[if_index].sum_x = sum_x;
	queue_array[if_index].sum_y = sum_y;

	WriteLog( NORMT, "[%s][%d]queue length=[%d]\n", __FILE__, __LINE__, GetQueueNum(queue) );

	if( enQueue( queue, (char *)&current_value ) != MY_SUCCESS )
	{
		WriteLog( ERR, "[%s][%d] enqueue error\n", __FILE__, __LINE__ );
		return -1;
	}

	if( isQueueFull(queue) )
	{
	WriteLog( NORMT, "[%s][%d]AR start\n", __FILE__, __LINE__ );
		//利用回归分析方法 预测Mi+1的值
		next_m = regression_analysis( queue, sum_x, sum_y );
	}

	return next_m;
}

//回归分析方程 预测Mi+1的值
int	regression_analysis( MyQueue *queue, TYPE sum_x, TYPE sum_y )
{
	int i = 0;
	double	queue_num = 0;

	double ax;	//x值
	double by;	//y值
	TYPE *a = NULL;	//从队列中取到的x的指针
	TYPE *b = NULL;	//从队列中取到的y的指针
	double avg_x;	//x均值
	double avg_y;	//y均值

	TYPE *current_m = NULL;

	double	numerator = 0; //分子
	double	denominator = 0; //分母

	double factor_first;	//回归模型中的第1个系数
	double factor_second;	//回归模型中的第2个系数
	double next_m;		//预测的下一个block时间段内m的值

	queue_num = GetQueueNum( queue )-1;
	if( queue_num == 0 )
	{
		WriteLog( ERR, "[%s][%d]queue_num is zero\n", __FILE__, __LINE__ );
		return -1;
	}
	avg_x = sum_x / queue_num;
	avg_y = sum_y / queue_num;

	WriteLog( NORMT, "[%s][%d]avg_x=[%.2f],avg_y=[%.2f]\n", __FILE__, __LINE__, avg_x, avg_y );

	//以下步骤按照回归分析方程计算其系数
	for( i=1; i<queue_num; i++ )
	{
		a = (TYPE *)GetQueueElem( queue, i );
		if( a == NULL )
		{
			WriteLog( ERR, "[%s][%d] GetQueueElem error\n", __FILE__, __LINE__ );
			return -1;
		}
		ax = *((TYPE *)a);
		ax -= avg_x;

		b = (TYPE *)GetQueueElem( queue, i+1 );
		if( b == NULL )
		{
			WriteLog( ERR, "[%s][%d] GetQueueElem error\n", __FILE__, __LINE__ );
			return -1;
		}
		by = *((TYPE *)b);
		by -= avg_y;

		numerator += ax * by;
		denominator += ax * ax;
	}	

	WriteLog( NORMT, "[%s][%d]numerator=[%.2f],denominator=[%.2f]\n", __FILE__, __LINE__, numerator, denominator );

	//得到系数
	if( denominator == 0 )
	{
		factor_second = 1;
		factor_first = 0;
	}
	else
	{
		factor_second = numerator / denominator;
		factor_first = avg_y - factor_second * avg_x;
	}

	WriteLog( NORMT, "[%s][%d]first=[%.2f],second=[%.2f]\n", __FILE__, __LINE__, factor_first, factor_second );

	current_m = (TYPE *)GetQueueLast( queue );
	WriteLog( NORMT, "[%s][%d]current_m=[%d]\n", __FILE__, __LINE__, *((TYPE *)current_m) );

	//预测下一个block时间段内m的值
	next_m = factor_first + factor_second * ( *((TYPE *)current_m) );

	WriteLog( NORMT, "[%s][%d]next_m=[%.2f]\n", __FILE__, __LINE__, next_m );

	return next_m;
}

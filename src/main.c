#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <pthread.h>
#include "public.h"
#include "interface.h"

void	*collect_traffic_info_func( void *arg );
void	*thread_statistics_operation();
void	*interface_info_func( void *arg );
pthread_t       traffic_thrd[MAX_INTERFACE_SIZE];

int	g_this_interface_size;

void	main()
{
	int		i = 0;
	pthread_t	interface_thrd;
	pthread_t	traffic_statistics_thrd;

	//初始化模块
	//1.init_module();
	init_module();
	g_this_interface_size = g_interface_size;

	WriteLog( DEBUG, "[%s][%d]DEBUG\n", __FILE__, __LINE__ );

	//1.为每个网卡创建一个线程
	if( pthread_create(&traffic_thrd[i], NULL, collect_traffic_info_func, (void *)&i) ) 
	{
		WriteLog( ERR, "[%s][%d]thread_create error=[%s]\n", __FILE__, __LINE__, strerror(errno) );	
		return ;
	}

	//2.统计流量信息并上传到PMS
	if( pthread_create(&traffic_statistics_thrd, NULL, thread_statistics_operation, NULL) ) 
	{
		WriteLog( ERR, "[%s][%d]thread_create error=[%s]\n", __FILE__, __LINE__, strerror(errno) );	
		return ;
	}

	sleep( 1 );
//	//3.创建获取接口总流量信息线程函数
//	pthread_create( &interface_thrd, NULL, interface_info_func, NULL );

	//4.等待线程结束并退出
	for( i=0; i<g_interface_size; i++ )
	{
		pthread_join( traffic_thrd[i], NULL );
	}
	pthread_join( traffic_statistics_thrd, NULL );
//	pthread_join( interface_thrd, NULL );
}

//线程函数
//producer function
void	*collect_traffic_info_func( void *arg )
{
	int	interval_time = 0;
	int	if_index = 0;
	int	i = 0;

        i = *((int *)arg);
	if_index = i;
        WriteLog( NORMT, "[%s][%d]tid=[%x]i=[%d]start new producer\n", __FILE__, __LINE__, pthread_self(), i );

        i = i + 1;
	if( i < g_interface_size )
	{
		if( pthread_create( &traffic_thrd[i], NULL, collect_traffic_info_func, (void *)&i ) )
		{
			WriteLog( NORMT, "[%s][%d]pthread create error=[%s]\n", __FILE__, __LINE__, strerror(errno) );
			return NULL;
		}
	}

	while( 1 )
	{
		//流量信息收集模块
		//2.collect_information();
		WriteLog( NORMT, "[%s][%d],if_index=[%d]\n", __FILE__, __LINE__, if_index );
		collect_information( if_index );

		//when value is true, producer notify consumer
		producer_notify_consumer( g_interface_size );

	}
}

//consumer function
void *thread_statistics_operation()
{
	int i = 0;

	while( 1 )
	{
		WriteLog( NORMT, "[%s][%d]tid=[%x]!!!!consume wait!!!!\n", __FILE__, __LINE__, pthread_self() );
		consumer_wait( g_interface_size );
		WriteLog( NORMT, "[%s][%d]tid=[%x]!!!!consume start!!!!\n", __FILE__, __LINE__, pthread_self() );

		for( i=0; i<g_interface_size; i++ )
		{

			//3.analysis_module();
			//统计模块
			//4.statistics_module();
			one_interface_statistics_module( i );
		}

		WriteLog( NORMT, "[%s][%d]total=[%d]\n", __FILE__, __LINE__,  interface_sampled_struct[0].total_packets );

		WriteLog( NORMT, "[%s][%d]total=[%d]\n", __FILE__, __LINE__,  interface_sampled_struct[0].sampled_packets );

		//2011-09-09 start 计算间隔时间
	/*
		cal_interface_sampled_rate( 0 );
		interval_time = get_interval_time( 0 );
		usleep( interval_time );
	*/

		for( i=0; i<g_interface_size; i++ )
		{
			//初始化系统变量
			memset( &(if_array[i].element), 0x00, sizeof(struct _element_struct) );
			if_array[i].element.maxsize = MAX_FLOW_NUM;
			WriteLog( NORMT, "[%s][%d]packets=[%d]\n", __FILE__, __LINE__, interface_sampled_struct[i].total_packets );
			WriteLog( NORMT, "[%s][%d]sampled_packets=[%d]\n", __FILE__, __LINE__, interface_sampled_struct[i].sampled_packets );

			cal_interface_sampled_rate( i );

			WriteLog( NORMT, "[%s][%d]sampled=[%.2f]\n", __FILE__, __LINE__, interface_sampled_struct[i].out_sampled_rate );

			interface_sampled_struct[i].total_packets = 0;
			interface_sampled_struct[i].sampled_packets = 0;
			interface_sampled_struct[i].sampled_index = 0;

			INIT_LIST_HEAD( &(ipv4_head_in[i].node_head) );
			INIT_LIST_HEAD( &(ipv4_head_out[i].node_head) );

			//2011-09-09 end

			//usleep( g_host_config.interval );
		}

		WriteLog( NORMT, "[%s][%d]\n", __FILE__, __LINE__ );

		//consumer notify producer
		consumer_notify_producer();
	}

	retrun NULL;
}

//上传接口信息线程函数
void	*interface_info_func( void *arg )
{
	return 0;
	int	interface_size = 0;
	int	s_net = 0;
	int	seq = 0;

	//1.get interface infomation
	get_all_interfaces( ife, &interface_size );	
	interface_size = g_this_interface_size;
	get_interfaces_info( interface_size, ife );
	print_interfaces_info( interface_size, ife );

	while( 1 )
	{
		//2.get rx bytes, tx bytes and drops
		//s_net = tc_netlink_open( &seq );
		//get_dev_info( s_net, seq, ife );
		get_dev_flow_info( ife, interface_size );
	
		//3.send interface info xml pack
		send_interface_info( interface_size );

		//notice:sleep function's INTERVAL_TIME is very important.
		//Donot modify!!!	
		sleep( INTERVAL_TIME );
	}
}
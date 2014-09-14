#include "public_include.h"
#include "interface.h"
#include "list.h"
#include "ip_list.h"
#include "packet.h"

static void operate_link_info_buff();
static void operate_flow_info_buff();
static void one_interface_traverse_sys_link( int if_index );
static void one_interface_operate_flow_info_buff( int if_index );
int one_interface_pack_flow_info_xml_buff( __u8 **xmlbuff, int *len, int if_index );

static void traverse_sys_link()
{
	struct list_head *l;	
	int i = 0;

	//ipv4 system link
	for( i=0; i<g_interface_size; i++ )
	{
		WriteLog( DEBUG, "[%s][%d]out data\n", __FILE__, __LINE__ );
		list_for_each( l, &ipv4_head_out[i].node_head )
		{
			struct _ipv4_struct *ipv4_struct = list_entry(l, struct _ipv4_struct, node );
			{
				//calculate module: 
				//get input interface and output interface
				cal_flow_in_out_interface( (char *)&(ipv4_struct->flow), i );

				WriteLog( DEBUG, "[%s][%d]ip_v=[%d]\n", __FILE__, __LINE__, ipv4_struct->ip_v );
				WriteLog( DEBUG, "[%s][%d]used=[%d]\n", __FILE__, __LINE__, ipv4_struct->used );

				WriteLog( DEBUG, "[%s][%d]\n", __FILE__, __LINE__ );
				WriteLog( DEBUG, "[%s][%d]start time: %ld secs %ld micros\n", __FILE__, __LINE__, (ipv4_struct->start).tv_sec, (ipv4_struct->start).tv_usec );
				if( (ipv4_struct->end).tv_sec == 0 )
				{
					WriteLog( ERR, "[%s][%d]0000000\n", __FILE__, __LINE__ );
				}
				WriteLog( DEBUG, "[%s][%d]end time: %ld secs %ld micros\n", __FILE__, __LINE__, (ipv4_struct->end).tv_sec, (ipv4_struct->end).tv_usec );


				//ipv4_print_data( (char *)(&(ipv4_struct->flow)) );
			}
		}

		WriteLog( DEBUG, "[%s][%d]in data\n", __FILE__, __LINE__ );
		list_for_each( l, &ipv4_head_in[i].node_head )
		{
			struct _ipv4_struct *ipv4_struct = list_entry(l, struct _ipv4_struct, node );
			{
				WriteLog( DEBUG, "[%s][%d]ip_v=[%d]\n", __FILE__, __LINE__, ipv4_struct->ip_v );
				WriteLog( DEBUG, "[%s][%d]used=[%d]\n", __FILE__, __LINE__, ipv4_struct->used );
				//WriteLog( DEBUG, "\n\n" );
				//ipv4_print_data( (char *)(&(ipv4_struct->flow)) );
			}
		}
	}

	//ipv6 system link
	for( i=0; i<g_interface_size; i++ )
	{
		list_for_each( l, &ipv6_head[i].node_head )
		{
			struct _ipv6_struct *ipv6_struct = list_entry(l, struct _ipv6_struct, node );
			{
				WriteLog( DEBUG, "[%s][%d]ip_v=[%d]\n", __FILE__, __LINE__, ipv6_struct->ip_v );
				WriteLog( DEBUG, "[%s][%d]used=[%d]\n", __FILE__, __LINE__, ipv6_struct->used );
				//WriteLog( DEBUG, "\n\n" );
				ipv6_print_data( (char *)(&(ipv6_struct->flow)) );

				//calculate module: 
				//get input interface and output interface
				//???
			}
		}
	}
}

//2011-09-12
static void one_interface_traverse_sys_link( int if_index )
{
	struct list_head *l;	
	int i = 0;
	i = if_index;

	//ipv4 system link
	WriteLog( DEBUG, "[%s][%d]out data,if_index=[%d] \n", __FILE__, __LINE__, if_index );
	list_for_each( l, &ipv4_head_out[i].node_head )
	{
		struct _ipv4_struct *ipv4_struct = list_entry(l, struct _ipv4_struct, node );
		{
			//calculate module: 
			//get input interface and output interface
			cal_flow_in_out_interface( (char *)&(ipv4_struct->flow), i );

			WriteLog( DEBUG, "[%s][%d]ip_v=[%d]\n", __FILE__, __LINE__, ipv4_struct->ip_v );
			WriteLog( DEBUG, "[%s][%d]used=[%d]\n", __FILE__, __LINE__, ipv4_struct->used );

			WriteLog( DEBUG, "[%s][%d]\n", __FILE__, __LINE__ );
			WriteLog( DEBUG, "[%s][%d]start time: %ld secs %ld micros\n", __FILE__, __LINE__, (ipv4_struct->start).tv_sec, (ipv4_struct->start).tv_usec );
			if( (ipv4_struct->end).tv_sec == 0 )
			{
				WriteLog( ERR, "[%s][%d]0000000\n", __FILE__, __LINE__ );
			}
			WriteLog( DEBUG, "[%s][%d]end time: %ld secs %ld micros\n", __FILE__, __LINE__, (ipv4_struct->end).tv_sec, (ipv4_struct->end).tv_usec );


			ipv4_print_data( (char *)(&(ipv4_struct->flow)) );
		}
	}

	WriteLog( DEBUG, "[%s][%d]in data\n", __FILE__, __LINE__ );
	/**********************
	list_for_each( l, &ipv4_head_in[i].node_head )
	{
		struct _ipv4_struct *ipv4_struct = list_entry(l, struct _ipv4_struct, node );
		{
			WriteLog( DEBUG, "[%s][%d]ip_v=[%d]\n", __FILE__, __LINE__, ipv4_struct->ip_v );
			WriteLog( DEBUG, "[%s][%d]used=[%d]\n", __FILE__, __LINE__, ipv4_struct->used );
			//WriteLog( DEBUG, "\n\n" );
			ipv4_print_data( (char *)(&(ipv4_struct->flow)) );
		}
	}
	************************/

	//ipv6 system link
	list_for_each( l, &ipv6_head[i].node_head )
	{
		struct _ipv6_struct *ipv6_struct = list_entry(l, struct _ipv6_struct, node );
		{
			WriteLog( DEBUG, "[%s][%d]ip_v=[%d]\n", __FILE__, __LINE__, ipv6_struct->ip_v );
			WriteLog( DEBUG, "[%s][%d]used=[%d]\n", __FILE__, __LINE__, ipv6_struct->used );
			//WriteLog( DEBUG, "\n\n" );
			ipv6_print_data( (char *)(&(ipv6_struct->flow)) );

			//calculate module: 
			//get input interface and output interface
			//???
		}
	}
}

//2011-09-12
//每块网卡单独统计流信息
void	one_interface_statistics_module( int if_index )
{
	one_interface_traverse_sys_link( if_index );

	one_interface_operate_flow_info_buff( if_index );

	//operate_link_info_buff();
}

//statistics data
void	statistics_module()
{
	traverse_sys_link();

	operate_flow_info_buff();

	//operate_link_info_buff();
}

static void operate_flow_info_buff()
{
	__u8 *xmlbuff = NULL;
	int	len = 0;
//2011-07-07
	char	value[50];
	memset( value, 0x00, sizeof(value) );
	get_data_time( value, sizeof(value)-1 );
	WriteLog( DEBUG, "[%s][%d]time=[%s]\n", __FILE__, __LINE__, value );
//2011-07-07
	//2.packet xml buffer
	pack_flow_info_xml_buff( &xmlbuff, &len );

	//3.send buffer to pma
	//send_data( xmlbuff, len );

	WriteLog( DEBUG, "[%s][%d]xml=[%s]\n", __FILE__, __LINE__, xmlbuff );

	free( xmlbuff );

	WriteLog( DEBUG, "[%s][%d]error=[%d]msg=[%s]\n", __FILE__, __LINE__, errno, strerror(errno) );
}

//2011-09-12
static void one_interface_operate_flow_info_buff( int if_index )
{
	__u8 *xmlbuff = NULL;
	int	len = 0;
	char	value[50];
	memset( value, 0x00, sizeof(value) );
	get_data_time( value, sizeof(value)-1 );
	WriteLog( DEBUG, "[%s][%d]time=[%s]\n", __FILE__, __LINE__, value );

	//2.packet xml buffer
	one_interface_pack_flow_info_xml_buff( &xmlbuff, &len, if_index );

	//3.send buffer to pma
	send_data( xmlbuff, len, TC_FLOW_INFO );

	WriteLog( NORMT, "[%s][%d]xml=[%s]\n", __FILE__, __LINE__, xmlbuff );

	if( xmlbuff != NULL )
	{
		xmlFree( xmlbuff );
	}

}

static void operate_link_info_buff()
{
	__u8 *xmlbuff = NULL;
	int	len = 0;
        int     s_net = 0;
        int     seq = 0;

	//1.get interface's sent data and dropped data
	s_net = tc_netlink_open( &seq );
	get_all_interfaces_info( s_net, seq, if_array );

	//2.packet xml buffer
	pack_link_traffic_xml_buff( &xmlbuff, &len );

	//3.send buffer to pma
	//send_data( xmlbuff, len );
	WriteLog( DEBUG, "[%s][%d]xml=[%s]\n", __FILE__, __LINE__, xmlbuff );

	free( xmlbuff );
}

int cal_flow_in_out_interface( char *flow, int out_if_index )
{
        struct list_head *tmp = NULL;
        struct list_head *head = NULL;
	struct _ipv4_struct *ipv4_node = NULL;
	struct _ipv4_flow *p = NULL;
	int	in = 0;
	int	iret = 0;

	for( in=0; in<g_interface_size; in++ )
	{
/* 2011-07-07 start
		if( in == out_if_index )
		{
			continue;
		}
* end */

		head = &( ipv4_head_in[in].node_head );

		//look up
		tmp = head->next;
		while( tmp != head )
		{
			ipv4_node = list_entry( tmp, struct _ipv4_struct, node );	
			//policy function
			iret = ipv4_policy_compare( (char *)(ipv4_node), flow );
			if( TC_TRUE == iret )
			{
				break;
			}

			tmp = tmp->next;
		}

		if( TC_TRUE == iret )
		{
			break;
		}
	}

	WriteLog( DEBUG, "[%s][%d]in=[%d]\n", __FILE__, __LINE__, in );
	if( in == g_interface_size )
	{
		in = -1;
	}

	//modify flow struct
	p = (struct _ipv4_flow *)flow;
	//2011-12-07 deleted
	//p->if_in = in; 
	//p->if_out = out_if_index;
	//2011-12-07 deleted
	p->if_in = if_array[in].ifindex; 
	p->if_out = if_array[out_if_index].ifindex;

	return 0;
}

//2011-09-20
//发送接口信息
int	send_interface_info( int size )
{
	__u8 *xmlbuff = NULL;
	int	len = 0;

	//1.packet xml buffer
	pack_interface_info_xml_buff( &xmlbuff, &len, size );

	//2.send buffer to pma
	send_data( xmlbuff, len, TC_INTERFACE_INFO );

	WriteLog( DEBUG, "[%s][%d]xml=[%s]\n", __FILE__, __LINE__, xmlbuff );
	WriteLog( DEBUG, "[%s][%d]\n", __FILE__, __LINE__ );

	if( xmlbuff != NULL )
	{
		WriteLog( ERR, "[%s][%d]error=[%d]msg=[%s]\n", __FILE__, __LINE__, errno, strerror(errno) );
		xmlFree( xmlbuff );
	}
}	
//2011-09-20

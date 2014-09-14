/************************************************************

Copyright (C), 2011-, BUPT
FileName:      init_module.c
Author:
Version :
Date:
Description:	this file included all kinds of init functions.
Function List:
Modify History:
<author> <time> <version > <desc>

**********************************************************/

#include "public_include.h"
#include "public.h"
#include "list.h"
#include "ip_list.h"
#include "packet.h"
#include "util.h"
#include "interface.h"

extern struct _ip_func flow_operation[2];

double get_zp();
int	get_n( double  zp, double p0 );

static int	init_flow_operation();
static void	init_tc_sys_list();
static void init_if_array( struct _interface_struct *if_array, int if_num, int max_num );

static void init_interface_sampled_struct( struct _sampled_struct *sampled_struct, int if_num );
static void print_interface_sampled_struct( struct _sampled_struct *sampled_struct, int if_num );

static void init_sys_sampled_param( struct _sys_sampled_param *sampled_struct, int num );
static void print_sys_sampled_param( struct _sys_sampled_param *sampled_struct, int num );

static void init_interface_sampled_param( struct _interface_sampled_param *if_sampled_array, int interface_num );
static void print_interface_sampled_param( struct _interface_sampled_param *if_sampled_array, int interface_num );

struct _ip_func *get_operation_func( unsigned int ip_v );
void	init_pcap_fp();
static void init_one_pcap_fp( int if_index );
static void init_queue_array( struct _interface_queue *queue_array, int if_num );
void	get_best_sampled_count();

char	*ipv4_operate( const u_char *packet, int if_index );
int	ipv4_print_data( const char *flow );
struct list_head*       ipv4_search( char *flow, int if_index );
int     ipv4_add( char *flow, int if_index );
int     ipv4_update( struct list_head *p, char *flow );
int     ipv4_delete( struct list_head *p );

char	*ipv6_operate( const u_char *packet, int if_index );
int	ipv6_print_data( const char *flow );
struct list_head*	ipv6_search( char *flow, int if_index );
int     ipv6_add( char *flow, int if_index );
int     ipv6_update( struct list_head *p, char *flow );
int     ipv6_delete( struct list_head *p );

struct _ipv4_struct *get_system_memory( int ifindex );


/**************************************

 *
 * function init_module()
 *
 * inputs:	null
 * outputs:     null
 * return: 
 * create date: 20110608
 * describe:
	1.init flow operation function, including ipv4 flow function
		and ipv6 flow function
	2.init system list that stored flow information

**************************************/

void	init_module()
{
	int	i = 0;
	int	s_net = 0;
	int	seq = 0;
	FILE	*fp = NULL;

	//1.init global var
	g_interface_size = 0;

	//2.read host_conf.xml file
	memset( &g_host_config, 0x00, sizeof(struct _tc_config_struct) );
	read_hostconfig_file( "etc/host_config.xml" );	
	
	//log file fp
	fp = fopen( "tc.log", "a+" );
	if( fp == NULL )
	{
		printf( "[%s][%d]fopen error=[%s]\n", __FILE__, __LINE__, strerror(errno) );
		return;
	}
	g_host_config.fp = fp;
	//printf( "[%s][%d]fp=[%x]\n", __FILE__, __LINE__, g_host_config.fp );
	//log file fp

	WriteLog( DEBUG, "[%s][%d]captured_time=[%d]\n", __FILE__, __LINE__, g_host_config.captured_time );
	WriteLog( DEBUG, "[%s][%d]interval=[%d]\n", __FILE__, __LINE__, g_host_config.interval );

	//3.get interface name
	s_net = tc_netlink_open( &seq );
	get_all_interfaces_info( s_net, seq, if_array );
	print_all_interfaces( if_array );

	//4. init if_array[NUM] struct
	init_if_array( if_array, g_interface_size, MAX_FLOW_NUM );

	// init sampled param
	//初始化网卡的抽样结构体用于存放当前抽样率
	init_interface_sampled_struct( interface_sampled_struct, g_interface_size );

	//初始化每块网卡的抽样参数初始值
	init_interface_sampled_param( if_sampled_array, g_interface_size );

	//print_interface_sampled_struct( interface_sampled_struct, g_interface_size );
	//print_interface_sampled_param( if_sampled_array, g_interface_size );
	//print_sys_sampled_param( sys_sampled_param, PARAM_LEN );
	
	//5.init flow operation function
	init_flow_operation();

	//6.init system list
	init_tc_sys_list();

/*
	//7.create pcap file description for each interface
	for( i=0; i<g_interface_size; i++ )
	{
		init_pcap_fp( i );
	}
*/
	//8.init queue struct
	init_if_array( if_array, g_interface_size, MAX_FLOW_NUM );
	init_queue_array( queue_array, g_interface_size );

	//9.get best sampled count
	get_best_sampled_count();
}

/**
 * get operateion function by ip_version
 * @param input:
        unsigned int ip_v
 * @param output:
	struct _ip_func *p:related operation function.
 * @return
        p != NULL:success
        p == NULL:fail
 */

struct _ip_func *get_operation_func( unsigned int ip_v )
{
	struct _ip_func *p = NULL;
	int	i = 0;

	for( i=0; i<2; i++ )	
	{
		if( flow_operation[i].ip_v == ip_v )
		{
			p = &flow_operation[i];
		}
	}

	return p;
}

/**
 * this function is used to handle ipv4 packet
 * @param input:
        const u_char *packet:ipv4 packet
 * @param output:
	char *p: this function convert type of struct _ipv4_flow to type of char
 * @return
        p != NULL:success
        p == NULL:fail
 */

char	*ipv4_operate( const u_char *packet, int if_index )
{
	if( packet == NULL )
	{
		return NULL;
	}

	struct ether_header *ethernet_protocol = NULL;
	struct ip *iph = NULL;

	memset( &ipv4_flow[if_index], 0x00, sizeof(ipv4_flow[if_index]) );
	//struct ether_header
	ethernet_protocol = (struct ether_header *)(packet);
	memcpy( ipv4_flow[if_index].ether_shost, ethernet_protocol->ether_shost, sizeof(ipv4_flow[if_index].ether_shost) );
	memcpy( ipv4_flow[if_index].ether_dhost, ethernet_protocol->ether_dhost, sizeof(ipv4_flow[if_index].ether_dhost) );

	//struct ip header
	iph = (struct ip *)(packet+sizeof(struct ether_header) );

	//add by Macro.Z 
	//read the port_dst port_src from tcp or udp 
	u_char * transpacket = (u_char*)(packet+sizeof(struct ether_header)+sizeof(struct ip));
	unsigned short port_src = *(unsigned short*)transpacket;
	unsigned short port_dst = *(unsigned short*)(transpacket+sizeof(port_src));
	ipv4_flow[if_index].port_dst = ntohs(port_dst);
	ipv4_flow[if_index].port_src = ntohs(port_src);

	//read the hscp value from tos
	ipv4_flow[if_index].dscp = iph->ip_tos>>2;

	memcpy( &(ipv4_flow[if_index].ip_src), &(iph->ip_src), sizeof(struct in_addr) );
	memcpy( &(ipv4_flow[if_index].ip_dst), &(iph->ip_dst), sizeof(struct in_addr) );
	//modify: len = ip_len + sizeof(struct ether_header)
	//2012-03-01
	ipv4_flow[if_index].total_send = ntohs(iph->ip_len)+sizeof(struct ether_header)+4;

	//default: flow's direction is FLOW_IN
	ipv4_flow[if_index].flow_direction = FLOW_IN;
	
	return (char *)(&ipv4_flow[if_index]);
}

/**
 * this function print data
 * @param input:
        const char *flow: flow
 * @param output:
 * @return
 */

int	ipv4_print_data( const char *flow )
{
	if( flow == NULL )
	{
		return TC_ERROR;
	}

	struct _ipv4_flow *p = NULL;
	u_char	*mac_string = NULL;

	p = (struct _ipv4_flow *)flow;

	//interface address
	WriteLog( DEBUG, "[%s][%d]Mac Source Address is : \n", __FILE__, __LINE__ );
	mac_string = p->ether_shost;
	if( mac_string != NULL )
	{
		WriteLog( DEBUG, "%02x:%02x:%02x:%02x:%02x:%02x\n", *mac_string, *(mac_string + 1), *(mac_string + 2), *(mac_string + 3), *(mac_string + 4), *(mac_string + 5));
	}

	WriteLog( DEBUG, "[%s][%d]Mac Destination Address is : \n", __FILE__, __LINE__ );
	mac_string = NULL;
	mac_string = p->ether_dhost;
	if( mac_string != NULL )
	{
		WriteLog( DEBUG, "%02x:%02x:%02x:%02x:%02x:%02x\n", *mac_string, *(mac_string + 1), *(mac_string + 2), *(mac_string + 3), *(mac_string + 4), *(mac_string + 5));
	}

	//ip address
	char	ip_addr[INET6_ADDRSTRLEN+1];
	memset( ip_addr, 0x00, sizeof(ip_addr) );
	inet_ntop(AF_INET, (void *)&(p->ip_src), ip_addr, sizeof(ip_addr)-1);
	WriteLog( DEBUG, "[%s][%d]src ip=[%s]\n", __FILE__, __LINE__, ip_addr );

	memset( ip_addr, 0x00, sizeof(ip_addr) );
	inet_ntop(AF_INET, (void *)&(p->ip_dst), ip_addr, sizeof(ip_addr)-1);
	WriteLog( DEBUG, "[%s][%d]dst ip=[%s]\n", __FILE__, __LINE__, ip_addr );

	//flow's length
	WriteLog( DEBUG, "[%s][%d]flow len=[%d]\n", __FILE__, __LINE__, p->total_send );

	//flow's input and output interface
	WriteLog( DEBUG, "[%s][%d]flow input interface=[%d]\n", __FILE__, __LINE__, p->if_in );
	WriteLog( DEBUG, "[%s][%d]flow output interface=[%d]\n", __FILE__, __LINE__, p->if_out );

	//flow's direction
	WriteLog( DEBUG, "[%s][%d]flow direction=[%d]\n", __FILE__, __LINE__, p->flow_direction );

	return 0;
}

/**
 * this function is looking up
 * @param input:
        char *flow
 * @param output:
	struct list_head *p;
 * @return
        p != NULL:success
        p == NULL:fail
 */

struct list_head*       ipv4_search( char *flow, int if_index )
//struct list_head*       ipv4_search( char *flow, int (*ipv4_policy_compare)( const char *, const char *) )
{
	if( flow == NULL )
	{
		return NULL;
	}

	struct _ipv4_flow *p = NULL;
	struct list_head *tmp = NULL;
	struct list_head *head = NULL;
	struct _ipv4_struct *ipv4_node = NULL;
	int	flow_direction = 0;

	p = (struct _ipv4_flow *)flow;
	WriteLog( DEBUG, "[%s][%d]ifx=[%d]\n", __FILE__, __LINE__, if_index );

	//get flow direction
	if( memcmp(p->ether_shost, if_array[if_index].ether_host, sizeof(p->ether_shost)-1) == 0 )
	{
		p->flow_direction = FLOW_OUT;
		p->if_out = if_array[if_index].ifindex;
		head = &( ipv4_head_out[if_index].node_head );
	}
	else if( memcmp(p->ether_dhost, if_array[if_index].ether_host, sizeof(p->ether_shost)-1) == 0 )
	{
		p->flow_direction = FLOW_IN;
		p->if_in = if_array[if_index].ifindex;
		head = &( ipv4_head_in[if_index].node_head );
	}	

	WriteLog( DEBUG, "[%s][%d]flow direction=[%d]\n", __FILE__, __LINE__, p->flow_direction );

	//look up
	tmp = head->next;
	while( tmp != head )
	{
		ipv4_node = list_entry( tmp, struct _ipv4_struct, node );	
		//policy function
		if( ipv4_policy_compare( (char *)(ipv4_node), flow ) == TC_TRUE )
		{
			break;
		}

		tmp = tmp->next;
	}

	if( tmp == head )
	{
		tmp = NULL;
	}

	return tmp;
}

/**
 * this function is add a node to a system list
 * @param input:
        char *flow
 * @param output:
	struct list_head *p;
 * @return
        p != NULL:success
        p == NULL:fail
 */

int     ipv4_add( char *flow, int if_index )
{
	if( flow == NULL )
	{
		return TC_ERROR;
	}
	struct timeval start;

	struct _ipv4_struct *ipv4_struct = NULL;

//2011-07-09 deleted start by gaowg
/*************************
	ipv4_struct = (struct _ipv4_struct *)malloc(sizeof(struct _ipv4_struct) );
*************************/
//2011-07-09 deleted end by gaowg

	ipv4_struct = get_system_memory( if_index );
	if( ipv4_struct == NULL )
	{
		return TC_ERROR;
	}

	//memset( ipv4_struct, 0x00, sizeof(struct _ipv4_struct) );

	//copy some value to this node
	ipv4_struct->ip_v = 4;
	ipv4_struct->used = 1;
	memcpy( &(ipv4_struct->flow), (struct _ipv4_flow *)flow, sizeof(struct _ipv4_flow) );

	//insert this node to system list

	if( FLOW_OUT == ((struct _ipv4_flow *)flow)->flow_direction )
	{
		list_add_tail( &(ipv4_struct->node), &(ipv4_head_out[if_index].node_head) );
	}
	else if( FLOW_IN == ((struct _ipv4_flow *)flow)->flow_direction )
	{
		list_add_tail( &(ipv4_struct->node), &(ipv4_head_in[if_index].node_head) );
	}

	//add time
	gettimeofday( &start, NULL );	
	memcpy( &(ipv4_struct->start), &start ,sizeof(struct timeval) ); 
	memcpy( &(ipv4_struct->end), &start ,sizeof(struct timeval) ); 

	WriteLog( DEBUG, "[%s][%d]start time: %ld secs %ld micros\n", __FILE__, __LINE__, (ipv4_struct->start).tv_sec, (ipv4_struct->start).tv_usec );

	
	return TC_SUCCESS;
}

int     ipv4_update( struct list_head *p, char *flow )
{
	struct _ipv4_struct *ipv4_struct = NULL;	
	struct timeval end;

	if( p == NULL || flow == NULL )
	{
		WriteLog( ERR, "[%s][%d]input param error\n", __FILE__, __LINE__ );
		return TC_ERROR;
	}

	ipv4_struct = list_entry( p, struct _ipv4_struct, node );

	if( ipv4_struct == NULL )
	{
		return TC_ERROR;
	}

	(ipv4_struct->flow).total_send += ((struct _ipv4_flow *)flow)->total_send;

	gettimeofday( &end, NULL );	
	memcpy( &(ipv4_struct->end), &end ,sizeof(struct timeval) ); 
	WriteLog( DEBUG, "[%s][%d]end time: %ld secs %ld micros\n", __FILE__, __LINE__, (ipv4_struct->end).tv_sec, (ipv4_struct->end).tv_usec );

	return TC_SUCCESS;
}

int     ipv4_delete( struct list_head *p )
{
	struct _ipv4_struct *ipv4_struct = NULL;

	if( p == NULL )
	{
		return TC_ERROR;
	}

	ipv4_struct = list_entry( p, struct _ipv4_struct, node );

	WriteLog( DEBUG, "[%s][%d]\n", __FILE__, __LINE__ );

	//ipv4_print_data( (char *)(&(ipv4_struct->flow)) );

	//delete this node
	//list_del( p );

	free( ipv4_struct );
	WriteLog( DEBUG, "[%s][%d]errro=[%d]errstr=[%s]\n", __FILE__, __LINE__, errno, strerror(errno) );


	ipv4_struct = NULL;

	return TC_SUCCESS;
}

char	*ipv6_operate( const u_char *packet, int if_index )
{
	if( packet == NULL )
	{
		return NULL;
	}

	struct ether_header *ethernet_protocol = NULL;
	struct ip6_hdr *ip6h = NULL;

	//struct ether_header
	ethernet_protocol = (struct ether_header *)(packet);
	memcpy( ipv6_flow.ether_shost, ethernet_protocol->ether_shost, sizeof(ipv6_flow.ether_shost) );
	memcpy( ipv6_flow.ether_dhost, ethernet_protocol->ether_dhost, sizeof(ipv6_flow.ether_dhost) );

	//struct ip6 header
	ip6h = (struct ip6_hdr *)(packet+sizeof(struct ether_header) );
	memcpy( &(ipv6_flow.ip6_src), &(ip6h->ip6_src), sizeof(struct in6_addr) );
	memcpy( &(ipv6_flow.ip6_dst), &(ip6h->ip6_dst), sizeof(struct in6_addr) );
	ipv6_flow.total_send = ntohs(ip6h->ip6_plen);
	
	return (char *)(&ipv6_flow);
}

int	ipv6_print_data( const char *flow )
{
	if( flow == NULL )
	{
		return TC_ERROR;
	}

	struct _ipv6_flow *p = NULL;
	u_char	*mac_string = NULL;

	p = (struct _ipv6_flow *)flow;

	//interface address
	WriteLog( DEBUG, "[%s][%d]Mac Source Address is : \n", __FILE__, __LINE__ );
	mac_string = p->ether_shost;
	if( mac_string != NULL )
	{
		WriteLog( ERR, "%02x:%02x:%02x:%02x:%02x:%02x\n", *mac_string, *(mac_string + 1), *(mac_string + 2), *(mac_string + 3), *(mac_string + 4), *(mac_string + 5));
	}

	WriteLog( DEBUG, "[%s][%d]Mac Destination Address is : \n", __FILE__, __LINE__ );
	mac_string = NULL;
	mac_string = p->ether_dhost;
	if( mac_string != NULL )
	{
		WriteLog( ERR, "%02x:%02x:%02x:%02x:%02x:%02x\n", *mac_string, *(mac_string + 1), *(mac_string + 2), *(mac_string + 3), *(mac_string + 4), *(mac_string + 5));
	}

	//ip address

	char	ip_addr[INET6_ADDRSTRLEN+1];
	memset( ip_addr, 0x00, sizeof(ip_addr) );
	inet_ntop(AF_INET6, (void *)&(p->ip6_src), ip_addr, sizeof(ip_addr)-1);
	WriteLog( DEBUG, "[%s][%d]src ip=[%s]\n", __FILE__, __LINE__, ip_addr );

	memset( ip_addr, 0x00, sizeof(ip_addr) );
	inet_ntop(AF_INET6, (void *)&(p->ip6_dst), ip_addr, sizeof(ip_addr)-1);
	WriteLog( DEBUG, "[%s][%d]dst ip=[%s]\n", __FILE__, __LINE__, ip_addr );

	WriteLog( DEBUG, "[%s][%d]flow len=[%d]\n", __FILE__, __LINE__, p->total_send );

	return 0;
}

struct list_head*	ipv6_search( char *flow, int if_index )
{
	if( flow == NULL )
	{
		return NULL;
	}

	struct _ipv6_flow *p = NULL;
	struct list_head *tmp = NULL;
	struct list_head *head = NULL;
	struct _ipv6_struct *ipv6_node = NULL;

	//p = (struct _ipv6_flow *)flow;

	//struct _tc_sys_list     ipv6_head;
	//look up
	head = &(ipv6_head[if_index].node_head);
	tmp = head->next;
	//tmp = ipv6_head.node_head.next;

	while( tmp != head )
	//while( tmp != &(ipv6_head.node_head) )
	{
	//WriteLog( DEBUG, "\n\n\n[%s][%d]\n", __FILE__, __LINE__ );
		ipv6_node = list_entry( tmp, struct _ipv6_struct, node );	
		//policy function
		if( ipv6_policy_compare( (char *)(ipv6_node), flow ) == TC_TRUE )
		{
			break;
		}

		tmp = tmp->next;
	}

	if( tmp == head )
	//if( tmp == &(ipv6_head.node_head) )
	{
		tmp = NULL;
	}

	return tmp;
}

int     ipv6_add( char *flow, int if_index )
{
	if( flow == NULL )
	{
		return TC_ERROR;
	}

	struct _ipv6_struct *ipv6_struct = NULL;

	ipv6_struct = (struct _ipv6_struct *)malloc(sizeof(struct _ipv6_struct) );
	if( ipv6_struct == NULL )
	{
		return TC_ERROR;
	}

	//memset( ipv6_struct, 0x00, sizeof(struct _ipv6_struct) );

	//copy some value to this node
	ipv6_struct->ip_v = 6;
	ipv6_struct->used = 1;
	memcpy( &(ipv6_struct->flow), (struct _ipv6_flow *)flow, sizeof(struct _ipv6_flow) );

	//insert this node to system list
	list_add_tail( &(ipv6_struct->node), &(ipv6_head[if_index].node_head) );

	return TC_SUCCESS;
}

int     ipv6_update( struct list_head *p, char *flow )
{
	struct _ipv6_struct *ipv6_struct = NULL;	

	if( p == NULL || flow == NULL )
	{
		return TC_ERROR;
	}

	ipv6_struct = list_entry( p, struct _ipv6_struct, node );

	if( ipv6_struct == NULL )
	{
		return TC_ERROR;
	}

	(ipv6_struct->flow).total_send += ((struct _ipv6_flow *)flow)->total_send;

	return TC_SUCCESS;
}

int     ipv6_delete( struct list_head *p )
{
	struct _ipv6_struct *ipv6_struct = NULL;

	if( p == NULL )
	{
		return TC_ERROR;
	}

	ipv6_struct = list_entry( p, struct _ipv6_struct, node );

	free( ipv6_struct );

	ipv6_struct = NULL;

	return TC_SUCCESS;
}

static int	init_flow_operation()
{
	flow_operation[0].ip_v = 4;
	flow_operation[0].operate = ipv4_operate;
	flow_operation[0].print_data = ipv4_print_data;
	flow_operation[0].search = ipv4_search;
	flow_operation[0].add = ipv4_add;
	flow_operation[0].update = ipv4_update;
	flow_operation[0].delete = ipv4_delete;

	flow_operation[1].ip_v = 6;
	flow_operation[1].operate = ipv6_operate;
	flow_operation[1].print_data = ipv6_print_data;
	flow_operation[1].search = ipv6_search;
	flow_operation[1].add = ipv6_add;
	flow_operation[1].update = ipv6_update;
	flow_operation[1].delete = ipv6_delete;

	return TC_SUCCESS;
}

/**************************************

 *
 * function init_tc_sys_list()
 *
 * inputs:	null
 * outputs:     null
 * return: 
 * create date: 20110608
 * describe: create system list that stored flow information

**************************************/

static void	init_tc_sys_list()
{
	int i = 0;

	for( i=0; i<g_interface_size&&i<MAX_INTERFACE_SIZE; i++ )
	{
		INIT_LIST_HEAD( &(ipv4_head_in[i].node_head) );
		INIT_LIST_HEAD( &(ipv4_head_out[i].node_head) );
		INIT_LIST_HEAD( &(ipv6_head[i].node_head) );
	}
}

/**************************************

 *
 * function init_if_array()
 *
 * inputs:	null
 * outputs:     null
 * return: 
 * create date: 20110709
 * describe: init if_array struct

**************************************/

static void init_if_array( struct _interface_struct *if_array, int if_num, int max_num )
{
	int i = 0;

	for( i=0; i<if_num ; i++ )
	{
		memset( &(if_array[i].element), 0x00, sizeof(struct _element_struct) );
		if_array[i].element.maxsize = max_num;
	}
}

//test test test
void test_free()
{
	int *a = NULL;
	a = (int *)malloc(sizeof(int) );
	*a = 4;
	WriteLog( DEBUG, "[%s][%d]a=[%d]\n", __FILE__, __LINE__, *a );
	free( a );
	WriteLog( DEBUG, "[%s][%d]errro=[%d]errstr=[%s]\n", __FILE__, __LINE__, errno, strerror(errno) );
}

struct _ipv4_struct *get_system_memory( int ifindex )
{
	struct _interface_struct *p = NULL;
	int	tail = 0;

	if( ifindex < 0 || ifindex > g_interface_size )
	{
		WriteLog( ERR, "[%s][%d]error\n", __FILE__, __LINE__ );
		return NULL;
	}	

	p = &( if_array[ifindex] );

	tail = (p->element).tail;
	if( tail + 1 == (p->element).maxsize )
	{
		WriteLog(  ERR, "[%s][%d]error\n", __FILE__, __LINE__ );
		return NULL;
	}

	(p->element).tail++;
	(p->element).length++;

	return &( (p->element).node[tail] );
}

/**************************************

 *
 * function init_if_array()
 *
 * inputs:	null
 * outputs:     null
 * return: 
 * create date: 20110908
 * describe: init sampled struct

**************************************/

static void init_interface_sampled_struct( struct _sampled_struct *sampled_struct, int if_num )
{
	int i = 0;

	//init	sample struct
	for( i=0; i<if_num ; i++ )
	{
		//2011-09-12 for test
		//sampled_struct[i].out_sampled_rate = 1;
		//2011-09-12 end
		sampled_struct[i].out_sampled_rate = 1;
		sampled_struct[i].total_packets = 0;
		sampled_struct[i].sampled_packets = 0;
		sampled_struct[i].sampled_index = 0;
		sampled_struct[i].captured_time = g_host_config.captured_time;
		sampled_struct[i].interval_time = g_host_config.interval;
		sampled_struct[i].total_send_rate = 0;
	}
}

static void print_interface_sampled_param( struct _interface_sampled_param *if_sampled_array, int interface_num )
{
	int	i = 0;
	for( i=0; i<interface_num; i++ )
	{
		print_sys_sampled_param( if_sampled_array[i].sys_sampled_param, PARAM_LEN );
	}
}

static void print_interface_sampled_struct( struct _sampled_struct *sampled_struct, int if_num )
{
	int i = 0;

	//init	sample struct
	for( i=0; i<if_num ; i++ )
	{
		WriteLog( DEBUG, "[%s][%d]captured_time=[%d]\n", __FILE__, __LINE__, sampled_struct[i].captured_time );
	}
}

/**************************************

 *
 * function init_interface_sampled_param()
 *
 * inputs:	null
 * outputs:     null
 * return: 
 * create date: 20110908
 * describe: init sampled struct 
	初始化每块网卡的抽样参数初始值

**************************************/

static void init_interface_sampled_param( struct _interface_sampled_param *if_sampled_array, int interface_num )
{
	int	i = 0;

	for( i=0; i<interface_num; i++ )
	{
		init_sys_sampled_param( if_sampled_array[i].sys_sampled_param, PARAM_LEN );
	}

}

/**************************************

 *
 * function init_if_array()
 *
 * inputs:	null
 * outputs:     null
 * return: 
 * create date: 20110908
 * describe: init sys sampled param

**************************************/

static void init_sys_sampled_param( struct _sys_sampled_param *sampled_struct, int num )
{
	int i = 0;

	//init	sample struct
	for( i=0; i<num ; i++ )
	{
		sampled_struct[i].out_sampled_rate = (i+1);
		//sampled_struct[i].out_sampled_rate = (i+1)/10.0;
		sampled_struct[i].interval_time = i+1;
	}
}

static void print_sys_sampled_param( struct _sys_sampled_param *sampled_struct, int num )
{
	int i = 0;

	//init	sample struct
	for( i=0; i<num ; i++ )
	{
		WriteLog( DEBUG, "[%s][%d]sampled_rate=[%d],interval=[%d]\n", __FILE__, __LINE__, sampled_struct[i].out_sampled_rate, sampled_struct[i].interval_time );
	}
}

void	init_pcap_fp()
{
	int	i = 0;
	for( i=0; i<g_interface_size; i++ )
	{
		init_one_pcap_fp( i );
	}

	return ;
}

static void init_one_pcap_fp( int if_index )
{
	char	error_content[PCAP_ERRBUF_SIZE];
	char	*net_interface;
	struct bpf_program bpf_filter;
	char	bpf_filter_string[MAX_BUFF_LEN];
	char	ether_host[MAX_BUFF_LEN];
	u_char	*mac_string = NULL;
	bpf_u_int32 net_mask;
	bpf_u_int32 net_ip;
	struct pcap_pkthdr protocol_header;
	pcap_t	*pcap_handle = NULL;

	memset( bpf_filter_string, 0x00, sizeof(bpf_filter_string) );
	memset( ether_host, 0x00, sizeof(ether_host) );

	//get mac
	mac_string = if_array[if_index].ether_host;
	snprintf( ether_host, sizeof(ether_host)-1, "%02x:%02x:%02x:%02x:%02x:%02x", *mac_string, *(mac_string + 1), *(mac_string + 2), *(mac_string + 3), *(mac_string + 4), *(mac_string + 5) );
	
	//config bpf_filter_string
	snprintf( bpf_filter_string, sizeof(bpf_filter_string)-1, "ip and (ether src %s or ether dst %s )", ether_host, ether_host );

	net_interface = if_array[if_index].name;
	WriteLog( DEBUG,  "[%s][%d]interface name=[%s]\n", __FILE__, __LINE__, net_interface );

	pcap_handle = pcap_open_live( net_interface, BUFSIZ, 1, 0, error_content);
	pcap_compile( pcap_handle, &bpf_filter, bpf_filter_string, 0, net_ip );
	pcap_setfilter( pcap_handle, &bpf_filter );
	if( pcap_datalink(pcap_handle) != DLT_EN10MB )
	{
		WriteLog( ERR,  "[%s][%d]\n", __FILE__, __LINE__ );
		return ;
	}

	interface_pacp_handle[if_index] = pcap_handle;
	
	WriteLog( DEBUG, "[%s][%d]pcap_handle=[%x]\n", __FILE__, __LINE__, pcap_handle );

	return ;
}

void	close_interface_pcap_fp()
{
	pcap_t	*pcap_handle = NULL;
	int	i = 0;
	for( i=0; i<g_interface_size; i++ )
	{
		pcap_handle = interface_pacp_handle[i];
		WriteLog( DEBUG, "[%s][%d]pcap_handle=[%x]\n", __FILE__, __LINE__, pcap_handle );
		if( pcap_handle != NULL )
		{
			pcap_close(pcap_handle);
		}
	}
}

/**************************************

 *
 * function init_queue_array()
 *
 * inputs:	null
 * outputs:     null
 * return: 
 * create date: 20110709
 * describe: init queue_array struct

**************************************/

static void init_queue_array( struct _interface_queue *queue_array, int if_num )
{
	int i = 0;
	MyQueue *queue = NULL;	

	for( i=0; i<if_num ; i++ )
	{
		queue = QueueCreate( sizeof(TYPE), QUEUENUM );
		if( queue == NULL )
		{
			WriteLog(  ERR, "[%s][%d]QueueCreate error\n", __FILE__, __LINE__ );
			return ;			
		}
		queue_array[i].queue = queue;
		
		queue_array[i].sum_x = 0;
		queue_array[i].sum_y = 0;
	}
}

//得到系统抽样包个数
void	get_best_sampled_count()
{
	int	nmax = 0;	//系统最大处理能力,即最大抽样包个数
	double	a = 0;		//系统最大处理能力前面的系数

	double	zp = 0;
	double	p0 = 0;
	int	np = 0;		//根据p0得到最优抽样包个数

	nmax = g_host_config.nmax;
	a = g_host_config.a;

	WriteLog( DEBUG, "[%s][%d]nmax=[%d]\n", __FILE__, __LINE__, nmax );
	WriteLog( DEBUG, "[%s][%d]a=[%.2f]\n", __FILE__, __LINE__, a );

	nmax = a * nmax;

	zp = get_zp();
	p0 = g_host_config.p;
	WriteLog( DEBUG, "[%s][%d]zp=[%.2f]\n", __FILE__, __LINE__, zp );
	WriteLog( DEBUG, "[%s][%d]p0=[%.2f]\n", __FILE__, __LINE__, p0 );

	np =  get_n( zp, p0 );
	WriteLog( NORMT, "[%s][%d]np=[%d]\n", __FILE__, __LINE__, np );

	system_best_sampled_n = nmax < np ? nmax:np;

	WriteLog( DEBUG, "[%s][%d]system_best_sampled_n=[%.2f]\n", __FILE__, __LINE__, system_best_sampled_n );

	return;
}

//得到zp
double get_zp()
{
	return 271;
}

//得到最小抽样包个数
int	get_n( double  zp, double p0 )
{
	int n = 0;
	double c0 = 0;
	double a = 0;

	if( 0 != p0 )
	{
		c0 = 1/p0 - 1;
	}

	n = (int)( zp * c0 );

	return n;
}

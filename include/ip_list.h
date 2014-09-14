#ifndef	_ANALYSIS_H
#define	_ANALYSIS_H

#include "list.h"
#include "public.h"

struct _tc_sys_list
{
	struct list_head	node_head;	//list head position
};

//struct _tc_sys_list	ipv4_head[MAX_INTERFACE_SIZE];
struct _tc_sys_list	ipv6_head[MAX_INTERFACE_SIZE];

struct _tc_sys_list	ipv4_head_in[MAX_INTERFACE_SIZE];
struct _tc_sys_list	ipv4_head_out[MAX_INTERFACE_SIZE];
//struct _tc_sys_list	ipv6_head_in[MAX_INTERFACE_SIZE];
//struct _tc_sys_list	ipv6_head_out[MAX_INTERFACE_SIZE];

/*
struct _tc_sys_list	ipv4_head;
struct _tc_sys_list	ipv6_head;
*/

//identify what flow is
struct _ipv4_flow
{
	struct in_addr ip_src;	
	struct in_addr ip_dst;
	__u16 port_src;
	__u16 port_dst;
	__u8 dscp;
	u_int8_t  ether_shost[ETH_LEN+1];      /* source ether addr    */	
	u_int8_t  ether_dhost[ETH_LEN+1];      /* destination eth addr */
	__u32	flow_direction:1;		/* flow direction 0:input;1:output*/
	__u32	if_in;				/* which interface receive the flow */
	__u32	if_out;				/* which interface send the flow */
	__u32	total_send;
};

//identify what flow is
struct _ipv6_flow
{
	struct in6_addr ip6_src;
	struct in6_addr ip6_dst;
	u_int8_t  ether_shost[ETH_LEN];      /* source ether addr    */	
	u_int8_t  ether_dhost[ETH_LEN];      /* destination eth addr */
	__u32	total_send;
};

struct	_ipv4_struct
{
	struct list_head	node;
	__u32	ip_v:4;
	struct _ipv4_flow flow;
	__u32	used:1;
	struct timeval start;
	struct timeval end;
};

struct	_ipv6_struct
{
	struct list_head	node;
	__u32	ip_v:4;
	struct _ipv6_flow flow;
	__u32	used:1;
};

struct _ipv4_flow ipv4_flow[MAX_INTERFACE_SIZE];
struct _ipv6_flow ipv6_flow;

struct _ip_func
{
	unsigned int	ip_v;
	// "char *flow" is equal to "ipv4_flow *flow"
	// or "char *flow" is equal to "ipv6_flow *flow"
	
	char *(*operate)( const u_char *packet, int if_index );
	int (*print_data)( const char *flow );
	struct list_head*	(*search)( char *flow, int if_index );
	//add one flow
	int	(*add)( char *flow, int if_index );
	//update one flow
	int	(*update)( struct list_head *p, char *flow );
	//delete one flow
	int	(*delete)( struct list_head *p );
};

//pma agent struct
/*
struct _tc_config_struct
{
	int			router_id;
	struct sockaddr_storage	pma_addr;
	int			pma_port;	
	//other information
	int			captured_time;
	int			interval;
};

struct _tc_config_struct g_host_config;
*/

struct _ip_func	flow_operation[2];

#define FLOW_IN 0
#define FLOW_OUT 1

#endif

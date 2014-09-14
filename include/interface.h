#ifndef _INTERFACE_H
#define _INTERFACE_H

#include "public.h"
#include "ip_list.h"
#include "queue.h"

#define MAX_FLOW_NUM 100
#define INTERVAL_TIME 2

//存放流结构信息的head结构体
//head struct
struct _element_struct
{
	struct _ipv4_struct node[MAX_FLOW_NUM];
	int	tail;		//队列尾
	int	length;		//队列当前长度
	int	maxsize;	//队列最大长度
};

//local interface struct
struct _interface_struct
{
	char    name[50];		//网卡名称
	char    address[50];		//网卡地址
	u_int8_t  ether_host[ETH_LEN+1];	//MAC地址
	int     active;				//网卡状态
	int	interval;			//间隔时间
	struct net_device_stats device_stats;	//设备信息
	struct _element_struct element;		//
	int	ifindex;
};

//以下为抽样部分结构体
//sampled struct	
//每个网卡上用于分层抽样的抽样结构体
//包括抽样率等信息
struct _sampled_struct
{
	float	out_sampled_rate;	//抽样率
	int	total_packets;		//抽样时间内总的包个数
	int	sampled_packets;	//抽样时间内抽样的包个数
	int	sampled_index;		//对应抽样参数数组中的数组下标
	int	captured_time;		//捕获时间
	int	interval_time;		//间隔时间
	int	total_send_rate;	//总的发送速率
};

//系统抽样参数
struct _sys_sampled_param
{
	int	out_sampled_rate;
	int	interval_time;
};

#define	PARAM_LEN	10
struct _sampled_struct		interface_sampled_struct[MAX_INTERFACE_SIZE];

//每块网卡抽样参数
struct _interface_sampled_param
{
	struct _sys_sampled_param	sys_sampled_param[PARAM_LEN];
};

struct _interface_sampled_param	if_sampled_array[MAX_INTERFACE_SIZE];

struct _interface_struct	if_array[MAX_INTERFACE_SIZE];
#define PATH_PROC_NET_DEV "/proc/net/dev"


#define _PATH_PROCNET_IFINET6           "/proc/net/if_inet6"

struct _interface_info
{
	char    name[50];
	char    ipv4_addr[INET6_ADDRSTRLEN];
	char    ipv4_broad_addr[INET6_ADDRSTRLEN];
	char    ipv4_netmask[INET6_ADDRSTRLEN];	
	char    ipv6_addr[512];
	//char    ipv6_addr[INET6_ADDRSTRLEN];
	int    ipv6_addr_netmask;
	u_int8_t  ether_host[ETH_LEN+1];
	struct net_device_stats device_stats;	//设备信息
	int     active;		//1:active; !1: not active
	int	if_index;
	unsigned long rx_bytes;
	unsigned long tx_bytes;
};

struct _interface_info	ife[MAX_INTERFACE_SIZE];
pcap_t *interface_pacp_handle[MAX_INTERFACE_SIZE];

//回归分析用
struct _interface_queue
{
	MyQueue	*queue;
	TYPE	sum_x;
	TYPE	sum_y;
};

struct _interface_queue queue_array[MAX_INTERFACE_SIZE];

#endif

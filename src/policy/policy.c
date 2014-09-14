/************************************************************

Copyright (C), 2011-, BUPT
FileName:      policy.c
Author:
Version :
Date:
Description:    this file included all kinds of policy functions.
Function List:
Modify History:
<author> <time> <version > <desc>

**********************************************************/

#include "public_include.h"
#include "public.h"
#include "ip_list.h"

int	ipv4_policy_compare( char *compared_ipv4_struct, char *current_ipv4_flow_struct )
{
	int	iret = TC_FAIL;
	struct _ipv4_struct *compared_node = NULL;
	struct _ipv4_flow *current_flow = NULL;

	compared_node = (struct _ipv4_struct *) compared_ipv4_struct;
	current_flow = (struct _ipv4_flow *) current_ipv4_flow_struct;

	//WriteLog( DEBUG, "[%s][%d]\n", __FILE__, __LINE__ );

	//以后需要根据策略表进行策略比较,即流特征比较 ???
	if( memcmp( &((compared_node->flow).ip_src), &(current_flow->ip_src), sizeof(struct in_addr) ) == 0 
		&& memcmp( &((compared_node->flow).ip_dst), &(current_flow->ip_dst), sizeof(struct in_addr) ) == 0 )
	{
		iret = TC_TRUE;
	}
	
	return iret;
}

int	ipv6_policy_compare( char *compared_ipv6_struct, char *current_ipv6_flow_struct )
{
	int	iret = TC_FAIL;
	struct _ipv6_struct *compared_node = NULL;
	struct _ipv6_flow *current_flow = NULL;

	compared_node = (struct _ipv6_struct *) compared_ipv6_struct;
	current_flow = (struct _ipv6_flow *) current_ipv6_flow_struct;

	//WriteLog( DEBUG, "[%s][%d]\n", __FILE__, __LINE__ );

	//以后需要根据策略表进行策略比较,即流特征比较 ???
	if( memcmp( &((compared_node->flow).ip6_src), &(current_flow->ip6_src), sizeof(struct in6_addr) ) == 0 
		&& memcmp( &((compared_node->flow).ip6_dst), &(current_flow->ip6_dst), sizeof(struct in6_addr) ) == 0 )
	{
		iret = TC_TRUE;
	}
	
	return iret;
}

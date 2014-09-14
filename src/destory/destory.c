/************************************************************

Copyright (C), 2011-, BUPT
FileName:       util.c
Author:
Version :
Date:
Description:    there are some free functions in this file.
Function List:
Modify History:
<author> <time> <version > <desc>

**********************************************************/
#include "public_include.h"
#include "ip_list.h"

extern pcap_t *pcap_handle;

void	free_module()
{
	WriteLog( LOGFILE, "[%s][%d]destory module here\n", __FILE__, __LINE__ );

	struct list_head *tmp = NULL;
	struct list_head *head = NULL;
	struct list_head *old = NULL;
	struct list_head *l = NULL;
	int	i = 0;

	//free ipv4
	for( i=0; i<g_interface_size; i++ )
	{
		//out data
		head = &(ipv4_head_out[i].node_head);
		tmp = head->next;
		while( tmp != head )
		{
			old = tmp;
			tmp = tmp->next;
			ipv4_delete( old );
		}
		head = NULL;

		//in data
		head = &(ipv4_head_in[i].node_head);
		tmp = head->next;
		while( tmp != head )
		{
			old = tmp;
			tmp = tmp->next;
			ipv4_delete( old );
		}
		head = NULL;
	}
	
	//free ipv6
	for( i=0; i<g_interface_size; i++ )
	{
		head = &(ipv6_head[i].node_head);
		tmp = head->next;
		//tmp = ipv6_head.node_head[i].next;
		while( tmp != head )
		//while( tmp != &(ipv6_head[i].node_head) ) 
		{
			old = tmp;
			tmp = tmp->next;
			ipv6_delete( old );
		}
	}

	//pcap_close(pcap_handle);
	//pcap_handle = NULL;

	exit( 1 );
}

void	local_free_module()
{
	//pcap_close(pcap_handle);
	//pcap_handle = NULL;

	WriteLog(LOGFILE,  "[%s][%d]destory module here\n", __FILE__, __LINE__ );
	sleep( 1 );

	struct list_head *tmp = NULL;
	struct list_head *head = NULL;
	struct list_head *old = NULL;
	struct list_head *l = NULL;
	int	i = 0;

	//free ipv4
	for( i=0; i<g_interface_size; i++ )
	{
		//out data
		head = &(ipv4_head_out[i].node_head);
		tmp = head->next;
		//tmp = ipv4_head.node_head.next;
		while( tmp != head )
		//while( tmp != &(ipv4_head[i].node_head) ) 
		{
			old = tmp;
			tmp = tmp->next;
			ipv4_delete( old );
		}
		head = NULL;

		//in data
		head = &(ipv4_head_in[i].node_head);
		tmp = head->next;
		while( tmp != head )
		{
			old = tmp;
			tmp = tmp->next;
			ipv4_delete( old );
		}
		head = NULL;
	}
	
	//free ipv6
	for( i=0; i<g_interface_size; i++ )
	{
		head = &(ipv6_head[i].node_head);
		tmp = head->next;
		//tmp = ipv6_head.node_head[i].next;
		while( tmp != head )
		//while( tmp != &(ipv6_head[i].node_head) ) 
		{
			old = tmp;
			tmp = tmp->next;
			ipv6_delete( old );
		}
	}

}

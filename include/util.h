#ifndef _UTIL_H
#define	_UTIL_H

#include "interface.h"

/* Local definitions */
//int nl_sequence_number = 0;
//int s_net = 0;

#define TRUE 1
#define FALSE 0

int get_all_interfaces_info( int s_net, int nl_sequence_number, struct _interface_struct *if_array );
int tc_netlink_open( int *nl_sequence_number );

void    init_list( struct _tc_sys_list *list );
int get_diff_time( struct timeval *start, struct timeval *end );

#endif

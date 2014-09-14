/************************************************************

Copyright (C), 2011-, BUPT
FileName:       util.c
Author:
Version :
Date:
Description:    there are some utility functions in this file.
Function List:
Modify History:
<author> <time> <version > <desc>

**********************************************************/
#include "public_include.h"
#include "public.h"
#include "ip_list.h"
//#include "interface.h"
#include "util.h"

#define SCH_LINE_MAX 512

void	gettime(char *cur_date, char *cur_time);
int     WriteLog( int level, char *cFormat, ... );
//int     WriteLog( char *filename, char *cFormat, ... );
int     WriteLog1( char *filename, char *cFormat, ... );
int	get_device_stats( struct net_device_stats *net_stats, int ifindex );

void	init_list( struct _tc_sys_list *list )
{
}

int	get_diff_time( struct timeval *start, struct timeval *end )	
{
	int timeuse = 0;

	//2011-07-07 added by gaowg
	if( end->tv_sec > start->tv_sec ||
	( end->tv_sec == start->tv_sec && end->tv_usec > start->tv_usec ) )
	{
		timeuse = 1000000*( (end->tv_sec)-(start->tv_sec) ) + ( (end->tv_usec)-(start->tv_usec) );
	}

	return timeuse;
}

/*********
int	get_all_interfaces( struct _interface_struct *if_array )
{
	FILE * fp;
	char line[SCH_LINE_MAX];
	int i;
	g_interface_size = 0;
	char * pc = NULL;

	if( !(fp = fopen(PATH_PROC_NET_DEV, "r")) )
	{
		perror("fopen while open dev");
		exit(1);
	}
	// Skip first two lines
	fgets(line, sizeof(line), fp);
	fgets(line, sizeof(line), fp);

	while( fgets(line, sizeof(line), fp) )
	{
		i = 0;
		// Truncate the line at char ':'
		if( (pc = strchr(line, ':')) )
		{
			*pc = '\0';
			pc = line;
			// Skip the spaces ahead
			while( *pc == ' ' )
				pc++;
			// now we got the interface name
			if( strncmp(pc, "lo", 2) 
				&& strncmp(pc, "sit", 3) 
				&& strncmp(pc, "teredo", 6) 
				&& strncmp(pc, "teql0", 5) )
			{
				memset( if_array[g_interface_size].name, '\0', sizeof(if_array[g_interface_size].name));
				memcpy( if_array[g_interface_size].name, pc, sizeof(if_array[g_interface_size].name) );

				g_interface_size++;
			}
		}
	}

	fclose(fp);

	return TC_SUCCESS;
}
**********/

void	print_all_interfaces( struct _interface_struct *if_array )
{
	int	i = 0;
	WriteLog( DEBUG, "[%s][%d]g_interface_size=[%d]\n", __FILE__, __LINE__, g_interface_size );
	for( i=0; i<g_interface_size; i++ )
	{
		//interface name
		WriteLog( DEBUG, "[%s][%d]idx=[%d],name=[%s]\n", __FILE__, __LINE__, i, if_array[i].name );

		//interface mac
		u_char *mac_string = if_array[i].ether_host;
		if( mac_string != NULL )
		{
			WriteLog(DEBUG, "[%s][%d]%02x:%02x:%02x:%02x:%02x:%02x\n", __FILE__, __LINE__, *mac_string, *(mac_string + 1), *(mac_string + 2), *(mac_string + 3), *(mac_string + 4), *(mac_string + 5));
		}

		//
	}
}

/*
 * function tc_netlink_open()
 *
 * Opens and binds a Netlink socket, setting s_net.
 *
 * Returns 0 on success, -1 otherwise.
 */

int tc_netlink_open( int *nl_sequence_number )
{
	struct sockaddr_nl local;
	int	s_net = 0;
	
	if ((s_net = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE)) < 0)
		return(-1);

	memset( &local, 0, sizeof(local));
	local.nl_family = AF_NETLINK;
	/* subscribe to link, IPv4/IPv6 address notifications */
	//local.nl_groups = 0;
	local.nl_groups = (RTMGRP_LINK| RTMGRP_IPV6_IFADDR);//delete by steven 0914 RTMGRP_IPV6_IFADDR
	
	if (bind(s_net, (struct sockaddr *)&local, sizeof(local)) < 0)
		return(-1);
	
	*nl_sequence_number = time(NULL);

	return s_net;
}

/* 
 * function get_my_addresses()
 *
 * Use the Netlink interface to retrieve a list of addresses for this
 * host's interfaces, and stores them into global my_addr_head list.
 */

int	get_all_interfaces_info( int s_net, int nl_sequence_number, struct _interface_struct *if_array )
{
	if( if_array == NULL )
	{
		return -1;
	}	

	/* these are used for passing messages */
	//struct sockaddr_storage ss_addr;
	//struct sockaddr *addr = (struct sockaddr*) &ss_addr;
	struct sockaddr_nl nladdr;
	char buf[8192];
	struct nlmsghdr *h;
	int status;
	char done;
	int	ifindex = 0;
	struct net_device_stats *net_stats = NULL;

	/* netlink packet */
	struct {
		struct nlmsghdr	n;
		struct rtgenmsg g;
	} req;

	struct iovec iov = { buf, sizeof(buf) };
	/* message response */
	struct msghdr msg = {
		(void*)&nladdr, sizeof(nladdr),
		&iov, 1,
		NULL, 0,
		0
	};

	/* setup request */
	memset(&req, 0, sizeof(req));
	req.n.nlmsg_len = sizeof(req);
	req.n.nlmsg_type = RTM_GETLINK;
	req.n.nlmsg_flags = NLM_F_REQUEST|NLM_F_DUMP;
	//req.n.nlmsg_flags = NLM_F_ROOT|NLM_F_MATCH|NLM_F_REQUEST;
	req.n.nlmsg_pid = 0;
	req.n.nlmsg_seq = ++nl_sequence_number;
	req.g.rtgen_family = 0;//AF_PACKET;//AF_UNSPEC;

	/* send request */
	memset(&nladdr, 0, sizeof(nladdr));

	nladdr.nl_family = AF_NETLINK;
	if (sendto(s_net, (void*)&req, sizeof(req), 0, 
		(struct sockaddr*)&nladdr,sizeof(nladdr)) < 0) {
		WriteLog( ERR, "Netlink: sentdo() error: %s\n", strerror(errno));
		return(-1);
	}

	/* receiving loop 1
	 * call recvmsg() repeatedly until we get a message
	 * with the NLMSG_DONE flag set
	 */
	done = FALSE;
	while(!done) {
		/* get response */
		if ((status = recvmsg(s_net, &msg, 0)) < 0) {
			WriteLog( ERR, "Netlink: recvmsg() error!\nerror: %s\n",
			    strerror(errno));
			return(-1);
		}

		/* parse response - loop 2
		 * walk list of NL messages returned by this recvmsg()
		 */
		h = (struct nlmsghdr*) buf;
		while (NLMSG_OK(h, (__u32)status)) {
			int len;
			struct ifinfomsg *ifi;
			struct rtattr *rta;

			/* exit this loop on end or error
			 */
			if (h->nlmsg_type == NLMSG_DONE) {
				done = TRUE;
				break;
			}
			if (h->nlmsg_type == NLMSG_ERROR) {
				WriteLog( ERR, "Error in Netlink response.\n");
				break;
			}

			ifi = NLMSG_DATA(h);
	/*
			//WriteLog( DEBUG, "[%s][%d]interface id=[%d]\n", __FILE__, __LINE__, ifi->ifi_index );
			if( (ifi -> ifi_flags & IFF_UP ) == IFF_UP )
			{
				if_array[ifi->ifi_index-2].active = 1;
				WriteLog( DEBUG, "\n\n UP\n\n" );
			}
	*/

			rta = IFLA_RTA(ifi);
			len = NLMSG_PAYLOAD( h, sizeof(struct ifinfomsg) );
			//len = h->nlmsg_len - NLMSG_LENGTH(sizeof(*ifa));

			/* parse list of attributes into table
			 * (same as parse_rtattr()) */
			while (RTA_OK(rta, len)) 
			{
				ifindex = ifi->ifi_index - 2;

				if( ifindex >= 0 && ifi->ifi_index < MAX_INTERFACE_SIZE )
				{
//printf( "[%s][%d]index=[%d]\n", __FILE__, __LINE__, ifi->ifi_index );
					if_array[ifindex].ifindex = ifi->ifi_index;

					int rta_type =  rta->rta_type;
					if( rta_type == IFLA_IFNAME )
					{
						//WriteLog( DEBUG, "[%s][%d]interface name=[%s]len=[%d]\n", __FILE__, __LINE__, (char *)RTA_DATA(rta), strlen((char *)RTA_DATA(rta)) );
						if( strcmp((char *)RTA_DATA(rta), "tap0") == 0 )
						{
							break;
						}
						memcpy( if_array[ifindex].name, (char *)RTA_DATA(rta), sizeof(if_array[ifindex].name) );
						//WriteLog( DEBUG, "interface name=[%s]\n", if_array[ifindex].name );
					}
					else if( rta_type == IFLA_ADDRESS )
					{
						if( ifi->ifi_type == ARPHRD_ETHER )
						{
							memcpy( if_array[ifindex].ether_host, RTA_DATA(rta), sizeof(if_array[ifindex].ether_host) );
							//WriteLog( DEBUG, "mac=[%s]\n", ether_ntoa(ether) );
							if( (ifi -> ifi_flags & IFF_UP ) == IFF_UP )
							{
								if_array[ifindex].active = 1;
								g_interface_size++;
							}
						}
					}
					else if( rta_type == IFLA_STATS )
					{
						net_stats = (struct net_device_stats *) RTA_DATA(rta);
						get_device_stats( net_stats, ifindex );	
					}
				}

				rta = RTA_NEXT(rta,len);
			}

			h = NLMSG_NEXT(h, status);
		} /* end while(NLMSG_OK) - loop 2 */
	} /* end while(!done) - loop 1 */ 
	
	//WriteLog( DEBUG, "\n");

	return(0);
}

/*
 *	get current data and time
 */

void    get_data_time( char *timestamp, int len )
{
        char    cur_data[20];
        char    cur_time[20];

        memset( cur_data, 0x00, sizeof(cur_data) );
        memset( cur_time, 0x00, sizeof(cur_time) );
        gettime( cur_data, cur_time );

        snprintf( timestamp, len, "%s %s", cur_data, cur_time );
}

/*
 *	get current time
 */

void	gettime(char *cur_date, char *cur_time)
{
	struct tm	*t;
	time_t		timer;

	timer = time(NULL);

	t = localtime(&timer);
	t->tm_year += 1900;
	t->tm_mon ++;

	sprintf(cur_date, "%04d-%02d-%02d", t->tm_year, t->tm_mon, t->tm_mday);
	sprintf(cur_time, "%02d:%02d:%02d", t->tm_hour, t->tm_min, t->tm_sec);

	//cur_date[8] = 0;
	//cur_time[8]=0;
}

/*
 * function read_hostconfig_file()
 *
 * Load configuration options from the XML file
 * stored in etc/host_config.xml
 *
 */

int read_hostconfig_file( char *filename )
{
	xmlDocPtr		doc = NULL;
	xmlNodePtr		node = NULL;
	char			*data = NULL;
	struct sockaddr_storage	ss_addr;
	struct sockaddr		*addr = NULL;

	addr = (struct sockaddr*) &ss_addr;

	//open xml file
	if( filename == NULL )
	{
		return -1;
	}

	doc = xmlParseFile( filename );
	if( doc == NULL ) 
	{
		WriteLog( ERR, "[%s][%d]Error parsing xml file (%s),err=[%s]\n", __FILE__, __LINE__,  filename, strerror(errno) );
		return( -1 );
	}

	memset( &g_host_config, 0x00, sizeof(g_host_config) );
	//get node info
	node = xmlDocGetRootElement( doc );
	for( node=node->children; node!=NULL; node=node->next )
	{
		/* skip entity refs */
		if( strcmp((char *)node->name, "text")==0 )
			continue;
		
		data = (char *)xmlNodeGetContent( node );

		if( strcmp((char *)node->name, "pma_addr") == 0 ) 
		{
			memset( addr, 0, sizeof(struct sockaddr_storage) );

			/* Determine address family - IPv6 must have a ':' */
			addr->sa_family = ((strchr(data, ':')==NULL) ? \
						AF_INET : AF_INET6);

			if( str_to_addr((__u8*)data, addr) > 0 ) 
			{
				memcpy( &g_host_config.pma_addr, addr, SALEN(addr));
			}
			else 
			{
				WriteLog( ERR, "%s invalid.\n", node->name );
			}
		}
		//pma port
		else if( strcmp((char *)node->name, "pma_port") == 0 ) 
		{
			sscanf( data, "%d", &g_host_config.pma_port );
		}
		//localhost id
		else if( strcmp((char *)node->name, "router_id") == 0 ) 
		{
			sscanf( data, "%d", &g_host_config.router_id );
		}
		//captuerd time. this is a block time.
		else if( strcmp((char *)node->name, "captured_time") == 0 ) 
		{
			sscanf( data, "%d", &g_host_config.captured_time );
		}
		//now the system omits this value
		else if( strcmp((char *)node->name, "interval") == 0 ) 
		{
			sscanf( data, "%d", &g_host_config.interval );
		}
		// 系统最大处理能力
		else if( strcmp((char *)node->name, "nmax") == 0 ) 
		{
			sscanf( data, "%d", &g_host_config.nmax );
		}
		else if( strcmp((char *)node->name, "a") == 0 ) 
		{
			//sscanf( data, "%f", &g_host_config.a );
			g_host_config.a = atof( data );
		}
		//大象流评判标准
		else if( strcmp((char *)node->name, "p") == 0 ) 
		{
			//sscanf( data, "%f", &g_host_config.a );
			g_host_config.p = atof( data );
		}
		else
		{
			WriteLog( ERR, "wrong field=[%s] \n", node->name );
		}

		xmlFree( data );
	}

	xmlFreeDoc( doc );

	return( 0 );
}

/* 
 *	transform string to IP address format	
 */

/*
int str_to_addr( __u8 *data, struct sockaddr *addr )
{
	if( addr == NULL )
	{
		return -1;
	}

	return(inet_pton(addr->sa_family, (char*)data, SA2IP(addr)));
}
*/

/*
 *	write log to file
 */

int     WriteLog( int level, char *cFormat, ... )
{
	FILE            *fp = NULL;
	char            cCurrDate[20];
	char            cCurrTime[20];
	va_list         args;
	FILE		*logfp = NULL;

	logfp = g_host_config.fp;

	switch( level )
	{	
		/* print to stderr */
		case ERR:
			fp = stderr;
			break;
		case DEBUG:
			#ifdef _DEBUG_SYMBOL
		//		printf( "[%s][%d]\n", __FILE__, __LINE__ );
				fp = logfp;
			#else
				//printf( "[%s][%d]\n", __FILE__, __LINE__ );
				//fclose( logfp );
				return 0;
			#endif
		/* log to file */
		case NORM:
		case NORMT:
		case WARN:
		default: 
			fp = logfp;
			break;			
	}

	if( fp == NULL )
	{
		printf( "[%s][%d] fp is null ***error***\n", __FILE__, __LINE__ );
		return 1;
	}

	//printf( "[%s][%d]fp=[%x]\n", __FILE__, __LINE__, fp );

	/* include the current time at beginning of log line */
	//if ( (level == DEBUG) )
	//{
		memset( cCurrDate, 0x00, sizeof(cCurrDate) );
		memset( cCurrTime, 0x00, sizeof(cCurrTime) );
		gettime( cCurrDate, cCurrTime );
	
		fprintf( fp, "T[%s]", cCurrTime );
		fprintf( fp, "P[%d]tid[%x]", getpid(), pthread_self() );
	//} 
	/* print warning symbol for errors */
	//else if ( level == WARN || level == ERR ) 
	if ( level == WARN || level == ERR ) 
	{
		fprintf(fp, "***ERROR*** ");
	}

	va_start( args, cFormat );
	(void)vfprintf( fp, cFormat, args );
	va_end( args );

	fflush( logfp );
//	fclose( logfp );

	return 0;
}


/*
 *	write log to file
 */

int     WriteLog1( char *filename, char *cFormat, ... )
{
	FILE            *fp = NULL;
	char            cCurrDate[20];
	char            cCurrTime[20];
	char            FileName[100 + 1];
	va_list         args;

	memset( cCurrDate, 0x00, sizeof(cCurrDate) );
	memset( cCurrTime, 0x00, sizeof(cCurrTime) );
	memset( FileName, 0x00, sizeof(FileName) );

	va_start( args, cFormat );

	gettime( cCurrDate, cCurrTime );

	sprintf( FileName, "%s.log", filename );

	fp = fopen( FileName, "a+" );
	if( fp == NULL )
	{
		return -1;
	}

	fprintf( fp, "T[%s]", cCurrTime );
	fprintf( fp, "P[%d]tid[%x]", getpid(), pthread_self() );

	(void)vfprintf( fp, cFormat, args );
	//fprintf( fp, "\n" );
	va_end( args );

	fclose( fp );

	return 0;
}

/*
	get pma agent IP address
 */

struct sockaddr	*get_pma_addr()
{
	//strcpy( addr, "2001:0:53aa:64c:1c92:4512:8488:da0" );
	struct sockaddr *addr = (struct sockaddr *)&(g_host_config.pma_addr);
	return addr;
}

/*
	get pma agent  port
 */

int	get_pma_port()
{
	return g_host_config.pma_port;
	//*port = 9999;
}

/* 
	get local router id
 */

int	get_router_id()
{
	return g_host_config.router_id;
}

/* 
 *	set socket' family
 */

int set_sock_family( struct sockaddr *addr, int family )
{
	if( addr == NULL )
	{
		WriteLog( ERR, "[%s][%d] addr is NULL\n", __FILE__, __LINE__ );
		return -1;
	}

	addr->sa_family = family;
}

/* 
 *	set socket' port
 */

int set_sock_port( struct sockaddr *addr, int port )
{
	int	iret = 0;

	if( addr == NULL )
	{
		return -1;
	}

	if( addr->sa_family == AF_INET6 )
	{
		((struct sockaddr_in6 *)(addr))->sin6_port = port;
		iret = 0;
	}	
	else if( addr->sa_family == AF_INET )
	{
		((struct sockaddr_in *)(addr))->sin_port = port;
		iret = 0;	
	}
	else
	{
		iret = -1;
	}

	return iret ;
}

/* 
 *	transform string to IP address format	
 */

int str_to_addr( __u8 *data, struct sockaddr *addr )
{
	if( addr == NULL )
	{
		return -1;
	}

	/* TODO: use platform-independent getaddrinfo() w/AI_NUMERICHOST */
	return(inet_pton(addr->sa_family, (char*)data, SA2IP(addr)));
}

/* 
 *	transform IP address to string format	
 */

int addr_to_str(struct sockaddr *addr, __u8 *data, int len)
{
	if( addr == NULL )
	{
		return -1;
	}

	return(inet_ntop(addr->sa_family, SA2IP(addr), (char*)data, len)==NULL);
}

int get_device_stats( struct net_device_stats *net_stats, int ifindex )
{
	if( net_stats == NULL || ifindex < 0 || ifindex > g_interface_size )
	{
		WriteLog( ERR, "[%s][%d]input error\n", __FILE__, __LINE__ );
		return -1;
	}

	if_array[ifindex].device_stats.tx_bytes = net_stats->tx_bytes;
	if_array[ifindex].device_stats.tx_dropped = net_stats->tx_dropped;

	//WriteLog( DEBUG, "[%s][%d]send bytes=[%lu],dropped=[%lu]\n", __FILE__, __LINE__, net_stats->tx_bytes, net_stats->tx_dropped );

	return 0;
}	

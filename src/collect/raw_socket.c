#include "public_include.h"
#include "public.h"
#include "interface.h"

void *capture_netlink_data( char *device, int captured_time, int if_index );
#define BUFF_LEN 1024

int	create_raw_socket( int protocol )
{
	int rawsock;

	rawsock = socket( PF_PACKET, SOCK_RAW, htons(protocol) );
	if( rawsock < 0 )
	{
		printf( "[%s][%d] socket error=[%s]\n", __FILE__, __LINE__, strerror(errno) );
		return -1;
	}

	return rawsock;
}

int	bind_rawsocket_to_interface( char *device, int rawsock, int protocol )
{
	struct sockaddr_ll sll;
	struct ifreq ifr;

	bzero( &sll, sizeof(sll) );
	bzero( &ifr, sizeof(ifr) );
	
	/* 1.First Get the Interface Index  */
	strncpy( (char *)ifr.ifr_name, device, IFNAMSIZ );
	if( (ioctl(rawsock, SIOCGIFINDEX, &ifr)) == -1 )
	{
		printf( "[%s][%d]Error getting Interface index !\n", __FILE__, __LINE__ );
		return (-1);
	}

	/* 2.Bind our raw socket to this interface */
	sll.sll_family = AF_PACKET;
	sll.sll_ifindex = ifr.ifr_ifindex;
	sll.sll_protocol = htons(protocol); 

	if( (bind(rawsock, (struct sockaddr *)&sll, sizeof(sll)))== -1 )
	{
		printf( "[%s][%d]Error binding raw socket to interface\n", __FILE__, __LINE__ );
		return (-1);
	}

	return 0;
}

void *capture_netlink_data( char *device, int captured_time, int if_index )
{
	int err, len, max_fd;
	fd_set fd;
	struct timeval timeout;
	__u8 packet_content[BUFF_LEN*4]; /* raw, encrypted data buffer */
	u_short ethernet_type = 0;
	struct ether_header *ethernet_protocol = NULL;
	u_char *mac_string = NULL;
	int	s_raw_netlink = 0;
	int	size = 32 *1024;
	struct ip	*iph = NULL;
	char    ip_addr[INET6_ADDRSTRLEN+1];
	struct timeval  start;
	struct timeval  end;
	int     difftime = 0;

	if( device == NULL || captured_time <= 0 )
	{
		printf( "[%s][%d]input param error\n", __FILE__, __LINE__ );
		return NULL;
	}

	// create the raw socket
	s_raw_netlink = create_raw_socket( ETH_P_ALL );
	if( s_raw_netlink < 0 )
	{
		printf( "[%s][%d] socket error=\n", __FILE__, __LINE__ );
		return NULL;
	}

	// Bind socket to interface
	err = bind_rawsocket_to_interface( device, s_raw_netlink, ETH_P_ALL );
	if( err < 0 )
	{
		printf( "[%s][%d] bind socket error=\n", __FILE__, __LINE__ );
		return NULL;
	}
	
	//set sock option
        err = setsockopt( s_raw_netlink, SOL_SOCKET, SO_RCVBUF, (char *)&size, sizeof(size) );
        if( err < 0 )
        {
		printf( "[%s][%d]setsockopt() error for netlink socket in,err=[%s]\n", __FILE__, __LINE__, strerror(errno) );
		return NULL;
        }

	err =  setsockopt( s_raw_netlink, SOL_SOCKET, SO_SNDBUF,  &size, sizeof(size) );
	if( err < 0 )
	{
		printf( "[%s][%d]setsockopt() error for netlink socket in,err=[%s]\n", __FILE__, __LINE__, strerror(errno) );
		return NULL;
	}

	gettimeofday( &start, NULL );
	WriteLog( LOGFILE,  "[%s][%d]start time: %ld secs %ld micros\n", __FILE__, __LINE__, start.tv_sec, start.tv_usec );

	while( 1 )
	{
		FD_ZERO(&fd);
		FD_SET((unsigned)s_raw_netlink, &fd);
		max_fd = s_raw_netlink;

		timeout.tv_sec = captured_time/1000000;
		//printf( "[%s][%d]timeout.tv_sec=[%d]\n", __FILE__, __LINE__, timeout.tv_sec );
		timeout.tv_usec = 0;
		memset( packet_content, 0x00, sizeof(packet_content) );

		if( (err = select(max_fd+1, &fd, NULL, NULL, &timeout)) < 0 ) 
		{
			printf( "select() error %s\n", strerror(errno) );
		}	
		else if( FD_ISSET(s_raw_netlink, &fd) )
		{
			len = read( s_raw_netlink, packet_content, sizeof(packet_content) );
			if( len < 0 )
			{
				printf( "[%s][%d]errmsg=[%s]\n", __FILE__, __LINE__, strerror(errno) );
				continue;
			}

			gettimeofday( &end, NULL );

			ethernet_protocol_packet_callback( NULL, NULL, packet_content, if_index );
			difftime = get_diff_time( &start, &end );
			if( difftime > captured_time )
			{
				WriteLog( LOGFILE,  "[%s][%d]\n\n\nbreak;\n\n", __FILE__, __LINE__ );
				break;
			}
		}
		else if (err == 0) 
		{
			/* idle cycle */
			WriteLog( LOGFILE, "[%s][%d]timeout\n", __FILE__, __LINE__ );
			break;
			/* TODO: implement SA timeout here */
		}
	}
	
	close(s_raw_netlink);

	WriteLog( LOGFILE,  "[%s][%d]this time: %ld secs %ld micros\n", __FILE__, __LINE__, end.tv_sec, end.tv_usec );
	if_array[if_index].interval = difftime;
}

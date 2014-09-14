#include "public_include.h"
#include "public.h"
#include "packet.h"

struct sockaddr *get_pma_addr();

int	packet_send_buff( __u8 *xmlbuff, int len, char *buff, int bufflen, int flag )
{
	struct _cpi_header *cpi = NULL;
	if( xmlbuff == NULL
		|| len < 0 
		|| buff == NULL
		|| bufflen < 0 || bufflen < len+sizeof(struct _cpi_header) )
	{
		return -1;
	}

	//send's header
	cpi = (struct _cpi_header *)buff;	
	cpi->pkg_type = flag;
	//cpi->pkg_type = TC_MONITOR_NO;
	cpi->pkg_version = CR_VERSION;
	cpi->device_type = htons( get_router_id() );
	//WriteLog( DEBUG, "[%s][%d]rt_id=[%d]\n", __FILE__, __LINE__, cpi->rt_id );
	cpi->pkg_len = htonl( sizeof(struct _cpi_header) + len );
	//WriteLog( DEBUG, "[%s][%d]len=[%d]\n", __FILE__, __LINE__, cpi->pkg_len );

	//send's body
	memcpy( buff+sizeof(struct _cpi_header), (char *)xmlbuff, len );

	//WriteLog( DEBUG, "[%s][%d]buff=[%s]\n", __FILE__, __LINE__, buff+sizeof(struct _cpi_header) );

	return 0;
}

/*
 *	send data to PMA agent
 */

int	send_data( __u8 *xmlbuff, int len, int flag )
{
	int	port;
	int	iret = 0;
	struct sockaddr_storage ss_dst;
	struct sockaddr *dst_addr = NULL;
	char	buff[20000];

	dst_addr = get_pma_addr();
	port = get_pma_port();
	set_sock_port( dst_addr, htons(port) );

	//WriteLog( DEBUG, "[%s][%d]pma_port=[%d]\n", __FILE__, __LINE__, g_host_config.pma_port );
	char	data[INET6_ADDRSTRLEN+1];
	memset( data, 0x00, sizeof(data) );
	addr_to_str( dst_addr, data, sizeof(data)-1 );

	//WriteLog( DEBUG, "[%s][%d]pma_addr=[%s]\n", __FILE__, __LINE__, data );

	packet_send_buff( xmlbuff, len, buff, sizeof(buff)-1, flag );

	iret = client_send( buff, sizeof(struct _cpi_header)+len, dst_addr );
	if( iret < 0 )
	{
		WriteLog( ERR, "[%s][%d]send_data error\n", __FILE__, __LINE__ );
		return -1;
	}

	return 0;
}

/*
 *	send function
 */

int	client_send( __u8 *data, int len, struct sockaddr* dst )
{
	int		sockfd = 0;
	int		ret = 0;
	struct timeval	timeout;	
	char		clen[10];
	int		ilen = 0;

	timeout.tv_sec = 15;
	timeout.tv_usec = 0;

	//create tcp socket
	if( (sockfd=socket(dst->sa_family, SOCK_STREAM, 0)) < 0 )
	{
		WriteLog( ERR, "[%s][%d]socket error=[%s]\n", __FILE__, __LINE__, strerror(errno) );
		return (-1);
	}

	//set time out
	/*
	ret = setsockopt( sockfd, SOL_SOCKET, SO_RCVTIMEO,(char*)&timeout, sizeof(timeout) );
	if( ret < 0 )
	{
		WriteLog( ERR, "[%s][%d]errmsg=[%s]\n", __FILE__, __LINE__, strerror(errno) );
		close(sockfd);
		return -1;
	}
	*/

	ret = connect( sockfd, dst, SALEN(dst) );
	if( ret < 0 )
	{
		WriteLog(  ERR,"[%s][%d]socket error=[%s]\n", __FILE__, __LINE__, strerror(errno) );
		close(sockfd);
		return (-1);
	}

	ret = write( sockfd, data, len );
	if( ret < 0 )
	{
		WriteLog( ERR, "[%s][%d]socket error=[%s]\n", __FILE__, __LINE__, strerror(errno) );
		close(sockfd);
		return -1;
	}
	
	close( sockfd );
	WriteLog( DEBUG, "[%s][%d]\n", __FILE__, __LINE__ );

	return 0;
}

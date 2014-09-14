#include "public_include.h"
#include "interface.h"
#include "util.h"

static char *get_name(char *name, char *p);
static int if_readlist_proc( struct _interface_info *ife );

//得到所有网卡名称
int	get_all_interfaces( struct _interface_info *if_array, int *size )
{
	FILE * fp;
	char line[512];
	int i;
	int	interface_size = 0;
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
				&& strncmp(pc, "tap0", 4) 
				&& strncmp(pc, "teql0", 5) )
			{
				memset( if_array[interface_size].name, '\0', sizeof(if_array[interface_size].name));
				memcpy( if_array[interface_size].name, pc, sizeof(if_array[interface_size].name) );

				interface_size++;
			}
		}
	}

	fclose(fp);

	*size = interface_size;

	return TC_SUCCESS;
}

/**
	得到设备上的流信息

 * function get_dev_info()
 *
 * Use the Netlink interface to retrieve a list of addresses for this
 * host's interfaces, and stores them into global my_addr_head list.

 **/

int	get_dev_info( int s_net, int nl_sequence_number, struct _interface_info *if_array )
{
	if( if_array == NULL )
	{
		return -1;
	}	

	/* these are used for passing messages */
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
		WriteLog( LOGFILE, "Netlink: sentdo() error: %s\n", strerror(errno));
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
			WriteLog( LOGFILE, "Netlink: recvmsg() error!\nerror: %s\n",
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
				WriteLog( LOGFILE, "Error in Netlink response.\n");
				break;
			}

			ifi = NLMSG_DATA(h);
			//WriteLog( LOGFILE, "[%s][%d]interface id=[%d]\n", __FILE__, __LINE__, ifi->ifi_index );
			if( (ifi -> ifi_flags & IFF_UP ) != IFF_UP )
			{
				h = NLMSG_NEXT(h, status);
				continue;
			}

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
					switch( rta->rta_type )
					{
						case IFLA_STATS:
							net_stats = (struct net_device_stats *) RTA_DATA(rta);
							memcpy( &if_array[ifindex].device_stats, net_stats, sizeof(struct net_device_stats) );

		/*
		printf( "RX packets:%ld RX bytes:%ld, errors:%ld dropped:%ld overruns:%ld frame:%ld\n",
		net_stats->rx_packets, net_stats->rx_bytes, net_stats->rx_errors,
		net_stats->rx_dropped, net_stats->rx_fifo_errors,
		net_stats->rx_frame_errors);
		*/

							break;
						default:
							break;
					}
				}

				rta = RTA_NEXT(rta,len);
			}

			h = NLMSG_NEXT(h, status);
		} /* end while(NLMSG_OK) - loop 2 */
	} /* end while(!done) - loop 1 */ 
	
	//WriteLog( LOGFILE, "\n");

	return(0);
}

/**
得到网卡信息
网卡信息包括
	ipv4 address
	ipv4 netmask
	ipv6 address
	ipv6 netmask
	mac address
	interface active
**/
int	get_interfaces_info( int interface_size, struct _interface_info *ife )
{
	int	i = 0;	

	for( i=0; i<interface_size; i++ )
	{
		get_one_interface_info( &ife[i] );
	}

	return 0;
}

int	get_one_interface_info( struct _interface_info *ife )
{
	int	skfd = 0;
	int	iret = 0;

	skfd = open_socket( AF_INET );

	get_dev_flag( skfd, ife );

	//printf( "[%s][%d]ife.active=[%d]\n", __FILE__, __LINE__, ife->active&IFF_UP );
	//printf( "[%s][%d]ife.active=[%x][%d]\n", __FILE__, __LINE__, ife->active, IFF_UP );
	if( (ife->active & IFF_UP) != IFF_UP )
	{
		//printf( "[%s][%d]\n", __FILE__, __LINE__ );
		close( skfd );
		return -1;
	}
	ife->active = IFF_UP;

	//index
	get_dev_index( skfd, ife );

	//MAC
	get_dev_hwaddr( skfd, ife );

	//printf( "[%s][%d]skfd=[%d]\n", __FILE__, __LINE__, skfd );

	//IPV4 address
	iret = get_dev_ipv4_addr( skfd, ife );
	if( iret == 0 )
	{
		//broadcast addr
		get_dev_ipv4_broadaddr( skfd, ife );

		//netmask
		get_dev_ipv4_netmask( skfd, ife );
	}

	//IPV6 address
	get_dev_ipv6_addr( ife );

	close( skfd );
}

int open_socket( int family )
{
	int	skfd = -1;

	skfd = socket( family, SOCK_DGRAM, 0 );
	if( skfd < 0 )
	{
		printf( "[%s][%d]error=[%s]\n", __FILE__, __LINE__, strerror(errno) );
	}

	return skfd;
}

//得到网卡状态
int	get_dev_flag( int skfd, struct _interface_info *ife )
{
	int	iret = 0;
	char	*ifname = NULL;
	struct ifreq	ifr;
	
	if( skfd <= 0 || ife == NULL )
	{
		printf( "[%s][%d]input errro\n", __FILE__, __LINE__ );
		return -1;
	}

	ifname = ife->name;
	strcpy(ifr.ifr_name, ifname);
	if( ioctl( skfd, SIOCGIFFLAGS, &ifr ) < 0 )
	{
		return (-1);
	}
	ife->active = ifr.ifr_flags;

	return iret;
}

//得到网卡MAC地址
int	get_dev_hwaddr( int skfd, struct _interface_info *ife )
{
	int	iret = 0;
	char	*ifname = NULL;
	struct ifreq	ifr;
	
	if( skfd <= 0 || ife == NULL )
	{
		printf( "[%s][%d]input errro\n", __FILE__, __LINE__ );
		return -1;
	}

	ifname = ife->name;
	strcpy( ifr.ifr_name, ifname);
	if( ioctl( skfd, SIOCGIFHWADDR, &ifr ) < 0 )
	{
		memset( ife->ether_host, 0, 32 );
	}
	else
	{
		memcpy( ife->ether_host, ifr.ifr_hwaddr.sa_data, 8 );
	}

	return iret;
}

int	get_dev_index( int skfd, struct _interface_info *ife )
{
	int	iret = 0;
	char	*ifname = NULL;
	struct ifreq	ifr;
	
	if( skfd <= 0 || ife == NULL )
	{
		printf( "[%s][%d]input errro\n", __FILE__, __LINE__ );
		return -1;
	}

	ifname = ife->name;
	strcpy( ifr.ifr_name, ifname);
	if( ioctl( skfd, SIOCGIFINDEX, &ifr ) < 0 )
	{
		return -1;
	}

	ife->if_index = ifr.ifr_ifindex;
	//printf( "[%s][%d]index=[%d]\n", __FILE__, __LINE__, ife->if_index );

	return 0;
}

//得到网卡ipv4地址
int	get_dev_ipv4_addr( int skfd, struct _interface_info *ife )
{
	int	iret = 0;
	char	*ifname = NULL;
	struct ifreq	ifr;
	
	if( skfd <= 0 || ife == NULL )
	{
		printf( "[%s][%d]input errro\n", __FILE__, __LINE__ );
		return -1;
	}

	ifname = ife->name;
	strcpy(ifr.ifr_name, ifname);
	if( ioctl( skfd, SIOCGIFADDR, &ifr ) < 0 )
	{
		return (-1);
	}
	
	//convert ipv4 address into a character string
	struct sockaddr_in *addr = NULL;
	char	ipaddr[INET6_ADDRSTRLEN];
	addr = ( struct sockaddr_in *)&(ifr.ifr_addr);

	inet_ntop( AF_INET, (char *)&(addr->sin_addr), ipaddr, sizeof(ipaddr)-1 );
	memcpy( ife->ipv4_addr, ipaddr, sizeof(ife->ipv4_addr) );

	return iret;
}

int	get_dev_ipv4_broadaddr( int skfd, struct _interface_info *ife )
{
	int	iret = 0;
	char	*ifname = NULL;
	struct ifreq	ifr;
	
	if( skfd <= 0 || ife == NULL )
	{
		printf( "[%s][%d]input errro\n", __FILE__, __LINE__ );
		return -1;
	}

	ifname = ife->name;
	strcpy(ifr.ifr_name, ifname);
	if( ioctl( skfd, SIOCGIFBRDADDR, &ifr ) < 0 )
	{
		return (-1);
	}
	
	//convert ipv4 broadcast address into a character string
	struct sockaddr_in *addr = NULL;
	char	ipaddr[INET6_ADDRSTRLEN];
	addr = ( struct sockaddr_in *)&(ifr.ifr_addr);

	inet_ntop( AF_INET, (char *)&(addr->sin_addr), ipaddr, sizeof(ipaddr)-1 );
	memcpy( ife->ipv4_broad_addr, ipaddr, sizeof(ife->ipv4_addr) );

	return iret;
}

int	get_dev_ipv4_netmask( int skfd, struct _interface_info *ife )
{
	int	iret = 0;
	char	*ifname = NULL;
	struct ifreq	ifr;
	
	if( skfd <= 0 || ife == NULL )
	{
		printf( "[%s][%d]input errro\n", __FILE__, __LINE__ );
		return -1;
	}

	ifname = ife->name;

	strcpy(ifr.ifr_name, ifname);
	if (ioctl(skfd, SIOCGIFNETMASK, &ifr) < 0)
	{
		return -1;
	}

	//convert ipv4 broadcast address into a character string
	struct sockaddr_in *addr = NULL;
	char	ipaddr[INET6_ADDRSTRLEN];
	addr = ( struct sockaddr_in *)&(ifr.ifr_netmask);

	inet_ntop( AF_INET, (char *)&(addr->sin_addr), ipaddr, sizeof(ipaddr)-1 );
	memcpy( ife->ipv4_netmask, ipaddr, sizeof(ife->ipv4_addr) );

	return iret;
}

int	get_dev_ipv6_addr( struct _interface_info *ife )
{
	char	addr6[INET6_ADDRSTRLEN];
	char	addr6p[8][5];
	char	ipaddr[INET6_ADDRSTRLEN];
	FILE	*f = NULL;
	int plen, scope, dad_status, if_idx;
	char	devname[20];
	struct in6_addr in6_addr;
	int	len = 0;

	memset( addr6, 0x00, sizeof(addr6) );
	memset( ipaddr, 0x00, sizeof(ipaddr) );
	memset( devname, 0x00, sizeof(devname) );

	if ((f = fopen(_PATH_PROCNET_IFINET6, "r")) != NULL) 
	{
		while (fscanf(f, "%4s%4s%4s%4s%4s%4s%4s%4s %02x %02x %02x %02x %20s\n", 
			addr6p[0], addr6p[1], addr6p[2], addr6p[3], 
			addr6p[4], addr6p[5], addr6p[6], addr6p[7], 
			&if_idx, &plen, &scope, &dad_status, devname) != EOF) 
		{
			if (!strcmp(devname, ife->name)) 
			{
				sprintf(addr6, "%s:%s:%s:%s:%s:%s:%s:%s",
				addr6p[0], addr6p[1], addr6p[2], addr6p[3],
				addr6p[4], addr6p[5], addr6p[6], addr6p[7]);

				//convert to ipv6 address
				memset( ipaddr, 0x00, sizeof(ipaddr) );
				inet_pton( AF_INET6, addr6, &in6_addr );
				inet_ntop( AF_INET6, &in6_addr, ipaddr, sizeof(ipaddr)-1 );

				memcpy( ife->ipv6_addr+len, ipaddr, strlen(ipaddr) );
				strcat( ife->ipv6_addr, "," );
				len = strlen(ipaddr)+1;

				//get ipv6 netmask		
				ife->ipv6_addr_netmask = plen;
			}
		}
	}

	return 0;
}

int print_mac( struct _interface_info *ife )
{
	u_char *mac_string = ife->ether_host;
	printf( "%02x:%02x:%02x:%02x:%02x:%02x\n", *mac_string, *(mac_string + 1), *(mac_string + 2), *(mac_string + 3), *(mac_string + 4), *(mac_string + 5));

}

int	print_interfaces_info( int size, struct _interface_info *ife )
{
	int	i = 0;

	for( i=0; i<size; i++ )
	{
		//name
		WriteLog( LOGFILE, "[%s][%d]eth name=[%s]\n", __FILE__, __LINE__, ife[i].name );
		//ipv4 addr
		WriteLog( LOGFILE, "[%s][%d]ipv4 addr=[%s]\n", __FILE__, __LINE__, ife[i].ipv4_addr );
		//ipv4 broad addr
		WriteLog( LOGFILE, "[%s][%d]ipv4 broad addr=[%s]\n", __FILE__, __LINE__, ife[i].ipv4_broad_addr );
		//ipv4 netmask
		WriteLog( LOGFILE, "[%s][%d]ipv4 netmask=[%s]\n", __FILE__, __LINE__, ife[i].ipv4_netmask );
		//ipv6 addr
		WriteLog( LOGFILE, "[%s][%d]ipv6 addr=[%s]\n", __FILE__, __LINE__, ife[i].ipv6_addr );
		//ipv6 addr netmask
		WriteLog( LOGFILE, "[%s][%d]ipv6 addr netmask=[%d]\n", __FILE__, __LINE__, ife[i].ipv6_addr_netmask );
		//active
		WriteLog( LOGFILE, "[%s][%d]active=[%d]\n", __FILE__, __LINE__, ife[i].active );
	}
}

static int get_dev_fields(char *bp, struct _interface_info *ife)
{
	sscanf( bp, 
	"%lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu", 
	       &ife->device_stats.rx_bytes,
	       &ife->device_stats.rx_packets,
	       &ife->device_stats.rx_errors,
	       &ife->device_stats.rx_dropped,
	       &ife->device_stats.rx_fifo_errors,
	       &ife->device_stats.rx_frame_errors,
	       &ife->device_stats.rx_compressed,
	       &ife->device_stats.multicast,

	       &ife->device_stats.tx_bytes,
	       &ife->device_stats.tx_packets,
	       &ife->device_stats.tx_errors,
	       &ife->device_stats.tx_dropped,
	       &ife->device_stats.tx_fifo_errors,
	       &ife->device_stats.collisions,
	       &ife->device_stats.tx_carrier_errors,
	       &ife->device_stats.tx_compressed );
    return 0;
}

int	get_dev_flow_info( struct _interface_info *ife, int num ) 
{
	struct _interface_struct ptr;
	int	i = 0;	
	for( i=0; i<num; i++ )
	{
		if( 1 == if_array[i].active )
		{
			if_readlist_proc( &ife[i] );
		}
	}
}

static int if_readlist_proc( struct _interface_info *ife )
{
	FILE	*fh = NULL;
	char	buf[512];
	struct _interface_info *ptr = NULL;
	int err;
	char	*target = NULL;

	if( ife == NULL || ife->name == NULL )
	{
		return 0;
	}

	ptr = ife;
	target = ife->name;

	fh = fopen(PATH_PROC_NET_DEV, "r");
	if (!fh) 
	{
		printf( ("Warning: cannot open %s (%s). Limited output.\n"), PATH_PROC_NET_DEV, strerror(errno) ); 
		return -1;
	}	

	fgets(buf, sizeof buf, fh);	/* eat line */
	fgets(buf, sizeof buf, fh);

	err = 0;
	while (fgets(buf, sizeof buf, fh)) 
	{
		char *s, name[IFNAMSIZ];
		s = get_name(name, buf);    
		get_dev_fields( s, ptr );
		if( strcmp(target,name) == 0 )
		{
			break;
		}
	}

	fclose(fh);

	return err;
}

static char *get_name(char *name, char *p)
{
    while (isspace(*p))
	p++;
    while (*p) {
	if (isspace(*p))
	    break;
	if (*p == ':') {	/* could be an alias */
	    char *dot = p, *dotname = name;
	    *name++ = *p++;
	    while (isdigit(*p))
		*name++ = *p++;
	    if (*p != ':') {	/* it wasn't, backup */
		p = dot;
		name = dotname;
	    }
	    if (*p == '\0')
		return NULL;
	    p++;
	    break;
	}
	*name++ = *p++;
    }
    *name++ = '\0';
    return p;
}

#include "public_include.h"
#include "public.h"
#include "interface.h"
#include "packet.h"

extern struct _tc_config_struct g_host_config;

void	ethernet_protocol_packet_callback( u_char *argument, const struct pcap_pkthdr *packet_header, const u_char *packet_content, int if_index )
{
	u_short ethernet_type;
	struct ether_header *ethernet_protocol;
	u_char *mac_string;
	u_int8_t  ether_host[ETH_LEN+1];
	//static int packet_number = 1;

	//WriteLog( DEBUG, "**************************************************\n");
	//WriteLog( DEBUG, "The %d Ethernet  packet is captured.\n", packet_number);
	//WriteLog( DEBUG, "-----------    Ehternet Potocol (Link Layer)  ------------\n");
	//WriteLog( DEBUG, "The %d Ethernet  packet is captured.\n", packet_number);
	ethernet_protocol = (struct ether_header*)packet_content;
	//WriteLog( DEBUG, "Ethernet type is :\n");
	ethernet_type = ntohs(ethernet_protocol->ether_type);
	//WriteLog( DEBUG, "%04x\n", ethernet_type);

	memset( ether_host, 0x00, sizeof(ether_host) );
	memcpy( ether_host, if_array[if_index].ether_host, sizeof(ether_host)-1 );
	if( memcmp(ethernet_protocol->ether_shost, ether_host, sizeof(ether_host)-1) != 0 
		&& memcmp(ethernet_protocol->ether_dhost, ether_host, sizeof(ether_host)-1) != 0 )
	{
		return ;
	}

	switch (ethernet_type)
	{
		case ETHERTYPE_IP:
		//case ETHERTYPE_IPV6:
			//WriteLog( DEBUG, "The network layer is IP protocol\n");
			//analysis module
			analysis_module( packet_content, if_index );
			break;
		case 0x0806:
			//WriteLog( DEBUG, "The network layer is ARP protocol\n");
			break;
		case 0x8035:
			//WriteLog( DEBUG, "The network layer is RARP protocol\n");
			break;
		default:
			break;
	}

	//WriteLog( DEBUG, "**************************************************\n");

	//packet_number++;
}

/*
	//char bpf_filter_string[] = "ip6 and ether src 00:23:54:51:f5:6b";
	//char bpf_filter_string[] = "ip6";
	//char bpf_filter_string[] = "ip";
	//char bpf_filter_string[] = "ip and (ether src 00:23:54:51:f5:6b or ether dst 00:23:54:51:f5:6b)";
*/

static void	get_interface_flow_info( int if_index )
{
	char	*net_interface;
	int	captured_time = 0;

	net_interface = if_array[if_index].name;
	if( net_interface == NULL )
	{
		WriteLog( ERR,  "[%s][%d]error \n", __FILE__, __LINE__ );
		return ;
	}
	WriteLog( DEBUG,  "[%s][%d]interface name=[%s]\n", __FILE__, __LINE__, net_interface );

	captured_time = g_host_config.captured_time;
	WriteLog( DEBUG,  "[%s][%d]%d\n", __FILE__, __LINE__, captured_time );
	capture_netlink_data( net_interface, captured_time, if_index );

	return ;
}

void collect_information( int if_index )
{
	int	i = 0;
	int	interval_time = 0;

	i = if_index;
	if( 1 == if_array[i].active )
	{
		get_interface_flow_info( i );
	}
}

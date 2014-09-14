#ifndef	_PACKET_H
#define	_PACKET_H

struct this_ether_header
{
    u_int8_t ether_dhost[6];
    u_int8_t ether_shost[6];
    u_int16_t ether_type;
};

//cpi message header
struct _cpi_header
{
	__u8 pkg_type;
	__u8 pkg_version;
	__u16 device_type;
	__u32 pkg_len;
//	__u32 padding;
//j	__u32 padding1;
//	__u32 padding2;
};

#define CR_VERSION 2
#define TC_MONITOR_NO 22
#define TC_INTERFACE_INFO 23
#define TC_FLOW_INFO 13

//pcap_t *pcap_handle;

#endif


#include "public_include.h"
#include "public.h"
#include "interface.h"
#include "list.h"
#include "ip_list.h"

xmlDocPtr create_xml_doc();
xmlNodePtr create_xml_root_node( xmlDocPtr doc, char *name );
xmlNodePtr add_xml_child( xmlNodePtr node, char *name, char *value );
xmlNodePtr packet_xml_interface( xmlNodePtr node, int if_index );
int	packet_xml_flow_info( xmlNodePtr node, char *flow, float sampled_rate, int send_flag );

int one_interface_pack_flow_info_xml_buff( __u8 **xmlbuff, int *len, int if_index );

int pack_flow_info_xml_buff( __u8 **xmlbuff, int *len )
{
	int		i = 0;
	xmlDocPtr	doc = NULL;
	xmlNodePtr	root_node = NULL;
	xmlNodePtr	node = NULL;
	char		value[50];
	int		this_len = 0;

	//create document
	doc = create_xml_doc();
	root_node = create_xml_root_node( doc, "TC_FLOW_INFO" );

	//1.add timestamp field
	memset( value, 0x00, sizeof(value) );
	get_data_time( value, sizeof(value)-1 );
	add_xml_child( root_node, "timestamp", value );

	//2.add router field ??????
	node = add_xml_child( root_node, "ROUTER", NULL );
	sprintf( value, "%d", g_host_config.router_id );
	add_xml_child_prop( node, "id", value );

	for( i=0; i<g_interface_size && i<MAX_INTERFACE_SIZE; i++ )
	{
		if( 1 == if_array[i].active )
		//????????
		//if( TC_ACTIVE == if_array[i].active )
		{
			op_interface_struct( i, node );
		}
	}

	//xmlSaveFormatFileEnc( "tmp.xml", doc, "UTF-8", 1 );
	//xmlDocDumpFormatMemory( doc, xmlbuff, &this_len, 0 );
	xmlDocDumpFormatMemoryEnc( doc, xmlbuff, &this_len, "UTF-8", 0 );
	*len = this_len;
	xmlFreeDoc( doc );

	return 0;
}

//2011-09-12
int one_interface_pack_flow_info_xml_buff( __u8 **xmlbuff, int *len, int if_index )
{
	int		i = 0;
	xmlDocPtr	doc = NULL;
	xmlNodePtr	root_node = NULL;
	xmlNodePtr	node = NULL;
	char		value[50];
	int		this_len = 0;

	i = if_index;
	//create document
	doc = create_xml_doc();
	root_node = create_xml_root_node( doc, "TC_FLOW_INFO" );

	//1.add timestamp field
	memset( value, 0x00, sizeof(value) );
	get_data_time( value, sizeof(value)-1 );
	add_xml_child( root_node, "timestamp", value );

	//2.add router field ??????
	node = add_xml_child( root_node, "ROUTER", NULL );
	sprintf( value, "%d", g_host_config.router_id );
	add_xml_child_prop( node, "id", value );

	if( 1 == if_array[i].active )
	//if( TC_ACTIVE == if_array[i].active )
	{
		op_interface_struct( i, node );
	}

	//xmlSaveFormatFileEnc( "tmp.xml", doc, "UTF-8", 1 );
	//xmlDocDumpFormatMemory( doc, xmlbuff, &this_len, 0 );
	xmlDocDumpFormatMemoryEnc( doc, xmlbuff, &this_len, "UTF-8", 0 );
	*len = this_len;
	xmlFreeDoc( doc );

	return 0;
}

int pack_link_traffic_xml_buff( __u8 **xmlbuff, int *len )
{
	int		i = 0;
	xmlDocPtr	doc = NULL;
	xmlNodePtr	root_node = NULL;
	xmlNodePtr	node = NULL;
	char		value[50];
	int		this_len = 0;

	//create document
	doc = create_xml_doc();
	root_node = create_xml_root_node( doc, "TC_LINK_INFO" );

	//1.add timestamp field
	memset( value, 0x00, sizeof(value) );
	get_data_time( value, sizeof(value)-1 );
	add_xml_child( root_node, "timestamp", value );

	//2.add router field ??????
	node = add_xml_child( root_node, "ROUTER", NULL );
	sprintf( value, "%d", g_host_config.router_id );
	add_xml_child_prop( node, "id", value );

	for( i=0; i<g_interface_size && i<MAX_INTERFACE_SIZE; i++ )
	{
		if( 1 == if_array[i].active )
		//????????
		//if( TC_ACTIVE == if_array[i].active )
		{
			op_interface_link_info( i, node );
		}
	}

	xmlDocDumpFormatMemoryEnc( doc, xmlbuff, &this_len, "UTF-8", 0 );
	*len = this_len;
	xmlFreeDoc( doc );

	return 0;
}

/*
	one interface's flow information
 */

int	op_interface_struct( int if_index, xmlNodePtr node )
{
	xmlNodePtr		node_child = NULL;
	char			value[50];
	int			i = 0;
	int			queue_num = 0;	
	struct list_head	*l = NULL;
	double			sampled_rate = 0;
	double		tmp_total;
	double		tmp_sampled_total;

	if( node == NULL 
		|| if_index < 0 
		|| if_index > MAX_INTERFACE_SIZE )
	{
		WriteLog( ERR, "[%s][%d]node is null\n", __FILE__, __LINE__ );
		return -1;
	}

	// interface 
	sprintf( value, "%d", if_array[if_index].ifindex );
	node_child = add_xml_child( node, "interface", NULL );
	add_xml_child_prop( node_child, "id", value );

	//get sampled rate
	sampled_rate = interface_sampled_struct[if_index].out_sampled_rate;
	//2012-03-14

	tmp_total = interface_sampled_struct[if_index].total_packets;
	tmp_sampled_total = interface_sampled_struct[if_index].sampled_packets;
	if( tmp_sampled_total < 0.0001)
		sampled_rate = tmp_sampled_total;
	else
	sampled_rate = tmp_sampled_total / tmp_total;
	WriteLog( NORMT, "[%s][%d]sampled_rate=[%.2f]\n", __FILE__, __LINE__, sampled_rate );

	//1.packet interface information
	node_child = packet_xml_interface( node_child, if_index );

	//2.packet output flow information in loop
	list_for_each( l, &ipv4_head_out[if_index].node_head )
	{
		struct _ipv4_struct *ipv4_struct = list_entry(l, struct _ipv4_struct, node );
		packet_xml_flow_info( node_child, (char *)(ipv4_struct), sampled_rate, FLOW_OUT );
	}

	//2011-09-14 start
	//3.packet input flow information in loop
	l = NULL;
	list_for_each( l, &ipv4_head_in[if_index].node_head )
	{
		struct _ipv4_struct *ipv4_struct = list_entry(l, struct _ipv4_struct, node );
		packet_xml_flow_info( node_child, (char *)(ipv4_struct), sampled_rate, FLOW_IN );
	}
	//2011-09-14 end

	return 0;
}

/*
	one interface's link information
 */

int	op_interface_link_info( int if_index, xmlNodePtr node )
{
	xmlNodePtr		node_child = NULL;
	//char			value[50];
	int			i = 0;
	int			queue_num = 0;	
	struct list_head	*l = NULL;
	char			value[INET6_ADDRSTRLEN+1];

	if( node == NULL 
		|| if_index < 0 
		|| if_index > MAX_INTERFACE_SIZE )
	{
		WriteLog( ERR, "[%s][%d]node is null\n", __FILE__, __LINE__ );
		return -1;
	}

	// interface 
	sprintf( value, "%d", if_index );
	node_child = add_xml_child( node, "interface", NULL );
	add_xml_child_prop( node_child, "id", value );

	//interface name
	add_xml_child( node_child, "interface_name", if_array[if_index].name );

	//interface bandwidth
	memset( value, 0x00, sizeof(value) );
	sprintf( value, "%d", 24000000 );
	add_xml_child( node_child, "interface_bandwidth", value );

	//interface send data
	memset( value, 0x00, sizeof(value) );
	sprintf( value, "%lu", if_array[if_index].device_stats.tx_bytes );
	add_xml_child( node_child, "current_send_data", value );

	//interface drops
	memset( value, 0x00, sizeof(value) );
	sprintf( value, "%lu", if_array[if_index].device_stats.tx_dropped );
	add_xml_child( node_child, "drops", value );

	return 0;
}

/*
	packet interface's information into xml format
 */

xmlNodePtr packet_xml_interface( xmlNodePtr node, int if_index )
{
	xmlNodePtr	node_child = NULL;
	char		value[50];

	if( node == NULL 
		|| if_index < 0 || if_index > g_interface_size )
	{
		WriteLog( ERR, "[%s][%d]input parameter is wrong\n", __FILE__, __LINE__ );
		return NULL;
	}

	//interface name
	add_xml_child( node, "interface_name", if_array[if_index].name );

	node_child = add_xml_child( node, "flow_info", NULL );

	return node_child;
}

/*
	packet flow's information into xml format
 */

int	packet_xml_flow_info( xmlNodePtr node, char *flow, float sampled_rate, int send_flag )
{
	char			value[INET6_ADDRSTRLEN+1];
	xmlNodePtr		node_child = NULL;
	struct _ipv4_struct	*q = NULL;
	double			current_interval;
	double			l_time;
	double			l_interval_time;
	double			l_sent_bytes;
	double			l_sent_rate;

	if( node == NULL || flow == NULL )
	{
		WriteLog( ERR, "[%s][%d]interface is null\n", __FILE__, __LINE__ );
		return -1;
	}

	//WriteLog( DEBUG, "[%s][%d]\n", __FILE__, __LINE__ );

	q = (struct _ipv4_struct *)flow;

	node_child = add_xml_child( node, "flow", NULL );

	//flow type
        memset( value, 0x00, sizeof(value) );
	sprintf( value, "%d", 4 );
	add_xml_child( node_child, "flow_type", value );
	
	//protocol
	add_xml_child( node_child, "protocol", NULL );

	//dscp value
    memset( value, 0x00, sizeof(value) );
	sprintf( value, "%d", (q->flow).dscp );
	add_xml_child( node_child, "dscp", value );

	//ip src
        memset( value, 0x00, sizeof(value) );
        inet_ntop(AF_INET, (void *)&((q->flow).ip_src), value, sizeof(value)-1);
	add_xml_child( node_child, "src", value );

	//ip dst
        memset( value, 0x00, sizeof(value) );
        inet_ntop(AF_INET, (void *)&((q->flow).ip_dst), value, sizeof(value)-1);
	add_xml_child( node_child, "dst", value );

	//src port
    memset( value, 0x00, sizeof(value) );
	sprintf( value, "%d", (q->flow).port_src );
	add_xml_child( node_child, "src_port", value );

	//dst port
    memset( value, 0x00, sizeof(value) );
	sprintf( value, "%d", (q->flow).port_dst );
	add_xml_child( node_child, "dst_port", value );

	//input interface
	sprintf( value, "%d", (q->flow).if_in );
	add_xml_child( node_child, "input_if", value );

	//output interface
	sprintf( value, "%d", (q->flow).if_out );
	add_xml_child( node_child, "output_if", value );

	//cal send_rate
	if( (q->start).tv_sec >= (q->end).tv_sec )
	{
		//?????
		current_interval = g_host_config.captured_time; 
		WriteLog( NORMT, "[%s][%d]***************time*************=[%d]\n", __FILE__, __LINE__, g_host_config.captured_time );
	}
	else
	{
		current_interval = get_diff_time( &(q->start), &(q->end) );
	}

WriteLog( DEBUG, "[%s][%d]current_interval=[%.2f]\n", __FILE__, __LINE__, current_interval ); 
	//2011-07-07 start
	if( current_interval == 0 )
	{
		current_interval = g_host_config.captured_time;
	}
	//2011-07-07 end

	l_time = 1000000;
	l_interval_time = current_interval / l_time;
	l_sent_bytes = (q->flow).total_send;
	l_sent_rate = (l_sent_bytes / l_interval_time) / sampled_rate;

WriteLog( DEBUG, "[%s][%d]total_send=[%.2f]\n", __FILE__, __LINE__, l_sent_bytes );
WriteLog( NORMT, "[%s][%d]sampled_rate=[%.2f]\n", __FILE__, __LINE__, sampled_rate );
WriteLog( DEBUG, "[%s][%d]l_send_rate=[%.2f]\n", __FILE__, __LINE__, l_sent_rate );

WriteLog( DEBUG, "[%s][%d]l_time=[%.2f]\n", __FILE__, __LINE__, l_time ); 
WriteLog( DEBUG, "[%s][%d]l_interval_time=[%.2f]\n", __FILE__, __LINE__, l_interval_time ); 
WriteLog( DEBUG, "[%s][%d]current_interval=[%.2f]\n", __FILE__, __LINE__, current_interval ); 

WriteLog( DEBUG, "[%s][%d]l_interval_time=[%.2f],l_sent_bytes=[%.2f],l_sent_rate=[%.2f]\n", __FILE__, __LINE__, l_interval_time, l_sent_bytes, l_sent_rate );

WriteLog( DEBUG, "[%s][%d]start time: %ld secs %ld micros\n", __FILE__, __LINE__, (q->start).tv_sec, (q->start).tv_usec );
WriteLog( DEBUG, "[%s][%d]end time: %ld secs %ld micros\n", __FILE__, __LINE__, (q->end).tv_sec, (q->end).tv_usec );


	//sprintf( value, "%d", (q->flow).total_send );
	//sprintf( value, "%.2f", l_sent_rate );
	sprintf( value, "%.0f", l_sent_rate*8 );
	if( FLOW_OUT == send_flag )
	{
		add_xml_child( node_child, "out_send_rate", value );
		add_xml_child( node_child, "in_send_rate", NULL );
	}
	else
	{
		add_xml_child( node_child, "out_send_rate", NULL );
		add_xml_child( node_child, "in_send_rate", value );
	}

	return 0;
}

/*
	create document
 */

xmlDocPtr create_xml_doc()
{
	xmlDocPtr	doc = NULL;

	doc = xmlNewDoc( BAD_CAST "1.0" );

	return doc;
}

/*
	add root node
 */

xmlNodePtr create_xml_root_node( xmlDocPtr doc, char *name )
{
	xmlNodePtr	root_node = NULL;

	root_node = xmlNewNode( NULL, BAD_CAST name );
	xmlDocSetRootElement( doc, root_node );

	return root_node;
}

/*
	add child node
 */

xmlNodePtr add_xml_child( xmlNodePtr node, char *name, char *value )
{
	xmlNodePtr	this_node = NULL;
	this_node = xmlNewChild( node, NULL, BAD_CAST name, value );

	return this_node;
}

/*
	add node's attribute
 */

int add_xml_child_prop( xmlNodePtr node, char *name, char *value )
{
	xmlNewProp( node, BAD_CAST name, BAD_CAST value );
	return 0;
}

//packet interface xml buffer
//组织接口信息报文
int pack_interface_info_xml_buff( __u8 **xmlbuff, int *len, int size )
{
	int		i = 0;
	xmlDocPtr	doc = NULL;
	xmlNodePtr	root_node = NULL;
	xmlNodePtr	node = NULL;
	char		value[50];
	int		this_len = 0;

	//create document
	doc = create_xml_doc();
	root_node = create_xml_root_node( doc, "INTERFACE_INFO" );

	//1.add timestamp field
	memset( value, 0x00, sizeof(value) );
	get_data_time( value, sizeof(value)-1 );
	add_xml_child( root_node, "timestamp", value );

	//2.add router field ??????
	node = add_xml_child( root_node, "ROUTER", NULL );
	sprintf( value, "%d", g_host_config.router_id );
	add_xml_child_prop( node, "id", value );

	for( i=0; i<size && i<MAX_INTERFACE_SIZE; i++ )
	{
		if( 1 == ife[i].active )
		{
			op_interface_info( i, node );
		}
	}

	//xmlSaveFormatFileEnc( "tmp.xml", doc, "UTF-8", 1 );
	//xmlDocDumpFormatMemory( doc, xmlbuff, &this_len, 0 );
	xmlDocDumpFormatMemoryEnc( doc, xmlbuff, &this_len, "UTF-8", 0 );
	*len = this_len;
	xmlFreeDoc( doc );

	return 0;
}

/*
	one interface's flow information
 */

int	op_interface_info( int if_index, xmlNodePtr node )
{
	xmlNodePtr		node_child = NULL;
	char			value[50];
	int			i = 0;
	int			queue_num = 0;	
	struct list_head	*l = NULL;
	double			sampled_rate = 0;
	unsigned long		old_rx_bytes = 0;
	unsigned long		new_rx_bytes = 0;
	unsigned long		old_tx_bytes = 0;
	unsigned long		new_tx_bytes = 0;

	if( node == NULL 
		|| if_index < 0 
		|| if_index > MAX_INTERFACE_SIZE )
	{
		WriteLog( DEBUG, "[%s][%d]node is null\n", __FILE__, __LINE__ );
		return -1;
	}

	// interface 
	sprintf( value, "%d", ife[if_index].if_index );
	node_child = add_xml_child( node, "interface", NULL );
	add_xml_child_prop( node_child, "id", value );

	//child node start
	// interface name
	add_xml_child( node_child, "interface_name", ife[if_index].name );

	//ipv4 addr
	add_xml_child( node_child, "interface_ipv4_address", ife[if_index].ipv4_addr );

	//ipv4 netmask
	add_xml_child( node_child, "interface_ipv4_netmask", ife[if_index].ipv4_netmask );

	//ipv6 address 
	add_xml_child( node_child, "interface_ipv6_address", ife[if_index].ipv6_addr );

	//ipv6 address netmask
	memset( value, 0x00, sizeof(value) );
	sprintf( value, "%d", ife[if_index].ipv6_addr_netmask );
	add_xml_child( node_child, "interface_ipv6_netmask", value );

	//mac
	u_char  *mac_string = NULL;

	memset( value, 0x00, sizeof(value) );
        mac_string = ife[if_index].ether_host;
	snprintf( value, sizeof(value)-1, "%02x:%02x:%02x:%02x:%02x:%02x", *mac_string, *(mac_string + 1), *(mac_string + 2), *(mac_string + 3), *(mac_string + 4), *(mac_string + 5) );
	add_xml_child( node_child, "interface_mac", value );

	//interface bandwidth
	add_xml_child( node_child, "interface_bandwidth", "24000000" );

//	printf( "[%s][%d]index=[%d]\n", __FILE__, __LINE__, if_index );
	//rx bytes
	memset( value, 0x00, sizeof(value) );
	old_rx_bytes = ife[if_index].rx_bytes;
	new_rx_bytes = ife[if_index].device_stats.rx_bytes;
	ife[if_index].rx_bytes = new_rx_bytes;
	sprintf( value, "%lu", (new_rx_bytes - old_rx_bytes)/INTERVAL_TIME );

	//printf( "[%s][%d]rx:old=[%lu],new=[%lu]\n", __FILE__, __LINE__, old_rx_bytes, new_rx_bytes );
	//printf( "[%s][%d]rx:change=[%s]\n", __FILE__, __LINE__, value );

	add_xml_child( node_child, "current_send_in_data", value );

	//tx bytes
	memset( value, 0x00, sizeof(value) );
	old_tx_bytes = ife[if_index].tx_bytes;
	new_tx_bytes = ife[if_index].device_stats.tx_bytes;
	ife[if_index].tx_bytes = new_tx_bytes;
	sprintf( value, "%lu", (new_tx_bytes - old_tx_bytes)/INTERVAL_TIME );

//	printf( "[%s][%d]tx:old=[%lu],new=[%lu]\n", __FILE__, __LINE__, old_tx_bytes, new_tx_bytes );
//	printf( "[%s][%d]tx:change=[%s]\n\n\n", __FILE__, __LINE__, value );

	add_xml_child( node_child, "current_send_out_data", value );

	//child node end

	return 0;
}

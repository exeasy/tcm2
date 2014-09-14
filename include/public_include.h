#ifndef	_PUBLIC_INCLUDE
#define	_PUBLIC_INCLUDE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/wait.h>           /* waitpid()                    */
#include <arpa/inet.h>          /* inet_addr()                  */
#include <netinet/in.h>         /* INADDR_NONE                  */
#include <netinet/ip.h>         /* INADDR_NONE                  */
#include <netinet/ip6.h>
#include <netdb.h>              /* gethostbyname                */
//#include <net/if.h>             /* get_my_addresses() support   */
#include <pthread.h>            /* pthreads support             */
#include <ctype.h>
#include <fcntl.h>
#include <stdarg.h>
#include <errno.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include <net/ethernet.h>
#include <linux/netlink.h>      /* get_my_addresses() support   */
#include <linux/rtnetlink.h>    /* get_my_addresses() support   */
#include <linux/if.h>           /* set_link_params() support    */
#include <sys/ioctl.h>          /* get_my_addresses() support   */

//#include <netinet/if_ether.h>
//#include <netinet/ether.h>
#include <net/if_arp.h>
#include <linux/netdevice.h>

#include <pcap.h>

#endif /* _PUBLIC_INCLUDE */

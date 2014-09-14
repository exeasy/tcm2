#ifndef	_TC_PUBLIC_
#define	_TC_PUBLIC_

//#define	_DEBUG_SYMBOL

#include "public_include.h"

typedef unsigned char __u8;
typedef unsigned short __u16;
typedef unsigned int __u32;

/* get pointer to IP from a sockaddr 
 *    useful for inet_ntop calls     */
#define SA2IP(x) (((struct sockaddr*)x)->sa_family==AF_INET) ? \
	(__u8*)&((struct sockaddr_in*)x)->sin_addr : \
	(__u8*)&((struct sockaddr_in6*)x)->sin6_addr
/* get socket address length in bytes */
#define SALEN(x) (((struct sockaddr*)x)->sa_family==AF_INET) ? \
	sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6)
/* get IP address length in bytes */
#define SAIPLEN(x) (((struct sockaddr*)x)->sa_family==AF_INET) ? 4 : 16
/* get (__u16) port from socket address */
#define SA2PORT(x) (((struct sockaddr*)x)->sa_family==AF_INET) ? \
	((struct sockaddr_in*)x)->sin_port : \
	((struct sockaddr_in6*)x)->sin6_port
/* cast to sockaddr */
#define SA(x) ((struct sockaddr*)x)

#define	SYS_LIST_LEN 30
#define MAX_INTERFACE_SIZE 10
#define	MAX_BUFF_LEN 200

#define	TC_TRUE 1
#define	TC_FAIL 0
#define	TC_ERROR -1
#define	TC_SUCCESS 0

#define ETH_LEN 6
int	g_interface_size;

//pma agent struct
struct _tc_config_struct
{
	int			router_id;
	struct sockaddr_storage	pma_addr;
	int			pma_port;	
	//other information
	int			captured_time;
	int			interval;
	int			nmax;
	double			a;
	double			p;
	FILE			*fp ;
};

struct _tc_config_struct g_host_config;

#define LOGFILE 1

pthread_mutex_t mutex;
pthread_mutex_t mutex_pro;
pthread_cond_t cond;
pthread_cond_t cond_pro;

#define QUEUENUM 6
typedef int TYPE;

double system_best_sampled_n;

typedef enum {
        NORM,
        NORMT,
        WARN,
        ERR,
        DEBUG,
} LOG_LEVELS;


#endif /* _TC_PUBLIC_ */

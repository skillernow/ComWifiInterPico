#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__

// Configuration minimale pour Pico W
#define NO_SYS 1
#define LWIP_SOCKET 0
#define LWIP_NETCONN 0
#define MEM_ALIGNMENT 4
#define MEM_SIZE (1600)
#define PBUF_POOL_SIZE 5
#define LWIP_DHCP 0
#define LWIP_IPV4 1
#define LWIP_TCP 1
#define LWIP_UDP 0
#define LWIP_DNS 0
#define LWIP_NETIF_HOSTNAME 0
#define LWIP_NETIF_STATUS_CALLBACK 1
#define LWIP_NETIF_LINK_CALLBACK 1
#define LWIP_DEBUG 0
#define TCP_MSS (1460)
#define TCP_SND_BUF (4 * TCP_MSS)
#define TCP_WND (2 * TCP_MSS)

#endif // __LWIPOPTS_H__

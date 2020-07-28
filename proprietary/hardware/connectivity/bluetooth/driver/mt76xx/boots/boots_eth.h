#ifndef __BOOTS_ETHERNET_H__
#define __BOOTS_ETHERNET_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#define ETH_UDP_SRV_PORT 4567
#define ETH_UDP_CLT_PORT 4568

//---------------------------------------------------------------------------
int boots_eth_create(int * sk, struct sockaddr_in * sockaddr, int client);

int boots_eth_close(int * sk);

ssize_t boots_eth_send(int s, const void * buf, size_t len, struct sockaddr_in * to);

ssize_t boots_eth_recv(int s, const void * buf, size_t len, int flags);

//---------------------------------------------------------------------------
#endif // __BOOTS_ETHERNET_H__


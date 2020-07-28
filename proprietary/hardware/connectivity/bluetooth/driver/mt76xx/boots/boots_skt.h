//- vim: set ts=4 sts=4 sw=4 et: --------------------------------------------
#ifndef __BOOTS_SOCKET_H__
#define __BOOTS_SOCKET_H__

#include <sys/socket.h>
#include <sys/un.h>

#include "boots_cfg.h"

//---------------------------------------------------------------------------
#ifndef BOOTS_SOCKET_PATH
#define BOOTS_SOCKET_PATH   ""
#endif

#define BOOTS_SRVSK_NAME    BOOTS_SOCKET_PATH"boots_srvsk"
#define BOOTS_CLISK_NAME    BOOTS_SOCKET_PATH"boots_clisk"

//---------------------------------------------------------------------------
#ifndef HCI_BUF_SIZE
#define HCI_BUF_SIZE 512
#endif

//---------------------------------------------------------------------------
int boots_sk_create(int * sk, struct sockaddr_un * sockaddr,
        socklen_t * socklen, char * name, int client);

int boots_sk_close(int * sk);

ssize_t boots_sk_send(int s, const void * buf, size_t len,
        struct sockaddr_un * to, char * name);

ssize_t boots_sk_recv(int s, const void * buf, size_t len, int flags,
        struct sockaddr * src_addr, socklen_t * addrlen);

//---------------------------------------------------------------------------
#endif // __BOOTS_SOCKET_H__

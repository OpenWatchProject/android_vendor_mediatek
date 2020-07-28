//- vim: set ts=4 sts=4 sw=4 et: --------------------------------------------
#include <errno.h>
#include <string.h>
#include <stddef.h>
#include <unistd.h>

#include "boots.h"
#include "boots_skt.h"

//---------------------------------------------------------------------------
#define LOG_TAG "boots_skt"

//---------------------------------------------------------------------------
int boots_sk_create(int * sk, struct sockaddr_un * sockaddr, socklen_t * socklen,
        char * name, int client)
{
    UNUSED(boots_btif);

    // Create server socket
    *sk = socket(client ? PF_LOCAL : AF_UNIX, SOCK_DGRAM, 0);
    if (*sk < 0) {
        BPRINT_E("Create socket failed: %s(%d)", strerror(errno), errno);
        return errno;
    }

    // Init server socket
    sockaddr->sun_family = AF_UNIX;
    memset(sockaddr->sun_path, 0, sizeof(sockaddr->sun_path));
    strncpy(sockaddr->sun_path, name, strlen(name));
    *socklen = (offsetof(struct sockaddr_un, sun_path) + strlen(sockaddr->sun_path) + 1);

    if (!client) {
        // Remove the socket first
        unlink(sockaddr->sun_path);
        if (bind(*sk, (struct sockaddr *)sockaddr, *socklen) < 0) {
            BPRINT_E("Bind socket failed: %s(%d)", strerror(errno), errno);
            return errno;
        }

    } else {
        int memsize = HCI_BUF_SIZE;
        if (setsockopt(*sk, SOL_SOCKET, SO_RCVBUF, &memsize, sizeof(memsize)) < 0)
            BPRINT_E("Set socket buffer size failed: %s(%d)", strerror(errno), errno);

        if (connect(*sk, (struct sockaddr *)sockaddr, *socklen) < 0) {
            BPRINT_E("Failed to connect to socket: %s(%d)", strerror(errno), errno);
            return errno;
        }
    }

    BPRINT_D("Socket %s create success", name);
    return 0;
}

//---------------------------------------------------------------------------
int boots_sk_close(int * sk)
{
    if (sk == NULL) {
        BPRINT_E("%s: Incorrect socket", __func__);
        return -1;
    }
    close(*sk);
    *sk = -1;
    return 0;
}

//---------------------------------------------------------------------------
ssize_t boots_sk_send(int s, const void * buf, size_t len, struct sockaddr_un * to,
        char * name)
{
    ssize_t ret = 0;

    if (!buf || !len) {
        BPRINT_E("%s: Incorrect argument", __func__);
        return -1;
    }
    if (s < 0) {
        BPRINT_E("%s: Incorrect FD(%d)", __func__, s);
        return -1;
    }
#if 0
    ret = sendto(s, buf, len, 0, (const struct sockaddr *)to,
            sizeof(struct sockaddr_un));
#else
    UNUSED(to);
    UNUSED(name);
    ret = send(s, buf, len, 0);
#endif
    if (ret < 0)
        BPRINT_E("%s: Send message failed: %s(%d)", __func__, strerror(errno), errno);

    return ret;
}

//---------------------------------------------------------------------------
ssize_t boots_sk_recv(int s, const void * buf, size_t len, int flags,
        struct sockaddr * src_addr, socklen_t * addrlen)
{
    ssize_t ret = 0;

    if (!buf || !len) {
        BPRINT_E("%s: Incorrect argument", __func__);
        return -1;
    }
    if (s < 0) {
        BPRINT_E("%s: Incorrect FD(%d)", __func__, s);
        return -1;
    }

    ret = recvfrom(s, (void *)buf, len, flags, src_addr, addrlen);
    if (ret < 0)
        BPRINT_E("%s: Receive message failed: %s(%d)", __func__, strerror(errno), errno);

    return ret;
}

//---------------------------------------------------------------------------

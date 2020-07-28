#include <errno.h>
#include <string.h>
#include <stddef.h>
#include <unistd.h>

#include "boots.h"
#include "boots_eth.h"

//---------------------------------------------------------------------------
#define LOG_TAG "boots_eth"

//---------------------------------------------------------------------------
int boots_eth_create(int * sk, struct sockaddr_in * sockaddr, int client)
{
    UNUSED(boots_btif);

    // Create server socket
    *sk = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (*sk < 0) {
        BPRINT_E("Create socket failed: %s(%d)", strerror(errno), errno);
        return errno;
    }

    // Init server socket
    if (!client) {
        memset(sockaddr, 0, sizeof(struct sockaddr_in));
        sockaddr->sin_family = AF_INET;
        sockaddr->sin_port = ETH_UDP_SRV_PORT;
        sockaddr->sin_addr.s_addr = INADDR_ANY;
        if (bind(*sk, (struct sockaddr *)sockaddr, sizeof(struct sockaddr)) < 0) {
            BPRINT_E("Bind socket failed: %s(%d)", strerror(errno), errno);
            return errno;
        }
    }

    BPRINT_D("Socket %s create success", client ? "Client" : "Server");
    return 0;
}

//---------------------------------------------------------------------------
int boots_eth_close(int * sk)
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
ssize_t boots_eth_send(int s, const void * buf, size_t len, struct sockaddr_in * to)
{
    ssize_t ret = 0;

    if (!buf || !len) {
        BPRINT_E("%s: Incorrect parameters", __func__);
        return -1;
    }
#if 1
    ret = sendto(s, buf, len, 0, (const struct sockaddr *)to,
            sizeof(struct sockaddr));
#else
    ret = send(s, buf, len, 0);
#endif
    if (ret < 0)
        BPRINT_E("%s: Send message failed: %s(%d)", __func__, strerror(errno), errno);

    return ret;
}

//---------------------------------------------------------------------------
ssize_t boots_eth_recv(int s, const void * buf, size_t len, int flags)
{
    ssize_t ret = 0;
    struct sockaddr srcaddr;
    socklen_t srcaddr_len = sizeof(struct sockaddr);

    if (!buf || !len) {
        BPRINT_E("%s: Incorrect parameters", __func__);
        return -1;
    }

    ret = recvfrom(s, (void *)buf, len, flags, &srcaddr, &srcaddr_len);
    if (ret < 0)
        BPRINT_E("%s: Receive message failed: %s(%d)", __func__, strerror(errno), errno);

    return ret;
}


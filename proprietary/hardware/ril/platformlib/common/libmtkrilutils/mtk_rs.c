/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define LOG_TAG "Ipv6Rs"


#include <cutils/sockets.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/icmp6.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <mtk_log.h>

static const char *IN6ADDR_ALL_ROUTERS = "FF02::2"; // v6 addr not built in

#ifdef byte
#undef byte
#endif

#define byte unsigned char

#define SUCCESS 0
#define ERROR -1

#define MAX_MAC_ADDR_LEN 6

static const int SEND_TIMEOUT_MS = 300;
static const int LINK_LOCAL_HOP_LIMIT = 255;
static const byte ICMPV6_ND_ROUTER_SOLICIT = 133;

int setupRaSocket(int fd, int ifIndex) {
    // Set an ICMPv6 filter that only passes Router Solicitations.
    struct icmp6_filter rs_only;
    ICMP6_FILTER_SETBLOCKALL(&rs_only);
    ICMP6_FILTER_SETPASS(ND_ROUTER_SOLICIT, &rs_only);
    socklen_t len = sizeof(rs_only);
    if (setsockopt(fd, IPPROTO_ICMPV6, ICMP6_FILTER, &rs_only, len) != 0) {
        mtkLogE(LOG_TAG, "setsockopt ICMP6_FILTER error:%s", strerror(errno));
        return ERROR;
    }

    // Set the multicast hoplimit to 255 (link-local only).
    int hops = LINK_LOCAL_HOP_LIMIT;
    len = sizeof(hops);
    if (setsockopt(fd, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &hops, len) != 0) {
        mtkLogE(LOG_TAG, "setsockopt IPV6_MULTICAST_HOPS error:%s", strerror(errno));
        return ERROR;
    }
    if (setsockopt(fd, IPPROTO_IPV6, IPV6_UNICAST_HOPS, &hops, len) != 0) {
        mtkLogE(LOG_TAG, "setsockopt IPV6_UNICAST_HOPS error:%s", strerror(errno));
        return ERROR;
    }

    // Explicitly disable multicast loopback.
    int off = 0;
    len = sizeof(off);
    if (setsockopt(fd, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, &off, len) != 0) {
        mtkLogE(LOG_TAG, "setsockopt IPV6_MULTICAST_LOOP error:%s", strerror(errno));
        return ERROR;
    }

    // Specify the IPv6 interface to use for outbound multicast.
    len = sizeof(ifIndex);
    if (setsockopt(fd, IPPROTO_IPV6, IPV6_MULTICAST_IF, &ifIndex, len) != 0) {
        mtkLogE(LOG_TAG, "setsockopt IPV6_MULTICAST_IF error:%s", strerror(errno));
        return ERROR;
    }

    // Additional options to be considered:
    //     - IPV6_TCLASS
    //     - IPV6_RECVPKTINFO
    //     - IPV6_RECVHOPLIMIT

    // Bind to [::].
    const struct sockaddr_in6 sin6 = {
            .sin6_family = AF_INET6,
            .sin6_port = 0,
            .sin6_flowinfo = 0,
            .sin6_addr = IN6ADDR_ANY_INIT,
            .sin6_scope_id = 0,
    };
    len = sizeof(sin6);

    if (bind(fd, (struct sockaddr*) &sin6, len) != 0) {
        mtkLogE(LOG_TAG, "bind(IN6ADDR_ANY):%s", strerror(errno));
        return ERROR;
    }

    // Join the all-routers multicast group, ff02::2%index.
    struct ipv6_mreq all_rtrs = {
        .ipv6mr_multiaddr = {{{0xff,2,0,0,0,0,0,0,0,0,0,0,0,0,0,2}}},
        .ipv6mr_interface = ifIndex,
    };
    len = sizeof(all_rtrs);
    if (setsockopt(fd, IPPROTO_IPV6, IPV6_JOIN_GROUP, &all_rtrs, len) != 0) {
        mtkLogE(LOG_TAG, "setsockopt (IPV6_JOIN_GROUP):%s", strerror(errno));
        return ERROR;
    }

    return SUCCESS;
}

int createRawSocket(const char* iface) {
    int rawSocket = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6);
    if (rawSocket < 0) {
        mtkLogE(LOG_TAG, "rawSocket error:%s", strerror(errno));
        return ERROR;
    }

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = SEND_TIMEOUT_MS * 1000;
    socklen_t len = sizeof(timeout);
    if (setsockopt(rawSocket, SOL_SOCKET, SO_SNDTIMEO, &timeout, len) < 0) {
        mtkLogE(LOG_TAG, "setsockopt SO_SNDTIMEO error:%s", strerror(errno));
        return ERROR;
    }
    if (setsockopt(rawSocket, SOL_SOCKET, SO_BINDTODEVICE, iface, strlen(iface) + 1) < 0) {
        mtkLogE(LOG_TAG, "setsockopt SO_BINDTODEVICE error:%s", strerror(errno));
        return ERROR;
    }

    unsigned int index = if_nametoindex(iface);
    if (setupRaSocket(rawSocket, index) < 0) {
        mtkLogE(LOG_TAG, "setupRaSocket error:%s", strerror(errno));
        return ERROR;
    }
    mtkLogI(LOG_TAG, "createRawSocket done");
    return rawSocket;
}

void getHwAddr(const char* iface, byte* mac) {
    int sockId;
    struct ifreq ifr_mac;
    char mac_addr[30];

    sockId = socket(AF_INET, SOCK_STREAM, 0);
    if (sockId == -1) {
        mtkLogE(LOG_TAG, "socket error:%s", strerror(errno));
        return;
    }
    memset(&ifr_mac, 0, sizeof(ifr_mac));
    strncpy(ifr_mac.ifr_name, iface, sizeof(ifr_mac.ifr_name)-1);
    if ((ioctl(sockId, SIOCGIFHWADDR, &ifr_mac)) < 0) {
        close(sockId);
        mtkLogE(LOG_TAG, "SIOCGIFHWADDR error:%s", strerror(errno));
        return;
    }

    memcpy(mac, ifr_mac.ifr_hwaddr.sa_data, MAX_MAC_ADDR_LEN);

    sprintf(mac_addr, "%02x:%02x:%02x:%02x:%02x:%02x",
            (unsigned char) mac[0],
            (unsigned char) mac[1],
            (unsigned char) mac[2],
            (unsigned char) mac[3],
            (unsigned char) mac[4],
            (unsigned char) mac[5]);

    close(sockId);
    mtkLogD(LOG_TAG, "getHwAddr:%s %s", iface, mac_addr);
}

int assebleRaMessage(byte* data, char* iface) {
    /**
        Router Solicitation Message Format
        0                   1                   2                   3
        0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |     Type      |     Code      |          Checksum             |
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |                         Reserved                              |
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |   Options ...
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
    */
    byte mac[MAX_MAC_ADDR_LEN];
    memset(mac, 0, sizeof(mac));

    data[0] = ICMPV6_ND_ROUTER_SOLICIT;
    data[8] = 0x01; // Source Link Address
    data[9] = 0x01; // Length

    getHwAddr(iface, mac);
    memcpy(&data[10], mac, MAX_MAC_ADDR_LEN);

    return 16;
}

int sendRs(int socketFd, char* iface) {
    size_t addrSize = 0;
    ssize_t numBytes = 0;
    byte raData[IPV6_MIN_MTU];
    int dataSize = 0;

    mtkLogD(LOG_TAG, "Construct RA message");
    memset(raData, 0, sizeof(raData) / sizeof(byte));
    dataSize = assebleRaMessage(raData, iface);

    mtkLogI(LOG_TAG, "send IPv6 RS");

    // The RS destiantion is ff02::1.
    struct sockaddr_in6 destAddr6 = {
            .sin6_family = AF_INET6,
            .sin6_port = 0,
            .sin6_flowinfo = 0,
            .sin6_addr = IN6ADDR_ANY_INIT,
            .sin6_scope_id = 0,
    };
    if (inet_pton(AF_INET6, IN6ADDR_ALL_ROUTERS, &destAddr6.sin6_addr) <= 0) {
        mtkLogE(LOG_TAG, "inet_pton failed");
        return ERROR;
    }
    addrSize = sizeof(destAddr6);

    numBytes = sendto(socketFd, raData, dataSize, 0,
                                (const struct sockaddr*) &destAddr6, addrSize);
    if (numBytes < 0) {
        mtkLogE(LOG_TAG, "Send failure with %s", strerror(errno));
        return ERROR;
    } else {
        mtkLogD(LOG_TAG, "Send IPv6 RS: %d:%zd", dataSize, numBytes);
    }

    return SUCCESS;
}

int triggerIpv6Rs(char* iface) {
    int sockFd = createRawSocket(iface);
    if (sockFd < 0) {
        mtkLogE(LOG_TAG, "Failed to createRawSocket with %s", iface);
        return ERROR;
    }
    sendRs(sockFd, iface);
    return SUCCESS;
}

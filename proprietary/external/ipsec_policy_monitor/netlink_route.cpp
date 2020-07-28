/* 

* Copyright Statement:

* This software/firmware and related documentation ("MediaTek Software") are

* protected under relevant copyright laws. The information contained herein

* is confidential and proprietary to MediaTek Inc. and/or its licensors.

* Without the prior written permission of MediaTek inc. and/or its licensors,

* any reproduction, modification, use or disclosure of MediaTek Software,

* and information contained herein, in whole or in part, shall be strictly prohibited.

* 

* Copyright  (C) 2019  MediaTek Inc. All rights reserved.

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

* THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSESss

* CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK

* SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR

* STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND

* CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,

* AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,

* OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO

* MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.

* The following software/firmware and/or related documentation ("MediaTek Software")

* have been modified by MediaTek Inc. All revisions are subject to any receiver's

* applicable license agreements with MediaTek Inc.

*/

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/uio.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <stdarg.h>
#include "utils_xfrm.h"
#define LOG_TAG "ipsec_policy_mont"
#include <log/log.h>
#include <string>
#include <list>
#include "netlink_route.h"

const int XFRM_ROUTE_TABLE_OFFSET_FROM_INDEX = 1000;
const char* const XFRM_IIF_NONE = NULL;
const char* const XFRM_OIF_NONE = NULL;
const sockaddr_nl KERNEL_NLADDR = {AF_NETLINK, 0, 0, 0};

constexpr uint16_t U16_RTA_LENGTH(uint16_t x) {
    return RTA_LENGTH(x);
}

rtattr RTATTR_TABLE = { U16_RTA_LENGTH(sizeof(uint32_t)), RTA_TABLE };
rtattr RTATTR_OIF   = { U16_RTA_LENGTH(sizeof(uint32_t)), RTA_OIF };

const uint16_t NETLINK_REQUEST_FLAGS = NLM_F_REQUEST | NLM_F_ACK;
const uint16_t NETLINK_CREATE_REQUEST_FLAGS = NETLINK_REQUEST_FLAGS | NLM_F_CREATE | NLM_F_EXCL;

static const char *actionName(uint16_t action) {
    static const char *ops[4] = {"adding", "deleting", "getting", "???"};
    return ops[action % 4];
}

static int XfrmPrefixParse(uint8_t *family, const char *prefix, void *address,
                int size, uint8_t *prefix_len) {
    size_t offset;
    int byteLength;
    char *endptr;
    unsigned templen;

    if (!prefix || !family || !address || !prefix_len) {
        return -EFAULT;
    }
    const char *slash = strchr(prefix, '/');
    const char *prefixlenString = slash + 1;
    if (!slash || !*prefixlenString) {
        ALOGE("slash == NULL or prefixlenString == NULL");
        return -EINVAL;
    }

    templen = strtoul(prefixlenString, &endptr, 10);
    if (*endptr || templen > 128) {
        return -EINVAL;
    }
    *prefix_len = templen;

    offset = slash - prefix;
    int is_ipv6 = !!strchr(prefix, ':');
    int addrlen = (is_ipv6) ? INET6_ADDRSTRLEN : INET_ADDRSTRLEN;
    char ip_addr[addrlen];
    memset(ip_addr, 0, addrlen);
    *family = is_ipv6 ? AF_INET6 : AF_INET;

    switch (*family) {
        case AF_INET: {
            if (*prefix_len > 32) {
                return -EINVAL;
            }
            byteLength = 4;
            break;
        }
        case AF_INET6: {
            if (*prefix_len > 128) {
                return -EINVAL;
            }
            byteLength = 16;
            break;
        }
        default: {
            ALOGE("ip type not support");
            return -EAFNOSUPPORT;
        }
    }
    if (byteLength > size) {
        return -ENOSPC;
    }
    memcpy(ip_addr, prefix, offset);
    if (inet_pton(*family, ip_addr, address) <= 0) {
        ALOGE("invalid ip address: '%s', errno = %s", ip_addr, strerror(errno));
        return -EINVAL;
    }
    return byteLength;
}

std::map<std::string, uint32_t> InterfaceToXfrmRouteTable;
uint32_t XfrmGetRouteTableForInterface(const char* interface) {
    auto iter = InterfaceToXfrmRouteTable.find(interface);
    if (iter != InterfaceToXfrmRouteTable.end()) {
        return iter->second;
    }

    uint32_t index = if_nametoindex(interface);
    if (index == 0) {
        ALOGE("cannot find interface %s", interface);
        return RT_TABLE_UNSPEC;
    }
    index += XFRM_ROUTE_TABLE_OFFSET_FROM_INDEX;
    InterfaceToXfrmRouteTable[interface] = index;
    return index;
}

void XfrmEarseInterfaceFromTable(const char* interface) {
    InterfaceToXfrmRouteTable.erase(interface);
}

static int XfrmOpenNetlinkSocket(int protocol) {
    int sock = socket(AF_NETLINK, SOCK_DGRAM | SOCK_CLOEXEC, protocol);
    if (sock == -1) {
        return -errno;
    }
    if (connect(sock, reinterpret_cast<const sockaddr*>(&KERNEL_NLADDR),
                sizeof(KERNEL_NLADDR)) == -1) {
        close(sock);
        return -errno;
    }
    return sock;
}

static int XfrmSendNetlinkRequest(uint16_t action, uint16_t flags, iovec* iov, int iovlen,
                                          const NetlinkDumpCallback *callback) {
    nlmsghdr nlmsg = {
        .nlmsg_type = action,
        .nlmsg_flags = flags,
    };
    iov[0].iov_base = &nlmsg;
    iov[0].iov_len = sizeof(nlmsg);
    for (int i = 0; i < iovlen; ++i) {
        nlmsg.nlmsg_len += iov[i].iov_len;
    }

    int sock = XfrmOpenNetlinkSocket(NETLINK_ROUTE);
    if (sock < 0) {
        return sock;
    }

    int ret = 0;
    if (writev(sock, iov, iovlen) == -1) {
        ret = -errno;
        ALOGE("netlink socket connect/writev failed (%s)", strerror(-ret));
        close(sock);
        return ret;
    }
    if (flags & NLM_F_ACK) {
        ret = XfrmRecvNetlinkAck(sock);
    }
    close(sock);
    return ret;
}

static int XfrmRecvNetlinkAck(int sock) {
    struct {
        nlmsghdr msg;
        nlmsgerr err;
    } ack;

    int ret = recv(sock, &ack, sizeof(ack), 0);

    if (ret == -1) {
        ret = -errno;
        ALOGE("netlink recv failed (%s)", strerror(-ret));
        return ret;
    }

    if (ret != sizeof(ack)) {
        ALOGE("netlink ack message size error (%d != %zu)", ret, sizeof(ack));
        return -EBADMSG;
    }

    return ack.err.error;
}

// Add or Delete ipsec monitor 3X route
int ModifyIpSecMonRoute(uint16_t action, uint32_t table, const char* interface, const char* dstip)
{
    uint8_t AddrFamily;
    uint8_t PrefixLength;
    uint16_t flags;
    uint32_t ifindex;
    int AddrByteLength;
    uint8_t BinaryAddress[sizeof(in6_addr)];

    if (!dstip) {
        ALOGE("dstip is null");
        return -EFAULT;
    }

    AddrByteLength = XfrmPrefixParse(&AddrFamily, dstip, BinaryAddress, sizeof(BinaryAddress),
                                     &PrefixLength);

    if (AddrByteLength < 0) {
        ALOGE("XfrmPrefixParse fail, dstip = %s, err: (%s)",
              dstip, strerror(-AddrByteLength));
        return AddrByteLength;
    }

    if (static_cast<size_t>(AddrByteLength) > sizeof(BinaryAddress)) {
        ALOGE("addr length error: (AddrByteLength=%d) vs (BinaryAddress=%zu)",
              AddrByteLength, sizeof(BinaryAddress));
        return -ENOBUFS;
    }

    if (interface != XFRM_OIF_NONE) {
        ifindex = if_nametoindex(interface);
        if (!ifindex) {
            ALOGE("if_nametoindex fail: %s", interface);
            return -ENODEV;
        }
    }

    rtmsg route = {
        .rtm_protocol = RTPROT_STATIC,
        .rtm_type = RTN_UNICAST,
        .rtm_family = AddrFamily,
        .rtm_dst_len = PrefixLength,
        .rtm_scope = static_cast<uint8_t>(RT_SCOPE_LINK),
    };

    rtattr rtaDst     = { U16_RTA_LENGTH(AddrByteLength), RTA_DST };
    rtattr rtaGateway = { U16_RTA_LENGTH(AddrByteLength), RTA_GATEWAY };

    iovec iov[] = {
        { NULL,          0 },
        { &route,        sizeof(route) },
        { &RTATTR_TABLE, sizeof(RTATTR_TABLE) },
        { &table,        sizeof(table) },
        { &rtaDst,       sizeof(rtaDst) },
        { BinaryAddress,    static_cast<size_t>(AddrByteLength) },
        { &RTATTR_OIF,   interface != XFRM_OIF_NONE ? sizeof(RTATTR_OIF) : 0 },
        { &ifindex,      interface != XFRM_OIF_NONE ? sizeof(ifindex) : 0 },
    };

    if (action == RTM_NEWROUTE)
        flags = NETLINK_CREATE_REQUEST_FLAGS;
    else
        flags = NETLINK_REQUEST_FLAGS;

    int ret = XfrmSendNetlinkRequest(action, flags, iov, ARRAY_SIZE(iov), nullptr);
    if (ret) {
        ALOGE("Error %s route %s -> %s to table %u: %s",
              actionName(action), dstip, interface, table, strerror(-ret));
    } else {
        ALOGI("Success %s route %s -> %s to table %u",
              actionName(action), dstip, interface, table);
    }
    return ret;
}

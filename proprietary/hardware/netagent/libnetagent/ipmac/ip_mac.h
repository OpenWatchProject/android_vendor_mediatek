/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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
#include <string.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/if_link.h>
#include <linux/if_addr.h>
#include <linux/neighbour.h>
#include <asm/types.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <fcntl.h>
#include <dirent.h>
#include <time.h>

typedef int64_t nsecs_t;

enum {
	SYSTEM_TIME_REALTIME,
	SYSTEM_TIME_MONOTONIC,
	SYSTEM_TIME_PROCESS,
	SYSTEM_TIME_THREAD,
	SYSTEM_TIME_BOOTTIME
};

struct wlan0_hw_addr {
	unsigned char wlan0_mac_addr[6];
};

struct rtnl_handle {
	int fd;
	int seq;
	int dump;
	struct sockaddr_nl local;
};

#define INET_ADDRSTRLEN 16
#define INET6_ADDRSTRLEN 46

#define MACSIZE      6
#define IN6ADDR_ANY_INIT {{{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}}}

#define NOT_MATCH 2
#define TIME_EXCEED 998
#define NO_REPLY 999

int rtnl_open(struct rtnl_handle *rth);
void rtnl_close(struct rtnl_handle *rth);
static int Ipv4GetMacByIoctl(const char *ipStr, const char *ifname,
				struct sockaddr *WlanSocketAddr);
char *SocketAddr_ntop(const struct sockaddr *SockAddr, char *bufp);
static void WlanMacAddr_ntop(const struct wlan0_hw_addr *WlanMacAddr, char *buf);
static int SendNewNeighborMessage(const char *ipStr, int ifIndex, bool is_ipv6,
				struct sockaddr *WlanSocketAddr);
static int inet_get_addr(const char *ip_str, bool is_ipv6, struct in_addr *ip,
				struct in6_addr *ip6);
int addattr_l(struct nlmsghdr *n, int maxlen, int type, const void *data, int alen);
void parse_rtattr(struct rtattr **tb, int max, struct rtattr *rta, int len);
const char *addr_to_mac(unsigned char *addr, int alen, char *buf, ssize_t blen);
static int read_neighbor_nlmsg(int fd, const char *ipStr,
				struct sockaddr *WlanSocketAddr);
static int neigh_mac_process(struct nlmsghdr *nh, const char *ipStr,
				struct sockaddr *WlanSocketAddr);
static const inline nsecs_t seconds_to_nanoseconds(nsecs_t secs);
static const inline nsecs_t nanoseconds_to_milliseconds(nsecs_t secs);
static nsecs_t elapsedRealtime(int clock);
static int SendIpv6_DumpRequest(const char *ipStr, int ifIndex,
				struct sockaddr *WlanSocketAddr);
int Get_Mac_Addr(const char *ipStr, const char *ifname,
				struct sockaddr *WlanSocketAddr);

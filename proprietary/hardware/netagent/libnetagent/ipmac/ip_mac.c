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

#include <stdio.h>
#include <linux/errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <endian.h>
#include <time.h>
#include <netdb.h>
#include <linux/netlink.h>
#include <linux/if_link.h>
#include <linux/rtnetlink.h>
#include <linux/in.h>
#include <linux/in6.h>
#include <linux/if.h>
#include <net/if.h>
#include <arpa/inet.h>
#define LOG_TAG "ip_mac"
#include <log/log.h>
#include <cutils/properties.h>
#include <sys/resource.h>
#include <sched.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <asm/types.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <linux/netdevice.h>
#include <linux/if_arp.h>
#include <linux/sockios.h>
#include <linux/neighbour.h>
#include <mtk_log.h>
#include "ip_mac.h"

#define BIT_IP(x)    ((unsigned char*)&(x))
#define STR_IP(x)    BIT_IP(x)[0], BIT_IP(x)[1], BIT_IP(x)[2], BIT_IP(x)[3]
#define STR_IPH(x)   BIT_IP(x)[3], BIT_IP(x)[2], BIT_IP(x)[1], BIT_IP(x)[0]
#define FMT_IP        "%d.%d.%d.%d"
#define NUD_VALID   (NUD_PERMANENT | NUD_NOARP | NUD_REACHABLE | NUD_PROBE | NUD_STALE | NUD_DELAY)

#ifndef NDA_RTA
#define NDA_RTA(r)  ((struct rtattr*)(((char*)(r)) + NLMSG_ALIGN(sizeof(struct ndmsg))))
#endif

#ifndef NDA_PAYLOAD
#define NDA_PAYLOAD(n)    NLMSG_PAYLOAD(n,sizeof(struct ndmsg))
#endif

#undef  NLMSG_TAIL
#define NLMSG_TAIL(nmsg) \
	((struct rtattr *) (((char *) (nmsg)) + NLMSG_ALIGN((nmsg)->nlmsg_len)))

struct rtnl_handle rth;

int rtnl_open(struct rtnl_handle *rth)
{
	memset(rth, 0, sizeof(*rth));

	rth->fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	if (rth->fd < 0){
		ALOGE("can't open netlink socket, err = %s (%d)", strerror(errno), errno);
		return -1;
	}
	memset(&rth->local, 0, sizeof(rth->local));
	rth->local.nl_family = AF_NETLINK;
	rth->local.nl_pid = 0;
	rth->local.nl_groups = RTMGRP_NEIGH;
	if (bind(rth->fd, (struct sockaddr *)&rth->local, sizeof(rth->local)) < 0) {
		ALOGE("can't bind netlink socket, err = %s (%d)", strerror(errno), errno);
		return -1;
	}
	return 0;
}

void rtnl_close(struct rtnl_handle *rth)
{
	if (rth->fd >= 0) {
		close(rth->fd);
		rth->fd = -1;
	}
}

/* ipv4 get MAC addr by arp_ioctl
 * ipStr: ip address need to query
 * ifname: interface name
 * WlanSocketAddr: store MAC address
 */
static int Ipv4GetMacByIoctl(const char *ipStr, const char *ifname,
				struct sockaddr *WlanSocketAddr)
{
	struct arpreq arp_query;
	struct sockaddr_in *ipaddr;
	int sock_fd = 0;
	char buf[20] = {0};
	int ret = -ENXIO;

	sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock_fd < 0) {
		ALOGE("socket fail, err = %s (%d)", strerror(errno), errno);
		ret = -errno;
		goto ioctl_out;
	}

	memset(&arp_query, 0, sizeof(struct arpreq));
	ipaddr = (struct sockaddr_in *)&arp_query.arp_pa;
	ipaddr->sin_family = AF_INET;
	ipaddr->sin_addr.s_addr = inet_addr(ipStr);
	strlcpy(arp_query.arp_dev, ifname, IFNAMSIZ);	//must need add ifname

	if (!ioctl(sock_fd, SIOCGARP, &arp_query)) {
		if (arp_query.arp_flags & ATF_COM) { //NUD_STATE == NUD_VALID
			memcpy((char *)WlanSocketAddr, (char *)&(arp_query.arp_ha), sizeof(struct sockaddr));
			WlanSocketAddr->sa_family = AF_INET;
			ALOGI("ioctl success, ipStr = %s, MAC address = %s",
				ipStr, SocketAddr_ntop(WlanSocketAddr, buf));
			ret = 0;
		} else {
			ALOGI("ioctl success, but nud_state not NUD_VALID, trigger arp");
		}
		goto ioctl_out;
	} else {
		ret = -errno;
		if (ret == -ENXIO) {
			ALOGI("ioctl fail, no neighbour, trigger arp, ifname = %s, err = %s (%d)",
				ifname, strerror(errno), errno);
		} else {
			ALOGE("ioctl exception, ipStr = %s, ifname = %s, err = %s (%d)",
				ipStr, ifname, strerror(errno), errno);
		}
		goto ioctl_out;
	}

ioctl_out:
	close(sock_fd);
	return ret;
}

/* display wlan0 socket address in readable format */
char *SocketAddr_ntop(const struct sockaddr *SockAddr, char *bufp)
{
	WlanMacAddr_ntop((const struct wlan0_hw_addr *)SockAddr->sa_data, bufp);
	return bufp;
}

/* display wlan0 MAC address in readable format */
static void WlanMacAddr_ntop(const struct wlan0_hw_addr *WlanMacAddr, char *buf)
{
	sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X",
		WlanMacAddr->wlan0_mac_addr[0], WlanMacAddr->wlan0_mac_addr[1],
		WlanMacAddr->wlan0_mac_addr[2], WlanMacAddr->wlan0_mac_addr[3],
		WlanMacAddr->wlan0_mac_addr[4], WlanMacAddr->wlan0_mac_addr[5]);
}

static int SendIpv6_DumpRequest(const char *ipStr, int ifIndex, struct sockaddr *WlanSocketAddr)
{
	int ret = 0;
	struct {
		struct nlmsghdr hdr;
		struct ndmsg msg;
		char buf[256];
	} DumpReq;

	memset(&DumpReq, 0, sizeof(DumpReq));

	//send wlanx ipv6 nd_tb dump request
	DumpReq.hdr.nlmsg_len = NLMSG_LENGTH(sizeof(struct ndmsg));
	DumpReq.hdr.nlmsg_type = RTM_GETNEIGH;
	DumpReq.hdr.nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST;
	DumpReq.hdr.nlmsg_seq = rth.dump = ++rth.seq;
	DumpReq.hdr.nlmsg_pid = getpid();

	DumpReq.msg.ndm_family = AF_INET6;

	addattr_l(&DumpReq.hdr, sizeof(DumpReq), NDA_IFINDEX, &ifIndex, sizeof(ifIndex));

	if (rtnl_open(&rth) < 0){
		ret = -errno;
		goto dump_clean;
	}
	if (send(rth.fd, &DumpReq, DumpReq.hdr.nlmsg_len, 0) < 0) {
		ALOGE("SendIpv6_DumpRequest exception: err = %s (%d)", strerror(errno), errno);
		ret =  -errno;
		goto dump_clean;
	}

	ret = read_neighbor_nlmsg(rth.fd, ipStr, WlanSocketAddr);
	if (ret == -NOT_MATCH || ret == -NO_REPLY){
		ret = -ENXIO;
		goto dump_clean;
	}

dump_clean:
	rtnl_close(&rth);
	return ret;
}

/*
 * For AF_INET, If can't get MAC address by ioctl
 * create an RTM_NEWNEIGH message, used to trigger kernel send neigh solicit
 * ipStr: The ip address to query
 * ifIndex: interface index(wlan0 or wlan1)
 * is_ipv6: judge whether is ipv6 address
 * WlanSocketAddr: store MAC address
 */
static int SendNewNeighborMessage(const char *ipStr, int ifIndex,
	bool is_ipv6, struct sockaddr *WlanSocketAddr)
{
	struct in_addr ip_addr;
	struct in6_addr ip6_addr = IN6ADDR_ANY_INIT;
	int ret = 0;
	struct Request {
		struct nlmsghdr hdr;
		struct ndmsg msg;
		char buf[256];
	}NeighRequest;

	ip_addr.s_addr = 0;
	memset(WlanSocketAddr, 0, sizeof(struct sockaddr));
	memset(&NeighRequest, 0, sizeof(NeighRequest));

	//set up a request to creat a new neighbor
	NeighRequest.hdr.nlmsg_len = NLMSG_LENGTH(sizeof(NeighRequest.msg));
	NeighRequest.hdr.nlmsg_type = RTM_NEWNEIGH;
	NeighRequest.hdr.nlmsg_flags = NLM_F_REQUEST | NLM_F_REPLACE | NLM_F_CREATE;

	NeighRequest.msg.ndm_family = is_ipv6 ? AF_INET6 : AF_INET;
	NeighRequest.msg.ndm_flags = NTF_USE;
	NeighRequest.msg.ndm_ifindex = ifIndex;

	if (inet_get_addr(ipStr, is_ipv6, &ip_addr, &ip6_addr) <= 0) {
		ALOGE("invalid ip address: '%s', errno = %s", ipStr, strerror(errno));
		ret =  -errno;
		goto out;
	}

	//create the IP attribute
	if (!is_ipv6)
		addattr_l(&NeighRequest.hdr, sizeof(NeighRequest), NDA_DST, &ip_addr, sizeof(struct in_addr));
	else
		addattr_l(&NeighRequest.hdr, sizeof(NeighRequest), NDA_DST, &ip6_addr, sizeof(struct in6_addr));

	//create the MAC attribute
	addattr_l(&NeighRequest.hdr, sizeof(NeighRequest), NDA_LLADDR, WlanSocketAddr->sa_data, MACSIZE);

	if (send(rth.fd, &NeighRequest, NeighRequest.hdr.nlmsg_len, 0) < 0) {
		ALOGE("send neigh_solicit netlink fail: %s", strerror(errno));
		ret =  -errno;
		goto out;
	}

out:
	return ret;
}

/*
 * convert ip string to binary network byte order
 */
static int inet_get_addr(const char *ip_str, bool is_ipv6, struct in_addr *ip,
						struct in6_addr *ip6)
{
	if (is_ipv6)
		return inet_pton(AF_INET6, ip_str, ip6);
	else
		return inet_pton(AF_INET, ip_str, ip);
}

/*
 * Add a variable length attribute of type and with value data and alen length
 * to netlink message n, which is part of a buffer of length maxlen
 */
int addattr_l(struct nlmsghdr *n, int maxlen, int type, const void *data, int alen)
{
	int len = RTA_LENGTH(alen);
	struct rtattr *rta;

	if (NLMSG_ALIGN(n->nlmsg_len) + RTA_ALIGN(len) > maxlen) {
		ALOGD("addattr_l ERROR: message exceeded bound of %d, nlmsg_len:%d, len:%d\n",
				maxlen, NLMSG_ALIGN(n->nlmsg_len), RTA_ALIGN(len));
		return -1;
	}
	rta = NLMSG_TAIL(n);
	rta->rta_type = type;
	rta->rta_len = len;
	if (alen)
		memcpy(RTA_DATA(rta), data, alen);
	n->nlmsg_len = NLMSG_ALIGN(n->nlmsg_len) + RTA_ALIGN(len);

	return 0;
}

/*
 * decode rta, save into tb
 */
void parse_rtattr(struct rtattr **tb, int max, struct rtattr *rta, int len)
{
    memset(tb, 0, sizeof(struct rtattr *) * (max + 1));
    while (RTA_OK(rta, len)) {
        if ((rta->rta_type <= max) && (!tb[rta->rta_type]))
            tb[rta->rta_type] = rta;
        rta = RTA_NEXT(rta,len);
    }
    if (len)
        fprintf(stderr, "!!!Deficit %d, rta_len=%d\n", len, rta->rta_len);

    return;
}

const char *addr_to_mac(unsigned char *addr, int alen, char *buf, ssize_t blen)
{
    int i = 0;
    int l = 0;

	snprintf(buf+l, blen, "%02x", addr[i]);
	blen -= 2;
	l += 2;
    for (i = 1; i < alen; i++) {
		snprintf(buf+l, blen, ":%02x", addr[i]);
		blen -= 3;
		l += 3;
    }
    return buf;
}

static int read_neighbor_nlmsg(int fd, const char *ipStr,
				struct sockaddr *WlanSocketAddr)
{
    char buff[4 * 1024] = {0};
    struct nlmsghdr *nh;
    struct nlmsgerr *err;
    int read_size;
    int read_ret = 0;

    read_size = read(fd, buff, sizeof(buff));
    if (read_size < 0) {
		read_ret = errno;
		ALOGE("receive from netlink: %s", strerror(errno));
		goto done;
    }

    for (nh = (struct nlmsghdr*)buff; NLMSG_OK(nh, read_size);
			nh = NLMSG_NEXT(nh, read_size)) {
        switch (nh->nlmsg_type) {
        case RTM_NEWNEIGH:
			read_ret = neigh_mac_process(nh, ipStr, WlanSocketAddr);
			if ((read_ret == 0) || (read_ret == NO_REPLY))
				goto done;
			else
				break;
		case NLMSG_ERROR:
			err = (struct nlmsgerr *)NLMSG_DATA(nh);
			if (!err->error)
				continue;
			ALOGE("nlmsg error %s", strerror(-err->error));
			read_ret = err->error;
			goto done;
        case NLMSG_DONE:
            break;
        }
    }
	read_ret = NOT_MATCH;
done:
    return -read_ret;
}

/*
 * compatible with ipv4 and ipv6 family
 * we dont just want to listen for specific pid nlmsg
 */
static int neigh_mac_process(struct nlmsghdr *nh, const char *ipStr,
				struct sockaddr *WlanSocketAddr)
{
	struct ndmsg *ndmsg;
	int len = 0;
	struct rtattr *tb[NDA_MAX + 1];
	char buf[1024] = {0};

	ndmsg = NLMSG_DATA(nh);
	len = nh->nlmsg_len - NLMSG_SPACE(sizeof(struct ndmsg));
	parse_rtattr(tb, NDA_MAX, NDA_RTA(ndmsg), len);

	if (ndmsg->ndm_family & (AF_INET | AF_INET6)) {
		int addrlen = (ndmsg->ndm_family == AF_INET) ? INET_ADDRSTRLEN : INET6_ADDRSTRLEN;
		char listen_ip[addrlen];
		memset(listen_ip, 0, addrlen);

		if (tb[NDA_DST]) {
			inet_ntop(ndmsg->ndm_family, RTA_DATA(tb[NDA_DST]),
					listen_ip, sizeof(listen_ip));

			ALOGI("ndmsg->ndm_family = %d, src_ip: %s, listen_ip: %s, addrlen = %d",
			ndmsg->ndm_family, ipStr, listen_ip, addrlen);

			if(!strncmp(ipStr, listen_ip, strlen(ipStr))) {
				if (ndmsg->ndm_state & NUD_VALID) {
					if (tb[NDA_LLADDR]) {
						memcpy((char *)WlanSocketAddr->sa_data,
							(char *)RTA_DATA(tb[NDA_LLADDR]), MACSIZE);
						WlanSocketAddr->sa_family = ndmsg->ndm_family;
						ALOGI("listen ip: %s , MAC addr: %s , sa_family = %d, ndmsg->ndm_state = %d",
							listen_ip, SocketAddr_ntop(WlanSocketAddr, buf),
							WlanSocketAddr->sa_family, ndmsg->ndm_state);

						return 0;
					}
				} else if (ndmsg->ndm_state & NUD_FAILED) {
					ALOGE("3 times no arp/na, ndmsg->ndm_state = %d",
					ndmsg->ndm_state, ipStr, listen_ip);

					return NO_REPLY;
				} else {
					ALOGE("ip match, ndm_state not NUD_FAILED or NUD_VALID");
				}
			}
		}
	}
	return NOT_MATCH;
}

static const inline nsecs_t seconds_to_nanoseconds(nsecs_t secs)
{
	return secs*1000000000;
}

static const inline nsecs_t nanoseconds_to_milliseconds(nsecs_t secs)
{
	return secs/1000000;
}

/*
 * used to calculate elapsed real time
 */
static nsecs_t elapsedRealtime(int clock)
{
	static const clockid_t clocks[] = {
		CLOCK_REALTIME,
		CLOCK_MONOTONIC,
		CLOCK_PROCESS_CPUTIME_ID,
		CLOCK_THREAD_CPUTIME_ID,
		CLOCK_BOOTTIME
	};

	struct timespec ts;
	ts.tv_sec = ts.tv_nsec = 0;
	int err = clock_gettime(clocks[clock], &ts);
	if (err) {
		//This should never happed, but just in case ...
		ALOGE("clock_gettime(CLOCK_BOOTTIME) failed: %s", strerror(errno));
		return 0;
	}
	return seconds_to_nanoseconds(ts.tv_sec) + ts.tv_nsec;
}

/*
 * API
 * inquire mac address by ipStr and ifname
 * store ip type and MAC addr in WlanSocketAddr
 */
int Get_Mac_Addr(const char *ipStr, const char *ifname, struct sockaddr *WlanSocketAddr)
{
	bool is_ipv6;
	unsigned int wlan_index = 0;
	char buf[20];
	int maxfd = -1;
	int rc = 0;
	int ret = 0;
	fd_set read_fds;
	struct timeval timeout;

	if (!ipStr || !ifname || !WlanSocketAddr) {
		ALOGE("parameter err");
		ret = -EINVAL;
		goto out;
	}

	wlan_index = if_nametoindex(ifname);
	if (wlan_index == 0) {
        ALOGE("cannot find interface %s", ifname);
        ret = -RT_TABLE_UNSPEC;
        goto out;
    }

	is_ipv6 = !!strchr(ipStr, ':');
	if (!is_ipv6) {
		ret = Ipv4GetMacByIoctl(ipStr, ifname, WlanSocketAddr);
	} else {
		ret = SendIpv6_DumpRequest(ipStr, wlan_index, WlanSocketAddr);
	}

	if (!ret) {
		ALOGI("[Get_Mac_Addr] get success, ip = %s, MAC = %s",
			ipStr, SocketAddr_ntop(WlanSocketAddr, buf));
		goto out;
	} else if (ret == -ENXIO) { //trigger neighbor solicit
		rtnl_close(&rth);
		if (rtnl_open(&rth) < 0){
			ret = -errno;
			goto clean_up;
		}

		ALOGI("can't find neighbor, send arp/NS");
		if ((ret = SendNewNeighborMessage(ipStr, wlan_index, is_ipv6,
				WlanSocketAddr)) < 0) {
			ALOGE("failed to sendNewNeighborMessage, err = %s (%d)",
				strerror(errno), errno);
			goto clean_up;
		}

		nsecs_t start = elapsedRealtime(SYSTEM_TIME_BOOTTIME);//ns
		while(1) {
			nsecs_t now = elapsedRealtime(SYSTEM_TIME_BOOTTIME);//ns
			nsecs_t next = nanoseconds_to_milliseconds(now-start);//ms
			if (next > 3100) {
				ALOGE(" Get_Mac_Addr timer exceed %s", strerror(errno));
				ret =  -TIME_EXCEED;
				goto clean_up;
			}

			memset(&timeout, 0, sizeof(timeout));
			timeout.tv_sec = (3100 - next)/1000;
			timeout.tv_usec = ((3100 - next) % 1000) * 1000;

			FD_ZERO(&read_fds);
			FD_SET(rth.fd, &read_fds);
			if (rth.fd > maxfd)
				maxfd = rth.fd;

			if ((rc = select(maxfd + 1, &read_fds, NULL, NULL, &timeout)) < 0) {
				if (errno == EINTR)
					continue;
				ret = -errno;
				ALOGE("select failed (%s)", strerror(errno));
				goto clean_up;
			} else if (!rc) {
				ALOGE("select timer exceed, ret = -TIME_EXCEED");
				ret =  -TIME_EXCEED;
				goto clean_up;
			}

			if (FD_ISSET(rth.fd, &read_fds)) {
				ret = read_neighbor_nlmsg(rth.fd, ipStr, WlanSocketAddr);
				if (!ret || (ret == -NO_REPLY)) {
					/* "get success" or "no reply" return */
					goto clean_up;
				} else if (ret == -NOT_MATCH) {
					ALOGI("have not read required nlmsg");
					continue;
				} else {
					ALOGE("read_netlink_message err = %s (%d)", strerror(errno), errno);
					goto clean_up;
				}
			}
		}//while end
	} else {
		ALOGE("Get_Mac_Addr fail: err = %s (%d)", strerror(errno), errno);
		goto clean_up;
	}

out:
	return ret;

clean_up:
	rtnl_close(&rth);
	goto out;
}

/*
int main(int argc, char **argv)
{
	char *ip_str;
	bool is_rhost = 0;
	char buf[20];
	struct sockaddr *WlanSocketAddr1 = (struct sockaddr *)malloc(sizeof(struct sockaddr));
	//struct wlan_addr *WlanAddr1 = (struct wlan_addr *)malloc(sizeof(struct wlan_addr));
	if (*argv == NULL) {
		ALOGE("parameter err");
		return -1;
	}

	if (!(strcmp(argv[1], "-r")))
		is_rhost = 1;

	ip_str = argv[is_rhost ? 2 : 1];
	ALOGI("main ip_str = %s", ip_str);

	if (argc > 0) {
		if (Get_Mac_Addr(ip_str, "wlan0", WlanSocketAddr1) >= 0) {
			ALOGI("main get mac success, MAC address = %s", SocketAddr_ntop(WlanSocketAddr1, buf));
		}
	}
}
*/

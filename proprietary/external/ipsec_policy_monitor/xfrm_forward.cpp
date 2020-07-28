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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <ifaddrs.h>
#include <net/if.h>
#define LOG_TAG "ipsec_policy_mont"
#include <log/log.h>
#include <linux/xfrm.h>
#include "utils_xfrm.h"
#include "ipsec_multilayer.h"
#include "netutils_wrapper_interface.h"
#include "ifcutils/ifc.h"

#define SIOCFWDFILTER (SIOCDEVPRIVATE+2)
#define IPV4 0x40
#define IPV6 0x60

typedef struct {

    char        local_ip_c[256];
    char        target_ip_s[256];
    char        tunnel_ip_s[256];
    char        tunnel_ip_d[256];
    char        ims_ifname[IFNAMSIZ];
    short               used;
    __u8                pref_s;
    __u8                pref_d;
    __u8                cnt;
    __u16               family;
} ccmni_forward_ipsec_rule_struct;



typedef struct ccmni_fwd_filter {
	__u32 action;
	__u16 ver;
	__u8 s_pref;
	__u8 d_pref;
	union {
		struct {
			__u32 saddr;
			__u32 daddr;
		}ipv4;
		struct {
			__u32 saddr[4];
			__u32 daddr[4];
		}ipv6;
	}ip;
}ccmni_fwd_filter_t;

enum CCMNI_ACTION{
	CCMNI_FLT_ADD = 1,
	CCMNI_FLT_DEL,
	CCMNI_FLT_FLUSH
};


ccmni_forward_ipsec_rule_struct ForwardRule[RECORD_COUNT] = {{"\0","\0","\0","\0","\0", 0,0,0,0,0}};

int isIfaceName(const char *name) {
    size_t i;
    size_t name_len = strlen(name);
    if ((name_len == 0) || (name_len > IFNAMSIZ)) {
        return 0;
    }

    /* First character must be alphanumeric */
    if (!isalnum(name[0])) {
        return 0;
    }

    for (i = 1; i < name_len; i++) {
        if (!isalnum(name[i]) && (name[i] != '_') && (name[i] != '-') && (name[i] != ':')) {
            return 0;
        }
    }

    return 1;
}

//1 means matched, 0 means not match
int matchifbyaddr(struct sockaddr *sock_addr_a, char *addr_b, __u8 prefix_len, __u16 family)
{
	int ret;

	if(sock_addr_a == NULL || addr_b == NULL)
		return 0;
	if(sock_addr_a->sa_family != family)
		return 0;

	switch(sock_addr_a->sa_family) {
		case AF_INET: {
			struct sockaddr_in *sock_addr_in_a = (struct sockaddr_in *)sock_addr_a;
			struct in_addr sin_addr_b;
			inet_pton(AF_INET, addr_b, &sin_addr_b);
			ret = (sock_addr_in_a->sin_addr.s_addr - sin_addr_b.s_addr) ? 0 : 1;
			ALOGD("matchifbyaddr, family AF_INET, prefix_len %d, addr_a %x, addr_b %x\n", prefix_len, sock_addr_in_a->sin_addr.s_addr, sin_addr_b.s_addr);
			break;
			}
		case AF_INET6: {
			struct sockaddr_in6 *sock_addr_in6_a = (struct sockaddr_in6 *)sock_addr_a;
			struct in6_addr sin6_addr_b;
			inet_pton(AF_INET6, addr_b, &sin6_addr_b);
			ret = (sock_addr_in6_a->sin6_addr.s6_addr32[0] - sin6_addr_b.s6_addr32[0]) ||
				  (sock_addr_in6_a->sin6_addr.s6_addr32[1] - sin6_addr_b.s6_addr32[1]) ||
				  (sock_addr_in6_a->sin6_addr.s6_addr32[2] - sin6_addr_b.s6_addr32[2]) ||
				  (sock_addr_in6_a->sin6_addr.s6_addr32[3] - sin6_addr_b.s6_addr32[3]) ? 0 : 1;
			ALOGD("matchifbyaddr, family AF_INET6, prefix_len %d, addr_a %x_%x_%x_%x, addr_b %x_%x_%x_%x\n", prefix_len,
					sock_addr_in6_a->sin6_addr.s6_addr32[0], sock_addr_in6_a->sin6_addr.s6_addr32[1],
					sock_addr_in6_a->sin6_addr.s6_addr32[2], sock_addr_in6_a->sin6_addr.s6_addr32[3],
					sin6_addr_b.s6_addr32[0], sin6_addr_b.s6_addr32[1], sin6_addr_b.s6_addr32[2], sin6_addr_b.s6_addr32[3]);
			break;
			}
		default:
			return 0;
	}
	return ret;
}

//0 means get failed, 1 means get success
int getifbyaddr(char *ifname, char *addr, __u8 prefix_len, __u16 family)
{
	struct ifaddrs *ifa_list, *ifa_iter;
	ccmni_forward_ipsec_rule_struct * tmp = NULL;
	int i;
	#define IPV4_PREFIX_LEN 32
	#define IPV6_PREFIX_LEN 128

	if((family == AF_INET && prefix_len == IPV4_PREFIX_LEN) ||
		(family == AF_INET6 && prefix_len == IPV6_PREFIX_LEN)) {
		for(i = 0; i < RECORD_COUNT;i++) {
			tmp = ForwardRule + i;
			if(!strcmp(tmp->local_ip_c, addr)) {
				strncpy(ifname, tmp->ims_ifname, strlen(tmp->ims_ifname) + 1);
#ifdef INIT_ENG_BUILD
				ALOGD("getifbyaddr, ifname %s\n", ifname);
#endif
				return 1;
			}
		}
		if(getifaddrs(&ifa_list) < 0) {
			ALOGE("getifaddrs failed(%s), ifname will be hard code as ccmni4\n", strerror(errno));
			return 0;
		}
		for (ifa_iter = ifa_list; ifa_iter; ifa_iter = ifa_iter->ifa_next) {
			if(matchifbyaddr(ifa_iter->ifa_addr, addr, prefix_len, family)) {
				strncpy(ifname, ifa_iter->ifa_name, strlen(ifa_iter->ifa_name));
				ALOGD("getifbyaddr, ifname %s\n", ifname);
				freeifaddrs(ifa_list);
				return  1;
			}
		}
		freeifaddrs(ifa_list);
		ALOGE("matchifbyaddr failed\n");
		return 0;
	} else {
		ALOGE("family %u, prefix_len %u is not valid\n", family, prefix_len);
		return 0;
	}
}

static void ifr_config(char *src, __u8 pref_s, char *dst, __u8 pref_d, __u16 family, char *interface, struct ifreq *ifr)
{
	ccmni_fwd_filter_t *ifr_ccmni_fwd_filter_p;

	ifr_ccmni_fwd_filter_p = (ccmni_fwd_filter_t *)ifr->ifr_ifru.ifru_data;
	strncpy(ifr->ifr_name, interface, IFNAMSIZ);
	ifr->ifr_name[IFNAMSIZ - 1] = '\0';
	ifr_ccmni_fwd_filter_p->s_pref = pref_s;
	ifr_ccmni_fwd_filter_p->d_pref = pref_d;
	switch(family) {
		case AF_INET:
		ifr_ccmni_fwd_filter_p->ver = IPV4;
		rt_addr_aton(family, src, (char *)&ifr_ccmni_fwd_filter_p->ip.ipv4.saddr);
		rt_addr_aton(family, dst, (char *)&ifr_ccmni_fwd_filter_p->ip.ipv4.daddr);
		ALOGD("ifr_ccmni_fwd_filter, ipv4, name %s, ver %u, pref_s %u, pref_d %u, saddr %x, daddr %x\n",
			ifr->ifr_name, ifr_ccmni_fwd_filter_p->ver, ifr_ccmni_fwd_filter_p->s_pref, ifr_ccmni_fwd_filter_p->d_pref,
			ifr_ccmni_fwd_filter_p->ip.ipv4.saddr, ifr_ccmni_fwd_filter_p->ip.ipv4.daddr);
		break;
		case AF_INET6:
		ifr_ccmni_fwd_filter_p->ver = IPV6;
		rt_addr_aton(family, src, (char *)ifr_ccmni_fwd_filter_p->ip.ipv6.saddr);
		rt_addr_aton(family, dst, (char *)ifr_ccmni_fwd_filter_p->ip.ipv6.daddr);
		ALOGD("ifr_ccmni_fwd_filter, ipv6, name %s, ver %u, pref_s %u, pref_d %u, saddr %x_%x_%x_%x, daddr %x_%x_%x_%x\n",
			ifr->ifr_name, ifr_ccmni_fwd_filter_p->ver, ifr_ccmni_fwd_filter_p->s_pref, ifr_ccmni_fwd_filter_p->d_pref,
			ifr_ccmni_fwd_filter_p->ip.ipv6.saddr[0], ifr_ccmni_fwd_filter_p->ip.ipv6.saddr[1],
			ifr_ccmni_fwd_filter_p->ip.ipv6.saddr[2], ifr_ccmni_fwd_filter_p->ip.ipv6.saddr[3],
			ifr_ccmni_fwd_filter_p->ip.ipv6.daddr[0], ifr_ccmni_fwd_filter_p->ip.ipv6.daddr[1],
			ifr_ccmni_fwd_filter_p->ip.ipv6.daddr[2], ifr_ccmni_fwd_filter_p->ip.ipv6.daddr[3]);
		break;
		default:
		ALOGE("ifreq config failed due to unknown family type\n");
		break;
	}
}

//0~31 means the old rule, -1 means the new rule
static int forward_rule_isold(char *src, __u8 pref_s, char *dst, __u8 pref_d, __u16 family)
{
	ccmni_forward_ipsec_rule_struct * tmp = NULL;
	int new_rule;
	int i;

	for(i = 0; i < RECORD_COUNT;i++) {
		tmp = ForwardRule + i;
		if(tmp->used == 0)
			continue;
		 new_rule = (strncmp(tmp->local_ip_c, src, strlen(src))) || (strncmp(tmp->target_ip_s, dst, strlen(dst)))
					|| (tmp->pref_s != pref_s) || (tmp->pref_d!=pref_d) || (tmp->family!=family);
		if(!new_rule)
			return i;
	}

	ALOGI("src %s/%d, dst %s/%d, family %d is a new forward rule\n", src, pref_s, dst, pref_d, family);
	return -1;
}

static void forward_rule_add_old(int i, char *tunnel_src, char *tunnel_dst)
{
	ccmni_forward_ipsec_rule_struct * tmp = NULL;
	int ismobike = 0;

	tmp = ForwardRule + i;
	ismobike = strncmp(tunnel_src, tmp->tunnel_ip_s, strlen(tunnel_src)) || strncmp(tunnel_dst, tmp->tunnel_ip_d, strlen(tunnel_dst));
	if (ismobike) {
		strncpy(tmp->tunnel_ip_s, tunnel_src, strlen(tunnel_src) + 1);
		strncpy(tmp->tunnel_ip_d, tunnel_dst, strlen(tunnel_dst) + 1);
		tmp->cnt = 1;
	} else
		++tmp->cnt;
	ALOGI("forward rule src %s/%d dst %s/%d has been added %d times with tunnel from %s to %s\n",
			tmp->local_ip_c, tmp->pref_s, tmp->target_ip_s, tmp->pref_d, tmp->cnt, tunnel_src, tunnel_dst);
}

//1 means delete success, 0 means delete fail
static int forward_rule_del_old(int i, char *tunnel_dst)
{
	ccmni_forward_ipsec_rule_struct * tmp = NULL;

	tmp = ForwardRule + i;
	if(--tmp->cnt == 0) {
		strncpy(tunnel_dst, tmp->tunnel_ip_d, strlen(tmp->tunnel_ip_d) + 1);
		memset(&ForwardRule[i], 0, sizeof(ccmni_forward_ipsec_rule_struct));
		ALOGI("forward rule is completely deleted\n");
		return 1;
	}
	ALOGI("forward rule src %s/%d dst %s/%d left %d times before deleted\n", tmp->local_ip_c, tmp->pref_s, tmp->target_ip_s, tmp->pref_d, tmp->cnt);
	return 0;
}

//0~31 means add rule success, -1 means fail
int forward_rule_add_new(char *src, __u8 pref_s, char *dst, __u8 pref_d, char *tunnel_src, char *tunnel_dst, char *ims_ifname, __u16 family)
{
	ccmni_forward_ipsec_rule_struct * tmp = NULL;
	int i;

	for(i=0; i < RECORD_COUNT; i++) {
		tmp = ForwardRule + i;
		if(tmp->used == 0) {
			strncpy(tmp->local_ip_c, src, strlen(src) + 1);
			strncpy(tmp->target_ip_s, dst, strlen(dst) + 1);
			strncpy(tmp->tunnel_ip_s, tunnel_src, strlen(tunnel_src) + 1);
			strncpy(tmp->tunnel_ip_d, tunnel_dst, strlen(tunnel_dst) + 1);
			strncpy(tmp->ims_ifname, ims_ifname, strlen(ims_ifname) + 1);
			tmp->pref_s = pref_s;
			tmp->pref_d = pref_d;
			tmp->family = family;
			tmp->used = 1;
			tmp->cnt = 1;
			ALOGI("src %s/%d, dst %s/%d, family %d has been added to forward rule\n", src, pref_s, dst, pref_d, family);
			return i;
		}
	}

	ALOGE("forward rule has run out of memory\n");
	return -1;
}

static int forward_rule_count()
{
	ccmni_forward_ipsec_rule_struct * tmp = NULL;
	int cnt = 0;
	int i;

	for(i =0; i<RECORD_COUNT;i++) {
		tmp = ForwardRule + i;
		if(tmp->used == 1)
			cnt++;
	}
	return cnt;
}

/*mtk for ipsec MTU issue
static int exec_netutils_wrapper(const char *inIface, const char *outIface, const char *nxthop, const char *tableId, int family, int enable)
{
    int res = 0;
    const char *FORWARD_MARK = "0x10000";
    if (!isIfaceName(inIface) || !isIfaceName(outIface)) {
        return -1;
    }
    ALOGI("Setting IP forward enable = %d from %s to %s\n", enable, inIface, outIface);
    if(enable) {
        //enable forwarding
        res |= execNdcCmd("ipfwd", "enable", inIface, NULL);
        //add rorward mark
        res |= execIptables(V4V6, "-t", "mangle", "-I", LOCAL_MANGLE_PREROUTING, "-i", inIface, "-j", "MARK", "--set-mark", FORWARD_MARK, NULL);
        //add forward exception iptables
        res |= execIptables(V4V6, "-t", "filter", "-I", LOCAL_FILTER_FORWARD, "-i", inIface, "-o", outIface, "-j", "ACCEPT", NULL);
        //add powersave or dozable output exception iptables
        res |= execIptables(V4V6, "-t", "filter", "-I", LOCAL_FILTER_OUT, "-o", outIface, "-m", "mark", "--mark", FORWARD_MARK, "-j", "ACCEPT", NULL);
        //add powersave or dozable input exception iptables
        res |= execIptables(V4V6, "-t", "filter", "-I", LOCAL_FILTER_INPUT, "-i", outIface, "-j", "ACCEPT", NULL);
        //add forward route
        res |= execIpCmd(family, "route", "add", nxthop, "dev", outIface, "table", tableId, NULL);
    } else {
        //disable forwarding
        res |= execNdcCmd("ipfwd", "disable", inIface, NULL);
        //del forward mark
        res |= execIptables(V4V6, "-t", "mangle", "-D", LOCAL_MANGLE_PREROUTING, "-i", inIface, "-j", "MARK", "--set-mark", FORWARD_MARK, NULL);
        //del forward exception iptables
        res |= execIptables(V4V6, "-t", "filter", "-D", LOCAL_FILTER_FORWARD, "-i", inIface, "-o", outIface, "-j", "ACCEPT", NULL);
        //del powersave or dozable output exception iptables
        res |= execIptables(V4V6, "-t", "filter", "-D", LOCAL_FILTER_OUT, "-o", outIface, "-m", "mark", "--mark", FORWARD_MARK, "-j", "ACCEPT", NULL);
        //del powersave or dozable input exception iptables
        res |= execIptables(V4V6, "-t", "filter", "-D", LOCAL_FILTER_INPUT, "-i", outIface, "-j", "ACCEPT", NULL);
        //del forward route
        res |= forward_rule_count() ? 0 : execIpCmd(family, "route", "del", nxthop, "dev", outIface, "table", tableId, NULL);
    }
    return res;
}*/

int interfaceToTable;
static int get_table_from_interface(const char* interface) {
	#define ROUTE_TABLE_OFFSET_FROM_INDEX 1000
	uint32_t index = if_nametoindex(interface);
	if (index) {
		index += ROUTE_TABLE_OFFSET_FROM_INDEX;
		interfaceToTable = index;
		return index;
	} else
		return interfaceToTable;
}

void reset_interface_ipforward(const char *outIface)
{
	ccmni_forward_ipsec_rule_struct * tmp = NULL;
	char *nxthop;
	char *tableId;
	int i;

	if(!isIfaceName(outIface)) { //interface check
		ALOGE("invalid interface %s\n", outIface);
		return;
	}

	/* when interface valid, wfc enable and wlan0 down->up, resetting ip forward rule */
	for(i = 0; i < RECORD_COUNT; i++) {
		tmp = ForwardRule + i;
		if(tmp->used == 1) {
			int table = get_table_from_interface(outIface);
			asprintf(&nxthop, "%s/%u", tmp->target_ip_s, tmp->pref_d);
			asprintf(&tableId, "%u", table);
			IptablesInterface* handler = new IptablesInterface(tmp->ims_ifname, outIface, nxthop, tableId, tmp->local_ip_c,tmp->tunnel_ip_d, tmp->family, 0, IptablesAction::IPTABLES_RESET);
			(void)handler->start();
			free(nxthop);
			free(tableId);
			ALOGD("ip forward is resetting with %s-->%s\n", tmp->local_ip_c, tmp->target_ip_s);
		}
	}
}

static int writeToFile(const char* filename, const char* value) {
    int fd = open(filename, O_WRONLY | O_CLOEXEC);
    if (fd < 0) {
        ALOGE("Failed to open %s: %s", filename, strerror(errno));
        return -1;
    }

    const ssize_t len = strlen(value);
    if (write(fd, value, len) != len) {
        ALOGE("Failed to write %s to %s: %s", value, filename, strerror(errno));
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}

static int set_interface_ipforward(const char* inIface, const char* outIface, const char * src,const char* dst, __u8 prefLen, const char* tunnelDst, __u16 family, int status)
{
	char *nxthop;
	char *tableId;
	char *IPV4_ACCEPTLOCAL_PROC_FILE;
	int ret = 0;
	IptablesAction iptablesAction = (IptablesAction)status;

	int table = get_table_from_interface(outIface);
	asprintf(&nxthop, "%s/%u", dst, prefLen);
	asprintf(&tableId, "%u", table);
	IptablesInterface* handler = new IptablesInterface(inIface, outIface, nxthop, tableId, src, tunnelDst, family, forward_rule_count(), iptablesAction);
	ret |= handler->start();
	asprintf(&IPV4_ACCEPTLOCAL_PROC_FILE, "/proc/sys/net/ipv4/conf/%s/accept_local", inIface);
	ret |= writeToFile(IPV4_ACCEPTLOCAL_PROC_FILE, status ? "1" : "0");
	free(nxthop);
	free(tableId);
	free(IPV4_ACCEPTLOCAL_PROC_FILE);
	return ret;
}

static int set_interface_driver_forward(char *src, __u8 pref_s, char *dst,__u8 pref_d, __u16 family, char *interface, int status)
{
	struct ifreq ifr;
	ccmni_fwd_filter_t ifr_ccmni_fwd_filter;
	int ifc_ctl_sock;

	//set driver layer forwarding
	memset(&ifr, 0, sizeof(struct ifreq));
	ifr.ifr_ifru.ifru_data = &ifr_ccmni_fwd_filter;
	ifr_config(src, pref_s, dst, pref_d, family, interface, &ifr);
	ifc_ctl_sock = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if(ifc_ctl_sock < 0) {
		ALOGE("ifc_ctl_sock create fail, %s\n", strerror(errno));
		return -errno;
	}
	ifr_ccmni_fwd_filter.action = status ? CCMNI_FLT_ADD : CCMNI_FLT_DEL;
	if(ioctl(ifc_ctl_sock, SIOCFWDFILTER, &ifr) < 0) {
		ALOGE("driver forward ioctl %d failded, %s\n", SIOCFWDFILTER, strerror(errno));
		close(ifc_ctl_sock);
		return -errno;
	}
	ALOGD("ioctl: src:%s/%u,dst:%s/%u,family:%u from %s driver forward %s\n",src, pref_s, dst,pref_d, family, interface, status?"enable":"disable");
	close(ifc_ctl_sock);
	return 0;
}

int enable_interface_forward(char *src, __u8 pref_s, char *dst,__u8 pref_d, char *tunnel_src, char *tunnel_dst, __u16 family, char *interface)
{
	int channel;

	if(!isIfaceName(interface)) {
		ALOGE("set interface %s forward failed\n", interface);
		return -ENOENT;
	}

	//set ip layer forwarding
	channel = forward_rule_isold(src, pref_s, dst, pref_d, family);
	if(channel >= 0) {
#ifdef INIT_ENG_BUILD
		ALOGI("forward rule is old and ioctl has been configured\n");
#endif
		forward_rule_add_old(channel, tunnel_src, tunnel_dst);
		return 0;
	}

	if(forward_rule_add_new(src, pref_s, dst, pref_d, tunnel_src, tunnel_dst, interface, family) == -1) {
		ALOGE("add new forward record failed\n");
		return -1;
	}
	//stop tx queue
	ifc_set_txq_state(interface,0x0);
	if(set_interface_ipforward(interface, "wlan0", src, dst, pref_d, tunnel_dst, family, IPFORWARD_ON))
		ALOGD("ip forward enabled failed\n");
	if(set_interface_driver_forward(src, pref_s, dst, pref_d, family, interface, IPFORWARD_ON))
		ALOGD("enable driver forward failed\n");
	return 0;
}

int disable_interface_forward(char * src, __u8 pref_s, char * dst,__u8 pref_d, __u16 family, char * interface)
{
	int channel;
	char tunnel_dst[256] = { 0 };

	if(!isIfaceName(interface)) {
		ALOGE("set interface %s forward failed\n", interface);
		return -ENOENT;
	}

	//set ip layer forwarding
	channel = forward_rule_isold(src, pref_s, dst, pref_d, family);
	if(channel < 0) {
		ALOGI("forward rule did not be recorded and cannot be deleted\n");
		return 0;
	}
	if(forward_rule_del_old(channel, tunnel_dst) == 0) {
		ALOGI("forward rule has not been deleted actually\n");
		return 0;
	}
	if(set_interface_ipforward(interface, "wlan0", src, dst, pref_d,  tunnel_dst, family, IPFORWARD_OFF))
		ALOGD("ip forward disable failed\n");
	if(set_interface_driver_forward(src, pref_s, dst, pref_d, family, interface, IPFORWARD_OFF))
		ALOGD("disable driver forward failed\n");
	return 0;
}

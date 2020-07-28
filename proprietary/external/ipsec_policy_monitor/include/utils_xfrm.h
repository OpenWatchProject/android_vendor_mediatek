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

#ifndef __UTILS_H__
#define __UTILS_H__ 1
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
#include <linux/xfrm.h>

struct typeent {
	const char *t_name;
	int t_type;
};
typedef struct
{
	__u8 family;
	__u8 bytelen;
	__s16 bitlen;
	__u32 flags;
	__u32 data[8];
} inet_prefix;

/*
 * Receiving buffer defines:
 * nlmsg
 *   data = struct xfrm_userpolicy_info
 *   rtattr
 *     data = struct xfrm_user_tmpl[]
 */
#define NLMSG_BUF_SIZE 4096
#define RTA_BUF_SIZE 2048
#define XFRM_TMPLS_BUF_SIZE 1024
#undef NLMSG_TAIL
#define NLMSG_TAIL(nmsg) \
	((struct rtattr *) (((char *) (nmsg)) + NLMSG_ALIGN((nmsg)->nlmsg_len)))
#define XFRM_POLICY_ERROR 255
enum PROTOCOL_TYPE{
	PROTOCOL_ICMP=1,
	PROTOCOL_IPV4=4,
	PROTOCOL_TCP=6,
	PROTOCOL_UDP=17,
	PROTOCOL_IPV6=41,
	PROTOCOL_GRE=47,
	PROTOCOL_ESP=50,
	PROTOCOL_AH=51,
	PROTOCOL_ICMPV6=58,
	PROTOCOL_IPCOMP=108,
	PROTOCOL_L2TP=115
};
struct rtnl_handle_xfrm
{
	int			fd;
	struct sockaddr_nl	local;
	struct sockaddr_nl	peer;
	__u32			seq;
	__u32			dump;
};

struct req_handle_xfrm {
	struct nlmsghdr			n;
	struct xfrm_userpolicy_info	xpinfo;
	char				buf[RTA_BUF_SIZE];
	} ;

#define PREFIXLEN_SPECIFIED 1

static inline __u32 nl_mgrp_xfrm(__u32 group)
{
	if (group > 31 ) {
		//ALOGD("Use setsockopt for this group %d\n", group);
		return -1;
	}
	return group ? (1 << (group - 1)) : 0;
}

typedef int (*rtnl_filter_t_xfrm)(struct nlmsghdr *n);
extern int record_only;


#define XFRMP_RTA_XFRM(x)  ((struct rtattr*)(((char*)(x)) + NLMSG_ALIGN(sizeof(struct xfrm_userpolicy_info))))



#define XFRMPID_RTA_XFRM(x)  ((struct rtattr*)(((char*)(x)) + NLMSG_ALIGN(sizeof(struct xfrm_userpolicy_id))))

#define XFRMPEXP_RTA_XFRM(x)	((struct rtattr*)(((char*)(x)) + NLMSG_ALIGN(sizeof(struct xfrm_user_polexpire))))


extern void rt_addr_ntoa(int af,const void *addr, char *buf, int buflen);
extern void rt_addr_aton(int af, const char *srcaddr, void *dstbuf);

extern int xfrm_xfrmproto_getbyname(char *name);

extern int rtnl_listen_xfrm(struct rtnl_handle_xfrm *rtnl_xfrm, rtnl_filter_t_xfrm handler);
extern int parse_rtattr(struct rtattr *tb[], int max, struct rtattr *rta, int len);
extern int rtnl_open_byproto_xfrm(struct rtnl_handle_xfrm *rth_xfrm, unsigned subscriptions, int protocol);
extern void rtnl_close_xfrm(struct rtnl_handle_xfrm *rth_xfrm);
extern int xfrm_state_process( struct nlmsghdr *n);
extern int xfrm_policy_flush(struct nlmsghdr *n);
extern int xfrm_policy_process(struct nlmsghdr *n);
extern int xfrm_acquire_process(struct nlmsghdr *n);
extern int xfrm_policy_record(struct nlmsghdr *n);
extern int rtnl_talk(struct rtnl_handle_xfrm *rtnl, struct nlmsghdr *n, 
	      unsigned groups, struct nlmsghdr *answer);

extern int disable_interface_forward(char * src, __u8 pref_s, char * dst,__u8 pref_d, __u16 family, char * interface);
extern int enable_interface_forward(char * src, __u8 pref_s, char * dst,__u8 pref_d, char *tunnel_src, char *tunnel_dst, __u16 family, char * interface);
extern void reset_interface_ipforward(const char *outIface);
extern int getifbyaddr(char *ifname, char *addr, __u8 prefix_len, __u16 family);

extern int mask2bits(__u32 netmask_xfrm);
extern int get_netmask(unsigned *val_xfrm, const char *arg_xfrm, int base_xfrm);
extern int get_unsigned(unsigned *val_xfrm, const char *arg_xfrm, int base_xfrm);
extern int get_u32(__u32 *val_xfrm, const char *arg_xfrm, int base_xfrm);
extern int get_u8(__u8 *val_xfrm, const char *arg_xfrm, int base_xfrm);
extern int get_addr_ipv4(__u8 *ap_xfrm, const char *cp_xfrm);
extern int get_addr_1(inet_prefix *addr_xfrm, const char *name_xfrm, int family);
extern int get_prefix(inet_prefix *dst_xfrm, char *arg_xfrm, int family);

#endif /* __UTILS_H__ */

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
#include <linux/xfrm.h>
#include <linux/ipsec.h>
#include <linux/in.h>
#include <linux/in6.h>
#include <linux/if.h>
#include <net/if.h>
#include <arpa/inet.h>
#include "ipsec_multilayer.h"
#define LOG_TAG "ipsec_policy_mont"
#include <log/log.h>
#include "utils_xfrm.h"
#include <cutils/properties.h>
#include <sys/resource.h>
#include <sched.h>

struct rtnl_handle_xfrm rth = { -1,{0,0,0,0},{0,0,0,0},0,0 };
struct rtnl_handle_xfrm rth_link = { -1,{0,0,0,0},{0,0,0,0},0,0 };
int record_only = 1;


int xfrm_xfrmproto_is_ipsec(__u8 proto)
{
	return (proto ==  IPPROTO_ESP ||
		proto ==  IPPROTO_AH  ||
		proto ==  IPPROTO_COMP);
}

static const struct typeent xfrmproto_types[]= {
	{ "esp", IPPROTO_ESP }, { "ah", IPPROTO_AH }, { "comp", IPPROTO_COMP },
	{ "route2", IPPROTO_ROUTING }, { "hao", IPPROTO_DSTOPTS },
	{ "ipsec-any", IPSEC_PROTO_ANY },
	{ NULL, -1 }
};

int parse_rtattr(struct rtattr *tb[], int max, struct rtattr *rta, int len)
{
	memset(tb, 0, sizeof(struct rtattr *) * (max + 1));
	while (RTA_OK(rta, len)) {
		if ((rta->rta_type <= max) && (!tb[rta->rta_type]))
			tb[rta->rta_type] = rta;
		rta = RTA_NEXT(rta,len);
	}
	if (len)
		fprintf(stderr, "!!!Deficit %d, rta_len=%d\n", len, rta->rta_len);
	return 0;
}



static inline __u32 nl_mgrp(__u32 group)
{
	if (group > 31 ) {
		ALOGD("Use setsockopt for this group %d\n", group);
		return -1;
	}
	return group ? (1 << (group - 1)) : 0;
}
int rtnl_wilddump_request_xfrm(struct rtnl_handle_xfrm *rth_xfrm, int family, int type)
{
	struct {
		struct nlmsghdr nlh;
		struct rtgenmsg g;
		__u16 align_rta;	/* attribute has to be 32bit aligned */
		struct rtattr ext_req;
		__u32 ext_filter_mask;
	} req_xfrm;

	memset(&req_xfrm, 0, sizeof(req_xfrm));
	req_xfrm.nlh.nlmsg_len = sizeof(req_xfrm);
	req_xfrm.nlh.nlmsg_type = type;
	req_xfrm.nlh.nlmsg_flags = NLM_F_DUMP|NLM_F_REQUEST;
	req_xfrm.nlh.nlmsg_pid = getpid();
	req_xfrm.nlh.nlmsg_seq = rth_xfrm->dump = ++rth_xfrm->seq;
	req_xfrm.g.rtgen_family = family;

	req_xfrm.ext_req.rta_type = IFLA_EXT_MASK;
	req_xfrm.ext_req.rta_len = RTA_LENGTH(sizeof(__u32));
	req_xfrm.ext_filter_mask = RTEXT_FILTER_VF;

	return send(rth_xfrm->fd, (void*)&req_xfrm, sizeof(req_xfrm), 0);
}


int rcvbuf = 1024 * 1024;

void rtnl_close_xfrm(struct rtnl_handle_xfrm *rth_xfrm)
{
	if (rth_xfrm->fd >= 0) {
		close(rth_xfrm->fd);
		rth_xfrm->fd = -1;
	}
}

int rtnl_open_byproto_xfrm(struct rtnl_handle_xfrm *rth_xfrm, unsigned subscriptions,
		      int protocol)
{
	//socklen_t addr_len;
	//int sndbuf = 32768;

	memset(rth_xfrm, 0, sizeof(*rth_xfrm));

	rth_xfrm->fd = socket(AF_NETLINK, SOCK_RAW, protocol);
	if (rth_xfrm->fd < 0) {
		ALOGD("Cannot open netlink socket,errno:%d\n",errno);
		return -1;
	}


	memset(&rth_xfrm->local, 0, sizeof(rth_xfrm->local));
	rth_xfrm->local.nl_family = AF_NETLINK;
	rth_xfrm->local.nl_groups = subscriptions;

	if (bind(rth_xfrm->fd, (struct sockaddr*)&rth_xfrm->local, sizeof(rth_xfrm->local)) < 0) {
		ALOGD("Cannot bind netlink socket\n");
		return -1;
	}
	return 0;
}

int rtnl_listen_xfrm(struct rtnl_handle_xfrm *rtnl_xfrm, rtnl_filter_t_xfrm handler)
{
	int status;
	struct nlmsghdr *h_xfrm;
	struct sockaddr_nl nladdr_xfrm;
	struct iovec iov;
	struct msghdr msg_xfrm = {
		.msg_name = &nladdr_xfrm,
		.msg_namelen = sizeof(nladdr_xfrm),
		.msg_iov = &iov,
		.msg_iovlen = 1,
	};
	char   buf[16384];

	memset(&nladdr_xfrm, 0, sizeof(nladdr_xfrm));
	nladdr_xfrm.nl_family = AF_NETLINK;
	nladdr_xfrm.nl_pid = 0;
	nladdr_xfrm.nl_groups = 0;
	iov.iov_base = buf;



	while (1) {
		iov.iov_len = sizeof(buf);
		status = recvmsg(rtnl_xfrm->fd, &msg_xfrm, 0);

		if (status < 0) {
			if (errno == EINTR || errno == EAGAIN)
				continue;
			ALOGD("netlink receive error %s (%d)\n",strerror(errno), errno);
			if (errno == ENOBUFS)
				continue;
			return -1;
		}
		if (status == 0) {
			ALOGE("EOF on netlink\n");
			return -1;
		}
		if (msg_xfrm.msg_namelen != sizeof(nladdr_xfrm)) {
			ALOGE("Sender address length == %d\n", msg_xfrm.msg_namelen);
			return -1;
		}
		for (h_xfrm = (struct nlmsghdr*)buf; status >= sizeof(*h_xfrm); ) {
			int err;
			int len = h_xfrm->nlmsg_len;
			int l = len - sizeof(*h_xfrm);

			if (l<0 || len>status) {
				if (msg_xfrm.msg_flags & MSG_TRUNC) {
					ALOGE("Truncated message\n");
					return -1;
				}
				ALOGE("!!!malformed message: len=%d\n", len);
				return -1;
			}
			if(h_xfrm->nlmsg_pid != (unsigned int)getpid()) {
				record_only = 0;
				//ALOGD( "message from other process:%d\n",h->nlmsg_pid);
			} else
				record_only = 1;
			err = handler(h_xfrm);
			if (err < 0)
				return err;

			status -= NLMSG_ALIGN(len);
			h_xfrm = (struct nlmsghdr*)((char*)h_xfrm + NLMSG_ALIGN(len));
		}
		if (msg_xfrm.msg_flags & MSG_TRUNC) {
			ALOGD( "Message truncated\n");
			continue;
		}
		if (status) {
			ALOGE("!!!Remnant of size %d\n", status);
			return -1;
		}
		return status;
	}
}

int rtnl_talk(struct rtnl_handle_xfrm *rtnl, struct nlmsghdr *n,
	      unsigned groups, struct nlmsghdr *answer)
{
	int status;
	unsigned seq;
	//struct nlmsghdr *h;
	struct sockaddr_nl nladdr;
	struct iovec iov = {
		.iov_base = (void*) n,
		.iov_len = n->nlmsg_len
	};
	struct msghdr msg = {
		.msg_name = &nladdr,
		.msg_namelen = sizeof(nladdr),
		.msg_iov = &iov,
		.msg_iovlen = 1,
	};


	memset(&nladdr, 0, sizeof(nladdr));
	nladdr.nl_family = AF_NETLINK;
	nladdr.nl_pid = getpid();
	nladdr.nl_groups = groups;

	n->nlmsg_seq = seq = ++rtnl->seq;

	if (answer == NULL)
		n->nlmsg_flags |= NLM_F_ACK;

	status = sendmsg(rtnl->fd, &msg, 0);

	if (status < 0) {
		ALOGD("Cannot talk to rtnetlink");
		return -1;
	}
	ALOGD("rtnl_talk fd:%d,status:%d,n->nlmsg_len:%d,n->nlmsg_seq:%d,errno:%d",rtnl->fd,status,n->nlmsg_len,n->nlmsg_seq,errno);
	return 0;
}

static int check_netlink_length(const struct nlmsghdr *nh, size_t size) {
    if (nh->nlmsg_len < NLMSG_LENGTH(size)) {
        return 0;
    }
    return 1;
}

static void link_state_process(const struct nlmsghdr *nh) {
	#define LINKDOWN 0
	#define LINKUP 1
	int current_status;
	static int last_status = LINKDOWN;
	static int route_ready = 0;
	int type = nh->nlmsg_type;

	switch (type) {
		case RTM_NEWLINK:
		case RTM_DELLINK: {
			struct ifinfomsg *ifi = (struct ifinfomsg *) NLMSG_DATA(nh);
			if (!check_netlink_length(nh, sizeof(*ifi)))
				break;
			if ((ifi->ifi_flags & IFF_LOOPBACK) != 0)
				break;
			size_t len = IFLA_PAYLOAD(nh);
			struct rtattr *rta;
			for (rta = IFLA_RTA(ifi); RTA_OK(rta, len); rta = RTA_NEXT(rta, len)) {
				if (rta->rta_type == IFLA_IFNAME) {
					if (!strncmp(static_cast<char *>RTA_DATA(rta), "wlan0", 5)) { //make sure wlan0 down->up happens
						current_status = (ifi->ifi_flags & IFF_LOWER_UP) ? LINKUP: LINKDOWN;
						if (last_status == current_status)
							break;
						last_status = current_status;
						if (current_status == LINKUP) {
							route_ready = 1;
							break;
						} else
							break;
					}
				}
			}
			break;
		}
		case RTM_NEWROUTE: {
			struct rtmsg *rtm = (struct rtmsg *) NLMSG_DATA(nh);
			if (!check_netlink_length(nh, sizeof(*rtm)))
				break;
			if ((rtm->rtm_protocol == RTPROT_STATIC || rtm->rtm_protocol == RTPROT_RA) && route_ready) { //make sure static route ready to added
				size_t len = RTM_PAYLOAD(nh);
				struct rtattr *rta;
				for (rta = RTM_RTA(rtm); RTA_OK(rta, len); rta = RTA_NEXT(rta, len)) {
					if (rta->rta_type == RTA_OIF) {  //make sure wlan0 static route could be added
						char dev[IFNAMSIZ] = { 0 };
						if (if_indextoname(* (int *) RTA_DATA(rta), dev) && !strncmp(dev, "wlan0", 5)) {
							reset_interface_ipforward("wlan0");
							route_ready = 0;
							break;
						}
					}
				}
			}
			break;
		}
	}
}

static int link_accept_msg(struct nlmsghdr *n)
{
	switch (n->nlmsg_type) {
		case RTM_NEWLINK:
		case RTM_DELLINK:
		case RTM_NEWROUTE:
			link_state_process(n);
			return 0;
	}
/*
	if (n->nlmsg_type != NLMSG_ERROR && n->nlmsg_type != NLMSG_NOOP &&
	    n->nlmsg_type != NLMSG_DONE) {
		ALOGE("Unknown message: %08d 0x%08x 0x%08x\n",
			n->nlmsg_len, n->nlmsg_type, n->nlmsg_flags);
	}
*/
	return 0;
}

static int xfrm_accept_msg(struct nlmsghdr *n)
{


  //ALOGD("xfrm_accept_msg type:%d,fd:%d ",n->nlmsg_type,rth.fd);
	switch (n->nlmsg_type) {
	case XFRM_MSG_NEWSA:
	case XFRM_MSG_DELSA:
	case XFRM_MSG_UPDSA:
	case XFRM_MSG_EXPIRE:
	case XFRM_MSG_FLUSHSA:
		ALOGE("xfrm state: %08x %08x %08x\n", n->nlmsg_len, n->nlmsg_type, n->nlmsg_flags);
		return 0;
	case XFRM_MSG_NEWPOLICY:
	case XFRM_MSG_DELPOLICY:
	case XFRM_MSG_UPDPOLICY:
	case XFRM_MSG_POLEXPIRE:
		xfrm_policy_process( n);
		return 0;
	case XFRM_MSG_GETPOLICY:
		xfrm_policy_record( n);
		return 0;
	case XFRM_MSG_FLUSHPOLICY:

		xfrm_policy_flush( n);
		return 0;
	case XFRM_MSG_ACQUIRE:
		ALOGE("xfrm acquire msg");
	default:
		break;
	}
/*
	if (n->nlmsg_type != NLMSG_ERROR && n->nlmsg_type != NLMSG_NOOP &&
	    n->nlmsg_type != NLMSG_DONE) {
		ALOGE("Unknown message: %08d 0x%08x 0x%08x\n",
			n->nlmsg_len, n->nlmsg_type, n->nlmsg_flags);
	}
*/
	return 0;
}


int main(void)
{
	int ret_send = 0;
	char version[128] = {0};
	unsigned groups = ~((unsigned)0); /* XXX */
	//memset(&filter, 0, sizeof(filter));

	pid_t tid = gettid();
	int ret_prio = setpriority(0, tid, -20);
	ALOGD("threaid priority %d,result:%d\n",getpriority(0,gettid()),ret_prio);

	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);
	CPU_SET(3,&cpuset);
	sched_setaffinity(0,sizeof(cpu_set_t),&cpuset);

	rtnl_close_xfrm(&rth);
	rtnl_close_xfrm(&rth_link);
	groups |= (nl_mgrp(XFRMNLGRP_ACQUIRE)|nl_mgrp(XFRMNLGRP_EXPIRE)|nl_mgrp(XFRMNLGRP_SA)|nl_mgrp(XFRMNLGRP_POLICY));

	if (rtnl_open_byproto_xfrm(&rth, groups, NETLINK_XFRM) < 0)
		exit(1);
	if (rtnl_open_byproto_xfrm(&rth_link, RTMGRP_LINK | RTMGRP_IPV4_ROUTE | RTMGRP_IPV6_ROUTE, NETLINK_ROUTE) < 0)
		exit(1);
	int ret = property_set("vendor.net.ims.ipsec.version","2.0");
	if(ret != 0) {
		ALOGE("set property failed,errno:%d\n",errno);
		exit(1);
	} else {
		property_get("vendor.net.ims.ipsec.version",version,"");
		ALOGD("getproperty-- vendor.net.ims.ipsec.version :%s\n",version);
	}
	/*dump previous policy rules*/
	ret_send = rtnl_wilddump_request_xfrm(&rth, AF_UNSPEC, XFRM_MSG_GETPOLICY);
	if(ret_send < 0)
		ALOGE("send error,errno:%d\n",errno);
	else
		ALOGD("send success to get previous policy,ret_send:%d\n",ret_send);

	while(1) {
		int maxfd = -1;
		int rc = 0;
		fd_set read_fds;

		FD_ZERO(&read_fds);
		FD_SET(rth.fd, &read_fds);
		if (rth.fd > maxfd)
			maxfd = rth.fd;
		FD_SET(rth_link.fd, &read_fds);
		if (rth_link.fd > maxfd)
			maxfd = rth_link.fd;
		if ((rc = select(maxfd + 1, &read_fds, NULL, NULL, NULL)) < 0) {
			if (errno == EINTR)
				continue;
			ALOGE("select failed (%s)\n", strerror(errno));
			sleep(1);
			continue;
		} else if (!rc)
			continue;
		if (FD_ISSET(rth_link.fd, &read_fds))
			if (rtnl_listen_xfrm(&rth_link, link_accept_msg) < 0)
				exit(2);
		if (FD_ISSET(rth.fd, &read_fds))
			if (rtnl_listen_xfrm(&rth, xfrm_accept_msg) < 0)
				exit(2);
	}
}

#ifndef __UTILS_XFRM_H__
#define __UTILS_XFRM_H__ 1
#include <string.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/if_link.h>
#include <linux/if_addr.h>
#include <linux/neighbour.h>
#include <cutils/properties.h>
#include <asm/types.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <linux/xfrm.h>


//#define NLMSG_DELETEALL_BUF_SIZE (4096-512)
#define NLMSG_DELETEALL_BUF_SIZE 8192

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
#define CTX_BUF_SIZE 256
#define XFRM_ALGO_KEY_BUF_SIZE 512

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

typedef struct
{
	__u8 family;
	__u8 bytelen;
	__s16 bitlen;
	__u32 flags;
	__u32 data[8];
} inet_prefix;

#define PREFIXLEN_SPECIFIED 1

static inline __u32 nl_mgrp_xfrm(__u32 group)
{
	if (group > 31 ) {
		//ALOGD("Use setsockopt for this group %d\n", group);
		return -1;
	}
	return group ? (1 << (group - 1)) : 0;
}


extern int mask2bits(__u32 netmask_xfrm);
extern int get_netmask(unsigned *val_xfrm, const char *arg_xfrm, int base_xfrm);
extern int get_unsigned(unsigned *val_xfrm, const char *arg_xfrm, int base_xfrm);
extern int get_u32(__u32 *val_xfrm, const char *arg_xfrm, int base_xfrm);
extern int get_u8(__u8 *val_xfrm, const char *arg_xfrm, int base_xfrm);
extern int get_addr_ipv4(__u8 *ap_xfrm, const char *cp_xfrm);
extern int get_addr_1(inet_prefix *addr_xfrm, const char *name_xfrm, int family);
extern int get_prefix(inet_prefix *dst_xfrm, char *arg_xfrm, int family);



typedef int (*rtnl_filter_t_xfrm)(struct rtnl_handle_xfrm * rth,struct nlmsghdr *n);

extern int xfrm_policy_process_delete_exist( struct rtnl_handle_xfrm * rth,struct nlmsghdr *n, pid_t volte_pid);
extern int xfrm_state_process_delete_exist( struct rtnl_handle_xfrm * rth,struct nlmsghdr *n, pid_t volte_pid);

extern void rtnl_close_xfrm(struct rtnl_handle_xfrm *rth);
extern int rtnl_open_byproto_xfrm(struct rtnl_handle_xfrm *rth, unsigned subscriptions,
		      int protocol);
extern int rtnl_listen_xfrm(struct rtnl_handle_xfrm *rtnl, rtnl_filter_t_xfrm handler);
extern int rtnl_accept_msg_xfrm(struct rtnl_handle_xfrm * rth,struct nlmsghdr *n);
#endif /* __UTILS_XFRM_H__ */

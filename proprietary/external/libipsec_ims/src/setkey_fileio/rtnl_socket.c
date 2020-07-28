#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <fcntl.h>
#include <dirent.h>
#include <time.h>
#include "utils_xfrm.h"
#include "setkey_xfrm_parse.h"
#define LOG_TAG "setkey"
#include <log/log.h>



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

	iov.iov_len = sizeof(buf);
		status = recvmsg(rtnl_xfrm->fd, &msg_xfrm, 0);
		ALOGD("netlink receive msg status:%d\n",status);
		if (status < 0) {
			if (errno == EINTR || errno == EAGAIN || errno == ENOBUFS)
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
			err = handler(rtnl_xfrm,h_xfrm);
			if (err == -2) /*no sa & sp entries*/
			{
				return err;
			}

			status -= NLMSG_ALIGN(len);
			h_xfrm = (struct nlmsghdr*)((char*)h_xfrm + NLMSG_ALIGN(len));
		}

	return 0;
}

/*
  -2: no previous volte_stack policy&& state is set;
  -1: unknown message type && delete policy or state failed
   0:  everything is good
*/
int rtnl_accept_msg_xfrm(struct rtnl_handle_xfrm * rth ,struct nlmsghdr *n)
{
	char pid[128] = {0};
	property_get("vendor.net.ims.volte.pid",pid,"-1");
	pid_t volte_pid =atoi(pid);
	/*no previous volte_stack policy&& state is set*/
	switch (n->nlmsg_type) {
	case XFRM_MSG_NEWSA:
	case XFRM_MSG_UPDSA:
		return xfrm_state_process_delete_exist( rth,n, volte_pid);
	case XFRM_MSG_NEWPOLICY:
	case XFRM_MSG_UPDPOLICY:
		return xfrm_policy_process_delete_exist( rth,n, volte_pid);
	case XFRM_MSG_EXPIRE:
	case XFRM_MSG_DELSA:
	case XFRM_MSG_FLUSHSA:
	case XFRM_MSG_GETPOLICY:
	case XFRM_MSG_FLUSHPOLICY:
	case XFRM_MSG_ACQUIRE:
	case XFRM_MSG_DELPOLICY:
	case XFRM_MSG_POLEXPIRE:
	default:
		ALOGD("receive netlink message: %08d 0x%08x 0x%08x\n",
			n->nlmsg_len, n->nlmsg_type, n->nlmsg_flags);
		break;
	}

	if (n->nlmsg_type != NLMSG_ERROR && n->nlmsg_type != NLMSG_NOOP &&
	    n->nlmsg_type != NLMSG_DONE) {
		ALOGE("Unknown message: %08d 0x%08x 0x%08x\n",
			n->nlmsg_len, n->nlmsg_type, n->nlmsg_flags);
	}
	return -2;
}


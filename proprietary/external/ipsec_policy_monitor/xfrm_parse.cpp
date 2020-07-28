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
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <fcntl.h>
#include <dirent.h>
#include <time.h>
#include <linux/capability.h>
#include <linux/ipsec.h>
#include <linux/rtnetlink.h>
#include <linux/netlink.h>
#include <sys/capability.h>
#include <cutils/properties.h>
#include "utils_xfrm.h"
#include "ipsec_multilayer.h"
#define LOG_TAG "ipsec_policy_mont"
#include <log/log.h>



const struct typeent xfrmproto_types[]= {
	{ "esp", IPPROTO_ESP }, { "ah", IPPROTO_AH }, { "comp", IPPROTO_COMP },
	{ "route2", IPPROTO_ROUTING }, { "hao", IPPROTO_DSTOPTS },
	{ "ipsec-any", IPSEC_PROTO_ANY },
	{ NULL, 0 }
};


int xfrm_id_parse(xfrm_address_t *saddr_xfrm, struct xfrm_id *id, __u16 *family,
		 char * src,char * dst,char * ipsec_type)
{

	inet_prefix dst_prefix;
	inet_prefix src_prefix;

	memset(&dst_prefix, 0, sizeof(dst_prefix));
	memset(&src_prefix, 0, sizeof(src_prefix));

	get_prefix(&src_prefix, src, AF_UNSPEC);
	if (family)
		*family = src_prefix.family;
	memcpy(saddr_xfrm, &src_prefix.data, sizeof(*saddr_xfrm));
	get_prefix(&dst_prefix, dst, AF_UNSPEC);
	memcpy(&id->daddr, &dst_prefix.data, sizeof(id->daddr));

	int ret = xfrm_xfrmproto_getbyname(ipsec_type);
	if(ret<0)
	{
		ALOGD("xfrm_id_parse %s is wrong\n",ipsec_type);
		return -1;
	}
	id->proto = (__u8)ret;


	return 0;
}

void xfrm_encry_algo_parse(char * encry_src, char *name)
{
	if(encry_src == NULL)
		memcpy(name,"ecb(cipher_null)",strlen("ecb(cipher_null)"));
	else 	if(strcmp(encry_src,"des-cbc")==0)
		memcpy(name,"cbc(des)",strlen("cbc(des)"));
	else 	if(strcmp(encry_src,"des-ede3-cbc")==0)
		memcpy(name,"cbc(des3_ede)",strlen("cbc(des3_ede)"));
	else 	if(strcmp(encry_src,"3des-cbc")==0)
		memcpy(name,"cbc(des3_ede)",strlen("cbc(des3_ede)"));
	else 	if(strcmp(encry_src,"cast5-cbc")==0)
		memcpy(name,"cbc(cast5)",strlen("cbc(cast5)"));
	else 	if(strcmp(encry_src,"blowfish-cbc")==0)
		memcpy(name,"cbc(blowfish)",strlen("cbc(blowfish)"));
	else 	if(strcmp(encry_src,"aes-cbc")==0)
		memcpy(name,"cbc(aes)",strlen("cbc(aes)"));
	else 	if(strcmp(encry_src,"serpent-cbc")==0)
		memcpy(name,"cbc(serpent)",strlen("cbc(serpent)"));
	else 	if(strcmp(encry_src,"camellia-cbc")==0)
		memcpy(name,"cbc(camellia)",strlen("cbc(camellia)"));
	else 	if(strcmp(encry_src,"twofish-cbc")==0)
		memcpy(name,"cbc(twofish)",strlen("cbc(twofish)"));
	else 	if(strcmp(encry_src,"aes-ctr-rfc3686")==0)
		memcpy(name,"rfc3686(ctr(aes))",strlen("rfc3686(ctr(aes))"));
	else    if(strcmp(encry_src,"null")==0)
		memcpy(name,"ecb(cipher_null)",strlen("ecb(cipher_null)"));
	else
	{
		memcpy(name,"not-supported",strlen("not-supported"));	
		ALOGD("xfrm_encry_algo_parse not supported algorithm--%s\n",encry_src);
	}
}

void xfrm_interg_algo_parse(char * interg_src, char *name)
{
	if(interg_src == NULL)
		memcpy(name,"digest_null",strlen("digest_null"));
	else 	if(strcmp(interg_src,"hmac-md5")==0)
		memcpy(name,"hmac(md5)",strlen("hmac(md5)"));
	else 	if(strcmp(interg_src,"hmac-sha1")==0)
		memcpy(name,"hmac(sha1)",strlen("hmac(sha1)"));
	else 	if(strcmp(interg_src,"hmac-sha256")==0)
		memcpy(name,"hmac(sha256)",strlen("hmac(sha256)"));
	else 	if(strcmp(interg_src,"hmac-sha384)")==0)
		memcpy(name,"hmac(sha384)",strlen("hmac(sha384)"));	
	else 	if(strcmp(interg_src,"hmac-sha512")==0)
		memcpy(name,"hmac(sha512)",strlen("hmac(sha512)"));
	else 	if(strcmp(interg_src,"hmac-rmd160")==0)
		memcpy(name,"hmac(rmd160)",strlen("hmac(rmd160)"));
	else 	if(strcmp(interg_src,"aes-xcbc")==0)
		memcpy(name,"xcbc(aes)",strlen("xcbc(aes)"));
	else 	if(strcmp(interg_src,"cmac(aes)")==0)
		memcpy(name,"aes-cmac",strlen("aes-cmac"));
	else 	if(strcmp(interg_src,"null")==0)
		memcpy(name,"digest_null",strlen("digest_null"));
	else
	{	
		memcpy(name,"not-supported",strlen("not-supported"));
		ALOGD("xfrm_interg_algo_parse not supported algorithm--%s\n",interg_src);
	}
}

__u8 xfrm_dir_parse(char * dir_str)
{
	__u8 dir;
	if(strcmp(dir_str,"out")==0)
		dir = XFRM_POLICY_OUT;
	else if(strcmp(dir_str,"in")==0)
		dir = XFRM_POLICY_IN;
	else if(strcmp(dir_str,"fwd")==0)
		dir = XFRM_POLICY_FWD;
	else
		dir = XFRM_POLICY_ERROR;
	return dir;
}

int xfrm_mode_parse(__u8 *mode, char * mode_str)
{


	if (strcmp(mode_str, "transport") == 0)
		*mode = XFRM_MODE_TRANSPORT;
	else if (strcmp(mode_str, "tunnel") == 0)
		*mode = XFRM_MODE_TUNNEL;
	else if (strcmp(mode_str, "ro") == 0)
		*mode = XFRM_MODE_ROUTEOPTIMIZATION;
	else if (strcmp(mode_str, "in_trigger") == 0)
		*mode = XFRM_MODE_IN_TRIGGER;
	else if (strcmp(mode_str, "beet") == 0)
		*mode = XFRM_MODE_BEET;
	else
		return -1;


	return 0;
}

void xfrm_selector_parse(struct xfrm_selector *sel, char * src,char * dst,enum PROTOCOL_TYPE protocol,char * src_port,char * dst_port)
{

	inet_prefix dst_prefix;
	inet_prefix src_prefix;

	memset(&dst_prefix, 0, sizeof(dst_prefix));
	memset(&src_prefix, 0, sizeof(src_prefix));


	get_prefix(&src_prefix, src, AF_UNSPEC);
	memcpy(&sel->saddr, &src_prefix.data, sizeof(sel->saddr));
	sel->prefixlen_s = src_prefix.bitlen;

	get_prefix(&dst_prefix, dst, AF_UNSPEC);
	memcpy(&sel->daddr, &dst_prefix.data, sizeof(sel->daddr));
	sel->prefixlen_d = dst_prefix.bitlen;

	sel->family = dst_prefix.family;

	sel->sport = htons(atoi(src_port));
	sel->dport = htons(atoi(dst_port));
	sel->dport_mask = ~((__u16)0);
	sel->sport_mask = ~((__u16)0);

	sel->user = getpid();

	sel->proto = protocol;
#ifdef INIT_ENG_BUILD
	if(sel->family == AF_INET)
	ALOGD("xfrm_selector_parse family:%u,prefix_d:%u,prefix_s:%u,daddr:0x%x,saddr:0x%x,sel->sport:%d,sel->dport:%d,proto:%u,user:%u\n",sel->family,sel->prefixlen_d,sel->prefixlen_s,sel->daddr.a4,sel->daddr.a4,sel->sport,sel->dport,sel->proto,sel->user);
	else
	ALOGD("xfrm_selector_parse family:%u,prefix_d:%u,prefix_s:%u,daddr:0x%x %x %x %x,saddr:0x%x %x %x %x ,sel->sport:%d,sel->dport:%d,proto:%u,user:%u\n",sel->family,sel->prefixlen_d,sel->prefixlen_s,sel->daddr.a6[0],sel->daddr.a6[1],sel->daddr.a6[2],sel->daddr.a6[3],sel->saddr.a6[0],sel->saddr.a6[1],sel->saddr.a6[2],sel->saddr.a6[3],sel->sport,sel->dport,sel->proto,sel->user);
#endif
}

static int addattr_l(struct nlmsghdr *n, int maxlen, int type, const void *data,
	      int alen)
{
	int len = RTA_LENGTH(alen);
	struct rtattr *rta = NULL;

	if (NLMSG_ALIGN(n->nlmsg_len) + RTA_ALIGN(len) > maxlen) {
		ALOGD( "addattr_l ERROR: message exceeded bound of %d\n",maxlen);
		return -1;
	}

	rta = NLMSG_TAIL(n);
	rta->rta_type = type;
	rta->rta_len = len;
	memcpy(RTA_DATA(rta), data, alen);
	n->nlmsg_len = NLMSG_ALIGN(n->nlmsg_len) + RTA_ALIGN(len);
	return 0;
}

int setkey_SP_xfrm(int cmd,char * src_range,char * dst_range,enum PROTOCOL_TYPE protocol,char * port_src,char * port_dst,char * src_tunnel,char * dst_tunnel,char * ipsec_type,char * mode, char * direction,int u_id)
{
	char src_arr[128] = {0};
	char dst_arr[128] = {0};
	char src_tunnel_arr[128] = {0};
	char dst_tunnel_arr[128] = {0};
	unsigned groups = ~((unsigned)0); /* XXX */
	struct rtnl_handle_xfrm rth = { -1,{0},{0},0,0 };

	if(src_range)
		strncpy(src_arr,src_range,strlen(src_range) + 1);
	if(dst_range)
		strncpy(dst_arr,dst_range,strlen(dst_range) + 1);

	if(src_tunnel)
		strncpy(src_tunnel_arr,src_tunnel,strlen(src_tunnel) + 1);
	if(dst_tunnel)
		strncpy(dst_tunnel_arr,dst_tunnel,strlen(dst_tunnel) + 1);

	struct req_handle_xfrm req;
	char tmpls_buf[XFRM_TMPLS_BUF_SIZE];
	struct xfrm_user_tmpl *tmpl = NULL;
	int tmpls_len = 0;

	memset(&req, 0, sizeof(req));
	memset(&tmpls_buf, 0, sizeof(tmpls_buf));

	req.n.nlmsg_len = NLMSG_LENGTH(sizeof(req.xpinfo));
	req.n.nlmsg_flags = NLM_F_REQUEST;
	req.n.nlmsg_type = cmd;

	req.xpinfo.lft.soft_byte_limit = XFRM_INF;
	req.xpinfo.lft.hard_byte_limit = XFRM_INF;
	req.xpinfo.lft.soft_packet_limit = XFRM_INF;
	req.xpinfo.lft.hard_packet_limit = XFRM_INF;
	req.xpinfo.dir = xfrm_dir_parse(direction);
	req.xpinfo.priority = 1000;
	xfrm_selector_parse(&req.xpinfo.sel, src_arr,dst_arr,protocol,port_src,port_dst);

	if(req.xpinfo.dir == XFRM_POLICY_ERROR)
	{
		ALOGD("setkey_SP_xfrm dir:%s is wrong",direction);
		return -1;
	}
	tmpl = (struct xfrm_user_tmpl *)((char *)tmpls_buf + tmpls_len);

	if(xfrm_mode_parse(&tmpl->mode, mode) ==-1)
	{
		ALOGD("setkey_SP_xfrm mode:%s is wrong",mode);
		return -1;
	}
	tmpl->family = req.xpinfo.sel.family;
	if(tmpl->mode == XFRM_MODE_TUNNEL)
	{
		xfrm_id_parse(&tmpl->saddr, &tmpl->id, &tmpl->family,
				      src_tunnel_arr,dst_tunnel_arr,ipsec_type);
	}

	tmpl->aalgos = (~(__u32)0);
	tmpl->ealgos = (~(__u32)0);
	tmpl->calgos = (~(__u32)0);
	tmpl->reqid = u_id;
	tmpl->id.proto  = xfrm_xfrmproto_getbyname(ipsec_type);
	if(!tmpl->id.proto)
	{
		ALOGD("setkey_SP_xfrm ipsec_type:%s is wrong",ipsec_type);
		return -1;
	}
	tmpls_len += sizeof(*tmpl);
	if (tmpls_len > 0) {
		addattr_l(&req.n, sizeof(req), XFRMA_TMPL,
			  (void *)tmpls_buf, tmpls_len);
	}

	groups |= (nl_mgrp_xfrm(XFRMNLGRP_ACQUIRE)|nl_mgrp_xfrm(XFRMNLGRP_EXPIRE)|nl_mgrp_xfrm(XFRMNLGRP_SA)|nl_mgrp_xfrm(XFRMNLGRP_POLICY));

	if (rtnl_open_byproto_xfrm(&rth, groups, NETLINK_XFRM) < 0)
	{
		rtnl_close_xfrm(&rth);
		return -1;
	}

	if(send(rth.fd, (void*)&req, sizeof(req), 0)<0)
	{
		ALOGD("set2layeripsecrules_xfrm send failed,errno:%d",errno);
		rtnl_close_xfrm(&rth);
		return -1;
	}
	if(req.xpinfo.dir ==XFRM_POLICY_IN)
	{
		req.xpinfo.dir = XFRM_POLICY_FWD;
		if(send(rth.fd, (void*)&req, sizeof(req), 0)<0)
		{
			ALOGD("set2layeripsecrules_xfrm send POLICY_FWD failed,errno:%d",errno);
			rtnl_close_xfrm(&rth);
			return -1;
		}	
	}
#ifdef INIT_ENG_BUILD
	ALOGD("setkey_SP_xfrm successed fd:%d --spdadd %s[%s] %s[%s] %d -P %s ipsec %s/%s//unique:%d;\n",rth.fd,src_range,port_src,dst_range,port_dst,protocol,direction,ipsec_type,mode,u_id);
#endif
	rtnl_close_xfrm(&rth);
	return 0;	
}

int setkey_SP_2layer_xfrm(int cmd,char * src_range,char * dst_range,enum PROTOCOL_TYPE protocol,char * port_src,char * port_dst,char * src_tunnel,char * dst_tunnel,char * ipsec_type1,char * mode1, char * ipsec_type2,char * mode2,char * direction,int u_id1,int u_id2)
{
	char src_arr[128] = {0};
	char dst_arr[128] = {0};
	char src_tunnel_arr[128] = {0};
	char dst_tunnel_arr[128] = {0};
	unsigned groups = ~((unsigned)0); /* XXX */
	struct rtnl_handle_xfrm rth = { -1,{0},{0},0,0 };

	if(src_range)
		strncpy(src_arr,src_range,strlen(src_range) + 1);
	if(dst_range)
		strncpy(dst_arr,dst_range,strlen(dst_range) + 1);
	if(src_tunnel)
		strncpy(src_tunnel_arr,src_tunnel,strlen(src_tunnel) + 1);
	if(dst_tunnel)
		strncpy(dst_tunnel_arr,dst_tunnel,strlen(dst_tunnel) + 1);


	struct req_handle_xfrm req;
	char tmpls_buf[XFRM_TMPLS_BUF_SIZE];
	struct xfrm_user_tmpl *tmpl = NULL;
	int tmpls_len = 0;


	memset(&req, 0, sizeof(req));
	memset(&tmpls_buf, 0, sizeof(tmpls_buf));

	req.n.nlmsg_len = NLMSG_LENGTH(sizeof(req.xpinfo));
	req.n.nlmsg_flags = NLM_F_REQUEST;
	req.n.nlmsg_type = cmd;

	req.xpinfo.lft.soft_byte_limit = XFRM_INF;
	req.xpinfo.lft.hard_byte_limit = XFRM_INF;
	req.xpinfo.lft.soft_packet_limit = XFRM_INF;
	req.xpinfo.lft.hard_packet_limit = XFRM_INF;
	req.xpinfo.dir = xfrm_dir_parse(direction);
	req.xpinfo.priority = 1000;
	xfrm_selector_parse(&req.xpinfo.sel, src_arr,dst_arr,protocol,port_src,port_dst);

	if(req.xpinfo.dir == XFRM_POLICY_ERROR)
	{
		ALOGD("setkey_SP_xfrm dir:%s is wrong",direction);
		return -1;
	}
	tmpl = (struct xfrm_user_tmpl *)((char *)tmpls_buf + tmpls_len);

	if(xfrm_mode_parse(&tmpl->mode, mode1) ==-1)
	{
		ALOGD("setkey_SP_xfrm mode:%s is wrong",mode1);
		return -1;
	}
	tmpl->family = req.xpinfo.sel.family;
	if(tmpl->mode == XFRM_MODE_TUNNEL)
	{
		xfrm_id_parse(&tmpl->saddr, &tmpl->id, &tmpl->family,
				      src_tunnel_arr,dst_tunnel_arr,ipsec_type1);
	}

	tmpl->aalgos = (~(__u32)0);
	tmpl->ealgos = (~(__u32)0);
	tmpl->calgos = (~(__u32)0);
	tmpl->reqid = u_id1;
	tmpl->id.proto  = xfrm_xfrmproto_getbyname(ipsec_type1);
	if(!tmpl->id.proto)
	{
		ALOGD("setkey_SP_xfrm ipsec_type:%s is wrong",ipsec_type1);
		return -1;
	}
	tmpls_len += sizeof(*tmpl);
	tmpl = (struct xfrm_user_tmpl *)((char *)tmpls_buf + tmpls_len);

	if(xfrm_mode_parse(&tmpl->mode, mode2) ==-1)
	{
		ALOGD("setkey_SP_xfrm mode:%s is wrong",mode1);
		return -1;
	}
	tmpl->family = req.xpinfo.sel.family;
	if(tmpl->mode == XFRM_MODE_TUNNEL)
	{
		xfrm_id_parse(&tmpl->saddr, &tmpl->id, &tmpl->family,
				      src_tunnel_arr,dst_tunnel_arr,ipsec_type2);
	}
	tmpl->aalgos = (~(__u32)0);
	tmpl->ealgos = (~(__u32)0);
	tmpl->calgos = (~(__u32)0);
	tmpl->reqid = u_id2;
	tmpl->id.proto  = xfrm_xfrmproto_getbyname(ipsec_type2);
	if(!tmpl->id.proto)
	{
		ALOGD("setkey_SP_xfrm ipsec_type:%s is wrong",ipsec_type2);
		return -1;
	}
	tmpls_len += sizeof(*tmpl);
	if (tmpls_len > 0) {
		addattr_l(&req.n, sizeof(req), XFRMA_TMPL,
			  (void *)tmpls_buf, tmpls_len);
	}

	groups |= (nl_mgrp_xfrm(XFRMNLGRP_ACQUIRE)|nl_mgrp_xfrm(XFRMNLGRP_EXPIRE)|nl_mgrp_xfrm(XFRMNLGRP_SA)|nl_mgrp_xfrm(XFRMNLGRP_POLICY));

	if (rtnl_open_byproto_xfrm(&rth, groups, NETLINK_XFRM) < 0)
	{
		rtnl_close_xfrm(&rth);
		return -1;
	}

	if(send(rth.fd, (void*)&req, sizeof(req), 0)<0)
	{
		ALOGD("set2layeripsecrules_xfrm send failed,errno:%d",errno);
		rtnl_close_xfrm(&rth);
		return -1;
	}
	if(req.xpinfo.dir ==XFRM_POLICY_IN)
	{
		req.xpinfo.dir = XFRM_POLICY_FWD;
		if(send(rth.fd, (void*)&req, sizeof(req), 0)<0)
		{
			ALOGD("set2layeripsecrules_xfrm send POLICY_FWD failed,errno:%d",errno);
			rtnl_close_xfrm(&rth);
			return -1;
		}	
	}
#ifdef INIT_ENG_BUILD
	ALOGD("setkey_SP_2layer_xfrm successed fd:%d --spdupdate %s[%s] %s[%s] %d -P %s prio 1000 ipsec %s/%s//unique:%d %s/%s/%s-%s/unique:%d;\n",rth.fd,src_range,port_src,dst_range,port_dst,protocol,direction,ipsec_type1,mode1,u_id1,ipsec_type2,mode2,src_tunnel,dst_tunnel,u_id2);
#endif
	rtnl_close_xfrm(&rth);
	return 0;
}

/*update one SP of one direction, just for transport mode*/
/*protocol:tcp icmp udp icmp6 ip4 gre
  direction:src->dst*/
int setkey_SP_update_transport(char * src_range,char * dst_range,enum PROTOCOL_TYPE protocol,char * port_src,char * port_dst,char * ipsec_type, char * direction,int u_id)
{
	int ret = setkey_SP_xfrm(XFRM_MSG_UPDPOLICY,src_range,dst_range,protocol,port_src,port_dst,NULL,NULL,ipsec_type,"transport",direction,u_id);
	return ret;
}

/*update one SP of one direction, for 2 layers' ipsec--tunnel mode+transport mode or transport mode+tunnel mode*/
/*protocol:tcp icmp udp icmp6 ip4 gre
  direction:src->dst
src_tunnel,dst_tunnel: tunnel src ip tunnel dst ip */
int setkey_SP_update_tunnel_transport(char * src_range,char * dst_range,enum PROTOCOL_TYPE protocol,char * port_src,char * port_dst,char * src_tunnel,char * dst_tunnel,char * ipsec_type1,char * mode1, char * ipsec_type2,char * mode2,char * direction,int u_id1,int u_id2)
{
	int ret = setkey_SP_2layer_xfrm(XFRM_MSG_UPDPOLICY,src_range,dst_range,protocol,port_src,port_dst,src_tunnel,dst_tunnel,ipsec_type1,mode1,ipsec_type2,mode2,direction,u_id1,u_id2);
	return ret;
}



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
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/socket.h>
#include <netinet/in.h>
#include <netinet/icmp6.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <linux/in.h>
#include <linux/if.h>
#include <linux/if_addr.h>
#include <linux/if_link.h>
#include <linux/netfilter/nfnetlink.h>
#include <linux/xfrm.h>
#include <linux/rtnetlink.h>
#include <cutils/properties.h>
#include <fcntl.h>
#include "utils_xfrm.h"
#include "ipsec_multilayer.h"
#define LOG_TAG "ipsec_policy_mont"
#include <log/log.h>

extern struct rtnl_handle_xfrm rth;

void rt_addr_ntoa(int af, const void *addr, char *buf, int buflen)
{
	switch (af) {
	case AF_INET:
	case AF_INET6:
		inet_ntop(af, addr, buf, buflen);
	}
}

void rt_addr_aton(int af, const char *srcaddr, void *dstbuf)
{
	switch (af) {
	case AF_INET:
	case AF_INET6:
		inet_pton(af, srcaddr, dstbuf);
	}
}

int compre_ip(__u16 family,xfrm_address_t * srcip,xfrm_address_t * dstip)
{
	switch(family)
	{
		case AF_INET:
		//ALOGD(" compre_ip: ipv4 src:%u, dst:%u\n",srcip->a4,dstip->a4);
		return (dstip->a4==srcip->a4);
        	case AF_INET6:
		//ALOGD(" compre_ip: ipv6 src:%u, dst:%u\n",srcip->a6[0],dstip->a6[0]);
		return  memcmp(srcip->a6,dstip->a6,16) ==0;
		default: return 0;
	}
}


void ipsectostr(char * str, __u8 proto)
{
	switch(proto)
	{
		case IPPROTO_ESP:
		memcpy(str,"esp",strlen("esp"));
		break;
		case IPPROTO_AH:
		memcpy(str,"ah",strlen("ah"));
		break;
		case IPPROTO_COMP :
		memcpy(str,"comp",strlen("comp"));
		break;
		default:
		str = NULL;
		break;
	}
}

void modetostr(__u8 mode,char * mode_str)
{
		switch(mode)
	{
		case XFRM_MODE_TRANSPORT: memcpy(mode_str,"transport",strlen("transport")); break;
		case XFRM_MODE_TUNNEL: memcpy(mode_str,"tunnel",strlen("tunnel"));break;
		default: memcpy(mode_str,"???",strlen("???"));break;
	}

}

int deleteRuledatabase(siptx_req_ipsec_connect_struct * Ruledatabase,struct xfrm_userpolicy_id *xpid,char * srcbuf, char * dstbuf)
{
	int i;
	siptx_req_ipsec_connect_struct * tmp = NULL;

	if((xpid->dir != XFRM_POLICY_OUT)&&(xpid->dir != XFRM_POLICY_IN))
	{
		ALOGD(" deleteRuledatabase: rule dir:%u, just ignore it\n",xpid->dir);
		return -1;
	}

        for(i =0; i<RECORD_COUNT;i++)
	{
	    tmp = Ruledatabase + i;
	    if(tmp->used == 0)
		continue;
			    if(xpid->dir != tmp->dir)
	    {
		continue;
	    }

	    if((strcmp(tmp->local_ip_c,srcbuf)==0)&&(strcmp(tmp->target_ip_s,dstbuf)==0)&&(tmp->proto == xpid->sel.proto) && xpid->sel.prefixlen_s == tmp->pref_s && xpid->sel.prefixlen_d == tmp->pref_d && xpid->dir == tmp->dir && tmp->local_port_c == ntohs(xpid->sel.sport) && tmp->target_port_s == ntohs(xpid->sel.dport))
	    {
#ifdef INIT_ENG_BUILD		
			ALOGD("deleteRuledatabase:It is a previous rule set, begin to delete it,src:%s/%u,dst:%s/%u,proto:%u,dir:%u,sport:%u,dprot:%u,tmp:%p,tmp->used:%d\n",tmp->local_ip_c,tmp->pref_s,tmp->target_ip_s,tmp->pref_d,tmp->proto,tmp->dir,tmp->local_port_c,tmp->target_port_s,tmp,tmp->used);
#endif
			/*2 layer policy will be set, enable ccmni4 forward once*/
			if(tmp->dir == XFRM_POLICY_OUT)
			{
				char ifname[IFNAMSIZ] = {0};
				if(getifbyaddr(ifname, tmp->local_ip_c, tmp->pref_s, tmp->family))
					disable_interface_forward(tmp->local_ip_c, tmp->pref_s, tmp->target_ip_s,tmp->pref_d, tmp->family, ifname);
				else
					ALOGE("getifbyaddr failed, interface cannot forward\n");
			}
			memset(tmp,0,sizeof(siptx_req_ipsec_connect_struct));
			return i;
	    }

	}
#ifdef INIT_ENG_BUILD
	if(i == RECORD_COUNT)
		ALOGD("deleteRuledatabase:there is no rule who is necessary to delete ,src:%s/%u,dst:%s/%u,proto:%u,dir:%u,sport:%u,dprot:%u\n",srcbuf,xpid->sel.prefixlen_s,dstbuf,xpid->sel.prefixlen_d,xpid->sel.proto,xpid->dir,ntohs(xpid->sel.sport),ntohs(xpid->sel.dport));
#endif
	return i;	
}
#if 0
void deleteRuledatabase_notmpl_prev(siptx_req_ipsec_connect_struct * Ruledatabase,struct xfrm_userpolicy_info * xpid)
{
	int i;
	siptx_req_ipsec_connect_struct * tmp = NULL;
	int cmpre = 0,transport_cmp =0;


	if((xpid->dir != XFRM_POLICY_OUT)&&(xpid->dir != XFRM_POLICY_IN))
	{
		ALOGD(" deleteRuledatabase: rule dir:%u, just ignore it\n",xpid->dir);
		return;
	}
        for(i =0; i<RECORD_COUNT;i++)
	{
	    tmp = Ruledatabase + i;
	    if(tmp->used == 0)
		continue;
	    if(xpid->dir != tmp->dir)
	    {
		continue;
	    }
	    /*switch(xpid->dir)
	    {
		case XFRM_POLICY_OUT:transport_cmp = (strcmp(tmp->local_ip_c,srcbuf)==0)&&xpid->sel.prefixlen_s == tmp->pref_s;break;
		case XFRM_POLICY_IN:transport_cmp = (strcmp(tmp->target_ip_s,dstbuf)==0)&&xpid->sel.prefixlen_d == tmp->pref_d;break;
		default:
		ALOGD(" deleteRuledatabase_notmpl: rule dir:%u, just ignore it\n",xpid->dir);
		return;
	   }
	   
	   switch(mode)
	   {
		case XFRM_MODE_TUNNEL: cmpre = (strcmp(tmp->local_ip_c,srcbuf)==0)&&(strcmp(tmp->target_ip_s,dstbuf)==0)&&(tmp->proto == xpid->sel.proto) && xpid->sel.prefixlen_s == tmp->pref_s && xpid->sel.prefixlen_d == tmp->pref_d && xpid->dir == tmp->dir && tmp->local_port_c == ntohs(xpid->sel.sport) && tmp->target_port_s == ntohs(xpid->sel.dport);break;
		case XFRM_MODE_TRANSPORT: cmpre = (xpid->dir == tmp->dir&& transport_cmp);break;
		default : cmpre = 0;break;
	  }
	    if(cmpre)
	    {*/
		
		ALOGD("deleteRuledatabase_notmpl_prev:It is a previous rule set, begin to delete it,src:%s/%u,dst:%s/%u,proto:%u,dir:%u,sport:%u,dprot:%u,tmp:%p,tmp->used:%d\n",tmp->local_ip_c,tmp->pref_s,tmp->target_ip_s,tmp->pref_d,tmp->proto,tmp->dir,tmp->local_port_c,tmp->target_port_s,tmp,tmp->used);
		memset(tmp,0,sizeof(siptx_req_ipsec_connect_struct));
	    //}
	}
}
#endif

int compare_oldRuledatabase(siptx_req_ipsec_connect_struct * Ruledatabase,struct xfrm_userpolicy_info *xpinfo,char * srcbuf, char * dstbuf,char * srcbuf_tun, char * dstbuf_tun,struct xfrm_user_tmpl * tmpl)
{
	int i,ret =-1;//-1:new record
	char mode[16] ={0};

	siptx_req_ipsec_connect_struct * tmp = NULL;
		int ignore_proto_spi_port = 1;
	if((xpinfo->dir != XFRM_POLICY_OUT)&&(xpinfo->dir != XFRM_POLICY_IN))
	{
		ALOGD(" addNewRuledatabase: rule dir:%u, just ignore it\n",xpinfo->dir);
		return -2;//wrong dir
	}

        for(i =0; i<RECORD_COUNT;i++)
	{
		tmp = Ruledatabase + i;
	        if(tmp->used == 0)
			continue;
		if(xpinfo->dir != tmp->dir)
		{
			continue;
		}
		modetostr(tmpl->mode,mode);
		if(tmpl->mode == XFRM_MODE_TRANSPORT)
			ignore_proto_spi_port=  (tmp->proto == xpinfo->sel.proto && tmp->local_port_c == ntohs(xpinfo->sel.sport) && tmp->target_port_s == ntohs(xpinfo->sel.dport) );

		else ignore_proto_spi_port = (strcmp(tmp->tunnel_ip_s,srcbuf_tun)==0)&&(strcmp(tmp->tunnel_ip_d,dstbuf_tun)==0);
		///ALOGD("begin to compare with set[%d],src:%s/%u,dst:%s/%u,proto:%u,dir:%u,spi:%x,sport:%u,dprot:%u src:%s/%u,dst:%s/%u,proto:%u,dir:%u,spi:%x,sport:%u,dprot:%u,srcbuf_tun:%s,tmp->tunnel_ip_s:%s,dstbuf_tun:%s,tmp->tunnel_ip_d:%s\n",i,tmp->local_ip_c,tmp->pref_s,tmp->target_ip_s,tmp->pref_d,tmp->proto,tmp->dir,tmp->local_spi_c,tmp->local_port_c,tmp->target_port_s,srcbuf,xpinfo->sel.prefixlen_s,dstbuf,xpinfo->sel.prefixlen_d,xpinfo->sel.proto,xpinfo->dir,ntohl(tmpl->id.spi),tmp->local_port_c,tmp->target_port_s,srcbuf_tun,tmp->tunnel_ip_s,dstbuf_tun,tmp->tunnel_ip_d);
	    	if(compare_ip_for_prefix(tmp->family, &tmp->local_ip_c[0], tmp->pref_s, xpinfo->sel.family, srcbuf, xpinfo->sel.prefixlen_s )&&compare_ip_for_prefix(tmp->family, &tmp->target_ip_s[0], tmp->pref_d, xpinfo->sel.family,dstbuf, xpinfo->sel.prefixlen_d))
	    	{
	    		if(ignore_proto_spi_port )
	    				ret =i;   //previous set
	    		else if (tmpl->mode == XFRM_MODE_TUNNEL)
	    			ret = i+RECORD_COUNT+1; //just tunnel src ip and tunnel dst ip are changed

	    		if(ret)
	    		{

			      tmp->used = 1;
			      tmp->family = xpinfo->sel.family;
			      memcpy (tmp ->tunnel_ip_s, srcbuf_tun,256);
			      memcpy (tmp ->tunnel_ip_d, dstbuf_tun,256);
			      /*ALOGD("It is a previous %s set[%d], src:%s/%u,dst:%s/%u,proto:%u,dir:%u,spi:%x,sport:%u,dprot:%u,ret:%d, tunnel_src:%s,tunnel_dst:%s\n",mode,i,srcbuf,xpinfo->sel.prefixlen_s,dstbuf,xpinfo->sel.prefixlen_d,xpinfo->sel.proto,xpinfo->dir,ntohl(tmpl->id.spi),tmp->local_port_c,tmp->target_port_s,ret,tmp ->tunnel_ip_s,tmp ->tunnel_ip_d);*/

			    }
	    	}

	}
	return ret;
}

int addNewRuledatabase(siptx_req_ipsec_connect_struct * Ruledatabase,struct xfrm_userpolicy_info *xpinfo,char * srcbuf, char * dstbuf,char * srcbuf_tun, char * dstbuf_tun,struct xfrm_user_tmpl * tmpl)
{
	int i,ret = -1;
	siptx_req_ipsec_connect_struct * tmp = NULL;
	char mode[16] ={0};


	modetostr(tmpl->mode,mode);
	for(i =0; i<RECORD_COUNT;i++)
	{
 		tmp = Ruledatabase + i;
		if(tmp->used == 0)
		{
			tmp->local_spi_c = ntohl(tmpl->id.spi);
			memcpy(tmp->local_ip_c,srcbuf,strlen(srcbuf));
			memcpy(tmp->target_ip_s,dstbuf,strlen(dstbuf));
        	tmp->proto = xpinfo->sel.proto;
        	tmp->pref_s = xpinfo->sel.prefixlen_s;
       		tmp->pref_d = xpinfo->sel.prefixlen_d;
			tmp->sport_mask = xpinfo->sel.sport_mask;
			tmp->dport_mask = xpinfo->sel.dport_mask;
			tmp->ifindex = xpinfo->sel.ifindex;
       		tmp->dir = xpinfo->dir;
       		tmp->local_port_c = ntohs(xpinfo->sel.sport);
			tmp->target_port_s = ntohs(xpinfo->sel.dport);
			tmp->request_id = tmpl->reqid;
			tmp->ipsec_type = tmpl->id.proto;
			tmp->used = 1;
			tmp->family = xpinfo->sel.family;
			tmp->priority = xpinfo->priority;
			tmp->tunnel_family = tmpl->family;
			tmp->user = xpinfo->sel.user;
			memcpy (tmp ->tunnel_ip_s, srcbuf_tun,256);
			memcpy (tmp ->tunnel_ip_d, dstbuf_tun,256);
#ifdef INIT_ENG_BUILD
			ALOGD("It is a new %s set[%d], tmp:%p,begin to record it,src:%s/%u[%u],dst:%s/%u[%u],proto:%u,dir:%u,spi:%x,used:%d,family:%u,priority:%d,user:%d\n",mode,i,tmp,srcbuf,xpinfo->sel.prefixlen_s,ntohs(xpinfo->sel.sport),dstbuf,xpinfo->sel.prefixlen_d,ntohs(xpinfo->sel.dport),xpinfo->sel.proto,xpinfo->dir,ntohl(tmpl->id.spi),tmp->used,tmp->family,tmp->priority,tmp->user);
#endif
			return i; //new set
		}
	}
	return ret;
}

int addattr_l(struct nlmsghdr *n, int maxlen, int type, const void *data,
	      int alen)
{
	int len = RTA_LENGTH(alen);
	struct rtattr *rta;

	if (NLMSG_ALIGN(n->nlmsg_len) + RTA_ALIGN(len) > (unsigned int)maxlen) {
		ALOGD( "addattr_l ERROR: message exceeded bound of %d,nlmsg_len:%d,len:%d\n",maxlen,NLMSG_ALIGN(n->nlmsg_len), RTA_ALIGN(len));
		return -1;
	}

	rta = NLMSG_TAIL(n);
	rta->rta_type = type;
	rta->rta_len = len;
	memcpy(RTA_DATA(rta), data, alen);
	n->nlmsg_len = NLMSG_ALIGN(n->nlmsg_len) + RTA_ALIGN(len);
	return 0;
}

void set2layeripsecrules_xfrm(siptx_req_ipsec_connect_struct * Tunndatabse,siptx_req_ipsec_connect_struct * Transdata_var,__u8 dir,int Handover)
{
	int i;
	char dir_c[8] = {0};
	siptx_req_ipsec_connect_struct * tmp = Tunndatabse;
    int  should_2layer = 0;

	struct req_handle_xfrm req;
	char tmpls_buf[XFRM_TMPLS_BUF_SIZE];
	struct xfrm_user_tmpl *tmpl;
	int tmpls_len = 0;

	memset(&req, 0, sizeof(req));
	memset(&tmpls_buf, 0, sizeof(tmpls_buf));

	req.n.nlmsg_len = NLMSG_LENGTH(sizeof(req.xpinfo));
	req.n.nlmsg_flags = NLM_F_REQUEST;
	req.n.nlmsg_type = XFRM_MSG_UPDPOLICY;


	req.xpinfo.lft.soft_byte_limit = XFRM_INF;
	req.xpinfo.lft.hard_byte_limit = XFRM_INF;
	req.xpinfo.lft.soft_packet_limit = XFRM_INF;
	req.xpinfo.lft.hard_packet_limit = XFRM_INF;
	req.xpinfo.sel.dport_mask = ~((__u16)0);
	req.xpinfo.sel.sport_mask = ~((__u16)0);
	
	

    for(i =0; i<RECORD_COUNT;i++)
	{
		tmp = Tunndatabse+i;
		if(tmp->dir != dir)
		{
			continue;
		}
		if(tmp->used == 0)
		{
			continue;
		}
		switch(dir)
		{
			case XFRM_POLICY_OUT:
			memcpy(dir_c,"out",strlen("out"));
			if(!Handover) {
				should_2layer = compare_ip_for_prefix(tmp->family, &tmp->local_ip_c[0], tmp->pref_s, Transdata_var->family, &Transdata_var->local_ip_c[0], Transdata_var->pref_s) && strcmp(tmp->tunnel_ip_s,Transdata_var->local_ip_c)!=0;
#ifdef INIT_ENG_BUILD
				ALOGD("compare out:should_2layer = %d;tmp->local_ip_c:%s,tmp->pref_s:%d,Transdata_var->local_ip_c:%s,tmp->tunnel_ip_s:%s,Transdata_var->local_ip_c:%s,Transdata_var->pref_s:%d\n",should_2layer, tmp->local_ip_c,tmp->pref_s,Transdata_var->local_ip_c,tmp->tunnel_ip_s,Transdata_var->local_ip_c,Transdata_var->pref_s);
#endif
			}
			else {
				should_2layer = compare_ip_for_prefix(tmp->family, &tmp->local_ip_c[0], tmp->pref_s, Transdata_var->family, &Transdata_var->local_ip_c[0], Transdata_var->pref_s) && strcmp(Transdata_var->tunnel_ip_s,tmp->local_ip_c)!=0;
#ifdef INIT_ENG_BUILD
				ALOGD("compare out:should_2layer = %d;tmp->local_ip_c:%s,tmp->pref_s:%d,Transdata_var->local_ip_c:%s,tmp->tunnel_ip_s:%s,Transdata_var->local_ip_c:%s,Transdata_var->pref_s:%d\n",should_2layer, tmp->local_ip_c,tmp->pref_s,Transdata_var->local_ip_c,Transdata_var->tunnel_ip_s,tmp->local_ip_c,Transdata_var->pref_s);
#endif
			}
			break;
			case XFRM_POLICY_IN:
			memcpy(dir_c,"in",strlen("in"));
			if(!Handover) {
				should_2layer = compare_ip_for_prefix(tmp->family, &tmp->target_ip_s[0], tmp->pref_d, Transdata_var->family, &Transdata_var->target_ip_s[0], Transdata_var->pref_d) && strcmp(tmp->tunnel_ip_d,Transdata_var->target_ip_s)!=0;
#ifdef INIT_ENG_BUILD
				ALOGD("compare in:should_2layer = %d;tmp->target_ip_s:%s,tmp->pref_d:%d,Transdata_var->target_ip_s:%s,Transdata_var->pref_d:%d,tmp->tunnel_ip_s:%s,Transdata_var->local_ip_c:%s\n",should_2layer, tmp->target_ip_s,tmp->pref_d,Transdata_var->target_ip_s,Transdata_var->pref_d,tmp->tunnel_ip_d,Transdata_var->target_ip_s);
#endif
			} else {
				should_2layer = compare_ip_for_prefix(tmp->family, &tmp->target_ip_s[0], tmp->pref_d, Transdata_var->family, &Transdata_var->target_ip_s[0], Transdata_var->pref_d) && strcmp(Transdata_var->tunnel_ip_d,tmp->target_ip_s)!=0;
#ifdef INIT_ENG_BUILD
				ALOGD("compare in:should_2layer = %d;tmp->target_ip_s:%s,tmp->pref_d:%d,Transdata_var->target_ip_s:%s,Transdata_var->pref_d:%d,tmp->tunnel_ip_s:%s,Transdata_var->local_ip_c:%s\n",should_2layer, tmp->target_ip_s,tmp->pref_d,Transdata_var->target_ip_s,Transdata_var->pref_d,Transdata_var->tunnel_ip_d,tmp->target_ip_s);
#endif
			}
			break;
			default: //forward or other action
            break;
		}
		if(should_2layer == 0)
		{
			ALOGD("no tunnel rule is matched\n");
			continue;
		}

		/*2 layer policy will be set, enable ccmni4 forward once*/
#ifdef INIT_ENG_BUILD
		ALOGD("check ccmni forwarding, dir %d, handover %d", tmp->dir, Handover);
#endif
		if(tmp->dir == XFRM_POLICY_OUT)
		{
			char ifname[IFNAMSIZ] = {0};
			if(!Handover) {
				if(getifbyaddr(ifname, Transdata_var->local_ip_c, Transdata_var->pref_s, Transdata_var->family))
					enable_interface_forward(Transdata_var->local_ip_c, Transdata_var->pref_s, Transdata_var->target_ip_s,Transdata_var->pref_d, tmp->tunnel_ip_s, tmp->tunnel_ip_d, Transdata_var->family, ifname);
				else
					ALOGE("getifbyaddr failed, interface cannot forward\n");
				continue;
			} else {
				if(getifbyaddr(ifname, tmp->local_ip_c, tmp->pref_s, tmp->family))
					enable_interface_forward(tmp->local_ip_c, tmp->pref_s, tmp->target_ip_s,tmp->pref_d, Transdata_var->tunnel_ip_s, Transdata_var->tunnel_ip_d, tmp->family, ifname);
				else
					ALOGE("getifbyaddr failed, interface cannot forward\n");
				continue;
			}
		}

		req.xpinfo.dir = dir;
		tmpl = (struct xfrm_user_tmpl *)((char *)tmpls_buf + tmpls_len);
		tmpl->family = tmp->family;
		tmpl->mode = XFRM_MODE_TRANSPORT;

		tmpl->aalgos = (~(__u32)0);
		tmpl->ealgos = (~(__u32)0);
		tmpl->calgos = (~(__u32)0);

		if(!Handover) {
			tmpl->reqid = Transdata_var->request_id;
			tmpl->id.proto = Transdata_var->ipsec_type;
		} else {
			tmpl->reqid = tmp->request_id;
			tmpl->id.proto = tmp->ipsec_type;
		}

	        tmpls_len += sizeof(*tmpl);
		tmpl = (struct xfrm_user_tmpl *)((char *)tmpls_buf + tmpls_len);
		tmpl->mode = XFRM_MODE_TUNNEL;
		tmpl->aalgos = (~(__u32)0);
		tmpl->ealgos = (~(__u32)0);
		tmpl->calgos = (~(__u32)0);
		if(!Handover) {
			tmpl->reqid =tmp->request_id;
			tmpl->id.proto = tmp->ipsec_type;
			tmpl->family = tmp->tunnel_family;
			rt_addr_aton(tmpl->family,tmp->tunnel_ip_s,&tmpl->saddr);
			rt_addr_aton(tmpl->family,tmp->tunnel_ip_d,&tmpl->id.daddr);
			rt_addr_aton(Transdata_var->family,Transdata_var->local_ip_c,&req.xpinfo.sel.saddr);
			rt_addr_aton(Transdata_var->family,Transdata_var->target_ip_s,&req.xpinfo.sel.daddr);
			req.xpinfo.sel.prefixlen_s = Transdata_var->pref_s;
			req.xpinfo.sel.prefixlen_d = Transdata_var->pref_d;
			req.xpinfo.sel.sport = htons(Transdata_var->local_port_c);
			req.xpinfo.sel.dport = htons(Transdata_var->target_port_s);
			req.xpinfo.sel.proto = Transdata_var->proto;
			req.xpinfo.sel.family = Transdata_var->family;
			req.xpinfo.priority = 1000;
			req.xpinfo.sel.dport_mask = Transdata_var->dport_mask;
			req.xpinfo.sel.sport_mask = Transdata_var->sport_mask;
			req.xpinfo.sel.ifindex = Transdata_var->ifindex;
			req.xpinfo.sel.user = Transdata_var->user;
#ifdef INIT_ENG_BUILD
			ALOGD("tmpl->reqid:%d,tmpl->id.proto:%d,tmpl_family:%d,mode:%d,proto:%d,tunnel_s:%s,tunnel_d:%s,local_ip_c:%s[%d/%x]/%d,target_ip_s:%s[%d/%x]/%d,family:%d,priority:0x%x,index:%d,user:%d\n",tmpl->reqid,tmpl->id.proto,tmpl->family,tmpl->mode,req.xpinfo.sel.proto,tmp->tunnel_ip_s,tmp->tunnel_ip_d,Transdata_var->local_ip_c,Transdata_var->local_port_c,req.xpinfo.sel.sport_mask,req.xpinfo.sel.prefixlen_s,Transdata_var->target_ip_s,Transdata_var->target_port_s,req.xpinfo.sel.dport_mask ,req.xpinfo.sel.prefixlen_d,req.xpinfo.sel.family,req.xpinfo.priority,req.xpinfo.sel.ifindex,req.xpinfo.sel.user);
#endif
		} else {
			tmpl->reqid =Transdata_var->request_id;
			tmpl->family = Transdata_var->tunnel_family;
			tmpl->id.proto = Transdata_var->ipsec_type;
			rt_addr_aton(tmpl->family,Transdata_var->tunnel_ip_s,&tmpl->saddr);
			rt_addr_aton(tmpl->family,Transdata_var->tunnel_ip_d,&tmpl->id.daddr);
			rt_addr_aton(tmp->family,tmp->local_ip_c,&req.xpinfo.sel.saddr);
			rt_addr_aton(tmp->family,tmp->target_ip_s,&req.xpinfo.sel.daddr);
			req.xpinfo.sel.prefixlen_s = tmp->pref_s;
			req.xpinfo.sel.prefixlen_d = tmp->pref_d;
			req.xpinfo.sel.sport = htons(tmp->local_port_c);
			req.xpinfo.sel.dport = htons(tmp->target_port_s);
			req.xpinfo.sel.proto = tmp->proto;
			req.xpinfo.sel.family = tmp->family;
			req.xpinfo.priority = 1000;
			req.xpinfo.sel.dport_mask = tmp->dport_mask;
			req.xpinfo.sel.sport_mask = tmp->sport_mask;
			req.xpinfo.sel.ifindex = tmp->ifindex;
			req.xpinfo.sel.user = tmp->user;
#ifdef INIT_ENG_BUILD
			ALOGD("tmpl->reqid:%d,tmpl->id.proto:%d,tunnel_family:%d,mode:%d,proto:%d,tunnel_s:%s,tunnel_d:%s,local_ip_c:%s[%d/%x]/%d,target_ip_s:%s[%d/%x]/%d,sel_family:%d,priority:0x%x,index:%d,user:%d\n",tmpl->reqid,tmpl->id.proto,tmpl->family,tmpl->mode,req.xpinfo.sel.proto,Transdata_var->tunnel_ip_s,Transdata_var->tunnel_ip_d,tmp->local_ip_c,tmp->local_port_c,tmp->sport_mask,req.xpinfo.sel.prefixlen_s,tmp->target_ip_s,tmp->target_port_s,tmp->dport_mask,req.xpinfo.sel.prefixlen_d,req.xpinfo.sel.family,req.xpinfo.priority,tmp->ifindex,tmp->user);
#endif
		}

		tmpls_len += sizeof(*tmpl);
		if (tmpls_len > 0) {
			addattr_l(&req.n, sizeof(req), XFRMA_TMPL,
							(void *)tmpls_buf, tmpls_len);
		}
		if(send(rth.fd, (void*)&req, sizeof(req), 0)<0)
			ALOGD("set2layeripsecrules_xfrm send failed,errno:%d",errno);
		if(req.xpinfo.dir ==XFRM_POLICY_IN)
		{
			req.xpinfo.dir = XFRM_POLICY_FWD;
			if(send(rth.fd, (void*)&req, sizeof(req), 0)<0)
				ALOGD("set2layeripsecrules_xfrm send POLICY_FWD failed,errno:%d",errno);
		}
		tmpls_len = 0;
		req.n.nlmsg_len = NLMSG_LENGTH(sizeof(req.xpinfo));
		memset(&tmpls_buf, 0, sizeof(tmpls_buf));
	}
}

void set2layeripsecrules(siptx_req_ipsec_connect_struct * Tunndatabse,siptx_req_ipsec_connect_struct * Transdata_var,__u8 dir,int Handover)
{
	int i;
	char sport[8] = {0};
	char dport[8] = {0};
	char dir_c[8] = {0};
	char ipsec_type_tunnel[8] = {0};
	char ipsec_type_transport[8] = {0};
	char transport_mode[16] = "transport";
	char tunnel_mode[16] = "tunnel";
	siptx_req_ipsec_connect_struct * tmp = Tunndatabse;
	enum PROTOCOL_TYPE protocol;
	int  protocol_int;
	int  should_2layer = 0;

	for(i =0; i<RECORD_COUNT;i++)
	{
		tmp = Tunndatabse+i;
		if(tmp->dir != dir)
		{
			continue;
		}
		if(tmp->used == 0)
		{
			continue;
		}
		switch(dir)
		{
			case XFRM_POLICY_OUT:
			strncpy(dir_c,"out",strlen("out"));
			if(!Handover) {
				should_2layer = compare_ip_for_prefix(tmp->family, &tmp->local_ip_c[0], tmp->pref_s, Transdata_var->family, &Transdata_var->local_ip_c[0], Transdata_var->pref_s) && strcmp(tmp->tunnel_ip_s,Transdata_var->local_ip_c)!=0;
				ALOGD("compare out:should_2layer = %d;tmp->local_ip_c:%s,Transdata_var->local_ip_c:%s,tmp->tunnel_ip_s:%s,Transdata_var->local_ip_c:%s\n",should_2layer, tmp->local_ip_c,Transdata_var->local_ip_c,tmp->tunnel_ip_s,Transdata_var->local_ip_c);
			}
			else {
				should_2layer = compare_ip_for_prefix(tmp->family, &tmp->local_ip_c[0], tmp->pref_s, Transdata_var->family, &Transdata_var->local_ip_c[0], Transdata_var->pref_s) && strcmp(Transdata_var->tunnel_ip_s,tmp->local_ip_c)!=0;
				ALOGD("compare out:should_2layer = %d;tmp->local_ip_c:%s,Transdata_var->local_ip_c:%s,tmp->tunnel_ip_s:%s,Transdata_var->local_ip_c:%s\n",should_2layer, tmp->local_ip_c,Transdata_var->local_ip_c,Transdata_var->tunnel_ip_s,tmp->local_ip_c);
			}
			break;
			case XFRM_POLICY_IN:
			strncpy(dir_c,"in",strlen("in"));
			if(!Handover) {
				should_2layer = compare_ip_for_prefix(tmp->family, &tmp->target_ip_s[0], tmp->pref_d, Transdata_var->family, &Transdata_var->target_ip_s[0], Transdata_var->pref_s) && strcmp(tmp->tunnel_ip_d,Transdata_var->target_ip_s)!=0;
				ALOGD("compare in:should_2layer = %d;tmp->target_ip_s:%s,Transdata_var->target_ip_s:%s,tmp->tunnel_ip_s:%s,Transdata_var->local_ip_c:%s\n",should_2layer, tmp->target_ip_s,Transdata_var->target_ip_s,tmp->tunnel_ip_d,Transdata_var->target_ip_s);
			} else {
				should_2layer = compare_ip_for_prefix(tmp->family, &tmp->target_ip_s[0], tmp->pref_d, Transdata_var->family, &Transdata_var->target_ip_s[0], Transdata_var->pref_s) && strcmp(Transdata_var->tunnel_ip_d,tmp->target_ip_s)!=0;
				ALOGD("compare in:should_2layer = %d;tmp->target_ip_s:%s,Transdata_var->target_ip_s:%s,tmp->tunnel_ip_s:%s,Transdata_var->local_ip_c:%s\n",should_2layer, tmp->target_ip_s,Transdata_var->target_ip_s,Transdata_var->tunnel_ip_d,tmp->target_ip_s);
			}
			break;
			default: //forward or other action
            break;
        }
        if(should_2layer == 0)
        {
            ALOGD("no tunnel rule is matched\n");
            continue;
		}
		if(!Handover) {
			ipsectostr(ipsec_type_tunnel,tmp->ipsec_type);

			sprintf(sport,"%u",Transdata_var->local_port_c);
			sprintf(dport,"%u",Transdata_var->target_port_s);
			ipsectostr(ipsec_type_transport,Transdata_var->ipsec_type);
			protocol_int = Transdata_var->proto;
		} else {
			ipsectostr(ipsec_type_tunnel,Transdata_var->ipsec_type);

			sprintf(sport,"%u",tmp->local_port_c);
			sprintf(dport,"%u",tmp->target_port_s);
			ipsectostr(ipsec_type_transport,tmp->ipsec_type);
			protocol_int = tmp->proto;
		}
		
		switch(protocol_int)
		{
		case 1: protocol = PROTOCOL_ICMP;break;
		case 4: protocol = PROTOCOL_IPV4;break;
		case 6: protocol = PROTOCOL_TCP;break;
		case 17: protocol = PROTOCOL_UDP;break;
		case 41: protocol = PROTOCOL_IPV6;break;
		case 47: protocol = PROTOCOL_GRE;break;
		case 50: protocol = PROTOCOL_ESP;break;
		case 51: protocol = PROTOCOL_AH;break;
		case 58: protocol = PROTOCOL_ICMPV6;break;
		case 108: protocol = PROTOCOL_IPCOMP;break;
		case 115: protocol = PROTOCOL_L2TP;break;
		default: protocol = PROTOCOL_IPV4;break;
		}
		
        //setkey_deleteSP(Transdata_var->local_ip_c,Transdata_var->target_ip_s,protocol,sport,dport,dir_c);
	if(Handover == 0)//wfc on, no handover
        	setkey_SP_update_tunnel_transport(Transdata_var->local_ip_c,Transdata_var->target_ip_s,protocol,
                sport,dport,tmp->tunnel_ip_s,tmp->tunnel_ip_d,
				ipsec_type_transport,transport_mode,ipsec_type_tunnel,tunnel_mode,dir_c,Transdata_var->request_id,tmp->request_id);
	else  //LTE handover wifi, or wifi tunnel ip is changed
        	setkey_SP_update_tunnel_transport(tmp->local_ip_c,tmp->target_ip_s,protocol,
                sport,dport,Transdata_var->tunnel_ip_s,Transdata_var->tunnel_ip_d,
				ipsec_type_transport,transport_mode,ipsec_type_tunnel,tunnel_mode,dir_c,tmp->request_id,Transdata_var->request_id);

    }
}

void change1layeripsecrules_xfrm(siptx_req_ipsec_connect_struct * Transdatabase,struct xfrm_userpolicy_id *xpid, char * srcbuf, char * dstbuf)
{
	int i;
	siptx_req_ipsec_connect_struct * tmp = Transdatabase;
	int  should_1layer = 0;


	struct req_handle_xfrm req;
	char tmpls_buf[XFRM_TMPLS_BUF_SIZE];
	struct xfrm_user_tmpl *tmpl;
	int tmpls_len = 0;

	memset(&req, 0, sizeof(req));
	memset(&tmpls_buf, 0, sizeof(tmpls_buf));

	req.n.nlmsg_len = NLMSG_LENGTH(sizeof(req.xpinfo));
	req.n.nlmsg_flags = NLM_F_REQUEST;
	req.n.nlmsg_type = XFRM_MSG_UPDPOLICY;


	req.xpinfo.lft.soft_byte_limit = XFRM_INF;
	req.xpinfo.lft.hard_byte_limit = XFRM_INF;
	req.xpinfo.lft.soft_packet_limit = XFRM_INF;
	req.xpinfo.lft.hard_packet_limit = XFRM_INF;



	if(xpid->dir != XFRM_POLICY_OUT && xpid->dir != XFRM_POLICY_IN)
	{
		return;
	}

	for(i =0; i<RECORD_COUNT;i++)
	{
		tmp = Transdatabase+i;
		if(tmp->dir != xpid->dir)
			continue;
		if(tmp->used == 0)
			continue;
                should_1layer = compare_ip_for_prefix(tmp->family, &tmp->local_ip_c[0], tmp->pref_s, xpid->sel.family, srcbuf, xpid->sel.prefixlen_s) && compare_ip_for_prefix(tmp->family,&tmp->target_ip_s[0], tmp->pref_d,  xpid->sel.family, dstbuf, xpid->sel.prefixlen_d);
#ifdef INIT_ENG_BUILD
		ALOGD("compare out:should_1layer = %d;tmp->local_ip_c:%s,tmp->pref_s:%d,srcbuf:%s,xpid->sel.prefixlen_s:%d,tmp->target_ip_s:%s,tmp->pref_d:%d, dstbuf:%s, xpid->sel.prefixlen_d:%d\n",should_1layer, tmp->local_ip_c,tmp->pref_s,srcbuf,xpid->sel.prefixlen_s,tmp->target_ip_s,tmp->pref_d,dstbuf, xpid->sel.prefixlen_d);
#endif
		if(!should_1layer)
			continue;

		/*1 layer policy will be set, disable ccmni4 forward*/
		if(tmp->dir == XFRM_POLICY_OUT)
		{
			char ifname[IFNAMSIZ] = {0};
			if(getifbyaddr(ifname, tmp->local_ip_c, tmp->pref_s, tmp->family))
				disable_interface_forward(tmp->local_ip_c, tmp->pref_s, tmp->target_ip_s,tmp->pref_d, tmp->family, ifname);
			else
				ALOGE("getifbyaddr failed, interface cannot forward\n");
		}

		req.xpinfo.dir = xpid->dir;
		tmpl = (struct xfrm_user_tmpl *)((char *)tmpls_buf + tmpls_len);
		tmpl->family = tmp->family;
		tmpl->mode = XFRM_MODE_TRANSPORT;

		tmpl->aalgos = (~(__u32)0);
		tmpl->ealgos = (~(__u32)0);
		tmpl->calgos = (~(__u32)0);
		tmpl->reqid = tmp->request_id;
		tmpl->id.proto = tmp->ipsec_type;
		tmpl->family = tmp->family;
		rt_addr_aton(tmp->family,tmp->local_ip_c,&req.xpinfo.sel.saddr);
		rt_addr_aton(tmp->family,tmp->target_ip_s,&req.xpinfo.sel.daddr);
		req.xpinfo.sel.prefixlen_s = tmp->pref_s;
		req.xpinfo.sel.prefixlen_d = tmp->pref_d;
		req.xpinfo.sel.sport = htons(tmp->local_port_c);
		req.xpinfo.sel.dport = htons(tmp->target_port_s);
		req.xpinfo.sel.proto = tmp->proto;
		req.xpinfo.sel.family = tmp->family;
		req.xpinfo.priority = 1000;
		req.xpinfo.sel.dport_mask = tmp->dport_mask;
		req.xpinfo.sel.sport_mask = tmp->sport_mask;
		req.xpinfo.sel.ifindex = tmp->ifindex;
		req.xpinfo.sel.user = tmp->user;
#ifdef INIT_ENG_BUILD
		ALOGD("change1layer---tmpl->reqid:%d,tmpl->id.proto:%d,tunnel_family:%d,mode:%d,proto:%d,local_ip_c:%s[%d/%x]/%d,target_ip_s:%s[%d/%x]/%d,sel_family:%d,priority:0x%x,index:%d,user:%d\n",tmpl->reqid,tmpl->id.proto,tmpl->family,tmpl->mode,req.xpinfo.sel.proto,tmp->local_ip_c,tmp->local_port_c,tmp->sport_mask,req.xpinfo.sel.prefixlen_s,tmp->target_ip_s,req.xpinfo.sel.dport,tmp->dport_mask,req.xpinfo.sel.prefixlen_d,req.xpinfo.sel.family,req.xpinfo.priority,tmp->ifindex,tmp->user);
#endif
		tmpls_len += sizeof(*tmpl);
		if (tmpls_len > 0) {
			addattr_l(&req.n, sizeof(req), XFRMA_TMPL,
						  (void *)tmpls_buf, tmpls_len);
		}
		if(send(rth.fd, (void*)&req, sizeof(req), 0)<0)
			ALOGD("change1layeripsecrules_xfrm send failed,errno:%d",errno);
		if(req.xpinfo.dir ==XFRM_POLICY_IN)
		{
			req.xpinfo.dir = XFRM_POLICY_FWD;
			if(send(rth.fd, (void*)&req, sizeof(req), 0)<0)
				ALOGD("change1layeripsecrules_xfrm send POLICY_FWD failed,errno:%d",errno);
		}
		tmpls_len = 0;
		memset(&tmpls_buf, 0, sizeof(tmpls_buf));

		req.n.nlmsg_len = NLMSG_LENGTH(sizeof(req.xpinfo));
	}
}


void change1layeripsecrules(siptx_req_ipsec_connect_struct * Transdatabase,struct xfrm_userpolicy_id *xpid, char * srcbuf, char * dstbuf)
{
	int i;
	char sport[8] = {0};
	char dport[8] = {0};
	char dir_c[8] = {0};
	char ipsec_type_transport[8] = {0};
	siptx_req_ipsec_connect_struct * tmp = Transdatabase;
        enum PROTOCOL_TYPE protocol;
	int  should_1layer = 0;
	switch(xpid->dir)
	{
		case XFRM_POLICY_OUT:
		memcpy(dir_c,"out",strlen("out"));
		break;
		case XFRM_POLICY_IN:
		memcpy(dir_c,"in",strlen("in"));
		break;
		default: //forward or other action
		return;
	}

	for(i =0; i<RECORD_COUNT;i++)
	{
		tmp = Transdatabase+i;
		if(tmp->dir != xpid->dir)
			continue;
		if(tmp->used == 0)
			continue;
                should_1layer = compare_ip_for_prefix(tmp->family, &tmp->local_ip_c[0], tmp->pref_s, xpid->sel.family, srcbuf, xpid->sel.prefixlen_s) && compare_ip_for_prefix(tmp->family,&tmp->target_ip_s[0], tmp->pref_d,  xpid->sel.family, dstbuf, xpid->sel.prefixlen_d);
		//ALOGD("compare out:should_1layer = %d;tmp->local_ip_c:%s,srcbuf:%s,tmp->target_ip_s:%s,dstbuf:%s\n",should_1layer, tmp->local_ip_c,srcbuf,tmp->target_ip_s,dstbuf);
		if(!should_1layer)
			continue;
		ipsectostr(ipsec_type_transport,tmp->ipsec_type);
		sprintf(sport,"%u",tmp->local_port_c);
		sprintf(dport,"%u",tmp->target_port_s);
		switch(tmp->proto)
		{
		case 1: protocol = PROTOCOL_ICMP;break;
		case 4: protocol = PROTOCOL_IPV4;break;
		case 6: protocol = PROTOCOL_TCP;break;
		case 17: protocol = PROTOCOL_UDP;break;
		case 41: protocol = PROTOCOL_IPV6;break;
		case 47: protocol = PROTOCOL_GRE;break;
		case 50: protocol = PROTOCOL_ESP;break;
		case 51: protocol = PROTOCOL_AH;break;
		case 58: protocol = PROTOCOL_ICMPV6;break;
		case 108: protocol = PROTOCOL_IPCOMP;break;
		case 115: protocol = PROTOCOL_L2TP;break;
		default: protocol = PROTOCOL_IPV4;break;
		}
		/*ALOGD("change1layeripsecrules:src:%s[%u],dst:%s[%u],protocol:%d,dir:%d\n",tmp->local_ip_c,tmp->local_port_c,tmp->target_ip_s,tmp->target_port_s,protocol,xpid->dir);*/
		setkey_SP_update_transport(tmp->local_ip_c,tmp->target_ip_s,protocol,sport,dport,ipsec_type_transport, dir_c,tmp->request_id);

	}
}

int compare_ip_for_prefix(int family1, char* ip1, int pfix1, int family2, char* ip2, int pfix2)
{
	unsigned char ip1_data[128] = {0};
	unsigned char ip2_data[128] = {0};
	unsigned int* ip1_ptr = (unsigned int*)&ip1_data[0];
	unsigned int* ip2_ptr = (unsigned int*)&ip2_data[0];
	unsigned int ip_num;
	int min_pfix = pfix1 < pfix2 ? pfix1 : pfix2;

	/*ALOGD("[compare_ip_for_prefix] family1 = %d; family2 = %d; ip1 = %s; pfix1 = %d; ip2 = %s; pfix2 = %d; min_pfix = %d\n",
				family1, family2, ip1, pfix1, ip2, pfix2, min_pfix);*/

	if (family1 != family2)
		return 0;
	if (min_pfix > 128)
		return 0;
	if (min_pfix <= 0)
		return 1;
	if (!inet_pton(family1, ip1, ip1_ptr))
		return 0;
	if (!inet_pton(family2, ip2, ip2_ptr))
		return 0;

    while (min_pfix > 0) {
    	/*ALOGD("[compare_ip_for_prefix] min_pfix = %d; ip1_ptr = 0x%08X; ip2_ptr = 0x%08X\n", min_pfix, *ip1_ptr, *ip2_ptr);*/
    	if (min_pfix - 32 >= 0) {
    		if (*ip1_ptr != *ip2_ptr)
    			return 0;
    	} else {
    		ip_num = (*ip1_ptr ^ *ip2_ptr);
    		int isequal = !(ip_num & htonl(0xFFFFFFFFu << (32 - min_pfix)));
    		return isequal;
    	}
    	ip1_ptr += 1;
    	ip2_ptr += 1;
    	min_pfix -= 32;
    }
	return 1;
}



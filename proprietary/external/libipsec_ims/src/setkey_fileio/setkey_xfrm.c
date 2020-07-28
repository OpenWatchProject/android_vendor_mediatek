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
#include <sys/capability.h>
#include <cutils/properties.h>
#include "setkey_fileio.h"
#include "setkey_xfrm_parse.h"
#include "utils_xfrm.h"
#define LOG_TAG "setkey"
#include <log/log.h>


int volte_pid = -1;
/*only record volte_stack's pid for flush_SA_SP_exist's use*/
void set_property_volte()
{
	char pid[8] ={0};
	if(strstr(getprogname(),"volte")!= NULL)
		if(volte_pid ==-1)
		{
			sprintf(pid,"%d",getpid());
			int ret = property_set("vendor.net.ims.volte.pid",pid);
  			if(ret != 0)
  			{
      				ALOGE("set property failed,errno:%d\n",errno);
				return;
  			}
			volte_pid =getpid();
		}
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

int setkey_deleteSP_xfrm(char * src,char * dst,enum PROTOCOL_TYPE protocol,char * src_port,char * dst_port,char * direction)
{

	char src_arr[128] = {0};
	char dst_arr[128] = {0};
	unsigned groups = ~((unsigned)0); /* XXX */
	struct rtnl_handle_xfrm rth = { -1,{0},{0},0,0 };


	memcpy(src_arr,src,strlen(src));
	memcpy(dst_arr,dst,strlen(dst));


/*fragment delete policy netlink msg*/
	struct {
		struct nlmsghdr			n;
		struct xfrm_userpolicy_id	xpid;
		char				buf[RTA_BUF_SIZE];
	} req;

	memset(&req, 0, sizeof(req));

	req.n.nlmsg_len = NLMSG_LENGTH(sizeof(req.xpid));
	req.n.nlmsg_flags = NLM_F_REQUEST;
	req.n.nlmsg_type = XFRM_MSG_DELPOLICY;

	req.xpid.dir = xfrm_dir_parse(direction);
	if(req.xpid.dir == XFRM_POLICY_ERROR)
	{
		ALOGD("setkey_deleteSP_xfrm dir:%s is wrong\n",direction);
		return -1;
	}
	xfrm_selector_parse(&req.xpid.sel, src_arr,dst_arr,protocol,src_port,dst_port);

	groups |= (nl_mgrp_xfrm(XFRMNLGRP_ACQUIRE)|nl_mgrp_xfrm(XFRMNLGRP_EXPIRE)|nl_mgrp_xfrm(XFRMNLGRP_SA)|nl_mgrp_xfrm(XFRMNLGRP_POLICY));

	if (rtnl_open_byproto_xfrm(&rth, groups, NETLINK_XFRM) < 0)
	{
		rtnl_close_xfrm(&rth);
		return -1;
	}

	if(send(rth.fd, (void*)&req, sizeof(req), 0)<0)
	{
		ALOGD("set2layeripsecrules_xfrm send failed,errno:%d\n",errno);
		rtnl_close_xfrm(&rth);
		return -1;
	}
	if(req.xpid.dir ==XFRM_POLICY_IN)
	{
		req.xpid.dir = XFRM_POLICY_FWD;
		if(send(rth.fd, (void*)&req, sizeof(req), 0)<0)
		{
			ALOGD("setkey_deleteSP_xfrm send POLICY_FWD failed,errno:%d",errno);
			rtnl_close_xfrm(&rth);
			return -1;
		}	
	}
#ifdef INIT_ENG_BUILD
	ALOGD("setkey_deleteSP_xfrm successed fd:%d --spddelete %s[%s] %s[%s] %d -P %s;\n",rth.fd,src,src_port,dst,dst_port,protocol,direction);
#endif
	rtnl_close_xfrm(&rth);
	return 0;
}

int setkey_deleteSA_xfrm(char * src,char * dst,char * ipsec_type,char * spi_src)
{

	char src_arr[128] = {0};
	char dst_arr[128] = {0};
	unsigned groups = ~((unsigned)0); /* XXX */
	struct rtnl_handle_xfrm rth = { -1,{0},{0},0,0 };

	strncpy(src_arr,src,strlen(src) + 1);
	strncpy(dst_arr,dst,strlen(dst) + 1);

	/*fragment delete state netlink msg*/
	struct {
		struct nlmsghdr 	n;
		struct xfrm_usersa_id	xsid;
		char   			buf[RTA_BUF_SIZE];
	} req;

	memset(&req, 0, sizeof(req));

	req.n.nlmsg_len = NLMSG_LENGTH(sizeof(req.xsid));
	req.n.nlmsg_flags = NLM_F_REQUEST;
	req.n.nlmsg_type = XFRM_MSG_DELSA;
	req.xsid.family = AF_UNSPEC;

	/* ID */
	struct xfrm_id id ;
	xfrm_address_t saddr_xfrm;
	memset(&id, 0, sizeof(id));
	memset(&saddr_xfrm, 0, sizeof(saddr_xfrm));
	xfrm_id_parse(&saddr_xfrm, &id, &req.xsid.family,src_arr,dst_arr,ipsec_type);
	__u32 spi;
	if (get_u32(&spi, spi_src, 0))
	{
		ALOGD("xfrm_id_parse spi:%s is wrong\n",spi_src);
		return -1;
	}

	spi = htonl(spi);
	id.spi = spi;
	memcpy(&req.xsid.daddr, &id.daddr, sizeof(req.xsid.daddr));
	req.xsid.spi = id.spi;
	req.xsid.proto = id.proto;

	addattr_l(&req.n, sizeof(req.buf), XFRMA_SRCADDR,
				  (void *)&saddr_xfrm, sizeof(saddr_xfrm));

	groups |= (nl_mgrp_xfrm(XFRMNLGRP_ACQUIRE)|nl_mgrp_xfrm(XFRMNLGRP_EXPIRE)|nl_mgrp_xfrm(XFRMNLGRP_SA)|nl_mgrp_xfrm(XFRMNLGRP_POLICY));

	if (rtnl_open_byproto_xfrm(&rth, groups, NETLINK_XFRM) < 0)
	{
		rtnl_close_xfrm(&rth);
		return -1;
	}

	if(send(rth.fd, (void*)&req, sizeof(req), 0)<0)
	{
		ALOGD("set2layeripsecrules_xfrm send failed,errno:%d\n",errno);
		rtnl_close_xfrm(&rth);
		return -1;
	}
#ifdef INIT_ENG_BUILD
	ALOGD("setkey_deleteSA_xfrm successed fd:%d --delete %s %s %s %s;\n",rth.fd,src,dst,ipsec_type,spi_src);
#endif
	rtnl_close_xfrm(&rth);
	return 0;
}

int setkey_setSA_xfrm(int cmd,char * ip_src,char * ip_dst,char * ipsec_type,char * spi_src,char * mode, 
                 char * encrp_algo_src,char * encrp_key_src,char * intergrity_algo_src,char * intergrity_key_src,int u_id)
{

	char encrp_algo_arr[128] = {0};
	char intergrity_algo_arr[128] = {0};
	char srcport_arr[64] = {0};
	char dstport_arr[64] = {0};
	char * port_head = NULL;
	char * port_tail = NULL;
	char src_arr[128] = {0};
	char dst_arr[128] = {0};
	unsigned groups = ~((unsigned)0); /* XXX */
	struct rtnl_handle_xfrm rth = { -1,{0},{0},0,0 };

	set_property_volte();

	/*sepearte src addr and src port\dst addr and dst port, state's format often is add xx[xx] xx[xx] esp xx ....*/
	port_head = strchr(ip_src,'[');
	if(port_head == NULL)
	{
		memcpy(src_arr,ip_src,strlen(ip_src));
		memcpy(srcport_arr,"0",1);
	}
	else
	{
		port_tail = strchr(ip_src,']');
		if (port_tail == NULL)
		{
#ifdef INIT_ENG_BUILD
			ALOGD("setkey_SA_xfrm wrong src ip:%s \n",ip_src);
#endif
			return -1;
		}
		else
		{
			memcpy(srcport_arr,port_head+1,port_tail-port_head-1);
			memcpy(src_arr,ip_src,port_head-ip_src);
		}
	}
	port_head = strchr(ip_dst,'[');
	if(port_head==NULL)
	{
		memcpy(dst_arr,ip_dst,strlen(ip_dst));
		memcpy(dstport_arr,"0",1);
	}
	else
	{
		port_tail = strchr(ip_dst,']');
		if (port_tail == NULL)
		{
#ifdef INIT_ENG_BUILD
			ALOGD("setkey_SA_xfrm wrong dst ip:%s \n",ip_dst);
#endif
			return -1;
		}
		else
		{
			memcpy(dstport_arr,port_head+1,port_tail-port_head-1);
			memcpy(dst_arr,ip_dst,port_head-ip_dst);
		}
	}

	/*fragment add/modify state netlink msg*/
	struct {
		struct nlmsghdr 	n;
		struct xfrm_usersa_info xsinfo;
		char   			buf[RTA_BUF_SIZE];
	} req;

	memset(&req, 0, sizeof(req));

	req.n.nlmsg_len = NLMSG_LENGTH(sizeof(req.xsinfo));
	req.n.nlmsg_flags = NLM_F_REQUEST;
	req.n.nlmsg_type = cmd;


	req.xsinfo.family = AF_UNSPEC;
	req.xsinfo.lft.soft_byte_limit = XFRM_INF;
	req.xsinfo.lft.hard_byte_limit = XFRM_INF;
	req.xsinfo.lft.soft_packet_limit = XFRM_INF;
	req.xsinfo.lft.hard_packet_limit = XFRM_INF;
	req.xsinfo.reqid = u_id;
	/* ID */
	if(xfrm_mode_parse(&req.xsinfo.mode, mode) ==-1)
	{
		ALOGD("setkey_SA_xfrm mode:%s is wrong\n",mode);
		return -1;
	}
	__u32 spi;
	if (get_u32(&spi, spi_src, 0))
	{
		ALOGD("xfrm_id_parse spi:%s is wrong\n",spi_src);
		return -1;
	}

	spi = htonl(spi);
	req.xsinfo.id.spi = spi;
	if(xfrm_id_parse(&req.xsinfo.saddr, &req.xsinfo.id,&req.xsinfo.family,
				      src_arr,dst_arr,ipsec_type)<0)
	{
#ifdef INIT_ENG_BUILD
		ALOGD("setkey_SA_xfrm->xfrm_id_parse failed ip_src:%s,ip_dst:%s,ipsec_type:%s",ip_src,ip_dst,ipsec_type);
#endif
		return -1;
	}
	xfrm_selector_parse(&req.xsinfo.sel, src_arr,dst_arr,0,srcport_arr,dstport_arr);
	/* ALGO */
	struct {
		union {
			struct xfrm_algo alg;
			struct xfrm_algo_aead aead;
			struct xfrm_algo_auth auth;
		} u;
		char buf[XFRM_ALGO_KEY_BUF_SIZE];
	} alg;
	int len;
	char *buf = NULL;
	memset(&alg, 0, sizeof(alg));
	buf = alg.u.alg.alg_key;
	len = sizeof(alg.u.alg);

	xfrm_encry_algo_parse(encrp_algo_src, encrp_algo_arr);
	if(strcmp(encrp_algo_arr,"not-supported") == 0)
		return -1;

	if((encrp_algo_src == NULL)||(strcmp(encrp_algo_src,"null")==0))
	{
			alg.u.alg.alg_key_len = 0;
			memcpy(alg.u.alg.alg_name, "ecb(cipher_null)", strlen("ecb(cipher_null)"));
	}
	else
	if(xfrm_algo_parse((void *)&alg,  encrp_algo_arr, encrp_key_src,
			   buf, sizeof(alg.buf))==-1)
	{
#ifdef INIT_ENG_BUILD
		ALOGD("setkey_SA_xfrm->xfrm_algo_parse failed encrp_algo:%s,encrp_key:%s,",encrp_algo_src, encrp_key_src);
#endif
		return -1;
	}
	len += alg.u.alg.alg_key_len;
	addattr_l(&req.n, sizeof(req.buf), XFRMA_ALG_CRYPT,(void *)&alg, len);

	memset((void *)&alg, 0,sizeof(alg));
	buf = alg.u.alg.alg_key;
	len = sizeof(alg.u.alg);
	xfrm_interg_algo_parse(intergrity_algo_src, intergrity_algo_arr);
	if(strcmp(intergrity_algo_arr ,"not-supported") == 0)
		return -1;
	if((intergrity_algo_src == NULL)||(strcmp(intergrity_algo_src,"null")==0))
	{
			alg.u.alg.alg_key_len = 0;
			memcpy(alg.u.alg.alg_name, "digest_null", strlen("digest_null"));
	}
	else
	if(xfrm_algo_parse((void *)&alg,  intergrity_algo_arr, intergrity_key_src,
			   buf, sizeof(alg.buf))<-1)
	{
#ifdef INIT_ENG_BUILD
		ALOGD("setkey_SA_xfrm->xfrm_algo_parse failed intergrity crp_algo:%s,intergrity _key:%s,",intergrity_algo_src, intergrity_key_src);
#endif
		return -1;
	}	
	len += alg.u.alg.alg_key_len;
	addattr_l(&req.n, sizeof(req.buf), XFRMA_ALG_AUTH,(void *)&alg, len);

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
#ifdef INIT_ENG_BUILD
	ALOGD("setkey_SA_xfrm successed fd:%d ---add %s %s %s %s  -m %s -E %s %s  -A %s %s -u %d; spi:%d\n",rth.fd,ip_src,ip_dst,ipsec_type,spi_src,mode, 
                 encrp_algo_src,encrp_key_src,intergrity_algo_src,intergrity_key_src,u_id,req.xsinfo.id.spi);
#endif
	rtnl_close_xfrm(&rth);
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

	set_property_volte();

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

	set_property_volte();

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
int setkey_flushSAD_xfrm(char * ipsec_type)
{
	unsigned groups = ~((unsigned)0); /* XXX */
	struct rtnl_handle_xfrm rth = { -1,{0},{0},0,0 };

	struct {
		struct nlmsghdr			n;
		struct xfrm_usersa_flush	xsf;
	} req;


	memset(&req, 0, sizeof(req));

	req.n.nlmsg_len = NLMSG_LENGTH(sizeof(req.xsf));
	req.n.nlmsg_flags = NLM_F_REQUEST;
	req.n.nlmsg_type = XFRM_MSG_FLUSHSA;
	req.xsf.proto = 0;

	req.xsf.proto = (__u8)xfrm_xfrmproto_getbyname(ipsec_type);
	if (!req.xsf.proto)
	{
		ALOGD("setkey_flushSAD_xfrm :%s is wrong",ipsec_type);
		return -1;
	}

	groups |= (nl_mgrp_xfrm(XFRMNLGRP_ACQUIRE)|nl_mgrp_xfrm(XFRMNLGRP_EXPIRE)|nl_mgrp_xfrm(XFRMNLGRP_SA)|nl_mgrp_xfrm(XFRMNLGRP_POLICY));

	if (rtnl_open_byproto_xfrm(&rth, groups, NETLINK_XFRM) < 0)
	{
		rtnl_close_xfrm(&rth);
		return -1;
	}

	if(send(rth.fd, (void*)&req, sizeof(req), 0)<0)
	{
		ALOGD("set2layeripsecrules_xfrm send POLICY_FWD failed,errno:%d",errno);
		rtnl_close_xfrm(&rth);
		return -1;
	}
	ALOGD("setkey_flushSAD_xfrm successed fd:%d --flush ;\n",rth.fd);
	rtnl_close_xfrm(&rth);

	return 0;
}
int setkey_flushSPD_xfrm(void)
{
	unsigned groups = ~((unsigned)0); /* XXX */
	struct rtnl_handle_xfrm rth = { -1,{0},{0},0,0 };


	struct {
		struct nlmsghdr	n;
		char		buf[RTA_BUF_SIZE];
	} req;

	memset(&req, 0, sizeof(req));


	req.n.nlmsg_len = NLMSG_LENGTH(0); /* nlmsg data is nothing */
	req.n.nlmsg_flags = NLM_F_REQUEST;
	req.n.nlmsg_type = XFRM_MSG_FLUSHPOLICY;

	groups |= (nl_mgrp_xfrm(XFRMNLGRP_ACQUIRE)|nl_mgrp_xfrm(XFRMNLGRP_EXPIRE)|nl_mgrp_xfrm(XFRMNLGRP_SA)|nl_mgrp_xfrm(XFRMNLGRP_POLICY));

	if (rtnl_open_byproto_xfrm(&rth, groups, NETLINK_XFRM) < 0)
	{
		rtnl_close_xfrm(&rth);
		return -1;
	}

	if(send(rth.fd, (void*)&req, sizeof(req), 0)<0)
	{
		ALOGD("set2layeripsecrules_xfrm send POLICY_FWD failed,errno:%d",errno);
		rtnl_close_xfrm(&rth);
		return -1;
	}
	ALOGD("setkey_flushSPD_xfrm successed fd:%d --flush ;\n",rth.fd);

	rtnl_close_xfrm(&rth);

	return 0;
}
int flush_SA_SP_exist_xfrm()
{
	unsigned groups = ~((unsigned)0); /* XXX */
	struct rtnl_handle_xfrm rth = { -1,{0},{0},0,0 };

	struct {
		struct nlmsghdr nlh;
		struct rtgenmsg g;
		__u16 align_rta;	/* attribute has to be 32bit aligned */
		struct rtattr ext_req;
		__u32 ext_filter_mask;
	} req;

	memset(&req, 0, sizeof(req));
	req.nlh.nlmsg_len = sizeof(req);
	req.nlh.nlmsg_type = XFRM_MSG_GETPOLICY;
	req.nlh.nlmsg_flags = NLM_F_DUMP|NLM_F_REQUEST;
	req.nlh.nlmsg_seq = rth.dump = ++rth.seq;
	req.g.rtgen_family = AF_UNSPEC;

	req.ext_req.rta_type = IFLA_EXT_MASK;
	req.ext_req.rta_len = RTA_LENGTH(sizeof(__u32));
	req.ext_filter_mask = RTEXT_FILTER_VF;

	groups |= (nl_mgrp_xfrm(XFRMNLGRP_ACQUIRE)|nl_mgrp_xfrm(XFRMNLGRP_EXPIRE)|nl_mgrp_xfrm(XFRMNLGRP_SA)|nl_mgrp_xfrm(XFRMNLGRP_POLICY));

	if (rtnl_open_byproto_xfrm(&rth, groups, NETLINK_XFRM) < 0)
	{
		rtnl_close_xfrm(&rth);
		return -1;
	}

	if(send(rth.fd, (void*)&req, sizeof(req), 0)<0)
	{
		ALOGD("flush_SA_SP_exist_xfrm send msg failed,errno:%d",errno);
		rtnl_close_xfrm(&rth);
		return -1;
	}
	if (rtnl_listen_xfrm(&rth, rtnl_accept_msg_xfrm) < 0)
	{
		rtnl_close_xfrm(&rth);
		ALOGD("flush_SA_SP_exist_xfrm <0 done");
		return -2;
	}
	ALOGD("flush_SA_SP_exist_xfrm done");
	rtnl_close_xfrm(&rth);

	return 0;
}

int xfrm_policy_process_delete_exist( struct rtnl_handle_xfrm * rth,struct nlmsghdr *n,pid_t volte_pid)
{

	struct xfrm_userpolicy_info *xpinfo = NULL;
	int len = n->nlmsg_len;


	xpinfo = NLMSG_DATA(n);
	len -= NLMSG_SPACE(sizeof(*xpinfo));
	if (len < 0) {
		ALOGE( "BUG: wrong nlmsg len %d\n", len);
		return -1;
	}
	if(xpinfo->sel.user != volte_pid)
	{
		ALOGE( "we donot need to process:%d's policy\n", xpinfo->sel.user);
		return 0;
	}

/*fragment delete policy*/
	struct {
		struct nlmsghdr			n;
		struct xfrm_userpolicy_id	xpid;
		char				buf[RTA_BUF_SIZE];
	} req;

	memset(&req, 0, sizeof(req));

	req.n.nlmsg_len = NLMSG_LENGTH(sizeof(req.xpid));
	req.n.nlmsg_flags = NLM_F_REQUEST;
	req.n.nlmsg_type = XFRM_MSG_DELPOLICY;

	req.xpid.dir = xpinfo->dir;

	memcpy(&req.xpid.sel,&xpinfo->sel,sizeof(req.xpid.sel));


	if(send(rth->fd, (void*)&req, sizeof(req), 0)<0)
	{
		ALOGD("xfrm_policy_process_delete_exist send failed,errno:%d",errno);
		return -1;
	}
#ifdef INIT_ENG_BUILD
	if(req.xpid.sel.family == AF_INET)
		ALOGD("xfrm_policy_process_delete_exist successed fd:%d --spddelete 0x%x[%d] 0x%x[%d] %d -P %d;\n",rth->fd,req.xpid.sel.saddr.a4,req.xpid.sel.sport,req.xpid.sel.daddr.a4,req.xpid.sel.dport,req.xpid.sel.proto,req.xpid.dir);
        else 
		ALOGD("xfrm_policy_process_delete_exist successed fd:%d --spddelete 0x%x %x %x %x[%d] 0x%x %x %x %x[%d] %d -P %d;\n",rth->fd,
		req.xpid.sel.saddr.a6[0],req.xpid.sel.saddr.a6[1],req.xpid.sel.saddr.a6[2],req.xpid.sel.saddr.a6[3],req.xpid.sel.sport,
		req.xpid.sel.daddr.a6[0],req.xpid.sel.daddr.a6[1],req.xpid.sel.daddr.a6[2],req.xpid.sel.daddr.a6[3],req.xpid.sel.dport,
		req.xpid.sel.proto,req.xpid.dir);
#endif
	return 0;
}

int xfrm_state_process_delete_exist( struct rtnl_handle_xfrm * rth,struct nlmsghdr *n,pid_t volte_pid)
{

	struct xfrm_usersa_info * xsinfo = NULL;
	int len = n->nlmsg_len;
	xfrm_address_t	saddr;

	xsinfo = NLMSG_DATA(n);
	len -= NLMSG_SPACE(sizeof(*xsinfo));
	if (len < 0) {
		ALOGE( "BUG: wrong nlmsg len %d\n", len);
		return -1;
	}

	if(xsinfo->sel.user != volte_pid)
	{
		ALOGE( "we donot need to process:%d's policy\n", xsinfo->sel.user);
		return 0;
	}

/*fragment delete state*/
	struct {
		struct nlmsghdr			n;
		struct xfrm_usersa_id		xsid;
		char				buf[RTA_BUF_SIZE];
	} req;

	memset(&req, 0, sizeof(req));

	req.n.nlmsg_len = NLMSG_LENGTH(sizeof(req.xsid));
	req.n.nlmsg_flags = NLM_F_REQUEST;
	req.n.nlmsg_type = XFRM_MSG_DELSA;

	req.xsid.family = xsinfo->family;
	memcpy(&req.xsid.daddr,&xsinfo->sel.daddr,sizeof(xsinfo->sel.daddr));
	memcpy(&saddr,&xsinfo->saddr,sizeof(xsinfo->saddr));
	addattr_l(&req.n, sizeof(req.buf), XFRMA_SRCADDR,
				  (void *)&saddr, sizeof(saddr));
	if(send(rth->fd, (void*)&req, sizeof(req), 0)<0)
	{
		ALOGD("xfrm_state_process_delete_exist send failed,errno:%d",errno);
		return -1;
	}
#ifdef INIT_ENG_BUILD
	if(xsinfo->sel.family == AF_INET)
		ALOGD("xfrm_state_process_delete_exist successed fd:%d --delete 0x%x 0x%x %d;\n",rth->fd,xsinfo->sel.saddr.a4,xsinfo->sel.daddr.a4,xsinfo->id.spi);
        else 
		ALOGD("xfrm_state_process_delete_exist successed fd:%d --delete 0x%x %x %x %x 0x%x %x %x %x %d ;\n",rth->fd,
		xsinfo->sel.saddr.a6[0],xsinfo->sel.saddr.a6[1],xsinfo->sel.saddr.a6[2],xsinfo->sel.saddr.a6[3],
		xsinfo->sel.daddr.a6[0],xsinfo->sel.daddr.a6[1],xsinfo->sel.daddr.a6[2],xsinfo->sel.daddr.a6[3],
		xsinfo->id.spi);
#endif
	return 0;
}


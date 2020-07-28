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

#ifndef _SETIPSECMULTILAYER_H
#define _SETIPSECMULTILAYER_H
#include <sys/un.h>
#include <linux/xfrm.h>
#include <linux/types.h>
#include <linux/socket.h>
#include <linux/rtnetlink.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <dirent.h>
#include <time.h>
#include "utils_xfrm.h"



typedef struct {

    unsigned int        local_spi_c;
    unsigned int        local_port_c;
    unsigned int        target_port_s;
    char        local_ip_c[256];
    char        target_ip_s[256];
    char        tunnel_ip_s[256];
    char        tunnel_ip_d[256];
    __be16      sport_mask; 
    __be16      dport_mask;
    int         ifindex;

    __u32               encry_algo;
    __u32               integrity_algo;

    short               is_wfc_enable;
    short               used;
    __u8                proto;
    __u8                dir;
    __u8                pref_s;
    __u8                pref_d;
    __u16               family;
    __u16               tunnel_family;
    unsigned int        request_id;          //< serial number of of inter-task request
    __u8                ipsec_type;
    __u32               priority;
    int                 user;   //volte_stack's pid
} siptx_req_ipsec_connect_struct;

#define RECORD_COUNT 128
#define IPFORWARD_ON 1
#define IPFORWARD_OFF 0

extern siptx_req_ipsec_connect_struct TunnelRule[RECORD_COUNT];
extern siptx_req_ipsec_connect_struct TransportRule[RECORD_COUNT];

/*set one SP of one direction just for transport mode*/
/*protocol:tcp icmp udp icmp6 ip4 gre
  direction:src->dst */
extern int setkey_SP_xfrm(int cmd,char * src_range,char * dst_range,enum PROTOCOL_TYPE protocol,char * port_src,char * port_dst,char * src_tunnel,char * dst_tunnel,char * ipsec_type,char * mode, char * direction,int u_id);

/*update one SP of one direction, for 2 layers' ipsec--tunnel mode+transport mode or transport mode+tunnel mode*/
/*protocol:tcp icmp udp icmp6 ip4 gre
  direction:src->dst
src_tunnel,dst_tunnel: tunnel src ip tunnel dst ip */
int setkey_SP_2layer_xfrm(int cmd,char * src_range,char * dst_range,enum PROTOCOL_TYPE protocol,char * port_src,char * port_dst,char * src_tunnel,char * dst_tunnel,char * ipsec_type1,char * mode1, char * ipsec_type2,char * mode2,char * direction,int u_id1,int u_id2);


extern int setkey_SP_update_transport(char * src_range,char * dst_range,enum PROTOCOL_TYPE protocol,
				char * port_src,char * port_dst,char * ipsec_type, char * direction,
				int u_id);
extern int setkey_SP_update_tunnel_transport(char * src_range,char * dst_range,
				enum PROTOCOL_TYPE protocol,char * port_src,char * port_dst,
				char * src_tunnel,char * dst_tunnel,char * ipsec_type1,char * mode1,
				char * ipsec_type2,char * mode2,char * direction,int u_id1,int u_id2);

int compre_ip(__u16 family,xfrm_address_t * srcip,xfrm_address_t * dstip);
int parse_rtattr(struct rtattr *tb[], int max, struct rtattr *rta, int len);

void ipsectostr(char * str, __u8 proto);
int deleteRuledatabase(siptx_req_ipsec_connect_struct * Ruledatabase,struct xfrm_userpolicy_id *xpid,
			char * srcbuf, char * dstbuf);
int compare_oldRuledatabase(siptx_req_ipsec_connect_struct * Ruledatabase,
			struct xfrm_userpolicy_info *xpinfo,char * srcbuf, char * dstbuf,
			char * srcbuf_tun, char * dstbuf_tun,struct xfrm_user_tmpl * tmpl);
int addNewRuledatabase(siptx_req_ipsec_connect_struct * Ruledatabase,struct xfrm_userpolicy_info *xpinfo,
			char * srcbuf, char * dstbuf,char * srcbuf_tun, char * dstbuf_tun,
			struct xfrm_user_tmpl * tmpl);
void set2layeripsecrules(siptx_req_ipsec_connect_struct * Tunndatabse,
			siptx_req_ipsec_connect_struct * Transdata_var,__u8 dir,int Handover);
void set2layeripsecrules_xfrm(siptx_req_ipsec_connect_struct * Tunndatabse,
			siptx_req_ipsec_connect_struct * Transdata_var,__u8 dir,int Handover);
void change1layeripsecrules(siptx_req_ipsec_connect_struct * Transdatabase,struct xfrm_userpolicy_id *xpid, 
			char * srcbuf, char * dstbuf);
void change1layeripsecrules_xfrm(siptx_req_ipsec_connect_struct * Transdatabase,
			struct xfrm_userpolicy_id *xpid, char * srcbuf, char * dstbuf);
int compare_ip_for_prefix(int family1, char* ip1, int pfix1, int family2, char* ip2, int pfix2);
#endif

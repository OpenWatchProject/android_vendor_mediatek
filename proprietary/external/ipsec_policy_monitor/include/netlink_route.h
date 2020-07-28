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

#ifndef __NETLINK_ROUTE_H__
#define __NETLINK_ROUTE_H__

#include <string>
#include <map>
#include <stdint.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

typedef std::function<void(nlmsghdr *)> NetlinkDumpCallback;

static int XfrmOpenNetlinkSocket(int protocol);
static const char *actionName(uint16_t action);
static int XfrmPrefixParse(uint8_t *family, const char *prefix, void *address,
            int size, uint8_t *prefix_len);
static int XfrmSendNetlinkRequest(uint16_t action, uint16_t flags, iovec* iov, int iovlen,
            const NetlinkDumpCallback *callback);
static int XfrmRecvNetlinkAck(int sock);
extern void XfrmEarseInterfaceFromTable(const char* interface);
extern int ModifyIpSecMonRoute(uint16_t action, uint32_t table, const char* interface,
            const char* dstip);
extern uint32_t XfrmGetRouteTableForInterface(const char* interface);

#endif

/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#ifndef __NETLINK_EVENT_HANDLER_H__
#define __NETLINK_EVENT_HANDLER_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include <netinet/in.h>
#include <cstdlib>
#include <sysutils/NetlinkEvent.h>
#include <sysutils/NetlinkListener.h>

#include "NetAgentService.h"

#include "utils/String8.h"
#include "utils/Vector.h"

using ::android::String8;
using ::android::Vector;
/*****************************************************************************
 * Class NetlinkEventHandler
 *****************************************************************************/
typedef enum {
    ACTION_UNKNOWN            = 0,
    ACTION_ADDR_UPDATED       = 1,
    ACTION_ADDR_REMOVED       = 2,
} ACTION;

typedef struct {
    int interfaceId;
    int ipv6PrefixLength;
    char address[INET6_ADDRSTRLEN];
    NA_CMD cmd;
    ACTION action;
} NetlinkEventReqInfo;

typedef enum {
    NETLINK_RET_GENERIC_FAILURE = 0,
    NETLINK_RET_SUCCESS = 1,
    NETLINK_RET_REQ_INFO_CHANGED = 2,
    NETLINK_RET_REQ_INFO_NO_CHANGED = 3,
    NETLINK_RET_REQ_INFO_NEW = 4
} NETLINK_RET_STATUS;

class NetlinkEventHandler : public NetlinkListener {
    public:
        NetlinkEventHandler(NetAgentService *na, int listenerSocket, int format);
        virtual ~NetlinkEventHandler();

        int start(void);
        int stop(void);
        int getCommandType(void *obj, NA_CMD *cmd);
        int getAddress(void *obj, char *address);
        int getInterfaceId(void *obj, unsigned int *interfaceId);
        int getIpv6PrefixLength(void *obj, int *ipv6PrefixLength);
        int getAction(void *obj, ACTION *action);
        void removeLastReqInfo(int interfaceId);
        int setLastReqInfo(void *obj);
        int hasLastReqInfoChanged(void *obj);
        void freeNetlinkEventObj(void *obj);
        void freeNetlinkEventObj(NetlinkEventReqInfo *pReqInfo);
        static int stringToBinaryBase(char *str, int base, int *err);
        static const char *actionToString(ACTION action);
        static String8 reqInfoToString(NetlinkEventReqInfo *pReqInfo);
        int getTransIntfId(int interfaceId);

    protected:
        void onEvent(NetlinkEvent *evt);
        void enqueueReqInfo(NA_CMD cmd, char *address, int ipv6PrefixLength, int interfaceId, ACTION action);

    private:
        NetAgentService *m_pNetAgent;
        Vector<NetlinkEventReqInfo*> *m_pLastNetlinkEventReqInfo;
};
#endif /* __NETLINK_EVENT_HANDLER_H__ */

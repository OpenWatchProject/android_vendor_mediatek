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

#ifndef __CCMMI_INTERFACE_MANAGER_H__
#define __CCMMI_INTERFACE_MANAGER_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <cutils/log.h>

#include "util/RpFeatureOptionUtils.h"
#include "RpDataConnectionInfo.h"

#include <telephony/mtk_ril.h>
/*****************************************************************************
 * Defines
 *****************************************************************************/

#define RP_IMF_LOG_TAG "RP_DC_IFM"

#define MAX_CCMMI_LOCAL_IF_ID_COUNT 8
#define MAX_CCMMI_IF_INFO_COUNT 6

#define MAX_COUNT_HIGH_THROUGHPUT_PROFILE_ID 4
#define MAX_COUNT_IMS_PROFILE_ID 2

#define MIN_ID_HIGH_THROUGHPUT 0
#define MAX_ID_HIGH_THROUGHPUT 1
#define MIN_ID_OTHERS_THROUGHPUT 2
#define MAX_ID_OTHERS_THROUGHPUT 3
#define MIN_ID_IMS_THROUGHPUT 4
#define MAX_ID_IMS_THROUGHPUT 6

#define RIL_CLIENT_ID_FOR_MAL 1001
#define REQUEST_FROM_MAL 1
#define REQUEST_FOR_IPV4V6_FALLBACK 2

#define IMS_CHANNEL_OFFSET 11
#define IMS_EMERGENCY_CHANNEL_OFFSET 13
#define OTHERS_CHANNEL_OFFSET 7
#define OTHERS_CHANNEL_MAX_NUM 2

#define MD_VERSION_LR12 4
// MPS feature end

// CCMMI interface info
typedef enum {
    DATA_IF_INFO_PROFILE_ID,
    DATA_IF_INFO_PROTOCOL,
    DATA_IF_INFO_STATE,
    DATA_IF_INFO_INTERFACE_ID,
    DATA_IF_INFO_TOKEN_ID,
    DATA_IF_INFO_REQUEST_TYPE
}DataCcmmiIfInfo;

// CCMMI interface state
typedef enum  {
    DATA_INTERFACE_STATE_CONNECTING,
    DATA_INTERFACE_STATE_CONNECTED,
    DATA_INTERFACE_STATE_DISCONNECTED
}DataCcmmiIfState;

/*****************************************************************************
 * Class NetAgentService
 *****************************************************************************/

class RpDataInterfaceManager {
public:
    RpDataInterfaceManager() :
        slotId(-1) {
    }

    RpDataInterfaceManager(int sid) :
        slotId(sid) {
    }

    ~RpDataInterfaceManager() {
    }

    void initDataInterfaceManager();
    int  getInterfaceId(int profileId, int protocolId);
    int  updateForSetupDataRequest(int profileId, int protocolId, int state, int Ifid, int tokenId);
    int  updateForSetupDataRequestFromMal(int ifId, int tokenId);
    void updateTokenIdForSetupDataRequest(int oldTokenId, int newTokenId);
    void updateForSetupDataResponse(int tokenId, bool isResponseFail);
    void updateForDeactivateDataRequest(int cId, bool isFromMal);
    void updateForDeactAllDataCallRequest();
    void updateForDataCallListChangeUrc(Vector<RpDataConnectionInfo*>* newConnectionInfo);

private:
    // Private functions
    int  getLocalIndexId(int profileId, int protocolId);
    bool updateInterfaceIdStatusByInterfaceId(int ifId, int state);
    bool updateInterfaceIdStatusByIndex(int tokenId, int state);
    void updateStateToDisconnected(int localIndex);
    bool isIpv4v6Fallback(int localIndex, int profileId, int protocolId);
    bool isHighThroughputProfile(int profileId);
    bool isImsProfile(int profileId);
    bool isInterfaceAvailable(int index);
    // MPS feature start
    int  adjustInterfaceIdForMultiSIM(int ifId, int protocolId);
    // MPS feature end

public:
    const int HIGH_THROUGHPUT_PROFILE_ID[MAX_COUNT_HIGH_THROUGHPUT_PROFILE_ID] = {
        RIL_DATA_PROFILE_DEFAULT,
        RIL_DATA_PROFILE_TETHERED,
        RIL_DATA_PROFILE_VENDOR_HIPRI,
        RIL_DATA_PROFILE_VENDOR_ALL
    };

    const int IMS_PROFILE_ID[MAX_COUNT_IMS_PROFILE_ID] = {
        RIL_DATA_PROFILE_IMS,
        RIL_DATA_PROFILE_VENDOR_EMERGENCY
    };

private:
    int slotId;
    int mCcmmiInterfaceStatus[MAX_CCMMI_LOCAL_IF_ID_COUNT][MAX_CCMMI_IF_INFO_COUNT];
};
#endif /* __CCMMI_INTERFACE_MANAGER_H__ */

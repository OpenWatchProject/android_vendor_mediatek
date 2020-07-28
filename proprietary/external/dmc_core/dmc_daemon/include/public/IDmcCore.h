/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2019. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#ifndef __IDMC_CORE_H__
#define __IDMC_CORE_H__

#include "DmcDefs.h"
#include "SessionInfo.h"
#include "ObjId.h"
#include "KpiObj.h"
#include <stdint.h>

typedef DMC_RESULT_CODE (*EventCallback)(
        int32_t sessionId, KPI_GROUP_TYPE group, ObjId &objId, KpiObj &obj, void *param);

class IDmcCore {
public:
    virtual ~IDmcCore() {};
    virtual DMC_RESULT_CODE createSession(const SessionInfo *info, int32_t &sessionId) = 0;
    virtual DMC_RESULT_CODE closeSession(int32_t sessionId) = 0;
    virtual bool isValidSession(int32_t sessionId) = 0;
    virtual DMC_RESULT_CODE registerEventCallback(EventCallback callback, void *param) = 0;
    //virtual DMC_RESULT_CODE registerApmControlCallback(ApmControlCallback callback, void *param) = 0;
    virtual DMC_RESULT_CODE enableSubscription() = 0;
    virtual DMC_RESULT_CODE disableSubscription() = 0;
    virtual DMC_RESULT_CODE setPacketPayloadSize(const ObjId *oid, uint32_t size) = 0;
    virtual DMC_RESULT_CODE setpacketpayloadType(const ObjId *oid, OTA_PACKET_TYPE type) = 0;

    virtual DMC_RESULT_CODE queryEvent(int32_t session, const ObjId *oid, KpiObj &obj) = 0;
    virtual DMC_RESULT_CODE queryEventAsync(int32_t session, uint8_t simId, const ObjId *oid) = 0;

    virtual DMC_RESULT_CODE getSessionStats(int32_t session, KPI_GROUP_TYPE group, SessionStats_t &stats);
    virtual DMC_RESULT_CODE subscribe(const ObjId *oid);
    virtual DMC_RESULT_CODE unSubscribe(const ObjId *oid);
};

#endif

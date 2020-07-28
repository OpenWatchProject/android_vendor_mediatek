/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
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

#ifndef __RTC_DATA_ALLOW_CONTROLLER_H__
#define __RTC_DATA_ALLOW_CONTROLLER_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include <map>
#include <utils/Vector.h>
#include "RfxController.h"
#include "RfxTimer.h"
#include "RfxMessageId.h"
#include "RfxDataMessageId.h"
#include "RfxIntsData.h"
#include "RfxVoidData.h"

#include <string.h>
#include <string>
#include <vector>

using namespace std;

/*****************************************************************************
 * Class RpDataAllowController
 *****************************************************************************/
// Mapping to err cause: 4112, 4117 (multiple PS allow error)
#define RIL_E_OEM_MULTI_ALLOW_ERR RIL_E_OEM_ERROR_1

#define INVAILD_ID             (-1)
#define ALLOW_DATA             (1)
#define DISALLOW_DATA          (0)

// For dequeueNetworkRequest return value
#define FINISH_ALL_REQUEST  true
#define WAIT_NEXT_REQUEST   false

class RtcDataAllowController : public RfxController {
    RFX_DECLARE_CLASS(RtcDataAllowController);  // Required: declare this class

struct onDemandRequest{
    int type;
    int slotId;
};

public:
    RtcDataAllowController();
    virtual void enqueueNetworkRequest(int r_id, int slotId);
    virtual bool dequeueNetworkRequest(int r_id, int slotId);
    virtual void resendAllowData();

    virtual ~RtcDataAllowController();

// Override
protected:
    virtual void onInit();
    virtual void onDeinit();
    virtual bool onHandleRequest(const sp<RfxMessage>& message);
    virtual bool onHandleResponse(const sp<RfxMessage>& message);
    virtual bool onPreviewMessage(const sp<RfxMessage>& message);
    virtual bool onCheckIfResumeMessage(const sp<RfxMessage>& message);
    virtual void handleSetDataAllowRequest(const sp<RfxMessage>& request);
    virtual void handleSetDataAllowResponse(const sp<RfxMessage>& response);
    virtual void handleMultiAllowError(int activePhoneId);
    virtual bool onCheckIfRejectMessage(const sp<RfxMessage>& message,
            bool isModemPowerOff, int radioState);
    virtual void handleDataConnectionAttachRequest(const sp<RfxMessage>& message);
    virtual void handleDataConnectionDetachRequest(const sp<RfxMessage>& message);
    virtual void onAttachOrDetachDone(const sp<RfxMessage> message);
    virtual void onSetDataAllow(int slotId);
    virtual int checkRequestExistInQueue(int type, int slotId);
    virtual int checkTypeExistInQueue(int type);
    virtual void onPreferredChanged(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value);
    virtual void onHidlStateChanged(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value);
    virtual void onSimMeLockChanged(int slotId, RfxStatusKeyEnum key,
            RfxVariant old_value, RfxVariant value);

private:
    // Private functions
    bool isNeedSuspendRequest(const sp<RfxMessage>& message);
    bool checkDisallowingPeer();
    bool preprocessRequest(const sp<RfxMessage>& request);
    bool isPreferredDataMode();
    void setAllowDataSlot(bool allow, int slot);
    int getAllowDataSlot();

private:
    // Private members
    bool mDoingDataAllow;
    bool mReqDataAllow;
    int mDisallowingPeer;
    sp<RfxMessage> mLastAllowTrueRequest;
    Vector<int> nonDdsRequestQueue;
    int mIsPreferredDataMode;
    vector<struct onDemandRequest> mOnDemandQueue;
    int isMdSelfEdallow;
};
#endif

/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2015. All rights reserved.
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

#ifndef __RP_DATA_ALLOW_CONTROLLER_H__
#define __RP_DATA_ALLOW_CONTROLLER_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include <map>
#include <utils/Vector.h>
#include "RfxController.h"
#include "RfxTimer.h"
#include "RpDataController.h"

#define MODE_SWITCH_UNKNOWN    (-1)

// Define request type for setDataAllow()
typedef enum {
    DATA_ALLOW_REQUEST_TYPE_GSM_OR_CDMA = 1,   // send request to GSM RILD only or CDMA RILD only
    DATA_ALLOW_REQUEST_TYPE_GSM_AND_CDMA = 2  // send request to both GSM and CDMA RILD
} DataAllow_Request_Type;

typedef struct AttachAvailable {
    bool fromFw;// default = false
    bool fromOpCtrl;//default = true
} AttachAvailable;

// On demand request based on prefer data mode
typedef struct OnDemandRequest {
    int type;
    int slotId;
} OnDemandRequest;

/*****************************************************************************
 * Class RpDataAllowController
 *****************************************************************************/
class RpDataAllowController : public RfxController {
    RFX_DECLARE_CLASS(RpDataAllowController);  // Required: declare this class

public:
    RpDataAllowController();

    virtual ~RpDataAllowController();

    bool setPsRegistration(int slotId, bool regAllow);
    bool isAttachAvailable(int slotId);
    static bool isDataAllow(int slotId);
    // On demand request based on prefer data mode
    virtual void enqueueNetworkRequest(int requestId, int slotId);
    virtual void dequeueNetworkRequest(int requestId, int slotId);

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
    virtual void handleMultiAttachError(int activePhoneId);
    virtual void onPreferredDataSimChanged(RfxStatusKeyEnum key,
            RfxVariant old_value, RfxVariant value);
    virtual void onSetDataAllow(int slotId);
    // On demand request based on prefer data mode
    virtual void onHidlStateChanged(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value);
    virtual void onSimMeLockChanged(int slotId, RfxStatusKeyEnum key, RfxVariant old_value,
            RfxVariant value);
    virtual int checkRequestExistInQueue(int type, int slotId);
    virtual int checkRequestExistInQueue(int type);

private:
    bool isNeedSuspendRequest(const sp<RfxMessage>& message);
    bool checkDetachingPeer();
    bool preprocessRequest(const sp<RfxMessage>& request);
    void precheckDetach(const sp<RfxMessage>& request);
    void sendToGsm(const sp<RfxMessage>& message);
    void sendToC2k(const sp<RfxMessage>& message);
    bool setActivePsSlotIfNeeded(bool allow, int slot);
    bool needSetDataAllowGsmCdma(bool isAllowData, int slotId);
    void setRequestDataAllowType(int requestToken, DataAllow_Request_Type requestType);
    DataAllow_Request_Type getRequestDataAllowType(int requestToken);
    void removeRequestDataAllowType(int requestToken);
    bool precheckImsStatus(const sp<RfxMessage>& message);
    void onImsConfirmed(const sp<RfxMessage> message);
    void updateAttachSim(int slotId, bool allow);
    void onModeChange(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value);
    void handleSetPsRegistrationResponse(const sp<RfxMessage>& response);
    // Multi-PS Attach start
    void preCheckIfNeedDeactAllData(const sp<RfxMessage>& message);
    void onDeactAllDataConfirmed(const sp<RfxMessage> message);
    // Multi-PS Atach end
    void updateDataAllowStatus(int slotId, bool allow);
    void obtainAndEnqueueDataAllowRequst(int slotId, bool allow);
    // On demand request based on prefer data mode
    void resetDataAllowStatus();
    void onSimChanged(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant new_value);
    void attachCapabilitySim();

private:
    bool mDoingDataAllow;
    bool mReqDataAllow;
    int mDetachingPeer;
    int mModeStatus;
    std::map<int, DataAllow_Request_Type> mDataAllowTypeMap;
    sp<RfxMessage> mLastAllowTrueRequest;
    AttachAvailable* mAttachAvailabe;
    // On demand request based on prefer data mode
    std::vector<OnDemandRequest> mOnDemandQueue;
    bool mAttachDefaultPhoneWhenNoSim;
    int mSimCount;
};
#endif

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

#ifndef __RP_DATA_CONTROLLER_H__
#define __RP_DATA_CONTROLLER_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include <list>
#include <map>
#include <utils/Vector.h>
#include "RfxController.h"
#include "RfxTimer.h"
#include "RfxAction.h"
#include "RpDataConnectionInfo.h"
#include "RpDataInterfaceManager.h"

/*****************************************************************************
 * Class RpDataController
 *****************************************************************************/
class RpIratController;
class RpDcApnHandler;

#define MAX_SIM_NUM 4
#define SOCKET_BUF_LEN 256
#define MAX_QUEUE_LEN 16
#define INIT_QUEUE_VAL -99
#define INVALID_CID -1

extern "C"
void *dispatchRequestToNetd(void *arg);

class CcmniData {
public:
    CcmniData() :
        slotId(-1),
        queueHead(0),
        requestNum(0),
        dispatchMutex(PTHREAD_MUTEX_INITIALIZER),
        dispatchCond(PTHREAD_COND_INITIALIZER) {
    }

    CcmniData(int sid) :
        slotId(sid),
        queueHead(0),
        requestNum(0),
        dispatchMutex(PTHREAD_MUTEX_INITIALIZER),
        dispatchCond(PTHREAD_COND_INITIALIZER) {
    }

    ~CcmniData() {
    }

    void initRequestQueue() {
        queueHead = 0;
        requestNum = 0;
        for (int i = 0; i < MAX_QUEUE_LEN; i++) {
            requestQueue[i] = INIT_QUEUE_VAL;
        }
    }

    bool isQueueEmpty() {
        return (requestNum == 0);
    }

    bool isQueueFull() {
        return (requestNum >= MAX_QUEUE_LEN);
    }

    int queueSize() {
        return requestNum;
    }

    int enqueueRequest(int val) {
        if (!isQueueFull()) {
            requestQueue[(queueHead + requestNum) % MAX_QUEUE_LEN] = val;
            requestNum++;
            return 0;
        }
        return -1;
    }

    int popRequest(int *val) {
        if (isQueueEmpty()) {
            return -1;
        }
        (*val) = requestQueue[queueHead];
        requestQueue[queueHead] = INIT_QUEUE_VAL;
        queueHead = (queueHead + 1) % MAX_QUEUE_LEN;
        requestNum--;
        return 0;
    }

public:
    int slotId;
    int requestQueue[MAX_QUEUE_LEN];
    int queueHead;
    int requestNum;
    pthread_mutex_t dispatchMutex;
    pthread_cond_t dispatchCond;
};

class RpDataController : public RfxController {
    RFX_DECLARE_CLASS(RpDataController);  // Required: declare this class

public:
    RpDataController();

    virtual ~RpDataController();
    void updateIratStatus(bool status);
    // M: MPS feature
    void handleDeactAllDataCall(int slotId, const sp<RfxAction>& action);
    void configDcStateForPlmnSearch(const int plmnSearchAction, const sp<RfxAction>& action);

// Override
protected:
    void sendToGsm(const sp<RfxMessage>& message);
    void sendToC2kIfSupport(const sp<RfxMessage>& message);
    virtual void onInit();
    virtual void onDeinit();
    virtual bool onHandleRequest(const sp<RfxMessage>& message);
    virtual bool onHandleUrc(const sp<RfxMessage>& message);
    virtual bool onHandleResponse(const sp<RfxMessage>& message);

    virtual void handleSetupDataRequest(const sp<RfxMessage>& request);
    virtual void handleDeactivateDataRequest(const sp<RfxMessage>& request);
    virtual void handleGetDataCallListRequest(const sp<RfxMessage>& request);
    virtual void handleSetInitialAttachApnRequest(const sp<RfxMessage>& request);
    virtual void handleGetLastFailCauseRequest(const sp<RfxMessage>& request);
    virtual void handleSetDataProfileRequest(const sp<RfxMessage>& request);
    virtual void handlePdnDeactFailureUrc(const sp<RfxMessage>& message);
    virtual void handleRequestDefault(const sp<RfxMessage>& request);
    virtual void handleSetupDataResponse(const sp<RfxMessage>& response);
    virtual void handleDeactivateDataResponse(const sp<RfxMessage>& response);
    virtual void handleGetDataCallListResponse(const sp<RfxMessage>& response);
    virtual void handleSetInitialAttachApnResponse(const sp<RfxMessage>& response);
    virtual void handleGetLastFailCauseResponse(const sp<RfxMessage>& response);
    virtual void handleSetDataProfileResponse(const sp<RfxMessage>& response);
    virtual void handleDataCallListChangeUrc(const sp<RfxMessage>& message);
    // M: Data Framework - common part enhancement @{
    virtual void handleSyncDataSettingsToMdRequest(const sp<RfxMessage>& request);
    virtual void handleSyncDataSettingsToMdResponse(const sp<RfxMessage>& response);
    virtual void updateDataEnableProperty(int slotid, int iEnable);
    virtual void updateDefaultDataProperty(int slotid);
    // M: Data Framework - common part enhancement @}
    // SETUP_DATA_CALL RIL interface AOSP refactoring start
    virtual void handleRequestDefault(const sp<RfxMessage>& request, int oldTokenId);
    // SETUP_DATA_CALL RIL interface AOSP refactoring end

    // For suspend/resume mechanism start
    virtual bool onPreviewMessage(const sp<RfxMessage>& message);
    virtual bool onCheckIfResumeMessage(const sp<RfxMessage>& message);
    virtual bool onCheckIfRemoveSuspendedMessage(const sp<RfxMessage>& message);
    // For suspend/resume mechanism end
    // M: MPS feature
    virtual void handleDeactAllDataCallResponse(const sp<RfxMessage>& response);
    virtual void writeStringToParcel(Parcel *p, const char *s);

    // M: [VzW] Data Framework
    virtual void handlePcoStatusChangeUrc(const sp<RfxMessage>& message, int urcId);

    //[IMS Data] Handover - Handle SETUP_DATA_CALL from MAL
    virtual void handleSetupDataRequestAlt(const sp<RfxMessage>& request);
    virtual void handleSetupDataResponseAlt(const sp<RfxMessage>& response);

    virtual void handleGetImsDataCallInfoResponse(const sp<RfxMessage>& response);
    virtual void handleReuseImsDataResponse(const sp<RfxMessage>& response);
    virtual int getInterfaceId(int profileId, int protocolId);
    virtual bool onPreviewSetupDataCallMessage(const sp<RfxMessage>& message);
    virtual bool onCheckIfResumeSetupDataCallMessage(const sp<RfxMessage>& message);

    virtual void handleSetLinkCapacityReportingCriteriaRequest(const sp<RfxMessage>& request);
    virtual void handleSetLinkCapacityReportingCriteriaResponse(const sp<RfxMessage>& response);
    virtual void handleSetPreferredDataModem(const sp<RfxMessage>& message);
    virtual void handleSetPreferredDataModemResponse(const sp<RfxMessage>& response);
    virtual void handleStartKeepAlive(const sp<RfxMessage>& message);
    virtual void handleStartKeepAliveResponse(const sp<RfxMessage>& response);
    virtual void handleStopKeepAlive(const sp<RfxMessage>& message);
    virtual void handleStopKeepAliveResponse(const sp<RfxMessage>& response);

private:
    RILD_RadioTechnology_Group getPsType();
    void initIratController();
    void onCdmaCardTypeChanged(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value);
    void updateApnHandler(int cardType);
    bool isNoNeedSuspendRequest(int requestId);
    void sendDeactAllDataCallRequest(int slotId);
    void updateDataConnectionStatus(int oldCount, int newCount);
    friend void *::dispatchRequestToNetd(void *arg);
    void onFastDormancyTimerChanged(RfxStatusKeyEnum key, RfxVariant oldValue, RfxVariant newValue);
    void onServiceStateChanged(RfxStatusKeyEnum key, RfxVariant oldValue, RfxVariant newValue);
    bool getRoamingState();
    void setRoamingState(bool isRoaming);
    // ON demand request based on prefer data mode
    void onAllowedChanged(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value);
    void enqueueForPreferredDataMode(const sp<RfxMessage>& message);
    void dequeueForPreferredDataMode(const sp<RfxMessage>& message);
    bool isAllowedStatusIndependent(int nProfileId);
    int updateApnProfileId(int profileId, int supportedTypesBitmask);
    int getApnProfileId(int apnTypeId);

private:
    // Private members
    int mInterfaceId;
    String16 mApnName;
    // Splict IMS cache parameters to support setup data call simultaneously.
    // Otherwise, the variable will be over-written by another data call request.
    int mImsInterfaceId;
    int mImsProfileId;
    String16 mImsApnName;
    // Flag to know IMS PND is pending or not.
    bool mImsPdnPending;
    // Flag to know IMS PDN is found or not.
    bool mImsPdnFound;
    // Flag to block IMS setup data call request until REQUEST_GET_IMS_DATA_CALL_INFO back.
    bool mIsCheckPdnReuse;
    Vector<RpDataConnectionInfo*>* mDataInfoList;
    // SETUP_DATA_CALL RIL interface AOSP refactoring
    static RpDataInterfaceManager *sCcmmiIfManager[MAX_SIM_NUM];
    RpIratController *mIratController;
    RpDcApnHandler *mApnHandler;
    bool mIsDuringIrat; // 0 irat not happen, 1 irat happening.
    bool mIsRoaming;
    // M: MPS feature
    sp<RfxAction> mDeactAllDataCallConfirmAction;
    std::map<int, int> mMapProfileIdToken; // <ProfileId, Token>
    std::map<int, int> mMapCidProfileId; // <Cid, ProfileId>
};

#endif /* __RP_DATA_CONTROLLER_H__ */


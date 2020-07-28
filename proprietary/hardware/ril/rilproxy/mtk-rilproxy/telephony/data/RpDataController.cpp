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

/*****************************************************************************
 * Include
 *****************************************************************************/

#include "RpDataController.h"
#include "RpDataUtils.h"
#include "RfxStatusDefs.h"
#include "RpIratController.h"
#include "RpDataAllowController.h"
#include "RpDcApnHandler.h"
#include "RpSetupDataCallParam.h"
#include "modecontroller/RpCdmaLteModeController.h"
#include "util/RpFeatureOptionUtils.h"
#include <cutils/sockets.h>
#include <sys/un.h>
#include "socket_channel.h"
#include <errno.h>
#include <unistd.h>
#include "atchannel.h"
#include <libmtkrilutils.h>
#include <vendor/mediatek/hardware/netdagent/1.0/INetdagent.h>
#include <hidl/HidlSupport.h>

using android::hardware::hidl_string;
using vendor::mediatek::hardware::netdagent::V1_0::INetdagent;

#ifdef RFX_LOG_TAG
#undef RFX_LOG_TAG
#endif
#define RFX_LOG_TAG "RP_DC"

// M: Data Framework - common part enhancement
#include "RfxRilUtils.h"
#include <cutils/properties.h>
#define PROPERTY_DEFAULT_DATA_ICCID "persist.vendor.radio.data.iccid"
#define PROPERTY_MOBILE_DATA_ENABLE "persist.vendor.radio.mobile.data"

// FastDormancy status that sync from EM. @{
#define RP_EM_FASTDORMANCY_SYNC  "EM_FASTDORMANCY_SYNC"
#define RP_EM_FASTDORMANCY_TIMER_LENGTH 3
// @}

// SETUP_DATA_CALL RIL interface AOSP refactoring start
RpDataInterfaceManager *RpDataController::sCcmmiIfManager[MAX_SIM_NUM] = {0};
// SETUP_DATA_CALL RIL interface AOSP refactoring end

CcmniData *mCcmniData[MAX_SIM_NUM] = {0};
extern void *dispatchRequestToNetd(void *arg);

/*****************************************************************************
 * Class RfxDataController
 * The class is created if the slot is single mode, LWG or C,
 * During class life time always communicate with one modem, gsm or c2k.
 *****************************************************************************/

RFX_IMPLEMENT_CLASS("RpDataController", RpDataController, RfxController);

RpDataController::RpDataController() :
    mInterfaceId(-1), mApnName(""),
    mImsInterfaceId(-1), mImsProfileId(RIL_DATA_PROFILE_INVALID), mImsApnName(""),
    mImsPdnPending(false), mImsPdnFound(false), mIsCheckPdnReuse(false),
    mDataInfoList(NULL), mIratController(NULL), mApnHandler(NULL), mIsDuringIrat(false),
    mIsRoaming(false), mDeactAllDataCallConfirmAction(NULL) {
}

RpDataController::~RpDataController() {
}

// M: MPS feature
void RpDataController::handleDeactAllDataCall(int slotId, const sp<RfxAction>& action) {

    RFX_LOG_D(RFX_LOG_TAG, "handleDeactAllDataCall: slotId=%d ", slotId);

    mDeactAllDataCallConfirmAction = action;
    sendDeactAllDataCallRequest(slotId);
}

void RpDataController::onInit() {
    RfxController::onInit();  // Required: invoke super class implementation

    logD(RFX_LOG_TAG, "onInit");
    mDataInfoList = new Vector<RpDataConnectionInfo*>();

    const int request_id_list[] = {
            RIL_REQUEST_SETUP_DATA_CALL,  // 27
            RIL_REQUEST_DEACTIVATE_DATA_CALL,  // 41
            RIL_REQUEST_LAST_DATA_CALL_FAIL_CAUSE,  // 56
            RIL_REQUEST_DATA_CALL_LIST,  // 57
            RIL_REQUEST_SET_INITIAL_ATTACH_APN,  // 111
            RIL_REQUEST_SET_DATA_PROFILE,    // 128
            RIL_REQUEST_SYNC_DATA_SETTINGS_TO_MD,
            RIL_REQUEST_SET_PREFERRED_DATA_MODEM,
            RIL_REQUEST_START_KEEPALIVE, // 146
            RIL_REQUEST_STOP_KEEPALIVE,  // 147
            // M: MPS feature
            RIL_LOCAL_REQUEST_DEACTIVATE_ALL_DATA_CALL,
            RIL_LOCAL_C2K_REQUEST_DEACTIVATE_ALL_DATA_CALL,
            RIL_LOCAL_REQUEST_LAST_DATA_CALL_FAIL_CAUSE_ALT,
            RIL_LOCAL_REQUEST_SETUP_DATA_CALL_ALT,
            RIL_LOCAL_REQUEST_REUSE_IMS_DATA_CALL,
            RIL_LOCAL_REQUEST_GET_IMS_DATA_CALL_INFO,
            RIL_REQUEST_SET_LINK_CAPACITY_REPORTING_CRITERIA,
            };
    const int urc_id_list[] = {
            RIL_UNSOL_DATA_CALL_LIST_CHANGED,  // 1010
            RIL_LOCAL_GSM_UNSOL_PDN_DEACT_FAILURE_IND,
            RIL_UNSOL_PCO_DATA,
            RIL_UNSOL_PCO_DATA_AFTER_ATTACHED,
        };
    // SETUP_DATA_CALL RIL interface AOSP refactoring start
    sCcmmiIfManager[m_slot_id] = new RpDataInterfaceManager(m_slot_id);
    sCcmmiIfManager[m_slot_id]->initDataInterfaceManager();
    // SETUP_DATA_CALL RIL interface AOSP refactoring end

    if (NULL == mCcmniData[m_slot_id]) {
        int result;
        pthread_attr_t attr;
        PthreadPtr threadFunction = dispatchRequestToNetd;
        pthread_t tidNetdLooper;

        mCcmniData[m_slot_id] = new CcmniData(m_slot_id);
        mCcmniData[m_slot_id]->initRequestQueue();

        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        result = pthread_create(&tidNetdLooper, &attr,
                threadFunction, (void *) &m_slot_id);
        pthread_attr_destroy(&attr);
        if (result < 0) {
            logD(RFX_LOG_TAG, "pthread_create failed result:%d", result);
        } else {
            logD(RFX_LOG_TAG, "pthread_create OK result:%d", result);
            logD(RFX_LOG_TAG, "main: process id %d, thread id = %ld", getpid(), pthread_self());
            logD(RFX_LOG_TAG, "mNetdLooperTid = %ld", tidNetdLooper);
        }

        // Add 'deny' cmd to netd to reset if TRM happens in the middle of 'allow' cmd.
        configDcStateForPlmnSearch(1, NULL);
    }

    // register request & URC id list
    // NOTE. one id can only be registered by one controller
    registerToHandleRequest(request_id_list,
            sizeof(request_id_list) / sizeof(int));
    registerToHandleUrc(urc_id_list, sizeof(urc_id_list) / sizeof(int));

    initIratController();
    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_CDMA_CARD_TYPE,
            RfxStatusChangeCallback(this, &RpDataController::onCdmaCardTypeChanged));
    int cardType = getStatusManager()->getIntValue(RFX_STATUS_KEY_CDMA_CARD_TYPE);
    updateApnHandler(cardType);
    // Register for FastDormancy timer changes.
    getStatusManager()->registerStatusChanged(
            RFX_STATUS_KEY_TELEPHONY_ASSISTANT_STATUS,
            RfxStatusChangeCallback(this, &RpDataController::onFastDormancyTimerChanged));
    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_SERVICE_STATE,
            RfxStatusChangeCallback(this, &RpDataController::onServiceStateChanged));
    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_SLOT_ALLOW,
            RfxStatusChangeCallback(this, &RpDataController::onAllowedChanged));
}

void RpDataController::onDeinit() {
    logD(RFX_LOG_TAG, "onDeinit");
    RpDataUtils::freeDataList(mDataInfoList);
    mDataInfoList = NULL;
    getStatusManager()->unRegisterStatusChanged(RFX_STATUS_KEY_CDMA_CARD_TYPE,
            RfxStatusChangeCallback(this, &RpDataController::onCdmaCardTypeChanged));
    getStatusManager()->unRegisterStatusChanged(RFX_STATUS_KEY_SERVICE_STATE,
            RfxStatusChangeCallback(this, &RpDataController::onServiceStateChanged));
    getStatusManager()->unRegisterStatusChanged(RFX_STATUS_KEY_SLOT_ALLOW,
            RfxStatusChangeCallback(this, &RpDataController::onAllowedChanged));
    if (mIratController != NULL) {
        RFX_OBJ_CLOSE(mIratController);
        mIratController = NULL;
    }
    if (mApnHandler != NULL) {
        RFX_OBJ_CLOSE(mApnHandler);
        mApnHandler = NULL;
    }
    RfxController::onDeinit();
    // SETUP_DATA_CALL RIL interface AOSP refactoring
    delete sCcmmiIfManager[m_slot_id];
    sCcmmiIfManager[m_slot_id] = NULL;
}

bool RpDataController::onHandleRequest(const sp<RfxMessage>& message) {
    RFX_LOG_D(RFX_LOG_TAG, "Handle request %s", RpDataUtils::requestToString(message->getId()));

    switch (message->getId()) {
    // common commands can send to md1(gsm)/md3(c2k) both.
    case RIL_REQUEST_SETUP_DATA_CALL:
        handleSetupDataRequest(message);
        break;
    case RIL_REQUEST_DEACTIVATE_DATA_CALL:
        handleDeactivateDataRequest(message);
        break;
    case RIL_REQUEST_DATA_CALL_LIST:
        handleGetDataCallListRequest(message);
        break;
    case RIL_REQUEST_LAST_DATA_CALL_FAIL_CAUSE:
        handleGetLastFailCauseRequest(message);
        break;
    case RIL_REQUEST_SET_INITIAL_ATTACH_APN:
        handleSetInitialAttachApnRequest(message);
        break;
    case RIL_REQUEST_SET_DATA_PROFILE:
        handleSetDataProfileRequest(message);
        break;
    case RIL_REQUEST_SYNC_DATA_SETTINGS_TO_MD:
        handleSyncDataSettingsToMdRequest(message);
        break;
    case RIL_REQUEST_SET_PREFERRED_DATA_MODEM:
        handleSetPreferredDataModem(message);
        break;
    case RIL_REQUEST_START_KEEPALIVE:
        handleStartKeepAlive(message);
        break;
    case RIL_REQUEST_STOP_KEEPALIVE:
        handleStopKeepAlive(message);
        break;
    case RIL_LOCAL_REQUEST_LAST_DATA_CALL_FAIL_CAUSE_ALT:
        handleGetLastFailCauseRequest(message);
        break;
    case RIL_LOCAL_REQUEST_SETUP_DATA_CALL_ALT:
        handleSetupDataRequestAlt(message);
        break;
    case RIL_REQUEST_SET_LINK_CAPACITY_REPORTING_CRITERIA:
        handleSetLinkCapacityReportingCriteriaRequest(message);
        break;
    default:
        logD(RFX_LOG_TAG, "unknown request, ignore!");
        break;
    }
    return true;
}

bool RpDataController::onHandleResponse(const sp<RfxMessage>& message) {
    RFX_LOG_D(RFX_LOG_TAG, "Handle response %s.", RpDataUtils::requestToString(message->getId()));

    switch (message->getId()) {
    case RIL_REQUEST_SETUP_DATA_CALL:
        handleSetupDataResponse(message);
        break;
    case RIL_REQUEST_DEACTIVATE_DATA_CALL:
        handleDeactivateDataResponse(message);
        break;
    case RIL_REQUEST_DATA_CALL_LIST:
        handleGetDataCallListResponse(message);
        break;
    case RIL_REQUEST_LAST_DATA_CALL_FAIL_CAUSE:
        handleGetLastFailCauseResponse(message);
        break;
    case RIL_REQUEST_SET_INITIAL_ATTACH_APN:
        handleSetInitialAttachApnResponse(message);
        break;
    case RIL_LOCAL_REQUEST_SETUP_DATA_CALL_ALT:
        handleSetupDataResponseAlt(message);
        break;
    case RIL_LOCAL_REQUEST_LAST_DATA_CALL_FAIL_CAUSE_ALT:
        handleGetLastFailCauseResponse(message);
        break;
    case RIL_REQUEST_SYNC_DATA_SETTINGS_TO_MD:
        handleSyncDataSettingsToMdResponse(message);
        break;
    case RIL_REQUEST_SET_PREFERRED_DATA_MODEM:
        handleSetPreferredDataModemResponse(message);
        break;
    case RIL_REQUEST_START_KEEPALIVE:
        handleStartKeepAliveResponse(message);
        break;
    case RIL_REQUEST_STOP_KEEPALIVE:
        handleStopKeepAliveResponse(message);
        break;
    case RIL_REQUEST_SET_DATA_PROFILE:
        handleSetDataProfileResponse(message);
        break;
    // M: MPS feature
    case RIL_LOCAL_REQUEST_DEACTIVATE_ALL_DATA_CALL:
    case RIL_LOCAL_C2K_REQUEST_DEACTIVATE_ALL_DATA_CALL:
        handleDeactAllDataCallResponse(message);
        break;
    case RIL_LOCAL_REQUEST_GET_IMS_DATA_CALL_INFO:
        handleGetImsDataCallInfoResponse(message);
        break;
    case RIL_LOCAL_REQUEST_REUSE_IMS_DATA_CALL:
        handleReuseImsDataResponse(message);
        break;
    case RIL_REQUEST_SET_LINK_CAPACITY_REPORTING_CRITERIA:
        handleSetLinkCapacityReportingCriteriaResponse(message);
        break;
    default:
        logD(RFX_LOG_TAG, "unknown response, ignore!");
        break;
    }
    return true;
}

bool RpDataController::onHandleUrc(const sp<RfxMessage>& message) {
    logD(RFX_LOG_TAG, "Handle URC %s", RpDataUtils::urcToString(message->getId()));

    switch (message->getId()) {
    case RIL_UNSOL_DATA_CALL_LIST_CHANGED: {
        handleDataCallListChangeUrc(message);
        break;
    }
    case RIL_LOCAL_GSM_UNSOL_PDN_DEACT_FAILURE_IND: {
        handlePdnDeactFailureUrc(message);
        break;
    }
    case RIL_UNSOL_PCO_DATA:
    case RIL_UNSOL_PCO_DATA_AFTER_ATTACHED: {
        handlePcoStatusChangeUrc(message, message->getId());
        break;
    }
    default:
        logD(RFX_LOG_TAG, "unknown urc, ignore!");
        break;
    }
    return true;
}

int RpDataController::getInterfaceId(int profileId, int protocolId) {
    // Data FWK will not pass interface id after refactoring
    return sCcmmiIfManager[m_slot_id]->getInterfaceId(profileId, protocolId);
}

void RpDataController::handleSetupDataRequest(const sp<RfxMessage>& request) {
    // update data connection info
    // request message to create a connection, record apn & interfaceId.
    RpSetupDataCallParam *pParam = new RpSetupDataCallParam(request, false);
    if (pParam == NULL) {
        responseToRilj(RfxMessage::obtainResponse(RIL_E_NO_MEMORY, request));
        return;
    }

    // SETUP_DATA_CALL RIL interface AOSP refactoring start
    // The range of values for mInterfaceId is 1~8 for each sim.
    // (according to profileId of request APN)
    //
    // mInterfaceId is mapping to CCMMI interface as below:
    // CCMMI1: Internet
    // CCMMI2: Tethering
    // CCMMI3: Other(MMS, ADMIN, AKA)
    // CCMMI4: Other(MMS, ADMIN, AKA)
    // CCMMI5: IMS
    // CCMMI6: IMS emergency
    // CCMMI7: SS XCAP(IMS)
    // CCMMI8: eMBMS
    //
    // For example, AP request IMS APN, profileId delivered as 2.
    // DataInterfaceManager will assign mInterfaceId=5 according to this profileId.
    // And DataInterfaceManager update status and related information(profileId, protocol..)
    // in internal table with index=4

    mApnName = pParam->apnName;
    int protocolId = pParam->getProtocolId();
    int profileId = updateApnProfileId(pParam->getProfileId(), pParam->getApnTypesBitmap());

    String16 tempIdString;
    char strTemp[10] = {0};
    Parcel* newParcel = NULL;

    // generate new parcel with interfaceId
    RILD_RadioTechnology_Group psType = RpDataUtils::getPsType(this);
    sp<RfxMessage> newMsg = RfxMessage::obtainRequest(psType, request->getId(), request, false);

    // Data FWK will not pass interface id after refactoring
    mInterfaceId = getInterfaceId(profileId, protocolId);
    if ((mInterfaceId <= 0) ||
            (sCcmmiIfManager[m_slot_id]->updateForSetupDataRequest(profileId,
            protocolId, DATA_INTERFACE_STATE_CONNECTING, mInterfaceId, newMsg->getToken()) < 0)) {
         logE(RFX_LOG_TAG, "[handleSetupDataRequest] can't find available interface Id");
         responseToRilj(RfxMessage::obtainResponse(RIL_E_GENERIC_FAILURE, request));
         goto exit;
    }

    RFX_LOG_I(RFX_LOG_TAG, "[%s] apnName: %s, interfaceId: %d, profile: %d",
            __FUNCTION__, String8(mApnName).string(), mInterfaceId, profileId);

    // Use channel DATA2 (10) to "reuse" IMS PDN instead of data channel, otherwise,
    // still use data channel to setup data call.
    if (RIL_DATA_PROFILE_IMS == profileId && RpDataUtils::isOP12Support()) {
        mIsCheckPdnReuse = false;
        mImsPdnPending = false;
        if (mImsPdnFound) {
            mImsApnName = mApnName;
            mImsInterfaceId = mInterfaceId;
            newMsg = RfxMessage::obtainRequest(
                    psType,
                    RIL_LOCAL_REQUEST_REUSE_IMS_DATA_CALL,
                    request,
                    false);
            logD(RFX_LOG_TAG, "record mImsApnName: %s mImsInterfaceId: %d",
                    String8(mImsApnName).string(), mImsInterfaceId);
        }
    }

    // for type convert
    sprintf(strTemp, "%d", mInterfaceId);
    tempIdString = String16(strTemp);

    newParcel = newMsg->getParcel();
    newParcel->writeInt32(pParam->contentLength + 1);  // conent length + 1 for interface Id
    newParcel->writeString16(pParam->radioTechnology);
    newParcel->writeString16(String16((std::to_string(profileId)).c_str()));
    newParcel->writeString16(mApnName);
    newParcel->writeString16(pParam->user);
    newParcel->writeString16(pParam->password);
    newParcel->writeString16(pParam->authType);
    if (strcmp(String8(pParam->setupDataCallVersion).string(),
            SETUP_DATA_CALL_VERSION_1_4) == 0) {
        // No roaming state parameter in V1.4 HIDL interface
        RFX_LOG_I(RFX_LOG_TAG, "[%s] roaming state: %d", __FUNCTION__, getRoamingState());
        newParcel->writeString16(getRoamingState() ? pParam->roamingProtocol : pParam->protocol);
    } else {
        newParcel->writeString16(pParam->protocol);
    }
    newParcel->writeString16(pParam->roamingProtocol);
    newParcel->writeString16(pParam->supportedApnTypesBitmap);
    newParcel->writeString16(pParam->bearerBitmap);
    newParcel->writeString16(pParam->modemCognitive);
    newParcel->writeString16(pParam->mtu);
    newParcel->writeString16(pParam->mvnoType);
    newParcel->writeString16(pParam->mvnoMatchData);
    newParcel->writeString16(pParam->roamingAllowed);
    newParcel->writeString16(tempIdString);

    logD(RFX_LOG_TAG, "record mApnName: %s mInterfaceId: %d", String8(mApnName).string(), mInterfaceId);

    handleRequestDefault(newMsg, newMsg->getToken());
    // SETUP_DATA_CALL RIL interface AOSP refactoring end

exit:
    if (pParam != NULL) {
        delete pParam;
    }
}

void RpDataController::handleDeactivateDataRequest(const sp<RfxMessage>& request) {
    // update data connection info
    // this is a request message to release a connection, should always success.
    Parcel* parcel = request->getParcel();
    int oldCount = 0;
    int newCount = 0;

    // SETUP_DATA_CALL RIL interface AOSP refactoring start
    // check client id of request message
    bool isFromMal = (RIL_CLIENT_ID_FOR_MAL == request->getClientId());
    // SETUP_DATA_CALL RIL interface AOSP refactoring end

    oldCount = mDataInfoList->size();
    parcel->readInt32();  // conent length
    String16 idString = parcel->readString16();  // interfaceId string
    int id = atoi(String8(idString).string());
    logD(RFX_LOG_TAG, "deactive interfaceId:%d", id);
    for (int i = 0; i < oldCount; i++) {
        RpDataConnectionInfo* info = mDataInfoList->itemAt(i);
        if (info->interfaceId == id) {
            delete info;
            mDataInfoList->removeAt(i);
            RFX_LOG_I(RFX_LOG_TAG, "remove interfaceId:%d from list.", id);
            break;
        }
    }
    newCount = mDataInfoList->size();
    updateDataConnectionStatus(oldCount, newCount);

    // SETUP_DATA_CALL RIL interface AOSP refactoring start
    sCcmmiIfManager[m_slot_id]->updateForDeactivateDataRequest(id, isFromMal);
    // SETUP_DATA_CALL RIL interface AOSP refactoring end

    handleRequestDefault(request);
}

void RpDataController::handleGetDataCallListRequest(const sp<RfxMessage>& request) {
    handleRequestDefault(request);
}

void RpDataController::handleSetInitialAttachApnRequest(const sp<RfxMessage>& request) {
    Parcel* parcel = request->getParcel();
    RpDataUtils::storeIaProperty(getSlotId(), parcel->readString16());
    // When set initial attach apn, if mode switch happens
    // before response and RpCdmaLteDataController changes
    // to RpDataController, it will response twice to RILJ
    // which cause the issue. So we:
    // 1. Send set initial attach apn request to both GSM and C2K.
    // 2. Only response GSM to RILJ.
    // sync attach APN to C2K also which will be sync to MD1
    // when IRAT, and MD1 will use MD3 sync value as first priority.
    if (mApnHandler != NULL) {
        sp<RfxMessage> newRequest = mApnHandler->onSetInitialAttachApn(request);
        sendToGsm(newRequest);
        sendToC2kIfSupport(newRequest);
    } else {
        sendToGsm(request);
        sendToC2kIfSupport(request);
    }
}

void RpDataController::handleGetLastFailCauseRequest(const sp<RfxMessage>& request) {
    handleRequestDefault(request);
}

void RpDataController::handleSetDataProfileRequest(const sp<RfxMessage>& request) {
    sendToGsm(request);
    sendToC2kIfSupport(request);
}

void RpDataController::handleSetupDataResponse(const sp<RfxMessage>& response) {
    Parcel* parcel = response->getParcel();

    if (response->getError() != RIL_E_SUCCESS) {
        logD(RFX_LOG_TAG, "setupdata response fail!");

        // SETUP_DATA_CALL RIL interface AOSP refactoring start
        sCcmmiIfManager[m_slot_id]->updateForSetupDataResponse(response->getToken(), true);
        // SETUP_DATA_CALL RIL interface AOSP refactoring end

        responseToRilj(response);
        return;
    }

    // update data connection info
    // this is a response message to create a connection, when success update.
    int oldCount = mDataInfoList->size();
    int newCount = 0;

    Vector<RpDataConnectionInfo*>* newList = RpDataUtils::parseDataConnectionInfo(parcel);
    RpDataConnectionInfo* p = newList->itemAt(0);
    // interface Id of rild index begin is 0, setupdata id delivered begin as cid + 1.
    if (p->interfaceId == mInterfaceId - 1) {
        p->apn = mApnName;
        logD(RFX_LOG_TAG, "update cid %d apn to %s", p->interfaceId, String8(p->apn).string());
        p->dump();
    }
    mDataInfoList->add(p);
    newCount = mDataInfoList->size();
    updateDataConnectionStatus(oldCount, newCount);

    int nCid = p->interfaceId;
    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] cid = %d", m_slot_id, __FUNCTION__, nCid);
    // SETUP_DATA_CALL RIL interface AOSP refactoring start
    if (nCid != INVALID_CID) {
        sCcmmiIfManager[m_slot_id]->updateForSetupDataResponse(response->getToken(), false);
    } else {
        sCcmmiIfManager[m_slot_id]->updateForSetupDataResponse(response->getToken(), true);
    }
    // SETUP_DATA_CALL RIL interface AOSP refactoring end

    // For preferred data mode
    if (RpDataUtils::isPreferredDataMode()) {
        std::map<int,int>::iterator it = mMapProfileIdToken.begin();
        for (; it != mMapProfileIdToken.end(); ++it) {
            if (it->second == response->getToken()) {
                break;
            }
        }
        if (it != mMapProfileIdToken.end()) {
            RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] profileId = %d", m_slot_id, __FUNCTION__, it->first);
            if (nCid != INVALID_CID) {
                mMapCidProfileId[nCid] = it->first;
            }
            mMapProfileIdToken.erase(it->first);
        }
    }

    delete newList;

    responseToRilj(response);
}

void RpDataController::handleReuseImsDataResponse(const sp<RfxMessage>& response) {
    Parcel* parcel = response->getParcel();

    sp<RfxMessage> newResponse = RfxMessage::obtainResponse(
            response->getSlotId(),
            response->getPId(),
            response->getPToken(),
            RIL_REQUEST_SETUP_DATA_CALL,
            response->getToken(),
            response->getError(),
            response->getSource(),
            RpDataUtils::createResponseParcelFromMessage(response),
            response->getPTimeStamp(),
            response->getRilToken());

    if (response->getError() != RIL_E_SUCCESS) {
        logD(RFX_LOG_TAG, "setup IMS data response fail!");
        responseToRilj(newResponse);
        return;
    }

    logD(RFX_LOG_TAG, "handleReuseImsDataResponse");

    // update data connection info
    // this is a response message to create a connection, when success update.
    int oldCount = mDataInfoList->size();
    int newCount = 0;

    Vector<RpDataConnectionInfo*>* newList = RpDataUtils::parseDataConnectionInfo(parcel);
    RpDataConnectionInfo* p = newList->itemAt(0);
    // interface Id of rild index begin is 0, setupdata id delivered begin as cid + 1.
    if (p->interfaceId == mImsInterfaceId - 1) {
        p->apn = mImsApnName;
        logD(RFX_LOG_TAG, "update cid %d apn to %s", p->interfaceId, String8(p->apn).string());
        p->dump();
    }
    mDataInfoList->add(p);
    newCount = mDataInfoList->size();
    updateDataConnectionStatus(oldCount, newCount);
    delete newList;

    responseToRilj(newResponse);
}

void RpDataController::handleGetImsDataCallInfoResponse(const sp<RfxMessage>& response) {

    Parcel* parcel = response->getParcel();
    int length = parcel->readInt32();  // conent length
    mIsCheckPdnReuse = true;

    if (length == 0 || response->getError() != RIL_E_SUCCESS) {
        logD(RFX_LOG_TAG, "No data call info found");
        mImsPdnFound = false;
    } else {
        // Check PDN info if required
        mImsPdnFound = true;
        logD(RFX_LOG_TAG, "Found IMS data call info, reuse it");
    }
}

void RpDataController::handleDeactivateDataResponse(const sp<RfxMessage>& response) {
    responseToRilj(response);
}

void RpDataController::handleGetDataCallListResponse(const sp<RfxMessage>& response) {
    responseToRilj(response);
}

void RpDataController::handleSetInitialAttachApnResponse(const sp<RfxMessage>& response) {
    if (response->getSource() == RADIO_TECH_GROUP_GSM) {
        // RpCdmaLteDataController will send the request to GSM and C2K
        // If mode switch happens before response, RpDataController will
        // got two response from GSM and C2K, in this case
        // only response GSM to RILJ and ignore C2K response
        responseToRilj(response);
    }
}

void RpDataController::handleGetLastFailCauseResponse(const sp<RfxMessage>& response) {
    responseToRilj(response);
}

void RpDataController::handleSetDataProfileResponse(const sp<RfxMessage>& response) {
    int reqId = response->getId();
    int source = response->getSource();
    sp<RfxMessage> outResponse = sp<RfxMessage>(NULL);

    if (!RpFeatureOptionUtils::isC2kSupport()) {
        responseToRilj(response);
    } else {
        ResponseStatus status = preprocessResponse(response, outResponse);
        //sync apn table request will send to gsm and c2k,
        //when the UE not support c slot or c slot change,preprocessResponse will
        //return RESPONSE_STATUS_HAVE_MATCHED_WITH_C_CHANGED status.
        if (status == RESPONSE_STATUS_HAVE_MATCHED ||
                status == RESPONSE_STATUS_HAVE_MATCHED_WITH_C_CHANGED) {
            RFX_LOG_I(RFX_LOG_TAG, "Response matched status=%d", status);
            responseToRilj(response);
        } else if (status == RESPONSE_STATUS_NO_MATCH_AND_SAVE) {
            RFX_LOG_I(RFX_LOG_TAG, "Response saved");
        } else if (status == RESPONSE_STATUS_ALREADY_SAVED) {
            logD(RFX_LOG_TAG, "Response have saved -> ignore");
        } else if (status == RESPONSE_STATUS_HAVE_BEEN_DELETED) {
            logD(RFX_LOG_TAG, "Response have time out! REQ=%s src=%s token=%d",
                    RpDataUtils::requestToString(reqId),
                    RpDataUtils::radioGroupToString(source),
                    response->getPToken());
        } else {
            logD(RFX_LOG_TAG, "Response misc error -> ignore, status=%d", status);
        }
    }
}

// M: MPS feature
void RpDataController::handleDeactAllDataCallResponse(const sp<RfxMessage>& response) {
    RFX_UNUSED(response);

    if (mDeactAllDataCallConfirmAction != NULL) {
        mDeactAllDataCallConfirmAction->act();
    }
    mDeactAllDataCallConfirmAction = NULL;
}

void RpDataController::handleDataCallListChangeUrc(const sp<RfxMessage>& message) {
    Parcel* parcel = message->getParcel();

    // update data connection info
    // urc data connection list is changed, sync info totally.
    int oldCount = 0;
    int newCount = 0;
    Vector<RpDataConnectionInfo*>* newList = RpDataUtils::parseDataConnectionInfo(parcel);
    oldCount = mDataInfoList->size();
    RpDataUtils::syncDataConnectionApn(mDataInfoList, newList);

    // SETUP_DATA_CALL RIL interface AOSP refactoring start
    sCcmmiIfManager[m_slot_id]->updateForDataCallListChangeUrc(newList);
    // SETUP_DATA_CALL RIL interface AOSP refactoring end

    RpDataUtils::freeDataList(mDataInfoList);
    mDataInfoList = newList;
    newCount = mDataInfoList->size();
    updateDataConnectionStatus(oldCount, newCount);

    responseToRilj(message);
}

void RpDataController::handlePdnDeactFailureUrc(const sp<RfxMessage>& urc) {
    Parcel *p = urc->getParcel();
    int skip = p->readInt32();
    int cause = p->readInt32();
    RFX_UNUSED(skip);
    bool forceNotify = false;

    if (cause == CME_LAST_PDN_NOT_ALLOW) {
        forceNotify = true;
    } else {
        forceNotify = false;
    }

    getStatusManager()->setIntValue(RFX_STATUS_KEY_PDN_DEACT_ERR_CAUSE, cause,
            forceNotify);
}

void RpDataController::handleRequestDefault(const sp<RfxMessage>& request) {
    RILD_RadioTechnology_Group psType = getPsType();

    sp<RfxMessage> message = RfxMessage::obtainRequest(psType,
                                                        request->getId(),
                                                        request,
                                                        true);
    if (RpDataUtils::isPreferredDataMode()
            && message->getId() == RIL_REQUEST_DEACTIVATE_DATA_CALL) {
        dequeueForPreferredDataMode(message);

        Parcel* parcel = message->getParcel();
        message->resetParcelDataStartPos();
        parcel->readInt32();  // conent length
        String16 idString = parcel->readString16();  // interfaceId string
        message->resetParcelDataStartPos();
        int id = atoi(String8(idString).string());
        if (id < 0) {
            logD(RFX_LOG_TAG, "Preferred data mode and id < 0, not send to MD");
            responseToRilj(RfxMessage::obtainResponse(RIL_E_SUCCESS, request));
            return;
        }
    }
    requestToRild(message);
    logD(RFX_LOG_TAG, "Send request %s to %s",
            RpDataUtils::requestToString(message->getId()),
            RpDataUtils::radioGroupToString(psType));
}

// SETUP_DATA_CALL RIL interface AOSP refactoring start
void RpDataController::handleRequestDefault(const sp<RfxMessage>& request, int oldTokenId) {
    RILD_RadioTechnology_Group psType = getPsType();

    sp<RfxMessage> message = RfxMessage::obtainRequest(psType,
                                                        request->getId(),
                                                        request,
                                                        true);

    //update new token id for sync to response
    sCcmmiIfManager[m_slot_id]->updateTokenIdForSetupDataRequest(oldTokenId, message->getToken());

    if (RpDataUtils::isPreferredDataMode()
            && message->getId() == RIL_REQUEST_SETUP_DATA_CALL) {
        enqueueForPreferredDataMode(message);
    } else {
        requestToRild(message);
        logD(RFX_LOG_TAG, "Send request %s to %s",
                RpDataUtils::requestToString(request->getId()),
                RpDataUtils::radioGroupToString(psType));
    }
}
// SETUP_DATA_CALL RIL interface AOSP refactoring end

// M: Data Framework - common part enhancement @{
void RpDataController::handleSyncDataSettingsToMdRequest(const sp<RfxMessage>& request) {
    // For sync the data settings.
    Parcel* parcel = request->getParcel();

    parcel->readInt32();  // conent length
    int dataEnabled = parcel->readInt32();  // data enable settings;
    int dataRoaming = parcel->readInt32();  // data roaming settings
    int defaultDataSelected = SKIP_DATA_SETTINGS; // default data selected

    if (parcel->dataAvail() > 0) {  // For telephony framework backward comparable.
        defaultDataSelected = parcel->readInt32();
    }

    logD(RFX_LOG_TAG,
         "SyncDataSettingsToMd dataEnabled: [%d], dataRoaming: [%d], defaultDataSelected: [%d]",
         dataEnabled, dataRoaming, defaultDataSelected);

    RfxDataSettings datasettings = getStatusManager()->getDataSettingsValue(RFX_STATUS_KEY_DATA_SETTINGS);

    if (dataEnabled != SKIP_DATA_SETTINGS) {
        datasettings.setDataEnabled(dataEnabled);
        updateDataEnableProperty(m_slot_id, dataEnabled);
    }

    if (dataRoaming != SKIP_DATA_SETTINGS) {
        datasettings.setDataRoaming(dataRoaming);
    }

    if (defaultDataSelected != SKIP_DATA_SETTINGS) {
        getNonSlotScopeStatusManager()->setIntValue(RFX_STATUS_KEY_DEFAULT_DATA_SIM,
                defaultDataSelected);
        updateDefaultDataProperty(defaultDataSelected);
    }

    getStatusManager()->setDataSettingsValue(RFX_STATUS_KEY_DATA_SETTINGS,
        datasettings);

    RFX_LOG_I(RFX_LOG_TAG, "setDataSettingsValue: %d, %d, %d\n",
        datasettings.getDataEnabled(), datasettings.getDataRoaming(), defaultDataSelected);

    sendToGsm(request);
}

void RpDataController::handleSetPreferredDataModem(const sp<RfxMessage>& message) {
    Parcel* parcel = message->getParcel();
    message->resetParcelDataStartPos();
    parcel->readInt32();  // pass length
    int preferredModemId = parcel->readInt32();  // 0 for slot0, 1 for slot1.
    RFX_LOG_I(RFX_LOG_TAG, "handleSetPreferredDataModem: preferredModemId = %d",
            preferredModemId);
    RIL_Errno result = RIL_E_SUCCESS;
    if (preferredModemId < 0 || preferredModemId >= RFX_SLOT_COUNT) {
        result = RIL_E_INVALID_ARGUMENTS;
    } else {
        // Update flags
        getNonSlotScopeStatusManager()->setIntValue(RFX_STATUS_KEY_PREFERRED_DATA_SIM,
                preferredModemId, true);
    }

    // Send response to RILJ
    sp<RfxMessage> voidResponse = RfxMessage::obtainResponse(result, message);
    responseToRilj(voidResponse);
}

void RpDataController::handleSetPreferredDataModemResponse(const sp<RfxMessage>& response) {
    responseToRilj(response);
}

void RpDataController::handleStartKeepAlive(const sp<RfxMessage>& message) {
    requestToRild(message);
}

void RpDataController::handleStartKeepAliveResponse(const sp<RfxMessage>& response) {
    responseToRilj(response);
}

void RpDataController::handleStopKeepAlive(const sp<RfxMessage>& message) {
    requestToRild(message);
}

void RpDataController::handleStopKeepAliveResponse(const sp<RfxMessage>& response) {
    responseToRilj(response);
}

void RpDataController::updateDataEnableProperty(int slotId, int iEnable) {
    char dataOnIccid[PROPERTY_VALUE_MAX] = {0};
    char currdDataOnIccid[PROPERTY_VALUE_MAX] = {0};

    if (iEnable) {
        property_get(PROPERTY_ICCID_SIM[slotId], dataOnIccid, "0");
    } else {
        strncpy(dataOnIccid, "0", 1);
    }

    logD(RFX_LOG_TAG, "setUserDataProperty[%d]: %d", slotId, iEnable);

    getMSimProperty(slotId, (char *) PROPERTY_MOBILE_DATA_ENABLE, currdDataOnIccid);

    if (strcmp(currdDataOnIccid, dataOnIccid) != 0) {
        setMSimProperty(slotId, (char *) PROPERTY_MOBILE_DATA_ENABLE, dataOnIccid);
    }
}
// M: Data Framework - common part enhancement @}

void RpDataController::updateDefaultDataProperty(int slotId) {
    char defaultIccid[PROPERTY_VALUE_MAX] = {0};
    char currDefaultIccid[PROPERTY_VALUE_MAX] = {0};

    RFX_LOG_I(RFX_LOG_TAG, "updateDefaultDataProperty slotId: %d", slotId);

    if (slotId == -1) {
        property_set(PROPERTY_DEFAULT_DATA_ICCID, "");
        return;
    } else if (slotId >= (int) (sizeof(PROPERTY_ICCID_SIM)/sizeof(PROPERTY_ICCID_SIM[0])) ) {
        logD(RFX_LOG_TAG, "updateDefaultDataProperty failed tue to slotId wrong.");
        return;
    }

    property_get(PROPERTY_ICCID_SIM[slotId], defaultIccid, 0);
    property_get(PROPERTY_DEFAULT_DATA_ICCID, currDefaultIccid, 0);

    if (strlen(defaultIccid) > 0) {
        if (strcmp(currDefaultIccid, defaultIccid) != 0) {
            property_set(PROPERTY_DEFAULT_DATA_ICCID, defaultIccid);
        }
    }
}

void RpDataController::sendToGsm(const sp<RfxMessage>& request) {
    RFX_LOG_D(RFX_LOG_TAG, "request send to Gsm.");
    sp<RfxMessage> message = RfxMessage::obtainRequest(RADIO_TECH_GROUP_GSM,
                                                        request->getId(),
                                                        request,
                                                        true);
    requestToRild(message);
}

void RpDataController::sendToC2kIfSupport(const sp<RfxMessage>& request) {
    if (!RpFeatureOptionUtils::isC2kSupport()) {
        logW(RFX_LOG_TAG, "None C2K project, ignore!");
        return;
    }
    RFX_LOG_D(RFX_LOG_TAG, "request send to C2k.");
    sp<RfxMessage> message = RfxMessage::obtainRequest(RADIO_TECH_GROUP_C2K,
                                                        request->getId(),
                                                        request,
                                                        true);
    requestToRild(message);
}

RILD_RadioTechnology_Group RpDataController::getPsType() {
    RILD_RadioTechnology_Group psType = RADIO_TECH_GROUP_GSM;
    RILD_RadioTechnology_Group psTypeFromStatus = RpDataUtils::getPsType(this);

    // If send request to C2K RILD when C capability is removed, RADIO_NOT_AVAILABLE will be
    // returned as response. This will cause the DcSwitchStateMachine directly from Detaching to
    // Idle State and PS can not detach normally.
    // Ex: CT 3G + CT 4G, data from CT 3G to CT 4G, CT 4G SIM can not register LTE.

    if (psTypeFromStatus == RADIO_TECH_GROUP_C2K) {
        if (getSlotId() == RpCdmaLteModeController::getCdmaSocketSlotId()) {
            logD(RFX_LOG_TAG, "Set psType as CDMA");
            psType = psTypeFromStatus;
        } else {
            logW(RFX_LOG_TAG, "PS type is CDMA, but C capability removed");
        }
    }
    return psType;
}

/*
 * for simplicity, we alwasy create IratController on both sim, though only one is used.
 */
void RpDataController::initIratController() {
    if (RpFeatureOptionUtils::isC2kSupport() && RpDataUtils::getMajorSlot() >= 0) {
        logD(RFX_LOG_TAG, "initIratController create.");
        RFX_OBJ_CREATE(mIratController, RpIratController, this);
    } else {
        logD(RFX_LOG_TAG, "None C2K&LTE project, no irat.");
    }
}

void RpDataController::onCdmaCardTypeChanged(RfxStatusKeyEnum key,
        RfxVariant old_value, RfxVariant value) {
    RFX_UNUSED(key);

    if (value != old_value) {
        logD(RFX_LOG_TAG, "onCdmaCardTypeChanged old_value:%d, value:%d",
                old_value.asInt(), value.asInt());
        updateApnHandler(value.asInt());
    }
}

void RpDataController::updateApnHandler(int cardType) {
    logD(RFX_LOG_TAG, "updateApnHandler cardType:%d", cardType);
    if (cardType == CARD_NOT_INSERTED) {
        // TODO: for unset(-1) case return too.
        logD(RFX_LOG_TAG, "updateApnHandler card not inserted, do nothing.");
        return;
    }

    if (mApnHandler != NULL) {
        // Fwk will call object.deinit when close controller.
        RFX_OBJ_CLOSE(mApnHandler);
        mApnHandler = NULL;
    }
    if (RpDataUtils::isOP09Cdma4GCard(cardType)) {
        RpDcApnHandler *p;
        RFX_OBJ_CREATE(p, RpDcApnHandler, this);
        mApnHandler = (RpDcApnHandler *)p;
    } else {
        logD(RFX_LOG_TAG, "unexpected case, card type not processed.");
    }
}

bool RpDataController::onPreviewMessage(const sp<RfxMessage>& message) {

    if (mIsDuringIrat && message->getType() == REQUEST
            && !isNoNeedSuspendRequest(message->getId())) {
        logD(RFX_LOG_TAG, "onPreviewMessage, put %s into pending list",
                RpDataUtils::requestToString(message->getId()));
        return false;
    } else {
        /** IMS PDN setup data call request is blocked by INTERNET PDN activation
         * due to VzW script limitation.
         * To reuse IMS PDN by another channel to avoid this issue.
         *
         * When RilProxy received setup data call request, queue the request if
         * 1.) operator 12.
         * 2.) IMS PDN (by check profile)
         * then send a request RIL_LOCAL_REQUEST_GET_IMS_DATA_CALL_INFO to
         * check the IMS PDN is activated by modem or not.
         * - If Yes, dequeue the setup data call request and replace
         *   with request RIL_LOCAL_REQUEST_REUSE_IMS_DATA_CALL so
         *   that RILD can executed on data2 channel and corresponding thread.
         * - If No, dequeue the setup data call request and send
         *   it to data channel as the legacy behavior.
         * Once received the response of RIL_LOCAL_REQUEST_REUSE_IMS_DATA_CALL,
         * replace reponse id back to REQUEST_SETUP_DATA_CALL and send back
         * to data framework.
         */
        if (message->getType() == REQUEST && message->getId() == RIL_REQUEST_SETUP_DATA_CALL) {
            if (!onPreviewSetupDataCallMessage(message)) {
                return false;
            }
        }
        return true;
    }
}

bool RpDataController::onPreviewSetupDataCallMessage(const sp<RfxMessage>& message) {
    RpSetupDataCallParam *pParam = new RpSetupDataCallParam(message, false);
    int profileId = updateApnProfileId(pParam->getProfileId(), pParam->getApnTypesBitmap());
    int interfaceId = getInterfaceId(profileId, pParam->getProtocolId());
    if (interfaceId < 0) {
         logE(RFX_LOG_TAG, "[%s] can't find available interface Id", __FUNCTION__);
    } else {
        RLOGI("[RP_DC] onPreviewMessage:=> RIL_REQUEST_SETUP_DATA_CALL");
        logD(RFX_LOG_TAG, "[%s] apnName: %s, interfaceId: %d, profile: %d",
                __FUNCTION__, String8(pParam->apnName).string(), interfaceId, profileId);

        // mIsCheckPdnReuse be true when received response of
        // RIL_LOCAL_REQUEST_GET_IMS_DATA_CALL_INFO
        // in order to get data call info once per setup data call process,
        // and reset to false when process RIL_REQUEST_SETUP_DATA_CALL of IMS data call.
        if (RpDataUtils::isOP12Support() &&
                RIL_DATA_PROFILE_IMS == profileId && !mIsCheckPdnReuse) {
            RLOGI("[RP_DC] Pending request %d", message->getId());
            sp<RfxMessage> localReq = RfxMessage::obtainRequest(
                    message->getSlotId(),
                    RADIO_TECH_GROUP_GSM,
                    RIL_LOCAL_REQUEST_GET_IMS_DATA_CALL_INFO);
            Parcel *p = localReq->getParcel();
            // Currently, these parameters are enough to judge.
            p->writeInt32(4);
            // Is this stupid?
            p->writeString16(String16((std::to_string(profileId)).c_str()));
            p->writeString16(pParam->apnName);
            p->writeString16(pParam->protocol);
            p->writeString16(String16(android::String8::format("%d", interfaceId)));

            handleRequestDefault(localReq);
            mImsApnName = pParam->apnName;
            mImsProfileId = profileId;
            mImsInterfaceId = interfaceId;
            // Reset when check start
            mImsPdnPending = true;
            mIsCheckPdnReuse = false;
            mImsPdnFound = false;
            delete pParam;
            return false;
        }
    }
    delete pParam;
    return true;
}

bool RpDataController::onCheckIfResumeSetupDataCallMessage(const sp<RfxMessage>& message) {
    int requestId = message->getId();
    if (message->getType() == REQUEST && requestId == RIL_REQUEST_SETUP_DATA_CALL) {
        if (!mImsPdnPending) {
            return true;
        }

        RpSetupDataCallParam *pParam = new RpSetupDataCallParam(message, false);
        int profileId = updateApnProfileId(pParam->getProfileId(), pParam->getApnTypesBitmap());
        int interfaceId = getInterfaceId(profileId, pParam->getProtocolId());
        RLOGI("[%s] interfaceId = %d, mImsInterfaceId = %d, mIsCheckPdnReuse = %d",
                __FUNCTION__, interfaceId, mImsInterfaceId, mIsCheckPdnReuse);
        if (mIsCheckPdnReuse && interfaceId == mImsInterfaceId) {
            RLOGI("[RP_DC] Resume request %d, apnName %s",
                    requestId, String8(mImsApnName).string());
            delete pParam;
            return true;
        } else {
            delete pParam;
            return false;
        }
    }
    return true;
}

bool RpDataController::onCheckIfResumeMessage(const sp<RfxMessage>& message) {
    if (!mIsDuringIrat && onCheckIfResumeSetupDataCallMessage(message)) {
        logD(RFX_LOG_TAG, "resume a request %d", message->getId());
        return true;
    } else {
        return false;
    }
}

bool RpDataController::onCheckIfRemoveSuspendedMessage(const sp<RfxMessage>& message) {
    RFX_UNUSED(message);
    // When radio unavailable, status is reset, needn't process pending requests.
    // Ril framework process all messages in one thread, so the process is atomic.
    if (RpDataUtils::isRadioUnAvailable(this)) {
        logD(RFX_LOG_TAG, "onCheckIfRemoveSuspendedMessage, remove for radio unavailable!");
        return true;
    }
    return false;
}

void RpDataController::updateIratStatus( bool status) {
    mIsDuringIrat = status;
}

bool RpDataController::isNoNeedSuspendRequest(int requestId) {
    /*
     * white list for IRAT suspend request.
     */
    if (requestId == RIL_REQUEST_SET_INITIAL_ATTACH_APN) {
        logD(RFX_LOG_TAG, "isNoNeedSuspendRequest: %d", requestId);
        return true;
    }
    return false;
}

// M: MPS feature
void RpDataController::sendDeactAllDataCallRequest(int slotId) {
    // send deactivate all data call request to RILD.
    RFX_LOG_D(RFX_LOG_TAG, "sendDeactAllDataCallRequest");

    RILD_RadioTechnology_Group psType = getPsType();

    sp<RfxMessage> request = RfxMessage::obtainRequest(slotId,
            psType,
            (psType == RADIO_TECH_GROUP_GSM) ?
            RIL_LOCAL_REQUEST_DEACTIVATE_ALL_DATA_CALL :
            RIL_LOCAL_C2K_REQUEST_DEACTIVATE_ALL_DATA_CALL);

    handleRequestDefault(request);
}

// M: [VzW] Data Framework @{
void RpDataController::handlePcoStatusChangeUrc(const sp<RfxMessage>& message, int urcId) {
    if (RpDataUtils::isOP12Support()) {  // only support VZW and PCO ID FF00.
        Parcel* p = message->getParcel();
        char *content = NULL;
        switch (urcId) {
            case RIL_UNSOL_PCO_DATA: {
                RIL_PCO_Data pcoData;
                memset (&pcoData, 0, sizeof(RIL_PCO_Data));

                p->readInt32(&pcoData.cid);
                pcoData.bearer_proto = RpDataUtils::strdupReadString(p);
                p->readInt32(&pcoData.pco_id);
                p->readInt32(&pcoData.contents_length);
                pcoData.contents = (char *) calloc(pcoData.contents_length, sizeof(char));
                RFX_ASSERT(pcoData.contents != NULL);
                p->read(pcoData.contents, pcoData.contents_length);

                if (pcoData.cid == -1) {
                    logD(RFX_LOG_TAG, "RIL_UNSOL_PCO_STATUS for interface id -1 case.");
                    return;
                }
                if (pcoData.contents_length == 1) {
                    asprintf(&content, "%s:%d", OP12_PCO_ID, pcoData.contents[0]);
                    logD(RFX_LOG_TAG, "PCO status:: [%s]", content);
                    if (content != NULL) {
                        getStatusManager()->setString8Value(RFX_STATUS_KEY_PCO_STATUS,
                                String8(content));
                    }
                } else {
                    logD(RFX_LOG_TAG, "PCO content length not 1, do not write.");
                }

                free(pcoData.bearer_proto);
                free(pcoData.contents);
                break;
            }
            case RIL_UNSOL_PCO_DATA_AFTER_ATTACHED: {
                RIL_PCO_Data_attached pcoDataAttached;
                memset (&pcoDataAttached, 0, sizeof(RIL_PCO_Data_attached));

                p->readInt32(&pcoDataAttached.cid);
                pcoDataAttached.apn_name = RpDataUtils::strdupReadString(p);
                pcoDataAttached.bearer_proto = RpDataUtils::strdupReadString(p);
                p->readInt32(&pcoDataAttached.pco_id);
                p->readInt32(&pcoDataAttached.contents_length);
                pcoDataAttached.contents = (char *) calloc(pcoDataAttached.contents_length,
                        sizeof(char));
                RFX_ASSERT(pcoDataAttached.contents != NULL);
                p->read(pcoDataAttached.contents, pcoDataAttached.contents_length);

                if (pcoDataAttached.contents_length == 1) {
                    asprintf(&content, "%s:%d", OP12_PCO_ID, pcoDataAttached.contents[0]);
                    logD(RFX_LOG_TAG, "PCO status:: [%s]", content);
                    if (content != NULL) {
                        getStatusManager()->setString8Value(RFX_STATUS_KEY_PCO_STATUS,
                                String8(content));
                    }
                } else {
                    logD(RFX_LOG_TAG, "PCO content length not 1, do not write.");
                }
                free(pcoDataAttached.apn_name);
                free(pcoDataAttached.bearer_proto);
                free(pcoDataAttached.contents);
                break;
            }
            default: {
                responseToRilj(message);
                return;
            }
        }
        if (content != NULL) {
            free(content);
        }
    }
    responseToRilj(message);
}
// M: [VzW] Data Framework @}

// M: MPS feature start
void RpDataController::writeStringToParcel(Parcel *p, const char *s) {
    char16_t *s16 = NULL;
    size_t s16_len = 0;
    s16 = strdup8to16(s, &s16_len);
    p->writeString16(s16, s16_len);
    free(s16);
}
// M: MPS feature end

void RpDataController::updateDataConnectionStatus(int oldCount, int newCount) {
    logD(RFX_LOG_TAG, "updateDataConnectionStatus oldCount %d, newCount %d", oldCount, newCount);
    if (oldCount != newCount) {
        if (oldCount == 0) {
            getStatusManager()->setIntValue(RFX_STATUS_KEY_DATA_CONNECTION, DATA_STATE_CONNECTED);
        } else if (newCount == 0) {
            getStatusManager()->setIntValue(RFX_STATUS_KEY_DATA_CONNECTION, DATA_STATE_DISCONNECTED);
        }
    }
}

void RpDataController::configDcStateForPlmnSearch(const int plmnSearchAction,
        const sp<RfxAction>& action) {
    logD(RFX_LOG_TAG, "[configDcStateForPlmnSearch] Action: %d", plmnSearchAction);
    pthread_mutex_lock(&mCcmniData[m_slot_id]->dispatchMutex);
    logD(RFX_LOG_TAG, "[configDcStateForPlmnSearch] mutex get. queueSize: %d",
            mCcmniData[m_slot_id]->queueSize());
    if (mCcmniData[m_slot_id]->isQueueEmpty()) {
        if (mCcmniData[m_slot_id]->enqueueRequest(plmnSearchAction) == 0) {
            logD(RFX_LOG_TAG, "Enqueue success. Signal looper. queueSize: %d",
                    mCcmniData[m_slot_id]->queueSize());
            pthread_cond_signal(&mCcmniData[m_slot_id]->dispatchCond);
        } else {
            logD(RFX_LOG_TAG, "Enqueue fail!! (%d)", mCcmniData[m_slot_id]->queueSize());
        }
    } else if (mCcmniData[m_slot_id]->enqueueRequest(plmnSearchAction) == 0) {
        logD(RFX_LOG_TAG, "Enqueue success. Looper already working. queueSize: %d",
                mCcmniData[m_slot_id]->queueSize());
    } else {
        logD(RFX_LOG_TAG, "Enqueue fail!! queueSize: %d", mCcmniData[m_slot_id]->queueSize());
    }
    pthread_mutex_unlock(&mCcmniData[m_slot_id]->dispatchMutex);

    if (action != NULL) {
        action->act();
    }
    logD(RFX_LOG_TAG, "[configDcStateForPlmnSearch] End");
}

void controlCcmniInterface(int reqVal) {
    int mask = 0;
    int channelInUsedMask = 0x000FF;  // ccmni uses 20 channels currently
    int imsChannelMask = 0x38070;  // IMS channels: 5,6,7,16,17,18th channel
    char ccmniDataBuf[SOCKET_BUF_LEN];
    hidl_string hidl_cmd;
    android::sp<INetdagent> pNetdagentService;

    //get Netdagent HIDL service
    pNetdagentService = INetdagent::getService();
    if (pNetdagentService == nullptr) {
        RLOGE("[RP_DC][controlCcmniInterface] get %s service failed", INetdagent::descriptor);
        return;
    }

    memset(ccmniDataBuf, 0, SOCKET_BUF_LEN * sizeof(char));
    // set channel mask
    mask = channelInUsedMask & (~imsChannelMask);  // do nothing to IMS channels(5,6,7,16,17,18)
    sprintf(ccmniDataBuf, "netdagent firewall set_plmn_iface_rule ccmni %d", mask);
    // set action
    if (reqVal == 0) {
        // search begin
        strncat(ccmniDataBuf, " allow", 6);
    } else if (reqVal == 1) {
        // search end or abort
        strncat(ccmniDataBuf, " deny", 5);
    }
    hidl_cmd = ccmniDataBuf;
    RLOGI("[RP_DC][controlCcmniInterface] cmd:%s", hidl_cmd.c_str());
    pNetdagentService->dispatchNetdagentCmd(hidl_cmd);
}

extern "C"
void *dispatchRequestToNetd(void *arg) {
    int err = 0;
    int queueVal = INIT_QUEUE_VAL;
    int simId = *((int *) arg);

    RLOGI("[RP_DC][dispatchRequestToNetd] Begin simId:%d, queueSize:%d",
            simId, mCcmniData[simId]->queueSize());

    do {
        pthread_mutex_lock(&mCcmniData[simId]->dispatchMutex);
        RLOGI("[RP_DC][dispatchRequestToNetd] mutex get. queueSize: %d",
                mCcmniData[simId]->queueSize());
        err = mCcmniData[simId]->popRequest(&queueVal);
        if (err == 0) {
            RLOGI("[RP_DC] popRequest OK. queueVal=%d", queueVal);
            pthread_mutex_unlock(&mCcmniData[simId]->dispatchMutex);
            controlCcmniInterface(queueVal);
        } else {
            if (mCcmniData[simId]->isQueueEmpty()) {
                RLOGI("[RP_DC] popRequest empty!! Go to sleep");
            } else {
                RLOGI("[RP_DC] popRequest err!! Go to sleep");
            }
            pthread_cond_wait(&mCcmniData[simId]->dispatchCond, &mCcmniData[simId]->dispatchMutex);
            pthread_mutex_unlock(&mCcmniData[simId]->dispatchMutex);
        }
    } while (1);

    pthread_detach(pthread_self());
    RLOGI("[RP_DC][dispatchRequestToNetd] End");
    return NULL;
}

void RpDataController::handleSyncDataSettingsToMdResponse(const sp<RfxMessage>& response) {
    responseToRilj(response);
}


void RpDataController::handleSetupDataRequestAlt(const sp<RfxMessage>& request) {
    // update data connection info
    // request message to create a connection, record apn & interfaceId.
    RpSetupDataCallParam *pParam = new RpSetupDataCallParam(request, true);

    // Only request from MAL will pass interfaceId.
    // SETUP_DATA_CALL RIL_ALT interface AOSP refactoring start
    // The range of values for mInterfaceId is 1~8 for each sim.
    // (according to profileId of request APN)
    //
    // mInterfaceId is mapping to CCMMI interface as below:
    // CCMMI1: Internet
    // CCMMI2: Tethering
    // CCMMI3: Other(MMS, ADMIN, AKA)
    // CCMMI4: Other(MMS, ADMIN, AKA)
    // CCMMI5: IMS
    // CCMMI6: IMS emergency
    // CCMMI7: SS XCAP(IMS)
    // CCMMI8: eMBMS
    //
    // For example, AP request IMS APN, profileId delivered as 2.
    // DataInterfaceManager will assign mInterfaceId=5 according to this profileId.
    // And DataInterfaceManager update status and related information(profileId, protocol..)
    // in internal table with index=4

    mInterfaceId = pParam->interfaceId;
    if ((mInterfaceId >= 0) &&
            (sCcmmiIfManager[m_slot_id]->updateForSetupDataRequestFromMal(mInterfaceId, request->getToken()) >= 0)) {
        logD(RFX_LOG_TAG, "record mApnName: %s, mInterfaceId: %d",
                String8(mApnName).string(), mInterfaceId);
        handleRequestDefault(request, request->getToken());
    } else {
        responseToRilj(RfxMessage::obtainResponse(RIL_E_GENERIC_FAILURE, request));
    }

    delete pParam;
    return;
    // SETUP_DATA_CALL RIL interface AOSP refactoring end
}


void RpDataController::handleSetupDataResponseAlt(const sp<RfxMessage>& response) {
    if (response->getError() != RIL_E_SUCCESS) {
        logD(RFX_LOG_TAG, "setupdata response fail!");

        // SETUP_DATA_CALL RIL interface AOSP refactoring start
        sCcmmiIfManager[m_slot_id]->updateForSetupDataResponse(response->getToken(), true);
        // SETUP_DATA_CALL RIL interface AOSP refactoring end

        responseToRilj(response);
        return;
    }


    // SETUP_DATA_CALL RIL interface AOSP refactoring start
    sCcmmiIfManager[m_slot_id]->updateForSetupDataResponse(response->getToken(), false);
    // SETUP_DATA_CALL RIL interface AOSP refactoring end

    responseToRilj(response);
}

void RpDataController::handleSetLinkCapacityReportingCriteriaRequest(const sp<RfxMessage>& request) {
    sendToGsm(request);
    sendToC2kIfSupport(request);
}

void RpDataController::handleSetLinkCapacityReportingCriteriaResponse(const sp<RfxMessage>& response) {
    int reqId = response->getId();
    int source = response->getSource();
    sp<RfxMessage> outResponse = sp<RfxMessage>(NULL);

    if (!RpFeatureOptionUtils::isC2kSupport()) {
        responseToRilj(response);
    } else {
        ResponseStatus status = preprocessResponse(response, outResponse);
        // set link capacity reporting criteria request will send to both gsm and c2k,
        // when the UE not support c slot or c slot change, preprocessResponse will
        // return RESPONSE_STATUS_HAVE_MATCHED_WITH_C_CHANGED status.
        if (status == RESPONSE_STATUS_HAVE_MATCHED ||
                status == RESPONSE_STATUS_HAVE_MATCHED_WITH_C_CHANGED) {
            RFX_LOG_I(RFX_LOG_TAG, "[handleSetLinkCapacityReportingCriteriaResponse] "
                    "Response matched status=%d", status);
            responseToRilj(response);
        } else if (status == RESPONSE_STATUS_NO_MATCH_AND_SAVE) {
            RFX_LOG_I(RFX_LOG_TAG, "[handleSetLinkCapacityReportingCriteriaResponse] "
                    "Response saved");
        } else if (status == RESPONSE_STATUS_ALREADY_SAVED) {
            RFX_LOG_D(RFX_LOG_TAG, "[handleSetLinkCapacityReportingCriteriaResponse] "
                    "Response have saved -> ignore");
        } else if (status == RESPONSE_STATUS_HAVE_BEEN_DELETED) {
            RFX_LOG_D(RFX_LOG_TAG, "[handleSetLinkCapacityReportingCriteriaResponse] "
                    "Response have time out! REQ=%s src=%s token=%d",
                    RpDataUtils::requestToString(reqId),
                    RpDataUtils::radioGroupToString(source),
                    response->getPToken());
        } else {
            RFX_LOG_D(RFX_LOG_TAG, "[handleSetLinkCapacityReportingCriteriaResponse] "
                    "Response misc error -> ignore, status=%d", status);
        }
    }
}

void RpDataController::onFastDormancyTimerChanged(RfxStatusKeyEnum key,
        RfxVariant oldValue, RfxVariant newValue) {
    RFX_UNUSED(key);
    RFX_UNUSED(oldValue);
    String8 strStatus = newValue.asString8();
    RFX_LOG_D(RFX_LOG_TAG, "[onFastDormancyTimerChanged] strStatus = %s", strStatus.string());
    if (strStatus.find(String8(RP_EM_FASTDORMANCY_SYNC)) != -1) {
        int timer[RP_EM_FASTDORMANCY_TIMER_LENGTH] = {0};

        char *tempFdSetting = strtok((char *)strStatus.string(), ":,");
        int i = 0;
        while (tempFdSetting != NULL && i < RP_EM_FASTDORMANCY_TIMER_LENGTH) {
            tempFdSetting = strtok(NULL, ":,");
            if (tempFdSetting != NULL) {
                timer[i++] = atoi(tempFdSetting);
            }
        }

        // mode 2 is for fastdormancy timer
        if (timer[0] == 2) {
            sp<RfxMessage> request = RfxMessage::obtainRequest(m_slot_id,
                RADIO_TECH_GROUP_GSM,
                RIL_REQUEST_SET_FD_MODE);

            Parcel *p = request->getParcel();
            p->writeInt32(3); // args num
            p->writeInt32(timer[0]); // mode
            p->writeInt32(timer[1]); // timer type
            p->writeInt32(timer[2]); // timer value

            sendToGsm(request);
        }
    }
}

void RpDataController::onServiceStateChanged(RfxStatusKeyEnum key,
        RfxVariant oldValue, RfxVariant newValue) {
    RFX_UNUSED(key);
    RfxNwServiceState oldSS = oldValue.asServiceState();
    RfxNwServiceState newSS = newValue.asServiceState();
    int oldVoiceRegState = oldSS.getRilVoiceRegState();
    int oldDataRegState = oldSS.getRilDataRegState();
    int newVoiceRegState = newSS.getRilVoiceRegState();
    int newDataRegState = newSS.getRilDataRegState();

    if ((oldVoiceRegState != newVoiceRegState) || (oldDataRegState != newDataRegState)) {
        RFX_LOG_D(RFX_LOG_TAG, "onServiceStateChanged newSS: %s",
                newSS.toString().string());
        if (newDataRegState == RIL_REG_STATE_HOME) {
            setRoamingState(false);
        } else if (newDataRegState == RIL_REG_STATE_ROAMING) {
            setRoamingState(true);
        } else {
            // PS is not in service, use CS state to guess home or roaming
            if (newVoiceRegState == RIL_REG_STATE_HOME) {
                setRoamingState(false);
            } else if (newVoiceRegState == RIL_REG_STATE_ROAMING) {
                setRoamingState(true);
            } else {
                // both PS & CS are out of service, do nothing
            }
        }
    }
}

bool RpDataController::getRoamingState() {
    return mIsRoaming;
}

void RpDataController::setRoamingState(bool isRoaming) {
    mIsRoaming = isRoaming;
}

void RpDataController::onAllowedChanged(RfxStatusKeyEnum key,
        RfxVariant old_value, RfxVariant value) {
    RFX_UNUSED(key);
    RFX_UNUSED(old_value);
    int allowed = value.asInt();

    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s]: allowed = %d", m_slot_id, __FUNCTION__, allowed);
    if ((getStatusManager(m_slot_id)->getIntValue(RFX_STATUS_KEY_SLOT_ALLOW, 0)) != 1) {
        RFX_LOG_D(RFX_LOG_TAG, "[%d][%s]: current not allowed, retrun", m_slot_id,__FUNCTION__);
        return;
    }
}

void RpDataController::enqueueForPreferredDataMode(const sp<RfxMessage>& message) {
    Parcel* parcel = message->getParcel();
    message->resetParcelDataStartPos();
    parcel->readInt32();  // content length
    parcel->readString16(); // radioTechnology
    int nProfileId = atoi(String8(parcel->readString16()).string()); // profile
    message->resetParcelDataStartPos();

    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] profileId: %d", m_slot_id, __FUNCTION__, nProfileId);

    if (!isAllowedStatusIndependent(nProfileId)) {
        if (nProfileId != RIL_DATA_PROFILE_DEFAULT && mMapProfileIdToken.count(nProfileId) == 0) {
            RpDataAllowController *pRpDataAllowController;
            pRpDataAllowController = (RpDataAllowController *)findController(message->getSlotId(),
                    RFX_OBJ_CLASS_INFO(RpDataAllowController));
            pRpDataAllowController->enqueueNetworkRequest(nProfileId, m_slot_id);
        }
        if ((getStatusManager(m_slot_id)->getIntValue(RFX_STATUS_KEY_SLOT_ALLOW, 0)) == 1) {
            RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] RFX_STATUS_KEY_SLOT_ALLOW == 1", m_slot_id,
            __FUNCTION__);
            mMapProfileIdToken[nProfileId] = message->getToken();
            requestToRild(message);
        } else {
            RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] RFX_STATUS_KEY_SLOT_ALLOW == 0", m_slot_id,
                __FUNCTION__);
            sCcmmiIfManager[m_slot_id]->updateForSetupDataResponse(message->getToken(), true);

            sp<RfxMessage> response = RfxMessage::obtainResponse(RIL_E_SUCCESS, message);
            Parcel* p = response->getParcel();
            // Construct MTK_RIL_Data_Call_Response_v11 parcel
            // Write version
            p->writeInt32(11);
            // Write number
            p->writeInt32(1);
            p->writeInt32((int)PDP_FAIL_DATA_NOT_ALLOW); // status
            p->writeInt32(-1); // suggestedRetryTime
            p->writeInt32(-1); // cid
            p->writeInt32(0); // active
            writeStringToParcel(p, NULL); // type
            writeStringToParcel(p, NULL); // ifname
            writeStringToParcel(p, NULL); // addresses
            writeStringToParcel(p, NULL); // dnses
            writeStringToParcel(p, NULL); // gateways
            writeStringToParcel(p, NULL); // pcscf
            p->writeInt32(0); // mtu
            p->writeInt32(1); // rat
            responseToRilj(response);
        }
    } else {
        requestToRild(message);
    }
}

void RpDataController::dequeueForPreferredDataMode(const sp<RfxMessage>& message) {
    Parcel* parcel = message->getParcel();
    message->resetParcelDataStartPos();
    parcel->readInt32();  // content length
    String16 idString = parcel->readString16();  // cid
    message->resetParcelDataStartPos();

    int nCid = atoi(String8(idString).string());
    int nProfileId = 0;

    if (nCid <= 0) {
        // This is for special case in preferred data mode.
        // For example, SIM1 is default data SIM, SIM2 send a MMS,
        // and MMS data connection setup fail until APP timeout and send release request
        // to mobile data service. The release request will not trigger deactivate flow
        // because data setup fail.For the cases of no deactivate flow, data will trigger
        // deactivate flow with (profileId * -1) as cid param
        nProfileId = -nCid;
    } else {
        nProfileId = mMapCidProfileId[nCid];
    }

    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] cid: %d, profileId: %d", m_slot_id,
            __FUNCTION__, nCid, nProfileId);

    if (isAllowedStatusIndependent(nProfileId) || nProfileId == RIL_DATA_PROFILE_DEFAULT) {
        mMapCidProfileId.erase(nCid);
        mMapProfileIdToken.erase(nProfileId);
        return;
    }

    RpDataAllowController *pRpDataAllowController =
            (RpDataAllowController *) findController(message->getSlotId(),
            RFX_OBJ_CLASS_INFO(RpDataAllowController));
    pRpDataAllowController->dequeueNetworkRequest(nProfileId, m_slot_id);
    mMapCidProfileId.erase(nCid);
    mMapProfileIdToken.erase(nProfileId);
}

bool RpDataController::isAllowedStatusIndependent(int nProfileId) {
    // By checking the profile id, decide if the set up data request can
    // ignore data allowed status.
    if (nProfileId == RIL_DATA_PROFILE_IMS
            || nProfileId == RIL_DATA_PROFILE_VENDOR_EMERGENCY
            || nProfileId == RIL_DATA_PROFILE_VENDOR_VSIM) {
        return true;
    }
    return false;
}

int RpDataController::updateApnProfileId(int profileId, int supportedTypesBitmask) {
    int nProfileId = profileId;
    if (nProfileId == -1) {
        for (int i = 0; i < RIL_APN_TYPE_COUNT; i++) {
            if (supportedTypesBitmask & (1 << i)) {
                if ((1 << i) == RIL_APN_TYPE_DEFAULT) {
                    int preferDataSim = getNonSlotScopeStatusManager()->getIntValue(
                            RFX_STATUS_KEY_PREFERRED_DATA_SIM);
                    if (preferDataSim != m_slot_id) {
                        continue;
                    }
                }
                nProfileId = getApnProfileId(1 << i);
                break;
            }
        }
    }

    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] profileId: %d supportedTypesBitmask %d",
            m_slot_id, __FUNCTION__, nProfileId, supportedTypesBitmask);

    return nProfileId;
}

int RpDataController::getApnProfileId(int apnTypeId) {
    switch (apnTypeId) {
        case RIL_APN_TYPE_DEFAULT: return RIL_DATA_PROFILE_DEFAULT;
        case RIL_APN_TYPE_DUN: return RIL_DATA_PROFILE_TETHERED;
        case RIL_APN_TYPE_FOTA: return RIL_DATA_PROFILE_FOTA;
        case RIL_APN_TYPE_IMS: return RIL_DATA_PROFILE_IMS;
        case RIL_APN_TYPE_CBS: return RIL_DATA_PROFILE_CBS;
        case RIL_APN_TYPE_MMS: return RIL_DATA_PROFILE_VENDOR_MMS;
        case RIL_APN_TYPE_SUPL: return RIL_DATA_PROFILE_VENDOR_SUPL;
        case RIL_APN_TYPE_XCAP: return RIL_DATA_PROFILE_VENDOR_XCAP;
        case RIL_APN_TYPE_BIP: return RIL_DATA_PROFILE_VENDOR_BIP;
        case RIL_APN_TYPE_HIPRI: return RIL_DATA_PROFILE_VENDOR_HIPRI;
        case RIL_APN_TYPE_EMERGENCY: return RIL_DATA_PROFILE_VENDOR_EMERGENCY;
        case RIL_APN_TYPE_WAP: return RIL_DATA_PROFILE_VENDOR_WAP;
        case RIL_APN_TYPE_RCS: return RIL_DATA_PROFILE_VENDOR_RCS;
        case RIL_APN_TYPE_VSIM: return RIL_DATA_PROFILE_VENDOR_VSIM;
        case RIL_APN_TYPE_MCX: return RIL_DATA_PROFILE_VENDOR_MCX;
        default: return RIL_DATA_PROFILE_DEFAULT;
    }
}

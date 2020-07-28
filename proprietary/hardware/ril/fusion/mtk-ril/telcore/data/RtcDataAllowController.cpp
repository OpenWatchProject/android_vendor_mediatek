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

/*****************************************************************************
 * Include
 *****************************************************************************/

#include "RtcDataAllowController.h"
#include <libmtkrilutils.h>
#include "RfxRilUtils.h"
#include "power/RtcRadioController.h"
#include "RtcDataUtils.h"
#include "RfxMainThread.h"
#include "wpfa.h"

#define RTC_DAC_LOG_TAG "RTC_DAC"

/// OEM temp data switch @{
#define TEMP_DATA_SWITCH_OFF       (0)
#define TEMP_DATA_SWITCH_ON        (1)
/// @}

static const int RETRY_TIME_MS = 100;
/*****************************************************************************
 * Class RtcDataAllowController
 * this is a none slot controller to manage DATA_ALLOW_REQUEST.
 *****************************************************************************/

RFX_IMPLEMENT_CLASS("RtcDataAllowController", RtcDataAllowController, RfxController);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, RFX_MSG_REQUEST_ALLOW_DATA);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, RFX_MSG_REQUEST_DATA_CONNECTION_ATTACH);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, RFX_MSG_REQUEST_DATA_CONNECTION_DETACH);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, RFX_MSG_REQUEST_RECOVERY_ALLOW_DATA);

RtcDataAllowController::RtcDataAllowController() :
    mDoingDataAllow(false),
    mReqDataAllow(false),
    mDisallowingPeer(0),
    mLastAllowTrueRequest(NULL),
    mIsPreferredDataMode(-1),
    isMdSelfEdallow(-1) {
}

RtcDataAllowController::~RtcDataAllowController() {
}

void RtcDataAllowController::onInit() {
    RfxController::onInit();  // Required: invoke super class implementation
    logD(RTC_DAC_LOG_TAG, "onInit");
    mDoingDataAllow = false;
    mReqDataAllow = false;
    mDisallowingPeer = 0;
    mLastAllowTrueRequest = NULL;
    mIsPreferredDataMode = -1;
    rfx_property_set("vendor.ril.data.legacy_allow_mode", "-1");

    char MdEdallowTriggered[] = "MD trigger edallow";
    isMdSelfEdallow = getFeatureVersion(MdEdallowTriggered, 0);
    logD(RTC_DAC_LOG_TAG, "isMdSelfEdallow = %d", isMdSelfEdallow);
    if (!isMdSelfEdallow) {
        getNonSlotScopeStatusManager()->registerStatusChanged(RFX_STATUS_KEY_PREFERRED_DATA_SIM,
                RfxStatusChangeCallback(this, &RtcDataAllowController::onPreferredChanged));
        for (int i = 0; i < RfxRilUtils::rfxGetSimCount(); i++) {
            getStatusManager(i)->registerStatusChanged(RFX_STATUS_CONNECTION_STATE,
                RfxStatusChangeCallback(this, &RtcDataAllowController::onHidlStateChanged));
            getStatusManager(i)->
                registerStatusChangedEx(RFX_STATUS_KEY_SML_SLOT_LOCK_POLICY_SERVICE_CAPABILITY,
                RfxStatusChangeCallbackEx(this, &RtcDataAllowController::onSimMeLockChanged));
        }
    }

    const int requestIdList[] = {
        RFX_MSG_REQUEST_ALLOW_DATA,  // 123
        RFX_MSG_REQUEST_DATA_CONNECTION_ATTACH,
        RFX_MSG_REQUEST_DATA_CONNECTION_DETACH,
        RFX_MSG_REQUEST_RECOVERY_ALLOW_DATA,
    };

    // register request
    // NOTE. one id can only be registered by one controller
    for (int i = 0; i < RfxRilUtils::rfxGetSimCount(); i++) {
        registerToHandleRequest(i, requestIdList, sizeof(requestIdList) / sizeof(int));
    }

    /**
     * WPFA initial: check IMS SUBMARINE
     *
     *    Enable : Create WPFA service
     *    Disable: Do not create WPFA service
     */
    char imsSubmarinefeature[] = "IMS SUBMARINE";
    int imsSubmarinesupport = getFeatureVersion(imsSubmarinefeature);
    logD(RTC_DAC_LOG_TAG,
            "MD Feature: imsSubmarinesupport=%d", imsSubmarinesupport);

    if (imsSubmarinesupport == 1) {
        logD(RTC_DAC_LOG_TAG, "Creating WPFA");
        wpfaInit();
    }

    // Resend data allow when RILD restarts.
    // resend according to property value.
    // If the value is invalid, it will not works.
    resendAllowData();
}

void RtcDataAllowController::onDeinit() {
    logD(RTC_DAC_LOG_TAG, "onDeinit");
    mDoingDataAllow = false;
    mReqDataAllow = false;
    mDisallowingPeer = 0;
    mLastAllowTrueRequest = NULL;
    mIsPreferredDataMode = -1;
    RfxController::onDeinit();
}

int RtcDataAllowController::checkRequestExistInQueue(int type, int slotId) {
    for(int i = 0 ; i < (int)mOnDemandQueue.size() ; i++){
        if(mOnDemandQueue[i].type == type &&
                mOnDemandQueue[i].slotId == slotId) {
            return i;
        }
    }
    return -1;
}

int RtcDataAllowController::checkTypeExistInQueue(int type) {
    for(int i = 0 ; i < (int)mOnDemandQueue.size() ; i++){
        if(mOnDemandQueue[i].type == type) {
            return i;
        }
    }
    return -1;
}

void RtcDataAllowController::enqueueNetworkRequest(int r_id, int slotId) {
    if (isMdSelfEdallow != 0) {
        return;
    }
    int defaultSim = getNonSlotScopeStatusManager()->getIntValue(
            RFX_STATUS_KEY_DEFAULT_DATA_SIM);
    int preferSim = getNonSlotScopeStatusManager()->getIntValue(
            RFX_STATUS_KEY_PREFERRED_DATA_SIM);
    /*RIL_DATA_PROFILE_VENDOR_MMS = 1001*/
    if (r_id != RIL_DATA_PROFILE_DEFAULT && r_id != RIL_DATA_PROFILE_IMS) {
        logD(RTC_DAC_LOG_TAG, "enqueueNetworkRequest: r_id=%d,slotId=%d,defaultS=%d,preferS=%d",
             r_id , slotId, defaultSim, preferSim);
        if (mOnDemandQueue.size() == 0) {
            logD(RTC_DAC_LOG_TAG, "enqueueNetworkRequest: set data allow to %d", slotId);

            struct onDemandRequest demandRequest;
            demandRequest.type = r_id;
            demandRequest.slotId = slotId;
            mOnDemandQueue.push_back(demandRequest);

            logD(RTC_DAC_LOG_TAG, "enqueueNetworkRequest: curr queue: %d",
                    (int)mOnDemandQueue.size());
            onSetDataAllow(slotId);
            return;
        }
        else {
            int ret = checkRequestExistInQueue(r_id, slotId);
            if (ret >= 0) {
                logD(RTC_DAC_LOG_TAG,
                     "enqueueNetworkRequest: type = %d , slotId = %d, already enqueue, return",
                     r_id , slotId);
                return;
            }

            struct onDemandRequest demandRequest;
            demandRequest.type = r_id;
            demandRequest.slotId = slotId;
            mOnDemandQueue.push_back(demandRequest);

            logD(RTC_DAC_LOG_TAG, "enqueueNetworkRequest: curr queue: %d",
                    (int)mOnDemandQueue.size());
            return;
        }
    }
}

bool RtcDataAllowController::dequeueNetworkRequest(int r_id, int slotId) {
    if (isMdSelfEdallow != 0) {
        // For Gen97, return false to caller(rtcdc) make it do nothing.
        // Do not trigger reseet retry timmer in this case.
        return false;
    }
    logD(RTC_DAC_LOG_TAG, "dequeueNetworkRequest: r_id = %d , slotId = %d,", r_id , slotId);
    int preferSim = getNonSlotScopeStatusManager()->getIntValue(
            RFX_STATUS_KEY_PREFERRED_DATA_SIM);
    if (r_id != RIL_DATA_PROFILE_DEFAULT && r_id != RIL_DATA_PROFILE_IMS) {
        int ret = checkRequestExistInQueue(r_id, slotId);
        if (ret >= 0) {

            mOnDemandQueue.erase(mOnDemandQueue.begin()+ret);
            logD(RTC_DAC_LOG_TAG, "dequeueNetworkRequest: curr queue: %d",
                    (int)mOnDemandQueue.size());
        }
        if (mOnDemandQueue.size() == 0) {
            onSetDataAllow(preferSim);
            return FINISH_ALL_REQUEST;
        } else if (mOnDemandQueue.size() > 0){
            onSetDataAllow(mOnDemandQueue[0].slotId);
        }
    }
    return WAIT_NEXT_REQUEST;
}

void RtcDataAllowController::onSetDataAllow(int slotId) {
    int allowMessage = DISALLOW_DATA;
    int i;
    if(slotId < 0 || slotId >= RfxRilUtils::rfxGetSimCount()){
        logD(RTC_DAC_LOG_TAG, "onSetDataAllow: inValid slotId", slotId);
        return;
    }
    if(getAllowDataSlot() == slotId) {
        logD(RTC_DAC_LOG_TAG, "onSetDataAllow: slotId[%d] = true", slotId);
        return;
    }
    for (i = 0; i < RfxRilUtils::rfxGetSimCount(); i++) {
        if(i != slotId){
            sp<RfxMessage> msgDisallow = RfxMessage::obtainRequest(
                i, RFX_MSG_REQUEST_ALLOW_DATA, RfxIntsData(&allowMessage, 1));
            msgDisallow->setSlotId(i);
            RfxMainThread::enqueueMessage(msgDisallow);
        }
    }
    allowMessage = ALLOW_DATA;
    sp<RfxMessage> msgAllow = RfxMessage::obtainRequest(
            slotId, RFX_MSG_REQUEST_ALLOW_DATA, RfxIntsData(&allowMessage, 1));
    msgAllow->setSlotId(slotId);
    RfxMainThread::enqueueMessage(msgAllow);
    setAllowDataSlot(true, slotId);
}

bool RtcDataAllowController::onHandleRequest(const sp<RfxMessage>& message) {
    logV(RTC_DAC_LOG_TAG, "[%d]Handle request %s",
            message->getPToken(), RFX_ID_TO_STR(message->getId()));

    switch (message->getId()) {
    case RFX_MSG_REQUEST_ALLOW_DATA:
        // From Gen97, md has the ability to set EDALLOW=1/0 itself.
        // Do nothing and return success to framework for VTS:setdataallowed testcase.
        if (isMdSelfEdallow == 0) {
            preprocessRequest(message);
        } else {
            sp<RfxMessage> responseMsg = RfxMessage::obtainResponse(message->getSlotId(),
            RFX_MSG_REQUEST_ALLOW_DATA, RIL_E_SUCCESS, RfxVoidData(), message);
            responseToRilj(responseMsg);
            mDoingDataAllow = false;
        }
        break;
    case RFX_MSG_REQUEST_DATA_CONNECTION_ATTACH:
        handleDataConnectionAttachRequest(message);
        break;
    case RFX_MSG_REQUEST_DATA_CONNECTION_DETACH:
        handleDataConnectionDetachRequest(message);
        break;
    default:
        logD(RTC_DAC_LOG_TAG, "unknown request, ignore!");
        break;
    }
    return true;
}

bool RtcDataAllowController::onHandleResponse(const sp<RfxMessage>& message) {
    logV(RTC_DAC_LOG_TAG, "[%d]Handle response %s.",
            message->getPToken(), RFX_ID_TO_STR(message->getId()));

    switch (message->getId()) {
    case RFX_MSG_REQUEST_ALLOW_DATA:
        handleSetDataAllowResponse(message);
        break;
    case RFX_MSG_REQUEST_DATA_CONNECTION_ATTACH:
    case RFX_MSG_REQUEST_DATA_CONNECTION_DETACH:
        responseToRilj(message);
        break;
    case RFX_MSG_REQUEST_RECOVERY_ALLOW_DATA:
        logD(RTC_DAC_LOG_TAG, "Not handle recovery allow data response");
        break;
    default:
        logD(RTC_DAC_LOG_TAG, "unknown response, ignore!");
        break;
    }
    return true;
}

bool RtcDataAllowController::onPreviewMessage(const sp<RfxMessage>& message) {
    // This function will be called in the case of the registered request/response and urc.
    // For instance, register RIL_REQUEST_ALLOW_DATA will receive its request and response.
    // Therefore, it will be called twice in the way and we only care REQUEST in preview message,
    // but still need to return true in the case of type = RESPONSE.
    int requestToken = message->getPToken();
    int requestId = message->getId();

    // Only log REQUEST type.
    if (message->getType() == REQUEST
            && isNeedSuspendRequest(message)) {
        return false;
    } else {
        if (message->getType() == REQUEST && requestId == RFX_MSG_REQUEST_ALLOW_DATA) {
            logD(RTC_DAC_LOG_TAG, "[%d]onPreviewMessage: execute %s, type = [%d]",
                    requestToken,
                    RFX_ID_TO_STR(message->getId()),message->getType());
        }
        return true;
    }
}

bool RtcDataAllowController::isNeedSuspendRequest(const sp<RfxMessage>& message) {
    /*
     * white list for suspend request.
     */
    int requestToken = message->getPToken();
    int requestId = message->getId();
    if (requestId == RFX_MSG_REQUEST_ALLOW_DATA) {
        if (!mDoingDataAllow) {
            logD(RTC_DAC_LOG_TAG, "[%d]isNeedSuspendRequest: First RFX_MSG_REQUEST_ALLOW_DATA"
                    ", set flag on", requestToken);
            mDoingDataAllow = true;
            return false;
        } else {
            return true;
        }
    }
    return false;
}

bool RtcDataAllowController::onCheckIfResumeMessage(const sp<RfxMessage>& message) {
    int requestToken = message->getPToken();
    int requestId = message->getId();

    if (!mDoingDataAllow) {
        return true;
    }
    return false;
}

void RtcDataAllowController::handleSetDataAllowRequest(const sp<RfxMessage>& request) {
    const int *pRspData = (const int *)request->getData()->getData();
    bool allowData = pRspData[0];
    mReqDataAllow = allowData;

    logD(RTC_DAC_LOG_TAG, "[%d]handleSetDataAllowRequest: requestId:%d, phone:%d, allow:%d",
            request->getPToken(), request->getId(), request->getSlotId(), allowData);

    // For Telephonyware, if other non-android platform use set-data-allow mode,
    // here should records which sim is allowed. Otherwise, in set-preferred-data mode, should
    // record it at enqueue/dequeue level.
    if (!isPreferredDataMode()) {
        // Save the allow info into system property.
        setAllowDataSlot(allowData, request->getSlotId());
    }

    sp<RfxMessage> message = RfxMessage::obtainRequest(request->getSlotId(),
            request->getId(), request, true);
    requestToMcl(message);
}

void RtcDataAllowController::handleSetDataAllowResponse(const sp<RfxMessage>& response) {
    logD(RTC_DAC_LOG_TAG,
            "[%d]handleSetDataAllowResponse: allowData = %d, response->getError()=%d, getSlot()=%d",
            response->getPToken(), mReqDataAllow, response->getError(), response->getSlotId());
    if (RIL_E_REQUEST_NOT_SUPPORTED == response->getError()) {
        int allowMessage = INVAILD_ID;
        allowMessage = (mReqDataAllow == true)? ALLOW_DATA : DISALLOW_DATA;
        sp<RfxMessage> msg = RfxMessage::obtainRequest(response->getSlotId(),
                RFX_MSG_REQUEST_ALLOW_DATA, RfxIntsData(&allowMessage, 1));
        msg->setSlotId(response->getSlotId());
        logD(RTC_DAC_LOG_TAG,
                "[%d]RIL_E_REQUEST_NOT_SUPPORTED, resend: allowMessage = %d, getSlot()=%d",
                response->getPToken(), allowMessage, response->getSlotId());
        requestToMcl(msg, false, ms2ns(RETRY_TIME_MS));
        return;
    }

    /// RILD temp data solution. Only for OEM customization, unused in internal solution. @{
    // Need to restore allow data and data connection when temp data switch end.
    if (RtcDataUtils::isSupportTempDataSwitchFromOem()
            && getStatusManager(response->getSlotId())->getIntValue(
                    RFX_STATUS_KEY_TEMP_DATA_SWTICH, 0) == TEMP_DATA_SWITCH_ON
            && !mReqDataAllow) {
        logD(RTC_DAC_LOG_TAG, "[%s] restore temp data for slot [%d]", __FUNCTION__,
                (1 - response->getSlotId()));

        getStatusManager(response->getSlotId())->setIntValue(
                RFX_STATUS_KEY_TEMP_DATA_SWTICH, TEMP_DATA_SWITCH_OFF);
        setAllowDataSlot(true, (1 - response->getSlotId()));

        int allowMessage = ALLOW_DATA;
        sp<RfxMessage> msg = RfxMessage::obtainRequest(
                1 - response->getSlotId(),
                RFX_MSG_REQUEST_RECOVERY_ALLOW_DATA, RfxIntsData(&allowMessage, 1));
        msg->setSlotId(1 - response->getSlotId());
        requestToMcl(msg);
    }
    /// @}

    /*
     * Modem will return EDALLOW error (4117), in the case of command conflict.
     * The reason for this would be AP send EDALLOW=1 to both SIMs, therefore we need
     * to do error handling in this case
     */
    if (mReqDataAllow && (RIL_E_OEM_MULTI_ALLOW_ERR == response->getError())) {
        // RILD temp data solution, only for OEM customization, unused in internal solution. @{
        // Framework will set allow data on both slot without disallow data on data slot
        // So we need to handle the allow data conflict here
        // Set temp data switch state for the slot which has calls and allow data (1)
        if (RtcDataUtils::isSupportTempDataSwitchFromOem()) {
            int slotId = response->getSlotId();
            bool isInCall = getStatusManager(slotId)->getBoolValue(
                    RFX_STATUS_KEY_IN_CALL, false);
            logD(RTC_DAC_LOG_TAG, "[%d][%s] isInCall = %d", slotId, __FUNCTION__, isInCall);
            if (isInCall) {
                getStatusManager(slotId)->setIntValue(RFX_STATUS_KEY_TEMP_DATA_SWTICH,
                        TEMP_DATA_SWITCH_ON);
            }
        }
        /// @}
        handleMultiAllowError(response->getSlotId());
        return;
    }

    if (checkDisallowingPeer()) {
        // Deact Peer Result couldn't pass to RILJ, it will re-attach directly
        logD(RTC_DAC_LOG_TAG, "handleSetDataAllowResponse checkDisallowingPeer");
        return;
    }

    responseToRilj(response);
    mDoingDataAllow = false;
}

bool RtcDataAllowController::preprocessRequest(const sp<RfxMessage>& request) {
    const int *pRspData = (const int *)request->getData()->getData();
    bool allowData = pRspData[0];
    mReqDataAllow = allowData;

    if (allowData) {
        // Copy the request
        // 1. if allow true,  apply for retry.
        mLastAllowTrueRequest = RfxMessage::obtainRequest(request->getSlotId(),
                request->getId(), request, true);
    }
    handleSetDataAllowRequest(request);

    return true;
}

/*
 * Create request to disallow peer phone.
 * The follow will be:
 *   handleMultiAllowError ->
 *   -> handleSetDataAllowResponse -> handleSetDataAllowRequest(mLastAllowTrueRequest)
 * The last step means that re-attach for original attach request.
 */
void RtcDataAllowController::handleMultiAllowError(int activePhoneId) {
    // Check with ims module if we could detach
    int i = 0;
    int allowMessage = INVAILD_ID;

    logD(RTC_DAC_LOG_TAG, "detachPeerPhone: activePhoneId = %d", activePhoneId);

    // Detach peer phone
    for (i = 0; i < RfxRilUtils::rfxGetSimCount(); i++) {
        if (i != activePhoneId) {
            // Create disallow request for common
            allowMessage = DISALLOW_DATA;
            sp<RfxMessage> msg = RfxMessage::obtainRequest(
                    i, RFX_MSG_REQUEST_ALLOW_DATA, RfxIntsData(&allowMessage, 1));
            msg->setSlotId(i);

            logD(RTC_DAC_LOG_TAG, "disallowPeerPhone: precheck PhoneId = %d", i);
            // Notify disallow precheck
            mDisallowingPeer++;
            handleSetDataAllowRequest(msg);
        }
    }
}

/*
  * Check if the process is detaching peer.
  * Return true for ignoring response to RILJ because the requests are created by RILProxy.
*/
bool RtcDataAllowController::checkDisallowingPeer() {
    if (mDisallowingPeer > 0) {
        mDisallowingPeer--;
        logD(RTC_DAC_LOG_TAG, "handleSetDataAllowResponse consume disallow peer,"
                " mDisallowingPeer %d", mDisallowingPeer);
        if (mDisallowingPeer == 0) {
            // resume the attach request
            handleSetDataAllowRequest(mLastAllowTrueRequest);
            logD(RTC_DAC_LOG_TAG, "handleSetDataAllowResponse re-attach");
        }
        return true;
    }
    return false;
}

bool RtcDataAllowController::onCheckIfRejectMessage(const sp<RfxMessage>& message,
        bool isModemPowerOff,int radioState) {
    // always execute request
    if((radioState == (int)RADIO_STATE_OFF || radioState == (int)RADIO_STATE_UNAVAILABLE) &&
            (message->getId() == RFX_MSG_REQUEST_ALLOW_DATA ||
            // OEM customization, allow attach request because Java Fw will send detach
            // request first and radio state maybe in radio_off state.
            message->getId() == RFX_MSG_REQUEST_DATA_CONNECTION_ATTACH)) {
        return false;
    }
    return RfxController::onCheckIfRejectMessage(message, isModemPowerOff, radioState);
}

void RtcDataAllowController::onAttachOrDetachDone(const sp<RfxMessage> message) {
    logD(RTC_DAC_LOG_TAG, "[%d][%s]", message->getSlotId(), __FUNCTION__);

    // Lock radio lock for data use only to avoid other module do radio on/off
    if (message->getId() == RFX_MSG_REQUEST_DATA_CONNECTION_DETACH) {
        getStatusManager(message->getSlotId())->setIntValue(RFX_STATUS_KEY_RADIO_LOCK,
                RADIO_LOCK_BY_DATA);
    }
    responseToRilj(RfxMessage::obtainResponse(RIL_E_SUCCESS, message, false));
}

void RtcDataAllowController::handleDataConnectionAttachRequest(const sp<RfxMessage>& message) {
    const int *pReqData = (const int *)message->getData()->getData();
    logD(RTC_DAC_LOG_TAG, "handleDataConnectionAttachRequest: type=%d", pReqData[0]);

    // 0, ps attach, 1, ps&cs attach.
    // For ps attach request, will be sent to RMC layer.
    // For ps&cs attach request, call poweron API to turn on radio.
    if (pReqData[0] == 1) {
        // Reset radio lock
        for (int slotId = RFX_SLOT_ID_0; slotId < RfxRilUtils::rfxGetSimCount(); slotId++) {
            getStatusManager(slotId)->setIntValue(RFX_STATUS_KEY_RADIO_LOCK, RADIO_LOCK_IDLE);
        }

        sp<RfxAction> action = new RfxAction1<const sp<RfxMessage>>(this,
                &RtcDataAllowController::onAttachOrDetachDone, message);
        RtcRadioController* radioController =
            (RtcRadioController *)findController(message->getSlotId(),
                    RFX_OBJ_CLASS_INFO(RtcRadioController));
        radioController->moduleRequestRadioPower(true, action, RFOFF_CAUSE_UNSPECIFIED);
    } else {
        requestToMcl(message);
    }
}

void RtcDataAllowController::handleDataConnectionDetachRequest(const sp<RfxMessage>& message) {
    const int *pReqData = (const int *)message->getData()->getData();
    logD(RTC_DAC_LOG_TAG, "handleDataConnectionDetachRequest: type=%d", pReqData[0]);

    // 0, ps detach, 1, ps&cs detach.
    // For ps detach request, will be sent to RMC layer.
    // For ps&cs detach request, call poweron API to turn off radio.
    if (pReqData[0] == 1) {
        // Reset radio lock
        for (int slotId = RFX_SLOT_ID_0; slotId < RfxRilUtils::rfxGetSimCount(); slotId++) {
            getStatusManager(slotId)->setIntValue(RFX_STATUS_KEY_RADIO_LOCK, RADIO_LOCK_IDLE);
        }

        sp<RfxAction> action = new RfxAction1<const sp<RfxMessage>>(this,
                &RtcDataAllowController::onAttachOrDetachDone, message);
        RtcRadioController* radioController =
            (RtcRadioController *)findController(message->getSlotId(),
                    RFX_OBJ_CLASS_INFO(RtcRadioController));
        radioController->moduleRequestRadioPower(false, action, RFOFF_CAUSE_UNSPECIFIED);
    } else {
        requestToMcl(message);
    }
}

/*
 *  To save the allow slot info for the purpose of resend allow data.
 *  It will trigger by each send allow request from the handleSetDataAllowRequest.
 */
void RtcDataAllowController::setAllowDataSlot(bool allow, int slot) {
    if (allow) {
        logI(RTC_DAC_LOG_TAG, "setAllowDataSlot allow= %d slot= %d", allow ,slot);
        rfx_property_set("vendor.ril.data.allow_data_slot", String8::format("%d", slot).string());
    }
}

/*
 *  Get the allw slot last set by RTC_DAC.
 */
int RtcDataAllowController::getAllowDataSlot() {
    int allowMessage = INVAILD_ID;
    char allowDataSlot[RFX_PROPERTY_VALUE_MAX] = { 0 };
    rfx_property_get("vendor.ril.data.allow_data_slot", allowDataSlot, "-1");
    int allowSlot = atoi(allowDataSlot);
    logI(RTC_DAC_LOG_TAG, "getAllowDataSlot slot prop = %d", allowSlot);
    return allowSlot;
}

/*
 * The use of resend allow data from non-Android platform, since MD may need the allow info
 * in the case of
 *     1. SIM switch (after +ESIMMAP)
 *     2. RILD-reconnect (TRM)
 * In the non-android platform framework, there's no such entry point to trigger resend data allow
 * from framework to modem.
 * Therefore, we need to do this in the native layer.
 */
void RtcDataAllowController::resendAllowData() {
    // From Gen97, md has the ability to do EDALLOW=1/0 itself.
    // There's no need to resend EDALLOW=1/0 anymore.
    if (isMdSelfEdallow) {
        logI(RTC_DAC_LOG_TAG, "resendAllowData MdSelfEdallow, do nothing");
        return;
    }
    // Get the ALLOW_DATA info from the property which set by ALLOW_DATA request from porting layer
    int i = 0;
    int allowMessage = INVAILD_ID;

    char allowDataSlot[RFX_PROPERTY_VALUE_MAX] = { 0 };
    rfx_property_get("vendor.ril.data.allow_data_slot", allowDataSlot, "-1");
    int allowSlot = atoi(allowDataSlot);
    logI(RTC_DAC_LOG_TAG, "resendAllowData slot prop = %d", allowSlot);

    // Process the request again
    for (i = 0; i < RfxRilUtils::rfxGetSimCount(); i++) {
    // Always resend the slot of EDALLOW=1
    //TODO: check if we need to re-send EDALLOW=0
        if (i == allowSlot) {
            allowMessage = ALLOW_DATA;
            sp<RfxMessage> msg = RfxMessage::obtainRequest(
                    i, RFX_MSG_REQUEST_ALLOW_DATA, RfxIntsData(&allowMessage, 1));
            msg->setSlotId(i);
            logD(RTC_DAC_LOG_TAG, "resendAllowData: PhoneId = %d, allow =%d", i, allowMessage);
            // Resend allow request to common RIL queue to avoid request mismatch
            // Otherwise, RIL will receive unexpected response resulting exception.
            RfxMainThread::enqueueMessage(msg);
        }
    }
}

bool RtcDataAllowController::isPreferredDataMode() {
    if (mIsPreferredDataMode != -1) {
        //logD(RTC_DAC_LOG_TAG, "isPreferredDataMode:  %d", mIsPreferredDataMode);
        return (mIsPreferredDataMode == 1) ? true : false;
    }
    char preferredDataMode[MTK_PROPERTY_VALUE_MAX] = {0};
    mtk_property_get("vendor.ril.data.preferred_data_mode", preferredDataMode, "0");
    logD(RTC_DAC_LOG_TAG, "isPreferredDataMode: preferredDataMode = %d", atoi(preferredDataMode));
    if (atoi(preferredDataMode) != 1) {
        mIsPreferredDataMode = 0;
        return false;
    }
    mIsPreferredDataMode = 1;
    return true;
}

void RtcDataAllowController::onPreferredChanged(RfxStatusKeyEnum key,
        RfxVariant old_value, RfxVariant value) {
    RFX_UNUSED(key);
    RFX_UNUSED(old_value);
    int dataSim = value.asInt();

    char legacyMode[RFX_PROPERTY_VALUE_MAX] = { 0 };
    rfx_property_get("vendor.ril.data.legacy_allow_mode", legacyMode, "-1");
    int isLegacyMode = atoi(legacyMode);
    if (isLegacyMode == 1) {
        logI(RTC_DAC_LOG_TAG, "onPreferredChanged, and isLegacyMode=1, set allowSlot to -1");
        rfx_property_set("vendor.ril.data.legacy_allow_mode", "-1");
        mIsPreferredDataMode = 1;
        setAllowDataSlot(true, -1);
    }

    if(mOnDemandQueue.size() == 0 && dataSim >= 0){
            logD(RTC_DAC_LOG_TAG, "onPreferredChanged, try to set allow to %d", dataSim);
            onSetDataAllow(dataSim);
    } else if(mOnDemandQueue.size() > 0) {
        logD(RTC_DAC_LOG_TAG, "onPreferredChanged, not acting, due to type=%d, slot=%d",
                mOnDemandQueue[0].type, mOnDemandQueue[0].slotId);
    }
}

void RtcDataAllowController::onHidlStateChanged(RfxStatusKeyEnum key,
        RfxVariant old_value, RfxVariant value) {
    bool oldState = false, newState = false;

    RFX_UNUSED(key);
    oldState = old_value.asBool();
    newState = value.asBool();

    logD(RTC_DAC_LOG_TAG, "onHidlStateChanged: old state:%d, newstate:%d, clear queue", oldState,
            newState);
    mOnDemandQueue.clear();
}

void RtcDataAllowController::onSimMeLockChanged(int slotId, RfxStatusKeyEnum key,
        RfxVariant old_value, RfxVariant value) {
    int policy = 0;
    int shouldService = 0;

    RFX_UNUSED(key);
    RFX_UNUSED(old_value);
    shouldService = value.asInt();
    policy = getNonSlotScopeStatusManager()->getIntValue(RFX_STATUS_KEY_SML_SLOT_LOCK_POLICY, 0);
    logD(RTC_DAC_LOG_TAG, "SimMeLockChanged[%d]: policy = %d, shouldService = %d",
            slotId, policy, shouldService);
    if (policy == SML_SLOT_LOCK_POLICY_UNKNOWN ||
            policy == SML_SLOT_LOCK_POLICY_NONE ||
            policy == SML_SLOT_LOCK_POLICY_LEGACY) {
        return;
    }
    if (shouldService != SML_SLOT_LOCK_POLICY_SERVICE_CAPABILITY_FULL &&
        shouldService != SML_SLOT_LOCK_POLICY_SERVICE_CAPABILITY_PS_ONLY) {
        mOnDemandQueue.erase(
                std::remove_if(mOnDemandQueue.begin(), mOnDemandQueue.end(),
                [slotId](const onDemandRequest & request) { return request.slotId == slotId; }),
                mOnDemandQueue.end());
        if (mOnDemandQueue.size() == 0) {
            int preferSim = getNonSlotScopeStatusManager()->getIntValue(
                    RFX_STATUS_KEY_PREFERRED_DATA_SIM);
            logD(RTC_DAC_LOG_TAG, "SimMeLockChanged[%d]: set back to prefer sim: %d",
                    slotId, preferSim);
            onSetDataAllow(preferSim);
        } else if (mOnDemandQueue.size() > 0) {
            logD(RTC_DAC_LOG_TAG, "SimMeLockChanged[%d]: set to on-demand: type: %d, sim: %d",
                    slotId, mOnDemandQueue[0].type, mOnDemandQueue[0].slotId);
            onSetDataAllow(mOnDemandQueue[0].slotId);
        }
    }
}
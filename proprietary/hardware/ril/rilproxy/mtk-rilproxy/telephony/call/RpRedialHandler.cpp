/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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
#include "assert.h"

#include <compiler_utils.h>
#include <cutils/properties.h>
#include <cutils/jstring.h>
#include <libmtkrilutils.h>
#include "RfxMainThread.h"
#include "RpRedialHandler.h"
#include "nw/RpNwUtils.h"
#include "RpCallController.h"
#include "power/RadioConstants.h"
#include "MTKPrimaryDevicesHalClientInterface.h"

extern "C" int RatConfig_isC2kSupported();
extern "C" int RatConfig_isLteFddSupported();
extern "C" int RatConfig_isLteTddSupported();
extern "C" int RatConfig_isWcdmaSupported();
extern "C" int RatConfig_isTdscdmaSupported();
extern "C" int RatConfig_isGsmSupported();

/*****************************************************************************
 * Class RfxController
 *****************************************************************************/
RFX_IMPLEMENT_CLASS("RpRedialHandler", RpRedialHandler, RfxController);

RpRedialHandler::RpRedialHandler() {
    mRedialState = REDIAL_NONE;
    mIsUserHangUping = false;
    mInEmergencyMode = false;
    mMoCall = NULL;
    mPreRatMode = REDIAL_NET_TYPE_NONE;
    mHvolteMode = HVOLTE_MODE_LTE_ONLY;
    mEcbm = E911_ECBM_NONE;
    mShouldDropGetCurrentCalls = false;
}

RpRedialHandler::~RpRedialHandler() {
    freeMoCallObject();
    return;
}

void RpRedialHandler::onInit() {
    RfxController::onInit();  // Required: invoke super class implementation
    RFX_LOG_D(RP_REDIAL_TAG, "onInit !!");

    const int request_id_list[] = {
        RIL_REQUEST_OEM_HOOK_ATCI_INTERNAL,
    };

    // register request & URC id list
    // NOTE. one id can only be registered by one controller
    registerToHandleRequest(request_id_list, sizeof(request_id_list)/sizeof(int), MEDIUM);

    // register to CS call count change
    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_VOICE_CALL_COUNT,
        RfxStatusChangeCallback(this, &RpRedialHandler::onCsCallCountChanged));

    return;
}

void RpRedialHandler::onDeinit() {
    getStatusManager()->unRegisterStatusChanged(RFX_STATUS_KEY_VOICE_CALL_COUNT,
        RfxStatusChangeCallback(this, &RpRedialHandler::onCsCallCountChanged));
    RFX_LOG_D(RP_REDIAL_TAG, "onDeinit");
    RfxController::onDeinit();

    return;
}

void RpRedialHandler::onGlobalModeSwitchDone(int slotId) {
    RFX_UNUSED(slotId);
    // If Redial Handler is not in emergency mode,
    // it's impossible to change to global mode.
    REDIAL_DBG_ASSERT(mInEmergencyMode == true);

    if (mRedialState != REDIAL_CHANGE_GLOBAL_MODE) {
        // This may occur when RedialHandler leave global mode before global
        // mode switch procedure complete.
        RFX_LOG_D(RP_REDIAL_TAG, "Already leave CHANGE_GLOBAL_MODE state, not redial to MD1");
        return;
    }

    RFX_LOG_D(RP_REDIAL_TAG, "Change to global mode compelte, redial to MD1");

    const sp<RfxMessage> rilRequest = getRedialRequest(RADIO_TECH_GROUP_GSM, false);

    // rilReqeust may be NULL when user hangup during global mode change.
    if (rilRequest != NULL) {
        requestToRild(rilRequest);
        setRedialState(REDIAL_TO_MD1);
    }

    return;
}

void RpRedialHandler::onGlobalModeResumeDone(int slotId) {
    // If Redial Handler is not in emergency mode,
    // it's impossible to change to global mode.
    REDIAL_DBG_ASSERT(mInEmergencyMode == true);
    REDIAL_DBG_ASSERT(mPreRatMode == REDIAL_NET_TYPE_RESUME_RAT_MODE);

    RFX_LOG_D(RP_REDIAL_TAG, "Resume from global mode complete");

    setEmergencyMode(false, slotId);
    setPreRatMode(REDIAL_NET_TYPE_NONE);

    return;
}

void RpRedialHandler::onAudioModeSwitchDone() {
    RFX_LOG_D(RP_REDIAL_TAG, "Switch audio mode complete");
    resumeMsg();
    return;
}

void RpRedialHandler::onCapabilityChanged(RfxStatusKeyEnum key,
        RfxVariant old_value, RfxVariant value) {
    RFX_UNUSED(key);
    int radioState;
    int suggestedCapability;
    int radioCapability = value.asInt();

    suggestedCapability = getStatusManager()->
        getIntValue(RFX_STATUS_KEY_SUGGESTED_RADIO_CAPABILITY, RIL_CAPABILITY_NONE);
    radioState = getStatusManager()->
        getIntValue(RFX_STATUS_KEY_RADIO_STATE, RADIO_STATE_OFF);

    RFX_LOG_D(RP_REDIAL_TAG,
            "Radio capability change: %d -> %d, suggested capability: %d, slot ID:%d",
            old_value.asInt(), radioCapability, suggestedCapability, getSlotId());

    if ((mInEmergencyMode == true) &&
        (radioState == RADIO_STATE_ON) &&
        (radioCapability == suggestedCapability)) {
        // UE turn on radio to dial emergency call. There may be deferred
        // emergency request which wait for execution after radio on.
        RFX_LOG_D(RP_REDIAL_TAG, "Resume deferred E911 message after radio on");
        resumeMsg();
    }

    return;
}

void RpRedialHandler::onRatModeSwitch(RfxStatusKeyEnum key,
        RfxVariant old_value, RfxVariant value) {
    RFX_UNUSED(key);
    bool isRatModeSwitch = value.asBool();

    RFX_LOG_D(RP_REDIAL_TAG,
            "RAT/NWS mode switching: %s -> %s, slot ID:%d",
            boolToString(old_value.asBool()), boolToString(isRatModeSwitch), getSlotId());

    if (isRatModeSwitch == false) {
        // There may be deferred msg during RAT/NW mode switch.
        // After switch complete, resume deferred msg and re-process again.
        RFX_LOG_D(RP_REDIAL_TAG, "Resume deferred E911 message after RAT/NWS mode switch done");

        getStatusManager()->unRegisterStatusChanged(RFX_STATUS_KEY_IS_RAT_MODE_SWITCHING,
                RfxStatusChangeCallback(this, &RpRedialHandler::onRatModeSwitch));

        resumeMsg();
    }

    return;
}

bool RpRedialHandler::onHandleAtciRequest(const sp<RfxMessage>& msg) {
    int     msgId   = msg->getId();
    int     slotId  = msg->getSlotId();
    bool    result  = false;
    Parcel  *p      = msg->getParcel();

    RFX_LOG_D(RP_REDIAL_TAG, "handle request:%s (slot:%d)", requestToString(msgId),
            msg->getSlotId());

    switch (msgId) {
    case RIL_REQUEST_OEM_HOOK_ATCI_INTERNAL:
        int         dataLength;
        const char  *data;

        dataLength = p->readInt32();
        data = p->readCString();

        RFX_LOG_D(RP_REDIAL_TAG, "Inject AT command %s (length:%d)", data, dataLength);

        if (strncmp(data, "ATD", 3) == 0) {
            result = handleAtciDialRequest(data, dataLength, slotId);
        }
        break;

    default:
        RFX_LOG_E(RP_REDIAL_TAG, "unexpected ATCI request:%s (slot:%d)",
                requestToString(msgId), msg->getSlotId());
        break;
    }

    return result;
}

bool RpRedialHandler::onHandleAtciResponse(const sp<RfxMessage>& msg) {
    int msgId = msg->getId();

    switch (msgId) {
    case RIL_REQUEST_OEM_HOOK_ATCI_INTERNAL:
        break;

    default:
        RFX_LOG_E(RP_REDIAL_TAG, "unexpected ATCI response:%s (slot:%d)",
                requestToString(msgId), msg->getSlotId());
        break;
    }

    return false;
}

void RpRedialHandler::onCsCallCountChanged(RfxStatusKeyEnum key,
    RfxVariant oldValue, RfxVariant newValue) {
    RFX_UNUSED(key);
    RFX_LOG_D(RP_REDIAL_TAG, "onCsCallCountChanged oldValue %d, newValue %d",
        oldValue.asInt(), newValue.asInt());

    if ((mPreRatMode == REDIAL_NET_TYPE_RESUME_RAT_MODE_PENDING) && (newValue.asInt() == 0)) {
        // Continue pending resume RAT mode action
        setRatMode(mPreRatMode, getSlotId());
    }
}

bool RpRedialHandler::notifyRilRequest(const sp<RfxMessage>& msg) {
    int     msgId = msg->getId();
    bool    isSendToRild = true;

    RFX_LOG_D(RP_REDIAL_TAG, "handle req:%s (slot:%d)",
            requestToString(msgId), msg->getSlotId());

    // In non-C2K project, there is no hVoLTE/E911 redial scenario.
    if ((checkRedialFeatureEnabled(REDIAL_SBP_ID_VZW, REDIAL_TC_ID_COMMON) == false) &&
        (checkRedialFeatureEnabled(REDIAL_SBP_ID_SPRINT, REDIAL_TC_ID_COMMON) == false)) {
        return true;
    }

    // Suspend incoming message if there is other suspended message
    if (mSuspendedMsgQueue.isEmpty() == false) {
        RFX_LOG_D(RP_REDIAL_TAG,
                "Suspend incoming message because other suspended message existed");
        deferMsg(msg);
        return false;
    }

    switch (msgId) {
        // Notify Redial Handler
        case RIL_REQUEST_DIAL:
        case RIL_REQUEST_IMS_DIAL:
        case RIL_LOCAL_REQUEST_EMERGENCY_DIAL:
        case RIL_LOCAL_REQUEST_IMS_EMERGENCY_DIAL:
            isSendToRild = handleDialRequest(msg);
            mShouldDropGetCurrentCalls = true;
            break;

        case RIL_REQUEST_HANGUP:
            isSendToRild = handleHangUp(msg);
            break;

        case RIL_REQUEST_HANGUP_ALL:
            if (mMoCall != NULL) {
                setUserHangUping(true);
            }
            break;

        case RIL_REQUEST_SET_ECC_MODE:
            if (mInEmergencyMode == false) {
                setEmergencyMode(true, msg->getSlotId());
            } else {
                RFX_LOG_D(RP_REDIAL_TAG, "Already in Emergency mode, send RIL response directly");

                sp<RfxMessage>  rilResponse = RfxMessage::obtainResponse(RIL_E_SUCCESS, msg);
                responseToRilj(rilResponse);
                isSendToRild = false;
            }
            break;

        default:
            RFX_LOG_E(RP_REDIAL_TAG, "unexpected request:%s (slot:%d)", requestToString(msgId),
                    msg->getSlotId());
            break;
    }

    RFX_LOG_D(RP_REDIAL_TAG, "notifyRilRequest complete(isSendToRild:%s)",
            boolToString(isSendToRild));

    return isSendToRild;
}

const sp<RfxMessage> RpRedialHandler::notifyRilResponse(const sp<RfxMessage>& msg) {
    int             msgId = msg->getId();
    sp<RfxMessage>  rilResponse = msg;

    // In non-C2K project, there is no hVoLTE/E911 redial scenario.
    // We don't need to modify the RIL response.
    if ((checkRedialFeatureEnabled(REDIAL_SBP_ID_VZW, REDIAL_TC_ID_COMMON) == false) &&
        (checkRedialFeatureEnabled(REDIAL_SBP_ID_SPRINT, REDIAL_TC_ID_COMMON) == false)) {
        return msg;
    }

    RFX_LOG_D(RP_REDIAL_TAG, "handle response:%s (slot:%d)", requestToString(msgId),
            msg->getSlotId());

    switch (msgId) {
    case RIL_REQUEST_GET_CURRENT_CALLS:
        if (checkDropUrc(msg) == true) {
            RFX_LOG_D(RP_REDIAL_TAG,
                    "Let poll call failed because already redial to other modem");

            // Modified the original RIL response
            rilResponse = RfxMessage::obtainResponse(RIL_E_GENERIC_FAILURE, msg);
        } else if (isImsSupport() != 1 && mShouldDropGetCurrentCalls && getCallCount(msg) == 0) {
            RFX_LOG_D(RP_REDIAL_TAG, "Let poll call failed during redial flow");

            // Modified the original RIL response
            rilResponse = RfxMessage::obtainResponse(RIL_E_GENERIC_FAILURE, msg);
        }
        break;

    case RIL_LOCAL_REQUEST_SET_HVOLTE_MODE:
        handleSetHvolteModeRsp(msg);
        break;

    case RIL_LOCAL_C2K_REQUEST_REDIAL:
    case RIL_LOCAL_REQUEST_EMERGENCY_REDIAL:
    case RIL_LOCAL_C2K_REQUEST_EMERGENCY_REDIAL:
        mShouldDropGetCurrentCalls = false;
        if (msg->getError() != RIL_E_SUCCESS) {
            sp<RfxMessage> callStateChanged = RfxMessage::obtainUrc(
                    msg->getSlotId(), RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED);

            responseToRilj(callStateChanged);
        }
        rilResponse = NULL;
        // fall through
        TELEPHONYWARE_FALLTHROUGH;

    case RIL_REQUEST_DIAL:
    case RIL_REQUEST_IMS_DIAL:
        if ((msg->getSource() == RADIO_TECH_GROUP_C2K) &&
            (msg->getError() != RIL_E_SUCCESS)) {
            // Notify MD1 to switch to SRLTE mode because MD3 call failed
            sendHvolteModeReq(0, 2);
        }
        break;


    case RIL_LOCAL_REQUEST_EMERGENCY_SESSION_BEGIN:
    case RIL_LOCAL_REQUEST_EMERGENCY_SESSION_END:
        // Local RIL REQUEST send by RpRedialController => Not send to RILJ
        rilResponse = NULL;
        break;

    default:
        RFX_LOG_E(RP_REDIAL_TAG, "unexpected response:%s (slot:%d)", requestToString(msgId),
                msg->getSlotId());
        break;
    }

    return rilResponse;
}

bool RpRedialHandler::notifyRilUrc(const sp<RfxMessage>& msg) {
    int     msgId = msg->getId();
    bool    isSendToRilj = true;

    // In non-C2K project, there is no hVoLTE/E911 redial scenario.
    if ((checkRedialFeatureEnabled(REDIAL_SBP_ID_VZW, REDIAL_TC_ID_COMMON) == false) &&
        (checkRedialFeatureEnabled(REDIAL_SBP_ID_SPRINT, REDIAL_TC_ID_COMMON) == false)) {
        // Enter or exit ECBM is common flow, need to handle in any project.
        if ((msgId != RIL_UNSOL_ENTER_EMERGENCY_CALLBACK_MODE) &&
            (msgId != RIL_UNSOL_EXIT_EMERGENCY_CALLBACK_MODE)) {
            return true;
        }
    }

    RFX_LOG_D(RP_REDIAL_TAG, "handle URC:%s (slot:%d)", requestToString(msgId),
            msg->getSlotId());

    switch (msgId) {
    case RIL_UNSOL_CALL_INFO_INDICATION:
        isSendToRilj = handleCallInfoInd(msg);
        break;

    case RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED:
        isSendToRilj = handleCallStateChanged(msg);
        break;

    case RIL_UNSOL_ENTER_EMERGENCY_CALLBACK_MODE:
        isSendToRilj = handleEnterEcbm(msg);
        break;

    case RIL_UNSOL_EXIT_EMERGENCY_CALLBACK_MODE:
        isSendToRilj = handleExitEcbm(msg);
        break;

    case RIL_UNSOL_SRVCC_STATE_NOTIFY:
        isSendToRilj = handleSrvcc(msg);
        break;

    default:
        RFX_LOG_E(RP_REDIAL_TAG, "unexpected URC:%s (slot:%d)", requestToString(msgId),
                msg->getSlotId());
        break;
    }

    RFX_LOG_D(RP_REDIAL_TAG, "notifyRilUrc complete(sendToRilj:%s)",
            boolToString(isSendToRilj));

    return isSendToRilj;
}

bool RpRedialHandler::handleDialRequest(const sp<RfxMessage>& msg) {
    char    *callNumber;
    bool    isSendToRild = true;
    bool    isEcc   = false;
    int     msgId   = msg->getId();
    int     slotId  = msg->getSlotId();
    Parcel  *p      = msg->getParcel();
    String8 currentAudioPhoneModemStr = String8("");

    if (msgId == RIL_LOCAL_REQUEST_EMERGENCY_DIAL ||
            msgId == RIL_LOCAL_REQUEST_IMS_EMERGENCY_DIAL) {
        isEcc = true;
    }
    mMoCall = new MoCallContext(*p, isEcc, slotId, msgId);

    currentAudioPhoneModemStr =
        android::MTKPrimaryDevicesHalClient::getInstance()->getParameters(
                RpAudioControlHandler::audioPhone1Md);
    String8 audioPhone1MdC2k = RpAudioControlHandler::audioPhone1Md +
        RpAudioControlHandler::modemC2k;
    // Dial to MD3 in LTE only mode => Change hVoLTE mode to SRLTE first
    if (currentAudioPhoneModemStr == audioPhone1MdC2k) {
        RFX_LOG_D(RP_REDIAL_TAG, "Notify MD1 to switch to SRLTE mode before dial to MD3");

        sendHvolteModeReq(0, 1); // SRLTE sub mode => search MD3 without swithing mode

        deferMsg(msg);

        isSendToRild = false;
    }
    // Not dial to MD3
    // => no need to change hVoLTE mode, dial directly

    return isSendToRild;
}

bool RpRedialHandler::handleAtciDialRequest(const char* data, int dataLength, int slotId) {
    int     cmdOffset   = 0;
    bool    isEcc       = false;
    char    *callNumber = NULL;
    char    simNo[PROPERTY_VALUE_MAX] = {0};

    property_get("persist.vendor.service.atci.sim", simNo, "0");
    if ((simNo[0] != '0') && (simNo[0] != '1')) {
        RFX_LOG_E(RP_REDIAL_TAG, "User inject MD1 dial command to wrong MD => ignore");
        return false;
    }

    if (mMoCall != NULL) {
        RFX_LOG_E(RP_REDIAL_TAG, "Simultaneous dial request, process late one");
        freeMoCallObject();
    }

    // ATDE: emergency call
    if (strncmp(data, "ATDE", 4) == 0) {
        setEmergencyMode(true, slotId);
        isEcc = true;
        cmdOffset = 4;
    }
    // ATD: normal call
    else {
        isEcc = false;
        cmdOffset = 3;
    }

    callNumber = new char[dataLength] {0};

    for (int i = 0; i < dataLength - cmdOffset; i++) {
        if (data[i + cmdOffset] == ';') {
            callNumber[i] = '\0';
            break;
        }
        callNumber[i] = data[i + cmdOffset];
    }

    mMoCall = new MoCallContext(callNumber, isEcc, slotId, 0);
    delete [] callNumber;

    return false;
}

bool RpRedialHandler::handleEnterEcbm(const sp<RfxMessage>& msg) {
    bool    isSendToRilj = true;

    if (msg->getSource() == RADIO_TECH_GROUP_C2K) {
        setEcbm(E911_ECBM_ENTER_C2K);

        // MD3 enter ECBM => Notify MD1 to reset WWAN timer
        sp<RfxMessage> rilRequest = RfxMessage::obtainRequest(
                msg->getSlotId(), RADIO_TECH_GROUP_GSM, RIL_REQUEST_SET_E911_STATE);

        Parcel *p = rilRequest->getParcel();
        p->writeInt32(2);                       // length
        p->writeInt32(E911_CALL_ENTER_ECBM);    // E911 state
        p->writeInt32(E911_RAT_C2K);            // RAT

        requestToRild(rilRequest);
    } else {
        setEcbm(E911_ECBM_ENTER_GSM_LTE);
    }

    RFX_LOG_D(RP_REDIAL_TAG, "handleEnterEcbm complete(sendToRilj:%s)",
            boolToString(isSendToRilj));

    return isSendToRilj;
}

bool RpRedialHandler::handleExitEcbm(const sp<RfxMessage>& msg) {
    bool    isSendToRilj = true;

    if (msg->getSource() == RADIO_TECH_GROUP_C2K) {
        // MD3 exit ECBM => Notify MD1 for MD3 ECBM status
        sp<RfxMessage> rilRequest = RfxMessage::obtainRequest(
                msg->getSlotId(), RADIO_TECH_GROUP_GSM, RIL_REQUEST_SET_E911_STATE);

        Parcel *p = rilRequest->getParcel();
        p->writeInt32(2);               // length
        p->writeInt32(E911_CALL_EXIT);  // E911 state
        p->writeInt32(E911_RAT_C2K);    // RAT

        requestToRild(rilRequest);
    }

    setEcbm(E911_ECBM_NONE);

    // If there is ongoing emergency call, no need to exit emergency mode
    if (mMoCall == NULL || (mMoCall->isEccCall() == false)) {
        handleExitEmergencyMode(msg);
    }

    RFX_LOG_D(RP_REDIAL_TAG, "handleExitEcbm complete(sendToRilj:%s)",
            boolToString(isSendToRilj));

    return isSendToRilj;
}

bool RpRedialHandler::handleExitEmergencyMode(const sp<RfxMessage>& msg) {
    RFX_UNUSED(msg);
    bool    isSendToRilj = true;

    if (mPreRatMode > REDIAL_NET_TYPE_NONE) {
        // Resume RAT mode first, then leave emergency mode
        setRatMode(mPreRatMode, getSlotId());
    }
    else if (mPreRatMode == REDIAL_NET_TYPE_NONE) {
        // Leave Emergency Callbaack mode => Also leave emergency mode
        setEmergencyMode(false, getSlotId());
    }

    RFX_LOG_D(RP_REDIAL_TAG, "handleExitEmergencyMode complete(sendToRilj:%s)",
            boolToString(isSendToRilj));

    return isSendToRilj;
}

bool RpRedialHandler::handleCallInfoInd(const sp<RfxMessage>& msg) {
    int     length = 0;
    int     callId, msgType, discCause = 0;
    bool    isSendToRilj = true;
    Parcel  *p = msg->getParcel();
    char*   callInfo[10] = {NULL};

    length = p->readInt32();
    RFX_LOG_D(RP_REDIAL_TAG, "ECPI parameter: %d", length);
    for (int i = 0; i < length; i++) {
        callInfo[i] = strdupReadString(*p);
    }

    RFX_LOG_D(RP_REDIAL_TAG, "ECPI: %s, %s, %s, %s, %s, %s, %s, %s, %s, %s",
            callInfo[0], callInfo[1], callInfo[2], callInfo[3], callInfo[4],
            callInfo[5], (isUserLoad() ? "[hidden]" : callInfo[6]), callInfo[7], callInfo[8], callInfo[9]);

    if ((length < 2) || (callInfo[0] == NULL) || (callInfo[1] == NULL)) {
        // TODO: error handling
        RFX_LOG_E(RP_REDIAL_TAG, "ECPI parameter error");
        REDIAL_ASSERT(0);
        return isSendToRilj;
    }

    callId  = atoi(callInfo[0]);
    msgType = atoi(callInfo[1]);

    if (callInfo[9] != NULL) {
        discCause = atoi(callInfo[9]);
    }

    for (int i = 0; i < length; i++) {
        if (callInfo[i] != NULL) {
            free(callInfo[i]);
        }
    }

    RFX_LOG_D(RP_REDIAL_TAG, "CALL_INFO_IND(call ID:%d, msg type:%d, disconnect cause:%d)",
            callId, msgType, discCause);

    // No MO dailed call => no need to parse URC
    if (mMoCall == NULL) {
        RFX_LOG_D(RP_REDIAL_TAG, "No ongoing MO dialing call, send to RILJ directly ");

        if ((msg->getSource() == RADIO_TECH_GROUP_C2K) &&
            (msgType == ECPI_MSG_TYPE_CALL_DISCONNECT)) {
            // Notify MD1 to switch to SRLTE mode because MD3 call end
            sendHvolteModeReq(0, 2);
        }
        return true;
    }

    // Received URC is not belonged to ongoing MO call => no need to parse URC
    if ((msgType != ECPI_MSG_TYPE_CALL_ID_ASSIGN) && (mMoCall->getCallId() != callId)) {
        RFX_LOG_D(RP_REDIAL_TAG, "Call ID mismatch, no need to parse CALL_INFO ");
        if (checkDropUrc(msg) == false) {
            return true;
        }
        else {
            return false;
        }
    }

    switch (msgType) {
    // Call connect
    case ECPI_MSG_TYPE_CALL_CONNECT:
        isSendToRilj = processCallConnect(msg);
        break;

    // Call ID assign
    case ECPI_MSG_TYPE_CALL_ID_ASSIGN:
        // Received the call ID assign for the redialed call => drop it
        if (mMoCall->getCallId() != 0) {
            REDIAL_DBG_ASSERT(mMoCall->getCallId() == callId);

            RFX_LOG_D(RP_REDIAL_TAG, "Redialed call, no need to notify Call ID assign again");
            isSendToRilj = false;
        }
        else {
            mMoCall->setCallId(callId);
        }
        break;

    // Call disconnect
    case ECPI_MSG_TYPE_CALL_DISCONNECT:
        isSendToRilj = processCallDisconnect(msg, discCause);
        break;

    default:
        if (checkDropUrc(msg) == true) {
            isSendToRilj = false;
        }
        break;
    }

    RFX_LOG_D(RP_REDIAL_TAG, "handleCallInfoInd complete(callID:%d, msg type:%d, sendToRilj:%s)",
           callId, msgType, boolToString(isSendToRilj));

    return isSendToRilj;
}

bool RpRedialHandler::handleCallStateChanged(const sp<RfxMessage>& msg) {
    bool isSendToRilj = true;

    // Send to RILJ if current phone object need to handle CALL STATE CHANGED
    if (checkDropUrc(msg) == true) {
        isSendToRilj = false;
    }

    RFX_LOG_D(RP_REDIAL_TAG, "handleCallStateChanged complete(sendToRilj:%s)",
            boolToString(isSendToRilj));

    return isSendToRilj;
}

bool RpRedialHandler::handleUpdateHvolteMode(const sp<RfxMessage>& msg) {
    int     length;
    bool    isSendToRilj = true;
    Parcel  *p = msg->getParcel();
    HvolteMode mode;

    length = p->readInt32();
    mode   = (HvolteMode)p->readInt32();

    setHvolteMode(mode, msg->getSlotId());

    RFX_LOG_D(RP_REDIAL_TAG, "handleUpdateHvolteMode complete(sendToRilj:%s)",
            boolToString(isSendToRilj));

    return isSendToRilj;
}

bool RpRedialHandler::handleSrvcc(const sp<RfxMessage>& msg) {
    int     length;
    int     srvccState;
    bool    isSendToRilj = true;
    Parcel  *p = msg->getParcel();

    if (mMoCall == NULL) {
        return isSendToRilj;
    }

    length      = p->readInt32();
    srvccState  = p->readInt32();
    // fake SRVCC start, regard as call connected.
    // The following CALL_STATE_CHANGED will be sent to RILJ
    if (srvccState == 0) {
        isSendToRilj = processCallConnect(msg);
    }

    return isSendToRilj;
}

bool RpRedialHandler::handleSetHvolteModeRsp(const sp<RfxMessage>& msg) {
    sp<RfxMessage> deferredMsg;
    int reqId;
    int dialId[4] = {RIL_REQUEST_DIAL,
                     RIL_REQUEST_IMS_DIAL,
                     RIL_LOCAL_C2K_REQUEST_REDIAL,
                     RIL_LOCAL_C2K_REQUEST_EMERGENCY_REDIAL};

    if (msg->getError() != RIL_E_SUCCESS) {
        RFX_LOG_D(RP_REDIAL_TAG, "Set hVolte Mode to mode failure(slotId:%d)",
                msg->getSlotId());
        // If set to SRLTE failure, try to dial emergency call anyway
    }

    // Set SRLTE mode complete => Get the deferred Dial request and sent to MD3
    for (unsigned int i = 0; i < (sizeof(dialId)/sizeof(int)); i++) {
        reqId = dialId[i];
        deferredMsg = getDeferredMsg(reqId);
        if (deferredMsg != NULL) {
            RFX_LOG_D(RP_REDIAL_TAG, "Get deferred DIAL request, sent to MD3 directly");
            sp<RfxMessage> c2k_request = RfxMessage::obtainRequest(
                    RADIO_TECH_GROUP_C2K, reqId, deferredMsg, true);

            requestToRild(c2k_request);
            break;
        }
    }

    // Enqueue the deferred message, let the deferred message enter
    // RpCallController again and decide the domain.
    resumeMsg();

    return false;
}

bool RpRedialHandler::handleHangUp(const sp<RfxMessage>& msg) {
    int hangUpCallId;
    Parcel *p;

    p = msg->getParcel();
    hangUpCallId = p->readInt32();

    RFX_LOG_D(RP_REDIAL_TAG, "RIL_REQUEST_HANGUP(call ID:%d)", hangUpCallId);

    if ((mMoCall != NULL) && (hangUpCallId == mMoCall->getCallId())) {
        // User cannot hang up call before call id assigned
        REDIAL_ASSERT(mMoCall->getcallId() != 0);

        setUserHangUping(true);
    }

    return true;
}

bool RpRedialHandler::processCallConnect(const sp<RfxMessage>& msg) {
    bool    isSendToRilj = true;

    RFX_LOG_D(RP_REDIAL_TAG, "Call connect(RedialState:%s)",
            redialStateToString(mRedialState));

    if ((isDialFromIms() == true) && (msg->getSource() == RADIO_TECH_GROUP_C2K)) {
        // Trigger fake SRVCC
        int srvccStart = 0;
        int srvccComplete = 1;
        Parcel *p;

        RFX_LOG_D(RP_REDIAL_TAG, "IMS call dial success on 1xRTT, trigger fake SRVCC");

        // Notify framework SRVCC start
        sp<RfxMessage> urcSrvccStart = RfxMessage::obtainUrc(
                msg->getSlotId(), RIL_UNSOL_SRVCC_STATE_NOTIFY);

        p = urcSrvccStart->getParcel();
        p->writeInt32(1);           // length
        p->writeInt32(srvccStart);  // SRVCC start
        responseToRilj(urcSrvccStart);

        // Notify framework SRVCC complete
        sp<RfxMessage> urcSrvccComplete = RfxMessage::obtainUrc(
                msg->getSlotId(), RIL_UNSOL_SRVCC_STATE_NOTIFY);

        p = urcSrvccComplete->getParcel();
        p->writeInt32(1);               // length
        p->writeInt32(srvccComplete);   // SRVCC complete
        responseToRilj(urcSrvccComplete);
    }

    setRedialState(REDIAL_NONE);
    mShouldDropGetCurrentCalls = false;

    RFX_LOG_D(RP_REDIAL_TAG, "processCallConnect complete(sendToRilj:%s)",
            boolToString(isSendToRilj));

    return isSendToRilj;
}

bool RpRedialHandler::processCallDisconnect(const sp<RfxMessage>& msg, int discCause) {
    bool            isSendToRilj = true;
    sp<RfxMessage>  rilRequest;

    RFX_LOG_D(RP_REDIAL_TAG, "Call disconnect(discCause:%d)", discCause);

    if (mIsUserHangUping == true) {
        RFX_LOG_D(RP_REDIAL_TAG, "User cancel dial, let this call failed directly");
        discCause = 0;
    }

    if ((discCause == CALL_DISC_CAUSE_REDIAL_MD1) && (mMoCall->isEccCall() == false)) {
        RFX_LOG_D(RP_REDIAL_TAG,
                "MD3 should not redial normal call to MD1, let this call failed directly");
        discCause = 0;
    }

    switch (discCause) {
        case CALL_DISC_CAUSE_REDIAL_MD1:
            rilRequest = getRedialRequest(RADIO_TECH_GROUP_GSM, false);

            if (rilRequest != NULL) {
                requestToRild(rilRequest);
                isSendToRilj = false;

                setRedialState(REDIAL_TO_MD1);
            }
            break;

        case CALL_DISC_CAUSE_REDIAL_MD3:
            rilRequest = getRedialRequest(RADIO_TECH_GROUP_C2K, true);

            if (rilRequest != NULL) {
                sendHvolteModeReq(0, 1); // SRLTE sub mode => search MD3 without swithing mode

                // Defer the dial request and wait for set hVoLTE complete
                deferMsg(rilRequest);
                isSendToRilj = false;

                setRedialState(REDIAL_TO_MD3);
            }
            break;

        case CALL_DISC_CAUSE_REDIAL_MD3_WITHOUT_TRY_VOLTE:
            rilRequest = getRedialRequest(RADIO_TECH_GROUP_C2K, false);

            if (rilRequest != NULL) {
                sendHvolteModeReq(0, 1); // SRLTE sub mode => search MD3 without swithing mode

                // Defer the dial request and wait for set hVoLTE complete
                deferMsg(rilRequest);
                isSendToRilj = false;

                setRedialState(REDIAL_TO_MD3);
            }
            break;

        case CALL_DISC_CAUSE_NEED_CHANGE_TO_GLOBAL_MODE:
            if (getStatusManager(msg->getSlotId())->
                    getIntValue(RFX_STATUS_KEY_PREFERRED_NW_TYPE) != getGlobalMode()) {

                setRedialState(REDIAL_CHANGE_GLOBAL_MODE);
                setRatMode(getGlobalMode(), msg->getSlotId());

                isSendToRilj = false;
            }
            // Already global mode => redial to MD1 directly
            else {
                rilRequest = getRedialRequest(RADIO_TECH_GROUP_GSM, false);

                if (rilRequest != NULL) {
                    requestToRild(rilRequest);
                    isSendToRilj = false;

                    setRedialState(REDIAL_TO_MD1);
                }
            }
            break;

        // Exactly CALL disconnect => Abort call redial
        default:
            if (mMoCall != NULL) {
                RFX_LOG_D(RP_REDIAL_TAG, "Call disconnect during dialing");

                if (mMoCall->isEccCall() == true) {
                    RFX_LOG_D(RP_REDIAL_TAG, "ECC call disconnect during dialing");

                    if (mPreRatMode != 0) {
                        RFX_LOG_D(RP_REDIAL_TAG, "Resume to %d from global mode", mPreRatMode);
                        // Resume RAT mode
                        setRatMode(mPreRatMode, msg->getSlotId());
                    }
                    else {
                        // ECC call disconnect before connect => emergency session end
                        setEmergencyMode(false, msg->getSlotId());
                    }
                }

                /* If the call is dialed from ImsPhone and dialed to CDMA, we
                 * need to call responseToRilj() here to ensure this URC will
                 * be sent back to ImsPhone.
                 */
                if ((isDialFromIms() == true) && (msg->getSource() == RADIO_TECH_GROUP_C2K)) {
                    responseToRilj(msg);
                    isSendToRilj = false;
                }
            }

            if (msg->getSource() == RADIO_TECH_GROUP_C2K) {
                // Notify MD1 to switch to SRLTE mode because MD3 call end
                sendHvolteModeReq(0, 2);
            }

            setUserHangUping(false);
            mShouldDropGetCurrentCalls = false;
            setRedialState(REDIAL_NONE);
            break;
    }

    RFX_LOG_D(RP_REDIAL_TAG, "processCallDisconnect complete(sendToRilj:%s)",
            boolToString(isSendToRilj));

    return isSendToRilj;
}

bool RpRedialHandler::responseToRilj(const sp<RfxMessage>& msg) {
    return RfxController::responseToRilj(msg);
}

bool RpRedialHandler::checkDropUrc(const sp<RfxMessage>& msg) {
    bool result = false;
    int  deviceMode = getDeviceMode(msg->getSlotId());

    switch (mRedialState) {
    case REDIAL_TO_MD1:
        // URC from MD3 => Drop
        if (msg->getSource() == RADIO_TECH_GROUP_C2K) {
            result = true;
        }
        else {
            // URC from MD1, Dial from CDMALTE phone => Drop
            if ((deviceMode  == NWS_MODE_CDMALTE) &&
                (isDialFromIms() == false)) {
               result = true;
            }
        }
        break;

    case REDIAL_TO_MD3:
        // URC from MD1 => Drop
        if (msg->getSource() == RADIO_TECH_GROUP_GSM) {
            result = true;
        }
        else {
            // URC from MD3, Dial from GSM phone => Drop
            if ((deviceMode == NWS_MODE_CSFB) || (deviceMode == NWS_MODE_LTEONLY)){
                result = true;
            }
        }
        break;

    case REDIAL_CHANGE_GLOBAL_MODE:
        result = true;
        break;

    case REDIAL_NONE:
    default:
        result = false;
        break;
    }

    RFX_LOG_D(RP_REDIAL_TAG, "Drop URC:%s, RedialState:%s, URC from:%s, DeviceMode:%s",
            boolToString(result), redialStateToString(mRedialState),
            radioTechToString(msg->getSource()), deviceModeToString(deviceMode));

    return result;
}

void RpRedialHandler::setRedialState(RedialState state) {
    if (state == REDIAL_NONE) {
        freeMoCallObject();
    }

    if (mRedialState == state) {
        RFX_LOG_D(RP_REDIAL_TAG, "Current Redial State:%s", redialStateToString(mRedialState));
        return;
    }

    RFX_LOG_D(RP_REDIAL_TAG, "Change RedialState from %s to %s",
                        redialStateToString(mRedialState), redialStateToString(state));

    mRedialState = state;
    return;
}

void RpRedialHandler::deferMsg(const sp<RfxMessage>& msg) {
    RFX_LOG_D(RP_REDIAL_TAG, "Defer the current message %s to suspended queue",
            requestToString(msg->getId()));

    mSuspendedMsgQueue.add(RfxSuspendedMsgEntry(this, msg));

    return;
}

void RpRedialHandler::resumeMsg() {
    const RfxSuspendedMsgEntry *msgEntry;

    while (mSuspendedMsgQueue.isEmpty() == false) {
        msgEntry = &(mSuspendedMsgQueue.itemAt(0));
        RfxMainThread::enqueueMessage(msgEntry->message);

        RFX_LOG_D(RP_REDIAL_TAG, "Resume the message %s from suspended queue",
                requestToString(msgEntry->message->getId()));

        mSuspendedMsgQueue.removeAt(0);
    }
    return;
}

const sp<RfxMessage> RpRedialHandler::getDeferredMsg(int msgId) {
    const   RfxSuspendedMsgEntry *msgEntry;
    int     size = mSuspendedMsgQueue.size();

    for (int i = 0; i < size; i++) {
        msgEntry = &(mSuspendedMsgQueue.itemAt(i));

        if (msgEntry->message->getId() == msgId) {
            RFX_LOG_D(RP_REDIAL_TAG, "Get matched deferred message:%s", requestToString(msgId));

            const sp<RfxMessage> msg = msgEntry->message;
            mSuspendedMsgQueue.removeAt(i);
            return msg;
        }
    }
    return NULL;
}

void RpRedialHandler::setEmergencyMode(bool emergencyMode, int slotId) {
    sp<RfxMessage> rilRequest = NULL;

    if (mInEmergencyMode == emergencyMode) {
        return;
    }

    // No need to update emergencyMode or send AT+EMCS to modem in non-C2K project.
    if ((checkRedialFeatureEnabled(REDIAL_SBP_ID_VZW, REDIAL_TC_ID_COMMON) == false) &&
            (checkRedialFeatureEnabled(REDIAL_SBP_ID_SPRINT, REDIAL_TC_ID_COMMON) == false)) {
        return;
    }

    RFX_LOG_D(RP_REDIAL_TAG, "Change mInEmergencyMode from %s to %s",
            boolToString(mInEmergencyMode), boolToString(emergencyMode));

    if (emergencyMode == true) {
        RFX_LOG_D(RP_REDIAL_TAG, "Notify MD1 Emergency session start");
        getStatusManager()->setBoolValue(RFX_STATUS_KEY_EMERGENCY_MODE, true);

        if (checkRedialFeatureEnabled(REDIAL_SBP_ID_VZW, REDIAL_TC_ID_01) == true) {
            // AP will notify MD emergency session start through
            // RIL_REQUEST_SET_ECC_MODE in TC1 project but we still
            // needs to notify eMBMS E911 start.
            notifyEmbmsE911Status(EMERGENCY_MODE_BEGIN, slotId);
        }
    }
    if (emergencyMode == false) {
        RFX_LOG_D(RP_REDIAL_TAG, "Notify MD1 Emergency session stop");
        getStatusManager()->setBoolValue(RFX_STATUS_KEY_EMERGENCY_MODE, false);
        setUserHangUping(false);

        if (checkRedialFeatureEnabled(REDIAL_SBP_ID_VZW, REDIAL_TC_ID_01) == true) {
            // Only TC01 VzW project needs to notify eMBMS E911 status
            notifyEmbmsE911Status(EMERGENCY_MODE_END, slotId);
        }

        rilRequest = RfxMessage::obtainRequest(
                slotId,
                RADIO_TECH_GROUP_GSM,
                RIL_LOCAL_REQUEST_EMERGENCY_SESSION_END);

        requestToRild(rilRequest);
    }

    mInEmergencyMode = emergencyMode;

    return;
}

void RpRedialHandler::setEcbm(EmergencyCallBackMode ecbm) {
    if (mEcbm == ecbm) {
        return;
    }

    RFX_LOG_D(RP_REDIAL_TAG, "Change mEcbm from %d to %d", mEcbm, ecbm);

    mEcbm = ecbm;

    getStatusManager()->setIntValue(RFX_STATUS_KEY_EMERGENCY_CALLBACK_MODE, mEcbm);

    return;
}

void RpRedialHandler::setUserHangUping(bool userHangUping) {
    if (mIsUserHangUping == userHangUping) {
        return;
    }

    RFX_LOG_D(RP_REDIAL_TAG, "Change mIsUserHangUping from %s to %s",
            boolToString(mIsUserHangUping), boolToString(userHangUping));

    mIsUserHangUping = userHangUping;
    mShouldDropGetCurrentCalls = false;

    return;
}

void RpRedialHandler::setHvolteMode(HvolteMode mode, int slotId) {
    RFX_UNUSED(slotId);
    if (mHvolteMode == mode) {
        RFX_LOG_D(RP_REDIAL_TAG, "Current hVoLTE mode:%s", hvolteModeToString(mHvolteMode));
        return;
    }

    RFX_LOG_D(RP_REDIAL_TAG, "Change mHvoltemode from %s to %s",
            hvolteModeToString(mHvolteMode), hvolteModeToString(mode));

    mHvolteMode = mode;

    return;
}

void RpRedialHandler::setRatMode(int ratMode, int slotId) {
    sp<RfxAction> action;
    RpNwRatController* nwRatController =
            (RpNwRatController *)findController(slotId, RFX_OBJ_CLASS_INFO(RpNwRatController));

    // Change to Global mode
    if (ratMode == getGlobalMode()) {
        setPreRatMode(
                getStatusManager(slotId)->getIntValue(RFX_STATUS_KEY_PREFERRED_NW_TYPE));

        RFX_LOG_D(RP_REDIAL_TAG, "Change to global mode(Preferred network type:%d -> %d)",
                mPreRatMode, getGlobalMode());

        action = new RfxAction1<int>(this, &RpRedialHandler::onGlobalModeSwitchDone, slotId);
        nwRatController->setPreferredNetworkType(getGlobalMode(), action);
    }
    // Resume to original RAT mode
    else {
        int csCallCount = getStatusManager(slotId)->
                            getIntValue(RFX_STATUS_KEY_VOICE_CALL_COUNT, 0);
        if (csCallCount > 0) {
            RFX_LOG_D(RP_REDIAL_TAG, "Has MT call, pending resume original RAT mode");
            // Resume from global mode needs to wait MT call, set mPreRatMode to
            // RESUME_RAT_MODE_PENDING before MT call disconnect
            setPreRatMode(REDIAL_NET_TYPE_RESUME_RAT_MODE_PENDING);
        } else {
            RFX_LOG_D(RP_REDIAL_TAG, "Resume from global mode to original RAT mode");

            // Resume from global mode, set mPreRatMode to RESUME_RAT_MODE before
            // resume RAT mode complete.
            setPreRatMode(REDIAL_NET_TYPE_RESUME_RAT_MODE);

            action = new RfxAction1<int>(this, &RpRedialHandler::onGlobalModeResumeDone, slotId);
            nwRatController->setPreferredNetworkType(REDIAL_NET_TYPE_RESUME_RAT_MODE, action);
        }
    }

    return;
}

void RpRedialHandler::setPreRatMode(int ratMode) {
    RFX_LOG_D(RP_REDIAL_TAG, "set mPreRatMode:%d -> %d", mPreRatMode, ratMode);

    mPreRatMode = ratMode;

    return;
}

void RpRedialHandler::sendHvolteModeReq(int mode, int subMode) {
    sp<RfxMessage> rilRequest = RfxMessage::obtainRequest(
            getSlotId(),
            RADIO_TECH_GROUP_GSM,
            RIL_LOCAL_REQUEST_SET_HVOLTE_MODE);

    Parcel  *p = rilRequest->getParcel();
    p->writeInt32(2);       // length
    p->writeInt32(mode);    // hVoLTE mode
    p->writeInt32(subMode); // hVoLTE sub-mode

    requestToRild(rilRequest);
    return;
}

void RpRedialHandler::sendOemHookString(char* atCmdLine, RILD_RadioTechnology_Group sendTarget) {
    sp<RfxMessage> rilRequest = RfxMessage::obtainRequest(
            getSlotId(), sendTarget, RIL_REQUEST_OEM_HOOK_STRINGS);
    Parcel  *p = rilRequest->getParcel();

    p->writeInt32(2);
    RpRedialHandler::writeStringToParcel(*p, atCmdLine);
    RpRedialHandler::writeStringToParcel(*p, "");

    requestToRild(rilRequest);

    return;
}

void RpRedialHandler::notifyEmbmsE911Status(EmergencyMode mode, int slotId) {
    int status = 0;
    sp<RfxMessage> rilRequest = NULL;

    rilRequest = RfxMessage::obtainRequest(
            slotId,
            RADIO_TECH_GROUP_GSM,
            RIL_LOCAL_REQUEST_EMBMS_SET_E911);

    // AT+EMEVT=<scenario>,<status>
    // <scenario> : integer
    //  1: E911
    //  2: hVoLTE
    //  3: flight mode
    // <status> : integer
    //  0: off
    //  1: on

    switch (mode) {
        case EMERGENCY_MODE_BEGIN:
            status = 1;
            break;

        case EMERGENCY_MODE_END:
            status = 0;
            break;

        default:
            break;
    }

    Parcel *p = rilRequest->getParcel();
    p->writeInt32(2);       // length
    p->writeInt32(1);       // <scenario>
    p->writeInt32(status);  // <status>

    requestToRild(rilRequest);

    return;
}

sp<RfxMessage> RpRedialHandler::getRedialRequest(
        RILD_RadioTechnology_Group dialTarget, bool isVolteTried) {
    int requestId;

    if (mIsUserHangUping == true) {
        RFX_LOG_D(RP_REDIAL_TAG, "User is hanguping, not to perform redial");
        return NULL;
    }

    if (mMoCall == NULL) {
        RFX_LOG_D(RP_REDIAL_TAG, "mMoCall is NULL pointer, not to perform redial");
        return NULL;
    }

    RpAudioControlHandler *audioHandler;

    audioHandler = (RpAudioControlHandler *)findController(
            mMoCall->getSlotId(), RFX_OBJ_CLASS_INFO(RpAudioControlHandler));

    if (mMoCall->isEccCall() == true) {
        if (dialTarget == RADIO_TECH_GROUP_GSM) {
            requestId = RIL_LOCAL_REQUEST_EMERGENCY_REDIAL;
            audioHandler->updateAudioPathAsync(RADIO_TECH_GROUP_GSM);

            //if (isDialFromIms() == true) {
                // Notify vendor RIL IMS call invoke to set dispatch flag to IMS
            //    sendOemHookString("DIALSOURCE_IMS", RADIO_TECH_GROUP_GSM);
            //}
        }
        else {
            requestId = RIL_LOCAL_C2K_REQUEST_EMERGENCY_REDIAL;
            audioHandler->updateAudioPathAsync(RADIO_TECH_GROUP_C2K);
        }
    }
    else {
        if (dialTarget == RADIO_TECH_GROUP_GSM) {
            RFX_LOG_E(RP_REDIAL_TAG, "We don't support redial to MD1 now");
            return NULL;
        }
        else {
            requestId = RIL_LOCAL_C2K_REQUEST_REDIAL;
            audioHandler->updateAudioPathAsync(RADIO_TECH_GROUP_C2K);
        }
    }

    sp<RfxMessage> rilRequest =
        RfxMessage::obtainRequest(mMoCall->getSlotId(), dialTarget, requestId);

    Parcel *newReqParcel = rilRequest->getParcel();
    Parcel *oldReqParcel = mMoCall->getDialParcel();

    // Copy dialed reqeust parcel to new request parcel
    newReqParcel->write(oldReqParcel->data(), oldReqParcel->dataSize());

    // redial mode
    newReqParcel->writeInt32(isVolteTried);

    // call ID
    newReqParcel->writeInt32(mMoCall->getCallId());

    if ((dialTarget == RADIO_TECH_GROUP_GSM) && isDialFromIms() == true) {
        // phone ID
        newReqParcel->writeInt32(mMoCall->getSlotId());

        char optr[PROPERTY_VALUE_MAX] = {0};
        property_get("persist.vendor.operator.optr", optr, "");
        if (strcmp(optr, "OP12") == 0 || strcmp(optr, "OP20") == 0) {
            // IMS socket
            newReqParcel->writeInt32(IMS_SOCKET);
        }
    }

    RFX_LOG_D(RP_REDIAL_TAG,
            "get redial request %s from old parcel(call ID:%d, dial to:%s, isVolteTried:%d)",
            requestToString(requestId), mMoCall->getCallId(),
            radioTechToString(dialTarget), isVolteTried);

    return rilRequest;
}

int RpRedialHandler::getDeviceMode(int slotId) {
    int mode = getStatusManager(slotId)->getIntValue(RFX_STATUS_KEY_NWS_MODE, NWS_MODE_CSFB);
    //RFX_LOG_D(RP_REDIAL_TAG, "Get device mode=%d", mode);

    return mode;
}

int RpRedialHandler::getGlobalMode() {
    if ((RatConfig_isLteFddSupported() == 1 || RatConfig_isLteTddSupported() == 1)
            && RatConfig_isC2kSupported() == 1 && RatConfig_isGsmSupported() == 0
            && RatConfig_isWcdmaSupported() == 0 && RatConfig_isTdscdmaSupported() == 0) {
        return PREF_NET_TYPE_LTE_CDMA_EVDO;             /* LTE, CDMA and EvDo */
    } else {
        return PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA;   /* LTE, CDMA, EvDo, GSM/WCDMA */
    }
}

EmergencyCallBackMode RpRedialHandler::getEcbm() {
    return mEcbm;
}

RedialState RpRedialHandler::getRedialState() {
    return mRedialState;
}

bool RpRedialHandler::isDialFromIms() {
    if (mMoCall == NULL) {
        RFX_LOG_D(RP_REDIAL_TAG, "No dialing call");
        return false;
    }

    if (mMoCall->getMsgId() == RIL_REQUEST_IMS_DIAL ||
        mMoCall->getMsgId() == RIL_LOCAL_REQUEST_IMS_EMERGENCY_DIAL) {
        return true;
    }
    return false;
}

void RpRedialHandler::freeMoCallObject() {
    if (mMoCall != NULL) {
        RFX_LOG_D(RP_REDIAL_TAG, "free mMoCall object");

        delete(mMoCall);
        mMoCall = NULL;
    }

    mMoCall = NULL;
    return;
}

char const * RpRedialHandler::boolToString(bool value) {
    if (value == true) {
        return "true";
    }
    return "false";
}

char const * RpRedialHandler::redialStateToString(RedialState state) {
    switch (state) {
    case REDIAL_NONE:
        return "REDIAL_NONE";
    case REDIAL_TO_MD1:
        return "REDIAL_TO_MD1";
    case REDIAL_TO_MD3:
        return "REDIAL_TO_MD3";
    case REDIAL_CHANGE_GLOBAL_MODE:
        return "REDIAL_CHANGE_GLOBAL_MODE";
    default:
        break;
    }
    return NULL;
}

char const * RpRedialHandler::deviceModeToString(int deviceMode) {
    switch (deviceMode) {
    case NWS_MODE_CDMALTE:
        return "CDMALTE";
    case NWS_MODE_CSFB:
        return "CSFB";
    default:
        break;
    }
    return NULL;
}

char const * RpRedialHandler::radioTechToString(int radio) {
    switch (radio) {
    case RADIO_TECH_GROUP_GSM:
        return "GSM";
    case RADIO_TECH_GROUP_C2K:
        return "C2K";
    default:
        break;
    }
    return NULL;
}

char const * RpRedialHandler::hvolteModeToString(HvolteMode mode) {
    switch (mode) {
        case HVOLTE_MODE_SRLTE:
            return "SRLTE";
        case HVOLTE_MODE_LTE_ONLY:
            return "LTE_ONLY";
        case HVOLTE_MODE_NONE:
            return "NONE";
        default:
            break;
    }
    return NULL;
}

char* RpRedialHandler::strdupReadString(Parcel &p) {
    size_t stringlen;
    const char16_t *s16;

    s16 = p.readString16Inplace(&stringlen);

    // RFX_LOG_D(RP_REDIAL_TAG, "%s, length:%d", s16, stringlen);
    return strndup16to8(s16, stringlen);
}

void RpRedialHandler::writeStringToParcel(Parcel &p, const char *s) {
    char16_t *s16;
    size_t s16_len = 0;
    s16 = strdup8to16(s, &s16_len);
    p.writeString16(s16, s16_len);
    free(s16);
}

int RpRedialHandler::getCallCount(const sp<RfxMessage>& message) {
    message->getParcel()->setDataPosition(CALL_COUNT_POS * sizeof(int));
    int callCount = message->getParcel()->readInt32();
    RFX_LOG_D(RP_REDIAL_TAG, "getCallCount, callCount: %d", callCount);
    return callCount;
}

bool RpRedialHandler::checkRedialFeatureEnabled(int sbpId, int tcId) {
    bool result = false;
    char optr[PROPERTY_VALUE_MAX] = {0};

    // The caller will set tcId to non-zero value when it needs to enter a code
    // block which is TCx proprietary. If the TC feature option is not defined,
    // this function will return false to disable the TCx proprietary behavior.
    switch (tcId) {
        case 1:
#ifndef MTK_TC1_FEATURE
            return false;
#endif
            break;

        default:
            break;
    }

    property_get("persist.vendor.operator.optr", optr, "");

    // The caller will set sbpId to a specific value when it needs to enter a
    // code block which is operator proprietary. If the SBP ID is not matched
    // function parameter "sbpId", this function will return false to disable
    // the operator proprietary behavior.
    switch (sbpId) {
        case 12:
            if (strcmp(optr, "OP12") == 0) {
                result = true;
            }
            break;

        case 20:
            if (strcmp(optr, "OP20") == 0) {
                result = true;
            }
            break;

        default:
            break;
    }

    return result;
}

MoCallContext::MoCallContext(char *callNumber, bool isEcc, int slotId, int msgId) {
    RFX_LOG_D(RP_REDIAL_TAG, "MoCallContext Constructor(CallNumber:%s, isEcc:%s, slotId:%d)",
            (isUserLoad() ? "[hidden]" : callNumber), RpRedialHandler::boolToString(isEcc), slotId);

    RpRedialHandler::writeStringToParcel(mDialParcel, callNumber);
    mDialParcel.writeInt32(0);
    mDialParcel.writeInt32(0);

    mIsEmergencyCall = isEcc;
    mCallId = 0;
    mSlotId = slotId;
    mMsgId = msgId;

    return;
}

MoCallContext::MoCallContext(Parcel& p, bool isEcc, int slotId, int msgId) {
    RFX_LOG_D(RP_REDIAL_TAG, "MoCallContext Constructor(isEcc:%s, slotId:%d)",
            RpRedialHandler::boolToString(isEcc), slotId);
/*
    for (int i = 0; i < p.dataSize(); i++) {
        RFX_LOG_D(RP_REDIAL_TAG, "%d, %d", i, *(p.data() + i));
    }
*/
    char *data = RpRedialHandler::strdupReadString(p);
    RpRedialHandler::writeStringToParcel(mDialParcel, data);
    mDialParcel.writeInt32(p.readInt32());  //clir
    if (data != NULL) {
        free(data);
    }

    int uusPresent;

    p.readInt32(&uusPresent);
    if (uusPresent == 0) {
        mDialParcel.writeInt32(0);
    } else {
        mDialParcel.writeInt32(1);
        mDialParcel.writeInt32(p.readInt32());  //uusType
        mDialParcel.writeInt32(p.readInt32());  //uusDcs
        int uusLength = p.readInt32();
        if (uusLength == 0) {
            mDialParcel.writeInt32(-1);
        } else {
            mDialParcel.writeInt32(uusLength);
            mDialParcel.writeByteArray(uusLength, (uint8_t *) p.readInplace(uusLength));
        }
    }

    mIsEmergencyCall = isEcc;
    mCallId = 0;
    mSlotId = slotId;
    mMsgId = msgId;

    return;
}

Parcel* MoCallContext::getDialParcel() {
    return &mDialParcel;
}

int MoCallContext::getCallId() {
    return mCallId;
}

int MoCallContext::getSlotId() {
    return mSlotId;
}

int MoCallContext::getMsgId() {
    return mMsgId;
}

void MoCallContext::setCallId(int callId) {
    RFX_LOG_D(RP_REDIAL_TAG, "Set call id from %d to %d", mCallId, callId);

    mCallId = callId;
    return;
}

bool MoCallContext::isEccCall() {
    return mIsEmergencyCall;
}

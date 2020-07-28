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
#include <compiler/compiler_utils.h>
#include <mtk_log.h>
#include <ratconfig.h>
#include <string.h>
#include <telephony/mtk_ril.h>
#include "RfxBasics.h"
#include "RfxDefs.h"
#include "RfxLog.h"
#include "RfxMainThread.h"
#include "RfxRilUtils.h"
#include "RfxStatusDefs.h"
#include "RtcRedialController.h"
#include "capabilityswitch/RtcCapabilitySwitchUtil.h"
#include "nw/RtcRatSwitchController.h"
#include "rfx_properties.h"

/*****************************************************************************
 * Utility function
 *****************************************************************************/
const char *boolToString(bool value) {
    return value ? "true" : "false";
}


/*****************************************************************************
 * Class RtcRedialController
 *****************************************************************************/
RFX_IMPLEMENT_CLASS("RtcRedialController", RtcRedialController, RfxController);

// register request to RfxData
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxRedialData, RfxVoidData, RFX_MSG_REQUEST_EMERGENCY_REDIAL);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxVoidData, RFX_MSG_REQUEST_EMERGENCY_SESSION_END);

RtcRedialController::RtcRedialController() {
    mIsUserHangUping = false;
    mInEmergencyMode = false;
    mMoCall = NULL;
    mEcbm = EMERGENCY_CALLBACK_MODE_NONE;
    mEmcsCount = 0;
    mIsEccModeSent = false;

    mRedialState = REDIAL_NONE;
    mRatState = REDIAL_RAT_NONE;
    mRatOp = REDIAL_RAT_OP_NONE;
    mPendingExitES = false;
}

RtcRedialController::~RtcRedialController() {
    logD(LOG_REDIAL_TAG, "~RtcRedialController");
    resetController();
    destroyMoCallContext();
}

void RtcRedialController::onInit() {
    RfxController::onInit();
    logD(LOG_REDIAL_TAG, "onInit !!");

    const int request_id_list[] = {
        RFX_MSG_REQUEST_OEM_HOOK_ATCI_INTERNAL,
        RFX_MSG_REQUEST_EMERGENCY_REDIAL,
        RFX_MSG_REQUEST_EMERGENCY_SESSION_END,
    };

    registerToHandleRequest(request_id_list, sizeof(request_id_list)/sizeof(int));
}

void RtcRedialController::onDeinit() {
    logD(LOG_REDIAL_TAG, "onDeinit");
    RfxController::onDeinit();
}

bool RtcRedialController::onHandleRequest(const sp<RfxMessage>& message) {
    bool ret = false;
    int msgId = message->getId();
    if (DBG) {
        logD(LOG_REDIAL_TAG, "onHandleRequest: %s", RFX_ID_TO_STR(msgId));
    }

    switch (msgId) {
        case RFX_MSG_REQUEST_EMERGENCY_SESSION_END:
            ret = notifyRilRequest(message);
            break;
        default:
            break;
    }

    return ret;
}

bool RtcRedialController::onHandleResponse(const sp<RfxMessage>& message) {
    return notifyRilResponse(message);
}


bool RtcRedialController::onHandleUrc(const sp<RfxMessage>& message) {
    return notifyRilUrc(message);
}

bool RtcRedialController::onHandleAtciRequest(const sp<RfxMessage>& message) {
    bool ret = false;
    int msgId = message->getId();

    logD(LOG_REDIAL_TAG, "message: %d, %s", msgId, RFX_ID_TO_STR(msgId));

    if (!checkFeatureEnabled()) {
        return ret;
    }

    if (msgId == RFX_MSG_REQUEST_OEM_HOOK_ATCI_INTERNAL) {
        char *pString = (char *)message->getData();
        const char  *data = &pString[0];
        int dataLength = strlen(data);
        int targetSlotId = 0;
        char simNo[RFX_PROPERTY_VALUE_MAX] = {0};

        rfx_property_get("persist.vendor.service.atci.sim", simNo, "0");
        logD(LOG_REDIAL_TAG, "[onHandleAtciRequest] simNo: %c", simNo[0]);

        if (simNo[0] == '0') {
            targetSlotId = 0;
        } else if (simNo[0] == '1') {
            targetSlotId = 1;
        } else {
            logE(LOG_REDIAL_TAG, "Not support slot: %d.", simNo[0]);
            return true;
        }

        if (targetSlotId != getSlotId()) {
            logE(LOG_REDIAL_TAG, "Controller cannot handle this slot request: %d,%d",
                    targetSlotId, getSlotId());
            return false;
        }

        logD(LOG_REDIAL_TAG, "Inject AT command %s (length:%d)", data, dataLength);
        if (strncmp(data, "ATD", 3) == 0) {
            ret = handleAtciDialRequest(data, dataLength);
        }
    }

    return false;
}

bool RtcRedialController::onHandleAtciResponse(const sp<RfxMessage>& message) {
    RFX_UNUSED(message);
    return false;
}

bool RtcRedialController::handleAtciDialRequest(const char* data, int dataLength) {
    char *callNumber = NULL;
    int offset = 4;

    // Only handle ATDE.
    if (strncmp(data, "ATDE", 4) != 0) {
        return false;
    }

    // check mo data
    setEmergencyModeExt(true);
    checkMoCallAndFree();

    callNumber = new char[dataLength] {0};
    strncpy(callNumber, data + offset, dataLength - offset - 1);
    logD(LOG_REDIAL_TAG, "ATCI number %s", (RfxRilUtils::isUserLoad() ? "[hidden]" : callNumber));

    mMoCall = new MoCallContext(callNumber);
    delete [] callNumber;

    return false;
}

bool RtcRedialController::onCheckIfRejectMessage(
        const sp<RfxMessage>& message, bool isModemPowerOff, int radioState) {
    int msgId = message->getId();
    if ((radioState == (int)RADIO_STATE_OFF) &&
            (msgId == RFX_MSG_REQUEST_EMERGENCY_REDIAL ||
             msgId == RFX_MSG_REQUEST_EMERGENCY_SESSION_END)) {
        return false;
    }

    return RfxController::onCheckIfRejectMessage(message, isModemPowerOff, radioState);
}

bool RtcRedialController::notifyRilRequest(const sp<RfxMessage>& message) {
    bool ret = false;
    int msgId = message->getId();
    int hangupCallId = -1;

    if (DBG) {
        logD(LOG_REDIAL_TAG, "message: %d, %s", msgId, RFX_ID_TO_STR(msgId));
    }

    if (!checkFeatureEnabled()) {
        return ret;
    }

    switch (msgId) {
        case RFX_MSG_REQUEST_LOCAL_EMERGENCY_DIAL:
        case RFX_MSG_REQUEST_LOCAL_IMS_EMERGENCY_DIAL:
            handleEmergencyDial(message);
            break;

        case RFX_MSG_REQUEST_HANGUP:
            hangupCallId = ((int *)message->getData()->getData())[0];
            if (mMoCall != NULL &&
                    mMoCall->getCallId() == hangupCallId) {
                setUserHangUping(true);
            }
            break;

        case RFX_MSG_REQUEST_HANGUP_ALL:
            if (mMoCall != NULL) {
                setUserHangUping(true);
            }
            break;

        // Just to record local variables mInEmergencyMode, EmcsCount for REQUEST_SET_ECC_MODE
        // No need to trigger AT+EMCS here
        case RFX_MSG_REQUEST_SET_ECC_MODE: {
            RfxStringsData* data = (RfxStringsData*)message->getData();
            char** params = (char**)data->getData();
            int emergencyMode = atoi(params[1]);
            bool needChangeEmcsMode = false;

            if (emergencyMode == true) {
                mIsEccModeSent = true;
                increaseEmcsCount();
                if (mInEmergencyMode == false) {
                    needChangeEmcsMode = true;
                }
            } else if (emergencyMode == false) {
                // skip if mode does not change or emcs count is not zero
                bool resultZero = decreaseEmcsCountAndIsZero();
                if (resultZero) {
                    mIsEccModeSent = false;
                    if (mInEmergencyMode == true) {
                        needChangeEmcsMode = true;
                    }
                }
            }

            if (!needChangeEmcsMode) {
                logD(LOG_REDIAL_TAG,
                        "setEmergencyMode update emcs count (mEmcsCount:%d, mInEmergencyMode:%s",
                        mEmcsCount, boolToString(mInEmergencyMode));

                sp<RfxMessage>  rilResponse = RfxMessage::obtainResponse(RIL_E_SUCCESS, message);
                responseToRilj(rilResponse);
                ret = true;
                return ret;
            }

            logD(LOG_REDIAL_TAG, "setEmergencyMode:(%s->%s)",
                    boolToString(mInEmergencyMode), boolToString(emergencyMode));
            getStatusManager()->setBoolValue(RFX_STATUS_KEY_EMERGENCY_MODE, emergencyMode);
            mInEmergencyMode = emergencyMode;
            break;
        }
        default:
            break;
    }

    return ret;
}

void RtcRedialController::handleEmergencyDial(const sp<RfxMessage>& message) {
    RtcModeSwitchController* modeSwitchController =
            (RtcModeSwitchController *)findController(RFX_OBJ_CLASS_INFO(RtcModeSwitchController));

    // Condition to set ECM: let RILD to check
    // - 6M(with C2K) project: only to C2K-enabled phone
    // - 5M(without C2K) project: specific OP such as Vzw
    // Timing to set ECM:
    // - Not Under flight mode:
    //    - 91-legacy: set in TeleService, use EFUN channel
    //    - 93: set in RILD , use ATD channel
    if (isEmergencyModeSupported(message->getSlotId())) {
        if (!mIsEccModeSent) {
            increaseEmcsCount();
        }
        logD(LOG_REDIAL_TAG, "handleEmergencyDial mEmcsCount:%d, mInEmergencyMode:%s",
                    mEmcsCount, boolToString(mInEmergencyMode));
        if (!mInEmergencyMode) {
            setEmergencyMode(true);
            int msg_data[2];
            msg_data[0] = 0;  // airplane
            msg_data[1] = 0;  // imsReg
            sp<RfxMessage> rilRequest = RfxMessage::obtainRequest(
                                                getSlotId(),
                                                RFX_MSG_REQUEST_LOCAL_CURRENT_STATUS,
                                                RfxIntsData(msg_data, 2));
            requestToMcl(rilRequest);
        }
    }
    mIsEccModeSent = false;
    // create MoCallContext
    resetController();
    mMoCall = new MoCallContext(message);

}

bool RtcRedialController::isEmergencyModeSupported(int slotId) {
    if (RfxRilUtils::isEmciSupport()) {
        return true;
    }

    RtcModeSwitchController* modeSwitchController =
            (RtcModeSwitchController *)findController(RFX_OBJ_CLASS_INFO(RtcModeSwitchController));

    if (modeSwitchController->getCCapabilitySlotId() == slotId) {
        return true;
    }

    logD(LOG_REDIAL_TAG, "isEmergencyModeSupported = false ");
    return false;
}

bool RtcRedialController::notifyRilResponse(const sp<RfxMessage>& message) {
    bool ret = false;
    int msgId = message->getId();

    if (DBG) {
        logD(LOG_REDIAL_TAG, "message: %d, %s", msgId, RFX_ID_TO_STR(msgId));
    }

    if (!checkFeatureEnabled()) {
        return ret;
    }

    switch (msgId) {
        case RFX_MSG_REQUEST_EMERGENCY_REDIAL:
            if (message->getError() != RIL_E_SUCCESS) {
                logE(LOG_REDIAL_TAG, "emergency redial failed!");
                ForceReleaseEmergency();
            }
            // fall through
            TELEPHONYWARE_FALLTHROUGH;

        case RFX_MSG_REQUEST_EMERGENCY_SESSION_END:
            // local request
            ret = true;
            break;

        case RFX_MSG_REQUEST_LOCAL_EMERGENCY_DIAL:
        case RFX_MSG_REQUEST_LOCAL_IMS_EMERGENCY_DIAL:
            if (message->getError() != RIL_E_SUCCESS) {
                logE(LOG_REDIAL_TAG, "emergency dial failed!");
                ForceReleaseEmergency();
            }
            break;

        case RFX_MSG_REQUEST_GET_CURRENT_CALLS:
            if (mRedialState == REDIAL_DIALING || mRedialState == REDIAL_DIALED) {
                // modify response message
                logD(LOG_REDIAL_TAG, "modify response message");
                sp<RfxMessage> msg = RfxMessage::obtainResponse(message->getSlotId(),
                                                            message->getId(),
                                                            RIL_E_GENERIC_FAILURE,
                                                            RfxVoidData(),
                                                            message);
                ret = true;
                responseToRilj(msg);
            }
            break;

        default:
            break;
    }

    return ret;
}

bool RtcRedialController::notifyRilUrc(const sp<RfxMessage>& message) {
    bool ret = false;  // do not handle it
    int msgId = message->getId();

    if (DBG) {
        logD(LOG_REDIAL_TAG, "redialState: %d, message: %d, %s", mRedialState, msgId,
                RFX_ID_TO_STR(msgId));
    }

    if (!checkFeatureEnabled()) {
        return ret;
    }

    switch (msgId) {
        case RFX_MSG_UNSOL_RESPONSE_CALL_STATE_CHANGED:
            // block this during redial
            if (mRedialState == REDIAL_DIALING || mRedialState == REDIAL_DIALED) {
                ret = true;
            }
            break;

        case RFX_MSG_UNSOL_CALL_INFO_INDICATION:
            ret = handleCallInfoUpdate(message);
            break;

        case RFX_MSG_UNSOL_ENTER_EMERGENCY_CALLBACK_MODE:
            handleEmergencyCallbackMode(EMERGENCY_CALLBACK_MODE_ENTER);
            break;

        case RFX_MSG_UNSOL_EXIT_EMERGENCY_CALLBACK_MODE:
            handleEmergencyCallbackMode(EMERGENCY_CALLBACK_MODE_EXIT);
            break;

        case RFX_MSG_UNSOL_NO_EMERGENCY_CALLBACK_MODE:
            handleEmergencyCallbackMode(EMERGENCY_CALLBACK_MODE_EXIT);
            break;

        default:
            break;
    }

    return ret;
}

bool RtcRedialController::checkFeatureEnabled() {
    // alway return true, feature on/off is decided by MD.
    return true;
}

bool RtcRedialController::handleCallInfoUpdate(const sp<RfxMessage>& message) {
    /* +ECPI: <call_id>, <msg_type>, <is_ibt>,
     *         <is_tch>, <dir>, <call_mode>, <number>, <type>, "<pau>", [<cause>]
     */
    int slotId = message->getSlotId();
    bool ret = false;
    int discCause = 16;

    RfxStringsData* data = (RfxStringsData*)message->getData();
    char** params = (char**)data->getData();
    int callId = atoi(params[0]);
    int msgType = atoi(params[1]);

    if (params[9] != NULL) {
        discCause = atoi(params[9]);
    }

    logD(LOG_REDIAL_TAG, "handleCallInfoUpdate slot:%d, callId:%d, msgType:%d, disc:%d, redial:%d",
                slotId, callId, msgType, discCause, mRedialState);

    if (mMoCall != NULL) {
        if (mMoCall->getCallId() > 0) {
            // moCall Id is valid, only handle the matched call
            if (mMoCall->getCallId() != callId) {
                return false;
            }
        } else {
            // moCall Id is invalid, only handle 130
            if (msgType != ECPI_MSG_TYPE_CALL_ID_ASSIGN) {
                return false;
            }
        }
    } else {
        // no moCall, no need to handle ECPI
        return false;
    }

    int callCount = getStatusManager()->getIntValue(RFX_STATUS_KEY_VOICE_CALL_COUNT, 0);
    switch (msgType) {
        case ECPI_MSG_TYPE_CALL_ID_ASSIGN:
            if (mRedialState != REDIAL_NONE) {
                getStatusManager()->setIntValue(RFX_STATUS_KEY_VOICE_CALL_COUNT, ++callCount);
                // 130 comes again, after redial, ignore it.
                setRedialState(REDIAL_DONE);
                ret = true;
            }

            if (mMoCall->getCallId() <= 0) {
                mMoCall->setCallId(callId);
            }
            break;

        case ECPI_MSG_TYPE_CALL_CONNECT:
            break;

        case ECPI_MSG_TYPE_CALL_DISCONNECT:
            if (mIsUserHangUping) {
                discCause = 0;
            }

            if (discCause == CALL_DISC_CAUSE_NEED_CHANGE_TO_GLOBAL_MODE) {
                if (mRedialState == REDIAL_NONE) {
                    getStatusManager()->setIntValue(RFX_STATUS_KEY_VOICE_CALL_COUNT, --callCount);
                    deferMsg(message);
                    setRedialState(REDIAL_DIALING);
                    setRatMode(REDIAL_RAT_OP_GLOBAL);
                    ret = true;
                } else {
                    logE(LOG_REDIAL_TAG, "redial again!!!, report disconnected and resume");
                    ForceReleaseEmergency();
                }
            } else {
                resetController();
                if (isPendingExitEmergencySession()) {
                    logD(LOG_REDIAL_TAG, "There is a pending exit emergency session to handle");
                    setPendingExitEmergencySession(false);
                    handleEmergencyCallbackMode(false);
                }
            }
            break;

        default:
            break;
    }

    return ret;
}

void RtcRedialController::handleEmergencyCallbackMode(int mode) {
    logD(LOG_REDIAL_TAG, "ECBM:%d->%d", mEcbm, mode);

    if (mEcbm == mode) {
        return;
    }

    mEcbm = mode;
    getStatusManager()->setIntValue(RFX_STATUS_KEY_EMERGENCY_CALLBACK_MODE, mode);

    if (EMERGENCY_CALLBACK_MODE_EXIT == mode) {
        if (decreaseEmcsCountAndIsZero()) {
            setRatMode(REDIAL_RAT_OP_RESUME);
        }

        // Maybe exit ECBM first and the next ECC end, it reports non ECBM
        mEcbm = EMERGENCY_CALLBACK_MODE_NONE;
    }
}

void RtcRedialController::resetController() {
    logD(LOG_REDIAL_TAG, "resetController");
    clearDeferMsg();
    setRedialState(REDIAL_NONE);
    setUserHangUping(false);
    destroyMoCallContext();
}

void RtcRedialController::ForceReleaseEmergency() {
    logE(LOG_REDIAL_TAG, "ForceReleaseEmergency!");

    destroyMoCallContext();

    if (mEcbm == EMERGENCY_CALLBACK_MODE_ENTER) {
        handleEmergencyCallbackMode(EMERGENCY_CALLBACK_MODE_EXIT);
    }else if (mRatOp != REDIAL_RAT_OP_NONE) {
        decreaseEmcsCountAndIsZero();
        setRatMode(REDIAL_RAT_OP_RESUME);
    } else if (mInEmergencyMode) {
        setEmergencyModeExt(false);
    }
}

void RtcRedialController::checkMoCallAndFree() {
    if (mMoCall != NULL) {
        logE(LOG_REDIAL_TAG, "has a mocall: %d, %s", mMoCall->getCallId(),
                (RfxRilUtils::isUserLoad() ? "[hidden]" : mMoCall->getNumber()));

        delete(mMoCall);
        mMoCall = NULL;
    }
}

void  RtcRedialController::destroyMoCallContext() {
    checkMoCallAndFree();
}

int RtcRedialController::getGlobalRatMode() {
    char temp_str[RFX_PROPERTY_VALUE_MAX] = {0};
    int c_slot = -1;
    int pref_nw = -1;
    int radio_capability = getStatusManager(m_slot_id)->getIntValue(
                                                            RFX_STATUS_KEY_SLOT_FIXED_CAPABILITY,
                                                            0);
    if (RatConfig_isC2kSupported()) {
        memset(temp_str, 0, sizeof(temp_str));
        rfx_property_get("persist.vendor.radio.c_capability_slot", temp_str, "1");
        c_slot = atoi(temp_str) - 1;
        logD(LOG_REDIAL_TAG, "getGlobalRatMode, cslot=%d", c_slot);
        if (c_slot == m_slot_id) {
            radio_capability |= (RAF_CDMA_GROUP | RAF_EVDO_GROUP);
        }
    }

    pref_nw = RtcCapabilitySwitchUtil::getNetworkTypeFromRaf(radio_capability);
    return pref_nw;
}

int RtcRedialController::getCurrentRatMode() {
    return getStatusManager(getSlotId())->getIntValue(RFX_STATUS_KEY_PREFERRED_NW_TYPE);
}

void RtcRedialController::setRedialState(RedialState state) {
    logD(LOG_REDIAL_TAG, "setRedialState reddialState:(%d->%d)", mRedialState, state);
    if (mRedialState == state) {
        return;
    }
    mRedialState = state;
}

sp<RfxMessage> RtcRedialController::getRedialRequest() {
    RfxRedialData *pRedial = NULL;

    if (mIsUserHangUping) {
        logD(LOG_REDIAL_TAG, "userhangup is %s", boolToString(mIsUserHangUping));
        setRatMode(REDIAL_RAT_OP_RESUME);
        resumeDeferMsg();
        return NULL;
    }

    if (mMoCall == NULL) {
        logE(LOG_REDIAL_TAG, "mMoCall is NULL", boolToString(mIsUserHangUping));
        return NULL;
    }

    pRedial = mMoCall->getRedialData();
    if (pRedial == NULL) {
        logE(LOG_REDIAL_TAG, "redial request is NULL");
        return NULL;
    }

    return RfxMessage::obtainRequest(getSlotId(),
                            RFX_MSG_REQUEST_EMERGENCY_REDIAL,
                            *pRedial);
}

void RtcRedialController::setUserHangUping(bool userHangUping) {
    logD(LOG_REDIAL_TAG, "setUserHangUping: %s to %s",
            boolToString(mIsUserHangUping), boolToString(userHangUping));
    if (mIsUserHangUping == userHangUping) {
        return;
    }

    mIsUserHangUping = userHangUping;
}

void RtcRedialController::setEmergencyModeExt(bool emergencyMode) {
    if (emergencyMode) {
        increaseEmcsCount();
    } else {
        decreaseEmcsCountAndIsZero();
    }

    setEmergencyMode(emergencyMode);
}

void RtcRedialController::setEmergencyMode(bool emergencyMode) {
    logD(LOG_REDIAL_TAG, "setEmergencyMode:(%s->%s)",
            boolToString(mInEmergencyMode), boolToString(emergencyMode));

    if (mInEmergencyMode == emergencyMode ||
            (emergencyMode == false && !canReleaseEmergencySession())) {
        return;
    }

    if (emergencyMode == false) {
        sp<RfxMessage> rilRequest = RfxMessage::obtainRequest(
                                            getSlotId(),
                                            RFX_MSG_REQUEST_EMERGENCY_SESSION_END,
                                            RfxVoidData());
        requestToMcl(rilRequest);

        // Since ECMI is also unset in modem with AT+EMCS=0
        // update EMCI status key when trigger AT+EMCS=0
        getNonSlotScopeStatusManager()->setBoolValue(
                RFX_STATUS_KEY_EMERGENCY_MODE_IN_FLIGHT_MODE, false);
    }

    getStatusManager()->setBoolValue(RFX_STATUS_KEY_EMERGENCY_MODE, emergencyMode);
    mInEmergencyMode = emergencyMode;
}

void RtcRedialController::redial() {
    const sp<RfxMessage> rilRequest = getRedialRequest();
    if (rilRequest != NULL) {
        requestToMcl(rilRequest);
        setRedialState(REDIAL_DIALED);
    }
}

void RtcRedialController::onRatModeSwitchDone(RedialRatOp ratOp) {
    logD(LOG_REDIAL_TAG, "onRatModeSwitchDone ratOp %d", ratOp);

    if (ratOp == REDIAL_RAT_OP_GLOBAL) {
        // FIXME: if need to check the rat again.
        redial();
    } else if (ratOp == REDIAL_RAT_OP_RESUME) {
        mRatOp = REDIAL_RAT_OP_NONE;
        setEmergencyMode(false);
    }
}

void RtcRedialController::setRatMode(RedialRatOp ratOp) {
    int ratMode = REDIAL_NET_TYPE_RESUME_RAT_MODE;
    int oldRatOp = mRatOp;
    sp<RfxAction> action;
    RtcRatSwitchController* nwRatController =
            (RtcRatSwitchController *)findController(RFX_OBJ_CLASS_INFO(RtcRatSwitchController));

    logD(LOG_REDIAL_TAG, "setRatMode, ratOp:%d->%d, ratState:%d", oldRatOp, ratOp, mRatState);

    // as ERAT may be sent to MD during ECBM/ES, global mode may be changed.
    if (mRatOp == ratOp && ratOp == REDIAL_RAT_OP_RESUME) {
        return;
    }

    if (ratOp == REDIAL_RAT_OP_RESUME &&
            mRatState == REDIAL_RAT_CHANGED &&
            (getStatusManager()->getIntValue(RFX_STATUS_KEY_VOICE_CALL_COUNT, 0) > 0)) {
        logD(LOG_REDIAL_TAG, "setRatMode, pending to resume due to call count not zero");
        mRatOp = REDIAL_RAT_OP_RESUME_PENDING;

        getStatusManager()->registerStatusChanged(
                RFX_STATUS_KEY_VOICE_CALL_COUNT,
                RfxStatusChangeCallback(this, &RtcRedialController::onCallCountChanged));

        return;
    }

    mRatOp = ratOp;

    // change to Global mode.
    if (ratOp == REDIAL_RAT_OP_GLOBAL) {
        int curRatMode = getStatusManager()->getIntValue(RFX_STATUS_KEY_PREFERRED_NW_TYPE);
        ratMode = getGlobalRatMode();

        logD(LOG_REDIAL_TAG, "swtich rat mode(Preferred network type:%d -> %d)",
                curRatMode, ratMode);
        if (curRatMode == ratMode) {  // already in global. maybe MD should handle this.
            if (mRatState != REDIAL_RAT_CHANGED) {
                mRatState = REDIAL_RAT_NONE;
            }
            onRatModeSwitchDone(REDIAL_RAT_OP_GLOBAL);
            return;
        }

        mRatState = REDIAL_RAT_CHANGED;
    } else if (ratOp == REDIAL_RAT_OP_RESUME) { // resume
        if (mRatState == REDIAL_RAT_NONE) {
            onRatModeSwitchDone(REDIAL_RAT_OP_RESUME);
            return;
        }

        mRatState = REDIAL_RAT_NONE;
    }

    action = new RfxAction1<RedialRatOp>(this, &RtcRedialController::onRatModeSwitchDone, ratOp);
    nwRatController->setPreferredNetworkType(ratMode, action);
}

void RtcRedialController::increaseEmcsCount() {
    mEmcsCount++;
    setPendingExitEmergencySession(false);
}

bool RtcRedialController::canReleaseEmergencySession() {
    return (mEmcsCount == 0) ? true : false;
}

bool RtcRedialController::decreaseEmcsCountAndIsZero() {
    bool ret = ((-- mEmcsCount) <= 0) ? true : false;
    logD(LOG_REDIAL_TAG, "mEmcsCount:%d", mEmcsCount);
    if (mEmcsCount < 0) {
        mEmcsCount = 0;
    }

    return ret;
}

void RtcRedialController::deferMsg(const sp<RfxMessage>& msg) {
    logD(LOG_REDIAL_TAG, "Defer the current message %s", RFX_ID_TO_STR(msg->getId()));

    mSuspendedMsgQueue.add(RfxSuspendedMsgEntry(this, msg));
}

void RtcRedialController::resumeDeferMsg() {
    const RfxSuspendedMsgEntry *msgEntry;

    while (mSuspendedMsgQueue.isEmpty() == false) {
        msgEntry = &(mSuspendedMsgQueue.itemAt(0));
        RfxMainThread::enqueueMessage(msgEntry->message);

        logD(LOG_REDIAL_TAG, "Resume the message %s", RFX_ID_TO_STR(msgEntry->message->getId()));

        mSuspendedMsgQueue.removeAt(0);
    }
}

void RtcRedialController::clearDeferMsg() {
    logD(LOG_REDIAL_TAG, "clearDeferMsg");
    mSuspendedMsgQueue.clear();
}

void RtcRedialController::onCallCountChanged(RfxStatusKeyEnum key, RfxVariant old_value,
        RfxVariant value) {
    RFX_UNUSED(key);
    RFX_UNUSED(old_value);
    int callCount = value.asInt();
    if (callCount == 0) {
        logD(LOG_REDIAL_TAG, "call count to zero.");
        getStatusManager()->unRegisterStatusChanged(
                RFX_STATUS_KEY_VOICE_CALL_COUNT,
                RfxStatusChangeCallback(this, &RtcRedialController::onCallCountChanged));

        if (mRatOp == REDIAL_RAT_OP_RESUME_PENDING) {
            setRatMode(REDIAL_RAT_OP_RESUME);
        }
    }
}

void RtcRedialController::setPendingExitEmergencySession(bool pending) {
    // Remove, when mEmcsCount is zero, it will call setEmergencyMode and register call count status.
    // mPendingExitES = pending;
    RFX_UNUSED(pending);
}

bool RtcRedialController::isPendingExitEmergencySession() {
    return mPendingExitES;
}

void RtcRedialController::setEccCallId(int callId) {
    if (mMoCall != NULL && mMoCall->getCallId() <= 0) {
        mMoCall->setCallId(callId);
        logD(LOG_REDIAL_TAG, "set call id.");
    }
}


/*****************************************************************************
 * Class MoCallContext
 *****************************************************************************/
MoCallContext::MoCallContext(const sp<RfxMessage>& msg, bool isEcc) {
    mCallId = -1;
    mClientId = msg->getClientId();
    mIsEcc = isEcc;
    mNumber = NULL;
    mRedialData = NULL;

    mDialData = new RfxDialData(msg->getData()->getData(),
                                        msg->getData()->getDataLength());
    if (DBG) {  // false
        mtkLogD(LOG_REDIAL_TAG,
                "MoCallContext Constructor(message:%s, clientId:%d, isEcc:%s), number =%s",
                msg->toString().string(), mClientId, boolToString(isEcc), mNumber);
    }
}

MoCallContext::MoCallContext(char *callNumber, bool isEcc) {
    mCallId = -1;
    mClientId = -1;
    mIsEcc = isEcc;
    mDialData = NULL;
    mRedialData = NULL;

    if (callNumber != NULL) {
        mNumber = strdup(callNumber);
    } else {
        mNumber = NULL;
    }

    if (DBG) {
        mtkLogD(LOG_REDIAL_TAG, "MoCallContext Constructor(CallNumber:%s, isEcc:%s)",
                callNumber, boolToString(isEcc));
    }
}

MoCallContext::~MoCallContext() {
    // free memory
    mtkLogD(LOG_REDIAL_TAG, "MoCallContext destructor");
    if (mNumber != NULL) {
        free(mNumber);
    }

    if (mDialData != NULL) {
        delete(mDialData);
    }

    if (mRedialData != NULL) {
        delete(mRedialData);
    }
}

void MoCallContext::setCallId(int callId) {
    if (DBG) {
        mtkLogD(LOG_REDIAL_TAG, "MoCallContext setCallId:%d", callId);
    }
    mCallId = callId;
}

char *MoCallContext::getNumber() const {
    char *number = mNumber;
    if (number == NULL && mDialData != NULL) {
        RIL_Dial *dial_data = (RIL_Dial *)mDialData->getData();;
        number = dial_data->address;
    }

    return number;
}

RfxRedialData *MoCallContext::getRedialData() {
    RIL_Dial *dial_data = NULL;
    bool is_free = false;

    if (DBG) {
        mtkLogD(LOG_REDIAL_TAG, "MoCallContext data null(number:%s,mDialData:%s)",
            boolToString(mNumber == NULL), boolToString(mDialData == NULL));
    }

    if (mDialData != NULL) {
        dial_data = (RIL_Dial *)mDialData->getData();
    } else if (mNumber != NULL) {
        dial_data = (RIL_Dial *)calloc(1, sizeof(RIL_Dial));
        RFX_ASSERT(dial_data != NULL);
        dial_data->address = mNumber;
        dial_data->clir = 0;
        dial_data->uusInfo = NULL;
        is_free = true;
    }

    if (dial_data != NULL) {
        if (mRedialData != NULL) {
            delete(mRedialData);
        }

        mRedialData = new RfxRedialData(dial_data, mCallId);
        if (is_free) {
            free(dial_data);
        }
        mRedialData->dump();
    }

    return mRedialData;
}

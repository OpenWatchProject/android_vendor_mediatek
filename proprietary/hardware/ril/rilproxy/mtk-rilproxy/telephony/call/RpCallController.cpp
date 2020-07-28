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
#include "RpCallController.h"
#include "util/RpFeatureOptionUtils.h"
#include <compiler_utils.h>
#include "phonenumber/RpPhoneNumberController.h"

#undef LOG_TAG
#define LOG_TAG "RpCallControl"
#define RFX_LOG_TAG "RpCallControl"

#define MIN_IMS_CALL_MODE 20
#define MAX_IMS_CALL_MODE 39

#ifndef UNUSED
#define UNUSED(x) (x)   // Eliminate "warning: unused parameter"
#endif
#define AUTO_ANSWER_DELAY 5 * 1000

using namespace android;

/*****************************************************************************
 * Class RpCallController
 *****************************************************************************/
RFX_IMPLEMENT_CLASS("RpCallController", RpCallController, RfxController);

RpCallController::RpCallController() :
mCsPhone(0), mCallCount(0)
, mCsCallCount(0), mPsCallCount(0)
, mEccNumber(""), mPreciseCallStateList(NULL), mCallRat(CALL_RAT_NONE)
, mRedialHandler(NULL), mAudioControlHandler(NULL)
, mOriginalDialMessage(NULL)
, mUseLocalCallFailCause(false), mDialLastError(0)
, mAutoAnswerTimerHandle(NULL), mCallCountForAutoAnswer(0)
{
    char value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("ro.vendor.mtk_switch_antenna", value, "");
    mSwitchAntennaSupported = !strcmp(value, "1");
}

RpCallController::~RpCallController() {
    if (mRedialHandler != NULL) {
        RFX_OBJ_CLOSE(mRedialHandler);
    }

    if (mAudioControlHandler != NULL) {
        RFX_OBJ_CLOSE(mAudioControlHandler);
    }
}

void RpCallController::onInit() {
    RfxController::onInit(); // Required: invoke super class implementation
    RLOGD("onInit E! (slot %d)", getSlotId());

    mPreciseCallStateList = new Vector<RfxPreciseCallState*>();

    const int request_id_list_gsm[] = {
        RIL_REQUEST_EMERGENCY_DIAL,
        RIL_REQUEST_IMS_EMERGENCY_DIAL,
        RIL_LOCAL_REQUEST_EMERGENCY_DIAL,
        RIL_LOCAL_REQUEST_IMS_EMERGENCY_DIAL,
        RIL_LOCAL_REQUEST_SET_ECC_SERVICE_CATEGORY,
    };

    const int urc_id_list_gsm[] = {
        RIL_UNSOL_CALL_INFO_INDICATION,
        RIL_UNSOL_CALL_RING,
    };

    const int request_id_list_c2k[] = {
        /* Common Request */
        RIL_REQUEST_GET_CURRENT_CALLS,
        RIL_REQUEST_DIAL,
        RIL_REQUEST_IMS_DIAL,
        RIL_REQUEST_HANGUP,
        RIL_REQUEST_HANGUP_WAITING_OR_BACKGROUND,
        RIL_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND,
        RIL_REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE,
        RIL_REQUEST_CONFERENCE,
        RIL_REQUEST_UDUB,
        RIL_REQUEST_LAST_CALL_FAIL_CAUSE,
        RIL_REQUEST_DTMF,
        RIL_REQUEST_ANSWER,
        RIL_REQUEST_DTMF_START,
        RIL_REQUEST_DTMF_STOP,
        RIL_REQUEST_SEPARATE_CONNECTION,
        RIL_REQUEST_SET_MUTE,
        RIL_REQUEST_CDMA_FLASH,
        RIL_REQUEST_EXIT_EMERGENCY_CALLBACK_MODE,
        /* MTK RIL Request */
        RIL_REQUEST_HANGUP_ALL,
        RIL_REQUEST_EMERGENCY_DIAL,
        RIL_REQUEST_IMS_EMERGENCY_DIAL,
        /* RpRedialHandler Request */
        RIL_LOCAL_C2K_REQUEST_REDIAL,
        RIL_LOCAL_REQUEST_EMERGENCY_REDIAL,
        RIL_LOCAL_C2K_REQUEST_EMERGENCY_REDIAL,
        RIL_LOCAL_REQUEST_EMERGENCY_SESSION_BEGIN,
        RIL_LOCAL_REQUEST_EMERGENCY_SESSION_END,
        RIL_LOCAL_REQUEST_SET_HVOLTE_MODE,
        RIL_REQUEST_SET_ECC_MODE,
        RIL_LOCAL_REQUEST_EMERGENCY_DIAL,
        RIL_LOCAL_REQUEST_IMS_EMERGENCY_DIAL,
        RIL_LOCAL_REQUEST_SET_ECC_SERVICE_CATEGORY,
        RIL_LOCAL_C2K_REQUEST_EMERGENCY_DIAL,
    };

    const int urc_id_list_c2k[] = {
        /* Common URC */
        RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED,
        RIL_UNSOL_CDMA_CALL_WAITING,
        RIL_UNSOL_ENTER_EMERGENCY_CALLBACK_MODE,
        RIL_UNSOL_EXIT_EMERGENCY_CALLBACK_MODE,
        RIL_UNSOL_CALL_RING,
        /* MTK RIL URC */
        RIL_UNSOL_CALL_INFO_INDICATION,
        /// M: CC: Proprietary incoming call handling (GSM)
        RIL_UNSOL_INCOMING_CALL_INDICATION,
        // IMS
        RIL_UNSOL_SRVCC_STATE_NOTIFY,
    };

    // register request & URC id list
    // NOTE. one id can only be registered by one controller
    if (RpFeatureOptionUtils::isC2kSupport()) {
        registerToHandleRequest(request_id_list_c2k, sizeof(request_id_list_c2k)/sizeof(int));
        registerToHandleUrc(urc_id_list_c2k, sizeof(urc_id_list_c2k)/sizeof(int));
        RLOGD("request size:%d urc size:%d!",
            (int) (sizeof(request_id_list_c2k)/sizeof(int)), (int) (sizeof(urc_id_list_c2k)/sizeof(int)));

        // Create RpRedialHandler object
        RFX_OBJ_CREATE(mRedialHandler, RpRedialHandler, this);
    } else {
        registerToHandleRequest(request_id_list_gsm, sizeof(request_id_list_gsm)/sizeof(int));
        registerToHandleUrc(urc_id_list_gsm, sizeof(urc_id_list_gsm)/sizeof(int));
        RLOGD("request size:%d urc size:%d!",
            (int) (sizeof(request_id_list_gsm)/sizeof(int)), (int) (sizeof(urc_id_list_gsm)/sizeof(int)));
    }

    // Create RpAudioControlHandler object
    RFX_OBJ_CREATE(mAudioControlHandler, RpAudioControlHandler, this);

    // register callbacks for CS phone
    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_NWS_MODE,
        RfxStatusChangeCallback(this, &RpCallController::onCsPhoneChanged));
    //Get the latest NWS_MODE since the default(intialization) mode may not be NWS_MODE_CSFB.
    int nws = getStatusManager(getSlotId())->getIntValue(RFX_STATUS_KEY_NWS_MODE, NWS_MODE_CSFB);
    mCsPhone = (nws == NWS_MODE_CDMALTE) ? RADIO_TECH_GROUP_C2K : RADIO_TECH_GROUP_GSM;

    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_SERVICE_STATE,
        RfxStatusChangeCallback(this, &RpCallController::onServiceStateChanged));

    char prop[PROPERTY_VALUE_MAX] = {0};
    property_get("ril.cdma.inecmmode", prop, "");
    if (strncmp(prop, "true", strlen("true")) == 0) {
        // Notify ECBM exited if modem reset
        responseToRilj(RfxMessage::obtainUrc(m_slot_id, RIL_UNSOL_EXIT_EMERGENCY_CALLBACK_MODE));
    }
}

void RpCallController::onDeinit() {
    RLOGD("onDeinit E! (slot %d)", getSlotId());
    freePreciseCallStateList(mPreciseCallStateList);
    mPreciseCallStateList = NULL;
    RfxController::onDeinit();
}

void RpCallController::onCsPhoneChanged(RfxStatusKeyEnum key,
        RfxVariant old_value, RfxVariant value) {
    RFX_UNUSED(key);
    int nws = value.asInt();
    int oldNws = old_value.asInt();

    int preCsPhone = (oldNws == NWS_MODE_CDMALTE) ? RADIO_TECH_GROUP_C2K : RADIO_TECH_GROUP_GSM;

    UNUSED(key);

    mCsPhone = (nws == NWS_MODE_CDMALTE) ? RADIO_TECH_GROUP_C2K : RADIO_TECH_GROUP_GSM;

    if (mAudioControlHandler != NULL && RpFeatureOptionUtils::isSvlteSupport()) {
        mAudioControlHandler->updateAudioPathAsync(mCsPhone);
    }

    int callCount = getStatusManager(getSlotId())->getIntValue(RFX_STATUS_KEY_VOICE_CALL_COUNT, 0);
    RLOGD("onCsPhoneChanged, (slot %d) preCsPhone %d, mCsPhone %d, callCount:%d",
            getSlotId(), preCsPhone, mCsPhone, callCount);

    // If redial state is set, notify CallTracker via system property
    if ((mRedialHandler == NULL || mRedialHandler->getRedialState() == REDIAL_NONE)
            && preCsPhone != mCsPhone) {
        sp<RfxMessage> rilRequest = RfxMessage::obtainRequest(getSlotId(),
                (RILD_RadioTechnology_Group)preCsPhone, RIL_REQUEST_HANGUP_ALL);
        requestToRild(rilRequest);
        mCallCount = 0;
        mCsCallCount = 0;
        mPsCallCount = 0;
        getStatusManager(getSlotId())->setIntValue(RFX_STATUS_KEY_VOICE_CALL_COUNT, mCallCount);
        if (!RpFeatureOptionUtils::isSvlteSupport()) {
            mAudioControlHandler->updateAudioPathSync(RADIO_TECH_GROUP_GSM, NULL);
        }
    }
}

void RpCallController::onServiceStateChanged(RfxStatusKeyEnum key,
        RfxVariant oldValue, RfxVariant newValue) {
    RFX_UNUSED(key);
    RfxNwServiceState oldSS = oldValue.asServiceState();
    RfxNwServiceState newSS = newValue.asServiceState();
    int voiceRadioTech = oldSS.getRilVoiceRadioTech();;
    int voiceRegState = oldSS.getRilVoiceRegState();;

    if (oldSS.getRilVoiceRadioTech() != newSS.getRilVoiceRadioTech()) {
        voiceRadioTech = newSS.getRilVoiceRadioTech();
        RLOGD("(slot %d) Voice service state changed: radioTech=%d (newSS: %s)",
                m_slot_id, voiceRadioTech, newSS.toString().string());
    }
    if (oldSS.getRilVoiceRegState() != newSS.getRilVoiceRegState()) {
        voiceRegState = newSS.getRilVoiceRegState();
        RLOGD("(slot %d) Voice service state changed: regState=%d (newSS: %s)",
                m_slot_id, voiceRegState, newSS.toString().string());
    }

    if (voiceRegState != RIL_REG_STATE_HOME && voiceRegState != RIL_REG_STATE_ROAMING) {
        mCallRat = CALL_RAT_NO_SERIVCE;
    } else {
        switch (voiceRadioTech) {
            case RADIO_TECH_GPRS:
            case RADIO_TECH_EDGE:
            case RADIO_TECH_GSM:
                mCallRat = CALL_RAT_GSM;
                break;
            case RADIO_TECH_UMTS:
            case RADIO_TECH_HSDPA:
            case RADIO_TECH_HSUPA:
            case RADIO_TECH_HSPA:
            case RADIO_TECH_HSPAP:
            case RADIO_TECH_TD_SCDMA:
                mCallRat = CALL_RAT_UMTS;
                break;
            case RADIO_TECH_IS95A:
            case RADIO_TECH_IS95B:
            case RADIO_TECH_1xRTT:
            case RADIO_TECH_EVDO_0:
            case RADIO_TECH_EVDO_A:
            case RADIO_TECH_EVDO_B:
            case RADIO_TECH_EHRPD:
                mCallRat = CALL_RAT_CDMA;
                break;
            case RADIO_TECH_LTE:
                mCallRat = CALL_RAT_LTE;
                break;
            case RADIO_TECH_IWLAN:
                mCallRat = CALL_RAT_WIFI;
                break;
            default:
                mCallRat = CALL_RAT_NONE;
                break;
            }
    }

    RLOGD("onServiceStateChanged, (slot %d) callRat=%d (newSS: %s)", m_slot_id, mCallRat,
            newSS.toString().string());
}

bool RpCallController::onPreHandleRequest(const sp<RfxMessage>& request) {
    int msg_id = request->getId();

    // Reject dial for 2 calls on different SIM (DSDS)
    RpCallCenterController *centerController =
            (RpCallCenterController *)findController(RFX_SLOT_ID_UNKNOWN,
             RFX_OBJ_CLASS_INFO(RpCallCenterController));
    if (msg_id == RIL_REQUEST_EMERGENCY_DIAL || msg_id == RIL_REQUEST_DIAL) {
        int modemOffState = getNonSlotScopeStatusManager()->getIntValue(
                RFX_STATUS_KEY_MODEM_OFF_STATE, MODEM_OFF_IN_IDLE);
        if (modemOffState == MODEM_OFF_BY_SIM_SWITCH) {
            mUseLocalCallFailCause = true;
            mDialLastError = CALL_FAIL_ERROR_UNSPECIFIED;
            sp<RfxMessage> response = RfxMessage::obtainResponse(
                    RIL_E_GENERIC_FAILURE, request);
            responseToRilj(response);
            RLOGD("Response RIL_E_GENERIC_FAILURE");
            return true;
        }

        if (SIM_COUNT < 2) {
            RLOGD("Skip for SS project");
        } else if (centerController != NULL) {
            if (mSwitchAntennaSupported || !centerController->isInDsdaMode()) {
                if (centerController->handleDualDialForDSDS(getSlotId())) {
                    mUseLocalCallFailCause = true;
                    mDialLastError = CALL_FAIL_ERROR_UNSPECIFIED;
                    sp<RfxMessage> response = RfxMessage::obtainResponse(
                            RIL_E_GENERIC_FAILURE, request);
                    responseToRilj(response);
                    RLOGD("Response RIL_E_GENERIC_FAILURE");
                    return true;
                }
            }
            mUseLocalCallFailCause = false;
            mDialLastError = 0;
        } else {
            RLOGE("Can't find RpCallCenterController!");
        }
    } else if (msg_id == RIL_REQUEST_LAST_CALL_FAIL_CAUSE) {
        if (mUseLocalCallFailCause) {
            sp<RfxMessage> response = RfxMessage::obtainResponse(
                    RIL_E_SUCCESS, request);
            Parcel* p = response->getParcel();
            p->writeInt32(mDialLastError);
            writeStringToParcel(p, "");
            responseToRilj(response);
            RLOGD("Use local call fail cause for slot%d", getSlotId());
            mUseLocalCallFailCause = false;
            mDialLastError = 0;
            return true;
        }
    }

    // Switch Antenna
    if (msg_id == RIL_REQUEST_DIAL || msg_id == RIL_REQUEST_EMERGENCY_DIAL
            || msg_id == RIL_REQUEST_ANSWER) {
        if (mSwitchAntennaSupported) {
            if (SIM_COUNT < 2) {
                RLOGD("Skip for SS project");
            } else if (centerController != NULL) {
                SWITCH_ANTENNA_RAT_MODE ratMode = RAT_MODE_NONE;
                if (mCsPhone == RADIO_TECH_GROUP_C2K) {
                    ratMode = RAT_MODE_C2K;
                } else if (mCsPhone == RADIO_TECH_GROUP_GSM) {
                    ratMode = RAT_MODE_GSM;
                }
                centerController->handleSwitchAntennaRequest(STATE_CONNECTED, ratMode);
            } else {
                RLOGE("Can't find RpCallCenterController!");
            }
        }
    }
    return false;
}

bool RpCallController::onHandleRequest(const sp<RfxMessage>& request) {
    int msg_id = request->getId();
    Parcel *p = request->getParcel();
    size_t dataPos = p->dataPosition();

    if (onPreHandleRequest(request)) {
        RLOGD("handle req %d %s (slot %d) within rilproxy, not to Rild", msg_id,
            requestToString(msg_id), getSlotId());
        return false;
    }
    // request might be modified, to reset parcel pointer position
    p->setDataPosition(dataPos);

    RLOGD("handle req %d %s (slot %d) CsPhone %d", msg_id,
            requestToString(msg_id), getSlotId(), mCsPhone);

    if ((msg_id == RIL_REQUEST_EMERGENCY_DIAL
            || msg_id == RIL_REQUEST_IMS_EMERGENCY_DIAL)) {
        mOriginalDialMessage = request;
        RIL_Dial dial;
        RIL_UUS_Info uusInfo;
        int32_t t;
        int32_t uusPresent;

        memset(&dial, 0, sizeof(dial));

        dial.address = strdupReadString(p);

        p->readInt32(&t);
        dial.clir = (int)t;

        p->readInt32(&uusPresent);

        if (uusPresent == 0) {
            dial.uusInfo = NULL;
        } else {
            int32_t len;

            memset(&uusInfo, 0, sizeof(RIL_UUS_Info));

            p->readInt32(&t);
            uusInfo.uusType = (RIL_UUS_Type) t;

            p->readInt32(&t);
            uusInfo.uusDcs = (RIL_UUS_DCS) t;

            p->readInt32(&len);

            // The java code writes -1 for null arrays
            if (((int) len) == -1) {
                uusInfo.uusData = NULL;
                len = 0;
            } else {
                uusInfo.uusData = (char*) p->readInplace(len);
            }

            uusInfo.uusLength = len;
            dial.uusInfo = &uusInfo;
        }

        p->readInt32(&t);
        EmergencyServiceCategory dialServiceCategory = (EmergencyServiceCategory)t;

        p->readInt32(&t);
        EmergencyCallRouting routing = (EmergencyCallRouting)t;

        p->readInt32(&t);
        int isTesting = (t != 0);

        String8 dialNumber = String8::format("%s", dial.address);
        RpPhoneNumberController* phoneNumberController = (RpPhoneNumberController *)findController(
                getSlotId(), RFX_OBJ_CLASS_INFO(RpPhoneNumberController));
        EmergencyCallRouting eccRouting =
                phoneNumberController->getEmergencyCallRouting(dialNumber);

        if (msg_id == RIL_REQUEST_IMS_EMERGENCY_DIAL
                /*|| eccNumberController->isEmergencyNumber(dialNumber)*/) {
            dial.clir = 0;
        }

        // for debug
        logD(RFX_LOG_TAG, "isTesting=%d, routing=%d, serviceCategory=%d ",
               isTesting, routing, dialServiceCategory);
        logD(RFX_LOG_TAG, "clir=%d, number=%s ", dial.clir, dial.address);

        int requestId = 0;
        RILD_RadioTechnology_Group requestType =
                (mCsPhone == RADIO_TECH_GROUP_GSM ? RADIO_TECH_GROUP_GSM : RADIO_TECH_GROUP_C2K);

        if (!isTesting &&
                ((routing == ECC_ROUTING_UNKNOWN && eccRouting != ECC_ROUTING_NORMAL) ||
                 routing == ECC_ROUTING_EMERGENCY)){
            int serviceCategory = dialServiceCategory;
            if (dialServiceCategory == UNSPECIFIED) {
                serviceCategory = phoneNumberController->getServiceCategory(dialNumber);
            }

            sp<RfxMessage> serviceCategoryRequest = RfxMessage::obtainRequest(
                    getSlotId(),
                    requestType,
                    RIL_LOCAL_REQUEST_SET_ECC_SERVICE_CATEGORY);
                    serviceCategoryRequest->getParcel()->writeInt32(1);
                    serviceCategoryRequest->getParcel()->writeInt32(serviceCategory);
            requestToRild(serviceCategoryRequest);

            requestId = request->getId() == RIL_REQUEST_EMERGENCY_DIAL
                    ? ((RADIO_TECH_GROUP_C2K == getRilRequestSendDomain(request))
                            ? RIL_LOCAL_C2K_REQUEST_EMERGENCY_DIAL
                            : RIL_LOCAL_REQUEST_EMERGENCY_DIAL)
                    : RIL_LOCAL_REQUEST_IMS_EMERGENCY_DIAL;
        } else {
            // For VTS, isTesting = true, just fake an SUCCESS
            if (isTesting) {
                sp<RfxMessage> responseMsg = RfxMessage::obtainResponse(RIL_E_SUCCESS, request,
                        true);
                responseToRilj(responseMsg);

                memset(dial.address, 0, strlen(dial.address));
                free(dial.address);
                memset(&dial, 0, sizeof(dial));

                return true;
            }
         // For either one of the following cases
         // pEccDial->isTesting
         // pEccDial->routing == NORMAL
         // pEccDial->routing == UNKNOWN && !isEccRouting
            requestId = request->getId() == RIL_REQUEST_EMERGENCY_DIAL
                    ? RIL_REQUEST_DIAL
                    : RIL_REQUEST_IMS_DIAL;
        }

        // Create dialParcel
        Parcel* dialParcel = new Parcel();

        dialParcel->writeInt32((requestId & 0xFFFFFFFF));
        dialParcel->setDataPosition(sizeof(int32_t)*2);

        char16_t *pString16;
        size_t len16;
        pString16 = strdup8to16(dial.address, &len16);
        dialParcel->writeString16(pString16, len16);
        free(pString16);

        dialParcel->writeInt32(dial.clir);
        if (dial.uusInfo != NULL) {
            dialParcel->writeInt32(1);
            dialParcel->writeInt32(dial.uusInfo->uusType);
            dialParcel->writeInt32(dial.uusInfo->uusDcs);
            if (dial.uusInfo->uusLength == 0) {
                dialParcel->writeInt32(-1);
            } else {
                dialParcel->writeInt32(dial.uusInfo->uusLength);
                dialParcel->write( (void*) dial.uusInfo->uusData, dial.uusInfo->uusLength);
            }
        } else {
            dialParcel->writeInt32(0);
        }

        memset(dial.address, 0, strlen(dial.address));
        free(dial.address);
        memset(&dial, 0, sizeof(dial));

        sp<RfxMessage> dialRequest = RfxMessage::obtainRequest(
                getSlotId(),
                requestType,
                requestId,
                request->getPToken(),
                dialParcel,
                request->getRilToken());
        return onHandleRequest(dialRequest);
    }


    // For customer who doesn't use MTK framework and always use dial() to make ECC call,
    // check if the number is ECC and make an emergency dial request and save the original
    // request for sending response later.
    // TODO



    switch (msg_id) {
        case RIL_LOCAL_REQUEST_EMERGENCY_DIAL:
        case RIL_LOCAL_REQUEST_IMS_EMERGENCY_DIAL:
        case RIL_LOCAL_C2K_REQUEST_EMERGENCY_DIAL:
            handleEmergencyDialRequest(request);
            // request might be modified, to reset parcel pointer position
            p->setDataPosition(dataPos);
            TELEPHONYWARE_FALLTHROUGH;
            // fall through
        case RIL_REQUEST_DIAL:
        case RIL_REQUEST_IMS_DIAL:
        case RIL_REQUEST_ANSWER:
            if (mAudioControlHandler != NULL) {
                if (getRilRequestSendDomain(request) == RADIO_TECH_GROUP_C2K) {
                    mAudioControlHandler->updateAudioPathAsync(RADIO_TECH_GROUP_C2K);
                }

                if (msg_id == RIL_REQUEST_ANSWER) {
                    break;
                }
            }
            TELEPHONYWARE_FALLTHROUGH;
            // fall through: Also notify RpRedialhandler
        case RIL_REQUEST_HANGUP:
        case RIL_REQUEST_HANGUP_ALL:
        case RIL_REQUEST_SET_ECC_MODE:
            if (mRedialHandler != NULL) {
                if (mRedialHandler->notifyRilRequest(request) == false) {
                    // The message was queued by RedialHandler,
                    // No need to handle this message
                    return true;
                }
            }
            // request might be modified, to reset parcel pointer position
            p->setDataPosition(dataPos);
            if (mCsPhone == RADIO_TECH_GROUP_GSM) {
                int argc = p->readInt32(); // count
                int hangupCallId = p->readInt32();
                if (hasPendingHangupRequest(hangupCallId)) {
                    sp<RfxMessage> responseMsg = RfxMessage::obtainResponse(RIL_E_SUCCESS, request,
                        true);
                    responseToRilj(responseMsg);
                    return true;
                }
                updateDisconnecting(mPreciseCallStateList, hangupCallId);
                // request might be modified, to reset parcel pointer position
                p->setDataPosition(dataPos);
            }
            break;

        case RIL_REQUEST_HANGUP_WAITING_OR_BACKGROUND:
            if (mCsPhone == RADIO_TECH_GROUP_GSM) {
                if (hasPendingHangupRequest(false/*isForegnd*/)) {
                    sp<RfxMessage> responseMsg = RfxMessage::obtainResponse(RIL_E_SUCCESS, request,
                            true);
                    responseToRilj(responseMsg);
                    return true;
                }
                updateDisconnecting(mPreciseCallStateList, false/*isForegnd*/);
            }
            break;
        case RIL_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND:
            if (mCsPhone == RADIO_TECH_GROUP_GSM) {
                if (hasPendingHangupRequest(true/*isForegnd*/)) {
                    sp<RfxMessage> responseMsg = RfxMessage::obtainResponse(RIL_E_SUCCESS, request,
                            true);
                    responseToRilj(responseMsg);
                    return true;
                }
                updateDisconnecting(mPreciseCallStateList, true/*isForegnd*/);
            }
            break;

        case RIL_REQUEST_CDMA_FLASH:
            handleCdmaFlashRequest(request);
            // request might be modified, to reset parcel pointer position
            p->setDataPosition(dataPos);
            break;

        default:
            break;
    }

    // fill in items in RfxMessage for c2k_request
    // such as new a parcel and attach it to c2k_request
    // NOTE. attached parcel will be free when the RfxMessage
    //       instance is destroyed, so don't attach one parcel
    //       object to more than one RfxMessage instance
    if (RADIO_TECH_GROUP_C2K == getRilRequestSendDomain(request)) {
        sp<RfxMessage> c2kRequest = RfxMessage::obtainRequest(
                RADIO_TECH_GROUP_C2K, msg_id, request, true);

        RLOGD("Sent RIL Request:%s to MD3", requestToString(msg_id));
        requestToRild(c2kRequest);
    } else {
        sp<RfxMessage> gsmRequest = RfxMessage::obtainRequest(
                RADIO_TECH_GROUP_GSM, msg_id, request, true);
        RLOGD("Sent RIL Request:%s to MD1", requestToString(msg_id));
        requestToRild(gsmRequest);
    }

    return true;
}

bool RpCallController::onHandleResponse(const sp<RfxMessage>& response) {
    int msg_id = response->getId();
    sp<RfxMessage> rilResponse = response;
    RIL_Errno errNo;

    RLOGD("handle resp %d %s (slot %d)", msg_id, requestToString(msg_id),
            getSlotId());

    switch (msg_id) {
        case RIL_REQUEST_GET_CURRENT_CALLS:
            if (mRedialHandler != NULL) {
                RLOGD("forward resp %s (slot %d) to RedialHandler",
                        requestToString(msg_id), getSlotId());

                // This reponse may be modified in RedialHandler
                rilResponse = mRedialHandler->notifyRilResponse(response);
            }

            onHandleGetCurrentCalls(rilResponse);
            break;

        case RIL_REQUEST_DIAL:
            errNo = response->getError();
            if (errNo != RIL_E_SUCCESS) {
                RLOGD("handle resp %d error %d", msg_id, errNo);
                if (!RpFeatureOptionUtils::isSvlteSupport()) {
                    mAudioControlHandler->updateAudioPathSync(RADIO_TECH_GROUP_GSM, NULL);
                }
            }
            TELEPHONYWARE_FALLTHROUGH;
            // fall through
        case RIL_REQUEST_IMS_DIAL:
        case RIL_LOCAL_C2K_REQUEST_REDIAL:
        case RIL_LOCAL_REQUEST_EMERGENCY_REDIAL:
        case RIL_LOCAL_C2K_REQUEST_EMERGENCY_REDIAL:
        case RIL_LOCAL_REQUEST_EMERGENCY_SESSION_BEGIN:
        case RIL_LOCAL_REQUEST_EMERGENCY_SESSION_END:
        case RIL_LOCAL_REQUEST_SET_HVOLTE_MODE:
            if (mRedialHandler != NULL) {
                rilResponse = mRedialHandler->notifyRilResponse(response);

                if (rilResponse != NULL) {
                    responseToRilj(rilResponse);
                }
            }
            break;

        case RIL_REQUEST_UDUB:
            {
                int callCount = getStatusManager(getSlotId())->getIntValue(
                        RFX_STATUS_KEY_VOICE_CALL_COUNT, 0);
                int card_types = getStatusManager(getSlotId())->getIntValue(
                        RFX_STATUS_KEY_CARD_TYPE, -1);
                if (callCount == 0 && card_types <= 0) {
                    RLOGI("handle UDUB response, update error code (callCount:%d, card_types %d)",
                            callCount, card_types);
                    /// M: CC: update response code to fix VTS
                    sp<RfxMessage> rilResponse2 = RfxMessage::obtainResponse(
                            RIL_E_INVALID_STATE, rilResponse, true);
                    responseToRilj(rilResponse2);
                } else {
                    responseToRilj(rilResponse);
                }
            }
            break;

        case RIL_LOCAL_REQUEST_EMERGENCY_DIAL:
        case RIL_LOCAL_C2K_REQUEST_EMERGENCY_DIAL:
            errNo = response->getError();
            if (errNo != RIL_E_SUCCESS) {
                RLOGD("handle resp %d error %d", msg_id, errNo);
                if (!RpFeatureOptionUtils::isSvlteSupport()) {
                    mAudioControlHandler->updateAudioPathSync(RADIO_TECH_GROUP_GSM, NULL);
                }
            }
            TELEPHONYWARE_FALLTHROUGH;
            // fall through
        case RIL_LOCAL_REQUEST_IMS_EMERGENCY_DIAL:
            if (mOriginalDialMessage != NULL) {
                // For customer who doesn't use MTK framework and always use dial() to make ECC call,
                // it should response the original dial request saved in onHandleRequest().
                // For customer who doesn't use MTK framework and always use dial() to make ECC call,
                // it should response the original dial request saved in onHandleRequest().
                Parcel* np = new Parcel();
                np->writeInt32(0);  // RESPONSE_SOLICITED
                np->writeInt32(response->getPToken() & 0xFFFFFFFF);
                np->writeInt32(response->getError() & 0xFFFFFFFF);

                sp<RfxMessage> newResponse = RfxMessage::obtainResponse(
                        response->getSlotId(),
                        mOriginalDialMessage->getId(),
                        response->getPToken(),
                        response->getId(),
                        response->getToken(),
                        response->getError(),
                        response->getSource(),
                        np,
                        response->getPTimeStamp(),
                        response->getRilToken());
                responseToRilj(newResponse);
                mOriginalDialMessage = NULL;
            } else {
                responseToRilj(response);
            }
           break;

        default:
            responseToRilj(rilResponse);
            break;
    }
    return true;
}

bool RpCallController::onHandleUrc(const sp<RfxMessage>& message) {
    int msg_id = message->getId();
    int ecbmSlotId = RFX_SLOT_ID_UNKNOWN;
    RLOGD("handle urc %d %s (slot %d)", msg_id, requestToString(msg_id),
            getSlotId());

    switch (msg_id) {
        case RIL_UNSOL_INCOMING_CALL_INDICATION:
            onHandleIncomingCallIndication(message);
            break;

        case RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED:
        case RIL_UNSOL_SRVCC_STATE_NOTIFY:
            // RpRedialHandler may block URC if needed
            if (mRedialHandler != NULL) {
                RLOGD("forward urc %s (slot %d) to RedialHandler",
                        requestToString(msg_id), getSlotId());

                if (mRedialHandler->notifyRilUrc(message) == false) {
                    break;
                }
            }
            responseToRilj(message);
            break;

        // For IMS call only : No ECPI URC to RILJ for CS call
        // Native RILD gurantee RING is received before ECPI URC to be sent to RILJ
        // If ECPI before RING, ECPI and then RING URC is sent to RILJ at the same time
        // If ECPI after RING, ECPI is sent to RILJ first, no RING is sent again from modem (IMS)
        case RIL_UNSOL_CALL_INFO_INDICATION:
            handleAutoAnswer(message);
            // Fall through
        case RIL_UNSOL_ENTER_EMERGENCY_CALLBACK_MODE:
            // RpRedialHandler may block URC if needed
            if (msg_id == RIL_UNSOL_ENTER_EMERGENCY_CALLBACK_MODE
                    && mCsPhone == RADIO_TECH_GROUP_C2K) {
                getStatusManager(RFX_SLOT_ID_UNKNOWN)->setIntValue(
                        RFX_STATUS_KEY_IN_CDMA_ECBM_SLOTID, getSlotId());
            }
            if (mRedialHandler != NULL) {
                RLOGD("forward urc %s (slot %d) to RedialHandler",
                        requestToString(msg_id), getSlotId());

                if (mRedialHandler->notifyRilUrc(message) == false) {
                    break;
                }
            }
            responseToRilj(message);
            break;

        case RIL_UNSOL_EXIT_EMERGENCY_CALLBACK_MODE:
            ecbmSlotId = getStatusManager(RFX_SLOT_ID_UNKNOWN)->getIntValue(
                    RFX_STATUS_KEY_IN_CDMA_ECBM_SLOTID, RFX_SLOT_ID_UNKNOWN);
            getStatusManager(RFX_SLOT_ID_UNKNOWN)->setIntValue(
                    RFX_STATUS_KEY_IN_CDMA_ECBM_SLOTID, RFX_SLOT_ID_UNKNOWN);
            RLOGD("ecbmSlotId:%d, currentSlotId:%d", ecbmSlotId, getSlotId());
            if (ecbmSlotId != RFX_SLOT_ID_UNKNOWN && ecbmSlotId != getSlotId()) {
                sp<RfxMessage> tmpUrc = RfxMessage::obtainUrc(ecbmSlotId,
                        RIL_UNSOL_EXIT_EMERGENCY_CALLBACK_MODE);
                responseToRilj(tmpUrc);
            } else {
                responseToRilj(message);
            }
            break;

        case RIL_UNSOL_CDMA_CALL_WAITING:
            onNotifyCdmaCallWait();
            responseToRilj(message);
            break;

        // For CS call only : IMS relies on ECPI URC
        // If ECPI after RING, CALL_STATE_CHANGED is sent to RILJ first,
        //                 RING is sent after next modem report (4 second periodically) (CS)
        // RING URC for CS is RING, for IMS is +CRING: VOICE
        case RIL_UNSOL_CALL_RING:
            handleAutoAnswer(message);
            break;

        default:
            responseToRilj(message);
            break;
    }
    return true;
}

// Support 1R(GSM, C2K, IMS) only
void RpCallController::handleAutoAnswer(const sp<RfxMessage>& msg) {
    // Answer MT call when Auto Answer is enabled in Engineer Mode
    // Only need to support answering a single incoming call
    char prop[PROPERTY_VALUE_MAX] = {0};
    property_get("persist.vendor.em.hidl.auto_answer", prop, "");

    if (strncmp(prop, "1", 1) == 0) {
        logD(RFX_LOG_TAG, "handleAutoAnswer, persist.vendor.em.hidl.auto_answer: %s, msg: %d",
                prop, msg->getId());
        // For IMS
        if (msg->getId() == RIL_UNSOL_CALL_INFO_INDICATION) {
            Parcel* p = msg->getParcel();
            size_t dataPos = p->dataPosition();
            p->readInt32();
            p->readString16();
            String8 str(p->readString16());
            p->setDataPosition(dataPos);

            int msgType = atoi(str);
            logD(RFX_LOG_TAG, "handleAutoAnswer, msgType %d", msgType);
            if (msgType == 0) {
                if (mAutoAnswerTimerHandle != NULL) {
                    RfxTimer::stop(mAutoAnswerTimerHandle);
                    mAutoAnswerTimerHandle = NULL;
                }

                mAutoAnswerTimerHandle = RfxTimer::start(RfxCallback0(this,
                        &RpCallController::onAutoAnswerTimer), ms2ns(AUTO_ANSWER_DELAY));
            } else if (msgType == 132 || msgType == 133) {
                // Stop auto answer if already connected/disconnected
                RfxTimer::stop(mAutoAnswerTimerHandle);
                mAutoAnswerTimerHandle = NULL;
            }

            if (msgType == 0 || msgType == 130) {
                mCallCountForAutoAnswer++;
            } else if (msgType == 133) {
                mCallCountForAutoAnswer--;
            }
        // For CS 1st RING
        } else if (msg->getId() == RIL_UNSOL_CALL_RING && mAutoAnswerTimerHandle == NULL) {
            mAutoAnswerTimerHandle = RfxTimer::start(RfxCallback0(this,
                    &RpCallController::onAutoAnswerTimer), ms2ns(AUTO_ANSWER_DELAY));
        }
    }
}

void RpCallController::onAutoAnswerTimer() {
    logD(RFX_LOG_TAG, "onAutoAnswerTimer");
    if (mCsCallCount > 1 || mCallCountForAutoAnswer > 1) {
        logD(RFX_LOG_TAG, "Ignore second call");
        return;
    }

    sp<RfxMessage> rilRequest = RfxMessage::obtainRequest(getSlotId(),
            mCsPhone == RADIO_TECH_GROUP_GSM ? RADIO_TECH_GROUP_GSM : RADIO_TECH_GROUP_C2K,
            RIL_REQUEST_ANSWER);
    requestToRild(rilRequest);
    mAutoAnswerTimerHandle = NULL;
}

void RpCallController::onHandleGetCurrentCalls(const sp<RfxMessage>& response) {
    bool reportToRilj = true;
    sp<RfxMessage> tmpMsg = RfxMessage::obtainResponse(response->getError(), response, true);
    tmpMsg->getParcel()->setDataPosition(3 * sizeof(int));
    int csCallCount = tmpMsg->getParcel()->readInt32();
    //RLOGD("onHandleGetCurrentCalls, cs callCount: %d->%d", mCsCallCount, csCallCount);
    logD(RFX_LOG_TAG, "onHandleGetCurrentCalls, cs callCount: %d->%d", mCsCallCount, csCallCount);

    mCsCallCount = csCallCount;
    int callCount = mPsCallCount + mCsCallCount;

    RpCallCenterController *centerController =
            (RpCallCenterController *)findController(RFX_SLOT_ID_UNKNOWN,
            RFX_OBJ_CLASS_INFO(RpCallCenterController));
    if (mCallCount > 0 && callCount == 0) {
        if (SIM_COUNT < 2) {
            RLOGD("Skip for SS project");
        } else if (centerController != NULL) {
            if (centerController->isInDsdaMode()) {
                if (mCsPhone == RADIO_TECH_GROUP_GSM) {
                    centerController->handleSwitchHPF(false);
                    if (mSwitchAntennaSupported) {
                        centerController->handleSwitchAntennaRequest(STATE_DISCONNECTED,
                                RAT_MODE_GSM);
                    }
                } else if (mCsPhone == RADIO_TECH_GROUP_C2K) {
                    if (mSwitchAntennaSupported) {
                        centerController->handleSwitchAntennaRequest(STATE_DISCONNECTED,
                                RAT_MODE_C2K);
                    }
                }
            }
        } else {
            RLOGE("Can't find RpCallCenterController!");
        }
    } else if (mCallCount == 0 && callCount > 0) {
        if (SIM_COUNT < 2) {
            RLOGD("Skip for SS project");
        } else if (centerController != NULL) {
            if (centerController->isInDsdaMode()) {
                if (mCsPhone == RADIO_TECH_GROUP_GSM) {
                    centerController->handleSwitchHPF(true);
                }
            }
            if ((mSwitchAntennaSupported || !centerController->isInDsdaMode())
                    && mCsPhone == RADIO_TECH_GROUP_C2K) {
                int value = -1;
                bool isMT = false;
                tmpMsg->getParcel()->setDataPosition(8 * sizeof(int));
                value = tmpMsg->getParcel()->readInt32();
                RLOGD("onHandleGetCurrentCalls, value:%d", value);
                isMT = value != 0;
                if (isMT) {
                    int callId = -1;
                    int seqNumber = -1;
                    tmpMsg->getParcel()->setDataPosition(3 * sizeof(int));
                    String8 str = String8(tmpMsg->getParcel()->readString16());  // callId
                    callId = atoi(str.string());
                    tmpMsg->getParcel()->readString16();  // number
                    tmpMsg->getParcel()->readString16();  // toa
                    tmpMsg->getParcel()->readString16();  // callMode
                    str = String8(tmpMsg->getParcel()->readString16());  // seqNumber
                    seqNumber = atoi(str.string());

                    reportToRilj = centerController->handleIncomingCallIndication(
                            RADIO_TECH_GROUP_C2K,
                            callId, seqNumber, tmpMsg->getSlotId());
                    if (!reportToRilj) {
                        sp<RfxMessage> tmp = RfxMessage::obtainResponse(
                                response->getError(), response);
                        tmp->getParcel()->writeInt32(0);
                        responseToRilj(tmp);
                        return;
                    }
                }
            }
        } else {
            RLOGE("Can't find RpCallCenterController!");
        }
    }

    if (mCallCount > 0 && callCount == 0) {
        // For Non-SVLTE project, always set default modem as MD1,
        // IMS call cannot tolerate audio path switch delay (SIP invite msg takes < 300ms)
        if (!RpFeatureOptionUtils::isSvlteSupport()) {
            mAudioControlHandler->updateAudioPathSync(RADIO_TECH_GROUP_GSM, NULL);
        }
    }

    // For C2K, update KEY_VOICE_CALL_COUNT via CLCC polling
    if (mCallCount != callCount) {
        mCallCount = callCount;
        getStatusManager(getSlotId())->setIntValue(RFX_STATUS_KEY_VOICE_CALL_COUNT, mCallCount);
    }

    /* Cache precise call state, previously for MDMI, currently for general purpose */
    onNotifyPreciseCallState(tmpMsg);

    if (reportToRilj) {
        responseToRilj(response);
    }
}

void RpCallController::onHandleIncomingCallIndication(const sp<RfxMessage>& message) {
    bool reportToRilj = true;
    RpCallCenterController *centerController =
            (RpCallCenterController *)findController(RFX_SLOT_ID_UNKNOWN,
            RFX_OBJ_CLASS_INFO(RpCallCenterController));
    if (SIM_COUNT < 2) {
        RLOGD("Skip for SS project");
    } else if (centerController != NULL) {
        if (mSwitchAntennaSupported || !centerController->isInDsdaMode()) {
            sp<RfxMessage> tmpMsg = RfxMessage::obtainUrc(getSlotId(), message->getId(),
                    message, true);

            int callId = -1;
            int seqNumber = -1;
            tmpMsg->getParcel()->setDataPosition(3 * sizeof(int));
            String8 str = String8(tmpMsg->getParcel()->readString16());  // callId
            callId = atoi(str.string());
            tmpMsg->getParcel()->readString16();  // number
            tmpMsg->getParcel()->readString16();  // toa
            tmpMsg->getParcel()->readString16();  // callMode
            str = String8(tmpMsg->getParcel()->readString16());  // seqNumber
            seqNumber = atoi(str.string());

            reportToRilj = centerController->handleIncomingCallIndication(
                    RADIO_TECH_GROUP_GSM,
                    callId, seqNumber, tmpMsg->getSlotId());
        }
    } else {
        RLOGE("Can't find RpCallCenterController!");
    }

    /* Terminal based call waiting handling */
    if (reportToRilj && (centerController != NULL)) {
        sp<RfxMessage> tmpMsg = RfxMessage::obtainUrc(getSlotId(), message->getId(),
                message, true);

        int callId = -1;
        int seqNumber = -1;
        int callMode = -1;
        tmpMsg->getParcel()->setDataPosition(3 * sizeof(int));
        String8 str = String8(tmpMsg->getParcel()->readString16());  // callId
        callId = atoi(str.string());
        tmpMsg->getParcel()->readString16();  // number
        tmpMsg->getParcel()->readString16();  // toa
        str = String8(tmpMsg->getParcel()->readString16());  // callMode
        callMode = atoi(str.string());
        str = String8(tmpMsg->getParcel()->readString16());  // seqNumber
        seqNumber = atoi(str.string());

        reportToRilj = centerController->handleTerminalBasedCallWaiting(
                callId, seqNumber, getSlotId());
        // Allow internally only with CS call when TBCW is false
        if (reportToRilj && !(callMode >= MIN_IMS_CALL_MODE && callMode <= MAX_IMS_CALL_MODE)) {
            sp<RfxMessage> rilRequest = RfxMessage::obtainRequest(getSlotId(),
                    RADIO_TECH_GROUP_GSM, RIL_REQUEST_SET_CALL_INDICATION);
            rilRequest->getParcel()->writeInt32(3);
            rilRequest->getParcel()->writeInt32(0);  // allow
            rilRequest->getParcel()->writeInt32(callId);
            rilRequest->getParcel()->writeInt32(seqNumber);
            requestToRild(rilRequest);
            reportToRilj = false;
        }
    }

    if (reportToRilj) {
        responseToRilj(message);
    }
}

bool RpCallController::hasPendingHangupRequest(bool isForegnd) {
    bool ret = false;
    int origSize = mPreciseCallStateList->size();
    for (int i = 0; i < origSize; i++) {
        RfxPreciseCallState* item = mPreciseCallStateList->itemAt(i);
        RLOGD("hasPendingHangupRequest, isForegnd:%d, callId=%d, state=%d",
                (isForegnd ? 1 : 0), item->mCallId, (int)item->mOrigState);
        if ((isForegnd && item->mOrigState == ORIG_FOREGND_DISCONNECTING) ||
                (!isForegnd && item->mOrigState == ORIG_BACKGND_DISCONNECTING)) {
            sp<RfxMessage> rilRequest = RfxMessage::obtainRequest(getSlotId(),
                    RADIO_TECH_GROUP_GSM, RIL_REQUEST_FORCE_RELEASE_CALL);
            rilRequest->getParcel()->writeInt32(1);
            rilRequest->getParcel()->writeInt32(item->mCallId);
            requestToRild(rilRequest);
            ret = true;
        }
    }
    return ret;
}

bool RpCallController::hasPendingHangupRequest(int hangupCallId) {
    int origSize = mPreciseCallStateList->size();
    for (int i = 0; i < origSize; i++) {
        RfxPreciseCallState* item = mPreciseCallStateList->itemAt(i);
        RLOGD("hasPendingHangupRequest (find %d), callId=%d, state=%d",
                hangupCallId, item->mCallId, (int)item->mOrigState);
        if (item->mCallId == hangupCallId && item->mOrigState == ORIG_DISCONNECTING) {
            sp<RfxMessage> rilRequest = RfxMessage::obtainRequest(getSlotId(),
                    RADIO_TECH_GROUP_GSM, RIL_REQUEST_FORCE_RELEASE_CALL);
            rilRequest->getParcel()->writeInt32(1);
            rilRequest->getParcel()->writeInt32(item->mCallId);
            requestToRild(rilRequest);
            return true;
        }
    }
    return false;
}

void RpCallController::handleEmergencyDialRequest(const sp<RfxMessage>& request) {
    Parcel* parcel = request->getParcel();

    mEccNumber = parcel->readString16();  // address

    RLOGD("handleEmergencyDialRequest, eccNumber = %s", String8(mEccNumber).string());
}

void RpCallController::handleCdmaFlashRequest(const sp<RfxMessage>& request) {
    Parcel* parcel = request->getParcel();
    String16 address = parcel->readString16();  // address
    if (address != String16("")) {
        RfxPreciseCallState* preciseCallState = new RfxPreciseCallState();

        preciseCallState->mSlot = getSlotId();
        preciseCallState->mCallId = mPreciseCallStateList->size() + 1;
        preciseCallState->mOrigState = ORIG_ACTIVE;
        preciseCallState->mCallType = CALL_TYPE_VOICE;
        preciseCallState->mCallRat = mCallRat;
        preciseCallState->mCallDir = CALL_DIR_MO;

        updatePreciseCallStateList(preciseCallState, mPreciseCallStateList);
    }
    RLOGD("handleCdmaFlashRequest");
}

void RpCallController::onNotifyCdmaCallWait() {
    RLOGD("onNotifyCdmaCallWait");

    RfxPreciseCallState* preciseCallState = new RfxPreciseCallState();

    preciseCallState->mSlot = getSlotId();
    preciseCallState->mCallId = mPreciseCallStateList->size() + 1;
    preciseCallState->mOrigState = ORIG_WAITING;
    preciseCallState->mCallType = CALL_TYPE_VOICE;
    preciseCallState->mCallRat = mCallRat;
    preciseCallState->mCallDir = CALL_DIR_MT;

    updatePreciseCallStateList(preciseCallState, mPreciseCallStateList);
}

void RpCallController::onNotifyPreciseCallState(const sp<RfxMessage>& message) {
    //RLOGD("onNotifyPreciseCallState");
    Vector<RfxPreciseCallState*>* currentList = parsePreciseCallState(message->getParcel());
    updateDisconnected(mPreciseCallStateList, currentList);
    freePreciseCallStateList(mPreciseCallStateList);
    mPreciseCallStateList = currentList;
}

Vector<RfxPreciseCallState*>* RpCallController::parsePreciseCallState(Parcel* parcel) {
    parcel->setDataPosition(3 * sizeof(int32_t));
    int count = parcel->readInt32();
    RLOGD("parsePreciseCallState, count: %d", count);
    Vector<RfxPreciseCallState*>* list = new Vector<RfxPreciseCallState*>();

    int id, status, type, dir;
    String16 callNumber;

    for (int i = 0; i < count; i++) {
        RfxPreciseCallState* preciseCallState = new RfxPreciseCallState();
        status = parcel->readInt32();  // state
        id = parcel->readInt32();  // index
        parcel->readInt32();  // toa
        parcel->readInt32();  // isMpty
        dir = parcel->readInt32();  // isMT
        parcel->readInt32();  // als
        type = parcel->readInt32();  // isVoice
        parcel->readInt32();  // isVoicePrivacy
        callNumber = parcel->readString16();  // number
        parcel->readInt32();  // numberPresentation
        parcel->readString16();  // name
        parcel->readInt32();  // namePresentation

        int uusInfoAvail = parcel->readInt32(); // check UUS Info exist
        if (uusInfoAvail == 1) {
            parcel->readInt32(); // uusType
            parcel->readInt32(); // uusDcs
            int uusInfoLen = parcel->readInt32(); // uusLength
            parcel->readInplace(uusInfoLen); // uusData
        }

        parcel->readInt32();  // speechCodec

        preciseCallState->mSlot = getSlotId();
        preciseCallState->mCallId = id;
        preciseCallState->mOrigState = RfxPreciseCallState::RILStateToOrigState(
                (RIL_CallState)status);

        if (type == 1) {
            if (callNumber == mEccNumber) {
                preciseCallState->mCallType = CALL_TYPE_EMERGENCY;
            } else {
                preciseCallState->mCallType = CALL_TYPE_VOICE;
            }
        } else {
            preciseCallState->mCallType = CALL_TYPE_VIDEO;
        }

        preciseCallState->mCallRat = mCallRat;
        preciseCallState->mCallDir = (CallDirection)dir;

        updatePreciseCallStateList(preciseCallState, list);
    }
    return list;
}

void RpCallController::updateDisconnecting(
        Vector<RfxPreciseCallState*>* origList, int hangupCallId) {
    int origSize = origList->size();
    for (int i = 0; i < origSize; i++) {
        if (origList->itemAt(i)->mCallId == hangupCallId) {
            origList->itemAt(i)->mOrigState = ORIG_DISCONNECTING;
            break;
        }
    }
}

void RpCallController::updateDisconnecting(
        Vector<RfxPreciseCallState*>* origList, bool isForegnd) {
    int origSize = origList->size();
    for (int i = 0; i < origSize; i++) {
        if (isForegnd) {
            if (origList->itemAt(i)->mOrigState == ORIG_ACTIVE) {
                origList->itemAt(i)->mOrigState = ORIG_FOREGND_DISCONNECTING;
            }
        } else {
            if (origList->itemAt(i)->mOrigState == ORIG_HOLDING ||
                    origList->itemAt(i)->mOrigState == ORIG_WAITING) {
                origList->itemAt(i)->mOrigState = ORIG_BACKGND_DISCONNECTING;
            }
        }
    }
}

void RpCallController::updateDisconnected(
        Vector<RfxPreciseCallState*>* oldList, Vector<RfxPreciseCallState*>* newList) {
    int oldSize = oldList->size();
    int newSize = newList->size();
    for (int i = 0; i < oldSize; i++) {
        bool disconnected = true;
        for (int j = 0; j < newSize; j++) {
            if (oldList->itemAt(i)->mCallId == newList->itemAt(j)->mCallId) {
                disconnected = false;
                break;
            }
        }
        if (disconnected) {
            RfxPreciseCallState* preciseCallState = oldList->itemAt(i);
            preciseCallState->mOrigState = ORIG_DISCONNECTED;
            updatePreciseCallStateList(preciseCallState, newList);
            if (preciseCallState->mCallType == CALL_TYPE_EMERGENCY) {
                mEccNumber = String16("");
            }
        }
    }
}

void RpCallController::updatePreciseCallStateList(
        RfxPreciseCallState* preciseCallState, Vector<RfxPreciseCallState*>* list) {
    preciseCallState->dump();
    if (preciseCallState->mOrigState != ORIG_DISCONNECTED) {
        list->add(preciseCallState);
    }
}

void RpCallController::freePreciseCallStateList(Vector<RfxPreciseCallState*>* list) {
    if (list != NULL) {
        int size = list->size();
        for (int i = 0; i < size; i++) {
            delete list->itemAt(i);
        }
        delete list;
    }
}

int RpCallController::getRilRequestSendDomain(const sp<RfxMessage>& request) {
    int msgId = request->getId();
    int dispatchDomain;

    switch (msgId) {
        // Exit ECBM request => Send to MD1 or MD3 according to ECBM.
        case RIL_REQUEST_EXIT_EMERGENCY_CALLBACK_MODE:
            if (E911_ECBM_ENTER_GSM_LTE == getStatusManager()->
                    getIntValue(RFX_STATUS_KEY_EMERGENCY_CALLBACK_MODE, E911_ECBM_NONE)) {
                RLOGD("Get RIL Request domain: MD3(ECBM: GSM/LTE)");
                dispatchDomain = RADIO_TECH_GROUP_GSM;
            }
            else {
                RLOGD("Get RIL Request domain: MD1(ECBM: 1xRTT)");
                dispatchDomain = RADIO_TECH_GROUP_C2K;
            }
            break;

        // IMS related request => Always send to MD1
        case RIL_REQUEST_SET_ECC_MODE:
        case RIL_LOCAL_REQUEST_SET_HVOLTE_MODE:
        case RIL_LOCAL_REQUEST_EMERGENCY_SESSION_BEGIN:
        case RIL_LOCAL_REQUEST_EMERGENCY_SESSION_END:
            RLOGD("Get RIL Request domain: MD1(MD1 only request)");
            dispatchDomain = RADIO_TECH_GROUP_GSM;
            break;

        // Emergency dial request => Always send to MD1 for Verizon project.
        case RIL_LOCAL_REQUEST_EMERGENCY_DIAL:
        case RIL_LOCAL_REQUEST_IMS_EMERGENCY_DIAL:
            if ((mRedialHandler != NULL) &&
                ((mRedialHandler->checkRedialFeatureEnabled(
                         REDIAL_SBP_ID_VZW, REDIAL_TC_ID_COMMON) == true) ||
                 (mRedialHandler->checkRedialFeatureEnabled(
                         REDIAL_SBP_ID_SPRINT, REDIAL_TC_ID_COMMON) == true))
                ) {
                RLOGD("Get RIL Request domain: MD1(hVoLTE enabled)");
                dispatchDomain = RADIO_TECH_GROUP_GSM;
                break;
            }
            TELEPHONYWARE_FALLTHROUGH;
            // else: fall through
        default:
            if (RADIO_TECH_GROUP_GSM == getCommonRilRequestSendDomain(request)) {
                dispatchDomain = RADIO_TECH_GROUP_GSM;
            } else {
                dispatchDomain = RADIO_TECH_GROUP_C2K;
            }
            break;
    }
    return dispatchDomain;
}

int RpCallController::getCommonRilRequestSendDomain(const sp<RfxMessage>& request) {
    int msgId = request->getId();

    // Step 1: Check redial state
    if (mRedialHandler != NULL) {
        switch (mRedialHandler->getRedialState()) {
            case REDIAL_TO_MD3:
                RLOGD("Get RIL Request domain: MD3(redial state: MD3)");
                return RADIO_TECH_GROUP_C2K;

            case REDIAL_TO_MD1:
            case REDIAL_CHANGE_GLOBAL_MODE:
                RLOGD("Get RIL Request domain: MD1(redial state: MD1/change global mode)");
                return RADIO_TECH_GROUP_GSM;

            default:
                break;
        }
    }

    // Step 2: Check IMS DIAL RIL REQUEST
    if (msgId == RIL_REQUEST_IMS_DIAL || msgId == RIL_LOCAL_REQUEST_IMS_EMERGENCY_DIAL) {
        RLOGD("Get RIL Request domain: MD1(request ID: %d)", msgId);
        return RADIO_TECH_GROUP_GSM;
    }

    // Step 3: Check phone type
    if (NWS_MODE_CDMALTE == getStatusManager()->
            getIntValue(RFX_STATUS_KEY_NWS_MODE, NWS_MODE_CSFB)) {
        RLOGD("Get RIL Request domain: MD3(device mode: CDMALTE)");
        return RADIO_TECH_GROUP_C2K;
    } else {
        RLOGD("Get RIL Request domain MD1(device mode: CSFB/LTEONLY)");
        return RADIO_TECH_GROUP_GSM;
    }
}


// The API will help to allocate memory so remember to free
// it while you don't use the buffer anymore
char* RpCallController::strdupReadString(Parcel *p) {
    size_t stringlen;
    const char16_t *s16;

    s16 = p->readString16Inplace(&stringlen);

    return strndup16to8(s16, stringlen);
}


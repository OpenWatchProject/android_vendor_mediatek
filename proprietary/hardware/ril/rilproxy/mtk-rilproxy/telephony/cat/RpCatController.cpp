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
#include "nw/RpNwRatController.h"
#include "RpCatController.h"
#include <cutils/jstring.h>
#include "util/RpFeatureOptionUtils.h"
#include "modecontroller/RpModeInitController.h"

#define RFX_LOG_TAG "RpCatController"

/*****************************************************************************
 * Class RfxController
 *****************************************************************************/

RFX_IMPLEMENT_CLASS("RpCatController", RpCatController, RfxController);

RpCatController::RpCatController():
mCardType(-1),
mcdmaCardType(-1),
mSetupMenuSource(INVALID_VALUE),
mSetupEventSource(INVALID_VALUE),
mOpenChannelSource(INVALID_VALUE),
// / M: OTASP {
mCsimStatus(INVALID_VALUE),
mOtaspActivationStatus(INVALID_VALUE),
mDataRadioTech(INVALID_VALUE),
mFailureCount(0),
mIsOpenChannelCommandQueued(false),
mPendingOpenChannelRequest(NULL),
mVoiceCallCount(0),
mSupportBipOtaspActivation(0) {
// / M: OTASP }
}

RpCatController::~RpCatController() {
}

void RpCatController::onInit() {
    RfxController::onInit();  // Required: invoke super class implementation
    logD(RFX_LOG_TAG,"onInit()");

    mProCmdRec.cmdType = 0;
    mProCmdRec.source = 0;

    const int request_id_list[] = {
        RIL_REQUEST_STK_GET_PROFILE,
        RIL_REQUEST_STK_SET_PROFILE,
        RIL_REQUEST_STK_SEND_ENVELOPE_COMMAND,
        RIL_REQUEST_STK_SEND_TERMINAL_RESPONSE,
        RIL_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM,
        RIL_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM_WITH_RESULT_CODE,
        RIL_REQUEST_REPORT_STK_SERVICE_IS_RUNNING,
        RIL_REQUEST_STK_SEND_ENVELOPE_WITH_STATUS,
        RIL_LOCAL_C2K_REQUEST_SET_UTK_MODE,
    };

    const int urc_id_list[] = {
        RIL_UNSOL_STK_SESSION_END,
        RIL_UNSOL_STK_PROACTIVE_COMMAND,
        RIL_UNSOL_STK_EVENT_NOTIFY,
        RIL_UNSOL_STK_CALL_SETUP,
        RIL_UNSOL_STK_CC_ALPHA_NOTIFY,
        RIL_UNSOL_STK_SETUP_MENU_RESET,
        RIL_UNSOL_STK_BIP_PROACTIVE_COMMAND,
    };

    // register request & URC id list
    // NOTE. one id can only be registered by one controller
    if (RpFeatureOptionUtils::isC2kSupport()) {
        registerToHandleRequest(request_id_list, sizeof(request_id_list)/sizeof(const int));
        registerToHandleUrc(urc_id_list, sizeof(urc_id_list)/sizeof(const int));
    }
    // Register callbacks for card type
    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_CARD_TYPE,
        RfxStatusChangeCallback(this, &RpCatController::onCardTypeChanged));

    // Callbacks for mode changed done
    RpModeInitController *modeController =
                        (RpModeInitController *)findController(
                        RFX_OBJ_CLASS_INFO(RpModeInitController));

    modeController->mSwitchCdmaSlotFinishSignal.connect(this,
                                   &RpCatController::onModeChanged);

    // / M: OTASP {
    // Register callbacks for roaming mode change
    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_RAT_SWITCH_DONE,
        RfxStatusChangeCallback(this, &RpCatController::onRatSwitchDone));

    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_UICC_SUB_CHANGED_STATUS,
        RfxStatusChangeCallback(this, &RpCatController::onCsimStateChanged));

    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_VOICE_CALL_COUNT,
        RfxStatusChangeCallback(this, &RpCatController::onVoiceCallCountChanged));

    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_OTA_STATUS,
        RfxStatusChangeCallback(this, &RpCatController::onOtaspActivationStatusChanged));

    for (int i = 0; i < RFX_SLOT_COUNT; i++) {
        getStatusManager(i)->registerStatusChangedEx(
                RFX_STATUS_KEY_SERVICE_STATE,
                RfxStatusChangeCallbackEx(this,
                        &RpCatController::onServiceStateChanged));
    }
    // / M: OTASP }
}

void RpCatController::onCardTypeChanged(RfxStatusKeyEnum key,
    RfxVariant oldValue, RfxVariant newValue) {
    RFX_UNUSED(key);

    // / M: OTASP {
    if (isVzWSupport()) {
        mCardType = getStatusManager()->getIntValue(RFX_STATUS_KEY_CARD_TYPE);
        if (mCardType & RFX_CARD_TYPE_CSIM || mCardType & RFX_CARD_TYPE_RUIM) {
            mSupportBipOtaspActivation = 1;
        } else {
            mSupportBipOtaspActivation = 0;
        }
        logD(RFX_LOG_TAG, "set mSupportBipOtaspActivation=%d",
                mSupportBipOtaspActivation);
    } else {
        mSupportBipOtaspActivation = 0;
    }
    // / M: OTASP }
}

void RpCatController::onModeChanged() {
    int activeCDMALTESlot = getNonSlotScopeStatusManager()->getIntValue(
                            RFX_STATUS_KEY_ACTIVE_CDMALTE_MODE_SLOT, -1);
    int cardType = getStatusManager()->getIntValue(
                            RFX_STATUS_KEY_CARD_TYPE, -1);
    logD(RFX_LOG_TAG, "onModeChanged active CDMALTE slot %d, cardtype %d",
            activeCDMALTESlot, cardType);
    if (cardType > 0) {
        if (getSlotId() != activeCDMALTESlot) {
            // need send AT+EUTK = 0 to MD1
            setSTKUTKMode(0);
        } else  {
            // need send AT+EUTK = 1 to MD1
            setSTKUTKMode(1);
            // need send AT+EUTK = 1 to MD3
            configUtkMode(1);
        }
    }
}

// / M: OTASP {
void RpCatController::onRatSwitchDone(RfxStatusKeyEnum key,
    RfxVariant oldValue, RfxVariant newValue) {
    RFX_UNUSED(key);
}

void RpCatController::onCsimStateChanged(RfxStatusKeyEnum key,
    RfxVariant oldValue, RfxVariant newValue) {
    RFX_UNUSED(key);

    mCsimStatus = getStatusManager()->getIntValue(RFX_STATUS_KEY_UICC_SUB_CHANGED_STATUS, -1);
    if (mCsimStatus == -1) {
        return;
    }

    if (mIsOpenChannelCommandQueued) {
        logD(RFX_LOG_TAG, "onCsimStateChanged(): mIsOpenChannelCommandQueued");
        mIsOpenChannelCommandQueued = false;
        if (checkIfNeedToTriggerOtasp()) {
            handleOtaspRequest(mPendingOpenChannelRequest);
        } else {
            responseToRilj(mPendingOpenChannelRequest);
        }
        mPendingOpenChannelRequest = NULL;
    }

    RetryOtaspIfNeeded();
}

void RpCatController::onVoiceCallCountChanged(RfxStatusKeyEnum key,
    RfxVariant oldValue, RfxVariant newValue) {
    RFX_UNUSED(key);

    mVoiceCallCount = getStatusManager()->getIntValue(RFX_STATUS_KEY_VOICE_CALL_COUNT, 0);
    RFX_ASSERT(mVoiceCallCount >= 0 && mVoiceCallCount <= 10);

    RetryOtaspIfNeeded();
}

void RpCatController::onOtaspActivationStatusChanged(RfxStatusKeyEnum key, RfxVariant oldValue,
            RfxVariant newValue) {
    RFX_UNUSED(key);

    logD(RFX_LOG_TAG, "onOtaspActivationStatusChanged mOtaspActivationStatus = %d",
            mOtaspActivationStatus);

    int oldStatus = mOtaspActivationStatus;
    int newStatus = getStatusManager()->getIntValue(RFX_STATUS_KEY_OTA_STATUS, 0);
    if (newStatus == OTASP_PROGRAMMING_STARTED || newStatus == OTASP_PROGRAMMING_SUCCESSFULLY
            || newStatus == OTASP_PROGRAMMING_UNSUCCESSFULLY) {
        mOtaspActivationStatus = newStatus;
    }

    if (oldStatus == OTASP_PROGRAMMING_STARTED) {
        if (mOtaspActivationStatus == OTASP_PROGRAMMING_SUCCESSFULLY) {
        } else if (mOtaspActivationStatus == OTASP_PROGRAMMING_UNSUCCESSFULLY) {
            mFailureCount++;
            RetryOtaspIfNeeded();
        }
    }
}

void RpCatController::onServiceStateChanged(int slotId, RfxStatusKeyEnum key, RfxVariant oldValue,
        RfxVariant newValue) {
    RFX_UNUSED(oldValue);
    RFX_ASSERT(key == RFX_STATUS_KEY_SERVICE_STATE);

    int cdmaSlotId =
            getNonSlotScopeStatusManager()->getIntValue(RFX_STATUS_KEY_CDMA_SOCKET_SLOT, 0);
    RfxNwServiceState newSS = newValue.asServiceState();

    if (slotId == cdmaSlotId) {
        int dataRegState = newSS.getRilDataRegState();
        if (RfxNwServiceState::isInService(dataRegState)) {
            int dataRadioTech = newSS.getRilDataRadioTech();
            logD(RFX_LOG_TAG, "cSlot=%d, SIM=%d, newSS: %s, dataTech=%d",
                    cdmaSlotId, slotId, newSS.toString().string(), dataRadioTech);
            mDataRadioTech = dataRadioTech;
        } else {
            logD(RFX_LOG_TAG, "onServiceStateChanged: not in service");
            mDataRadioTech = INVALID_VALUE;
        }
    }

    RetryOtaspIfNeeded();
}
// / M: OTASP }

void RpCatController::onDeinit() {
    logD(RFX_LOG_TAG,"onDeinit()");

    // Unregister callbacks for card type
    getStatusManager()->unRegisterStatusChanged(RFX_STATUS_KEY_CARD_TYPE,
        RfxStatusChangeCallback(this, &RpCatController::onCardTypeChanged));
    // / M: OTASP {
    // Unregister callbacks for roaming mode change
    getStatusManager()->unRegisterStatusChanged(RFX_STATUS_KEY_RAT_SWITCH_DONE,
        RfxStatusChangeCallback(this, &RpCatController::onRatSwitchDone));

    getStatusManager()->unRegisterStatusChanged(RFX_STATUS_KEY_UICC_SUB_CHANGED_STATUS,
        RfxStatusChangeCallback(this, &RpCatController::onCsimStateChanged));

    getStatusManager()->unRegisterStatusChanged(RFX_STATUS_KEY_VOICE_CALL_COUNT,
        RfxStatusChangeCallback(this, &RpCatController::onVoiceCallCountChanged));

    getStatusManager()->unRegisterStatusChanged(RFX_STATUS_KEY_OTA_STATUS,
        RfxStatusChangeCallback(this, &RpCatController::onOtaspActivationStatusChanged));

    for (int i = 0; i < RFX_SLOT_COUNT; i++) {
        getStatusManager(i)->unRegisterStatusChangedEx(
                RFX_STATUS_KEY_SERVICE_STATE,
                RfxStatusChangeCallbackEx(this,
                        &RpCatController::onServiceStateChanged));
    }
    // / M: OTASP }
    // Required: invoke super class implementation
    RfxController::onDeinit();
}

bool RpCatController::onHandleRequest(const sp<RfxMessage>& message) {
    int msgId = message->getId();
    int cmdType = 0;
    int envCmdType = 0;
    int eventType = 0;

    Parcel  *p = message->getParcel();
    size_t dataPos = p->dataPosition();

    logD(RFX_LOG_TAG,"req %d %s", msgId, requestToString(msgId));

    switch (msgId) {
        case RIL_REQUEST_STK_SEND_ENVELOPE_COMMAND:
            parseStkEnvCmdType(message, &envCmdType, &eventType);

            switch (envCmdType) {
                case MENU_SELECTION_TAG:
                    logD(RFX_LOG_TAG,"mSetupMenuSource:%d", mSetupMenuSource);
                    if (RADIO_TECH_GROUP_C2K == mSetupMenuSource) {
                        sp<RfxMessage> c2k_request = RfxMessage::obtainRequest(
                            RADIO_TECH_GROUP_C2K, msgId, message, true);
                        requestToRild(c2k_request);
                    } else {
                        requestToRild(message);
                    }
                    break;

                case EVENT_DOWNLOAD_TAG:
                    switch (eventType) {
                        case EVENT_DATA_AVAILABLE:
                        case EVENT_CHANNEL_STATUS:
                            logD(RFX_LOG_TAG,"eventType:0x%02X mOpenChannelSource:%d",
                                eventType, mOpenChannelSource);
                            if (RADIO_TECH_GROUP_C2K == mOpenChannelSource) {
                                sp<RfxMessage> c2k_request =
                                    RfxMessage::obtainRequest(RADIO_TECH_GROUP_C2K,
                                    msgId, message, true);
                                requestToRild(c2k_request);
                            } else if (RADIO_TECH_GROUP_GSM == mOpenChannelSource){
                                requestToRild(message);
                            } else {
                                logD(RFX_LOG_TAG,"Unexpected DATA_AVAILABLE"
                                "/CHANNEL_STATUS Event. Warning!!!");
                            }
                            break;

                        default:
                            logD(RFX_LOG_TAG,"eventType:0x%02X mSetupEventSource:%d",
                                eventType, mSetupEventSource);
                            if (RADIO_TECH_GROUP_C2K == mSetupEventSource) {
                                sp<RfxMessage> c2k_request =
                                    RfxMessage::obtainRequest(RADIO_TECH_GROUP_C2K,
                                    msgId, message, true);
                                requestToRild(c2k_request);
                            } else {
                                requestToRild(message);
                            }
                            break;
                    }
                    break;

                default:
                    logD(RFX_LOG_TAG,"Unknow Envelope Command Warning!!!");
                    requestToRild(message);
                    break;
            }
            break;

        case RIL_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM:
        case RIL_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM_WITH_RESULT_CODE:
            logD(RFX_LOG_TAG,"mProCmdRec: cmdType:0x%02X source:%d",
                mProCmdRec.cmdType, mProCmdRec.source);

            if (CMD_SETUP_CALL == mProCmdRec.cmdType ||
                CMD_OPEN_CHAN == mProCmdRec.cmdType) {

                // switch audio path for STK/UTK call
                if (RADIO_TECH_GROUP_C2K == mProCmdRec.source &&
                        CMD_SETUP_CALL == mProCmdRec.cmdType) {
                    p->readInt32();  // length
                    int accept = p->readInt32();
                    logD(RFX_LOG_TAG, "accept=%d", accept);
                    if (accept > 0) {
                        RpAudioControlHandler *audioHandler;
                        audioHandler = (RpAudioControlHandler *)findController(
                                getSlotId(), RFX_OBJ_CLASS_INFO(RpAudioControlHandler));
                        audioHandler->updateAudioPathAsync(RADIO_TECH_GROUP_C2K);
                    }
                    p->setDataPosition(dataPos);
                }

                if (RADIO_TECH_GROUP_C2K == mProCmdRec.source) {
                    sp<RfxMessage> c2k_request = RfxMessage::obtainRequest(
                        RADIO_TECH_GROUP_C2K, msgId, message, true);
                    requestToRild(c2k_request);
                } else {
                    requestToRild(message);
                }
            } else {
                logD(RFX_LOG_TAG,"Unexpected CALL_SETUP_REQUESTED. Warning!!!");
                requestToRild(message);
            }

            // reset reset mProCmdRec.cmdType
            mProCmdRec.cmdType = 0;
            break;

        case RIL_REQUEST_STK_SEND_TERMINAL_RESPONSE:
            parseStkCmdType(message, &cmdType);

            logD(RFX_LOG_TAG,"mProCmdRec: cmdType:0x%02X source:%d",
                mProCmdRec.cmdType, mProCmdRec.source);

            if (0 != mProCmdRec.cmdType && mProCmdRec.cmdType == cmdType) {
                if (RADIO_TECH_GROUP_C2K == mProCmdRec.source) {
                    sp<RfxMessage> c2k_request = RfxMessage::obtainRequest(
                        RADIO_TECH_GROUP_C2K, msgId, message, true);
                    requestToRild(c2k_request);
                } else {
                    requestToRild(message);
                }

                // reset mProCmdRec.cmdType
                mProCmdRec.cmdType = 0;
            // If received SET UP MENU before SET UP EVENT LIST, mProCmdRec.cmdType
            // will be coverd.According mSetupMenuSource to send TR.
            } else if (CMD_SETUP_MENU == cmdType) {
                if (RADIO_TECH_GROUP_C2K == mSetupMenuSource) {
                    sp<RfxMessage> c2k_request = RfxMessage::obtainRequest(
                        RADIO_TECH_GROUP_C2K, msgId, message, true);
                    requestToRild(c2k_request);
                } else {
                    requestToRild(message);
                }
                // reset mProCmdRec.cmdType
                mProCmdRec.cmdType = 0;
            } else if (CMD_SETUP_EVENT_LIST== cmdType) {
                if (RADIO_TECH_GROUP_C2K == mSetupEventSource) {
                    sp<RfxMessage> c2k_request = RfxMessage::obtainRequest(
                        RADIO_TECH_GROUP_C2K, msgId, message, true);
                    requestToRild(c2k_request);
                } else {
                    requestToRild(message);
                }
                // reset mProCmdRec.cmdType
                mProCmdRec.cmdType = 0;
            } else {
                logD(RFX_LOG_TAG,"Not Expected TR. Warning!!!");
                requestToRild(message);
            }
            break;

        case RIL_REQUEST_REPORT_STK_SERVICE_IS_RUNNING:
            requestToRild(message);

            mCardType = getStatusManager()->getIntValue(RFX_STATUS_KEY_CARD_TYPE, -1);
            mcdmaCardType = getStatusManager()->getIntValue(
                RFX_STATUS_KEY_CDMA_CARD_TYPE, -1);
            logD(RFX_LOG_TAG,"mCardType:0x%02X mcdmaCardType:%d", mCardType, mcdmaCardType);

            // When c2k-rild receive STK_SERVICE_IS_RUNNING, it will
            // send AT+UTKPD to c2k modem
            if (mCardType & RFX_CARD_TYPE_CSIM || mCardType & RFX_CARD_TYPE_RUIM) {
                sp<RfxMessage> c2k_request = RfxMessage::obtainRequest(
                    RADIO_TECH_GROUP_C2K, msgId, message);
                requestToRild(c2k_request);
            }
            break;
        default:
            requestToRild(message);
            break;
    }

    return true;
}

bool RpCatController::onHandleUrc(const sp<RfxMessage>& message) {
    int msgId = message->getId();
    int sourceId = message->getSource();
    int cmdType = 0;

    logD(RFX_LOG_TAG,"urc %d %s source:%d", msgId,
        urcToString(msgId), sourceId);

    switch (msgId) {
        case RIL_UNSOL_STK_PROACTIVE_COMMAND:
        case RIL_UNSOL_STK_EVENT_NOTIFY:
            parseStkCmdType(message, &cmdType);
            switch (cmdType) {
                case CMD_SETUP_MENU:
                    mSetupMenuSource = sourceId;
                    mProCmdRec.cmdType = cmdType;
                    mProCmdRec.source = sourceId;
                    break;

                case CMD_SETUP_EVENT_LIST:
                    mSetupEventSource = sourceId;
                    mProCmdRec.cmdType = cmdType;
                    mProCmdRec.source = sourceId;
                    break;

                case CMD_SETUP_CALL:
                case CMD_LAUNCH_BROWSER:
                case CMD_PLAY_TONE:
                case CMD_DSPL_TXT:
                case CMD_GET_INKEY:
                case CMD_GET_INPUT:
                case CMD_SELECT_ITEM:
                case CMD_PROVIDE_LOCAL_INFO:
                case CMD_IDLE_MODEL_TXT:
                case CMD_LANGUAGE_NOTIFY:
                // If open channel come with UNSOL_STK_PROACTIVE_COMMAND, it
                // need user confirm. After user confirm, the STK apk will
                // send the result with STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM
                case CMD_CLOSE_CHAN:
                case CMD_RECEIVE_DATA:
                case CMD_SEND_DATA:
                case CMD_GET_CHAN_STATUS:
                    mProCmdRec.cmdType = cmdType;
                    mProCmdRec.source = sourceId;
                    break;
                case CMD_OPEN_CHAN:
                    mProCmdRec.cmdType = cmdType;
                    mProCmdRec.source = sourceId;
                    // / M: OTASP {
                    if (mSupportBipOtaspActivation) {
                        logD(RFX_LOG_TAG, "onHandleUrc(): CMD_OPEN_CHAN");
                        mIsOpenChannelCommandQueued = false;
                        if (checkIfNeedToTriggerOtasp()) {
                            handleOtaspRequest(message);
                            return true;
                        } else if (mIsOpenChannelCommandQueued) {
                            mPendingOpenChannelRequest = message;
                            return true;
                        }
                    }
                    // / M: OTASP }
                    break;
                default:
                    break;
            }

            responseToRilj(message);
            break;

        case RIL_UNSOL_STK_BIP_PROACTIVE_COMMAND:
            parseStkCmdType(message, &cmdType);
            switch (cmdType) {
                // If open channel command with UNSOL_STK_BIP_PROACTIVE_COMMAND, it
                // may not have alpha ID(No need user confirm) or it has been
                // confirmed by user
                case CMD_OPEN_CHAN:
                    mOpenChannelSource = sourceId;
                    mProCmdRec.cmdType = cmdType;
                    mProCmdRec.source = sourceId;
                    // / M: OTASP {
                    if (mSupportBipOtaspActivation) {
                        mIsOpenChannelCommandQueued = false;
                        logD(RFX_LOG_TAG, "onHandleUrc(): CMD_BIP_OPEN_CHAN");
                        if (checkIfNeedToTriggerOtasp()) {
                            handleOtaspRequest(message);
                            return true;
                        } else if (mIsOpenChannelCommandQueued) {
                            mPendingOpenChannelRequest = message;
                            return true;
                        }
                    }
                    // / M: OTASP }
                    break;

                case CMD_CLOSE_CHAN:
                    mOpenChannelSource = INVALID_VALUE;
                    mProCmdRec.cmdType = cmdType;
                    mProCmdRec.source = sourceId;
                    break;

                case CMD_RECEIVE_DATA:
                case CMD_SEND_DATA:
                case CMD_GET_CHAN_STATUS:
                    mProCmdRec.cmdType = cmdType;
                    mProCmdRec.source = sourceId;
                    break;

                default:
                    break;
            }

            responseToRilj(message);
            break;
        default:
            responseToRilj(message);
            break;
    }

    return true;
}

bool RpCatController::onHandleResponse(const sp<RfxMessage>& message) {
    int msgId = message->getId();
    int sourceId = message->getSource();

    logD(RFX_LOG_TAG,"response %d %s source:%d", msgId,
        requestToString(msgId), sourceId);

    switch (msgId) {
        case RIL_REQUEST_REPORT_STK_SERVICE_IS_RUNNING:
            // For CDMA SIM card, this request will send to GSM and C2K RILD.
            // But can't send two responses to RILJ, so ignore response from C2K
            if (RADIO_TECH_GROUP_C2K == sourceId) {
                logD(RFX_LOG_TAG,"Ignore response %d %s", msgId, requestToString(msgId));
                break;
            }
            responseToRilj(message);
        // / M: OTASP {
        case RIL_REQUEST_OEM_HOOK_STRINGS:
            break;
        // / M: OTASP }
        case RIL_LOCAL_C2K_REQUEST_SET_UTK_MODE:
            break;
        default:
            responseToRilj(message);
            break;
    }

    return true;
}

char* RpCatController::strdupReadString(Parcel* p) {
    size_t stringlen;
    const char16_t *s16;

    int pos = p->dataPosition();
    s16 = p->readString16Inplace(&stringlen);
    p->setDataPosition(pos);

    return strndup16to8(s16, stringlen);
}

void RpCatController::writeStringToParcel(Parcel *p, const char *s) {
    char16_t *s16;
    size_t s16_len = 0;
    s16 = strdup8to16(s, &s16_len);
    p->writeString16(s16, s16_len);
    free(s16);
}

int RpCatController::getStkCommandType(char *cmd) {
    char temp[3] = {0};
    int cmdType = 0;

    strncpy(temp, cmd, 2);
    cmdType = strtoul(temp, NULL, 16);
    cmdType = 0xFF & cmdType;

    return cmdType;
}

void RpCatController::parseStkCmdType(const sp<RfxMessage>& message, int* cmdType) {
    int msgId = message->getId();
    int typePos = 0;
    char* cmd = NULL;

    // Get command from message
    cmd = strdupReadString(message->getParcel());
    if (cmd) {

        // decide typePos value
        switch (msgId) {
            case RIL_UNSOL_STK_PROACTIVE_COMMAND:
            case RIL_UNSOL_STK_BIP_PROACTIVE_COMMAND:
            case RIL_UNSOL_STK_EVENT_NOTIFY:
                if (cmd[2] <= '7') {
                    typePos = 10;
                } else {
                    typePos = 12;
                }
                break;

            case RIL_REQUEST_STK_SEND_TERMINAL_RESPONSE:
                typePos = 6;
                break;

            default:
                logD(RFX_LOG_TAG,"Not Support msgId:%d", msgId);
                break;
        }

        // check command type
        *cmdType = getStkCommandType(&cmd[typePos]);

        free(cmd);
    } else {
        logD(RFX_LOG_TAG,"cmd is NULL. Error!!!");
    }
}

void RpCatController::parseStkEnvCmdType(const sp<RfxMessage>& message,
                                         int* envCmdType,
                                         int* eventType) {
    int msgId = message->getId();
    int envCmdPos = 0;
    int eventPos = 0;
    char* cmd = NULL;

    // Get command from message
    cmd = strdupReadString(message->getParcel());
    if (cmd) {

        // decide typePos value
        switch (msgId) {
            case RIL_REQUEST_STK_SEND_ENVELOPE_COMMAND:
                envCmdPos = 0;
                eventPos = 8;
                break;

            default:
                logD(RFX_LOG_TAG,"Not Support msgId:%d", msgId);
                break;
        }

        // check command type
        *envCmdType = getStkCommandType(&cmd[envCmdPos]);
        *eventType = getStkCommandType(&cmd[eventPos]);

        free(cmd);
    } else {
        logD(RFX_LOG_TAG,"cmd is NULL. Error!!!");
    }
}

const char* RpCatController::requestToString(int reqId) {
    switch (reqId) {
        case RIL_REQUEST_STK_GET_PROFILE: return "STK_GET_PROFILE";
        case RIL_REQUEST_STK_SET_PROFILE: return "STK_SET_PROFILE";
        case RIL_REQUEST_STK_SEND_ENVELOPE_COMMAND:
            return "STK_SEND_ENVELOPE_COMMAND";
        case RIL_REQUEST_STK_SEND_TERMINAL_RESPONSE:
            return "STK_SEND_TERMINAL_RESPONSE";
        case RIL_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM:
            return "STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM";
        case RIL_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM_WITH_RESULT_CODE:
            return "STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM_WITH_RESULT_CODE";
        case RIL_REQUEST_REPORT_STK_SERVICE_IS_RUNNING:
            return "REPORT_STK_SERVICE_IS_RUNNING";
        case RIL_REQUEST_STK_SEND_ENVELOPE_WITH_STATUS:
            return "STK_SEND_ENVELOPE_WITH_STATUS";
        default:
            return "UNKNOWN_REQUEST";
    }
}

void RpCatController::setSTKUTKMode(int mode) {
    sp<RfxMessage> new_msg = RfxMessage::obtainRequest(getSlotId(), RADIO_TECH_GROUP_GSM,
            RIL_REQUEST_SET_STK_UTK_MODE);
    Parcel* newMsgParcel = new_msg->getParcel();
    newMsgParcel->writeInt32(1);
    newMsgParcel->writeInt32(mode);
    requestToRild(new_msg);
    return;
}

const char* RpCatController::urcToString(int urcId) {
    switch (urcId) {
        case RIL_UNSOL_STK_SESSION_END: return "UNSOL_STK_SESSION_END";
        case RIL_UNSOL_STK_PROACTIVE_COMMAND: return "UNSOL_STK_PROACTIVE_COMMAND";
        case RIL_UNSOL_STK_EVENT_NOTIFY: return "UNSOL_STK_EVENT_NOTIFY";
        case RIL_UNSOL_STK_CALL_SETUP: return "UNSOL_STK_CALL_SETUP";
        case RIL_UNSOL_STK_SETUP_MENU_RESET: return "UNSOL_STK_SETUP_MENU_RESET";
        case RIL_UNSOL_STK_CC_ALPHA_NOTIFY: return "UNSOL_STK_CC_ALPHA_NOTIFY";
        case RIL_UNSOL_STK_BIP_PROACTIVE_COMMAND:
            return "UNSOL_STK_BIP_PROACTIVE_COMMAND";
        default:
            return "UNKNOWN_URC";
    }
}

void RpCatController::configUtkMode(int mode) {
    logD(RFX_LOG_TAG, "set UTK mode to %d", mode);
    sp<RfxMessage> new_msg = RfxMessage::obtainRequest(getSlotId(), RADIO_TECH_GROUP_C2K,
            RIL_LOCAL_C2K_REQUEST_SET_UTK_MODE);
    Parcel* newMsgParcel = new_msg->getParcel();
    newMsgParcel->writeInt32(1);
    newMsgParcel->writeInt32(mode);
    requestToRild(new_msg);
    return;
}

// / M: OTASP {
int RpCatController::getNwsMode() {
    RpNwRatController* mNwRatController =
        (RpNwRatController *)findController(getSlotId(),
        RFX_OBJ_CLASS_INFO(RpNwRatController));

    if (NULL != mNwRatController)
        return mNwRatController->getNwsMode();
    else
        return NWS_MODE_UNKNOWN;
}

bool RpCatController::checkIfNeedToTriggerOtasp() {
    if (mSupportBipOtaspActivation == 0) {
        logD(RFX_LOG_TAG, "checkIfNeedToTriggerOtasp(): mSupportBipOtaspActivation=0");
        return false;
    }

    logD(RFX_LOG_TAG, "checkIfNeedToTriggerOtasp(): otasp=%d", mOtaspActivationStatus);
    if (mOtaspActivationStatus == OTASP_PROGRAMMING_STARTED) {
        logD(RFX_LOG_TAG, "checkIfNeedToTriggerOtasp(): true - OTASP_PROGRAMMING_STARTED");
        return true;
    }

    logD(RFX_LOG_TAG, "checkIfNeedToTriggerOtasp(): csimStatus=%d", mCsimStatus);
    if (mCsimStatus == INVALID_VALUE) {
        mIsOpenChannelCommandQueued = true;
        logD(RFX_LOG_TAG, "checkIfNeedToTriggerOtasp(): false - unknown IMSI_M");
        return false;
    } else if (mCsimStatus == 1) {
        logD(RFX_LOG_TAG, "checkIfNeedToTriggerOtasp(): false - IMSI_M programmed");
        return false;
    }

    logD(RFX_LOG_TAG, "checkIfNeedToTriggerOtasp(): nw=%d", mDataRadioTech);
    if (mDataRadioTech == RADIO_TECH_1xRTT) {
        logD(RFX_LOG_TAG, "checkIfNeedToTriggerOtasp(): true - 1xRTT");
        return true;
    }

    logD(RFX_LOG_TAG, "checkIfNeedToTriggerOtasp(): false - not 1xRTT");
    return false;
}

void RpCatController::handleOtaspRequest(const sp<RfxMessage>& message) {
    logD(RFX_LOG_TAG, "handleOtaspRequest()");
    sendOpenChannelTRForOtasp(message);

    if ((mOtaspActivationStatus != OTASP_PROGRAMMING_STARTED) && (mFailureCount < 3)) {
        sendTriggerOtaspUrc(message);
    }
}

uint8_t RpCatController::toByte(char c) {
    if (c >= '0' && c <= '9') return (c - '0');
    if (c >= 'A' && c <= 'F') return (c - 'A' + 10);
    if (c >= 'a' && c <= 'f') return (c - 'a' + 10);

    logD(RFX_LOG_TAG, "toByte Error: %c", c);
    return 0;
}

bool RpCatController::findComprehensionTlvTag(char* cmd, int start_pos, uint8_t tagSearch,
        int* pTagStart, int* pTagTotal) {
    RFX_ASSERT(cmd != NULL);

    int end_pos = strlen(cmd);
    int pos = start_pos;
    uint8_t tag;
    uint32_t tagLength;

    while (pos < end_pos) {
        tag = (uint8_t) ((toByte(cmd[pos]) << 4) | toByte(cmd[pos+1]));
        tagLength = (uint32_t) ((toByte(cmd[pos+2]) << 4) | toByte(cmd[pos+3]));

        if (tag == tagSearch) {
            if (pTagStart != NULL) {
                *pTagStart = pos;
            }
            if (pTagTotal != NULL) {
                *pTagTotal = ((tagLength+2)*2);
            }
            logD(RFX_LOG_TAG, "Found TAG 0x%x at %d, len=%d", tagSearch, pos, ((tagLength+2)*2));
            // RFX_ASSERT((pos+ ((tagLength+2)*2)) <= end_pos);
            return true;
        }

        pos += ((tagLength+2)*2);
    }

    logD(RFX_LOG_TAG, "TAG 0x%x not found", tagSearch);
    return false;
}

void RpCatController::sendOpenChannelTRForOtasp(const sp<RfxMessage>& message) {
    logD(RFX_LOG_TAG, "sendOpenChannelTRForOtasp()");

    char* cmd = NULL;
    Parcel* parcel = message->getParcel();
    cmd = strdupReadString(parcel);

    sp<RfxMessage> new_msg = RfxMessage::obtainRequest(message->getSlotId(), message->getSource(),
            RIL_REQUEST_STK_SEND_TERMINAL_RESPONSE);
    Parcel* newMsgParcel = new_msg->getParcel();

    // network currently unable to process command (0x21)
    char terminalResponseString[64] = "810301400182028281830121";
    int stringLength = strlen(terminalResponseString);
    if (cmd != NULL) {
        int startPos;
        int tagStart;
        int tagTotal;
        bool found;
        if (cmd[2] <= '7') {
            startPos = 4;
        } else {
            startPos = 6;
        }

        found = findComprehensionTlvTag(cmd, startPos, 0x81, &tagStart, &tagTotal);
        RFX_ASSERT(found);  // Command Detail is mandatory
        if (found) {
            memcpy(&terminalResponseString[0], &cmd[tagStart], tagTotal);
        }

        // append BEARER_DESCRIPTION if found
        found = findComprehensionTlvTag(cmd, startPos, 0x35, &tagStart, &tagTotal);
        if (found) {
            memcpy(&terminalResponseString[stringLength], &cmd[tagStart], tagTotal);
            stringLength += tagTotal;
        }

        // append BUFFER_SIZE if found
        found = findComprehensionTlvTag(cmd, startPos, 0x39, &tagStart, &tagTotal);
        if (found) {
            memcpy(&terminalResponseString[stringLength], &cmd[tagStart], tagTotal);
            stringLength += tagTotal;
        }

        terminalResponseString[stringLength] = 0;  // null terminator

        free(cmd);
    }
    writeStringToParcel(newMsgParcel, terminalResponseString);

    requestToRild(new_msg);

    return;
}

void RpCatController::sendTriggerOtaspUrc(const sp<RfxMessage>& message) {
    logD(RFX_LOG_TAG, "sendTriggerOtaspUrc()");

    sp<RfxMessage> urcToRilj = RfxMessage::obtainUrc(message->getSlotId(), RIL_UNSOL_TRIGGER_OTASP);

    responseToRilj(urcToRilj);
}

void RpCatController::RetryOtaspIfNeeded() {
    if (mSupportBipOtaspActivation == 0) {
        return;
    }

    logD(RFX_LOG_TAG, "RetryOtaspIfNeeded() failure count = %d, call count = %d",
            mFailureCount, mVoiceCallCount);
    if ((mFailureCount >= 1) && (mFailureCount < 3)) {
        if ((mVoiceCallCount == 0) && checkIfNeedToTriggerOtasp()) {
            sendOtaspRetryRequestToModem3();
        }
    }
}

void RpCatController::sendOtaspRetryRequestToModem3() {
    logD(RFX_LOG_TAG, "sendOtaspRetryRequestToModem3()");
    if (mOtaspActivationStatus == OTASP_PROGRAMMING_STARTED) {
        logD(RFX_LOG_TAG, "sendOtaspRetryRequestToModem3(): already start programming");
        return;
    }

    sp<RfxMessage> new_msg = RfxMessage::obtainRequest(getSlotId(), RADIO_TECH_GROUP_C2K,
            RIL_REQUEST_OEM_HOOK_STRINGS);
    Parcel* newMsgParcel = new_msg->getParcel();
    newMsgParcel->writeInt32(2);
    writeStringToParcel(newMsgParcel, "AT+CDV=*22899");
    writeStringToParcel(newMsgParcel, "");

    requestToRild(new_msg);

    return;
}

bool RpCatController::isVzWSupport() {
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("persist.vendor.operator.optr", property_value, "0");
    return strcmp(property_value, "OP12") == 0;
}
// / M: OTASP }

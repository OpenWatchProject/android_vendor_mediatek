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
#include "RfxRootController.h"
#include "RpGsmSmsCtrl.h"
#include "RpSmsCtrl.h"
#include "RpSmsNSlotController.h"
#include "RfxRilUtils.h"
#include "nw/RpNwDefs.h"

/*****************************************************************************
 * Class RpSmsCtrl
 *****************************************************************************/
RFX_IMPLEMENT_CLASS("RpGsmSmsCtrl", RpGsmSmsCtrl, RpSmsCtrlBase);

RpGsmSmsCtrl::RpGsmSmsCtrl() {
    setName(String8("RpGsmSmsCtrl"));
    mSmsFwkReady = false; //TODO
    mSmsTimerHandle = NULL;
    mSmsSending = false;
    mNeedStatusReport = false;
}

void RpGsmSmsCtrl::onInit() {
    // Required: invoke super class implementation
    RpSmsCtrlBase::onInit();

    log(String8("onInit"));
    const int request_id_list[] = {
        RIL_REQUEST_SEND_SMS,
        RIL_REQUEST_SEND_SMS_EXPECT_MORE,
        //RIL_REQUEST_SMS_ACKNOWLEDGE,
        //RIL_REQUEST_SMS_ACKNOWLEDGE_EX,
        RIL_REQUEST_SET_SMS_FWK_READY,
    };

    const int urc_id_list[] = {
        RIL_UNSOL_RESPONSE_NEW_SMS,
        RIL_UNSOL_RESPONSE_NEW_BROADCAST_SMS,
        RIL_UNSOL_RESPONSE_ETWS_NOTIFICATION,
        RIL_UNSOL_RESPONSE_NEW_SMS_ON_SIM,
        RIL_UNSOL_RESPONSE_NEW_SMS_STATUS_REPORT,
        RIL_UNSOL_SIM_SMS_STORAGE_FULL,
        RIL_UNSOL_CDMA_RUIM_SMS_STORAGE_FULL
    };
    registerToHandleRequest(request_id_list, (sizeof(request_id_list)/sizeof(int)));
    registerToHandleUrc(urc_id_list, (sizeof(urc_id_list)/sizeof(int)));
    // register callbacks to get required information
    getStatusManager()->registerStatusChanged(RFX_STATUS_CONNECTION_STATE,
            RfxStatusChangeCallback(this, &RpGsmSmsCtrl::onHidlStateChanged));
}

int RpGsmSmsCtrl::isHandledInternal(const sp<RfxMessage>& message) {
    int result = RIL_REQUEST_HANDLE_REASON_NOT_HANDLE;
    int msgId = message->getId();
    log(String8::format("isHandledInternal %d %s", msgId, requestToString(msgId)));
    switch (msgId) {
        case RIL_REQUEST_IMS_SEND_SMS:
        case RIL_REQUEST_IMS_SEND_SMS_EX:
            if (handleImsSendSms(message)) {
                result = RIL_REQUEST_HANDLE_REASON_HANDLED;
            }
            break;

        default:
            break;
    }

    return result;
}

bool RpGsmSmsCtrl::onHandleRequest(const sp<RfxMessage>& message) {
    int msgId = message->getId();
    log(String8::format("onHandleRequest %d %s", msgId, requestToString(msgId)));
    switch (msgId) {
        case RIL_REQUEST_SEND_SMS:
        case RIL_REQUEST_SEND_SMS_EXPECT_MORE: {
            mSmsSending = true;
            requestToRild(
                RfxMessage::obtainRequest(RADIO_TECH_GROUP_GSM, message->getId(), message, true));
            break;
        }

        case RIL_REQUEST_SET_SMS_FWK_READY: {
            sp<RfxMessage> resToRilj = RfxMessage::obtainResponse(RIL_E_SUCCESS, message);
            mSmsFwkReady = true;
            responseToRilj(resToRilj);
            break;
        }
        default:
            break;
    }
    return true;
}

bool RpGsmSmsCtrl::handleResponse(const sp<RfxMessage>& message) {
    return onHandleResponse(message);
}

bool RpGsmSmsCtrl::onHandleResponse(const sp<RfxMessage>& message) {
    int msgId = message->getId();
    log(String8::format("onHandleResponse %d %s", msgId, requestToString(msgId)));
    switch (msgId) {
        case RIL_REQUEST_SEND_SMS:
        case RIL_REQUEST_SEND_SMS_EXPECT_MORE:
        case RIL_REQUEST_IMS_SEND_SMS: {
            responseToRilj(message);
            mSmsSending = false;
            break;
        }
        case RIL_REQUEST_IMS_SEND_SMS_EX: {
            // Update vector
            if (message->getError() == RIL_E_SUCCESS && mNeedStatusReport) {
                int32_t messageRef = -1;
                message->getParcel()->readInt32(&messageRef);
                message->resetParcelDataStartPos();
                //RIL_SMS_Response* data = (RIL_SMS_Response*)message->getData()->getData();
                log(String8::format("Ref %d is waitting for status report", messageRef));
                ((RpSmsCtrl *)getParent())->addReferenceId(messageRef);
            }
            mNeedStatusReport = false;
            responseToRilj(message);
            mSmsSending = false;
            break;
        }
        default:
            break;
    }
    return true;
}

bool RpGsmSmsCtrl::onHandleUrc(const sp<RfxMessage>& message) {
    int msg_id = message->getId();
    switch (msg_id) {
        case RIL_UNSOL_RESPONSE_NEW_SMS: {
                // Send to supl parse
                RfxRootController *root = RFX_OBJ_GET_INSTANCE(RfxRootController);
                RpSmsNSlotController *ctrl = (RpSmsNSlotController *)root->findController(
                        RFX_OBJ_CLASS_INFO(RpSmsNSlotController));
                ctrl->dispatchSms(message);
                // Send RILJ directly
                responseToRilj(message);
            }
            break;
        case RIL_UNSOL_SIM_SMS_STORAGE_FULL: {
                if (isCdmaPhoneMode()) {
                    log(String8::format("CS_RAT register to CDMA, not notify GSM SMS FULL"));
                    return true;
                }
                responseToRilj(message);
            }
            break;
        case RIL_UNSOL_RESPONSE_NEW_BROADCAST_SMS:
        case RIL_UNSOL_RESPONSE_ETWS_NOTIFICATION:
        case RIL_UNSOL_RESPONSE_NEW_SMS_ON_SIM:
        case RIL_UNSOL_CDMA_RUIM_SMS_STORAGE_FULL: {
                // Send RILJ directly
                responseToRilj(message);
            }
            break;
        case RIL_UNSOL_RESPONSE_NEW_SMS_STATUS_REPORT: {
                String8 pdu = String8(message->getParcel()->readString16());
                int ref = getReferenceIdFromCDS(pdu.string());
                log(String8::format("Ref %d is coming, cachedSize= %d", ref,
                        ((RpSmsCtrl *)getParent())->getCacheSize()));
                if (((RpSmsCtrl *)getParent())->removeReferenceIdCached(ref)) {
                    // This request comes from Ims, so we send it to Ims RIL indication
                    sp<RfxMessage> unsol = RfxMessage::obtainUrc(
                            m_slot_id, RIL_UNSOL_RESPONSE_NEW_SMS_STATUS_REPORT_EX, message, true);
                    responseToRilj(unsol);
                } else {
                    // Send RILJ directly
                    responseToRilj(message);
                }
            }
            break;
        default:
            log(String8::format("Not Support the urc %s", requestToString(msg_id)));
            break;
    }

    return true;
}

int RpGsmSmsCtrl::isPreviewedInternal(const sp<RfxMessage>& message) {
    if (!onPreviewMessage(message)) {
        return RIL_REQUEST_PREVIEW_REASON_PREVIEWED;
    }
    return RIL_REQUEST_PREVIEW_REASON_NOT_PREVIEW;
}

int RpGsmSmsCtrl::isResumedInternal(const sp<RfxMessage>& message) {
    if (onCheckIfResumeMessage(message)) {
        return RIL_REQUEST_RESUME_REASON_RESUMED;
    }
    return RIL_REQUEST_RESUME_REASON_NOT_RESUME;
}

bool RpGsmSmsCtrl::onPreviewMessage(const sp<RfxMessage>& message) {
    int msgId = message->getId();
    switch (msgId) {
        case RIL_REQUEST_SEND_SMS:
        case RIL_REQUEST_SEND_SMS_EXPECT_MORE:
        case RIL_REQUEST_IMS_SEND_SMS:
        case RIL_REQUEST_IMS_SEND_SMS_EX: {
            if (mSmsSending && (message->getType() == REQUEST)) {
                //We only send one request to rmc, the following request should queue in rtc
                log(String8::format("the previous request is sending, queue %s",
                            requestToString(msgId)));
                return false;
            }
            break;
        }

        case RIL_UNSOL_RESPONSE_NEW_SMS:
        case RIL_UNSOL_RESPONSE_NEW_BROADCAST_SMS:
        case RIL_UNSOL_RESPONSE_ETWS_NOTIFICATION:
        case RIL_UNSOL_RESPONSE_NEW_SMS_ON_SIM:
        case RIL_UNSOL_RESPONSE_NEW_SMS_STATUS_REPORT:
        case RIL_UNSOL_SIM_SMS_STORAGE_FULL:
        case RIL_UNSOL_CDMA_RUIM_SMS_STORAGE_FULL: {
            // If SMS framework is not ready, we should not allow NEW_SMS, NEW_BROADCAST,
            // NEW_SMS_ON_SIM and STATUS_REPORT
            if (!mSmsFwkReady) {
                log(String8::format("SMS framework isn't ready yet. queue %s",
                            requestToString(msgId)));
                return false;
            }
            break;
        }
    }
    return true;
}

bool RpGsmSmsCtrl::onCheckIfResumeMessage(const sp<RfxMessage>& message) {
    int msgId = message->getId();
    switch (msgId) {
        case RIL_REQUEST_SEND_SMS:
        case RIL_REQUEST_SEND_SMS_EXPECT_MORE:
        case RIL_REQUEST_IMS_SEND_SMS:
        case RIL_REQUEST_IMS_SEND_SMS_EX: {
            if (!mSmsSending && (message->getType() == REQUEST)) {
                //We only send one request to rmc, the following request should queue in rtc
                log(String8::format("the previous request is done, resume %s",
                            requestToString(msgId)));
                return true;
            }
            break;
        }

        case RIL_UNSOL_RESPONSE_NEW_SMS:
        case RIL_UNSOL_RESPONSE_NEW_BROADCAST_SMS:
        case RIL_UNSOL_RESPONSE_ETWS_NOTIFICATION:
        case RIL_UNSOL_RESPONSE_NEW_SMS_ON_SIM:
        case RIL_UNSOL_RESPONSE_NEW_SMS_STATUS_REPORT:
        case RIL_UNSOL_SIM_SMS_STORAGE_FULL:
        case RIL_UNSOL_CDMA_RUIM_SMS_STORAGE_FULL: {
            // If SMS framework is not ready, we should not allow NEW_SMS, NEW_BROADCAST,
            // NEW_SMS_ON_SIM and STATUS_REPORT
            if (mSmsFwkReady) {
                log(String8::format("SMS framework is ready. Let's resume %s",
                            requestToString(msgId)));
                return true;
            }
            break;
        }
    }
    return false;
}

bool RpGsmSmsCtrl::handleImsSendSms(const sp <RfxMessage>& message) {
    Parcel *imsSendSmsParcel;
    RIL_RadioTechnologyFamily radioTech;
    uint8_t retry = 0;
    int32_t messageRef = -1;
    char * smsc = NULL;
    char * pdu = NULL;
    bool result = false;

    imsSendSmsParcel = message->getParcel();
    radioTech = (RIL_RadioTechnologyFamily) imsSendSmsParcel->readInt32();
    imsSendSmsParcel->read(&retry,sizeof(retry));  // read the retry value
    imsSendSmsParcel->read(&messageRef,sizeof(messageRef));  // read message reference

    if (radioTech == RADIO_TECH_3GPP) {
        int32_t countStrings = -1;
        imsSendSmsParcel->readInt32 (&countStrings);
        log(String8::format("handleImsSendSms retry=%d, ref=%d, countStrings=%d",
                   retry, messageRef, countStrings));
        smsc = strdupReadString(imsSendSmsParcel);
        pdu = strdupReadString(imsSendSmsParcel);
        // reset the data position to the beginning
        message->resetParcelDataStartPos();

        int type = smsHexCharToDecInt(pdu, 2);
        if (smsc != NULL) {
            free(smsc);
        }
        if (pdu != NULL) {
            free(pdu);
        }
        if (message->getId() == RIL_REQUEST_IMS_SEND_SMS_EX) {
            if ((type & 0x20) == 0x20) {
                log(String8::format("Status report is needed, for IMS_SEND_SMS_EX"));
                mNeedStatusReport = true;
            } else {
                log(String8::format("Don't need status report"));
            }
        }
        // by-pass to GSM rild directly
        mSmsSending = true;
        requestToRild(
                RfxMessage::obtainRequest(RADIO_TECH_GROUP_GSM, message->getId(), message, true));
        result = true;
    } else {
        // reset the data position to the beginning
        message->resetParcelDataStartPos();
    }

    return result;
}

void RpGsmSmsCtrl::onHidlStateChanged(RfxStatusKeyEnum key,
        RfxVariant old_value, RfxVariant value) {
    bool oldState = false, newState = false;

    RFX_UNUSED(key);
    oldState = old_value.asBool();
    newState = value.asBool();

    log(String8::format("onHidlStateChanged (%s, %s, %s)", boolToString(oldState),
            boolToString(newState), boolToString(mSmsFwkReady)));

    if (mSmsTimerHandle != NULL) {
        RfxTimer::stop(mSmsTimerHandle);
    }
    mSmsTimerHandle = NULL;
    if (!newState) {
        mSmsFwkReady = false;
    } else if (newState && !mSmsFwkReady){
        //When RILD and RILJ Hidl connected, maybe InboundSmsHandler is initializing
        //So we start timer to wait InboundSmsHandler finish init and mCi.setOnNewGsmSms
        mSmsTimerHandle = RfxTimer::start(RfxCallback0(this,
                &RpGsmSmsCtrl::delaySetSmsFwkReady), ms2ns(DELAY_SET_SMS_FWK_READY_TIMER));
    }
}

void RpGsmSmsCtrl::delaySetSmsFwkReady() {
    log(String8::format("delaySetSmsFwkReady(%s to true)", boolToString(mSmsFwkReady)));
    mSmsFwkReady = true;
}

const char* RpGsmSmsCtrl::boolToString(bool value) {
    return value ? "true" : "false";
}

int RpGsmSmsCtrl::getReferenceIdFromCDS(const char *hex) {
    int smscLength = smsHexCharToDecInt(hex, 2);
    //CDS format: smscLength(2) - smsc(smscLength * 2) - type(2) - reference id - ...
    return smsHexCharToDecInt(hex + 2 + smscLength * 2 + 2, 2);
}

int RpGsmSmsCtrl::smsHexCharToDecInt(const char *hex, int length) {
    int i = 0;
    int value, digit;

    for (i = 0, value = 0; i < length && hex[i] != '\0'; i++) {
        if (hex[i]>='0' && hex[i]<='9') {
            digit = hex[i] - '0';
        } else if ( hex[i]>='A' && hex[i] <= 'F') {
            digit = hex[i] - 'A' + 10;
        } else if ( hex[i]>='a' && hex[i] <= 'f') {
            digit = hex[i] - 'a' + 10;
        } else {
            return -1;
        }
        value = value*16 + digit;
    }

    return value;
}

char *RpGsmSmsCtrl::strdupReadString(Parcel *p) {
    size_t stringlen;
    const char16_t *s16;
    s16 = p->readString16Inplace(&stringlen);
    return strndup16to8(s16, stringlen);
}

bool RpGsmSmsCtrl::isCdmaPhoneMode() {
    int preferedNwType = getStatusManager()->getIntValue(RFX_STATUS_KEY_PREFERRED_NW_TYPE, -1);
    int nwsMode = getStatusManager()->getIntValue(RFX_STATUS_KEY_NWS_MODE, -1);
    int tech = RADIO_TECH_UNKNOWN;
    log(String8::format("isCdmaPhoneMode:%d, %d", preferedNwType, nwsMode));
    switch (preferedNwType) {
        case PREF_NET_TYPE_CDMA_ONLY:
        case PREF_NET_TYPE_CDMA_EVDO_AUTO:
        case PREF_NET_TYPE_EVDO_ONLY:
        case PREF_NET_TYPE_LTE_CDMA_EVDO:
            tech = RADIO_TECH_1xRTT;
            break;

        case PREF_NET_TYPE_NR_ONLY:
        case PREF_NET_TYPE_NR_LTE:
        case PREF_NET_TYPE_NR_LTE_GSM_WCDMA:
        case PREF_NET_TYPE_NR_LTE_WCDMA:
        case PREF_NET_TYPE_NR_LTE_TDSCDMA:
        case PREF_NET_TYPE_NR_LTE_TDSCDMA_GSM:
        case PREF_NET_TYPE_NR_LTE_TDSCDMA_GSM_WCDMA:
        case PREF_NET_TYPE_NR_LTE_TDSCDMA_WCDMA:
            tech = RADIO_TECH_NR;
            break;

        case PREF_NET_TYPE_NR_LTE_CDMA_EVDO:
        case PREF_NET_TYPE_NR_LTE_CDMA_EVDO_GSM_WCDMA:
        case PREF_NET_TYPE_NR_LTE_TDSCDMA_CDMA_EVDO_GSM_WCDMA:
            tech = RADIO_TECH_1xRTT;
            if (nwsMode == NWS_MODE_CSFB) {
                tech = RADIO_TECH_NR;
            }
            break;

        case PREF_NET_TYPE_TD_SCDMA_LTE_CDMA_EVDO_GSM_WCDMA:
        case PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA_CDMA_EVDO_AUTO:
        case PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO:
        case PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA:
        case PREF_NET_TYPE_CDMA_GSM:
        case PREF_NET_TYPE_CDMA_EVDO_GSM:
        case PREF_NET_TYPE_LTE_CDMA_EVDO_GSM:
            tech = RADIO_TECH_1xRTT;
            if (nwsMode == NWS_MODE_CSFB) {
                tech = RADIO_TECH_GPRS;
            }
            break;
        default:
            break;
    }
    return RfxNwServiceState::isCdmaGroup(tech);
}




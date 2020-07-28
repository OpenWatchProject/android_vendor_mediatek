/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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
#include "RtcGsmSmsController.h"
#include <telephony/mtk_ril.h>
#include "RfxMessageId.h"
#include "RfxStringsData.h"
#include "RfxStringData.h"
#include "RfxSmsRspData.h"
#include "rfx_properties.h"
#include "RfxRilUtils.h"
#include "RfxRootController.h"
#include "RtcImsSmsController.h"
#include "RtcSmsMessage.h"
#include "RfxIntsData.h"
#include "RtcSmsNSlotController.h"
#include "RfxRilUtils.h"
#include "nw/RtcNwDefs.h"


using ::android::String8;

#define PROPERTY_DECRYPT "vold.decrypt"

RFX_IMPLEMENT_CLASS("RtcGsmSmsController", RtcGsmSmsController, RfxController);

// Register dispatch and response class
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringsData, RfxSmsRspData, \
        RFX_MSG_REQUEST_IMS_SEND_GSM_SMS);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringsData, RfxSmsRspData, \
        RFX_MSG_REQUEST_IMS_SEND_GSM_SMS_EX);

RFX_REGISTER_DATA_TO_URC_ID(RfxStringData, \
        RFX_MSG_URC_RESPONSE_NEW_SMS_STATUS_REPORT_EX);

/*****************************************************************************
 * Class RtcGsmSmsController
 *****************************************************************************/
RtcGsmSmsController::RtcGsmSmsController() {
    setTag(String8("RtcGsmSmsCtrl"));
    mSmsFwkReady = false;
    mSmsMdReady = false;
    mSmsTimerHandle = NULL;
    mSmsSending = false;
    mNeedStatusReport = false;
}

RtcGsmSmsController::~RtcGsmSmsController() {
}

void RtcGsmSmsController::onInit() {
    // Required: invoke super class implementation
    RfxController::onInit();

    const int request_id_list[] = {
        RFX_MSG_REQUEST_IMS_SEND_GSM_SMS,
        RFX_MSG_REQUEST_IMS_SEND_GSM_SMS_EX,
        RFX_MSG_REQUEST_SEND_SMS,
        RFX_MSG_REQUEST_SEND_SMS_EXPECT_MORE,
        RFX_MSG_REQUEST_WRITE_SMS_TO_SIM,
        RFX_MSG_REQUEST_DELETE_SMS_ON_SIM,
        RFX_MSG_REQUEST_REPORT_SMS_MEMORY_STATUS,
        RFX_MSG_REQUEST_GET_SMS_SIM_MEM_STATUS,
        RFX_MSG_REQUEST_GSM_GET_BROADCAST_SMS_CONFIG,
        RFX_MSG_REQUEST_GSM_SET_BROADCAST_SMS_CONFIG,
        RFX_MSG_REQUEST_GSM_GET_BROADCAST_LANGUAGE,
        RFX_MSG_REQUEST_GSM_SET_BROADCAST_LANGUAGE,
        RFX_MSG_REQUEST_GSM_SMS_BROADCAST_ACTIVATION,
        RFX_MSG_REQUEST_GET_GSM_SMS_BROADCAST_ACTIVATION,
        RFX_MSG_REQUEST_SMS_ACKNOWLEDGE,
        RFX_MSG_REQUEST_SMS_ACKNOWLEDGE_EX,
        RFX_MSG_REQUEST_SMS_ACKNOWLEDGE_INTERNAL
    };

    const int urc_id_list[] = {
        RFX_MSG_URC_RESPONSE_NEW_SMS,
        RFX_MSG_URC_RESPONSE_NEW_BROADCAST_SMS,
        RFX_MSG_URC_RESPONSE_ETWS_NOTIFICATION,
        RFX_MSG_URC_RESPONSE_NEW_SMS_ON_SIM,
        RFX_MSG_URC_RESPONSE_NEW_SMS_STATUS_REPORT,
        RFX_MSG_URC_SIM_SMS_STORAGE_FULL,
        RFX_MSG_URC_CDMA_RUIM_SMS_STORAGE_FULL,
        RFX_MSG_URC_SMS_READY_NOTIFICATION
    };

    // register request & URC id list
    // NOTE. one id can only be registered by one controller
    if (RfxRilUtils::isSmsSupport()) {
        registerToHandleRequest(request_id_list, sizeof(request_id_list)/sizeof(const int));

        registerToHandleUrc(urc_id_list, sizeof(urc_id_list)/sizeof(const int));

        // register callbacks to get required information
        getStatusManager()->registerStatusChanged(RFX_STATUS_CONNECTION_STATE,
                RfxStatusChangeCallback(this, &RtcGsmSmsController::onHidlStateChanged));
    }
}

bool RtcGsmSmsController::onCheckIfRejectMessage(
        const sp<RfxMessage>& message, bool isModemPowerOff, int radioState) {
    int msgId = message->getId();
    bool isWfcSupport = RfxRilUtils::isWfcSupport();

    if (!isModemPowerOff && (radioState == (int)RADIO_STATE_OFF) &&
            (msgId == RFX_MSG_REQUEST_SEND_SMS ||
            msgId == RFX_MSG_REQUEST_SEND_SMS_EXPECT_MORE) &&
            (isWfcSupport)) {
        logD(mTag, "onCheckIfRejectMessage, isModemPowerOff %d, isWfcSupport %d",
                isModemPowerOff, isWfcSupport);
        return false;
    } else if (!isModemPowerOff && (radioState == (int)RADIO_STATE_OFF) &&
            (msgId == RFX_MSG_REQUEST_WRITE_SMS_TO_SIM ||
            msgId == RFX_MSG_REQUEST_DELETE_SMS_ON_SIM ||
            msgId == RFX_MSG_REQUEST_REPORT_SMS_MEMORY_STATUS ||
            msgId == RFX_MSG_REQUEST_GET_SMS_SIM_MEM_STATUS ||
            msgId == RFX_MSG_REQUEST_GSM_GET_BROADCAST_SMS_CONFIG ||
            msgId == RFX_MSG_REQUEST_GSM_SET_BROADCAST_SMS_CONFIG ||
            msgId == RFX_MSG_REQUEST_GSM_GET_BROADCAST_LANGUAGE ||
            msgId == RFX_MSG_REQUEST_GSM_SET_BROADCAST_LANGUAGE ||
            msgId == RFX_MSG_REQUEST_GSM_SMS_BROADCAST_ACTIVATION ||
            msgId == RFX_MSG_REQUEST_GET_GSM_SMS_BROADCAST_ACTIVATION ||
            msgId == RFX_MSG_REQUEST_SMS_ACKNOWLEDGE ||
            msgId == RFX_MSG_REQUEST_SMS_ACKNOWLEDGE_INTERNAL ||
            msgId == RFX_MSG_REQUEST_SMS_ACKNOWLEDGE_EX)) {
        logD(mTag, "onCheckIfRejectMessage, isModemPowerOff %d, radioState %d",
                isModemPowerOff, radioState);
        return false;
    }

    if (msgId == RFX_MSG_URC_SMS_READY_NOTIFICATION) {
      return false;
    }

    return RfxController::onCheckIfRejectMessage(message, isModemPowerOff, radioState);
}

void RtcGsmSmsController::handleRequest(const sp<RfxMessage>& msg) {
    int msg_id = msg->getId();
    switch (msg_id) {
        case RFX_MSG_REQUEST_SET_SMS_FWK_READY: {
                mSmsFwkReady = true;
            }
            break;
        case RFX_MSG_REQUEST_IMS_SEND_SMS:
        case RFX_MSG_REQUEST_IMS_SEND_SMS_EX: {
                RIL_IMS_SMS_Message *pIms = (RIL_IMS_SMS_Message*)msg->getData()->getData();
                char** pStrs = pIms->message.gsmMessage;
                char* pdu = pStrs[1];
                int countStr = GSM_SMS_MESSAGE_STRS_COUNT;
                int type = smsHexCharToDecInt(pdu, 2);
                sp<RfxMessage> req;

                if (msg_id == RFX_MSG_REQUEST_IMS_SEND_SMS_EX) {
                    if ((type & 0x20) == 0x20) {
                        logD(mTag, "Status report is needed");
                        mNeedStatusReport = true;
                    } else {
                        logD(mTag, "Don't need status report");
                    }
                }

                int newId = (msg_id == RFX_MSG_REQUEST_IMS_SEND_SMS)?
                        RFX_MSG_REQUEST_IMS_SEND_GSM_SMS : RFX_MSG_REQUEST_IMS_SEND_GSM_SMS_EX;
                req = RfxMessage::obtainRequest(newId,
                        RfxStringsData(pStrs, countStr), msg, false);

                mSmsSending = true;
                requestToMcl(req);
            }
            break;
    }
}

bool RtcGsmSmsController::onHandleResponse(const sp<RfxMessage>& msg) {
    int msg_id = msg->getId();
    switch (msg_id) {
        case RFX_MSG_REQUEST_SEND_SMS:
        case RFX_MSG_REQUEST_SEND_SMS_EXPECT_MORE: {
            responseToRilj(msg);
            mSmsSending = false;
            break;
        }
        case RFX_MSG_REQUEST_WRITE_SMS_TO_SIM:
        case RFX_MSG_REQUEST_DELETE_SMS_ON_SIM:
        case RFX_MSG_REQUEST_REPORT_SMS_MEMORY_STATUS:
        case RFX_MSG_REQUEST_GET_SMS_SIM_MEM_STATUS:
        case RFX_MSG_REQUEST_GSM_GET_BROADCAST_SMS_CONFIG:
        case RFX_MSG_REQUEST_GSM_SET_BROADCAST_SMS_CONFIG:
        case RFX_MSG_REQUEST_GSM_GET_BROADCAST_LANGUAGE:
        case RFX_MSG_REQUEST_GSM_SET_BROADCAST_LANGUAGE:
        case RFX_MSG_REQUEST_GSM_SMS_BROADCAST_ACTIVATION:
        case RFX_MSG_REQUEST_GET_GSM_SMS_BROADCAST_ACTIVATION:
        case RFX_MSG_REQUEST_SMS_ACKNOWLEDGE:
        case RFX_MSG_REQUEST_SMS_ACKNOWLEDGE_EX: {
                // Send RILJ directly
                responseToRilj(msg);
            }
            break;
        case RFX_MSG_REQUEST_IMS_SEND_GSM_SMS:
        case RFX_MSG_REQUEST_IMS_SEND_GSM_SMS_EX: {
                if (msg_id == RFX_MSG_REQUEST_IMS_SEND_GSM_SMS_EX) {
                    // Update vector
                    if (msg->getError() == RIL_E_SUCCESS && mNeedStatusReport) {
                        RIL_SMS_Response* data = (RIL_SMS_Response*)msg->getData()->getData();
                        logD(mTag, "Ref %d is waitting for status report", data->messageRef);
                        ((RtcImsSmsController *)getParent())->addReferenceId(data->messageRef);
                    }
                    mNeedStatusReport = false;
                }
                sp<RfxMessage> rsp;
                int newId = (msg_id == RFX_MSG_REQUEST_IMS_SEND_GSM_SMS)?
                        RFX_MSG_REQUEST_IMS_SEND_SMS : RFX_MSG_REQUEST_IMS_SEND_SMS_EX;
                rsp = RfxMessage::obtainResponse(newId, msg);

                responseToRilj(rsp);
                mSmsSending = false;
            }
            break;
        case RFX_MSG_REQUEST_SMS_ACKNOWLEDGE_INTERNAL: {
                sp<RfxMessage> rsp =
                    RfxMessage::obtainResponse(RFX_MSG_REQUEST_CDMA_SMS_ACKNOWLEDGE, msg);
                responseToRilj(rsp);
            }
            break;
        default: {
            logD(mTag, "Not Support the req %d", msg_id);
            break;
        }
    }

    return true;
}

bool RtcGsmSmsController::previewMessage(const sp<RfxMessage>& message) {
    return onPreviewMessage(message);
}

bool RtcGsmSmsController::onPreviewMessage(const sp<RfxMessage>& msg) {
    int msgId = msg->getId();
    switch (msgId) {
        case RFX_MSG_REQUEST_IMS_SEND_SMS:
        case RFX_MSG_REQUEST_IMS_SEND_SMS_EX:
        case RFX_MSG_REQUEST_SEND_SMS:
        case RFX_MSG_REQUEST_SEND_SMS_EXPECT_MORE: {
            if (mSmsSending && (msg->getType() == REQUEST)) {
                //We only send one request to rmc, the following request should queue in rtc
                logD(mTag, "the previous request is sending, queue %s", idToString(msgId));
                return false;
            }
            break;
        }

        case RFX_MSG_URC_RESPONSE_NEW_SMS:
        case RFX_MSG_URC_RESPONSE_NEW_BROADCAST_SMS:
        case RFX_MSG_URC_RESPONSE_ETWS_NOTIFICATION:
        case RFX_MSG_URC_RESPONSE_NEW_SMS_ON_SIM:
        case RFX_MSG_URC_RESPONSE_NEW_SMS_STATUS_REPORT:
        case RFX_MSG_URC_SIM_SMS_STORAGE_FULL:
        case RFX_MSG_URC_CDMA_RUIM_SMS_STORAGE_FULL:
        case RFX_MSG_URC_SMS_READY_NOTIFICATION: {
            // If SMS framework is not ready, we should not allow NEW_SMS, NEW_BROADCAST,
            // NEW_SMS_ON_SIM and STATUS_REPORT
            if (!mSmsFwkReady) {
                logD(mTag, "SMS framework isn't ready yet. queue %s", idToString(msgId));
                return false;
            }
            break;
        }
        case RFX_MSG_REQUEST_GSM_SET_BROADCAST_SMS_CONFIG:
        case RFX_MSG_REQUEST_GSM_SET_BROADCAST_LANGUAGE:{
            // If SMS md is not ready, we should not allow to config broadcast
            if (!mSmsMdReady) {
                logD(mTag, "SMS Md isn't ready yet. queue %s", idToString(msgId));
                return false;
            }
            break;
        }
    }
    return true;
}

bool RtcGsmSmsController::checkIfResumeMessage(const sp<RfxMessage>& message) {
    return onCheckIfResumeMessage(message);
}

bool RtcGsmSmsController::onCheckIfResumeMessage(const sp<RfxMessage>& msg) {
    int msgId = msg->getId();
    switch (msgId) {
        case RFX_MSG_REQUEST_IMS_SEND_SMS:
        case RFX_MSG_REQUEST_IMS_SEND_SMS_EX:
        case RFX_MSG_REQUEST_SEND_SMS:
        case RFX_MSG_REQUEST_SEND_SMS_EXPECT_MORE: {
            if (!mSmsSending && (msg->getType() == REQUEST)) {
                //We only send one request to rmc, the following request should queue in rtc
                logD(mTag, "the previous request is done, resume %s", idToString(msgId));
                return true;
            }
            break;
        }

        case RFX_MSG_URC_RESPONSE_NEW_SMS:
        case RFX_MSG_URC_RESPONSE_NEW_BROADCAST_SMS:
        case RFX_MSG_URC_RESPONSE_ETWS_NOTIFICATION:
        case RFX_MSG_URC_RESPONSE_NEW_SMS_ON_SIM:
        case RFX_MSG_URC_RESPONSE_NEW_SMS_STATUS_REPORT:
        case RFX_MSG_URC_SIM_SMS_STORAGE_FULL:
        case RFX_MSG_URC_CDMA_RUIM_SMS_STORAGE_FULL:
        case RFX_MSG_URC_SMS_READY_NOTIFICATION: {
            // If SMS framework is not ready, we should not allow NEW_SMS, NEW_BROADCAST,
            // NEW_SMS_ON_SIM and STATUS_REPORT
            if (mSmsFwkReady) {
                logD(mTag, "SMS framework is ready. Let's resume %s!", idToString(msgId));
                return true;
            }
            break;
        }
        case RFX_MSG_REQUEST_GSM_SET_BROADCAST_SMS_CONFIG:
        case RFX_MSG_REQUEST_GSM_SET_BROADCAST_LANGUAGE:{
            // If SMS md is not ready, we should not allow to config broadcast
            if (mSmsMdReady) {
                logD(mTag, "SMS Md is ready. Let's resume %s", idToString(msgId));
                return true;
            }
            break;
        }
    }
    return false;
}

bool RtcGsmSmsController::onHandleRequest(const sp<RfxMessage>& message) {
    int msg_id = message->getId();
    switch (msg_id) {
        case RFX_MSG_REQUEST_SEND_SMS:
        case RFX_MSG_REQUEST_SEND_SMS_EXPECT_MORE: {
            mSmsSending = true;
            requestToMcl(message);
            break;
        }
        case RFX_MSG_REQUEST_WRITE_SMS_TO_SIM:
        case RFX_MSG_REQUEST_DELETE_SMS_ON_SIM:
        case RFX_MSG_REQUEST_REPORT_SMS_MEMORY_STATUS:
        case RFX_MSG_REQUEST_GET_SMS_SIM_MEM_STATUS:
        case RFX_MSG_REQUEST_GSM_GET_BROADCAST_SMS_CONFIG:
        case RFX_MSG_REQUEST_GSM_SET_BROADCAST_SMS_CONFIG:
        case RFX_MSG_REQUEST_GSM_GET_BROADCAST_LANGUAGE:
        case RFX_MSG_REQUEST_GSM_SET_BROADCAST_LANGUAGE:
        case RFX_MSG_REQUEST_GSM_SMS_BROADCAST_ACTIVATION:
        case RFX_MSG_REQUEST_GET_GSM_SMS_BROADCAST_ACTIVATION:
        case RFX_MSG_REQUEST_SMS_ACKNOWLEDGE_EX: {
            // Send RMC directly
            requestToMcl(message);
            break;
        }
        case RFX_MSG_REQUEST_SMS_ACKNOWLEDGE:
            handNewSmsAck(message);
            break;
        default: {
            logD(mTag, "onHandleRequest, not Support the req %s", idToString(msg_id));
            break;
        }
    }

    return true;
}

bool RtcGsmSmsController::onHandleUrc(const sp<RfxMessage>& msg) {
    int msg_id = msg->getId();
    switch (msg_id) {
        case RFX_MSG_URC_RESPONSE_NEW_SMS: {
                // Send to supl parse
                RfxRootController *root = RFX_OBJ_GET_INSTANCE(RfxRootController);
                RtcSmsNSlotController *ctrl = (RtcSmsNSlotController *)root->findController(
                        RFX_OBJ_CLASS_INFO(RtcSmsNSlotController));
                ctrl->dispatchSms(msg);

                // Send RILJ directly
                if (isSupportSmsFormatConvert()) {
                    handleNewSms(msg);
                } else {
                    responseToRilj(msg);
                }
            }
            break;
        case RFX_MSG_URC_SIM_SMS_STORAGE_FULL: {
                if (isCdmaPhoneMode()) {
                  logD(mTag, "CS_RAT register to CDMA, not notify GSM SMS FULL");
                  return true;
                }
                responseToRilj(msg);
            }
            break;
        case RFX_MSG_URC_SMS_READY_NOTIFICATION: {
                mSmsMdReady = true;
                // Send RILJ directly
                responseToRilj(msg);
            }
            break;
        case RFX_MSG_URC_RESPONSE_NEW_BROADCAST_SMS:
        case RFX_MSG_URC_RESPONSE_ETWS_NOTIFICATION:
        case RFX_MSG_URC_RESPONSE_NEW_SMS_ON_SIM:
        case RFX_MSG_URC_CDMA_RUIM_SMS_STORAGE_FULL:{
                // Send RILJ directly
                responseToRilj(msg);
            }
            break;
        case RFX_MSG_URC_RESPONSE_NEW_SMS_STATUS_REPORT: {
                char* pdu = (char*)msg->getData()->getData();
                int ref = getReferenceIdFromCDS(pdu);
                logD(mTag, "Ref %d is coming, cachedSize= %d", ref,
                        ((RtcImsSmsController *)getParent())->getCacheSize());
                if (((RtcImsSmsController *)getParent())->removeReferenceIdCached(ref)) {
                    // This request comes from Ims, so we send it to Ims RIL indication
                    sp<RfxMessage> unsol = RfxMessage::obtainUrc(
                            m_slot_id, RFX_MSG_URC_RESPONSE_NEW_SMS_STATUS_REPORT_EX, msg);
                    responseToRilj(unsol);
                } else {
                    // Send RILJ directly
                    responseToRilj(msg);
                }
            }
            break;
        default:
            logD(mTag, "Not Support the urc %s", idToString(msg_id));
            break;
    }

    return true;
}

void RtcGsmSmsController::onHidlStateChanged(RfxStatusKeyEnum key,
        RfxVariant old_value, RfxVariant value) {
    bool oldState = false, newState = false;

    RFX_UNUSED(key);
    oldState = old_value.asBool();
    newState = value.asBool();

    logD(mTag, "onHidlStateChanged (%s, %s, %s) (slot %d)", boolToString(oldState),
            boolToString(newState), boolToString(mSmsFwkReady), getSlotId());

    if (mSmsTimerHandle != NULL) {
        RfxTimer::stop(mSmsTimerHandle);
    }
    mSmsTimerHandle = NULL;
    if (!newState) {
        mSmsFwkReady = false;
        if (oldState) {
            // if phone process die, clear the flag
            getStatusManager()->setIntValue(
                    RFX_STATUS_KEY_GSM_INBOUND_SMS_TYPE, SMS_INBOUND_NONE);
        }
    } else if (newState && !mSmsFwkReady && !isUnderCryptKeeper()) {
        //When RILD and RILJ Hidl connected, maybe InboundSmsHandler is initializing
        //So we start timer to wait InboundSmsHandler finish init and mCi.setOnNewGsmSms
        mSmsTimerHandle = RfxTimer::start(RfxCallback0(this,
                &RtcGsmSmsController::delaySetSmsFwkReady), ms2ns(DELAY_SET_SMS_FWK_READY_TIMER));
    }
}

void RtcGsmSmsController::delaySetSmsFwkReady() {
    logD(mTag, "delaySetSmsFwkReady(%s to true)(slot%d)", boolToString(mSmsFwkReady), getSlotId());
    mSmsFwkReady = true;
}


/*****************************************************************************
 * Utility function
 *****************************************************************************/
const char* RtcGsmSmsController::boolToString(bool value) {
    return value ? "true" : "false";
}

int RtcGsmSmsController::getReferenceIdFromCDS(char *hex) {
    int smscLength = smsHexCharToDecInt(hex, 2);
    //CDS format: smscLength(2) - smsc(smscLength * 2) - type(2) - reference id - ...
    return smsHexCharToDecInt(hex + 2 + smscLength * 2 + 2, 2);
}

int RtcGsmSmsController::smsHexCharToDecInt(char *hex, int length) {
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

void RtcGsmSmsController::handleNewSms(const sp<RfxMessage>& msg) {
    unsigned char* pdu = (unsigned char*)msg->getData()->getData();
    int length = msg->getData()->getDataLength();
    RtcGsmSmsMessage *gsmMessage = new RtcGsmSmsMessage(pdu, length);
    RtcSmsMessage *convertedMessage = NULL;
    RtcImsSmsController *parentCtrl = (RtcImsSmsController *)getParent();
    RtcConCatSmsGroup *group = NULL;
    RtcConCatSmsSender *sender = NULL;
    RtcConCatSmsRoot *root = NULL;
    bool needFree = true;
    if (parentCtrl != NULL) {
        root = parentCtrl->getConCatSmsRoot();
    }
    if (gsmMessage != NULL && gsmMessage->isConcatSms()) {
        if (root != NULL) {
            sender = root->getSmsSender(
                    gsmMessage->getSmsAddress()->getAddressString());
            if (sender != NULL) {
                group = sender->getSmsGroup(
                    gsmMessage->getUserDataHeader()->getRefNumber(),
                    gsmMessage->getUserDataHeader()->getMsgCount());
                if (group != NULL) {
                    RtcConCatSmsPart *part = group->getSmsPart(
                            gsmMessage->getUserDataHeader()->getSeqNumber());
                     if (part != NULL) {
                        part->setFormat3Gpp(true);
                        part->setMessage(gsmMessage);
                        convertedMessage = part->getConvertedMessage();
                        needFree = false;
                    }
                    group->updateTimeStamp();
                }
           }
       }
    }

    if (convertedMessage == NULL || convertedMessage->isError()) {
        responseToRilj(msg);
    } else {
        if (parentCtrl != NULL) {
            // send the converted Message
            parentCtrl->sendCdmaSms((RtcCdmaSmsMessage *)convertedMessage);
        }
    }
    if (root != NULL) {
        root->cleanUpObj();
    }
    if (needFree) {
        delete gsmMessage;
    }
}

void RtcGsmSmsController::handNewSmsAck(const sp<RfxMessage>& message) {
    bool onGoing = getStatusManager()->getBoolValue(
            RFX_STATUS_KEY_SMS_FORMAT_CHANGE_ONGOING);
    if (onGoing) {
        getStatusManager()->setBoolValue(
                RFX_STATUS_KEY_SMS_FORMAT_CHANGE_ONGOING, false);
        RtcImsSmsController *parentCtrl = (RtcImsSmsController *)getParent();
        if (parentCtrl != NULL) {
            parentCtrl->sendCdmaSmsAck(message);
        }
    } else {
        requestToMcl(message);
    }
}

void RtcGsmSmsController::sendGsmSms(RtcGsmSmsMessage * msg) {
    sp<RfxMessage> urc = RfxMessage::obtainUrc(getSlotId(), RFX_MSG_URC_RESPONSE_NEW_SMS,
        RfxStringData((void *)msg->getHexPdu().string(), msg->getHexPdu().length()));
    responseToRilj(urc);
    getStatusManager()->setBoolValue(
            RFX_STATUS_KEY_SMS_FORMAT_CHANGE_ONGOING, true);
}

void RtcGsmSmsController::sendGsmSmsAck(int success, int cause, const sp<RfxMessage> & message) {
    int data[2];
    data[0] = success;
    data[1] = cause;
    sp<RfxMessage> msg = RfxMessage::obtainRequest(
        RFX_MSG_REQUEST_SMS_ACKNOWLEDGE_INTERNAL,
        RfxIntsData(data, 2), message, false);
    requestToMcl(msg);
}

bool RtcGsmSmsController::isSupportSmsFormatConvert() {
    RtcImsSmsController *parentCtrl = (RtcImsSmsController *)getParent();
    if (parentCtrl != NULL) {
        return parentCtrl->isSupportSmsFormatConvert();
    }
    return false;
}

bool RtcGsmSmsController::isUnderCryptKeeper() {
    char vold_decrypt[RFX_PROPERTY_VALUE_MAX] = { 0 };
    rfx_property_get(PROPERTY_DECRYPT, vold_decrypt, "false");

    if (!strcmp("trigger_restart_min_framework", vold_decrypt)) {
        logD(mTag, "UnderCryptKeeper");
        return true;
    }
    return false;
}

bool RtcGsmSmsController::isCdmaPhoneMode() {
    int preferedNwType = getStatusManager()->getIntValue(RFX_STATUS_KEY_PREFERRED_NW_TYPE, -1);
    int nwsMode = getStatusManager()->getIntValue(RFX_STATUS_KEY_NWS_MODE, -1);
    int tech = RADIO_TECH_UNKNOWN;
    logD(mTag, "isCdmaPhoneMode:%d, %d", preferedNwType, nwsMode);
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

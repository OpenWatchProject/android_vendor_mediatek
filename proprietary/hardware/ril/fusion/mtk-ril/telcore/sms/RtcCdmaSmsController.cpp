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
#include "RtcCdmaSmsController.h"
#include "RfxCdmaSmsMessageId.h"
#include "sms/RmcCdmaMoSms.h"
#include "sms/RmcCdmaMtSms.h"
#include "sms/RmcCdmaSmsConverter.h"
#include "ratconfig.h"
#include "RtcImsSmsController.h"
#include "sms/RmcCdmaSmsAck.h"
#include "RfxRilUtils.h"
#include "nw/RtcNwDefs.h"
#include <compiler/compiler_utils.h>

/*****************************************************************************
 * Class RtcCdmaSmsController
 *****************************************************************************/
RFX_IMPLEMENT_CLASS("RtcCdmaSmsController", RtcCdmaSmsController, RfxController);
RtcCdmaSmsController::RtcCdmaSmsController() :m_timer(NULL), m_needStatusReport(false) {
}


RtcCdmaSmsController::~RtcCdmaSmsController() {
}


void RtcCdmaSmsController::onInit() {
    // Required: invoke super class implementation
    RfxController::onInit();

    const int request[] = {
        RFX_MSG_REQUEST_IMS_SEND_CDMA_SMS,
        RFX_MSG_REQUEST_IMS_SEND_CDMA_SMS_EX,
        RFX_MSG_REQUEST_CDMA_SEND_SMS,
        RFX_MSG_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION,
        RFX_MSG_REQUEST_CDMA_DELETE_SMS_ON_RUIM,
        RFX_MSG_REQUEST_CDMA_WRITE_SMS_TO_RUIM,
        RFX_MSG_REQUEST_GET_SMS_RUIM_MEM_STATUS,
        RFX_MSG_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG,
        RFX_MSG_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG,
        RFX_MSG_REQUEST_GET_SMSC_ADDRESS,
        RFX_MSG_REQUEST_SET_SMSC_ADDRESS,
        RFX_MSG_REQUEST_CDMA_SMS_ACKNOWLEDGE_EX,
        RFX_MSG_REQUEST_CDMA_SMS_ACKNOWLEDGE,
        RFX_MSG_REQUEST_CDMA_SMS_ACKNOWLEDGE_INTERNAL
    };
    const int urc[] = {
        RFX_MSG_URC_CDMA_NEW_SMS,
        RFX_MSG_URC_CDMA_RUIM_SMS_STORAGE_FULL
    };

    registerToHandleRequest(request, sizeof(request)/sizeof(const int));
    registerToHandleUrc(urc, sizeof(urc)/sizeof(const int));
}


bool RtcCdmaSmsController::onHandleRequest(const sp<RfxMessage>& message) {
    handleRequest(message);
    return true;
}

bool RtcCdmaSmsController::onHandleUrc(const sp<RfxMessage>& message) {
    if (message->getId() == RFX_MSG_URC_CDMA_NEW_SMS) {
        logD(TAG, "Start MT SMS ack timer");
        m_timer = RfxTimer::start(RfxCallback0(this, &RtcCdmaSmsController::onTimer),
                s2ns(MT_SMS_ACK_TIME));
        int messageId = -1;
        bool statusReport = RmcCdmaSmsConverter::isStatusReport(
                (RIL_CDMA_SMS_Message *)message->getData()->getData(), &messageId);;
        logD(TAG, "status report = %d, cache size = %d", statusReport,
                ((RtcImsSmsController *)getParent())->getCacheSize());
        if (statusReport &&
                ((RtcImsSmsController *)getParent())->removeReferenceIdCached(
                        messageId)) {
            sp<RfxMessage> urc;
            urc = RfxMessage::obtainUrc(getSlotId(), RFX_MSG_URC_CDMA_NEW_SMS_EX, message);
            return RfxController::onHandleUrc(urc);
        }
        if (isSupportSmsFormatConvert() && onHandleNewSms(message)) {
            return true;
        }
    } else if (message->getId() == RFX_MSG_URC_CDMA_RUIM_SMS_STORAGE_FULL) {
        int c2k_volte_state = -1;
        c2k_volte_state = getStatusManager()->getIntValue(RFX_STATUS_KEY_VOLTE_STATE, -1);
        if (RfxRilUtils::isCtVolteSupport() && (c2k_volte_state == 1) &&
                (isCdmaPhoneMode() == false)) {
            logD(TAG, "CT Volter is On and CS_RAT register to GSM, not notify CDMA SMS FULL");
            return true;
        }
    }
    return RfxController::onHandleUrc(message);
}

void RtcCdmaSmsController::onTimer() {
    logD(TAG, "MT SMS ack timeout");
    getStatusManager()->setIntValue(
            RFX_STATUS_KEY_CDMA_INBOUND_SMS_TYPE,
            CDMA_SMS_INBOUND_NONE);
    getStatusManager()->setBoolValue(
            RFX_STATUS_KEY_SMS_FORMAT_CHANGE_ONGOING, false);
    m_timer = NULL;
}


void RtcCdmaSmsController::handleMoSmsRequests(const sp<RfxMessage>& message) {
    if (message->getId() == RFX_MSG_REQUEST_IMS_SEND_SMS ||
            message->getId() == RFX_MSG_REQUEST_IMS_SEND_SMS_EX) {
        RIL_IMS_SMS_Message *pIms = (RIL_IMS_SMS_Message*)message->getData()->getData();
        sp<RfxMessage> req;
        int newId = (message->getId() == RFX_MSG_REQUEST_IMS_SEND_SMS) ?
                RFX_MSG_REQUEST_IMS_SEND_CDMA_SMS : RFX_MSG_REQUEST_IMS_SEND_CDMA_SMS_EX;
        req = RfxMessage::obtainRequest(newId,
                RmcCdmaMoSmsMessage(pIms->message.cdmaMessage), message, false);
        if (newId == RFX_MSG_REQUEST_IMS_SEND_CDMA_SMS_EX) {
            m_needStatusReport = RmcCdmaSmsConverter::isNeedStatusReport(
                    pIms->message.cdmaMessage);
        }
        requestToMcl(req);
    } else {
        RFX_ASSERT(message->getId() == RFX_MSG_REQUEST_CDMA_SEND_SMS);
        requestToMcl(message);
    }
    getStatusManager()->setIntValue(
        RFX_STATUS_KEY_CDMA_MO_SMS_STATE, CDMA_MO_SMS_SENDING);
}


void RtcCdmaSmsController::handleSmscAdressResponses(const sp<RfxMessage>& message) {
    int type = getStatusManager()->getIntValue(RFX_STATUS_KEY_CDMA_CARD_TYPE);
    if (message->getError() != RIL_E_SUCCESS) {
        if (type == CT_3G_UIM_CARD || type == UIM_CARD ||
                type == CT_UIM_SIM_CARD || type == UIM_SIM_CARD) {
            sp<RfxMessage> newMsg =
                RfxMessage::obtainResponse(RIL_E_REQUEST_NOT_SUPPORTED, message);
            RfxController::onHandleResponse(newMsg);
            return;
        }
    }
    RfxController::onHandleResponse(message);
}


void RtcCdmaSmsController::handleRequest(const sp<RfxMessage>& message) {
    int msgId = message->getId();
    switch (msgId) {
        case RFX_MSG_REQUEST_IMS_SEND_SMS:
        case RFX_MSG_REQUEST_IMS_SEND_SMS_EX:
        case RFX_MSG_REQUEST_CDMA_SEND_SMS:
            handleMoSmsRequests(message);
            break;

        case RFX_MSG_REQUEST_GET_SMSC_ADDRESS:
        case RFX_MSG_REQUEST_SET_SMSC_ADDRESS:
        case RFX_MSG_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION:
        case RFX_MSG_REQUEST_CDMA_DELETE_SMS_ON_RUIM:
        case RFX_MSG_REQUEST_CDMA_WRITE_SMS_TO_RUIM:
        case RFX_MSG_REQUEST_GET_SMS_RUIM_MEM_STATUS:
        case RFX_MSG_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG:
        case RFX_MSG_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG:
        case RFX_MSG_REQUEST_CDMA_SMS_ACKNOWLEDGE_EX:
            requestToMcl(message);
            break;

        case RFX_MSG_REQUEST_CDMA_SMS_ACKNOWLEDGE:
            handleCdmaSmsAck(message);
            break;

        default:
            RFX_ASSERT(0);
            break;
    }
    if (msgId == RFX_MSG_REQUEST_CDMA_SMS_ACKNOWLEDGE ||
        msgId == RFX_MSG_REQUEST_CDMA_SMS_ACKNOWLEDGE_EX) {
        if (m_timer != NULL) {
            logD(TAG, "Stop MT SMS ack timer");
            RfxTimer::stop(m_timer);
            m_timer = NULL;
        }
    }
}

void RtcCdmaSmsController::handleMoSmsResponses(const sp<RfxMessage>& msg) {
    int msg_id = msg->getId();
    switch (msg_id) {
        case RFX_MSG_REQUEST_IMS_SEND_CDMA_SMS:
        case RFX_MSG_REQUEST_IMS_SEND_CDMA_SMS_EX:
            {
                sp<RfxMessage> rsp;
                int newId = (msg_id == RFX_MSG_REQUEST_IMS_SEND_CDMA_SMS) ?
                        RFX_MSG_REQUEST_IMS_SEND_SMS : RFX_MSG_REQUEST_IMS_SEND_SMS_EX;
                if (newId  == RFX_MSG_REQUEST_IMS_SEND_SMS_EX) {
                    RIL_SMS_Response *moRsp = (RIL_SMS_Response*)msg->getData()->getData();
                    logD(TAG, "Need status report %d, messageId = %d", m_needStatusReport,
                            moRsp->messageRef);
                    if (m_needStatusReport && (msg->getError() == RIL_E_SUCCESS) &&
                            moRsp->messageRef > 0) {
                        ((RtcImsSmsController *)getParent())->addReferenceId(
                                moRsp->messageRef);
                        m_needStatusReport = false;
                    }
                }
                rsp = RfxMessage::obtainResponse(newId, msg);
                responseToRilj(rsp);
            }
            break;
        case RFX_MSG_REQUEST_CDMA_SEND_SMS:
            RfxController::onHandleResponse(msg);
            break;
        default:
            RFX_ASSERT(0);
            break;
    }
    getStatusManager()->setIntValue(
        RFX_STATUS_KEY_CDMA_MO_SMS_STATE, CDMA_MO_SMS_SENT);
}

bool RtcCdmaSmsController::onCheckCdmaSupported(const sp<RfxMessage>& msg) {
    if (RatConfig_isC2kSupported() == 0) {
        int msgId = msg->getId();
        switch (msgId) {
            case RFX_MSG_REQUEST_CDMA_SEND_SMS: {
                getStatusManager()->setIntValue(
                        RFX_STATUS_KEY_CDMA_MO_SMS_STATE, CDMA_MO_SMS_SENT);
                /* falls through */
                TELEPHONYWARE_FALLTHROUGH;
            }
            case RFX_MSG_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION:
            case RFX_MSG_REQUEST_CDMA_DELETE_SMS_ON_RUIM:
            case RFX_MSG_REQUEST_CDMA_WRITE_SMS_TO_RUIM:
            case RFX_MSG_REQUEST_GET_SMS_RUIM_MEM_STATUS:
            case RFX_MSG_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG:
            case RFX_MSG_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG: {
                sp<RfxMessage> rsp =
                        RfxMessage::obtainResponse(RIL_E_REQUEST_NOT_SUPPORTED, msg);
                responseToRilj(rsp);
                return true;
            }

            default:
                break;
        }
    }
    return false;
}

bool RtcCdmaSmsController::onCheckSimStatus(const sp<RfxMessage>& msg) {
    int state = getStatusManager()->getIntValue(RFX_STATUS_KEY_SIM_STATE);
    if ((state != RFX_SIM_STATE_READY) && (msg->getError() != RIL_E_SUCCESS)) {
        int msgId = msg->getId();
        switch (msgId) {
            case RFX_MSG_REQUEST_CDMA_DELETE_SMS_ON_RUIM:
            case RFX_MSG_REQUEST_CDMA_WRITE_SMS_TO_RUIM:
            case RFX_MSG_REQUEST_GET_SMS_RUIM_MEM_STATUS: {
                sp<RfxMessage> rsp = RfxMessage::obtainResponse(RIL_E_SIM_ABSENT, msg);
                responseToRilj(rsp);
                return true;
            }

            default:
                break;
        }
    }
    return false;
}

bool RtcCdmaSmsController::onHandleResponse(const sp<RfxMessage>& msg) {
    if (onCheckCdmaSupported(msg) || onCheckSimStatus(msg)) {
        return true;
    }
    int msgId = msg->getId();
    switch (msgId) {
        case RFX_MSG_REQUEST_IMS_SEND_CDMA_SMS:
        case RFX_MSG_REQUEST_IMS_SEND_CDMA_SMS_EX:
        case RFX_MSG_REQUEST_CDMA_SEND_SMS:
            handleMoSmsResponses(msg);
            break;

        case RFX_MSG_REQUEST_GET_SMSC_ADDRESS:
        case RFX_MSG_REQUEST_SET_SMSC_ADDRESS:
            handleSmscAdressResponses(msg);
            break;

        case RFX_MSG_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION:
        case RFX_MSG_REQUEST_CDMA_DELETE_SMS_ON_RUIM:
        case RFX_MSG_REQUEST_CDMA_WRITE_SMS_TO_RUIM:
        case RFX_MSG_REQUEST_GET_SMS_RUIM_MEM_STATUS:
        case RFX_MSG_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG:
        case RFX_MSG_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG:
        case RFX_MSG_REQUEST_CDMA_SMS_ACKNOWLEDGE:
        case RFX_MSG_REQUEST_CDMA_SMS_ACKNOWLEDGE_EX:
            return RfxController::onHandleResponse(msg);
        case RFX_MSG_REQUEST_CDMA_SMS_ACKNOWLEDGE_INTERNAL: {
                sp<RfxMessage> rsp = RfxMessage::obtainResponse(
                        RFX_MSG_REQUEST_SMS_ACKNOWLEDGE, msg);
                responseToRilj(rsp);
            }
            break;
        default:
            RFX_ASSERT(0);
            break;
    }
    return true;
}


bool RtcCdmaSmsController::onPreviewMessage(const sp<RfxMessage>& message) {

    switch (message->getId()) {
        case RFX_MSG_REQUEST_IMS_SEND_SMS:
        case RFX_MSG_REQUEST_IMS_SEND_SMS_EX:
        case RFX_MSG_REQUEST_CDMA_SEND_SMS: {
            int value = getStatusManager()->getIntValue(
                RFX_STATUS_KEY_CDMA_MO_SMS_STATE,CDMA_MO_SMS_SENT);
            if (value == CDMA_MO_SMS_SENDING && (message->getType() == REQUEST)) {
                return false;
            }
            break;
        }
    }
    return true;
}

bool RtcCdmaSmsController::onCheckIfResumeMessage(const sp<RfxMessage>& message) {
    switch (message->getId()) {
        case RFX_MSG_REQUEST_IMS_SEND_SMS:
        case RFX_MSG_REQUEST_IMS_SEND_SMS_EX:
        case RFX_MSG_REQUEST_CDMA_SEND_SMS: {
            int value = getStatusManager()->getIntValue(
                RFX_STATUS_KEY_CDMA_MO_SMS_STATE,CDMA_MO_SMS_SENT);
            if ( (value == CDMA_MO_SMS_SENT) && (message->getType() == REQUEST)) {
                return true;
            }
            break;
        }
    }
    return false;
}

bool RtcCdmaSmsController::onCheckIfRejectMessage(
        const sp<RfxMessage>& message, bool isModemPowerOff, int radioState) {
    int msgId = message->getId();
    if (!isModemPowerOff && (radioState == (int)RADIO_STATE_OFF) &&
            (msgId == RFX_MSG_REQUEST_GET_SMSC_ADDRESS ||
             msgId == RFX_MSG_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION ||
             msgId == RFX_MSG_REQUEST_CDMA_DELETE_SMS_ON_RUIM ||
             msgId == RFX_MSG_REQUEST_CDMA_WRITE_SMS_TO_RUIM ||
             msgId == RFX_MSG_REQUEST_GET_SMS_RUIM_MEM_STATUS ||
             msgId == RFX_MSG_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG ||
             msgId == RFX_MSG_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG ||
             msgId == RFX_MSG_REQUEST_SET_SMSC_ADDRESS )) {
        return false;
    }
    // Always send this request to MCL to reset the status value
    if (RFX_MSG_REQUEST_CDMA_SMS_ACKNOWLEDGE == msgId ||
        RFX_MSG_REQUEST_CDMA_SMS_ACKNOWLEDGE_EX == msgId ||
        RFX_MSG_REQUEST_CDMA_SMS_ACKNOWLEDGE_INTERNAL == msgId) {
        return false;
    }
    return RfxController::onCheckIfRejectMessage(message, isModemPowerOff, radioState);
}

bool RtcCdmaSmsController::previewMessage(const sp<RfxMessage>& message) {
    return onPreviewMessage(message);
}

bool RtcCdmaSmsController::checkIfResumeMessage(const sp<RfxMessage>& message) {
    return onCheckIfResumeMessage(message);
}
#include "RfxStringData.h"
bool RtcCdmaSmsController::onHandleNewSms(const sp<RfxMessage> & msg) {
    RIL_CDMA_SMS_Message *message = (RIL_CDMA_SMS_Message*)msg->getData()->getData();
    RtcCdmaSmsMessage *cdmaMessage = new RtcCdmaSmsMessage(message);
    RtcSmsMessage *convertedMessage = NULL;
    RtcImsSmsController *parentCtrl = (RtcImsSmsController *)getParent();
    RtcConCatSmsSender *sender = NULL;
    RtcConCatSmsGroup *group = NULL;
    bool ret = false;
    RtcConCatSmsRoot *root = NULL;
    bool needFree = true;
    if (parentCtrl != NULL) {
        root = parentCtrl->getConCatSmsRoot();
    }
    if (cdmaMessage != NULL && cdmaMessage->isConcatSms() && parentCtrl != NULL) {
        if (root != NULL) {
            sender = root->getSmsSender(
                    cdmaMessage->getSmsAddress()->getAddressString());
            if (sender != NULL) {
                group = sender->getSmsGroup(
                        cdmaMessage->getUserDataHeader()->getRefNumber(),
                        cdmaMessage->getUserDataHeader()->getMsgCount());
                if (group != NULL) {
                    RtcConCatSmsPart *part = group->getSmsPart(
                            cdmaMessage->getUserDataHeader()->getSeqNumber());
                    if (part != NULL) {
                        part->setFormat3Gpp(false);
                        part->setMessage(cdmaMessage);
                        convertedMessage = part->getConvertedMessage();
                        needFree = false;
                    }
                    group->updateTimeStamp();
                }
            }
        }
    }
    if (convertedMessage == NULL || convertedMessage->isError()) {
        ret = false;
    } else {
        if (parentCtrl != NULL) {
            // send the converted Message
            parentCtrl->sendGsmSms((RtcGsmSmsMessage *)convertedMessage);
            ret = true;
        }
    }
    if (root != NULL) {
        root->cleanUpObj();
    }
    if (needFree) {
        delete cdmaMessage;
    }
    return ret;
}

void RtcCdmaSmsController::handleCdmaSmsAck(const sp<RfxMessage>& message) {
    bool onGoing = getStatusManager()->getBoolValue(
            RFX_STATUS_KEY_SMS_FORMAT_CHANGE_ONGOING);
    if (onGoing) {
        getStatusManager()->setBoolValue(
                RFX_STATUS_KEY_SMS_FORMAT_CHANGE_ONGOING, false);
        RIL_CDMA_SMS_Ack *ack = (RIL_CDMA_SMS_Ack*)message->getData()->getData();
        int success;
        int cause;
        if (ack->uErrorClass == RIL_CDMA_SMS_NO_ERROR) {
            success = 1;
            cause = 0;
        } else {
            success = 0;
            cause = 0xFF;
        }
        RtcImsSmsController *parentCtrl = (RtcImsSmsController *)getParent();
        if (parentCtrl != NULL) {
            parentCtrl->sendGsmSmsAck(success, cause, message);
        }
    } else {
        requestToMcl(message);
    }
}

void RtcCdmaSmsController::sendCdmaSms(RtcCdmaSmsMessage * msg) {
    sp<RfxMessage> urc = RfxMessage::obtainUrc(getSlotId(), RFX_MSG_URC_CDMA_NEW_SMS,
        RmcCdmaMtSmsMessage((void *)&msg->getMessage(), sizeof(RIL_CDMA_SMS_Message)));
    responseToRilj(urc);
    getStatusManager()->setBoolValue(
            RFX_STATUS_KEY_SMS_FORMAT_CHANGE_ONGOING, true);
}

void RtcCdmaSmsController::sendCdmaSmsAck(const sp<RfxMessage>& message) {
    if (m_timer != NULL) {
        logD(TAG, "Stop MT SMS ack timer");
        RfxTimer::stop(m_timer);
        m_timer = NULL;
    }

    RIL_CDMA_SMS_Ack ackCdma;
    int *ackGsm = (int *)message->getData()->getData();
    if ((ackGsm != NULL) && (ackGsm[0] == 1)) {
        ackCdma.uErrorClass = RIL_CDMA_SMS_NO_ERROR;
        ackCdma.uSMSCauseCode = 0;
    } else {
        ackCdma.uErrorClass = RIL_CDMA_SMS_ERROR;
        ackCdma.uSMSCauseCode = 39;
    }
    sp<RfxMessage> newMsg = RfxMessage::obtainRequest(
            RFX_MSG_REQUEST_CDMA_SMS_ACKNOWLEDGE_INTERNAL,
            RmcCdmaSmsAck((void *)&ackCdma, sizeof(ackCdma)), message, false);
    requestToMcl(newMsg);
}

bool RtcCdmaSmsController::isSupportSmsFormatConvert() {
    RtcImsSmsController *parentCtrl = (RtcImsSmsController *)getParent();
    if (parentCtrl != NULL) {
        return parentCtrl->isSupportSmsFormatConvert();
    }
    return false;
}

bool RtcCdmaSmsController::isCdmaPhoneMode() {
    int preferedNwType = getStatusManager()->getIntValue(RFX_STATUS_KEY_PREFERRED_NW_TYPE, -1);
    int nwsMode = getStatusManager()->getIntValue(RFX_STATUS_KEY_NWS_MODE, -1);
    int tech = RADIO_TECH_UNKNOWN;
    logD(TAG, "isCdmaPhoneMode:%d, %d", preferedNwType, nwsMode);
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




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
#include "RpCdmaSmsCtrl.h"
#include "RpCdmaSmsLocalReqRsp.h"
#include "util/RpFeatureOptionUtils.h"
#include <cutils/jstring.h>
#include "nw/RpNwDefs.h"
#include "RpSmsCtrl.h"

/*****************************************************************************
 * Class RpCdmaSmsCtrl
 * Control the flow of 3GPP2 SMS over IMS.
 *****************************************************************************/
RFX_IMPLEMENT_CLASS("RpCdmaSmsCtrl", RpCdmaSmsCtrl, RpSmsCtrlBase);

RpCdmaSmsCtrl::RpCdmaSmsCtrl() :
    mCodec(NULL),
    mIsHandlingImsMoSms(false),
    mIsHandlingImsMtSms(false),
    mIsHandlingCsMtSms(false),
    mMtSmsSeqId(-1),
    mNeedStatusReport(false) {

    setName(String8("RpCdmaSmsCtrl"));
    memset(&mMtSmsAddress, 0, sizeof(RIL_CDMA_SMS_Address));
}

void RpCdmaSmsCtrl::onInit() {
    // Required: invoke super class implementation
    RpSmsCtrlBase::onInit();

    log(String8("onInit"));

    // Create the codec to encode/decode the class to pdu and visa verse
    RFX_OBJ_CREATE(mCodec, RpCdmaSmsCodec, this);

    // Start to listen the IMS send SMS requests
    const int request_id_list[] = {
        RIL_REQUEST_CDMA_SEND_SMS,
        RIL_REQUEST_GET_SMSC_ADDRESS,
        RIL_REQUEST_SET_SMSC_ADDRESS,
        RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE,
        RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE_EX,
        RIL_LOCAL_REQUEST_CDMA_SMS_SPECIFIC_TO_GSM,
        RIL_REQUEST_CDMA_WRITE_SMS_TO_RUIM,
        RIL_REQUEST_CDMA_DELETE_SMS_ON_RUIM
    };
    const int urc_id_list[] = {
        RIL_UNSOL_RESPONSE_CDMA_NEW_SMS,
        RIL_LOCAL_GSM_UNSOL_CDMA_SMS_SPECIFIC_FROM_GSM
    };
    /**
     * Only register if CDMA2000 supports.
     * If CDMA2000 doesn't support, we just by-pass this request to GSM rild to handle.
     */
    if (RpFeatureOptionUtils::isC2kSupport()) {
        registerToHandleRequest(request_id_list, (sizeof(request_id_list)/sizeof(int)));
        registerToHandleUrc(urc_id_list, (sizeof(urc_id_list)/sizeof(int)));
    }
}

bool RpCdmaSmsCtrl::onHandleRequest(const sp<RfxMessage>& message) {
    log(String8::format("onHandleRequest: request: %d, slot %d", message->getId(), getSlotId()));
    bool result = false;

    switch (message->getId()) {
        case RIL_REQUEST_CDMA_SEND_SMS:
            onCdmaSmsSend(message);
            return true;

        case RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE:
        case RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE_EX:
            result = handleCdmaSmsAck(message);
            break;

        case RIL_REQUEST_GET_SMSC_ADDRESS:
        case RIL_REQUEST_SET_SMSC_ADDRESS:
            onHandleSmscAddress(message);
            return true;

        case RIL_REQUEST_CDMA_WRITE_SMS_TO_RUIM:
        case RIL_REQUEST_CDMA_DELETE_SMS_ON_RUIM:
            onHandleRuimSms(message);
            return true;

        default:
            result = RpSmsCtrlBase::onHandleRequest(message);
            break;
    }

    return result;
}

bool RpCdmaSmsCtrl::onHandleResponse(const sp<RfxMessage>& message) {
    bool result = false;

    switch (message->getId()) {
        case RIL_LOCAL_REQUEST_CDMA_SMS_SPECIFIC_TO_GSM:
            result = handleCdmaSmsSpecificToGsmRsp(message);
            break;

        case RIL_REQUEST_CDMA_SEND_SMS:
            getStatusManager()->setIntValue(
                    RFX_STATUS_KEY_CDMA_MO_SMS_STATE, CDMA_MO_SMS_SENT);
            result = RpSmsCtrlBase::onHandleResponse(message);
            break;

        default:
            result = RpSmsCtrlBase::onHandleResponse(message);
            break;
    }

    return result;
}

void RpCdmaSmsCtrl::onHandleSmscAddress(const sp<RfxMessage>& message) {
    RILD_RadioTechnology_Group group = RADIO_TECH_GROUP_GSM;
    int cdma_card_type = getStatusManager()->getIntValue(RFX_STATUS_KEY_CDMA_CARD_TYPE);
    if (cdma_card_type == CT_3G_UIM_CARD
            || cdma_card_type == CT_UIM_SIM_CARD
            || cdma_card_type == UIM_CARD
            || cdma_card_type == UIM_SIM_CARD) {
        int nwMode = getStatusManager(getSlotId())->getIntValue(
                RFX_STATUS_KEY_NWS_MODE, NWS_MODE_CSFB);
        if (nwMode == NWS_MODE_CDMALTE) {
            group = RADIO_TECH_GROUP_C2K;
        }
    }
    requestToRild(RfxMessage::obtainRequest(group, message->getId(), message, true));
}

void RpCdmaSmsCtrl::onHandleRuimSms(const sp<RfxMessage>& message) {
    int state = getStatusManager()->getIntValue(RFX_STATUS_KEY_SIM_STATE);
    if (state != RFX_SIM_STATE_READY) {
        sp<RfxMessage> rsp = RfxMessage::obtainResponse(RIL_E_SIM_ABSENT, message);
        responseToRilj(rsp);
        return;
    }
    requestToRild(message);
}

bool RpCdmaSmsCtrl::onHandleUrc(const sp<RfxMessage>& message) {
    bool result = false;

    switch (message->getId()) {
        case RIL_UNSOL_RESPONSE_CDMA_NEW_SMS:
            result = handleCdmaNewSms(message);
            break;

        case RIL_LOCAL_GSM_UNSOL_CDMA_SMS_SPECIFIC_FROM_GSM:
            result = handleCdmaSmsSpecificToGsmUrc(message);
            break;

        default:
            result = RpSmsCtrlBase::onHandleUrc(message);
            break;
    }

    return result;
}

int RpCdmaSmsCtrl::isPreviewedInternal(const sp<RfxMessage>& message) {
    int result = RIL_REQUEST_PREVIEW_REASON_NOT_PREVIEW;

    switch (message->getId()) {
        case RIL_REQUEST_IMS_SEND_SMS:
        case RIL_REQUEST_IMS_SEND_SMS_EX:
            if ((message->getType() == REQUEST) && is3gpp2ImsReq(message)) {
                if (!onPreviewMessage(message)) {
                    result = RIL_REQUEST_PREVIEW_REASON_PREVIEWED;
                }
            }

        default:
            break;
    }

    return result;
}

int RpCdmaSmsCtrl::isHandledInternal(const sp <RfxMessage>& message) {
    int result = RIL_REQUEST_HANDLE_REASON_NOT_HANDLE;

    switch (message->getId()) {
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

int RpCdmaSmsCtrl::isResumedInternal(const sp<RfxMessage>& message) {
    int result = RIL_REQUEST_RESUME_REASON_NOT_RESUME;

    switch (message->getId()) {
        case RIL_REQUEST_IMS_SEND_SMS:
        case RIL_REQUEST_IMS_SEND_SMS_EX:
            if ((message->getType() == REQUEST) && is3gpp2ImsReq(message)) {
                if (onCheckIfResumeMessage(message)) {
                    result = RIL_REQUEST_RESUME_REASON_RESUMED;
                }
            }

        default:
            break;
    }

    return result;
}

bool RpCdmaSmsCtrl::handleImsSendSms(const sp <RfxMessage>& message) {
    bool result = false;

    if (is3gpp2ImsReq(message)) {
        // Need to transfer to RIL_LOCAL_REQUEST_CDMA_SMS_SPECIFIC_TO_GSM and send to GSM rild
        handleImsSendCdmaSms(message);
        result = true;
    }

    return result;
}

bool RpCdmaSmsCtrl::handleImsSendCdmaSms(const sp <RfxMessage>& message) {
    int32_t  t;
    uint8_t retry;
    int32_t messageRef;
    Parcel *imsSendSmsParcel;
    bool result = false;

    imsSendSmsParcel = message->getParcel();
    imsSendSmsParcel->readInt32(&t);  // read radio tech, it should be 3GPP2
    imsSendSmsParcel->read(&retry,sizeof(retry));  // read the retry value
    imsSendSmsParcel->read(&messageRef,sizeof(messageRef));  // read message reference

    // construct as class
    RIL_CDMA_SMS_Message rcsm;
    String8 pduAtCmd;
    mCodec->decodeRilCdmaSmsMsg(imsSendSmsParcel, &rcsm);
    pduAtCmd = mCodec->encodeC2kcmgs(&rcsm);

    if (pduAtCmd == String8("")) {
        // Error response to Rilj
        log(String8("Pdu encode error"));
        genericErrorRspToRilj(message);
        result = false;
    } else {
        if (message->getId() == RIL_REQUEST_IMS_SEND_SMS_EX) {
            mNeedStatusReport = RpCdmaSmsConverter::isNeedStatusReport(&rcsm);
        }
        sp<RfxMessage> localReq = RfxMessage::obtainRequest(RADIO_TECH_GROUP_GSM,
                RIL_LOCAL_REQUEST_CDMA_SMS_SPECIFIC_TO_GSM, message);
        Parcel *p = localReq->getParcel();
        // 3 strings
        p->writeInt32(3);
        // User data for this local request and the response will take back in the beginning
        p->writeString16(String16(CDMA_SMS_LOCAL_USER_DATA_MO_SMS));
        // Request at comamnds, ex, AT+C2KCMGS=""
        p->writeString16(String16(pduAtCmd));
        // Wait response string
        p->writeString16(String16("+C2KCMGS"));
        mIsHandlingImsMoSms = true;
        requestToRild(localReq);
        result = true;
        getStatusManager()->setIntValue(
            RFX_STATUS_KEY_CDMA_MO_SMS_STATE, CDMA_MO_SMS_SENDING);
    }

    return result;
}

bool RpCdmaSmsCtrl::handleCdmaSmsAck(const sp <RfxMessage>& message) {
    bool result = false;
    log(String8::format("mIsHandlingCsMtSms: %d, mIsHandlingImsMtSms %d, SeqId: %d, Address: %s",
            mIsHandlingCsMtSms, mIsHandlingImsMtSms, mMtSmsSeqId, ""));
    // check if 1xRTT ack
    if (mIsHandlingCsMtSms) {
        // SMS framework ack to c2k rild, by-pass it directly
        mIsHandlingCsMtSms = false;
        requestToRild(message);
        return true;
    }

    // check if ims ack
    if (mIsHandlingImsMtSms == false) {
        // No CS/IMS MT coming, response fail it directly
        sp<RfxMessage> rsp = RfxMessage::obtainResponse(RIL_E_NO_SMS_TO_ACK, message);
        responseToRilj(rsp);
        return true;
    }

    // No need to ack to network, return success always.
    if (mMtSmsSeqId == -1) {
        // There is a IMS MT but no need to ack to network. Reset as false.
        mIsHandlingImsMtSms = false;
        sp<RfxMessage> rsp = RfxMessage::obtainResponse(RIL_E_SUCCESS, message);
        responseToRilj(rsp);
        return true;
    }

    RIL_CDMA_SMS_Ack rcsa;
    mCodec->decodeRilCdmaSmsAckMsg(message->getParcel(), &rcsa);
    String8 atCmd = mCodec->encodeRilCdmaSmsAck(&rcsa, mMtSmsSeqId, &mMtSmsAddress);
    if (atCmd == String8("")) {
        log(String8("Encode Ack to AT+C2KCNMA failed"));
        genericErrorRspToRilj(message);
        result = false;
    } else {
        sp<RfxMessage> localReq = RfxMessage::obtainRequest(RADIO_TECH_GROUP_GSM,
                RIL_LOCAL_REQUEST_CDMA_SMS_SPECIFIC_TO_GSM, message);
        Parcel *p = localReq->getParcel();
        // 3 strings
        p->writeInt32(3);
        // User data for this local request and the response will take back in the beginning
        p->writeString16(String16(CDMA_SMS_LOCAL_USER_DATA_MT_CNMA));
        // Request at comamnds, ex, AT+CNMA=""
        p->writeString16(String16(atCmd));
        // No need to wait any special response
        p->writeString16(String16(""));
        requestToRild(localReq);
        result = true;
    }

    return result;
}

bool RpCdmaSmsCtrl::handleCdmaSmsSpecificToGsmRsp(const sp<RfxMessage>& message) {
    if (message->getSource() != RADIO_TECH_GROUP_GSM) {
        // ignore
        log(String8("Source is not from GSM"));
        return false;
    }

    // Extract the result
    RpCdmaSmsLocalReqRsp localReqRsp = mCodec->decodelocalReq(message);
    log(String8::format("mIsHandlingImsMoSms: %d, mIsHandlingImsMtSms: %d, mIsHandlingCsMtSms: %d",
            mIsHandlingImsMoSms, mIsHandlingImsMtSms, mIsHandlingCsMtSms));
    log(localReqRsp.toString());
    if (mIsHandlingImsMoSms && localReqRsp.getType() == CDMA_SMS_LOCAL_REQ_CMGS_RSP) {
        sp<RfxMessage> imsSmsRsp;
        if (localReqRsp.getResult()) {
            // Success
            imsSmsRsp = RfxMessage::obtainResponse(RIL_E_SUCCESS, message);
        } else {
            // failed
            imsSmsRsp = RfxMessage::obtainResponse(RIL_E_SYSTEM_ERR, message);
        }
        Parcel *p = imsSmsRsp->getParcel();
        p->writeInt32(localReqRsp.getId());
        p->writeString16(String16(localReqRsp.getAckPdu()));
        p->writeInt32(localReqRsp.getError());
        responseToRilj(imsSmsRsp);
        mIsHandlingImsMoSms = false;
        log(String8::format("Need Status report %d, messageId %d",
                mNeedStatusReport, localReqRsp.getId()));
        if (mNeedStatusReport && (localReqRsp.getError() == RIL_E_SUCCESS) &&
                localReqRsp.getId() > 0) {
            ((RpSmsCtrl *)getParent())->addReferenceId(localReqRsp.getId());
            mNeedStatusReport = false;
        }
        getStatusManager()->setIntValue(
                RFX_STATUS_KEY_CDMA_MO_SMS_STATE, CDMA_MO_SMS_SENT);
    } else if (mIsHandlingImsMtSms && localReqRsp.getType() == CDMA_SMS_LOCAL_REQ_CNMA_RSP) {
        sp<RfxMessage> cnmaRsp = RfxMessage::obtainResponse((RIL_Errno)localReqRsp.getError(),
                message);
        responseToRilj(cnmaRsp);
        mIsHandlingImsMtSms = false;
    } else if (mIsHandlingCsMtSms && localReqRsp.getType() == CDMA_SMS_LOCAL_REQ_CNMA_RSP) {
        log(String8("CS MT is handling and reject the IMS MT"));
    } else {
        log(String8("No IMS MO, IMS MT and CS MT handling"));
    }

    return true;
}

bool RpCdmaSmsCtrl::handleCdmaNewSms(const sp<RfxMessage>& message) {
    if (mIsHandlingImsMtSms) {
        // reject the sms directly
        sp<RfxMessage> ack = RfxMessage::obtainRequest(RADIO_TECH_GROUP_C2K,
                RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE, message);
        Parcel *p = ack->getParcel();
        p->writeInt32(RP_CDMA_SMS_CTRL_ACK_ERROR_CLASS_TEMP_ERROR);
        p->writeInt32(RP_CDMA_SMS_CTRL_ACK_ERROR_CAUSE_DESTINATION_BUSY);
        requestToRild(ack);
        // No need to handle the respnose.
        // Try to pass to rilj will fail because the token is non-exist.
    } else {
        RIL_CDMA_SMS_Message rcsm;
        int messageId = -1;
        mCodec->decodeRilCdmaSmsMsg(message->getParcel(), &rcsm);
        bool statusReport = RpCdmaSmsConverter::isStatusReport(&rcsm, &messageId);
        log(String8::format("status report = %d, cache size = %d", statusReport,
                ((RpSmsCtrl* )getParent())->getCacheSize()));
        if (statusReport &&
                ((RpSmsCtrl *)getParent())->removeReferenceIdCached(messageId)) {
            sp<RfxMessage> urcMsg = RfxMessage::obtainUrc(message->getSlotId(),
                    RIL_UNSOL_RESPONSE_CDMA_NEW_SMS_EX, message, true);
            responseToRilj(urcMsg);
        } else {
            // Pass to sms framework
            responseToRilj(message);
        }
        mIsHandlingCsMtSms = true;
    }

    return true;
}

bool RpCdmaSmsCtrl::handleCdmaSmsSpecificToGsmUrc(const sp<RfxMessage>& message) {
    bool result = false;

    // decode the urc raw data
    RIL_CDMA_SMS_Message rcsm;
    RpCdmaSmsMtUrc urcResult = mCodec->decodeC2kcmtUrc(message->getParcel(), &rcsm);
    if (mIsHandlingCsMtSms) {
        // reject directly
        sp<RfxMessage> ack = RfxMessage::obtainRequest(RADIO_TECH_GROUP_GSM,
                RIL_LOCAL_REQUEST_CDMA_SMS_SPECIFIC_TO_GSM, message);
        RIL_CDMA_SMS_Ack errorAck;
        errorAck.uErrorClass = (RIL_CDMA_SMS_ErrorClass) 2;  // Temp error
        errorAck.uSMSCauseCode = 33;  // Desination busy
        String8 atCmd = mCodec->encodeRilCdmaSmsAck(&errorAck, urcResult.getSeqId(),
                urcResult.getAddress());
        Parcel *p = ack->getParcel();
        // 3 strings
        p->writeInt32(3);
        // User data for this local request and the response will take back in the beginning
        p->writeString16(String16(CDMA_SMS_LOCAL_USER_DATA_MT_CNMA));
        // Request at comamnds, ex, AT+CNMA=""
        p->writeString16(String16(atCmd));
        // No need to wait any special response
        p->writeString16(String16(""));
        requestToRild(ack);
        return true;
    }
    int messageId = -1;
    int urcId = RIL_UNSOL_RESPONSE_CDMA_NEW_SMS;
    bool statusReport = RpCdmaSmsConverter::isStatusReport(&rcsm, &messageId);
    log(String8::format("status report = %d, cache size = %d", statusReport,
            ((RpSmsCtrl* )getParent())->getCacheSize()));
    if (statusReport &&
            ((RpSmsCtrl *)getParent())->removeReferenceIdCached(messageId)) {
        urcId = RIL_UNSOL_RESPONSE_CDMA_NEW_SMS_EX;
    }

    sp<RfxMessage> urcMsg = RfxMessage::obtainUrc(message->getSlotId(),
            urcId);

    urcResult.setResult(mCodec->encodeRilCdmaSmsMsg(&rcsm, urcMsg->getParcel()));
    if (urcResult.getResult()) {
        responseToRilj(urcMsg);
        mIsHandlingImsMtSms = true;
        mMtSmsSeqId = urcResult.getSeqId();
        memcpy(&mMtSmsAddress, urcResult.getAddress(),
                sizeof(mMtSmsAddress));
        result = true;
    } else {
        log(String8::format("Error to decode, ignore this urc"));
        mIsHandlingImsMtSms = false;
        mMtSmsSeqId = -1;
        memset(&mMtSmsAddress, 0, sizeof(mMtSmsAddress));
        result = false;
    }

    return result;
}

void RpCdmaSmsCtrl::genericErrorRspToRilj(const sp <RfxMessage>& message) {
    sp<RfxMessage> rsp = RfxMessage::obtainResponse(RIL_E_SYSTEM_ERR, message);
    responseToRilj(rsp);
}

bool RpCdmaSmsCtrl::is3gpp2ImsReq(const sp<RfxMessage>& message) {
    Parcel *imsSendSmsParcel;
    RIL_RadioTechnologyFamily radioTech;
    size_t parcelDataPos;
    bool result = false;

    if ((message->getId() != RIL_REQUEST_IMS_SEND_SMS) &&
            (message->getId() != RIL_REQUEST_IMS_SEND_SMS_EX)) {
        return false;
    }

    imsSendSmsParcel = message->getParcel();
    parcelDataPos = imsSendSmsParcel->dataPosition();
    radioTech = (RIL_RadioTechnologyFamily) imsSendSmsParcel->readInt32();
    // reset the data position to the beginning
    imsSendSmsParcel->setDataPosition(parcelDataPos);
    if (radioTech == RADIO_TECH_3GPP2) {
        result = true;
    }

    return result;
}

bool RpCdmaSmsCtrl::isCtSimCard(void) {
    bool ret = false;
    int cdma_card_type = getStatusManager()->getIntValue(RFX_STATUS_KEY_CDMA_CARD_TYPE);
    if (cdma_card_type == CT_4G_UICC_CARD ||
            cdma_card_type == CT_UIM_SIM_CARD ||
            cdma_card_type == CT_3G_UIM_CARD) {
        ret = true;
    }
    return ret;
}

void RpCdmaSmsCtrl::onCdmaSmsSend(const sp<RfxMessage>& message) {
    if (isCtSimCard()) {
        Parcel *p = message->getParcel();
        int32_t teleServiceId = p->readInt32();
        /*
         * According to the spec of China Telecom, it needs the teleservice
         * id of long sms is TELESERVICE_WMT(0x1002). However, AOSP will use
         * TELESERVICE_WEMT(0x1005) as the teleservice id in SmsMessage.java.
         * In fact, most of China Telecom's network will work fine for 0x1005.
         * Only in Senzhen we will meet the problem that the message center cannot
         * handle the teleservice id 0x1005, so we need to convert the teleservice
         * id from 0x1005 to 0x1002 to make sure the long sms can work in all China
         * Telecom's network.
         */
        if (teleServiceId == TELESERVICE_WEMT) {
            message->resetParcelDataStartPos();
            p->writeInt32(TELESERVICE_WMT);
        }
    }
    requestToRild(message);
    getStatusManager()->setIntValue(
        RFX_STATUS_KEY_CDMA_MO_SMS_STATE, CDMA_MO_SMS_SENDING);
}

bool RpCdmaSmsCtrl::onPreviewMessage(const sp<RfxMessage>& message) {
    switch (message->getId()) {
        case RIL_REQUEST_IMS_SEND_SMS:
        case RIL_REQUEST_IMS_SEND_SMS_EX:
        case RIL_REQUEST_CDMA_SEND_SMS: {
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

bool RpCdmaSmsCtrl::onCheckIfResumeMessage(const sp<RfxMessage>& message) {
    switch (message->getId()) {
        case RIL_REQUEST_IMS_SEND_SMS:
        case RIL_REQUEST_IMS_SEND_SMS_EX:
        case RIL_REQUEST_CDMA_SEND_SMS: {
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

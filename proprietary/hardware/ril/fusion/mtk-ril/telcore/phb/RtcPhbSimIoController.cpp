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
#include "RtcPhbSimIoController.h"
#include "RfxVoidData.h"
#include "RfxMessageId.h"
#include "RfxIntsData.h"
#include "rfx_properties.h"
#include "RfxRilUtils.h"
#include <libmtkrilutils.h>

#define PHBSIMIO_LOG_TAG "RtcPhbSimIo"

#define DlogD(x...) if (mIsEngLoad == 1) logD( x )

static const int USIM_TYPE1_TAG = 0xA8;
static const int USIM_TYPE2_TAG = 0xA9;
static const int USIM_TYPE3_TAG = 0xAA;

using ::android::String8;

/*****************************************************************************
 * Class RtcPhbSimIoController
 *****************************************************************************/

RFX_IMPLEMENT_CLASS("RtcPhbSimIoController", RtcPhbSimIoController, RfxController);


RtcPhbSimIoController::RtcPhbSimIoController() {
}

RtcPhbSimIoController::~RtcPhbSimIoController() {
}

void RtcPhbSimIoController::onInit() {
    // Required: invoke super class implementation
    RfxController::onInit();
    logD(PHBSIMIO_LOG_TAG, "[%s]", __FUNCTION__);

    const int request1[] = {
        RFX_MSG_REQUEST_PHB_SIM_IO,
        RFX_MSG_REQUEST_PHB_PBR_SIM_IO,
    };
    registerToHandleRequest(request1, sizeof(request1)/sizeof(int));
    int i = 0;
    for (i = 0; i < PBR_FILE_LENGTH; i++) {
        pbrFile[i] = 0xFF;
    }
    // register callbacks to get card type change event
    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_CARD_TYPE,
            RfxStatusChangeCallback(this, &RtcPhbSimIoController::onCardTypeChanged));
    mIsEngLoad = RfxRilUtils::isEngLoad();
}

bool RtcPhbSimIoController::onCheckIfRejectMessage(const sp<RfxMessage>& message,
        bool isModemPowerOff, int radioState) {
    int msgId = message->getId();
    if ((radioState == (int)RADIO_STATE_OFF) && (msgId == RFX_MSG_REQUEST_PHB_SIM_IO)) {
        DlogD(PHBSIMIO_LOG_TAG, "onCheckIfRejectMessage, id = %d, isModemPowerOff = %d, radioState = %d",
                msgId, isModemPowerOff, radioState);
        return false;
    }
    return RfxController::onCheckIfRejectMessage(message, isModemPowerOff, radioState);
}

int RtcPhbSimIoController::parsePbrFileId(char *hex, int length) {
    int i = 0;
    char *tempStr = hex;
    int tag = 0;
    int len = 0;
    int value = 0;
    int current = 0;
    int inTag = 0;
    int inLen = 0;
    int inValue = 0;
    int j = 0;
    DlogD(PHBSIMIO_LOG_TAG, "parsePbrFileId length = %d", length);
    for (i = 0; i < length && hex[i] != '\0';) {
        tag = hexCharToDecInt(tempStr, 2);
        if (tag == USIM_TYPE1_TAG || tag == USIM_TYPE2_TAG || tag == USIM_TYPE3_TAG) {
            tempStr = tempStr + 2;
            len = hexCharToDecInt(tempStr, 2);
            tempStr = tempStr + 2;
            current = i + 4;
        } else {
            logD(PHBSIMIO_LOG_TAG, "parsePbrFileId tag break tag = %d", tag);
            break;
        }
        i = current + len * 2;
        DlogD(PHBSIMIO_LOG_TAG, "parsePbrFileId tag = %d, len = %d, current = %d,i = %d",
                tag, len, current, i);
        for (j = current; j < i;) {
            inTag = hexCharToDecInt(tempStr, 2);
            tempStr = tempStr + 2;
            inLen = hexCharToDecInt(tempStr, 2);
            tempStr = tempStr + 2;
            inValue = hexCharToDecInt(tempStr, 4);
            if (false == addFileId(inValue)) {

            }
            tempStr = tempStr + inLen * 2; // move to next tag
            j = j + inLen * 2 + 4;
            DlogD(PHBSIMIO_LOG_TAG, "parsePbrFileId inTag = %d, inValue = %d, j = %d, inLen = %d",
                    inTag, inValue, j, inLen);
        }
    }
    return i;
}

bool RtcPhbSimIoController::addFileId(int fileId) {
    int i = 0;
    bool retValue = false;
    for (i = 0; i < PBR_FILE_LENGTH && pbrFile[i] != 0xFF; i++) {
        if (fileId == pbrFile[i]) {
            retValue = true;
            break;
        }
    }
    if (false == retValue && i < PBR_FILE_LENGTH) {
        pbrFile[i] = fileId;
    }
    DlogD(PHBSIMIO_LOG_TAG, "addFileId retValue = %d, i = %d, fileId = %d", retValue, i, fileId);
    return retValue;
}

bool RtcPhbSimIoController::onCheckIfPhbRequest(int fileId) {
    int i = 0;
    bool retValue = false;
    for (i = 0; i < PBR_FILE_LENGTH && pbrFile[i] != 0xFF; i++) {
        if (fileId == pbrFile[i]) {
            retValue = true;
            break;
        }
    }
    return retValue;
}

int RtcPhbSimIoController::hexCharToDecInt(char *hex, int length) {
    int i = 0;
    int value, digit;

    for (i = 0, value = 0; i < length && hex[i] != '\0'; i++) {
        if (hex[i] >= '0' && hex[i] <= '9') {
            digit = hex[i] - '0';
        }
        else if (hex[i] >= 'A' && hex[i] <= 'F') {
            digit = hex[i] - 'A' + 10;
        }
        else if (hex[i] >= 'a' && hex[i] <= 'f') {
            digit = hex[i] - 'a' + 10;
        }
        else {
            return -1;
        }
        value = value*16 + digit;
    }
    return value;
}

bool RtcPhbSimIoController::onHandleResponse(const sp<RfxMessage>& message) {
    int msgId = message->getId();
    DlogD(PHBSIMIO_LOG_TAG, "onHandleResponse, handle %s", RFX_ID_TO_STR(msgId));
    if(msgId == RFX_MSG_REQUEST_PHB_SIM_IO) {
        sp<RfxMessage> rsp = RfxMessage::obtainResponse(RFX_MSG_REQUEST_SIM_IO, message);
        responseToRilj(rsp);
    } else if (msgId == RFX_MSG_REQUEST_PHB_PBR_SIM_IO) {
        RIL_SIM_IO_Response *pData = (RIL_SIM_IO_Response*)(message->getData()->getData());
        if (pData != NULL && pData->simResponse != NULL) {
            parsePbrFileId(pData->simResponse, strlen(pData->simResponse));
        }
        sp<RfxMessage> rsp = RfxMessage::obtainResponse(RFX_MSG_REQUEST_SIM_IO, message);
        responseToRilj(rsp);
    } else {
        responseToRilj(message);
    }
    return true;
}

void RtcPhbSimIoController::onCardTypeChanged(RfxStatusKeyEnum key,
    RfxVariant oldValue, RfxVariant newValue) {
    RFX_UNUSED(key);
    if (oldValue.asInt() != newValue.asInt()) {
        if ((newValue.asInt() == 0)) {
            // When SIM plugged out, hang up the mormal call directly.
            logD(PHBSIMIO_LOG_TAG, "SIM plug out, clear store PBR file");
            int i = 0;
            for (i = 0; i < PBR_FILE_LENGTH; i++) {
                pbrFile[i] = 0xFF;
            }
        }
    }
}

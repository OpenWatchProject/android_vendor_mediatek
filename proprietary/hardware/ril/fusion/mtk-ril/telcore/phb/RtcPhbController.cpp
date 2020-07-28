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
#include "RtcPhbController.h"
#include "RfxVoidData.h"
#include "RfxMessageId.h"
#include "RfxIntsData.h"
#include "rfx_properties.h"
#include <libmtkrilutils.h>

#define RFX_LOG_TAG "RtcPhb"
#define PROPERTY_RIL_PHB_READY "vendor.gsm.sim.ril.phbready"

using ::android::String8;

/*****************************************************************************
 * Class RtcPhbController
 *****************************************************************************/

RFX_IMPLEMENT_CLASS("RtcPhbController", RtcPhbController, RfxController);


RtcPhbController::RtcPhbController() {
}

RtcPhbController::~RtcPhbController() {
}

void RtcPhbController::onInit() {
    // Required: invoke super class implementation
    RfxController::onInit();
    logD(RFX_LOG_TAG, "[%s]", __FUNCTION__);

    const int request1[] = {
        RFX_MSG_REQUEST_QUERY_PHB_STORAGE_INFO,
        RFX_MSG_REQUEST_WRITE_PHB_ENTRY,
        RFX_MSG_REQUEST_READ_PHB_ENTRY,
        RFX_MSG_REQUEST_QUERY_UPB_CAPABILITY,
        RFX_MSG_REQUEST_EDIT_UPB_ENTRY,
        RFX_MSG_REQUEST_DELETE_UPB_ENTRY,
        RFX_MSG_REQUEST_READ_UPB_GAS_LIST,
        RFX_MSG_REQUEST_READ_UPB_GRP,
        RFX_MSG_REQUEST_WRITE_UPB_GRP,
        RFX_MSG_REQUEST_GET_PHB_STRING_LENGTH,
        RFX_MSG_REQUEST_GET_PHB_MEM_STORAGE,
        RFX_MSG_REQUEST_SET_PHB_MEM_STORAGE,
        RFX_MSG_REQUEST_READ_PHB_ENTRY_EXT,
        RFX_MSG_REQUEST_WRITE_PHB_ENTRY_EXT,
        RFX_MSG_REQUEST_QUERY_UPB_AVAILABLE,
        RFX_MSG_REQUEST_READ_EMAIL_ENTRY,
        RFX_MSG_REQUEST_READ_SNE_ENTRY,
        RFX_MSG_REQUEST_READ_ANR_ENTRY,
        RFX_MSG_REQUEST_READ_UPB_AAS_LIST,
        RFX_MSG_REQUEST_SET_PHONEBOOK_READY,
    };
    registerToHandleRequest(request1, sizeof(request1)/sizeof(int));

    // register callbacks to md power off event
    getNonSlotScopeStatusManager()->registerStatusChanged(RFX_STATUS_KEY_MODEM_POWER_OFF,
        RfxStatusChangeCallback(this, &RtcPhbController::onMdOffChanged));
}

void RtcPhbController::onMdOffChanged(RfxStatusKeyEnum key,
    RfxVariant oldValue, RfxVariant newValue) {
    RFX_UNUSED(key);
    RFX_UNUSED(oldValue);
    logI(RFX_LOG_TAG, "[%s] newValue %d", __FUNCTION__, newValue.asBool());
    if (newValue.asBool()) {
        sp<RfxMessage> request = RfxMessage::obtainRequest(m_slot_id,
                RFX_MSG_REQUEST_PHB_CURRENT_STORAGE_RESET, RfxVoidData());
        requestToMcl(request);

        int isPhbReady = 0;
        setMSimProperty(m_slot_id, (char*)PROPERTY_RIL_PHB_READY, (char*)"false");

        sp<RfxMessage> response = RfxMessage::obtainUrc(m_slot_id,
            RFX_MSG_URC_PHB_READY_NOTIFICATION, RfxIntsData((void*)&isPhbReady, sizeof(int)));
        responseToRilj(response);
    }
}

bool RtcPhbController::onCheckIfRejectMessage(const sp<RfxMessage>& message,
        bool isModemPowerOff, int radioState) {
    int msgId = message->getId();
    if (msgId == RFX_MSG_REQUEST_SET_PHONEBOOK_READY) {
        return false;
    }
    if ((radioState == (int)RADIO_STATE_OFF) &&
            (msgId == RFX_MSG_REQUEST_QUERY_PHB_STORAGE_INFO ||
             msgId == RFX_MSG_REQUEST_WRITE_PHB_ENTRY ||
             msgId == RFX_MSG_REQUEST_READ_PHB_ENTRY ||
             msgId == RFX_MSG_REQUEST_QUERY_UPB_CAPABILITY ||
             msgId == RFX_MSG_REQUEST_EDIT_UPB_ENTRY ||
             msgId == RFX_MSG_REQUEST_DELETE_UPB_ENTRY ||
             msgId == RFX_MSG_REQUEST_READ_UPB_GAS_LIST ||
             msgId == RFX_MSG_REQUEST_READ_UPB_GRP ||
             msgId == RFX_MSG_REQUEST_WRITE_UPB_GRP ||
             msgId == RFX_MSG_REQUEST_GET_PHB_STRING_LENGTH ||
             msgId == RFX_MSG_REQUEST_GET_PHB_MEM_STORAGE ||
             msgId == RFX_MSG_REQUEST_SET_PHB_MEM_STORAGE ||
             msgId == RFX_MSG_REQUEST_READ_PHB_ENTRY_EXT ||
             msgId == RFX_MSG_REQUEST_WRITE_PHB_ENTRY_EXT ||
             msgId == RFX_MSG_REQUEST_QUERY_UPB_AVAILABLE ||
             msgId == RFX_MSG_REQUEST_READ_EMAIL_ENTRY ||
             msgId == RFX_MSG_REQUEST_READ_SNE_ENTRY ||
             msgId == RFX_MSG_REQUEST_READ_ANR_ENTRY ||
             msgId == RFX_MSG_REQUEST_READ_UPB_AAS_LIST)) {
        logD(RFX_LOG_TAG, "onCheckIfRejectMessage, id = %d, isModemPowerOff = %d, radioState = %d",
            msgId, isModemPowerOff, radioState);
        return false;
    }
    return RfxController::onCheckIfRejectMessage(message, isModemPowerOff, radioState);
}

bool RtcPhbController::onHandleRequest(const sp<RfxMessage>& message) {
    int msgId = message->getId();
    logD(RFX_LOG_TAG, "onHandleRequest, handle %s", RFX_ID_TO_STR(msgId));
    switch (msgId) {
    case RFX_MSG_REQUEST_READ_PHB_ENTRY:
    case RFX_MSG_REQUEST_READ_UPB_GRP:
    case RFX_MSG_REQUEST_READ_PHB_ENTRY_EXT:
    case RFX_MSG_REQUEST_READ_EMAIL_ENTRY:
    case RFX_MSG_REQUEST_READ_SNE_ENTRY:
    case RFX_MSG_REQUEST_READ_ANR_ENTRY:
    case RFX_MSG_REQUEST_READ_UPB_AAS_LIST: {
            message->setPriority(MTK_RIL_REQUEST_PRIORITY_LOW);
            requestToMcl(message);
        }
        break;
    default: {
            requestToMcl(message);
        }
        break;
    }
    return true;
}

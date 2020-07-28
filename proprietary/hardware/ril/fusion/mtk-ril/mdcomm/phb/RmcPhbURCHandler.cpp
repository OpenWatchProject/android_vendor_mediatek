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

#include "RmcPhbURCHandler.h"
#include <telephony/mtk_ril.h>
#include "RfxMessageId.h"
#include <libmtkrilutils.h>

#define TRUE  1
#define FALSE 0
#define RFX_LOG_TAG "RmcPhbUrc"

using ::android::String8;

RFX_IMPLEMENT_HANDLER_CLASS(RmcPhbURCHandler, RIL_CMD_PROXY_URC);
RFX_REGISTER_DATA_TO_URC_ID(RfxIntsData, RFX_MSG_URC_PHB_READY_NOTIFICATION);

RmcPhbURCHandler::RmcPhbURCHandler(int slot_id, int channel_id) :
        RfxBaseHandler(slot_id, channel_id) {
    const char* urc[] = {
        "+EIND: 2",
        "+EIND: 32"
    };
    registerToHandleURC(urc, sizeof(urc)/sizeof(char *));
    mLock = PTHREAD_MUTEX_INITIALIZER;
    mPLock = &mLock;
}

RmcPhbURCHandler::~RmcPhbURCHandler() {
}

void RmcPhbURCHandler::onHandleUrc(const sp<RfxMclMessage>& msg) {
    char *urc = (char*)msg->getRawUrc()->getLine();
    if (strStartsWith(urc, "+EIND: 2")) {
        onPhbStateChanged(TRUE);
    } else if (strStartsWith(urc, "+EIND: 32")) {
        onPhbStateChanged(FALSE);
    }
}

bool RmcPhbURCHandler::onCheckIfRejectMessage(const sp<RfxMclMessage>& msg,
        RIL_RadioState radioState) {
    bool reject = false;
    RFX_UNUSED(msg);
    logD(RFX_LOG_TAG, "onCheckIfRejectMessage: %d %d", radioState, reject);
    return reject;
}

void RmcPhbURCHandler::onPhbStateChanged(int isPhbReady) {
    bool isModemResetStarted =
        getNonSlotMclStatusManager()->getBoolValue(RFX_STATUS_KEY_MODEM_POWER_OFF, false);
    int isSimInserted = getMclStatusManager()->getIntValue(RFX_STATUS_KEY_CARD_TYPE, FALSE);

    logI(RFX_LOG_TAG, "onPhbStateChanged slot=%d, isPhbReady=%d", m_slot_id, isPhbReady);

    if (RFX_SLOT_COUNT >= 2) {
        logI(RFX_LOG_TAG, "onPhbStateChanged isSimInserted=%d, isModemResetStarted=%d",
                isSimInserted, isModemResetStarted);

        if (isSimInserted == FALSE) {
            return;
        }

        if ((isPhbReady == TRUE) && (isModemResetStarted)) {
            return;
        }
    }

    if (isPhbReady == TRUE) {
        setMSimPropertyThreadSafe(
                m_slot_id, (char*)PROPERTY_RIL_PHB_READY, (char*)"true", mPLock);
    } else {
        sendEvent(RFX_MSG_EVENT_PHB_CURRENT_STORAGE_RESET, RfxVoidData(), RIL_CMD_PROXY_1, m_slot_id);
        setMSimPropertyThreadSafe(
                m_slot_id, (char*)PROPERTY_RIL_PHB_READY, (char*)"false", mPLock);
    }

    // response to TeleCore
    sp<RfxMclMessage> urc = RfxMclMessage::obtainUrc(RFX_MSG_URC_PHB_READY_NOTIFICATION,
            m_slot_id, RfxIntsData((void*)&isPhbReady, sizeof(int)));
    responseToTelCore(urc);
}

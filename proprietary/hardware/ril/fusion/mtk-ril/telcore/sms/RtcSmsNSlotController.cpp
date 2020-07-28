/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2019. All rights reserved.
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
#include "RtcSmsNSlotController.h"
#include <stdlib.h>
#include "RfxLog.h"
#include "telephony/mtk_ril.h"
#include "RfxRilUtils.h"

/*****************************************************************************
 * Defines
 *****************************************************************************/
#define EVENT_SMS_DISPATCH_SUPL_SMS         1

/*****************************************************************************
 * Class RfxController
 *****************************************************************************/
RFX_IMPLEMENT_CLASS("RtcSmsNSlotController", RtcSmsNSlotController, RfxController);

RtcSmsNSlotController::RtcSmsNSlotController() :
        m_smsThread(NULL),
        mDispatcherList(NULL) {
}

RtcSmsNSlotController::~RtcSmsNSlotController() {
}

void RtcSmsNSlotController::onInit() {
    RfxController::onInit();
    m_smsThread = RtcSmsThread::create();
    m_smsThread->run("RtcSmsThread");

    mDispatcherList = new Vector<SuplMsgDispatcher*>();

    for (int i = 0; i < RfxRilUtils::rfxGetSimCount(); i++) {
        SuplMsgDispatcher* messageDispatcher;
        RFX_OBJ_CREATE(messageDispatcher, SuplMsgDispatcher, this);
        mDispatcherList->add(messageDispatcher);
    }

}

void RtcSmsNSlotController::onDeinit() {
    m_smsThread->requestExit();
    if (m_smsThread->getLooper() != NULL) {
        m_smsThread->getLooper()->wake();
    }
    m_smsThread = NULL;

    if (mDispatcherList != NULL) {
        for (SuplMsgDispatcher* item : *mDispatcherList) {
            RFX_OBJ_CLOSE(item);
        }
        mDispatcherList->clear();
        delete mDispatcherList;
    }
    mDispatcherList = NULL;

    RfxController::onDeinit();
}

void RtcSmsNSlotController::onHandleSmsMessage(const sp<RtcSmsParsingMessage> & message) {
    logD(SMS_TAG, "[RtcSmsNSlotController]onHandleSmsMessage %d",
        message->getId());
    switch (message->getId()) {
        case EVENT_SMS_DISPATCH_SUPL_SMS: {
            Parcel *p = message->getParcel();
            p->setDataPosition(0);
            int slotId = p->readInt32();
            const char* data = p->readCString();
            string str = data;
            SuplMsgDispatcher* messageDispatcher = mDispatcherList->itemAt(slotId);
            messageDispatcher->dispatchSuplMsg(str);
            break;
        }
        default: {
            RFX_LOG_E(SMS_TAG, "[RtcSmsNSlotController]onHandleSmsMessage unknown!");
            break;
        }
    }
}

sp<RtcSmsThread> RtcSmsNSlotController::getSmsThread() {
    return m_smsThread;
}

void RtcSmsNSlotController::dispatchSms(const sp<RfxMessage>& msg) {
    if (mDispatcherList == NULL) {
        return;
    } else {
        SuplMsgDispatcher* messageDispatcher = mDispatcherList->itemAt(0);
        if (messageDispatcher == NULL || !messageDispatcher->doesSuplExist()) {
            return;
        }
    }

    char* data = (char*)msg->getData()->getData();
    int slotId = msg->getSlotId();
    // logD(SMS_TAG, "dispatchSms: %s, slotId: %d", data, slotId);
    if (data == NULL) {
        logD(SMS_TAG, "handleNewSms data is null.");
        return;
    }
    int length = strlen(data);
    char* pdu = (char*)malloc(length + 1);
    RFX_ASSERT(pdu != NULL);
    strncpy(pdu, data, length);
    pdu[length] = '\0';

    Parcel *p = new Parcel();
    p->writeInt32(slotId);
    p->writeCString(pdu);
    sp<RtcSmsParsingMessage> smsMsg = RtcSmsParsingMessage::obtainMessage(
              EVENT_SMS_DISPATCH_SUPL_SMS, p);
    sp<RtcSmsParsingThreadHandler> handler = new RtcSmsParsingThreadHandler(smsMsg);
    if (m_smsThread->getLooper() == NULL) {
        logD(SMS_TAG, "[RtcSmsNSlotController]sendSmsSmsMessage: ignore message");
        free(pdu);
        return;
    }
    handler->sendMessage(m_smsThread->getLooper());
    free(pdu);
}

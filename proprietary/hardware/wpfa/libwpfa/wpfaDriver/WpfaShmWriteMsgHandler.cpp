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

#include "WpfaDriver.h"
#include "WpfaShmWriteMsgHandler.h"

#define WPFA_D_LOG_TAG "WpfaShmWriteMsgHandler"

using ::android::Mutex;

WpfaShmWriteMsgHandler *WpfaShmWriteMsgHandler::s_self = NULL;

static sem_t sWaitLooperSem;
static bool sNeedWaitLooper = true;
static Mutex sWaitLooperMutex;


/*****************************************************************************
 * Class RuleReqMsgHandler
 *****************************************************************************/
void WpfaShmWriteMsgHandler::ShmWriteMsgHandler::handleMessage(
        const Message& message) {
    WPFA_UNUSED(message);
    sender->processMessage(msg);
}

/*****************************************************************************
 * Class WpfaShmWriteMsgHandler
 *****************************************************************************/
WpfaShmWriteMsgHandler::WpfaShmWriteMsgHandler() {
    mDriverAdapter = NULL;
    mShmSynchronizer = NULL;
}

void WpfaShmWriteMsgHandler::init() {
    mtkLogD(WPFA_D_LOG_TAG, "init");
    sem_init(&sWaitLooperSem, 0, 0);
    s_self = new WpfaShmWriteMsgHandler();
    s_self->run("WpfaShmWriteMsgHandler init");
}

bool WpfaShmWriteMsgHandler::threadLoop() {
    mtkLogD(WPFA_D_LOG_TAG, "threadLoop");
    int err = 0;
    mLooper = Looper::prepare(0);
    sem_post(&sWaitLooperSem);

    int result;
    do {
        result = mLooper->pollAll(-1);
        mtkLogD(WPFA_D_LOG_TAG, "threadLoop, result = %d, err=%d", result, err);
    } while (result == Looper::POLL_WAKE || result == Looper::POLL_CALLBACK);

    WPFA_D_ASSERT(0); // Can't go here
    return true;
}

void WpfaShmWriteMsgHandler::enqueueShmWriteMessage(const sp<WpfaDriverMessage>& message) {
    //if (!RfxRilUtils::isInLogReductionList(message->getId())) {
        mtkLogD(WPFA_D_LOG_TAG, "enqueueShmWriteMessage: %s", message->toString().string());
    //}
    WpfaShmWriteMsgHandler* sender = s_self;
    sp<MessageHandler> handler = new ShmWriteMsgHandler(sender, message);
    s_self->mLooper->sendMessage(handler, s_self->mDummyMsg);
}

void WpfaShmWriteMsgHandler::enqueueShmWriteMessageFront(const sp<WpfaDriverMessage>& message) {
    mtkLogD(WPFA_D_LOG_TAG, "enqueueShmWriteMessageFront: %s", message->toString().string());
    WpfaShmWriteMsgHandler* sender = s_self;
    sp<MessageHandler> handler = new ShmWriteMsgHandler(sender, message);
    s_self->mLooper->sendMessageAtTime(0, handler, s_self->mDummyMsg);
}

sp<Looper> WpfaShmWriteMsgHandler::waitLooper() {
    WPFA_D_ASSERT(s_self != NULL);
    sWaitLooperMutex.lock();
    if (sNeedWaitLooper) {
        mtkLogD(WPFA_D_LOG_TAG, "waitLooper() begin");
        sem_wait(&sWaitLooperSem);
        sNeedWaitLooper = false;
        sem_destroy(&sWaitLooperSem);
        mtkLogD(WPFA_D_LOG_TAG, "waitLooper() end");
    }
    sWaitLooperMutex.unlock();
    return s_self->mLooper;
}

void WpfaShmWriteMsgHandler::processMessage(const sp<WpfaDriverMessage>& msg) {
    uint16_t msgId = msg->getMsgId();
    uint16_t tId = msg->getTid();
    mtkLogD(WPFA_D_LOG_TAG, "processMessage, msgId=%d tId=%d", msgId, tId);
    switch (msgId) {
        case MSG_A2M_DATA_READY:
        case MSG_A2M_REQUEST_DATA_ACK:
            if (checkDriverAdapterState() == 0) {
                mDriverAdapter->sendMsgToControlMsgDispatcher(msg);
            }
            break;
        case MSG_M2A_REQUEST_DATA:
        case MSG_M2A_REQUEST_DATA_DONE:
            if (checkShmControllerState() == 0) {
                mShmSynchronizer->processControlMessage(tId, msgId);
            }
            break;
        default:
            mtkLogE(WPFA_D_LOG_TAG, "processMessage, type=%d not support", msgId);
    }
}

int WpfaShmWriteMsgHandler::checkDriverAdapterState() {
    int retValue = 0;
    if (mDriverAdapter != NULL) {
        return retValue;
    } else {
        mDriverAdapter = WpfaDriverAdapter::getInstance();
        if (mDriverAdapter == NULL) {
            mtkLogE(WPFA_D_LOG_TAG, "getInstance of WpfaDriverAdapter fail");
            retValue = -1;
        }
    }
    return retValue;
}

int WpfaShmWriteMsgHandler::checkShmControllerState() {
    int retValue = 0;
    if (mShmSynchronizer != NULL) {
        return retValue;
    } else {
        mShmSynchronizer = WpfaShmSynchronizer::getInstance();
        if (mShmSynchronizer == NULL) {
            mtkLogE(WPFA_D_LOG_TAG, "getInstance of checkShmControllerState fail");
            retValue = -1;
        }
    }
    return retValue;
}
int WpfaShmWriteMsgHandler::sendMessageToModem(uint16_t msgId, uint16_t tId) {
    int retValue = checkDriverAdapterState();
    if (retValue == 0) {
        sp<WpfaDriverMessage> msg = WpfaDriverMessage::obtainMessage(
        msgId,
        tId,
        CCCI_CTRL_MSG,
        0);
        retValue = mDriverAdapter->sendMsgToControlMsgDispatcher(msg);
    }
    return retValue;
}

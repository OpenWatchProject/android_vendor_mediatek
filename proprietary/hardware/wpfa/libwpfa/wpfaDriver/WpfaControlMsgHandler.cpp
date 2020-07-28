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
#include "WpfaControlMsgHandler.h"

extern "C" {
#include <hardware/ccci_intf.h>
}

#define WPFA_D_LOG_TAG "WpfaControlMsgHandler"

using ::android::Mutex;

WpfaControlMsgHandler *WpfaControlMsgHandler::s_self = NULL;
int WpfaControlMsgHandler::mCcciFd = -1;

static sem_t sWaitLooperSem;
static bool sNeedWaitLooper = true;
static Mutex sWaitLooperMutex;

/*****************************************************************************
 * Class CcciMsgHandler
 *****************************************************************************/
void WpfaControlMsgHandler::CcciMsgHandler::handleMessage(
        const Message& message) {
    WPFA_UNUSED(message);

    uint16_t msgId = msg->getMsgId();
    //mtkLogD(WPFA_D_LOG_TAG, "handleMessage, msgId=%d", msgId);
    switch (msgId) {
        case MSG_A2M_WPFA_INIT:
        case MSG_A2M_WPFA_VERSION:
        case MSG_A2M_REG_REPLY:
        case MSG_A2M_DEREG_REPLY:
        case MSG_A2M_DATA_READY:
        case MSG_A2M_REQUEST_DATA_ACK:
        case MSG_A2M_REQUEST_DATA:
        case MSG_A2M_REQUEST_DATA_DONE:
            sender->sendMessageToCcci(msg);
            break;
        case MSG_M2A_WPFA_VERSION:
        case MSG_M2A_REG_DL_FILTER:
        case MSG_M2A_DEREG_DL_FILTER:
            sender->sendMessageToRuleHandler(msg);
            break;
        case MSG_M2A_DATA_READY:
        case MSG_M2A_REQUEST_DATA_ACK:
        case MSG_M2A_UL_IP_PKT:
            sender->sendMessageToShmReadMsgHandler(msg);
            break;
        case MSG_M2A_REQUEST_DATA:
        case MSG_M2A_REQUEST_DATA_DONE:
            sender->sendMessageToShmWriteMsgHandler(msg);
            break;
        default:
            mtkLogE(WPFA_D_LOG_TAG, "handleMessage, type=%d not support", msgId);
    }
}

/*****************************************************************************
 * Class WpfaControlMsgHandler
 *****************************************************************************/
WpfaControlMsgHandler::WpfaControlMsgHandler() {
    mDriverAdapter = NULL;
    mWpfaDriverUtilis = NULL;
    mReader = NULL;
    mSender = NULL;
}

void WpfaControlMsgHandler::init(int ccciHandler) {
    mtkLogD(WPFA_D_LOG_TAG, "init ccciHandler:%d", ccciHandler);
    sem_init(&sWaitLooperSem, 0, 0);
    mCcciFd = ccciHandler;
    s_self = new WpfaControlMsgHandler();
    s_self->run("Wpfa control message dispatcher");
}

bool WpfaControlMsgHandler::threadLoop() {
    mtkLogD(WPFA_D_LOG_TAG, "threadLoop");
    int err = 0;
    mLooper = Looper::prepare(0);


    // 1. open ccci
    //err = openCcciDriver();
    if (mCcciFd < 0) {
        mtkLogE(WPFA_D_LOG_TAG, "fail to open ccci, err = %d", err);
        // ASSERT
        //WPFA_D_ASSERT(0); // Can't go here
    }
    //else {  // TODO: only for test
        mtkLogD(WPFA_D_LOG_TAG, "new Reader/Sender");
        mReader = new WpfaCcciReader(mCcciFd);
        mReader->run("WpfaCcciReader");

        mSender = new WpfaCcciSender(mCcciFd);
        mSender->run("WpfaCcciSender");

        // UT MODE
        if (WPFA_DRIVER_TEST_MODE_ENABLE) {
            if (mWpfaDriverUtilis == NULL) {
                mWpfaDriverUtilis = WpfaDriverUtilis::getInstance();
                if (mWpfaDriverUtilis == NULL) {
                    mtkLogE(WPFA_D_LOG_TAG, "getInstance of DriverUtilis fail");
                }
            }
            mWpfaDriverUtilis->setCcciReader(mReader);
            mWpfaDriverUtilis->setCcciSender(mSender);
            mtkLogD(WPFA_D_LOG_TAG, "!!! Test Mode!!!");
        }
    //}

    sem_post(&sWaitLooperSem);

    int result;
    do {
        result = mLooper->pollAll(-1);
        mtkLogD(WPFA_D_LOG_TAG, "threadLoop, result = %d", result);
    } while (result == Looper::POLL_WAKE || result == Looper::POLL_CALLBACK);

    WPFA_D_ASSERT(0); // Can't go here
    return true;
}

void WpfaControlMsgHandler::enqueueDriverMessage(const sp<WpfaDriverMessage>& message) {
    //if (!RfxRilUtils::isInLogReductionList(message->getId())) {
        mtkLogD(WPFA_D_LOG_TAG, "enqueueDriverMessage: %s", message->toString().string());
    //}
    WpfaControlMsgHandler* dispatcher = s_self;
    sp<MessageHandler> handler = new CcciMsgHandler(dispatcher, message);
    s_self->mLooper->sendMessage(handler, s_self->mDummyMsg);
}

void WpfaControlMsgHandler::enqueueDriverlMessageFront(const sp<WpfaDriverMessage>& message) {
    mtkLogD(WPFA_D_LOG_TAG, "enqueueDriverMessageFront: %s", message->toString().string());
    WpfaControlMsgHandler* dispatcher = s_self;
    sp<MessageHandler> handler = new CcciMsgHandler(dispatcher, message);
    s_self->mLooper->sendMessageAtTime(0, handler, s_self->mDummyMsg);
}

sp<Looper> WpfaControlMsgHandler::waitLooper() {
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

void WpfaControlMsgHandler::sendMessageToCcci(const sp<WpfaDriverMessage>& msg) {
    mSender->enqueueCcciMessage(msg);
}

void WpfaControlMsgHandler::sendMessageToRuleHandler(const sp<WpfaDriverMessage>& msg) {
    if (checkDriverAdapterState() == 0) {
        mDriverAdapter->sendMsgToFilterRuleReqHandler(msg);
    } else {
        mtkLogE(WPFA_D_LOG_TAG, "sendMessageToRuleHandler fail");
    }
}

void WpfaControlMsgHandler::sendMessageToShmReadMsgHandler(const sp<WpfaDriverMessage>& msg) {
    if (checkDriverAdapterState() == 0) {
        mDriverAdapter->sendMsgToShmReadMsgHandler(msg);
    } else {
        mtkLogE(WPFA_D_LOG_TAG, "sendMessageToRuleHandler fail");
    }
}

void WpfaControlMsgHandler::sendMessageToShmWriteMsgHandler(const sp<WpfaDriverMessage>& msg) {
    if (checkDriverAdapterState() == 0) {
        mDriverAdapter->sendMsgToShmWriteMsgHandler(msg);
    } else {
        mtkLogE(WPFA_D_LOG_TAG, "sendMessageToShmWriteMsgHandler fail");
    }
}

int WpfaControlMsgHandler::openCcciDriver() {
    int result = -1;
    const uint8_t maxTryCnt = 200;
    uint8_t tryCnt = 0;

    for (tryCnt = 0; tryCnt < maxTryCnt; tryCnt++) {
        if (tryCnt == 0) {
            mtkLogD(WPFA_D_LOG_TAG, "%s() maxTryCnt: %d", __FUNCTION__, maxTryCnt);
        }
        // open ccci
        mCcciFd = open("/dev/ccci_wifi_proxy", O_RDWR);
        if (mCcciFd < 0) {
            mtkLogD(WPFA_D_LOG_TAG, "openCcciDriver: mCcciFd: %d errno:%d ", mCcciFd, errno);
            mtkLogD(WPFA_D_LOG_TAG, "#%d, sleep 10 ms & retry openCcciDriver", tryCnt);
            usleep(10 * 1000); /* 10 ms */
        } else {
            mtkLogD(WPFA_D_LOG_TAG, "/dev/ccci_wifi_proxy is opened.");

            struct termios ios;
            tcgetattr(mCcciFd, &ios);
            ios.c_lflag = 0; /* disable ECHO, ICANON, etc... */
            ios.c_iflag = 0;
            tcsetattr(mCcciFd, TCSANOW, &ios);

            result = 0;
            break;
        }
    }

    return result;
}

int WpfaControlMsgHandler::checkDriverAdapterState() {
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

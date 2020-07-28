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
/*
 * File name:  RfxSocketStateManager.cpp
 * Author: Weilun Liu (MTK02772)
 * Description:
 *  Implementation of socket state manager
 */

/*****************************************************************************
 * Include
 *****************************************************************************/
#include <utils/Looper.h>
#include <utils/threads.h>

#include "RfxDefs.h"
#include "RfxLog.h"
#include "RfxMainThread.h"
#include "RfxRootController.h"
#include "RfxSocketStateManager.h"
#include "RfxStatusManager.h"
#include "RfxDispatchThread.h"
#include "modecontroller/RpCdmaLteModeController.h"

using ::android::Looper;
using ::android::Thread;
using ::android::Message;
using ::android::sp;

#define RFX_LOG_TAG "RfxSocketSM"

/*****************************************************************************
 * Class RfxSocketMessageHandler
 *****************************************************************************/

class RfxSocketMessageHandler : public RfxMainHandler {
public:
    explicit RfxSocketMessageHandler(
            const sp<RfxSocketStateMessage>& msg) : m_msg(msg) {}

protected:
    /**
     * Handles a message.
     */
    virtual void onHandleMessage(const Message& message) {
        RFX_UNUSED(message);
        RFX_LOG_D(RFX_LOG_TAG, "handle msg end, socketMsgHandler = %p", this);
        // dispatch msg to socket manager, it will update socket state to
        // rfx framework
        RfxSocketStateManager *mgr = RFX_OBJ_GET_INSTANCE(RfxSocketStateManager);
        mgr->processMessage(m_msg);
    }

private:
    sp<RfxSocketStateMessage> m_msg;
};

/*****************************************************************************
 * Class RfxSocketStateManager
 *****************************************************************************/
RFX_IMPLEMENT_CLASS("RfxSocketStateManager", RfxSocketStateManager, RfxController);
RFX_OBJ_IMPLEMENT_SINGLETON_CLASS(RfxSocketStateManager);

RfxSocketStateManager::RfxSocketStateManager() : mIsC2kSocketConnected(false), mC2kSlotId(RFX_SLOT_ID_0) {
}

void RfxSocketStateManager::onInit() {
    RfxController::onInit(); // Required: invoke super class implementation

    RFX_LOG_D(RFX_LOG_TAG, "onInit");
    for (int i=0; i< RFX_SLOT_COUNT; i++) {
        mSocketState[i] = 0;
    }
    mC2kSlotId = RpCdmaLteModeController::getCdmaSocketSlotId();
}

void RfxSocketStateManager::onDeinit() {
    RfxController::onDeinit();
}

void RfxSocketStateManager::setSocketState(RILD_RadioTechnology_Group groupId,
            bool isConnected, int slotId, int socFd) {
    if(groupId == RADIO_TECH_GROUP_C2K) {
        // get c2k slot id from svlte mode controller from constructor
        slotId = mC2kSlotId;
        mIsC2kSocketConnected = isConnected;
    }

    RfxSocketState::SOCKET_ID socId = RfxSocketState::SOCKET_INVALID;
    if (groupId == RADIO_TECH_GROUP_GSM) {
        socId = RfxSocketState::SOCKET_GSM;
    } else if (groupId == RADIO_TECH_GROUP_C2K){
        socId = RfxSocketState::SOCKET_C2K;
    // External SIM [Start]
    } else if (groupId == RADIO_TECH_GROUP_VSIM){
        if (socFd > 0) {
            RfxRilAdapter::getInstance()->setVsimSocket(slotId, socFd);
        }
        return;
    // External SIM [End]
    } else {
        RFX_LOG_D(RFX_LOG_TAG, "invalid group: %d", socId);
        return;
    }

    if(isConnected) {
        RFX_FLAG_SET(mSocketState[slotId], socId);
        RfxRilAdapter::getInstance()->setSocket(slotId, groupId, socFd);
    } else {
        RFX_FLAG_CLEAR(mSocketState[slotId], socId);
        RfxRilAdapter::getInstance()->closeSocket(slotId, groupId);
    }

    RFX_LOG_D(RFX_LOG_TAG, "setSocketState");
    RFX_LOG_D(RFX_LOG_TAG, "socId:%d, isConn:%d, slotId:%d, c2kSlotId:%d, mSocketState[%d]:%d",
            socId, (isConnected == true) ? 1 : 0, slotId, mC2kSlotId, slotId, mSocketState[slotId]);

    getStatusManager(slotId)->setSocketStateValue(RFX_STATUS_KEY_SOCKET_STATE,
            RfxSocketState(mSocketState[slotId], (mC2kSlotId == slotId), slotId));

    if(!isConnected) {
        for(int i=0; i<RFX_SLOT_COUNT; i++) {
            getStatusManager(i)->setIntValue(RFX_STATUS_KEY_RADIO_STATE, RADIO_STATE_UNAVAILABLE);
        }
        RfxMainThread::clearMessages();
        RfxDispatchThread::clearPendingQueue();
    }
}

void RfxSocketStateManager::setCdmaSocketSlotChange(int c2kSlot) {
    RfxRilAdapter* rfxRilAdapter = RfxRilAdapter::getInstance();

    if (mC2kSlotId != c2kSlot) {
        mC2kSlotId = c2kSlot;

        // notify RA to update mapping beteen c2k socket and slot id
        RfxRilAdapter::getInstance()->switchC2kSocket(mC2kSlotId);

        for(int i=0; i<RFX_SLOT_COUNT; i++) {
            if(i == c2kSlot && mIsC2kSocketConnected) {
                RFX_FLAG_SET(mSocketState[i], RfxSocketState::SOCKET_C2K);
            } else {
                RFX_FLAG_CLEAR(mSocketState[i], RfxSocketState::SOCKET_C2K);
            }

            getStatusManager(i)->setSocketStateValue(RFX_STATUS_KEY_SOCKET_STATE,
                    RfxSocketState(mSocketState[i], (c2kSlot == i), i));
            RFX_LOG_D(RFX_LOG_TAG, "onCdmaSocketSlotChange mSocketState[%d]:%d, c2kSlot:%d",
                    i, mSocketState[i], c2kSlot);
        }
    }
}

void RfxSocketStateManager::processMessage(const sp<RfxSocketStateMessage>& message) {
    setSocketState(message->getGroupId(), message->getIsConnected(), message->getSlotId(),
            message->getSocketFd());
}

void RfxSocketStateManager::notifySocketState(
        RILD_RadioTechnology_Group group, int slotId, int fd, bool isConnected) {
    RfxSocketState::SOCKET_ID socId;
    RFX_LOG_D(RFX_LOG_TAG,
            "notifySocketState group: %d, slotId: %d, fd:%d, isConn:%d",
            group, slotId, fd, isConnected);
    Message dummyMsg;
    sp<RfxSocketStateMessage> new_msg = new RfxSocketStateMessage(group, isConnected, slotId, fd);
    sp<MessageHandler> handler = new RfxSocketMessageHandler(new_msg);
    RfxMainThread::waitLooper()->sendMessageAtTime(0, handler, dummyMsg);
}


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
 * File name:  rfx_message.cpp
 * Author: Jun Liu (MTK80064)
 * Description:
 * Implement RfxMessage which is the general message type in RIL proxy framework
 */

/*****************************************************************************
 * Include
 *****************************************************************************/

#include "RfxMessage.h"
#include "RfxLog.h"
#include "RfxBasics.h"
#include "utils/Mutex.h"
#include <inttypes.h>
#include "RfxIdToStringUtils.h"

using ::android::Mutex;

#define RFX_LOG_TAG "RfxMessage"

namespace android {
extern RIL_RadioFunctions s_callbacks;
}

/*****************************************************************************
 * Class RfxMessage
 *****************************************************************************/
extern "C" const char * requestToString(int request);

int intMin = std::numeric_limits<int>::min();
int intMax = std::numeric_limits<int>::max();

pthread_mutex_t RfxMessage::token_mutex = PTHREAD_MUTEX_INITIALIZER;
int RfxMessage::tokenCounter = 0;
pthread_mutex_t RfxMessage::pTokenMutex = PTHREAD_MUTEX_INITIALIZER;
int RfxMessage::pTokenCounter = 0;

#ifdef RFX_OBJ_DEBUG
Vector<RfxDebugInfo*> *RfxMessage::s_obj_debug_info = NULL;
static Mutex s_obj_debug_info_mutex; // RfxMessage would be create/destroy in different thread
#endif //#ifdef RFX_OBJ_DEBUG

RfxMessage::RfxMessage() :
            type(REQUEST), source(RADIO_TECH_GROUP_GSM), dest(RADIO_TECH_GROUP_GSM), pId(0),
            pToken(0), id(0), token(0), slotId(0), clientId(-1), error(RIL_E_SUCCESS),
            timeStamp(0), pTimeStamp(0), isSentOnCdmaCapabilitySlot(C_SLOT_STATUS_OTHERS),
            data(NULL), urcDispatchRule(URC_DISPATCH_TO_RILJ),
            key(RfxStatusKeyEnum(-1)), value(RfxVariant()), force_notify(false),
            is_default(false), update_for_mock(false), rilToken(NULL), sendToMainProtocol(false),
            mainProtocolSlotId(0), addAtFront(false),
            m_priority(MTK_RIL_REQUEST_PRIORITY::MTK_RIL_REQUEST_PRIORITY_MEDIUM) {
#ifdef RFX_OBJ_DEBUG
    s_obj_debug_info_mutex.lock();
    if (RfxDebugInfo::isRfxDebugInfoEnabled()) {
        m_debug_info = new RfxDebugInfo(static_cast<IRfxDebugLogger *>(this), (void *)this);
        if (s_obj_debug_info == NULL) {
            s_obj_debug_info = new Vector<RfxDebugInfo*>();
        }
        s_obj_debug_info->add(m_debug_info);
    } else {
        m_debug_info = NULL;
    }
    s_obj_debug_info_mutex.unlock();
#endif //#ifdef RFX_OBJ_DEBUG
}

RfxMessage::~RfxMessage() {
    // RLOGD("~RfxMessage(): type=%d, source=%d, dest=%d, pId=%d, pToken=%d, id=%d, token=%d,\
    //        slotId=%d, error=%d", type, source, dest, pId, pToken, id, token, slotId, error);
    if (data != NULL) {
        delete(data);
    }
#ifdef RFX_OBJ_DEBUG
    if (m_debug_info != NULL) {
        s_obj_debug_info_mutex.lock();
        size_t size = s_obj_debug_info->size();
        for (size_t i = 0; i < size; i++) {
            const RfxDebugInfo *item = s_obj_debug_info->itemAt(i);
            if (item == m_debug_info) {
                s_obj_debug_info->removeAt(i);
                break;
            }
        }
        delete(m_debug_info);
        m_debug_info = NULL;
        s_obj_debug_info_mutex.unlock();
    }
#endif //#ifdef RFX_OBJ_DEBUG
}

int RfxMessage::generateToken() {
    int next = 0;
    pthread_mutex_lock(&token_mutex);
    next = tokenCounter++;
    if (next == intMax) {
        resetToken();
    }
    pthread_mutex_unlock(&token_mutex);
    return next;
}

void RfxMessage::resetToken() {
    pthread_mutex_lock(&token_mutex);
    tokenCounter = 0;
    pthread_mutex_unlock(&token_mutex);
}

int RfxMessage::generatePToken() {
    int next = 0;
    pthread_mutex_lock(&pTokenMutex);
    next = pTokenCounter--;
    if (next == intMin) {
        resetPToken();
    }
    pthread_mutex_unlock(&pTokenMutex);
    return next;
}

void RfxMessage::resetPToken() {
    pthread_mutex_lock(&pTokenMutex);
    pTokenCounter = 0;
    pthread_mutex_unlock(&pTokenMutex);
}

String8 RfxMessage::toString() const {
    return String8::format(
            "type=%s, source=%s, dest=%s, pId=%s(%d), pToken=%d, id=%s(%d), token=%d, slotId=%d, \
            clientId = %d, error = %d, isSentOnCdmaCapabilitySlot = %d, sendToMainProtocol = %d,\
            m_priority = %d, pTimeStamp = %" PRId64, typeToString(type), groupToString(source),
            groupToString(dest), RFX_ID_TO_STR(pId), pId, pToken, RFX_ID_TO_STR(id), id, token,
            slotId, clientId, error, isSentOnCdmaCapabilitySlot, sendToMainProtocol, m_priority,
            pTimeStamp);
}

const char* RfxMessage::typeToString(RFX_MESSAGE_TYPE type) const {
    switch (type) {
        case RFX_MESSAGE_TYPE::REQUEST:
            return "REQUEST";
        case RFX_MESSAGE_TYPE::RESPONSE:
            return "RESPONSE";
        case RFX_MESSAGE_TYPE::URC:
            return "URC";
        case RFX_MESSAGE_TYPE::REQUEST_ACK:
            return "REQUEST_ACK";
        case RFX_MESSAGE_TYPE::STATUS_SYNC:
            return "STATUS_SYNC";
        case RFX_MESSAGE_TYPE::SAP_REQUEST:
            return "SAP_REQUEST";
        case RFX_MESSAGE_TYPE::SAP_RESPONSE:
            return "SAP_RESPONSE";
        case RFX_MESSAGE_TYPE::SAP_URC:
            return "SAP_URC";
        default:
            return "unknown type";
    }
}

const char* RfxMessage::groupToString(RILD_RadioTechnology_Group group) const {
        switch (group) {
        case 0:
            return "GSM";
            break;
        case 1:
            return "C2K";
            break;
        case 2:
            return "ATCI";
            break;
        case 3:
            return "SAP_GSM";
            break;
        case 4:
            return "SAP_C2K";
            break;
        case 5:
            return "SAP_BT";
            break;
        default:
            return "unkonwn group";
            break;
    }
}

// private function. We assume module must call obtain API with RfxBaseData
sp<RfxMessage> RfxMessage::obtainRequest(int slotId, int id, RILD_RadioTechnology_Group dest) {
    sp<RfxMessage> msg = new RfxMessage();

    msg->type = REQUEST;
    msg->slotId = slotId;
    msg->id = id;
    msg->token = RfxMessage::generateToken();
    msg->timeStamp = systemTime(SYSTEM_TIME_MONOTONIC);
    msg->pTimeStamp = systemTime(SYSTEM_TIME_MONOTONIC);
    msg->pToken = RfxMessage::generatePToken();
    msg->dest = dest;

    return msg;
}

sp<RfxMessage> RfxMessage::obtainRequest(int slotId, int id, const sp<RfxMessage>& msg,
        bool copyData, RILD_RadioTechnology_Group dest) {

    if (msg == NULL || msg.get() == NULL)
        return NULL;

    sp<RfxMessage> new_msg = RfxMessage::obtainRequest(slotId, id, dest);
    new_msg->pId = msg->pId;
    new_msg->pToken = msg->pToken;
    new_msg->timeStamp = msg->timeStamp;
    new_msg->pTimeStamp = msg->pTimeStamp;
    new_msg->clientId = msg -> clientId;
    new_msg->rilToken = msg->rilToken;
    // copy data
    if (copyData) {
        new_msg->data = RfxDataCloneManager::copyData(id, msg->getData(), REQUEST);
    }

    return new_msg;
}

sp<RfxMessage> RfxMessage::obtainRequest(int slotId, int id, const RfxBaseData &data,
        RILD_RadioTechnology_Group dest) {
    sp<RfxMessage> msg = obtainRequest(slotId, id, dest);
    msg->data = RfxDataCloneManager::copyData(id, &data, REQUEST);
    return msg;
}

sp<RfxMessage> RfxMessage::obtainRequest(int id, const RfxBaseData &data,
            const sp<RfxMessage>& msg, bool copyData) {
    sp<RfxMessage> newMsg = RfxMessage::obtainRequest(msg->getSlotId(), id, msg, false);
    if (copyData) {
        newMsg->data = RfxDataCloneManager::copyData(id, msg->getData(), REQUEST);
    } else {
        newMsg->data = RfxDataCloneManager::copyData(id, &data, REQUEST);
    }

    return newMsg;
}

sp<RfxMessage> RfxMessage::obtainRequest(int slotId, int pId,
        int pToken, void* data, int length, RIL_Token t, int clientId,
        RILD_RadioTechnology_Group dest) {
    sp<RfxMessage> msg = new RfxMessage();

    msg->type = REQUEST;
    msg->slotId = slotId;
    msg->pId = pId;
    msg->pToken = pToken;
    msg->id = pId; // msg could be sent directly
    msg->token = RfxMessage::generateToken();
    msg->timeStamp = systemTime(SYSTEM_TIME_MONOTONIC);
    msg->pTimeStamp = systemTime(SYSTEM_TIME_MONOTONIC);
    msg->data = RfxDataCloneManager::copyData(pId, data, length, REQUEST);
    msg->rilToken = t;
    msg->clientId = clientId;
    msg->dest = dest;

    return msg;
}

sp<RfxMessage> RfxMessage::obtainResponse(int slotId, int pId, int pToken, int id,
        int token, RIL_Errno e, RfxBaseData* data, nsecs_t pTimeStamp, RIL_Token t,
        int clientId, RILD_RadioTechnology_Group source) {
    sp<RfxMessage> msg = new RfxMessage();
    msg->type = RESPONSE;
    msg->slotId = slotId;
    msg->pId = pId;
    msg->pToken = pToken;
    msg->id = id;
    msg->token = token;
    msg->error = e;
    msg->timeStamp = systemTime(SYSTEM_TIME_MONOTONIC);
    msg->pTimeStamp = pTimeStamp;
    msg->data = RfxDataCloneManager::copyData(id, data, RESPONSE);
    msg->rilToken = t;
    msg->clientId = clientId;
    msg->source = source;
    return msg;
}

sp<RfxMessage> RfxMessage::obtainResponse(RIL_Errno e, const sp<RfxMessage>& msg,
        bool copyData) {
    sp<RfxMessage> new_msg = new RfxMessage();
    new_msg->error = e;
    new_msg->type = RESPONSE;
    new_msg->source = msg->getSource();
    new_msg->slotId = msg->getSlotId();
    new_msg->pId = msg->getPId();
    new_msg->pToken = msg->getPToken();
    new_msg->id = msg->getId();
    new_msg->token = msg->getToken();
    new_msg->timeStamp = systemTime(SYSTEM_TIME_MONOTONIC);
    new_msg->clientId = msg -> clientId;
    new_msg->rilToken = msg->getRilToken();
    if (copyData) {
        new_msg->data = RfxDataCloneManager::copyData(msg->getId(), msg->getData(), RESPONSE);
    }
    return new_msg;
}

// Besides id and RfxabseData, copy from msg
sp<RfxMessage> RfxMessage::obtainResponse(int slotId, int id, RIL_Errno e,
        const RfxBaseData &data, const sp<RfxMessage>& msg) {
    sp<RfxMessage> new_msg = new RfxMessage();
    new_msg->error = e;
    new_msg->type = RESPONSE;
    new_msg->source = msg->getSource();
    new_msg->slotId = slotId;
    new_msg->pId = id;
    new_msg->pToken = msg->getPToken();
    new_msg->id = id;
    new_msg->token = msg->getToken();
    new_msg->timeStamp = systemTime(SYSTEM_TIME_MONOTONIC);
    new_msg->clientId = msg->clientId;
    new_msg->rilToken = msg->getRilToken();
    new_msg->data = RfxDataCloneManager::copyData(id, &data, RESPONSE);
    return new_msg;
}

sp<RfxMessage> RfxMessage::obtainResponse(int id, const sp<RfxMessage>& msg) {
    return RfxMessage::obtainResponse(msg->getSlotId(), id, msg->getError(), *(msg->getData()),
            msg);
}

sp<RfxMessage> RfxMessage::obtainUrc(int slotId, int id, const RfxBaseData &data,
        RILD_RadioTechnology_Group source) {
    sp<RfxMessage> msg = new RfxMessage();
    msg->type = URC;
    msg->source = source;
    msg->slotId = slotId;
    msg->pId = id;
    msg->id = id;
    msg->timeStamp = systemTime(SYSTEM_TIME_MONOTONIC);
    msg->data = RfxDataCloneManager::copyData(id, &data, URC);
    return msg;
}

sp<RfxMessage> RfxMessage::obtainUrc(int slotId, int id, const sp<RfxMessage>& msg,
        bool copyData, RILD_RadioTechnology_Group source) {
    sp<RfxMessage> new_msg = new RfxMessage();
    new_msg->type = URC;
    new_msg->source = source;
    new_msg->slotId = slotId;
    new_msg->pId = id;
    new_msg->id = id;
    new_msg->timeStamp = systemTime(SYSTEM_TIME_MONOTONIC);
    if (copyData) {
        new_msg->data = RfxDataCloneManager::copyData(msg->getId(), msg->getData(), URC);
    }
    return new_msg;
}

sp<RfxMessage> RfxMessage::obtainStatusSync(int slotId, RfxStatusKeyEnum key,
        const RfxVariant value, bool force_notify, bool is_default, bool update_for_mock) {
    sp<RfxMessage> msg = new RfxMessage();
    msg->type = STATUS_SYNC;
    msg->slotId = slotId;
    msg->key = key;
    msg->value = value;
    msg->force_notify = force_notify;
    msg->is_default = is_default;
    msg->update_for_mock = update_for_mock;
    return msg;
}

sp<RfxMessage> RfxMessage::obtainSapRequest(int slotId, int pId, int pToken, void *data,
        int length, RIL_Token t) {
    sp<RfxMessage> msg = new RfxMessage();
    msg->type = SAP_REQUEST;
    msg->slotId = slotId;
    msg->pId = pId;
    msg->pToken = pToken;
    msg->id = pId; // msg could be sent directly
    msg->token = RfxMessage::generateToken();
    msg->timeStamp = systemTime(SYSTEM_TIME_MONOTONIC);
    msg->pTimeStamp = systemTime(SYSTEM_TIME_MONOTONIC);
    msg->data = RfxDataCloneManager::copyData(pId, data, length, REQUEST);
    msg->rilToken = t;
    return msg;
}

sp<RfxMessage> RfxMessage::obtainSapResponse(int slotId, int pId, int pToken, int id, int token,
        RIL_Errno e, RfxBaseData *data, nsecs_t pTimeStamp, RIL_Token t) {
    sp<RfxMessage> msg = new RfxMessage();
    msg->type = SAP_RESPONSE;
    msg->slotId = slotId;
    msg->pId = pId;
    msg->pToken = pToken;
    msg->id = id;
    msg->token = token;
    msg->error = e;
    msg->timeStamp = systemTime(SYSTEM_TIME_MONOTONIC);
    msg->pTimeStamp = pTimeStamp;
    msg->data = RfxDataCloneManager::copyData(id, data, RESPONSE);
    msg->rilToken = t;
    return msg;
}

sp<RfxMessage> RfxMessage::obtainSapUrc(int slotId, int id, const RfxBaseData &data) {
    sp<RfxMessage> msg = new RfxMessage();
    msg->type = SAP_URC;
    msg->slotId = slotId;
    msg->pId = id;
    msg->id = id;
    msg->timeStamp = systemTime(SYSTEM_TIME_MONOTONIC);
    msg->data = RfxDataCloneManager::copyData(id, &data, URC);
    return msg;
}

#ifdef RFX_OBJ_DEBUG
void RfxMessage::dump(int level) const {
    RFX_LOG_D(RFX_DEBUG_INFO_TAG, "%p, wc = %d, sc = %d, %s",
                  this, getWeakRefs()->getWeakCount(), getStrongCount(), toString().string());
    RFX_UNUSED(level);
}

void RfxMessage::dumpMsgList() {
    s_obj_debug_info_mutex.lock();
    size_t size = RfxMessage::s_obj_debug_info->size();
    RFX_LOG_D(RFX_DEBUG_INFO_TAG, "dumpMsgList() Msg count is %d", (int)size);
    for (size_t i = 0; i < size; i++) {
        const RfxDebugInfo *item = RfxMessage::s_obj_debug_info->itemAt(i);
        item->getLogger()->dump();
    }
    s_obj_debug_info_mutex.unlock();
}

#endif // #ifdef RFX_OBJ_DEBUG

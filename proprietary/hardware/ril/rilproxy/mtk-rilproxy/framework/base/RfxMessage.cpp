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
            pToken(0), id(0), token(0), slotId(0), clientId(-1), parcel(NULL),
            error(RIL_E_SUCCESS), timeStamp(0), pTimeStamp(0), dataObj(NULL), rilToken(NULL),
            key(RfxStatusKeyEnum(-1)), value(RfxVariant()), force_notify(false), is_default(false) {
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
    // RFX_LOG_D(RFX_LOG_TAG,
    //        "~RfxMessage(): type=%d, source=%d, dest=%d, pId=%d, pToken=%d, id=%d, token=%d,\
    //        slotId=%d, error=%d", type, source, dest, pId, pToken, id, token, slotId, error);
    if (parcel != NULL) {
        delete(parcel);
        parcel = NULL;
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

bool RfxMessage::resetParcelDataStartPos() {
    int dataStartPos = 0;
    if (type == RFX_MESSAGE_TYPE::REQUEST) {
        dataStartPos = (sizeof(int32_t) * 2);
    } else if (type == RFX_MESSAGE_TYPE::REQUEST_ACK) {
        dataStartPos = (sizeof(int32_t) * 2);
    } else if (type == RFX_MESSAGE_TYPE::RESPONSE) {
        dataStartPos = (sizeof(int32_t) * 3);
    } else if (type == RFX_MESSAGE_TYPE::URC) {
        dataStartPos = (sizeof(int32_t) * 2);
    } else {
        RFX_LOG_E(RFX_LOG_TAG, "rfxMessage resetParcelDataStartPos unknown message type: %d", type);
        return false;
    }

    if (this->parcel == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "rfxMessage resetParcelDataStartPos parcel NULL");
        return false;
    }

    if ((int)this->parcel->dataSize() < dataStartPos) {
        RFX_LOG_E(RFX_LOG_TAG,
                "rfxMessage resetParcelDataStartPos invalid data size: %zu (type=%d, expect=%d)",
                this->parcel->dataSize(), type, dataStartPos);
        return false;
    }

    this->parcel->setDataPosition(dataStartPos);
    RFX_LOG_D(RFX_LOG_TAG,
            "rfxMessage resetParcelDataStartPos (type=%d, set to: %d)", type, dataStartPos);
    return true;
}

String8 RfxMessage::toString() const {
    return String8::format(
            "type=%s, source=%s, dest=%s, pId=%s(%d), pToken=%d, id=%s(%d), token=%d, slotId=%d, \
            error = %d, isSentOnCdmaCapabilitySlot = %d, pTimeStamp = %" PRId64, typeToString(type),
            groupToString(source), groupToString(dest), requestToString(pId), pId, pToken,
            requestToString(id), id, token, slotId, error, isSentOnCdmaCapabilitySlot, pTimeStamp);
}

const char* RfxMessage::typeToString(RFX_MESSAGE_TYPE type) const {
    switch (type) {
        case RFX_MESSAGE_TYPE::REQUEST:
            return "REQUEST";
            break;
        case RFX_MESSAGE_TYPE::RESPONSE:
            return "RESPONSE";
            break;
        case RFX_MESSAGE_TYPE::URC:
            return "URC";
            break;
        case RFX_MESSAGE_TYPE::REQUEST_ACK:
            return "REQUEST_ACK";
            break;
        default:
            return "unknown type";
            break;
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

sp<RfxMessage> RfxMessage::obtainRequest(RILD_RadioTechnology_Group target, int id,
        const sp<RfxMessage>& msg) {
    return obtainRequest(target, id, msg, false);
}

sp<RfxMessage> RfxMessage::obtainRequest(RILD_RadioTechnology_Group target, int id,
        const sp<RfxMessage>& msg, bool copyParcel) {

    if (msg==NULL || msg.get()==NULL)
        return NULL;

    sp<RfxMessage> new_msg = RfxMessage::obtainRequest(msg->slotId, target, id);
    new_msg->pId = msg->pId;
    new_msg->pToken = msg->pToken;
    new_msg->timeStamp = msg->timeStamp;
    new_msg->pTimeStamp = msg->pTimeStamp;
    new_msg->clientId = msg -> clientId;
    new_msg->rilToken = msg->rilToken;
    Parcel* msg_parcel = msg->getParcel();
    int defaultDataStartPosition = (2 * sizeof(int32_t));
    if (copyParcel && new_msg->parcel != NULL && msg_parcel != NULL
            && (int)msg_parcel->dataSize() > defaultDataStartPosition) {
        new_msg->parcel->appendFrom(msg_parcel, defaultDataStartPosition,
                msg_parcel->dataSize() - defaultDataStartPosition);
    }

    return new_msg;
}

sp<RfxMessage> RfxMessage::obtainRequest(int slotId, RILD_RadioTechnology_Group target, int id) {
    sp<RfxMessage> new_msg = new RfxMessage();

    new_msg->type = RFX_MESSAGE_TYPE::REQUEST;
    new_msg->slotId = slotId;
    new_msg->dest = target;
    new_msg->id = id;
    new_msg->token = RfxMessage::generateToken();
    new_msg->timeStamp = systemTime(SYSTEM_TIME_MONOTONIC);
    new_msg->pTimeStamp = systemTime(SYSTEM_TIME_MONOTONIC);
    new_msg->pToken = RfxMessage::generatePToken();

    new_msg->parcel = new Parcel();
    new_msg->parcel->writeInt32((new_msg->id & 0xFFFFFFFF));
    new_msg->parcel->writeInt32((new_msg->token & 0xFFFFFFFF));

    return new_msg;
}

sp<RfxMessage> RfxMessage::obtainRequest(int slotId,
        RILD_RadioTechnology_Group target, int pId, int pToken, Parcel* parcel) {
    sp<RfxMessage> new_msg = new RfxMessage();

    new_msg->type = RFX_MESSAGE_TYPE::REQUEST;
    new_msg->slotId = slotId;
    new_msg->pId = pId;
    new_msg->pToken = pToken;
    new_msg->id = pId; // msg could be sent directly
    new_msg->token = RfxMessage::generateToken();
    new_msg->dest = target;
    new_msg->parcel = parcel;
    new_msg->timeStamp = systemTime(SYSTEM_TIME_MONOTONIC);
    new_msg->pTimeStamp = systemTime(SYSTEM_TIME_MONOTONIC);
    new_msg->parcel->setDataPosition(sizeof(int32_t));
    new_msg->parcel->writeInt32((new_msg->token & 0xFFFFFFFF));
    return new_msg;
}

sp<RfxMessage> RfxMessage::obtainRequest(int slotId,
        RILD_RadioTechnology_Group target, int pid, int ptoken, Parcel* parcel, int clientId) {
    sp<RfxMessage> new_msg = obtainRequest(slotId, target, pid, ptoken, parcel);
    new_msg->clientId = clientId;
    return new_msg;
}

sp<RfxMessage> RfxMessage::obtainRequest(int slotId, RILD_RadioTechnology_Group target, int pId,
        int pToken, Parcel* parcel, RIL_Token t) {
    sp<RfxMessage> result = obtainRequest(slotId, target, pId, pToken, parcel);
    result->rilToken = t;

    return result;
}

sp<RfxMessage> RfxMessage::obtainResponse(int slotId, int pId, int pToken, int id,
            int token, RIL_Errno e, RILD_RadioTechnology_Group source, Parcel* parcel) {
    sp<RfxMessage> msg = new RfxMessage();
    msg->type = RESPONSE;
    msg->source = source;
    msg->slotId = slotId;
    msg->pId = pId;
    msg->pToken = pToken;
    msg->id = id;
    msg->token = token;
    msg->error = e;
    msg->timeStamp = systemTime(SYSTEM_TIME_MONOTONIC);
    msg->clientId = -1;
    // this parcel is from gsm-rild or c2k-rild
    msg->parcel = parcel;
    // set to start position of data
    parcel->setDataPosition(sizeof(int32_t)*3);
    return msg;
}

sp<RfxMessage> RfxMessage::obtainResponse(int slotId, int pId, int pToken, int id,
            int token, RIL_Errno e, RILD_RadioTechnology_Group source, Parcel* parcel,
            nsecs_t pTimeStamp) {
    sp<RfxMessage> msg = new RfxMessage();
    msg->type = RESPONSE;
    msg->source = source;
    msg->slotId = slotId;
    msg->pId = pId;
    msg->pToken = pToken;
    msg->id = id;
    msg->token = token;
    msg->error = e;
    msg->timeStamp = systemTime(SYSTEM_TIME_MONOTONIC);
    msg->pTimeStamp = pTimeStamp;
    msg->clientId = -1;
    // this parcel is from gsm-rild or c2k-rild
    msg->parcel = parcel;
    // set to start position of data
    parcel->setDataPosition(sizeof(int32_t)*3);
    return msg;
}

sp<RfxMessage> RfxMessage::obtainRequestAck(int slotId, int pId, int pToken, int id,
            int token, RILD_RadioTechnology_Group source, Parcel* parcel) {
    sp<RfxMessage> msg = new RfxMessage();
    msg->type = REQUEST_ACK;
    msg->source = source;
    msg->slotId = slotId;
    msg->pId = pId;
    msg->pToken = pToken;
    msg->id = id;
    msg->token = token;
    msg->error = (RIL_Errno)0;
    msg->timeStamp = systemTime(SYSTEM_TIME_MONOTONIC);
    msg->clientId = -1;
    // this parcel is from gsm-rild or c2k-rild
    msg->parcel = parcel;
    // set to start position of data
    parcel->setDataPosition(sizeof(int32_t)*2);
    return msg;
}

sp<RfxMessage> RfxMessage::obtainRequestAck(int slotId, int pId, int pToken, int id,
            int token, RILD_RadioTechnology_Group source, Parcel* parcel,
            nsecs_t pTimeStamp) {
    sp<RfxMessage> msg = new RfxMessage();
    msg->type = REQUEST_ACK;
    msg->source = source;
    msg->slotId = slotId;
    msg->pId = pId;
    msg->pToken = pToken;
    msg->id = id;
    msg->token = token;
    msg->error = (RIL_Errno)0;
    msg->timeStamp = systemTime(SYSTEM_TIME_MONOTONIC);
    msg->pTimeStamp = pTimeStamp;
    msg->clientId = -1;
    // this parcel is from gsm-rild or c2k-rild
    msg->parcel = parcel;
    // set to start position of data
    parcel->setDataPosition(sizeof(int32_t)*2);
    return msg;
}

sp<RfxMessage> RfxMessage::obtainRequestAck(int slotId, int pId, int pToken, int id,
            int token, RILD_RadioTechnology_Group source, Parcel* parcel, int clientId,
            nsecs_t pTimeStamp) {
    sp<RfxMessage> new_msg = obtainRequestAck(slotId, pId, pToken, id, token, source, parcel);
    new_msg->clientId = clientId;
    new_msg->pTimeStamp = pTimeStamp;
    return new_msg;
}

sp<RfxMessage> RfxMessage::obtainRequestAck(int slotId, int pId, int pToken, int id,
            int token, RILD_RadioTechnology_Group source, Parcel* parcel,
            nsecs_t pTimeStamp, RIL_Token t) {
    sp<RfxMessage> msg = obtainRequestAck(slotId, pId, pToken, id, token, source, parcel,
            pTimeStamp);
    msg->rilToken = t;
    return msg;
}

sp<RfxMessage> RfxMessage::obtainResponse(int slotId, int pId, int pToken, int id,
            int token, RIL_Errno e, RILD_RadioTechnology_Group source, Parcel* parcel, int clientId,
            nsecs_t pTimeStamp) {
    sp<RfxMessage> new_msg = obtainResponse(slotId, pId, pToken, id, token, e, source, parcel);
    new_msg->clientId = clientId;
    new_msg->pTimeStamp = pTimeStamp;
    return new_msg;
}

sp<RfxMessage> RfxMessage::obtainResponse(int slotId, int pId, int pToken, int id,
            int token, RIL_Errno e, RILD_RadioTechnology_Group source, Parcel* parcel,
            nsecs_t pTimeStamp, RIL_Token t) {
    sp<RfxMessage> msg = obtainResponse(slotId, pId, pToken, id, token, e, source, parcel,
            pTimeStamp);
    msg->rilToken = t;
    return msg;
}


sp<RfxMessage> RfxMessage::obtainResponse(RIL_Errno e, const sp<RfxMessage>& msg) {
    return obtainResponse(e, msg, false);
}

sp<RfxMessage> RfxMessage::obtainResponse(RIL_Errno e, const sp<RfxMessage>& msg,
        bool copyParcel) {
    return obtainResponse(msg->getSlotId(), msg->getSource(), e, msg, copyParcel);
}

sp<RfxMessage> RfxMessage::obtainResponse(int slotId, RILD_RadioTechnology_Group source,
        RIL_Errno e, const sp<RfxMessage>& msg, bool copyParcel) {
    sp<RfxMessage> result = new RfxMessage();
    result->error = e;
    result->type = RESPONSE;
    result->source = source;
    result->slotId = slotId;
    result->pId = msg->getPId();
    result->pToken = msg->getPToken();
    result->id = msg->getId();
    result->token = msg->getToken();
    result->timeStamp = systemTime(SYSTEM_TIME_MONOTONIC);
    result->clientId = msg -> clientId;
    result->rilToken = msg->getRilToken();
    result->pTimeStamp = msg->pTimeStamp;

    // create pid's parcel
    result->parcel = new Parcel();
    result->parcel->writeInt32(RESPONSE_SOLICITED); // RESPONSE_SOLICITED
    result->parcel->writeInt32(msg->getPToken() & 0xFFFFFFFF);
    result->parcel->writeInt32(e & 0xFFFFFFFF);
    int offset = sizeof(int32_t) * 3;
    if (copyParcel && msg != NULL && (int)msg->parcel->dataSize() > offset) {
        result->parcel->appendFrom(msg->parcel, offset, msg->parcel->dataSize()-offset);
    }
    return result;
}

sp<RfxMessage> RfxMessage::obtainUrc(int slotId, int urcId,
            RILD_RadioTechnology_Group source, Parcel* parcel) {
    sp<RfxMessage> msg = new RfxMessage();
    msg->type = URC;
    msg->slotId = slotId;
    msg->pId = urcId;
    msg->source = source;
    msg->id = urcId;
    msg->parcel = parcel;
    msg->timeStamp = systemTime(SYSTEM_TIME_MONOTONIC);
    // set to start position of data
    parcel->setDataPosition(sizeof(int32_t)*2);
    return msg;
}

sp<RfxMessage> RfxMessage::obtainUrc(int slotId, int urcId) {
    return obtainUrc(slotId, urcId, NULL, false);
}

sp<RfxMessage> RfxMessage::obtainUrc(int slotId, int urcId, const sp<RfxMessage>& msg,
        bool copyParcel) {
    sp<RfxMessage> result = new RfxMessage();
    result->type = URC;
    result->slotId = slotId;
    result->pId = urcId;
    result->id = urcId;
    result->timeStamp = systemTime(SYSTEM_TIME_MONOTONIC);
    result->parcel = new Parcel();
    if (android::s_callbacks.version >= 13) {
        result->parcel->writeInt32(RESPONSE_UNSOLICITED_ACK_EXP); // RESPONSE_UNSOLICITED
    } else {
        result->parcel->writeInt32(RESPONSE_UNSOLICITED); // RESPONSE_UNSOLICITED
    }
    result->parcel->writeInt32 (urcId & 0xFFFFFFFF);
    int offset = sizeof(int32_t) * 2;
    if (copyParcel && msg != NULL && (int)msg->parcel->dataSize() > offset) {
        result->parcel->appendFrom(msg->parcel, offset, msg->parcel->dataSize()-offset);
    }
    return result;
}

sp<RfxMessage> RfxMessage::obtainStatusSync(int slotId, RfxStatusKeyEnum key,
        const RfxVariant value, bool force_notify, bool is_default) {
    sp<RfxMessage> msg = new RfxMessage();
    msg->type = STATUS_SYNC;
    msg->slotId = slotId;
    msg->key = key;
    msg->value = value;
    msg->force_notify = force_notify;
    msg->is_default = is_default;
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

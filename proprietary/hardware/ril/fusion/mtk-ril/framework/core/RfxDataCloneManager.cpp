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

#include "RfxDataCloneManager.h"
#include "RfxIdToStringUtils.h"

#define RFX_LOG_TAG "RfxCloneMgr"

RfxDataCloneManager* RfxDataCloneManager::s_self = NULL;

void RfxDataCloneManager::init() {
    if (s_self == NULL) {
        RFX_LOG_D(RFX_LOG_TAG, "init");
        s_self = new RfxDataCloneManager();
    }
}

void RfxDataCloneManager::registerRequestId(RfxCopyDataByDataFuncptr copyByData,
        RfxCopyDataByObjFuncptr copyByObj, int id) {
    init();
    RFX_LOG_D(RFX_LOG_TAG, "registerRequestId: id: %s(%d)", RFX_ID_TO_STR(id), id);
    s_self->registerInternal(s_self->m_request_list, copyByData, copyByObj, id);
}

void RfxDataCloneManager::registerResponseId(RfxCopyDataByDataFuncptr copyByData,
        RfxCopyDataByObjFuncptr copyByObj, int id) {
    init();
    RFX_LOG_D(RFX_LOG_TAG, "registerResponseId: id: %s(%d)", RFX_ID_TO_STR(id), id);
    s_self->registerInternal(s_self->m_response_list, copyByData, copyByObj, id);
}

void RfxDataCloneManager::registerUrcId(RfxCopyDataByDataFuncptr copyByData,
        RfxCopyDataByObjFuncptr copyByObj, int id) {
    init();
    RFX_LOG_D(RFX_LOG_TAG, "registerUrcId: id: %s(%d)", RFX_ID_TO_STR(id), id);
    s_self->registerInternal(s_self->m_urc_list, copyByData, copyByObj, id);
}

void RfxDataCloneManager::registerEventId(RfxCopyDataByDataFuncptr copyByData,
        RfxCopyDataByObjFuncptr copyByObj, int id) {
    init();
    RFX_LOG_D(RFX_LOG_TAG, "registerEventId: id: %s(%d)", RFX_ID_TO_STR(id), id);
    s_self->registerInternal(s_self->m_event_list, copyByData, copyByObj, id);
}

RfxBaseData* RfxDataCloneManager::copyData(int id, void *data, int length, int type) {
    SortedVector<RfxDataCloneEntry> entry = s_self->findDataCloneEntryList(type);
    RfxCopyDataByDataFuncptr ptr = s_self->findCopyDataByDataFuncptr(entry, id);
    if (ptr != NULL) {
        RFX_LOG_D(RFX_LOG_TAG, "copyData id = %d, ptr = %p", id, ptr);
        return ptr(data, length);
    }

    if (id != INVALID_ID) {
        RFX_LOG_D(RFX_LOG_TAG, "did not have corresponding RfxBaseData");
    }
    return NULL;
}

RfxBaseData* RfxDataCloneManager::copyData(int id, const RfxBaseData *data, int type) {
    SortedVector<RfxDataCloneEntry> entry = s_self->findDataCloneEntryList(type);
    RfxCopyDataByObjFuncptr ptr = s_self->findCopyDataByObjFuncptr(entry, id);
    if (ptr != NULL) {
        RFX_LOG_D(RFX_LOG_TAG, "copyData id = %d, ptr = %p", id, ptr);
        return ptr(data);
    }
    if (id != INVALID_ID) {
        RFX_LOG_D(RFX_LOG_TAG, "did not have corresponding RfxBaseData");
    }
    return NULL;
}

void RfxDataCloneManager::registerInternal(SortedVector<RfxDataCloneEntry> &list,
        RfxCopyDataByDataFuncptr copyByData, RfxCopyDataByObjFuncptr copyByObj,
        int id) {
    RfxCopyDataByDataFuncptr dataFuncptr = copyByData;
    RfxCopyDataByObjFuncptr objFuncptr = copyByObj;

    RfxDataCloneEntry entry(dataFuncptr, objFuncptr, id);
    size_t old_size = list.size();
    list.add(entry);
    if (list.size() == old_size) {
        // if same RfxBaseData, don't trigger assert
        const RfxDataCloneEntry& dup_entry = findDataCloneEntry(list, copyByData, copyByObj, id);
        if (dup_entry != entry) {
            RFX_LOG_E(RFX_LOG_TAG, "duplicated register the same request: %d", id);
            RFX_ASSERT(0);
        }
    }
    RFX_LOG_D(RFX_LOG_TAG, "id = %d, copyByData = %p, copyByObj = %p", id, copyByData, copyByObj);
}

const RfxDataCloneEntry& RfxDataCloneManager::findDataCloneEntry(SortedVector<RfxDataCloneEntry> &list,
        RfxCopyDataByDataFuncptr copyByData, RfxCopyDataByObjFuncptr copyByObj, int id) {
    RfxDataCloneEntry query_entry(copyByData, copyByObj, id);
    ssize_t index = list.indexOf(query_entry);
    if (index >= 0) {
        // for debug
        const RfxDataCloneEntry& result = list.itemAt(index);
        // RFX_LOG_D(RFX_LOG_TAG, "findDataCloneEntry, find entry for index = %zd, id = %d,\
// dataPtr = %p, objPtr = %p", index, result.m_id, result.m_copyByData, result.m_copyByObj);
        return list.itemAt(index);
    }

    if (id != INVALID_ID) {
        RFX_LOG_D(RFX_LOG_TAG, "findDataCloneEntry, doesn't find any entry for id = %d", id);
    }
    static RfxDataCloneEntry sDummyEntry = RfxDataCloneEntry();
    return sDummyEntry;
}

RfxCopyDataByDataFuncptr RfxDataCloneManager::findCopyDataByDataFuncptr(
        SortedVector<RfxDataCloneEntry> &entry, int id) {
    const RfxDataCloneEntry& result = findDataCloneEntry(entry, NULL, NULL, id);
    if (/*result != NULL &&*/ result != RfxDataCloneEntry()) {
        // RFX_LOG_D(RFX_LOG_TAG, "findCopyDataByObjFuncptr, result dataPtr = %p, objPtr = %p, id = %d",
        //         result.m_copyByData, result.m_copyByObj, result.m_id);
        return result.m_copyByData;
    }
    RFX_LOG_D(RFX_LOG_TAG, "findCopyDataByObjFuncptr, result is NULL");
    return NULL;
}

RfxCopyDataByObjFuncptr RfxDataCloneManager::findCopyDataByObjFuncptr(
        SortedVector<RfxDataCloneEntry> &entry, int id) {
    const RfxDataCloneEntry& result = findDataCloneEntry(entry, NULL, NULL, id);
    if (/*result != NULL &&*/ result != RfxDataCloneEntry()) {
        // RFX_LOG_D(RFX_LOG_TAG, "findCopyDataByObjFuncptr, result dataPtr = %p, objPtr = %p, id = %d",
        //         result.m_copyByData, result.m_copyByObj, result.m_id);
        return result.m_copyByObj;
    }
    RFX_LOG_D(RFX_LOG_TAG, "findCopyDataByObjFuncptr, result is NULL");
    return NULL;
}

SortedVector<RfxDataCloneEntry> RfxDataCloneManager::findDataCloneEntryList(int type) {
    switch(type) {
        case REQUEST:
            return m_request_list;
        case RESPONSE:
            return m_response_list;
        case URC:
            return m_urc_list;
        case EVENT:
            return m_event_list;
        default:
            RFX_LOG_E(RFX_LOG_TAG, "findDataCloneEntry: should not be here");
            RFX_ASSERT(0);
            break;
    }
}
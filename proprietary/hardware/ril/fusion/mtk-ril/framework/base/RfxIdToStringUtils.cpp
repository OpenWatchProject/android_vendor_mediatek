 /*
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

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RfxIdToStringUtils.h"
#include "RfxLog.h"

#define RFX_LOG_TAG "RfxIdToStr"

RfxIdToStringUtils* RfxIdToStringUtils::sSelf = NULL;

void RfxIdToStringUtils::init() {
    if (sSelf == NULL) {
        RFX_LOG_D(RFX_LOG_TAG, "init");
        sSelf = new RfxIdToStringUtils();
    }
}

void RfxIdToStringUtils::registerId(int id, char* str) {
    init();
    sSelf->registerInternal(sSelf->mIdList, id, str);
}

const char* RfxIdToStringUtils::idToString(int id) {
    init();
    Mutex::Autolock autoLock(sSelf->mMutex);
    const RfxIdMappingEntry& entry = sSelf->findIdEntry(sSelf->mIdList, id);
    return entry.mStr.string();
}

void RfxIdToStringUtils::registerInternal(SortedVector<RfxIdMappingEntry> &list, int id,
        char* str) {
    RfxIdMappingEntry entry(id, str);
    size_t old_size = list.size();
    list.add(entry);
    if (list.size() == old_size) {
        // if same RfxBaseData, don't trigger assert
        const RfxIdMappingEntry& dup_entry = findIdEntry(list, id);
        if (dup_entry != entry) {
            RFX_LOG_E(RFX_LOG_TAG, "duplicated register the same request: %d", id);
            RFX_ASSERT(0);
        }
    }
    RFX_LOG_D(RFX_LOG_TAG, "id = %d, string = %s", entry.mId, entry.mStr.string());
}

const RfxIdMappingEntry& RfxIdToStringUtils::findIdEntry(SortedVector<RfxIdMappingEntry> &list,
        int id) {
    RfxIdMappingEntry query_entry(id, String8());
    ssize_t index = list.indexOf(query_entry);
    if (index >= 0) {
        return list.itemAt(index);
    }

    // add unknow id to list and query again. SortVector would copy entry.
    // We can avoid to return local object
    RfxIdMappingEntry unknownEntry = RfxIdMappingEntry(id,
            String8::format("UNKNOWN ID: %d", id).string());
    list.add(unknownEntry);
    index = list.indexOf(query_entry);
    return list.itemAt(index);
}

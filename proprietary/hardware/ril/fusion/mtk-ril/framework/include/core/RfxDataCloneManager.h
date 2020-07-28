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

#ifndef __RFX_DATA_CLONE_MANAGER__H__
#define __RFX_DATA_CLONE_MANAGER__H__

#include "utils/SortedVector.h"
#include "utils/RefBase.h"
#include "RfxBaseData.h"
#include "RfxLog.h"
#include "RfxDefs.h"

using ::android::sp;
using ::android::wp;
using ::android::SortedVector;
using ::android::Vector;

/*****************************************************************************
 * Class RfxDataCloneEntry
 *****************************************************************************/

class RfxDataCloneEntry {
    public:
        RfxDataCloneEntry() :
            m_copyByData(NULL),
            m_copyByObj(NULL),
            m_id(-1){
        }

        RfxDataCloneEntry(RfxCopyDataByDataFuncptr copyByData, RfxCopyDataByObjFuncptr copyByObj, int id) :
            m_copyByData(copyByData),
            m_copyByObj(copyByObj),
            m_id(id){
        }

        RfxDataCloneEntry(const RfxDataCloneEntry &entry) :
            m_copyByData(entry.m_copyByData),
            m_copyByObj(entry.m_copyByObj),
            m_id(entry.m_id) {
        }

        bool operator< (const RfxDataCloneEntry &entry) const {
            return (m_id < entry.m_id);
        }

        bool operator> (const RfxDataCloneEntry &entry) const {
            return (m_id > entry.m_id);
        }

        bool operator== (const RfxDataCloneEntry &entry) const {
            return (m_id == entry.m_id);
        }
        bool operator!= (const RfxDataCloneEntry &entry) const {
            return (m_id != entry.m_id);
        }

    public:
        RfxCopyDataByDataFuncptr m_copyByData;
        RfxCopyDataByObjFuncptr m_copyByObj;
        int m_id;
};

/*****************************************************************************
 * Class RfxDataCloneManager
 *****************************************************************************/

class RfxDataCloneManager {
    public:
        RfxDataCloneManager() {}
        virtual ~RfxDataCloneManager(){}

        static void init();
        // register API
        static void registerRequestId(RfxCopyDataByDataFuncptr copyByData,
                RfxCopyDataByObjFuncptr copyByObj, int id);
        static void registerResponseId(RfxCopyDataByDataFuncptr copyByData,
                RfxCopyDataByObjFuncptr copyByObj, int id);
        static void registerUrcId(RfxCopyDataByDataFuncptr copyByData,
                RfxCopyDataByObjFuncptr copyByObj, int id);
        static void registerEventId(RfxCopyDataByDataFuncptr copyByData,
                RfxCopyDataByObjFuncptr copyByObj, int id);

        // copy API
        static RfxBaseData* copyData(int id, void *data, int length, int type);
        static RfxBaseData* copyData(int id, const RfxBaseData *data, int type);

    private:
        void registerInternal(SortedVector<RfxDataCloneEntry> &list,
                RfxCopyDataByDataFuncptr copyByData, RfxCopyDataByObjFuncptr copyByObj,
                int id);
        const RfxDataCloneEntry& findDataCloneEntry(SortedVector<RfxDataCloneEntry> &list,
                RfxCopyDataByDataFuncptr copyByData, RfxCopyDataByObjFuncptr copyByObj,
                int id);
        RfxCopyDataByDataFuncptr findCopyDataByDataFuncptr(
                SortedVector<RfxDataCloneEntry> &entry, int id);
        RfxCopyDataByObjFuncptr findCopyDataByObjFuncptr(
                SortedVector<RfxDataCloneEntry> &entry, int id);
        SortedVector<RfxDataCloneEntry> findDataCloneEntryList(int type);

    private:
        static RfxDataCloneManager* s_self;
        SortedVector<RfxDataCloneEntry> m_request_list;
        SortedVector<RfxDataCloneEntry> m_response_list;
        SortedVector<RfxDataCloneEntry> m_urc_list;
        SortedVector<RfxDataCloneEntry> m_event_list;
};

#endif

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

#ifndef __RFX_ID_TO_STRING_UTILS__
#define __RFX_ID_TO_STRING_UTILS__

#include "utils/String8.h"
#include "utils/SortedVector.h"
#include "utils/Mutex.h"

using ::android::Vector;
using ::android::SortedVector;
using ::android::String8;
using ::android::Mutex;

#define RFX_ID_TO_STR(id) RfxIdToStringUtils::idToString(id)

class RfxIdMappingEntry {
    public:
        RfxIdMappingEntry():
                mId(-1),
                mStr(String8()){
        }

        RfxIdMappingEntry(int id, const char* str):
                mId(id),
                mStr(str) {
        }

        RfxIdMappingEntry(const RfxIdMappingEntry &other):
                mId(other.mId),
                mStr(other.mStr){
        }

        bool operator< (const RfxIdMappingEntry &other) const {
            return (mId < other.mId);
        }

        bool operator> (const RfxIdMappingEntry &other) const {
            return (mId > other.mId);
        }

        bool operator== (const RfxIdMappingEntry &other) const {
            return (mId == other.mId);
        }

        bool operator!= (const RfxIdMappingEntry &other) const {
            return (mId != other.mId);
        }

    public:
        int mId;
        String8 mStr;
};

class RfxIdToStringUtils {

    public:
        RfxIdToStringUtils() {}
        virtual ~RfxIdToStringUtils(){}

        static void init();
        static void registerId(int id, char* str);
        static const char* idToString(int id);

    private:
        void registerInternal(SortedVector<RfxIdMappingEntry> &list, int id, char* str);
        const RfxIdMappingEntry& findIdEntry(SortedVector<RfxIdMappingEntry> &list, int id);

    private:
        static RfxIdToStringUtils* sSelf;
        SortedVector<RfxIdMappingEntry> mIdList;
        Mutex mMutex;
};

#endif

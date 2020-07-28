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

#ifndef __RFX_BASE_DATA__H__
#define __RFX_BASE_DATA__H__

//#include "RfxDataCloneManager.h"
#include <stdlib.h>
#include "RfxIdToStringUtils.h"
#include "RfxBasics.h"

// Predefined class
class RfxDataCloneManager;
class RfxBaseData;

typedef RfxBaseData* (*RfxCopyDataByDataFuncptr)(void *data, int length);
typedef RfxBaseData* (*RfxCopyDataByObjFuncptr)(const RfxBaseData *data);

#define RFX_DECLARE_DATA_CLASS(_class_name)                                  \
    private:                                                                 \
        _class_name(const _class_name& other);                               \
    public:                                                                  \
        _class_name(void *data, int length);                                 \
        virtual ~_class_name();                                              \
    public:                                                     \
        static RfxBaseData *copyDataByData(void *data, int length);          \
        static RfxBaseData *copyDataByObj(const RfxBaseData *data)

#define RFX_IMPLEMENT_DATA_CLASS(_class_name)                                 \
    RfxBaseData *_class_name::copyDataByData(void *data, int length) {         \
        _class_name *ret = new _class_name(data, length);                       \
        return ret;                                                             \
    }                                                                           \
                                                                                \
    RfxBaseData *_class_name::copyDataByObj(const RfxBaseData *data) {                     \
        _class_name *ret = new _class_name(data->getData(), data->getDataLength());           \
        return ret;                                                             \
    }

#define RFX_REGISTER_DATA_TO_REQUEST_ID(dispatch_data_class_name, response_data_class_name, id)  \
    class dispatch_data_class_name##dispatch##id {                                                 \
    public:                                                                                        \
        dispatch_data_class_name##dispatch##id(int _id) {                                           \
            RfxIdToStringUtils::registerId(id, (char *) #id);                               \
            RfxDataCloneManager::registerRequestId(&dispatch_data_class_name::copyDataByData, &dispatch_data_class_name::copyDataByObj, _id);\
        }                                                                          \
    };                                                                           \
    class response_data_class_name##response##id {                               \
    public:                                                                      \
        response_data_class_name##response##id(int _id) {                         \
            RfxDataCloneManager::registerResponseId(&response_data_class_name::copyDataByData, &response_data_class_name::copyDataByObj, _id);\
        }                                                                         \
    };                                                                            \
                                                                                  \
    dispatch_data_class_name##dispatch##id s_init_val##dispatch_data_class_name##dispatch##id(id);    \
    response_data_class_name##response##id s_init_val##response_data_class_name##response##id(id)


#define RFX_REGISTER_DATA_TO_URC_ID(data_class_name, id)                           \
        class data_class_name##id {                                                \
        public:                                                                      \
            data_class_name##id(int _id) {                                          \
                RfxIdToStringUtils::registerId(id, (char *) #id);                             \
                RfxDataCloneManager::registerUrcId(&data_class_name::copyDataByData, &data_class_name::copyDataByObj, _id);\
            }                                                                        \
        };                                                                           \
                                                                                     \
    data_class_name##id s_init_val##data_class_name##id(id)

#define RFX_REGISTER_DATA_TO_EVENT_ID(data_class_name, id)                           \
        class data_class_name##id {                                                \
        public:                                                                      \
            data_class_name##id(int _id) {                                          \
                RfxIdToStringUtils::registerId(id, (char *) #id);                             \
                RfxDataCloneManager::registerEventId(&data_class_name::copyDataByData, &data_class_name::copyDataByObj, _id);\
            }                                                                        \
        };                                                                           \
                                                                                     \
    data_class_name##id s_init_val##data_class_name##id(id)


class RfxBaseData {

    public:

        RfxBaseData(void *data, int length) : m_data(NULL), m_length(0) {
            RFX_UNUSED(data);
            RFX_UNUSED(length);
        }

        virtual ~RfxBaseData() {}

    public:
        void *getData() const {
            return m_data;
        }

        int getDataLength() const {
            return m_length;
        }

    protected:
        void *m_data;
        int m_length;
};

#endif

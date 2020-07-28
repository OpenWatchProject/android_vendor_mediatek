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

#ifndef WPFA_DRIVER_BASE_DATA_H
#define WPFA_DRIVER_BASE_DATA_H

#include <stdlib.h>


/*
 * =============================================================================
 *                     define
 * =============================================================================
 */

class WpfaDriverBaseData;  // Predefined class

#define WPFA_DECLARE_DATA_CLASS(_class_name)                                    \
    private:                                                                    \
        _class_name(const _class_name& other);                                  \
    public:                                                                     \
        _class_name(void *data, int length);                                    \
        virtual ~_class_name();                                                 \
    public:                                                                     \
        static WpfaDriverBaseData *copyDataByData(void *data, int length);      \
        static WpfaDriverBaseData *copyDataByObj(const WpfaDriverBaseData *data)

#define WPFA_IMPLEMENT_DATA_CLASS(_class_name)                                  \
    WpfaDriverBaseData *_class_name::copyDataByData(void *data, int length) {   \
        _class_name *ret = new _class_name(data, length);                       \
        return ret;                                                             \
    }                                                                           \
                                                                                \
    WpfaDriverBaseData *_class_name::copyDataByObj(const WpfaDriverBaseData *data) {    \
        _class_name *ret = new _class_name(data->getData(), data->getDataLength());     \
        return ret;                                                                     \
    }

#define WPFA_UNUSED(x)    ((void)(x))
/*
 * =============================================================================
 *                     class
 * =============================================================================
 */

class WpfaDriverBaseData {

    public:
        WpfaDriverBaseData(void *data, int length) : mData(NULL), mLength(0) {
            WPFA_UNUSED(data);
            WPFA_UNUSED(length);
        }

        virtual ~WpfaDriverBaseData() {}

        void *getData() const {
            return mData;
        }

        int getDataLength() const {
            return mLength;
        }


    protected:
        void *mData;
        int mLength;
};

#endif  /* end of WPFA_DRIVER_BASE_DATA_H */
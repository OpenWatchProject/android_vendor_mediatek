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

/*
**
** Copyright 2016, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#define LOG_NDEBUG 0
#define LOG_TAG "IWhiteListService"
#include <utils/String8.h>
#include <utils/String16.h>
#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>

#include "IMtkWLService.h"

namespace android
{

//#define ALOGV ALOGE

enum
{
    CHECKOPTION = IBinder::FIRST_CALL_TRANSACTION,
    OPTIONFORAPP,
};

class BpWhiteListService: public BpInterface<IWhiteListService>
{
    public:
        BpWhiteListService(const sp<IBinder> &impl)
            : BpInterface<IWhiteListService>(impl)
        {
        }

        virtual bool CheckOptionOnorOff(String8 FeatureName, String8 AppcationName, String8 CallerApp)
        {
            Parcel data, reply;
            data.writeInterfaceToken(IWhiteListService::getInterfaceDescriptor());
            data.writeString8(FeatureName);
            data.writeString8(AppcationName);
            data.writeString8(CallerApp);
            remote()->transact(CHECKOPTION, data, &reply); 
            return (bool)(reply.readInt32());
        }

        virtual bool GetOptionForApp(int32_t index, String8 *name, String8 *value)
        {
            Parcel data, reply;
            data.writeInterfaceToken(IWhiteListService::getInterfaceDescriptor());
            data.writeInt32(index);
            remote()->transact(OPTIONFORAPP, data, &reply); 
            bool err = (bool) reply.readInt32();
            if (err == true) {
                *name = reply.readString8();
                *value = reply.readString8();
                ALOGV("BpWhiteListService : %d -> name %s, value %s", index, name->string(), value->string());
            }
            else {
                ALOGV("BpWhiteListService : %d", index);
            }
            
            return err;
        }
};

IMPLEMENT_META_INTERFACE(WhiteListService, "android.option.IWhiteListService");

// ----------------------------------------------------------------------

status_t BnWhiteListService::onTransact(
    uint32_t code, const Parcel &data, Parcel *reply, uint32_t flags)
{
    switch (code)
    {
        case CHECKOPTION:
        {
            CHECK_INTERFACE(IWhiteListService, data, reply);
            String8 FeatureName = data.readString8();
            String8 AppcationName = data.readString8();
            String8 CallerApp = data.readString8();
            reply->writeInt32(CheckOptionOnorOff(FeatureName, AppcationName, CallerApp));
            return NO_ERROR;
        }
        break;
        case OPTIONFORAPP:
        {
            CHECK_INTERFACE(IWhiteListService, data, reply);
            int32_t index = data.readInt32();
            String8 _name_, _value_;
            bool err =  GetOptionForApp(index, &_name_, &_value_);
            reply->writeInt32(err);

            if (err == true) {
                ALOGV("BnWhiteListService : %d -> name %s, value %s", index, _name_.string(), _value_.string());
                reply->writeString8(_name_);
                reply->writeString8(_value_);
            }
            else {
                ALOGV("BnWhiteListService : %d", index);
            }
            return NO_ERROR;
        }
        break;
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

}; // namespace android


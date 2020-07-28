/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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
#include <utils/Log.h>
#include <binder/IServiceManager.h>
#include <binder/IPCThreadState.h>
#include <binder/Binder.h>
#include "ITerService.h"

namespace android {

enum {
    IS_EARLY_READ_SERVICE_ENABLED_TRANSACTION = IBinder::FIRST_CALL_TRANSACTION,
    SET_EARLY_READ_SERVICE_ENABLED_TRANSACTION,
    IS_EARLY_DATA_READY_TRANSACTION,
    GET_SIM_MCC_MNC_TRANSACTION
};

class BpTerService : public BpInterface<ITerService>
{
public:
    BpTerService(const sp<IBinder>& impl)
        : BpInterface<ITerService>(impl)
    {
    }

    virtual bool isEarlyReadServiceEnabled()
    {
        int32_t result = 0;

        Parcel data, reply;
        status_t err;

        RLOGI("[BpTerService] isEarlyReadServiceEnabled");

        data.writeInterfaceToken(ITerService::getInterfaceDescriptor());
        err = remote()->transact(IS_EARLY_READ_SERVICE_ENABLED_TRANSACTION, data, &reply);
        if (err == NO_ERROR)
        {
            reply.readInt32(&result);
        }
        else
        {
            RLOGE("[BpTerService] isEarlyReadServiceEnabled err=%d, exception code=%d", err, reply.readExceptionCode());
        }
        return (result == 1 ? true : false);
    }

    virtual void setEarlyReadServiceEnable(bool onoff)
    {
        Parcel data, reply;
        status_t err;

        RLOGI("[BpTerService] setEarlyReadServiceEnable");

        data.writeInterfaceToken(ITerService::getInterfaceDescriptor());
        if (onoff == true)
            data.writeInt32(1); // true
        else
            data.writeInt32(0); // false

        err = remote()->transact(SET_EARLY_READ_SERVICE_ENABLED_TRANSACTION, data, &reply);
        if (err != NO_ERROR)
        {
            RLOGE("[BpTerService] setEarlyReadServiceEnable onoff=%d, err=%d, exception code=%d", (onoff ? 1 : 0), err, reply.readExceptionCode());
        }

        return;
    }

    virtual bool isEarlyDataReady()
    {
        int32_t result = 0;

        Parcel data, reply;
        status_t err;

        RLOGI("[BpTerService] isEarlyDataReady");

        data.writeInterfaceToken(ITerService::getInterfaceDescriptor());
        err = remote()->transact(IS_EARLY_DATA_READY_TRANSACTION, data, &reply);
        if (err == NO_ERROR)
        {
            reply.readInt32(&result);
        }
        else
        {
            RLOGE("[BpTerService] isEarlyDataReady err=%d, exception code=%d", err, reply.readExceptionCode());
        }
        return (result == 1 ? true : false);
    }

    virtual status_t getSimMccMnc(String8* outStr)
    {
        status_t err = NO_ERROR;
        Parcel data, reply;

        RLOGI("[BpTerService] getSimMccMnc");

        data.writeInterfaceToken(ITerService::getInterfaceDescriptor());
        err = remote()->transact(GET_SIM_MCC_MNC_TRANSACTION, data, &reply);
        if (err == NO_ERROR)
        {
            const char* str = reply.readCString();
            outStr->setTo(str);
            RLOGI("[BpTerService] getSimMccMnc return1 (%s)(%zd)", outStr->string(),
                    outStr->length());
        }
        else
        {
            RLOGE("[BpTerService] getSimMccMnc err=%d, exception code=%d", err, reply.readExceptionCode());
        }

        return err;
    }

    virtual status_t getSimMccMncGemini(uint32_t simId, String8* outStr)
    {
        RLOGI("[BpTerService] getSimMccMncGemini simId=%d, outStr=\"%s\"", simId, outStr->string());

        return INVALID_OPERATION; // support single sim card only for now
    }

};

IMPLEMENT_META_INTERFACE(TerService, "com.mediatek.telephony.TerService");

status_t BnTerService::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch(code) {
        case IS_EARLY_READ_SERVICE_ENABLED_TRANSACTION: {
            bool result = false;

            CHECK_INTERFACE(ITerServer, data, reply);

            result = isEarlyReadServiceEnabled();
            if (result == true)
                reply->writeInt32(1); // true
            else
                reply->writeInt32(0); // false

            return NO_ERROR;
        } break;
        case SET_EARLY_READ_SERVICE_ENABLED_TRANSACTION: {
            bool onoff = false;

            CHECK_INTERFACE(ITerServer, data, reply);
            int32_t param = data.readInt32();
            if (param == 1)
                onoff = true;
            else
                onoff = false;

            setEarlyReadServiceEnable(onoff);

            return NO_ERROR;
        } break;
        case IS_EARLY_DATA_READY_TRANSACTION: {
            bool result = false;

            CHECK_INTERFACE(ITerServer, data, reply);

            result = isEarlyDataReady();
            if (result == true)
                reply->writeInt32(1); // true
            else
                reply->writeInt32(0); // false

            return NO_ERROR;
        } break;
        case GET_SIM_MCC_MNC_TRANSACTION: {
            status_t result = NO_ERROR;

            CHECK_INTERFACE(ITerServer, data, reply);

            mMccMnc.clear();
            result = getSimMccMnc(&mMccMnc);
            if (result == NO_ERROR)
            {
                reply->writeCString(mMccMnc.string());
            }

            return result;
        } break;
    }
    return BBinder::onTransact(code, data, reply, flags);
}

};

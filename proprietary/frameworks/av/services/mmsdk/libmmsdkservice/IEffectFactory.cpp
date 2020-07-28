/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

//#include <cutils/xlog.h>
#include <cutils/log.h>
#include <utils/Errors.h>
#include <gui/IGraphicBufferProducer.h>
#include <mmsdk/IEffectHal.h>


/******************************************************************************
 *
 ******************************************************************************/
#define FUNCTION_LOG_START      ALOGD("[%s] - E.", __FUNCTION__)
#define FUNCTION_LOG_END        ALOGD("[%s] - X.", __FUNCTION__)


using namespace NSCam;
using namespace android;

//
enum {
    CREATE_CALLBACK_CLIENT = IBinder::FIRST_CALL_TRANSACTION,
};


class BpEffectFactory : public BpInterface<IEffectFactory>
{
public:
    BpEffectFactory(const sp<IBinder>& impl)
        : BpInterface<IEffectFactory>(impl)
    {
        FUNCTION_LOG_START;
        //@todo implement this
        FUNCTION_LOG_END;
    }


    virtual ~BpEffectFactory()
    {
        FUNCTION_LOG_START;
        //@todo implement this
        FUNCTION_LOG_END;
    }

    virtual status_t            createCallbackClient(EffectHalVersion nameVersion, sp<ICallbackClient> &callbackClient)
    {
        FUNCTION_LOG_START;
        Parcel data, reply;
        data.writeInterfaceToken(IEffectFactory::getInterfaceDescriptor());
        data.write(nameVersion);

        status_t result = remote()->transact(CREATE_CALLBACK_CLIENT, data, &reply);

        if (reply.readExceptionCode()) return -EPROTO;
        if (reply.readInt32() != 0)
        {
            callbackClient = interface_cast<ICallbackClient>(reply.readStrongBinder());
        }
        FUNCTION_LOG_END;
        return result;
    }
};


IMPLEMENT_META_INTERFACE(EffectFactory, "com.mediatek.mmsdk.IEffectFactory");


status_t BnEffectFactory::onTransact(
        uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    FUNCTION_LOG_START;
    ALOGD("[%s] - code=%d, %zu,  %zu", __FUNCTION__, code, data.dataSize(), data.dataPosition());

    switch(code) {
        case CREATE_CALLBACK_CLIENT: {
            CHECK_INTERFACE(IEffectFactory, data, reply);
            EffectHalVersion version;
            version.effectName = "vsdofcb";
            version.major = 100;
            version.minor = 0;
            if (data.readInt32() == 1)
            {
                version.effectName = String8(data.readString16());
                version.major = data.readInt32();
                version.minor = data.readInt32();
            }

            //
            sp<ICallbackClient> callbackclient;
            status_t status = createCallbackClient(version, callbackclient);
            reply->writeNoException();
            reply->writeInt32(status);
            if(callbackclient != 0) {
                reply->writeInt32(1);
                //reply->writeStrongBinder(effect->asBinder());
                reply->writeStrongBinder(IInterface::asBinder(callbackclient));
            } else {
                reply->writeInt32(0);
            }
            //
            return NO_ERROR;
        } break;
    }
    FUNCTION_LOG_END;
    return BBinder::onTransact(code, data, reply, flags);
}


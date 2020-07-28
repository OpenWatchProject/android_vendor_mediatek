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
#define LOG_TAG "mmsdk/ICallbackClient"

#include <sys/types.h>
#include <cutils/log.h>
#include <utils/Errors.h>
#include <utils/String8.h>
#include <gui/IGraphicBufferProducer.h>
#include <gui/Surface.h>
#include <gui/view/Surface.h>
#include <mmsdk/IEffectHal.h>

#include <utils/List.h>


/******************************************************************************
 *
 ******************************************************************************/
#define FUNCTION_LOG_START      ALOGD("[%s] - E.", __FUNCTION__)
#define FUNCTION_LOG_END        ALOGD("[%s] - X.", __FUNCTION__)


using namespace NSCam;
using namespace android;

enum
{
//public: // may change state
    START = IBinder::FIRST_CALL_TRANSACTION,
    STOP,
    SET_OUTPUT_SURFACES,
};


class BpCallbackClient : public BpInterface<ICallbackClient>
{
public:
    BpCallbackClient(const sp<IBinder> &impl)
        : BpInterface<ICallbackClient>(impl)
    {
    }

public: // may change state
    virtual uint64_t   start()
    {
        FUNCTION_LOG_START;
        Parcel data, reply;
        data.writeInterfaceToken(ICallbackClient::getInterfaceDescriptor());
        remote()->transact(START, data, &reply);

        uint64_t uid = 0;
        int32_t exceptionCode = reply.readExceptionCode();
        if (!exceptionCode)
        {
            uid = reply.readInt64();
        }
        else
        {
            // An exception was thrown back; fall through to return failure
            ALOGE("caught exception %d\n", exceptionCode);
        }
        FUNCTION_LOG_END;
        return uid;
    }
    virtual uint64_t   stop()
    {
        FUNCTION_LOG_START;
        Parcel data, reply;
        data.writeInterfaceToken(ICallbackClient::getInterfaceDescriptor());
        remote()->transact(STOP, data, &reply);

        uint64_t uid = 0;
        int32_t exceptionCode = reply.readExceptionCode();
        if (!exceptionCode)
        {
            uid = reply.readInt64();
        }
        else
        {
            // An exception was thrown back; fall through to return failure
            ALOGE("caught exception %d\n", exceptionCode);
        }
        FUNCTION_LOG_END;
        return uid;
    }
    virtual status_t   setOutputSurfaces(Vector< sp<IGraphicBufferProducer> > &input, Vector<sp<EffectParameter> > &effectParam)
    {
        // avoid compile unsed parameter warning
        (void)input;
        (void)effectParam;
        FUNCTION_LOG_START;
        FUNCTION_LOG_END;
        return OK;
    }
    virtual bool isDualCB()
    {
        return true;
    }

};


IMPLEMENT_META_INTERFACE(CallbackClient, "com.mediatek.mmsdk.callback.ICallbackClient");


status_t BnCallbackClient::onTransact(
    uint32_t code, const Parcel &data, Parcel *reply, uint32_t flags)
{
    FUNCTION_LOG_START;
    ALOGD("[%s] - code=%d, %zu,  %zu", __FUNCTION__, code, data.dataSize(), data.dataPosition());
    data.setDataPosition(0);
    switch(code)
    {
//public: // may change state
    case START:
    {
        CHECK_INTERFACE(ICallbackClient, data, reply);
        uint64_t uid = start();
        reply->writeNoException();
        reply->writeInt64(uid);
        return OK;
    }
    break;

    case STOP:
    {
        CHECK_INTERFACE(ICallbackClient, data, reply);
        uint64_t uid = stop();
        reply->writeNoException();
        reply->writeInt64(uid);
        return OK;
    }
    break;

    case SET_OUTPUT_SURFACES:
    {
        CHECK_INTERFACE(ICallbackClient, data, reply);
        //get vector size
        int size = data.readInt32();
        if(size == -1)
        {
            ALOGD("List is null, size=%d", size);
            return OK;
        }

        Vector<sp<IGraphicBufferProducer> > outputSurfaces;
        int num=0;
        //get surface
        while(num<size)
        {
            int is_surface = data.readInt32();
            if(is_surface != 0)
            {
                view::Surface surface;
                if(surface.readFromParcel(&data) != OK)
                {
                    ALOGE("%s: SET_OUTPUT_SURFACES: Surface.readFromParcel error!", __FUNCTION__);
                }

                outputSurfaces.push_back(surface.graphicBufferProducer);
            }
#if 0
            String16 name;
            int is_surface = data.readInt32();
            //check each surface is null or not
            if(is_surface != 0)
            {
                // String16 name = readMaybeEmptyString16(data);
                size_t len;
                const char16_t* str = data.readString16Inplace(&len);
                if (str != NULL)
                {
                    name = String16(str, len);
                } else
                {
                    name = String16();
                }
                ALOGD("SET_OUTPUT_SURFACES name=%s", String8(name).string());

                //convert to IGraphicBufferProducer pointer and push_back to outputSurfaces
                sp<IGraphicBufferProducer> surface = interface_cast<IGraphicBufferProducer>(data.readStrongBinder());
                if (surface == NULL)
                {
                    ALOGE("%s: SET_OUTPUT_SURFACES: surface = unset, name= %s", __FUNCTION__, String8(name).string());
                }
                outputSurfaces.push_back(surface);
            }
            else
            {
                ALOGE("%s: SET_OUTPUT_SURFACES: surface = unset, name = unset", __FUNCTION__);
            }
#endif
            num++;
        }

        //get effectParameter
        Vector<sp<EffectParameter> > parameters;
        num=0;
        size = data.readInt32();
        if(size == -1)
        {
            ALOGD("EffectParameter List is null, size=%d", size);
            return OK;
        }
        while(num<size)
        {
            sp<EffectParameter> parameter = new EffectParameter();
            //EffectParameter parameter;
            int size = data.readInt32();
            if(size != 0)
            {
                const String8 param = String8(data.readString16());
                ALOGD("[%s]: param string=%s", __FUNCTION__, param.string());
                parameter->unflatten(param);
            }
            parameters.push_back(parameter);
            num++;
        }
        status_t _result;
        //check the amount of surfaces and parameters are same
        if (parameters.size() != outputSurfaces.size())
        {
            ALOGE("[%s]: parameters.size(%zu) not equal to outputSurfaces.size(%zu)",
                __FUNCTION__, parameters.size(), outputSurfaces.size());
            _result = BAD_VALUE;
        }
        else
        {
            _result = setOutputSurfaces(outputSurfaces, parameters);
        }

        reply->writeNoException();
        reply->writeInt32(_result);
        return OK;
    }
    break;
    }
    FUNCTION_LOG_END;
    return BBinder::onTransact(code, data, reply, flags);
}

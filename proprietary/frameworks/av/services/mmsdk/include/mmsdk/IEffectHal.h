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

#ifndef _MTK_MMSDK_IEFFECTHAL_H_
#define _MTK_MMSDK_IEFFECTHAL_H_

#include <utils/String8.h>
#include <utils/Vector.h>
#include <utils/RefBase.h>
#include <utils/KeyedVector.h>
#include <utils/Errors.h>
#include <utils/Singleton.h>
#include <utils/Flattenable.h>

#include <binder/IInterface.h>
#include <binder/Parcel.h>

#include <gui/IGraphicBufferProducer.h>
#include <gui/IGraphicBufferConsumer.h>
#include <gui/BufferItemConsumer.h>
#include <gui/IGraphicBufferProducer.h>
#include <gui/BufferItem.h>

//get camera3_jpeg_blob
#include <hardware/camera3.h>
//
#include <mmsdk/BasicParameters.h>
//
#include <vendor/mediatek/hardware/camera/callbackclient/1.1/IMtkCallbackClient.h>
#include <vendor/mediatek/hardware/camera/callbackclient/1.1/IMtkBufferCallback.h>
//
#include<mutex>

typedef uint64_t            MUINT64;
typedef unsigned int        MUINT32;
typedef unsigned short      MUINT16;
typedef unsigned char       MUINT8;

typedef int64_t             MINT64;
typedef signed int          MINT32;
typedef signed short        MINT16;
typedef signed char         MINT8;

typedef int                 MBOOL;
typedef float               MFLOAT;
typedef void                MVOID;

typedef uintptr_t           MUINTPTR;
typedef intptr_t            MINTPTR;

/******************************************************************************
 *
 ******************************************************************************/

namespace NSCam
{
using namespace android;

//@todo remove this
class ICallbackClient;

class EffectHalVersion : public LightFlattenable<EffectHalVersion>
{
public:    //LightFlattenable
    inline  bool        isFixedSize() const
    {
        return false;
    }
    size_t              getFlattenedSize() const;
    status_t   flatten(void* buffer, size_t size) const;
    status_t   unflatten(void const* buffer, size_t size);

private:
    static void flattenString8(void*& buffer, size_t& size, const String8& string8);
    static bool unflattenString8(void const*& buffer, size_t& size, String8& outputString8);

public:    //@todo private
    String8     effectName;
    uint32_t    major;
    uint32_t    minor;
};

//-----------------------------------------------------------------------------
//IEffectFactory
//-----------------------------------------------------------------------------
class IEffectFactory : public IInterface
{
public:
    DECLARE_META_INTERFACE(EffectFactory);

public:
    /**
     *  @brief                  Create ICallbackClient by name and version.
     *  @param[in] nameVersion  Specific effect name and verison
     *  @param[out] callbackClient   Point to created ICallbackClient instance.
     *  @return                 status_t
     */
    virtual status_t   createCallbackClient(EffectHalVersion nameVersion, sp<ICallbackClient> &callbackClient) = 0;
};


class BnEffectFactory : public BnInterface<IEffectFactory>
{
public:
    virtual status_t    onTransact( uint32_t code,
                                    const Parcel& data,
                                    Parcel* reply,
                                    uint32_t flags = 0);
};


class EffectFactory : public BnEffectFactory
{
public:
    //EffectFactory(sp<IEffectFactory> const &client);
    //virtual     ~EffectFactory(){};
    EffectFactory();
    virtual     ~EffectFactory();

public:
    virtual status_t   createCallbackClient(EffectHalVersion nameVersion, sp<ICallbackClient> &callbackClient);

// Using static variables for saving the current ICallbackClient instance.
// While the next time that caller invokes EffectFactory::createCallbackClient,
// we should clear the ICallbackClient resource (Surfaces) first, or the memory
// may have probability to leak.
private:
    static std::mutex                   msCBClientMutex;
    static android::wp<ICallbackClient> mwpCBClient;
    static android::wp<ICallbackClient> mwpCBClientDual;
};

//-----------------------------------------------------------------------------
//ICallbackClient
//-----------------------------------------------------------------------------
class ICallbackClient : public IInterface
{
public:
    DECLARE_META_INTERFACE(CallbackClient);
//-----------------------------------------------------------------------------
//API
//-----------------------------------------------------------------------------
public:
    virtual uint64_t   start() = 0;
    virtual uint64_t   stop() = 0;
    virtual status_t   setOutputSurfaces(Vector< sp<IGraphicBufferProducer> > &input, Vector<sp<EffectParameter> > &effectParams) = 0;

public:
    virtual bool isDualCB() = 0;
};

class BnCallbackClient : public BnInterface<ICallbackClient>
{
public:
    virtual status_t    onTransact( uint32_t code,
                                    const Parcel& data,
                                    Parcel* reply,
                                    uint32_t flags = 0);
};

class CallbackClient :
          public BnCallbackClient,
          public virtual RefBase,
          public virtual vendor::mediatek::hardware::camera::callbackclient::V1_1::IMtkBufferCallback,
          public virtual hardware::hidl_death_recipient
{
public:
    CallbackClient(String8 name, MUINT64 timestamp);
    virtual     ~CallbackClient();
    virtual bool       init();

public:
    virtual uint64_t   start();
    virtual uint64_t   stop();
    virtual status_t   setOutputSurfaces(Vector< sp<IGraphicBufferProducer> > &input, Vector<sp<EffectParameter> > &effectParams);

public:
    virtual bool isDualCB();

public:
    hardware::Return<void>                                    dequeueBuffer(int32_t msgType, dequeueBuffer_cb _hidl_cb) override;
    hardware::Return<hardware::camera::common::V1_0::Status>  enqueueBuffer(int32_t msgType, uint64_t bufferId)         override;
    hardware::Return<hardware::camera::common::V1_0::Status>  cancelBuffer (int32_t msgType, uint64_t bufferId)         override;
    hardware::Return<hardware::camera::common::V1_0::Status>  setTimestamp (int32_t msgType, int64_t timestamp)         override;
    hardware::Return<hardware::camera::common::V1_0::Status>  lock()   override;
    hardware::Return<hardware::camera::common::V1_0::Status>  unlock() override;

public:
    // hidl_death_recipient interface
    virtual void serviceDied(uint64_t cookie, const wp<hidl::base::V1_0::IBase>& who) override;

private:
    std::pair<bool, uint64_t>   getBufferId(ANativeWindowBuffer* anb);
    void cleanupCirculatingBuffers();

protected:
    MINT32                      mLogLevel;

private:
    String8                     mName;
    MUINT64                     mTimeStamp;
    MUINT32                     mSize; // size of surfaces
    MUINT32                     mCallbackID; // 0: single, 1: dual
    std::mutex                  mMutex;
    KeyedVector<MINT32, sp<Surface>>  mOutputSurfacesMap;  // <msgType, surface>

    std::mutex                  mOutPutSurfacesMutex;
//-----------------------------------------------------------------------------
// buffer map
//-----------------------------------------------------------------------------
    struct CbBufHasher {
        size_t operator()(const buffer_handle_t& buf) const {
            if (buf == nullptr)
                return 0;

            size_t result = 1;
            result = 31 * result + buf->numFds;
            result = 31 * result + buf->numInts;
            int length = buf->numFds + buf->numInts;
            for (int i = 0; i < length; i++) {
                result = 31 * result + buf->data[i];
            }
            return result;
        }
    };

    struct CbBufComparator {
        bool operator()(const buffer_handle_t& buf1, const buffer_handle_t& buf2) const {
            if (buf1->numFds == buf2->numFds && buf1->numInts == buf2->numInts) {
                int length = buf1->numFds + buf1->numInts;
                for (int i = 0; i < length; i++) {
                    if (buf1->data[i] != buf2->data[i]) {
                        return false;
                    }
                }
                return true;
            }
            return false;
        }
    };
    std::mutex                  mBufferIdMapLock;
    typedef std::unordered_map<const buffer_handle_t, uint64_t, CbBufHasher, CbBufComparator> BufferIdMap;
    BufferIdMap mBufferIdMap;
    std::unordered_map<uint64_t, ANativeWindowBuffer*> mReversedBufMap;
    uint64_t mNextBufferId = 1;
    android::sp<vendor::mediatek::hardware::camera::callbackclient::V1_1::IMtkCallbackClient> mCallbackClient;

};
};  //namespace NSCam {
android::sp<NSCam::CallbackClient> makeCallbackClient(android::String8 name, MUINT64 timestamp);

#endif  //_MTK_MMSDK_IEFFECTHAL_H_


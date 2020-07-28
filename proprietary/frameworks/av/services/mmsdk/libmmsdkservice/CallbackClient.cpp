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

#define LOG_TAG "mmsdk/CallbackClient"

#include <log/log.h>
#include <utils/Errors.h>

#include <mmsdk/IEffectHal.h>
#include <gui/Surface.h>
#include <utils/List.h>

#include <ui/GraphicBufferMapper.h>

//use property_get
#include <cutils/properties.h>
//
//#include <camera/mediatek/MtkCamera.h>
#include <system/camera.h>
#define MTK_CAMERA_MSG_EXT_DATA_STEREO_CLEAR_IMAGE 0x00000015 // REMOVE ME!!
//
#define CAM_LOGV ALOGV
#define CAM_LOGD ALOGD
#define CAM_LOGI ALOGI
#define CAM_LOGW ALOGW
#define CAM_LOGE ALOGE
#define CAM_LOGA ALOGA
#define CAM_LOGF ALOGF
/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

// #define FUNCTION_LOG_START          MY_LOGD_IF(1<=mLogLevel, "+");
// #define FUNCTION_LOG_END            MY_LOGD_IF(1<=mLogLevel, "-");
#define FUNCTION_LOG_START          MY_LOGD_IF(1<=1, " +");
#define FUNCTION_LOG_END            MY_LOGD_IF(1<=1, " -");
/******************************************************************************
 *
 ******************************************************************************/

#include <vendor/mediatek/hardware/camera/callbackclient/1.1/IMtkCallbackClient.h>
#include <vendor/mediatek/hardware/camera/callbackclient/1.0/IMtkBufferCallback.h>
#include <vendor/mediatek/hardware/camera/callbackclient/1.1/types.h>
#include <android/hardware/camera/device/3.2/types.h>
#include <android/hardware/camera/common/1.0/types.h>

//get camera3_jpeg_blob
#include <hardware/camera3.h>

/******************************************************************************
 *
 ******************************************************************************/

using namespace std;
using namespace NSCam;
using namespace android;
using vendor::mediatek::hardware::camera::callbackclient::V1_1::IMtkCallbackClient;
using vendor::mediatek::hardware::camera::callbackclient::V1_1::MtkCBClientID;
using vendor::mediatek::hardware::camera::callbackclient::V1_1::MtkCBClientCmdType;
using namespace hardware::camera::common::V1_0;
using hardware::hidl_handle;
using ::android::hardware::hidl_vec;


/******************************************************************************
 *
 ******************************************************************************/
android::sp<CallbackClient> makeCallbackClient(String8 name, MUINT64 timestamp)
{
    android::sp<CallbackClient> c = new CallbackClient(name, timestamp);
    if  (c.get() && c->init()) {
        return c;
    }
    return nullptr;
}


/******************************************************************************
 *
 ******************************************************************************/
void
CallbackClient::
serviceDied(uint64_t cookie __unused, const wp<hidl::base::V1_0::IBase>& who __unused)
{
    MY_LOGI("cookie:%" PRIu64 "", cookie);

    if (!mOutPutSurfacesMutex.try_lock()) {
        MY_LOGW("mOutPutSurfacesMutex is still locked during serviceDied");
    }
    MY_LOGD("unlock mOutPutSurfacesMutex for CallbackClient recovery");
    mOutPutSurfacesMutex.unlock();

    mCallbackClient = nullptr;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
CallbackClient::
init()
{
    FUNCTION_LOG_START;

    //create CallbackClientStub by HIDL
    mCallbackClient = IMtkCallbackClient::tryGetService("internal/0");
    if (mCallbackClient != NULL)
    {
        hardware::Return<bool> linked = mCallbackClient->linkToDeath(this, /*cookie*/ 0);
        if (!linked.isOk()) {
            MY_LOGE("Transaction error in linking to IMtkCallbackClient death: %s", linked.description().c_str());
        } else if (!linked) {
            MY_LOGW("Unable to link to IMtkCallbackClient death notifications");
        }

        mCallbackClient->createCallbackClient(mTimeStamp);
        mCallbackClient->sendCommand(mTimeStamp, MtkCBClientCmdType::MTK_CALLBACK_CLIENT_CMD_SET_CALLBACK_ID, mCallbackID, 0);
    }
    else
    {
        MY_LOGD("ERROR : mCallbackClient is NULL");
        return false;
    }

    FUNCTION_LOG_END;
    return true;
}
/******************************************************************************
 *
 ******************************************************************************/
bool
CallbackClient::
isDualCB()
{
  return (mCallbackID == (MUINT32)MtkCBClientID::MTK_CALLBACK_CLIENT_ID_DUAL);
}
/******************************************************************************
 *
 ******************************************************************************/
CallbackClient::
CallbackClient(String8 name, MUINT64 timestamp)
{
    char cLogLevel[PROPERTY_VALUE_MAX];
#if 0
    ::property_get("debug.camera.log", cLogLevel, "0");
    mLogLevel = ::atoi(cLogLevel);
    if ( 0 == mLogLevel ) {
        ::property_get("debug.camera.log.callback", cLogLevel, "1");
        mLogLevel = ::atoi(cLogLevel);
    }
#else
    mLogLevel = 0;
#endif
    mName = name;
    mTimeStamp = timestamp;
    mSize = 0;
    mCallbackID = (mName == "Null" || mName == "") ? (MUINT32)MtkCBClientID::MTK_CALLBACK_CLIENT_ID_DUAL : (MUINT32)atoi(mName);
    MY_LOGD("mCallbackID %d", mCallbackID);
}

/******************************************************************************
 *
 ******************************************************************************/
CallbackClient::
~CallbackClient()
{
    FUNCTION_LOG_START;
    {
        std::unique_lock <std::mutex> l(mOutPutSurfacesMutex);
        mOutputSurfacesMap.clear();
    }

    //destroy CallbackClientStub by HIDL
    if (mCallbackClient != NULL)
    {
        mCallbackClient->destroyCallbackClient(mTimeStamp);
        mCallbackClient.clear();
    }
    else
    {
      MY_LOGD("ERROR : pCallbackClient is NULL");
    }
    FUNCTION_LOG_END;
}

/******************************************************************************
 *
 ******************************************************************************/
uint64_t
CallbackClient::
start()
{
    FUNCTION_LOG_START;
    FUNCTION_LOG_END;
    return 0;
}

/******************************************************************************
 *
 ******************************************************************************/
uint64_t
CallbackClient::
stop()
{
    FUNCTION_LOG_START;
    {
        std::unique_lock <std::mutex> l(mOutPutSurfacesMutex);
        /* Clear surface to trigger CallbackClient destuctor, why ? */
        mOutputSurfacesMap.clear();
    }
    //destroy CallbackClientStub by HIDL
    if (mCallbackClient != NULL)
    {
        mCallbackClient->destroyCallbackClient(mTimeStamp);
    }
    else
    {
      MY_LOGD("ERROR : pCallbackClient is NULL");
    }

    FUNCTION_LOG_END;
    return 0;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
CallbackClient::
setOutputSurfaces(Vector< sp<IGraphicBufferProducer> > &ouput, Vector<sp<EffectParameter> > &effectParams)
{
    FUNCTION_LOG_START;
    std::unique_lock <std::mutex> l(mOutPutSurfacesMutex);
    std::vector<int32_t> vMsgType;
    // init surfaces and parameters
    mOutputSurfacesMap.clear();

    size_t outputFrameCount = 0;
    //configure
    for (size_t idx = 0; idx < ouput.size(); ++idx)
    {
        sp<Surface> s = new Surface(ouput[idx]);
        sp<ANativeWindow> anw = s;
        MINT32 msgType = effectParams[idx]->getInt("callback-msg-type");
        MY_LOGD("add msgType =%d, native window is: %p", msgType, anw.get());
        mOutputSurfacesMap.add(msgType, s);
        status_t res;

        MY_LOGD_IF(mLogLevel, "idx=%zu, producer(Binder):%p", idx, IInterface::asBinder(ouput[idx]).get());

        // connect to bufferQueue
        res = native_window_api_connect(anw.get(), NATIVE_WINDOW_API_CAMERA);
        MY_LOGD_IF(mLogLevel, "connect=%d, native window is: %p", res, anw.get());
        if (res != OK)
        {
            MY_LOGE("Unable to connect ANativeWindow: %s (%d)  [%s]", strerror(-res), res, mName.string());
            return res;
        }

        // query width, height format and usage info. for debug
        int width, height, format, size;
        int32_t consumerUsage;
        if ((res = anw->query(anw.get(), NATIVE_WINDOW_WIDTH, &width)) != OK) {
            MY_LOGE("Failed to query Surface width");
            return res;
        }
        if ((res = anw->query(anw.get(), NATIVE_WINDOW_HEIGHT, &height)) != OK) {
            MY_LOGE("Failed to query Surface height");
            return res;
        }
        if ((res = anw->query(anw.get(), NATIVE_WINDOW_FORMAT, &format)) != OK) {
            MY_LOGE("Failed to query Surface format");
            return res;
        }
        if ((res = anw->query(anw.get(), NATIVE_WINDOW_CONSUMER_USAGE_BITS, &consumerUsage)) != OK) {
            MY_LOGE("Failed to query consumer usage");
            return res;
        }
        MY_LOGD("(w = %d, h = %d), format:(0x%x), consumerUsage=%d", width, height, format, consumerUsage);


        // Configure consumer-side ANativeWindow interface
        //GRALLOC_USAGE_HW_CAMERA_MASK        = 0x00060000
        //GRALLOC_USAGE_SW_WRITE_OFTEN        = 0x00000030
        //GRALLOC_USAGE_SW_READ_OFTEN         = 0x00000003
//        int32_t effectUsage = 0x60033;
        int32_t effectUsage = GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_SW_READ_OFTEN;
        res = native_window_set_usage(anw.get(), effectUsage);
        MY_LOGD_IF(mLogLevel, "[%s]: native_window_set_usage :%08x", __FUNCTION__, effectUsage);
        if (res != OK) {
            MY_LOGE("Unable to configure usage %08x", effectUsage);
        }

        res = native_window_set_scaling_mode(anw.get(), NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW);
        if (res != OK) {
            MY_LOGE("Unable to configure stream scaling: %s (%d)",
                    strerror(-res), res);
        }
#if 1
        size = (msgType == MTK_CAMERA_MSG_EXT_DATA_STEREO_CLEAR_IMAGE || msgType == CAMERA_MSG_COMPRESSED_IMAGE) ?
               ((width * height * 15 / 10) + 328448) : (width * height);
        size += sizeof(camera3_jpeg_blob) +64;
        res = native_window_set_buffers_dimensions(anw.get(), size , 1);
#else
        if (msgType == MTK_CAMERA_MSG_EXT_DATA_STEREO_CLEAR_IMAGE || msgType == CAMERA_MSG_COMPRESSED_IMAGE)
        {
            res = native_window_set_buffers_dimensions(anw.get(), (width * height * 15 / 10) + 328448, 1);
        }
        else
        {
            res = native_window_set_buffers_dimensions(anw.get(), width, height);
        }
#endif
        MY_LOGD_IF(mLogLevel, "set dimension: (w = %d, h = %d)", width, height);

        if (res != OK) {
            MY_LOGE("Unable to configure stream buffer dimensions %d x %d", width, height);
        }

        //set buffer format, can remove if ap is configed.
        res = native_window_set_buffers_format(anw.get(), format);
        MY_LOGD_IF(mLogLevel, "native_window_set_buffers_format :format:(0x%x)", format);
        if (res != OK) {
            MY_LOGE("Unable to configure stream buffer format %#x", format);
            return res;
        }

        int maxConsumerBuffers;
        res = anw->query(anw.get(), NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS, &maxConsumerBuffers);
        if (res != OK) {
            MY_LOGE("Unable to query consumer undequeued");
            return res;
        }

        MY_LOGD_IF(mLogLevel, "Consumer wants %d buffers", maxConsumerBuffers);
        MY_LOGD_IF(mLogLevel, "picture-numbers=%d", effectParams[idx]->getInt("picture-number"));

        outputFrameCount = effectParams[idx]->getInt("picture-number");
        //ap have to set this
        res = native_window_set_buffer_count(anw.get(), 10);
        if (res != OK)
        {
            MY_LOGE("Unable to native_window_set_buffer_count: %s (%d)", strerror(-res), res);
            return OK;
        }
        vMsgType.push_back(msgType);
#if 0
        //for testing
        MY_LOGD("Dequeue Test++");
        ANativeWindowBuffer* anb;
        res = native_window_dequeue_buffer_and_wait(anw.get(), &anb);
        if (res != OK)
        {
            MY_LOGE("Unable to dequeue buffer: %s (%d) msgType:%d",
                strerror(-res), res, msgType);
        }
        else
        {
            sp<GraphicBuffer> buf(new GraphicBuffer(anb, false));
            MY_LOGD("cancelBuffer++");
            res = anw->cancelBuffer(anw.get(), buf->getNativeBuffer(), -1);
            if (res != OK)
            {
                MY_LOGE("Unable to cancelBuffer %s (%d) msgType:%d",
                    strerror(-res), res, msgType);
            }
            MY_LOGD("cancelBuffer--");
        }
        MY_LOGD("Dequeue Test--");
#endif
    }

    mSize = mOutputSurfacesMap.size();
    cleanupCirculatingBuffers();
    // setOutputSurfaces by HIDL, pass this instance for HAL to callback
    if (mCallbackClient != NULL)
    {
        android::hardware::hidl_vec<int32_t> vHidlMsgType = vMsgType;
        mCallbackClient->setOutputSurfaces(mTimeStamp, this, vHidlMsgType);
    }
    else
    {
        MY_LOGD("ERROR : pCallbackClient is NULL");
    }

    FUNCTION_LOG_END;
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
hardware::Return<void>
CallbackClient::
dequeueBuffer(int32_t msgType, dequeueBuffer_cb _hidl_cb)
{
    FUNCTION_LOG_START;
    std::unique_lock <std::mutex> l(mMutex);
    sp<ANativeWindow> anw;
    ANativeWindowBuffer* anb;
    Status s  = Status::INTERNAL_ERROR;
    uint64_t bufferId = 0;
    uint32_t stride   = 0;
    hidl_handle buf = nullptr;
    status_t rc;

    if ( mOutputSurfacesMap.indexOfKey(msgType) < 0  )
    {
        MY_LOGE("error msgType(%d)", msgType);
        goto lbExit;
    }
    anw = mOutputSurfacesMap.valueFor(msgType);

    rc = native_window_dequeue_buffer_and_wait(anw.get(), &anb);
    if (rc == OK) {
        s = Status::OK;
        auto pair = getBufferId(anb);
        buf = (pair.first) ? anb->handle : nullptr;
        bufferId = pair.second;
        stride = anb->width;//anb->stride; // MW need to know width for filling jpeg_end
    }

lbExit:
    _hidl_cb(s, bufferId, buf, stride);

    FUNCTION_LOG_END;
    return hardware::Void();
}
/******************************************************************************
 *
 ******************************************************************************/
hardware::Return<Status>
CallbackClient::
enqueueBuffer(int32_t msgType, uint64_t bufferId)
{
    FUNCTION_LOG_START;
    std::unique_lock <std::mutex> l(mMutex);
    sp<ANativeWindow> anw;
    ANativeWindowBuffer* anb;
    Status s;
    status_t rc;
    if ( mOutputSurfacesMap.indexOfKey(msgType) < 0  )
    {
        MY_LOGE("error msgType(%d)", msgType);
        s = Status::INTERNAL_ERROR;
        goto lbExit;
    }
    anw = mOutputSurfacesMap.valueFor(msgType);

    if (mReversedBufMap.count(bufferId) == 0)
    {
        MY_LOGE("bufferId %" PRIu64 " not found", bufferId);
        s = Status::ILLEGAL_ARGUMENT;
        goto lbExit;
    }
    rc = anw->queueBuffer(anw.get(), mReversedBufMap.at(bufferId), -1);
    s = (rc == 0) ? Status::OK : Status::INTERNAL_ERROR;

lbExit:

    FUNCTION_LOG_END;

    return s;
}
/******************************************************************************
 *
 ******************************************************************************/
hardware::Return<Status>
CallbackClient::
cancelBuffer(int32_t msgType, uint64_t bufferId)
{
    FUNCTION_LOG_START;
    std::unique_lock <std::mutex> l(mMutex);
    sp<ANativeWindow> anw;
    ANativeWindowBuffer* anb;
    Status s;
    status_t rc;
    if ( mOutputSurfacesMap.indexOfKey(msgType) < 0  )
    {
        MY_LOGE("error msgType(%d)", msgType);
        s = Status::INTERNAL_ERROR;
        goto lbExit;
    }
    anw = mOutputSurfacesMap.valueFor(msgType);

    if (mReversedBufMap.count(bufferId) == 0) {
        MY_LOGE("bufferId %" PRIu64 " not found", bufferId);
        s = Status::ILLEGAL_ARGUMENT;
        goto lbExit;
    }
    rc = anw->cancelBuffer(anw.get(), mReversedBufMap.at(bufferId), -1);
    s = (rc == 0) ? Status::OK : Status::INTERNAL_ERROR;

lbExit:

    FUNCTION_LOG_END;
    return s;
}
/******************************************************************************
 *
 ******************************************************************************/
hardware::Return<Status>
CallbackClient::
setTimestamp(int32_t msgType, int64_t timestamp)
{
    FUNCTION_LOG_START;
    std::unique_lock <std::mutex> l(mMutex);
    sp<ANativeWindow> anw;
    ANativeWindowBuffer* anb;
    Status s;
    status_t rc;
    if ( mOutputSurfacesMap.indexOfKey(msgType) < 0  )
    {
        MY_LOGE("error msgType(%d)", msgType);
        s = Status::INTERNAL_ERROR;
        goto lbExit;
    }
    anw = mOutputSurfacesMap.valueFor(msgType);
    rc = native_window_set_buffers_timestamp(anw.get(), timestamp);
    s = (rc == 0) ? Status::OK : Status::INTERNAL_ERROR;

lbExit:
    FUNCTION_LOG_END;
    return s;
}
/******************************************************************************
 *
 ******************************************************************************/
hardware::Return<Status>
CallbackClient::
lock()
{
    FUNCTION_LOG_START;
    mOutPutSurfacesMutex.lock();
    FUNCTION_LOG_END;
    return Status::OK;
}
/******************************************************************************
 *
 ******************************************************************************/
hardware::Return<Status>
CallbackClient::
unlock()
{
    FUNCTION_LOG_START;
    mOutPutSurfacesMutex.unlock();
    FUNCTION_LOG_END;
    return Status::OK;
}
/******************************************************************************
 *
 ******************************************************************************/
std::pair<bool, uint64_t>
CallbackClient::
getBufferId(ANativeWindowBuffer* anb)
{
    std::lock_guard<std::mutex> lock(mBufferIdMapLock);

    buffer_handle_t& buf = anb->handle;
    auto it = mBufferIdMap.find(buf);
    if (it == mBufferIdMap.end()) {
        uint64_t bufId = mNextBufferId++;
        mBufferIdMap[buf] = bufId;
        mReversedBufMap[bufId] = anb;
        return std::make_pair(true, bufId);
    } else {
        return std::make_pair(true, it->second);
    }
}
/******************************************************************************
 *
 ******************************************************************************/
void CallbackClient::
cleanupCirculatingBuffers()
{
    std::lock_guard<std::mutex> lock(mBufferIdMapLock);
    mBufferIdMap.clear();
    mReversedBufMap.clear();
}
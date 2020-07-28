/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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

#define LOG_TAG "MtkCallbackClient_impl"
#include <mtkcam/utils/std/Log.h>
#include <android/hardware/camera/device/3.4/types.h>
#include <system/camera_metadata.h>

//#include <ui/GraphicBuffer.h>
#include <ui/Rect.h>
#include <ui/GraphicBufferMapper.h>
#include <gralloc_mtk_defs.h>
#include <system/graphics.h>

#include <camera_custom_stereo.h>
#include <mtkcam/feature/effectHalBase/ICallbackClientMgr.h>

//get camera3_jpeg_blob
#include <hardware/camera3.h>
//
#include "CallbackClient.h"

//
#include <cutils/properties.h> // ::property_get_int32
#include <memory> // std::unique_ptr
#include <functional> // std::function
#include <string> // std::string
#include <stdio.h> // ::fopen, ::fwrite, ::fclose

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

#define FUNCTION_LOG_START          MY_LOGD_IF(1<=1, " +");
#define FUNCTION_LOG_END            MY_LOGD_IF(1<=1, " -");

namespace vendor {
namespace mediatek {
namespace hardware {
namespace camera {
namespace callbackclient {
namespace V1_1 {
namespace implementation {

using namespace NSCam;
using vendor::mediatek::hardware::camera::callbackclient::V1_1::IMtkBufferCallback;

HandleImporter CallbackClientWrap::sHandleImporter;

// Methods from ::vendor::mediatek::hardware::camera::callbackclient::V1_0::IMtkCallbackClient follow.
Return<Status> MtkCallbackClient::existCallbackClient() {
    Status ret;
    FUNCTION_LOG_START
    ret = canSupportBGService() ? Status::OK : Status::INTERNAL_ERROR;
    FUNCTION_LOG_END
    return ret;
}

Return<Status> MtkCallbackClient::createCallbackClient(int64_t timestamp) {
    Status ret;
    FUNCTION_LOG_START
    ret = CallbackClientWrap::getInstance()->createCallbackClient(timestamp);
    FUNCTION_LOG_END
    return ret;
}

Return<Status> MtkCallbackClient::setOutputSurfaces(int64_t timestamp, const sp<vendor::mediatek::hardware::camera::callbackclient::V1_0::IMtkBufferCallback>& bufferCB, const hidl_vec<int32_t>& msgType) {
    Status ret;
    FUNCTION_LOG_START
    ret = CallbackClientWrap::getInstance()->setOutputSurfaces(timestamp, IMtkBufferCallback::castFrom(bufferCB), msgType.size());
    FUNCTION_LOG_END
    return ret;
}

Return<Status> MtkCallbackClient::destroyCallbackClient(int64_t timestamp) {
    Status ret;
    FUNCTION_LOG_START
    ret = CallbackClientWrap::getInstance()->destroyCallbackClient(timestamp);
    FUNCTION_LOG_END
    return ret;
}

// Methods from IMtkCallbackClient follow.
Return<Status> MtkCallbackClient::sendCommand(int64_t timestamp, MtkCBClientCmdType cmd, int32_t arg1, int32_t arg2) {
    Status ret;
    FUNCTION_LOG_START
    ret = CallbackClientWrap::getInstance()->sendCommand(timestamp, cmd, arg1, arg2);
    FUNCTION_LOG_END
    return ret;
}

// Methods from ::android::hidl::base::V1_0::IBase follow.

IMtkCallbackClient* HIDL_FETCH_IMtkCallbackClient(const char* /* name */) {
    return new MtkCallbackClient();
}

class ScopeLockSurfaceWorker {
public:
    ScopeLockSurfaceWorker(sp<IMtkBufferCallback> pBufferCB)
    {
        mpBufferCB = pBufferCB;
        mpBufferCB->lock();
    }
    ~ScopeLockSurfaceWorker()
    {
        mpBufferCB->unlock();
    };
private:
    sp<IMtkBufferCallback> mpBufferCB;
};

/* CallbackClientWrap */
CallbackClientWrap*
CallbackClientWrap::getInstance()
{
    static CallbackClientWrap inst;
    return &inst;
}

Return<Status> CallbackClientWrap::createCallbackClient(int64_t timestamp) {
    std::lock_guard <std::mutex> l(mMutex);
    if (mCbIdMap.size() == 0)
    {
        ICallbackClientMgr::getInstance()->registerBufferCB(bufferCallback);
        ICallbackClientMgr::getInstance()->registerBufferCB(postviewCallback);
    }
    mBufferCallbackMap.add(timestamp, nullptr);   // The initial is null
    mCbIdMap.add(timestamp, (uint32_t)MtkCBClientID::MTK_CALLBACK_CLIENT_ID_DUAL);
    mlTimestamp.push_front(timestamp);
    ICallbackClientMgr::getInstance()->createCallbackClient(timestamp);
    return ::android::hardware::camera::common::V1_0::Status {};
}

Return<Status> CallbackClientWrap::setOutputSurfaces(int64_t timestamp, const sp<IMtkBufferCallback>& bufferCB, int32_t size) {
    std::lock_guard <std::mutex> l(mMutex);
    mBufferCallbackMap.replaceValueFor(timestamp, bufferCB);
    return ::android::hardware::camera::common::V1_0::Status {};
}

Return<Status> CallbackClientWrap::destroyCallbackClient(int64_t timestamp) {
    std::lock_guard <std::mutex> l(mMutex);
    MBOOL bCB = MTRUE;
    if (!mlTimestamp.empty())
    {
        ssize_t idx = mCbIdMap.indexOfKey(timestamp);
        if (__builtin_expect( idx < 0, false ))
        {
            MY_LOGE("timestamp(%" PRId64 "), not found!!", timestamp); // key not found.
            return Status::ILLEGAL_ARGUMENT;
        }
        int32_t cbId = mCbIdMap.valueAt(idx);
        mCbIdMap.removeItemsAt(idx);
        mBufferCallbackMap.removeItem(timestamp);

        std::list<int64_t>::iterator iter = mlTimestamp.begin();
        for(;iter != mlTimestamp.end();++iter)
        {
            if (*iter == timestamp)
            {
                iter = mlTimestamp.erase(iter);
                break;
            }
        }

        // check cbId
        iter = mlTimestamp.begin();
        for(; iter != mlTimestamp.end(); iter++)
        {
            int32_t tmpid = mCbIdMap.valueFor(*iter);
            if (cbId == tmpid)
            {
                bCB = MFALSE;
                break;
            }
        }
        ICallbackClientMgr::getInstance()->removeCallbackClient(timestamp, bCB, cbId);
    }
    return ::android::hardware::camera::common::V1_0::Status {};
}

Return<Status> CallbackClientWrap::sendCommand(int64_t timestamp, MtkCBClientCmdType cmd, int32_t arg1, int32_t arg2){
    std::lock_guard <std::mutex> l(mMutex);
    switch(cmd)
    {
        case MtkCBClientCmdType::MTK_CALLBACK_CLIENT_CMD_SET_CALLBACK_ID:
        {
            MY_LOGD("timestamp(%" PRId64 "), Callback ID: %d", timestamp, arg1);
            mCbIdMap.replaceValueFor(timestamp, arg1);
            break;
        }
        case MtkCBClientCmdType::MTK_CALLBACK_CLIENT_CMD_DUMP:
        {
            std::list<int64_t>::iterator iter = mlTimestamp.begin();
            for(; iter != mlTimestamp.end(); iter++)
            {
              MY_LOGD("timestamp(%" PRId64 "), Callback ID: %d", timestamp, mCbIdMap.valueFor(*iter));
            }
            break;
        }
        default:
            break;
    }
    return ::android::hardware::camera::common::V1_0::Status {};
}

bool
CallbackClientWrap::
onBufferCallback( int64_t const   i8Timestamp,
                  uint32_t const  u4BitstreamSize,
                  uint8_t const*  puBitstreamBuf,
                  uint32_t const  u4CallbackIndex,
                  bool            fgIsFinalImage,
                  uint32_t const  msgType,
                  uint32_t const  u4BufOffset4Ts,
                  uint32_t const  u4CbId)
{
    FUNCTION_LOG_START
    std::lock_guard <std::mutex> l(mCbMutex);
    sp<IMtkBufferCallback> pBufferCallback = NULL;
    // get IMtkBufferCallback (i.e. which callbacklclient)
    {
        std::lock_guard <std::mutex> l(mMutex);
        if (mlTimestamp.empty())
        {
            MY_LOGE("CallbackClient list is empty!!!");
        }
        else
        {
            int32_t cbId = 0;
            std::list<int64_t>::iterator iter = mlTimestamp.begin();
            for(; iter != mlTimestamp.end(); iter++)
            {
                cbId = mCbIdMap.valueFor(*iter);
                if (cbId != u4CbId) continue;

                pBufferCallback = mBufferCallbackMap.valueFor(*iter);
                if (i8Timestamp < *iter) continue;

                if (pBufferCallback == NULL)
                {
                    MY_LOGE("CallbackClient is erased!!!");
                }
                break;
            }
        }
    }
    if (pBufferCallback == NULL)
    {
        MY_LOGE("Can't find CallbackClient to callback with CbId(%d)!!!", u4CbId);
        return false;
    }

    ScopeLockSurfaceWorker ___scope_worker(pBufferCallback);

    // deque buffer
    uint64_t bufferId;
    buffer_handle_t importedBuf = nullptr;
    uint32_t stride;
    {
        MY_LOGI("IMtkBufferCallback::dequeBuffer [+]");
        Return<void> ret = pBufferCallback->dequeueBuffer(msgType,
            [&](auto status, uint64_t bufId, const auto& buf, uint32_t strd) {
                if (status == Status::OK)
                {
                    importedBuf = buf.getNativeHandle();
                    stride      = strd;
                    bufferId    = bufId;
                    sHandleImporter.importBuffer(importedBuf);
                    if (importedBuf == nullptr)
                    {
                        MY_LOGE("callback buffer import failed!");
                        return;
                    }
                    else
                    {
                        MY_LOGD("dequeue_buffer with importedBuf=%p, bufferId(%llu), stride(%d)", importedBuf, bufferId, stride);
                    }
                }
            });
        MY_LOGI("IMtkBufferCallback::dequeBuffer [-]");
        if (!ret.isOk())
        {
            MY_LOGE("Transaction error in IMtkBufferCallback::dequeueBuffer: %s", ret.description().c_str());
            return false;
        }
        if (importedBuf == nullptr)
        {
            MY_LOGE("dequeueBuffer fail!!");
            return false;
        }
    }
    // get buffer address to write
    status_t err = 0;
    uint8_t* img = NULL;
    uint8_t* img_start = NULL;
    Rect const bounds(0, 0, stride, 1);  // Rect const bounds(stride, 1);
    err = GraphicBufferMapper::get().lock(importedBuf, GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_OFTEN, bounds, (void**)(&img));
    if (err)
    {
        MY_LOGE("GraphicBufferMapper.lock failed: status[%s(%d)]", ::strerror(-err), -err);
        // cancel buffer
        Return<Status> r = pBufferCallback->cancelBuffer(msgType, bufferId);
        if (!r.isOk()) {
            MY_LOGE("Transaction error in IMtkBufferCallback::cancelBuffer: %s", r.description().c_str());
        }
        return false;
    }

    img_start = img;
    img += u4BufOffset4Ts; // move start address.

    MY_LOGD("data start addr=%p, data addr=%p", img_start, img);

    /* check if we have to insert timestamp and bitstream size in leading u4BufOffset4Ts bytes */
    if (u4BufOffset4Ts > 0)
    {
        /* we're going to write 2 int64_t */
        if (__builtin_expect( u4BufOffset4Ts < sizeof(int64_t) * 2, false )) {
            MY_LOGF("offset size(%u) is smaller than sync data size(%zu)",
                    u4BufOffset4Ts, sizeof(int64_t));
            *(volatile uint32_t*)(0x0) = 0xBADFEED;
        }
        /* write timestamp in [offset+0, offset+8) */
        volatile int64_t* pTS = reinterpret_cast<volatile int64_t*>(img_start);
        *pTS = i8Timestamp;
        MY_LOGD("inserted timestamp(%" PRId64 ") in range %p to %p",
                i8Timestamp,
                img_start, img_start + u4BufOffset4Ts);

        /* write bitstream size in [offset+8, offset+16) */
        *(++pTS) = u4BitstreamSize;
        MY_LOGD("insert bitstreamSize(%d) in range %p to %p", u4BitstreamSize, (pTS - 1), (pTS));
    }

    // write camera3_jpeg_blob to buffer end for AP to read
    {
        volatile camera3_jpeg_blob* pTransport = reinterpret_cast<volatile camera3_jpeg_blob*>(img_start + stride - sizeof(camera3_jpeg_blob));
        pTransport->jpeg_blob_id      = CAMERA3_JPEG_BLOB_ID;
        pTransport->jpeg_size         = u4BitstreamSize + sizeof(int64_t) * 2;
        if ((u4BitstreamSize + sizeof(camera3_jpeg_blob)) > stride)
        {
            MY_LOGE("wrong size, msgType(%d), u4BitstreamSize(%d), stride(%d)", msgType, u4BitstreamSize, stride);
            return false;
        }
    }

    //copy image buffer
    ::memcpy(img, puBitstreamBuf, u4BitstreamSize);
    GraphicBufferMapper::get().unlock(importedBuf);
    // free importedBuf
    sHandleImporter.freeBuffer(importedBuf);
    // set timestamp
    {
        Return<Status> r = pBufferCallback->setTimestamp(msgType, bufferId);
        if (!r.isOk()) {
            MY_LOGE("Transaction error in IMtkBufferCallback::setTimestamp: %s", r.description().c_str());
            return false;
        }
    }
    // enque buffer
    {
        Return<Status> r = pBufferCallback->enqueueBuffer(msgType, bufferId);
        if (!r.isOk()) {
            MY_LOGE("Transaction error in IMtkBufferCallback::enqueueBuffer: %s", r.description().c_str());
            return false;
        }
    }
    FUNCTION_LOG_END
    return true;
}

bool
CallbackClientWrap::
onPostviewCallback(int64_t const  i8Timestamp,
                   uint32_t const planeCount,
                   uint32_t const width,
                   uint32_t const height,
                   uint32_t const fmt,
                   uint8_t* const u4Bitstream[],
                   uint32_t const widthInBytes[],
                   uint32_t const strideInBytes[],
                   uint32_t const scanlines[],
                   uint32_t const u4BufOffset4Ts,
                   uint32_t const u4CbId)
{
    const uint32_t msgType = CAMERA_MSG_POSTVIEW_FRAME;

    FUNCTION_LOG_START
    std::lock_guard <std::mutex> l(mCbMutex);
    MY_LOGD("enter onPostviewCallback");
    sp<IMtkBufferCallback> pBufferCallback = NULL;
    // get IMtkBufferCallback (i.e. which callbacklclient)
    {
        std::lock_guard <std::mutex> l(mMutex);
        if (mlTimestamp.empty())
        {
            MY_LOGE("CallbackClient list is empty!!!");
        }
        else
        {
            int32_t cbId = 0;
            std::list<int64_t>::iterator iter = mlTimestamp.begin();
            for(; iter != mlTimestamp.end(); iter++)
            {
                cbId = mCbIdMap.valueFor(*iter);
                if (cbId != u4CbId) continue;

                pBufferCallback = mBufferCallbackMap.valueFor(*iter);
                if (i8Timestamp < *iter) continue;

                if (pBufferCallback == NULL)
                {
                    MY_LOGE("CallbackClient is erased!!!");
                }
                break;
            }
        }
    }
    if (pBufferCallback == NULL)
    {
        MY_LOGE("Can't find CallbackClient to callback with CbId(%d)!!!", u4CbId);
        return false;
    }

    ScopeLockSurfaceWorker ___scope_worker(pBufferCallback);

    // deque buffer
    uint64_t bufferId;
    buffer_handle_t importedBuf = nullptr;
    uint32_t stride;
    {
        MY_LOGI("IMtkBufferCallback::dequeBuffer [+]");
        Return<void> ret = pBufferCallback->dequeueBuffer(msgType,
            [&](auto status, uint64_t bufId, const auto& buf, uint32_t strd) {
                if (status == Status::OK)
                {
                    importedBuf = buf.getNativeHandle();
                    stride      = strd;
                    bufferId    = bufId;
                    sHandleImporter.importBuffer(importedBuf);
                    if (importedBuf == nullptr)
                    {
                        MY_LOGE("callback buffer import failed!");
                        return;
                    }
                    else
                    {
                        MY_LOGD("dequeue_buffer with importedBuf=%p, bufferId(%llu), stride(%d)", importedBuf, bufferId, stride);
                    }
                }
            });
        MY_LOGI("IMtkBufferCallback::dequeBuffer [-]");
        if (!ret.isOk())
        {
            MY_LOGE("Transaction error (msgType=%u) in IMtkBufferCallback::dequeueBuffer: %s",
                    msgType, ret.description().c_str());
            return false;
        }
        if (importedBuf == nullptr)
        {
            MY_LOGE("dequeueBuffer fail!!");
            return false;
        }
    }
    // get buffer address to write
    status_t err = 0;
    uint8_t* img = NULL;
    uint8_t* img_start = NULL;
    Rect const bounds(0, 0, stride, 1);  // Rect const bounds(stride, 1);
    err = GraphicBufferMapper::get().lock(importedBuf, GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_OFTEN, bounds, (void**)(&img));
    if (err)
    {
        MY_LOGE("GraphicBufferMapper.lock failed: status[%s(%d)]", ::strerror(-err), -err);
        // cancel buffer
        Return<Status> r = pBufferCallback->cancelBuffer(msgType, bufferId);
        if (!r.isOk()) {
            MY_LOGE("Transaction error in IMtkBufferCallback::cancelBuffer: %s", r.description().c_str());
        }
        return false;
    }

    img_start = img;
    img += u4BufOffset4Ts; // move start address.

    MY_LOGD("data start addr=%p, data addr=%p", img_start, img);

    /* check if we have to insert timestamp and image size in leading u4BufOffset4Ts bytes */
    /* Bytes[0,8) : int64    , timestamp */
    /* Bytes[8,12): uint32   , resolution: Bits[0:15] Width, Bits[16:31] Height */
    if (u4BufOffset4Ts > 0)
    {
        if (__builtin_expect( u4BufOffset4Ts < sizeof(int64_t) * 2, false )) {
            MY_LOGF("offset size(%u) is smaller than sync data size(%zu)",
                    u4BufOffset4Ts, sizeof(int64_t));
            *(volatile uint32_t*)(0x0) = 0xBADFEED;
        }

        /* timestamp, costs 8 bytes */
        volatile int64_t* pTS = reinterpret_cast<volatile int64_t*>(img_start);
        *pTS = i8Timestamp;
        MY_LOGD("inserted timestamp(%" PRId64 ") in range %p to %p",
                i8Timestamp,
                img_start, img_start + u4BufOffset4Ts);

        /* resolution, costs 4 bytes */
        volatile uint16_t* pu16Data = reinterpret_cast<volatile uint16_t*>(img_start + 8);
        *(pu16Data++) = width;
        *(pu16Data)   = height;

        /* formatted, costs 4 bytes */
        volatile uint32_t* pFormatted = reinterpret_cast<volatile uint32_t*>(img_start + 8 + 4);
        *pFormatted = fmt;

    }

    size_t bufferSize = 0;

    // dump postview buffer if property set
    std::unique_ptr<FILE, std::function<void(FILE*)> > pfd;
    std::string dump_file_name;
    if (__builtin_expect(::property_get_int32("debug.bgservice.dump.postview", 0), false)) {
        // constructs file name:
        // --> /sdcard/camera_dump/postview_{TIMESTAMP}_{WIDTH}x{HEIGHT}.nv21
        using std::to_string;
        dump_file_name  = "/sdcard/camera_dump/";
        dump_file_name += "postview_" + to_string(i8Timestamp) + "_";
        dump_file_name += to_string(width) + "x" + to_string(height);
        dump_file_name += ".nv21";

        pfd = decltype(pfd)(
                // open file descriptor
                ::fopen(dump_file_name.c_str(), "w"),
                // destructor of variable pfd
                [](FILE* fp) { ::fclose(fp); }
                );
    }

    // copy image buffer w/o stride
    for (size_t i = 0; i < planeCount; ++i)
    {
        uint8_t* src_img = u4Bitstream[i];

        // for all lines
        for(size_t h = 0; h < scanlines[i]; ++h)
        {
            // check if out-of-memory, where stride is the destination buffer size
            if (__builtin_expect( (bufferSize + widthInBytes[i]) > stride, false ))
            {
                MY_LOGF("image buffer size(%zu) is greater than surface one(%u), assert",
                        (bufferSize + widthInBytes[i]), stride);
                *(volatile uint32_t*)(0x0) = 0xDEADC0DE;
            }
            // copy line by line
            ::memcpy(img, src_img, widthInBytes[i]);

            // write file if pfd exists
            if (__builtin_expect( pfd.get() != nullptr, false )) {
                ::fwrite(src_img, 1L, widthInBytes[i], pfd.get());
            }

            // move addresses
            img     += widthInBytes[i];
            src_img += strideInBytes[i];
        }
    }
    GraphicBufferMapper::get().unlock(importedBuf);
    // free importedBuf
    sHandleImporter.freeBuffer(importedBuf);
    // set timestamp
    {
        Return<Status> r = pBufferCallback->setTimestamp(msgType, bufferId);
        if (!r.isOk()) {
            MY_LOGE("Transaction error in IMtkBufferCallback::setTimestamp: %s", r.description().c_str());
            return false;
        }
    }
    // enque buffer
    {
        Return<Status> r = pBufferCallback->enqueueBuffer(msgType, bufferId);
        if (!r.isOk()) {
            MY_LOGE("Transaction error in IMtkBufferCallback::enqueueBuffer: %s", r.description().c_str());
            return false;
        }
    }
    FUNCTION_LOG_END
    return true;
}

bool
CallbackClientWrap::
bufferCallback( int64_t const   i8Timestamp,
                uint32_t const  u4BitstreamSize,
                uint8_t const*  puBitstreamBuf,
                uint32_t const  u4CallbackIndex,
                bool            fgIsFinalImage,
                uint32_t const  msgType,
                uint32_t const  u4BufOffset4Ts,
                uint32_t const  u4CbId)
{
    return CallbackClientWrap::getInstance()->onBufferCallback(i8Timestamp,
                                                               u4BitstreamSize,
                                                               puBitstreamBuf,
                                                               u4CallbackIndex,
                                                               fgIsFinalImage,
                                                               msgType,
                                                               u4BufOffset4Ts,
                                                               u4CbId);
}

bool
CallbackClientWrap::
postviewCallback( int64_t const  i8Timestamp,
                  uint32_t const planeCount,
                  uint32_t const width,
                  uint32_t const height,
                  uint32_t const fmt,
                  uint8_t* const u4Bitstream[],
                  uint32_t const widthInBytes[],
                  uint32_t const strideInBytes[],
                  uint32_t const scanlines[],
                  uint32_t const u4BufferOffsetForTimeStamp,
                  uint32_t const u4CbId)
{
    return CallbackClientWrap::getInstance()->onPostviewCallback(
            i8Timestamp,
            planeCount,
            width,
            height,
            fmt,
            u4Bitstream,
            widthInBytes,
            strideInBytes,
            scanlines,
            u4BufferOffsetForTimeStamp,
            u4CbId
            );
}

}  // namespace implementation
}  // namespace V1_1
}  // namespace callbackclient
}  // namespace camera
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor

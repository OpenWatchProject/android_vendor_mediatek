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

#include "CameraDevice1Base.h"
#include "MyUtils.h"
//
#include <list>
//
#include <media/hardware/HardwareAPI.h> // For VideoNativeHandleMetadata
#include <hidlmemory/mapping.h>
//
using namespace std;
using namespace android;
using namespace NSCam;

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("%d[CameraDevice1Base::%s] " fmt, getInstanceId(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("%d[CameraDevice1Base::%s] " fmt, getInstanceId(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("%d[CameraDevice1Base::%s] " fmt, getInstanceId(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("%d[CameraDevice1Base::%s] " fmt, getInstanceId(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("%d[CameraDevice1Base::%s] " fmt, getInstanceId(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("%d[CameraDevice1Base::%s] " fmt, getInstanceId(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("%d[CameraDevice1Base::%s] " fmt, getInstanceId(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)
//

HandleImporter CameraDevice1Base::sHandleImporter;
extern list<wp<CameraDevice1Base>>  glwpCameraDevice1Base;
extern Mutex                        gCameraDevice1Baselock;

/******************************************************************************
 * Check if device is connected/disconnected.
 ******************************************************************************/
static std::unordered_map<std::string, bool> mDevConnectedMap;
static android::Mutex mDevConLock;   //used for shared memory write/release

void
CameraDevice1Base::
sSetDevConnect(const std::string mInstanceName, bool isConnect)
{
    ::android::Mutex::Autolock _l(mDevConLock);
    mDevConnectedMap[mInstanceName] = isConnect;
}

bool
CameraDevice1Base::
sGetDevConnect(const std::string mInstanceName)
{
    ::android::Mutex::Autolock _l(mDevConLock);
    return mDevConnectedMap[mInstanceName];
}


/******************************************************************************
 * Camera Heap Memory implementation.
 ******************************************************************************/
CameraDevice1Base::CameraHeapMemory::CameraHeapMemory(
    int fd, size_t buf_size, uint_t num_buffers) :
        mBufSize(buf_size),
        mNumBufs(num_buffers) {
    mHidlHandle = native_handle_create(1,0);
    mHidlHandle->data[0] = fcntl(fd, F_DUPFD_CLOEXEC, 0);
    const size_t pagesize = getpagesize();
    size_t size = ((buf_size * num_buffers + pagesize-1) & ~(pagesize-1));
    mHidlHeap = hidl_memory("ashmem", mHidlHandle, size);
    commonInitialization();
}

CameraDevice1Base::CameraHeapMemory::CameraHeapMemory(
    sp<IAllocator> ashmemAllocator,
    size_t buf_size, uint_t num_buffers) :
        mBufSize(buf_size),
        mNumBufs(num_buffers) {
    const size_t pagesize = getpagesize();
    size_t size = ((buf_size * num_buffers + pagesize-1) & ~(pagesize-1));
    ashmemAllocator->allocate(size,
        [&](bool success, const hidl_memory& mem) {
            if (!success) {
                ALOGE("%s: allocating ashmem of %zu bytes failed!",
                        __FUNCTION__, buf_size * num_buffers);
                return;
            }
            mHidlHandle = native_handle_clone(mem.handle());
            mHidlHeap = hidl_memory("ashmem", mHidlHandle, size);
        });

    commonInitialization();
}

void CameraDevice1Base::CameraHeapMemory::commonInitialization() {
    mHidlHeapMemory = mapMemory(mHidlHeap);
    if (mHidlHeapMemory == nullptr) {
        ALOGE("%s: memory map failed!", __FUNCTION__);
        native_handle_close(mHidlHandle); // close FD for the shared memory
        native_handle_delete(mHidlHandle);
        mHidlHeap = hidl_memory();
        mHidlHandle = nullptr;
        return;
    }
    mHidlHeapMemData = mHidlHeapMemory->getPointer();
    handle.data = mHidlHeapMemData;
    handle.size = mBufSize * mNumBufs;
    handle.handle = this;
    handle.release = sPutMemory;
}

CameraDevice1Base::CameraHeapMemory::~CameraHeapMemory() {
    if (mHidlHeapMemory != nullptr) {
        mHidlHeapMemData = nullptr;
        mHidlHeapMemory.clear(); // The destructor will trigger munmap
    }

    if (mHidlHandle) {
        native_handle_close(mHidlHandle); // close FD for the shared memory
        native_handle_delete(mHidlHandle);
    }
}

/******************************************************************************
 * Camera Preview Window implementation.
 ******************************************************************************/
void CameraDevice1Base::CameraPreviewWindow::cleanUpCirculatingBuffers() {
    Mutex::Autolock _l(mLock);
    for (auto pair : mCirculatingBuffers) {
        try{
            sHandleImporter.freeBuffer(pair.second);
        } catch (const std::ios_base::failure & e){
            ALOGE("%s: freeBuffer from sHandleImporter failed", __FUNCTION__);
        }
    }
    mCirculatingBuffers.clear();
    mBufferIdMap.clear();
}

/******************************************************************************
 * Operation.
 ******************************************************************************/
void CameraDevice1Base::initHalPreviewWindow()
{
    mHalPreviewWindow.cancel_buffer = sCancelBuffer;
    mHalPreviewWindow.lock_buffer = sLockBuffer;
    mHalPreviewWindow.dequeue_buffer = sDequeueBuffer;
    mHalPreviewWindow.enqueue_buffer = sEnqueueBuffer;
    mHalPreviewWindow.set_buffer_count = sSetBufferCount;
    mHalPreviewWindow.set_buffers_geometry = sSetBuffersGeometry;
    mHalPreviewWindow.set_crop = sSetCrop;
    mHalPreviewWindow.set_timestamp = sSetTimestamp;
    mHalPreviewWindow.set_usage = sSetUsage;
    mHalPreviewWindow.set_swap_interval = sSetSwapInterval;

    mHalPreviewWindow.get_min_undequeued_buffer_count =
            sGetMinUndequeuedBufferCount;
}

void
CameraDevice1Base::
releaseRecordingFrame_oldStyle(const void *opaque)
{
    if  ( mpCamClient != 0 )
    {
        mpCamClient->releaseRecordingFrame(opaque);
    }
    // forward to registered clients
    Vector<sp<ICamClient> >::const_iterator it;
    for (it = vmpCamClient.begin(); it != vmpCamClient.end(); ++it)
    {
        (*it)->releaseRecordingFrame(opaque);
    }
}

void
CameraDevice1Base::
onCallbackTimestamp(nsecs_t timestamp, int32_t msg_type,
                    MemoryId memId , unsigned index, native_handle_t* handle)
{
    uint32_t batchSize = 0;
    {
        Mutex::Autolock _l(mBatchLock);
        batchSize = mBatchSize;
    }

    if (batchSize == 0) { // non-batch mode
        {
            hardware::Return<void> ret = mDeviceCallback->handleCallbackTimestamp(
                                             (DataCallbackMsg) msg_type, handle, memId, index, timestamp);
            if (!ret.isOk()) {
                ALOGE("Transaction error in ICameraDeviceCallback::handleCallbackTimestamp: %s", ret.description().c_str());
                return;
            }
        }
    } else { // batch mode
        Mutex::Autolock _l(mBatchLock);
        size_t inflightSize = mInflightBatch.size();
        if (inflightSize == 0) {
            mBatchMsgType = msg_type;
        } else if (mBatchMsgType != msg_type) {
            ALOGE("%s: msg_type change (from %d to %d) is not supported!",
                    __FUNCTION__, mBatchMsgType, msg_type);
            return;
        }
        mInflightBatch.push_back({handle, memId, index, timestamp});

        // Send batched frames to camera framework
        if (mInflightBatch.size() >= batchSize) {
            {
                hardware::Return<void> ret = mDeviceCallback->handleCallbackTimestampBatch(
                                                 (DataCallbackMsg) mBatchMsgType, mInflightBatch);
                if (!ret.isOk()) {
                    ALOGE("Transaction error in ICameraDeviceCallback::handleCallbackTimestampBatch: %s", ret.description().c_str());
                }
            }
            mInflightBatch.clear();
        }
    }
}

/******************************************************************************
 * Preview window callback forwarding methods
 ******************************************************************************/
int CameraDevice1Base::sDequeueBuffer(struct preview_stream_ops* w,
                                   buffer_handle_t** buffer, int *stride) {
    CameraPreviewWindow* object = static_cast<CameraPreviewWindow*>(w);
    if (object->mPreviewCallback == nullptr) {
        ALOGE("%s: camera HAL calling preview ops while there is no preview window!", __FUNCTION__);
        return INVALID_OPERATION;
    }

    if (buffer == nullptr || stride == nullptr) {
        ALOGE("%s: buffer (%p) and stride (%p) must not be null!", __FUNCTION__, buffer, stride);
        return BAD_VALUE;
    }

    Status s;
    {
        hardware::Return<void> ret = object->mPreviewCallback->dequeueBuffer(
            [&](auto status, uint64_t bufferId, const auto& buf, uint32_t strd) {
                s = status;
                if (s == Status::OK) {
                    Mutex::Autolock _l(object->mLock);
                    if (object->mCirculatingBuffers.count(bufferId) == 0) {
                        buffer_handle_t importedBuf = buf.getNativeHandle();
                        sHandleImporter.importBuffer(importedBuf);
                        if (importedBuf == nullptr) {
                            ALOGE("%s: preview buffer import failed!", __FUNCTION__);
                            s = Status::INTERNAL_ERROR;
                            return;
                        } else {
                            object->mCirculatingBuffers[bufferId] = importedBuf;
                            object->mBufferIdMap[&(object->mCirculatingBuffers[bufferId])] = bufferId;
                        }
                    }
                    *buffer = &(object->mCirculatingBuffers[bufferId]);
                    *stride = strd;
                }
            });
        if (!ret.isOk()) {
            ALOGE("Transaction error in ICameraDevicePreviewCallback::dequeueBuffer: %s", ret.description().c_str());
            s = Status::INTERNAL_ERROR;
        }
    }
    //[TODO] Check the return data type compatibility
    return getFromHidlCameraStatus(s);
}

int CameraDevice1Base::sLockBuffer(struct preview_stream_ops*, buffer_handle_t*) {
    //lock buffer should be a no-op
    return 0;
}

int CameraDevice1Base::sEnqueueBuffer(struct preview_stream_ops* w, buffer_handle_t* buffer) {
    CameraPreviewWindow* object = static_cast<CameraPreviewWindow*>(w);
    if (object->mPreviewCallback == nullptr) {
        ALOGE("%s: camera HAL calling preview ops while there is no preview window!", __FUNCTION__);
        return INVALID_OPERATION;
    }
    uint64_t bufferId = object->mBufferIdMap.at(buffer);
    hardware::Return<Status> ret = object->mPreviewCallback->enqueueBuffer(bufferId);
    if (!ret.isOk()) {
        ALOGE("Transaction error in ICameraDevicePreviewCallback::enqueueBuffer: %s", ret.description().c_str());
        return DEAD_OBJECT;
    }
    else{
        Status s = (Status)ret;
        return getFromHidlCameraStatus(s);
    }
}

int CameraDevice1Base::sCancelBuffer(struct preview_stream_ops* w, buffer_handle_t* buffer) {
    CameraPreviewWindow* object = static_cast<CameraPreviewWindow*>(w);
    if (object->mPreviewCallback == nullptr) {
        ALOGE("%s: camera HAL calling preview ops while there is no preview window!", __FUNCTION__);
        return INVALID_OPERATION;
    }
    uint64_t bufferId = object->mBufferIdMap.at(buffer);
    hardware::Return<Status> ret = object->mPreviewCallback->cancelBuffer(bufferId);
    if (!ret.isOk()) {
        ALOGE("Transaction error in ICameraDevicePreviewCallback::cancelBuffer: %s", ret.description().c_str());
        return DEAD_OBJECT;
    }
    else{
        Status s = (Status)ret;
        return getFromHidlCameraStatus(s);
    }
}

int CameraDevice1Base::sSetBufferCount(struct preview_stream_ops* w, int count) {
    CameraPreviewWindow* object = static_cast<CameraPreviewWindow*>(w);
    if (object->mPreviewCallback == nullptr) {
        ALOGE("%s: camera HAL calling preview ops while there is no preview window!", __FUNCTION__);
        return INVALID_OPERATION;
    }

    object->cleanUpCirculatingBuffers();
    hardware::Return<Status> ret = object->mPreviewCallback->setBufferCount(count);
    if (!ret.isOk()) {
        ALOGE("Transaction error in ICameraDevicePreviewCallback::setBufferCount: %s", ret.description().c_str());
        return DEAD_OBJECT;
    }
    else{
        Status s = (Status)ret;
        return getFromHidlCameraStatus(s);
    }
}

int CameraDevice1Base::sSetBuffersGeometry(struct preview_stream_ops* w,
                                         int width, int height, int format) {
    CameraPreviewWindow* object = static_cast<CameraPreviewWindow*>(w);
    if (object->mPreviewCallback == nullptr) {
        ALOGE("%s: camera HAL calling preview ops while there is no preview window!", __FUNCTION__);
        return INVALID_OPERATION;
    }

    object->cleanUpCirculatingBuffers();
    hardware::Return<Status> ret = object->mPreviewCallback->setBuffersGeometry(width, height, (android::hardware::graphics::common::V1_0::PixelFormat) format);
    if (!ret.isOk()) {
        ALOGE("Transaction error in ICameraDevicePreviewCallback::setBuffersGeometry: %s", ret.description().c_str());
        return DEAD_OBJECT;
    }
    else{
        Status s = (Status)ret;
        return getFromHidlCameraStatus(s);
    }
}

int CameraDevice1Base::sSetCrop(struct preview_stream_ops *w,
                             int left, int top, int right, int bottom) {
    CameraPreviewWindow* object = static_cast<CameraPreviewWindow*>(w);
    if (object->mPreviewCallback == nullptr) {
        ALOGE("%s: camera HAL calling preview ops while there is no preview window!", __FUNCTION__);
        return INVALID_OPERATION;
    }
    hardware::Return<Status> ret = object->mPreviewCallback->setCrop(left, top, right, bottom);
    if (!ret.isOk()) {
        ALOGE("Transaction error in ICameraDevicePreviewCallback::setCrop: %s", ret.description().c_str());
        return DEAD_OBJECT;
    }
    else{
        Status s = (Status)ret;
        return getFromHidlCameraStatus(s);
    }
}

int CameraDevice1Base::sSetTimestamp(struct preview_stream_ops *w, int64_t timestamp) {
    CameraPreviewWindow* object = static_cast<CameraPreviewWindow*>(w);
    if (object->mPreviewCallback == nullptr) {
        ALOGE("%s: camera HAL calling preview ops while there is no preview window!", __FUNCTION__);
        return INVALID_OPERATION;
    }
    hardware::Return<Status> ret = object->mPreviewCallback->setTimestamp(timestamp);
    if (!ret.isOk()) {
        ALOGE("Transaction error in ICameraDevicePreviewCallback::setTimestamp: %s", ret.description().c_str());
        return DEAD_OBJECT;
    }
    else{
        Status s = (Status)ret;
        return getFromHidlCameraStatus(s);
    }
}

int CameraDevice1Base::sSetUsage(struct preview_stream_ops* w, int usage) {
    CameraPreviewWindow* object = static_cast<CameraPreviewWindow*>(w);
    if (object->mPreviewCallback == nullptr) {
        ALOGE("%s: camera HAL calling preview ops while there is no preview window!", __FUNCTION__);
        return INVALID_OPERATION;
    }

    object->cleanUpCirculatingBuffers();

    hardware::Return<Status> ret = object->mPreviewCallback->setUsage((BufferUsage)usage);
    if (!ret.isOk()) {
        ALOGE("Transaction error in ICameraDevicePreviewCallback::setUsage: %s", ret.description().c_str());
        return DEAD_OBJECT;
    }
    else{
        Status s = (Status)ret;
        return getFromHidlCameraStatus(s);
    }
}

int CameraDevice1Base::sSetSwapInterval(struct preview_stream_ops *w, int interval) {
    CameraPreviewWindow* object = static_cast<CameraPreviewWindow*>(w);
    if (object->mPreviewCallback == nullptr) {
        ALOGE("%s: camera HAL calling preview ops while there is no preview window!", __FUNCTION__);
        return INVALID_OPERATION;
    }
    hardware::Return<Status> ret = object->mPreviewCallback->setSwapInterval(interval);
    if (!ret.isOk()) {
        ALOGE("Transaction error in ICameraDevicePreviewCallback::setSwapInterval: %s", ret.description().c_str());
        return DEAD_OBJECT;
    }
    else{
        Status s = (Status)ret;
        return getFromHidlCameraStatus(s);
    }
}

int CameraDevice1Base::sGetMinUndequeuedBufferCount(
                  const struct preview_stream_ops *w,
                  int *count) {
    const CameraPreviewWindow* object =  static_cast<const CameraPreviewWindow*>(w);
    if (object->mPreviewCallback == nullptr) {
        ALOGE("%s: camera HAL calling preview ops while there is no preview window!", __FUNCTION__);
        return INVALID_OPERATION;
    }
    if (count == nullptr) {
        ALOGE("%s: count is null!", __FUNCTION__);
        return BAD_VALUE;
    }

    Status s;
    {
        hardware::Return<void> ret = object->mPreviewCallback->getMinUndequeuedBufferCount(
            [&](auto status, uint32_t cnt) {
                s = status;
                if (s == Status::OK) {
                    *count = cnt;
                }
            });
        if (!ret.isOk()) {
            ALOGE("Transaction error in ICameraDevicePreviewCallback::getMinUndequeuedBufferCount: %s", ret.description().c_str());
        }
    }
    return getFromHidlCameraStatus(s);
}

/******************************************************************************
 * Device Callback forwarding methods
 ******************************************************************************/
void CameraDevice1Base::sNotifyCb(int32_t msg_type, int32_t ext1, int32_t ext2, void *user) {
    ALOGV("%s", __FUNCTION__);
    int cameraId = (int)(uintptr_t) user;
    sp<CameraDevice1Base> object = nullptr;
    {
        Mutex::Autolock lock(gCameraDevice1Baselock);
        for(list<wp<CameraDevice1Base>>::iterator it = glwpCameraDevice1Base.begin(); it != glwpCameraDevice1Base.end(); it++)
        {
            if( (*it).promote() != 0 &&
                (*it).promote()->getInstanceId() == cameraId)
            {
                object = (*it).promote();
                break;
            }
        }
        if(object == nullptr)
        {
            ALOGW("%s: cameraId %d object is nullptr", __FUNCTION__, cameraId);
            return;
        }
    }
    if (object->mDeviceCallback != nullptr) {
        if ( sGetDevConnect(object->mInstanceName) ){   //check if device is disconneted
            hardware::Return<void> ret = object->mDeviceCallback->notifyCallback((NotifyCallbackMsg) msg_type, ext1, ext2);
            if (!ret.isOk()) {
                ALOGE("Transaction error in ICameraDeviceCallback::notifyCallback: %s", ret.description().c_str());
                return;
            }
        }
        else{
            ALOGW("ICameraDeviceCallback::notifyCallback is called after device has been closed");
        }
    }
}

void CameraDevice1Base::sDataCb(int32_t msg_type, const camera_memory_t *data, unsigned int index,
        camera_frame_metadata_t *metadata, void *user) {
    ALOGV("%s", __FUNCTION__);
    int cameraId = (int)(uintptr_t) user;
    sp<CameraDevice1Base> object = nullptr;
    {
        Mutex::Autolock lock(gCameraDevice1Baselock);
        for(list<wp<CameraDevice1Base>>::iterator it = glwpCameraDevice1Base.begin(); it != glwpCameraDevice1Base.end(); it++)
        {
            if( (*it).promote() != 0 &&
                (*it).promote()->getInstanceId() == cameraId)
            {
                object = (*it).promote();
                break;
            }
        }
        if(object == nullptr)
        {
            ALOGW("%s: cameraId %d object is nullptr", __FUNCTION__, cameraId);
            return;
        }
    }
    sp<CameraHeapMemory> mem(static_cast<CameraHeapMemory*>(data->handle));
    if (index >= mem->mNumBufs) {
        ALOGE("%s: invalid buffer index %d, max allowed is %d", __FUNCTION__,
             index, mem->mNumBufs);
        return;
    }
    if (object->mDeviceCallback != nullptr) {
        CameraFrameMetadata hidlMetadata;
        if (metadata) {
            hidlMetadata.faces.resize(metadata->number_of_faces);
            for (size_t i = 0; i < hidlMetadata.faces.size(); i++) {
                hidlMetadata.faces[i].score = metadata->faces[i].score;
                hidlMetadata.faces[i].id = metadata->faces[i].id;
                for (int k = 0; k < 4; k++) {
                    hidlMetadata.faces[i].rect[k] = metadata->faces[i].rect[k];
                }
                for (int k = 0; k < 2; k++) {
                    hidlMetadata.faces[i].leftEye[k] = metadata->faces[i].left_eye[k];
                }
                for (int k = 0; k < 2; k++) {
                    hidlMetadata.faces[i].rightEye[k] = metadata->faces[i].right_eye[k];
                }
                for (int k = 0; k < 2; k++) {
                    hidlMetadata.faces[i].mouth[k] = metadata->faces[i].mouth[k];
                }
            }
        }

        if ( sGetDevConnect(object->mInstanceName) ){   //check if device is disconneted
            hardware::Return<void> ret = object->mDeviceCallback->dataCallback(
                    (DataCallbackMsg) msg_type, mem->handle.mId, index, hidlMetadata);
            if (!ret.isOk()) {
                ALOGE("Transaction error in ICameraDeviceCallback::dataCallback: %s", ret.description().c_str());
            }
        }
        else{
            ALOGW("ICameraDeviceCallback::dataCallback is called after device has been closed");
        }
    }
}

void CameraDevice1Base::sDataCbTimestamp(nsecs_t timestamp, int32_t msg_type,
        const camera_memory_t *data, unsigned index, void *user) {
    ALOGV("%s", __FUNCTION__);
    int cameraId = (int)(uintptr_t) user;
    sp<CameraDevice1Base> object = nullptr;
    {
        Mutex::Autolock lock(gCameraDevice1Baselock);
        for(list<wp<CameraDevice1Base>>::iterator it = glwpCameraDevice1Base.begin(); it != glwpCameraDevice1Base.end(); it++)
        {
            if( (*it).promote() != 0 &&
                (*it).promote()->getInstanceId() == cameraId)
            {
                object = (*it).promote();
                break;
            }
        }
        if(object == nullptr)
        {
            ALOGW("%s: cameraId %d object is nullptr", __FUNCTION__, cameraId);
            return;
        }
    }
    // Start refcounting the heap object from here on.  When the clients
    // drop all references, it will be destroyed (as well as the enclosed
    // MemoryHeapBase.
    sp<CameraHeapMemory> mem(static_cast<CameraHeapMemory*>(data->handle));
    if (index >= mem->mNumBufs) {
        ALOGE("%s: invalid buffer index %d, max allowed is %d", __FUNCTION__,
             index, mem->mNumBufs);
        return;
    }

    native_handle_t* handle = nullptr;
    if (object->mMetadataMode) {
        if (mem->mBufSize == sizeof(VideoNativeHandleMetadata)) {
            VideoNativeHandleMetadata* md = (VideoNativeHandleMetadata*)
                    ((uint8_t*) mem->mHidlHeapMemData + index * mem->mBufSize);
            if (md->eType == kMetadataBufferTypeNativeHandleSource) {
                handle = md->pHandle;
            }
        }
    }

    if (object->mDeviceCallback != nullptr) {
        if ( sGetDevConnect(object->mInstanceName) ){   //check if device is disconneted
            if (handle == nullptr) {
                hardware::Return<void> ret = object->mDeviceCallback->dataCallbackTimestamp(
                        (DataCallbackMsg) msg_type, mem->handle.mId, index, timestamp);
                if (!ret.isOk()) {
                    ALOGE("Transaction error in ICameraDeviceCallback::dataCallbackTimestamp: %s", ret.description().c_str());
                }
            } else {
                object->onCallbackTimestamp(timestamp, msg_type, mem->handle.mId, index, handle);
            }
        }
        else{
            ALOGW("ICameraDeviceCallback::dataCallbackTimestamp is called after device has been closed");
        }
    }
}

/******************************************************************************
 * Shared memory methods
 ******************************************************************************/
android::Mutex gCamDev1BaseMemeryMapLock;   //used for mMemoryMap read/write protection

camera_memory_t* CameraDevice1Base::sGetMemory(int fd, size_t buf_size, uint_t num_bufs, void *user) {
    ALOGV("%s", __FUNCTION__);

    int cameraId = (int)(uintptr_t) user;
    sp<CameraDevice1Base> object = nullptr;
    {
        Mutex::Autolock lock(gCameraDevice1Baselock);
        for(list<wp<CameraDevice1Base>>::iterator it = glwpCameraDevice1Base.begin(); it != glwpCameraDevice1Base.end(); it++)
        {
            if( (*it).promote() != 0 &&
                (*it).promote()->getInstanceId() == cameraId)
            {
                object = (*it).promote();
                break;
            }
        }
        if(object == nullptr)
        {
            ALOGW("%s: cameraId %d object is nullptr", __FUNCTION__, cameraId);
            return nullptr;
        }
    }

    if (object->mDeviceCallback == nullptr) {
        ALOGE("%s: camera HAL request memory while camera is not opened!", __FUNCTION__);
        return nullptr;
    }

    sp<CameraHeapMemory> mem = nullptr;
    if (fd < 0) {
        if(object->mMemAllocator == nullptr){
            ALOGE("%s: cannot get ashmemAllocator", __FUNCTION__);
            return nullptr;
        }
        else{
            mem = new CameraHeapMemory(object->mMemAllocator, buf_size, num_bufs);
        }
    } else {
        mem = new CameraHeapMemory(fd, buf_size, num_bufs);
    }
    hidl_handle hidlHandle = mem->mHidlHandle;

    if ( sGetDevConnect(object->mInstanceName) ){   //check if device is disconneted
        Mutex::Autolock _l(gCamDev1BaseMemeryMapLock);
        hardware::Return<uint32_t> ret = object->mDeviceCallback->registerMemory(hidlHandle, buf_size, num_bufs);
        if (!ret.isOk()) {
            ALOGE("Transaction error in ICameraDeviceCallback::registerMemory: %s", ret.description().c_str());
            mem = nullptr;
            return nullptr;
        }
        else{
            MemoryId id = ret;
            mem->handle.mId = id;
            if (object->mMemoryMap.count(id) != 0) {
                ALOGE("%s: duplicate MemoryId %d returned by client!", __FUNCTION__, id);
            }
            object->mMemoryMap[id] = mem;

            mem->handle.mDevice = object;
            return &mem->handle;
        }
    }
    else{
        ALOGW("ICameraDeviceCallback::registerMemory is called after device has been closed");
        return nullptr;
    }
}

void CameraDevice1Base::sPutMemory(camera_memory_t *data) {

    if (!data)
        return;

    sp<CameraHeapMemory> mem = static_cast<CameraHeapMemory *>(data->handle);
    sp<CameraDevice1Base> object = mem->handle.mDevice;
    if (object == nullptr) {
        ALOGE("%s: camera HAL return memory for a null device!", __FUNCTION__);
        return;
    }
    if (object->mDeviceCallback == nullptr) {
        ALOGE("%s: camera HAL return memory while camera is not opened!", __FUNCTION__);
        return;
    }

    Mutex::Autolock _l(gCamDev1BaseMemeryMapLock);
    if ( sGetDevConnect(object->mInstanceName) ){   //check if device is disconneted
        hardware::Return<void> ret = object->mDeviceCallback->unregisterMemory(mem->handle.mId);
        if (!ret.isOk()) {
            ALOGE("Transaction error in ICameraDeviceCallback::unregisterMemory: %s", ret.description().c_str());

        }
    }
    else{
        ALOGW("ICameraDeviceCallback::unregisterMemory is called after device has been closed");
    }

    object->mMemoryMap.erase(mem->handle.mId);

    return;
}

/******************************************************************************
 * Release Recording Frame Template
 ******************************************************************************/
void CameraDevice1Base::releaseRecordingFrameLocked(
        uint32_t memId, uint32_t bufferIndex, const native_handle_t* handle) {

    Mutex::Autolock _l(this->mOpsLock);
    sp<CameraHeapMemory> camMemory = nullptr;
    {
        Mutex::Autolock _l(gCamDev1BaseMemeryMapLock);
        auto search = mMemoryMap.find(memId);
        if(search != mMemoryMap.end()) {
            camMemory = mMemoryMap.at(memId);
        }
        else{
            MY_LOGW("Cannot find memId(%d)", memId);
            return;
        }
    }
    if (bufferIndex >= camMemory->mNumBufs) {
        MY_LOGE("%s: bufferIndex %d exceeds number of buffers %d",
                __FUNCTION__, bufferIndex, camMemory->mNumBufs);
        return;
    }
    void *data = ((uint8_t *) camMemory->mHidlHeapMemData) + bufferIndex * camMemory->mBufSize;
    if (handle) {
        VideoNativeHandleMetadata* md = (VideoNativeHandleMetadata*) data;
        if (md->eType == kMetadataBufferTypeNativeHandleSource) {
            // Input handle will be closed by HIDL transport later, so clone it
            // HAL implementation is responsible to close/delete the clone
            native_handle_t* clone = native_handle_clone(handle);
            if (!clone) {
                MY_LOGE("%s: failed to clone buffer %p", __FUNCTION__, handle);
                return;
            }
            md->pHandle = clone;
        } else {
            MY_LOGE("%s:Malform VideoNativeHandleMetadata at memId %d, bufferId %d",
                    __FUNCTION__, memId, bufferIndex);
            return;
        }
    }
    this->releaseRecordingFrame_oldStyle(data);
}

void CameraDevice1Base::sMtkDataCb(int32_t msg_type, camera_metadata_t *result,
                        camera_metadata_t *charateristic, void *user) {
    ALOGV("%s: CameraDevice1Base.wrapper.cpp Start!",
__FUNCTION__);

    ::android::sp<IMetadataConverter>  mMetadataConverter =
    IMetadataConverter::createInstance(IDefaultMetadataTagSet::singleton()->getTagSet());

    IMetadata  staticIMetadata, resultIMetadata;
    ::android::hardware::hidl_vec<uint8_t> staticMetadataHvec, resultMetadataMetadataHvec;

    mMetadataConverter->convert(result, resultIMetadata);
    mMetadataConverter->convert(charateristic, staticIMetadata);

    mMetadataConverter->convertToHidl( resultIMetadata, &resultMetadataMetadataHvec);
    mMetadataConverter->convertToHidl( staticIMetadata, &staticMetadataHvec);

    int cameraId = (int)(uintptr_t) user;
    sp<CameraDevice1Base> object = nullptr;
    {
        Mutex::Autolock lock(gCameraDevice1Baselock);
        for(list<wp<CameraDevice1Base>>::iterator it = glwpCameraDevice1Base.begin(); it != glwpCameraDevice1Base.end(); it++)
        {
            if( (*it).promote() != 0 &&
                (*it).promote()->getInstanceId() == cameraId)
            {
                object = (*it).promote();
                break;
            }
        }
        if(object == nullptr)
        {
            ALOGW("%s: cameraId %d object is nullptr", __FUNCTION__, cameraId);
            return;
        }
    }
    if (object->mMtkDeviceCallback != nullptr) {

        if ( sGetDevConnect(object->mInstanceName) ){   //check if device is disconneted
            hardware::Return<void> ret = object->mMtkDeviceCallback->mtkMetadataCallback((MtkMetadataCallbackMsg) msg_type,
            resultMetadataMetadataHvec, staticMetadataHvec);
            if (!ret.isOk()) {
                ALOGE("Transaction error in ICameraDeviceCallback::sMtkDataCb: %s", ret.description().c_str());
            }
        }
        else{
            ALOGW("sMtkDataCb is called after device has been closed");
        }
    }
    else
        ALOGW("%s: object->mMtkDeviceCallback == nullprt!", __FUNCTION__);

}

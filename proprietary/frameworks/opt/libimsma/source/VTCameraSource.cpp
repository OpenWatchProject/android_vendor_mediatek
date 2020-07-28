/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//#define LOG_NDEBUG 0
#define LOG_TAG "VTCameraSource"

#include <inttypes.h>

#include <media/stagefright/foundation/ADebug.h>
#include "VTCameraSource.h"
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MetaData.h>
#include <OMX_IVCommon.h>
#include <media/hardware/HardwareAPI.h>
#include <media/hardware/MetadataBufferType.h>

#include <ui/GraphicBuffer.h>
#include <gui/BufferItem.h>
#include <gui/ISurfaceComposer.h>
#include <OMX_Component.h>

///*for HUAWEI hidl*/ #include <ui/gralloc_extra.h>

#include <utils/Log.h>
#include <utils/String8.h>

//#include <private/gui/ComposerService.h>
#include <ui/GraphicBufferMapper.h>
#include "comutils.h"
#include <cutils/properties.h>

#undef LOG_TAG
#define LOG_TAG "[VT][Source][CAM]"

#ifdef MTK_AOSP_ENHANCEMENT
#define ATRACE_TAG ATRACE_TAG_GRAPHICS
#include <utils/Trace.h>

#endif

#include <hardware/gralloc1.h>


enum {
    kKeyViLTERotateMethod        = 'rotM',
    kKeyViLTERotateTarWidth     = 'rotW',
    kKeyViLTERotateTarHeight    = 'rotH',
    kKeyViLTELongTimeNoData     = 'lono',
};

namespace android
{

VTCameraSource::VTCameraSource(int32_t multiID) :
    mMultiInstanceID(multiID),
    mCurrentSlot(BufferQueue::INVALID_BUFFER_SLOT),
    mNumPendingBuffers(0),
    mCurrentTimestamp(0),
    mFrameRate(30),
    mStarted(false),
    mNumFramesReceived(0),
    mNumFramesEncoded(0),
    mFirstFrameTimestamp(0),
    mMaxAcquiredBufferCount(4),    // XXX double-check the default
    mUseAbsoluteTimestamps(false)
{
    VT_LOGD("[ID=%d]VTCameraSource",mMultiInstanceID);

    mSrcWidth =  640;
    mSrcHeight = 480;
    mTarWidth = mSrcHeight;
    mTarHeight = mSrcWidth;

    mDegree = 270;
    mRotateMethod = 4;
    mStartTimeNs = 0;
    BufferQueue::createBufferQueue(&mProducer, &mConsumer);

	mBufferType = kMetadataBufferTypeANWBuffer;
    mLongTimeNoDataTd = 260;
    char value[PROPERTY_VALUE_MAX];

    if(property_get("persist.vendor.vt.cam_nodata", value, NULL)) {       //resolution change interval
        mLongTimeNoDataTd = atoi(value);
        VT_LOGI("[ID=%d]mLongTimeNoDataTd %d",mMultiInstanceID,mLongTimeNoDataTd);
    }
	if(property_get("persist.vendor.vt.bufctl", value, NULL)) {       //buffer flow
        mBufferType = (MetadataBufferType)(atoi(value));
        VT_LOGI("[ID=%d]mBufferType %d",mMultiInstanceID,mBufferType);
    }
}

VTCameraSource::~VTCameraSource()
{
    VT_LOGD("[ID=%d]~VTCameraSource mStarted %d",mMultiInstanceID,mStarted);
    CHECK(!mStarted);
}

nsecs_t VTCameraSource::getTimestamp()
{
    //VT_LOGD("[ID=%d]getTimestamp %lld ",mMultiInstanceID,mCurrentTimestamp);
    Mutex::Autolock lock(mMutex);
    return mCurrentTimestamp;
}

void VTCameraSource::setFrameAvailableListener(
    const sp<FrameAvailableListener>& listener)
{
    VT_LOGD("[ID=%d]setFrameAvailableListener",mMultiInstanceID);
    Mutex::Autolock lock(mMutex);
    mFrameAvailableListener = listener;
}
#if 0
void VTCameraSource::dump(String8& result) const
{
    char buffer[1024];
    dump(result, "", buffer, 1024);
}

void VTCameraSource::dump(
    String8& result,
    const char* /* prefix */,
    char* buffer,
    size_t /* SIZE */) const
{
    Mutex::Autolock lock(mMutex);

    result.append(buffer);
    mConsumer->dump(result, "");
}
#endif
status_t VTCameraSource::setFrameRate(int32_t fps)
{
    VT_LOGD("[ID=%d]setFrameRate %d",mMultiInstanceID,fps);
    Mutex::Autolock lock(mMutex);
    const int MAX_FRAME_RATE = 60;

    if(fps < 0 || fps > MAX_FRAME_RATE) {
        return BAD_VALUE;
    }

    mFrameRate = fps;
    return OK;
}


status_t VTCameraSource::setResolutionDegree(int srcWidth, int srcHight,int tarWidth, int tarHeight,int degree,int rotateMethod)
{
    VT_LOGD("[ID=%d]setWHWHDM  [%d : %d] [%d : %d] : %d : %d",mMultiInstanceID,
            srcWidth, srcHight,tarWidth,  tarHeight,degree,rotateMethod);
    Mutex::Autolock lock(mMutex);
    mSrcWidth = srcWidth;
    mSrcHeight = srcHight;
    mTarWidth = tarWidth;
    mTarHeight = tarHeight;
    mDegree = degree;
    mRotateMethod = rotateMethod;
    mConsumer->setDefaultBufferSize(mSrcWidth, mSrcHeight);

    return OK;
}

status_t VTCameraSource::createInputProducer(sp<IGraphicBufferProducer>* outBufferProducer,int32_t hal)
{
    if(mSrcWidth <= 0 || mSrcHeight <= 0 || hal <=0 ) {
        VT_LOGE("[ID=%d]Invalid dimensions %d x %d,hal %d",mMultiInstanceID, mSrcWidth, mSrcHeight,hal);
    }

    mConsumer->setDefaultBufferSize(mSrcWidth, mSrcHeight);
    mConsumer->setConsumerUsageBits(GRALLOC_USAGE_HW_VIDEO_ENCODER);

    PixelFormat format = HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED;//HAL_PIXEL_FORMAT_YV12;
    /*
    HAL_PIXEL_FORMAT_RGBA_8888 = 0x1
    HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED 0x22
    HAL_PIXEL_FORMAT_YV12://0x32315659:842094169
    HAL_PIXEL_FORMAT_YCRCB_420_SP:// 0x11
    */
    if(hal == 1) {
		format = HAL_PIXEL_FORMAT_RGBA_8888;
    }else if(hal == 3){
		format = HAL_PIXEL_FORMAT_YV12;
    }

    char value[PROPERTY_VALUE_MAX];

    if(property_get("persist.vendor.vt.cam.format", value, NULL)) {
        format = atoi(value);
    }

    VT_LOGI("[ID=%d]format %s,hal %d",mMultiInstanceID,PixelFormatToString(format),hal);
    mConsumer->setDefaultBufferFormat(format);    //HAL_PIXEL_FORMAT_YV12);

    mConsumer->setConsumerName(String8("[VT][Source][CAM]"));

    //sp<ISurfaceComposer> composer(ComposerService::getComposerService());

    // Note that we can't create an sp<...>(this) in a ctor that will not keep a
    // reference once the ctor ends, as that would cause the refcount of 'this'
    // dropping to 0 at the end of the ctor.  Since all we need is a wp<...>
    // that's what we create.
    wp<ConsumerListener> listener = static_cast<ConsumerListener*>(this);
    sp<BufferQueue::ProxyConsumerListener> proxy = new BufferQueue::ProxyConsumerListener(listener);

    status_t err = mConsumer->consumerConnect(proxy, false);

    if(err != NO_ERROR) {
        VT_LOGE("[ID=%d]error connecting to BufferQueue: %s (%d)",mMultiInstanceID, strerror(-err), err);
    }

    //set mProducer
    // mProducer->set

    *outBufferProducer = mProducer;
    return err;
}


MetadataBufferType VTCameraSource::metaDataStoredInVideoBuffers() const
{
    VT_LOGV("[ID=%d]isMetaDataStoredInVideoBuffers mBufferType %d",mMultiInstanceID,mBufferType);
	if(mBufferType == 2){
    	return kMetadataBufferTypeANWBuffer;
	}
	return kMetadataBufferTypeInvalid;
}

int32_t VTCameraSource::getFrameRate() const
{
    VT_LOGD("[ID=%d]getFrameRate %d",mMultiInstanceID,mFrameRate);
    Mutex::Autolock lock(mMutex);
    return mFrameRate;
}

status_t VTCameraSource::start(MetaData *params)
{
    VT_LOGD("[ID=%d]start",mMultiInstanceID);

    Mutex::Autolock lock(mMutex);

    //CHECK(!mStarted);//we can start repeatly

    //  mStartTimeNs = 0;

    int64_t startTimeUs;
    //  int32_t bufferCount = 0;

    if(params) {
        if(params->findInt64(kKeyTime, &startTimeUs)) {
            mStartTimeNs = startTimeUs * 1000;
            VT_LOGD("[ID=%d]start @ =%" PRId64 "  Us",mMultiInstanceID,startTimeUs);
        }
    }

    /*
        int64_t startTimeUs;
        int32_t bufferCount = 0;

        if (params) {
            if (params->findInt64(kKeyTime, &startTimeUs)) {
                mStartTimeNs = startTimeUs * 1000;
            }

            if (!params->findInt32(kKeyNumBuffers, &bufferCount)) {
                VT_LOGE("[ID=%d]Failed to find the advertised buffer count");
                return UNKNOWN_ERROR;
            }

            if (bufferCount <= 1) {
                VT_LOGE("[ID=%d]bufferCount %d is too small", bufferCount);
                return BAD_VALUE;
            }

            mMaxAcquiredBufferCount = bufferCount;
        }


        CHECK_GT(mMaxAcquiredBufferCount, 1);

        status_t err =
            mConsumer->setMaxAcquiredBufferCount(mMaxAcquiredBufferCount);

        if (err != OK) {
            return err;
        }

    */
    mNumPendingBuffers = 0;
    mStarted = true;

    return OK;
}

status_t VTCameraSource::setMaxAcquiredBufferCount(size_t count)
{
    VT_LOGD("[ID=%d]setMaxAcquiredBufferCount(%zu)",mMultiInstanceID, count);
    Mutex::Autolock lock(mMutex);

    CHECK_GT(count, 1u);
    mMaxAcquiredBufferCount = count;

    CHECK_GT(mMaxAcquiredBufferCount, 1u);

    status_t err = mConsumer->setMaxAcquiredBufferCount(mMaxAcquiredBufferCount);
    VT_LOGD("[ID=%d]err(%d)",mMultiInstanceID, err);

    if(err != OK) {
        return err;
    }

    return OK;
}

status_t VTCameraSource::setUseAbsoluteTimestamps()
{
    VT_LOGD("[ID=%d]setUseAbsoluteTimestamps",mMultiInstanceID);
    Mutex::Autolock lock(mMutex);
    mUseAbsoluteTimestamps = true;

    return OK;
}

status_t VTCameraSource::stop()
{
    VT_LOGD("[ID=%d]stop,mStarted %d",mMultiInstanceID,mStarted);
    Mutex::Autolock lock(mMutex);

    if(!mStarted) {
        return OK;
    }

    mStarted = false;
    mFrameAvailableCondition.signal();

    while(mNumPendingBuffers > 0) {
        VT_LOGI("[ID=%d]Still waiting for %zu buffers to be returned.",mMultiInstanceID,mNumPendingBuffers);

#if DEBUG_PENDING_BUFFERS

        for(size_t i = 0; i < mPendingBuffers.size(); ++i) {
            VT_LOGI("[ID=%d]mPendingBuffers:%d: %p",mMultiInstanceID, i, mPendingBuffers.itemAt(i));
        }

#endif

        mMediaBuffersAvailableCondition.wait(mMutex);
    }

    mMediaBuffersAvailableCondition.signal();

    return mConsumer->consumerDisconnect();
}

sp<MetaData> VTCameraSource::getFormat()
{
    VT_LOGD("[ID=%d]getFormat",mMultiInstanceID);

    Mutex::Autolock lock(mMutex);
    sp<MetaData> meta = new MetaData;

    meta->setInt32(kKeyWidth, mSrcWidth);
    meta->setInt32(kKeyHeight, mSrcHeight);
    meta->setInt32(kKeyViLTERotateTarWidth,mTarWidth);
    meta->setInt32(kKeyViLTERotateTarHeight, mTarHeight);
    // The encoder format is set as an opaque colorformat
    // The encoder will later find out the actual colorformat
    // from the GL Frames itself.
    meta->setInt32(kKeyPixelFormat, HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED);    //TODO
    meta->setInt32(kKeyStride, mSrcWidth);
    meta->setInt32(kKeySliceHeight, mSrcHeight);
    meta->setInt32(kKeyFrameRate, mFrameRate);
    meta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_VIDEO_RAW);
    return meta;
}

// Pass the data to the MediaBuffer. Pass in only the metadata
// Note: Call only when you have the lock
void VTCameraSource::passMetadataBuffer_l(MediaBufferBase **buffer,
        ANativeWindowBuffer *bufferHandle) const
{
    *buffer = MediaBufferBase::Create(sizeof(VideoNativeMetadata));
    VideoNativeMetadata *data = (VideoNativeMetadata *)(*buffer)->data();

    if(data == NULL) {
        VT_LOGE("[ID=%d]Cannot allocate memory for metadata buffer!",mMultiInstanceID);
        return;
    }

    data->eType = metaDataStoredInVideoBuffers();
    data->pBuffer = bufferHandle;
    data->nFenceFd = -1;
    VT_LOGV("[ID=%d]handle = %p, offset = %zu, length = %zu  size %zu",mMultiInstanceID,
            bufferHandle, (*buffer)->range_offset(), (*buffer)->range_length(),sizeof(VideoNativeMetadata));
}

status_t VTCameraSource::read(
    MediaBufferBase **buffer, const ReadOptions *  options)
{

    VT_LOGV("[ID=%d]read+ mNumPendingBuffers %zu",mMultiInstanceID,mNumPendingBuffers);
    sp<Fence> bufferFence;
    Mutex::Autolock lock(mMutex);
    static int32_t contiueNotData = 0;

    *buffer = NULL;

    while(mStarted && mNumPendingBuffers == mMaxAcquiredBufferCount) {
        VT_LOGD("[ID=%d]read+mNumPendingBuffers %zu >= mMaxAcquiredBufferCount %zu",
                mMultiInstanceID,mNumPendingBuffers,mMaxAcquiredBufferCount);

        if(options->getNonBlocking()) {
            return -EAGAIN;
        } else {
            mMediaBuffersAvailableCondition.wait(mMutex);
        }
    }

    // Update the current buffer info
    // TODO: mCurrentSlot can be made a bufferstate since there
    // can be more than one "current" slots.

    BufferItem item;

    // If the recording has started and the queue is empty, then just
    // wait here till the frames come in from the client side
    while(mStarted) {

        status_t err = mConsumer->acquireBuffer(&item, 0);

        if(err == BufferQueue::NO_BUFFER_AVAILABLE) {
            // wait for a buffer to be queued
            if(options->getNonBlocking()) {
                if(contiueNotData > 20) {
                    VT_LOGD("[ID=%d]NO_BUFFER_AVAILABLE for 20 times in a row ",mMultiInstanceID);
                    contiueNotData = 0;
                }

                contiueNotData++;
                return -EAGAIN;
            } else {
                mFrameAvailableCondition.wait(mMutex);
            }
        } else if(err == OK) {
#ifdef MTK_AOSP_ENHANCEMENT
            bufferFence = item.mFence;
#else
            err = item.mFence->waitForever("VTCameraSource::read");
#endif

            if(err) {
                VT_LOGW("[ID=%d]read: failed to wait for buffer fence: %d",mMultiInstanceID, err);
            }

            if(item.mSlot < 0 || item.mSlot >= BufferQueue::NUM_BUFFER_SLOTS){
                VT_LOGE("error item slot number %d NUM_BUFFER_SLOTS=%d", item.mSlot, BufferQueue::NUM_BUFFER_SLOTS);
                return ERROR_OUT_OF_RANGE;
            }

            if(item.mGraphicBuffer != NULL) {
                mSlots[item.mSlot].mGraphicBuffer = item.mGraphicBuffer;
            }

            mSlots[item.mSlot].mFrameNumber = item.mFrameNumber;

            // check for the timing of this buffer
            if(/*mNumFramesReceived == 0 && !*/mUseAbsoluteTimestamps) {    //camera is  systime, startTime is systime, use time to skip old buffer
                if(mNumFramesReceived == 0) {
                    mFirstFrameTimestamp = item.mTimestamp;
                    VT_LOGD("[ID=%d]mFirstFrameTimestamp: =%" PRId64 " ms",mMultiInstanceID, mFirstFrameTimestamp/1000000);
                }

                // Initial delay
                if(mStartTimeNs > 0) {
                    if(item.mTimestamp < mStartTimeNs) {
                        VT_LOGD("[ID=%d][ms] mTimestamp: =%" PRId64 " < mTimestamp: =%" PRId64 " ",
                                mMultiInstanceID, item.mTimestamp/1000000,mStartTimeNs/1000000);
                        // This frame predates start of record, discard
                        mConsumer->releaseBuffer(
                            item.mSlot, item.mFrameNumber, EGL_NO_DISPLAY, EGL_NO_SYNC_KHR, Fence::NO_FENCE);
                        continue;
                    }

                    //mStartTimeNs = item.mTimestamp - mStartTimeNs;//mUseAbsoluteTimestamps
                }
            }

            //item.mTimestamp = mStartTimeNs + (item.mTimestamp - mFirstFrameTimestamp);

            mNumFramesReceived++;

            break;
        } else {
            VT_LOGE("[ID=%d]read: acquire failed with error code %d",mMultiInstanceID, err);
            return ERROR_END_OF_STREAM;
        }

    }

    // If the loop was exited as a result of stopping the recording,
    // it is OK
    if(!mStarted) {
        VT_LOGD("[ID=%d]Read: stopped. Returning ERROR_END_OF_STREAM.",mMultiInstanceID);
        return ERROR_END_OF_STREAM;
    }

    mCurrentSlot = item.mSlot;

    // First time seeing the buffer?  Added it to the SMS slot
    if(item.mGraphicBuffer != NULL) {
        mSlots[item.mSlot].mGraphicBuffer = item.mGraphicBuffer;
    }

    mSlots[item.mSlot].mFrameNumber = item.mFrameNumber;//should we need to move this code to mGraphicBuffer!=NULL case?

//#ifdef MTK_AOSP_ENHANCEMENT
    uint32_t width = 0;
    uint32_t height = 0;
    PixelFormat format = 0;

    if(CC_LIKELY(mSlots[mCurrentSlot].mGraphicBuffer == NULL)) {
        VT_LOGW("[ID=%d]read: acquire slot(%d) buffer is NULL",mMultiInstanceID, mCurrentSlot);
        return ERROR_END_OF_STREAM;
    } else {
        width = mSlots[mCurrentSlot].mGraphicBuffer->getWidth();
        height = mSlots[mCurrentSlot].mGraphicBuffer->getHeight();
        format = mSlots[mCurrentSlot].mGraphicBuffer->getPixelFormat();
        VT_LOGV("[ID=%d]read w %d, h %d",mMultiInstanceID, width,height);
        CHECK(width > 0);
        CHECK(height > 0);

        if((mSrcWidth > 0) && (mSrcHeight > 0)) {
            if((width != (uint32_t) mSrcWidth) || (height != (uint32_t) mSrcHeight)) {
                mConsumer->releaseBuffer(item.mSlot, item.mFrameNumber,EGL_NO_DISPLAY, EGL_NO_SYNC_KHR, Fence::NO_FENCE);
                VT_LOGW("[ID=%d]come[%d, %d] <-> set[%d, %d] is not match, release buffer item",mMultiInstanceID, width,height, mSrcWidth, mSrcHeight);
                return -EAGAIN;
            }
        } else {
            CHECK(false);    //Is there possible to get buffer before set W/H?
            //mConsumer->releaseBuffer(item.mSlot, item.mFrameNumber,
            //                 EGL_NO_DISPLAY, EGL_NO_SYNC_KHR, Fence::NO_FENCE);
            //return -EAGAIN;
        }
    }

//#endif

    mCurrentBuffers.push_back(mSlots[mCurrentSlot].mGraphicBuffer);
    int64_t prevTimeStamp = mCurrentTimestamp;
    mCurrentTimestamp = item.mTimestamp;

    mNumFramesEncoded++;
    // Pass the data to the MediaBuffer. Pass in only the metadata

    passMetadataBuffer_l(buffer, mSlots[mCurrentSlot].mGraphicBuffer->getNativeBuffer());

    (*buffer)->setObserver(this);
    (*buffer)->add_ref();
    (*buffer)->meta_data().setInt64(kKeyTime, mCurrentTimestamp / 1000);
    (*buffer)->meta_data().setInt32(kKeyWidth,  width);
    (*buffer)->meta_data().setInt32(kKeyHeight,  height);
    (*buffer)->meta_data().setInt32(kKeyRotation,  mDegree);
    (*buffer)->meta_data().setInt32(kKeyViLTERotateMethod,  mRotateMethod);
    (*buffer)->meta_data().setInt32(kKeyViLTERotateTarWidth,  mTarWidth);
    (*buffer)->meta_data().setInt32(kKeyViLTERotateTarHeight,  mTarHeight);
    (*buffer)->meta_data().setInt32(kKeyPixelFormat,  format);
    VT_LOGD("[ID=%d]Frames encoded = %d, timestamp = % " PRId64 " , time diff = %" PRId64",format[%s],WH[%d,%d][%d,%d],",
            mMultiInstanceID,
            mNumFramesEncoded, mCurrentTimestamp /1000000,
            mCurrentTimestamp /1000000 - prevTimeStamp /1000000,
            PixelFormatToString(format),width,height,mTarWidth,mTarHeight);

    if(mCurrentTimestamp /1000000 - prevTimeStamp /1000000 > mLongTimeNoDataTd) {
        ALOGD("jzm set logtime flag %" PRId64 " > %d", mCurrentTimestamp /1000000 - prevTimeStamp /1000000, mLongTimeNoDataTd);
        (*buffer)->meta_data().setInt32(kKeyViLTELongTimeNoData,  1);
    } else {
        (*buffer)->meta_data().setInt32(kKeyViLTELongTimeNoData,  0);
    }

    ++mNumPendingBuffers;
    contiueNotData = 0;

#if DEBUG_PENDING_BUFFERS
    mPendingBuffers.push_back(*buffer);
#endif

    VT_LOGV("[ID=%d]returning mbuf %p",mMultiInstanceID, *buffer);
#ifdef MTK_AOSP_ENHANCEMENT

    // wait fence here to avoid blocking
    // 1. onFrameAvailable() callback
    // 2. signalBufferReturned()
    if(bufferFence != NULL) {
        bufferFence->waitForever("VTCameraSource::read");
    }

    //ATRACE_ASYNC_END(atrace_tag, ext_info.sequence);
#endif

    return OK;
}


//need link  libgralloc_extra_sys.so for system image
static int32_t getBufferFormat(buffer_handle_t  handle)
{
       int32_t format = 0;
	   /* //HUAWEI
	int ret = gralloc_extra_query((buffer_handle_t) handle, GRALLOC_EXTRA_GET_FORMAT, &format);

	if(ret != OK) {
		return -1;
	}*/
	const hw_module_t *hwMod;
	gralloc1_device_t *grDev;
	GRALLOC1_PFN_GET_FORMAT grFuncGetFormat;

	if(hw_get_module(GRALLOC_HARDWARE_MODULE_ID, &hwMod) < 0 ){
		return -1;
	}
	if(gralloc1_open(hwMod, &grDev)<0){
		return -1;
	}

	grFuncGetFormat = (GRALLOC1_PFN_GET_FORMAT)grDev->getFunction(grDev, GRALLOC1_FUNCTION_GET_FORMAT);
	grFuncGetFormat(grDev, handle, &format);
       gralloc1_close(grDev);

    VT_LOGD("format %s",PixelFormatToString(format)); ///*for HUAWEI hidl*/
       return format;
}
// google issue, fixed by MTK temporarily ==>N
static buffer_handle_t getMediaBufferHandle(MediaBufferBase *buffer)
{
    VideoNativeMetadata *data = (VideoNativeMetadata *) buffer->data();

    if(data == NULL) {
        VT_LOGE("NULL metadata buffer data!");
        return 0;
    }

    ANativeWindowBuffer *bufferHandle = data->pBuffer;
    return bufferHandle->handle;
}

//M and O

/*static buffer_handle_t getMediaBufferHandle(MediaBuffer *buffer) {
    // need to convert to char* for pointer arithmetic and then
    // copy the byte stream into our handle
    buffer_handle_t bufferHandle;
    memcpy(&bufferHandle, (char*)(buffer->data()) + 4, sizeof(buffer_handle_t));
    return bufferHandle;
}*/


void* VTCameraSource::getMediaBufferHandleVA(MediaBufferBase *buffer,int32_t *format)
{

    int32_t width = 0;
    int32_t height = 0;
    status_t err = OK;

    if(!(err = buffer->meta_data().findInt32(kKeyWidth,  &width))) {
        VT_LOGE("err %d",err);
        return NULL;
    }

    if(!(err = buffer->meta_data().findInt32(kKeyHeight,  &height))) {
        VT_LOGE("err %d",err);
        return NULL;
    }

    void*  ptr = NULL;
    buffer_handle_t _handle = getMediaBufferHandle(buffer);


    *format = getBufferFormat(_handle);

    Rect  mRect(width, height);
    err = GraphicBufferMapper::getInstance().lock(_handle,
            GraphicBuffer::USAGE_SW_READ_OFTEN,
            mRect,
            &ptr);

    GraphicBufferMapper::getInstance().unlock(_handle);

    if(err != OK) {
        VT_LOGE("err %d",err);
        return NULL;
    }

    return ptr;

}

void VTCameraSource::signalBufferReturned(MediaBufferBase *buffer)
{
    VT_LOGV("[ID=%d]signalBufferReturned",mMultiInstanceID);

    bool foundBuffer = false;

    Mutex::Autolock lock(mMutex);

    buffer_handle_t bufferHandle = getMediaBufferHandle(buffer);

    for(size_t i = 0; i < mCurrentBuffers.size(); i++) {
        if(mCurrentBuffers[i]->handle == bufferHandle) {
            mCurrentBuffers.removeAt(i);
            foundBuffer = true;
            break;
        }
    }

    if(!foundBuffer) {
        VT_LOGW("[ID=%d]returned buffer was not found in the current buffer list",mMultiInstanceID);
    }

    for(int id = 0; id < BufferQueue::NUM_BUFFER_SLOTS; id++) {
        if(mSlots[id].mGraphicBuffer == NULL) {
            continue;
        }

        if(bufferHandle == mSlots[id].mGraphicBuffer->handle) {
            VT_LOGV("[ID=%d]Slot %d returned, matches handle = %p", id,
                    mMultiInstanceID,mSlots[id].mGraphicBuffer->handle);

            mConsumer->releaseBuffer(id, mSlots[id].mFrameNumber,
                                     EGL_NO_DISPLAY, EGL_NO_SYNC_KHR,
                                     Fence::NO_FENCE);

            buffer->setObserver(0);
            buffer->release();

            foundBuffer = true;
            break;
        }
    }

    if(!foundBuffer) {
        CHECK(!"signalBufferReturned: bogus buffer");
    }

#if DEBUG_PENDING_BUFFERS

    for(size_t i = 0; i < mPendingBuffers.size(); ++i) {
        if(mPendingBuffers.itemAt(i) == buffer) {
            mPendingBuffers.removeAt(i);
            break;
        }
    }

#endif

    --mNumPendingBuffers;
    mMediaBuffersAvailableCondition.broadcast();
}

// Part of the BufferQueue::ConsumerListener
void VTCameraSource::onFrameAvailable(const BufferItem& /* item */)
{
    VT_LOGV("[ID=%d]onFrameAvailable",mMultiInstanceID);

    sp<FrameAvailableListener> listener;
    {
        // scope for the lock
        Mutex::Autolock lock(mMutex);
        mFrameAvailableCondition.broadcast();
        listener = mFrameAvailableListener;
    }

    if(listener != NULL) {
        VT_LOGV("[ID=%d]actually calling onFrameAvailable",mMultiInstanceID);
        listener->onFrameAvailable();
    }
}

// VTCameraSource hijacks this event to assume
// the prodcuer is disconnecting from the BufferQueue
// and that it should stop the recording
void VTCameraSource::onBuffersReleased()
{
    VT_LOGV("[ID=%d]onBuffersReleased",mMultiInstanceID);

    Mutex::Autolock lock(mMutex);

    mFrameAvailableCondition.signal();

    for(int i = 0; i < BufferQueue::NUM_BUFFER_SLOTS; i++) {
        mSlots[i].mGraphicBuffer = 0;
    }
}

void VTCameraSource::onSidebandStreamChanged()
{
    ALOG_ASSERT(false, "VTCameraSource can't consume sideband streams");
}

} // end of namespace android

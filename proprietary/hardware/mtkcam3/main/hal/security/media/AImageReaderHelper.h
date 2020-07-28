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
 * MediaTek Inc. (C) 2019. All rights reserved.
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

#ifndef VENDOR_MEDIATEK_HARDWARE_CAMERA3_MAIN_HAL_SECURITY_MEDIA_AIMAGEREADERHELPER_H
#define VENDOR_MEDIATEK_HARDWARE_CAMERA3_MAIN_HAL_SECURITY_MEDIA_AIMAGEREADERHELPER_H

#include <media/NdkImageReader.h>

#include <hidl/HybridInterface.h>

#include <android/hardware/graphics/bufferqueue/1.0/IGraphicBufferProducer.h>

#include "system/types.h"

#include <atomic>

#include <set>

// ------------------------------------------------------------------------

namespace NSCam {

class AImageReaderHelper final
{
public:
    // The maxImages parameter determines the maximum number of AImage objects
    // that can be acquired from the AImageReader simultaneously.
    //
    // Requesting more buffers will use up more memory, so it is important to
    // use only the minimum number necessary for the use case.
    AImageReaderHelper(
            int32_t width, int32_t height, int32_t format,
            uint64_t usage, int32_t maxImages, bool async = false);

    ~AImageReaderHelper();

    int initAImageReader();

    native_handle_t* getNativeWindow() { return mImageReaderAnw; }

    // NOTE: The default behavior is to return the amount of acquired images
    //       and return -1 if the image callback is available
    ssize_t getAcquiredImageCount();

    // AImageReader getter functions
    int32_t getWidth() const;
    int32_t getHeight() const;
    int32_t getFormat() const;

    using HGraphicBufferProducer =
        ::android::hardware::graphics::bufferqueue::V1_0::IGraphicBufferProducer;
    ::android::sp<HGraphicBufferProducer> getHGBPFromHandle();

    static bool getSecureHandle(
            const android::hardware::hidl_handle& bufferHandle,
            SECHAND& secureHandle);
    static bool getPixelFormat(
            const android::hardware::hidl_handle& bufferHandle,
            ::android::hardware::graphics::common::V1_0::PixelFormat& pixelFormat);

    using imageAvailable_cb = std::function<void(AImageReader*)>;
    void setImageCallback(imageAvailable_cb cb);

    void handleImageAvailable(AImageReader* reader);

    static void onImageAvailable(void* context, AImageReader* reader);

    static void onBufferRemoved(void* context, AImageReader* reader,
            AHardwareBuffer* buffer)
    {
        (void)context;
        (void)reader;
        (void)buffer;
    }

private:
    int32_t mWidth;
    int32_t mHeight;
    int32_t mFormat;
    uint64_t mUsage;
    int32_t mMaxImages;
    bool mAsync;

    // used to avoid deleting AImages after its parent AImageReader is closed.
    // also, use try_lock to avoid a deadlock when a ALooper thread calls
    // AImageReader_ImageListener::onImageAvailable() after
    // its parent AImageReader acqured the lock.
    mutable std::mutex mExitingLock;

    mutable std::mutex mImageAvailableLock;
    imageAvailable_cb mImageAvailable_cb;

    std::atomic_size_t mAcquiredImageCount;

    AImageReader *mImageReader = nullptr;
    // a native handle corresponding to the ANativeWindow, which is
    // the producer end of an image queue, owned by a AImageReader
    native_handle_t *mImageReaderAnw = nullptr;

    AImageReader_ImageListener mImageReaderImageAvailableListener {
        this, onImageAvailable };
    AImageReader_BufferRemovedListener mImageReaderBufferRemovedListener {
        this, onBufferRemoved };

    // a bookkeeping of all active AImageReaderHelper instances
    static std::mutex sAImageReaderHelperObserverLock;
    static std::set<void *> sAImageReaderHelperObserver;

    ::android::sp<HGraphicBufferProducer> convertNativeHandleToHGBP(
            const native_handle_t* handle);

    bool isFormatSupported(uint32_t format);

    struct Plane {
        Plane(uint32_t _width, uint32_t _height,
                uint32_t _pixelStride, uint32_t _rowStride)
        : width(_width),
          height(_height),
          pixelStride(_pixelStride),
          rowStride(_rowStride) {
        }

        // width in pixels
        uint32_t width;
        // height in pixels
        uint32_t height;
        // distance in bytes from the color channel of one pixel to the next
        uint32_t pixelStride;
        // distance in bytes from the first value of one row of the image to
        // the first value of the next row
        uint32_t rowStride;
    };
    std::vector<Plane> getPlanes(const AHardwareBuffer_Desc& desc);

    void dumpBuffer(const AHardwareBuffer_Desc& desc,
            const native_handle_t* bufferHandle, size_t imageCount);

}; // class AImageReaderHelper

} // namespace NSCam

#endif // VENDOR_MEDIATEK_HARDWARE_CAMERA3_MAIN_HAL_SECURITY_MEDIA_AIMAGEREADERHELPER_H
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

#define LOG_TAG "securecamera_test"

#include "utils/TestBase.h"
#include "media/AImageReaderHelper.h"

#include <mtkcam/utils/std/ULog.h>

#include <system/window.h>

// Define the MTK specific enumeration values for gralloc usage in order to
// avoid "polute" AOSP file
// (hardware/libhardware/include/hardware/gralloc1.h)
// The enumeration value definition must not be conflict with the gralloc1.h
// in original AOSP file
#include <gralloc1_mtk_defs.h>

#include <sync/sync.h>

#include <ui/BufferQueueDefs.h>

// ------------------------------------------------------------------------

namespace NSCam {
namespace tests {

CAM_ULOG_DECLARE_MODULE_ID(MOD_CAMERA_HAL_SERVER);

using ::android::sp;
using HGraphicBufferProducer = AImageReaderHelper::HGraphicBufferProducer;
using HFrameEventHistoryDelta = HGraphicBufferProducer::FrameEventHistoryDelta;
using ::android::hardware::graphics::common::V1_0::PixelFormat;
using ::android::hardware::hidl_handle;
using ::android::hardware::media::V1_0::AnwBuffer;
using ::android::hardware::media::V1_0::Fence;

// ------------------------------------------------------------------------

inline int readFdFromNativeHandle(const native_handle_t* nh, int index = 0)
{
    return ((nh == nullptr) || (nh->numFds == 0) ||
            (nh->numFds <= index) || (index < 0)) ?
            -1 : nh->data[index];
}

inline native_handle_t* createNativeHandleFromFd(int fd)
{
    if (fd < 0)
        return native_handle_create(0, 0);

    native_handle_t* nh = native_handle_create(1, 0);
    if (nh == nullptr)
        return nullptr;

    nh->data[0] = fd;
    return nh;
}

// ------------------------------------------------------------------------

/**
 * This test fixture is for testing class NdkImageReader with the private consumer usage.
 *
 * Note that not all format and usage flag combination is supported by the AImageReader,
 * especially if format is AIMAGE_FORMAT_PRIVATE, usage must not include either
 * AHARDWAREBUFFER_USAGE_READ_RARELY or AHARDWAREBUFFER_USAGE_READ_OFTEN.
 *
 * Reference: testImageReaderPrivateWithProtectedUsageFlag in ImageReaderTest.java
 */
class AImageReaderTest : public TestBase,
    public ::testing::WithParamInterface<uint64_t>
{
protected:
    void SetUp() override;
    void TearDown() override;

    static constexpr int32_t kImageWidth = 640;
    static constexpr int32_t kImageHeight = 480;
    /**
     * AIMAGE_FORMAT_* flags.
     *
     * Android private opaque image format.
     *
     * The choices of the actual format and pixel data layout are entirely up to
     * the device-specific and framework internal implementations,
     * and may vary depending on use cases even for the same device.
     *
     * Also note that the contents of these buffers are not directly accessible to
     * the application.
     *
     * NOTE: AIMAGE_FORMAT_* is a subset of ::android::PublicFormat::*;
     *       AIMAGE_FORMAT_PRIVATE (0x22) in
     *       frameworks/av/media/ndk/include/media/NdkImage.h
     *       is equivalent to
     *       HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED (34) in
     *       system/core/libsystem/include/system/graphics-base-v1.0.h or
     *       PixelFormat::IMPLEMENTATION_DEFINED (34) in
     *       hardware/interfaces/graphics/common/1.0/types.hal.
     */
    static constexpr int32_t kAImageFormat = AIMAGE_FORMAT_PRIVATE;

    /**
     * AHARDWAREBUFFER_USAGE_* flags.
     *
     * The buffer is protected from direct CPU access or being read by
     * non-secure hardware, such as video encoders.
     *
     * This flag is incompatible with CPU read and write flags. It is
     * mainly used when handling DRM video. Refer to the EGL extension
     * EGL_EXT_protected_content and GL extension
     * GL_EXT_protected_textures for more information on how these
     * buffers are expected to behave.
     */
    static constexpr uint64_t kAHardwareBufferUsage =
        AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT;

    static constexpr int32_t kMaxImages = 1;

    std::unique_ptr<AImageReaderHelper> mAImageReaderHelper;
}; // class AImageReaderTest

void AImageReaderTest::SetUp()
{
    TestBase::SetUp();

    // replace with a concrete AImageReaderHelper instance
    mAImageReaderHelper.reset(new AImageReaderHelper(
                kImageWidth, kImageHeight, kAImageFormat,
                kAHardwareBufferUsage, kMaxImages, false));

    // initialize AImageReader
    ASSERT_TRUE(mAImageReaderHelper->initAImageReader() == AMEDIA_OK);
}

void AImageReaderTest::TearDown()
{
    TestBase::TearDown();

    // Delete an AImageReader and return all images generated by
    // this reader to system.
    mAImageReaderHelper.reset(nullptr);
}

// ------------------------------------------------------------------------

TEST_P(AImageReaderTest, GetSecureHandleFromHGBP)
{
    constexpr int32_t kMaxDequeuedBuffers = 3;
    // [Producer]
    // Check that the HGBP (a.k.a. HIDL IGraphicBufferProducer or
    // HGraphicBufferProducer) can be retrieved from the handle.
    // AOSP design: Retrieves HGraphicBufferProducer corresponding to
    // the native_handle_t provided (this native handle MUST have been obtained
    // by AImageReader_getWindowNativeHandle()).
    sp<HGraphicBufferProducer> hgbp = mAImageReaderHelper->getHGBPFromHandle();
    ASSERT_NE(hgbp, nullptr);

    // connect attempts to connect a client API to the IGraphicBufferProducer.
    // This must be called before any other IGraphicBufferProducer methods are
    // called except for getAllocator. A consumer must be already connected.
    int32_t nwAPI = NATIVE_WINDOW_API_CAMERA;

    HGraphicBufferProducer::Status status;

    HGraphicBufferProducer::QueueBufferOutput output;
    hgbp->connect(nullptr, nwAPI, false,
            [&output, &status](const auto& _status, const auto& _output) {
                status = _status;
                ASSERT_EQ(::android::OK, _status);
                output = _output;
            });
    CAM_ULOGMD("%s", toString(output).c_str());

    // sets the maximum number of buffers that can be dequeued by
    // the producer at one time.
    ASSERT_EQ(hgbp->setMaxDequeuedBufferCount(kMaxDequeuedBuffers), ::android::OK);

    const int32_t width = mAImageReaderHelper->getWidth();
    const int32_t height = mAImageReaderHelper->getHeight();
    const int32_t format = mAImageReaderHelper->getFormat();

    std::vector<int32_t> dequeuedSlots;
    for (auto i = 0; i < kMaxDequeuedBuffers; i++)
    {
        // A non-negative value with flags set (see above) will be returned upon
        // success as status.
        //
        // Return of a negative means an error has occurred:
        // * NO_INIT - the buffer queue has been abandoned or the producer is not
        //             connected.
        // * BAD_VALUE - both in async mode and buffer count was less than the
        //               max numbers of buffers that can be allocated at once.
        // * INVALID_OPERATION - cannot attach the buffer because it would cause
        //                       too many buffers to be dequeued, either because
        //                       the producer already has a single buffer dequeued
        //                       and did not set a buffer count, or because a
        //                       buffer count was set and this call would cause
        //                       it to be exceeded.
        // * WOULD_BLOCK - no buffer is currently available, and blocking is disabled
        //                 since both the producer/consumer are controlled by app
        // * NO_MEMORY - out of memory, cannot allocate the graphics buffer.
        // * TIMED_OUT - the timeout set by setDequeueTimeout was exceeded while
        //               waiting for a buffer to become available.
        int32_t dequeuedSlot;
        hgbp->dequeueBuffer(width, height, PixelFormat(format), GetParam(),
                false /* getFrameTimestamps */,
               [&status, &dequeuedSlot](int32_t _status, int32_t slot,
                   const hidl_handle& fence,
                   const HFrameEventHistoryDelta& outTimestamps)
                {
                    (void)outTimestamps;
                    status = _status;
                    EXPECT_GE(_status, 0);

                    CAM_ULOGMD("[dequeueBuffer] status(%d) slot(%d)",
                        _status, slot);

                    if (_status >= 0)
                    {
                        dequeuedSlot = slot;
                        // wait fence (if exist) for 1000 ms
                        int releaseFenceFd = readFdFromNativeHandle(fence);
                        if (releaseFenceFd != kNoFenceFd)
                        {
                            if (sync_wait(releaseFenceFd, 1000) < 0)
                            {
                                CAM_ULOGME("sync_wait() fd(%d) failed: %s",
                                        releaseFenceFd, strerror(errno));
                            }
                        }
                    }
                });

        if (status == ::android::BufferQueueDefs::BUFFER_NEEDS_REALLOCATION)
        {
            // if dequeueBuffer returns the BUFFER_NEEDS_REALLOCATION flag,
            // the client is expected to call requestBuffer immediately.
            AnwBuffer anwBuffer;
            hgbp->requestBuffer(dequeuedSlot, [&status, &anwBuffer](int32_t _status,
                        const AnwBuffer& _buffer)
                    {
                        status = _status;
                        anwBuffer = _buffer;
                    });
            ASSERT_EQ(status, ::android::OK);

            SECHAND secureHandle = 0;
            if ((anwBuffer.attr.usage & AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT)
                    == AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT)
            {
                ASSERT_TRUE(mAImageReaderHelper->getSecureHandle(
                        anwBuffer.nativeHandle, secureHandle));
            }

            PixelFormat pixelFormat;
            ASSERT_TRUE(mAImageReaderHelper->getPixelFormat(
                        anwBuffer.nativeHandle, pixelFormat));

            CAM_ULOGMD("secure handle(0x%x) pixel format(%s) %s",
                    secureHandle, toString(pixelFormat).c_str(),
                    toString(anwBuffer.attr).c_str());
        }
        else if (status == ::android::BufferQueueDefs::RELEASE_ALL_BUFFERS)
        {
            // ff dequeueBuffer returns the RELEASE_ALL_BUFFERS flag, the client is
            // expected to release all of the mirrored slot->buffer mappings.
            // TODO: release all buffers
        }

        // record the dequeues slots
        dequeuedSlots.push_back(dequeuedSlot);
    }

    for (auto dequeuedSlot : dequeuedSlots)
    {
        // cancelBuffer indicates that the client does not wish to fill in the
        // buffer associated with slot and transfers ownership of the slot back to
        // the server.
        Fence dummyFence(createNativeHandleFromFd(kNoFenceFd));
        ASSERT_EQ(hgbp->cancelBuffer(dequeuedSlot, dummyFence), ::android::OK);
    }

    // disconnect attempts to disconnect a client API from the
    // IGraphicBufferProducer.
    // Calling this method will cause any subsequent
    // calls to other IGraphicBufferProducer methods to fail except for
    // getAllocator and connect.
    // Successfully calling connect after this will
    // allow the other methods to succeed again.
    ASSERT_EQ(hgbp->disconnect(
            nwAPI, HGraphicBufferProducer::DisconnectMode::API), ::android::OK);
}
INSTANTIATE_TEST_SUITE_P(GRALLOC, AImageReaderTest,
    ::testing::Values(
        /* default case: inaccessible by secure camera, ION_HEAP_TYPE_MULTIMEDIA_SEC */
        //GRALLOC1_PRODUCER_USAGE_PROTECTED,
        /* ImageReader case: ION_HEAP_TYPE_MULTIMEDIA_PROT */
        GRALLOC1_PRODUCER_USAGE_CAMERA,
        /* ImageWriter case: ION_HEAP_TYPE_MULTIMEDIA_PROT */
        GRALLOC1_CONSUMER_USAGE_CAMERA,
        /* ImageReader and ImageWriter case: ION_HEAP_TYPE_MULTIMEDIA_PROT */
        GRALLOC1_USAGE_PROT,
        /* intra camera HAL3 secure buffer heap: ION_HEAP_TYPE_MULTIMEDIA_2D_FR */
        GRALLOC1_USAGE_SECURE_CAMERA));
        /* alias of GRALLOC1_PRODUCER_USAGE_PROTECTED */
        //GRALLOC1_USAGE_SECURE));

} // namespace tests
} // namespace NSCam

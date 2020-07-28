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

#include <inttypes.h>
#include "TestDefs.h"
#include "utils/TestBase.h"
#include "system/types.h"
#include <gralloc1_mtk_defs.h>
#include <graphics_mtk_defs.h>
#include <android/hardware/graphics/common/1.0/types.h>

#include "graphics/AllocatorHelper.h"
#include "media/AImageReaderHelper.h"

#include <utility>
#include <sstream>

#include <utils/StrongPointer.h>

#include <mtkcam/utils/std/ULog.h>
#include <mtkcam/utils/std/TypeTraits.h>
#include <mtkcam/utils/imgbuf/ISecureImageBufferHeap.h>
#include <mtkcam/utils/imgbuf/IGraphicImageBufferHeap.h>
#include <mtkcam/def/ImageFormat.h>

// ------------------------------------------------------------------------

namespace NSCam {
namespace tests {

CAM_ULOG_DECLARE_MODULE_ID(MOD_CAMERA_HAL_SERVER);

using ::android::sp;
using ::android::hardware::hidl_handle;
using ::android::hardware::graphics::common::V1_0::PixelFormat;
using namespace NSCam;
using ::NSCam::ISecureImageBufferHeap;
using ::NSCam::IGraphicImageBufferHeap;
using SecureBufferTypeAndCameraUsage = std::pair<SecType, int>;

// ------------------------------------------------------------------------

static inline bool _has_usage(uint64_t usage_flag, uint64_t usage)
{
    return (usage & usage_flag) == usage_flag;
}

static inline bool _does_not_have_usage(uint64_t usage_flag, uint64_t usage)
{
    return (usage ^ usage_flag) == usage_flag;
}
// ------------------------------------------------------------------------

struct LiteralConstant
{
    static constexpr int kHWBufferUsage = NSCam::eBUFFER_USAGE_HW_CAMERA_READWRITE;
    static constexpr int kSWBufferUsage =
        NSCam::eBUFFER_USAGE_SW_READ_OFTEN | NSCam::eBUFFER_USAGE_SW_WRITE_OFTEN;
    static constexpr int kHybridBufferUsage = kHWBufferUsage | kSWBufferUsage;
}; // struct LiteralConstant

/**
 * This test fixture is for testing class ISecureImageBufferHeap.
 */
class SecureImageBufferHeapTest : public TestBase,
    public ::testing::WithParamInterface<SecureBufferTypeAndCameraUsage>
{
}; // class SecureImageBufferHeapTest

// ------------------------------------------------------------------------

/**
 * This test fixture is for testing class IGraphicImageBufferHeap.
 */
class GraphicImageBufferHeapTest : public TestBase,
    public ::testing::WithParamInterface<uint64_t>
{
protected:
    AllocatorHelper& mAllocator = AllocatorHelper::getInstance();
}; // class GraphicImageBufferHeapTest

// ------------------------------------------------------------------------

TEST_P(SecureImageBufferHeapTest, LockAndUnlockImageBuffer)
{
    EImageFormat format = eImgFmt_NV12;
    MSize size(640, 480);
    size_t bufStridesInBytes[3] { 1280, 640, 640 };
    size_t bufBoundaryInBytes[3] {};
    size_t planeCount = 2;
    // Protected memory allocation
    SecType secType = GetParam().first;
    const ::testing::TestInfo* testInfo =
        ::testing::UnitTest::GetInstance()->current_test_info();

    IImageBufferAllocator::ImgParam imgParam(
        format, size, bufStridesInBytes, bufBoundaryInBytes, planeCount);

    ISecureImageBufferHeap::AllocExtraParam extraParam(
            0/*nocache*/, 1/*security*/, 0/*coherence*/, false, secType/*SecType*/);

    sp<IImageBufferHeap> bufferHeap =
        ISecureImageBufferHeap::create(testInfo->test_case_name(), imgParam, extraParam);
    ASSERT_TRUE(bufferHeap) << "create image buffer heap failed";

    ImgBufCreator creator(bufferHeap->getImgFormat());
    sp<IImageBuffer> buffer = bufferHeap->createImageBuffer(&creator);

    const auto bufferUsage = GetParam().second;

    EXPECT_TRUE(buffer->lockBuf(testInfo->test_case_name(), bufferUsage));
    {
        std::string caseName;
        const auto fileDescriptor(buffer->getFD(0));
        const auto secureHandleSW(buffer->getBufVA(0));
        const auto secureHandleHW(buffer->getBufPA(0));

        EXPECT_TRUE(fileDescriptor >= 0) << "invalid file descriptor";

        // software usage
        if ((bufferUsage & LiteralConstant::kSWBufferUsage) > 0)
        {
            caseName += caseName.length() == 0 ? "SW" : "/SW";
            EXPECT_TRUE(secureHandleSW != 0) << "invalid secure handle (SW)";
        }
        else
        {
            EXPECT_TRUE(secureHandleSW == 0) << "invalid secure handle (SW)";
        }

        // hardware usage
        if ((bufferUsage & LiteralConstant::kHWBufferUsage) > 0)
        {
            caseName += caseName.length() == 0 ? "HW" : "/HW";
            EXPECT_TRUE(secureHandleHW != 0) << "invalid secure handle (HW)";
        }
        else
        {
            EXPECT_TRUE(secureHandleHW == 0) << "invalid secure handle (HW)";
        }

        CAM_ULOGMD("[%s] FD(%d) secureHandleSW(0x%" PRIxPTR ") "
                "secureHandleHW(0x%" PRIxPTR ")",
                caseName.c_str(), fileDescriptor, secureHandleSW, secureHandleHW);
    }
    EXPECT_TRUE(buffer->unlockBuf(testInfo->test_case_name()));
}
INSTANTIATE_TEST_SUITE_P(BufferHeap, SecureImageBufferHeapTest,
        ::testing::Values(
            std::make_pair(SecType::mem_protected, LiteralConstant::kSWBufferUsage),
            std::make_pair(SecType::mem_protected, LiteralConstant::kHWBufferUsage),
            std::make_pair(SecType::mem_protected, LiteralConstant::kHybridBufferUsage),
            std::make_pair(SecType::mem_secure, LiteralConstant::kSWBufferUsage),
            std::make_pair(SecType::mem_secure, LiteralConstant::kHWBufferUsage),
            std::make_pair(SecType::mem_secure, LiteralConstant::kHybridBufferUsage)));

TEST_P(GraphicImageBufferHeapTest, LockAndUnlockImageBuffer)
{
    // FIXME: convert from gralloc1 to gralloc0 usage
    const auto kBufferUsage = GetParam();
    const auto kSecureType = [&](uint64_t bufferUsage)
    {
        if (_does_not_have_usage(GRALLOC1_PRODUCER_USAGE_PROTECTED, bufferUsage))
            return SecType::mem_normal;

        if (_has_usage(GRALLOC1_PRODUCER_USAGE_PRIVATE_2, bufferUsage))
            return SecType::mem_secure;

        if (_does_not_have_usage(GRALLOC1_CONSUMER_USAGE_CAMERA, bufferUsage) &&
                _does_not_have_usage(GRALLOC1_PRODUCER_USAGE_CAMERA, bufferUsage))
            CAM_ULOGM_FATAL("wrong buffer usage(0x%" PRIx64 ")", bufferUsage);

        return SecType::mem_protected;
    }(kBufferUsage);
    const auto kPixelFormat = PixelFormat::IMPLEMENTATION_DEFINED;
    const MSize kImageSize(TestDefs::kAImageWidth, TestDefs::kAImageHeight);
    // allocate graphics buffer
    hidl_handle hidlHandle;
    mAllocator.allocateGraphicBuffer(
            kImageSize.w, kImageSize.h,
            kBufferUsage, kPixelFormat, &hidlHandle);
    const auto kImageFormat = [&](
            const hidl_handle& bufferHandle,
            const PixelFormat pixelFormat) -> EImageFormat
    {
        PixelFormat tempPixelFormat = pixelFormat;
        // expand implementation-specific or platform-specific pixel format
        if ((pixelFormat ==  PixelFormat::IMPLEMENTATION_DEFINED) ||
            (pixelFormat ==  PixelFormat::YCBCR_420_888))
        {
            CAM_ULOGM_ASSERT(AImageReaderHelper::getPixelFormat(
                        bufferHandle, tempPixelFormat),
                    "get pixel format failed");
        }

        // convert from PixelFormat or HAL pixel format (including vendor defiend)
        // to EImageFormat
        switch (tempPixelFormat)
        {
            case PixelFormat::YCRCB_420_SP:
                return EImageFormat::eImgFmt_NV21;
            case PixelFormat::YV12:
                return EImageFormat::eImgFmt_YV12;
            case PixelFormat::RGBA_8888:
                return EImageFormat::eImgFmt_RGBA8888;
            default:
                // NOTE: graphics_mtk_defs.h defines vendor formats
                //       which are not covered by PixelFormat
                if (static_cast<int32_t>(tempPixelFormat) == int32_t(HAL_PIXEL_FORMAT_NV12))
                    return EImageFormat::eImgFmt_NV12;

                CAM_ULOGM_FATAL("unknown pixel format(%d)", toLiteral(tempPixelFormat));
                return EImageFormat::eImgFmt_UNKNOWN;
        };
    }(hidlHandle, kPixelFormat);

    // create IGraphicImageBufferHeap with a graphics buffer
    std::ostringstream heapName;
    heapName << TestDefs::kAImageWidth << "x" << TestDefs::kAImageHeight
        << "u" << std::showbase << std::hex << kBufferUsage
        << "p" << std::dec << toLiteral(kPixelFormat)
        << toString(hidlHandle);

    buffer_handle_t bufferHandle(hidlHandle);
    sp<IGraphicImageBufferHeap> imageBufferHeap =
        IGraphicImageBufferHeap::create(
                heapName.str().c_str(), kBufferUsage, kImageSize, kImageFormat,
                &bufferHandle, kNoFenceFd, kNoFenceFd, 0, kSecureType);

    SECHAND secureHandle;
    ASSERT_TRUE(AImageReaderHelper::getSecureHandle(hidlHandle, secureHandle));

    // buffer lock/unlock
    const ::testing::TestInfo* testInfo =
        ::testing::UnitTest::GetInstance()->current_test_info();
    EXPECT_TRUE(imageBufferHeap->lockBuf(testInfo->test_case_name()));
    {
        std::string caseName;
        const auto heapID(imageBufferHeap->getHeapID(0));
        const auto secureHandleSW(imageBufferHeap->getBufVA(0));
        const auto secureHandleHW(imageBufferHeap->getBufPA(0));

        EXPECT_TRUE(heapID >= 0) << "invalid file descriptor";
        EXPECT_EQ(secureHandle, secureHandleSW);
        EXPECT_EQ(secureHandle, secureHandleHW);

        CAM_ULOGMD("heapFD(%d) secureHandle(0x%" PRIxPTR ") "
                "secureHandleSW(0x%" PRIxPTR ") "
                "secureHandleHW(0x%" PRIxPTR ")",
                heapID, secureHandle, secureHandleSW, secureHandleHW);
    }
    EXPECT_TRUE(imageBufferHeap->unlockBuf(testInfo->test_case_name()));
}
INSTANTIATE_TEST_SUITE_P(BufferHeap, GraphicImageBufferHeapTest,
    ::testing::Values(
        GRALLOC1_USAGE_PROT_PRODUCER   /* ImageReader case */,
        GRALLOC1_USAGE_PROT_CONSUMER   /* ImageWriter case */,
        GRALLOC1_USAGE_PROT            /* ImageReader and ImageWriter case */,
        GRALLOC1_USAGE_SECURE_CAMERA   /* intra camera HAL3 secure buffer heap */));

} // namespace tests
} // namespace NSCam

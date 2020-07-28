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
#include "graphics/AllocatorHelper.h"

#include <mtkcam/utils/std/ULog.h>

// Define the MTK specific enumeration values for gralloc usage in order to
// avoid "polute" AOSP file
// (hardware/libhardware/include/hardware/gralloc1.h)
// The enumeration value definition must not be conflict with the gralloc1.h
// in original AOSP file
#include <gralloc1_mtk_defs.h>

#include <sync/sync.h>

// ------------------------------------------------------------------------

namespace NSCam {
namespace tests {

CAM_ULOG_DECLARE_MODULE_ID(MOD_CAMERA_HAL_SERVER);

using ::android::hardware::graphics::common::V1_0::BufferUsage;
using ::android::hardware::graphics::common::V1_0::PixelFormat;
using ::android::hardware::hidl_handle;

// ------------------------------------------------------------------------

/**
 * This test fixture is for testing class
 * ::android::hardware::graphics::mapper::V2_0::IMapper and
 * ::android::hardware::graphics::allocator::V2_0::IAllocator
 * with gralloc1's private producer usage.
 */
class MapperAndAllocatorTest : public TestBase,
    public ::testing::WithParamInterface<uint64_t>
{
protected:
    void SetUp() override;
    void TearDown() override;

    static constexpr uint32_t kImageWidth = 640;
    static constexpr uint32_t kImageHeight = 480;

   AllocatorHelper& mAllocator = AllocatorHelper::getInstance();
}; // class MapperAndAllocatorTest

void MapperAndAllocatorTest::SetUp()
{
    TestBase::SetUp();
}

void MapperAndAllocatorTest::TearDown()
{
    TestBase::TearDown();
}

// ------------------------------------------------------------------------

TEST_P(MapperAndAllocatorTest, AllocateWithPrivateUsage)
{
    const auto bufferUsage = GetParam();

    hidl_handle bufferHandle;
    mAllocator.allocateGraphicBuffer(
            kImageWidth, kImageHeight, bufferUsage,
            PixelFormat::IMPLEMENTATION_DEFINED, &bufferHandle);
}
// NOTE: BufferUsage::PROTECTED (a.k.a. GRALLOC1_USAGE_SECURE) is
//       occupied by DRM and can not be used by camera HAL3, so we
//       do not add this buffer usage for testing.
INSTANTIATE_TEST_SUITE_P(GRALLOC, MapperAndAllocatorTest,
    ::testing::Values(
        GRALLOC1_USAGE_PROT_PRODUCER   /* ImageReader case */,
        GRALLOC1_USAGE_PROT_CONSUMER   /* ImageWriter case */,
        GRALLOC1_USAGE_PROT            /* ImageReader and ImageWriter case */,
        GRALLOC1_USAGE_SECURE_CAMERA   /* intra camera HAL3 secure buffer heap */));

} // namespace tests
} // namespace NSCam

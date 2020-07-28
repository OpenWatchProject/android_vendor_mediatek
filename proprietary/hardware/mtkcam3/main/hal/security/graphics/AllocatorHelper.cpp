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

#define LOG_TAG "ACameraHelper"

#include "AllocatorHelper.h"
#include "MapperHelper.h"

#include <mtkcam/utils/std/ULog.h>

// ------------------------------------------------------------------------

namespace NSCam {

CAM_ULOG_DECLARE_MODULE_ID(MOD_CAMERA_HAL_SERVER);

using ::android::sp;
using ::android::hardware::graphics::common::V1_0::PixelFormat;
using ::android::hardware::graphics::allocator::V2_0::IAllocator;
using ::android::hardware::graphics::mapper::V2_0::IMapper;
using ::android::hardware::graphics::mapper::V2_0::BufferDescriptor;
using ::android::hardware::hidl_handle;

// ------------------------------------------------------------------------

AllocatorHelper::AllocatorHelper()
{
    // get IAllocator service
    mAllocator = IAllocator::getService();
    if (!mAllocator)
        CAM_ULOGME("failed to get IAllocator interface");
}

void AllocatorHelper::allocateGraphicBuffer(
        uint32_t width, uint32_t height, uint64_t usage, PixelFormat format,
        hidl_handle* handle, uint32_t* rowStride) const
{
    if (handle == nullptr)
    {
        CAM_ULOGME("invalid hidl handle, do nothing");
        return;
    }

    IMapper::BufferDescriptorInfo descriptorInfo {
            .width = width,
            .height = height,
            /*
             * NOTE: A buffer with multiple layers may be used as
             * the backing store of an array texture.
             * All layers of a buffer share the same characteristics (e.g.,
             * dimensions, format, usage). Devices that do not support
             * GRALLOC1_CAPABILITY_LAYERED_BUFFERS must allocate only buffers
             * with a single layer.
             */
            .layerCount = 1,
            .format = format,
            .usage = usage
        };

    BufferDescriptor descriptor;

    MapperHelper::getInstance().createDescriptor(descriptorInfo, descriptor);
    auto ret = mAllocator->allocate(descriptor, 1u,
        [&](auto err, uint32_t stride, const auto& buffers) {
            CAM_ULOGMD("stride(%u)", stride);
            if (rowStride)
                *rowStride = stride;
            if (err != ::android::hardware::graphics::mapper::V2_0::Error::NONE)
                CAM_ULOGME("IAllocator::allocate() failed(%s)", toString(err).c_str());

            if (buffers.size() != 1u)
                CAM_ULOGME("the amount of buffers is greater than 1");
            *handle = buffers[0];
        });

    if (!ret.isOk())
        CAM_ULOGME("IAllocator::allocate() transaction failed");
}

} // namespace NSCam

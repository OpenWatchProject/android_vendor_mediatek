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

#include "MapperHelper.h"

#include <mtkcam/utils/std/ULog.h>

#include <sync/sync.h>

// ------------------------------------------------------------------------

namespace NSCam {

CAM_ULOG_DECLARE_MODULE_ID(MOD_CAMERA_HAL_SERVER);

using ::android::hardware::graphics::mapper::V2_0::IMapper;
using ::android::hardware::graphics::mapper::V2_0::BufferDescriptor;
using ::android::hardware::graphics::mapper::V2_0::YCbCrLayout;
using ::android::hardware::hidl_handle;

// ------------------------------------------------------------------------

MapperHelper::MapperHelper()
{
    // get IMapper service
    mMapper = IMapper::getService();
    if (!mMapper)
        CAM_ULOGME("failed to get IMapper interface");
}

void MapperHelper::createDescriptor(
        const IMapper::BufferDescriptorInfo& descriptorInfo,
        BufferDescriptor& descriptor)
{
    auto ret = mMapper->createDescriptor(
        descriptorInfo, [&descriptor](const auto& err, const auto& desc) {
            if (err != ::android::hardware::graphics::mapper::V2_0::Error::NONE)
                CAM_ULOGME("IMapper::createDescriptor() failed(%s)", toString(err).c_str());
            descriptor = desc;
        });

    if (!ret.isOk())
        CAM_ULOGME("IMapper::createDescriptor() transaction failed");
}

void MapperHelper::importBuffer(buffer_handle_t& outBufferHandle)
{
    buffer_handle_t importedHandle;
    auto ret = mMapper->importBuffer(hidl_handle(outBufferHandle),
            [&importedHandle](const auto& err, const auto& buffer)
            {
                if (err != ::android::hardware::graphics::mapper::V2_0::Error::NONE)
                    CAM_ULOGME("IMapper::importBuffer() failed(%s)", toString(err).c_str());
                importedHandle = static_cast<buffer_handle_t>(buffer);
            });

    if (!ret.isOk())
        CAM_ULOGME("IMapper::importBuffer() transaction failed");

    outBufferHandle = importedHandle;
}

void MapperHelper::freeBuffer(buffer_handle_t bufferHandle) const
{
    if (!bufferHandle)
        return;

    auto ret = mMapper->freeBuffer(const_cast<native_handle_t*>(bufferHandle));
    if (!ret.isOk())
    {
        CAM_ULOGME("IMapper::freeBuffer() transaction failed: %s",
                toString(ret).c_str());
    }
}

void MapperHelper::lock(buffer_handle_t bufferHandle, uint64_t cpuUsage,
            const IMapper::Rect& accessRegion, int acquireFence, void*& buffer) const
{
    // put acquireFence in a hidl_handle
    hidl_handle acquireFenceHandle;
    NATIVE_HANDLE_DECLARE_STORAGE(acquireFenceStorage, 1, 0);
    if (acquireFence >= 0)
    {
        auto h = native_handle_init(acquireFenceStorage, 1, 0);
        h->data[0] = acquireFence;
        acquireFenceHandle = h;
    }

    auto _bufferHandle = const_cast<native_handle_t*>(bufferHandle);
    auto ret = mMapper->lock(_bufferHandle, cpuUsage, accessRegion, acquireFenceHandle,
            [&](const auto& err, const auto& tmpBuffer)
            {
                if (err != ::android::hardware::graphics::mapper::V2_0::Error::NONE)
                {
                    CAM_ULOGME("IMapper::lock() failed: bufferHandle(0x%" PRIxPTR ")",
                        reinterpret_cast<intptr_t>(_bufferHandle));
                }

                buffer = tmpBuffer;

                CAM_ULOGMD("lock addr(0x%" PRIxPTR ")",
                    reinterpret_cast<intptr_t>(tmpBuffer));
            });

    // we own acquireFence even on errors
    if (acquireFence >= 0)
        close(acquireFence);

    if (!ret.isOk())
        CAM_ULOGME("IMapper::lock() transaction failed");
}

void MapperHelper::lockYCbCr(buffer_handle_t bufferHandle, uint64_t cpuUsage,
        const IMapper::Rect& accessRegion, int acquireFence,
        YCbCrLayout& layout) const
{
    // put acquireFence in a hidl_handle
    hidl_handle acquireFenceHandle;
    NATIVE_HANDLE_DECLARE_STORAGE(acquireFenceStorage, 1, 0);
    if (acquireFence >= 0)
    {
        auto h = native_handle_init(acquireFenceStorage, 1, 0);
        h->data[0] = acquireFence;
        acquireFenceHandle = h;
    }

    auto _bufferHandle = const_cast<native_handle_t*>(bufferHandle);
    auto ret = mMapper->lockYCbCr(_bufferHandle, cpuUsage, accessRegion, acquireFenceHandle,
            [&](const auto& err, const auto& tmpLayout)
            {
                if (err != ::android::hardware::graphics::mapper::V2_0::Error::NONE)
                {
                    CAM_ULOGME("IMapper::lockYCbCr() failed: bufferHandle(0x%" PRIxPTR ")",
                        reinterpret_cast<intptr_t>(_bufferHandle));
                }

                layout = tmpLayout;

                CAM_ULOGMD("YCbCrLayout: %s", toString(tmpLayout).c_str());
            });

    // we own acquireFence even on errors
    if (acquireFence >= 0)
        close(acquireFence);

    if (!ret.isOk())
        CAM_ULOGME("IMapper::lockYCbCr() transaction failed");
}

void MapperHelper::unlock(
        buffer_handle_t bufferHandle, int& releaseFence) const
{
    auto _bufferHandle = const_cast<native_handle_t*>(bufferHandle);
    releaseFence = kNoFenceFd;
    auto ret = mMapper->unlock(_bufferHandle,
            [&](const auto& err, const auto& tmpReleaseFence)
            {
                if (err != ::android::hardware::graphics::mapper::V2_0::Error::NONE)
                    CAM_ULOGME("IMapper::unlock() failed");

                auto fenceHandle = tmpReleaseFence.getNativeHandle();
                if (fenceHandle && fenceHandle->numFds == 1)
                {
                    int fd = dup(fenceHandle->data[0]);
                    if (fd >= 0) {
                        releaseFence = fd;
                    } else {
                        CAM_ULOGMD("failed to dup unlock release fence");
                        int err = sync_wait(fenceHandle->data[0], kTimeOutNever);
                        if (err != ::android::NO_ERROR)
                            CAM_ULOGME("sync_wait() failed: %s", strerror(errno));
                    }
                }
            });

    if (!ret.isOk())
        CAM_ULOGME("IMapper::unlock() transaction failed");
}

} // namespace NSCam

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
/*
**
** Copyright 2009, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/


#define LOG_TAG "MtkCam/GrallocMapper"

#include "mapper.h"
#include "../MyUtils.h"

#include <log/log.h>
#include <sync/sync.h>

using namespace android;
using namespace NSCam;
using namespace NSCam::GrallocMapper;

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)
//

static constexpr Error kTransactionError = Error::NO_RESOURCES;

/******************************************************************************
 *
 ******************************************************************************/
Mapper::Mapper()
{
    mMapper = IMapper::getService();
    if (mMapper != nullptr && mMapper->isRemote()) {
        MY_LOGF("gralloc-mapper must be in passthrough mode");
    }
}

/******************************************************************************
 *
 ******************************************************************************/
Error
Mapper::createDescriptor(
    const IMapper::BufferDescriptorInfo& descriptorInfo,
    BufferDescriptor* outDescriptor
) const
{
    Error error;
    auto ret = mMapper->createDescriptor(descriptorInfo,
            [&](const auto& tmpError, const auto& tmpDescriptor)
            {
                error = tmpError;
                if (error != Error::NONE) {
                    return;
                }

                *outDescriptor = tmpDescriptor;
            });

    return (ret.isOk()) ? error : kTransactionError;
}


/******************************************************************************
 *
 ******************************************************************************/
Error
Mapper::importBuffer(
    const hardware::hidl_handle& rawHandle,
    buffer_handle_t* outBufferHandle
) const
{
    Error error;
    auto ret = mMapper->importBuffer(rawHandle,
            [&](const auto& tmpError, const auto& tmpBuffer)
            {
                error = tmpError;
                if (error != Error::NONE) {
                    return;
                }

                *outBufferHandle = static_cast<buffer_handle_t>(tmpBuffer);
            });

    return (ret.isOk()) ? error : kTransactionError;
}


/******************************************************************************
 *
 ******************************************************************************/
void
Mapper::freeBuffer(buffer_handle_t bufferHandle) const
{
    auto buffer = const_cast<native_handle_t*>(bufferHandle);
    auto ret = mMapper->freeBuffer(buffer);

    auto error = (ret.isOk()) ? static_cast<Error>(ret) : kTransactionError;
    MY_LOGE_IF(error != Error::NONE, "freeBuffer(%p) failed with %d",
            buffer, error);
}


/******************************************************************************
 *
 ******************************************************************************/
Error
Mapper::lock(
    buffer_handle_t bufferHandle, uint64_t usage,
    const IMapper::Rect& accessRegion,
    int acquireFence, void** outData
) const
{
    auto buffer = const_cast<native_handle_t*>(bufferHandle);

    // put acquireFence in a hidl_handle
    hardware::hidl_handle acquireFenceHandle;
    NATIVE_HANDLE_DECLARE_STORAGE(acquireFenceStorage, 1, 0);
    if (acquireFence >= 0) {
        auto h = native_handle_init(acquireFenceStorage, 1, 0);
        h->data[0] = acquireFence;
        acquireFenceHandle = h;
    }

    Error error;
    auto ret = mMapper->lock(buffer, usage, accessRegion, acquireFenceHandle,
            [&](const auto& tmpError, const auto& tmpData)
            {
                error = tmpError;
                if (error != Error::NONE) {
                    return;
                }

                *outData = tmpData;
            });

    // we own acquireFence even on errors
    if (acquireFence >= 0) {
        close(acquireFence);
    }

    return (ret.isOk()) ? error : kTransactionError;
}



/******************************************************************************
 *
 ******************************************************************************/
Error
Mapper::lock(
    buffer_handle_t bufferHandle, uint64_t usage,
    const IMapper::Rect& accessRegion,
    int acquireFence, YCbCrLayout* outLayout
) const
{
    auto buffer = const_cast<native_handle_t*>(bufferHandle);

    // put acquireFence in a hidl_handle
    hardware::hidl_handle acquireFenceHandle;
    NATIVE_HANDLE_DECLARE_STORAGE(acquireFenceStorage, 1, 0);
    if (acquireFence >= 0) {
        auto h = native_handle_init(acquireFenceStorage, 1, 0);
        h->data[0] = acquireFence;
        acquireFenceHandle = h;
    }

    Error error;
    auto ret = mMapper->lockYCbCr(buffer, usage, accessRegion,
            acquireFenceHandle,
            [&](const auto& tmpError, const auto& tmpLayout)
            {
                error = tmpError;
                if (error != Error::NONE) {
                    return;
                }

                *outLayout = tmpLayout;
            });

    // we own acquireFence even on errors
    if (acquireFence >= 0) {
        close(acquireFence);
    }

    return (ret.isOk()) ? error : kTransactionError;
}


/******************************************************************************
 *
 ******************************************************************************/
int
Mapper::unlock(buffer_handle_t bufferHandle) const
{
    auto buffer = const_cast<native_handle_t*>(bufferHandle);

    int releaseFence = -1;
    Error error;
    auto ret = mMapper->unlock(buffer,
            [&](const auto& tmpError, const auto& tmpReleaseFence)
            {
                error = tmpError;
                if (error != Error::NONE) {
                    return;
                }

                auto fenceHandle = tmpReleaseFence.getNativeHandle();
                if (fenceHandle && fenceHandle->numFds == 1) {
                    int fd = dup(fenceHandle->data[0]);
                    if (fd >= 0) {
                        releaseFence = fd;
                    } else {
                        MY_LOGD("failed to dup unlock release fence");
                        ::sync_wait(fenceHandle->data[0], -1);
                    }
                }
            });

    if (!ret.isOk()) {
        error = kTransactionError;
    }

    if (error != Error::NONE) {
        MY_LOGE("unlock(%p) failed with %d", buffer, error);
    }

    return releaseFence;
}

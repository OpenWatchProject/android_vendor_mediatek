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


#define LOG_TAG "MtkCam/GrallocAllocator"

#include "allocator.h"
#include "../MyUtils.h"

#include <log/log.h>
#include <sync/sync.h>

using namespace android;
using namespace NSCam;
using namespace NSCam::GrallocAllocator;
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
Allocator::Allocator(const Mapper& mapper)
    : mMapper(mapper)
{
    mAllocator = IAllocator::getService();
    if (mAllocator == nullptr) {
        MY_LOGF("gralloc-alloc is missing ");
    }
}

/******************************************************************************
 *
 ******************************************************************************/
Error
Allocator::allocate(
    BufferDescriptor descriptor, uint32_t count,
    uint32_t* outStride, buffer_handle_t* outBufferHandles
) const
{
    Error error;
    auto ret = mAllocator->allocate(descriptor, count,
            [&](const auto& tmpError, const auto& tmpStride,
                const auto& tmpBuffers) {
                error = tmpError;
                if (tmpError != Error::NONE) {
                    return;
                }

                // import buffers
                for (uint32_t i = 0; i < count; i++) {
                    error = mMapper.importBuffer(tmpBuffers[i],
                            &outBufferHandles[i]);
                    if (error != Error::NONE) {
                        for (uint32_t j = 0; j < i; j++) {
                            mMapper.freeBuffer(outBufferHandles[j]);
                            outBufferHandles[j] = nullptr;
                        }
                        return;
                    }
                }

                *outStride = tmpStride;
            });

    return (ret.isOk()) ? error : kTransactionError;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
Allocator::allocate(
    uint32_t width, uint32_t height, PixelFormat format,
    uint32_t layerCount, uint64_t usage,
    buffer_handle_t* handle, uint32_t* stride, uint64_t /*graphicBufferId*/,
    std::string /*requestorName*/
)  const
{
    MY_LOGD("allocate: %dx%d format(%#x) layerCount(%d) usage(%#" PRIx64 ") handle(%p) stride(%p)",
            width, height, format, layerCount, usage, handle, stride);
    // make sure to not allocate a N x 0 or 0 x N buffer, since this is
    // allowed from an API stand-point allocate a 1x1 buffer instead.
    if (!width || !height)
        width = height = 1;

    // Ensure that layerCount is valid.
    if (layerCount < 1)
        layerCount = 1;

    IMapper::BufferDescriptorInfo info =
    {
        .width      = width,
        .height     = height,
        .layerCount = layerCount,
        .format     = static_cast<PixelFormat>(format),
        .usage      = usage,
    };

    BufferDescriptor descriptor;
    Error error = mMapper.createDescriptor(info, &descriptor);
    if ( error != Error::NONE )
    {
        MY_LOGE("Failed to createDescriptor");
        return NO_MEMORY;
    }
    uint32_t count = 1;

    error = allocate(descriptor, count, stride, handle);
    if ( error != Error::NONE ) {
        MY_LOGE("Failed to allocate (%u x %u) layerCount %u format %d "
                "usage %" PRIx64 ": %d",
                width, height, layerCount, format, usage,
                error);
        return NO_MEMORY;
    }
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
Allocator::free(buffer_handle_t handle)  const
{
    mMapper.freeBuffer(handle);
    return OK;
}
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

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_GRALLOC_MAPPER_MAPPER_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_GRALLOC_MAPPER_MAPPER_H_

#include <string>

#include <android/hardware/graphics/mapper/2.0/IMapper.h>
#include <system/window.h>
#include <utils/StrongPointer.h>

namespace NSCam {
namespace GrallocMapper {

using android::hardware::graphics::mapper::V2_0::Error;
using android::hardware::graphics::mapper::V2_0::IMapper;
using android::hardware::graphics::mapper::V2_0::YCbCrLayout;
using android::hardware::graphics::mapper::V2_0::BufferDescriptor;

class Mapper {
public:
    Mapper();

    Error createDescriptor(
            const IMapper::BufferDescriptorInfo& descriptorInfo,
            BufferDescriptor* outDescriptor) const;

    // The returned handle must be freed with freeBuffer.
    Error importBuffer(const android::hardware::hidl_handle& rawHandle,
            buffer_handle_t* outBufferHandle) const;

    void freeBuffer(buffer_handle_t bufferHandle) const;

    // The ownership of acquireFence is always transferred to the callee, even
    // on errors.
    Error lock(buffer_handle_t bufferHandle, uint64_t usage,
            const IMapper::Rect& accessRegion,
            int acquireFence, void** outData) const;

    // The ownership of acquireFence is always transferred to the callee, even
    // on errors.
    Error lock(buffer_handle_t bufferHandle, uint64_t usage,
            const IMapper::Rect& accessRegion,
            int acquireFence, YCbCrLayout* outLayout) const;

    // unlock returns a fence sync object (or -1) and the fence sync object is
    // owned by the caller
    int unlock(buffer_handle_t bufferHandle) const;

private:
    android::sp<IMapper> mMapper;
};

} // namespace GrallocMapper
} // namespace NSCam

#endif // _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_GRALLOC_MAPPER_MAPPER_H_

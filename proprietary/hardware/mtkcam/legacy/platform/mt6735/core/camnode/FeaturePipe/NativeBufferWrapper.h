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
 * MediaTek Inc. (C) 2017. All rights reserved.
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

#ifndef _MTK_PLATFORM_HARDWARE_CORE_FEATUREIO_PIPE_NATIVEBUFFERWRAPPER_H_
#define _MTK_PLATFORM_HARDWARE_CORE_FEATUREIO_PIPE_NATIVEBUFFERWRAPPER_H_

#define SUPPORT_AHARDWAREBUFFER 1


#include <string>
#include <utils/RefBase.h>

#if SUPPORT_AHARDWAREBUFFER
#include <vndk/hardware_buffer.h>
#else
#include <ui/GraphicBuffer.h>
#endif


namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

#if SUPPORT_AHARDWAREBUFFER
typedef AHardwareBuffer* NativeBuffer_PTR;
typedef NativeBuffer_PTR Warp_SPTR;
#else
typedef android::sp<android::GraphicBuffer> NativeBuffer_PTR;
typedef NativeBuffer_PTR* Warp_SPTR;
#endif

class NativeBufferWrapper: public android::RefBase
{
public:
    static const MUINT64 USAGE_HW_TEXTURE;
    static const MUINT64 USAGE_HW_RENDER;
    static const MUINT64 USAGE_SW;

    NativeBufferWrapper(NativeBuffer_PTR buffer, const std::string &name = "Unknown");
    NativeBufferWrapper(MUINT32 width, MUINT32 height, android_pixel_format_t format, MUINT64 usage, const std::string &name = "Unknown");
    virtual ~NativeBufferWrapper();

public:
    buffer_handle_t getHandle();
    NativeBuffer_PTR getBuffer();
    Warp_SPTR getWarp_SPTR();
    MBOOL lock(MUINT64 usage, void** vaddr);
    MBOOL unlock();

#if SUPPORT_AHARDWAREBUFFER
    MUINT32 toNativeFormat(android_pixel_format_t format);
#endif

public:
    static MBOOL lock(Warp_SPTR buffer, MUINT64 usage, void** vaddr);
    static MBOOL unlock(Warp_SPTR buffer);

private:
    MBOOL allocate(MUINT32 width, MUINT32 height, android_pixel_format_t format, MUINT64 usage);

private:
    const std::string mName;
    NativeBuffer_PTR mBuffer;
};



}; // namespace NSFeaturePipe
}; // namespace NSCamFeature
}; // namespace NSCam
#endif  //_MTK_PLATFORM_HARDWARE_CORE_FEATUREIO_PIPE_NATIVEBUFFERWRAPPER_H_

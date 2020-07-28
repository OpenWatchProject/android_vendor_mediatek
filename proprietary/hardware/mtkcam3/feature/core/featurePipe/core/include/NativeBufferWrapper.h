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

#ifndef _MTK_CAMERA_FEATURE_PIPE_NATIVEBUFFER_WRAPPER_H_
#define _MTK_CAMERA_FEATURE_PIPE_NATIVEBUFFER_WRAPPER_H_


#include <string>
#include <utils/RefBase.h>


#include <vndk/hardware_buffer.h>

#include "MtkHeader.h"

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

typedef AHardwareBuffer* NB_PTR;
typedef NB_PTR NB_SPTR;
typedef AHardwareBuffer NativeBuffer;

class NativeBufferWrapper: public android::RefBase
{
public:
    enum ColorSpace
    {
        NOT_SET,
        YUV_BT601_NARROW,
        YUV_BT601_FULL,
        YUV_BT709_NARROW,
        YUV_BT709_FULL,
        YUV_BT2020_NARROW,
        YUV_BT2020_FULL,
    };

    static const MUINT64 USAGE_HW_TEXTURE;
    static const MUINT64 USAGE_HW_RENDER;
    static const MUINT64 USAGE_SW;

    NativeBufferWrapper(NB_PTR buffer, const std::string &name = "Unknown");
    NativeBufferWrapper(MUINT32 width, MUINT32 height, android_pixel_format_t format, MUINT64 usage, const std::string &name = "Unknown");
    virtual ~NativeBufferWrapper();

public:
    buffer_handle_t getHandle();
    NB_PTR getBuffer();
    NB_SPTR getBuffer_SPTR();
    MBOOL lock(MUINT64 usage, void** vaddr);
    MBOOL unlock();
    MBOOL setGrallocExtraParam(ColorSpace color);
    MUINT32 toNativeFormat(android_pixel_format_t format);
    MUINT32 toGrallocExtraColor(ColorSpace color);

private:
    MBOOL allocate(MUINT32 width, MUINT32 height, android_pixel_format_t format, MUINT64 usage);
    static MUINT32 getLogLevel();

private:
    const std::string mName;
    NB_PTR mBuffer;
};

NB_PTR getNativeBuffer(NB_SPTR buffer);
NativeBuffer* getNativeBufferPtr(NB_SPTR buffer);
MBOOL lockNativeBuffer(NB_SPTR buffer, MUINT64 usage, void** vaddr);
MBOOL unlockNativeBuffer(NB_SPTR buffer);

}; // namespace NSFeaturePipe
}; // namespace NSCamFeature
}; // namespace NSCam
#endif  //_MTK_CAMERA_FEATURE_PIPE_NATIVEBUFFER_WRAPPER_H_

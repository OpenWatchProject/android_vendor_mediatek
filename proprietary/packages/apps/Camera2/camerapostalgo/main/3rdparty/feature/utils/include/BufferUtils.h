/*
 *   Copyright Statement:
 *
 *     This software/firmware and related documentation ("MediaTek Software") are
 *     protected under relevant copyright laws. The information contained herein is
 *     confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 *     the prior written permission of MediaTek inc. and/or its licensors, any
 *     reproduction, modification, use or disclosure of MediaTek Software, and
 *     information contained herein, in whole or in part, shall be strictly
 *     prohibited.
 *
 *     MediaTek Inc. (C) 2019. All rights reserved.
 *
 *     BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *    THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *     RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 *     ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 *     WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 *     WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 *     NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 *     RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 *     TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 *     RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 *     OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 *     SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 *     RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 *     STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 *     ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 *     RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 *     MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 *     CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     The following software/firmware and/or related documentation ("MediaTek
 *     Software") have been modified by MediaTek Inc. All revisions are subject to
 *     any receiver's applicable license agreements with MediaTek Inc.
 */
#ifndef BUFFERUTILS_H
#define BUFFERUTILS_H

#include <vndk/hardware_buffer.h>
#include <system/graphics-base-v1.0.h>
#include <vendor/mediatek/hardware/mms/1.2/IMms.h>
#include "utils/imgbuf/IImageBuffer.h"

using ::vendor::mediatek::hardware::mms::V1_2::IMms;
using ::vendor::mediatek::hardware::mms::V1_0::HwCopybitParam;
using android::hardware::hidl_handle;
using android::sp;
using NSCam::IImageBuffer;
using NSCam::MSize;

class BufferUtils {
public:
    static AHardwareBuffer* getAHWBuffer(IImageBuffer *buf);
    static hidl_handle getHidlHandle(IImageBuffer *buf);
    static IImageBuffer* acquireWorkingBuffer(MSize imgSize, MINT32 imgfmt);
    static void deallocBuffer(IImageBuffer* pBuf);
    static int mdpCopyBit(const HwCopybitParam* copybitParam);
    static int mdpResizeBuffer(IImageBuffer* inBuffer,
        MINT32 outputW, MINT32 outputH, IImageBuffer* outBuffer);
    static int mdpResizeAndConvert(IImageBuffer* inBuffer, IImageBuffer* outBuffer);
    static int mdpResizeAndConvert(IImageBuffer* inBuffer, IImageBuffer* outBuffer,
        int rotationDegree);
    static int mdpResizeAndConvert(IImageBuffer* inBuffer, IImageBuffer* outBuffer,
        int rotationDegree,bool mirror);
    static IImageBuffer* mdpConvertWithoutOutputBuffer(IImageBuffer* inBuffer, MINT32 outFormat);
    static void dumpBuffer(IImageBuffer* buffer, char* fileNamePrefix);
    static void dumpBufferX(unsigned char* buffer, char* fileNamePrefix, int size);
    static int32_t toPixelFormat(NSCam::EImageFormat fmt);
private:
    static sp<IMms> mIMmsService;
};
#endif // BUFFERUTILS_H

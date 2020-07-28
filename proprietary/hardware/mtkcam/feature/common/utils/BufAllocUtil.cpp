/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

#define LOG_TAG "MtkCam/BufAllocUtil"

#include <utils/StrongPointer.h>
#include "BufAllocUtil.h"
#include "common.h"
#include <mtkcam/utils/std/common.h>
#include <mtkcam/utils/imgbuf/IIonImageBufferHeap.h>

using namespace NSCam;
using namespace NSCam::Utils;
using namespace NSCam::Utils::Format;

// ---------------------------------------------------------------------------

ANDROID_SINGLETON_STATIC_INSTANCE(BufAllocUtil);

IImageBuffer*
BufAllocUtil::
allocMem(char const *name, MUINT32 fmt, MUINT32 w, MUINT32 h, MBOOL isContinous)
{
    CAM_LOGD("[%s], (%s) Start Alloc Mem, fmt=%d, w=%d, h=%d", __FUNCTION__, name, fmt, w, h);

    IImageBuffer* pBuf;
    if( fmt != eImgFmt_JPEG )
    {
        // Calculate related non-jpg parameter

        MUINT32 plane = queryPlaneCount(fmt);
        size_t bufStridesInBytes[3] = {0};
        size_t bufBoundaryInBytes[3] = {0, 0, 0};
        size_t bufSize = 0;

        for (MUINT32 i = 0; i < plane; i++)
        {
            bufStridesInBytes[i] = queryPlaneWidthInPixels(fmt,i, w) * queryPlaneBitsPerPixel(fmt,i) / 8;
            bufSize += (queryPlaneWidthInPixels(fmt,i, w) * queryPlaneBitsPerPixel(fmt,i) / 8) * queryPlaneHeightInPixels(fmt, i, h);
        }

        if( isContinous )
        {
            // create BLOB imageBufferHeap first
            IImageBufferAllocator::ImgParam blobParam(bufSize, 0);
            android::sp<IImageBufferHeap> pHeap = IIonImageBufferHeap::create(name, blobParam);

            // create Image Buffer
            pBuf = pHeap->createImageBuffer_FromBlobHeap(0, fmt, MSize(w, h), bufStridesInBytes);
            pBuf->incStrong(pBuf);
        }
        else
        {
            IImageBufferAllocator::ImgParam imgParam(
                fmt,
                MSize(w,h),
                bufStridesInBytes,
                bufBoundaryInBytes,
                plane
                );

            pBuf = (IImageBufferAllocator::getInstance())->alloc_ion(name, imgParam);
        }

    }
    else
    {
        // create JPG Image Buffer
        MINT32 bufBoundaryInBytes = 0;
        IImageBufferAllocator::ImgParam imgParam(
                MSize(w,h),
                w * h * 6 / 5,  //FIXME
                bufBoundaryInBytes
                );

        pBuf = (IImageBufferAllocator::getInstance())->alloc_ion(name, imgParam);
    }

    if (!pBuf || !pBuf->lockBuf( LOG_TAG, eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_OFTEN ) )
    {
        CAM_LOGD("Null allocated or lock Buffer failed, name=%s", name);
        return  NULL;
    }

    pBuf->syncCache(eCACHECTRL_INVALID);
    CAM_LOGD("[%s], (%s) Alloc Mem over, fmt=%d, w=%d, h=%d", __FUNCTION__, name, fmt, w, h);

    return pBuf;

}

void
BufAllocUtil::
deAllocMem(char const *name, IImageBuffer *pBuf)
{
    CAM_LOGD("[%s], (%s) DeAlloc Mem", __FUNCTION__, name);
    pBuf->unlockBuf(name);
    (IImageBufferAllocator::getInstance())->free(pBuf);
    CAM_LOGD("[%s], (%s) DeAlloc Mem over", __FUNCTION__, name);
}

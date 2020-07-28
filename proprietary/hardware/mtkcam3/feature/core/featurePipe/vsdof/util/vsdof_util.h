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
#ifndef _MTK_CAMERA_FEATURE_PIPE_VSDOF_UTIL_H_
#define _MTK_CAMERA_FEATURE_PIPE_VSDOF_UTIL_H_
//
#include <DpDataType.h>
#include <DpBlitStream.h>
#include <utils/RefBase.h>

#include <mtkcam/utils/imgbuf/IImageBuffer.h>
//
using namespace NSCam;

class DpBlitStream;
class DpIspStream;

namespace VSDOF{
namespace util{
    struct sMDP_Config
    {
        DpBlitStream* pDpStream = nullptr;
        IImageBuffer* pSrcBuffer = nullptr;
        IImageBuffer* pDstBuffer = nullptr;
        MINT32 rotAngle = 0;
        DpPqParam* pDpPqParam = nullptr;

        // These 2 size are used for special request that the
        // allocated buffer size is bigger than actual content size
        MBOOL useSrcCustomizedStride = MFALSE;
        MSize customizedSrcSize = MSize(0, 0);

        MBOOL useDstCustomizedStride = MFALSE;
        MSize customizedDstSize = MSize(0, 0);
    };

    struct sMDP_extConfig
    {
        MINT32 srcBufSize = 0;
        MINT32 dstBufSize = 0;
        MSize srcSize = MSize(0,0);
        MSize dstSize = MSize(0,0);
        MINT32 srcOffset = 0;
        MINT32 dstOffset = 0;
        MINT32 srcStride = 0;
        MINT32 dstStride = 0;
    };

    DpColorFormat getDpColorFormat(
                        MINT32 format);
    MBOOL excuteMDP(
                        sMDP_Config config);
    MBOOL
    calcDstImageCrop(
                        MSize const &srcSize,
                        MSize const &dstSize,
                        MSize &result,
                        MPoint &retStartPoint);

    MBOOL excuteMDPWithCrop(
                        sMDP_Config config);

    MBOOL excuteMDPBayer12(
                        sMDP_Config config);

    // This is a special MDP process which aim to seperate 1 single
    // MDP process to many sub process.
    // Use srcOffset and dst offset to assign the correct start point
    MBOOL excuteMDPSubProcess( sMDP_Config config, sMDP_extConfig extConfig );

    android::sp<IImageBuffer> createEmptyImageBuffer(
                        const IImageBufferAllocator::ImgParam imgParam,
                        const char* name,
                        MINT usage);

    // DpIspStream has special crop sequence:
    //              SrcCrop sequence:  Resize -> crop
    struct sDpIspConfig
    {
        DpIspStream* pDpIspStream = nullptr;
        IImageBuffer* pSrcBuffer = nullptr;
        IImageBuffer* pDstBuffer = nullptr;
        MINT32 rotAngle = 0;
        DpPqParam* pDpPqParam = nullptr;
    };

    MBOOL excuteDpIspStream(
            sDpIspConfig config);
}
}

#endif // _MTK_CAMERA_FEATURE_PIPE_VSDOF_UTIL_H_

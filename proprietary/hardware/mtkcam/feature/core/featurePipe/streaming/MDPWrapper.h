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

#ifndef _MTK_CAMERA_FEATURE_PIPE_MDP_WRAPPER_H_
#define _MTK_CAMERA_FEATURE_PIPE_MDP_WRAPPER_H_

#include "MtkHeader.h"
#include <vector>
#include <utility>

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

class MDPWrapper
{
public:
    typedef NSCam::NSIoPipe::Output Output;
    typedef NSCam::NSIoPipe::MCropRect MCropRect;
    struct MDPOutput {
        Output      mOutput;
        MCropRect   mCropRect;
        MRectF      mCropF;
        DpPqParam   mPqParam;
        MDPOutput(){}
        MDPOutput(Output output, MCropRect cropRect):mOutput(output), mCropRect(cropRect)
        {
            mCropF = MRectF(MPointF(cropRect.p_integral.x, cropRect.p_integral.y), MSizeF(cropRect.s.w,cropRect.s.h) );
        }
        MDPOutput(Output output, MCropRect cropRect, DpPqParam pqParam):mOutput(output),mCropRect(cropRect),mPqParam(pqParam)
        {
            mCropF = MRectF(MPointF(cropRect.p_integral.x, cropRect.p_integral.y), MSizeF(cropRect.s.w,cropRect.s.h) );
        }
        MDPOutput(Output output, MCropRect cropRect, DpPqParam pqParam, MRectF cropF):mOutput(output),mCropRect(cropRect),mCropF(cropF),mPqParam(pqParam) {}
    };
    typedef std::vector<MDPOutput> OUTPUT_ARRAY;

    static MBOOL toDpColorFormat(NSCam::EImageFormat fmt, DpColorFormat &dpFmt);
    static MBOOL toDpColorFormat(MINT fmt, DpColorFormat &dpFmt);
    static MBOOL toDpTransform(MUINT32 transform, MUINT32 &rotation, MUINT32 &flip);
    static MBOOL isRecordPort(const Output &output);

    MDPWrapper();
    MBOOL process(IImageBuffer *src, const OUTPUT_ARRAY &dst);

private:
    MBOOL prepareSubDst(const OUTPUT_ARRAY &dst, OUTPUT_ARRAY::const_iterator &start, MUINT32 count, OUTPUT_ARRAY &subDst);
    MBOOL prepareMDPSrc(IImageBuffer *src);
    MBOOL prepareMDPDst(const OUTPUT_ARRAY &dst);
    MBOOL prepareMDPOut(MUINT32 outSize);
    MBOOL queueMDPBuffer(MINT32 port, const IImageBuffer *buffer);
    MBOOL prepareMDPDstConfig(MUINT32 index, const Output &output, MUINT32 &rotate, MUINT32 &flip, DpColorFormat &colorFormat, DP_PROFILE_ENUM &profile);
    MVOID dumpInfo(IImageBuffer *src, const OUTPUT_ARRAY &dst);

private:
    DpIspStream mDpIspStream;
    const MUINT32 mMaxMDPDsts;
};

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam

#endif // _MTK_CAMERA_FEATURE_PIPE_MDP_WRAPPER_H_

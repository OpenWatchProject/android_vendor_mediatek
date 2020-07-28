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
#include <mtkcam3/feature/utils/p2/P2IOClassfier.h>
#include <vector>
#include <utility>
#include <queue>

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

class MDPWrapper
{
public:
    using P2IO = NSCam::Feature::P2Util::P2IO;
    typedef NSCam::NSIoPipe::Output Output;
    typedef NSCam::NSIoPipe::MCropRect MCropRect;
    struct MDPOutput {
        Output      mOutput;
        MCropRect   mCropRect;
        DpPqParam   mPqParam;
        MBOOL       mDummy = MFALSE;
        MDPOutput(){}
        MDPOutput(Output output, MCropRect cropRect):mOutput(output), mCropRect(cropRect){}
        MDPOutput(Output output, MCropRect cropRect, DpPqParam pqParam):mOutput(output),mCropRect(cropRect),mPqParam(pqParam){}

    };
    typedef std::vector<MDPOutput> OUTPUT_ARRAY;
    typedef std::vector<P2IO> P2IO_OUTPUT_ARRAY;

    static MBOOL toDpColorFormat(NSCam::EImageFormat fmt, DpColorFormat &dpFmt);
    static MBOOL toDpColorFormat(MINT fmt, DpColorFormat &dpFmt);
    static MBOOL toDpTransform(MUINT32 transform, MUINT32 &rotation, MUINT32 &flip);
    static MBOOL isRecordPort(const Output &output);
    static MBOOL findUnusedMDPPort(const OUTPUT_ARRAY &outputs, unsigned &index);

    MDPWrapper(const std::string& name = "p2s");
    MUINT32 getNumOutPort() const;
    MBOOL process(IImageBuffer *src, const OUTPUT_ARRAY &dst, MBOOL printIO=MFALSE, MUINT32 cycleTimeMs = 0);
    MBOOL process(IImageBuffer *src, const P2IO_OUTPUT_ARRAY &dst, MBOOL printIO=MFALSE, MUINT32 cycleTimeMs = 0);
    MBOOL process(IImageBuffer *src, IImageBuffer *dst, MBOOL printIO=MFALSE, MUINT32 cycleTimeMs = 0);

    MVOID generateOutArray(const P2IO_OUTPUT_ARRAY &inList, OUTPUT_ARRAY &outList, MBOOL allowDummy = MFALSE); // public for test

private:
    MBOOL prepareSubDst(const OUTPUT_ARRAY &dst, OUTPUT_ARRAY::const_iterator &start, MUINT32 count, OUTPUT_ARRAY &subDst);
    MBOOL prepareMDPSrc(IImageBuffer *src);
    MBOOL prepareMDPDst(const OUTPUT_ARRAY &dst);
    MBOOL prepareMDPOut(MUINT32 outSize, MUINT32 cycleTimeMs);
    MBOOL queueMDPBuffer(MINT32 port, const IImageBuffer *buffer);
    MBOOL prepareMDPDstConfig(MUINT32 index, const Output &output, MUINT32 &rotate, MUINT32 &flip, DpColorFormat &colorFormat, DP_PROFILE_ENUM &profile);
    MVOID dumpInfo(IImageBuffer *src, const OUTPUT_ARRAY &dst, MUINT32 cycleTimeMs);

    MVOID pushMDPOut(OUTPUT_ARRAY &outArray, NSIoPipe::PortID &portID, const P2IO &out);
    MVOID collectOneRun(NSCam::Feature::P2Util::P2IOClassfier &collect, OUTPUT_ARRAY &outArray, MBOOL allowDummy);

private:
    DpIspStream mDpIspStream;
    const MUINT32 mMaxMDPDsts;
    const std::string mName;
};

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam

#endif // _MTK_CAMERA_FEATURE_PIPE_MDP_WRAPPER_H_

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

#ifndef _MTK_CAMERA_STREAMING_FEATURE_PIPE_WARP_NODE_H_
#define _MTK_CAMERA_STREAMING_FEATURE_PIPE_WARP_NODE_H_

#include "StreamingFeatureNode.h"
#include "WarpStreamBase.h"

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

class WarpEnqueData
{
public:
    RequestPtr mRequest;
    ImgBuffer mSrc;
    ImgBuffer mDst;
    ImgBuffer mWarp;
    BasicImg mWarpMap;
    MBOOL mNeedDump = MFALSE;
};

class WarpSrcInfo
{
public:
    MUINT32 mID = 0;
    MBOOL mValid = MFALSE;
    MBOOL mNeedWarp = MFALSE;
    BasicImg mInput;
    MSizeF mWarpDomain;
    MRectF mCrop;
};

class WarpDstInfo
{
public:
    MUINT32 mID = 0;
    MBOOL mNeedTempOut = MFALSE;
    IImageBuffer *mOutputBuffer = NULL;
};

class WarpNode : public virtual StreamingFeatureNode, public virtual WarpStreamBase<WarpEnqueData>
{
public:
    WarpNode(const char *name);
    virtual ~WarpNode();

    MVOID setInputBufferPool(const android::sp<IBufferPool> &pool);
    MVOID setOutputBufferPool(const android::sp<IBufferPool> &pool);
    MVOID clearTSQ();

public:
    virtual MBOOL onData(DataID id, const BasicImgData &data);

protected:
    virtual MBOOL onInit();
    virtual MBOOL onUninit();
    virtual MBOOL onThreadStart();
    virtual MBOOL onThreadStop();
    virtual MBOOL onThreadLoop();

private:
    MVOID tryAllocateExtraInBuffer();
    MBOOL processWarp(const RequestPtr &request, const BasicImg &inputImg, const BasicImg &warpMap);
    ImgBuffer prepareOutBuffer(const RequestPtr &request, WarpParam &param);
    MBOOL needTempOutBuffer(const RequestPtr &request);
    MSizeF getWarpOutSize(const RequestPtr &request);
    MVOID applyDZ(const RequestPtr &request, WarpParam &param);
    MVOID enqueWarpStream(const WarpParam &param, const WarpEnqueData &data);
    virtual MVOID onWarpStreamBaseCB(const WarpParam &param, const WarpEnqueData &data);
    MVOID handleResultData(const RequestPtr &request);
    MVOID dump(const WarpEnqueData &data, const WarpParam::OUTPUT_ARRAY &out) const;

    ImgBuffer prepareOutBuffer(const WarpSrcInfo &src, const WarpDstInfo &dst);
    WarpSrcInfo prepareSrc(const RequestPtr &request, const BasicImg &inputImg);
    WarpSrcInfo extractSrc(const RequestPtr &request, const BasicImg &inputImg);
    WarpDstInfo prepareDst(const RequestPtr &request);
    WarpParam prepareWarpParam(const RequestPtr &request, const WarpSrcInfo &src, WarpDstInfo &dst, const BasicImg &warpMap, MBOOL bypass = MFALSE);
    WarpParam::OUTPUT_ARRAY prepareMDPParam(const RequestPtr &request, const WarpSrcInfo &src, MSizeF domainOffset);
    MRectF prepareMDPCrop(const WarpSrcInfo &src, const MSizeF &domainOffset);

    static MSize toWarpOutSize(const MSizeF &inSize);

private:
    WarpStream *mWarpStream;
    WaitQueue<BasicImgData> mWarpMapDatas;
    WaitQueue<BasicImgData> mInputImgDatas;
    android::sp<IBufferPool> mInputBufferPool;
    android::sp<IBufferPool> mOutputBufferPool;

    MUINT32 mExtraInBufferNeeded;

    std::deque<WarpSrcInfo> mWarpSrcInfoQueue;
};

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam

#endif // _MTK_CAMERA_STREAMING_FEATURE_PIPE_WARP_NODE_H_

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

#ifndef _MTK_CAMERA_STREAMING_FEATURE_PIPE_RSC_NODE_H_
#define _MTK_CAMERA_STREAMING_FEATURE_PIPE_RSC_NODE_H_

#include "StreamingFeatureNode.h"
#include "RSCStreamBase.h"
#include "MDPWrapper.h"

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

struct FSCinRSCImgPair
{
    MINT64          mRssoTimestamp;
    ImgBuffer       mFSCRsso;
    MRectF          mCropRectF;

    FSCinRSCImgPair() : mRssoTimestamp(0)
    {
    }
};

class RSCEnqueData
{
public:
    RequestPtr mRequest;
    IImageBuffer *mPrevRsso;
    IImageBuffer *mCurrRsso;
    ImgBuffer mMV;
    ImgBuffer mBV;
    ImgBuffer mPrevMV;
    ImgBuffer mPrevFSCRsso;
    ImgBuffer mCurrFSCRsso;
    IImageBuffer *mPrevFSCOrgRsso;
    IImageBuffer *mCurrFSCOrgRsso;
    MBOOL useDummy;
};

class RSCNode : public virtual StreamingFeatureNode , public virtual RSCStreamBase<RSCEnqueData>
{
public:
    RSCNode(const char *name);
    virtual ~RSCNode();

public:
    virtual MBOOL onData(DataID id, const RequestPtr &data);

protected:
    virtual MBOOL onInit();
    virtual MBOOL onUninit();
    virtual MBOOL onThreadStart();
    virtual MBOOL onThreadStop();
    virtual MBOOL onThreadLoop();

private:
    MBOOL processRSC(const RequestPtr &request);
    MVOID prepareRSCEnqueData(const RequestPtr &request, RSCEnqueData &data);
    MBOOL prepareFSCRsso(IImageBuffer *iRsso, FSCinRSCImgPair &fscinRSCImgPair);
    MVOID prepareRSCConfig(RSCEnqueData &data, NSIoPipe::RSCConfig &config);
    MVOID enqueRSCStream(const RSCParam &param, const RSCEnqueData &data);
    virtual MVOID onRSCStreamBaseCB(const RSCParam &param, const RSCEnqueData &data);
    MVOID handleResultData(const RequestPtr &request, const RSCResult &result);

    static MVOID IImageBuffer2RSCBufInfo(NSIoPipe::RSCDMAPort dmaPort, IImageBuffer *pImageBuffer, NSIoPipe::RSCBufInfo& rscBufInfo);

private:
    NSCam::NSIoPipe::NSRsc::IHalRscPipe *mRSCStream;
    WaitQueue<RequestPtr> mRequests;
    android::sp<ImageBufferPool> mMVBufferPool;
    android::sp<ImageBufferPool> mBVBufferPool;
    android::sp<ImageBufferPool> mDummyBufferPool;
    android::sp<ImageBufferPool> mFSCRSSOBufferPool;
    ImgBuffer mLastMV;
    ImgBuffer mDummy;
    FSCinRSCImgPair mPrevFSCinRSCImgPair;
    FSCinRSCImgPair mCurrFSCinRSCImgPair;
    MDPWrapper      mMDP;
};

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam

#endif // _MTK_CAMERA_STREAMING_FEATURE_PIPE_RSC_NODE_H_

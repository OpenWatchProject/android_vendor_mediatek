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
 * MediaTek Inc. (C) 2018. All rights reserved.
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

#ifndef _MTK_CAMERA_STREAMING_FEATURE_PIPE_P2NR_NODE_H_
#define _MTK_CAMERA_STREAMING_FEATURE_PIPE_P2NR_NODE_H_

#include "StreamingFeatureNode.h"
#include "DIPStreamBase.h"
#include "MtkHeader.h"

#include <mtkcam/drv/iopipe/PostProc/IHalPostProcPipe.h>
#include <common/3dnr/3dnr_hal_base.h>
#include <mtkcam3/feature/utils/p2/DIPStream.h>
#include <common/3dnr/3dnr_hal_base.h>

using NSCam::NR3D::NR3DHALResult;

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

class P2NREnqueData
{
public:
    RequestPtr  mRequest;
    DSDNImg     mDSDNImg;
    SmartTuningBuffer   mTuningBuf;
    ImgBuffer   mPrevFullImg = NULL; // VIPI use
};

class P2NRNode : public StreamingFeatureNode, public DIPStreamBase<P2NREnqueData>
{
public:
    P2NRNode(const char *name);
    virtual ~P2NRNode();

public:
    virtual MBOOL onData(DataID id, const DSDNData &data);
    virtual IOPolicyType getIOPolicy(StreamType stream, const StreamingReqInfo &reqInfo) const;

protected:
    virtual MVOID onDIPStreamBaseCB(const Feature::P2Util::DIPParams &params, const P2NREnqueData &request);

protected:
    virtual MBOOL onInit();
    virtual MBOOL onUninit();
    virtual MBOOL onThreadStart();
    virtual MBOOL onThreadStop();
    virtual MBOOL onThreadLoop();

private:
    MBOOL prepare3DNR(Feature::P2Util::DIPParams &params, const RequestPtr &request, MUINT32 sensorID, DSDNImg &dsdnImg);

    MVOID prepareVIPI(Feature::P2Util::DIPFrameParams &frame, const RequestPtr &request, P2NREnqueData &data);
    MBOOL processP2NR(const RequestPtr &request, DSDNImg &dsdnImg);
    MVOID enqueFeatureStream(Feature::P2Util::DIPParams &params, P2NREnqueData &data);
    MBOOL checkInput(const RequestPtr &request, const DSDNImg &dsdnImg);
    MVOID handleResultData(const RequestPtr &request, const DSDNImg &dsdnImg);
    MVOID handleDump(const RequestPtr &request, const DSDNImg &dsdnImg);

    MBOOL prepareTuning(Feature::P2Util::DIPParams &params, const RequestPtr &request, P2NREnqueData &data);
    MBOOL prepareOutput(Feature::P2Util::DIPParams &params, const RequestPtr &request, P2NREnqueData &data);
    MVOID prepareDs2Img(Feature::P2Util::DIPFrameParams &frame, const RequestPtr &request, ImgBuffer &outImg, const FrameInInfo &inInfo);
    MVOID prepareFDImg(Feature::P2Util::DIPFrameParams &frame, const RequestPtr &request, const BasicImg &ds1Img);

private:
    WaitQueue<DSDNData> mP2NRDatas;
    android::sp<IBufferPool> mDS2ImgPool;
    DSDNCfg mDSDNCfg;

    MINT32 m3dnrLogLevel = 0;
    NR3DHALResult mNr3dHalResult;
    Feature::P2Util::DIPStream *mDIPStream = NULL;
    android::sp<TuningBufferPool> mTuningPool;
};

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam

#endif // _MTK_CAMERA_STREAMING_FEATURE_PIPE_P2NR_NODE_H_

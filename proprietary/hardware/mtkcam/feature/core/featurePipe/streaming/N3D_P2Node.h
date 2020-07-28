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
 * MediaTek Inc. (C) 2016. All rights reserved.
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
#ifndef _MTK_CAMERA_STREAMING_FEATURE_PIPE_N3D_P2NODE_H_
#define _MTK_CAMERA_STREAMING_FEATURE_PIPE_N3D_P2NODE_H_

#include "StreamingFeatureNode.h"
#include "NormalStreamBase.h"
#include "P2CamContext.h"
#include "N3DConfig.h"

#define LOG_TAG "N3D_P2Node"
#include <utils/KeyedVector.h>

#include <mtkcam/drv/iopipe/PostProc/IHalPostProcPipe.h>
#include <featurePipe/core/include/CamThreadNode.h>
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#include <mtkcam/feature/stereo/hal/stereo_size_provider.h>
#include <stereo_tuning_provider.h>
#include <n3d_hal.h>
#include <featurePipe/vsdof/util/TuningBufferPool.h>




namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

class N3DP2EnqueData
{
public:
    RequestPtr mRequest;
    N3DResult mN3DResult;
};

class N3DP2BufConfig
{
public:
    MSize mFEBInputImg_Master;
    MSize mFEBInputImg_Slave;
    MSize mFECInputImg_Master;
    MSize mFECInputImg_Slave;
    MSize mFEBImgSize;
    MSize mFECImgSize;
    MSize mFEBOImgSize;
    MSize mFECOImgSize;
    MSize mFMBOImgSize;
    MSize mFMCOImgSize;
    MSize mCCinImgSize;
    MSize mRectin_Master;
    MSize mRectin_Slave;
    StereoArea P2A_MAIN2_FEBO_AREA;
    StereoArea P2A_MAIN2_FECO_AREA;
};

class N3D_P2Node : public StreamingFeatureNode, public NormalStreamBase<N3DP2EnqueData>
{
public:
    N3D_P2Node(const char *name);
    virtual ~N3D_P2Node();
public:
    virtual MBOOL onData(DataID id, const RequestPtr &data);
protected:
    virtual MBOOL onInit();
    virtual MBOOL onUninit();
    virtual MBOOL onThreadStart();
    virtual MBOOL onThreadStop();
    virtual MBOOL onThreadLoop();
protected:
    virtual MVOID onNormalStreamBaseCB(const QParams &params, const N3DP2EnqueData &request);
private:
    MVOID handleResultData(const RequestPtr &request, const N3DP2EnqueData &data);
    MBOOL initN3DP2();
    MVOID uninitN3DP2();
    MBOOL processN3DP2(const RequestPtr &request);
public:
    // FE Tuning Buffer Map, key=stage, value= tuning buffer
    android::KeyedVector<MUINT, NSCam::NSIoPipe::FEInfo> mFETuningBufferMap;
    // FM Tuning Buffer Map, key=frame ID, value= tuning buffer
    android::KeyedVector<MUINT, NSCam::NSIoPipe::FMInfo> mFMTuningBufferMap;
private:
    MVOID prepareN3DImgs(QParams &params, const RequestPtr &request, N3DP2EnqueData &data);
    MVOID prepareCropInfo_N3D(QParams &params, const RequestPtr &request, N3DP2EnqueData &data);
    MVOID queryFEOBufferSize(MSize iBufSize, MUINT iBlockSize, MUINT32 &riFEOWidth, MUINT32 &riFEOHeight);
    MVOID queryFMOBufferSize(MUINT32 iFEOWidth, MUINT32 iFEOHeight, MUINT32 &riFMOWidth, MUINT32 &riFMOHeight);
    MVOID createImgBufPool();
    MVOID createFEFMBufferPool();
    MVOID prepareTemParams();
    MVOID setFESizInfo();
    MBOOL calCropForScreen(MPoint &rCropStartPt, MSize& rCropSize);
    MBOOL configN3DFrame_0(const RequestPtr &request, N3DResult& rP2AResult, NSCam::NSIoPipe::FrameParams& rFrame, MINT32 iModuleTrans);
    MBOOL configN3DFrame_1(const RequestPtr &request, N3DResult& rP2AResult, NSCam::NSIoPipe::FrameParams& rFrame, MINT32 iModuleTrans);
    MBOOL configN3DFrame_2(const RequestPtr &request, N3DResult& rP2AResult, const NSCam::NSIoPipe::FEInfo& pFEInfo, NSCam::NSIoPipe::FrameParams& rFrame);
    MBOOL configN3DFrame_3(const RequestPtr &request, N3DResult& rP2AResult, const NSCam::NSIoPipe::FEInfo& pFEInfo, NSCam::NSIoPipe::FrameParams& rFrame);
    MBOOL configN3DFrame_4(const RequestPtr &request, N3DResult& rP2AResult, const NSCam::NSIoPipe::FEInfo& pFEInfo, NSCam::NSIoPipe::FrameParams& rFrame);
    MBOOL configN3DFrame_5(const RequestPtr &request, N3DResult& rP2AResult, const NSCam::NSIoPipe::FEInfo& pFEInfo, NSCam::NSIoPipe::FrameParams& rFrame);
    MBOOL configN3DFrame_6(const RequestPtr &request, N3DResult& rP2AResult, const NSCam::NSIoPipe::FMInfo& pFMInfo, NSCam::NSIoPipe::FrameParams& rFrame);
    MBOOL configN3DFrame_7(const RequestPtr &request, N3DResult& rP2AResult, const NSCam::NSIoPipe::FMInfo& pFMInfo, NSCam::NSIoPipe::FrameParams& rFrame);
    MBOOL configN3DFrame_8(const RequestPtr &request, N3DResult& rP2AResult, const NSCam::NSIoPipe::FMInfo& pFMInfo, NSCam::NSIoPipe::FrameParams& rFrame);
    MBOOL configN3DFrame_9(const RequestPtr &request, N3DResult& rP2AResult, const NSCam::NSIoPipe::FMInfo& pFMInfo, NSCam::NSIoPipe::FrameParams& rFrame);
    MVOID setupEmptyTuningWithFM(NSCam::NSIoPipe::FMInfo& fmInfo, MUINT iFrameIdx);
    MBOOL cropN3DFrame_0(const RequestPtr &request, N3DResult& rP2AResult, NSCam::NSIoPipe::FrameParams& rFrame);
    MBOOL cropN3DFrame_1(const RequestPtr &request, N3DResult& rP2AResult, NSCam::NSIoPipe::FrameParams& rFrame);
    MBOOL cropN3DFrame_2(const RequestPtr &request, N3DResult& rP2AResult, NSCam::NSIoPipe::FrameParams& rFrame);
    MBOOL cropN3DFrame_3(const RequestPtr &request, N3DResult& rP2AResult, NSCam::NSIoPipe::FrameParams& rFrame);
    MBOOL cropN3DFrame_4(const RequestPtr &request, N3DResult& rP2AResult, NSCam::NSIoPipe::FrameParams& rFrame);
    MBOOL cropN3DFrame_5(const RequestPtr &request, N3DResult& rP2AResult, NSCam::NSIoPipe::FrameParams& rFrame);
    MVOID debugQParams(const QParams& rInputQParam);//debug
private:
    N3DP2BufConfig config;
    android::sp<ImageBufferPool> mN3dFEBMasterImgPool;
    android::sp<ImageBufferPool> mN3dFEBSlaveImgPool;
    android::sp<ImageBufferPool> mN3dFECMasterImgPool;
    android::sp<ImageBufferPool> mN3dFECSlaveImgPool;
    android::sp<ImageBufferPool> mN3dFEBOImgPool;
    android::sp<ImageBufferPool> mN3dFECOImgPool;
    android::sp<ImageBufferPool> mN3dFMBOImgPool;
    android::sp<ImageBufferPool> mN3dFMCOImgPool;
    android::sp<ImageBufferPool> mN3dCCinImgPool;
    android::sp<ImageBufferPool> mN3dRectinMasterImgPool;
    android::sp<ImageBufferPool> mN3dRectinSlaveImgPool;
    // tuning buffers
    android::sp<TuningBufferPool> mN3dTuningBufferPool;
    //
    _SRZ_SIZE_INFO_ mN3dStage1SrzInfo;
    _SRZ_SIZE_INFO_ mN3dStage2SrzInfo;
    //
    WaitQueue<RequestPtr> mRequests;
    NSCam::NSIoPipe::NSPostProc::INormalStream *mNormalStream;
};
} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam

#endif
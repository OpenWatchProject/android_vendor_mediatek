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


#include "P2ANode.h"
#include <mtkcam/feature/DualCam/FOVHal.h>
#include <mtkcam/drv/iopipe/PortMap.h>

#if (MTKCAM_HAVE_DUAL_ZOOM_SUPPORT == 1)
#include <camera_custom_dualzoom.h>
#endif

#define PIPE_CLASS_TAG "P2A_FOV"
#define PIPE_TRACE TRACE_P2A_FOV
#include <featurePipe/core/include/PipeLog.h>
#include <mtkcam/feature/featurePipe/IStreamingFeaturePipe_var.h>
#include <mtkcam/drv/iopipe/PostProc/INormalStream.h>

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

using NSIoPipe::Input;
using NSIoPipe::Output;
using NSIoPipe::ModuleInfo;
using NSIoPipe::FrameParams;
using NSIoPipe::ExtraParam;
using NSIoPipe::EPIPE_FE_INFO_CMD;
using NSIoPipe::EPIPE_FM_INFO_CMD;
using NSIoPipe::EDIPHWVersion_40;
using NSIoPipe::EDIPHWVersion_50;
using NSIoPipe::EDIPINFO_DIPVERSION;
using NSIoPipe::EDIPInfoEnum;
using NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_Normal;
using NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_FM;
using NSCam::NSIoPipe::PORT_DEPI;
using NSCam::NSIoPipe::PORT_DMGI;
using NSCam::NSIoPipe::PORT_FEO;
using NSCam::NSIoPipe::PORT_IMGI;
using NSCam::NSIoPipe::PORT_MFBO;
using NSCam::NSIoPipe::PORT_PAK2O;
using NSIoPipe::NSPostProc::INormalStream;
using NSImageio::NSIspio::EPortIndex_IMGI;

FOVHal::RAW_RATIO queryInputRawRatio(const RequestPtr &request)
{
    IImageBuffer* pImgBuf = request->getInputBuffer();
    return FOVHal::queryRatio(pImgBuf->getImgSize());
}

MBOOL configFOVFrame_0(
    const RequestPtr &request,
    FovP2AResult& rP2AResult,
    NSCam::NSIoPipe::FEInfo* pFEInfo,
    NSCam::NSIoPipe::FrameParams& rFrame
)
{
    // frame 0 : input already exist in the QParams
    MINT32 senID_master = request->getVar<MINT32>(VAR_DUALCAM_FOV_MASTER_ID, -1);
    // output: FEO
    Output output(PORT_FEO, rP2AResult.mFEO_Master->getImageBufferPtr());
    rFrame.mvOut.push_back(output);
    // FE config
    ExtraParam extra;
    extra.CmdIdx = EPIPE_FE_INFO_CMD;
    extra.moduleStruct = (MVOID*)pFEInfo;
    rFrame.mvExtraParam.push_back(extra);
    return MTRUE;
}

MBOOL configFOVFrame_1(
    const RequestPtr &request,
    FovP2AResult& rP2AResult,
    NSCam::NSIoPipe::FEInfo* pFEInfo,
    NSCam::NSIoPipe::FrameParams& rFrame
)
{
    // Get the slave feature param
    FeaturePipeParam fparam_slave;
    if(!request->tryGetVar<FeaturePipeParam>(VAR_DUALCAM_FOV_SLAVE_PARAM, fparam_slave))
    {
        MY_LOGE("Failed to get slave feature params. Cannot do FEFM!");
        return MFALSE;
    }
    QParams& qParam_Slave = fparam_slave.mQParams;
    //
    MINT32 senID_slave = request->getVar<MINT32>(VAR_DUALCAM_FOV_SLAVE_ID, -1);
    rFrame.mSensorIdx = senID_slave;
    rFrame.mStreamTag = ENormalStreamTag_Normal;
    // input needs to retrieve from slave feature pipe param
    for(size_t index=0; index<qParam_Slave.mvFrameParams.itemAt(0).mvIn.size(); ++index)
    {
        rFrame.mvIn.push_back(qParam_Slave.mvFrameParams.itemAt(0).mvIn.itemAt(index));
    }
    // output : FEO
    Output output = Output(PORT_FEO, rP2AResult.mFEO_Slave->getImageBufferPtr());
    rFrame.mvOut.push_back(output);
    // tuning buffer: use slave tuning
    rFrame.mTuningData = qParam_Slave.mvFrameParams.itemAt(0).mTuningData;
    // FE config
    ExtraParam extra;
    extra.CmdIdx = EPIPE_FE_INFO_CMD;
    extra.moduleStruct = (MVOID*)pFEInfo;
    rFrame.mvExtraParam.push_back(extra);
    return MTRUE;
}

MBOOL configFOVFrame_2(
    const RequestPtr &request,
    FovP2AResult& rP2AResult,
    NSCam::NSIoPipe::FMInfo* pFMInfo,
    NSCam::NSIoPipe::FrameParams& rFrame,
    MUINT32 iIspVersion
)
{
    MINT32 senID_master = request->getVar<MINT32>(VAR_DUALCAM_FOV_MASTER_ID, -1);
    rFrame.mSensorIdx = senID_master;
    rFrame.mStreamTag = ENormalStreamTag_FM;
    // input
    Input input = Input(PORT_DEPI, rP2AResult.mFEO_Master->getImageBufferPtr());
    rFrame.mvIn.push_back(input);

    input = Input(PORT_DMGI, rP2AResult.mFEO_Slave->getImageBufferPtr());
    rFrame.mvIn.push_back(input);
    // output
    Output output = Output( ( iIspVersion == EDIPHWVersion_50 ) ? PORT_PAK2O : PORT_MFBO, rP2AResult.mFMO_MtoS->getImageBufferPtr());
    rFrame.mvOut.push_back(output);
    // tuning
    rFrame.mTuningData = rP2AResult.mFMTuningBuf0->mpVA;
    // FM config
    ExtraParam extra;
    extra.CmdIdx = EPIPE_FM_INFO_CMD;
    extra.moduleStruct = (MVOID*)pFMInfo;
    rFrame.mvExtraParam.push_back(extra);
    return MTRUE;
}

MBOOL configFOVFrame_3(
    const RequestPtr &request,
    FovP2AResult& rP2AResult,
    NSCam::NSIoPipe::FMInfo* pFMInfo,
    NSCam::NSIoPipe::FrameParams& rFrame,
    MUINT32 iIspVersion
)
{
    MINT32 senID_slave = request->getVar<MINT32>(VAR_DUALCAM_FOV_SLAVE_ID, -1);
    rFrame.mSensorIdx = senID_slave;
    rFrame.mStreamTag = ENormalStreamTag_FM;
    // input
    Input input = Input(PORT_DMGI, rP2AResult.mFEO_Master->getImageBufferPtr());
    rFrame.mvIn.push_back(input);

    input = Input(PORT_DEPI, rP2AResult.mFEO_Slave->getImageBufferPtr());
    rFrame.mvIn.push_back(input);
    // output
    Output output = Output( ( iIspVersion == EDIPHWVersion_50 ) ? PORT_PAK2O : PORT_MFBO, rP2AResult.mFMO_StoM->getImageBufferPtr());
    rFrame.mvOut.push_back(output);
    // tuning
    rFrame.mTuningData = rP2AResult.mFMTuningBuf1->mpVA;
    // FM config
    ExtraParam extra;
    extra.CmdIdx = EPIPE_FM_INFO_CMD;
    extra.moduleStruct = (MVOID*)pFMInfo;
    rFrame.mvExtraParam.push_back(extra);
    return MTRUE;
}

MBOOL P2ANode::prepareFOVFEFM(QParams &params, const RequestPtr &request, P2AEnqueData &data)
{
    TRACE_FUNC_ENTER();

    if(params.mvFrameParams.size() != 1)
    {
        MY_LOGE("Non-supported situationa, mvFrameParams size=%zu", params.mvFrameParams.size());
        return MFALSE;
    }

    if(request->needEIS25())
    {
        MY_LOGE("FOV/EIS25 is not legal feature-combination!");
        return MFALSE;
    }

    FovP2AResult& rP2AResult = data.mFovP2AResult;
    rP2AResult.mFEO_Master = mFovFEOImgPool->requestIIBuffer();
    rP2AResult.mFEO_Slave = mFovFEOImgPool->requestIIBuffer();
    rP2AResult.mFMO_MtoS = mFovFMOImgPool->requestIIBuffer();
    rP2AResult.mFMO_StoM = mFovFMOImgPool->requestIIBuffer();
    rP2AResult.mFMTuningBuf0 = mFovTuningBufferPool->request();
    rP2AResult.mFMTuningBuf1 = mFovTuningBufferPool->request();
    // reset tuning to zero
    memset((void*)rP2AResult.mFMTuningBuf0->mpVA, 0, INormalStream::getRegTableSize());
    memset((void*)rP2AResult.mFMTuningBuf1->mpVA, 0, INormalStream::getRegTableSize());
    // set active area
    FOVHal::RAW_RATIO ratio = queryInputRawRatio(request);
    FOVHal::SizeConfig config = FOVHal::getSizeConfig(ratio);
    rP2AResult.mFEO_Master->getImageBufferPtr()->setExtParam(config.mFEOSize);
    rP2AResult.mFEO_Slave->getImageBufferPtr()->setExtParam(config.mFEOSize);
    rP2AResult.mFMO_MtoS->getImageBufferPtr()->setExtParam(config.mFMOSize);
    rP2AResult.mFMO_StoM->getImageBufferPtr()->setExtParam(config.mFMOSize);
    // query tuning information
    if(!FOVHal::configTuning_FE(config, &mFovFEInfo) || !FOVHal::configTuning_FM(config, &mFovFMInfo))
    {
        MY_LOGE("Failed to query tuning!");
        return MFALSE;
    }
    // query ISP platform version
    map<EDIPInfoEnum, MUINT32> mDipInfo;
                mDipInfo[EDIPINFO_DIPVERSION] = EDIPHWVersion_40;
                MBOOL bDipReturn = NSCam::NSIoPipe::NSPostProc::INormalStream::queryDIPInfo(mDipInfo);
                if (!bDipReturn) {
                    MY_LOGE("queryDIPInfo fail!");
                    return MFALSE;
                }

    MBOOL bRet = MFALSE;
    int iFrameIdx = 0;
    bRet = configFOVFrame_0(request, rP2AResult, &mFovFEInfo,
                                params.mvFrameParams.editItemAt(iFrameIdx++));

    // frame 1
    params.mvFrameParams.push_back(FrameParams());
    FrameParams& frameData = params.mvFrameParams.editItemAt(iFrameIdx++);
    bRet &= configFOVFrame_1(request, rP2AResult, &mFovFEInfo, frameData);
    // frame 2
    params.mvFrameParams.push_back(FrameParams());
    bRet &= configFOVFrame_2(request, rP2AResult, &mFovFMInfo,
                                params.mvFrameParams.editItemAt(iFrameIdx++),
                                mDipInfo[EDIPINFO_DIPVERSION]);
    // frame 3
    params.mvFrameParams.push_back(FrameParams());
    bRet &= configFOVFrame_3(request, rP2AResult, &mFovFMInfo,
                                params.mvFrameParams.editItemAt(iFrameIdx++),
                                mDipInfo[EDIPINFO_DIPVERSION]);
    if(!bRet)
    {
        MY_LOGE("Failed to config FOV Input/Output!");
        return MFALSE;
    }

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL P2ANode::prepareCropInfo_FOV(QParams &params, const RequestPtr &request, P2AEnqueData &data)
{
#if (MTKCAM_HAVE_DUAL_ZOOM_SUPPORT == 1)
    TRACE_FUNC_ENTER();
    int iFrameIdx = 0;
    //
    MINT32 senID_master = request->getVar<MINT32>(VAR_DUALCAM_FOV_MASTER_ID, -1);
    MINT32 senID_slave = request->getVar<MINT32>(VAR_DUALCAM_FOV_SLAVE_ID, -1);
    FOVHal* pFovHal = FOVHal::getInstance();
    FeaturePipeParam fparam_slave;
    fparam_slave = request->getVar<FeaturePipeParam>(VAR_DUALCAM_FOV_SLAVE_PARAM, fparam_slave);

    MINT32 userZoomFactor = -1;
    if( senID_master == DUALZOOM_WIDE_CAM_ID )
        userZoomFactor = request->getVar<MUINT32>(VAR_DUALCAM_ZOOM_RATIO, 0);
    else
        userZoomFactor = fparam_slave.mVarMap.get<MUINT32>(VAR_DUALCAM_ZOOM_RATIO, 0);
    // frame 0
    FOVHal::Configs config;
    DUAL_ZOOM_FOV_FEFM_INFO fovInfo;
    DUAL_ZOOM_FOV_FEFM_INFO* pFovInfo = nullptr;
    DUAL_ZOOM_FOV_FEFM_INFO fovInfo2;
    DUAL_ZOOM_FOV_FEFM_INFO* pFovInfo2 = nullptr;
    IImageBuffer* pImgBuf = NULL;
    if(request->tryGetVar<DUAL_ZOOM_FOV_FEFM_INFO>(VAR_DUALCAM_FOV_CALIB_INFO, fovInfo))
        pFovInfo = &fovInfo;
    // get fov info
    if(fparam_slave.mVarMap.tryGet(VAR_DUALCAM_FOV_CALIB_INFO, fovInfo2))
        pFovInfo2 = &fovInfo2;
    else
        pFovInfo2 = nullptr;
    for(size_t index=0; index < fparam_slave.mQParams.mvFrameParams[0].mvIn.size(); index++)
    {
        if( fparam_slave.mQParams.mvFrameParams[0].mvIn[index].mPortID.index == EPortIndex_IMGI )
        {
            pImgBuf = fparam_slave.mQParams.mvFrameParams[0].mvIn[index].mBuffer;
            break;
        }
    }
    config.CamID = senID_master;
    config.MainStreamingSize = request->getInputBuffer()->getImgSize();
    config.ZoomRatio = userZoomFactor;
    config.SlaveStreamingSize = pImgBuf->getImgSize();
    if(!pFovHal->getFEFMSRZ1Info(config, pFovInfo, mFovSrzInfo[0], pFovInfo2, mFovSrzInfo[1]))
    {
        MY_LOGE("Failed to query SRZ1 info.");
        return MFALSE;
    }
    // main SRZ1
    ModuleInfo moduleInfo;
    moduleInfo.moduleTag = EDipModule_SRZ1;
    moduleInfo.moduleStruct = reinterpret_cast<MVOID*> (&mFovSrzInfo[0]);
    params.mvFrameParams.editItemAt(iFrameIdx).mvModuleData.push_back(moduleInfo);
    // config fe input size
    data.mFovP2AResult.mFEInSize_Master = MSize(fovInfo.P1CropW, fovInfo.P1CropH);

    // frame 1
    iFrameIdx++;
    // salve SRZ1
    moduleInfo.moduleTag = EDipModule_SRZ1;
    moduleInfo.moduleStruct = reinterpret_cast<MVOID*> (&mFovSrzInfo[1]);
    params.mvFrameParams.editItemAt(iFrameIdx).mvModuleData.push_back(moduleInfo);
    // config fe input size
    data.mFovP2AResult.mFEInSize_Slave = MSize(fovInfo2.P1CropW, fovInfo2.P1CropH);

    TRACE_FUNC_EXIT();
#else
    (void)params;
    (void)request;
    (void)data;
#endif
    return MTRUE;
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam


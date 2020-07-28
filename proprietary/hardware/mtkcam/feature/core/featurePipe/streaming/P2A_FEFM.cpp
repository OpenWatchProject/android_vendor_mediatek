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

#include "P2ANode.h"
#include "FMHal.h"

#define PIPE_CLASS_TAG "P2A_FEFM"
#define PIPE_TRACE TRACE_P2A_FEFM
#include <featurePipe/core/include/PipeLog.h>

using NSCam::NSIoPipe::ExtraParam;
using NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_Normal;
using NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_FM;
using NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_FE;
using NSCam::NSIoPipe::Input;
using NSCam::NSIoPipe::Output;
using NSCam::NSIoPipe::FEInfo;
using NSCam::NSIoPipe::FMInfo;
typedef NSCam::NSIoPipe::FrameParams FrameParams;
using NSImageio::NSIspio::EPortIndex_LCEI;
using NSImageio::NSIspio::EPortIndex_IMG3O;
using NSImageio::NSIspio::EPortIndex_WDMAO;
using NSImageio::NSIspio::EPortIndex_WROTO;
using NSImageio::NSIspio::EPortIndex_VIPI;
using NSImageio::NSIspio::EPortIndex_IMGI;
using NSImageio::NSIspio::EPortIndex_DEPI;//left
using NSImageio::NSIspio::EPortIndex_DMGI;//right
using NSImageio::NSIspio::EPortIndex_FEO;
using NSImageio::NSIspio::EPortIndex_MFBO;
using NSImageio::NSIspio::EPortIndex_REGI;
using NSCam::NSIoPipe::EPIPE_FE_INFO_CMD;
using NSCam::NSIoPipe::EPIPE_FM_INFO_CMD;

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

/* NOTE : this file just implements some FEFM function in P2ANode.h
* Some object might declare in P2ANode
*/

MBOOL P2ANode::initFEFM()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MVOID P2ANode::uninitFEFM()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

MBOOL P2ANode::prepareCropInfo_FE(QParams &params, const RequestPtr &request, P2AEnqueData &data)
{
    TRACE_FUNC_ENTER();

    //----------- round 1------------------------------------------
    MCrpRsInfo crop;
    crop.mGroupID = 2;//wdma
    crop.mCropRect = MCropRect(MPoint(0, 0), request->mFullImgSize);
    crop.mResizeDst = data.mFE1Img->getImageBuffer()->getImgSize();
    params.mvFrameParams.editItemAt(0).mvCropRsInfo.push_back(crop);

    if( request->needP2AEarlyDisplay() )
    {
        MCrpRsInfo displaycrop;
        if( request->getDisplayCrop(displaycrop, RRZO_DOMAIN) ||
            request->getRecordCrop(displaycrop, RRZO_DOMAIN) )
        {
            crop.mCropRect = displaycrop.mCropRect;
            crop.mResizeDst = displaycrop.mResizeDst;
        }
        else
        {
            MY_LOGD("default display crop");
            crop.mCropRect = MCropRect(MPoint(0, 0), request->mFullImgSize);
            crop.mResizeDst = request->mFullImgSize;
        }
        crop.mGroupID = 3;//wrot
        crop.mFrameGroup = 0;
        params.mvFrameParams.editItemAt(0).mvCropRsInfo.push_back(crop);
    }

    //srz config
    NSCam::NSIoPipe::ModuleInfo module;
    module.moduleTag = EDipModule_SRZ1;
    //_SRZ_SIZE_INFO_ *mpsrz1Param = new _SRZ_SIZE_INFO_;
    mpsrz1Param.in_w = request->getInputSize().w;
    mpsrz1Param.in_h = request->getInputSize().h;
    mpsrz1Param.crop_floatX = 0;
    mpsrz1Param.crop_floatY = 0;
    mpsrz1Param.crop_x = 0;
    mpsrz1Param.crop_y = 0;
    mpsrz1Param.crop_w = request->getInputSize().w;
    mpsrz1Param.crop_h = request->getInputSize().h;
    mpsrz1Param.out_w = mFM_FE_cfg[0].mImageSize.w;
    mpsrz1Param.out_h = mFM_FE_cfg[0].mImageSize.h;
    module.moduleStruct = reinterpret_cast<MVOID*> (&mpsrz1Param);
    params.mvFrameParams.editItemAt(0).mvModuleData.push_back(module);
    //----------- round 1------------------------------------------

    //----------- round 2------------------------------------------
    crop.mGroupID = 2;//wdma
    crop.mCropRect = MCropRect(MPoint(0, 0), data.mFE1Img->getImageBuffer()->getImgSize());
    crop.mResizeDst = data.mFE2Img->getImageBuffer()->getImgSize();
    params.mvFrameParams.editItemAt(1).mvCropRsInfo.push_back(crop);

    //srz config
    NSCam::NSIoPipe::ModuleInfo module_2;
    module_2.moduleTag = EDipModule_SRZ1;
    //_SRZ_SIZE_INFO_ *mpsrz1_2Param = new _SRZ_SIZE_INFO_;
    mpsrz1_2Param.in_w = data.mFE1Img->getImageBuffer()->getImgSize().w;
    mpsrz1_2Param.in_h = data.mFE1Img->getImageBuffer()->getImgSize().h;
    mpsrz1_2Param.crop_floatX = 0;
    mpsrz1_2Param.crop_floatY = 0;
    mpsrz1_2Param.crop_x = 0;
    mpsrz1_2Param.crop_y = 0;
    mpsrz1_2Param.crop_w = data.mFE1Img->getImageBuffer()->getImgSize().w;
    mpsrz1_2Param.crop_h = data.mFE1Img->getImageBuffer()->getImgSize().h;
    mpsrz1_2Param.out_w = mFM_FE_cfg[1].mImageSize.w;//960;//data.mFE2Img->getImageBuffer()->getImgSize().w;
    mpsrz1_2Param.out_h = mFM_FE_cfg[1].mImageSize.h;//544;//data.mFE2Img->getImageBuffer()->getImgSize().h;
    module_2.moduleStruct = reinterpret_cast<MVOID*> (&mpsrz1_2Param);
    params.mvFrameParams.editItemAt(1).mvModuleData.push_back(module_2);
    //----------- round 2------------------------------------------

    //----------- round 3------------------------------------------
    crop.mGroupID = 2;//wdma, need to remove later
    crop.mCropRect = MCropRect(MPoint(0, 0), data.mFE2Img->getImageBuffer()->getImgSize());
    crop.mResizeDst = data.mFE2Img->getImageBuffer()->getImgSize();
    params.mvFrameParams.editItemAt(2).mvCropRsInfo.push_back(crop);

    //srz config
    NSCam::NSIoPipe::ModuleInfo module_3;
    module_3.moduleTag = EDipModule_SRZ1;
    module_3.frameGroup = 2;
    //_SRZ_SIZE_INFO_ *mpsrz1_3Param = new _SRZ_SIZE_INFO_;
    mpsrz1_3Param.in_w = data.mFE2Img->getImageBuffer()->getImgSize().w;
    mpsrz1_3Param.in_h = data.mFE2Img->getImageBuffer()->getImgSize().h;
    mpsrz1_3Param.crop_floatX = 0;
    mpsrz1_3Param.crop_floatY = 0;
    mpsrz1_3Param.crop_x = 0;
    mpsrz1_3Param.crop_y = 0;
    mpsrz1_3Param.crop_w = data.mFE2Img->getImageBuffer()->getImgSize().w;
    mpsrz1_3Param.crop_h = data.mFE2Img->getImageBuffer()->getImgSize().h;
    mpsrz1_3Param.out_w = data.mFE2Img->getImageBuffer()->getImgSize().w;
    mpsrz1_3Param.out_h = data.mFE2Img->getImageBuffer()->getImgSize().h;
    module_3.moduleStruct = reinterpret_cast<MVOID*> (&mpsrz1_3Param);
    params.mvFrameParams.editItemAt(2).mvModuleData.push_back(module_3);
    //----------- round 3------------------------------------------

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL P2ANode::prepareFE(QParams &params, const RequestPtr &request, P2AEnqueData &data)
{
    TRACE_FUNC_ENTER();
    Input input;
    Output output;

    //----------- round 1------------------------------------------
    output.mPortID = PortID(EPortType_Memory, EPortIndex_FEO, PORTID_OUT);
    output.mBuffer = data.mFMResult.FE.High->getImageBufferPtr();
    params.mvFrameParams.editItemAt(0).mvOut.push_back(output);

    if( request->needP2AEarlyDisplay() )
    {
        Output display_output;
        if( request->getDisplayOutput(display_output) )
        {
            display_output.mPortID = PortID(EPortType_Memory, EPortIndex_WROTO, PORTID_OUT);
            params.mvFrameParams.editItemAt(0).mvOut.push_back(display_output);
        }
    }

    output.mPortID = PortID(EPortType_Memory, EPortIndex_WDMAO, PORTID_OUT);
    output.mBuffer = data.mFE1Img->getImageBufferPtr();
    params.mvFrameParams.editItemAt(0).mvOut.push_back(output);

    if( params.mvFrameParams.editItemAt(0).mTuningData == NULL )
    {
        params.mvFrameParams.editItemAt(0).mTuningData = (MVOID*)mFEFMTuning[0];
        MY_LOGW("No tuning data! Use dummy FEFM tuning data.");
    }

    //tuning config(FE MODE 16)
    FMHal::configTuning_FE(FMHal::CFG_FHD, mFEInfo[0], FE_MODE_16);
    {
        ExtraParam extra;
        extra.CmdIdx = EPIPE_FE_INFO_CMD;
        extra.moduleStruct = &mFEInfo[0];
        params.mvFrameParams.editItemAt(0).mvExtraParam.push_back(extra);
    }
    //----------- round 1------------------------------------------

    //----------- round 2------------------------------------------
    params.mvFrameParams.push_back(FrameParams());
    input.mPortID = PortID(EPortType_Memory, EPortIndex_IMGI, PORTID_IN);
    input.mBuffer = data.mFE1Img->getImageBufferPtr();
    params.mvFrameParams.editItemAt(1).mvIn.push_back(input);

    output.mPortID = PortID(EPortType_Memory, EPortIndex_WDMAO, PORTID_OUT);
    output.mBuffer = data.mFE2Img->getImageBufferPtr();
    params.mvFrameParams.editItemAt(1).mvOut.push_back(output);

    output.mPortID = PortID(EPortType_Memory, EPortIndex_FEO, PORTID_OUT);
    output.mBuffer = data.mFMResult.FE.Medium->getImageBufferPtr();
    params.mvFrameParams.editItemAt(1).mvOut.push_back(output);

    params.mvFrameParams.editItemAt(1).mTuningData = (MVOID*)mFEFMTuning[1];

    //tuning config(FE MODE 16)
    params.mvFrameParams.editItemAt(1).mStreamTag = ENormalStreamTag_Normal;
    FMHal::configTuning_FE(FMHal::CFG_FHD, mFEInfo[1], FE_MODE_16);
    {
        ExtraParam extra;
        extra.CmdIdx = EPIPE_FE_INFO_CMD;
        extra.moduleStruct = &mFEInfo[1];
        params.mvFrameParams.editItemAt(1).mvExtraParam.push_back(extra);
    }
    //----------- round 2------------------------------------------

    //----------- round 3------------------------------------------
    params.mvFrameParams.push_back(FrameParams());
    input.mPortID = PortID(EPortType_Memory, EPortIndex_IMGI, PORTID_IN);
    input.mBuffer = data.mFE2Img->getImageBufferPtr();
    params.mvFrameParams.editItemAt(2).mvIn.push_back(input);

    //need to add output bufer for MDP
    output.mPortID = PortID(EPortType_Memory, EPortIndex_WDMAO, PORTID_OUT);
    output.mBuffer = data.mFE3Img->getImageBufferPtr();
    params.mvFrameParams.editItemAt(2).mvOut.push_back(output);

    output.mPortID = PortID(EPortType_Memory, EPortIndex_FEO, PORTID_OUT);
    output.mBuffer = data.mFMResult.FE.Low->getImageBufferPtr();
    params.mvFrameParams.editItemAt(2).mvOut.push_back(output);

    params.mvFrameParams.editItemAt(2).mTuningData = (MVOID*)mFEFMTuning[2];

    //tuning config(FE MODE 8)
    params.mvFrameParams.editItemAt(2).mStreamTag = ENormalStreamTag_Normal;
    FMHal::configTuning_FE(FMHal::CFG_FHD, mFEInfo[2], FE_MODE_8);
    {
        ExtraParam extra;
        extra.CmdIdx = EPIPE_FE_INFO_CMD;
        extra.moduleStruct = &mFEInfo[2];
        params.mvFrameParams.editItemAt(2).mvExtraParam.push_back(extra);
    }

    //----------- round 3------------------------------------------
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL P2ANode::prepareFM(QParams &params, const RequestPtr &request, P2AEnqueData &data)
{
    TRACE_FUNC_ENTER();
    (void)(request);
    Input input;
    Output output;
    MSize FMSize = mFM_FE_cfg[1].mImageSize;



    //-----------------------Previous to Current----------------------------
    //----------------------high-------------------------------
    params.mvFrameParams.push_back(FrameParams());
    input.mPortID = PortID(EPortType_Memory, EPortIndex_REGI, PORTID_IN);
    input.mBuffer = data.mFMResult.FM_A.Register_High->getImageBufferPtr();
    params.mvFrameParams.editItemAt(3).mvIn.push_back(input);

    input.mPortID = PortID(EPortType_Memory, EPortIndex_DEPI, PORTID_IN);
    input.mBuffer = data.mFMResult.PrevFE.High->getImageBufferPtr();
    params.mvFrameParams.editItemAt(3).mvIn.push_back(input);

    input.mPortID = PortID(EPortType_Memory, EPortIndex_DMGI, PORTID_IN);
    input.mBuffer = data.mFMResult.FE.High->getImageBufferPtr();
    params.mvFrameParams.editItemAt(3).mvIn.push_back(input);

    output.mPortID = PortID(EPortType_Memory, EPortIndex_MFBO, PORTID_OUT);
    output.mBuffer =  data.mFMResult.FM_A.High->getImageBufferPtr();
    params.mvFrameParams.editItemAt(3).mvOut.push_back(output);

    params.mvFrameParams.editItemAt(3).mTuningData = (MVOID*)mFEFMTuning[3];

    //tuning config(FE MODE 16)
    params.mvFrameParams.editItemAt(3).mStreamTag = ENormalStreamTag_FM;
    FMHal::configTuning_FM(FMHal::CFG_FHD, mFMInfo[0], mFM_FE_cfg[0].mImageSize, FE_MODE_16);
    {
        ExtraParam extra;
        extra.CmdIdx = EPIPE_FM_INFO_CMD;
        extra.moduleStruct = &mFMInfo[0];
        params.mvFrameParams.editItemAt(3).mvExtraParam.push_back(extra);
    }
    //----------------------high-------------------------------

    //----------------------Medium----------------------------
    params.mvFrameParams.push_back(FrameParams());
    input.mPortID = PortID(EPortType_Memory, EPortIndex_REGI, PORTID_IN);
    input.mBuffer = data.mFMResult.FM_A.Register_Medium->getImageBufferPtr();
    params.mvFrameParams.editItemAt(4).mvIn.push_back(input);

    input.mPortID = PortID(EPortType_Memory, EPortIndex_DEPI, PORTID_IN);
    input.mBuffer = data.mFMResult.PrevFE.Medium->getImageBufferPtr();
    params.mvFrameParams.editItemAt(4).mvIn.push_back(input);

    input.mPortID = PortID(EPortType_Memory, EPortIndex_DMGI, PORTID_IN);
    input.mBuffer = data.mFMResult.FE.Medium->getImageBufferPtr();
    params.mvFrameParams.editItemAt(4).mvIn.push_back(input);

    output.mPortID = PortID(EPortType_Memory, EPortIndex_MFBO, PORTID_OUT);
    output.mBuffer =  data.mFMResult.FM_A.Medium->getImageBufferPtr();
    params.mvFrameParams.editItemAt(4).mvOut.push_back(output);

    params.mvFrameParams.editItemAt(4).mTuningData = (MVOID*)mFEFMTuning[4];

    //tuning config(FE MODE 16)
    params.mvFrameParams.editItemAt(4).mStreamTag = ENormalStreamTag_FM;
    FMHal::configTuning_FM(FMHal::CFG_FHD, mFMInfo[1], FMSize, FE_MODE_16);
    {
        ExtraParam extra;
        extra.CmdIdx = EPIPE_FM_INFO_CMD;
        extra.moduleStruct = &mFMInfo[1];
        params.mvFrameParams.editItemAt(4).mvExtraParam.push_back(extra);
    }
    //----------------------Medium----------------------------

    //----------------------Low-------------------------------
    params.mvFrameParams.push_back(FrameParams());
    input.mPortID = PortID(EPortType_Memory, EPortIndex_REGI, PORTID_IN);
    input.mBuffer = data.mFMResult.FM_A.Register_Low->getImageBufferPtr();
    params.mvFrameParams.editItemAt(5).mvIn.push_back(input);

    input.mPortID = PortID(EPortType_Memory, EPortIndex_DEPI, PORTID_IN);
    input.mBuffer = data.mFMResult.PrevFE.Low->getImageBufferPtr();
    params.mvFrameParams.editItemAt(5).mvIn.push_back(input);

    input.mPortID = PortID(EPortType_Memory, EPortIndex_DMGI, PORTID_IN);
    input.mBuffer = data.mFMResult.FE.Low->getImageBufferPtr();
    params.mvFrameParams.editItemAt(5).mvIn.push_back(input);

    output.mPortID = PortID(EPortType_Memory, EPortIndex_MFBO, PORTID_OUT);
    output.mBuffer =  data.mFMResult.FM_A.Low->getImageBufferPtr();
    params.mvFrameParams.editItemAt(5).mvOut.push_back(output);

    params.mvFrameParams.editItemAt(5).mTuningData = (MVOID*)mFEFMTuning[5];

    //tuning config(FE MODE 8)
    params.mvFrameParams.editItemAt(5).mStreamTag = ENormalStreamTag_FM;
    FMHal::configTuning_FM(FMHal::CFG_FHD, mFMInfo[2], data.mFE2Img->getImageBuffer()->getImgSize(), FE_MODE_8);
    {
        ExtraParam extra;
        extra.CmdIdx = EPIPE_FM_INFO_CMD;
        extra.moduleStruct = &mFMInfo[2];
        params.mvFrameParams.editItemAt(5).mvExtraParam.push_back(extra);
    }

    //----------------------Low-------------------------------
    //-----------------------Previous to Current----------------------------


    //-----------------------Current to Previous----------------------------
    //----------------------high-------------------------------
    params.mvFrameParams.push_back(FrameParams());
    input.mPortID = PortID(EPortType_Memory, EPortIndex_REGI, PORTID_IN);
    input.mBuffer = data.mFMResult.FM_B.Register_High->getImageBufferPtr();
    params.mvFrameParams.editItemAt(6).mvIn.push_back(input);

    input.mPortID = PortID(EPortType_Memory, EPortIndex_DEPI, PORTID_IN);
    input.mBuffer = data.mFMResult.FE.High->getImageBufferPtr();
    //input.mBuffer = data.mFMResult.PrevFE.High->getImageBufferPtr();
    params.mvFrameParams.editItemAt(6).mvIn.push_back(input);

    input.mPortID = PortID(EPortType_Memory, EPortIndex_DMGI, PORTID_IN);
    input.mBuffer = data.mFMResult.PrevFE.High->getImageBufferPtr();
    //input.mBuffer = data.mFMResult.FE.High->getImageBufferPtr();
    params.mvFrameParams.editItemAt(6).mvIn.push_back(input);

    output.mPortID = PortID(EPortType_Memory, EPortIndex_MFBO, PORTID_OUT);
    output.mBuffer =  data.mFMResult.FM_B.High->getImageBufferPtr();
    params.mvFrameParams.editItemAt(6).mvOut.push_back(output);

    params.mvFrameParams.editItemAt(6).mTuningData = (MVOID*)mFEFMTuning[6];

    //tuning config(FE MODE 16)
    params.mvFrameParams.editItemAt(6).mStreamTag = ENormalStreamTag_FM;
    FMHal::configTuning_FM(FMHal::CFG_FHD, mFMInfo[3],  mFM_FE_cfg[0].mImageSize, FE_MODE_16);
    {
        ExtraParam extra;
        extra.CmdIdx = EPIPE_FM_INFO_CMD;
        extra.moduleStruct = &mFMInfo[3];
        params.mvFrameParams.editItemAt(6).mvExtraParam.push_back(extra);
    }
    //----------------------high-------------------------------

    //----------------------Medium----------------------------
    params.mvFrameParams.push_back(FrameParams());
    input.mPortID = PortID(EPortType_Memory, EPortIndex_REGI, PORTID_IN);
    input.mBuffer = data.mFMResult.FM_B.Register_Medium->getImageBufferPtr();
    params.mvFrameParams.editItemAt(7).mvIn.push_back(input);

    input.mPortID = PortID(EPortType_Memory, EPortIndex_DEPI, PORTID_IN);
    input.mBuffer = data.mFMResult.FE.Medium->getImageBufferPtr();
    //input.mBuffer = data.mFMResult.PrevFE.Medium->getImageBufferPtr();
    params.mvFrameParams.editItemAt(7).mvIn.push_back(input);

    input.mPortID = PortID(EPortType_Memory, EPortIndex_DMGI, PORTID_IN);
    input.mBuffer = data.mFMResult.PrevFE.Medium->getImageBufferPtr();
    //input.mBuffer = data.mFMResult.FE.Medium->getImageBufferPtr();
    params.mvFrameParams.editItemAt(7).mvIn.push_back(input);

    output.mPortID = PortID(EPortType_Memory, EPortIndex_MFBO, PORTID_OUT);
    output.mBuffer =  data.mFMResult.FM_B.Medium->getImageBufferPtr();
    params.mvFrameParams.editItemAt(7).mvOut.push_back(output);

    params.mvFrameParams.editItemAt(7).mTuningData = (MVOID*)mFEFMTuning[7];

    //tuning config(FE MODE 16)
    params.mvFrameParams.editItemAt(7).mStreamTag = ENormalStreamTag_FM;
    FMHal::configTuning_FM(FMHal::CFG_FHD, mFMInfo[4], FMSize, FE_MODE_16);
    {
        ExtraParam extra;
        extra.CmdIdx = EPIPE_FM_INFO_CMD;
        extra.moduleStruct = &mFMInfo[4];
        params.mvFrameParams.editItemAt(7).mvExtraParam.push_back(extra);
    }
    //----------------------Medium----------------------------

    //----------------------Low-------------------------------
    params.mvFrameParams.push_back(FrameParams());
    input.mPortID = PortID(EPortType_Memory, EPortIndex_REGI, PORTID_IN);
    input.mBuffer = data.mFMResult.FM_B.Register_Low->getImageBufferPtr();
    params.mvFrameParams.editItemAt(8).mvIn.push_back(input);

    input.mPortID = PortID(EPortType_Memory, EPortIndex_DEPI, PORTID_IN);
    input.mBuffer = data.mFMResult.FE.Low->getImageBufferPtr();
    //input.mBuffer = data.mFMResult.PrevFE.Low->getImageBufferPtr();
    params.mvFrameParams.editItemAt(8).mvIn.push_back(input);

    input.mPortID = PortID(EPortType_Memory, EPortIndex_DMGI, PORTID_IN);
    input.mBuffer = data.mFMResult.PrevFE.Low->getImageBufferPtr();
    //input.mBuffer = data.mFMResult.FE.Low->getImageBufferPtr();
    params.mvFrameParams.editItemAt(8).mvIn.push_back(input);

    output.mPortID = PortID(EPortType_Memory, EPortIndex_MFBO, PORTID_OUT);
    output.mBuffer =  data.mFMResult.FM_B.Low->getImageBufferPtr();
    params.mvFrameParams.editItemAt(8).mvOut.push_back(output);

    params.mvFrameParams.editItemAt(8).mTuningData = (MVOID*)mFEFMTuning[8];

    //tuning config(FE MODE 8)
    params.mvFrameParams.editItemAt(8).mStreamTag = ENormalStreamTag_FM;
    FMHal::configTuning_FM(FMHal::CFG_FHD, mFMInfo[5], data.mFE2Img->getImageBuffer()->getImgSize(), FE_MODE_8);
    {
        ExtraParam extra;
        extra.CmdIdx = EPIPE_FM_INFO_CMD;
        extra.moduleStruct = &mFMInfo[5];
        params.mvFrameParams.editItemAt(8).mvExtraParam.push_back(extra);
    }

    //----------------------Low-------------------------------
    //-----------------------Current to Previous----------------------------

    TRACE_FUNC_EXIT();
    return MTRUE;
}



} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam

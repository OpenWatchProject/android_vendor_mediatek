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
#define PIPE_MODULE_TAG "vsdof_util"
#define PIPE_CLASS_TAG "QParamTemplate"
#define PIPE_LOG_TAG PIPE_MODULE_TAG PIPE_CLASS_TAG
#include <featurePipe/core/include/PipeLog.h>

#include "QParamTemplate.h"

using namespace NSCam::Utils;
using android::sp;

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  QParamTemplateGenerator class
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QParamTemplateGenerator::
QParamTemplateGenerator()
{
}

QParamTemplateGenerator::
QParamTemplateGenerator(MUINT32 frameID, MUINT32 iSensorIdx, ENormalStreamTag streamTag)
: miFrameID(frameID)
{
    mPass2EnqueFrame.mStreamTag = streamTag;
    mPass2EnqueFrame.mSensorIdx = iSensorIdx;
}

QParamTemplateGenerator&
QParamTemplateGenerator::
addCrop(eCropGroup groupID, MPoint startLoc, MSize cropSize, MSize resizeDst, bool isMDPCrop)
{
    MCrpRsInfo cropInfo;
    cropInfo.mGroupID = (MUINT32) groupID;
    cropInfo.mCropRect.p_fractional.x=0;
    cropInfo.mCropRect.p_fractional.y=0;
    cropInfo.mCropRect.p_integral.x=startLoc.x;
    cropInfo.mCropRect.p_integral.y=startLoc.y;
    cropInfo.mCropRect.s=cropSize;
    cropInfo.mResizeDst=resizeDst;
    cropInfo.mMdpGroup = (isMDPCrop) ? 1 : 0;
    mPass2EnqueFrame.mvCropRsInfo.push_back(cropInfo);

    return *this;
}

QParamTemplateGenerator&
QParamTemplateGenerator::
addInput(NSCam::NSIoPipe::PortID portID)
{
    Input src;
    src.mPortID = portID;
    src.mBuffer = NULL;
    mPass2EnqueFrame.mvIn.push_back(src);

    return *this;
}

QParamTemplateGenerator &
QParamTemplateGenerator::
addOutput(NSCam::NSIoPipe::PortID portID, MINT32 transform)
{
    Output out;
    out.mPortID = portID;
    out.mTransform = transform;
    out.mBuffer = NULL;
    mPass2EnqueFrame.mvOut.push_back(out);

    return *this;
}


QParamTemplateGenerator &
QParamTemplateGenerator::
addExtraParam(EPostProcCmdIndex cmdIdx, MVOID* param)
{
    ExtraParam extra;
    extra.CmdIdx = cmdIdx;
    extra.moduleStruct = param;
    mPass2EnqueFrame.mvExtraParam.push_back(extra);

    return *this;
}

QParamTemplateGenerator&
QParamTemplateGenerator::addModuleInfo(MUINT32 moduleTag, MVOID* moduleStruct)
{
    ModuleInfo moduleInfo;
    moduleInfo.moduleTag = moduleTag;
    moduleInfo.moduleStruct = moduleStruct;
    mPass2EnqueFrame.mvModuleData.push_back(moduleInfo);
    return *this;
}

MBOOL
QParamTemplateGenerator::
generate(QParams& rQParam)
{
    if(checkValid())
    {
        rQParam.mvFrameParams.push_back(mPass2EnqueFrame);
        return MTRUE;
    }
    else
        return MFALSE;
}

MBOOL
QParamTemplateGenerator::checkValid()
{
    bool bAllSuccess = MTRUE;

    // check in/out size
    if((mPass2EnqueFrame.mvIn.size() == 0 &&  mPass2EnqueFrame.mvOut.size() != 0) ||
        (mPass2EnqueFrame.mvIn.size() != 0 &&  mPass2EnqueFrame.mvOut.size() == 0))
    {
        MY_LOGE("FrameID:%d In/Out buffer size is not consistent, in:%d out:%d",
                    miFrameID, mPass2EnqueFrame.mvIn.size(), mPass2EnqueFrame.mvOut.size());
        bAllSuccess = MFALSE;
    }

    // for each mvOut, check the corresponding crop setting is ready
    for(Output output: mPass2EnqueFrame.mvOut)
    {
        MINT32 findTarget;
        if(output.mPortID.index == NSImageio::NSIspio::EPortIndex_WROTO)
            findTarget = (MINT32)eCROP_WROT;
        else if(output.mPortID.index == NSImageio::NSIspio::EPortIndex_WDMAO)
            findTarget = (MINT32)eCROP_WDMA;
        else if(output.mPortID.index == NSImageio::NSIspio::EPortIndex_IMG2O)
            findTarget = (MINT32)eCROP_CRZ;
        else
            continue;

        bool bIsFind = MFALSE;
        for(MCrpRsInfo rsInfo : mPass2EnqueFrame.mvCropRsInfo)
        {
            if(rsInfo.mGroupID == findTarget)
            {
                bIsFind = MTRUE;
                break;
            }
        }
        if(!bIsFind)
        {
            MY_LOGE("FrameID:%d has output buffer with portID=%d, but has no the needed crop:%d",
                    miFrameID, output.mPortID.index, findTarget);
            bAllSuccess = MFALSE;
        }
    }

    // check duplicated input port
    for(Input input: mPass2EnqueFrame.mvIn)
    {
        bool bSuccess = MTRUE;
        int count = 0;
        for(Input chkIn : mPass2EnqueFrame.mvIn)
        {
            if(chkIn.mPortID.index == input.mPortID.index)
                count++;

            if(count>1)
            {
                bSuccess = MFALSE;
                MY_LOGE("FrameID=%d, Duplicated mvIn portID:%d!!", miFrameID, chkIn.mPortID.index);
                break;
            }
        }
        if(!bSuccess)
        {
            bAllSuccess = MFALSE;
        }
    }

    // check duplicated output port
    for(Output output: mPass2EnqueFrame.mvOut)
    {
        bool bSuccess = MTRUE;
        int count = 0;
        for(Output chkOut : mPass2EnqueFrame.mvOut)
        {
            if(chkOut.mPortID.index == output.mPortID.index)
                count++;

            if(count>1)
            {
                bSuccess = MFALSE;
                MY_LOGE("FrameID=%d, Duplicated mvOut portID:%d!!", miFrameID, chkOut.mPortID.index);
                break;
            }
        }
        if(!bSuccess)
        {
            bAllSuccess = MFALSE;
        }
    }

    return bAllSuccess;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  QParamTemplateFiller class
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QParamTemplateFiller::
QParamTemplateFiller(
    QParams& target
)
: mTargetQParam(target)
, mbSuccess(MTRUE)
{}

QParamTemplateFiller&
QParamTemplateFiller::insertTuningBuf(
    MUINT frameID,
    MVOID* pTuningBuf
)
{
    FrameParams& frameParam = mTargetQParam.mvFrameParams.editItemAt(frameID);
    frameParam.mTuningData = pTuningBuf;
    return *this;
}

QParamTemplateFiller&
QParamTemplateFiller::delOutputPort(
    MUINT frameID,
    NSCam::NSIoPipe::PortID portID,
    eCropGroup cropGID
)
{
    FrameParams& frameParam = mTargetQParam.mvFrameParams.editItemAt(frameID);

    // del output
    size_t indexToDel = -1;
    for(size_t index = 0; index<frameParam.mvOut.size(); ++index)
    {
        if(frameParam.mvOut[index].mPortID.index == portID.index)
        {
            indexToDel = index;
            break;
        }
    }
    if(indexToDel != -1)
        frameParam.mvOut.removeAt(indexToDel);

    // del crop
    indexToDel = -1;
    for(size_t index = 0; index<frameParam.mvCropRsInfo.size(); ++index)
    {
        if(frameParam.mvCropRsInfo[index].mGroupID == (MINT32)cropGID)
        {
            indexToDel = index;
            break;
        }
    }
    if(indexToDel != -1)
        frameParam.mvCropRsInfo.removeAt(indexToDel);

    return *this;
}

QParamTemplateFiller&
QParamTemplateFiller::insertInputBuf(
    MUINT frameID,
    NSCam::NSIoPipe::PortID portID,
    sp<EffectFrameInfo> pFrameInfo
)
{
    sp<IImageBuffer> pImgBuf;
    pFrameInfo->getFrameBuffer(pImgBuf);
    this->insertInputBuf(frameID, portID, pImgBuf.get());
    return *this;
}

QParamTemplateFiller&
QParamTemplateFiller::insertInputBuf(
    MUINT frameID,
    NSCam::NSIoPipe::PortID portID,
    IImageBuffer* pImgBuf
)
{
    FrameParams& frameParam = mTargetQParam.mvFrameParams.editItemAt(frameID);

    MBOOL bFound = MFALSE;
    for(Input& input: frameParam.mvIn)
    {
        if(input.mPortID.index == portID.index)
        {
            input.mBuffer = pImgBuf;
            bFound = MTRUE;
            break;
        }
    }

    if(!bFound)
    {
        MY_LOGE("Error, cannot find the mvIn to insert buffer, frameID=%d portID=%d ", frameID, portID.index);
        mbSuccess = MFALSE;
    }

    return *this;
}

QParamTemplateFiller&
QParamTemplateFiller::insertOutputBuf(
    MUINT frameID,
    NSCam::NSIoPipe::PortID portID,
    sp<EffectFrameInfo> pFrameInfo
)
{
    sp<IImageBuffer> pImgBuf;
    pFrameInfo->getFrameBuffer(pImgBuf);
    this->insertOutputBuf(frameID, portID, pImgBuf.get());
    return *this;
}


QParamTemplateFiller&
QParamTemplateFiller::insertOutputBuf(
    MUINT frameID,
    NSCam::NSIoPipe::PortID portID,
    IImageBuffer* pImgBuf
)
{
    FrameParams& frameParam = mTargetQParam.mvFrameParams.editItemAt(frameID);

    MBOOL bFound = MFALSE;
    for(Output& output: frameParam.mvOut)
    {
        if(output.mPortID.index == portID.index)
        {
            output.mBuffer = pImgBuf;
            bFound = MTRUE;
            break;
        }
    }

    if(!bFound)
    {
        MY_LOGE("Error, cannot find the mvOut to insert buffer, frameID=%d portID=%d ", frameID, portID.index);
        mbSuccess = MFALSE;
    }

    return *this;
}

QParamTemplateFiller&
QParamTemplateFiller::setCrop(
    MUINT frameID,
    eCropGroup groupID,
    MPoint startLoc,
    MSize cropSize,
    MSize resizeDst,
    bool isMDPCrop
)
{
    FrameParams& frameParam = mTargetQParam.mvFrameParams.editItemAt(frameID);

    MBOOL bFound = MFALSE;
    for(MCrpRsInfo& cropInfo: frameParam.mvCropRsInfo)
    {
        if(cropInfo.mGroupID == (MINT32)groupID)
        {
            cropInfo.mGroupID = (MUINT32) groupID;
            cropInfo.mCropRect.p_fractional.x=0;
            cropInfo.mCropRect.p_fractional.y=0;
            cropInfo.mCropRect.p_integral.x=startLoc.x;
            cropInfo.mCropRect.p_integral.y=startLoc.y;
            cropInfo.mCropRect.s=cropSize;
            cropInfo.mResizeDst=resizeDst;
            cropInfo.mMdpGroup = (isMDPCrop) ? 1 : 0;
            bFound = MTRUE;
            break;
        }
    }

    if(!bFound)
    {
         MY_LOGE("Error, cannot find the crop info to config, frameID=%d groupID=%d ", frameID, groupID);
        mbSuccess = MFALSE;
    }

    return *this;
}

QParamTemplateFiller&
QParamTemplateFiller::setCropResize(
    MUINT frameID,
    eCropGroup groupID,
    MSize resizeDst
)
{
    FrameParams& frameParam = mTargetQParam.mvFrameParams.editItemAt(frameID);

    MBOOL bFound = MFALSE;
    for(MCrpRsInfo& cropInfo: frameParam.mvCropRsInfo)
    {
        if(cropInfo.mGroupID == (MINT32)groupID)
        {
            cropInfo.mGroupID = (MUINT32) groupID;
            cropInfo.mResizeDst=resizeDst;
            bFound = MTRUE;
            break;
        }
    }

    if(!bFound)
    {
         MY_LOGE("Error, cannot find the crop info to config, frameID=%d groupID=%d ", frameID, groupID);
        mbSuccess = MFALSE;
    }

    return *this;
}

QParamTemplateFiller&
QParamTemplateFiller::setExtOffset(
    MUINT frameID,
    PortID portID,
    MINT32 offsetInBytes
)
{
    FrameParams& frameParam = mTargetQParam.mvFrameParams.editItemAt(frameID);

    MBOOL bFound = MFALSE;
    for(Output& output: frameParam.mvOut)
    {
        if(output.mPortID.index == portID.index)
        {
            output.mOffsetInBytes = offsetInBytes;
            bFound = MTRUE;
            break;
        }
    }

    if(!bFound)
    {
        MY_LOGE("Error, cannot find the mvOut to setExtOffset, frameID=%d portID=%d ", frameID, portID.index);
        mbSuccess = MFALSE;
    }

    return *this;
}

QParamTemplateFiller&
QParamTemplateFiller::setInfo(
    MUINT frameID,
    MUINT32 frameNo,
    MUINT32 requestNo,
    MUINT32 timestamp
)
{
    FrameParams& frameParam = mTargetQParam.mvFrameParams.editItemAt(frameID);

    frameParam.FrameNo = frameNo;
    frameParam.RequestNo = requestNo;
    frameParam.Timestamp = timestamp;

    return *this;
}

QParamTemplateFiller&
QParamTemplateFiller::delInputPort(
    MUINT frameID,
    NSCam::NSIoPipe::PortID portID
)
{
    FrameParams& frameParam = mTargetQParam.mvFrameParams.editItemAt(frameID);

    // del output
    size_t indexToDel = -1;
    for(size_t index = 0; index<frameParam.mvIn.size(); ++index)
    {
        if(frameParam.mvIn[index].mPortID.index == portID.index)
        {
            indexToDel = index;
            break;
        }
    }
    if(indexToDel != -1)
        frameParam.mvIn.removeAt(indexToDel);

    return *this;
}

MBOOL
QParamTemplateFiller::validate()
{
    return mbSuccess;
}

} //NSFeaturePipe
} //NSCamFeature
} //NSCam
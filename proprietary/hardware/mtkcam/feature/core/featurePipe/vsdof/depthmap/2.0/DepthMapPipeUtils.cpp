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

#include <mtkcam/feature/stereo/pipe/IDepthMapPipe.h>
#include <mtkcam/feature/eis/eis_ext.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>

#include "DepthMapPipeUtils.h"
#include "DepthMapPipe_Common.h"
#include <stereo_tuning_provider.h>

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {

static bool do_mkdir(char const*const path, uint_t const mode)
{
    struct stat st;
    //
    if  ( 0 != ::stat(path, &st) )
    {
        //  Directory does not exist.
        if  ( 0 != ::mkdir(path, mode) && EEXIST != errno )
        {
            MY_LOGE("fail to mkdir [%s]: %d[%s]", path, errno, ::strerror(errno));
            return  false;
        }
    }
    else if ( ! S_ISDIR(st.st_mode) )
    {
        MY_LOGE("!S_ISDIR");
        return  false;
    }
    //
    return  true;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  QParamTemplateGenerator class
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QParamTemplateGenerator::
QParamTemplateGenerator(MUINT32 frameID, MUINT32 iSensorIdx, ENormalStreamTag streamTag)
: miFrameID(frameID)
{
    mPass2EnqueFrame.mStreamTag = streamTag;
    mPass2EnqueFrame.mSensorIdx = iSensorIdx;
}

QParamTemplateGenerator&
QParamTemplateGenerator::
addCrop(eCropGroup groupID, MPoint startLoc, MSize cropSize, MSize resizeDst)
{
    MCrpRsInfo cropInfo;
    cropInfo.mGroupID = (MUINT32) groupID;
    cropInfo.mCropRect.p_fractional.x=0;
    cropInfo.mCropRect.p_fractional.y=0;
    cropInfo.mCropRect.p_integral.x=startLoc.x;
    cropInfo.mCropRect.p_integral.y=startLoc.y;
    cropInfo.mCropRect.s=cropSize;
    cropInfo.mResizeDst=resizeDst;
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
addOutput(
    NSCam::NSIoPipe::PortID portID,
    MINT32 transform,
    EPortCapbility cap
)
{
    Output out;
    out.mPortID = portID;
    out.mPortID.capbility = cap;
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
QParamTemplateFiller::setOutputTransform(
    MUINT frameID,
    NSCam::NSIoPipe::PortID portID,
    MINT32 transform
)
{
    FrameParams& frameParam = mTargetQParam.mvFrameParams.editItemAt(frameID);
    MBOOL bFound = MFALSE;
    for(Output& output: frameParam.mvOut)
    {
        if(output.mPortID.index == portID.index)
        {
            output.mTransform = transform;
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
            cropInfo.mCropRect.p_fractional.x=0;
            cropInfo.mCropRect.p_fractional.y=0;
            cropInfo.mCropRect.p_integral.x=startLoc.x;
            cropInfo.mCropRect.p_integral.y=startLoc.y;
            cropInfo.mCropRect.s=cropSize;
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

MBOOL
QParamTemplateFiller::validate()
{
    return mbSuccess;
}

QParamTemplateFiller&
QParamTemplateFiller::
addExtraParam(
    MUINT frameID,
    EPostProcCmdIndex cmdIdx,
    MVOID* param
)
{
    FrameParams& frameParam = mTargetQParam.mvFrameParams.editItemAt(frameID);
    ExtraParam extra;
    extra.CmdIdx = cmdIdx;
    extra.moduleStruct = param;
    frameParam.mvExtraParam.push_back(extra);

    return *this;
}

//  ============================  EIS utility  ============================
MBOOL
queryEisRegion(
    IMetadata* const inHal,
    eis_region& region
)
{
    IMetadata::IEntry entry = inHal->entryFor(MTK_EIS_REGION);
    // get EIS's region
    if (entry.count() > 5)
    {
        region.x_int        = entry.itemAt(0, Type2Type<MINT32>());
        region.x_float      = entry.itemAt(1, Type2Type<MINT32>());
        region.y_int        = entry.itemAt(2, Type2Type<MINT32>());
        region.y_float      = entry.itemAt(3, Type2Type<MINT32>());
        region.s.w          = entry.itemAt(4, Type2Type<MINT32>());
        region.s.h          = entry.itemAt(5, Type2Type<MINT32>());

       return MTRUE;
    }
    MY_LOGE("wrong eis region count %zu", entry.count());
    return MFALSE;
}

/**
 * Check EIS is on or off
 * @param[in]  P1 HalMeta
 *
 * @return
 *  -  [bool] on/off status
 */

MBOOL
isEISOn(
    IMetadata* const inApp
)
{
    MUINT8 eisMode = MTK_CONTROL_VIDEO_STABILIZATION_MODE_OFF;
    if( !tryGetMetadata<MUINT8>(inApp, MTK_CONTROL_VIDEO_STABILIZATION_MODE, eisMode) ) {
        MY_LOGD("no MTK_CONTROL_VIDEO_STABILIZATION_MODE");
    }

    return eisMode == MTK_CONTROL_VIDEO_STABILIZATION_MODE_ON;
}

MVOID debugQParams(const QParams& rInputQParam)
{
    if(!DepthPipeLoggingSetup::mbDebugLog)
        return;

    MY_LOGD("Frame size = %d", rInputQParam.mvFrameParams.size());

    for(size_t index = 0; index < rInputQParam.mvFrameParams.size(); ++index)
    {
        auto& frameParam = rInputQParam.mvFrameParams.itemAt(index);
        MY_LOGD("=================================================");
        MY_LOGD("Frame index = %d", index);
        MY_LOGD("mStreamTag=%d mSensorIdx=%d", frameParam.mStreamTag, frameParam.mSensorIdx);

        MY_LOGD("=== mvIn section ===");
        for(size_t index2=0;index2<frameParam.mvIn.size();++index2)
        {
            Input data = frameParam.mvIn[index2];
            MY_LOGD("Index = %d", index2);
            MY_LOGD("mvIn.PortID.index = %d", data.mPortID.index);
            MY_LOGD("mvIn.PortID.type = %d", data.mPortID.type);
            MY_LOGD("mvIn.PortID.inout = %d", data.mPortID.inout);

            MY_LOGD("mvIn.mBuffer=%x", data.mBuffer);
            if(data.mBuffer !=NULL)
            {
                MY_LOGD("mvIn.mBuffer->getImgSize = %dx%d", data.mBuffer->getImgSize().w,
                                                    data.mBuffer->getImgSize().h);

                MY_LOGD("mvIn.mBuffer->getImgFormat = %x", data.mBuffer->getImgFormat());
                MY_LOGD("mvIn.mBuffer->getPlaneCount = %d", data.mBuffer->getPlaneCount());
                for(int j=0;j<data.mBuffer->getPlaneCount();j++)
                {
                    MY_LOGD("mvIn.mBuffer->getBufVA(%d) = %X", j, data.mBuffer->getBufVA(j));
                    MY_LOGD("mvIn.mBuffer->getBufStridesInBytes(%d) = %d", j, data.mBuffer->getBufStridesInBytes(j));
                }
            }
            else
            {
                MY_LOGD("mvIn.mBuffer is NULL!!");
            }


            MY_LOGD("mvIn.mTransform = %d", data.mTransform);
        }

        MY_LOGD("=== mvOut section ===");
        for(size_t index2=0;index2<frameParam.mvOut.size(); index2++)
        {
            Output data = frameParam.mvOut[index2];
            MY_LOGD("Index = %d", index2);

            MY_LOGD("mvOut.PortID.index = %d", data.mPortID.index);
            MY_LOGD("mvOut.PortID.type = %d", data.mPortID.type);
            MY_LOGD("mvOut.PortID.inout = %d", data.mPortID.inout);

            MY_LOGD("mvOut.mBuffer=%x", data.mBuffer);
            if(data.mBuffer != NULL)
            {
                MY_LOGD("mvOut.mBuffer->getImgSize = %dx%d", data.mBuffer->getImgSize().w,
                                                    data.mBuffer->getImgSize().h);

                MY_LOGD("mvOut.mBuffer->getImgFormat = %x", data.mBuffer->getImgFormat());
                MY_LOGD("mvOut.mBuffer->getPlaneCount = %d", data.mBuffer->getPlaneCount());
                for(size_t j=0;j<data.mBuffer->getPlaneCount();j++)
                {
                    MY_LOGD("mvOut.mBuffer->getBufVA(%d) = %X", j, data.mBuffer->getBufVA(j));
                    MY_LOGD("mvOut.mBuffer->getBufStridesInBytes(%d) = %d", j, data.mBuffer->getBufStridesInBytes(j));
                }
            }
            else
            {
                MY_LOGD("mvOut.mBuffer is NULL!!");
            }
            MY_LOGD("mvOut.mTransform = %d", data.mTransform);
        }

        MY_LOGD("=== mvCropRsInfo section ===");
        for(size_t i=0;i<frameParam.mvCropRsInfo.size(); i++)
        {
            MCrpRsInfo data = frameParam.mvCropRsInfo[i];
            MY_LOGD("Index = %d", i);
            MY_LOGD("CropRsInfo.mGroupID=%d", data.mGroupID);
            MY_LOGD("CropRsInfo.mResizeDst=%dx%d", data.mResizeDst.w, data.mResizeDst.h);
            MY_LOGD("CropRsInfo.mCropRect.p_fractional=(%d,%d) ", data.mCropRect.p_fractional.x, data.mCropRect.p_fractional.y);
            MY_LOGD("CropRsInfo.mCropRect.p_integral=(%d,%d) ", data.mCropRect.p_integral.x, data.mCropRect.p_integral.y);
            MY_LOGD("CropRsInfo.mCropRect.s=%dx%d ", data.mCropRect.s.w, data.mCropRect.s.h);
        }

        MY_LOGD("=== mvModuleData section ===");
        for(size_t i=0;i<frameParam.mvModuleData.size(); i++)
        {
            ModuleInfo data = frameParam.mvModuleData[i];
            MY_LOGD("Index = %d", i);
            MY_LOGD("ModuleData.moduleTag=%d", data.moduleTag);

            _SRZ_SIZE_INFO_ *SrzInfo = (_SRZ_SIZE_INFO_ *) data.moduleStruct;
            MY_LOGD("SrzInfo->in_w=%d", SrzInfo->in_w);
            MY_LOGD("SrzInfo->in_h=%d", SrzInfo->in_h);
            MY_LOGD("SrzInfo->crop_w=%d", SrzInfo->crop_w);
            MY_LOGD("SrzInfo->crop_h=%d", SrzInfo->crop_h);
            MY_LOGD("SrzInfo->crop_x=%d", SrzInfo->crop_x);
            MY_LOGD("SrzInfo->crop_y=%d", SrzInfo->crop_y);
            MY_LOGD("SrzInfo->crop_floatX=%d", SrzInfo->crop_floatX);
            MY_LOGD("SrzInfo->crop_floatY=%d", SrzInfo->crop_floatY);
            MY_LOGD("SrzInfo->out_w=%d", SrzInfo->out_w);
            MY_LOGD("SrzInfo->out_h=%d", SrzInfo->out_h);
        }
        MY_LOGD("TuningData=%x", frameParam.mTuningData);
        MY_LOGD("=== mvExtraData section ===");
        for(size_t i=0;i<frameParam.mvExtraParam.size(); i++)
        {
            auto extraParam = frameParam.mvExtraParam[i];
            if(extraParam.CmdIdx == EPIPE_FE_INFO_CMD)
            {
                FEInfo *feInfo = (FEInfo*) extraParam.moduleStruct;
                MY_LOGD("mFEDSCR_SBIT=%d  mFETH_C=%d  mFETH_G=%d", feInfo->mFEDSCR_SBIT, feInfo->mFETH_C, feInfo->mFETH_G);
                MY_LOGD("mFEFLT_EN=%d  mFEPARAM=%d  mFEMODE=%d", feInfo->mFEFLT_EN, feInfo->mFEPARAM, feInfo->mFEMODE);
                MY_LOGD("mFEYIDX=%d  mFEXIDX=%d  mFESTART_X=%d", feInfo->mFEYIDX, feInfo->mFEXIDX, feInfo->mFESTART_X);
                MY_LOGD("mFESTART_Y=%d  mFEIN_HT=%d  mFEIN_WD=%d", feInfo->mFESTART_Y, feInfo->mFEIN_HT, feInfo->mFEIN_WD);

            }
            else if(extraParam.CmdIdx == EPIPE_FM_INFO_CMD)
            {
                FMInfo *fmInfo = (FMInfo*) extraParam.moduleStruct;
                MY_LOGD("mFMHEIGHT=%d  mFMWIDTH=%d  mFMSR_TYPE=%d", fmInfo->mFMHEIGHT, fmInfo->mFMWIDTH, fmInfo->mFMSR_TYPE);
                MY_LOGD("mFMOFFSET_X=%d  mFMOFFSET_Y=%d  mFMRES_TH=%d", fmInfo->mFMOFFSET_X, fmInfo->mFMOFFSET_Y, fmInfo->mFMRES_TH);
                MY_LOGD("mFMSAD_TH=%d  mFMMIN_RATIO=%d", fmInfo->mFMSAD_TH, fmInfo->mFMMIN_RATIO);
            }
            else if(extraParam.CmdIdx == EPIPE_MDP_PQPARAM_CMD)
            {
                PQParam* param = reinterpret_cast<PQParam*>(extraParam.moduleStruct);
                if(param->WDMAPQParam != nullptr)
                {
                    DpPqParam* dpPqParam = (DpPqParam*)param->WDMAPQParam;
                    DpIspParam& ispParam = dpPqParam->u.isp;
                    VSDOFParam& vsdofParam = dpPqParam->u.isp.vsdofParam;
                    MY_LOGD("WDMAPQParam %x enable = %d, scenario=%d", dpPqParam, dpPqParam->enable, dpPqParam->scenario);
                    MY_LOGD("WDMAPQParam iso = %d, frameNo=%d requestNo=%d", ispParam.iso , ispParam.frameNo, ispParam.requestNo);
                    MY_LOGD("WDMAPQParam lensId = %d, isRefocus=%d defaultUpTable=%d",
                            ispParam.lensId , vsdofParam.isRefocus, vsdofParam.defaultUpTable);
                    MY_LOGD("WDMAPQParam defaultDownTable = %d, IBSEGain=%d", vsdofParam.defaultDownTable, vsdofParam.IBSEGain);
                }
                if(param->WROTPQParam != nullptr)
                {
                    DpPqParam* dpPqParam = (DpPqParam*)param->WROTPQParam;
                    DpIspParam&ispParam = dpPqParam->u.isp;
                    VSDOFParam& vsdofParam = dpPqParam->u.isp.vsdofParam;
                    MY_LOGD("WROTPQParam %x enable = %d, scenario=%d", dpPqParam, dpPqParam->enable, dpPqParam->scenario);
                    MY_LOGD("WROTPQParam iso = %d, frameNo=%d requestNo=%d", ispParam.iso , ispParam.frameNo, ispParam.requestNo);
                    MY_LOGD("WROTPQParam lensId = %d, isRefocus=%d defaultUpTable=%d",
                            ispParam.lensId , vsdofParam.isRefocus, vsdofParam.defaultUpTable);
                    MY_LOGD("WROTPQParam defaultDownTable = %d, IBSEGain=%d", vsdofParam.defaultDownTable, vsdofParam.IBSEGain);
                }

            }
        }
    }
}

MINT32 remapTransform(ENUM_ROTATION rot)
{
    switch(rot)
    {
        case eRotate_0:
            return 0;
        case eRotate_90:
            return eTransform_ROT_90;
        case eRotate_180:
            return eTransform_ROT_180;
        case eRotate_270:
            return eTransform_ROT_270;
        default:
            MY_LOGE("Not support module rotation =%d", rot);
            return -1;
    }
}

MBOOL
configureDpPqParam(
    sp<DepthMapEffectRequest> pRequest,
    uint32_t sensorDevId,
    DpPqParam& dpPQParam
)
{
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    IMetadata* inHal = pBufferHandler->requestMetadata(eDPETHMAP_PIPE_NODEID_P2AFM, BID_META_IN_HAL_MAIN1);
    MINT32 iso;
    if(!tryGetMetadata<MINT32>(inHal, MTK_VSDOF_P1_MAIN1_ISO, iso))
    {
        MY_LOGE("Failed to get ISO value. Meta=%x", inHal);
        return MFALSE;
    }
    MINT64 timestamp;
    if (!tryGetMetadata<MINT64>(inHal, MTK_P1NODE_FRAME_START_TIMESTAMP, timestamp))
    {
        MY_LOGE("Failed to get timestamp value.");
        return MFALSE;
    }

    dpPQParam.scenario = (pRequest->getRequestAttr().opState == eSTATE_CAPTURE) ?
                            MEDIA_ISP_CAPTURE : MEDIA_ISP_PREVIEW;
    dpPQParam.enable = PQ_REFOCUS_EN;
    dpPQParam.u.isp.timestamp = timestamp;
    dpPQParam.u.isp.iso = iso;
    dpPQParam.u.isp.frameNo = pRequest->getRequestNo();
    dpPQParam.u.isp.requestNo = pRequest->getRequestNo();
    dpPQParam.u.isp.lensId = sensorDevId;

    MY_LOGD("scenario=%d enable=%d timestamp=%ld iso=%d frameNo=%d requestNo=%d lensId=%d",
            dpPQParam.scenario, dpPQParam.enable, dpPQParam.u.isp.timestamp, dpPQParam.u.isp.iso,
            dpPQParam.u.isp.frameNo, dpPQParam.u.isp.requestNo, dpPQParam.u.isp.lensId);
    return MTRUE;
}

}; // namespace NSFeaturePipe_DepthMap
}; // namespace NSCamFeature
}; // namespace NSCam
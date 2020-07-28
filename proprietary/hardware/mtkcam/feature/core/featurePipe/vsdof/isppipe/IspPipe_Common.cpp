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
 * MediaTek Inc. (C) 2017. All rights reserved.
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


// Standard C header file
#include <string>
#include <stdarg.h>
// Android system/core header file

// mtkcam custom header file

// mtkcam global header file
#include <common/vsdof/hal/ProfileUtil.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/drv/def/Dip_Notify_datatype.h>
#include <mtkcam/drv/def/IPostProcDef.h>
#include <DpDataType.h>
// Module header file

// Local header file
#include "IspPipe_Common.h"

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace DualCamThirdParty {

using namespace NSCam::NSIoPipe::NSPostProc;
using std::string;
using NSCam::NSIoPipe::MCropRect;
//static variables
MBOOL IspPipeLoggingSetup::mbProfileLog = MFALSE;
MBOOL IspPipeLoggingSetup::mbDebugLog = MFALSE;

std::string
fnPrefix_lambda(const string &tag, int reqID=-1)
{
    if(reqID == -1)
        return "[" + tag + "] ";
    else
        return "[" + tag + string("] [reqID=") + std::to_string(reqID) + "] ";
}

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

AutoProfileLogging::
AutoProfileLogging(
    const char* blockName,
    int reqID,
    const char* fmt,
    ...
)
: mReqID(reqID)
, msTag(blockName)
{
    mbForceError = ::property_get_int32("vendor.debug.isp.pipe.force_error", 0);

    char log[1024];
    string camTrace = fnPrefix_lambda(msTag, reqID);
    string logPrefix= fnPrefix_lambda(msTag, reqID) + "+ , " + fmt;
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(log, 1024, logPrefix.c_str(), ap);
    va_end(ap);
    // logging
    if(mbForceError)
        CAM_LOGE("%s", log);
    else if(IspPipeLoggingSetup::mbProfileLog || IspPipeLoggingSetup::mbDebugLog)
        CAM_LOGD("%s", log);
    // camtrace
    CAM_TRACE_BEGIN(camTrace.c_str());
}

AutoProfileLogging::
AutoProfileLogging(
    const char* blockName,
    int reqID
)
: mReqID(reqID)
, msTag(blockName)
{
    mbForceError = ::property_get_int32("vendor.debug.isp.pipe.force_error", 0);
    string log = fnPrefix_lambda(msTag, reqID) + "+";
    // logging
    if(mbForceError)
        CAM_LOGE("%s", log.c_str());
    else if(IspPipeLoggingSetup::mbProfileLog || IspPipeLoggingSetup::mbDebugLog)
        CAM_LOGD("%s", log.c_str());
    // camtrace
    CAM_TRACE_BEGIN(log.c_str());
}

AutoProfileLogging::
AutoProfileLogging(
    const char* blockName
)
: mReqID(-1)
, msTag(blockName)
{
    mbForceError = ::property_get_int32("vendor.debug.isp.pipe.force_error", 0);
    string log = fnPrefix_lambda(msTag) + "+";
    // logging
    if(mbForceError)
        CAM_LOGE("%s", log.c_str());
    else if(IspPipeLoggingSetup::mbProfileLog || IspPipeLoggingSetup::mbDebugLog)
        CAM_LOGD("%s", log.c_str());
    // camtrace
    CAM_TRACE_BEGIN(log.c_str());
}

AutoProfileLogging::
~AutoProfileLogging()
{
    CAM_TRACE_END();
    string log = fnPrefix_lambda(msTag) + "-";
    if(mbForceError)
        CAM_LOGE("%s", log.c_str());
    else if(IspPipeLoggingSetup::mbProfileLog || IspPipeLoggingSetup::mbDebugLog)
        CAM_LOGD("%s", log.c_str());
}

MVOID
AutoProfileLogging::
logging(const char* fmt, ...)
{
    char log[1024];
    string logPrefix= fnPrefix_lambda(msTag, mReqID) + fmt;
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(log, 1024, logPrefix.c_str(), ap);
    va_end(ap);
    // logging
    if(mbForceError)
        CAM_LOGE("%s", log);
    else if(IspPipeLoggingSetup::mbProfileLog || IspPipeLoggingSetup::mbDebugLog)
        CAM_LOGD("%s", log);
}

MVOID debugQParams(const QParams& rInputQParam)
{
    if(!IspPipeLoggingSetup::mbDebugLog)
        return;

    MY_LOGD("debugQParams start");
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

MBOOL
calcDstImageCrop(
    MSize const &srcSize,
    MSize const &dstSize,
    MCropRect &result
)
{
    PIPE_LOGD("srcSize=%dx%d dstSize=%dx%d",
                srcSize.w, srcSize.h, dstSize.w, dstSize.h);

    if(srcSize.h*dstSize.w>dstSize.h*srcSize.w)
    {
        // align width
        int crop_height = (1.0*dstSize.h/dstSize.w) * srcSize.w;
        crop_height &= ~(0x1);
        result.s = MSize(srcSize.w, crop_height);
        result.p_integral = MPoint(0, abs(srcSize.h-crop_height)/2);
        PIPE_LOGD("Align width: crop_height=%d crop=(%d,%d) size=%dx%d"
                , crop_height, result.p_integral.x, result.p_integral.y,
                result.s.w, result.s.h);
    }
    else
    {
        // align height
        int crop_width = (1.0*dstSize.w/dstSize.h) * srcSize.h;
        crop_width &= ~(0x1);
        result.s = MSize(crop_width, srcSize.h);
        result.p_integral = MPoint(abs(srcSize.w-crop_width)/2, 0);
        PIPE_LOGD("Align height: crop_width=%d crop=(%d,%d) size=%dx%d"
                , crop_width, result.p_integral.x, result.p_integral.y,
                result.s.w, result.s.h);
    }

    return MTRUE;
}


}; // DualCamThirdParty
}; // NSFeaturePipe
}; // NSCamFeature
}; // NSCam

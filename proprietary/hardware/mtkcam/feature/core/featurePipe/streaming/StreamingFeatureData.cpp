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

#include "StreamingFeatureData.h"
#include "StreamingFeature_Common.h"
#include <camera_custom_eis.h>
#include <camera_custom_dualzoom.h>

#include <utility>

#define PIPE_CLASS_TAG "Data"
#define PIPE_TRACE TRACE_STREAMING_FEATURE_DATA
#include <featurePipe/core/include/PipeLog.h>
#include <mtkcam/feature/DualCam/DualCam.Common.h>

using NSCam::NSIoPipe::QParams;
using NSCam::NSIoPipe::MCropRect;
using NSCam::FSC::FSC_CROPPING_RESULT_STRUCT;

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

std::unordered_map<MUINT32, std::string> StreamingFeatureRequest::mFeatureMaskNameMap;

StreamingFeatureRequest::StreamingFeatureRequest(const StreamingFeaturePipeUsage &pipeUsage, const FeaturePipeParam &extParam, MUINT32 requestNo, MUINT32 recordNo, const EISQState &eisQ)
    : mExtParam(extParam)
    , mVarMap(mExtParam.mVarMap)
    , mQParams(mExtParam.mQParams)
    , mvFrameParams(mQParams.mvFrameParams)
    , mPipeUsage(pipeUsage)
    , mFeatureMask(extParam.mFeatureMask)
    , mRequestNo(requestNo)
    , mRecordNo(recordNo)
    , mAppMode(IStreamingFeaturePipe::APP_PHOTO_PREVIEW)
    , mDebugDump(0)
    , mDisplayFPSCounter(NULL)
    , mFrameFPSCounter(NULL)
    , mResult(MTRUE)
    , mNeedDump(MFALSE)
    , mForceIMG3O(MFALSE)
    , mForceWarpPass(MFALSE)
    , mForceGpuOut(NO_FORCE)
    , mForceGpuRGBA(MFALSE)
    , mForcePrintIO(MFALSE)
    , mIs4K2K(MFALSE)
    , mEISQState(eisQ)
{
    mQParams.mDequeSuccess = MFALSE;
    mIs4K2K = NSFeaturePipe::is4K2K(this->getInputSize());
    // 3DNR + EIS1.2 in 4K2K record mode use CRZ to reduce throughput
    mIsP2ACRZMode = getVar<MBOOL>(VAR_3DNR_EIS_IS_CRZ_MODE, MFALSE);
    mAppMode = getVar<IStreamingFeaturePipe::eAppMode>(VAR_APP_MODE, IStreamingFeaturePipe::APP_PHOTO_PREVIEW);
    fsc_cropping_result = getVar<FSC_CROPPING_RESULT_STRUCT>(VAR_FSC_CROP_RESULT, fsc_cropping_result);

    for( unsigned i = 0, n = mvFrameParams.size(); i < n; ++i )
    {
        mvFrameParams.editItemAt(i).UniqueKey = mRequestNo;
    }

    mTimer.start();
}

StreamingFeatureRequest::~StreamingFeatureRequest()
{
    CAM_TRACE_CALL();

    double frameFPS = 0, displayFPS = 0;
    if( mDisplayFPSCounter )
    {
        mDisplayFPSCounter->update(mTimer.getDisplayMark());
        displayFPS = mDisplayFPSCounter->getFPS();
    }
    if( mFrameFPSCounter )
    {
        mFrameFPSCounter->update(mTimer.getFrameMark());
        frameFPS = mFrameFPSCounter->getFPS();
    }
    mTimer.print(mRequestNo, mRecordNo, displayFPS, frameFPS);
}

MVOID StreamingFeatureRequest::setDisplayFPSCounter(FPSCounter *counter)
{
    mDisplayFPSCounter = counter;
}

MVOID StreamingFeatureRequest::setFrameFPSCounter(FPSCounter *counter)
{
    mFrameFPSCounter = counter;
}

MBOOL StreamingFeatureRequest::updateResult(MBOOL result)
{
    mResult = (mResult && result);
    mQParams.mDequeSuccess = mResult;
    return mResult;
}

MBOOL StreamingFeatureRequest::doExtCallback(FeaturePipeParam::MSG_TYPE msg)
{
    MBOOL ret = MFALSE;
    if( msg == FeaturePipeParam::MSG_FRAME_DONE )
    {
        mTimer.stop();
    }
    if( mExtParam.mCallback )
    {
        ret = mExtParam.mCallback(msg, mExtParam);
    }
    return ret;
}

MSize StreamingFeatureRequest::getMaxOutSize() const
{
    MSize maxSize = MSize(0, 0);
    MUINT32 max = 0;
    if( mQParams.mvFrameParams.size() )
    {
        for( unsigned i = 0, n = mQParams.mvFrameParams[0].mvOut.size(); i < n; ++i )
        {
            MSize size = mQParams.mvFrameParams[0].mvOut[i].mBuffer->getImgSize();
            MUINT32 temp = size.w * size.h;
            if( temp > max )
            {
                maxSize = size;
                max = temp;
            }
        }
    }
    return maxSize;
}

MSize StreamingFeatureRequest::getInputSize() const
{
    MSize inputSize = MSize(0, 0);
    IImageBuffer *buffer = findInBuffer(mQParams);
    if( buffer )
    {
        inputSize = buffer->getImgSize();
    }
    return inputSize;
}
#define CALC_MAX_P2_CROP
#ifdef CALC_MAX_P2_CROP

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))
MCropRect StreamingFeatureRequest::getP2Crop() const
{
    MINT32 prev_leftmost_x = 0;
    MINT32 prev_topmost_y = 0;
    MINT32 leftmost_x = 0;
    MINT32 topmost_y = 0;
    MINT32 max_width = 0;
    MINT32 max_height = 0;

    MCrpRsInfo crop;
    MCrpRsInfo maxCrop;
    for ( unsigned i = 0, n = mQParams.mvFrameParams[0].mvCropRsInfo.size(); i < n; ++i )
    {
        crop = mQParams.mvFrameParams[0].mvCropRsInfo[i];

        if (i == 0)
        {
            max_width = crop.mCropRect.s.w;
            max_height = crop.mCropRect.s.h;

            leftmost_x = crop.mCropRect.p_integral.x;
            topmost_y = crop.mCropRect.p_integral.y;
        }
        else
        {
            prev_leftmost_x = leftmost_x;
            prev_topmost_y = topmost_y;

            leftmost_x = MIN(leftmost_x, crop.mCropRect.p_integral.x);
            topmost_y = MIN(topmost_y, crop.mCropRect.p_integral.y);
            max_width = MAX(prev_leftmost_x+max_width, crop.mCropRect.p_integral.x + crop.mCropRect.s.w) - prev_leftmost_x;
            max_height = MAX(prev_topmost_y+max_height, crop.mCropRect.p_integral.y + crop.mCropRect.s.h) - prev_topmost_y;
        }
    }
    maxCrop.mCropRect.p_integral.x = leftmost_x;
    maxCrop.mCropRect.p_integral.y = topmost_y;
    maxCrop.mCropRect.p_fractional.x = 0;
    maxCrop.mCropRect.p_fractional.y = 0;
    maxCrop.mCropRect.s.w = max_width & ~1;
    maxCrop.mCropRect.s.h = max_height & ~1;

    return maxCrop.mCropRect;
}

#else // ori

MCropRect StreamingFeatureRequest::getP2Crop() const
{
  MCropRect crop;
    if( mQParams.mvFrameParams.size() )
    {
        for( unsigned i = 0, n = mQParams.mvFrameParams[0].mvCropRsInfo.size(); i < n; ++i )
        {
          if( mQParams.mvFrameParams[0].mvCropRsInfo[i].mGroupID == 2 )
          {
              crop = mQParams.mvFrameParams[0].mvCropRsInfo[i].mCropRect;
              break;
          }
          else if( mQParams.mvFrameParams[0].mvCropRsInfo[i].mGroupID == 3 )
          {
              crop = mQParams.mvFrameParams[0].mvCropRsInfo[i].mCropRect;
          }
        }
    }
  return crop;
}
#endif

IImageBuffer* StreamingFeatureRequest::getInputBuffer()
{
    IImageBuffer *buffer = NULL;
    if( mQParams.mvFrameParams.size() )
    {
        for( unsigned i = 0, size = mQParams.mvFrameParams[0].mvIn.size(); i < size; ++i )
        {
            if( mQParams.mvFrameParams[0].mvIn[i].mPortID.index == NSImageio::NSIspio::EPortIndex_IMGI )
            {
                buffer = mQParams.mvFrameParams[0].mvIn[i].mBuffer;
                break;
            }
        }
    }
    return buffer;
}

MBOOL StreamingFeatureRequest::getDisplayOutput(NSCam::NSIoPipe::Output &output)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = getOutBuffer(mQParams, IO_TYPE_DISPLAY, output);
    if( !ret )
    {
        TRACE_FUNC("frame %d: No display buffer", mRequestNo);
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL StreamingFeatureRequest::getRecordOutput(NSCam::NSIoPipe::Output &output)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = getOutBuffer(mQParams, IO_TYPE_RECORD, output);
    if( !ret )
    {
        TRACE_FUNC("frame %d: No record buffer", mRequestNo);
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL StreamingFeatureRequest::getExtraOutput(NSCam::NSIoPipe::Output &output)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = getOutBuffer(mQParams, IO_TYPE_EXTRA, output);
    if( !ret )
    {
        TRACE_FUNC("frame %d: No extra buffer", mRequestNo);
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL StreamingFeatureRequest::getFDOutput(NSCam::NSIoPipe::Output &output)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = getOutBuffer(mQParams, IO_TYPE_FD, output);
    if( !ret )
    {
        TRACE_FUNC("frame %d: No FD buffer", mRequestNo);
    }
    TRACE_FUNC_EXIT();
    return ret;
}

IImageBuffer* StreamingFeatureRequest::getRecordOutputBuffer()
{
    NSCam::NSIoPipe::Output output;
    IImageBuffer *outputBuffer = NULL;
    if( getRecordOutput(output) )
    {
        outputBuffer = output.mBuffer;
    }
    return outputBuffer;
}

MBOOL StreamingFeatureRequest::getDisplayCrop(NSCam::NSIoPipe::MCrpRsInfo &crop, Domain domain, double cropRatio, MRectF *cropF)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    ret = getCropInfo(NSCam::NSIoPipe::EPortCapbility_Disp, WROTO_CROP_GROUP, crop, cropF);

    if(cropRatio > 0)
    {
        crop = applyCropRatio(crop, domain, cropRatio);
    }

    TRACE_FUNC( "display crop--after: pos=(%d,%d), size=(%dx%d)",
                    crop.mCropRect.p_integral.x, crop.mCropRect.p_integral.y,
                    crop.mCropRect.s.w, crop.mCropRect.s.h);

    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL StreamingFeatureRequest::getRecordCrop(NSCam::NSIoPipe::MCrpRsInfo &crop, Domain domain, double cropRatio, MRectF *cropF)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    ret = getCropInfo(NSCam::NSIoPipe::EPortCapbility_Rcrd, WDMAO_CROP_GROUP, crop, cropF);
    if(cropRatio > 0)
    {
        crop = applyCropRatio(crop, domain, cropRatio);
    }

    TRACE_FUNC( "record crop: pos=(%d,%d), size=(%dx%d)",
                    crop.mCropRect.p_integral.x, crop.mCropRect.p_integral.y,
                    crop.mCropRect.s.w, crop.mCropRect.s.h);

    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL StreamingFeatureRequest::getExtraCrop(NSCam::NSIoPipe::MCrpRsInfo &crop, Domain domain, double cropRatio, MRectF *cropF)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    ret = getCropInfo(NSCam::NSIoPipe::EPortCapbility_None, WDMAO_CROP_GROUP, crop, cropF);
    if(cropRatio > 0)
    {
        crop = applyCropRatio(crop, domain, cropRatio);
    }

    TRACE_FUNC( "extra crop: pos=(%d,%d), size=(%dx%d)",
                    crop.mCropRect.p_integral.x, crop.mCropRect.p_integral.y,
                    crop.mCropRect.s.w, crop.mCropRect.s.h);


    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL StreamingFeatureRequest::getFDCrop(NSCam::NSIoPipe::MCrpRsInfo &crop, Domain domain, double cropRatio)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    ret = getCropInfo(NSCam::NSIoPipe::EPortCapbility_None, IMG2O_CROP_GROUP, crop);
    if(cropRatio > 0)
    {
        crop = applyCropRatio(crop, domain, cropRatio);
    }

    TRACE_FUNC( "fd crop: pos=(%d,%d), size=(%dx%d)",
                    crop.mCropRect.p_integral.x, crop.mCropRect.p_integral.y,
                    crop.mCropRect.s.w, crop.mCropRect.s.h);


    TRACE_FUNC_EXIT();
    return ret;
}

MSize StreamingFeatureRequest::getEISInputSize() const
{
    MSize size = mFullImgSize;
    if( mPipeUsage.supportDual() && !mPipeUsage.supportFOVCombineEIS() )
    {
        size = size - getFOVMarginPixel() * 2;
    }
    return size;
}

MSize StreamingFeatureRequest::getFOVAlignSize() const
{
    MSize size = mFullImgSize;
    if( mPipeUsage.supportDual() )
    {
        size = size - getFOVMarginPixel() * 2;
    }
    return size;
}

MBOOL StreamingFeatureRequest::getWDMADpPqParam(DpPqParam &dpPqParam) const
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;

    NSCam::NSIoPipe::PQParam pqParam;
    if( getPQParam(pqParam) && pqParam.WDMAPQParam )
    {
        dpPqParam = *static_cast<DpPqParam*>(pqParam.WDMAPQParam);
        ret = MTRUE;
    }

    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL StreamingFeatureRequest::getWROTDpPqParam(DpPqParam &dpPqParam) const
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;

    NSCam::NSIoPipe::PQParam pqParam;
    if( getPQParam(pqParam) && pqParam.WROTPQParam )
    {
        dpPqParam = *static_cast<DpPqParam*>(pqParam.WROTPQParam);
        ret = MTRUE;
    }

    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL StreamingFeatureRequest::getPQParam(NSCam::NSIoPipe::PQParam &pqParam) const
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;

    if( mQParams.mvFrameParams.size() )
    {
        for( unsigned i = 0, size = mQParams.mvFrameParams[0].mvExtraParam.size(); i < size; ++i )
        {
            if( mQParams.mvFrameParams[0].mvExtraParam[i].CmdIdx == NSCam::NSIoPipe::EPIPE_MDP_PQPARAM_CMD &&
                mQParams.mvFrameParams[0].mvExtraParam[i].moduleStruct != NULL )
            {
                pqParam = *static_cast<NSCam::NSIoPipe::PQParam*>(mQParams.mvFrameParams[0].mvExtraParam[i].moduleStruct);
                ret = MTRUE;
                break;
            }
        }
    }

    TRACE_FUNC_EXIT();
    return ret;
}

MVOID StreamingFeatureRequest::setDumpProp(MINT32 start, MINT32 count, MBOOL byRecordNo)
{
    MINT32 debugDumpNo = byRecordNo ? mRecordNo : mRequestNo;
    mNeedDump = (start < 0) ||
                (((MINT32)debugDumpNo >= start) && (((MINT32)debugDumpNo - start) < count));
}

MVOID StreamingFeatureRequest::setForceIMG3O(MBOOL forceIMG3O)
{
    mForceIMG3O = forceIMG3O;
}

MVOID StreamingFeatureRequest::setForceWarpPass(MBOOL forceWarpPass)
{
    mForceWarpPass = forceWarpPass;
}

MVOID StreamingFeatureRequest::setForceGpuOut(MUINT32 forceGpuOut)
{
    mForceGpuOut = forceGpuOut;
}

MVOID StreamingFeatureRequest::setForceGpuRGBA(MBOOL forceGpuRGBA)
{
    mForceGpuRGBA = forceGpuRGBA;
}

MVOID StreamingFeatureRequest::setForcePrintIO(MBOOL forcePrintIO)
{
    mForcePrintIO = forcePrintIO;
}

const char* StreamingFeatureRequest::getFeatureMaskName() const
{
    std::unordered_map<MUINT32, std::string>::const_iterator iter = mFeatureMaskNameMap.find(mFeatureMask);

    if( iter == mFeatureMaskNameMap.end() )
    {
        string str;

        appendVendorTag(str, mFeatureMask);
        append3DNRTag(str, mFeatureMask);
        appendFSCTag(str, mFeatureMask);
        appendEisTag(str, mFeatureMask);
        appendFOVTag(str, mFeatureMask);
        appendN3DTag(str, mFeatureMask);
        appendVendorFOVTag(str, mFeatureMask);
        appendNoneTag(str, mFeatureMask);
        appendDefaultTag(str, mFeatureMask);

        iter = mFeatureMaskNameMap.insert(std::make_pair(mFeatureMask, str)).first;
    }

    return iter->second.c_str();
}

MBOOL StreamingFeatureRequest::need3DNR() const
{
    return HAS_3DNR(mFeatureMask);
}

MBOOL StreamingFeatureRequest::needVHDR() const
{
    return HAS_VHDR(mFeatureMask);
}

MBOOL StreamingFeatureRequest::needVFB() const
{
    return HAS_VFB(mFeatureMask);
}

MBOOL StreamingFeatureRequest::needExVFB() const
{
    return HAS_VFB_EX(mFeatureMask) && HAS_VFB(mFeatureMask);
}

MBOOL StreamingFeatureRequest::needEIS() const
{
    return HAS_EIS(mFeatureMask);
}

MBOOL StreamingFeatureRequest::needEIS22() const
{
    return HAS_EIS(mFeatureMask) && mPipeUsage.supportEIS_22();
}

MBOOL StreamingFeatureRequest::needEIS25() const
{
    return HAS_EIS(mFeatureMask) && mPipeUsage.supportEIS_25();
}

MBOOL StreamingFeatureRequest::needEIS30() const
{
    return HAS_EIS(mFeatureMask) && mPipeUsage.supportEIS_30();
}

MBOOL StreamingFeatureRequest::needVendor() const
{
    return HAS_VENDOR(mFeatureMask);
}

MBOOL StreamingFeatureRequest::needVendorMDP() const
{
    return HAS_VENDOR(mFeatureMask) || needVendorFOV();
}

MBOOL StreamingFeatureRequest::needVendorFOV() const
{
    return HAS_VENDOR_FOV(mFeatureMask);
}

MBOOL StreamingFeatureRequest::needVendorFullImg() const
{
    return needVendor() && mPipeUsage.supportVendorFullImg();
}

MBOOL StreamingFeatureRequest::needEarlyFSCVendorFullImg() const
{
    // no MTK EIS and vendor node take action-->need early FSC in P2A
    return needVendor() && mPipeUsage.supportVendorFSCFullImg();
}

MBOOL StreamingFeatureRequest::needWarp() const
{
    return HAS_EIS(mFeatureMask) ||
           (HAS_VFB_EX(mFeatureMask) && HAS_VFB(mFeatureMask));
}

MBOOL StreamingFeatureRequest::needFullImg() const
{
    return mForceIMG3O || HAS_3DNR(mFeatureMask) || HAS_VFB(mFeatureMask) || HAS_EIS(mFeatureMask)
        || (HAS_VENDOR(mFeatureMask) && !needVendorFullImg()) || HAS_FOV(mFeatureMask)
        || HAS_VENDOR_FOV(mFeatureMask) || HAS_N3D(mFeatureMask);
}

MBOOL StreamingFeatureRequest::needDsImg() const
{
    return HAS_VFB(mFeatureMask);
}

MBOOL StreamingFeatureRequest::needFEFM() const
{
    return needEIS25() && mPipeUsage.supportFEFM();
}

MBOOL StreamingFeatureRequest::needEarlyDisplay() const
{
    // return needEIS25() || needEIS22();
    return HAS_EIS(mFeatureMask);
}

MBOOL StreamingFeatureRequest::needP2AEarlyDisplay() const
{
    return needEarlyDisplay() && !needVendor() && !needFOV() &&
                !needVendorFOV();
}

MBOOL StreamingFeatureRequest::skipMDPDisplay() const
{
    return needEarlyDisplay();
}

MBOOL StreamingFeatureRequest::needRSC() const
{
    return mPipeUsage.supportRSCNode() &&
           (needEIS30() || (need3DNR() && HAS_3DNR_RSC(mFeatureMask)));
}

MBOOL StreamingFeatureRequest::needFSC() const
{
    return mPipeUsage.supportFSC() && HAS_FSC(mFeatureMask);
}

MBOOL StreamingFeatureRequest::needDump() const
{
    return mNeedDump;
}

MBOOL StreamingFeatureRequest::isLastNodeP2A() const
{
    return !HAS_VENDOR(mFeatureMask) &&
           !HAS_EIS(mFeatureMask) &&
           !HAS_VFB(mFeatureMask) &&
           !HAS_FOV(mFeatureMask) &&
           !HAS_VENDOR_FOV(mFeatureMask);
}

MBOOL StreamingFeatureRequest::is4K2K() const
{
    return mIs4K2K;
}

MUINT32 StreamingFeatureRequest::getMasterID() const
{
    MINT32 masterId = mPipeUsage.getSensorIndex();
    if( mPipeUsage.supportDual() )
    {
        masterId = this->getVar<MINT32>(VAR_DUALCAM_FOV_MASTER_ID, masterId);
    }
    return masterId;
}

MBOOL StreamingFeatureRequest::needFOV() const
{
    return HAS_FOV(mFeatureMask);
}

MBOOL StreamingFeatureRequest::needN3D() const
{
    return HAS_N3D(mFeatureMask);
}

MBOOL StreamingFeatureRequest::needFOVFEFM() const
{
    MINT32 iDoFrameSync = 0;
    iDoFrameSync = this->getVar<MINT32>(VAR_DUALCAM_DO_SYNC, iDoFrameSync);

    //return needFOV() && iDoFrameSync;
    return mPipeUsage.supportDual() && !mPipeUsage.supportVendorFOV() && iDoFrameSync;
}

MBOOL StreamingFeatureRequest::isOnFOVSensor() const
{
    //TODO need change for fov on tele
    return mPipeUsage.supportDual() && (getMasterID() == 2);
}

MBOOL StreamingFeatureRequest::needEISFullImg() const
{
    return (mPipeUsage.supportDual() || mPipeUsage.supportVendor()) && needEIS();
}

MBOOL StreamingFeatureRequest::needP2AEarlyEISFullImg() const
{
    return needEISFullImg() && !needFOV() && !needVendor() && !needVendorFOV();
}

MBOOL StreamingFeatureRequest::needHWFOVWarp() const
{
    return !needEIS() ||
           (mPipeUsage.supportWPE() && !mPipeUsage.support4K2K());
}

MBOOL StreamingFeatureRequest::needVendorFOVFullImg() const
{
    return mPipeUsage.supportDual() && mPipeUsage.supportVendorFOV() && needVendorFOV();
}

MSize StreamingFeatureRequest::getFOVMarginPixel() const
{
    return this->getVar<MSize>(VAR_DUALCAM_FOV_RRZO_MARGIN, MSize(0,0));
}

MSizeF StreamingFeatureRequest::getEISMarginPixel() const
{
    MRectF region = this->getVar<MRectF>(VAR_EIS_RRZO_CROP, MRectF(0, 0));
    return MSizeF(region.p.x, region.p.y);
}

MRectF StreamingFeatureRequest::getEISCropRegion() const
{
    return this->getVar<MRectF>(VAR_EIS_RRZO_CROP, MRectF(0, 0));
}

MSizeF StreamingFeatureRequest::getFSCMarginPixel() const
{
    MRectF FSCRRZOCrop = this->getVar<MRectF>(VAR_FSC_RRZO_CROP_REGION, MRectF(0, 0));
    return MSizeF(FSCRRZOCrop.p.x, FSCRRZOCrop.p.y);
}

MSize StreamingFeatureRequest::getFSCMaxMarginPixel() const
{
    MSize maxFSCMargin(0, 0);
    FSC_WARPING_DATA_STRUCT fscData = this->getVar<FSC_WARPING_DATA_STRUCT>(VAR_FSC_RRZO_WARP_DATA, fscData);
    maxFSCMargin = MSize(FSC_CROP_INT(fscData.maxRRZOCropRegion.p.x), FSC_CROP_INT(fscData.maxRRZOCropRegion.p.y));
    return maxFSCMargin;
}

MRectF StreamingFeatureRequest::getFSCCropRegion() const
{
    MRectF FSCRRZOCrop = this->getVar<MRectF>(VAR_FSC_RRZO_CROP_REGION, MRectF(0, 0));
    return FSCRRZOCrop;
}

MBOOL StreamingFeatureRequest::isP2ACRZMode() const
{
    return mIsP2ACRZMode;
}

EISQ_ACTION StreamingFeatureRequest::getEISQAction() const
{
    return mEISQState.mAction;
}

MUINT32 StreamingFeatureRequest::getEISQCounter() const
{
    return mEISQState.mCounter;
}

MBOOL StreamingFeatureRequest::useWarpPassThrough() const
{
    return mForceWarpPass;
}

MBOOL StreamingFeatureRequest::useDirectGpuOut() const
{
    MBOOL val = MFALSE;
    if( mForceGpuRGBA == MFALSE )
    {
        if( mForceGpuOut )
        {
            val = (mForceGpuOut == FORCE_ON);
        }
        else
        {
            val = this->is4K2K() && !mPipeUsage.supportWPE() && !EISCustom::isEnabled4K2KMDP();
        }
    }
    return val;
}

MBOOL StreamingFeatureRequest::needPrintIO() const
{
    return mForcePrintIO;
}

MBOOL StreamingFeatureRequest::getCropInfo(NSCam::NSIoPipe::EPortCapbility cap, MUINT32 defCropGroup, NSCam::NSIoPipe::MCrpRsInfo &crop, MRectF *cropF)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    unsigned count = 0;
    MUINT32 cropGroup = defCropGroup;

    if( mQParams.mvFrameParams.size() )
    {
        if( cropGroup != IMG2O_CROP_GROUP )
        {
            for( unsigned i = 0, size = mQParams.mvFrameParams[0].mvOut.size(); i < size; ++i )
            {
                if( mQParams.mvFrameParams[0].mvOut[i].mPortID.capbility == cap )
                {
                    switch( mQParams.mvFrameParams[0].mvOut[i].mPortID.index )
                    {
                    case NSImageio::NSIspio::EPortIndex_WDMAO:
                        cropGroup = WDMAO_CROP_GROUP;
                        break;
                    case NSImageio::NSIspio::EPortIndex_WROTO:
                        cropGroup = WROTO_CROP_GROUP;
                        break;
                    }
                }
            }
        }

        TRACE_FUNC("wanted crop group = %d, found group = %d", defCropGroup, cropGroup);

        for( unsigned i = 0, size = mQParams.mvFrameParams[0].mvCropRsInfo.size(); i < size; ++i )
        {
            if( mQParams.mvFrameParams[0].mvCropRsInfo[i].mGroupID == cropGroup )
            {
                if( ++count == 1 )
                {
                    crop = mQParams.mvFrameParams[0].mvCropRsInfo[i];
                    TRACE_FUNC("Found crop(%d): %dx%d", crop.mGroupID, crop.mCropRect.s.w, crop.mCropRect.s.h);
                    if( cropF != NULL )
                    {
                        MBOOL foundResult = MFALSE;
                        if( needFSC() )
                        {
                            for( unsigned j = 0; j < FSC_CROPPING_GROUP_NUM; ++j )
                            {
                                if( fsc_cropping_result.croppingGroup[j].groupID == cropGroup )
                                {
                                    foundResult = MTRUE;
                                    *cropF = fsc_cropping_result.croppingGroup[j].resultF;
                                    break;
                                }
                            }
                            if( !foundResult )
                            {
                                MY_LOGI("found FSC cropping result error! cropGroup(%d) [0]%d [1]%d", cropGroup,
                                    fsc_cropping_result.croppingGroup[0].groupID, fsc_cropping_result.croppingGroup[1].groupID);
                            }
                        }

                        if( !foundResult )
                        {
                            cropF->p = crop.mCropRect.p_integral;
                            cropF->s = crop.mCropRect.s;
                        }
                    }
                }
            }
        }
    }

    if( count > 1 )
    {
        TRACE_FUNC("frame %d: suspicious crop(ask/found: %d/%d) number = %d", mRequestNo, defCropGroup, cropGroup, count);
    }
    TRACE_FUNC_EXIT();
    return count >= 1;
}

NSCam::NSIoPipe::MCrpRsInfo StreamingFeatureRequest::applyCropRatio(NSCam::NSIoPipe::MCrpRsInfo crop, Domain domain, double ratio, const MSize &maxSize)
{
    MSize size = crop.mCropRect.s;
    if( maxSize.w && (maxSize.w < size.w) )
    {
        size.w = maxSize.w;
    }
    if( maxSize.h && (maxSize.h < size.h) )
    {
        size.h = maxSize.h;
    }
    MSize newSize = MSize(size.w*ratio, size.h*ratio);

    if( domain == RRZO_DOMAIN )
    {
        crop.mCropRect.p_integral.x += (size.w - newSize.w) >> 1;
        crop.mCropRect.p_integral.y += (size.h - newSize.h) >> 1;
        crop.mCropRect.s = newSize;
    }
    else // if( domain == WARP_DOMAIN )
    {
        crop.mCropRect.p_integral.x *= ratio;
        crop.mCropRect.p_integral.y *= ratio;
        crop.mCropRect.p_fractional.x = 0;
        crop.mCropRect.p_fractional.y = 0;
        crop.mCropRect.s = newSize;
    }

    crop.mCropRect.s.w &= ~(0x1);
    crop.mCropRect.s.h &= ~(0x1);

    return crop;
}

void StreamingFeatureRequest::appendEisTag(string& str, MUINT32 mFeatureMask) const
{
    if( HAS_EIS(mFeatureMask) )
    {
        if(!str.empty())
        {
            str += "+";
        }

        if( mPipeUsage.supportEIS_25() )
        {
            str += TAG_EIS_25();
        }
        else if( mPipeUsage.supportEIS_30() )
        {
            str += TAG_EIS_30();
        }
        else
        {
            str += TAG_EIS();
        }

        if( mPipeUsage.supportEIS_Q() )
        {
            str += TAG_EIS_QUEUE();
        }
    }
}

void StreamingFeatureRequest::append3DNRTag(string& str, MUINT32 mFeatureMask) const
{
    if( HAS_3DNR(mFeatureMask) )
    {
        if(!str.empty())
        {
            str += "+";
        }

        if( HAS_3DNR_RSC(mFeatureMask) )
        {
            str += TAG_3DNR_RSC();
        }
        else
        {
            str += TAG_3DNR();
        }
    }
}

void StreamingFeatureRequest::appendFSCTag(string& str, MUINT32 mFeatureMask) const
{
    if( HAS_FSC(mFeatureMask) )
    {
        if(!str.empty())
        {
            str += "+";
        }

        str += TAG_FSC();
    }
}

void StreamingFeatureRequest::appendVendorTag(string& str, MUINT32 mFeatureMask) const
{
    if( HAS_VENDOR(mFeatureMask) )
    {
        if(!str.empty())
        {
            str += "+";
        }

        str += TAG_VENDOR();
    }
}

void StreamingFeatureRequest::appendFOVTag(string& str, MUINT32 mFeatureMask) const
{
    if( HAS_FOV(mFeatureMask) )
    {
        if(!str.empty())
        {
            str += "+";
        }

        str += TAG_FOV();
    }
}

void StreamingFeatureRequest::appendN3DTag(string& str, MUINT32 mFeatureMask) const
{
    if( HAS_N3D(mFeatureMask) )
    {
        if(!str.empty())
        {
            str += "+";
        }

        str += TAG_N3D();
    }
}

void StreamingFeatureRequest::appendNoneTag(string& str, MUINT32 mFeatureMask) const
{
    if( mFeatureMask == 0 )
    {
        str += "NONE";
    }
}

void StreamingFeatureRequest::appendDefaultTag(string& str, MUINT32 mFeatureMask) const
{
    (void)(mFeatureMask);
    if(str.empty())
    {
        str += "UNKNOWN";
    }
}

void StreamingFeatureRequest::appendVendorFOVTag(string& str, MUINT32 mFeatureMask) const
{
    if( HAS_VENDOR_FOV(mFeatureMask) )
    {
        if(!str.empty())
        {
            str += "+";
        }

        str += TAG_VENDOR_FOV();
    }
}

VFBResult::VFBResult()
{
}

VFBResult::VFBResult(const ImgBuffer &dsImg, const ImgBuffer &alphaCL, const ImgBuffer &alphaNR, const ImgBuffer &pca)
    : mDsImg(dsImg)
    , mAlphaCL(alphaCL)
    , mAlphaNR(alphaNR)
    , mPCA(pca)
{
}

MVOID FEFMGroup::clear()
{
    this->High = NULL;
    this->Medium = NULL;
    this->Low = NULL;
}

MBOOL FEFMGroup::isValid() const
{
    return this->High != NULL;
}

RSCResult::RSCResult()
    : mMV(NULL)
    , mBV(NULL)
    , mIsValid(MFALSE)
{
}

RSCResult::RSCResult(const ImgBuffer &mv, const ImgBuffer &bv, const MSize& rssoSize, const RSC_STA_0& rscSta, MBOOL valid)
    : mMV(mv)
    , mBV(bv)
    , mRssoSize(rssoSize)
    , mRscSta(rscSta)
    , mIsValid(valid)
{
}

BasicImg::BasicImg()
    : mBuffer(NULL)
    , mIsReady(MTRUE)
{
}

BasicImg::BasicImg(const ImgBuffer &img)
    : mBuffer(img)
    , mIsReady(MTRUE)
{
}

BasicImg::BasicImg(const ImgBuffer &img, const MSizeF &offset)
    : mBuffer(img)
    , mDomainOffset(offset)
    , mIsReady(MTRUE)
{
}

BasicImg::BasicImg(const ImgBuffer &img, const MSizeF &offset, const MBOOL &isReady)
    : mBuffer(img)
    , mDomainOffset(offset)
    , mIsReady(isReady)
{
}

BasicIOImg::BasicIOImg()
{
}

BasicIOImg::BasicIOImg(const BasicImg &in)
    : mIn(in)
{
}

BasicIOImg::BasicIOImg(const BasicImg &in, const BasicImg &out)
    : mIn(in)
    , mOut(out)
{
}

DualBasicIOImg::DualBasicIOImg()
{
}

DualBasicIOImg::DualBasicIOImg(const BasicIOImg &master, const BasicIOImg &slave)
    : mMaster(master)
    , mSlave(slave)
{
}

} // NSFeaturePipe
} // NSCamFeature
} // NSCam

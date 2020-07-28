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
#include "StreamingFeatureNode.h"
#include "StreamingFeature_Common.h"
#include <camera_custom_eis.h>
#include <camera_custom_dualzoom.h>

#include <utility>

#define PIPE_CLASS_TAG "Data"
#define PIPE_TRACE TRACE_STREAMING_FEATURE_DATA
#include <featurePipe/core/include/PipeLog.h>
#include <mtkcam3/feature/DualCam/DualCam.Common.h>

#include <mtkcam/utils/std/ULog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_FPIPE_STREAMING);
using namespace NSCam::Utils::ULog;

using NSCam::NSIoPipe::QParams;
using NSCam::NSIoPipe::MCropRect;
using NSCam::Feature::P2Util::P2IO;
using NSCam::Feature::P2Util::P2Pack;
using NSCam::Feature::P2Util::P2SensorData;

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

std::unordered_map<MUINT32, std::string> StreamingFeatureRequest::mFeatureMaskNameMap;

StreamingFeatureRequest::StreamingFeatureRequest(const StreamingFeaturePipeUsage &pipeUsage, const FeaturePipeParam &extParam, MUINT32 requestNo, MUINT32 recordNo, const EISQState &eisQ)
    : mExtParam(extParam)
    , mDIPParams(mExtParam.mDIPParams)
    , mvDIPFrameParams(mDIPParams.mvDIPFrameParams)
    , mPipeUsage(pipeUsage)
    , mVarMap(mExtParam.mVarMap)
    , mP2Pack(mExtParam.mP2Pack)
    , mLog(mP2Pack.mLog)
    , mSFPIOManager(mExtParam.mSFPIOManager)
    , mMasterID(mExtParam.getVar<MUINT32>(SFP_VAR::DUALCAM_FOV_MASTER_ID, pipeUsage.getSensorIndex()))
    , mIORequestMap({{mMasterID, IORequest<StreamingFeatureNode, StreamingReqInfo>()}})
    , mMasterIOReq(mIORequestMap[mMasterID])
    , mFeatureMask(extParam.mFeatureMask)
    , mRequestNo(requestNo)
    , mRecordNo(recordNo)
    , mMWFrameNo(extParam.mP2Pack.getFrameData().mMWFrameNo)
    , mAppMode(IStreamingFeaturePipe::APP_PHOTO_PREVIEW)
    , mDebugDump(0)
    , mP2DumpType(mExtParam.mDumpType)
    , mSlaveParamMap(mExtParam.mSlaveParamMap)
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
    , mOutputControl(pipeUsage.getAllSensorIDs())
{
    mDIPParams.mDequeSuccess = MFALSE;
    // 3DNR + EIS1.2 in 4K2K record mode use CRZ to reduce throughput
    mIsP2ACRZMode = getVar<MBOOL>(SFP_VAR::NR3D_EIS_IS_CRZ_MODE, MFALSE);
    mAppMode = getVar<IStreamingFeaturePipe::eAppMode>(SFP_VAR::APP_MODE, IStreamingFeaturePipe::APP_PHOTO_PREVIEW);

    for( unsigned i = 0, n = mvDIPFrameParams.size(); i < n; ++i )
    {
        mvDIPFrameParams.at(i).UniqueKey = mRequestNo;
    }

    mMasterID = getVar<MUINT32>(SFP_VAR::DUALCAM_FOV_MASTER_ID, pipeUsage.getSensorIndex());
    mSlaveID = getVar<MUINT32>(SFP_VAR::DUALCAM_FOV_SLAVE_ID, INVALID_SENSOR); // TODO use sub sensor list id to replace it
    if(mMasterID == mSlaveID || (mSlaveID != INVALID_SENSOR && !hasSlave(mSlaveID)))
    {
        MY_LOGE("Parse Request Sensor ID & slave FeatureParam FAIL! , master(%u), slave(%u), slaveParamExist(%d)",
                mMasterID, mSlaveID, hasSlave(mSlaveID));
    }

    mTPILog = mPipeUsage.supportVendorLog();
    if( mPipeUsage.supportVendorDebug() )
    {
        mTPILog = mTPILog || property_get_int32(KEY_DEBUG_TPI_LOG, mTPILog);
        mTPIDump = property_get_int32(KEY_DEBUG_TPI_DUMP, mTPIDump);
        mTPIScan = property_get_int32(KEY_DEBUG_TPI_SCAN, mTPIScan);
        mTPIBypass = property_get_int32(KEY_DEBUG_TPI_BYPASS, mTPIBypass);
    }

    mHasGeneralOutput = hasDisplayOutput() || hasRecordOutput() || hasExtraOutput();

    mAppFPS = std::max(mP2Pack.getFrameData().mMinFps, 30);
    mNodeCycleTimeMs = 1000 / mAppFPS;

    mTimer.start();
}

StreamingFeatureRequest::~StreamingFeatureRequest()
{
    P2_CAM_TRACE_CALL(TRACE_ADVANCED);

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
    const Feature::P2Util::P2FrameData& data = mP2Pack.getFrameData();
    mTimer.print(mRequestNo, mRecordNo, displayFPS, frameFPS, data.mMinFps, data.mMaxFps);
}

MVOID StreamingFeatureRequest::setDisplayFPSCounter(FPSCounter *counter)
{
    mDisplayFPSCounter = counter;
}

MVOID StreamingFeatureRequest::setFrameFPSCounter(FPSCounter *counter)
{
    mFrameFPSCounter = counter;
}

MVOID StreamingFeatureRequest::calSizeInfo()
{
    SrcCropInfo cInfo;
    calNonLargeSrcCrop(cInfo, mMasterID);
    mNonLargeSrcCrops[mMasterID] = cInfo;

    mFullImgSize = cInfo.mSrcCrop.s;
    mIs4K2K = NSFeaturePipe::is4K2K(mFullImgSize);

    if(mSlaveID != INVALID_SENSOR)
    {
        calNonLargeSrcCrop(cInfo, mSlaveID);
        mNonLargeSrcCrops[mSlaveID] = cInfo;
    }

    if(mHasGeneralOutput)
    {
        calGeneralZoomROI();
    }
}

MVOID StreamingFeatureRequest::calGeneralZoomROI()
{
    const SFPIOMap &generalIO = mSFPIOManager.getFirstGeneralIO();
    MBOOL found = MFALSE;
    for( const SFPOutput &out : generalIO.mOutList)
    {
        if( isFDOutput(out) )
        {
            continue;
        }
        if( found )
        {
            accumulateUnion(mGeneralZoomROI, out.mCropRect);
        }
        else
        {
            mGeneralZoomROI = out.mCropRect;
            found = MTRUE;
        }
    }
    MY_S_LOGE_IF(!found, mLog, "Can not find general zoom ROI");
}

MBOOL StreamingFeatureRequest::updateResult(MBOOL result)
{
    mResult = (mResult && result);
    mDIPParams.mDequeSuccess = mResult;
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
        if(msg == FeaturePipeParam::MSG_FRAME_DONE)
        {
            CAM_ULOG_EXIT(MOD_FPIPE_STREAMING, REQ_STR_FPIPE_REQUEST, mRequestNo);
            CAM_ULOG_EXIT(MOD_P2_STR_PROC, REQ_P2_STR_REQUEST, mExtParam.mP2Pack.mLog.getLogFrameID());
        }
        ret = mExtParam.mCallback(msg, mExtParam);
    }
    return ret;
}

MVOID StreamingFeatureRequest::calNonLargeSrcCrop(SrcCropInfo &info, MUINT32 sensorID)
{
    const SFPIOMap &generalIO = mSFPIOManager.getFirstGeneralIO();
    const SFPSensorInput &sensorIn = mSFPIOManager.getInput(sensorID);
    SFPSensorTuning tuning;

    if(generalIO.isValid() && generalIO.hasTuning(sensorID))
    {
        tuning = generalIO.getTuning(sensorID);
    }
    else if(mSFPIOManager.hasPhysicalIO(sensorID))
    {
        tuning = mSFPIOManager.getPhysicalIO(sensorID).getTuning(sensorID);
    }

    info.mIMGOSize = (sensorIn.mIMGO.size() && sensorIn.mIMGO[0]) ? sensorIn.mIMGO[0]->getImgSize() : MSize(0,0);
    info.mRRZOSize = (sensorIn.mRRZO.size() && sensorIn.mRRZO[0]) ? sensorIn.mRRZO[0]->getImgSize()
                      : mP2Pack.isValid() ? mP2Pack.getSensorData(sensorID).mP1OutSize
                      : getSensorVarMap(sensorID).get<MSize>(SFP_VAR::HAL1_P1_OUT_RRZ_SIZE, MSize(0,0));
    info.mIMGOin = (tuning.isIMGOin() && !tuning.isRRZOin());
    if(tuning.isRRZOin())
    {
        info.mSrcCrop = MRect(MPoint(0,0), info.mRRZOSize);
    }
    info.mIsSrcCrop = MFALSE;

    if(info.mIMGOin)
    {
        info.mSrcCrop = (mP2Pack.isValid()) ? mP2Pack.getSensorData(sensorID).mP1Crop
                    : getSensorVarMap(sensorID).get<MRect>(SFP_VAR::IMGO_2IMGI_P1CROP, MRect(0,0));
        info.mIsSrcCrop = MTRUE;
        // HW limitation
        const MUINT32 alignMask = mP2Pack.getPlatInfo()->getImgoAlignMask();
        info.mSrcCrop.p.x &= alignMask;
    }
    // NOTE : currently not consider CRZ mode
}

MVOID StreamingFeatureRequest::checkBufferHoldByReq() const
{
    for(auto& it : mIORequestMap)
    {
        if(it.second.isFullBufHold())
        {
            MY_LOGW("Request hold working buf in SensorID (%d)", it.first);
        }
    }
}

IImageBuffer* StreamingFeatureRequest::getMasterInputBuffer()
{
    IImageBuffer *buffer = NULL;
    const SFPIOMap &generalIO = mSFPIOManager.getFirstGeneralIO();
    const SFPSensorInput &masterIn = mSFPIOManager.getInput(mMasterID);
    SFPSensorTuning tuning;

    if(generalIO.isValid() && generalIO.hasTuning(mMasterID))
    {
        tuning = generalIO.getTuning(mMasterID);
    }
    else if(mSFPIOManager.hasPhysicalIO(mMasterID))
    {
        tuning = mSFPIOManager.getPhysicalIO(mMasterID).getTuning(mMasterID);
    }
    buffer =  tuning.isRRZOin() ? get(masterIn.mRRZO)
        : tuning.isIMGOin() ? get(masterIn.mIMGO)
        : NULL;
    return buffer;
}

MBOOL StreamingFeatureRequest::popDisplayOutput(const StreamingFeatureNode *node, P2IO &output)
{
    TRACE_FUNC_ENTER();
    const SFPIOMap &generalIO = mSFPIOManager.getFirstGeneralIO();
    MBOOL ret = needDisplayOutput(node) && getOutBuffer(generalIO, IO_TYPE_DISPLAY, output);
    if( !ret )
    {
        TRACE_FUNC("frame %d: No display buffer", mRequestNo);
    }
    ret = ret && mOutputControl.registerFillOut(OutputControl::GENERAL_DISPLAY, output);
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL StreamingFeatureRequest::popRecordOutput(const StreamingFeatureNode *node, P2IO &output)
{
    TRACE_FUNC_ENTER();
    const SFPIOMap &generalIO = mSFPIOManager.getFirstGeneralIO();
    MBOOL ret = needRecordOutput(node) && getOutBuffer(generalIO, IO_TYPE_RECORD, output);
    if( !ret )
    {
        TRACE_FUNC("frame %d: No record buffer", mRequestNo);
    }
    ret = ret && mOutputControl.registerFillOut(OutputControl::GENERAL_RECORD, output);
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL StreamingFeatureRequest::popRecordOutputs(const StreamingFeatureNode *node, std::vector<P2IO> &outList)
{
    TRACE_FUNC_ENTER();
    const SFPIOMap &generalIO = mSFPIOManager.getFirstGeneralIO();
    MBOOL ret = needRecordOutput(node) && getMultiOutBuffer(generalIO, IO_TYPE_RECORD, outList);
    if( !ret )
    {
        TRACE_FUNC("frame %d: No record buffer", mRequestNo);
    }
    ret = ret && mOutputControl.registerFillOuts(OutputControl::GENERAL_RECORD, outList);
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL StreamingFeatureRequest::popExtraOutputs(const StreamingFeatureNode *node, std::vector<P2IO> &outList)
{
    TRACE_FUNC_ENTER();
    const SFPIOMap &generalIO = mSFPIOManager.getFirstGeneralIO();
    MBOOL ret = needExtraOutput(node) && getOutBuffer(generalIO, IO_TYPE_EXTRA, outList);
    if( !ret )
    {
        TRACE_FUNC("frame %d: No extra buffer", mRequestNo);
    }
    ret = ret && mOutputControl.registerFillOuts(OutputControl::GENERAL_EXTRA, outList);
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL StreamingFeatureRequest::popPhysicalOutput(const StreamingFeatureNode *node, MUINT32 sensorID, P2IO &output)
{
    TRACE_FUNC_ENTER();
    const SFPIOMap &phyIO = mSFPIOManager.getPhysicalIO(sensorID);
    MBOOL ret = needPhysicalOutput(node, sensorID) && getOutBuffer(phyIO, IO_TYPE_PHYSICAL, output);
    if( !ret )
    {
        TRACE_FUNC("frame %d: No physical buffer", mRequestNo);
    }
    ret = ret && mOutputControl.registerFillOut(OutputControl::SENSOR_PHYSICAL, sensorID, output);
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL StreamingFeatureRequest::popLargeOutputs(const StreamingFeatureNode *node, MUINT32 sensorID, std::vector<P2IO> &outList)
{
    TRACE_FUNC_ENTER();
    // NOTE currently Large output has no need to consider IOPath
    (void)node;
    MBOOL ret = MFALSE;
    const SFPIOMap &largeIO = mSFPIOManager.getLargeIO(sensorID);
    if(largeIO.isValid())
    {
        largeIO.getAllOutput(outList);
        ret = MTRUE;
    }
    if( !ret )
    {
        TRACE_FUNC("frame %d: No Large buffer", mRequestNo);
    }
    ret = ret && mOutputControl.registerFillOuts(OutputControl::SENSOR_LARGE, sensorID, outList);
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL StreamingFeatureRequest::popFDOutput(const StreamingFeatureNode *node, P2IO &output)
{
    TRACE_FUNC_ENTER();
    // NOTE currently FD output has no need to consider IOPath
    (void)node;
    const SFPIOMap &generalIO = mSFPIOManager.getFirstGeneralIO();
    MBOOL ret = getOutBuffer(generalIO, IO_TYPE_FD, output);
    if( !ret )
    {
        TRACE_FUNC("frame %d: No FD buffer", mRequestNo);
    }
    ret = ret && mOutputControl.registerFillOut(OutputControl::GENERAL_FD, output);
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL StreamingFeatureRequest::getOutputInfo(IO_TYPE ioType, OutputInfo &bufInfo)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    if(ioType == IO_TYPE_EXTRA || ioType == IO_TYPE_PHYSICAL)
    {
        MY_LOGE("Not support getOutputInfo of type(%d)", ioType);
    }
    else
    {
        const SFPIOMap &generalIO = mSFPIOManager.getFirstGeneralIO();
        P2IO output;
        ret = getOutBuffer(generalIO, ioType, output);
        if(ret)
        {
            bufInfo.mCropRect = output.mCropRect;
            bufInfo.mOutSize = output.getImgSize();
        }
    }
    TRACE_FUNC_EXIT();
    return ret;
}

EISSourceDumpInfo StreamingFeatureRequest::getEISDumpInfo()
{
    return mEisDumpInfo;
}

IImageBuffer* StreamingFeatureRequest::getDisplayOutputBuffer()
{
    P2IO output;
    IImageBuffer *outputBuffer = NULL;
    const SFPIOMap &generalIO = mSFPIOManager.getFirstGeneralIO();
    MBOOL ret = getOutBuffer(generalIO, IO_TYPE_DISPLAY, output);
    if( ret )
    {
        outputBuffer = output.mBuffer;
    }
    return outputBuffer;
}

IImageBuffer* StreamingFeatureRequest::getRecordOutputBuffer()
{
    P2IO output;
    IImageBuffer *outputBuffer = NULL;
    const SFPIOMap &generalIO = mSFPIOManager.getFirstGeneralIO();
    MBOOL ret = getOutBuffer(generalIO, IO_TYPE_RECORD, output);
    if( ret )
    {
        outputBuffer = output.mBuffer;
    }
    return outputBuffer;
}

MSize StreamingFeatureRequest::getMasterInputSize()
{
    MSize size;
    IImageBuffer* pImgBuf = getMasterInputBuffer();
    if (pImgBuf != NULL)
    {
        size = pImgBuf->getImgSize();
    }
    return size;
}

MSize StreamingFeatureRequest::getDisplaySize() const
{
    P2IO output;
    MSize size;
    const SFPIOMap &generalIO = mSFPIOManager.getFirstGeneralIO();
    if( getOutBuffer(generalIO, IO_TYPE_DISPLAY, output) &&
        output.mBuffer != NULL )
    {
        size = output.mBuffer->getImgSize();
    }
    return size;
}

MRectF StreamingFeatureRequest::getRecordCrop() const
{
    P2IO output;
    MRectF crop;
    const SFPIOMap &generalIO = mSFPIOManager.getFirstGeneralIO();
    if( getOutBuffer(generalIO, IO_TYPE_RECORD, output) )
    {
        crop = output.mCropRect;
    }
    return crop;
}

MRectF StreamingFeatureRequest::getZoomROI() const
{
    return mGeneralZoomROI;
}

MUINT32 StreamingFeatureRequest::getExtraOutputCount() const
{
    return getOutBufferCount(mSFPIOManager.getFirstGeneralIO(), IO_TYPE_EXTRA);
}

ImgBuffer StreamingFeatureRequest::popAsyncImg(const StreamingFeatureNode *node)
{
    TRACE_FUNC_ENTER();
    NextFullInfo info;
    ImgBuffer async = mMasterIOReq.getAsyncImg(node, info);
    if( mOutputControl.registerFillOut(OutputControl::GENERAL_ASYNC, async, MFALSE) &&
        async != NULL )
    {
        info.mResize = isValid(info.mResize) ? info.mResize : getDisplaySize();
        async->setExtParam(info.mResize);
        if( mPipeUsage.supportVendorErase() )
        {
            async->erase();
            async->syncCache(eCACHECTRL_FLUSH);
        }
    }
    TRACE_FUNC_EXIT();
    return async;
}

ImgBuffer StreamingFeatureRequest::getAsyncImg(const StreamingFeatureNode *node) const
{
    TRACE_FUNC_ENTER();
    NextFullInfo info;
    ImgBuffer async = mMasterIOReq.getAsyncImg(node, info);
    TRACE_FUNC_ENTER();
    return async;
}

ImgBuffer StreamingFeatureRequest::requestNextFullImg(const StreamingFeatureNode *node, MUINT32 sensorID, NextFullInfo &info)
{
    TRACE_FUNC_ENTER();
    if(mIORequestMap.count(sensorID) > 0 && mIORequestMap.at(sensorID).needNextFull(node))
    {
        return mIORequestMap.at(sensorID).getNextFullImg(node, info);
    }

    TRACE_FUNC_EXIT();
    return NULL;
}

NextFullInfo StreamingFeatureRequest::getNextFullInfo(const StreamingFeatureNode *node, MUINT32 sensorID) const
{
    TRACE_FUNC_ENTER();
    NextFullInfo info;
    if(mIORequestMap.count(sensorID) > 0 && mIORequestMap.at(sensorID).needNextFull(node))
    {
        mIORequestMap.at(sensorID).getNextFullImgInfo(node, info);
    }
    TRACE_FUNC_EXIT();
    return info;
}

MBOOL StreamingFeatureRequest::needDisplayOutput(const StreamingFeatureNode *node)
{
    return mMasterIOReq.needPreview(node);
}

MBOOL StreamingFeatureRequest::needRecordOutput(const StreamingFeatureNode *node)
{
    return mMasterIOReq.needRecord(node);
}

MBOOL StreamingFeatureRequest::needExtraOutput(const StreamingFeatureNode *node)
{
    return mMasterIOReq.needPreviewCallback(node);
}

MBOOL StreamingFeatureRequest::needFullImg(const StreamingFeatureNode *node, MUINT32 sensorID)
{
    return (mIORequestMap.count(sensorID) > 0)
            && (mIORequestMap.at(sensorID).needFull(node));
}

MBOOL StreamingFeatureRequest::needNextFullImg(const StreamingFeatureNode *node, MUINT32 sensorID)
{
    return (mIORequestMap.count(sensorID) > 0)
            && (mIORequestMap.at(sensorID).needNextFull(node));
}

MBOOL StreamingFeatureRequest::needPhysicalOutput(const StreamingFeatureNode *node, MUINT32 sensorID)
{
    return (mIORequestMap.count(sensorID) > 0)
            && (mIORequestMap.at(sensorID).needPhysicalOut(node));
}

MBOOL StreamingFeatureRequest::hasGeneralOutput() const
{
    return mHasGeneralOutput;
}

MBOOL StreamingFeatureRequest::hasDisplayOutput() const
{
    return existOutBuffer(mSFPIOManager.getFirstGeneralIO(), IO_TYPE_DISPLAY);
}

MBOOL StreamingFeatureRequest::hasFDOutput() const
{
    return existOutBuffer(mSFPIOManager.getFirstGeneralIO(), IO_TYPE_FD);
}

MBOOL StreamingFeatureRequest::hasRecordOutput() const
{
    return existOutBuffer(mSFPIOManager.getFirstGeneralIO(), IO_TYPE_RECORD);
}

MBOOL StreamingFeatureRequest::hasExtraOutput() const
{
    return existOutBuffer(mSFPIOManager.getFirstGeneralIO(), IO_TYPE_EXTRA);
}

MBOOL StreamingFeatureRequest::hasPhysicalOutput(MUINT32 sensorID) const
{
    return mSFPIOManager.getPhysicalIO(sensorID).isValid();
}

MBOOL StreamingFeatureRequest::hasLargeOutput(MUINT32 sensorID) const
{
    return mSFPIOManager.getLargeIO(sensorID).isValid();
}

MSize StreamingFeatureRequest::getEISInputSize() const
{
    MSize size = mFullImgSize;
    return size;
}

SrcCropInfo StreamingFeatureRequest::getSrcCropInfo(MUINT32 sensorID)
{
    if(mNonLargeSrcCrops.count(sensorID) == 0)
    {
        MY_LOGW("sID(%d) srcCropInfo not exist!, create dummy.", sensorID);
        mNonLargeSrcCrops[sensorID] = SrcCropInfo();
    }
    return mNonLargeSrcCrops.at(sensorID);
}

MVOID StreamingFeatureRequest::setDumpProp(MINT32 start, MINT32 count, MBOOL byRecordNo)
{
    MINT32 debugDumpNo = byRecordNo ? mRecordNo : mRequestNo;
    mNeedDump = (start < 0) ||
                (((MINT32)debugDumpNo >= start) && (((MINT32)debugDumpNo - start) < count));
    mNeedDump = mNeedDump || mP2DumpType == Feature::P2Util::P2_DUMP_DEBUG;
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
    mLog = makeRequestLogger(mLog, (mForcePrintIO ? 1 : 0), mRequestNo);
}

MVOID StreamingFeatureRequest::setEISDumpInfo(const EISSourceDumpInfo& info)
{
    mEisDumpInfo = info;
}

MBOOL StreamingFeatureRequest::isForceIMG3O() const
{
    return mForceIMG3O;
}

MBOOL StreamingFeatureRequest::hasSlave(MUINT32 sensorID) const
{
    return (mSlaveParamMap.count(sensorID) != 0);
}

FeaturePipeParam& StreamingFeatureRequest::getSlave(MUINT32 sensorID)
{
    if(hasSlave(sensorID))
    {
        return mSlaveParamMap.at(sensorID);
    }
    else
    {
        MY_LOGE("sensor(%d) has no slave FeaturePipeParam !! create Dummy", sensorID);
        mSlaveParamMap[sensorID] = FeaturePipeParam();
        return mSlaveParamMap.at(sensorID);
    }
}

const SFPSensorInput& StreamingFeatureRequest::getSensorInput() const
{
    return mSFPIOManager.getInput(mMasterID);
}

const SFPSensorInput& StreamingFeatureRequest::getSensorInput(MUINT32 sensorID) const
{
    return mSFPIOManager.getInput(sensorID);
}

VarMap<SFP_VAR>& StreamingFeatureRequest::getSensorVarMap(MUINT32 sensorID)
{
    if(sensorID == mMasterID)
    {
        return mVarMap;
    }
    else
    {
        return mSlaveParamMap[sensorID].mVarMap;
    }
}

BasicImg::SensorClipInfo StreamingFeatureRequest::getSensorClipInfo(MUINT32 sensorID) const
{
    const P2SensorData &sensorData = mP2Pack.getSensorData(sensorID);
    return BasicImg::SensorClipInfo(sensorID, sensorData.mP1Crop, sensorData.mSensorSize);
}

MBOOL StreamingFeatureRequest::getMasterFrameBasic(Feature::P2Util::DIPFrameParams &output)
{
    MBOOL ret = MTRUE;
    output.UniqueKey = mRequestNo;
    output.mSensorIdx = getMasterID();
    output.mStreamTag = NSIoPipe::NSPostProc::ENormalStreamTag_Normal;

    if( !mExtParam.mP2Pack.isValid() )
    {
        MY_LOGE("QParamValid = false but w/o valid P2Pack!");
        ret = MFALSE;
    }
    else
    {
        output.FrameNo = mExtParam.mP2Pack.getFrameData().mMWFrameNo;
        output.RequestNo = mExtParam.mP2Pack.getFrameData().mMWFrameRequestNo;
        output.Timestamp = mExtParam.mP2Pack.getSensorData().mP1TS;
    }
    return ret;
}

    // Legacy code for Hal1, w/o dynamic tuning & P2Pack
MBOOL StreamingFeatureRequest::getMasterFrameTuning(Feature::P2Util::DIPFrameParams &output)
{
    if( mDIPParams.mvDIPFrameParams.size() )
    {
        output.mTuningData = mDIPParams.mvDIPFrameParams.at(0).mTuningData;
        output.mvModuleData = mDIPParams.mvDIPFrameParams.at(0).mvModuleData;
        return MTRUE;
    }
    return MFALSE;
}

MBOOL StreamingFeatureRequest::getMasterFrameInput(Feature::P2Util::DIPFrameParams &output)
{
    if( mDIPParams.mvDIPFrameParams.size() )
    {
        output.mvIn = mDIPParams.mvDIPFrameParams.at(0).mvIn;
        return MTRUE;
    }
    return MFALSE;
}



const char* StreamingFeatureRequest::getFeatureMaskName() const
{
    std::unordered_map<MUINT32, std::string>::const_iterator iter = mFeatureMaskNameMap.find(mFeatureMask);

    if( iter == mFeatureMaskNameMap.end() )
    {
        string str;

        appendVendorTag(str, mFeatureMask);
        append3DNRTag(str, mFeatureMask);
        appendRSCTag(str, mFeatureMask);
        appendDSDNTag(str, mFeatureMask);
        appendFSCTag(str, mFeatureMask);
        appendEisTag(str, mFeatureMask);
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

MBOOL StreamingFeatureRequest::needDSDN20() const
{
    return HAS_DSDN20(mFeatureMask);
}

MBOOL StreamingFeatureRequest::needEIS() const
{
    return HAS_EIS(mFeatureMask) && (hasRecordOutput() || needPreviewEIS());
}

MBOOL StreamingFeatureRequest::needPreviewEIS() const
{
    return mPipeUsage.supportPreviewEIS();
}

MBOOL StreamingFeatureRequest::needTPIYuv() const
{
    return HAS_TPI_YUV(mFeatureMask) && hasGeneralOutput();
}

MBOOL StreamingFeatureRequest::needTPIAsync() const
{
    return HAS_TPI_ASYNC(mFeatureMask) && hasGeneralOutput();
}

MBOOL StreamingFeatureRequest::needVendorMDP() const
{
    return needTPIYuv() ||
           (needDSDN20() && ( !needWarp() || !needPreviewEIS() ) );
}

MBOOL StreamingFeatureRequest::needEarlyFSCVendorFullImg() const
{
    // no MTK EIS and vendor node take action-->need early FSC in P2A
    return mPipeUsage.supportTPI(TPIOEntry::YUV) &&
           mPipeUsage.supportVendorFSCFullImg() && needFSC();
}

MBOOL StreamingFeatureRequest::needWarp() const
{
    return needEIS();
}

MBOOL StreamingFeatureRequest::needEarlyDisplay() const
{
    return HAS_EIS(mFeatureMask) && !needPreviewEIS();
}

MBOOL StreamingFeatureRequest::needP2AEarlyDisplay() const
{
    return needEarlyDisplay() && !needTPIYuv();
}

MBOOL StreamingFeatureRequest::skipMDPDisplay() const
{
    return needEarlyDisplay();
}

MBOOL StreamingFeatureRequest::needRSC() const
{
    return mPipeUsage.supportRSCNode() &&
           (mPipeUsage.supportEISRSC() || HAS_RSC(mFeatureMask));
}

MBOOL StreamingFeatureRequest::needFSC() const
{
    return mPipeUsage.supportFSC() && HAS_FSC(mFeatureMask);
}

MBOOL StreamingFeatureRequest::needDump() const
{
    return mNeedDump;
}

MBOOL StreamingFeatureRequest::needNddDump() const
{
    return mP2DumpType == Feature::P2Util::P2_DUMP_NDD && mP2Pack.isValid();
}

MBOOL StreamingFeatureRequest::needRegDump() const
{
    return needDump() || needNddDump();
}

MBOOL StreamingFeatureRequest::is4K2K() const
{
    return mIs4K2K;
}

MUINT32 StreamingFeatureRequest::getMasterID() const
{
    return mMasterID;
}

MBOOL StreamingFeatureRequest::needTOF() const
{
    return mPipeUsage.supportTOF();
}

MUINT32 StreamingFeatureRequest::needTPILog() const
{
    return mTPILog;
}

MUINT32 StreamingFeatureRequest::needTPIDump() const
{
    return mTPIDump;
}

MUINT32 StreamingFeatureRequest::needTPIScan() const
{
    return mTPIScan;
}

MUINT32 StreamingFeatureRequest::needTPIBypass() const
{
    return mTPIBypass;
}

MUINT32 StreamingFeatureRequest::needTPIBypass(unsigned tpiNodeID) const
{
    return mTPIBypass || !mTPIFrame.needNode(tpiNodeID);
}

IMetadata* StreamingFeatureRequest::getAppMeta() const
{
    return getSensorInput().mAppIn;
}

MVOID StreamingFeatureRequest::getTPIMeta(TPIRes &res) const
{
    const SFPSensorInput &master = getSensorInput(mMasterID);
    const SFPIOMap &generalIO = mSFPIOManager.getFirstGeneralIO();

    res.setMeta(TPI_META_ID_MTK_IN_APP, master.mAppIn);
    res.setMeta(TPI_META_ID_MTK_IN_P1_HAL, master.mHalIn);
    res.setMeta(TPI_META_ID_MTK_IN_P1_APP, master.mAppDynamicIn);

    res.setMeta(TPI_META_ID_MTK_OUT_P2_APP, generalIO.mAppOut);
    res.setMeta(TPI_META_ID_MTK_OUT_P2_HAL, generalIO.mHalOut);

    if( mPipeUsage.supportDual() )
    {
        const SFPSensorInput &slave = getSensorInput(mSlaveID);
        res.setMeta(TPI_META_ID_MTK_IN_P1_HAL_2, slave.mHalIn);
        res.setMeta(TPI_META_ID_MTK_IN_P1_APP_2, slave.mAppDynamicIn);
    }
}

MBOOL StreamingFeatureRequest::isSlaveParamValid() const
{
    return mSlaveID != INVALID_SENSOR && hasSlave(mSlaveID);
}

MSizeF StreamingFeatureRequest::getEISMarginPixel() const
{
    MRectF region = this->getVar<MRectF>(SFP_VAR::EIS_RRZO_CROP, MRectF(0, 0));
    TRACE_FUNC("FSC(%d). RRZO crop(%f,%f)(%fx%f)", needFSC(),
               region.p.x, region.p.y, region.s.w, region.s.h);
    return MSizeF(region.p.x, region.p.y);
}

MRectF StreamingFeatureRequest::getEISCropRegion() const
{
    return this->getVar<MRectF>(SFP_VAR::EIS_RRZO_CROP, MRectF(0, 0));
}

MSize StreamingFeatureRequest::getFSCMaxMarginPixel() const
{
    MSize maxFSCMargin(0, 0);
    FSC::FSC_WARPING_DATA_STRUCT fscData;
    if( this->tryGetVar<FSC::FSC_WARPING_DATA_STRUCT>(SFP_VAR::FSC_RRZO_WARP_DATA, fscData) )
    {
        maxFSCMargin = MSize(FSC_CROP_INT(fscData.maxRRZOCropRegion.p.x), FSC_CROP_INT(fscData.maxRRZOCropRegion.p.y));
        TRACE_FUNC("Max FSC RRZO crop(%d,%d)(%dx%d)",
               FSC_CROP_INT(fscData.maxRRZOCropRegion.p.x), FSC_CROP_INT(fscData.maxRRZOCropRegion.p.y),
               FSC_CROP_INT(fscData.maxRRZOCropRegion.s.w), FSC_CROP_INT(fscData.maxRRZOCropRegion.s.h));
    }
    else
    {
        MY_LOGW("Cannot get FSC_WARPING_DATA_STRUCT. Use default (%dx%d)", maxFSCMargin.w, maxFSCMargin.h);
    }
    return maxFSCMargin;
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

MUINT32 StreamingFeatureRequest::getAppFPS() const
{
    return mAppFPS;
}

MUINT32 StreamingFeatureRequest::getNodeCycleTimeMs() const
{
    return mNodeCycleTimeMs;
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

MBOOL StreamingFeatureRequest::getCropInfo(NSCam::NSIoPipe::EPortCapbility cap, MUINT32 defCropGroup, NSCam::NSIoPipe::MCrpRsInfo &crop/*, MRectF *cropF*/)
{
    TRACE_FUNC_ENTER();
    unsigned count = 0;
    MUINT32 cropGroup = defCropGroup;

    if( mDIPParams.mvDIPFrameParams.size() )
    {
        if( cropGroup != IMG2O_CROP_GROUP )
        {
            for( unsigned i = 0, size = mDIPParams.mvDIPFrameParams[0].mvOut.size(); i < size; ++i )
            {
                if( mDIPParams.mvDIPFrameParams[0].mvOut[i].mPortID.capbility == cap )
                {
                    switch( mDIPParams.mvDIPFrameParams[0].mvOut[i].mPortID.index )
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

        for( unsigned i = 0, size = mDIPParams.mvDIPFrameParams[0].mvCropRsInfo.size(); i < size; ++i )
        {
            if( mDIPParams.mvDIPFrameParams[0].mvCropRsInfo[i].mGroupID == (MINT32)cropGroup )
            {
                if( ++count == 1 )
                {
                    crop = mDIPParams.mvDIPFrameParams[0].mvCropRsInfo[i];
                    TRACE_FUNC("Found crop(%d): %dx%d", crop.mGroupID, crop.mCropRect.s.w, crop.mCropRect.s.h);
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

void StreamingFeatureRequest::appendEisTag(string& str, MUINT32 mFeatureMask) const
{
    if( HAS_EIS(mFeatureMask) )
    {
        if(!str.empty())
        {
            str += "+";
        }
        str += TAG_EIS();

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
        str += TAG_3DNR();
    }
}

void StreamingFeatureRequest::appendRSCTag(string& str, MUINT32 mFeatureMask) const
{
    if( HAS_RSC(mFeatureMask) )
    {
        if(!str.empty())
        {
            str += "+";
        }
        str += TAG_RSC();
    }
}

void StreamingFeatureRequest::appendDSDNTag(string& str, MUINT32 mFeatureMask) const
{
    if( HAS_DSDN20(mFeatureMask) )
    {
        if(!str.empty())
        {
            str += "+";
        }
        str += TAG_DSDN20();
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
    if( HAS_TPI_YUV(mFeatureMask) )
    {
        if(!str.empty())
        {
            str += "+";
        }
        str += TAG_TPI_YUV();
    }
    if( HAS_TPI_ASYNC(mFeatureMask) )
    {
        if(!str.empty())
        {
            str += "+";
        }
        str += TAG_TPI_ASYNC();
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

FeaturePipeParam::MSG_TYPE toFPPMsg(HelperMsg msg)
{
    switch(msg)
    {
    case HMSG_FRAME_DONE:   return FeaturePipeParam::MSG_FRAME_DONE;
    case HMSG_DISPLAY_DONE: return FeaturePipeParam::MSG_DISPLAY_DONE;
    default:                return FeaturePipeParam::MSG_INVALID;
    };
}

MVOID HelperRWData::markReady(HelperMsg msg)
{
    if( msg < HMSG_COUNT )
    {
        mMsg[msg] |= HMSG_STATE_READY;
    }
}

MVOID HelperRWData::markDone(HelperMsg msg)
{
    if( msg < HMSG_COUNT )
    {
        mMsg[msg] |= HMSG_STATE_DONE;
    }
}

MBOOL HelperRWData::isReady(HelperMsg msg) const
{
    return (msg < HMSG_COUNT) && (mMsg[msg] & HMSG_STATE_READY) && !(mMsg[msg] & HMSG_STATE_DONE);
}

MBOOL HelperRWData::isReadyOrDone(HelperMsg msg) const
{
    return (msg < HMSG_COUNT) &&
           (mMsg[msg] & (HMSG_STATE_READY|HMSG_STATE_DONE));
}

MBOOL HelperRWData::isReadyOrDone(const std::set<HelperMsg> &msgs) const
{
    MBOOL ret = MTRUE;
    for( const HelperMsg &m : msgs )
    {
        ret = ret && m < HMSG_COUNT &&
              (mMsg[m] & (HMSG_STATE_READY|HMSG_STATE_DONE));
    }
    return ret;
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

DSDNImg::DSDNImg()
{
}

DSDNImg::DSDNImg(const BasicImg &full)
    : mFullImg(full)
{
}

MVOID DomainTransform::accumulate(const char* name, const Feature::ILog &log, const MSize &inSize, const MRectF &inCrop, const MSize &outSize)
{
    accumulate(name, log, MSizeF(inSize), inCrop, MSizeF(outSize));
}

MVOID DomainTransform::accumulate(const char* name, const Feature::ILog &log, const MSizeF &inSize, const MRectF &inCrop, const MSizeF &outSize)
{
    if(isValid(inSize) && isValidCrop(inSize, inCrop) && isValid(outSize))
    {
        MPointF calX((inSize.w - inCrop.s.w) / 2.0f, (inSize.h - inCrop.s.h) / 2.0f);
        MSizeF calS(1.0f, 1.0f);
        if(outSize != inCrop.s)
        {
            calS.w = outSize.w / inCrop.s.w;
            calS.h = outSize.h / inCrop.s.h;
            MY_S_LOGI_IF(log.getLogLevel(), log, "Transform (%s) inCrop.s != outSize," MCropF_STR "," MSizeF_STR,
                            name, MCropF_ARG(inCrop), MSizeF_ARG(outSize));
        }

        MPointF newOffset;
        MSizeF newScale;
        newOffset.x = (mOffset.x + calX.x) * calS.w;
        newOffset.y = (mOffset.y + calX.y) * calS.h;
        newScale.w = mScale.w * calS.w;
        newScale.h = mScale.h * calS.h;

        MY_S_LOGD_IF(log.getLogLevel(), log, "Transform (%s) old=" MTransF_STR " + inSize=" MSizeF_STR ", inCrop=" MCropF_STR ", outSize=" MSizeF_STR "calX=" MPointF_STR ",calS=" MSizeF_STR ",->new=" MTransF_STR,
                        name, MTransF_ARG(mOffset, mScale), MSizeF_ARG(inSize), MCropF_ARG(inCrop), MSizeF_ARG(outSize), MPointF_ARG(calX), MSizeF_ARG(calS), MTransF_ARG(newOffset, newScale));
        mOffset = newOffset;
        mScale = newScale;
    }
    else
    {
        MY_S_LOGE(log, "Transform accumulate error!!! (%s) inCrop=" MCropF_STR " inSize=" MSizeF_STR " outSize=" MSizeF_STR,
                    name, MCropF_ARG(inCrop), MSizeF_ARG(inSize), MSizeF_ARG(outSize));
    }
}

MVOID DomainTransform::accumulate(const char* name, const Feature::ILog &log, const MRectF &inZoomROI, const MRectF &outZoomROI, const MSizeF &outSize)
{
    if(isValid(inZoomROI.s) && isValidCrop(outSize, outZoomROI))
    {
        MSizeF calS(outZoomROI.s.w / inZoomROI.s.w, outZoomROI.s.h / inZoomROI.s.h);
        MPointF newOffset;
        MSizeF newScale;
        newOffset.x = (mOffset.x + inZoomROI.p.x) * calS.w - outZoomROI.p.x;
        newOffset.y = (mOffset.y + inZoomROI.p.y) * calS.h - outZoomROI.p.y;
        newScale.w = mScale.w * calS.w;
        newScale.h = mScale.h * calS.h;

        MY_S_LOGD_IF(log.getLogLevel(), log, "Transform (%s) old=" MTransF_STR " + inZoomROI=" MCropF_STR ", outZoomROI=" MCropF_STR ",outSize=" MSizeF_STR ",calS=" MSizeF_STR ",->new=" MTransF_STR,
                        name, MTransF_ARG(mOffset, mScale), MCropF_ARG(inZoomROI), MCropF_ARG(outZoomROI), MSizeF_ARG(outSize), MSizeF_ARG(calS), MTransF_ARG(newOffset, newScale));
        mOffset = newOffset;
        mScale = newScale;
    }
    else
    {
        MY_S_LOGE(log, "Transform accumulate error!!! (%s) inZoomROI=" MCropF_STR " outZoomROI=" MCropF_STR,
                    name, MCropF_ARG(inZoomROI), MCropF_ARG(outZoomROI));
    }
}

MRectF DomainTransform::applyTo(const MRectF &crop) const
{
    MRectF out;
    out.p.x = crop.p.x * mScale.w - mOffset.x;
    out.p.y = crop.p.y * mScale.h - mOffset.y;
    out.s.w = crop.s.w * mScale.w;
    out.s.h = crop.s.h * mScale.h;


    if(SFP_ABS(out.p.x, 0.0f) < 0.0001f)
    {
        out.p.x = 0.0f;
    }
    if(SFP_ABS(out.p.y, 0.0f) < 0.0001f)
    {
        out.p.y = 0.0f;
    }
    return out;
}

BasicImg::BasicImg()
    : mBuffer(NULL)
    , mIsReady(MTRUE)
{
}

BasicImg::BasicImg(const ImgBuffer &img, MBOOL isReady)
    : mBuffer(img)
    , mIsReady(isReady)
{
}

BasicImg::BasicImg(const ImgBuffer &img, MUINT32 sensorID, const MRectF &crop, const MSize &sensorSize, MBOOL isReady)
    : mBuffer(img)
    , mSensorClipInfo(sensorID, crop, sensorSize)
    , mIsReady(isReady)
{
}

MBOOL BasicImg::isValid() const
{
    return mBuffer != NULL;
}

MVOID BasicImg::setAllInfo(const BasicImg &img, const MSize &size)
{
    mSensorClipInfo = img.mSensorClipInfo;
    mTransform = img.mTransform;
    if( mBuffer != NULL && img.mBuffer != NULL )
    {
        mBuffer->setExtParam(NSFeaturePipe::isValid(size) ? size : img.mBuffer->getImgSize());
        mBuffer->setTimestamp(img.mBuffer->getTimestamp());
    }
}

MBOOL BasicImg::syncCache(NSCam::eCacheCtrl ctrl)
{
    return (mBuffer != NULL) && mBuffer->syncCache(ctrl);
}

MVOID BasicImg::accumulate(const char* name, const Feature::ILog &log, const MSize &inSize, const MRectF &crop, const MSize &outSize)
{
    mSensorClipInfo.accumulate(name, log, inSize, crop);
    mTransform.accumulate(name, log, inSize, crop, outSize);
}

MVOID BasicImg::accumulate(const char* name, const Feature::ILog &log, const MSizeF &inSize, const MRectF &crop, const MRectF &inZoomROI, const MRectF &outZoomROI, const MSizeF &outSize)
{
    mSensorClipInfo.accumulate(name, log, inSize, crop);
    if( isValidCrop(inSize, inZoomROI) && isValidCrop(outSize, outZoomROI) )
    {
        mTransform.accumulate(name, log, inZoomROI, outZoomROI, outSize);
    }
    else
    {
        MY_S_LOGE(log, "ZoomROI inValid (%s) inSize=" MSizeF_STR ",inZoomROI=" MCropF_STR ",outSize=" MSizeF_STR ", outZoomROI=" MCropF_STR,
                    name, MSizeF_ARG(inSize), MCropF_ARG(inZoomROI), MSizeF_ARG(outSize), MCropF_ARG(outZoomROI));
    }
}

BasicImg::SensorClipInfo::SensorClipInfo()
{}

BasicImg::SensorClipInfo::SensorClipInfo(MUINT32 sensorID, const MRectF &crop, const MSize &sensorSize)
    : mSensorID(sensorID)
    , mSensorCrop(crop)
    , mSensorSize(sensorSize)
{}

BasicImg::SensorClipInfo::SensorClipInfo(MUINT32 sensorID, const MRect &crop, const MSize &sensorSize)
    : mSensorID(sensorID)
    , mSensorCrop(crop.p, crop.s)
    , mSensorSize(sensorSize)
{}

MVOID BasicImg::SensorClipInfo::accumulate(const char* name, const Feature::ILog &log, const MSize &inputSize, const MRectF &cropInInput)
{
    accumulate(name, log, MSizeF(inputSize), cropInInput);
}

MVOID BasicImg::SensorClipInfo::accumulate(const char* name, const Feature::ILog &log, const MSizeF &inputSize, const MRectF &cropInInput)
{
    if( NSFeaturePipe::isValid(inputSize) )
    {
        MRectF newCrop;
        MSizeF ratio = MSizeF(mSensorCrop.s.w / inputSize.w , mSensorCrop.s.h / inputSize.h);
        newCrop.p.x = mSensorCrop.p.x + cropInInput.p.x * ratio.w;
        newCrop.p.y = mSensorCrop.p.y + cropInInput.p.y * ratio.h;
        newCrop.s.w = cropInInput.s.w * ratio.w;
        newCrop.s.h = cropInInput.s.h * ratio.h;

        MY_S_LOGD_IF(log.getLogLevel(), log, "SensorClipInfo (%s) old=" MCropF_STR " + inSize=" MSizeF_STR "/inCrop=" MCropF_STR "->newCrop=" MCropF_STR,
                        name, MCropF_ARG(mSensorCrop), MSizeF_ARG(inputSize), MCropF_ARG(cropInInput), MCropF_ARG(newCrop));
        if( !isValidCrop(mSensorSize, newCrop) )
        {
            MY_S_LOGE(log, "SensorClipInfo result error! (%s) old=" MCropF_STR " + inSize=" MSizeF_STR "/inCrop=" MCropF_STR "->newCrop=" MCropF_STR ", sSize=" MSize_STR,
                        name, MCropF_ARG(mSensorCrop), MSizeF_ARG(inputSize), MCropF_ARG(cropInInput), MCropF_ARG(newCrop), MSize_ARG(mSensorSize));
        }
        mSensorCrop = newCrop;
    }
    else
    {
        MY_S_LOGE(log, "SensorClipInfo size error!!! (%s) oldCrop=" MCropF_STR " inSize=" MSizeF_STR,
                    name, MCropF_ARG(mSensorCrop), MSizeF_ARG(inputSize));
    }
}

WarpImg::WarpImg()
{
}

WarpImg::WarpImg(const ImgBuffer &img, const MSizeF &targetInSize, const MRectF &targetCrop)
    : mBuffer(img)
    , mInputSize(targetInSize)
    , mInputCrop(targetCrop)
{
}

DualBasicImg::DualBasicImg()
{
}

DualBasicImg::DualBasicImg(const BasicImg &master)
    : mMaster(master)
{
}

DualBasicImg::DualBasicImg(const BasicImg &master, const BasicImg &slave)
    : mMaster(master)
    , mSlave(slave)
{
}

VMDPReq::VMDPReq()
{
}

VMDPReq::VMDPReq(const BasicImg &full)
    : mGeneralInput(full)
{
}

HelpReq::HelpReq()
{
}

HelpReq::HelpReq(FeaturePipeParam::MSG_TYPE msg)
    : mCBMsg(msg)
{
}

HelpReq::HelpReq(FeaturePipeParam::MSG_TYPE msg, HelperMsg helperMsg)
    : mCBMsg(msg)
    , mHelperMsg(helperMsg)
{
}

HelperMsg HelpReq::toHelperMsg() const
{
    HelperMsg msg = mHelperMsg;
    if( msg == HMSG_UNKNOWN )
    {
        switch( mCBMsg )
        {
        case FeaturePipeParam::MSG_FRAME_DONE:
            msg = HMSG_FRAME_DONE;
            break;
        case FeaturePipeParam::MSG_DISPLAY_DONE:
            msg = HMSG_DISPLAY_DONE;
            break;
        default:
            msg = HMSG_UNKNOWN;
            break;
        }
    }
    return msg;
}

TPIRes::TPIRes()
{
}

TPIRes::TPIRes(const BasicImg &yuv)
{
    mSFP[TPI_BUFFER_ID_MTK_YUV] = yuv;
}

TPIRes::TPIRes(const DualBasicImg &dual)
{
    mSFP[TPI_BUFFER_ID_MTK_YUV] = dual.mMaster;
    mSFP[TPI_BUFFER_ID_MTK_YUV_2] = dual.mSlave;
}

MVOID TPIRes::add(const DepthImg &depth)
{
    mSFP[TPI_BUFFER_ID_MTK_DEPTH] = depth.mDepthMapImg;
    mSFP[TPI_BUFFER_ID_MTK_DEPTH_INTENSITY] = depth.mDepthIntensity;
}

MVOID TPIRes::setZoomROI(const MRectF &roi)
{
    mZoomROI = roi;
}

MRectF TPIRes::getZoomROI() const
{
    return mZoomROI;
}

BasicImg TPIRes::getSFP(unsigned id) const
{
    BasicImg ret;
    auto it = mSFP.find(id);
    if( it != mSFP.end() )
    {
        ret = it->second;
    }
    return ret;
}

BasicImg TPIRes::getTP(unsigned id) const
{
    BasicImg ret;
    auto it = mTP.find(id);
    if( it != mTP.end() )
    {
        ret = it->second;
    }
    return ret;
}

IMetadata* TPIRes::getMeta(unsigned id) const
{
    IMetadata *ret = NULL;
    auto it = mMeta.find(id);
    if( it != mMeta.end() )
    {
        ret = it->second;
    }
    return ret;
}

MVOID TPIRes::setSFP(unsigned id, const BasicImg &img)
{
    mSFP[id] = img;
}

MVOID TPIRes::setTP(unsigned id, const BasicImg &img)
{
    mTP[id] = img;
}

MVOID TPIRes::setMeta(unsigned id, IMetadata *meta)
{
    mMeta[id] = meta;
}

MUINT32 TPIRes::getImgArray(TPI_Image imgs[], unsigned count) const
{
    MUINT32 index = 0;
    for( auto &img : mTP )
    {
        if( index < count && img.second.mBuffer != NULL )
        {
            imgs[index].mBufferID = img.first;
            imgs[index].mBufferPtr = img.second.mBuffer->getImageBufferPtr();
            imgs[index].mViewInfo = makeViewInfo(img.first, img.second);
            ++index;
        }
    }
    return index;
}

MUINT32 TPIRes::getMetaArray(TPI_Meta metas[], unsigned count) const
{
    MUINT32 index = 0;
    for( auto &meta : mMeta )
    {
        if( index < count )
        {
            metas[index].mMetaID = meta.first;
            metas[index].mMetaPtr = meta.second;
            ++index;
        }
    }
    return index;
}

TPI_ViewInfo TPIRes::makeViewInfo(unsigned id, const BasicImg &img) const
{
    TPI_ViewInfo info;
    // is input yuv
    if( id == TPI_BUFFER_ID_MTK_YUV ||
        id == TPI_BUFFER_ID_MTK_YUV_2 )
    {
        info.mSensorID = img.mSensorClipInfo.mSensorID;
        info.mSensorSize = img.mSensorClipInfo.mSensorSize;
        info.mSensorClip = img.mSensorClipInfo.mSensorCrop;
        if( id == TPI_BUFFER_ID_MTK_YUV )
        {
            info.mSrcZoomROI = mZoomROI;
        }
    }
    return info;
}

MVOID TPIRes::updateViewInfo(const char* name, const Feature::ILog &log, const TPI_Image imgs[], unsigned count)
{
    for( unsigned i = 0; i < count; ++i )
    {
        unsigned id = imgs[i].mBufferID;
        unsigned src = id;
        // TODO eisq: need to handle output base on main 2 case
        switch( id )
        {
        case TPI_BUFFER_ID_MTK_OUT_YUV:
        case TPI_BUFFER_ID_MTK_OUT_DISPLAY:
            src = TPI_BUFFER_ID_MTK_YUV;
            break;
        case TPI_BUFFER_ID_MTK_OUT_RECORD:
            src = TPI_BUFFER_ID_MTK_Q_RECORD_IN;
            break;
        case TPI_BUFFER_ID_MTK_OUT_YUV_2:
            src = TPI_BUFFER_ID_MTK_YUV_2;
            break;
        default:
            break;
        }
        if( src != id )
        {
            updateViewInfo(name, log, imgs[i], src, id);
        }
    }
}

MVOID TPIRes::updateViewInfo(const char* name, const Feature::ILog &log, const TPI_Image &img, unsigned srcID, unsigned dstID)
{
    auto srcIt = mTP.find(srcID);
    auto dstIt = mTP.find(dstID);

    if( srcIt != mTP.end() && dstIt != mTP.end() )
    {
        const BasicImg &src = srcIt->second;
        const TPI_ViewInfo &info = img.mViewInfo;
        BasicImg &dst = dstIt->second;

        if( isValid(info.mDstZoomROI.s) )
        {
            // use zoom roi to update transform
            // sensor info need update by real src sensor
            // transform need update by main1 transform
            MSizeF dstSize = isValid(info.mDstImageClip.s) ? info.mDstImageClip.s : MSizeF(dst.mBuffer->getImgSize());
            MSize srcSize = src.mBuffer->getImgSize();
            MRectF srcClip = isValid(info.mSrcImageClip.s) ? info.mSrcImageClip : MRectF(MPoint(), srcSize);
            dst.accumulate(name, log, srcSize, srcClip, getZoomROI(), info.mDstZoomROI, dstSize);
            dst.mBuffer->setExtParam(dstSize.toMSize());
        }
        else if( isValid(info.mSrcImageClip.s) && isValid(info.mDstImageClip.s) )
        {
            MSize dstSize = info.mDstImageClip.s.toMSize();
            dst.accumulate(name, log, src.mBuffer->getImgSize(), info.mSrcImageClip, dstSize);
            dst.mBuffer->setExtParam(dstSize);
        }
        else
        {
            MY_S_LOGI_IF(log.getLogLevel(), log,
               "(%s) not accumulate. src/dst ID(0x%x/0x%x), dst size(" MSize_STR "),trans(" MTransF_STR "),sID(%d),sCrop(" MCropF_STR ")",
                name, srcID, dstID, MSize_ARG(dst.mBuffer->getImgSize()), MTransF_ARG(dst.mTransform.mOffset, dst.mTransform.mScale),
                dst.mSensorClipInfo.mSensorID, MCropF_ARG(dst.mSensorClipInfo.mSensorCrop));
        }

    }
    else
    {
        MY_S_LOGW(log, "(%s) buffer not found! , src:ID(0x%x)/found(%d), dst:ID(0x%x)/found(%d)",
                name, srcID, srcIt != mTP.end(), dstID, dstIt != mTP.end());
    }
}

MVOID WarpController::setWarpmap(WARPMAP_TYPE type, const android::sp<IIBuffer>& warpmap, const MSizeF &targetInSize, const MRectF &targetCrop)
{
    if (type == TYPE_DISPLAY)
    {
        mDisWarpmap = warpmap;
        mDisTargetInSize = targetInSize;
        mDisTargetCrop = targetCrop;
    }
    else
    {
        mRecWarpmap = warpmap;
        mRecTargetInSize = targetInSize;
        mRecTargetCrop = targetCrop;
    }
}

WarpImg WarpController::getWarpmap(WARPMAP_TYPE type) const
{
    return (type == TYPE_DISPLAY) ? WarpImg(mDisWarpmap, mDisTargetInSize, mDisTargetCrop) :
                                    WarpImg(mRecWarpmap, mRecTargetInSize, mRecTargetCrop);
}

MBOOL WarpController::hasWarpmap(WARPMAP_TYPE type) const
{
    return (type == TYPE_DISPLAY) ? (mDisWarpmap != NULL) :
                                    (mRecWarpmap != NULL);
}

BasicImgData toBasicImgData(const DualBasicImgData &data)
{
    return BasicImgData(data.mData.mMaster, data.mRequest);
}

VMDPReqData toVMDPReqData(const DualBasicImgData &data)
{
    return VMDPReqData(VMDPReq(data.mData.mMaster), data.mRequest);
}

} // NSFeaturePipe
} // NSCamFeature
} // NSCam

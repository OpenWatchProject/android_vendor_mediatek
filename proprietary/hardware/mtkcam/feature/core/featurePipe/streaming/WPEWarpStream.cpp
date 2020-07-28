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

#include "WPEWarpStream.h"

#include "DebugControl.h"
#define PIPE_CLASS_TAG "WPEWarpStream"
#define PIPE_TRACE TRACE_WPE_WARP_STREAM_BASE
#include <featurePipe/core/include/PipeLog.h>

#include <time.h>
#include <unistd.h>
#include <cstdlib>

#define WPE_NORMAL_STREAM_NAME "WPEWarpStream"

#define PORTID_IN 0
#define PORTID_OUT 1

#define WDMAO_CROP_GROUP 2
#define WROTO_CROP_GROUP 3


using NSCam::NSIoPipe::ExtraParam;
using NSCam::NSIoPipe::Input;
using NSCam::NSIoPipe::Output;
using NSCam::NSIoPipe::MCrpRsInfo;
using NSCam::NSIoPipe::MCropRect;
using NSCam::NSIoPipe::EPortType_Memory;
using NSCam::NSIoPipe::PortID;
using NSCam::NSIoPipe::EStreamPipeID_WarpEG;
using NSImageio::NSIspio::EPortIndex_WPEI;
using NSImageio::NSIspio::EPortIndex_WPEO;
using NSImageio::NSIspio::EPortIndex_WDMAO;
using NSImageio::NSIspio::EPortIndex_WROTO;
using NSCam::NSIoPipe::EPIPE_WPE_INFO_CMD;
using NSCam::NSIoPipe::NSWpe::WPE_MODE_WPEO;
using NSCam::NSIoPipe::NSWpe::WPE_MODE_MDP;
using NSCam::NSIoPipe::NSWpe::WPE_BUS_SIZE_32_BITS;

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

WPEWarpStream* WPEWarpStream::createInstance()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
    return new WPEWarpStream();
}

WPEWarpStream::WPEWarpStream()
    : mNormalStream(NULL)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

WPEWarpStream::~WPEWarpStream()
{
    TRACE_FUNC_ENTER();
    this->uninit();
    TRACE_FUNC_EXIT();
}

MBOOL WPEWarpStream::init(const MUINT32 &sensorIdx, const MSize &maxImageSize, const MSize &maxWarpSize)
{
    TRACE_FUNC_ENTER();
    (void)(maxImageSize);
    (void)(maxWarpSize);
    MBOOL ret = MFALSE;
    if( mNormalStream == NULL )
    {
        mNormalStream = NSIoPipe::NSPostProc::INormalStream::createInstance(sensorIdx);
        if( mNormalStream == NULL )
        {
            MY_LOGE("Cannot create NormalStream for WPE");
        }
        else
        {
            ret = mNormalStream->init(WPE_NORMAL_STREAM_NAME, EStreamPipeID_WarpEG);
        }
    }

#if USE_WPE_STAND_ALONE
    if( mProcessThread == NULL )
    {
        mProcessThread = new ProcessThread();
        mProcessThread->run("Cam@fpipe.wpe.mdp");
    }
#endif

    TRACE_FUNC_EXIT();
    return ret;
}

MVOID WPEWarpStream::uninit()
{
    TRACE_FUNC_ENTER();
    if( mNormalStream )
    {
        this->waitNormalStreamBaseDone();
        mNormalStream->uninit(WPE_NORMAL_STREAM_NAME);
        mNormalStream->destroyInstance();
        mNormalStream = NULL;
    }

    if( mProcessThread != NULL )
    {
        mProcessThread->signalStop();
        mProcessThread->join();
        mProcessThread = NULL;
    }

    TRACE_FUNC_EXIT();
}

MBOOL WPEWarpStream::enque(WarpParam warpParam)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    if( mNormalStream )
    {
        QParams param;
        WPEEnqueData data;
        prepareQParams(param, data, warpParam);

        if( mProcessThread != NULL && warpParam.mByPass )
        {
            warpParam.mResult = MTRUE;
            mProcessThread->threadEnque(warpParam);
        }
        else
        {
            MY_LOGD("Frame %d wpewarp enque start", warpParam.mRequest->mRequestNo);
            this->enqueNormalStreamBase(mNormalStream, param, data);
        }

        ret = MTRUE;
    }

    TRACE_FUNC_EXIT();
    return ret;
}

MVOID WPEWarpStream::prepareQParams(QParams &param, WPEEnqueData &data, const WarpParam &warpParam)
{
    TRACE_FUNC_ENTER();

    data.mWarpParam = warpParam;
    checkMDPOut(data);

    FrameParams fparam;
    setBasic(fparam, data);
    setInBuffer(fparam, data);
    setOutBuffer(fparam, data);
    setWPEQParams(fparam, data);
    setMDPCrop(fparam, data);
    setPqParam(fparam, data);
    param.mvFrameParams.push_back(fparam);

    TRACE_FUNC_EXIT();
}

MVOID WPEWarpStream::checkMDPOut(WPEEnqueData &data)
{
    // TODO: record buffer handling
#if !USE_WPE_STAND_ALONE
    // create dummy buffer
    if( !data.mWarpParam.mMDPOut.size() )
    {
        MSize dummySize = data.mWarpParam.mOutSize;
        data.mWarpParam.mWarpOut->getImageBuffer()->setExtParam(dummySize);

        Output output;
        output.mPortID = PortID(EPortType_Memory, NSImageio::NSIspio::EPortIndex_WDMAO, PORTID_OUT);
        output.mBuffer = data.mWarpParam.mWarpOut->getImageBufferPtr();

        MCropRect cropRect(dummySize.w, dummySize.h);
        DpPqParam pqParam;
        pqParam.scenario = MEDIA_ISP_PREVIEW;
        pqParam.enable = false;
        data.mWarpParam.mMDPOut.push_back(MDPWrapper::MDPOutput(output, cropRect, pqParam));
        MY_LOGD("No record buffer, use dummy");
    }
#endif
}

MVOID WPEWarpStream::setBasic(FrameParams &fparam, WPEEnqueData &data)
{
    TRACE_FUNC_ENTER();

    const RequestPtr &request = data.mWarpParam.mRequest;

    if( request->mQParams.mvFrameParams.size() )
    {
        fparam.FrameNo = request->mQParams.mvFrameParams[0].FrameNo;
        fparam.RequestNo = request->mQParams.mvFrameParams[0].RequestNo;
        fparam.Timestamp = request->mQParams.mvFrameParams[0].Timestamp;
    }

    TRACE_FUNC_EXIT();
}

MVOID WPEWarpStream::setInBuffer(FrameParams &fparam, WPEEnqueData &data)
{
    TRACE_FUNC_ENTER();

    Input input;
    input.mPortID = PortID(EPortType_Memory, EPortIndex_WPEI, PORTID_IN);
    input.mBuffer = data.mWarpParam.mIn->getImageBufferPtr();
    fparam.mvIn.push_back(input);
    TRACE_FUNC("PortID index(%d), in/out(%d)", input.mPortID.index, input.mPortID.inout);

    TRACE_FUNC_EXIT();
}

MVOID WPEWarpStream::setOutBuffer(FrameParams &fparam, WPEEnqueData &data)
{
    TRACE_FUNC_ENTER();

#if USE_WPE_STAND_ALONE
    Output output;
    output.mPortID = PortID(EPortType_Memory, EPortIndex_WPEO, PORTID_OUT);
    output.mBuffer = data.mWarpParam.mWarpOut->getImageBufferPtr(); // WPEO
    TRACE_FUNC("PortID index(%d), in/out(%d)", output.mPortID.index, output.mPortID.inout);
    fparam.mvOut.push_back(output);
#else
    for( unsigned i = 0, n = data.mWarpParam.mMDPOut.size(); i < n; ++i )
    {
        Output output;
        output = data.mWarpParam.mMDPOut[i].mOutput;
        TRACE_FUNC("PortID index(%d), in/out(%d)", output.mPortID.index, output.mPortID.inout);
        fparam.mvOut.push_back(output);
    }
#endif
    TRACE_FUNC_EXIT();
}

MVOID WPEWarpStream::setMDPCrop(FrameParams &fparam, WPEEnqueData &data)
{
    TRACE_FUNC_ENTER();
#if !(USE_WPE_STAND_ALONE)
    for( unsigned i = 0, n = data.mWarpParam.mMDPOut.size(); i < n; ++i )
    {
        MCrpRsInfo crop;
        crop.mGroupID = (data.mWarpParam.mMDPOut[i].mOutput.mPortID.index == NSImageio::NSIspio::EPortIndex_WDMAO) ?
                         WDMAO_CROP_GROUP : WROTO_CROP_GROUP;
        crop.mCropRect = toWPEScaleCrop(data.mWarpParam, data.mWarpParam.mMDPOut[i].mCropF);
        crop.mResizeDst = MSize(0, 0); // (0, 0) use mCropRect

        MSize dstSize = toWPEOutSize(data.mWarpParam);
        if( crop.mCropRect.s.w > dstSize.w || crop.mCropRect.s.h > dstSize.h )
        {
            MCropRect newCrop = crop.mCropRect;
            newCrop.s.w = min(crop.mCropRect.s.w, dstSize.w);
            newCrop.s.h = min(crop.mCropRect.s.h, dstSize.h);
            crop.mCropRect = newCrop;
            MY_LOGD("MDP Crop > WPEOutSize, force use new Crop = (%dx%d)",
                    newCrop.s.w, newCrop.s.h);
        }
        crop.mCropRect.p_integral.x &= (~1);
        crop.mCropRect.p_integral.y &= (~1);

        fparam.mvCropRsInfo.push_back(crop);
        TRACE_FUNC("MDP Crop = In(%f,%f)(%fx%f), WPEScaleCrop Out(%d,%d)(%dx%d), groupID(%d)",
                   data.mWarpParam.mMDPOut[i].mCropF.p.x, data.mWarpParam.mMDPOut[i].mCropF.p.y,
                   data.mWarpParam.mMDPOut[i].mCropF.s.w, data.mWarpParam.mMDPOut[i].mCropF.s.h,
                   crop.mCropRect.p_integral.x, crop.mCropRect.p_integral.y, crop.mCropRect.s.w, crop.mCropRect.s.h,
                   crop.mGroupID);
    }
#endif
    TRACE_FUNC_EXIT();
}

MVOID WPEWarpStream::setWPEQParams(FrameParams &fparam, WPEEnqueData &data)
{
    TRACE_FUNC_ENTER();

    setWPEMode(data);
    setWPECrop(data);
    setWPEBuffer(data);

    ExtraParam extraParam;
    extraParam.CmdIdx = EPIPE_WPE_INFO_CMD;
    extraParam.moduleStruct = &data.mWPEQParam;
    fparam.mvExtraParam.push_back(extraParam);

    TRACE_FUNC_EXIT();
}

MVOID WPEWarpStream::setWPEMode(WPEEnqueData &data)
{
    TRACE_FUNC_ENTER();
#if USE_WPE_STAND_ALONE
    data.mWPEQParam.wpe_mode = NSCam::NSIoPipe::NSWpe::WPE_MODE_WPEO;
#else
    data.mWPEQParam.wpe_mode = NSCam::NSIoPipe::NSWpe::WPE_MODE_MDP;
#endif
    data.mWPEQParam.vgen_hmg_mode = 0; // no use z plane
    TRACE_FUNC("WPE Mode = (%d), z(%d)", data.mWPEQParam.wpe_mode, data.mWPEQParam.vgen_hmg_mode);

    TRACE_FUNC_EXIT();
}

MVOID WPEWarpStream::setWPECrop(WPEEnqueData &data)
{
    TRACE_FUNC_ENTER();

    // Source Crop
    MCrpRsInfo srcCrop;
    srcCrop.mCropRect = NSIoPipe::MCropRect(0, 0);
    data.mWPEQParam.mwVgenCropInfo.push_back(srcCrop);

    // Target Crop
    MSize dstSize = toWPEOutSize(data.mWarpParam);
    data.mWPEQParam.wpecropinfo.x_start_point = 0;
    data.mWPEQParam.wpecropinfo.y_start_point = 0;
    data.mWPEQParam.wpecropinfo.x_end_point = (dstSize.w & ~(0x1)) - 1;
    data.mWPEQParam.wpecropinfo.y_end_point = (dstSize.h & ~(0x1)) - 1;
    TRACE_FUNC("WPE Target Crop = x_start(%d), y_start(%d), x_end(%d), y_end(%d)",
               data.mWPEQParam.wpecropinfo.x_start_point, data.mWPEQParam.wpecropinfo.y_start_point,
               data.mWPEQParam.wpecropinfo.x_end_point, data.mWPEQParam.wpecropinfo.y_end_point);

    TRACE_FUNC_EXIT();
}

MVOID WPEWarpStream::setWPEBuffer(WPEEnqueData &data)
{
    TRACE_FUNC_ENTER();

    (data.mWarpParam.mWarpMap.mBuffer->getImageBuffer())->syncCache(eCACHECTRL_FLUSH);

    // x plane
    data.mWPEQParam.warp_veci_info.width    = data.mWarpParam.mWarpMap.mBuffer->getImageBuffer()->getImgSize().w;
    data.mWPEQParam.warp_veci_info.height   = data.mWarpParam.mWarpMap.mBuffer->getImageBuffer()->getImgSize().h;
    data.mWPEQParam.warp_veci_info.stride   = data.mWarpParam.mWarpMap.mBuffer->getImageBuffer()->getBufStridesInBytes(0);
    data.mWPEQParam.warp_veci_info.bus_size = NSCam::NSIoPipe::NSWpe::WPE_BUS_SIZE_32_BITS;
    data.mWPEQParam.warp_veci_info.virtAddr = (MUINTPTR)((data.mWarpParam.mWarpMap.mBuffer->getImageBuffer())->getBufVA(0));
    data.mWPEQParam.warp_veci_info.phyAddr  = (MUINTPTR)((data.mWarpParam.mWarpMap.mBuffer->getImageBuffer())->getBufPA(0));

    // y plane
    data.mWPEQParam.warp_vec2i_info.width    = data.mWarpParam.mWarpMap.mBuffer->getImageBuffer()->getImgSize().w;
    data.mWPEQParam.warp_vec2i_info.height   = data.mWarpParam.mWarpMap.mBuffer->getImageBuffer()->getImgSize().h;
    data.mWPEQParam.warp_vec2i_info.stride   = data.mWarpParam.mWarpMap.mBuffer->getImageBuffer()->getBufStridesInBytes(1);
    data.mWPEQParam.warp_vec2i_info.bus_size = NSCam::NSIoPipe::NSWpe::WPE_BUS_SIZE_32_BITS;
    data.mWPEQParam.warp_vec2i_info.virtAddr = (MUINTPTR)((data.mWarpParam.mWarpMap.mBuffer->getImageBuffer())->getBufVA(1));
    data.mWPEQParam.warp_vec2i_info.phyAddr  = (MUINTPTR)((data.mWarpParam.mWarpMap.mBuffer->getImageBuffer())->getBufPA(1));

    TRACE_FUNC("WPE Warp Info = Grid W (%d), H (%d), stride(X)=(%d), stride(Y)=(%d)",
               data.mWPEQParam.warp_veci_info.width, data.mWPEQParam.warp_veci_info.height,
               data.mWPEQParam.warp_veci_info.stride, data.mWPEQParam.warp_vec2i_info.stride);

    TRACE_FUNC_EXIT();
}

MVOID WPEWarpStream::setPqParam(FrameParams &fparam, WPEEnqueData &data)
{
    TRACE_FUNC_ENTER();

    for( unsigned i = 0, n = data.mWarpParam.mMDPOut.size(); i < n; ++i )
    {
        PortID portID = data.mWarpParam.mMDPOut[i].mOutput.mPortID;
        if( portID.index == NSImageio::NSIspio::EPortIndex_WDMAO )
        {
            data.mPQParam.WDMAPQParam = static_cast<void*>(&data.mWarpParam.mMDPOut[i].mPqParam);
        }
        else if( portID.index == NSImageio::NSIspio::EPortIndex_WROTO )
        {
            data.mPQParam.WROTPQParam = static_cast<void*>(&data.mWarpParam.mMDPOut[i].mPqParam);
        }
        TRACE_FUNC("port.index:%d enable:%d scenario:%d ", portID.index,
                   data.mWarpParam.mMDPOut[i].mPqParam.enable, data.mWarpParam.mMDPOut[i].mPqParam.scenario);
    }

    ExtraParam extraParam;
    extraParam.CmdIdx = EPIPE_MDP_PQPARAM_CMD;
    extraParam.moduleStruct = static_cast<void*>(&data.mPQParam);
    fparam.mvExtraParam.push_back(extraParam);

    TRACE_FUNC_EXIT();
}

MSize WPEWarpStream::toWPEOutSize(const WarpParam &param)
{
    TRACE_FUNC_ENTER();
    MSize size = USE_WPE_STAND_ALONE ? param.mInSize : param.mInSize;
    TRACE_FUNC_EXIT();
    return size;
}

MVOID WPEWarpStream::onNormalStreamBaseCB(const QParams &param, const WPEEnqueData &data)
{
    // This function is not thread safe,
    // avoid accessing WPEWarpStream class members
    TRACE_FUNC_ENTER();

    if( !param.mDequeSuccess )
    {
        MY_LOGW("Enque WPE NormalStream result failed");
    }

    WarpParam retData = data.mWarpParam;
    retData.mResult = param.mDequeSuccess;
    retData.mRequest->mTimer.stopEnqueWarp(Timer::STOP_RUNNING);
    MY_LOGD("Frame %d wpewarp enque done in %d ms, result = %d", retData.mRequest->mRequestNo, retData.mRequest->mTimer.getElapsedEnqueWarp(), retData.mResult);

    if( mProcessThread != NULL )
    {
        mProcessThread->threadEnque(retData);
    }
    else if( data.mWarpParam.mCallback )
    {
        data.mWarpParam.mCallback(retData);
    }
    else
    {
        MY_LOGE("Missing callback");
    }

    TRACE_FUNC_EXIT();
}

MCropRect WPEWarpStream::toCropRect(const MRectF &inCropF)
{
    TRACE_FUNC_ENTER();
    MCropRect outCrop = MCropRect(MPoint(inCropF.p.x + 0.5, inCropF.p.y + 0.5), MSize(inCropF.s.w + 0.5, inCropF.s.h + 0.5));

    TRACE_FUNC("inCrop(%f,%f)(%fx%f), outCrop(%d,%d)(%dx%d)",
               inCropF.p.x, inCropF.p.y, inCropF.s.w, inCropF.s.h,
               outCrop.p_integral.x, outCrop.p_integral.y, outCrop.s.w, outCrop.s.h);
    TRACE_FUNC_EXIT();
    return outCrop;
}

MCropRect WPEWarpStream::toWPEScaleCrop(const WarpParam &param, const MRectF &srcCrop)
{
    TRACE_FUNC_ENTER();
    MCropRect crop;
    MRectF initCrop = srcCrop;
    MRectF wpeScaleCrop;
    wpeScaleCrop.p.x = srcCrop.p.x * param.mInSize.w / param.mWarpOutSize.w;
    wpeScaleCrop.p.y = srcCrop.p.y * param.mInSize.h / param.mWarpOutSize.h;
    wpeScaleCrop.s.w = srcCrop.s.w * param.mInSize.w / param.mWarpOutSize.w;
    wpeScaleCrop.s.h = srcCrop.s.h * param.mInSize.h / param.mWarpOutSize.h;
    // Round to integral
    crop = toCropRect(wpeScaleCrop);
    TRACE_FUNC("inSize(%dx%d) warpOutSize(%fx%f), In Crop=(%fx%f)@(%f,%f), Final crop=(%dx%d)@(%d,%d)",
               param.mInSize.w, param.mInSize.h,
               param.mWarpOutSize.w, param.mWarpOutSize.h,
               initCrop.s.w, initCrop.s.h,
               initCrop.p.x, initCrop.p.y,
               crop.s.w, crop.s.h,
               crop.p_integral.x, crop.p_integral.y);
    TRACE_FUNC_EXIT();
    return crop;
}

WPEWarpStream::ProcessThread::ProcessThread()
    : mStop(MFALSE)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

WPEWarpStream::ProcessThread::~ProcessThread()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

MVOID WPEWarpStream::ProcessThread::threadEnque(const WarpParam &param)
{
    TRACE_FUNC_ENTER();
    android::Mutex::Autolock lock(mThreadMutex);
    mQueue.push(param);
    mThreadCondition.broadcast();
    TRACE_FUNC_EXIT();
}

MVOID WPEWarpStream::ProcessThread::signalStop()
{
    TRACE_FUNC_ENTER();
    android::Mutex::Autolock lock(mThreadMutex);
    mStop = MTRUE;
    mThreadCondition.broadcast();
    TRACE_FUNC_EXIT();
}

android::status_t WPEWarpStream::ProcessThread::readyToRun()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
    return android::NO_ERROR;
}

bool WPEWarpStream::ProcessThread::threadLoop()
{
    TRACE_FUNC_ENTER();
    WarpParam param;

    while( waitParam(param) )
    {
        processParam(param);
        param = WarpParam();
    }

    TRACE_FUNC_EXIT();

    return false;
}

MBOOL WPEWarpStream::ProcessThread::waitParam(WarpParam &param)
{
    TRACE_FUNC_ENTER();
    android::Mutex::Autolock lock(mThreadMutex);
    MBOOL ret = MFALSE, done = MFALSE;

    do
    {
        if( mQueue.size() )
        {
            param = mQueue.front();
            mQueue.pop();

            ret = MTRUE;
            done = MTRUE;
        }
        else if( mStop )
        {
            ret = MFALSE;
            done = MTRUE;
        }
        else
        {
            mThreadCondition.wait(mThreadMutex);
        }
    }
    while( !done );

    TRACE_FUNC_EXIT();

    return ret;
}

MVOID WPEWarpStream::ProcessThread::processParam(WarpParam param)
{
    TRACE_FUNC_ENTER();

    if( param.mResult && param.mMDPOut.size() )
    {
        if( param.mRequest == NULL )
        {
            MY_LOGE("Missing request");
            param.mResult = MFALSE;
        }
        else
        {
            param.mRequest->mTimer.startWarpMDP();

            ImgBuffer srcBuffer = param.mByPass ? param.mIn : param.mWarpOut;
            if(USE_WPE_STAND_ALONE && !param.mByPass)
            {
                for( auto &out : param.mMDPOut )
                {
                    out.mCropRect = toWPEScaleCrop(param, out.mCropF);
                }
            }
            param.mResult = mMDP.process(srcBuffer->getImageBufferPtr(), param.mMDPOut);

            param.mRequest->mTimer.stopWarpMDP();
        }
    }

    if( param.mCallback )
    {
        param.mCallback(param);
    }
    else
    {
        MY_LOGE("Missing callback");
    }

    TRACE_FUNC_EXIT();
}


} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam

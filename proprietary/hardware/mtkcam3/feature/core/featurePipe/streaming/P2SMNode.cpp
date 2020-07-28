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

#include "P2SMNode.h"
#include <mtkcam3/feature/smvr/SMVRData.h>

#define PIPE_CLASS_TAG "P2SMNode"
#define PIPE_TRACE TRACE_P2SM_NODE
#include <featurePipe/core/include/PipeLog.h>

#include "TuningHelper.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_FPIPE_STREAMING);

using NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_Normal;
using NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_Vss;
using NSImageio::NSIspio::EPortIndex_IMG2O;
using NSImageio::NSIspio::EPortIndex_IMG3O;

using namespace NSCam::Feature::P2Util;
using namespace NSCam::NSIoPipe;
using NSCam::Feature::SMVR::SMVRResult;

#define NUM_BASIC_TUNING 3

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

P2SMNode::P2SMNode(const char *name)
    : StreamingFeatureNode(name)
{
    TRACE_FUNC_ENTER();
    this->addWaitQueue(&mRequests);
    TRACE_FUNC_EXIT();
}

P2SMNode::~P2SMNode()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

MVOID P2SMNode::setNormalStream(Feature::P2Util::DIPStream *stream)
{
    TRACE_FUNC_ENTER();
    mDIPStream = stream;
    TRACE_FUNC_EXIT();
}

MVOID P2SMNode::flush()
{
    TRACE_FUNC_ENTER();
    MY_LOGI("flush %zu data from SMVR queue", mSubQueue.size());
    while( mSubQueue.size() )
    {
        mSubQueue.pop();
    }
    TRACE_FUNC_EXIT();
}

MBOOL P2SMNode::onInit()
{
    TRACE_FUNC_ENTER();
    StreamingFeatureNode::onInit();

    mSMVRSpeed = mPipeUsage.getSMVRSpeed();

    mTuningPool = TuningBufferPool::create("fpipe.p2sm_tuning", NSIoPipe::NSPostProc::INormalStream::getRegTableSize(), TuningBufferPool::BUF_PROTECT_RUN);

    MSize fullsize = align(mPipeUsage.getStreamingSize(), BUF_ALLOC_ALIGNMENT_BIT);
    mFullImgPool = ImageBufferPool::create("fpipe.p2sm_full", fullsize, eImgFmt_YV12, ImageBufferPool::USAGE_HW);

    mSMVRSpeed = getPropertyValue("vendor.debug.fpipe.p2sm.speed", mSMVRSpeed);
    mSMVRSpeed = std::max<MUINT32>(mSMVRSpeed, 1);
    mExpectMS = getPropertyValue("vendor.debug.fpipe.p2sm.expect", mExpectMS);
    mBurst = getPropertyValue("vendor.debug.fpipe.p2sm.burst", mBurst);
    mSimulate = getPropertyValue("vendor.debug.fpipe.p2sm.simulate", mSimulate);
    mSingle = getPropertyValue("vendor.debug.fpipe.p2sm.single", mSingle);
    mSkip = getPropertyValue("vendor.debug.fpipe.p2sm.skip", mSkip);
    mRecover = mBurst/mSMVRSpeed;
    mOptimizeFirst = !(mSingle & DEBUG_SINGLE_DISPLAY_RUN);

    MY_LOGI("SMVR StreamingSize=(%dx%d) speed=%d->%d burst/recover=%d/%d expectMS=%d simulate=%d single=%d skip=%d", fullsize.w, fullsize.h, mPipeUsage.getSMVRSpeed(), mSMVRSpeed, mBurst, mRecover, mExpectMS, mSimulate, mSingle, mSkip);

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL P2SMNode::onUninit()
{
    TRACE_FUNC_ENTER();
    MY_LOGI("SMVR Q=%zu", mSubQueue.size());
    ImageBufferPool::destroy(mFullImgPool);
    TuningBufferPool::destroy(mTuningPool);
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL P2SMNode::onThreadStart()
{
    TRACE_FUNC_ENTER();

    if( mTuningPool != NULL )
    {
        Timer timer(MTRUE);
        mTuningCount = NUM_BASIC_TUNING;
        mTuningPool->allocate(mTuningCount);
        timer.stop();
        MY_LOGD("Dynamic Tuning %s %d bufs in %d ms", STR_ALLOCATE, mTuningCount, timer.getElapsed());
    }

    if( mFullImgPool != NULL )
    {
        Timer timer(MTRUE);
        MUINT32 count = 3;
        mFullImgPool->allocate(count);
        timer.stop();
        MY_LOGD("mFullImg %s %d buf in %d ms", STR_ALLOCATE, count, timer.getElapsed());
    }

    initHalISP();
    initP2();

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL P2SMNode::onThreadStop()
{
    TRACE_FUNC_ENTER();
    this->waitDIPStreamBaseDone();
    uninitHalISP();
    uninitP2();

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL P2SMNode::onData(DataID id, const RequestPtr &data)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC("Frame %d: %s arrived", data->mRequestNo, ID2Name(id));
    MBOOL ret = MFALSE;

    switch( id )
    {
    case ID_ROOT_TO_P2SM:
        mRequests.enque(data);
        ret = MTRUE;
        break;
    default:
        ret = MFALSE;
        break;
    }

    TRACE_FUNC_EXIT();
    return ret;
}

IOPolicyType P2SMNode::getIOPolicy(StreamType /*stream*/, const StreamingReqInfo &reqInfo) const
{
    (void)reqInfo;
    IOPolicyType policy = IOPOLICY_INOUT;
    return policy;
}

MBOOL P2SMNode::onThreadLoop()
{
    TRACE_FUNC("Waitloop");
    RequestPtr request;

    P2_CAM_TRACE_CALL(TRACE_DEFAULT);

    if( !waitAllQueue() )
    {
        return MFALSE;
    }
    if( !mRequests.deque(request) )
    {
        MY_LOGE("Request deque out of sync");
        return MFALSE;
    }
    else if( request == NULL )
    {
        MY_LOGE("Request out of sync");
        return MFALSE;
    }

    TRACE_FUNC_ENTER();

    request->mTimer.startP2SM();
    processP2SM(request);
    request->mTimer.stopP2SM();
    tryAllocateTuningBuffer();

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MVOID P2SMNode::onDIPStreamBaseCB(const Feature::P2Util::DIPParams &params, const P2SMEnqueData &data)
{
    // This function is not thread safe,
    // avoid accessing P2SMNode class members
    TRACE_FUNC_ENTER();

    RequestPtr request = data.mRequest;
    if( request == NULL )
    {
        MY_LOGE("Missing request");
    }
    else
    {
        request->mTimer.stopP2SMEnque();
        MY_S_LOGD(data.mRequest->mLog, "sensor(%d) Frame %d enque done in %d ms, result = %d", mSensorIndex, request->mRequestNo, request->mTimer.getElapsedP2SMEnque(), params.mDequeSuccess);

        if( !params.mDequeSuccess )
        {
            MY_LOGW("Frame %d enque result failed", request->mRequestNo);
        }

        request->updateResult(params.mDequeSuccess);
        handleResultData(request, data);
        request->mTimer.stopP2SM();
    }

    this->decExtThreadDependency();
    TRACE_FUNC_EXIT();
}

MVOID P2SMNode::handleResultData(const RequestPtr &request, const P2SMEnqueData &data)
{
    // This function is not thread safe,
    // because it is called by onQParamsCB,
    // avoid accessing P2SMNode class members
    TRACE_FUNC_ENTER();
    handleData(ID_P2SM_TO_HELPER, HelperData(FeaturePipeParam::MSG_FRAME_DONE, request));

    if( request->needNddDump() )
    {
        if( data.mFullImg != NULL )
        {
            TuningUtils::FILE_DUMP_NAMING_HINT hint = request->mP2Pack.getSensorData().mNDDHint;
            data.mFullImg->getImageBuffer()->syncCache(eCACHECTRL_INVALID);
            dumpNddData(&hint, data.mFullImg->getImageBufferPtr(), TuningUtils::YUV_PORT_IMG3O);
        }
    }
    if( request->needDump() )
    {
        if( data.mFullImg != NULL )
        {
            data.mFullImg->getImageBuffer()->syncCache(eCACHECTRL_INVALID);
            dumpData(data.mRequest, data.mFullImg->getImageBufferPtr(), "full");
        }
    }
    TRACE_FUNC_EXIT();
}

MBOOL P2SMNode::initP2()
{
    TRACE_FUNC_ENTER();
    MBOOL ret = (mDIPStream != NULL);
    TRACE_FUNC_EXIT();
    return ret;
}

MVOID P2SMNode::uninitP2()
{
    TRACE_FUNC_ENTER();
    mDIPStream = NULL;
    TRACE_FUNC_EXIT();
}

MBOOL P2SMNode::processP2SM(const RequestPtr &request)
{
    TRACE_FUNC_ENTER();
    P2_CAM_TRACE_CALL(TRACE_ADVANCED);

    P2SMEnqueData data;
    Feature::P2Util::DIPParams param;
    sp<MainData> mainData;

    data.mRequest = request;

    mainData = new MainData(request->mP2Pack);
    mainData->mReqCount = request->getVar<MUINT32>(SFP_VAR::SMVR_REQ_COUNT, 0);
    mainData->mIsRecord = request->mP2Pack.getFrameData().mIsRecording;
    mainData->mRecordCrop = request->getRecordCrop();

    prepareDrop(request, data, mainData);

    request->mTimer.startP2SMTuning();
    prepareTuning(request, mainData);
    request->mTimer.stopP2SMTuning();

    prepareIO(param, request, data, mainData);
    SMVRResult result;
    result.mRunCount = data.mRecordRunCount;
    result.mDropCount = data.mDropCount;
    result.mQueueCount = mSubQueue.size();
    request->setVar<SMVRResult>(SFP_VAR::SMVR_RESULT, result);

    MY_S_LOGD(request->mLog, "#req=%d, #out=%d isRec=%d #drop=%d #Q=%zu crop=(%fx%f)@(%f,%f)", mainData->mReqCount, data.mRecordRunCount, mainData->mIsRecord, data.mDropCount, mSubQueue.size(), mainData->mRecordCrop.s.w, mainData->mRecordCrop.s.h, mainData->mRecordCrop.p.x, mainData->mRecordCrop.p.y);

    NSCam::Feature::P2Util::updateExpectEndTime(param, mExpectMS);

    if( request->needPrintIO() )
    {
        NSCam::Feature::P2Util::printDIPParams(request->mLog, param);
    }

    enqueFeatureStream(param, data);

    P2_CAM_TRACE_END(TRACE_ADVANCED);
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL P2SMNode::prepareDrop(const RequestPtr &request, P2SMEnqueData &data, const sp<MainData> &mainData)
{
    TRACE_FUNC_ENTER();
    if( mainData != NULL && !mainData->mIsRecord )
    {
        if( !mSubQueue.empty() )
        {
            data.mDropCount += mSubQueue.size();
            flush();
        }
        if( mainData->mReqCount != 0 )
        {
            MY_S_LOGW(request->mLog, "SMVR mark drop but reqCount(%d) != 0, force drop", mainData->mReqCount);
            data.mDropCount += mainData->mReqCount;
            mainData->mReqCount = 0;
        }
    }
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL P2SMNode::prepareTuning(const RequestPtr &request, const sp<MainData> &mainData)
{
    TRACE_FUNC_ENTER();
    P2_CAM_TRACE_CALL(TRACE_ADVANCED);
    MBOOL ret = MTRUE;

    const SFPIOMap &generalIO = request->mSFPIOManager.getFirstGeneralIO();
    const SFPSensorInput &sensorIn = request->getSensorInput(request->mMasterID);

    mainData->mTuningBuffer = mTuningPool->request();
    memset(mainData->mTuningBuffer->mpVA, 0, mTuningPool->getBufSize());

    TuningHelper::Config config;
    config.mUsage = TuningHelper::Tuning_SMVR;
    config.mHalISP = mHalISP;
    config.mTag = mSMVRTag;
    config.mTuningBuf = mainData->mTuningBuffer->mpVA;

    config.mAppOut = generalIO.mAppOut;
    config.mHalOut = generalIO.mHalOut;
    config.mAppIn = sensorIn.mAppIn;
    config.mHalIn = sensorIn.mHalIn;
    config.mLCSO = get(sensorIn.mLCSO);
    config.mLCSHO = get(sensorIn.mLCSHO);

    config.mResized = MTRUE;
    config.mUseFDCrop = request->mVarMap.tryGet<MRect>(SFP_VAR::FD_CROP_ACTIVE_REGION, config.mFDCrop);

    TuningHelper::makeTuningParam_SMVR(request->mP2Pack, config, mainData->mTuningParam);

    updateLCSO(request, mainData);

    P2_CAM_TRACE_END(TRACE_ADVANCED);
    TRACE_FUNC_EXIT();
    return ret;
}

MVOID P2SMNode::updateLCSO(const RequestPtr &request, const sp<MainData> &mainData)
{
    TRACE_FUNC_ENTER();
    const SFPSensorInput &sensorIn = request->getSensorInput(request->mMasterID);
    if( mainData->mTuningParam.pLcsBuf )
    {
        MSize lcsoSize;
        IImageBuffer *firstLCSO = get(sensorIn.mLCSO);
        if( mainData->mTuningParam.pLcsBuf != firstLCSO )
        {
            MY_LOGW("tuning->pLcsBuf(%p) != firstLCSO(%p)", mainData->mTuningParam.pLcsBuf, firstLCSO);
        }
        lcsoSize = firstLCSO ? firstLCSO->getImgSize() : MSize(0,0);
        for( unsigned i = 1, n = sensorIn.mLCSO.size(); i < n; ++i )
        {
            if( sensorIn.mLCSO[i] )
            {
                sensorIn.mLCSO[i]->setExtParam(lcsoSize);
            }
        }
    }
    TRACE_FUNC_EXIT();
}

MBOOL P2SMNode::prepareIO(Feature::P2Util::DIPParams &param, const RequestPtr &request, P2SMEnqueData &data, const sp<MainData> &mainData)
{
    TRACE_FUNC_ENTER();
    P2_CAM_TRACE_CALL(TRACE_ADVANCED);
    MBOOL ret = MTRUE;
    MBOOL optimizeFirstRecord = MFALSE;
    MUINT32 first = 0;
    const SFPSensorInput &sensorIn = request->getSensorInput(request->mMasterID);
    std::vector<P2IO> recordOut;

    if( !request->popRecordOutputs(this, recordOut) )
    {
        recordOut.clear();
    }

    optimizeFirstRecord = mOptimizeFirst && !recordOut.empty() && mSubQueue.empty();
    first = optimizeFirstRecord ? 1 : 0;

    prepareMainRun(param, request, data, mainData, recordOut, optimizeFirstRecord);
    for( unsigned i = first; i < mainData->mReqCount; ++i )
    {
        mSubQueue.emplace(SubData(mainData, sensorIn.mRRZO[i], sensorIn.mLCSO[i], get(sensorIn.mLCSHO, i)));
    }

    prepareRecordRun(param, request, data, recordOut, optimizeFirstRecord);

    P2_CAM_TRACE_END(TRACE_ADVANCED);
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL P2SMNode::prepareMainRun(Feature::P2Util::DIPParams &params, const RequestPtr &request, P2SMEnqueData &data, const sp<MainData> &mainData, const std::vector<P2IO> &recordOut, MBOOL optimizeFirstRecord)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MTRUE;

    const SFPSensorInput &sensorIn = request->getSensorInput();

    sp<P2SMEnqueRunData> runData;
    P2IOPack mainIO, vssIO;
    MINT64 timestamp = 0;
    MUINT32 optimizedVSS = 0;

    mainIO.mFlag |= P2Flag::FLAG_RESIZED;
    mainIO.mIMGI.mBuffer = get(sensorIn.mRRZO);
    mainIO.mLCSO.mBuffer = get(sensorIn.mLCSO);
    mainIO.mLCSHO.mBuffer = get(sensorIn.mLCSHO);
    vssIO = mainIO;
    timestamp = mainIO.mIMGI.mBuffer->getTimestamp();

    std::vector<P2IO> vss;
    P2IO output;
    if( request->popDisplayOutput(this, output) && output.isValid() )
    {
        output.mBuffer->setTimestamp(timestamp);
        if( !fillIOPack(mainIO, output) )
        {
            MY_LOGE("cannot fit display output into main run");
        }
    }
    if( optimizeFirstRecord && recordOut.size() )
    {
        output = recordOut[0];
        setRecordTimestamp(output.mBuffer, timestamp, mainData);
        if( !fillIOPack(mainIO, output) )
        {
            MY_LOGE("cannot fit record output into main run");
        }
        ++data.mRecordRunCount;
    }
    if( request->popFDOutput(this, output) && output.isValid() )
    {
        output.mBuffer->setTimestamp(timestamp);
        mainIO.mIMG2O = output;
    }
    if( request->isForceIMG3O() )
    {
        sp<IImageBuffer> buffer;
        data.mFullImg = mFullImgPool->requestIIBuffer();
        buffer = data.mFullImg->getImageBuffer();
        buffer->setTimestamp(timestamp);
        buffer->setExtParam(mainIO.mIMGI.mBuffer->getImgSize());
        mainIO.mIMG3O.mBuffer = buffer.get();
    }

    request->popExtraOutputs(this, vss);
    optimizedVSS = 0;
    optimizedVSS = fillIOPack(mainIO, vss, optimizedVSS);

    runData = addRunData(request, data, mainData);
    copyRunTuning(runData, mainData, DEBUG_COPY_DISP_TUNING);
    addRun(params, request, runData, mainData, mainIO);

    while( vss.size() > optimizedVSS )
    {
        P2IOPack extraIO = vssIO;
        optimizedVSS = fillIOPack(extraIO, vss, optimizedVSS);
        runData = addRunData(request, data, mainData);
        copyRunTuning(runData, mainData, DEBUG_COPY_REC_TUNING);
        addRun(params, request, runData, mainData, extraIO);
    }

    if( mSimulate && recordOut.size() )
    {
        MY_LOGD("simulate %d extra runs", mSimulate);
        for( unsigned i = 0; i < mSimulate; ++i )
        {
            P2IOPack extraIO = vssIO;
            output = recordOut[0];
            fillIOPack(extraIO, output);
            runData = addRunData(request, data, mainData);
            copyRunTuning(runData, mainData, DEBUG_COPY_REC_TUNING);
            addRun(params, request, runData, mainData, extraIO);
        }
    }

    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL P2SMNode::prepareRecordRun(Feature::P2Util::DIPParams &params, const RequestPtr &request, P2SMEnqueData &data, const std::vector<P2IO> &recordOut, MBOOL optimizeFirstRecord)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MTRUE;

    size_t recordRun = mSMVRSpeed;
    MUINT32 first = optimizeFirstRecord ? 1 : 0;

    recordRun = std::min(recordRun, recordOut.size());
    recordRun = std::min(recordRun, mSubQueue.size() + first);

    params.mvDIPFrameParams.reserve(params.mvDIPFrameParams.size() + (recordRun-first));

    MY_S_LOGD(request->mLog, "speed=%d out=%zu: record[%d~%zu] queue=%zu",
              mSMVRSpeed, recordOut.size(), first, recordRun, mSubQueue.size());

    for( unsigned i = first; i < recordRun; ++i )
    {
        const SubData &sub = mSubQueue.front();
        P2IOPack io;
        io.mFlag |= P2Flag::FLAG_RESIZED;
        io.mIMGI.mBuffer = sub.mRRZO;
        io.mLCSO.mBuffer = sub.mLCSO;
        io.mLCSHO.mBuffer = sub.mLCSHO;
        P2IO out = recordOut[i];
        out.mCropRect = sub.mMainData->mRecordCrop;
        setRecordTimestamp(out.mBuffer, io.mIMGI.mBuffer->getTimestamp(), sub.mMainData);
        // TODO: setPQParam() if enable PQ feature
        io.mWROTO = out;

        sp<P2SMEnqueRunData> runData;
        runData = addRunData(request, data, sub.mMainData);
        copyRunTuning(runData, sub.mMainData, DEBUG_COPY_REC_TUNING);
        addRun(params, request, runData, sub.mMainData, io);
        ++data.mRecordRunCount;
        mSubQueue.pop();
    }

    TRACE_FUNC_EXIT();
    return ret;
}

MVOID P2SMNode::setRecordTimestamp(IImageBuffer *buffer, MINT64 timestamp, const sp<MainData> &mainData)
{
    TRACE_FUNC_ENTER();
    if( buffer != NULL )
    {
        MUINT32 count = mainData != NULL ? mainData->mReqCount : 1;
        MUINT64 ts = (MUINT64)timestamp;
        ts = ts/1000 * 1000 + count;
        buffer->setTimestamp((MINT64)ts);
    }
    TRACE_FUNC_EXIT();
}

MUINT32 P2SMNode::fillIOPack(P2IOPack &io, const P2IO &output)
{
    TRACE_FUNC_ENTER();
    MUINT ret = 0;
    if( !output.mTransform && !io.mWDMAO.isValid() )
    {
        io.mWDMAO = output;
        ret = 1;
    }
    else if( !io.mWROTO.isValid() )
    {
        io.mWROTO = output;
        ret = 1;
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MUINT32 P2SMNode::fillIOPack(P2IOPack &io, const std::vector<P2IO> &outputs, MUINT32 index)
{
    TRACE_FUNC_ENTER();
    while( outputs.size() > index )
    {
        if( !outputs[index].mTransform && !io.mWDMAO.isValid() )
        {
            io.mWDMAO = outputs[index++];
        }
        else if( !io.mWROTO.isValid() )
        {
            io.mWROTO = outputs[index++];
        }
        else
        {
            break;
        }
    }
    TRACE_FUNC_EXIT();
    return index;
}

sp<P2SMEnqueRunData> P2SMNode::addRunData(const RequestPtr &request, P2SMEnqueData &data, const sp<MainData> &mainData)
{
    (void)request;
    TRACE_FUNC_ENTER();
    sp<P2SMEnqueRunData> runData;
    runData = new P2SMEnqueRunData();
    runData->mTuningBuffer = mainData->mTuningBuffer;
    data.mRunData.push_back(runData);
    TRACE_FUNC_EXIT();
    return runData;
}

MBOOL P2SMNode::addRun(Feature::P2Util::DIPParams &params, const RequestPtr &request, const sp<P2SMEnqueRunData> &runData, const sp<MainData> &mainData, const P2IOPack &io)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MTRUE;

    if( mSkip && params.mvDIPFrameParams.size() )
    {
        // TODO: remove for normal p2 run
        MY_S_LOGD(request->mLog, "skip extra p2 run");
        return MTRUE;
    }

    if( io.isValid() )
    {
        TuningParam tuningParam = mainData->mTuningParam;
        tuningParam.pRegBuf = runData->mTuningBuffer->mpVA;
        if( tuningParam.pLcsBuf )
        {
            tuningParam.pLcsBuf = io.mLCSO.mBuffer;
        }
        if( tuningParam.pLceshoBuf )
        {
            tuningParam.pLceshoBuf = io.mLCSHO.mBuffer;
        }

        params.mvDIPFrameParams.push_back(makeDIPFrameParams(mainData->mP2Pack, mSMVRTag, io, runData->mP2Obj.toPtrTable(), tuningParam, request->needRegDump()));
    }
    else
    {
        MY_S_LOGD(request->mLog, "no i/o, skip run");
        ret = MFALSE;
    }

    TRACE_FUNC_EXIT();
    return ret;
}

#if NEED_EXTRA_P2_TUNING
MVOID P2SMNode::copyRunTuning(const sp<P2SMEnqueRunData> &runData, const sp<MainData> &mainData, MUINT32 mask)
{
    TRACE_FUNC_ENTER();
    if( mSingle & mask )
    {
        runData->mTuningBuffer = mTuningPool->request();
        memcpy(runData->mTuningBuffer->mpVA, mainData->mTuningBuffer->mpVA, mTuningPool->getBufSize());
    }
    TRACE_FUNC_EXIT();
}
#endif // NEED_EXTRA_P2_TUNING

MVOID P2SMNode::enqueFeatureStream(Feature::P2Util::DIPParams &params, P2SMEnqueData &data)
{
    TRACE_FUNC_ENTER();
    MY_S_LOGD(data.mRequest->mLog, "sensor(%d) Frame %d enque start", mSensorIndex, data.mRequest->mRequestNo);
    data.mRequest->mTimer.startP2SMEnque();
    this->incExtThreadDependency();
    this->enqueDIPStreamBase(mDIPStream, params, data);
    TRACE_FUNC_EXIT();
}

MVOID P2SMNode::tryAllocateTuningBuffer()
{
    TRACE_FUNC_ENTER();

    if( mTuningCount < (mRecover+NUM_BASIC_TUNING) && mTuningPool != NULL )
    {
        mTuningPool->allocate();
        ++mTuningCount;
    }
    TRACE_FUNC_EXIT();
}

MBOOL P2SMNode::initHalISP()
{
    TRACE_FUNC_ENTER();
    if( mHalISP == NULL && SUPPORT_ISP_HAL )
    {
        mHalISP = MAKE_HalISP(mSensorIndex, PIPE_CLASS_TAG);
    }
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MVOID P2SMNode::uninitHalISP()
{
    TRACE_FUNC_ENTER();
    if( mHalISP )
    {
        mHalISP->destroyInstance(PIPE_CLASS_TAG);
        mHalISP = NULL;
    }
    TRACE_FUNC_EXIT();
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam

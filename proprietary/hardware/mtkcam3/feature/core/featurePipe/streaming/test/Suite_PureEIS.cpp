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

#include <gtest/gtest.h>

#include <cstdlib>
#include <iostream>

#include "DebugControl.h"
#define PIPE_TRACE TRACE_PURE_EIS
#define PIPE_CLASS_TAG "Suite_EIS_Test"
#include <featurePipe/core/include/PipeLog.h>

#include <mtkcam3/feature/featurePipe/IStreamingFeaturePipe.h>
#include <featurePipe/core/include/ImageBufferPool.h>
#include <featurePipe/core/include/GraphicBufferPool.h>
#include <mtkcam3/feature/eis/eis_ext.h>

#include <functional>

#include "TestUtil.h"
#include "TestEISData.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_FPIPE_STREAMING);

using android::sp;
using NSCam::NSIoPipe::QParams;

#define NUM_ENQUE_TEST 200

#define SENSOR_WIDTH  2672
#define SENSOR_HEIGHT 2008
#define SENSOR_SIZE   MSize(SENSOR_WIDTH, SENSOR_HEIGHT)

using NSCam::NSIoPipe::Input;
using NSCam::NSIoPipe::Output;
using NSCam::NSIoPipe::EPortType_Memory;
using NSCam::NSIoPipe::PortID;
using NSImageio::NSIspio::EPortIndex_IMGI;
using NSImageio::NSIspio::EPortIndex_WDMAO;
using NSImageio::NSIspio::EPortIndex_WROTO;
using eAppMode = NSCam::NSCamFeature::NSFeaturePipe::IStreamingFeaturePipe::eAppMode;
#define APP_VIDEO_PREVIEW IStreamingFeaturePipe::APP_VIDEO_PREVIEW
#define APP_VIDEO_RECORD IStreamingFeaturePipe::APP_VIDEO_RECORD
#define APP_VIDEO_STOP IStreamingFeaturePipe::APP_VIDEO_STOP
#define PORTID_IN   0
#define PORTID_OUT  1

#define VAR_EIS_COOKIE "eis_cookie"
#define VAR_EIS_USER   "eis_user"

namespace NSCam{
namespace NSCamFeature{
namespace NSFeaturePipe{

enum QueueUsage
{
    USE_NOQ,
    USE_REQUESTQ,
    USE_TSQ,
};

struct EISInput
{
    EISInput() {}
    EISInput(eAppMode appMode, MINT64 ts)
        : mAppMode(appMode)
        , mTS(ts)
    {}

    eAppMode mAppMode = APP_VIDEO_PREVIEW;
    MINT64 mTS = 0;
};

struct EISOutput
{
    EISOutput() {}
    EISOutput(MBOOL dropRecord, MINT64 ts)
        : mDropRecord(dropRecord)
        , mTS(ts)
    {}

    MBOOL mDropRecord = MFALSE;
    MINT64 mTS = 0;
};

bool operator==(const EISOutput &lhs, const EISOutput &rhs)
{
    return (lhs.mDropRecord == rhs.mDropRecord) &&
           (lhs.mTS == rhs.mTS);
}

typedef std::vector<EISInput> EISInputVector;
typedef std::vector<EISOutput> EISOutputVector;

class Simulator
{
public:
    Simulator(MUINT32 pSize, MUINT32 qSize, QueueUsage qUsage);
    MVOID append(eAppMode appMode, MUINT32 count);
    MVOID clear();

    EISInputVector genEISInputVector();
    EISOutputVector genAnswerVector();

private:
    MINT64 mStartTS = 1000;
    MUINT32 mPSize;
    MUINT32 mQSize;
    QueueUsage mQUsage;
    EISInputVector mEISInputs;
};

Simulator::Simulator(MUINT32 pSize, MUINT32 qSize, QueueUsage qUsage)
    : mPSize(pSize)
    , mQSize(qSize)
    , mQUsage(qUsage)
{
}

MVOID Simulator::append(eAppMode appMode, MUINT32 count)
{
    for( unsigned i = 0; i < count; ++i )
    {
        mEISInputs.push_back(EISInput(appMode, mStartTS + 100*mEISInputs.size()));
    }
}

MVOID Simulator::clear()
{
    mEISInputs.clear();
}

EISInputVector Simulator::genEISInputVector()
{
    return mEISInputs;
}

EISOutputVector Simulator::genAnswerVector()
{
    EISOutputVector answer(mEISInputs.size());
    unsigned size = mEISInputs.size();
    eAppMode last = APP_VIDEO_PREVIEW;
    MBOOL isRec, isQ;
    MUINT32 recNo = 0, qNo = 0;
    MBOOL useTSQ = (mQUsage == USE_TSQ);
    MBOOL useRequestQ = (mQUsage == USE_REQUESTQ);

    for( unsigned i = 0; i < size; ++i )
    {
        isRec = (mEISInputs[i].mAppMode == APP_VIDEO_STOP ||
                 (mEISInputs[i].mAppMode == APP_VIDEO_RECORD && last != APP_VIDEO_STOP));
        isQ = isRec && recNo > mPSize && mEISInputs[i].mAppMode != APP_VIDEO_STOP;

        if( useTSQ )
        {
            // drop first Q request
            if( isQ && qNo < mQSize )
            {
                answer[i].mDropRecord = MTRUE;
            }
            answer[i].mTS = mEISInputs[i].mTS;
            if( isRec && qNo >= mQSize )
            {
                answer[i].mTS = mEISInputs[i-mQSize].mTS;
            }
        }
        else if( useRequestQ && !isRec && qNo )
        {
            // drop last Q request
            for( unsigned j = 0; j < qNo && j < mQSize; ++j )
            {
                answer[i-j-1].mDropRecord = MTRUE;
            }
        }

        recNo = isRec ? recNo+1 : 0;
        qNo = isQ ? qNo+1 : isRec ? qNo : 0;
        last = mEISInputs[i].mAppMode;
    }

    return answer;
}

class EISCookie : public RefBase
{
public:
    EISCookie(MUINT32 id) : mID(id) {}

    MUINT32 mID;
    TestEISData mEISData;
    android::sp<IIBuffer> mInput;
    android::sp<IIBuffer> mDisplay;
    android::sp<IIBuffer> mRecord;
};

class EISUser : public RefBase
{
public:
    EISUser(MUINT32 eisMode);
    virtual ~EISUser();

    MVOID setSimpleSize(MUINT32 factor, const MSize &size);

    MVOID clearPool()
    {
        GraphicBufferPool::destroy(mInputPool);
        ImageBufferPool::destroy(mDisplayPool);
        GraphicBufferPool::destroy(mRecordPool);
    }

    MVOID setEISMode(MUINT32 mode) { mUsageHint.mEISInfo.mMode = mode; }
    MVOID setInputSize(MSize size) { mInputSize = mUsageHint.mStreamingSize = size; }
    MVOID setDisplaySize(MSize size) { mDisplaySize = size; }
    MVOID setRecordSize(MSize size) { mRecordSize = size; }
    MVOID setEISFactor(MUINT32 factor) { mUsageHint.mEISInfo.mFactor = factor; }
    MVOID setEISPQInfo(MUINT32 p, MUINT32 q, QueueUsage useTSQ);

    MBOOL init(MUINT32 inputCount, MUINT32 displayCount, MUINT32 recordCount);
    MVOID uninit();
    MBOOL enque(MUINT32 index, const EISInput &eisInput);
    MVOID flush();
    MVOID sync();

    IStreamingFeaturePipe::UsageHint getUsageHint() { return mUsageHint; }

    MVOID createPool(MUINT32 inputCount, MUINT32 displayCount, MUINT32 recordCount);
    MVOID prepareParam(FeaturePipeParam &param, MUINT32 index, const EISInput &eisInput);
    MBOOL onCB(FeaturePipeParam::MSG_TYPE msg, FeaturePipeParam &data);
    MINT64 getRecordTimestamp(FeaturePipeParam &data);

    android::Mutex mMutex;
    MUINT32 mSensorID = 0;
    IStreamingFeaturePipe::UsageHint mUsageHint;
    MSize mSensorSize;
    MSize mInputSize;
    MSize mDisplaySize;
    MSize mRecordSize;
    MSize mMarginSize;
    EISOutputVector mResults;
    android::sp<GraphicBufferPool> mInputPool;
    android::sp<ImageBufferPool> mDisplayPool;
    android::sp<GraphicBufferPool> mRecordPool;
    MUINT32 mInputCount = 0;
    MUINT32 mDisplayCount = 0;
    MUINT32 mRecordCount = 0;
    IStreamingFeaturePipe *mPipe = NULL;
};

MBOOL onPipeCB(FeaturePipeParam::MSG_TYPE msg, FeaturePipeParam &data)
{
    sp<EISUser> user;
    sp<EISCookie> cookie;
    if( data.tryGetVar<sp<EISUser>>(VAR_EIS_USER, user) && user != NULL)
    {
        user->onCB(msg, data);
    }
    return MTRUE;
}

EISUser::EISUser(MUINT32 eisMode)
    : mSensorSize(SENSOR_SIZE)
{
    mUsageHint.mMode = IStreamingFeaturePipe::USAGE_FULL;
    mUsageHint.mEISInfo.mMode = eisMode;
}

EISUser::~EISUser()
{
    clearPool();
}

MVOID EISUser::setSimpleSize(MUINT32 factor, const MSize &size)
{
    setEISFactor(factor);
    setInputSize(multiply(size, factor/100.0));
    setDisplaySize(size);
    setRecordSize(size);
}

MVOID EISUser::setEISPQInfo(MUINT32 p, MUINT32 q, QueueUsage qUsage)
{
    mUsageHint.mEISInfo.mStartFrame = p;
    mUsageHint.mEISInfo.mQueueSize = q;
    mUsageHint.mEISInfo.mSupportFrame = q;
    mUsageHint.mUseTSQ = (qUsage == USE_TSQ);
}

MVOID EISUser::createPool(MUINT32 inputCount, MUINT32 displayCount, MUINT32 recordCount)
{
    clearPool();
    mInputCount = inputCount;
    mDisplayCount = displayCount;
    mRecordCount = recordCount;

    mInputPool = GraphicBufferPool::create("input", mInputSize.w, mInputSize.h, HAL_PIXEL_FORMAT_YV12, GraphicBufferPool::USAGE_HW_TEXTURE);
    mInputPool->allocate(mInputCount);

    mDisplayPool = ImageBufferPool::create("display", mDisplaySize.w, mDisplaySize.h, eImgFmt_YV12, ImageBufferPool::USAGE_HW);
    mDisplayPool->allocate(mDisplayCount);

    mRecordPool = GraphicBufferPool::create("record", mRecordSize.w, mRecordSize.h, HAL_PIXEL_FORMAT_YV12, GraphicBufferPool::USAGE_HW_TEXTURE);
    mRecordPool->allocate(mRecordCount);
}

MBOOL EISUser::init(MUINT32 inputCount, MUINT32 displayCount, MUINT32 recordCount)
{
    MBOOL ret = MFALSE;
    if( mPipe == NULL )
    {
        double factor = mUsageHint.mEISInfo.mFactor;
        mMarginSize = multiply(mInputSize, (factor-100)/factor);
        createPool(inputCount, displayCount, recordCount);
        mPipe = IStreamingFeaturePipe::createInstance(mSensorID, mUsageHint);
        if( mPipe )
        {
            ret = mPipe->init(PIPE_CLASS_TAG);
        }
    }
    return ret;
}

MVOID EISUser::uninit()
{
    if( mPipe )
    {
        mPipe->uninit(PIPE_CLASS_TAG);
        mPipe->destroyInstance();
        mPipe = NULL;
    }
}

MBOOL EISUser::enque(MUINT32 index, const EISInput &eisInput)
{
    FeaturePipeParam param(onPipeCB);
    prepareParam(param, index, eisInput);
    return mPipe->enque(param);
}

MVOID EISUser::flush()
{
    if( mPipe )
    {
        mPipe->flush();
    }
}

MVOID EISUser::sync()
{
    if( mPipe )
    {
        mPipe->sync();
    }
}

MVOID prepareQParams(QParams &params, IImageBuffer *in, IImageBuffer *display, IImageBuffer *record)
{
    NSCam::NSIoPipe::FrameParams frameParam;
    if( in )
    {
        Input input;
        input.mPortID = PortID(EPortType_Memory, EPortIndex_IMGI, PORTID_IN);
        input.mBuffer = in;
        frameParam.mvIn.push_back(input);
    }
    if( display )
    {
        Output output;
        output.mPortID = PortID(EPortType_Memory, EPortIndex_WROTO, PORTID_OUT);
        output.mBuffer = display;
        output.mPortID.capbility = NSIoPipe::EPortCapbility_Disp;
        frameParam.mvOut.push_back(output);
    }
    if( record )
    {
        Output output;
        output.mPortID = PortID(EPortType_Memory, EPortIndex_WDMAO, PORTID_OUT);
        output.mPortID.capbility = NSIoPipe::EPortCapbility_Rcrd;
        output.mBuffer = record;
        frameParam.mvOut.push_back(output);
    }
    if( in || display || record )
    {
        params.mvFrameParams.push_back(frameParam);
    }
}

MVOID prepareEIS(FeaturePipeParam &param, const TestEISData &data, const MSize &margin, const EISInput &eisInput)
{
    param.setVar<MSize>(VAR_EIS_SENSOR_SIZE, data.mSensorSize);
    param.setVar<MRect>(VAR_EIS_SCALER_CROP, data.mScalerCrop);
    param.setVar<MSize>(VAR_EIS_SCALER_SIZE, data.mScalerOutSize);
    param.setVar<MINT32>(VAR_EIS_GMV_X, data.mGMV.x);
    param.setVar<MINT32>(VAR_EIS_GMV_Y, data.mGMV.y);
    param.setVar<MUINT32>(VAR_EIS_CONF_X, data.mConf.x);
    param.setVar<MUINT32>(VAR_EIS_CONF_Y, data.mConf.y);
    param.setVar<MINT32>(VAR_EIS_EXP_TIME, data.mExpTime);
    param.setVar<MINT64>(VAR_EIS_TIMESTAMP, data.mTimeStamp);
    param.setVar<MSize>(VAR_EIS_RRZO_MARGIN, margin);

    param.setVar<IStreamingFeaturePipe::eAppMode>(VAR_APP_MODE, eisInput.mAppMode);
    param.setVar<MINT64>(VAR_P1_TS, eisInput.mTS);
}

MVOID EISUser::prepareParam(FeaturePipeParam &param, MUINT32 index, const EISInput &eisInput)
{
    QParams qparams;
    sp<EISCookie> cookie = new EISCookie(index);
    param.setFeatureMask(MASK_EIS, true);
    cookie->mEISData = getSimulateEISData(mSensorSize, mUsageHint.mStreamingSize);
    cookie->mInput = mInputPool->requestIIBuffer();
    cookie->mInput->getImageBuffer()->setExtParam(mInputSize);
    cookie->mDisplay = mDisplayPool->requestIIBuffer();
    cookie->mDisplay->getImageBuffer()->setExtParam(mDisplaySize);
    cookie->mRecord = mRecordPool->requestIIBuffer();
    cookie->mRecord->getImageBuffer()->setExtParam(mRecordSize);
    param.setVar<sp<EISUser>>(VAR_EIS_USER, this);
    param.setVar<sp<EISCookie>>(VAR_EIS_COOKIE, cookie);

    IImageBuffer *in = cookie->mInput->getImageBufferPtr();
    IImageBuffer *display = cookie->mDisplay->getImageBufferPtr();
    IImageBuffer *record = cookie->mRecord->getImageBufferPtr();
    prepareQParams(qparams, in, display, record);
    param.setDIPParams(qparams);
    prepareEIS(param, cookie->mEISData, mMarginSize, eisInput);
}

MBOOL EISUser::onCB(FeaturePipeParam::MSG_TYPE msg, FeaturePipeParam &data)
{
    sp<EISUser> user;
    if( msg == FeaturePipeParam::MSG_DISPLAY_DONE )
    {
        sp<EISCookie> cookie;
        if( data.tryGetVar<sp<EISCookie>>(VAR_EIS_COOKIE, cookie) )
        {
            cookie->mInput = NULL;
            cookie->mDisplay = NULL;
        }
    }
    else if( msg == FeaturePipeParam::MSG_FRAME_DONE )
    {
        {
            android::Mutex::Autolock lock(mMutex);
            MBOOL skipRecord = data.getVar<MBOOL>(VAR_EIS_SKIP_RECORD, MFALSE);
            MINT64 ts = getRecordTimestamp(data);
            mResults.push_back(EISOutput(skipRecord, ts));
        }
        data.clearVar<sp<EISUser>>(VAR_EIS_USER);
        data.clearVar<sp<EISCookie>>(VAR_EIS_COOKIE);
    }
    return MTRUE;
}

MINT64 EISUser::getRecordTimestamp(FeaturePipeParam &data)
{
    MINT64 ts = 0;
    const SFPIOMap &generalIO = data.mSFPIOManager.getFirstGeneralIO();
    for(const SFPOutput& out : generalIO.mOutList)
    {
        if(out.mTargetType == SFPOutput::OUT_TARGET_RECORD)
        {
            ts = out.mBuffer->getTimestamp();
        }
    }
    return ts;
}

TEST(PureEIS, SimpleInit)
{
    TRACE_FUNC_ENTER();
    MUINT32 sensorID = 0;
    IStreamingFeaturePipe *pipe = NULL;
    IStreamingFeaturePipe::UsageHint usageHint;
    MUINT32 eisMode = EISMODE_25 | EISMODE_GYRO;
    usageHint.mMode = IStreamingFeaturePipe::USAGE_STEREO_EIS;
    usageHint.mStreamingSize = FHD_SIZE;
    usageHint.mEISInfo.mMode = eisMode;

    ASSERT_TRUE(pipe == NULL);
    pipe = IStreamingFeaturePipe::createInstance(sensorID, usageHint);
    ASSERT_TRUE(pipe != NULL);
    ASSERT_TRUE(pipe->init(PIPE_CLASS_TAG));

    pipe->sync();
    pipe->uninit(PIPE_CLASS_TAG);
    pipe->destroyInstance();
    pipe = NULL;
    TRACE_FUNC_EXIT();
}

MBOOL checkResult(MUINT32 count, const EISOutputVector &result, const EISOutputVector &answer)
{
    MBOOL ret = MTRUE;
    if( count != result.size() || count != answer.size() )
    {
        MY_LOGE("count=%d result.size()=%zu answer.size()=%zu", count, result.size(), answer.size());
        ret = MFALSE;
    }
    else
    {
        for( unsigned i = 0; ret && i < count; ++i )
        {
            ret = (result[i] == answer[i]);
        }
        for( unsigned i = 0; !ret && i < count; ++i )
        {
            MY_LOGE("result[%d]=(%d,%ld) answer=(%d,%ld) match=(%d)", i, result[i].mDropRecord, (long)result[i].mTS, answer[i].mDropRecord, (long)answer[i].mTS, (result[i] == answer[i]));
        }
    }
    return ret;
}

TEST(PureEIS, Full25Gyro)
{
    TRACE_FUNC_ENTER();
    sp<EISUser> user = new EISUser(EISMODE_25|EISMODE_GYRO);
    user->setSimpleSize(125, FHD_SIZE);
    ASSERT_TRUE(user->init(8, 8, 8));

    Simulator sim(0, 0, USE_NOQ);
    sim.append(APP_VIDEO_PREVIEW, 50);
    sim.append(APP_VIDEO_RECORD, NUM_ENQUE_TEST);

    EISInputVector inputs = sim.genEISInputVector();
    for( unsigned i = 0; i < inputs.size(); ++i )
    {
        MY_LOGD("Prepare TestParam #%04d", i);
        MBOOL result = user->enque(i, inputs[i]);
        MY_LOGD("Finish enque #%04d = %d", i, result);
    }

    EISOutputVector answer = sim.genAnswerVector();

    // no EISQ, should not need flush
    // user->flush();
    user->sync();
    user->uninit();

    ASSERT_TRUE(checkResult(inputs.size(), user->mResults, answer));
    TRACE_FUNC_EXIT();
}

TEST(PureEIS, Full25GyroRequestQ)
{
    TRACE_FUNC_ENTER();
    sp<EISUser> user = new EISUser(EISMODE_25|EISMODE_GYRO|EISMODE_QUEUE);
    user->setSimpleSize(125, FHD_SIZE);
    user->setEISPQInfo(19, 25, USE_REQUESTQ);
    ASSERT_TRUE(user->init(8, 8, 8+25));

    Simulator sim(19, 25, USE_REQUESTQ);
    sim.append(APP_VIDEO_PREVIEW, 50);
    sim.append(APP_VIDEO_RECORD, NUM_ENQUE_TEST-50);
    sim.append(APP_VIDEO_STOP, 50);
    sim.append(APP_VIDEO_PREVIEW, 5);

    EISInputVector inputs = sim.genEISInputVector();
    for( unsigned i = 0; i < inputs.size(); ++i )
    {
        MY_LOGD("Prepare TestParam #%04d", i);
        MBOOL result = user->enque(i, inputs[i]);
        MY_LOGD("Finish enque #%04d = %d", i, result);
    }
    // flush remaining eis queue
    // PREVIEW request at end should automatically trigger flush
    // user->flush();
    user->sync();
    user->uninit();

    EISOutputVector answer = sim.genAnswerVector();
    ASSERT_TRUE(checkResult(inputs.size(), user->mResults, answer));
    TRACE_FUNC_EXIT();
}

TEST(PureEIS, Full25GyroNoStopRequestQ)
{
    TRACE_FUNC_ENTER();
    sp<EISUser> user = new EISUser(EISMODE_25|EISMODE_GYRO|EISMODE_QUEUE);
    user->setSimpleSize(125, FHD_SIZE);
    user->setEISPQInfo(19, 25, USE_REQUESTQ);
    ASSERT_TRUE(user->init(8, 8, 8+25));

    Simulator sim(19, 25, USE_REQUESTQ);
    sim.append(APP_VIDEO_PREVIEW, 50);
    sim.append(APP_VIDEO_RECORD, NUM_ENQUE_TEST-50);
    sim.append(APP_VIDEO_PREVIEW, 5);

    EISInputVector inputs = sim.genEISInputVector();
    for( unsigned i = 0; i < inputs.size(); ++i )
    {
        MY_LOGD("Prepare TestParam #%04d", i);
        MBOOL result = user->enque(i, inputs[i]);
        MY_LOGD("Finish enque #%04d = %d", i, result);
    }
    // flush remaining eis queue
    // PREVIEW request at end should automatically trigger flush
    // user->flush();
    user->sync();
    user->uninit();

    EISOutputVector answer = sim.genAnswerVector();
    ASSERT_TRUE(checkResult(inputs.size(), user->mResults, answer));
    TRACE_FUNC_EXIT();
}

TEST(PureEIS, Full30Gyro)
{
    TRACE_FUNC_ENTER();
    sp<EISUser> user = new EISUser(EISMODE_30|EISMODE_GYRO);
    user->setSimpleSize(125, FHD_SIZE);
    ASSERT_TRUE(user->init(8, 8, 8));

    Simulator sim(0, 0, USE_NOQ);
    sim.append(APP_VIDEO_PREVIEW, 50);
    sim.append(APP_VIDEO_RECORD, NUM_ENQUE_TEST-50);
    sim.append(APP_VIDEO_PREVIEW, 5);

    EISInputVector inputs = sim.genEISInputVector();
    for( unsigned i = 0; i < inputs.size(); ++i )
    {
        MY_LOGD("Prepare TestParam #%04d", i);
        MBOOL result = user->enque(i, inputs[i]);
        MY_LOGD("Finish enque #%04d = %d", i, result);
    }
    // flush remaining eis queue
    // no EISQ, should not need flush
    // user->flush();
    user->sync();
    user->uninit();

    EISOutputVector answer = sim.genAnswerVector();
    ASSERT_TRUE(checkResult(inputs.size(), user->mResults, answer));
    TRACE_FUNC_EXIT();
}

TEST(PureEIS, Full30GyroRequestQ)
{
    TRACE_FUNC_ENTER();
    sp<EISUser> user = new EISUser(EISMODE_30|EISMODE_GYRO|EISMODE_QUEUE);
    user->setSimpleSize(125, FHD_SIZE);
    user->setEISPQInfo(19, 25, USE_REQUESTQ);
    ASSERT_TRUE(user->init(8, 8, 8+25));

    Simulator sim(19, 25, USE_REQUESTQ);
    sim.append(APP_VIDEO_PREVIEW, 50);
    sim.append(APP_VIDEO_RECORD, NUM_ENQUE_TEST-50);
    sim.append(APP_VIDEO_STOP, 50);
    sim.append(APP_VIDEO_PREVIEW, 5);

    EISInputVector inputs = sim.genEISInputVector();
    for( unsigned i = 0; i < inputs.size(); ++i )
    {
        MY_LOGD("Prepare TestParam #%04d", i);
        MBOOL result = user->enque(i, inputs[i]);
        MY_LOGD("Finish enque #%04d = %d", i, result);
    }
    // flush remaining eis queue
    // PREVIEW request at end should automatically trigger flush
    // user->flush();
    user->sync();
    user->uninit();

    EISOutputVector answer = sim.genAnswerVector();
    ASSERT_TRUE(checkResult(inputs.size(), user->mResults, answer));
    TRACE_FUNC_EXIT();
}

TEST(PureEIS, Full30GyroNoStopRequestQ)
{
    TRACE_FUNC_ENTER();
    sp<EISUser> user = new EISUser(EISMODE_30|EISMODE_GYRO|EISMODE_QUEUE);
    user->setSimpleSize(125, FHD_SIZE);
    user->setEISPQInfo(19, 25, USE_REQUESTQ);
    ASSERT_TRUE(user->init(8, 8, 8+25));

    Simulator sim(19, 25, USE_REQUESTQ);
    sim.append(APP_VIDEO_PREVIEW, 50);
    sim.append(APP_VIDEO_RECORD, NUM_ENQUE_TEST-50);
    sim.append(APP_VIDEO_PREVIEW, 5);

    EISInputVector inputs = sim.genEISInputVector();
    for( unsigned i = 0; i < inputs.size(); ++i )
    {
        MY_LOGD("Prepare TestParam #%04d", i);
        MBOOL result = user->enque(i, inputs[i]);
        MY_LOGD("Finish enque #%04d = %d", i, result);
    }
    // flush remaining eis queue
    // PREVIEW request at end should automatically trigger flush
    // user->flush();
    user->sync();
    user->uninit();

    EISOutputVector answer = sim.genAnswerVector();
    ASSERT_TRUE(checkResult(inputs.size(), user->mResults, answer));
    TRACE_FUNC_EXIT();
}

TEST(PureEIS, Full30GyroTSQ)
{
    TRACE_FUNC_ENTER();
    sp<EISUser> user = new EISUser(EISMODE_30|EISMODE_GYRO|EISMODE_QUEUE);
    user->setSimpleSize(125, FHD_SIZE);
    user->setEISPQInfo(19, 25, USE_TSQ);
    ASSERT_TRUE(user->init(8, 8, 8+25));

    Simulator sim(19, 25, USE_TSQ);
    sim.append(APP_VIDEO_PREVIEW, 50);
    sim.append(APP_VIDEO_RECORD, NUM_ENQUE_TEST-50);
    sim.append(APP_VIDEO_STOP, 50);
    sim.append(APP_VIDEO_PREVIEW, 5);

    EISInputVector inputs = sim.genEISInputVector();
    for( unsigned i = 0; i < inputs.size(); ++i )
    {
        MY_LOGD("Prepare TestParam #%04d", i);
        MBOOL result = user->enque(i, inputs[i]);
        MY_LOGD("Finish enque #%04d = %d", i, result);
    }
    // flush remaining eis queue
    // TSQ should not need flush
    // user->flush();

    user->sync();
    user->uninit();

    EISOutputVector answer = sim.genAnswerVector();
    ASSERT_TRUE(checkResult(inputs.size(), user->mResults, answer));
    TRACE_FUNC_EXIT();
}

TEST(PureEIS, Full30GyroNoStopTSQ)
{
    TRACE_FUNC_ENTER();
    sp<EISUser> user = new EISUser(EISMODE_30|EISMODE_GYRO|EISMODE_QUEUE);
    user->setSimpleSize(125, FHD_SIZE);
    user->setEISPQInfo(19, 25, USE_TSQ);
    ASSERT_TRUE(user->init(8, 8, 8+25));

    Simulator sim(19, 25, USE_TSQ);
    sim.append(APP_VIDEO_PREVIEW, 50);
    sim.append(APP_VIDEO_RECORD, NUM_ENQUE_TEST-50);
    sim.append(APP_VIDEO_PREVIEW, 5);

    EISInputVector inputs = sim.genEISInputVector();
    for( unsigned i = 0; i < inputs.size(); ++i )
    {
        MY_LOGD("Prepare TestParam #%04d", i);
        MBOOL result = user->enque(i, inputs[i]);
        MY_LOGD("Finish enque #%04d = %d", i, result);
    }
    // flush remaining eis queue
    // user->flush();
    user->sync();
    user->uninit();

    EISOutputVector answer = sim.genAnswerVector();
    ASSERT_TRUE(checkResult(inputs.size(), user->mResults, answer));
    TRACE_FUNC_EXIT();
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam

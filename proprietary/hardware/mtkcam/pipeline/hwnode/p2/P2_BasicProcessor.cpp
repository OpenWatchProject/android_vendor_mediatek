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

#include "P2_BasicProcessor.h"
#include "P2_Util.h"

#define P2_BASIC_THREAD_NAME "p2_basic"
#define FORCE_BURST 0

namespace P2
{

#include "P2_DebugControl.h"
#define P2_CLASS_TAG    BasicProcessor
#define P2_TRACE        TRACE_BASIC_PROCESSOR
#include "P2_LogHeader.h"

BasicProcessor::BasicProcessor()
    : Processor(P2_BASIC_THREAD_NAME)
    , mNormalStream(NULL)
    , mHal3A(NULL)
    , mTuningSize(0)
    , mMDPProcessor("p2_mdp")
    , mEnableVencStream(MFALSE)
{
    MY_LOG_FUNC_ENTER();
    MY_LOG_FUNC_EXIT();
}

BasicProcessor::~BasicProcessor()
{
    MY_LOG_S_FUNC_ENTER(mP2Info);
    this->uninit();
    MY_LOG_S_FUNC_EXIT(mP2Info);
}

MBOOL BasicProcessor::onInit(const P2InitParam &param)
{
    MY_LOG_S_FUNC_ENTER(param.mInfo);
    CAM_TRACE_NAME("P2_Basic:init()");

    MBOOL ret = MFALSE;

    mP2Info = param.mInfo;
    mConfigParam = param.mConfig;
    if( mP2Info == NULL )
    {
        MY_S_LOGE(mP2Info, "Invalid P2Info = NULL");
    }
    else
    {
        ret = initNormalStream() && init3A();
        if( ret )
        {
            mMDPProcessor.init(0);
            mTuningSize = mNormalStream->getRegTableSize();
        }
        else
        {
            uninitNormalStream();
            uninit3A();
        }
    }

    MY_LOG_S_FUNC_EXIT(param.mInfo);
    return ret;
}

MVOID BasicProcessor::onUninit()
{
    MY_LOG_S_FUNC_ENTER(mP2Info);
    CAM_TRACE_NAME("P2_Basic:uninit()");
    uninitNormalStream();
    uninit3A();
    MY_LOG_S_FUNC_EXIT(mP2Info);
}

MVOID BasicProcessor::onThreadStart()
{
    MY_LOG_S_FUNC_ENTER(mP2Info);
    CAM_TRACE_NAME("P2_Basic:threadStart()");
    MY_LOG_S_FUNC_EXIT(mP2Info);
}

MVOID BasicProcessor::onThreadStop()
{
    MY_LOG_S_FUNC_ENTER(mP2Info);
    CAM_TRACE_NAME("P2_Basic:threadStop()");
    MY_LOG_S_FUNC_EXIT(mP2Info);
}

MBOOL BasicProcessor::onConfig(const P2ConfigParam &param)
{
    MY_LOG_S_FUNC_ENTER(mP2Info);
    CAM_TRACE_NAME("P2_Basic:config()");
    mConfigParam = param;
    MY_LOG_S_FUNC_EXIT(mP2Info);
    return MTRUE;
}

MBOOL BasicProcessor::onEnque(const sp<P2Request> &request)
{
    TRACE_S_FUNC_ENTER(request);
    CAM_TRACE_NAME("P2_Basic:enque()");
    MBOOL ret = MFALSE;

    processVenc(request);

    if( request != NULL && request->hasInput() && request->hasOutput() )
    {
        sp<P2Payload> payload = new P2Payload(request);

        if( payload == NULL )
        {
            MY_S_LOGW(request, "cannot allocate Payload");
        }
        else
        {
            MUINT32 portFlag = mEnableVencStream ? P2Util::USE_VENC : 0;
            payload->mIO = P2Util::extractSimpleIO(request, portFlag);
            payload->mIO.setUseLMV(request->getCropper().isEISAppOn() ? MTRUE : MFALSE);

            if( payload->mIO.hasInput() &&
                payload->mIO.hasOutput() )
            {
                P2MetaSet metaSet = request->getMetaSet();
                process3A(payload, metaSet);
                prepareQParams(payload);
                payload->mRequest->releaseResource(P2Request::RES_IN_IMG);
                ret = processBurst(payload) || processP2(payload);
            }
            else
            {
                checkBurst();
            }
        }
    }

    TRACE_S_FUNC_EXIT(request);
    return ret;
}

MVOID BasicProcessor::onNotifyFlush()
{
    MY_LOG_S_FUNC_ENTER(mP2Info);
    CAM_TRACE_NAME("P2_Basic:notifyFlush()");
    if( mBurstQueue.size() )
    {
        updateResult(mBurstQueue, MFALSE);
        mBurstQueue.clear();
    }
    MY_LOG_S_FUNC_EXIT(mP2Info);
}

MVOID BasicProcessor::onWaitFlush()
{
    MY_LOG_S_FUNC_ENTER(mP2Info);
    CAM_TRACE_NAME("P2_Basic:waitFlush()");
    waitP2CBDone();
    mMDPProcessor.flush();
    MY_LOG_S_FUNC_EXIT(mP2Info);
}

MBOOL BasicProcessor::initNormalStream()
{
    TRACE_S_FUNC_ENTER(mP2Info);
    MBOOL ret = MFALSE;

    CAM_TRACE_BEGIN("P2_Basic:NormalStream create");
    mNormalStream = INormalStream::createInstance(mP2Info->mSensorID);
    CAM_TRACE_END();
    if( mNormalStream == NULL )
    {
        MY_S_LOGE(mP2Info, "OOM: cannot create NormalStream");
    }
    else
    {
        CAM_TRACE_BEGIN("P2_Basic:NormalStream init");
        ret = mNormalStream->init(getName());
        CAM_TRACE_END();
        if( !ret )
        {
            MY_S_LOGE(mP2Info, "NormalStream init failed");
        }
    }
    TRACE_S_FUNC_EXIT(mP2Info);
    return ret;
}

MVOID BasicProcessor::uninitNormalStream()
{
    TRACE_S_FUNC_ENTER(mP2Info);
    if( mNormalStream )
    {
        configVencStream(MFALSE);
        mNormalStream->uninit(getName());
        mNormalStream->destroyInstance();
        mNormalStream = NULL;
        mMDPProcessor.uninit();
    }
    TRACE_S_FUNC_EXIT(mP2Info);
}

MBOOL BasicProcessor::init3A()
{
    TRACE_S_FUNC_ENTER(mP2Info);
    MBOOL ret = MTRUE;
    CAM_TRACE_BEGIN("P2_Basic:3A create");
    mHal3A = MAKE_Hal3A(mP2Info->mSensorID, getName());
    CAM_TRACE_END();
    if( mHal3A == NULL )
    {
        MY_S_LOGE(mP2Info, "OOM: cannot create Hal3A");
        ret = MFALSE;
    }
    TRACE_S_FUNC_EXIT(mP2Info);
    return ret;
}

MVOID BasicProcessor::uninit3A()
{
    TRACE_S_FUNC_ENTER(mP2Info);
    if( mHal3A != NULL )
    {
        mHal3A->destroyInstance(getName());
        mHal3A = NULL;
    }
    TRACE_S_FUNC_EXIT(mP2Info);
}

MVOID BasicProcessor::onP2CB(const QParams &qparams, sp<P2Payload> &payload)
{
    TRACE_S_FUNC_ENTER(payload->mRequest);
    CAM_TRACE_NAME("P2_Basic:onP2CB()");
    payload->mIO.updateResult(qparams.mDequeSuccess);
    MDPParam mdpParam;
    mdpParam.mRequest = payload->mRequest;
    mdpParam.mSrc = payload->mIO.getMDPSrc();
    for( auto &&out : payload->mRequest->mImgOutArray )
    {
        if( isValid(out) )
        {
            mdpParam.mDst.push_back(out);
        }
    }
    mMDPProcessor.enque(mdpParam);
    TRACE_S_FUNC_EXIT(payload->mRequest);
}

MVOID BasicProcessor::process3A(sp<P2Payload> &payload, P2MetaSet &metaSet)
{
    TRACE_S_FUNC_EXIT(payload->mRequest);
    CAM_TRACE_NAME("P2_Basic:process3A()");
    sp<P2Request> &request = payload->mRequest;
    const P2Util::SimpleIO &io = payload->mIO;
    TuningParam &tuning = payload->mTuning;

    P2Util::process3A(io, mTuningSize, mHal3A, tuning, metaSet, request->getLogger());
    P2Util::updateDebugExif(metaSet.mInHal, metaSet.mOutHal, request->getLogger());
    P2Util::updateExtraMeta(request->getExtraData(), metaSet.mOutHal, request->getLogger());
    request->updateMetaSet(metaSet);
    TRACE_S_FUNC_ENTER(payload->mRequest);
}

MVOID BasicProcessor::prepareQParams(sp<P2Payload> &payload)
{
    TRACE_S_FUNC_ENTER(payload->mRequest);
    sp<P2Request> &request = payload->mRequest;
    const P2Util::SimpleIO &io = payload->mIO;
    TuningParam &tuning = payload->mTuning;
    QParams &qparams = payload->mQParams;

    qparams = P2Util::makeSimpleQParams(ENormalStreamTag_Prv, io, tuning, request->getCropper(), request->getLogger());
    P2Util::prepareExtraModule(qparams, request->getExtraData(), request->getLogger());
    TRACE_S_FUNC_EXIT(payload->mRequest);
}

MBOOL BasicProcessor::processVenc(const sp<P2Request> &request)
{
    TRACE_S_FUNC_ENTER(request);
    MBOOL ret = MTRUE;
    if( request != NULL )
    {
        MINT32 fps = 0;
        MSize size;
        sp<P2Meta> meta = request->getMeta(IN_P1_HAL);
        if( tryGet<MINT32>(meta, MTK_P2NODE_HIGH_SPEED_VDO_FPS, fps) &&
            tryGet<MSize>(meta, MTK_P2NODE_HIGH_SPEED_VDO_SIZE, size) )
        {
            MBOOL enable = fps && size.w && size.h;
            ret = configVencStream(enable, fps, size);
        }
    }
    TRACE_S_FUNC_EXIT(request);
    return ret;
}

MBOOL BasicProcessor::configVencStream(MBOOL enable, MINT32 fps, MSize size)
{
    TRACE_S_FUNC_ENTER(mP2Info);
    MBOOL ret = MTRUE;
    if( enable != mEnableVencStream )
    {
        ret = enable ? mNormalStream->sendCommand(ESDCmd_CONFIG_VENC_DIRLK, fps, size.w, size.h)
                     : mNormalStream->sendCommand(ESDCmd_RELEASE_VENC_DIRLK);
        if( !ret )
        {
            MY_S_LOGW(mP2Info, "Config venc stream failed: enable(%d) fps(%d) size(%dx%d)", enable, fps, size.w, size.h);
        }
        else
        {
            mEnableVencStream = enable;
        }
    }
    TRACE_S_FUNC_EXIT(mP2Info);
    return ret;
}

BasicProcessor::P2Payload::P2Payload()
{
}

BasicProcessor::P2Payload::P2Payload(const sp<P2Request> &request)
    : mRequest(request)
{
}

BasicProcessor::P2Payload::~P2Payload()
{
    P2Util::releaseTuning(mTuning);
    P2Util::releaseExtraModule(mQParams);
}

BasicProcessor::P2Cookie::P2Cookie(BasicProcessor *parent, const sp<P2Payload> &payload)
    : mParent(parent)
{
    mPayloads.push_back(payload);
}

BasicProcessor::P2Cookie::P2Cookie(BasicProcessor *parent, const std::vector<sp<P2Payload>> &payloads)
    : mParent(parent)
    , mPayloads(payloads)
{
}

Logger BasicProcessor::P2Cookie::getLogger()
{
    Logger logger = NULL;
    if( mPayloads.size() && mPayloads[0]->mRequest != NULL )
    {
        logger = mPayloads[0]->mRequest->getLogger();
    }
    return logger;
}

Logger BasicProcessor::getLogger(const sp<P2Payload> &payload)
{
    return payload->mRequest->getLogger();
}

Logger BasicProcessor::getLogger(const std::vector<sp<P2Payload>> &payloads)
{
    return payloads[0]->mRequest->getLogger();
}

template <typename T>
MBOOL BasicProcessor::processP2(T payload)
{
    Logger logger = getLogger(payload);
    TRACE_S_FUNC_ENTER(logger);
    MBOOL ret = MFALSE;
    P2Cookie *cookie = createCookie(payload, logger);
    if( cookie != NULL )
    {
        CAM_TRACE_NAME("P2_Basic:drv enq");
        QParams qparams = prepareEnqueQParams(payload);
        qparams.mpCookie = (void*)cookie;
        qparams.mpfnCallback = BasicProcessor::p2CB;
        ret = mNormalStream->enque(qparams);
        if( !ret )
        {
            MY_S_LOGW(logger, "enque failed");
            updateResult(payload, MFALSE);
            freeCookie(cookie, NO_CHECK_ORDER);
        }
    }
    TRACE_S_FUNC_EXIT(logger);
    return ret;
}

QParams BasicProcessor::prepareEnqueQParams(sp<P2Payload> payload)
{
    TRACE_S_FUNC_ENTER(payload->mRequest);
    TRACE_S_FUNC_EXIT(payload->mRequest);
    return payload->mQParams;
}

QParams BasicProcessor::prepareEnqueQParams(std::vector<sp<P2Payload>> payloads)
{
    TRACE_S_FUNC_ENTER(payloads[0]->mRequest);
    QParams qparams;
    for( sp<P2Payload> payload : payloads )
    {
        qparams.mvFrameParams.appendVector(payload->mQParams.mvFrameParams);
    }
    TRACE_S_FUNC_EXIT(payloads[0]->mRequest);
    return qparams;
}

MVOID BasicProcessor::updateResult(sp<P2Payload> payload, MBOOL result)
{
    TRACE_S_FUNC_ENTER(payload->mRequest);
    payload->mQParams.mDequeSuccess = result;
    payload->mRequest->updateResult(result);
    TRACE_S_FUNC_EXIT(payload->mRequest);
}

MVOID BasicProcessor::updateResult(std::vector<sp<P2Payload>> payloads, MBOOL result)
{
    TRACE_S_FUNC_ENTER(payloads[0]->mRequest);
    for( sp<P2Payload> payload : payloads )
    {
        payload->mQParams.mDequeSuccess = result;
        payload->mRequest->updateResult(result);
    }
    TRACE_S_FUNC_EXIT(payloads[0]->mRequest);
}

MVOID BasicProcessor::processP2CB(const QParams &qparams, P2Cookie *cookie)
{
    TRACE_S_FUNC_ENTER(mP2Info);
    if( cookie )
    {
        for( sp<P2Payload> payload : cookie->mPayloads )
        {
            payload->mQParams.mDequeSuccess = qparams.mDequeSuccess;
            onP2CB(payload->mQParams, payload);
        }
        freeCookie(cookie, CHECK_ORDER);
        cookie = NULL;
    }
    TRACE_S_FUNC_EXIT(mP2Info);
}

template <typename T>
BasicProcessor::P2Cookie* BasicProcessor::createCookie(const T &payload, const Logger &logger)
{
    TRACE_S_FUNC_ENTER(logger);
    P2Cookie *cookie = NULL;
    cookie = new P2Cookie(this, payload);
    if( cookie == NULL )
    {
        MY_S_LOGE(logger, "OOM: cannot create P2Cookie");
    }
    else
    {
        android::Mutex::Autolock _lock(mP2CookieMutex);
        mP2CookieList.push_back(cookie);
    }
    TRACE_S_FUNC_EXIT(logger);
    return cookie;
}

MBOOL BasicProcessor::freeCookie(P2Cookie *cookie, MBOOL checkOrder)
{
    TRACE_S_FUNC_ENTER(mP2Info);
    MBOOL ret = MFALSE;
    if( cookie == NULL )
    {
        MY_S_LOGW(mP2Info, "invalid cookie = NULL");
    }
    else
    {
        android::Mutex::Autolock _lock(mP2CookieMutex);
        auto it = find(mP2CookieList.begin(), mP2CookieList.end(), cookie);
        if( it != mP2CookieList.end() )
        {
            if( checkOrder && it != mP2CookieList.begin() )
            {
                MY_S_LOGW(cookie->getLogger(), "callback out of order");
            }
            delete cookie;
            cookie = NULL;
            mP2CookieList.erase(it);
            mP2Condition.broadcast();
            ret = MTRUE;
        }
        else
        {
            MY_S_LOGE(mP2Info, "Cookie not freed: invalid data=%p", cookie);
        }
    }
    TRACE_S_FUNC_EXIT(mP2Info);
    return ret;
}

MVOID BasicProcessor::p2CB(QParams &qparams)
{
    TRACE_FUNC_ENTER();
    P2Cookie *cookie = (P2Cookie*)qparams.mpCookie;
    if( cookie && cookie->mParent )
    {
        cookie->mParent->processP2CB(qparams, cookie);
    }
    TRACE_FUNC_EXIT();
}

MVOID BasicProcessor::waitP2CBDone()
{
    TRACE_S_FUNC_ENTER(mP2Info);
    android::Mutex::Autolock _lock(mP2CookieMutex);
    while( mP2CookieList.size() )
    {
        mP2Condition.wait(mP2CookieMutex);
    }
    TRACE_S_FUNC_EXIT(mP2Info);
}

MBOOL BasicProcessor::processBurst(sp<P2Payload> payload)
{
    TRACE_S_FUNC_ENTER(payload->mRequest);
    MUINT32 burst = payload->mRequest->getExtraData().mBurstNum;
    burst = FORCE_BURST ? 4 : burst;
    burst = payload->mRequest->isResized() ? burst : 0;
    if( burst > 1)
    {
        mBurstQueue.push_back(payload);
    }
    if( mBurstQueue.size() && mBurstQueue.size() >= burst )
    {
        processP2(mBurstQueue);
        mBurstQueue.clear();
    }
    TRACE_S_FUNC_EXIT(payload->mRequest);
    return (burst > 1);
}

MBOOL BasicProcessor::checkBurst()
{
    TRACE_S_FUNC_ENTER(mP2Info);
    MBOOL ret = MFALSE;
    if( mBurstQueue.size() )
    {
        processP2(mBurstQueue);
        mBurstQueue.clear();
        ret = MTRUE;
    }
    TRACE_S_FUNC_EXIT(mP2Info);
    return ret;
}

} // namespace P2

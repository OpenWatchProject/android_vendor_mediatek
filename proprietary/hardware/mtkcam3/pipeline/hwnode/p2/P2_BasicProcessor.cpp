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

#define P2_EXPECT_MS 28

namespace P2
{

#include "P2_DebugControl.h"
#define P2_CLASS_TAG    BasicProcessor
#define P2_TRACE        TRACE_BASIC_PROCESSOR
#include "P2_LogHeader.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_P2_BASIC_PROC);

BasicProcessor::BasicProcessor()
    : Processor(P2_BASIC_THREAD_NAME)
    , mMDPProcessor("p2_mdp")
{
//    MY_LOG_S_FUNC_ENTER(mLog);
    mExpectMS = property_get_int32("vendor.debug.p2basic.expect", P2_EXPECT_MS);
//    MY_LOG_S_FUNC_EXIT(mLog);
}

BasicProcessor::~BasicProcessor()
{
//    MY_LOG_S_FUNC_ENTER(mLog);
    this->uninit();
//    MY_LOG_S_FUNC_EXIT(mLog);
}

MBOOL BasicProcessor::onInit(const P2InitParam &param)
{
    ILog log = param.mP2Info.mLog;
    MY_LOG_S_FUNC_ENTER(log);
    P2_CAM_TRACE_NAME(TRACE_ADVANCED, "P2_Basic:init()");

    MBOOL ret = MFALSE;

    mP2Info = param.mP2Info;
    mLog = mP2Info.mLog;
    ret = initNormalStream() && init3A();
    if( ret )
    {
        mMDPProcessor.init(0);
    }
    else
    {
        uninitNormalStream();
        uninit3A();
    }
    MY_LOGI("p2 ETime=%d ret=%d", mExpectMS, ret);
    MY_LOG_S_FUNC_EXIT(log);
    return ret;
}

MVOID BasicProcessor::onUninit()
{
    MY_LOG_S_FUNC_ENTER(mLog);
    P2_CAM_TRACE_NAME(TRACE_ADVANCED, "P2_Basic:uninit()");
    uninitNormalStream();
    uninit3A();
    MY_LOGI("p2 ETime=%d", mExpectMS);
    MY_LOG_S_FUNC_EXIT(mLog);
}

MVOID BasicProcessor::onThreadStart()
{
    MY_LOG_S_FUNC_ENTER(mLog);
    P2_CAM_TRACE_NAME(TRACE_ADVANCED, "P2_Basic:threadStart()");
    MY_LOG_S_FUNC_EXIT(mLog);
}

MVOID BasicProcessor::onThreadStop()
{
    MY_LOG_S_FUNC_ENTER(mLog);
    P2_CAM_TRACE_NAME(TRACE_ADVANCED, "P2_Basic:threadStop()");
    MY_LOG_S_FUNC_EXIT(mLog);
}

MBOOL BasicProcessor::onConfig(const P2ConfigParam &param)
{
    MY_LOG_S_FUNC_ENTER(mLog);
    P2_CAM_TRACE_NAME(TRACE_ADVANCED, "P2_Basic:config()");
    mP2Info = param.mP2Info;
    MY_LOG_S_FUNC_EXIT(mLog);
    return MTRUE;
}

MBOOL BasicProcessor::onEnque(const sp<P2Request> &request)
{
    ILog log = spToILog(request);
    TRACE_S_FUNC_ENTER(log);
    P2_CAM_TRACE_NAME(TRACE_ADVANCED, "P2_Basic:enque()");
    MBOOL ret = MFALSE;
    request->beginBatchRelease();

    processVenc(request);

    if( request != NULL && request->hasInput() && request->hasOutput() )
    {
        sp<P2Payload> payload = new P2Payload(request, ++mCounter);
        payload->mTimer.startEnque();
        MUINT32 portFlag = mEnableVencStream ? P2Util::USE_VENC : 0;
        payload->mIO = P2Util::extractSimpleIO(request, portFlag);
        payload->mIO.setUseLMV(request->getCropper()->isEISAppOn());

        if( payload->mIO.hasInput() &&
            payload->mIO.hasOutput() )
        {
            P2MetaSet metaSet = request->getMetaSet();
            payload->mTimer.startSetIsp();
            payload->mTuning = P2Util::xmakeTuning(request->mP2Pack, payload->mIO, mHalISP, metaSet);
            payload->mTimer.stopSetIsp();
            request->updateMetaSet(metaSet);
            payload->mDIPParams = P2Util::xmakeDIPParams(request->mP2Pack, payload->mIO, payload->mTuning, payload->mP2Obj.toPtrTable());

            payload->mRequest->releaseResource(P2Request::RES_IMG);
            ret = processBurst(payload) || processP2(payload);
        }
        else
        {
            checkBurst();
        }
    }

    TRACE_S_FUNC_EXIT(log);
    return ret;
}

MVOID BasicProcessor::onNotifyFlush()
{
    MY_LOG_S_FUNC_ENTER(mLog);
    P2_CAM_TRACE_NAME(TRACE_ADVANCED, "P2_Basic:notifyFlush()");
    MY_LOG_S_FUNC_EXIT(mLog);
}

MVOID BasicProcessor::onWaitFlush()
{
    MY_LOG_S_FUNC_ENTER(mLog);
    P2_CAM_TRACE_NAME(TRACE_ADVANCED, "P2_Basic:waitFlush()");
    waitP2CBDone();
    mMDPProcessor.flush();
    if( mBurstQueue.size() )
    {
        updateResult(mBurstQueue, MFALSE);
        mBurstQueue.clear();
    }
    MY_LOG_S_FUNC_EXIT(mLog);
}

MBOOL BasicProcessor::initNormalStream()
{
    TRACE_S_FUNC_ENTER(mLog);
    MBOOL ret = MFALSE;

    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "P2_Basic:NormalStream create");
    mDIPStream = DIPStream::createInstance(mP2Info.getConfigInfo().mMainSensorID);
    P2_CAM_TRACE_END(TRACE_ADVANCED);
    if( mDIPStream == NULL )
    {
        MY_S_LOGE(mLog, "OOM: cannot create NormalStream");
    }
    else
    {
        P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "P2_Basic:NormalStream init");
        ret = mDIPStream->init(getName());
        P2_CAM_TRACE_END(TRACE_ADVANCED);
        if( !ret )
        {
            MY_S_LOGE(mLog, "NormalStream init failed");
        }
    }
    TRACE_S_FUNC_EXIT(mLog);
    return ret;
}

MVOID BasicProcessor::uninitNormalStream()
{
    TRACE_S_FUNC_ENTER(mLog);
    if( mDIPStream )
    {
        configVencStream(MFALSE);
        mDIPStream->uninit(getName());
        mDIPStream->destroyInstance();
        mDIPStream = NULL;
        mMDPProcessor.uninit();
    }
    TRACE_S_FUNC_EXIT(mLog);
}

MBOOL BasicProcessor::init3A()
{
    TRACE_S_FUNC_ENTER(mLog);
    MBOOL ret = MTRUE;
    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "P2_Basic:3A create");
    mHal3A = MAKE_Hal3A(mP2Info.getConfigInfo().mMainSensorID, getName());
    mHalISP = MAKE_HalISP(mP2Info.getConfigInfo().mMainSensorID, getName());
    P2_CAM_TRACE_END(TRACE_ADVANCED);
    if( mHal3A == NULL )
    {
        MY_S_LOGE(mLog, "OOM: cannot create Hal3A");
        ret = MFALSE;
    }

    if( mHalISP == NULL )
    {
        MY_S_LOGE(mLog, "OOM: cannot create HalISP");
        ret = MFALSE;
    }
    TRACE_S_FUNC_EXIT(mLog);
    return ret;
}

MVOID BasicProcessor::uninit3A()
{
    TRACE_S_FUNC_ENTER(mLog);
    if( mHal3A != NULL )
    {
        mHal3A->destroyInstance(getName());
        mHal3A = NULL;
    }
    if( mHalISP != NULL )
    {
        mHalISP->destroyInstance(getName());
        mHalISP = NULL;
    }
    TRACE_S_FUNC_EXIT(mLog);
}

MVOID BasicProcessor::onP2CB(const DIPParams &dipParams, sp<P2Payload> &payload)
{
    TRACE_S_FUNC_ENTER(payload->mRequest->mLog);
    P2_CAM_TRACE_NAME(TRACE_ADVANCED, "P2_Basic:onP2CB()");
    mFPSCounter.update();
    payload->mTimer.setFPS(mFPSCounter.getFPS());
    payload->mTimer.stopP2();
    payload->mTimer.startHelper();
    payload->mIO.updateResult(dipParams.mDequeSuccess);
    payload->mRequest->updateMetaResult(dipParams.mDequeSuccess);
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
    if( mdpParam.mDst.size() > 0)
    {
        mMDPProcessor.enque(mdpParam);
    }
    TRACE_S_FUNC_EXIT(payload->mRequest->mLog, "MDP Proc Dst Size(%zu)", mdpParam.mDst.size());
}

MBOOL BasicProcessor::processVenc(const sp<P2Request> &request)
{
    TRACE_S_FUNC_ENTER(request->mLog);
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
    TRACE_S_FUNC_EXIT(request->mLog);
    return ret;
}

MBOOL BasicProcessor::configVencStream(MBOOL enable, MINT32 fps, MSize size)
{
    TRACE_S_FUNC_ENTER(mLog);
    MBOOL ret = MTRUE;
    if( enable != mEnableVencStream )
    {
        ret = enable ? mDIPStream->sendCommand(ESDCmd_CONFIG_VENC_DIRLK, fps, size.w, size.h)
                     : mDIPStream->sendCommand(ESDCmd_RELEASE_VENC_DIRLK);
        if( !ret )
        {
            MY_S_LOGW(mLog, "Config venc stream failed: enable(%d) fps(%d) size(%dx%d)", enable, fps, size.w, size.h);
        }
        else
        {
            mEnableVencStream = enable;
        }
    }
    TRACE_S_FUNC_EXIT(mLog);
    return ret;
}

BasicProcessor::MyTimer::MyTimer()
{
}

BasicProcessor::MyTimer::MyTimer(const ILog &log, MUINT32 index)
    : mLog(log)
    , mIndex(index)
{
}

MVOID BasicProcessor::MyTimer::print() const
{
    MUINT32 total = getElapsedTotal();
    MUINT32 enque = getElapsedEnque();
    MUINT32 isp = getElapsedSetIsp();
    MUINT32 deque = getElapsedDeque();
    MUINT32 p2 = getElapsedP2();
    MUINT32 helper = getElapsedHelper();
    double fps = getFPS();

    MY_S_LOGD(mLog, "Frame timer [#%5d/%4d][t%4d][en%3d/%3d(isp)][de%3d/%3d(p2)/%3d(mw)][fps%.2f]",
       mIndex, mIndex, total, enque, isp, deque, p2, helper, fps);
}

MVOID BasicProcessor::MyTimer::setFPS(double fps)
{
    mFPS = fps;
}

double BasicProcessor::MyTimer::getFPS() const
{
    return mFPS;
}

BasicProcessor::P2Payload::P2Payload()
{
    mTimer.startTotal();
}

BasicProcessor::P2Payload::P2Payload(const sp<P2Request> &request, MUINT32 counter)
    : mRequest(request)
    , mTimer(spToILog(request), counter)
{
    mTimer.startTotal();
    mP2Obj.ispObj.run = NSCam::NSIoPipe::P2_RUN_S_P2A;
}

BasicProcessor::P2Payload::~P2Payload()
{
    P2Util::releaseTuning(mTuning);
    mRequest = NULL;
    mTimer.stopHelper();
    mTimer.stopDeque();
    mTimer.stopTotal();
    mTimer.print();
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

BasicProcessor::P2Cookie::~P2Cookie()
{
    for( sp<P2Payload> &payload : mPayloads )
    {
        payload = NULL;
    }
}

ILog BasicProcessor::P2Cookie::getILog()
{
    ILog log;
    if( mPayloads.size() && mPayloads[0]->mRequest != NULL )
    {
        log = mPayloads[0]->mRequest->mLog;
    }
    return log;
}

ILog BasicProcessor::getILog(const sp<P2Payload> &payload)
{
    return payload->mRequest->mLog;
}

ILog BasicProcessor::getILog(const std::vector<sp<P2Payload>> &payloads)
{
    return payloads[0]->mRequest->mLog;
}

template <typename T>
MBOOL BasicProcessor::processP2(T payload)
{
    ILog log = getILog(payload);
    TRACE_S_FUNC_ENTER(log);
    MBOOL ret = MFALSE;
    P2Cookie *cookie = createCookie(log, payload);
    if( cookie != NULL )
    {
        P2_CAM_TRACE_NAME(TRACE_ADVANCED, "P2_Basic:drv enq");
        DIPParams dipParams;
        dipParams = prepareEnqueDIPParams(payload);
        dipParams.mpCookie = (void*)cookie;
        dipParams.mpfnDIPCallback = BasicProcessor::p2DIPCB;
        ret = mDIPStream->enque(dipParams);
        if( !ret )
        {
            MY_S_LOGW(log, "enque failed");
            updateResult(payload, MFALSE);
            freeCookie(cookie, NO_CHECK_ORDER);
        }
    }
    TRACE_S_FUNC_EXIT(log);
    return ret;
}

DIPParams BasicProcessor::prepareEnqueDIPParams(sp<P2Payload> payload)
{
    TRACE_S_FUNC_ENTER(payload->mRequest->mLog);
    DIPParams dipParams = payload->mDIPParams;
    NSCam::Feature::P2Util::updateExpectEndTime(dipParams, mExpectMS);
    NSCam::Feature::P2Util::printDIPParams(getILog(payload), dipParams);
    payload->mTimer.stopEnque();
    payload->mTimer.startDeque();
    payload->mTimer.startP2();
    TRACE_S_FUNC_EXIT(payload->mRequest->mLog);
    return dipParams;
}

DIPParams BasicProcessor::prepareEnqueDIPParams(std::vector<sp<P2Payload>> payloads)
{
    TRACE_S_FUNC_ENTER(payloads[0]->mRequest->mLog);
    DIPParams dipParams;
    for( sp<P2Payload> payload : payloads )
    {
        dipParams.mvDIPFrameParams.insert(dipParams.mvDIPFrameParams.end(), payload->mDIPParams.mvDIPFrameParams.begin(), payload->mDIPParams.mvDIPFrameParams.end());
        payload->mTimer.startDeque();
        payload->mTimer.startP2();
    }
    NSCam::Feature::P2Util::updateExpectEndTime(dipParams, mExpectMS);
    TRACE_S_FUNC_EXIT(payloads[0]->mRequest->mLog);
    return dipParams;
}

MVOID BasicProcessor::updateResult(sp<P2Payload> payload, MBOOL result)
{
    TRACE_S_FUNC_ENTER(payload->mRequest->mLog);
    payload->mTimer.stopP2();
    payload->mTimer.startHelper();
    payload->mDIPParams.mDequeSuccess = result;
    payload->mRequest->updateResult(result);
    payload->mRequest->updateMetaResult(result);
    TRACE_S_FUNC_EXIT(payload->mRequest->mLog);
}

MVOID BasicProcessor::updateResult(std::vector<sp<P2Payload>> payloads, MBOOL result)
{
    TRACE_S_FUNC_ENTER(payloads[0]->mRequest->mLog);
    for( sp<P2Payload> payload : payloads )
    {
        payload->mTimer.stopP2();
        payload->mTimer.startHelper();
        payload->mDIPParams.mDequeSuccess = result;
        payload->mRequest->updateResult(result);
        payload->mRequest->updateMetaResult(result);
    }
    TRACE_S_FUNC_EXIT(payloads[0]->mRequest->mLog);
}

MVOID BasicProcessor::processP2CB(const DIPParams &dipParams, P2Cookie *cookie)
{
    TRACE_S_FUNC_ENTER(mLog);
    if( cookie )
    {
        // release display buf asap
        MBOOL found = MFALSE;
        for( sp<P2Payload> payload : cookie->mPayloads )
        {
            found = payload->mIO.findDisplayAndRelease(dipParams.mDequeSuccess);
            if (found)
            {
                payload->mRequest->endBatchRelease();
                payload->mRequest->beginBatchRelease();
                break;
            }
        }
        if (found == MFALSE)
        {
            MY_LOGW("smvrC: display buffer NOT found!!");
        }

        // release the rest
        for( sp<P2Payload> payload : cookie->mPayloads )
        {
            payload->mDIPParams.mDequeSuccess = dipParams.mDequeSuccess;
            onP2CB(payload->mDIPParams, payload);
        }
        freeCookie(cookie, CHECK_ORDER);
        cookie = NULL;
    }
    TRACE_S_FUNC_EXIT(mLog);
}

template <typename T>
BasicProcessor::P2Cookie* BasicProcessor::createCookie(const ILog &log, const T &payload)
{
    TRACE_S_FUNC_ENTER(log);
    P2Cookie *cookie = NULL;
    cookie = new P2Cookie(this, payload);
    if( cookie == NULL )
    {
        MY_S_LOGE(log, "OOM: cannot create P2Cookie");
    }
    else
    {
        android::Mutex::Autolock _lock(mP2CookieMutex);
        mP2CookieList.push_back(cookie);
    }
    TRACE_S_FUNC_EXIT(log);
    return cookie;
}

MBOOL BasicProcessor::freeCookie(P2Cookie *cookie, MBOOL checkOrder)
{
    TRACE_S_FUNC_ENTER(mLog);
    MBOOL ret = MFALSE;
    if( cookie == NULL )
    {
        MY_S_LOGW(mLog, "invalid cookie = NULL");
    }
    else
    {
        android::Mutex::Autolock _lock(mP2CookieMutex);
        auto it = find(mP2CookieList.begin(), mP2CookieList.end(), cookie);
        if( it != mP2CookieList.end() )
        {
            if( checkOrder && it != mP2CookieList.begin() )
            {
                MY_S_LOGW(cookie->getILog(), "callback out of order");
            }
            delete cookie;
            cookie = NULL;
            mP2CookieList.erase(it);
            mP2Condition.broadcast();
            ret = MTRUE;
        }
        else
        {
            MY_S_LOGE(mLog, "Cookie not freed: invalid data=%p", cookie);
        }
    }
    TRACE_S_FUNC_EXIT(mLog);
    return ret;
}

MVOID BasicProcessor::p2DIPCB(DIPParams &dipParams)
{
    TRACE_FUNC_ENTER();
    P2Cookie *cookie = (P2Cookie*)dipParams.mpCookie;
    if( cookie && cookie->mParent )
    {
        cookie->mParent->processP2CB(dipParams, cookie);
    }
    TRACE_FUNC_EXIT();
}

MVOID BasicProcessor::waitP2CBDone()
{
    TRACE_S_FUNC_ENTER(mLog);
    android::Mutex::Autolock _lock(mP2CookieMutex);
    while( mP2CookieList.size() )
    {
        mP2Condition.wait(mP2CookieMutex);
    }
    TRACE_S_FUNC_EXIT(mLog);
}

MBOOL BasicProcessor::processBurst(const sp<P2Payload> &payload)
{
    TRACE_S_FUNC_ENTER(payload->mRequest->mLog);
    MBOOL ret = MFALSE;
    MUINT32 burst = payload->mRequest->mP2Pack.getConfigInfo().mBurstNum;
    burst = FORCE_BURST ? 4 : burst;
    burst = payload->mRequest->isResized() ? burst : 0;

    MUINT8 reqtSmvrFps = (MUINT8) MTK_SMVR_FPS_30;
    if ( payload->mRequest != NULL )
    {
        sp<P2Meta> spHalMeta = payload->mRequest->getMeta(IN_P1_HAL);
        tryGet<MUINT8>(spHalMeta, MTK_HAL_REQUEST_SMVR_FPS, reqtSmvrFps);
    }
    payload->mTimer.stopEnque();

    if ( reqtSmvrFps != (MUINT8) MTK_SMVR_FPS_30 )
    {
        if( burst > 1)
        {
            mBurstQueue.push_back(payload);
            ret = MTRUE;
        }
        if( mBurstQueue.size() && mBurstQueue.size() >= burst )
        {
            processP2(mBurstQueue);
            mBurstQueue.clear();
        }
    }
    else if ( prevReqtSmvrFps != reqtSmvrFps )
    {
        if ( mBurstQueue.size() > 0 )
        {
            MY_S_LOGW(payload->mRequest->mLog, "burstQ-size(%zu) > 0 when fps-change ( %d -> %d )",
                mBurstQueue.size(), prevReqtSmvrFps, reqtSmvrFps);
        }

        mBurstQueue.push_back(payload);
        processP2(mBurstQueue);
        mBurstQueue.clear();
        ret = MTRUE;
    }

    TRACE_S_FUNC_EXIT(payload->mRequest->mLog, "fps-change( %d -> %d ), burst(%d), queueSize(%zu)",
        prevReqtSmvrFps, reqtSmvrFps, burst, mBurstQueue.size() );

    prevReqtSmvrFps = reqtSmvrFps;
    return ret;
}

MBOOL BasicProcessor::checkBurst()
{
    TRACE_S_FUNC_ENTER(mLog);
    MBOOL ret = MFALSE;
    if( mBurstQueue.size() )
    {
        processP2(mBurstQueue);
        mBurstQueue.clear();
        ret = MTRUE;
    }
    TRACE_S_FUNC_EXIT(mLog);
    return ret;
}

} // namespace P2

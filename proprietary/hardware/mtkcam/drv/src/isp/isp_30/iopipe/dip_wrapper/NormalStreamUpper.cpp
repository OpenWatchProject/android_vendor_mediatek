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

#define LOG_TAG "NormalStreamUpper"

#include <cstdint>
#include <cutils/properties.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/Trace.h>
#include <mtkcam/drv/iopipe/Event/IoPipeEvent.h>
#include "NormalStreamUpper.h"


using namespace android;
using namespace NSCam::NSIoPipe;


namespace NSCam {
namespace NSIoPipe {
namespace NSPostProc {
namespace Wrapper {

class LogLife
{
public:
    LogLife(const char *name, unsigned int debugFlag) : mName(name), mDebugFlag(debugFlag) {
        if (mDebugFlag & NormalStreamUpper::DEBUG_FUNC_LIFE)
            CAM_LOGD("[%s] +", mName);
    }
    ~LogLife() {
        if (mDebugFlag & NormalStreamUpper::DEBUG_FUNC_LIFE)
            CAM_LOGD("[%s] -", mName);
    }
private:
    const char *mName;
    unsigned int mDebugFlag;
};

#define LOG_FUNCTION_LIFE(debugFlag) LogLife _log_(__func__, debugFlag)


Mutex IpRawController::sInstanceMutex;
wp<IpRawController> IpRawController::sInstance = NULL;


sp<IpRawController> IpRawController::getInstance()
{
    Mutex::Autolock lock(sInstanceMutex);

    sp<IpRawController> inst = sInstance.promote();

    if (inst == NULL) {
        inst = new IpRawController;
        inst->init();
        sInstance = inst;
    }

    return inst;
}


IpRawController::~IpRawController()
{
    CAM_TRACE_CALL();

    {
        Mutex::Autolock lock(mStateMutex);
        mAsyncThread->requestExit();
        mCond.broadcast();
    }

    mAsyncThread->join();
    mAsyncThread = NULL;

    {
        Mutex::Autolock lock(mStateMutex);

        if (mNumOfP1Acquired > 0) {
            CAM_LOGE("Did not broadcast P1 released: acquired = %d", mNumOfP1Acquired);
        }

        // It's normal that IpRawController was released but
        // the asyncRelease was not executed on time yet
        if (mP1IsOccupied)
            sendReleasedEvent();
    }
}


void IpRawController::init()
{
    mAsyncThread = new AsyncThread(*this);
    mAsyncThread->run("IpRawController");
}


bool IpRawController::acquireP1()
{
    CAM_TRACE_CALL();

    Mutex::Autolock lock(mStateMutex);

    if (mNumOfP1Acquired == 0) {
        // The AsyncThread may not be invoked to release yet
        // We shall not acquire again
        // mNumOfP1Acquired++ will avoid the released even if the AsyncThread
        // wake up afterwards
        if (!mP1IsOccupied) {
            CAM_LOGD("acquireP1: sendSyncEvent(IpRawP1AcquiringEvent): +");
            IpRawP1AcquiringEvent acquiringEvent;
            IoPipeEventSystem &eventSystem = IoPipeEventSystem::getGlobal();
            eventSystem.sendSyncEvent(acquiringEvent);
            CAM_LOGD("acquireP1: sendSyncEvent(IpRawP1AcquiringEvent): -");

            if (acquiringEvent.getResult() != IoPipeEvent::RESULT_OK) {
                CAM_LOGE("acquireP1 failed: result = %d", acquiringEvent.getResult());
                return false;
            }

            // Will be set back to false only if P1Released event is sent
            mP1IsOccupied = true;
        }

        mNumOfP1Acquired++;
    } else {
        mNumOfP1Acquired++;
    }

    CAM_LOGD("IpRawController::acquireP1: mNumOfP1Acquired = %d -> %d %s",
        mNumOfP1Acquired - 1, mNumOfP1Acquired,
        (mNumOfP1Acquired == 1 ? "ACQUIRED" : ""));

    return true;
}


void IpRawController::asyncReleaseP1()
{
    Mutex::Autolock lock(mStateMutex);

    if (mNumOfP1Acquired > 0) {
        mNumOfP1Acquired--;
        if (mNumOfP1Acquired == 0)
            mCond.broadcast();
    } else {
        CAM_LOGE("asyncReleaseP1: double release");
    }

    CAM_LOGD("IpRawController::asyncReleaseP1: mNumOfP1Acquired = %d -> %d %s",
        mNumOfP1Acquired + 1, mNumOfP1Acquired,
        (mNumOfP1Acquired == 0 ? "RELEASED" : ""));
}


void IpRawController::sendReleasedEvent()
{
    CAM_TRACE_CALL();

    CAM_LOGD("sendSyncEvent(IpRawP1ReleasedEvent): +");
    IpRawP1ReleasedEvent releasedEvent;
    IoPipeEventSystem &eventSystem = IoPipeEventSystem::getGlobal();
    eventSystem.sendSyncEvent(releasedEvent);
    CAM_LOGD("sendSyncEvent(IpRawP1ReleasedEvent): -");
}


bool IpRawController::AsyncThread::threadLoop()
{
    Mutex::Autolock lock(mController.mStateMutex);

    while (!exitPending()) {
        if (mController.mNumOfP1Acquired == 0 && mController.mP1IsOccupied) {
            mController.sendReleasedEvent();
            mController.mP1IsOccupied = false;
        } else {
            mController.mCond.wait(mController.mStateMutex);
        }
    }

    return true;
}


inline NormalStreamUpper::UserData::UserData(
    MVOID *myCookie, const QParams &qParams)
{
    mMyCookie = myCookie;
    mpfnCallback = qParams.mpfnCallback;
    mpfnEnQFailCallback = qParams.mpfnEnQFailCallback;
    mpfnEnQBlockCallback = qParams.mpfnEnQBlockCallback;
    mpCookie = qParams.mpCookie;
}


NormalStreamUpper::NormalStreamUpper(MUINT32 openedSensorIndex, INormalStream *normalStream) :
    mDebugFlag(0),
    mOpenedSensorIndex(openedSensorIndex),
    mNormalStream(normalStream)
{
    (void)mOpenedSensorIndex; // Unused, avoid warning

    for (int i = 0; i < NUM_OF_COOKIES; i++) {
        mCookiePool[i] = this;
        mFreeCookies.push_back(static_cast<MVOID*>(&mCookiePool[i]));
    }

    mDebugFlag = property_get_int32("vendor.debug.normalstream.upper", 0);
}


NormalStreamUpper::~NormalStreamUpper()
{
    mNormalStream->destroyInstance();
    memset(mCookiePool, 0, sizeof(mCookiePool));
}


MVOID NormalStreamUpper::destroyInstance()
{
    delete this;
}


inline android::sp<IpRawController> NormalStreamUpper::getIpRawController()
{
    // Cache the sp<> here to avoid frequently create/release
    if (mpIpRawController == NULL)
        mpIpRawController = IpRawController::getInstance();
    return mpIpRawController;
}


inline bool NormalStreamUpper::isMyCookieValid(MVOID *cookie)
{
    if (cookie == NULL)
        return false;

    intptr_t cookieAddr = reinterpret_cast<intptr_t>(cookie);
    intptr_t myCookieBegin = reinterpret_cast<intptr_t>(&mCookiePool[0]);
    intptr_t myCookieLast = reinterpret_cast<intptr_t>(&mCookiePool[NUM_OF_COOKIES - 1]);

    // Is the address inside this instance?
    if (!(myCookieBegin <= cookieAddr && cookieAddr <= myCookieLast))
        return false;

    // Its de-reference must equals to this. Here is safe since we have checked the address
    // There may be still ABA problem. But this is only a mistake prevention,
    // we ignore the problem since we don't have simple way
    return (*static_cast<NormalStreamUpper**>(cookie) == this);
}


inline MVOID *NormalStreamUpper::allocateCookie()
{
    if (mFreeCookies.size() == 0)
        return NULL;

    MVOID *cookie = mFreeCookies.front();
    mFreeCookies.pop_front();

    return cookie;
}


inline void NormalStreamUpper::freeCookie(MVOID *cookie)
{
    mFreeCookies.push_back(cookie);
}


MBOOL NormalStreamUpper::enque(QParams const& rParams)
{
    CAM_TRACE_CALL();
    LOG_FUNCTION_LIFE(mDebugFlag);

    if (!isIpRawRequest(rParams)) {
        // Not IP-RAW, bypass to lower wrapper directly
        return mNormalStream->enque(rParams);
    }

    // Only IP-RAW will be translated and pushed into mUserData
    MVOID *myCookie = NULL;
    {
        Mutex::Autolock lock(mMutex);

        myCookie = allocateCookie();
        if (myCookie == NULL) {
            CAM_LOGE("Too many IP-RAW requests in single NormalStream instance");
            return MFALSE;
        }

        if (!getIpRawController()->acquireP1()) {
            freeCookie(myCookie);
            return MFALSE;
        }

        mUserData.emplace_back(myCookie, rParams);
    }

    QParams wrpQParams = rParams;
    wrpQParams.mpCookie = myCookie;
    if (rParams.mpfnCallback != NULL) {
        wrpQParams.mpfnCallback = &enQCallback;
        wrpQParams.mpfnEnQFailCallback = &enQFailCallback;
        wrpQParams.mpfnEnQBlockCallback = &enQBlockCallback;
    }
    markIpRawRequest(wrpQParams);

    MBOOL result = mNormalStream->enque(wrpQParams);

    if (!result) {
        Mutex::Autolock lock(mMutex);
        getIpRawController()->asyncReleaseP1();
        UserData userData;
        popUserData(myCookie, userData);
        return MFALSE;
    }

    return MTRUE;
}


MBOOL NormalStreamUpper::deque(QParams& rParams, MINT64 i8TimeoutNs)
{
    CAM_TRACE_CALL();
    LOG_FUNCTION_LIFE(mDebugFlag);

    if (!mNormalStream->deque(rParams, i8TimeoutNs)) {
        CAM_LOGE("Deque failed, timeout = %" PRId64 " ns", i8TimeoutNs);
        return MFALSE;
    }

    if (!isMyCookieValid(rParams.mpCookie)) {
        // User's cookie
        return MTRUE;
    }

    Mutex::Autolock lock(mMutex);

    UserData userData;
    if (!popUserData(rParams.mpCookie, userData))  {
        // Coincidence, but not mine
        return MTRUE;
    }

    getIpRawController()->asyncReleaseP1();

    rParams.mpfnCallback = userData.mpfnCallback;
    rParams.mpfnEnQFailCallback = userData.mpfnEnQFailCallback;
    rParams.mpfnEnQBlockCallback = userData.mpfnEnQBlockCallback;
    rParams.mpCookie = userData.mpCookie;

    return MTRUE;
}


bool NormalStreamUpper::isIpRawRequest(QParams const& rParams)
{
    for (auto &frameParam : rParams.mvFrameParams)
        for (auto &input : frameParam.mvIn) {
            MINT64 rawType;
            if (input.mBuffer->getImgDesc(eIMAGE_DESC_ID_RAW_TYPE, rawType) &&
                rawType == eIMAGE_DESC_RAW_TYPE_PURE)
            {
                return true;
            }
        }

    return false;
}


void NormalStreamUpper::markIpRawRequest(QParams& wrpQParams)
{
    for (auto &frameParam : wrpQParams.mvFrameParams)
        for (auto &input : frameParam.mvIn) {
            MINT64 rawType;
            if (input.mBuffer->getImgDesc(eIMAGE_DESC_ID_RAW_TYPE, rawType) &&
                rawType == eIMAGE_DESC_RAW_TYPE_PURE)
            {
                frameParam.mStreamTag = ENormalStreamTag_IP_Tpipe;
                break;
            }
        }
}


template <typename _GetCallback>
MVOID NormalStreamUpper::transCallback(QParams& rParams, _GetCallback &&getCallback)
{
    NormalStreamUpper *inst = *static_cast<NormalStreamUpper**>(rParams.mpCookie);

    if (!inst->isMyCookieValid(rParams.mpCookie)) {
        CAM_LOGE("transCallback: Invalid cookie received: %p", rParams.mpCookie);
        return;
    }

    LOG_FUNCTION_LIFE(inst->mDebugFlag);

    UserData userData;
    unsigned int debugFlag = 0;
    sp<IpRawController> ipRawController;
    {
        Mutex::Autolock lock(inst->mMutex);

        // Cookie was recycled in popUserData()
        if (!inst->popUserData(rParams.mpCookie, userData)) {
            CAM_LOGE("transCallback: Unable to find UserData: %p", rParams.mpCookie);
            return;
        }

        debugFlag = inst->mDebugFlag;
        ipRawController = inst->getIpRawController();
    }

    struct {
        QParams::PFN_CALLBACK_T mpfnCallback, mpfnEnQFailCallback, mpfnEnQBlockCallback;
        MVOID *mpCookie;
    } ori;

    // Store the fields may be altered
    ori.mpfnCallback = rParams.mpfnCallback;
    ori.mpfnEnQFailCallback = rParams.mpfnEnQFailCallback;
    ori.mpfnEnQBlockCallback = rParams.mpfnEnQBlockCallback;
    ori.mpCookie = rParams.mpCookie;

    rParams.mpfnCallback = userData.mpfnCallback;
    rParams.mpfnEnQFailCallback = userData.mpfnEnQFailCallback;
    rParams.mpfnEnQBlockCallback = userData.mpfnEnQBlockCallback;
    rParams.mpCookie = userData.mpCookie;

    QParams::PFN_CALLBACK_T pCallback = getCallback(userData);
    {
        LogLife _cblife_("Callback", debugFlag);
        (*pCallback)(rParams);
    }

    // Restore the fields, maybe unnecessary but safer
    rParams.mpfnCallback = ori.mpfnCallback;
    rParams.mpfnEnQFailCallback = ori.mpfnEnQFailCallback;
    rParams.mpfnEnQBlockCallback = ori.mpfnEnQBlockCallback;
    rParams.mpCookie = ori.mpCookie;

    if (ipRawController != NULL)
        ipRawController->asyncReleaseP1();
}


MVOID NormalStreamUpper::enQCallback(QParams& rParams)
{
    CAM_TRACE_CALL();

    transCallback(rParams,
        [] (const UserData &userData) {
            return userData.mpfnCallback;
        }
    );
}


MVOID NormalStreamUpper::enQFailCallback(QParams& rParams)
{
    CAM_TRACE_CALL();

    transCallback(rParams,
        [] (const UserData &userData) {
            return userData.mpfnEnQFailCallback;
        }
    );
}


MVOID NormalStreamUpper::enQBlockCallback(QParams& rParams)
{
    CAM_TRACE_CALL();

    transCallback(rParams,
        [] (const UserData &userData) {
            return userData.mpfnEnQBlockCallback;
        }
    );
}


bool NormalStreamUpper::popUserData(MVOID *myCookie, UserData &userData)
{
    // mMutex MUST be already locked

    auto dataPos = std::find_if(mUserData.begin(), mUserData.end(),
        [myCookie] (const UserData &data) {
            return (data.mMyCookie == myCookie);
        }
    );

    if (dataPos == mUserData.end())
        return false;

    userData = *dataPos;
    mUserData.erase(dataPos);
    if (myCookie != NULL) {
        freeCookie(userData.mMyCookie);
        userData.mMyCookie = NULL;
    }

    return true;
}


MBOOL NormalStreamUpper::setJpegParam(EJpgCmd jpgCmd,int arg1,int arg2)
{
    return mNormalStream->setJpegParam(jpgCmd, arg1, arg2);
}

MBOOL NormalStreamUpper::setFps(MINT32 fps)
{
    return mNormalStream->setFps(fps);
}

MBOOL NormalStreamUpper::sendCommand(ESDCmd cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3)
{
    return mNormalStream->sendCommand(cmd, arg1, arg2, arg3);
}

MBOOL NormalStreamUpper::init(char const* szCallerName, MUINT32 secTag)
{
    return mNormalStream->init(szCallerName, secTag);
}

MBOOL NormalStreamUpper::init(char const* szCallerName, NSCam::NSIoPipe::EStreamPipeID mPipeID, MUINT32 secTag)
{
    return mNormalStream->init(szCallerName, mPipeID, secTag);
}

MBOOL NormalStreamUpper::uninit(char const* szCallerName)
{
    return mNormalStream->uninit(szCallerName);
}

MBOOL NormalStreamUpper::uninit(char const* szCallerName, NSCam::NSIoPipe::EStreamPipeID mPipeID)
{
    return mNormalStream->uninit(szCallerName, mPipeID);
}

MERROR NormalStreamUpper::getLastErrorCode() const
{
    return mNormalStream->getLastErrorCode();
}

MBOOL NormalStreamUpper::startVideoRecord(MINT32 wd,MINT32 ht, MINT32 fps)
{
    return mNormalStream->startVideoRecord(wd, ht, fps);
}

MBOOL NormalStreamUpper::stopVideoRecord()
{
    return mNormalStream->stopVideoRecord();
}


}
}
}
}


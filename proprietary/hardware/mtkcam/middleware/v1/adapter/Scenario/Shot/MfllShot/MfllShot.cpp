/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2015. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#define DEBUG_LOG_TAG "MfllShot"
#define LOG_TAG "MfllShot"

#include "MfllShot.h"

#include <mtkcam/def/Modes.h>

#include <mtkcam/middleware/v1/camshot/_callbacks.h>

#include <mtkcam/middleware/v1/camshot/CamShotUtils.h>

#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/aaa/IIspMgr.h>

#include <mtkcam/utils/metadata/IMetadataConverter.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>

#include <mtkcam/utils/hw/HwInfoHelper.h>
#include <mtkcam/utils/hw/CamManager.h>

#include <mtkcam/middleware/v1/LegacyPipeline/ILegacyPipeline.h>
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
#include <mtkcam/middleware/v1/LegacyPipeline/IResourceContainer.h>
#include <mtkcam/middleware/v1/LegacyPipeline/LegacyPipelineBuilder.h>
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/StreamBufferProviderFactory.h>
#include <mtkcam/middleware/v1/LegacyPipeline/mfc/buffer/Selector.h> // MfcSelector

#include <mtkcam/pipeline/hwnode/MfllNode.h>

#include <sys/resource.h>

#include <utils/Thread.h>
#include <cutils/properties.h>

#include <mtkcam/feature/mfnr/MfllLog.h>

#include <mutex>
#include <condition_variable>


/**
 *  Define this flag to true to enable apply the current flare offset
 */
#define APPLY_CURRENT_FLARE_OFFSET 1

/**
 *  Define PIPELINE_MFNR to use Dual Phase MFNR in ZSD mode, default is off due
 *  to unexpected system behaviors
 */
#define PIPELINE_MFNR

/**
 *  To pre-allocate RAW buffers for ZSD buffer pool
 *
 *  #define PREALLOCATE_RAW_BUFFERS
 */

/**
 *  Preview frozen time
 *
 *  ZSD+MFNR costs lots traffic of pass 2 therefore to pause preview for a while
 *  is a good idea to reduce pass 2 traffic. The delay time will be calculated by
 *
 *  capture_frame_number * UNIT_PAUSE_PREVIEW_MSEC
 *
 *  in milliseconds, defines to 0 to disable
 */
#define UNIT_PAUSE_PREVIEW_MSEC     80

/**
 *  Define POSTVIEW_SUPPORT to return postview YUV
 *  0: do not enable postview
 *  1: enable postview
 */
#define POSTVIEW_SUPPORT        1

using namespace android;
using namespace NSCam::Utils;
using namespace NSCam::v1;
using namespace NSCam::v1::NSLegacyPipeline;
using namespace NSCamHW;
using namespace NSShot;
using namespace NSCamShot;
using namespace NS3Av3;
using namespace mfll;
using namespace NSCam;

// ---------------------------------------------------------------------------

template <typename T>
class WorkerThread final : public Thread
{
public:
    WorkerThread(const char* name, const uint32_t shotMode, const sp<T>& shot);

    status_t queryResult() const { Mutex::Autolock _l(mLock); return mStatus; }

private:
    mutable Mutex mLock;

    char mThreadName[256];

    sp<T> mShot;

    const uint32_t mShotMode;

    // used to report the execution status
    status_t mStatus;

    // good place to do one-time initializations
    status_t readyToRun();

    bool threadLoop();
};

// ---------------------------------------------------------------------------

extern
sp<IShot> createInstance_MfllShot(
        char const * const pszShotName,
        uint32_t const u4ShotMode,
        int32_t const  i4OpenId)
{
    sp<IShot> shot;
    sp<MfllShot> myShot;

    // new implementator
    myShot = new MfllShot(pszShotName, u4ShotMode, i4OpenId);
    if (myShot == NULL)
    {
        mfllLogE("create MfllShot failed");
        goto lbExit;
    }

    // new interface
    shot = new IShot(myShot);
    if (shot == NULL)
    {
        mfllLogE("create IShot failed");
        goto lbExit;
    }

lbExit:
    // free all resources if instantiation failed
    if ((shot == 0) && (myShot != NULL))
    {
        myShot.clear();
    }

    return shot;
}

// ---------------------------------------------------------------------------
// Because the flare offset won't be applied if using manual mode AE, we have to
// tell AE manager to apply the current flare offset for the following manual AE
// controls.
// To invoke this function to create a std::shared_ptr contains a IHal3A instance,
// after manual AE done, clear the std::shared_ptr<IHal3A> to do the fact that
// tell AE manager restore to the default configuration
//  @param openId               opended sensor ID
//  @return                     return std::shared_ptr<IHal3A> instance
std::shared_ptr<IHal3A> askFlareOffset(int openId)
{
#if APPLY_CURRENT_FLARE_OFFSET
    /* create IHal3A instance to tell 3A manager we need to apply flare offset */
    std::shared_ptr<IHal3A> hal3a_ask_manual_flare_offset(
            // create instance
            MAKE_Hal3A(openId, LOG_TAG),
            // manual destructor
            [](auto *p) -> void { // destructor
                if (p) {
                    mfllLogI("ask flare offset -> no");
                    p->send3ACtrl(E3ACtrl_EnableFlareInManualCtrl, 0, 0);
                    p->destroyInstance(LOG_TAG);
                }
            }
        );

    if (hal3a_ask_manual_flare_offset.get()) {
        mfllLogI("ask flare offset -> yes");
        hal3a_ask_manual_flare_offset->send3ACtrl(E3ACtrl_EnableFlareInManualCtrl, 1, 0);
    }

    return hal3a_ask_manual_flare_offset;
#else
    return NULL;
#endif
}

// ---------------------------------------------------------------------------

static NSCam::MtQueue< sp<MfllShot> > gMfllShotQueue(
        []()->sp<MfllShot>{
            return NULL;
        });

// ---------------------------------------------------------------------------
static sp<CallbackBufferPool> prepareCallbackBufferPool(
        sp<BufferCallbackHandler>& callbackHandler,
        const sp<IImageStreamInfo>& streamInfo,
        bool allocateBuffer)
{
    sp<CallbackBufferPool> cbBufferPool = new CallbackBufferPool(streamInfo);

    if (allocateBuffer)
    {
        cbBufferPool->allocateBuffer(
                streamInfo->getStreamName(),
                streamInfo->getMaxBufNum(),
                streamInfo->getMinInitBufNum());
    }
    callbackHandler->setBufferPool(cbBufferPool);

    return cbBufferPool;
}

static void onFakeJpegCallback(const sp<IShotCallback>& callback)
{
    if (callback == NULL)
    {
        mfllLogE("shot callback is null");
        return;
    }

    mfllLogW("send dummy jpeg callback");

    const uint32_t jpegSize = 512;
    const uint8_t  jpegBuffer[jpegSize] {};
    const uint32_t exifHeaderSize = 512;
    const uint8_t  exifHeaderBuffer[exifHeaderSize] {};

    callback->onCB_CompressedImage(
            0, jpegSize, jpegBuffer, exifHeaderSize, exifHeaderBuffer, 0, true);
}

auto dumpImageStreamInfo = [](const sp<IImageStreamInfo>& info)
{
    mfllLogD("streamInfo(%#" PRIx64 ") name(%s) format(%#08x) size(%dx%d) min/max(%zu/%zu)",
            info->getStreamId(), info->getStreamName(),
            info->getImgFormat(), info->getImgSize().w, info->getImgSize().h,
            info->getMinInitBufNum(), info->getMaxBufNum());
};

auto dumpMetadata = [](const IMetadata& metadata, const char* msg = "")
{
    sp<IMetadataConverter> metadataConverter =
        IMetadataConverter::createInstance(
                IDefaultMetadataTagSet::singleton()->getTagSet());

    mfllLogD("dump %s metadata(%p)", msg, &metadata);
    // list all settings
    if (metadataConverter.get())
        metadataConverter->dumpAll(metadata);
};

/**
 *  update manual 3A info to app metadata
 *  @param pAppMeta             Pointer of app metadata
 *  @param iso                  Iso value to specify
 *  @param exp                  Exposure time to specify, unit is ms
 */
static void updateManual3A(IMetadata *pAppMeta, unsigned int iso, unsigned int exp)
{
    IMetadata::setEntry<MUINT8>(
            pAppMeta,
            MTK_CONTROL_AE_MODE,
            MTK_CONTROL_AE_MODE_OFF);
    IMetadata::setEntry<MINT32>(
            pAppMeta,
            MTK_SENSOR_SENSITIVITY,
            iso);
    IMetadata::setEntry<MINT64>(
            pAppMeta,
            MTK_SENSOR_EXPOSURE_TIME,
            exp * 1000 // unit is ns
            );
}

static void updateIsLockAwb(IMetadata *pAppMeta, MBOOL bLock)
{
    if (pAppMeta == NULL) {
        mfllLogE("%s: pAppMeta is NULL, ignored", __FUNCTION__);
        return;
    }

    IMetadata::setEntry<MUINT8>(
            pAppMeta,
            MTK_CONTROL_AWB_LOCK,
            bLock);
}
// ---------------------------------------------------------------------------

template <typename T>
WorkerThread<T>::WorkerThread(
        const char* name, const uint32_t shotMode, const sp<T>& shot)
    : mShot(shot)
    , mShotMode(shotMode)
    , mStatus(NO_ERROR)
{
    snprintf(mThreadName, sizeof(mThreadName), "%s", name);
}

template <typename T>
status_t WorkerThread<T>::readyToRun()
{
    //  set thread policy & priority
    //  NOTE:
    //  Even if pthread_create() with SCHED_OTHER policy, a newly-created thread
    //  may inherit the non-SCHED_OTHER policy & priority of the thread creator.
    //  And thus, we must set the expected policy & priority after a thread creation.

    // retrieve the parameters of the calling process
    struct sched_param schedParam;
    sched_getparam(0, &schedParam);

    if (setpriority(PRIO_PROCESS, 0, ANDROID_PRIORITY_NORMAL))
    {
        mfllLogW("set priority failed(%s)", strerror(errno));
    }

    schedParam.sched_priority = 0;
    sched_setscheduler(0, SCHED_OTHER, &schedParam);

    return OK;
}

template <typename T>
bool WorkerThread<T>::threadLoop()
{
    mfllAutoTraceFunc();

    Mutex::Autolock _l(mLock);

    if (!mShot.get())
    {
        mfllLogE("shot is not set");
        mStatus = NO_INIT;
        return false;
    }

    if (eShotMode_ZsdMfllShot == mShotMode)
    {
        mStatus = mShot->zsdCapture() ? NO_ERROR : UNKNOWN_ERROR;
    }
    else
    {
        mStatus = mShot->normalCapture() ? NO_ERROR : UNKNOWN_ERROR;
    }

    mShot->invokeJpegCallback(NULL);

    // one-shot execution
    return false;
}

// ---------------------------------------------------------------------------

static Semaphore sSemDualPhse(1);
static void wait_capture_done()
{
    mfllAutoLogFunc();
    sSemDualPhse.wait();
}
static void announce_capture_done()
{
    mfllAutoLogFunc();
    sSemDualPhse.post();
}

// ---------------------------------------------------------------------------

MfllShot::MfllShot(
        const char *pszShotName,
        uint32_t const u4ShotMode,
        int32_t const i4OpenId)
    : ImpShot(pszShotName, u4ShotMode, i4OpenId)
    , mSequence(0)
    , mDataMsgSet(0)
    , mFrameCapture(0)
    , mFrameDoneCnt(0)
    , mIsZsdAisMode(false)
    , mbOpenLCS(MFALSE)
{
    m_atomicJpegDone.store(0);
    m_atomicCaptureDoneTimes.store(0);
    mSelectorMetadataPack.bInited = false;

    // debug property setting
    {
        char value[PROPERTY_VALUE_MAX];

        // log level
        property_get("vendor.mfll.log", value, "0");
        mLogLevel = atoi(value);
        if (mLogLevel > 0)
            mfllLogD("log level is set to %d", mLogLevel);
    }
}

MfllShot::~MfllShot()
{
    if (mpPipeline.get())
        mpPipeline->flush();

    /* for safety, announceCaptureDone here ... */
    announceCaptureDone();
    mfllLogD("MfllShot force unlock");
}

// metadata event handler
void MfllShot::onResultReceived(
        MUINT32    const requestNo,
        StreamId_T const streamId,
        MBOOL      const errorResult,
        IMetadata  const result)
{
    mfllAutoLogFunc();
    mfllAutoTraceFunc();

    mfllLogD("metadata request(%u) streamID(%#" PRIx64 ")", requestNo, streamId);

    if (errorResult)
    {
        mfllLogE("the result metadata may be wrong...");
    }

    switch (streamId)
    {
        case eSTREAMID_META_HAL_DYNAMIC_P1:
            mfllLogD("send shutter callback");
            mpShotCallback->onCB_Shutter(true, 0, mu4ShotMode);
        break;

        /* for DNG callback */
        case eSTREAMID_META_APP_FULL:
            /* return DNG metadata */
            if (eShotMode_ZsdMfllShot == mu4ShotMode) {
                /**
                 *  for ZSD mode, we need update app metadata from selector because
                 *  the metadata we got here is not complete
                 */
                IMetadata m;
                auto r = getSelectorAppMetadataLocked(&m); // get selector metadata in thread-safe method
                switch (r) {
                case SAM_STATUS_OK:
                    m = (m + (*const_cast<IMetadata*>(&result)));
                    mpShotCallback->onCB_DNGMetaData(reinterpret_cast<MUINTPTR>(&m));
                    break;
                case SAM_STATUS_NOT_INITED:
                    mfllLogE("get selector app metadata failed");
                    break;
                case SAM_STATUS_NO_NEED:
                default:
                    break;
                } // switch
            }
            else {
                mpShotCallback->onCB_DNGMetaData(reinterpret_cast<MUINTPTR>(&result));
            }
        break;

        case eSTREAMID_META_HAL_FULL:
        case eSTREAMID_META_HAL_DYNAMIC_MFLL:
        break;

        default:
            mfllLogW("unsupported streamID(%#" PRIx64 ")", streamId);
    }
}

// image event handler
MERROR MfllShot::onResultReceived(
        MUINT32    const requestNo,
        StreamId_T const streamId,
        MBOOL      const errorBuffer,
        android::sp<IImageBuffer>& pBuffer)
{
    mfllAutoTraceFunc();

    mfllLogD("image request(%d) streamID(%#" PRIx64 ")", requestNo, streamId);

    if (errorBuffer)
    {
        mfllLogE("the content of buffer may not correct...");
    }

    switch (streamId)
    {
        case eSTREAMID_IMAGE_PIPE_RAW_OPAQUE:
            mFrameDoneCnt++;

            /**
             *  Middleware v1.4 limitation: all request must be returned in ordered.
             *
             *  The first RAW is captured, when we got the first RAW callback,
             *  which means we can release all RAW buffers.
             */
            if (mFrameDoneCnt == 1)
            {
#if UNIT_PAUSE_PREVIEW_MSEC
                /**
                 *  In ZSD mode, to resume preview before p2done callback if necessary
                 */
                if (mPreviewResumeOps.resumePreviewOnce() == 0) {
                    mfllLogD("resume preview by p2done cb");
                }
#endif
                /* handle RAW if necessary */
                if (isDataMsgEnabled(EIShot_DATA_MSG_RAW))
                {
                    if (pBuffer.get() != NULL && errorBuffer != MTRUE) {
                        pBuffer->lockBuf(LOG_TAG, eBUFFER_USAGE_SW_READ_MASK); // lock for use
                        mpShotCallback->onCB_Raw16Image(reinterpret_cast<IImageBuffer const*>(pBuffer.get()));
                        pBuffer->unlockBuf(LOG_TAG); // unlock
                    }
                    else {
                        mfllLogE("to dump DNG but RAW buffer is NULL");
                    }
                }
            }

            // return buffer to pool
            returnSelectedBuffer(pBuffer);

            if (mFrameDoneCnt == 1)
            {
#ifdef PIPELINE_MFNR
                if (mu4ShotMode == eShotMode_ZsdMfllShot)
                    mpShotCallback->onCB_P2done();
#endif
                /* this round is finished */
                announceCaptureDone();
            }
            break;

        case eSTREAMID_IMAGE_MFLL:
            // dummy raw callback
            mpShotCallback->onCB_RawImage(0, 0, NULL);
            invokeJpegCallback(errorBuffer ? NULL : pBuffer);
            break;

        case eSTREAMID_IMAGE_PIPE_YUV_00:
            // we send postview YUV back if buffer exist and without error
            if (pBuffer.get() && (errorBuffer == MFALSE))
            {
                pBuffer->lockBuf(LOG_TAG, eBUFFER_USAGE_SW_READ_MASK);
                int dumpPostview = property_get_int32("vendor.mediatek.mfll.dump.postview", -1);
                if (dumpPostview > 0)
                {
                    char filename[64];
                    MSize const &imgSize = pBuffer->getImgSize();
                    sprintf(filename, "/sdcard/MFLL.postview.%dx%d.yv12", imgSize.w, imgSize.h);
                    pBuffer->saveToFile(filename);
                }
                mpShotCallback->onCB_PostviewClient(0, pBuffer.get());
                pBuffer->unlockBuf(LOG_TAG);
            }
            else
            {
                mfllLogE("skip postview callback, because NULL buffer!!");
            }
            break;
        default:
            mfllLogW("unsupported streamID(%#" PRIx64 ")", streamId);
    }

    return OK;
}

String8 MfllShot::getUserName()
{
    return String8(MFLL_LOG_KEYWORD);
}

bool MfllShot::sendCommand(
        uint32_t const cmd,
        MUINTPTR const arg1,
        uint32_t const arg2,
        uint32_t const /*arg3*/)
{
    mfllAutoLogFunc();
    mfllAutoTraceFunc();

    bool ret = true;

    switch (cmd)
    {
        // eCmd_reset is to reset this class. After captures and then reset,
        // performing a new capture should work well, no matter whether previous
        // captures failed or not.
        case eCmd_reset:
            ret = onCmd_reset();
            break;
        // eCmd_capture is to perform capture.
        case eCmd_capture:
            ret = onCmd_capture();
            break;
        // eCmd_cancel is to perform cancel capture.
        case eCmd_cancel:
            onCmd_cancel();
            break;
        default:
            ret = ImpShot::sendCommand(cmd, arg1, arg2);
    }

    return ret;
}

bool MfllShot::onCmd_reset()
{
    return true;
}

bool MfllShot::onCmd_capture()
{
    mfllAutoLogFunc();

    mfllLogD("[%d] %s", mSequence, __FUNCTION__);

    mfllAutoTraceFunc();

    bool bCancel = mStateMgr.doWork<bool>(
            [this](ShotState &state, void* /* arg */)->bool {
                if (state == SHOTSTATE_CANCEL) {
                    mSemCaptureDone.post();
                    return true;
                }
                else {
                    state = SHOTSTATE_CAPTURE;
                    gMfllShotQueue.enque(this);
                    return false;
                }
            },
            NULL);

    if (bCancel) {
        invokeJpegCallback(NULL);
        return true;
    }

    // dump metadata
    if (CC_UNLIKELY(mLogLevel > 0))
        dumpMetadata(mShotParam.mAppSetting, "app");

    if (POSTVIEW_SUPPORT && (mShotParam.miPostviewClientFormat != eImgFmt_UNKNOWN))
        enableDataMsg(NSCamShot::ECamShot_DATA_MSG_POSTVIEW);

    // update post nr settings
    updatePostNRSetting(MTK_NR_MODE_AUTO, true, &mShotParam.mHalSetting);

    // update LCS setting
    mbOpenLCS =
        (getLcsStatus() == LCSSTATUS_DISABLED) ? MFALSE : MTRUE;

    // need to wait for previous Mfll input frames are received
    // before start this round
    {
        mfllLogD("waiting for capture done...");
        waitCaptureDone();
        mfllLogD("wait capture done");
    }

    if (eShotMode_MfllShot == mu4ShotMode)
    {
        return normalCapture();
    }

    // otherwise, asynchronous capture and return as soon as possible
    String8 name;
    name.appendFormat("[%d] worker", mSequence);
    sp<WorkerThread<MfllShot>> workerThread =
        new WorkerThread<MfllShot>(name.string(), mu4ShotMode, this);
    workerThread->run(name.string());

    return true;
}

void MfllShot::onCmd_cancel()
{
    mfllAutoLogFunc();

    /**
     *  To tell all the MfllShot instances in queue, excludes self, to cancel
     *  using async calls and sync all async calls while existing this scope.
     */
    std::deque< std::future<void> > future_teller;

    gMfllShotQueue.getMutex().lock();
    {
        std::deque< sp<MfllShot> > shots = std::move(gMfllShotQueue.getQueue());
        for (size_t i = 0; i < shots.size(); i++) {
            sp<MfllShot> s = shots[i];
            if (s.get() != this) {
                auto f = std::async(std::launch::async, [s]()->void{
                    s->sendCommand(eCmd_cancel, 0, 0, 0);
                });
                future_teller.push_back(std::move(f));
            }
        }
    }
    gMfllShotQueue.getMutex().unlock();

    /* represents the primitive status */
    ShotState primitiveState = SHOTSTATE_INIT;

    /* checking current state and update state to cancel */
    bool isCapturing = mStateMgr.doWork<bool>(
            [&primitiveState](ShotState &state, void* /* arg */){
                primitiveState = state;
                state = SHOTSTATE_CANCEL;
                /* wait capture done semaphore signaled */
                return (primitiveState == SHOTSTATE_CAPTURE);
            });

    /* wait catpure done */
    if (isCapturing) {
        mfllLogD("%s: wait capture semaphore signaled", __FUNCTION__);
        mSemCaptureDone.wait();
    }

    /* if the primitive state is SHOTSTATE_INIT, do not invoke fake JPEG done */
    if (primitiveState != SHOTSTATE_INIT)
        invokeJpegCallback(NULL); // fake JPEG cb
}

bool MfllShot::normalCapture()
{
    /* notify as capture done while exiting */
    ScopeWorker ___scope_worker([this](void* /* arg */){
        mStateMgr.updateState(SHOTSTATE_DONE);
        gMfllShotQueue.deque();
        mSemCaptureDone.post();
    });

    mfllAutoLogFunc();
    mfllAutoTraceFunc();

    bool ret = true;

    // before operation, check state first, if it's cancel, no need to capture
    // anymore
    if (mStateMgr.getState() == SHOTSTATE_CANCEL) {
        return false;
    }

    // 0. Update capture frame parameter set first due to need of the number
    //    of frame to capture
    Vector<CaptureParam_T> vCaptureParams;
    // get Mfll capture parameters
    MfllNode::getCaptureParamSet(getOpenId(), vCaptureParams);
    // Update frame capture number
    mFrameCapture = vCaptureParams.size();
    if (mFrameCapture <= 0)
    {
        mfllLogE("create frame number is 0, ignore this capture");
        invokeJpegCallback(NULL);
        return false;
    }

    // 1. set sensor parameters and expected callback message type
    {
        CamManager* pCamMgr = CamManager::getInstance();
        const MUINT32 scenario =
            (pCamMgr->isMultiDevice() && (pCamMgr->getFrameRate(getOpenId()) == 0)) ?
            SENSOR_SCENARIO_ID_NORMAL_PREVIEW :
            SENSOR_SCENARIO_ID_NORMAL_CAPTURE;

        mSensorParam = NSCamShot::SensorParam(
                getOpenId(),  // sensor idx
                scenario,     // Scenaio
                getShotRawBitDepth(),   // bit depth
                MFALSE,       // bypass delay
                MFALSE);      // bypass scenario

        mDataMsgSet = ECamShot_DATA_MSG_JPEG | ECamShot_DATA_MSG_EIS;

        if (isDataMsgEnabled(EIShot_DATA_MSG_POSTVIEW) ||
            property_get_int32("debug.shot.force.postview", -1) > 0) {
            mDataMsgSet |= ECamShot_DATA_MSG_POSTVIEW;
        }

        /* if ask for RAW, which means enable DNG request */
        if (isDataMsgEnabled(EIShot_DATA_MSG_RAW)) {
            mDataMsgSet |= ECamShot_DATA_MSG_APPMETA;
        }
    }

    // 2. create streams
    if (createStreams() != OK)
    {
        mfllLogE("create streams failed");
        return false;
    }

    // 3. create pipeline
    sp<CamManager::UsingDeviceHelper> spDeviceHelper = new CamManager::UsingDeviceHelper(mSensorParam.u4OpenID);
    if (createPipeline() != OK)
    {
        mfllLogE("create pipeline failed");
        invokeJpegCallback(NULL);
        return false;
    }
    spDeviceHelper->configDone();

    // 4. prepare app and hal metadata
    /* ask for flare offset */
    std::shared_ptr<IHal3A> flareOffsetAsker = askFlareOffset(getOpenId());
    if (flareOffsetAsker.get() == NULL) {
        mfllLogE("flare offset may not be applied");
    }

    Vector<SettingSet> vSettings;
    {
        vSettings.resize(vCaptureParams.size());

        // set parameters into metadata
        for (size_t i = 0; i < vSettings.size(); i++)
        {
            vSettings.editItemAt(i).appSetting = mShotParam.mAppSetting;
            vSettings.editItemAt(i).halSetting = mShotParam.mHalSetting;

            // modify app control metadata
            IMetadata &appSetting(vSettings.editItemAt(i).appSetting);
            {
                /* using manual AE mode */
                int iso = vCaptureParams[i].u4RealISO;
                int exp = vCaptureParams[i].u4Eposuretime;
                updateManual3A(&appSetting, iso, exp);

                /* enable for EIS info */
                IMetadata::setEntry<MUINT8>(
                        &appSetting,
                        MTK_CONTROL_VIDEO_STABILIZATION_MODE,
                        MTK_CONTROL_VIDEO_STABILIZATION_MODE_ON
                        );

                /* make sure AWB is locked, locked for (N - 1) frames and resume for the last frame */
                updateIsLockAwb(
                        &appSetting,
                        (i + 1) == vSettings.size() ? MFALSE : MTRUE
                        );
            }

            // modify hal control metadata
            IMetadata &halSetting(vSettings.editItemAt(i).halSetting);
            {
                IMetadata::setEntry<MSize>(
                        &halSetting, MTK_HAL_REQUEST_SENSOR_SIZE, mSensorSetting.size);
                // pause AF for (N - 1) frames and resume for the last frame
                IMetadata::setEntry<MUINT8>(
                        &halSetting, MTK_FOCUS_PAUSE,
                        (i + 1) == vSettings.size() ? 0 : 1);
                IMetadata::setEntry<MUINT8>(
                        &halSetting, MTK_HAL_REQUEST_REQUIRE_EXIF, 1);
            }
        }
    }

    // 5. submit setting to pipeline
    for (size_t i = 0; i < vSettings.size(); i++)
    {
        if (mpPipeline->submitSetting(
                    i, vSettings.editItemAt(i).appSetting, vSettings.editItemAt(i).halSetting) != OK)
        {
            mfllLogE("submit setting failed");
            return false;
        }
    }

    // 6. soldier on until pipeline has finished its job
    if (mpPipeline->waitUntilDrainedAndFlush() != OK)
    {
        mfllLogE("drained failed");
        return false;
    }

    releaseResource();
    spDeviceHelper = NULL;

    return ret;
}

bool MfllShot::zsdCapture()
{
    /* notify as capture done while exiting */
    ScopeWorker ___scope_worker([this](void*/*arg*/){
        mStateMgr.updateState(SHOTSTATE_DONE);
        gMfllShotQueue.deque();
        mSemCaptureDone.post();
    });

    mfllAutoLogFunc();

    mfllAutoTraceFunc();

    bool ret = true;

    // before operation, check state first, if it's cancel, no need to capture
    // anymore
    if (mStateMgr.getState() == SHOTSTATE_CANCEL) {
        return false;
    }

    // 0. Update capture frame parameter set first due to need of the number
    //    of frame to capture
    Vector<CaptureParam_T> vCaptureParams;
    Vector<CaptureParam_T> vOrigCaptureParams; // for 3A stable
    // get Mfll capture parameters
    MfllNode::getCaptureParamSet(getOpenId(), vCaptureParams, &vOrigCaptureParams);
    // update frame capture number
    mFrameCapture = vCaptureParams.size();
    if (mFrameCapture <= 0)
    {
        mfllLogE("create frame number is 0, ignore this capture");
        invokeJpegCallback(NULL);
        return false;
    }

    // 1. set sensor parameters, retrieve stream buffer provider
    // and set expected callback message type
    {
        // force the sensor scenario to capture
        // (reference: ZsdShot's implementation)
        // NOTE:
        // the sensor scenario under ZSD mode is SENSOR_SCENARIO_ID_NORMAL_PREVIEW,
        // which is wrong for capture request and
        // will cause unexpected image quality defects (such as wrong image color).
        mSensorParam = NSCamShot::SensorParam(
                getOpenId(),  // sensor idx
                SENSOR_SCENARIO_ID_NORMAL_CAPTURE, // force here
                getShotRawBitDepth(),   // bit depth
                MFALSE,       // bypass delay
                MFALSE);      // bypass scenario

        // retrieve stream buffer provider
        mpBufferProvider =
            IResourceContainer::getInstance(
                    getOpenId())->queryConsumer(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE);
        if (mpBufferProvider == NULL)
        {
            mfllLogE("can't find StreamBufferProvider in ConsumerContainer");
            return false;
        }

        if (mbOpenLCS) {
            mpLCSOBufferProvider =
                IResourceContainer::getInstance(
                        getOpenId())->queryConsumer(eSTREAMID_IMAGE_PIPE_RAW_LCSO);
            if (mpLCSOBufferProvider == NULL) {
                mfllLogE("can't find StreamBufferProvider(LCSO) in ConsumerContainer");
                return false;
            }
        }

        // set the expected callback message type
        mDataMsgSet = ECamShot_DATA_MSG_JPEG;

        if (isDataMsgEnabled(EIShot_DATA_MSG_POSTVIEW) ||
            property_get_int32("debug.shot.force.postview", -1) > 0) {
            mDataMsgSet |= ECamShot_DATA_MSG_POSTVIEW;
        }

        /* if ask for RAW, which means enable DNG request */
        if (isDataMsgEnabled(EIShot_DATA_MSG_RAW)) {
            mDataMsgSet |= ECamShot_DATA_MSG_APPMETA;
        }
    }

    // 2. create streams
    if (createStreams(mpBufferProvider) != OK)
    {
        mfllLogE("create streams failed");
        return false;
    }

    // 3. create pipeline
    if (createPipeline(mpBufferProvider) != OK)
    {
        mfllLogE("create pipeline failed");
        invokeJpegCallback(NULL);
        return false;
    }

    // 4. prepare app and hal metadata
    Vector<SettingSet> vSettings;
    Vector<MINT32>     vCaptureRequest;
    {
        // resize to the amount of Mfll capture parameters
        vCaptureRequest.resize(vCaptureParams.size());

        // update the number of capture frames
        mFrameCapture = vCaptureParams.size();

#ifdef PREALLOCATE_RAW_BUFFERS
        // increase full raw buffer count
        if (mFrameCapture > 0)
        {
            const size_t maxBufferCount =
                mpImageInfo_fullRaw->getMaxBufNum() + mFrameCapture;

            mfllLogD("increase full raw buffer count(%zu)", maxBufferCount);
            mpBufferProvider->updateBufferCount(LOG_TAG, maxBufferCount);
        }
#endif
        // then append the amount of the original capture parameters
        // at the end of Mfll capture parameters as a series of delayed frames
        //
        // NOTE: the frame delayed time is only needed under ZSD mode
        //
        // even though capture can leverage per-frame control, however,
        // to avoid unstable exposure variation is seen on the display,
        // extra frame dealyed time is needed for AE convergence
        vCaptureParams.appendVector(vOrigCaptureParams);

        // finally, the amount of settings should be a total of
        // the following parameters:
        // 1. Mfll capture parameters
        // 2. the original capture parameters
        vSettings.resize(vCaptureParams.size());

        // ZSD+AIS needs frame delay (maybe)
        if (vOrigCaptureParams.size() > 0)
            mIsZsdAisMode = true;

        // set parameters into metadata
        for (size_t i = 0; i < vSettings.size(); i++)
        {
            vSettings.editItemAt(i).appSetting = mShotParam.mAppSetting;
            vSettings.editItemAt(i).halSetting = mShotParam.mHalSetting;

            auto isDummyFrames = (i >= mFrameCapture);

            // modify app control metadata
            IMetadata &appSetting(vSettings.editItemAt(i).appSetting);
            {
                /* using manual AE mode */
                int iso = vCaptureParams[i].u4RealISO;
                int exp = vCaptureParams[i].u4Eposuretime;

                if (!isDummyFrames) {
                    /* set AE to manual mode */
                    updateManual3A(&appSetting, iso, exp);
                    /* enable for EIS info */
                    IMetadata::setEntry<MUINT8>(
                            &appSetting,
                            MTK_CONTROL_VIDEO_STABILIZATION_MODE,
                            MTK_CONTROL_VIDEO_STABILIZATION_MODE_ON
                            );

                    /* make sure AWB is locked, locked for (N - 1) frames and resume for the last frame */
                    updateIsLockAwb(
                            &appSetting,
                            (i + 1) == vSettings.size() ? MFALSE : MTRUE
                            );
                }
            }

            // modify hal control metadata
            IMetadata &halSetting(vSettings.editItemAt(i).halSetting);
            {
                IMetadata::setEntry<MSize>(
                        &halSetting, MTK_HAL_REQUEST_SENSOR_SIZE, mSensorSetting.size);
                // pause AF for (N - 1) frames and resume for the last frame
                IMetadata::setEntry<MUINT8>(
                        &halSetting, MTK_FOCUS_PAUSE,
                        (i + 1) == vSettings.size() ? 0 : 1);

                if (!isDummyFrames) {
                    IMetadata::setEntry<MUINT8>(
                            &halSetting, MTK_HAL_REQUEST_REQUIRE_EXIF, 1);
                }
            }

            // use capture count as capture request number
            if (i < vCaptureRequest.size())
            {
                vCaptureRequest.editItemAt(i) = i;
            }
        }
    }

    /**
     *  Here comes a job: Update JPEG Orientation.
     *
     *  Why we need this because for scenario ZSD+MFNR we will ask for selector
     *  RAW buffers ASAP and the RAW buffer MfllShot retrieved may not be the
     *  one that AP request (the one may be captured before AP requested),
     *  which means that the APP metadata may be unsynced. And perhaps it will
     *  cause unexpected behaviors such as wrong JPEG orientation information
     *  between metadata and stream info.
     *
     *  Hence we need a job that to update JPEG orientation info if using
     *  ZSD+MFNR.
     */
    std::function<void(IMetadata *ap, IMetadata *t)> job_update_jpeg_orientation;

    /**
     *  thread to late resume preview feature flow control
     */
    std::future<void> future_delay_resume_preview;

    /**
     *  Here comes some jobs:
     *    - Update post NR information
     */
    std::function<void(IMetadata *hal, IMetadata *t)> job_update_hal_metadata;

    /* ask for flare offset */
    std::shared_ptr<IHal3A> flareOffsetAsker = askFlareOffset(getOpenId());
    if (flareOffsetAsker.get() == NULL) {
        mfllLogE("flare offset may not be applied");
    }

    // 5. submit settings to flow control
    sp<ISelector> oriSelector;
    sp<ISelector> selector;
    {
        // backup the original selector
        oriSelector = mpBufferProvider->querySelector();
        if (oriSelector.get() == NULL)
        {
            mfllLogE("the original selector is NULL");
            // OMG, no idea why, but we can create one
        }

        selector = new MfcSelector();
        if (selector.get() == NULL) {
            mfllLogE("create MFC selector failed");
            return false;
        }

        auto status = mpBufferProvider->switchSelector(selector);
        if (status != OK) {
            mfllLogE("change to MFC selector failed");
            return false;
        }

        // send capture requests to flow control
        sp<IFeatureFlowControl> previewFlowCtrl =
            IResourceContainer::getInstance(getOpenId())->queryFeatureFlowControl();
        if (previewFlowCtrl == NULL)
        {
            mfllLogE("previewFlowCtrl is NULL");
            return false;
        }

        // to reduce in-flight requests within the pipeline,
        // pause preview thread to send request to pipeline
        if (previewFlowCtrl->pausePreview(true))
        {
            mfllLogE("pause preview failed, stop capture");
            return false;
        }

        // we need resize RAW because EIS only supports RRZO output.
        BufferList vDstStreams;
        {
            vDstStreams.push_back(
                    BufferSet {
                    .streamId       = eSTREAMID_IMAGE_PIPE_RAW_OPAQUE,
                    .criticalBuffer = MTRUE });

            // for EIS
            vDstStreams.push_back(
                    BufferSet {
                    .streamId       = eSTREAMID_IMAGE_PIPE_RAW_RESIZER,
                    .criticalBuffer = MTRUE });

            if (mbOpenLCS) {
                vDstStreams.push_back(
                    BufferSet{
                    .streamId       = eSTREAMID_IMAGE_PIPE_RAW_LCSO,
                    .criticalBuffer = MTRUE });
            }
        }

        Vector<MINT32> resultRequestNo;
        if (previewFlowCtrl->submitRequest(
                    vSettings, vDstStreams, resultRequestNo) != OK)
        {
            mfllLogE("submit settings failed");
            return false;
        }

#if UNIT_PAUSE_PREVIEW_MSEC
        mPreviewResumeOps.setFlowCtrlOnce(previewFlowCtrl);
        // resume preview thread in N milliseconds later
        future_delay_resume_preview
            = std::async(std::launch::async, [this]() -> void {
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(mFrameCapture * UNIT_PAUSE_PREVIEW_MSEC)
                    );

                if (mPreviewResumeOps.resumePreviewOnce() == 0) {
                    mfllLogD("resume preview by timeout");
                }
            });
#else
        // resume preview thread to send request to pipeline
        if (previewFlowCtrl->resumePreview())
        {
            mfllLogE("resume preview failed, stop capture");
            return false;
        }
#endif

        /* change to directly retrieve frames in pool */
        do {
            /* if using AIS, sorry, we need to wait */
            if (mIsZsdAisMode)
                break;

            /* if ask for DNG (RAW), sorry again, we need to wait meta info we need */
            if (isDataMsgEnabled(EIShot_DATA_MSG_RAW))
                break;

            /* if using EngMode, makes sure ISO/EXP is fixed */
            if (mu4ShotMode == eShotMode_EngShot)
                break;

            for (size_t i = 0; i < resultRequestNo.size(); i++)
                resultRequestNo.editItemAt(i) = MfcSelector::REQUEST_NO_ANY;

            /* describe job of update jpeg orientation */
            // {{{
            job_update_jpeg_orientation = [](IMetadata *ap, IMetadata *t) -> void {
                /* describe JPEG orientation retrieve function */
                const MUINT32 TAG = MTK_JPEG_ORIENTATION;
                auto retrieveOrientation = [](IMetadata *m, MINT32 &rOrientation) -> MBOOL {
                    MINT32 orientation = -1;
                    if (m == NULL)
                        return MFALSE;

                    IMetadata::IEntry entry = m->entryFor(TAG);
                    if (!entry.isEmpty()) {
                        rOrientation = entry.itemAt(0, Type2Type<MINT32>());
                        return MTRUE;
                    }
                    return MFALSE;
                };

                /* check if the orientation is different */
                MINT32 requestOrientation = -1;
                MINT32 targetOrientation = -1;
                if (!retrieveOrientation(ap, requestOrientation) ||
                        !retrieveOrientation(t, targetOrientation)) {
                    mfllLogW("cannot retrieve JPEG orientation neither request \
                            metadata nor target metadata");
                    return;
                }

                /**
                 *  if the orientation are different between target & request,
                 *  update target one by request
                 */
                if (requestOrientation != targetOrientation) {
                    IMetadata::setEntry<MINT32>(t, TAG, requestOrientation);
                }
            };
            //}}}

            job_update_hal_metadata = [this](IMetadata* /*hal*/, IMetadata *t) -> void {
                // job: update post NR information
                // {{{
                updatePostNRSetting(MTK_NR_MODE_AUTO, true, t);
                // }}}
            };

        } while(0);

        // set expected request numbers
        reinterpret_cast<MfcSelector*>(selector.get())->setWaitRequestNo(resultRequestNo);
    }

    // 6. get capture results from selector,
    //    pass app result metadata to timestamp processor and
    //    submit setting to pipeline
    { ///{{{
        IMetadata appSetting;
        IMetadata halSetting;
        size_t sentRequestCnt = 0;
        // we try to get selected result from selector, if still failed in following
        // BREAK_TIMES times, give up all frames indeed
        size_t contFailedTimes = 0;
        const size_t BREAK_TIMES = 3;

        for (size_t i = 0; i < vCaptureRequest.size(); i++)
        {
            /* if asking for cancel, don't wait anymore */
            if (mStateMgr.getState() == SHOTSTATE_CANCEL)
                break;

            // blocking call to get captured result
            auto err = getSelectResult(selector, appSetting, halSetting);

            if (err)
            {
                mfllLogW("get selected result failed idx(%zu), failed times(%zu), " \
                        "adds a dummpy request", i, ++contFailedTimes);
                continue; // give up this stage.
            }

            // NOTE: migrate jpeg thumbnail size and jpeg orientation
            // from adapter's control metadata to MFC pipeline's control metadata
            // TODO: pass the entire adapter's control metadata into MFC pipeline
            {
                MSize thumbnailsize;
                if (IMetadata::getEntry<MSize>(
                            &mShotParam.mAppSetting,
                            MTK_JPEG_THUMBNAIL_SIZE,
                            thumbnailsize))
                {
                    IMetadata::setEntry<MSize>(
                            &appSetting, MTK_JPEG_THUMBNAIL_SIZE,
                            thumbnailsize);
                }
                else
                {
                    mfllLogE("update MTK_JPEG_THUMBNAIL_SIZE failed");
                }

                MINT32 orientation;
                if (IMetadata::getEntry<MINT32>(
                            &mShotParam.mAppSetting,
                            MTK_JPEG_ORIENTATION,
                            orientation))
                {
                    IMetadata::setEntry<MINT32>(
                            &appSetting, MTK_JPEG_ORIENTATION,
                            orientation);
                }
                else
                {
                    mfllLogE("update MTK_JPEG_ORIENTATION failed");
                }
            }

            // check if we need to do the job of update JPEG orientation
            if (job_update_jpeg_orientation) {
                job_update_jpeg_orientation(
                        &mShotParam.mAppSetting,
                        &appSetting);
            }

            // update hal metadata if the job exists
            if (job_update_hal_metadata) {
                job_update_hal_metadata(
                        &mShotParam.mHalSetting,
                        &halSetting);
            }

            // if continuously fail more than BREAK_TIMES, stop waiting anymore
            if (contFailedTimes >= BREAK_TIMES)
            {
                mfllLogE("continuous failed %zu times, don't wait anymore", BREAK_TIMES);
                break; // give up all frames
            }
            else
                contFailedTimes = 0;


            /* get the first app metadata */
            saveSelectorAppMetadataLocked(appSetting);

            // pass app result metadata to timestamp processor
            {
                sp<TimestampProcessor> timestampProcessor =
                    mpPipeline->getTimestampProcessor().promote();
                if (timestampProcessor.get())
                {
                    timestampProcessor->onResultReceived(
                            0,
                            eSTREAMID_META_APP_DYNAMIC_P1,
                            MFALSE,
                            appSetting);
                }
                else
                {
                    mfllLogE("promote timestamp processor failed");
                }
            }

            // submit request to pipeline
            if (mpPipeline->submitSetting(
                        vCaptureRequest[sentRequestCnt], appSetting, halSetting) != OK)
            {
                mfllLogE("submit setting failed");
                return false;
            }

            sentRequestCnt++;
        } // for-loop

        /* check sent frame count, if any frame dropped... */
        for (size_t i = sentRequestCnt; i < vCaptureRequest.size(); i++, sentRequestCnt++)
        {
            mfllLogW("send dummy request %zu", i);

            /* set a dummpy request */
            appSetting = IMetadata();
            halSetting = IMetadata();

            // NOTE: migrate jpeg thumbnail size and jpeg orientation
            // from adapter's control metadata to MFC pipeline's control metadata
            // TODO: pass the entire adapter's control metadata into MFC pipeline
            {
                MSize thumbnailsize;
                if (IMetadata::getEntry<MSize>(
                            &mShotParam.mAppSetting,
                            MTK_JPEG_THUMBNAIL_SIZE,
                            thumbnailsize))
                {
                    IMetadata::setEntry<MSize>(
                            &appSetting, MTK_JPEG_THUMBNAIL_SIZE,
                            thumbnailsize);
                }
                else
                {
                    mfllLogE("update MTK_JPEG_THUMBNAIL_SIZE failed");
                }

                MINT32 orientation;
                if (IMetadata::getEntry<MINT32>(
                            &mShotParam.mAppSetting,
                            MTK_JPEG_ORIENTATION,
                            orientation))
                {
                    IMetadata::setEntry<MINT32>(
                            &appSetting, MTK_JPEG_ORIENTATION,
                            orientation);
                }
                else
                {
                    mfllLogW("update MTK_JPEG_ORIENTATION failed");
                }
            }

            // indicate that this request is used for error handling
            IMetadata::setEntry<MUINT8>(&halSetting, MTK_HAL_REQUEST_ERROR_FRAME, 1);

            // pass app result metadata to timestamp processor
            sp<TimestampProcessor> timestampProcessor =
                mpPipeline->getTimestampProcessor().promote();
            if (timestampProcessor.get())
            {
                timestampProcessor->onResultReceived(
                        0,
                        eSTREAMID_META_APP_DYNAMIC_P1,
                        MFALSE,
                        appSetting);
            }
            else
            {
                mfllLogE("promote timestamp processor failed");
            }

            // submit request to pipeline
            if (mpPipeline->submitSetting(
                        vCaptureRequest[sentRequestCnt], appSetting, halSetting) != OK)
            {
                mfllLogE("submit setting failed");
                return false;
            }
        }

        // we have received all input frames now,
        // send metadata event for shutter callback
        onResultReceived(
                vCaptureRequest[sentRequestCnt - 1],
                eSTREAMID_META_HAL_DYNAMIC_P1,
                MFALSE,
                halSetting);

        vCaptureRequest.clear();
    } // }}} Step 6

    /* clear flare offset akser after captured frames */
    flareOffsetAsker = NULL;

    // after frames captured, set selector back to the original one
    mpBufferProvider->switchSelector(oriSelector);

    // 7. soldier on until pipeline has finished its job
    if (mpPipeline->waitUntilDrained() != OK)
    {
        mfllLogE("drained failed");
        return false;
    }

    releaseResource();

    return ret;
}

MUINT32 MfllShot::getRotation() const
{
    return mShotParam.mu4Transform;
}

status_t MfllShot::createStreams(const sp<StreamBufferProvider>& provider)
{
    mfllAutoLogFunc();

    mfllAutoTraceFunc();

    MUINT32 const openId     = mSensorParam.u4OpenID;
    MUINT32 const sensorMode = mSensorParam.u4Scenario;
    MUINT32 const bitDepth   = mSensorParam.u4Bitdepth;

    MSize const postviewsize =
        MSize(mShotParam.mi4PostviewWidth, mShotParam.mi4PostviewHeight);
    MINT const postviewfmt   = mShotParam.miPostviewClientFormat;
    // NOTE: jpeg & thumbnail yuv only support YUY2
    MINT const yuvfmt        = eImgFmt_YUY2;
    MSize const jpegsize     = (getRotation() & eTransform_ROT_90) ?
        MSize(mShotParam.mi4PictureHeight, mShotParam.mi4PictureWidth):
        MSize(mShotParam.mi4PictureWidth, mShotParam.mi4PictureHeight);
    MSize const thumbnailsize =
        MSize(mJpegParam.mi4JpegThumbWidth, mJpegParam.mi4JpegThumbHeight);

    MBOOL const openLCS = mbOpenLCS;

    HwInfoHelper helper(openId);
    if (!helper.updateInfos())
    {
        mfllLogE("update sensor static information failed");
        return BAD_VALUE;
    }

    if (!helper.getSensorSize(sensorMode, mSensorSetting.size) ||
        !helper.getSensorFps(sensorMode, mSensorSetting.fps) ||
        !helper.queryPixelMode(
            sensorMode, mSensorSetting.fps, mSensorSetting.pixelMode))
    {
        mfllLogE("cannot get params about sensor");
        return BAD_VALUE;
    }

    if (provider == NULL)
    {
        mfllAutoLog("normal mode");

        // full raw
        {
            MSize size = mSensorSetting.size;
            MINT format;
            size_t stride;
            MUINT const usage =
                eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE;
            MBOOL useUFOfmt = MTRUE;
            if (getShotMode() == eShotMode_EngShot)
            {
                useUFOfmt = MFALSE;
            }

            if (!helper.getImgoFmt(bitDepth, format, useUFOfmt) ||
                !helper.alignPass1HwLimitation(
                    mSensorSetting.pixelMode, format, true, size, stride))
            {
                mfllLogE("wrong parameter for IMGO");
                return BAD_VALUE;
            }

            mpImageInfo_fullRaw = createRawImageStreamInfo(
                    "MfllShot:FullRaw",
                    eSTREAMID_IMAGE_PIPE_RAW_OPAQUE,
                    eSTREAMTYPE_IMAGE_INOUT,
                    mFrameCapture, 1, // MAX, MIN
                    usage, format, size, stride);

            if (mpImageInfo_fullRaw == NULL)
            {
                mfllLogE("create ImageStreamInfo FullRaw failed");
                return NO_INIT;
            }

            dumpImageStreamInfo(mpImageInfo_fullRaw);
        }

        // resized raw
        if (helper.isRaw() && !!postviewsize)
        {
            MSize size;
            MINT format;
            size_t stride;
            MUINT const usage =
                eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE;

            MBOOL useUFOfmt = MFALSE;
            if (getShotMode() == eShotMode_EngShot)
            {
                useUFOfmt = MFALSE;
            }
            if(!helper.getRrzoFmt(bitDepth, format, useUFOfmt) ||
                    !helper.alignRrzoHwLimitation(
                        postviewsize, mSensorSetting.size, size) ||
                    !helper.alignPass1HwLimitation(
                        mSensorSetting.pixelMode, format, false, size, stride))
            {
                mfllLogE("wrong parameter for RRZO");
                return BAD_VALUE;
            }

            mpImageInfo_resizedRaw = createRawImageStreamInfo(
                    "MfllShot:ResizedRaw",
                    eSTREAMID_IMAGE_PIPE_RAW_RESIZER,
                    eSTREAMTYPE_IMAGE_INOUT,
                    mFrameCapture, 0, // MfllShot don't need resized raw, but
                                      // EIS does. So we still need these frames
                    usage, format, size, stride);

            if (mpImageInfo_resizedRaw == NULL)
            {
                mfllLogE("create ImageStreamInfo ResizedRaw failed");
                return BAD_VALUE;
            }

            dumpImageStreamInfo(mpImageInfo_resizedRaw);
        }

        // lcso raw (the buffer count should be the same as that of RAW buffer)
        if (openLCS)
        {
            LCSO_Param lcsoParam;
            if ( auto pIspMgr = MAKE_IspMgr() ) {
                pIspMgr->queryLCSOParams(lcsoParam);
            }

            MINT const usage = 0; // not necessary here

            mpImageInfo_lcsoRaw = createRawImageStreamInfo(
                    "MfllShot:LcsoRaw",
                    eSTREAMID_IMAGE_PIPE_RAW_LCSO,
                    eSTREAMTYPE_IMAGE_INOUT,
                    mFrameCapture, 1,
                    usage, lcsoParam.format, lcsoParam.size, lcsoParam.stride);

            if (mpImageInfo_lcsoRaw == NULL)
            {
                mfllLogE("create ImageStreamInfo LcsoRaw failed");
                return NO_INIT;
            }

            dumpImageStreamInfo(mpImageInfo_lcsoRaw);
        }
    }
    else
    {
        mfllAutoLog("ZSD mode");

        // ZSD mode
        mpImageInfo_fullRaw = provider->queryImageStreamInfo();
        dumpImageStreamInfo(mpImageInfo_fullRaw);

        // retrieve LCSO buffer provider if exists
        if (mpLCSOBufferProvider.get()) {
            mpImageInfo_lcsoRaw = mpLCSOBufferProvider->queryImageStreamInfo();
            if (mpImageInfo_lcsoRaw.get() == NULL) {
                mfllLogE("%s: queryImageStreamInfo from LCSO buffer provider returns NULL", __FUNCTION__);
                return UNKNOWN_ERROR;
            }

            dumpImageStreamInfo(mpImageInfo_lcsoRaw);
        }

        mpImageInfo_resizedRaw = NULL;

    }

    // postview
    if (mDataMsgSet & ECamShot_DATA_MSG_POSTVIEW)
    {
        MSize size  = postviewsize;
        MINT format = postviewfmt;
        MUINT const usage = 0; //not necessary here
        MUINT32 transform = 0;

        mpImageInfo_yuvPostview = createImageStreamInfo(
                "MfllShot:Postview",
                eSTREAMID_IMAGE_PIPE_YUV_00,
                eSTREAMTYPE_IMAGE_INOUT,
                1, 1,
                usage, format, size, transform);

        if (mpImageInfo_yuvPostview == NULL)
        {
            mfllLogE("create ImageStreamInfo Postview failed");
            return BAD_VALUE;
        }

        dumpImageStreamInfo(mpImageInfo_yuvPostview);
    }

    // jpeg yuv
    if (mDataMsgSet & (ECamShot_DATA_MSG_YUV|ECamShot_DATA_MSG_JPEG))
    {
        MSize size  = jpegsize;
        MINT format = yuvfmt;
        MUINT const usage = 0; //not necessary here
        MUINT32 transform = getRotation();

        mpImageInfo_yuvJpeg = createImageStreamInfo(
                "MfllShot:YuvJpeg",
                eSTREAMID_IMAGE_PIPE_YUV_JPEG,
                eSTREAMTYPE_IMAGE_INOUT,
                1, 1,
                usage, format, size, transform);

        if (mpImageInfo_yuvJpeg == NULL)
        {
            mfllLogE("create ImageStreamInfo YuvJpeg failed");
            return BAD_VALUE;
        }

        dumpImageStreamInfo(mpImageInfo_yuvJpeg);
    }

    // thumbnail yuv
    if (mDataMsgSet & ECamShot_DATA_MSG_JPEG)
    {
        MSize size  = thumbnailsize;
        MINT format = yuvfmt;
        MUINT const usage = 0; //not necessary here
        MUINT32 transform = 0;

        mpImageInfo_yuvThumbnail = createImageStreamInfo(
                "MfllShot:YuvThumbnail",
                eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL,
                eSTREAMTYPE_IMAGE_INOUT,
                1, 1,
                usage, format, size, transform);

        if (mpImageInfo_yuvThumbnail == NULL)
        {
            mfllLogE("create ImageStreamInfo YuvThumbnail failed");
            return BAD_VALUE;
        }

        dumpImageStreamInfo(mpImageInfo_yuvThumbnail);
    }

    // jpeg
    if (mDataMsgSet & ECamShot_DATA_MSG_JPEG)
    {
        MSize size  = jpegsize;
        MINT format = eImgFmt_BLOB;
        MUINT const usage = 0; //not necessary here
        MUINT32 transform = 0;

        mpImageInfo_jpeg = createImageStreamInfo(
                "MfllShot:Jpeg",
                eSTREAMID_IMAGE_MFLL,
                eSTREAMTYPE_IMAGE_INOUT,
                1, 1,
                usage, format, size, transform);

        if (mpImageInfo_jpeg == NULL)
        {
            mfllLogE("create ImageStreamInfo Jpeg failed");
            return BAD_VALUE;
        }

        dumpImageStreamInfo(mpImageInfo_jpeg);
    }

    return OK;
}

status_t MfllShot::createPipeline(const sp<StreamBufferProvider>& provider)
{
    mfllAutoLogFunc();
    mfllAutoTraceFunc();

    auto shotMode = getShotMode();
    if ((shotMode != eShotMode_MfllShot) &&
        (shotMode != eShotMode_ZsdMfllShot) &&
        (shotMode != eShotMode_EngShot))
    {
        mfllLogE("shot mode is not MfllShot, it's %u", shotMode);
        return BAD_VALUE;
    }

    LegacyPipelineBuilder::ConfigParams LPBConfigParams;
    /* PipelineMode_ZsdMfll is only for shot mode ZsdMfllShot */
    LPBConfigParams.mode = (shotMode == eShotMode_ZsdMfllShot)
        ? LegacyPipelineMode_T::PipelineMode_ZsdMfll
        : LegacyPipelineMode_T::PipelineMode_Mfll
        ;

    LPBConfigParams.enableEIS = MTRUE;
    LPBConfigParams.enableLCS = mbOpenLCS;

    HwInfoHelper helper(mSensorParam.u4OpenID);
    if( ! helper.updateInfos() ) {
        mfllLogE("cannot properly update infos");
        return BAD_VALUE;
    }
    //
    if (helper.getDualPDAFSupported(mSensorParam.u4Scenario))
    {
        LPBConfigParams.enableDualPD = MTRUE;
    }
    //
    MUINT32 pipeBit;
    MINT ImgFmt = eImgFmt_BAYER12;
    if( helper.getLpModeSupportBitDepthFormat(ImgFmt, pipeBit) )
    {
        using namespace NSCam::NSIoPipe::NSCamIOPipe;
        if(pipeBit & CAM_Pipeline_14BITS)
        {
            LPBConfigParams.pipeBit = CAM_Pipeline_14BITS;
        }
        else
        {
            LPBConfigParams.pipeBit = CAM_Pipeline_12BITS;
        }
    }
    //
    sp<LegacyPipelineBuilder> builder =
        LegacyPipelineBuilder::createInstance(
                mSensorParam.u4OpenID,
                "MfllShot",
                LPBConfigParams);

    if (builder == NULL) {
        mfllLogE("create LegacyPipelineBuilder failed");
        return BAD_VALUE;
    }

    mpCallbackHandler = new BufferCallbackHandler(mSensorParam.u4OpenID);

    // register image listener
    mpCallbackHandler->setImageCallback(this);

    sp<StreamBufferProviderFactory> bufferProvider =
        StreamBufferProviderFactory::createInstance();

    Vector<PipelineImageParam> vImageParam;

    if (provider == NULL)
    {
        mfllAutoLog("normal mode");

        // normal mode
        //
        {
            sp<IScenarioControl> scenarioCtrl =
                IScenarioControl::create(mSensorParam.u4OpenID);
            if (scenarioCtrl == NULL)
            {
                mfllLogE("get scenario control failed");
                return UNKNOWN_ERROR;
            }

            IScenarioControl::ControlParam param;
            param.scenario   = IScenarioControl::Scenario_Capture;
            param.sensorSize = mSensorSetting.size;
            param.sensorFps  = mSensorSetting.fps;
            if(LPBConfigParams.enableDualPD)
                FEATURE_CFG_ENABLE_MASK(param.featureFlag,IScenarioControl::FEATURE_DUAL_PD);

            scenarioCtrl->enterScenario(param);
            builder->setScenarioControl(scenarioCtrl);
        }

        // full raw
        if (mpImageInfo_fullRaw.get())
        {
            sp<IImageStreamInfo>& pStreamInfo(mpImageInfo_fullRaw);

            sp<CallbackBufferPool> cbBufferPool =
                prepareCallbackBufferPool(mpCallbackHandler, pStreamInfo, true);
            bufferProvider->setImageStreamInfo(pStreamInfo);
            bufferProvider->setUsersPool(cbBufferPool);

            PipelineImageParam imageParam =
            {
                .pInfo     = pStreamInfo,
                .pProvider = bufferProvider->create(),
                .usage     = 0
            };
            vImageParam.push_back(imageParam);

            mfllLogD("add full raw");
        }

        // resized raw
        if (mpImageInfo_resizedRaw.get())
        {
            sp<IImageStreamInfo>& pStreamInfo(mpImageInfo_resizedRaw);

            PipelineImageParam imageParam =
            {
                .pInfo     = pStreamInfo,
                .pProvider = NULL,
                .usage     = 0
            };
            vImageParam.push_back(imageParam);

            mfllLogD("add resized raw");
        }

        // lcso raw
        if (mpImageInfo_lcsoRaw.get())
        {
            sp<IImageStreamInfo>& pStreamInfo(mpImageInfo_lcsoRaw);

            PipelineImageParam imageParam =
            {
                .pInfo     = pStreamInfo,
                .pProvider = NULL,
                .usage     = 0
            };
            vImageParam.push_back(imageParam);

            mfllLogD("add lcso raw");
        }

        PipelineSensorParam sensorParam =
        {
            mSensorParam.u4Scenario,
            mSensorParam.u4RawType,
            mSensorSetting.size,
            (MUINT32)mSensorSetting.fps,
            mSensorSetting.pixelMode,
            SENSOR_VHDR_MODE_NONE
        };

        if (OK != builder->setSrc(sensorParam))
        {
            mfllLogE("normal mode - setSrc failed");
            return UNKNOWN_ERROR;
        }
    }
    else
    {
        mfllAutoLog("ZSD mode");

        // ZSD mode
        //
        Vector<PipelineImageParam> imgSrcParams;
        sp<IImageStreamInfo>& pStreamInfo(mpImageInfo_fullRaw);

        mpP1Pool = prepareCallbackBufferPool(mpCallbackHandler, pStreamInfo, false);
        bufferProvider->setImageStreamInfo(pStreamInfo);
        bufferProvider->setUsersPool(mpP1Pool);

        PipelineImageParam imageParam =
        {
            .pInfo     = pStreamInfo,
            .pProvider = bufferProvider->create(false), // we don't need sync buffer and timestamp
            .usage     = 0
        };
        imgSrcParams.push_back(imageParam);

        if (mpImageInfo_lcsoRaw.get()) {
            sp<IImageStreamInfo>& pStreamInfo(mpImageInfo_lcsoRaw);
            mpLCSOPool = prepareCallbackBufferPool(mpCallbackHandler, pStreamInfo, false);
            bufferProvider->setImageStreamInfo(pStreamInfo);
            bufferProvider->setUsersPool(mpLCSOPool);

            PipelineImageParam imageParam = {
                .pInfo      = pStreamInfo,
                .pProvider  = bufferProvider->create(),
                .usage      = 0
            };
            imgSrcParams.push_back(imageParam);

            mfllLogD("add LCSO raw");
        }

        if (OK != builder->setSrc(imgSrcParams))
        {
            mfllLogE("ZSD mode - setSrc failed");
            return UNKNOWN_ERROR;
        }
    }

    // jpeg yuv
    if (mpImageInfo_yuvJpeg.get())
    {
        sp<IImageStreamInfo>& pStreamInfo(mpImageInfo_yuvJpeg);

        sp<CallbackBufferPool> cbBufferPool =
            prepareCallbackBufferPool(mpCallbackHandler, pStreamInfo, true);
        bufferProvider->setImageStreamInfo(pStreamInfo);
        bufferProvider->setUsersPool(cbBufferPool);

        PipelineImageParam imageParam =
        {
            .pInfo     = pStreamInfo,
            .pProvider = bufferProvider->create(),
            .usage     = 0
        };
        vImageParam.push_back(imageParam);

        mfllLogD("add jpeg");
    }

    // postview
    if (mpImageInfo_yuvPostview.get())
    {
        sp<IImageStreamInfo>& pStreamInfo(mpImageInfo_yuvPostview);

        sp<CallbackBufferPool> cbBufferPool =
            prepareCallbackBufferPool(mpCallbackHandler, pStreamInfo, true);
        bufferProvider->setImageStreamInfo(pStreamInfo);
        bufferProvider->setUsersPool(cbBufferPool);

        PipelineImageParam imageParam =
        {
            .pInfo     = pStreamInfo,
            .pProvider = bufferProvider->create(),
            .usage     = 0
        };
        vImageParam.push_back(imageParam);

        mfllLogD("add postview");
    }

    // thumbnail yuv
    if (mpImageInfo_yuvThumbnail.get())
    {
        sp<IImageStreamInfo>& pStreamInfo(mpImageInfo_yuvThumbnail);

        PipelineImageParam imageParam =
        {
            .pInfo     = pStreamInfo,
            .pProvider = NULL,
            .usage     = 0
        };
        vImageParam.push_back(imageParam);

        mfllLogD("add thumbnail yuv");
    }

    // jpeg
    if (mpImageInfo_jpeg.get())
    {
        sp<IImageStreamInfo>& pStreamInfo(mpImageInfo_jpeg);

        sp<CallbackBufferPool> cbBufferPool =
            prepareCallbackBufferPool(mpCallbackHandler, pStreamInfo, true);
        bufferProvider->setImageStreamInfo(pStreamInfo);
        bufferProvider->setUsersPool(cbBufferPool);

        PipelineImageParam imageParam =
        {
            .pInfo     = pStreamInfo,
            .pProvider = bufferProvider->create(),
            .usage     = 0
        };
        vImageParam.push_back(imageParam);

        mfllLogD("add jpeg yuv");
    }

    mfllLogD("image param count(%zu)", vImageParam.size());

    if (OK != builder->setDst(vImageParam))
    {
        mfllLogE("setDst failed");
        return UNKNOWN_ERROR;
    }

    mpPipeline = builder->create();
    if (mpPipeline == NULL) {
        mfllLogE("pBuffer->create returns NULL");
        return BAD_VALUE;
    }

    {
        sp<ResultProcessor> resultProcessor =
            mpPipeline->getResultProcessor().promote();
        if (resultProcessor == NULL) {
            mfllLogE("getResultProcessor failed");
            return BAD_VALUE;
        }

        // register partial metadata listener
        resultProcessor->registerListener(0, 0, true, this);
        // register full metadata listener
        resultProcessor->registerListener(0, 0, false, this);
    }

    return OK;
}

status_t MfllShot::getSelectResult(
        const sp<ISelector>& selector,
        IMetadata& appSetting,
        IMetadata& halSetting)
{
    if (selector == NULL)
    {
        mfllLogE("selector is NULL");
        return NO_INIT;
    }

    MINT32 requestNo;
    Vector<ISelector::MetaItemSet> resultMeta;
    sp<IImageBufferHeap> heap;
    Vector<ISelector::BufferItemSet> rvBufferSet;
    status_t err = selector->getResult(requestNo, resultMeta, rvBufferSet);
    if (err)
    {
        mfllLogE("get select result failed(%s)", strerror(-err));
        return err;
    }

    if(rvBufferSet.size() <= 0 || rvBufferSet[0].id != eSTREAMID_IMAGE_PIPE_RAW_OPAQUE)
    {
        mfllLogE("Selector get input raw buffer failed! bufferSetSize(%zu)", rvBufferSet.size());
        return UNKNOWN_ERROR;
    }
    heap = rvBufferSet[0].heap;

    mfllLogD("getSelectResult(%d)", requestNo);

    for (size_t i = 0; i < resultMeta.size(); i++)
    {
        if (resultMeta[i].id == eSTREAMID_META_APP_DYNAMIC_P1)
        {
            appSetting = resultMeta[i].meta;
            mfllLogD("P1 app dynamic found");
        }
        else if (resultMeta[i].id == eSTREAMID_META_HAL_DYNAMIC_P1)
        {
            halSetting = resultMeta[i].meta;
            mfllLogD("P1 hal dynamic found");
        }
    }

    if (appSetting.isEmpty() || halSetting.isEmpty())
    {
        mfllLogE("either app(%s) or hal(%s) result metadata is empty",
                appSetting.isEmpty() ? "y" : "n",
                halSetting.isEmpty() ? "y" : "n");
        return BAD_VALUE;
    }

    // get and register captured image buffer into callback buffer pool
    for (size_t i = 0; i < rvBufferSet.size(); i++)
    {
        sp<IImageBuffer> buffer = rvBufferSet[i].heap->createImageBuffer();
        if (buffer == NULL)
        {
            mfllLogE("result buffer %zu is NULL", i);
            return BAD_VALUE;
        }

        switch (rvBufferSet[i].id)
        {
            case eSTREAMID_IMAGE_PIPE_RAW_OPAQUE:
                {
                    err = mpP1Pool->addBuffer(buffer);
                    std::lock_guard<std::mutex> __l(mMutexSelectedBuffer);
                    mpSelectedBuffer.push_back(buffer);
                    mfllLogD("full raw found");
                }
                break;
            case eSTREAMID_IMAGE_PIPE_RAW_LCSO:
                {
                    err = mpLCSOPool->addBuffer(buffer);
                    mpSelectedLCSOBuffer.push_back(buffer);
                    mfllLogD("lcso raw found");
                }
                break;
            default:
                mfllLogW("unsupported image buffer got, id(%#" PRIxPTR ")", rvBufferSet[i].id);
        }
    }

    return err;
}

status_t MfllShot::returnSelectedBuffers()
{
    // selected buffers are only available in ZSD mode
    if (eShotMode_ZsdMfllShot != mu4ShotMode)
        return OK;

    // return selected buffers to selector and then restore selector
    if (mpBufferProvider.get())
    {
        // return selected buffers to selector
        sp<ISelector> selector = mpBufferProvider->querySelector();
        if (selector.get())
        {
            do {
                std::lock_guard<std::mutex> __l(mMutexSelectedBuffer); // lock for vector operation
                for (size_t i = 0; i < mpSelectedBuffer.size(); i++)
                {
                    mfllLogD("return buffer(%zu)", i);
                    ISelector::BufferItemSet bufferSet{eSTREAMID_IMAGE_PIPE_RAW_OPAQUE, mpSelectedBuffer[i]->getImageBufferHeap(), NULL};
                    selector->returnBuffer(bufferSet);
                }
                mpSelectedBuffer.clear();
            } while(0);

            for (size_t i = 0; i < mpSelectedLCSOBuffer.size(); i++)
            {
                mfllLogD("return lcso buffer(%zu)", i);
                ISelector::BufferItemSet bufferSet {
                        eSTREAMID_IMAGE_PIPE_RAW_LCSO,
                        mpSelectedLCSOBuffer[i]->getImageBufferHeap(),
                        NULL};
                selector->returnBuffer(bufferSet);
            }
            mpSelectedLCSOBuffer.clear();

            // return all buffers kept in selector to buffer pool
            selector->flush();
        }

#ifdef PREALLOCATE_RAW_BUFFERS
        // decrease full raw buffer count
        if (mFrameCapture > 0)
        {
            const size_t maxBufferCount = mpImageInfo_fullRaw->getMaxBufNum() - mFrameCapture;

            mfllLogD("decrease full raw buffer count(%zu)", maxBufferCount);
            mpBufferProvider->updateBufferCount(LOG_TAG, maxBufferCount);
            mFrameCapture = 0;
        }
#endif
    }

    return OK;
}

status_t MfllShot::returnSelectedBuffer(sp<IImageBuffer> &buffer)
{
    if (eShotMode_ZsdMfllShot != mu4ShotMode)
        return OK;

    if (buffer.get() == NULL) {
        mfllLogE("%s: cannot return NULL buffer to selector", __FUNCTION__);
        return BAD_VALUE;
    }

    if (mpBufferProvider.get() == NULL) {
        mfllLogW("%s: buffer provider is NULL, cannot return buffer", __FUNCTION__);
        return OK;
    }

    sp<ISelector> selector = mpBufferProvider->querySelector();
    if (selector.get() == NULL) {
        mfllLogW("%s: selector is NULL, cannot return buffer", __FUNCTION__);
        return OK;
    }

    // return buffer
    mfllLogD("%s: return buffer %p", __FUNCTION__, buffer->getImageBufferHeap());
    ISelector::BufferItemSet bufferSet{eSTREAMID_IMAGE_PIPE_RAW_OPAQUE, buffer->getImageBufferHeap(), NULL};
    selector->returnBuffer(bufferSet);

    // remove from stack
    do {
        std::lock_guard<std::mutex> __l(mMutexSelectedBuffer);
        auto itr = std::find_if(mpSelectedBuffer.begin(), mpSelectedBuffer.end(),
                [&buffer](auto &element) -> bool {
                    return (element->getImageBufferHeap() == buffer->getImageBufferHeap());
                });
        if (itr != mpSelectedBuffer.end()) {
            mpSelectedBuffer.erase(itr);
        }
    } while(0);

    return OK;
}

status_t MfllShot::releaseResource()
{
    mfllAutoLogFunc();
    mfllAutoTraceFunc();

    mfllLogD("[%d] release resource", mSequence);

    // release pipeline
    mpPipeline.clear();

    // NOTE:
    // we call returnSelectedBuffers() again just in case
    // if it did not be executed from onResultReceived()
    if (eShotMode_ZsdMfllShot == mu4ShotMode)
        returnSelectedBuffers();

    mpCallbackHandler.clear();

    if (eShotMode_ZsdMfllShot == mu4ShotMode)
    {
        mpBufferProvider.clear();
        mpLCSOBufferProvider.clear();
        mpP1Pool.clear();
        mpLCSOPool.clear();
    }

    return OK;
}

void MfllShot::invokeJpegCallback(sp<IImageBuffer> buffer)
{
    // check invoke times first and increase
    auto times = m_atomicJpegDone++;
    if (times > 0)
    {
        return;
    }

    if (buffer.get() == NULL)
    {
        mfllLogD("invoke fake JPEG done CB");
        onFakeJpegCallback(mpShotCallback);
    }
    else
    {
        // set callback index to 1 and the final image flag to true
        buffer->lockBuf(LOG_TAG, eBUFFER_USAGE_SW_READ_MASK);
        {
            const uint32_t u4Size = buffer->getBitstreamSize();
            const uint8_t *puBuf  = (uint8_t const*)buffer->getBufVA(0);
            mpShotCallback->onCB_CompressedImage_packed(
                    0, u4Size, puBuf, 0, true, MTK_CAMERA_MSG_EXT_DATA_COMPRESSED_IMAGE, getShotMode());
        }
        buffer->unlockBuf(LOG_TAG);
    }
}

void MfllShot::waitCaptureDone()
{
    wait_capture_done();
}

void MfllShot::announceCaptureDone()
{
    int times = m_atomicCaptureDoneTimes++;
    if (times <= 0)
        announce_capture_done();
}

void MfllShot::saveSelectorAppMetadataLocked(const IMetadata &m)
{
    if (!isDataMsgEnabled(EIShot_DATA_MSG_RAW))
        return;

    Mutex::Autolock _l(&mSelectorMetadataPack.mx);

    if (mSelectorMetadataPack.bInited)
        return;

    mSelectorMetadataPack.bInited = true;
    mSelectorMetadataPack.meta = m;
}

MfllShot::SelectorAppMetaStatus
MfllShot::getSelectorAppMetadataLocked(IMetadata *pOutMeta)
{
    if (!isDataMsgEnabled(EIShot_DATA_MSG_RAW))
        return SAM_STATUS_NO_NEED; // No need

    Mutex::Autolock _l(&mSelectorMetadataPack.mx);

    if (mSelectorMetadataPack.bInited == false)
        return SAM_STATUS_NOT_INITED;

    *pOutMeta = mSelectorMetadataPack.meta;
    return SAM_STATUS_OK;
}

MfllShot::LCSSTATUS MfllShot::getLcsStatus()
{
    return (mShotParam.mbEnableLtm == MTRUE
            ? LCSSTATUS_ENABLED
            : LCSSTATUS_DISABLED
           );
}


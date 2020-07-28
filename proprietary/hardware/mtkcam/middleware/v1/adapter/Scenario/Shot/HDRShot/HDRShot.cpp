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

#define DEBUG_LOG_TAG "HDRShot"

#include "HDRShot.h"

#include <mtkcam/middleware/v1/camshot/_callbacks.h>

#include <mtkcam/middleware/v1/camshot/CamShotUtils.h>

#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/aaa/IIspMgr.h>

#include <mtkcam/utils/metadata/IMetadataConverter.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>

#include <mtkcam/utils/hw/HwInfoHelper.h>
#include <mtkcam/utils/hw/CamManager.h>

#include <mtkcam/middleware/v1/LegacyPipeline/IResourceContainer.h>
#include <mtkcam/middleware/v1/LegacyPipeline/LegacyPipelineBuilder.h>
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/StreamBufferProviderFactory.h>

#include <mtkcam/pipeline/hwnode/HDRNode.h>

#include <mtkcam/feature/hdr/HDRDefs2.h>

#include <cutils/properties.h>

#include "utils/Misc.h"

#define PIPELINED_HDR

//#define FORCE_SINGLE_FRAME

#define POSTVIEW_ENABLED 0

//YUV sensor. HDR_FRAME_COUNT may modified by custom.
//Delay frame queried by 3A.
#define HDR_FRAME_COUNT 2
#define HDR_DELAY_COUNT 3

using namespace android;
using namespace NSCam::Utils;
using namespace NSCam::v1;
using namespace NSCam::v1::NSLegacyPipeline;
using namespace NSCamHW;
using namespace android::NSShot;
using namespace NSCamShot;
using namespace NS3Av3;
using namespace NSCam;
using namespace NSCam::HDR2;

// ---------------------------------------------------------------------------

extern sp<IShot> createInstance_HdrShot(
        char const * const pszShotName,
        uint32_t const u4ShotMode,
        int32_t const  i4OpenId)
{
    sp<IShot> shot;
    sp<HDRShot> hdrShot;

    // new implementator
    hdrShot = new HDRShot(pszShotName, u4ShotMode, i4OpenId);
    if (hdrShot == NULL)
    {
        HDR_LOGE("create HDRShot failed");
        goto lbExit;
    }

    // new interface
    shot = new IShot(hdrShot);
    if (shot == NULL)
    {
        HDR_LOGE("create IShot failed");
        goto lbExit;
    }

lbExit:
    // free all resources if instantiation failed
    if ((shot == 0) && (hdrShot != NULL))
    {
        hdrShot.clear();
    }

    return shot;
}

// ---------------------------------------------------------------------------

MINT32 HDRShot::sSequence = 0;
Mutex HDRShot::sInstanceLock;
MINT32 HDRShot::sInstanceCount = 0;

Mutex HDRShot::sCaptureDoneLock;
sem_t HDRShot::sSemCaptureDone;

sem_t HDRShot::sSemPostProcessingDone;

MtQueue< sp<HDRShot> > HDRShot::sShotQueue;

HDRShot::HDRShot(
        const char *pszShotName,
        uint32_t const u4ShotMode,
        int32_t const i4OpenId)
    : ImpShot(pszShotName, u4ShotMode, i4OpenId),
      mDataMsgSet(0)
{
    mSequence = sSequence++;
    sSequence = (sSequence >= 32) ? 0 : sSequence;

    // init semaphore
    ::memset(&mSemCaptureDone, 0x00, sizeof( sem_t ));
    sem_init(&mSemCaptureDone, 0, 0);

    {
        Mutex::Autolock _l(sInstanceLock);
        if (sInstanceCount == 0)
        {
            // enable performance mode
            mPerf = std::shared_ptr<IPerf>(HDRPerfFactory::getIPerf());

            Mutex::Autolock _l(sCaptureDoneLock);
            sem_init(&sSemCaptureDone, 0, 1);
            sem_init(&sSemPostProcessingDone, 0, 1);
            HDR_LOGD("init semaphore");
        }
    }

    // debug property setting
    {
        char value[PROPERTY_VALUE_MAX];

        // log level
        property_get("vendor.mediatek.hdr.log", value, "0");
        mLogLevel = atoi(value);
        HDR_LOGD_IF(mLogLevel > 0, "log level is set to %d", mLogLevel);
    }

    // lambda to create a CpuCtrl instance, and initialize it
    auto createCputCtrl = []()->CpuCtrl*
    {
        auto c = CpuCtrl::createInstance();
        if ( __builtin_expect( c == nullptr, false )) {
            CAM_LOGW("create CpuCtrl failed");
            return nullptr;
        }

        // init CpuCtrl
        if ( ! c->init() ) {
            c->destroyInstance();
            CAM_LOGW("init CpuCtrl failed");
            return nullptr;
        }

        return c;
    };

    // lambda to delete the CpuCtrl instance
    auto cpuCtrlDestuctor = [](CpuCtrl* c)->void
    {
        if ( __builtin_expect( c != nullptr, true )) {
            c->uninit();
            c->destroyInstance();
        }
    };

    // create CpuCtrl
    mCpuCtrl = decltype(mCpuCtrl)(
            createCputCtrl(),
            cpuCtrlDestuctor
            );
}

HDRShot::~HDRShot()
{
    if (mpPipeline.get())
        mpPipeline->flush();

    // destroy semaphore
    sem_destroy(&mSemCaptureDone);
    {
        Mutex::Autolock _l(sInstanceLock);
        if (sInstanceCount == 0)
        {
            // disable performance mode
            mPerf.reset();

            Mutex::Autolock _l(sCaptureDoneLock);
            sem_destroy(&sSemCaptureDone);
            sem_destroy(&sSemPostProcessingDone);
            HDR_LOGD("destroy semaphore");
        }
    }
}

// metadata event handler
void HDRShot::onResultReceived(
        MUINT32    const requestNo,
        StreamId_T const streamId,
        MBOOL      const errorResult,
        IMetadata  const /*result*/)
{
    HDR_TRACE_CALL();

    HDR_LOGD("metadata request(%d) streamID(%#" PRIx64 ")", requestNo, streamId);

    if (errorResult)
    {
        HDR_LOGE("the result metadata may be wrong...");
    }

    switch (streamId)
    {
        case eSTREAMID_META_HAL_DYNAMIC_P1:
            // called as near as possible to the moment when
            // a photo is captured from the sensor
            mpShotCallback->onCB_Shutter(true, 0, mu4ShotMode);
            HDR_LOGD("send shutter callback");
#ifdef PIPELINED_HDR
            if (eShotMode_ZsdHdrShot == mu4ShotMode)
            {
                sem_post(&sSemCaptureDone);

                Mutex::Autolock _l(sInstanceLock);
                if (sInstanceCount <= 1)
                {
                    // the previous round has finished
                    // notify application to enable the shutter button
                    // and unlock this round
                    HDR_TRACE_NAME("Capture_done");
                    mpShotCallback->onCB_P2done();
                    HDR_LOGD("send P2Done callback");
                }
            }
#endif
            break;

        case eSTREAMID_META_APP_DYNAMIC_HDR:
#ifdef PIPELINED_HDR
            if (eShotMode_ZsdHdrShot == mu4ShotMode)
            {
                sem_post(&sSemPostProcessingDone);

                Mutex::Autolock _l(sInstanceLock);
                if (sInstanceCount > 1)
                {
                    // the previous round has finished
                    // notify application to enable the shutter button
                    // and unlock this round
                    HDR_TRACE_NAME("HDR_done");
                    mpShotCallback->onCB_P2done();
                    HDR_LOGD("send P2Done callback");
                }
            }
#endif
            break;

        case eSTREAMID_META_HAL_DYNAMIC_HDR:
        case eSTREAMID_META_APP_DYNAMIC_JPEG:
        case eSTREAMID_META_APP_FULL:
        case eSTREAMID_META_HAL_FULL:
        case eSTREAMID_META_APP_DYNAMIC_P1:
            break;

        default:
            HDR_LOGW("unsupported streamID(%#" PRIx64 ")", streamId);
    }
}

// image event handler
MERROR HDRShot::onResultReceived(
        MUINT32    const requestNo,
        StreamId_T const streamId,
        MBOOL      const errorBuffer,
        android::sp<IImageBuffer>& pBuffer)
{
    HDR_TRACE_CALL();

    HDR_LOGD("image request(%d) streamID(%#" PRIx64 ")", requestNo, streamId);

    if (pBuffer == NULL)
    {
        HDR_LOGE("buffer is NULL");
        return BAD_VALUE;
    }

    HDR_LOGE_IF(errorBuffer, "the content of buffer may not correct...");

    switch (streamId)
    {
        case eSTREAMID_IMAGE_PIPE_RAW_OPAQUE:
            if (eShotMode_ZsdHdrShot == mu4ShotMode)
            {
                /**
                 *  When HDRShot got the first RAW callback which means HDR
                 *  doesn't need these RAW buffers anymore. Hence we return
                 *  these RAW buffers back to pool ASAP
                 */
                if (requestNo <= 0)
                    returnSelectedBuffers();
            }
#ifndef PIPELINED_HDR
            sem_post(&sSemCaptureDone);
#endif
            break;

        case eSTREAMID_IMAGE_HDR:
            // dummy raw callback
            mpShotCallback->onCB_RawImage(0, 0, NULL);

            if (CC_UNLIKELY(pBuffer == NULL))
            {
                // send fake jpeg callback
                onFakeJpegCallback(mpShotCallback);
            }
            else
            {
                // set callback index to 1 and the final image flag to true
                pBuffer->lockBuf(LOG_TAG, eBUFFER_USAGE_SW_READ_MASK);
                {
                    const uint32_t u4Size = pBuffer->getBitstreamSize();
                    const uint8_t *puBuf  = (uint8_t const*)pBuffer->getBufVA(0);
                    mpShotCallback->onCB_CompressedImage_packed(
                            0, u4Size, puBuf, 0, true, MTK_CAMERA_MSG_EXT_DATA_COMPRESSED_IMAGE, getShotMode());
                }
                pBuffer->unlockBuf(LOG_TAG);
            }
            break;

#if POSTVIEW_ENABLED
        case eSTREAMID_IMAGE_PIPE_YUV_00:
            if (pBuffer.get())
            {
                pBuffer->lockBuf(LOG_TAG, eBUFFER_USAGE_SW_READ_MASK);
                if (property_get_int32("mediatek.hdr.dump.postview", -1) > 0)
                {
                    char filename[64];
                    MSize const &imgSize = pBuffer->getImgSize();
                    sprintf(filename, "/sdcard/hdr-postview.%dx%d.yv12", imgSize.w, imgSize.h);
                    pBuffer->saveToFile(filename);
                }
                mpShotCallback->onCB_PostviewClient(0, pBuffer.get());
                pBuffer->unlockBuf(LOG_TAG);
            }
            else
            {
                HDR_LOGE("skip postview callback, because NULL buffer!!")
            }
            break;
#endif

        case eSTREAMID_IMAGE_PIPE_YUV_JPEG:
        case eSTREAMID_IMAGE_PIPE_RAW_LCSO:
            break;

        default:
            HDR_LOGW("unsupported streamID(%#" PRIx64 ")", streamId);
    }

    return OK;
}

String8 HDRShot::getUserName()
{
    HDR_TRACE_CALL();

    return String8(DEBUG_LOG_TAG);
}

bool HDRShot::sendCommand(
        uint32_t const cmd,
        MUINTPTR const arg1,
        uint32_t const arg2,
        uint32_t const /*arg3*/)
{
    HDR_TRACE_CALL();

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

bool HDRShot::onCmd_reset()
{
    HDR_TRACE_CALL();

    return true;
}

bool HDRShot::onCmd_capture()
{
    FUNCTION_LOG_START;

    HDR_LOGD("[%d] %s", mSequence, __FUNCTION__);

    HDR_TRACE_CALL();

    // dump metadata
    if (CC_UNLIKELY(mLogLevel > 0))
        dumpMetadata(mShotParam.mAppSetting, "app");

    bool bCancel = mStateMgr.doWork<bool>(
            [this](ShotState& state, void* /*arg*/) -> bool {
                if (state == SHOTSTATE_CANCEL) {
                    sem_post(&mSemCaptureDone);
                    return true;
                }
                else {
                    state = SHOTSTATE_CAPTURE;
                    sShotQueue.enque(this);
                    return false;
                }
            });

    if (bCancel)
    {
        HDR_LOGD("canceled capture");
        return true;
    }

    // boost CPU for a second
    if (mCpuCtrl.get()) {
        mCpuCtrl->cpuPerformanceMode(3);
    }

    // update post nr settings
    updatePostNRSetting(MTK_NR_MODE_AUTO, false, &mShotParam.mHalSetting);

#if POSTVIEW_ENABLED
    if (mShotParam.miPostviewClientFormat != eImgFmt_UNKNOWN)
        enableDataMsg(NSCamShot::ECamShot_DATA_MSG_POSTVIEW);
#endif

#ifndef PIPELINED_HDR
    // need to wait for previous capture done
    // before start this round
    {
        HDR_TRACE_NAME("wait_cap_done");
        HDR_LOGD("wait for previous capture done...");
        sem_wait(&sSemCaptureDone);
        HDR_LOGD("wait previous capture done");
    }
#endif

    String8 name;
    name.appendFormat("Cam@HDRWorker_%d", mSequence);
    sp<WorkerThread> workerThread =
        new WorkerThread(name.string(), mu4ShotMode, this);
    workerThread->run(name.string());

    // for normal mode, we need to wait capture done before starting preview
    if (eShotMode_HdrShot == mu4ShotMode)
        workerThread->join();

    FUNCTION_LOG_END_MUM;

    return true;
}

void HDRShot::onCmd_cancel()
{
    HDR_LOGD("%s: +", __FUNCTION__);
    HDR_TRACE_CALL();

    /**
     *  To tell all the HDRShot instances in queue, excludes self, to cancel
     *  using async calls and sync all async calls while existing this scope.
     */
    std::vector< std::future<void> > future_teller;

    sShotQueue.getMutex().lock();
    {
        std::deque< sp<HDRShot> > shots = std::move(sShotQueue.getQueue());
        for (size_t i = 0; i < shots.size(); i++) {
            sp<HDRShot> s = shots[i];
            if (s.get() != this) {
                auto f = std::async(std::launch::async, [s]()->void{
                    s->sendCommand(eCmd_cancel, 0, 0, 0);
                });
                future_teller.push_back(std::move(f));
            }
        }
    }
    sShotQueue.getMutex().unlock();

    /* checking current state and update state to cancel */
    ShotState currentState = mStateMgr.doWork<ShotState>(
            [this](ShotState& state, void* /*arg*/) {
                ShotState currState = state;
                state = SHOTSTATE_CANCEL;
                /* wait capture done semaphore signaled */
                return currState;
            });

    /* wait catpure done */
    if (currentState == SHOTSTATE_CAPTURE) {
        HDR_LOGD("%s: wait capture semaphore signaled", __FUNCTION__);
        sem_wait(&mSemCaptureDone);
    }

    if(currentState != SHOTSTATE_DONE) {
        onFakeJpegCallback(mpShotCallback);
    }

    HDR_LOGD("%s: -", __FUNCTION__);
}

static void getHDRYuvEv(std::vector<MINT32> &vEvParam, std::vector<MINT32> *vEvOrgParam)
{
    int frameCount = HDR_FRAME_COUNT;
    int delayCount = HDR_DELAY_COUNT;
    int i;

    MINT32 tmp_ev = -1;

    vEvParam.resize(frameCount);

    //EV:-1,1
    for(i = 0; i < frameCount; i++)
    {
        vEvParam.at(i) = tmp_ev;
        tmp_ev += 2;
    }

    //ZSD mode
    if(vEvOrgParam)
    {
        tmp_ev = 0;
        vEvOrgParam->resize(delayCount);

        for(i = 0; i < delayCount; i++)
            vEvOrgParam->at(i) = tmp_ev;
    }

    return;
}

bool HDRShot::normalCapture()
{
    FUNCTION_LOG_START;

    HDR_TRACE_CALL();

    bool ret = true;

    /* monitor this scope, if exited, mark as capture done */
    ScopeWorker ___scope_worker([this](void* /*arg*/) {
        mStateMgr.updateState(SHOTSTATE_DONE);
        sShotQueue.deque();
        sem_post(&mSemCaptureDone);
    });

    // before operation, check state first, if it's cancel, no need to capture
    // anymore
    if (mStateMgr.getState() == SHOTSTATE_CANCEL) {
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

        mDataMsgSet = ECamShot_DATA_MSG_JPEG;

#if POSTVIEW_ENABLED
        if (isDataMsgEnabled(EIShot_DATA_MSG_POSTVIEW) ||
            property_get_int32("debug.shot.force.postview", -1) > 0)
        {
            mDataMsgSet |= ECamShot_DATA_MSG_POSTVIEW;
        }
#endif
    }

    // 2. create streams
    if (createStreams() != OK)
    {
        HDR_LOGE("create streams failed");
        return false;
    }

    // 3. create pipeline
    sp<CamManager::UsingDeviceHelper> spDeviceHelper = new CamManager::UsingDeviceHelper(mSensorParam.u4OpenID);
    if (createPipeline() != OK)
    {
        HDR_LOGE("create pipeline failed");
        onFakeJpegCallback(mpShotCallback);
        return false;
    }
    spDeviceHelper->configDone();

    //Get sensor type
    HwInfoHelper helper(getOpenId());
    if (!helper.updateInfos())
    {
        HDR_LOGE("cannot helper update");
        onFakeJpegCallback(mpShotCallback);
        return false;
    }

    // 4. prepare app and hal metadata
    Vector<SettingSet> vSettings;
    {
        std::vector<HDRCaptureParam> vCaptureParams;
        //For YUV sensor
        std::vector<MINT32> vEvParam;

        // get HDR capture parameters
        HDRNode::getCaptureParamSet(getOpenId(), vCaptureParams);

        if (helper.isYuv())
        {
            getHDRYuvEv(vEvParam, NULL);
            vSettings.resize(vEvParam.size());
        }
        else
        {
            vSettings.resize(vCaptureParams.size());
        }

        // set parameters into metadata
        for (size_t i = 0; i < vSettings.size(); i++)
        {
            const HDRCaptureParam& captureParam(vCaptureParams.at(i));

            IMetadata::Memory capParams;
            capParams.resize(sizeof(CaptureParam_T));
            memcpy(capParams.editArray(),
                    &captureParam.exposureParam, sizeof(CaptureParam_T));

            vSettings.editItemAt(i).appSetting = mShotParam.mAppSetting;
            vSettings.editItemAt(i).halSetting = mShotParam.mHalSetting;

            // modify hal control metadata
            IMetadata &halSetting(vSettings.editItemAt(i).halSetting);
            IMetadata &appSetting(vSettings.editItemAt(i).appSetting);
            {
                if (helper.isYuv())
                {
                    //YUV sensor use android defined metadata to indicate HAL3A ae compensation
                    IMetadata::setEntry<MINT32>(
                            &appSetting, MTK_CONTROL_AE_EXPOSURE_COMPENSATION, vEvParam.at(i));
                }
                else
                {
                    IMetadata::setEntry<IMetadata::Memory>(
                            &halSetting, MTK_3A_AE_CAP_PARAM, capParams);
                }
                IMetadata::setEntry<MSize>(
                        &halSetting, MTK_HAL_REQUEST_SENSOR_SIZE, mSensorSetting.size);
                // pause AF for (N - 1) frames and resume for the last frame
                IMetadata::setEntry<MUINT8>(
                        &halSetting, MTK_FOCUS_PAUSE,
                        (i + 1) == vSettings.size() ? 0 : 1);
                IMetadata::setEntry<MUINT8>(
                        &halSetting, MTK_HAL_REQUEST_REQUIRE_EXIF, 1);
                IMetadata::setEntry<MUINT8>(
                        &halSetting, MTK_HAL_REQUEST_DUMP_EXIF, 1);

                // fall back to single capture if HDR only needs single capture frame
                if (CC_UNLIKELY(isSingleCapture(vSettings.size())))
                {
                    IMetadata::setEntry<MUINT8>(
                            &halSetting, MTK_HAL_REQUEST_ERROR_FRAME, 1);
                    MINT32 bypassLTM = 1;
                    IMetadata::setEntry<MINT32>(
                            &halSetting, MTK_3A_ISP_BYPASS_LCE, bypassLTM);
                }
                else
                {
                    // configure LTM per HDR input frame
                    IMetadata::setEntry<MINT32>(
                            &halSetting, MTK_3A_ISP_BYPASS_LCE,
                            captureParam.bypassLTM());
                }
            }
        }
    }

    // 5. submit setting to pipeline
    for (size_t i = 0; i < vSettings.size(); i++)
    {
        if (mpPipeline->submitSetting(
                    i,
                    vSettings.editItemAt(i).appSetting,
                    vSettings.editItemAt(i).halSetting) != OK)
        {
            HDR_LOGE("submit setting failed");
            return false;
        }
    }

    // 6. soldier on until pipeline has finished its job
    if (mpPipeline->waitUntilDrainedAndFlush() != OK)
    {
        HDR_LOGE("drained failed");
        return false;
    }

    if (releaseResource() != OK)
    {
        HDR_LOGE("release resource failed");
        return false;
    }
    spDeviceHelper = NULL;

    FUNCTION_LOG_END;

    return ret;
}

bool HDRShot::zsdCapture()
{
    FUNCTION_LOG_START;

    HDR_TRACE_CALL();

    bool ret = true;

    status_t err = OK;

    /* monitor this scope, if exited, mark as capture done */
    ScopeWorker ___scope_worker([this](void* /*arg*/) {
        mStateMgr.updateState(SHOTSTATE_DONE);
        sShotQueue.deque();
        sem_post(&mSemCaptureDone);
    });

    // before operation, check state first, if it's cancel, no need to capture
    // anymore
    if (mStateMgr.getState() == SHOTSTATE_CANCEL) {
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
            HDR_LOGE("can't find StreamBufferProvider in ConsumerContainer");
            return false;
        }

        // set the expected callback message type
        mDataMsgSet = ECamShot_DATA_MSG_JPEG;

#if POSTVIEW_ENABLED
        if (isDataMsgEnabled(EIShot_DATA_MSG_POSTVIEW) ||
            property_get_int32("debug.shot.force.postview", -1) > 0)
        {
            mDataMsgSet |= ECamShot_DATA_MSG_POSTVIEW;
        }
#endif
    }

    // 2. create streams
    if (createStreams(mpBufferProvider) != OK)
    {
        HDR_LOGE("create streams failed");
        return false;
    }

#ifdef PIPELINED_HDR
    // need to wait for previous capture done
    // before start this round
    {
        HDR_TRACE_NAME("wait_cap_done");
        HDR_LOGD("wait for previous capture done...");
        sem_wait(&sSemCaptureDone);
        HDR_LOGD("wait previous capture done");
    }
#endif

    // 3. create pipeline
    if (createPipeline(mpBufferProvider) != OK)
    {
        HDR_LOGE("create pipeline failed");
#ifdef PIPELINED_HDR
        // release capture lock to
        // ensure the next capture would not be blocked
        sem_post(&sSemCaptureDone);
#endif
        onFakeJpegCallback(mpShotCallback);
        return false;
    }

    // 4. prepare app and hal metadata
    Vector<SettingSet> vSettings;
    Vector<MINT32>     vCaptureRequest;
    size_t delayedFrames = 0;

     //Get sensor type
    HwInfoHelper helper(getOpenId());
    if ( ! helper.updateInfos() )
    {
        HDR_LOGE("cannot helper update");
        sem_post(&sSemCaptureDone);
        onFakeJpegCallback(mpShotCallback);
        return false;
    }

    {
        std::vector<HDRCaptureParam> vCaptureParams;
        std::vector<HDRCaptureParam> vOrigCaptureParams;
        //For YUV sensor
        std::vector<MINT32> vEvParam;
        std::vector<MINT32> vOrgEvParam;

        // get HDR capture parameters
        HDRNode::getCaptureParamSet(
                getOpenId(), vCaptureParams, &vOrigCaptureParams);

        // keep the amount of delayed frames for later use
        delayedFrames = vOrigCaptureParams.size();

        // resize to the amount of HDR capture parameters
        vCaptureRequest.resize(vCaptureParams.size());

        if (helper.isYuv())
        {
            getHDRYuvEv(vEvParam, &vOrgEvParam);
            vEvParam.insert(vEvParam.end(),
                    std::make_move_iterator(vOrgEvParam.begin()),
                    std::make_move_iterator(vOrgEvParam.end()));
            vSettings.resize(vEvParam.size());
        }
        else
        {
            // then append the amount of the original capture parameters
            // at the end of HDR capture parameters as a series of delayed frames
            //
            // NOTE: the frame delayed time is only needed under ZSD mode
            //
            // even though capture can leverage per-frame control, however,
            // to avoid unstable exposure variation is seen on the display,
            // extra frame dealyed time is needed for AE convergence
            vCaptureParams.insert(vCaptureParams.end(),
                    std::make_move_iterator(vOrigCaptureParams.begin()),
                    std::make_move_iterator(vOrigCaptureParams.end()));

            // finally, the amount of settings should be a total of
            // the following parameters:
            // 1. HDR capture parameters
            // 2. the original capture parameters
            vSettings.resize(vCaptureParams.size());
        }

        // set parameters into metadata
        for (size_t i = 0; i < vSettings.size(); i++)
        {
            const HDRCaptureParam& captureParam(vCaptureParams.at(i));

            IMetadata::Memory capParams;
            capParams.resize(sizeof(CaptureParam_T));
            memcpy(capParams.editArray(),
                    &captureParam.exposureParam, sizeof(CaptureParam_T));

            vSettings.editItemAt(i).appSetting = mShotParam.mAppSetting;
            vSettings.editItemAt(i).halSetting = mShotParam.mHalSetting;

            // modify hal control metadata
            IMetadata &halSetting(vSettings.editItemAt(i).halSetting);
            IMetadata &appSetting(vSettings.editItemAt(i).appSetting);
            {
                if (helper.isYuv())
                {
                    //YUV sensor use android defined metadata to indicate HAL3A ae compensation
                    IMetadata::setEntry<MINT32>(
                            &appSetting, MTK_CONTROL_AE_EXPOSURE_COMPENSATION, vEvParam.at(i));
                }
                else
                {
                    IMetadata::setEntry<IMetadata::Memory>(
                            &halSetting, MTK_3A_AE_CAP_PARAM, capParams);
                }
                IMetadata::setEntry<MSize>(
                        &halSetting, MTK_HAL_REQUEST_SENSOR_SIZE, mSensorSetting.size);
                // pause AF for (N - 1) frames and resume for the last frame
                IMetadata::setEntry<MUINT8>(
                        &halSetting, MTK_FOCUS_PAUSE,
                        (i + 1) == vSettings.size() ? 0 : 1);
                IMetadata::setEntry<MUINT8>(
                        &halSetting, MTK_HAL_REQUEST_REQUIRE_EXIF, 1);
                IMetadata::setEntry<MUINT8>(
                        &halSetting, MTK_HAL_REQUEST_DUMP_EXIF, 1);

                // fall back to single capture if HDR only needs single capture frame
                if (CC_UNLIKELY(isSingleCapture(vSettings.size(), delayedFrames)))
                {
                    IMetadata::setEntry<MUINT8>(
                            &halSetting, MTK_HAL_REQUEST_ERROR_FRAME, 1);
                    MINT32 bypassLTM = 1;
                    IMetadata::setEntry<MINT32>(
                            &halSetting, MTK_3A_ISP_BYPASS_LCE, bypassLTM);
                }
                else
                {
                    // configure LTM per HDR input frame
                    IMetadata::setEntry<MINT32>(
                            &halSetting, MTK_3A_ISP_BYPASS_LCE,
                            captureParam.bypassLTM());
                }
            }

            if (i < vCaptureRequest.size())
            {
                // use capture count as capture request number
                vCaptureRequest.editItemAt(i) = i;
            }
        }
    }

    // 5. submit settings to flow control
    sp<ISelector> oriSelector;
    sp<MfcSelector> selector = new MfcSelector();

    // backup the original selector
    oriSelector = mpBufferProvider->querySelector();
    if (oriSelector.get() == NULL)
    {
        HDR_LOGE("the original selector is NULL");
    }

    Vector<MINT32> resultRequestNo;
    {
        // set HDR selector into stream buffer provider
        status_t status = mpBufferProvider->switchSelector(selector);
        if (status != OK)
        {
            HDR_LOGE("change to HDR selector failed");
            return false;
        }
        HDR_LOGD("change to HDR selector");

        // send capture requests to flow control
        sp<IFeatureFlowControl> previewFlowCtrl =
            IResourceContainer::getInstance(getOpenId())->queryFeatureFlowControl();
        if (previewFlowCtrl == NULL)
        {
            HDR_LOGE("previewFlowCtrl is NULL");
            return false;
        }

        // to reduce in-flight requests within the pipeline,
        // pause preview thread to send request to pipeline
        previewFlowCtrl->pausePreview(true);

        // only output full and statistics buffers
        Vector<sp<IImageStreamInfo>> rawInputInfos;
        selector->queryCollectImageStreamInfo(rawInputInfos, MTRUE);
        BufferList vDstStreams;
        for (size_t i = 0; i < rawInputInfos.size(); i++)
        {
            const StreamId_T streamID(rawInputInfos[i]->getStreamId());

            HDR_LOGE_IF(CC_UNLIKELY(streamID == eSTREAMID_IMAGE_PIPE_RAW_RESIZER),
                    "resized raw if found");

            vDstStreams.push_back(BufferSet { streamID, MTRUE });
        }

        if (previewFlowCtrl->submitRequest(
                    vSettings, vDstStreams, resultRequestNo) != OK)
        {
            HDR_LOGE("submit settings failed");
            return false;
        }

        // resume preview thread to send request to pipeline
        previewFlowCtrl->resumePreview();

        // exclude delayed frames
        resultRequestNo.removeItemsAt(
                resultRequestNo.size() - delayedFrames, delayedFrames);

        // set expected request numbers
        selector->setWaitRequestNo(resultRequestNo);
    }

#ifdef PIPELINED_HDR
    // need to wait for previous HDR post-processing done
    // before start this round
    {
        HDR_TRACE_NAME("wait_HDR_done");
        HDR_LOGD("wait for previous HDR done...");
        sem_wait(&sSemPostProcessingDone);
        HDR_LOGD("wait previous HDR done");
    }
#endif

    // 6. get capture results from selector,
    //    pass app result metadata to timestamp processor
    Vector<SettingSet> vCapSettings;
    MBOOL useSingleFrame = MFALSE;
    for (size_t i = 0; i < vCaptureRequest.size(); i++)
    {
        MINT32 requestNo;
        IMetadata appSetting;
        IMetadata halSetting;

        // blocking call to get captured result
        err = getSelectResult(
                selector, requestNo, appSetting, halSetting);

        // TODO: check if LTM configration is sent from flow control
        {
            MINT32 bypassLTM = 0;
            IMetadata::getEntry<MINT32>(
                    &halSetting, MTK_3A_ISP_BYPASS_LCE,
                    bypassLTM);
            HDR_LOGD("zsdCapture bypassLTM[%zu](%d)", i, bypassLTM);
        }

#ifdef FORCE_SINGLE_FRAME
        err = BAD_VALUE;
#endif
        if (CC_UNLIKELY(OK != err))
        {
            HDR_LOGW("get selected result failed(%s), fall back to single capture",
                    strerror(-err));

            // HDR input frame has been dropped,
            // we need to fall back to single capture
            selector->setWaitRequestNo(MfcSelector::REQUEST_NO_ANY);

            // flush previous selected buffers and settings
            returnSelectedBuffers();
            for (size_t i = 0 ; i < mP1PoolMap.size(); i++)
                mP1PoolMap.editValueAt(i)->flush();
            vCapSettings.clear();

            // get 0EV capture frame, and the current timestamp
            const MINT32 zeroEVRequestNo = resultRequestNo.top() + delayedFrames + 1;
            nsecs_t zeroEVRequestNoTS = ::systemTime(CLOCK_MONOTONIC);

            while (MTRUE)
            {
                MINT32 requestNo;
                err = getSelectResult(selector, requestNo, appSetting, halSetting);
                if (err != OK)
                {
                    if (err == WOULD_BLOCK)
                    {
                        // the preview pipeline has abandoned us,
                        // there is no more result for selector now.
                        //
                        // we have nothing to do but give up this capture request
                        HDR_LOGW("no more result cab be received from selector, " \
                                "give up this capture request");

#ifdef PIPELINED_HDR
                        // release capture and post-processing lock to
                        // ensure the next capture would not be blocked
                        sem_post(&sSemCaptureDone);
                        sem_post(&sSemPostProcessingDone);
#endif
                        onFakeJpegCallback(mpShotCallback);
                        break;
                    }

                    // the frame may be dropped, try again
                    continue;
                }

                auto diffMs = ::ns2ms(::systemTime(CLOCK_MONOTONIC) - zeroEVRequestNoTS);

                // need to handle overflow case of requestNo.
                // requestNo range is [0, N], (N maybe 1000 or INT_MAX).
                // Hence if we've already wait 500 ms, use anyframe as output.
                if (requestNo < zeroEVRequestNo && diffMs <= 500)
                {
                    // pass app result metadata to timestamp processor
                    onSelectedResultReceived(mpPipeline, appSetting);

                    returnSelectedBuffers();
                    for (size_t i = 0 ; i < mP1PoolMap.size(); i++)
                        mP1PoolMap.editValueAt(i)->flush();
                    continue;
                }

                // stop select arbitrary result
                selector->clearWaitRequestNo();

                // indicate that this request is used for error handling
                IMetadata::setEntry<MUINT8>(
                        &halSetting, MTK_HAL_REQUEST_ERROR_FRAME, 1);

                break;
            }

            if (err == OK)
                useSingleFrame = MTRUE;
        }

        if (err == OK)
        {
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
                    HDR_LOGE("update MTK_JPEG_THUMBNAIL_SIZE failed");
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
                    HDR_LOGE("update MTK_JPEG_ORIENTATION failed");
                }
            }

            vCapSettings.push_back(SettingSet{ appSetting, halSetting });

            // pass app result metadata to timestamp processor
            onSelectedResultReceived(mpPipeline, appSetting);
        }

        // stop selecting result if using single frame or error occurs
        if (useSingleFrame || (err != OK))
            break;
    }

    // NOTE: Due to HDR detection mechanism, it is possible that
    // HDRShot and ZSDShot share the same selector.
    //
    // To avoid no incoming buffers after switching to ZSDShot,
    // we need to configure the selector to keep the latest buffer received once
    // receiving all capture results in HDRShot.
    selector->setWaitRequestNo(MfcSelector::REQUEST_NO_ANY);

    // NOTE: Due to HDR detection mechanism, it is possible that
    // HDRShot and ZSDShot share the same selector.
    //
    // To avoid no incoming buffers after switching to ZSDShot,
    // we need to configure the selector to keep the latest buffer received once
    // receiving all capture results in HDRShot.
    selector->setWaitRequestNo(MfcSelector::REQUEST_NO_ANY);

    // 7. submit setting to pipeline
    if (err == OK)
    {
        if (CC_LIKELY(!useSingleFrame))
        {
            for (size_t i = 0; i < vCaptureRequest.size(); i++)
            {
                IMetadata& appSetting(vCapSettings.editItemAt(i).appSetting);
                IMetadata& halSetting(vCapSettings.editItemAt(i).halSetting);

                // submit request to pipeline
                if (mpPipeline->submitSetting(
                            vCaptureRequest[i], appSetting, halSetting) != OK)
                {
                    HDR_LOGE("submit setting failed");
#ifdef PIPELINED_HDR
                    // release post-processing lock to
                    // ensure the next capture would not be blocked
                    sem_post(&sSemPostProcessingDone);
#endif
                    return false;
                }

                if ((i + 1) == vCaptureRequest.size())
                {
                    // we have received all input frames now,
                    // send metadata event for shutter callback
                    onResultReceived(
                            vCaptureRequest[i],
                            eSTREAMID_META_HAL_DYNAMIC_P1,
                            MFALSE,
                            halSetting);
                }
            }
        }
        else
        {
            const MINT32 requestNo = 0;
            IMetadata& appSetting(vCapSettings.editTop().appSetting);
            IMetadata& halSetting(vCapSettings.editTop().halSetting);

            // submit request to pipeline
            if (mpPipeline->submitSetting(requestNo, appSetting, halSetting) != OK)
            {
                HDR_LOGE("submit setting failed");
#ifdef PIPELINED_HDR
                // release post-processing lock to
                // ensure the next capture would not be blocked
                sem_post(&sSemPostProcessingDone);
#endif
                return false;
            }

            // we have received all input frames now,
            // send metadata event for shutter callback
            onResultReceived(
                    requestNo,
                    eSTREAMID_META_HAL_DYNAMIC_P1,
                    MFALSE,
                    halSetting);
        }
    }
    vCaptureRequest.clear();

    // after frames captured, set selector back to the original one
    mpBufferProvider->switchSelector(oriSelector);

    // 8. soldier on until pipeline has finished its job
    if (mpPipeline->waitUntilDrained() != OK)
    {
        HDR_LOGE("drained failed");
#ifdef PIPELINED_HDR
        // release post-processing lock to
        // ensure the next capture would not be blocked
        sem_post(&sSemPostProcessingDone);
#endif
        return false;
    }

    if (releaseResource() != OK)
    {
        HDR_LOGE("release resource failed");
        return false;
    }

    FUNCTION_LOG_END;

    return ret;
}

void HDRShot::onCaptureBegin()
{
    Mutex::Autolock _l(sInstanceLock);
    sInstanceCount++;

    HDR_LOGD("instanceCount(%d) +", sInstanceCount);
    HDR_TRACE_INT("InFlight(hdr)", sInstanceCount);
}

void HDRShot::onCaptureEnd()
{
    Mutex::Autolock _l(sInstanceLock);
    sInstanceCount--;

    HDR_LOGD("instanceCount(%d) -", sInstanceCount);
    HDR_TRACE_INT("InFlight(hdr)", sInstanceCount);
}

MUINT32 HDRShot::getRotation() const
{
    return mShotParam.mu4Transform;
}

status_t HDRShot::createStreams(const sp<StreamBufferProvider>& provider)
{
    FUNCTION_LOG_START;

    HDR_TRACE_CALL();

    MUINT32 const openId     = mSensorParam.u4OpenID;
    MUINT32 const sensorMode = mSensorParam.u4Scenario;
    MUINT32 const bitDepth   = mSensorParam.u4Bitdepth;
    MSize const previewsize   = MSize(mShotParam.mi4PostviewWidth, mShotParam.mi4PostviewHeight);
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
    MBOOL const openLCS =
        (getLcsStatus() == LCSSTATUS_ENABLED) ? MTRUE : MFALSE;

    HwInfoHelper helper(openId);
    if (!helper.updateInfos())
    {
        HDR_LOGE("update sensor static information failed");
        return BAD_VALUE;
    }

    if (!helper.getSensorSize(sensorMode, mSensorSetting.size) ||
        !helper.getSensorFps(sensorMode, mSensorSetting.fps) ||
        !helper.queryPixelMode(
            sensorMode, mSensorSetting.fps, mSensorSetting.pixelMode))
    {
        HDR_LOGE("cannot get params about sensor");
        return BAD_VALUE;
    }

    if (provider == NULL)
    {
        HDR_LOGD("normal mode");

        // full raw
        {
            MSize size = mSensorSetting.size;
            MINT format;
            size_t stride;
            MUINT const usage = 0; // not necessary here
            MBOOL useUFOfmt = MTRUE;
            if (getShotMode() == eShotMode_EngShot)
            {
                useUFOfmt = MFALSE;
            }

            if (!helper.getImgoFmt(bitDepth, format, useUFOfmt) ||
                !helper.alignPass1HwLimitation(
                    mSensorSetting.pixelMode, format, true, size, stride))
            {
                HDR_LOGE("wrong parameter for IMGO");
                return BAD_VALUE;
            }

            mpImageInfo_fullRaw = createRawImageStreamInfo(
                    "HDRShot:FullRaw",
                    eSTREAMID_IMAGE_PIPE_RAW_OPAQUE,
                    eSTREAMTYPE_IMAGE_INOUT,
                    3, 1,
                    usage, format, size, stride);

            if (mpImageInfo_fullRaw == NULL)
            {
                HDR_LOGE("create ImageStreamInfo FullRaw failed");
                return NO_INIT;
            }

            dumpImageStreamInfo(mpImageInfo_fullRaw);
        }

        // Resize Raw
        {
            MSize size;
            MINT format;
            size_t stride;
            MBOOL useUFOfmt = MTRUE;
            MUINT const usage = 0; //not necessary here
            if( ! helper.getRrzoFmt(bitDepth, format, useUFOfmt) ||
                    ! helper.alignRrzoHwLimitation(previewsize, mSensorSetting.size, size) ||
                    ! helper.alignPass1HwLimitation(mSensorSetting.pixelMode, format, false, size, stride) )
            {
                HDR_LOGE("wrong params about imgo");
                return MFALSE;
            }
            //
            sp<IImageStreamInfo>
                pStreamInfo = createRawImageStreamInfo(
                        "HDRShot:Resizedraw",
                        eSTREAMID_IMAGE_PIPE_RAW_RESIZER,
                        eSTREAMTYPE_IMAGE_INOUT,
                        1, 1,
                        usage, format, size, stride
                        );
            if( pStreamInfo == nullptr ) {
                return MFALSE;
            }
            mpImageInfo_ResizedRaw = pStreamInfo;
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
                    "HDRShot:LcsoRaw",
                    eSTREAMID_IMAGE_PIPE_RAW_LCSO,
                    eSTREAMTYPE_IMAGE_INOUT,
                    3, 1,
                    usage, lcsoParam.format, lcsoParam.size, lcsoParam.stride);

            if (mpImageInfo_lcsoRaw == NULL)
            {
                HDR_LOGE("create ImageStreamInfo LcsoRaw failed");
                return NO_INIT;
            }

            dumpImageStreamInfo(mpImageInfo_lcsoRaw);
        }
    }
    else
    {
        HDR_LOGD("ZSD mode");

        // ZSD mode
        // query IImageStreamInfo of raw stream from provider
        Vector<sp<IImageStreamInfo>> rawInputInfos;
        provider->querySelector()->queryCollectImageStreamInfo(rawInputInfos);

        for (size_t i = 0; i < rawInputInfos.size(); i++)
            dumpImageStreamInfo(rawInputInfos[i]);
    }

    // postview
    if (POSTVIEW_ENABLED && (mDataMsgSet & ECamShot_DATA_MSG_POSTVIEW))
    {
        MSize size  = postviewsize;
        MINT format = postviewfmt;
        MUINT const usage = 0; //not necessary here
        MUINT32 transform = 0;

        mpImageInfo_yuvPostview = createImageStreamInfo(
                "HDRShot:Postview",
                eSTREAMID_IMAGE_PIPE_YUV_00,
                eSTREAMTYPE_IMAGE_INOUT,
                1, 1,
                usage, format, size, transform);

        if (mpImageInfo_yuvPostview == NULL)
        {
            HDR_LOGE("create ImageStreamInfo Postview failed");
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
                "HDRShot:YuvJpeg",
                eSTREAMID_IMAGE_PIPE_YUV_JPEG,
                eSTREAMTYPE_IMAGE_INOUT,
                1, 1,
                usage, format, size, transform);

        if (mpImageInfo_yuvJpeg == NULL)
        {
            HDR_LOGE("create ImageStreamInfo YuvJpeg failed");
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
                "HDRShot:YuvThumbnail",
                eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL,
                eSTREAMTYPE_IMAGE_INOUT,
                1, 1,
                usage, format, size, transform);

        if (mpImageInfo_yuvThumbnail == NULL)
        {
            HDR_LOGE("create ImageStreamInfo YuvThumbnail failed");
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
                "HDRShot:Jpeg",
                eSTREAMID_IMAGE_HDR,
                eSTREAMTYPE_IMAGE_INOUT,
                1, 1,
                usage, format, size, transform);

        if (mpImageInfo_jpeg == NULL)
        {
            HDR_LOGE("create ImageStreamInfo Jpeg failed");
            return BAD_VALUE;
        }

        dumpImageStreamInfo(mpImageInfo_jpeg);
    }

    FUNCTION_LOG_END_MUM;

    return OK;
}

status_t HDRShot::createPipeline(const sp<StreamBufferProvider>& provider)
{
    FUNCTION_LOG_START;

    HDR_TRACE_CALL();

    auto shotMode = getShotMode();
    if ((shotMode != eShotMode_HdrShot) && (shotMode != eShotMode_ZsdHdrShot))
    {
        HDR_LOGE("shot mode is not HDRShot, it's %#x", shotMode);
        return BAD_VALUE;
    }

    LegacyPipelineBuilder::ConfigParams LPBConfigParams;
    LPBConfigParams.mode = (shotMode == eShotMode_HdrShot)
        ? LegacyPipelineMode_T::PipelineMode_Hdr
        : LegacyPipelineMode_T::PipelineMode_ZsdHdr;

    LPBConfigParams.enableEIS = MFALSE;
    LPBConfigParams.enableLCS =
        (getLcsStatus() == LCSSTATUS_ENABLED) ? MTRUE : MFALSE;

    HwInfoHelper helper(mSensorParam.u4OpenID);
    if (!helper.updateInfos())
    {
        HDR_LOGE("cannot properly update infos");
        return BAD_VALUE;
    }

    LPBConfigParams.enableDualPD =
        helper.getDualPDAFSupported(mSensorParam.u4Scenario) ? MTRUE : MFALSE;

    MUINT32 pipeBit;
    MINT ImgFmt = eImgFmt_BAYER12;
    if (helper.getLpModeSupportBitDepthFormat(ImgFmt, pipeBit))
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

    sp<LegacyPipelineBuilder> builder =
        LegacyPipelineBuilder::createInstance(
                mSensorParam.u4OpenID,
                "HDRShot",
                LPBConfigParams);

    CHECK_OBJECT(builder, BAD_VALUE);

    mpCallbackHandler = new BufferCallbackHandler(mSensorParam.u4OpenID);

    // register image listener
    mpCallbackHandler->setImageCallback(this);

    sp<StreamBufferProviderFactory> bufferProvider =
        StreamBufferProviderFactory::createInstance();

    Vector<PipelineImageParam> vImageParam;

    if (provider == NULL)
    {
        HDR_LOGD("normal mode");

        // normal mode
        //
        {
            sp<IScenarioControl> scenarioCtrl =
                IScenarioControl::create(mSensorParam.u4OpenID);
            if (scenarioCtrl == NULL)
            {
                HDR_LOGE("get scenario control failed");
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

            PipelineImageParam imageParam
            {
                pStreamInfo,
                bufferProvider->create(),
                0
            };
            vImageParam.push_back(imageParam);

            HDR_LOGD("add full raw");
        }

        // resized raw
        if( mpImageInfo_ResizedRaw.get() )
        {
            sp<IImageStreamInfo> pStreamInfo = mpImageInfo_ResizedRaw;
            //
            PipelineImageParam imgParam = {
                .pInfo     = pStreamInfo,
                .pProvider = nullptr,
                .usage     = 0
            };
            vImageParam.push_back(imgParam);

            HDR_LOGD("add Resized raw");
        }
        // lcso raw
        if (mpImageInfo_lcsoRaw.get())
        {
            sp<IImageStreamInfo>& pStreamInfo(mpImageInfo_lcsoRaw);

            PipelineImageParam imageParam
            {
                pStreamInfo,
                nullptr,
                0
            };
            vImageParam.push_back(imageParam);

            HDR_LOGD("add lcso raw");
        }

        PipelineSensorParam sensorParam
        {
            mSensorParam.u4Scenario,
            mSensorParam.u4RawType,
            mSensorSetting.size,
            static_cast<MUINT>(mSensorSetting.fps),
            mSensorSetting.pixelMode,
            SENSOR_VHDR_MODE_NONE
        };

        if (OK != builder->setSrc(sensorParam))
        {
            HDR_LOGE("normal mode - setSrc failed");
            return UNKNOWN_ERROR;
        }
    }
    else
    {
        HDR_LOGD("ZSD mode");

        // ZSD mode
        Vector<sp<IImageStreamInfo>> rawInputInfos;
        provider->querySelector()->queryCollectImageStreamInfo(rawInputInfos);
        Vector<PipelineImageParam> imgSrcParams;

        for (size_t i = 0; i < rawInputInfos.size(); i++)
        {
            const sp<IImageStreamInfo>& pStreamInfo(rawInputInfos[i]);

            sp<CallbackBufferPool> pPool =
                prepareCallbackBufferPool(mpCallbackHandler, pStreamInfo, false);
            bufferProvider->setImageStreamInfo(pStreamInfo);
            bufferProvider->setUsersPool(pPool);

            const StreamId_T rStreamId = pStreamInfo->getStreamId();
            if(mP1PoolMap.indexOfKey(rStreamId) >= 0)
                mP1PoolMap.removeItem(rStreamId);
            mP1PoolMap.add(rStreamId, pPool);

            PipelineImageParam imageParam
            {
                pStreamInfo,
                bufferProvider->create(),
                0
            };

            imgSrcParams.push_back(imageParam);
        }

        if (OK != builder->setSrc(imgSrcParams))
        {
            HDR_LOGE("ZSD mode - setSrc failed");
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

        PipelineImageParam imageParam
        {
            pStreamInfo,
            bufferProvider->create(),
            0
        };
        vImageParam.push_back(imageParam);

        HDR_LOGD("add jpeg");
    }

    // postview
    if (mpImageInfo_yuvPostview.get())
    {
        sp<IImageStreamInfo>& pStreamInfo(mpImageInfo_yuvPostview);

        sp<CallbackBufferPool> cbBufferPool =
            prepareCallbackBufferPool(mpCallbackHandler, pStreamInfo, true);
        bufferProvider->setImageStreamInfo(pStreamInfo);
        bufferProvider->setUsersPool(cbBufferPool);

        PipelineImageParam imageParam
        {
            pStreamInfo,
            bufferProvider->create(),
            0
        };
        vImageParam.push_back(imageParam);

        HDR_LOGD("add postview");
    }

    // thumbnail yuv
    if (mpImageInfo_yuvThumbnail.get())
    {
        sp<IImageStreamInfo>& pStreamInfo(mpImageInfo_yuvThumbnail);

        PipelineImageParam imageParam
        {
            pStreamInfo,
            NULL,
            0
        };
        vImageParam.push_back(imageParam);

        HDR_LOGD("add thumbnail yuv");
    }

    // jpeg
    if (mpImageInfo_jpeg.get())
    {
        sp<IImageStreamInfo>& pStreamInfo(mpImageInfo_jpeg);

        sp<CallbackBufferPool> cbBufferPool =
            prepareCallbackBufferPool(mpCallbackHandler, pStreamInfo, true);
        bufferProvider->setImageStreamInfo(pStreamInfo);
        bufferProvider->setUsersPool(cbBufferPool);

        PipelineImageParam imageParam
        {
            pStreamInfo,
            bufferProvider->create(),
            0
        };
        vImageParam.push_back(imageParam);

        HDR_LOGD("add jpeg yuv");
    }

    HDR_LOGD("image param count(%zu)", vImageParam.size());

    if (OK != builder->setDst(vImageParam))
    {
        HDR_LOGE("setDst failed");
        return UNKNOWN_ERROR;
    }

    mpPipeline = builder->create();
    CHECK_OBJECT(mpPipeline, BAD_VALUE);

    {
        sp<ResultProcessor> resultProcessor =
            mpPipeline->getResultProcessor().promote();
        CHECK_OBJECT(resultProcessor, BAD_VALUE);

        // register partial metadata listener
        resultProcessor->registerListener(0, 0, true, this);
        // register full metadata listener
        resultProcessor->registerListener(0, 0, false, this);
    }

    FUNCTION_LOG_END_MUM;

    return OK;
}

status_t HDRShot::getSelectResult(
        const sp<ISelector>& selector,
        MINT32& requestNo,
        IMetadata& appSetting,
        IMetadata& halSetting)
{
    HDR_TRACE_CALL();

    if (selector == NULL)
    {
        HDR_LOGE("selector is NULL");
        return NO_INIT;
    }

    Vector<ISelector::MetaItemSet> resultMeta;
    Vector<ISelector::BufferItemSet> resultBuffers;
    status_t err = selector->getResult(requestNo, resultMeta, resultBuffers);
    if (err)
    {
        HDR_LOGE("get select result failed(%s)", strerror(-err));
        return err;
    }

    if (CC_UNLIKELY((resultBuffers.size() <= 0) ||
        (resultBuffers[0].id != eSTREAMID_IMAGE_PIPE_RAW_OPAQUE)))
    {
        HDR_LOGE("Selector get input raw buffer failed, bufferSetSize(%zu)",
                resultBuffers.size());
        return UNKNOWN_ERROR;
    }

    HDR_LOGD("getSelectResult(%d)", requestNo);

    for (size_t i = 0; i < resultMeta.size(); i++)
    {
        const ISelector::MetaItemSet& item(resultMeta[i]);

        if (item.id == eSTREAMID_META_APP_DYNAMIC_P1)
        {
            appSetting = item.meta;
            HDR_LOGD("P1 app dynamic found");
        }
        else if (item.id == eSTREAMID_META_HAL_DYNAMIC_P1)
        {
            halSetting = item.meta;
            HDR_LOGD("P1 hal dynamic found");
        }
    }

    if (appSetting.isEmpty() || halSetting.isEmpty())
    {
        HDR_LOGE("either app(%s) or hal(%s) result metadata is empty",
                appSetting.isEmpty() ? "y" : "n",
                halSetting.isEmpty() ? "y" : "n");
        return BAD_VALUE;
    }

    // get and register captured image buffer into callback buffer pool
    for (size_t i = 0; i < resultBuffers.size(); i++)
    {
        sp<IImageBuffer> buffer = resultBuffers[i].heap->createImageBuffer();
        if (buffer == NULL)
        {
            HDR_LOGE("result buffer %zu is NULL", i);
            return BAD_VALUE;
        }

        err = mP1PoolMap.editValueFor(resultBuffers[i].id)->addBuffer(buffer);
    }
    mpSelectedResult.push_back(
            MfcSelector::ResultSet(requestNo, resultMeta, resultBuffers, -1));

    return err;
}

status_t HDRShot::returnSelectedBuffers()
{
    HDR_TRACE_CALL();

    // selected buffers are only available in ZSD mode
    if (eShotMode_ZsdHdrShot != mu4ShotMode)
        return OK;

    // return selected buffers to selector and then restore selector
    if (mpBufferProvider.get())
    {
        // return selected buffers to selector
        sp<ISelector> selector = mpBufferProvider->querySelector();
        if (selector.get())
        {
            for (MfcSelector::ResultSet& resultSet : mpSelectedResult)
            {
                HDR_LOGD("return selected buffer heap(%s)",
                        ISelector::logBufferSets(resultSet.bufferSet).string());

                for (size_t i = 0; i < resultSet.bufferSet.size(); i++)
                    selector->returnBuffer(resultSet.bufferSet.editItemAt(i));
            }
            mpSelectedResult.clear();

            // return all buffers kept in selector to buffer pool
            selector->flush();
        }
    }

    return OK;
}

status_t HDRShot::releaseResource()
{
    HDR_TRACE_CALL();

    HDR_LOGD("[%d] release resource", mSequence);

    // release pipeline
    mpPipeline.clear();

    // NOTE:
    // we call returnSelectedBuffers() again just in case
    // if it did not be executed from onResultReceived()
    if (eShotMode_ZsdHdrShot == mu4ShotMode)
        returnSelectedBuffers();

    mpCallbackHandler.clear();

    if (eShotMode_ZsdHdrShot == mu4ShotMode)
    {
        mpBufferProvider.clear();
        mP1PoolMap.clear();
    }

    return OK;
}

HDRShot::LCSSTATUS HDRShot::getLcsStatus()
{
    return (mShotParam.mbEnableLtm == MTRUE
            ? LCSSTATUS_ENABLED
            : LCSSTATUS_DISABLED
           );
}

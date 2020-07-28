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

#define DEBUG_LOG_TAG "VHDRShot"

#include "VHDRShot.h"

#include <mtkcam/middleware/v1/camshot/_callbacks.h>

#include <mtkcam/middleware/v1/camshot/CamShotUtils.h>

#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/aaa/IIspMgr.h>

#include <mtkcam/utils/metadata/IMetadataConverter.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>

#include <mtkcam/utils/hw/HwInfoHelper.h>
#include <mtkcam/utils/hw/CamManager.h>

#include <mtkcam/utils/std/Time.h>
#include <mtkcam/utils/std/Misc.h>

#include <mtkcam/middleware/v1/LegacyPipeline/IResourceContainer.h>
#include <mtkcam/middleware/v1/LegacyPipeline/LegacyPipelineBuilder.h>
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/StreamBufferProviderFactory.h>

#include <cutils/properties.h>

#include "utils/Misc.h"

#define PIPELINED_HDR

#include <mtkcam/feature/utils/FeatureProfileHelper.h>

#define POSTVIEW_ENABLED 0

using namespace android;
using namespace NSCam::Utils;
using namespace NSCam::v1;
using namespace NSCamHW;
using namespace android::NSShot;
using namespace NSCamShot;

// ---------------------------------------------------------------------------

extern sp<IShot> createInstance_VHdrShot(
        char const * const pszShotName,
        uint32_t const u4ShotMode,
        int32_t const  i4OpenId,
        uint32_t const u4VHDRMode,
        bool const bIsAutoHDR)
{
    sp<IShot> shot;
    sp<VHDRShot> hdrShot;

    // new implementator
    hdrShot = new VHDRShot(
            pszShotName, u4ShotMode, i4OpenId, u4VHDRMode, bIsAutoHDR);
    if (hdrShot == NULL)
    {
        HDR_LOGE("create VHDRShot failed");
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

MINT32 VHDRShot::sSequence = 0;
Mutex VHDRShot::sInstanceLock;
MINT32 VHDRShot::sInstanceCount = 0;

Mutex VHDRShot::sCaptureDoneLock;
sem_t VHDRShot::sSemCaptureDone;

sem_t VHDRShot::sSemPostProcessingDone;

VHDRShot::VHDRShot(
        const char *pszShotName,
        uint32_t const u4ShotMode,
        int32_t const i4OpenId,
        uint32_t const u4VHDRMode,
        bool const bIsAutoHDR)
    : ImpShot(pszShotName, u4ShotMode, i4OpenId),
      mVHDRMode(u4VHDRMode), mIsAutoHDR(bIsAutoHDR),
      mDataMsgSet(0)
{
    mSequence = sSequence++;
    sSequence = (sSequence >= 32) ? 0 : sSequence;
    mTimestamp = TimeTool::getReadableTime();

    // init semaphore
    {
        Mutex::Autolock _l(sInstanceLock);
        if (sInstanceCount == 0)
        {
            Mutex::Autolock _l(sCaptureDoneLock);
            sem_init(&sSemCaptureDone, 0, 1);
            sem_init(&sSemPostProcessingDone, 0, 1);
            HDR_LOGD("init semaphore");
        }
    }

    char value[PROPERTY_VALUE_MAX];
    {
        // debug mode
        property_get("vendor.mediatek.hdr.debug", value, "0");
        mDebugMode = atoi(value);
        HDR_LOGD_IF(mDebugMode == 1, "hdr debug mode ON");

        // make debug dump path
        if (mDebugMode && !makePath(HDR_DUMP_PATH, 0660))
        {
            HDR_LOGW("make debug dump path %s failed", HDR_DUMP_PATH);
        }
    }

    // debug property setting
    {
        // log level
        property_get("vendor.mediatek.hdr.log", value, "0");
        mLogLevel = atoi(value);
        HDR_LOGD_IF(mLogLevel > 0, "log level is set to %d", mLogLevel);
    }
}

VHDRShot::~VHDRShot()
{
    if (mpPipeline.get())
        mpPipeline->flush();

    // destroy semaphore
    {
        Mutex::Autolock _l(sInstanceLock);
        if (sInstanceCount == 0)
        {
            Mutex::Autolock _l(sCaptureDoneLock);
            sem_destroy(&sSemCaptureDone);
            sem_destroy(&sSemPostProcessingDone);
            HDR_LOGD("destroy semaphore");
        }
    }
}

// metadata event handler
void VHDRShot::onResultReceived(
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
            break;

        case eSTREAMID_META_APP_DYNAMIC_P2:
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

        case eSTREAMID_META_HAL_DYNAMIC_P2:
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
MERROR VHDRShot::onResultReceived(
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

            if (CC_UNLIKELY(mDebugMode))
            {
                char filename[256];
                sprintf(filename, "%09" PRId64 "-%04d-1_p1_%04dx%04d_%04zu.raw",
                    mTimestamp, mSequence,
                    pBuffer->getImgSize().w,
                    pBuffer->getImgSize().h,
                    pBuffer->getBufStridesInBytes(0));
                dumpBuffer(pBuffer, filename);
            }
#ifndef PIPELINED_HDR
            sem_post(&sSemCaptureDone);
#endif
            break;

        case eSTREAMID_IMAGE_JPEG:
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

        case eSTREAMID_IMAGE_PIPE_YUV_JPEG:
        case eSTREAMID_IMAGE_PIPE_RAW_LCSO:
            break;

        default:
            HDR_LOGW("unsupported streamID(%#" PRIx64 ")", streamId);
    }

    return OK;
}

String8 VHDRShot::getUserName()
{
    HDR_TRACE_CALL();

    return String8(DEBUG_LOG_TAG);
}

bool VHDRShot::sendCommand(
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

bool VHDRShot::onCmd_reset()
{
    HDR_TRACE_CALL();

    return true;
}

bool VHDRShot::onCmd_capture()
{
    FUNCTION_LOG_START;

    HDR_LOGD("[%d] %s", mSequence, __FUNCTION__);

    HDR_TRACE_CALL();

    // dump metadata
    if (CC_UNLIKELY(mLogLevel > 0))
        dumpMetadata(mShotParam.mAppSetting, "app");

    // update post nr settings
    updatePostNRSetting(MTK_NR_MODE_AUTO, false, &mShotParam.mHalSetting);

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
    name.appendFormat("Cam@VHDRWorker_%d", mSequence);
    sp<WorkerThread> workerThread =
        new WorkerThread(name.string(), mu4ShotMode, this);
    workerThread->run(name.string());

    // for normal mode, we need to wait capture done before starting preview
    if (eShotMode_HdrShot == mu4ShotMode)
        workerThread->join();

    FUNCTION_LOG_END_MUM;

    return true;
}

void VHDRShot::onCmd_cancel()
{
    HDR_TRACE_CALL();
}

bool VHDRShot::normalCapture()
{
    FUNCTION_LOG_START;

    HDR_TRACE_CALL();

    bool ret = true;

    // 1. set sensor parameters, expected callback message type
    {
        CamManager* pCamMgr = CamManager::getInstance();
        const MUINT32 scenario =
            (pCamMgr->isMultiDevice() && (pCamMgr->getFrameRate(getOpenId()) == 0)) ?
            SENSOR_SCENARIO_ID_NORMAL_PREVIEW :
            SENSOR_SCENARIO_ID_NORMAL_CAPTURE;

        mSensorParam = NSCamShot::SensorParam(
                getOpenId(),              // sensor idx
                scenario,                 // Scenaio
                getShotRawBitDepth(),     // bit depth
                MFALSE,                   // bypass delay
                MFALSE,                   // bypass scenario
                ECamShot_RAWTYPE_PURE,    // raw type
                mVHDRMode);               // vhdr mode

        EImageFormat ePostViewFmt =
            static_cast<EImageFormat>(mShotParam.miPostviewDisplayFormat);

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
    sp<CamManager::UsingDeviceHelper> spDeviceHelper =
        new CamManager::UsingDeviceHelper(mSensorParam.u4OpenID);
    if (createPipeline() != OK)
    {
        HDR_LOGE("create pipeline failed");
        onFakeJpegCallback(mpShotCallback);
        return false;
    }
    spDeviceHelper->configDone();

    // 4. prepare app and hal metadata
    SettingSet vSettings;
    {
        // update ISP profile
        {
            // Prepare query Feature Shot ISP Profile
            ProfileParam profileParam
            {
                mSensorSetting.size,
                mVHDRMode,
                mSensorParam.u4Scenario,
                ProfileParam::FLAG_NONE,
                mIsAutoHDR ? ProfileParam::FMASK_AUTO_HDR_ON :
                             ProfileParam::FMASK_NONE
            };

            MUINT8 profile = 0;
            if (FeatureProfileHelper::getShotProf(profile, profileParam))
            {
                IMetadata::setEntry<MUINT8>(
                        &mShotParam.mHalSetting , MTK_3A_ISP_PROFILE , profile);
                HDR_LOGD("ISP profile is set(%u)", profile);
            }
            else
            {
                HDR_LOGW("ISP profile is not set(%u)", profile);
            }
        }

        // set parameters into metadata
        vSettings.appSetting = mShotParam.mAppSetting;
        vSettings.halSetting = mShotParam.mHalSetting;

        // modify hal control metadata
        IMetadata &halSetting(vSettings.halSetting);
        {
            IMetadata::setEntry<MSize>(
                    &halSetting, MTK_HAL_REQUEST_SENSOR_SIZE, mSensorSetting.size);
            IMetadata::setEntry<MUINT8>(
                    &halSetting, MTK_HAL_REQUEST_REQUIRE_EXIF, 1);
            IMetadata::setEntry<MUINT8>(
                    &halSetting, MTK_3A_AE_CAP_SINGLE_FRAME_HDR, 1);
        }
    }

    // 5. submit setting to pipeline
    if (mpPipeline->submitSetting(
                0, vSettings.appSetting, vSettings.halSetting) != OK)
    {
        HDR_LOGE("submit setting failed");
        return false;
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

bool VHDRShot::zsdCapture()
{
    FUNCTION_LOG_START;

    HDR_TRACE_CALL();

    bool ret = true;

    status_t err = OK;

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
                getOpenId(),                       // sensor idx
                SENSOR_SCENARIO_ID_NORMAL_CAPTURE, // force here
                getShotRawBitDepth(),              // bit depth
                MFALSE,                            // bypass delay
                MFALSE,                            // bypass scenario
                ECamShot_RAWTYPE_PURE,             // raw type
                mVHDRMode);                        // vhdr mode

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
    {
        // update ISP profile
        {
            // Prepare query Feature Shot ISP Profile
            ProfileParam profileParam
            {
                mSensorSetting.size,
                mVHDRMode,
                mSensorParam.u4Scenario,
                ProfileParam::FLAG_NONE,
                mIsAutoHDR ? ProfileParam::FMASK_AUTO_HDR_ON :
                             ProfileParam::FMASK_NONE
            };

            MUINT8 profile = 0;
            if (FeatureProfileHelper::getShotProf(profile, profileParam))
            {
                IMetadata::setEntry<MUINT8>(
                        &mShotParam.mHalSetting , MTK_3A_ISP_PROFILE , profile);
                HDR_LOGD("ISP profile is set(%u)", profile);
            }
            else
            {
                HDR_LOGW("ISP profile is not set(%u)", profile);
            }
        }

        // set parameters into metadata
        SettingSet setting = { mShotParam.mAppSetting, mShotParam.mHalSetting };

        // modify hal control metadata
        IMetadata &halSetting(setting.halSetting);
        {
            IMetadata::setEntry<MSize>(
                    &halSetting, MTK_HAL_REQUEST_SENSOR_SIZE, mSensorSetting.size);
            IMetadata::setEntry<MUINT8>(
                    &halSetting, MTK_HAL_REQUEST_REQUIRE_EXIF, 1);
            IMetadata::setEntry<MUINT8>(
                    &halSetting, MTK_3A_AE_CAP_SINGLE_FRAME_HDR, 1);
        }
        vSettings.push_back(setting);
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

        // only output full and statistics buffers
        Vector<sp<IImageStreamInfo>> rawInputInfos;
        selector->queryCollectImageStreamInfo(rawInputInfos, MTRUE);
        BufferList vDstStreams;
        for (size_t i = 0; i < rawInputInfos.size(); i++)
        {
            vDstStreams.push_back(
                    BufferSet { rawInputInfos[i]->getStreamId(), MFALSE });
        }

        if (previewFlowCtrl->submitRequest(
                    vSettings, vDstStreams, resultRequestNo) != OK)
        {
            HDR_LOGE("submit settings failed");
            return false;
        }

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
    SettingSet capSetting;
    {
        MINT32 requestNo;
        IMetadata appSetting;
        IMetadata halSetting;

        // blocking call to get captured result
        err = getSelectResult(
                selector, requestNo, appSetting, halSetting);
        if (CC_LIKELY(OK == err))
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

            capSetting = { appSetting, halSetting };

            // pass app result metadata to timestamp processor
            onSelectedResultReceived(mpPipeline, appSetting);
        }
        else
        {
            // flush previous selected buffers and settings
            returnSelectedBuffers();
            for (size_t i = 0; i < mP1PoolMap.size(); i++)
                mP1PoolMap.editValueAt(i)->flush();

            HDR_LOGW("get selected result failed(%s)", strerror(-err));
            // the preview pipeline has abandoned us,
            // there is no more result for selector now.
            //
            // we have nothing to do but give up this capture request
            HDR_LOGW_IF((err == WOULD_BLOCK),
                    "no more result cab be received from selector, " \
                    "give up this capture request");

#ifdef PIPELINED_HDR
            // release capture and post-processing lock to
            // ensure the next capture would not be blocked
            sem_post(&sSemCaptureDone);
            sem_post(&sSemPostProcessingDone);
#endif
            onFakeJpegCallback(mpShotCallback);
        }
    }

    // 7. submit setting to pipeline
    if (err == OK)
    {
        const MINT32 requestNo = 0;
        IMetadata& appSetting(capSetting.appSetting);
        IMetadata& halSetting(capSetting.halSetting);

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

void VHDRShot::onCaptureBegin()
{
    Mutex::Autolock _l(sInstanceLock);
    sInstanceCount++;

    HDR_LOGD("instanceCount(%d) +", sInstanceCount);
    HDR_TRACE_INT("InFlight(hdr)", sInstanceCount);
}

void VHDRShot::onCaptureEnd()
{
    Mutex::Autolock _l(sInstanceLock);
    sInstanceCount--;

    HDR_LOGD("instanceCount(%d) -", sInstanceCount);
    HDR_TRACE_INT("InFlight(hdr)", sInstanceCount);
}

MUINT32 VHDRShot::getRotation() const
{
    return mShotParam.mu4Transform;
}

status_t VHDRShot::createStreams(const sp<StreamBufferProvider>& provider)
{
    FUNCTION_LOG_START;

    HDR_TRACE_CALL();

    MUINT32 const openId     = mSensorParam.u4OpenID;
    MUINT32 const sensorMode = mSensorParam.u4Scenario;
    MUINT32 const bitDepth   = mSensorParam.u4Bitdepth;

    MSize const postviewsize =
        MSize(mShotParam.mi4PostviewWidth, mShotParam.mi4PostviewHeight);
    MINT const postviewfmt   = mShotParam.miPostviewDisplayFormat;
    // NOTE: jpeg & thumbnail yuv only support YUY2
    MINT const yuvfmt        = eImgFmt_YUY2;
    MSize const jpegsize     = (getRotation() & eTransform_ROT_90) ?
        MSize(mShotParam.mi4PictureHeight, mShotParam.mi4PictureWidth):
        MSize(mShotParam.mi4PictureWidth, mShotParam.mi4PictureHeight);
    MSize const thumbnailsize =
        MSize(mJpegParam.mi4JpegThumbWidth, mJpegParam.mi4JpegThumbHeight);

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
                    "VHDRShot:FullRaw",
                    eSTREAMID_IMAGE_PIPE_RAW_OPAQUE,
                    eSTREAMTYPE_IMAGE_INOUT,
                    1, 1,
                    usage, format, size, stride);

            if (mpImageInfo_fullRaw == NULL)
            {
                HDR_LOGE("create ImageStreamInfo FullRaw failed");
                return NO_INIT;
            }

            dumpImageStreamInfo(mpImageInfo_fullRaw);
        }

        // Lcso raw
        {
            NS3Av3::LCSO_Param lcsoParam;
            if ( auto pIspMgr = MAKE_IspMgr() ) {
                pIspMgr->queryLCSOParams(lcsoParam);
            }

            MUINT const usage = 0; // not necessary here

            mpImageInfo_LcsoRaw = createRawImageStreamInfo(
                    "VHDRShot:LcsoRaw",
                    eSTREAMID_IMAGE_PIPE_RAW_LCSO,
                    eSTREAMTYPE_IMAGE_INOUT,
                    1, 1,
                    usage, lcsoParam.format, lcsoParam.size, lcsoParam.stride);

            if (mpImageInfo_LcsoRaw == NULL)
            {
                HDR_LOGE("create ImageStreamInfo LcsoRaw failed");
                return NO_INIT;
            }

            dumpImageStreamInfo(mpImageInfo_LcsoRaw);
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
                "VHDRShot:Postview",
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
                "VHDRShot:YuvJpeg",
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
                "VHDRShot:YuvThumbnail",
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
                "VHDRShot:Jpeg",
                eSTREAMID_IMAGE_JPEG,
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

status_t VHDRShot::createPipeline(const sp<StreamBufferProvider>& provider)
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
        ? LegacyPipelineMode_T::PipelineMode_Feature_Capture
        : LegacyPipelineMode_T::PipelineMode_Feature_Capture_VSS;

    LPBConfigParams.enableEIS = MFALSE;
    LPBConfigParams.enableLCS = MTRUE;

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
                "VHDRShot",
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

        // lcso raw
        if (mpImageInfo_LcsoRaw.get())
        {
            sp<IImageStreamInfo>& pStreamInfo(mpImageInfo_LcsoRaw);

            PipelineImageParam imgParam
            {
                pStreamInfo,
                nullptr,
                0
            };
            vImageParam.push_back(imgParam);

            HDR_LOGD("add lcso raw");
        }

        PipelineSensorParam sensorParam
        {
            mSensorParam.u4Scenario,
            mSensorParam.u4RawType,
            mSensorSetting.size,
            static_cast<MUINT>(mSensorSetting.fps),
            mSensorSetting.pixelMode,
            mSensorParam.u4VHDRMode
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

        for (size_t i = 0; i < rawInputInfos.size() ; i++)
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

status_t VHDRShot::getSelectResult(
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
    for(size_t i = 0; i < resultBuffers.size() ; i++)
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

status_t VHDRShot::returnSelectedBuffers()
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

status_t VHDRShot::releaseResource()
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

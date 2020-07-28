/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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
#define __DEBUG // enable debug
// #define __SCOPE_TIMER // enable log of scope timer

#define LOG_TAG "MfllController_v1_6"
static const char* __CALLERNAME__ = LOG_TAG;

#include "MFNRCtrler_v1.6.h"
#include "../../utils/VendorUtils.h"

// MTKCAM
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/Trace.h>
#include <mtkcam/utils/std/Time.h>
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/middleware/v1/camutils/IDetachJobManager.h>

// MFNR
#include <mtkcam/feature/mfnr/MfllLog.h>

// AOSP
#include <cutils/compiler.h>
#include <cutils/properties.h>

// STL
#include <atomic>
#include <cstdint>
#include <string>
#include <fstream>

//tuning utils
#include <mtkcam/utils/TuningUtils/FileReadRule.h>

// MFNR deque frame, postview timeout. Unit is: ms
#if defined(MTKCAM_LOG_LEVEL_DEFAULT) && (MTKCAM_LOG_LEVEL_DEFAULT <= 2)
// user load, reduce timeout
#define MFNRCTRLER_DEQUE_NEW_REQUEST_FRAME_TIMEOUT_1ST  2500  // non-zsd, ais, or flash, the first frame.
#define MFNRCTRLER_DEQUE_NEW_REQUEST_FRAME_TIMEOUT      500   // non-zsd or ais (request new frame buffers)
#define MFNRCTRLER_DEQUE_CACHED_FRAME_TIMEOUT           5000  // zsd and non-ais (use cached selector buffer)
#define MFNRCTRLER_POSTVIEW_TIMEOUT                     5000
#else
#define MFNRCTRLER_DEQUE_NEW_REQUEST_FRAME_TIMEOUT_1ST  4000
#define MFNRCTRLER_DEQUE_NEW_REQUEST_FRAME_TIMEOUT      600   // non-zsd or ais (request new frame buffers)
#define MFNRCTRLER_DEQUE_CACHED_FRAME_TIMEOUT           5000  // zsd and non-ais (use cached selector buffer)
#define MFNRCTRLER_POSTVIEW_TIMEOUT                     10000
#endif

static_assert( MFNRCTRLER_DEQUE_NEW_REQUEST_FRAME_TIMEOUT_1ST >= MFNRCTRLER_DEQUE_NEW_REQUEST_FRAME_TIMEOUT,
        "The timeout of the first frame is supposed to be greater than the followings");

#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

#define MY_DBG_COND(level)          __builtin_expect( m_dbgLevel >= level, false )
#define MY_LOGD3(...)               do { if ( MY_DBG_COND(3) ) MY_LOGD(__VA_ARGS__); } while(0)
// FUNCTION_SCOPE
#ifdef __DEBUG
#define FUNCTION_SCOPE          auto __scope_logger__ = create_scope_logger(__FUNCTION__)
#include <memory>
#include <functional>
static std::shared_ptr<char> create_scope_logger(const char* functionName)
{
    bool bEnableLog = mfll::MfllProperty::getDebugLevel() >= 3;
    char* pText = const_cast<char*>(functionName);
    CAM_LOGD_IF(bEnableLog, "[%s] + ", pText);
    return std::shared_ptr<char>(pText, [bEnableLog](char* p){ CAM_LOGD_IF(bEnableLog, "[%s] -", p); });
}
#else
#define FUNCTION_SCOPE          do{}while(0)
#endif
// SCOPE_TIMER
#ifdef __SCOPE_TIMER
#define SCOPE_TIMER(VAR, TEXT)  auto VAR = create_scope_timer(TEXT)
#include <memory>
#include <functional>
#include <utils/SystemClock.h> // elapsedRealtime(), uptimeMillis()
static std::unique_ptr<const char, std::function<void(const char*)> >
create_scope_timer(const char* text)
{
    auto t1 = android::elapsedRealtimeNano();
    return std::unique_ptr<const char, std::function<void(const char*)> >
        (
            text,
            [t1](const char* t)->void
            {
                auto t2 = android::elapsedRealtimeNano();
                MY_LOGD("%s --> duration(ns): %" PRId64 "", t, (t2 -t1));
            }
        );
}
#else
#define SCOPE_TIMER(VAR, TEXT)  do{}while(0)
#endif

using namespace android;
using namespace mfll;
using namespace NSCam;
using namespace NSCam::plugin;
using namespace NSCam::TuningUtils;
// ----------------------------------------------------------------------------
// MFNRCtrler
// ----------------------------------------------------------------------------
MFNRCtrler_v1_6::MFNRCtrler_v1_6(
        int sensorId,
        MfllMode mfllMode,
        MUINT32 sensorMode,
        int realIso,
        int exposureTime,
        bool isFlashOn)
    : MFNRCtrler(sensorId, mfllMode, sensorMode, realIso, exposureTime, isFlashOn)
    , m_droppedFrameNum(0)
    , m_bNeedExifInfo(false)
    , m_bFired(false)
    , m_effectMode(MTK_CONTROL_EFFECT_MODE_OFF)
    , m_spNvramProvider(nullptr)
{
    // ISO/Exposure information has been updated by MFNRCtrler.
    // here we need to update capture info by MfllStrategy
    updateMfllStrategy();
}


MFNRCtrler_v1_6::~MFNRCtrler_v1_6()
{
    FUNCTION_SCOPE;
}


// ----------------------------------------------------------------------------
// Event Listener: Event listener bypass events to MFNR controller
// ----------------------------------------------------------------------------
namespace __mfnrctrler_v1_6__ {
static void __set_postview_buffer_to_core(
        IMfllCore*          pCore,
        IImageBuffer*       pBuffer,
        const IMetadata*    pAppMeta,
        const IMetadata*    pHalMeta,
        const int           openId
        )
{
    FUNCTION_SCOPE;
    if ( CC_UNLIKELY( pCore == nullptr ) ) {
        MY_LOGE("%s: IMfllCore is null", __FUNCTION__);
        return;
    }

    if ( CC_UNLIKELY( pBuffer == nullptr ) ) {
        MY_LOGD("%s: Postview buffer is nullptr", __FUNCTION__);
        return;
    }

    if ( CC_UNLIKELY( pAppMeta == nullptr ) ) {
        MY_LOGE("%s: App metadata is nullptr", __FUNCTION__);
        return;
    }

    if ( CC_UNLIKELY( pHalMeta == nullptr ) ) {
        MY_LOGE("%s: Hal metadata is nullptr", __FUNCTION__);
        return;
    }

    sp<IMfllImageBuffer> __postview = IMfllImageBuffer::createInstance("postview");
    if ( CC_UNLIKELY( __postview.get() == nullptr ) ) {
        MY_LOGE("%s: create IMfllImageBuffer instance failed", __FUNCTION__);
        return;
    }

    // calculate crop
    MRect cropRgn = calCropRegin(
            pAppMeta, pHalMeta, pBuffer->getImgSize(), openId);

    MfllRect_t __srcCropRgn(cropRgn.p.x, cropRgn.p.y, cropRgn.s.w, cropRgn.s.h);

    __postview->setImageBuffer(reinterpret_cast<void*>(pBuffer));
    pCore->setPostviewBuffer(__postview, __srcCropRgn);
}

static void __set_thumbnail_buffer_to_core(
        IMfllCore*          pCore,
        IImageBuffer*       pBuffer,
        const IMetadata*    pAppMeta,
        const IMetadata*    pHalMeta,
        const int           openId
        )
{
    FUNCTION_SCOPE;
    if ( CC_UNLIKELY( pCore == nullptr ) ) {
        MY_LOGE("%s: IMfllCore is null", __FUNCTION__);
        return;
    }

    if ( CC_UNLIKELY( pBuffer == nullptr ) ) {
        MY_LOGD("%s: Postview buffer is nullptr", __FUNCTION__);
        return;
    }

    if ( CC_UNLIKELY( pAppMeta == nullptr ) ) {
        MY_LOGE("%s: App metadata is nullptr", __FUNCTION__);
        return;
    }

    if ( CC_UNLIKELY( pHalMeta == nullptr ) ) {
        MY_LOGE("%s: Hal metadata is nullptr", __FUNCTION__);
        return;
    }

    sp<IMfllImageBuffer> __thumbnail = IMfllImageBuffer::createInstance("thumbnail");
    if ( CC_UNLIKELY( __thumbnail.get() == nullptr ) ) {
        MY_LOGE("%s: create IMfllImageBuffer instance failed", __FUNCTION__);
        return;
    }

    // calculate crop
    MRect cropRgn = calCropRegin(
            pAppMeta, pHalMeta, pBuffer->getImgSize(), openId);

    MfllRect_t __srcCropRgn(cropRgn.p.x, cropRgn.p.y, cropRgn.s.w, cropRgn.s.h);

    __thumbnail->setImageBuffer(reinterpret_cast<void*>(pBuffer));
    pCore->setThumbnailBuffer(__thumbnail, __srcCropRgn);
}

}; // namespace __mfnrctrler_v1_6__
using __mfnrctrler_v1_6__::__set_postview_buffer_to_core;
using __mfnrctrler_v1_6__::__set_thumbnail_buffer_to_core;

// ----------------------------------------------------------------------------
// Event Listener: Event listener bypass events to MFNR controller
// ----------------------------------------------------------------------------
class ListenerImp4Ctrler : public IMfllEventListener {
public:
    ListenerImp4Ctrler(MFNRCtrler_v1_6* ctrl) : m_pCtrl(ctrl) {};
    virtual ~ListenerImp4Ctrler() = default;

public:
    void onEvent(enum EventType t, MfllEventStatus_t &status, void *mfllCore, void *param1 = NULL, void *param2 = NULL)
    {
        m_pCtrl->onEvent(t, status, mfllCore, param1, param2);
    }

    void doneEvent(enum EventType t, MfllEventStatus_t &status, void *mfllCore, void *param1 = NULL, void *param2 = NULL)
    {
        m_pCtrl->doneEvent(t, status, mfllCore, param1, param2);
    }

    vector<mfll::EventType> getListenedEventTypes()
    {
        vector<enum EventType> m;
        #define LISTEN(x) m.push_back(x)
        LISTEN(EventType_Capture); // all frame are captured
        LISTEN(EventType_Bss); // to release 1~N RAW buffers
        LISTEN(EventType_EncodeYuvBase);
        LISTEN(EventType_EncodeYuvGolden); // to release the base (index is 0) RAW buffer
        LISTEN(EventType_CaptureYuvQ);
        LISTEN(EventType_CaptureEis);
        LISTEN(EventType_MotionEstimation);
        LISTEN(EventType_MotionCompensation);
        LISTEN(EventType_Blending);
        LISTEN(EventType_Mixing);
        #undef LISTEN
        return m; // RVO
    }

private:
    MFNRCtrler_v1_6* m_pCtrl;
};


// ----------------------------------------------------------------------------
// MFNRCtrler_v1_6 methods
// ----------------------------------------------------------------------------
void MFNRCtrler_v1_6::updateMfllStrategy()
{
    FUNCTION_SCOPE;
    SCOPE_TIMER(__t0, __FUNCTION__);

    bool bIsAis = mfll::isAisMode(getMfbMode());

    MfllStrategyConfig_t strategyCfg;

#define __DEFAULT_CAPTURE_NUM__ 4
    int captureNum = __DEFAULT_CAPTURE_NUM__; // makes default 4 frame
    int blendNum = __DEFAULT_CAPTURE_NUM__;
#undef  __DEFAULT_CAPTURE_NUM__

    int suggestedIso = getFinalIso(); // current iso from PLINE
    int suggestedExp = getFinalShutterTime();

    sp<IMfllStrategy> pStrategy = IMfllStrategy::createInstance();
    m_spNvramProvider = IMfllNvram::createInstance();

    if (CC_UNLIKELY(pStrategy.get() == nullptr)) {
        MY_LOGE("IMfllStrategy create failed");
        goto LB_EXIT;
    }
    if (CC_UNLIKELY(m_spNvramProvider.get() == nullptr)) {
        MY_LOGE("create IMfllNvram failed");
        goto LB_EXIT;
    }

    {
        strategyCfg.original_iso = getRealIso();
        strategyCfg.original_exp = getShutterTime();
        strategyCfg.iso = suggestedIso;
        strategyCfg.exp = suggestedExp;
        strategyCfg.isAis = bIsAis ? 1 : 0;

        IMfllNvram::ConfigParams nvramCfg;
        nvramCfg.iSensorId = getOpenId();
        nvramCfg.bFlashOn  = getIsFlashOn();
        if (CC_UNLIKELY(m_spNvramProvider->init(nvramCfg) != MfllErr_Ok)) {
            MY_LOGE("init IMfllNvram failed");
            goto LB_EXIT;
        }

        if (CC_UNLIKELY(pStrategy->init(m_spNvramProvider) != MfllErr_Ok)) {
            MY_LOGE("init IMfllStrategy failed");
            goto LB_EXIT;
        }

        if (CC_UNLIKELY(pStrategy->queryStrategy(strategyCfg, &strategyCfg) != MfllErr_Ok)) {
            MY_LOGE("IMfllStrategy::queryStrategy returns error");
            goto LB_EXIT;
        }

        captureNum = static_cast<int>(strategyCfg.frameCapture);
        blendNum = static_cast<int>(strategyCfg.frameBlend);
        suggestedExp = static_cast<int>(strategyCfg.exp);
        suggestedIso = static_cast<int>(strategyCfg.iso);
    }

    if (getIsFlashOn()) {
        //TODO: need to refine it someday.
        MY_LOGD("Flash MFNR tricky patch: force set to 3 captureNum");
        captureNum = 3;
        blendNum = 3;
    }

LB_EXIT:
    // update again
    setFinalIso(suggestedIso);
    setFinalShutterTime(suggestedExp);
    setCaptureNum(captureNum);
    setBlendNum(blendNum);
    setEnableMfnr(strategyCfg.enableMfb);
    setFullSizeMc(strategyCfg.isFullSizeMc);
    setEnableDownscale(strategyCfg.enableDownscale);
    setDownscaleRatio(strategyCfg.downscaleRatio);

#ifdef __DEBUG
    MY_LOGD3("original:");
    MY_LOGD3("-------------");
    MY_LOGD3("iso = %d", getRealIso());
    MY_LOGD3("exp = %d", getShutterTime());
    MY_LOGD3("final:");
    MY_LOGD3("-------------");
    MY_LOGD3("iso = %d", getFinalIso());
    MY_LOGD3("exp = %d", getFinalShutterTime());
    MY_LOGD3("cap = %d", getCaptureNum());
    MY_LOGD3("bld = %d", getBlendNum());
    MY_LOGD3("fmc = %d", getIsFullSizeMc() ? 1 : 0);
    MY_LOGD3("mfb = %d", getIsEnableMfnr() ? 1 : 0);
    MY_LOGD3("fmc = %d", getIsFullSizeMc() ? 1 : 0);
    MY_LOGD3("downscale(enabled:%d, ratio%d)", getEnableDownscale(), getDownscaleRatio());
#endif
}


bool MFNRCtrler_v1_6::initMfnrCore()
{
    FUNCTION_SCOPE;
    SCOPE_TIMER(__t0, __FUNCTION__);

    // check source resolution is valid or not.
    if (getSizeSrc().w <= 0 || getSizeSrc().h <= 0) {
        MY_LOGE("source resolution is invalid (w,h)=(%d,%d)",
                getSizeSrc().w, getSizeSrc().h);
        return false;
    }

    // check blending/capture number
    if (getBlendNum() <= 0 || getCaptureNum() <= 0) {
        MY_LOGE("capture or blend number is wrong (0)");
        return false;
    }

    if (getBlendNum() > getCaptureNum()) {
        MY_LOGE("blend number(%d) is greater than capture(%d)."
                , getBlendNum(), getCaptureNum());
        setBlendNum(getCaptureNum());
    }

    MY_LOGD3("init MFNR Core Lib");

    // create instance.
    m_pCore = IMfllCore::createInstance();
    m_pCapturer = IMfllCapturer::createInstance();
    m_pMfb = IMfllMfb::createInstance();

    if (m_pCore.get() == nullptr) {
        MY_LOGE("create MFNR Core Library failed");
        return false;
    }

    if (m_pCapturer.get() == nullptr) {
        MY_LOGE("create MFNR Capturer failed");
        return false;
    }

    if (m_pMfb.get() == nullptr) {
        MY_LOGE("create MFNR MFB failed");
        return false;
    }

    // capturer
    m_pCapturer->setMfllCore(m_pCore.get());
    m_pCapturer->setShotMode(getMfbMode());
    m_pCapturer->setPostNrType(NoiseReductionType_None);

    // mfb
    m_pMfb->init(getOpenId());
    m_pMfb->setMfllCore(m_pCore.get());
    m_pMfb->setShotMode(getMfbMode());
    m_pMfb->setPostNrType(NoiseReductionType_None);

    // core
    MfllBypassOption byPassOption;
    for (int i = 0; i < MFLL_MAX_FRAMES; i++) {
        byPassOption.bypassAllocRawBuffer[i] = 1;
        byPassOption.bypassAllocRrzoBuffer[i] = 1;
    }

    m_pCore->setBypassOption(byPassOption);
    if (getCaptureNum() <= 1)
        m_pCore->setBypassOptionAsSingleFrame();

    m_pCore->setDownscaleInfo(
            getEnableDownscale(), getDownscaleRatio());

    m_pCore->setCaptureResolution(
            getSizeSrc().w, getSizeSrc().h);

    m_pCore->setRrzoResolution(
            getSizeRrzo().w, getSizeRrzo().h);

    m_pCore->setCapturer(m_pCapturer);
    m_pCore->setMfb(m_pMfb);
    m_pCore->setNvramProvider(m_spNvramProvider);
    m_pCore->registerEventListener(new ListenerImp4Ctrler(this)); // register event listener

    // prepare MfllConfig
    MfllConfig cfg;
    cfg.sensor_id = getOpenId();
    cfg.capture_num = getCaptureNum();
    cfg.blend_num = getBlendNum();
    cfg.mfll_mode = getMfbMode();
    cfg.post_nr_type = NoiseReductionType_None;
    cfg.full_size_mc = getIsFullSizeMc();
    cfg.iso = getFinalIso();
    cfg.exp = getFinalShutterTime();
    cfg.original_iso = getRealIso();
    cfg.original_exp = getShutterTime();
    cfg.flash_on = getIsFlashOn();

    if (m_pCore->init(cfg) != MfllErr_Ok) {
        MY_LOGE("Init MFNR Core returns fail");
        return false;
    }

    return true;
}


bool MFNRCtrler_v1_6::doMfnr()
{
    FUNCTION_SCOPE;
    SCOPE_TIMER(__t0, __FUNCTION__);
    CAM_TRACE_CALL();

    if (m_bFired) {
        MY_LOGE("MFNR has been fired, cannot fire twice");
        return false;
    }

    m_bFired = true;

    if (m_pCore.get() == nullptr) {
        MY_LOGE("MFNR Core is NULL");
        return false;
    }

    auto t1 = std::async(std::launch::async, [this]() {
        MFLL_THREAD_NAME("PROC_DEQUE_FRAMES");
        FileReadRule rule;
        // deque frame.
        MINT32 uniqueKey = static_cast<MINT32>(NSCam::Utils::TimeTool::getReadableTime()); // update unique key
        if (rule.isREADEnable("MFNR") && rule.getFile_uniqueKey("MFNR", getOpenId())!= 0)
            uniqueKey = rule.getFile_uniqueKey("MFNR", getOpenId());
        MINT32 isoVal    = 0;
        MINT32 shutterUs = 0;
        for (size_t i = 0; i < static_cast<size_t>(getCaptureNum()); i++) {
            mfllAutoLog("deque RequestFrame");
            MY_LOGD("deque RequestFrame %zu", i);

            // deque frame with a timeout
            int __TIMEOUT = MFNRCTRLER_DEQUE_CACHED_FRAME_TIMEOUT;
            if ( !isZsdMode(getMfbMode()) || isAisMode(getMfbMode()) ) { // non-zsd or ais mode (request new frames)
                __TIMEOUT = (i == 0) // we should wait long at the first frame
                    ? MFNRCTRLER_DEQUE_NEW_REQUEST_FRAME_TIMEOUT_1ST
                    : MFNRCTRLER_DEQUE_NEW_REQUEST_FRAME_TIMEOUT;

                MY_LOGD("request new frame, wait for up to %dms for request frame", __TIMEOUT);
            }
            else { // zsd and non-ais (use cached AF stable seletor buffers)
                __TIMEOUT = MFNRCTRLER_DEQUE_CACHED_FRAME_TIMEOUT;
                MY_LOGD("use cached buffer, wait for up to %dms for AF stable frame", __TIMEOUT);
            }
            std::shared_ptr<RequestFrame> pFrame;
            auto status = MFNRCtrler::dequeFrame(pFrame, __TIMEOUT); // deque frame for 1.5 seconds
            if (CC_UNLIKELY( status == std::cv_status::timeout )) {
                MY_LOGE("deque frame timout (%dms), ignore this frame", __TIMEOUT);
                pFrame = nullptr;
            }

            // check frame.
            if (pFrame.get() == nullptr || pFrame->isBroken()) {
                m_droppedFrameNum++;
                if (pFrame.get() == nullptr) {
                    MY_LOGE("dequed a empty RequestFrame, idx=%zu", i);
                }
                else if (pFrame->isBroken()) {
                    MY_LOGE("dequed a RequestFrame but marked as error, idx=%zu", i);
                }

                // main RequestFrame is broken, discard this operation
                if (i == 0) {
                    m_pCore->doCancel();
                    m_droppedFrameNum = getCaptureNum();
                    break; // break for-loop
                }
                else {
                    continue; // keep do MFNR.
                }
            }
            /* retrieve metadata */
            IMetadata* pAppMeta = pFrame->getMetadata(RequestFrame::eRequestMeta_InAppRequest);
            IMetadata* pHalMeta = pFrame->getMetadata(RequestFrame::eRequestMeta_InHalP1);
            /* retrieve image buffer that we care */
            IImageBuffer* pImageBuffer = pFrame->getImageBuffer(RequestFrame::eRequestImg_FullSrc);
            IImageBuffer* pImageLcsoRaw = pFrame->getImageBuffer(RequestFrame::eRequestImg_LcsoRaw);
            IImageBuffer* pImageResizeRaw = pFrame->getImageBuffer(RequestFrame::eRequestImg_ResizedSrc);
            do {
                rule.getFile_RAW(i, "MFNR", pImageBuffer, "MFNR", getOpenId());
                rule.getFile_LCSO(i, "MFNR", pImageLcsoRaw, "MFNR", getOpenId());
                rule.getFile_RRZO(i, "MFNR", pImageResizeRaw, "MFNR", getOpenId());
            } while(0);
            // check these metadata and image buffer
            if (pAppMeta == nullptr) {
                MY_LOGD("no in app request metadata");
                // it's ok if no AppMeta...maybe.
            }
            if (pHalMeta == nullptr) {
                MY_LOGE("no in hal P1 metadata, fatal error");
                m_droppedFrameNum++;
                continue;
            }
            if (pImageBuffer == nullptr) {
                MY_LOGE("no src image (full size), fatal error");
                m_droppedFrameNum++;
                continue;
            }
            if (pImageResizeRaw == nullptr) {
                MY_LOGE("no src image (rrzo), fatal error");
                m_droppedFrameNum++;
                continue;
            }
            else {
                MY_LOGD3("get src image (rrzo), addr=%p", pImageResizeRaw);
            }
            if (pImageLcsoRaw == nullptr) {
                MY_LOGD("no src image (lcso), it's ok");
            }

            {
                struct T {
                    MINT32 val;
                    MBOOL result;
                    T() : val(-1), result(MFALSE) {};
                } iso, exp;

                MBOOL checkResult = MFALSE;
                IMetadata exifMeta;

                checkResult = IMetadata::getEntry<IMetadata>(
                        const_cast<IMetadata*>(pHalMeta),
                        MTK_3A_EXIF_METADATA,
                        exifMeta);

                if (checkResult != MFALSE) {
                    iso.result = IMetadata::getEntry<MINT32>(&exifMeta, MTK_3A_EXIF_AE_ISO_SPEED,        iso.val);
                    exp.result = IMetadata::getEntry<MINT32>(&exifMeta, MTK_3A_EXIF_CAP_EXPOSURE_TIME,   exp.val);
                }

                MY_LOGD3("%s:=========================", __FUNCTION__);
                MY_LOGD3("%s: Has 3A exif metadata: %d", __FUNCTION__, checkResult);
                MY_LOGD3("%s: Get Iso -> %d, iso->: %d", __FUNCTION__, iso.result, iso.val);
                MY_LOGD3("%s: Get Exp -> %d, exp->: %d", __FUNCTION__, exp.result, exp.val);
                MY_LOGD3("%s:=========================", __FUNCTION__);

                isoVal = iso.val;
                shutterUs = exp.val;
            }
            // update unique key, request number
            IMetadata::setEntry(pHalMeta, MTK_PIPELINE_UNIQUE_KEY, uniqueKey);
            IMetadata::setEntry(pHalMeta, MTK_PIPELINE_REQUEST_NUMBER, static_cast<MINT32>(i));

            // first frame is always a main frame.
            if (m_vUsedRequestFrame.size() <= 0) {
                m_mainRequestFrame = pFrame;

                // set middleware info to MFNR Core Lib
                do {
                    CAM_TRACE_NAME("get_debug_info");

                    MfllMiddlewareInfo_t __middlewareInfo =
                        m_pCore->getMiddlewareInfo();

                    auto _get_sensor_bit = [](int enumRawBit)->int
                    {
                        switch (enumRawBit) {
                        case RAW_SENSOR_8BIT:       return 8;
                        case RAW_SENSOR_10BIT:      return 10;
                        case RAW_SENSOR_12BIT:      return 12;
                        case RAW_SENSOR_14BIT:      return 14;
                        default:                    return 0xFF;
                        }
                        return 0xFF;
                    };

                    // query bayer order and raw bit
                    const IHalSensorList* pSlist = MAKE_HalSensorList();
                    if (CC_UNLIKELY( pSlist == nullptr )) {
                        MY_LOGW("make hal sensor list failed");
                    }
                    else {
                        MUINT32 sensorDev = static_cast<MUINT32>(pSlist->querySensorDevIdx(getOpenId()));
                        NSCam::SensorStaticInfo _sinfo;
                        pSlist->querySensorStaticInfo(sensorDev, &_sinfo);
                        __middlewareInfo.bayerOrder = static_cast<int>(
                                _sinfo.sensorFormatOrder);
                        __middlewareInfo.rawBitNum = _get_sensor_bit(
                                _sinfo.rawSensorBit);
                    }

                    // Get middleware info and partial update
                    __middlewareInfo.uniqueKey = uniqueKey;
                    __middlewareInfo.iso       = static_cast<int>(isoVal);
                    __middlewareInfo.shutterUs = static_cast<int>(shutterUs);
                    m_pCore->setMiddlewareInfo(__middlewareInfo);
                    MY_LOGD3("set uniqueKey(%d) to MFNR Core Lib", uniqueKey);
                } while(0);

                auto appMeta = pFrame->getMetadata(RequestFrame::eRequestMeta_InAppRequest);
                auto halMeta = pFrame->getMetadata(RequestFrame::eRequestMeta_InHalP1);

                // do check postview & effect mode
                do {
                    // {{{
                    auto pPostviewYuv = pFrame->getImageBuffer(RequestFrame::eRequestImg_PostviewOut);
                    if (pPostviewYuv == NULL) {
                        MY_LOGD("no postview buffer in main RequestFrame");
                        break;
                    }

                    if (appMeta == NULL) {
                        MY_LOGE("cannot get app metadata, query efeect mode failed");
                        break;
                    }

                    MUINT8 effectMode = 0;
                    if (CC_UNLIKELY( !IMetadata::getEntry<MUINT8>(appMeta, MTK_CONTROL_EFFECT_MODE, effectMode) )) {
                        MY_LOGW("cannot get effect mode, assume it OFF");
                    }
                    m_effectMode = effectMode;

                    if (halMeta == NULL) {
                        MY_LOGE("cannot calculate the crop rgn of postview, use default");
                        break;
                    }
                    // set postview buffer
                    __set_postview_buffer_to_core(m_pCore.get(), pPostviewYuv, appMeta, halMeta, getOpenId());
                    //
                    MY_LOGD3("set postview image buffer to MfllCore");
                    // }}}
                } while(0);


                // set thumbnail yuv to mfllcore
                do {
                    auto pThumbYuv = pFrame->getImageBuffer(RequestFrame::eRequestImg_ThumbnailOut);
                    if (CC_UNLIKELY(pThumbYuv == NULL)) {
                        MY_LOGW("no thumbnail yuv buffer in main RequestFrame");
                    }
                    else {
                        // set thumbnail yuv buffer
                        __set_thumbnail_buffer_to_core(m_pCore.get(), pThumbYuv, appMeta, halMeta, getOpenId());
                    }
                } while (0);


                // set Mixing yuv buffer (use base or working buffer)
                do {
                    auto pFrameYuvJpeg = pFrame->getImageBuffer(RequestFrame::eRequestImg_FullOut);
                    auto pFrameWorking = pFrame->getImageBuffer(RequestFrame::eRequestImg_WorkingBufferOut);
                    if (pFrameYuvJpeg == NULL && pFrameWorking != NULL) {
                        MY_LOGD("get working buffer, use it to as mixing dst buffer");
                        /* create IMfllImageBuffer of mixing YUV working buffer */
                        sp<IMfllImageBuffer> mfllMixWorkingBuf = IMfllImageBuffer::createInstance("mixWorking");
                        if (CC_UNLIKELY(mfllMixWorkingBuf == NULL)) {
                            mfllLogW("%s: create YUV working buffer instance failed", __FUNCTION__);
                            MY_LOGD("cannot get working buffer, use base yuv buffer as mixing dst buffer");
                            m_pCore->setMixingBuffer(MixYuvType_Base, nullptr);
                        }
                        else {
                            mfllMixWorkingBuf->setImageBuffer(pFrameWorking);
                            m_pCore->setMixingBuffer(MixYuvType_Working, mfllMixWorkingBuf);
                        }
                    }
                    else {
                        if (pFrameYuvJpeg != NULL && pFrameWorking != NULL) {
                            MY_LOGD("weird, yuv jpeg buffer and working buffer exist at the same time");
                        }
                        MY_LOGD("cannot get working buffer, use base yuv buffer as mixing dst buffer");
                        m_pCore->setMixingBuffer(MixYuvType_Base, nullptr);
                    }
                } while (0);
                // update request Exif
                {
                    MUINT8 isRequestExif = 0;
                    tryGetMetadata<MUINT8>(
                            pFrame->getMetadata(RequestFrame::eRequestMeta_InHalP1),
                            MTK_HAL_REQUEST_REQUIRE_EXIF,
                            isRequestExif);

                    m_bNeedExifInfo = (isRequestExif != 0);
                }
            } // fi (frame 0)

            m_vUsedRequestFrameMx.lock();
            m_vUsedRequestFrame.push_back(pFrame);
            m_vUsedRequestFrameMx.unlock();


            /* create IMfllImageBuffer of RAW */
            sp<IMfllImageBuffer> mfllImgBuf = IMfllImageBuffer::createInstance("raw");
            mfllImgBuf->setImageBuffer(pImageBuffer);

            // TODO: Gyro info here...

            /* retrieve GMV */
            MfllMotionVector mv = calMotionVector(pHalMeta, i);

            // add buffer to MFNR core.
            addDataToMfnrCore(mfllImgBuf, mv, pAppMeta, pHalMeta, pImageLcsoRaw, pImageResizeRaw);
        } // for-loop: for every frames

        // If we got some error frames, we need gives dummy frames
        for (int i = 0; i < m_droppedFrameNum; i++) {
            sp<IMfllImageBuffer> dummy = IMfllImageBuffer::createInstance("dummy");
            addDummyFrameToMfnrCore(dummy);
            std::shared_ptr<RequestFrame> dummyRequestFrame(nullptr);
            m_vUsedRequestFrameMx.lock();
            m_vUsedRequestFrame.push_back(dummyRequestFrame);
            m_vUsedRequestFrameMx.unlock();
        }

        invalidate();

        // wait until postview ready
        {
            onShutter(); // capture done, sends shutter sound

            std::unique_lock<std::mutex> __l(m_postViewReadyPack.locker);
            if ( ! m_postViewReadyPack.isReady ) {
                constexpr const int __TIMEOUT = MFNRCTRLER_POSTVIEW_TIMEOUT;
                MY_LOGD("%s: wait until postview generated [+]", __FUNCTION__);
                auto tt = m_postViewReadyPack.cond.wait_for(__l, std::chrono::milliseconds(__TIMEOUT));
                MY_LOGD("%s: wait until postview generated [-]", __FUNCTION__);
                if ( CC_UNLIKELY(tt == std::cv_status::timeout) ) {
                    MY_LOGW("%s: wait postview cond timeout (%d seconds)",
                            __FUNCTION__,
                            __TIMEOUT);
                    return;
                }
            }

            auto pMainFrame = m_mainRequestFrame.lock();
            if (pMainFrame.get() == NULL) {
                MY_LOGE("No main request frame ...");
                return;
            }
            pMainFrame->releaseImageBuffer(RequestFrame::eRequestImg_PostviewOut);
            pMainFrame = nullptr;
        }
    }); // async call

    // do MFNR
    {
        mfllAutoLog("doMfnr");
        m_pCore->doMfll();
        processMixedYuv();
        cleanUp();
    }

    return true;
}


void MFNRCtrler_v1_6::doCancel()
{
    FUNCTION_SCOPE;
    SCOPE_TIMER(__t0, __FUNCTION__);

    auto status = getExecutionStatus();

    {
        std::lock_guard<decltype(m_pCoreMx)> lk(m_pCoreMx);
        if ((status == ES_RUNNING) && m_pCore.get()) {
            m_pCore->doCancel(); // async call
        }
    }

    // Invalidate controller by BaseController
    BaseController::doCancel();
}


void MFNRCtrler_v1_6::addDataToMfnrCore(
        sp<IMfllImageBuffer>&   sourceImg,
        MfllMotionVector        mv,
        IMetadata*              pAppMeta,
        IMetadata*              pHalMeta,
        IImageBuffer*           pLcsoBuffer,
        IImageBuffer*           pRrzoBuffer
        )
{
    FUNCTION_SCOPE;
    SCOPE_TIMER(__t0, __FUNCTION__);

    std::deque<void*> metaset;
    metaset.push_back(static_cast<void*>(pAppMeta));
    metaset.push_back(static_cast<void*>(pHalMeta));
    metaset.push_back(static_cast<void*>(pLcsoBuffer));

    m_pCore->setSyncPrivateData(metaset);

    // enque LCSO to capturer for dump
    sp<IMfllImageBuffer> _lcsoImg = IMfllImageBuffer::createInstance();
    if (CC_LIKELY( pLcsoBuffer != nullptr )) {
        _lcsoImg->setImageBuffer(static_cast<void*>(pLcsoBuffer));
    }
    m_pCapturer->queueSourceLcsoImage(_lcsoImg);
    //
    // enque RRZO to capturer for dump and raw domain bss
    sp<IMfllImageBuffer> _rrzoImg = IMfllImageBuffer::createInstance();
    if (CC_LIKELY( pRrzoBuffer != nullptr )) {
        _rrzoImg->setImageBuffer(static_cast<void*>(pRrzoBuffer));
    }
    m_pCapturer->queueSourceRrzoImage(_rrzoImg);
    //
    m_pCapturer->queueSourceImage(sourceImg);
    m_pCapturer->queueGmv(mv);
}


void MFNRCtrler_v1_6::addDummyFrameToMfnrCore(
        sp<IMfllImageBuffer>& dummyFrame
        )
{
    FUNCTION_SCOPE;
    SCOPE_TIMER(__t0, __FUNCTION__);

    m_pCapturer->queueSourceImage(dummyFrame);
    m_pCapturer->queueGmv(MfllMotionVector());

    sp<IMfllImageBuffer> dummyRrzo = IMfllImageBuffer::createInstance("dummy");
    m_pCapturer->queueSourceRrzoImage(dummyRrzo);
}


MfllMotionVector
MFNRCtrler_v1_6::calMotionVector(const IMetadata* pHalMeta, size_t index)
{
    FUNCTION_SCOPE;
    SCOPE_TIMER(__t0, __FUNCTION__);

    MfllMotionVector    mv;
    MSize               rzoSize;
    MRect               p1ScalarRgn;
    MBOOL               ret = MTRUE;

    /* to get GMV info and the working resolution */
    ret = retrieveGmvInfo(pHalMeta, mv.x, mv.y, rzoSize);
    if (ret == MTRUE) {
        ret = tryGetMetadata<MRect>(
                pHalMeta,
                MTK_P1NODE_SCALAR_CROP_REGION,
                p1ScalarRgn);
    }

    /* if works, mapping it from rzoDomain to MfllCore domain */
    if (ret == MTRUE) {
        /* the first frame, set GMV as zero */
        if (index == 0) {
            mv.x = 0;
            mv.y = 0;
        }

        MY_LOGD3("GMV(x,y)=(%d,%d), unit based on resized RAW",
                mv.x, mv.y);

        MY_LOGD3("p1node scalar crop rgion (width): %d, gmv domain(width): %d",
                p1ScalarRgn.s.w, rzoSize.w);
        /**
         *  the cropping crops height only, not for width. Hence, just
         *  simply uses width to calculate the ratio.
         */
        float ratio =
            static_cast<float>(p1ScalarRgn.s.w)
            /
            static_cast<float>(rzoSize.w)
            ;
        MY_LOGD3("%s: ratio = %f", __FUNCTION__, ratio);

        // we don't need floating computing because GMV is formated
        // with 8 bits floating point
        mv.x *= ratio;
        mv.y *= ratio;

        /* normalization */
        mv.x = mv.x >> 8;
        mv.y = mv.y >> 8;

        // assume the ability of EIS algo, which may seach near by
        // N pixels only, so if the GMV is more than N pixels,
        // we clip it

        auto CLIP = [](int x, const int n) -> int {
            if (x < -n)     return -n;
            else if(x > n)  return n;
            else            return x;
        };

        // Hence we've already known that search region is 32 by 32
        // pixel based on RRZO domain, we can map it to full size
        // domain and makes clip if it's out-of-boundary.
        int c = static_cast<int>(ratio * 32.0f);
        mv.x = CLIP(mv.x, c);
        mv.y = CLIP(mv.y, c);

        MY_LOGD3("GMV'(x,y)=(%d,%d), unit: Mfll domain", mv.x, mv.y);
    }
    return mv;
}


void MFNRCtrler_v1_6::releaseSourceImages(int index)
{
    FUNCTION_SCOPE;
    SCOPE_TIMER(__t0, __FUNCTION__);

    {
        std::lock_guard<NSCam::SpinLock> __l(m_vUsedRequestFrameMx);

        if (CC_UNLIKELY(index >= static_cast<int>(m_vUsedRequestFrame.size()))) {
            // using SpinLock, do not print error message here.
            goto LB_ERR_OUT_OF_SIZE;
        }

        if (CC_LIKELY(m_vUsedRequestFrame[index].get())) {
            // partial release all source images
            const int begin = static_cast<int>(RequestFrame::eRequestImg_Src_Start);
            const int end   = static_cast<int>(RequestFrame::eRequestImg_Src_End);
            for (int i = begin; i < end; i++){
                auto reqId = static_cast<RequestFrame::eRequestImg>(i);
                m_vUsedRequestFrame[index]->releaseImageBuffer(reqId);
            }

            // also release extra output frame w/ errors
            const int begin2 = static_cast<int>(RequestFrame::eRequestImg_Out_Extra_Start);
            const int end2 = static_cast<int>(RequestFrame::eRequestImg_Out_Extra_End);
            for (int i = begin2; i < end2; i++) {
                auto reqId = static_cast<RequestFrame::eRequestImg>(i);
                m_vUsedRequestFrame[index]->markError(reqId);
                m_vUsedRequestFrame[index]->releaseImageBuffer(reqId);
            }
        }
    }
    return;

LB_ERR_OUT_OF_SIZE:
    MY_LOGW("release RequestFrame(%d) failed, frame not exist", index);
    return;
}


void MFNRCtrler_v1_6::processMixedYuv()
{
    FUNCTION_SCOPE;
    SCOPE_TIMER(__t0, __FUNCTION__);
    CAM_TRACE_CALL();

    if (m_pCore.get() == nullptr) {
        MY_LOGE("MFNR Core Lib is NULL");
        return;
    }

    if (m_pCore->getFrameCapturedNum() <= 0) {
        MY_LOGE("No frame has been captured by MFNR Core");
        return;
    }

    // process pResult to output YUV.
    std::shared_ptr<RequestFrame> pMainFrame = m_mainRequestFrame.lock();
    if (pMainFrame.get() == nullptr) {
        MY_LOGE("main request frame is NULL");
        return;
    }

    // get the index of the result frame
    auto resultIndex = static_cast<size_t>(m_pCore->getIndexByNewIndex(0));
    if (resultIndex < m_vUsedRequestFrame.size()) {
        if (!setMetadtaToResultFrame(
                    m_vUsedRequestFrame[resultIndex],
                    pMainFrame)) {
            MY_LOGE("Set resultIndex(%zu) failed", resultIndex);
        }
    }
    else {
        MY_LOGE("result index(%zu) is out of range(size=%zu)",
                resultIndex, m_vUsedRequestFrame.size());
    }

    auto appMetaIn     = pMainFrame->getMetadata(RequestFrame::eRequestMeta_InAppRequest);
    auto halMetaIn     = pMainFrame->getMetadata(RequestFrame::eRequestMeta_InHalP1);
    auto appMetaOut    = pMainFrame->getMetadata(RequestFrame::eRequestMeta_OutAppResult);
    auto halMetaOut    = pMainFrame->getMetadata(RequestFrame::eRequestMeta_OutHalResult);
    //
    auto pFrameYuvJpeg = pMainFrame->getImageBuffer(RequestFrame::eRequestImg_FullOut);
    auto pFrameYuvThub = pMainFrame->getImageBuffer(RequestFrame::eRequestImg_ThumbnailOut);
    auto pFrameWorking = pMainFrame->getImageBuffer(RequestFrame::eRequestImg_WorkingBufferOut);
    auto outOrientaion = pMainFrame->getOrientation(RequestFrame::eRequestImg_FullOut);
    auto requestNo     = pMainFrame->getRequestNo();

    if (appMetaOut == nullptr || halMetaOut == nullptr) {
        MY_LOGW("output metadata is NULL");
    }

    // restore both input/output app setting
    restoreAppSetting(appMetaOut);
    restoreAppSetting(appMetaIn);

    // store framecount for clear-zoom
    IMetadata::setEntry<MINT32>(
        halMetaIn,
        MTK_PLUGIN_PROCESSED_FRAME_COUNT,
        m_pCore->getFrameBlendedNum() + 1);
    IMetadata::setEntry<MINT32>(
        halMetaOut,
        MTK_PLUGIN_PROCESSED_FRAME_COUNT,
        m_pCore->getFrameBlendedNum() + 1);

    // append debug exif if need
    if (m_bNeedExifInfo) {
        auto&& pExifContainer = m_pCore->getExifContainer();
        if (pExifContainer.get()) {
            makesDebugInfo(pExifContainer->getInfoMap(), halMetaOut);
        }
        else {
            MY_LOGW("ask for debug exif but exif container is empty");
        }
    }

    // retrieve mixed YUV image buffer
    auto mfllImgBuf = m_pCore->retrieveBuffer(MfllBuffer_MixedYuv);
    if (mfllImgBuf.get() == nullptr) {
        MY_LOGE("mixed output buffer is NULL");
        return;
    }

    bool bIsIgnoredMix = false;
    MfllErr bErrOfMix  = MfllErr_Ok;
    do {
        std::lock_guard< std::mutex > __l( m_mfnrStageStatusLock );
        auto itr = m_mfnrStageStatus.find(EventType_Mixing);
        if (itr == m_mfnrStageStatus.end()) {
            MY_LOGW("cannot find Mixing result, assume it were failed");
            bIsIgnoredMix = true;
            bErrOfMix = MfllErr_UnexpectedError;
            break;
        }
        // where itr->second is std::pair<bool, MfllErr>,
        bIsIgnoredMix = itr->second.first;
        bErrOfMix = itr->second.second;
        MY_LOGD3("Mix status:(ignored,err)=(%d,%d)",
                bIsIgnoredMix ? 1 : 0,
                bErrOfMix
               );
    } while(0);

    IImageBuffer* pResult = static_cast<IImageBuffer*>(mfllImgBuf->getImageBuffer());
    if (pResult == nullptr) {
        MY_LOGE("IMfllImageBuffer is not NULL but result in it is NULL");
        return;
    }

    // handle yuv output
    if (m_pCore->getMixingBufferType() == MixYuvType_Base || bIsIgnoredMix || bErrOfMix) {
        bool ret = true;
        IImageBuffer* dst1 = (pFrameYuvJpeg == nullptr) ? pFrameWorking : pFrameYuvJpeg;
        IImageBuffer* dst2 = m_pCore->isThumbnailDone() ? nullptr : pFrameYuvThub;
        std::string str = (pFrameYuvJpeg == nullptr) ? "Process output working buffer."
                                                     : "Process output capture buffer.";
        bool needCrop = (pFrameYuvJpeg == nullptr) ? false : true;
        bool needClearZoom = (pFrameYuvJpeg == nullptr) ? false : true;
        ret  = handleYuvOutput(str,
                    pResult, dst1, dst2,
                    appMetaIn, halMetaIn,
                    getOpenId(), outOrientaion, requestNo, needClearZoom, needCrop, true,
                    halMetaOut // update EXIF of MDP
                    );
        if (!ret) MY_LOGE("handle yuv output fail.");
    }
}

void MFNRCtrler_v1_6::cleanUp()
{
    FUNCTION_SCOPE;
    SCOPE_TIMER(__t0, __FUNCTION__);
    CAM_TRACE_CALL();

    std::deque< std::shared_ptr<RequestFrame> > usedFrames;
    {
        std::lock_guard<NSCam::SpinLock> __l(m_vUsedRequestFrameMx);
        usedFrames = std::move(m_vUsedRequestFrame);
    }

    /* retrieve sync token if exists. */
    MINT64 syncToken = 0;
    if (usedFrames.size() > 0 && usedFrames.front().get()) {
        IMetadata::getEntry<MINT64>(
                usedFrames.front()->getMetadata(RequestFrame::eRequestMeta_InHalP1),
                MTK_PLUGIN_DETACT_JOB_SYNC_TOKEN,
                syncToken
                );
    }

    usedFrames.clear();
    m_pMfb = nullptr;
    m_pCapturer = nullptr;
    m_spNvramProvider = nullptr;

    // Trick: android::RefBase may have race condition during counting ref count
    //        to 0, it's safer to use a mutex to protect it.
    {
        std::lock_guard<decltype(m_pCoreMx)> _l(m_pCoreMx);
        m_pCore = nullptr;
    }

#if 0
    // 2018/02/07 Rynn: disable for saving memory
    // clear token asap but memory footprint may be higher
    if (syncToken > 0)
        IDetachJobManager::getInstance()->unregisterToken(syncToken);
#endif
}

bool MFNRCtrler_v1_6::isEnabledEarlyPostview() const
{
    if (m_pCore->isSingleFrame()) {
        MY_LOGD("disable early postview due to single frame processing");
        return false;
    }

    if (MFLL_FLOW_EARLY_POSTVIEW == 0) {
        MY_LOGD("disable early postview due to MFLL_FLOW_EARLY_POSTVIEW = 0");
        return false;
    }

    if (getCaptureNum() <= 1) {
        MY_LOGD("disable early postview due to catpure frame number <= 1");
        return false;
    }

    if (m_effectMode != MTK_CONTROL_EFFECT_MODE_OFF) {
        MY_LOGD("disable early postview due to effect mode is not OFF");
        return false;
    }

    return true;
}

// ----------------------------------------------------------------------------
// Events
// ----------------------------------------------------------------------------
void MFNRCtrler_v1_6::onEvent(
        enum EventType t,
        MfllEventStatus_t &status,
        void *mfllCore,
        void *param1 __attribute__((unused)) /* = NULL */,
        void *param2 __attribute__((unused)) /* = NULL */
        )
{
    IMfllCore *c = reinterpret_cast<IMfllCore*>(mfllCore);

    /* get available frame count */
    const int frameCount = getCaptureNum() - getDroppedFrameNum();

    auto ignoreCapture = [this, &param1, &status, frameCount]()->void {
        int index = (int)(long long)param1;
        /* frame is not enought for encoding */
        if (index >= frameCount) {
            MY_LOGE("frame %d has been ignored for encoding YUV, MEMC, blend", index);
            status.ignore = 1;
        }
    };

    auto ignoreBss = [this, &c, &status, frameCount, param1]()->void {
        if (frameCount <= 1) {
            MY_LOGD("frames size(%d) <= 1, ignore BSS", frameCount);
            status.ignore = 1;
            return;
        }

        /* update bss frame count to the number of available frames */
        int *pBssFrameCount = reinterpret_cast<int*>(param1);
        *pBssFrameCount = frameCount;
    };

    switch (t) {
    case EventType_CaptureYuvQ:
        if (isEnabledEarlyPostview()) {
            if (param1 == 0) {
                // early callback, set postview buffer to MfllCore for generating
                // it during stage BFBLD
                std::shared_ptr<RequestFrame> pMainFrame = m_mainRequestFrame.lock();
                if (CC_UNLIKELY( pMainFrame.get() == nullptr )) {
                    MY_LOGE("disable early postview due to null main request frame");
                }
                else {
                    __set_postview_buffer_to_core(
                            c,
                            pMainFrame->getImageBuffer(RequestFrame::eRequestImg_PostviewOut),
                            pMainFrame->getMetadata(RequestFrame::eRequestMeta_InAppRequest),
                            pMainFrame->getMetadata(RequestFrame::eRequestMeta_InHalP1),
                            getOpenId()
                            );
                }
            }
        }
        ignoreCapture();
        break;

    case EventType_CaptureEis:
    case EventType_MotionCompensation:
    case EventType_Blending:
        ignoreCapture();
        break;
    case EventType_MotionEstimation:
        ignoreCapture();
        if (status.ignore == 0) {
            std::shared_ptr<RequestFrame> pMainFrame = m_mainRequestFrame.lock();
            if (pMainFrame.get() == nullptr) {
                MY_LOGW("main request frame doesn't exists, no need to wait sync token");
                break;
            }

            MINT64 syncToken = 0;
            /* to check MTK_PLUGIN_DETACT_JOB_SYNC_TOKEN */
            IMetadata::getEntry<MINT64>(
                    pMainFrame->getMetadata(RequestFrame::eRequestMeta_InHalP1),
                    MTK_PLUGIN_DETACT_JOB_SYNC_TOKEN,
                    syncToken);
            if (syncToken > 0) {
                MY_LOGI("wait syncToken (%" PRId64 ") [+]", syncToken);
                IDetachJobManager::getInstance()->waitTrigger(syncToken, INT_MAX);
                MY_LOGI("wait syncToken (%" PRId64 ") [-]", syncToken);
            }
            else {
                MY_LOGD3("MTK_PLUGIN_DETACT_JOB_SYNC_TOKEN not exists, no need to wait sync token");
            }
        }
        break;
    case EventType_Bss:
        ignoreBss();
        break;

    case EventType_EncodeYuvGolden:
        if (isEnabledEarlyPostview() == false) {
            std::shared_ptr<RequestFrame> pMainFrame = m_mainRequestFrame.lock();
            if (CC_UNLIKELY( pMainFrame.get() == nullptr )) {
                MY_LOGE("disable early postview due to null main request frame");
            }
            else {
                __set_postview_buffer_to_core(
                        c,
                        pMainFrame->getImageBuffer(RequestFrame::eRequestImg_PostviewOut),
                        pMainFrame->getMetadata(RequestFrame::eRequestMeta_InAppRequest),
                        pMainFrame->getMetadata(RequestFrame::eRequestMeta_InHalP1),
                        getOpenId()
                        );
            }
        }
        break;
    default:;
    }// switch;
}


void MFNRCtrler_v1_6::doneEvent(
        enum EventType      t,
        MfllEventStatus_t&  status      __attribute__((unused)),
        void*               mfllCore,
        void*               param1      __attribute__((unused)) /* = NULL */,
        void*               param2      __attribute__((unused)) /* = NULL */
        )
{
    /* get available frame count */
    const int frameCount = getCaptureNum() - getDroppedFrameNum();
    IMfllCore *c = reinterpret_cast<IMfllCore*>(mfllCore);
    int index = (int)(long long)param1;

    auto freeUnusedRawBuffers = [this, &c, &status, frameCount]()->void {
        MY_LOGD3("encode QYuv and Bss done, free the unused Raw buffers");

        if (frameCount <= 0) {
            MY_LOGW("frameCount=%d", frameCount);
        }
        else {
            int baseRawIdx = static_cast<int>(c->getIndexByNewIndex(0));
            MY_LOGD3("bss index=%d", baseRawIdx);
            MY_LOGD3("frameCount=%d", frameCount);
            /* release all RAWs except base RAW */
            for (int i = 0; i < frameCount; i++) {
                if (baseRawIdx == i)
                    continue;

                releaseSourceImages(i);
            }
        }

        onNextCaptureReady();
    };


    switch (t) {
    case EventType_Capture:

        break;

    case EventType_Bss:
        do {
            std::lock_guard<std::mutex> __l(m_mfnrStageStatusLock);
            m_mfnrStageStatus[EventType_Bss] = std::make_pair(
                    status.ignore, status.err);
        } while(0);

        do {
            std::lock_guard< std::mutex > __l( m_mfnrStageStatusLock );
            auto itr = m_mfnrStageStatus.find(EventType_CaptureYuvQ);
            if (itr != m_mfnrStageStatus.end()) {
                freeUnusedRawBuffers();
            }
        } while(0);

        break;

    case EventType_EncodeYuvBase:

        break;

    case EventType_EncodeYuvGolden:
        if (isEnabledEarlyPostview() == false) {
            c->releaseBuffer(mfll::MfllBuffer_PostviewYuv); // release buffer
            std::lock_guard<std::mutex> __l(m_postViewReadyPack.locker);
            m_postViewReadyPack.isReady = true;
            m_postViewReadyPack.cond.notify_one();

        }

        if (frameCount <= 0) {
            /* no frames, no result */
            return;
        }
        else {
            /* release all RAWs */
            for (int i = 0; i < frameCount; i++) {
                releaseSourceImages(i);
            }
        }
        break;
    case EventType_CaptureYuvQ:
        if (isEnabledEarlyPostview()) {
            // Early postview
            if (param1 == 0) {
                /* to remove postview YUV ownership of MfllCore first */
                c->releaseBuffer(mfll::MfllBuffer_PostviewYuv);
                std::lock_guard<std::mutex> __l(m_postViewReadyPack.locker);
                m_postViewReadyPack.isReady = true;
                m_postViewReadyPack.cond.notify_one();
            }
        }

        if (index == (frameCount - 1)) {
            MY_LOGD3("all raw buffers have been encode as QYuv");
            do {
                std::lock_guard<std::mutex> __l(m_mfnrStageStatusLock);
                m_mfnrStageStatus[EventType_CaptureYuvQ] = std::make_pair(
                        status.ignore, status.err);
            } while(0);

            do {
                std::lock_guard< std::mutex > __l( m_mfnrStageStatusLock );
                auto itr = m_mfnrStageStatus.find(EventType_Bss);
                if (itr != m_mfnrStageStatus.end()) {
                    freeUnusedRawBuffers();
                }
            } while(0);
        }
        else{
            MY_LOGD3("raw[%d] has been encode as QYuv", index);
        }
        break;

    case EventType_Mixing:
        do {
            std::lock_guard<std::mutex> __l(m_mfnrStageStatusLock);
            m_mfnrStageStatus[EventType_Mixing] = std::make_pair(
                    status.ignore, status.err);
        } while(0);
        break;

    default:
        break;
    }; // switch
}

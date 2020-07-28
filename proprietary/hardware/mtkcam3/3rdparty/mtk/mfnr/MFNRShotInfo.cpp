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
 * MediaTek Inc. (C) 2018. All rights reserved.
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
#define LOG_TAG "MFNRProvider"
static const char* __CALLERNAME__ = LOG_TAG;

//
#include <mtkcam/utils/std/ULog.h>
#include <mtkcam/utils/exif/DebugExifUtils.h>
#include <mtkcam/aaa/IHal3A.h>
#include <mtkcam/drv/IHalSensor.h>
//
#include <custom/debug_exif/IDebugExif.h>
//
#include <stdlib.h>
#include <utils/Errors.h>
#include <utils/List.h>
#include <utils/RefBase.h>
#include <sstream>
//

#include "MFNRShotInfo.h"

//
using namespace NSCam;
using namespace plugin;
using namespace android;
using namespace mfll;
using namespace std;
using namespace NS3Av3;
/******************************************************************************
 *
 ******************************************************************************/
CAM_ULOG_DECLARE_MODULE_ID(MOD_LIB_MFNR);
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
//
#define ASSERT(cond, msg)           do { if (!(cond)) { printf("Failed: %s\n", msg); return; } }while(0)


#define MY_DBG_COND(level)          __builtin_expect( m_dbgLevel >= level, false )
#define MY_LOGD3(...)               do { if ( MY_DBG_COND(3) ) MY_LOGD(__VA_ARGS__); } while(0)


#define __DEBUG // enable debug


#ifdef __DEBUG
#include <mtkcam/utils/std/Trace.h>
#define FUNCTION_TRACE()                            CAM_ULOGM_FUNCLIFE()
#define FUNCTION_TRACE_NAME(name)                   CAM_ULOGM_TAGLIFE(name)
#define FUNCTION_TRACE_BEGIN(name)                  CAM_ULOGM_TAG_BEGIN(name)
#define FUNCTION_TRACE_END()                        CAM_ULOGM_TAG_END()
#define FUNCTION_TRACE_ASYNC_BEGIN(name, cookie)    CAM_TRACE_ASYNC_BEGIN(name, cookie)
#define FUNCTION_TRACE_ASYNC_END(name, cookie)      CAM_TRACE_ASYNC_END(name, cookie)
#else
#define FUNCTION_TRACE()
#define FUNCTION_TRACE_NAME(name)
#define FUNCTION_TRACE_BEGIN(name)
#define FUNCTION_TRACE_END()
#define FUNCTION_TRACE_ASYNC_BEGIN(name, cookie)
#define FUNCTION_TRACE_ASYNC_END(name, cookie)
#endif

#ifdef __DEBUG
#include <memory>
#define FUNCTION_SCOPE \
auto __scope_logger__ = [](char const* f)->std::shared_ptr<const char>{ \
    CAM_ULOGMD("(%d)[%s] + ", ::gettid(), f); \
    return std::shared_ptr<const char>(f, [](char const* p){CAM_ULOGMD("(%d)[%s] -", ::gettid(), p);}); \
}(__FUNCTION__)
#else
#define FUNCTION_SCOPE
#endif


#define MFNR_FLASH_DUMMY_FRAMES_FRONT       2
#define MFNR_FLASH_DUMMY_FRAMES_BACK        2

#if defined(MTKCAM_LOG_LEVEL_DEFAULT) && (MTKCAM_LOG_LEVEL_DEFAULT <= 2)
#define MFNR_WAIT_DONE_TIMEOUT              5000
#else
#define MFNR_WAIT_DONE_TIMEOUT              10000
#endif



#if 1
// LMV
#include <mtkcam3/feature/lmv/lmv_ext.h>
#include <mtkcam/utils/hw/HwInfoHelper.h>
/**
 *  The confidence threshold of GMV, which means if the confidence is not
 *  enough then the GMV won't be applied.
 */
#define MFC_GMV_CONFX_TH 25
#define MFC_GMV_CONFY_TH 25

MBOOL
retrieveGmvInfo(const IMetadata* pMetadata, int& x, int& y, MSize& size)
{
    MBOOL  ret = MTRUE;
    MSize  rzoSize;
    MUINT  entryCount = 0;
    IMetadata::IEntry entry;
    struct __confidence{
        MINT32 x;
        MINT32 y;
        __confidence() : x(0), y(0) {}
    } confidence;

    /* get size first */
    ret = IMetadata::getEntry<MSize>(pMetadata, MTK_P1NODE_RESIZER_SIZE, rzoSize);
    if (ret != MTRUE) {
        MY_LOGE("%s: cannot get rzo size", __FUNCTION__);
        goto lbExit;
    }

    entry = pMetadata->entryFor(MTK_EIS_REGION);

    /* check if a valid MTK_EIS_REGION */
    entryCount = entry.count();
    if (LMV_REGION_INDEX_CONFX >= entryCount || LMV_REGION_INDEX_CONFY >= entryCount ||
        LMV_REGION_INDEX_GMVX >= entryCount || LMV_REGION_INDEX_GMVY >= entryCount)
    {
        MY_LOGE("%s: entry is not a valid LMV_REGION, size = %d",
                __FUNCTION__,
                entry.count());
        ret = MFALSE;
        goto lbExit;
    }

    /* read confidence */
    confidence.x = static_cast<MINT32>(entry.itemAt(LMV_REGION_INDEX_CONFX, Type2Type<MINT32>()));
    confidence.y = static_cast<MINT32>((MINT32)entry.itemAt(LMV_REGION_INDEX_CONFY, Type2Type<MINT32>()));

    /* to read GMV if confidence is enough */
    if (confidence.x > MFC_GMV_CONFX_TH) {
        x = entry.itemAt(LMV_REGION_INDEX_GMVX, Type2Type<MINT32>());
    }

    if (confidence.y > MFC_GMV_CONFY_TH) {
        y = entry.itemAt(LMV_REGION_INDEX_GMVY, Type2Type<MINT32>());
    }

    size = rzoSize;

    MY_LOGD("LMV info conf(x,y) = (%d, %d), gmv(x, y) = (%d, %d)",
            confidence.x, confidence.y, x, y);

lbExit:
    return ret;
}
#endif

/******************************************************************************
*
******************************************************************************/
MFNRShotInfo::MFNRShotInfo(
        int uniqueKey,
        int sensorId,
        MfllMode mfllMode,
        int realIso,
        int exposureTime,
        bool isFlashOn,
        bool isIspHidlProcess)
    : m_uniqueKey(uniqueKey)
    , m_openId(sensorId)
    , m_mfbMode(mfllMode)
    , m_realIso(realIso)
    , m_shutterTime(exposureTime)
    , m_finalRealIso(0)
    , m_NvramIndex(-1)
    , m_finalShutterTime(0)
    , m_captureNum(0)
    , m_blendNum(0)
    , m_realBlendNum(0)
    , m_bDoMfb(false)
    , m_bForceMfnr(false)
    , m_bFullSizeMc(false)
    , m_bFlashOn(isFlashOn)
    , m_bFlashMfllSupport(false)
    , m_dummyFrame(0)
    , m_delayFrame(0)
    , m_mulitCamFeatureMode(-1)
    , m_bDoDownscale(false)
    , m_downscaleRatio(0)
    , m_downscaleDividend(0)
    , m_downscaleDivisor(0)
    , m_dbgLevel(0)
    , m_droppedFrameNum(0)
    , m_bNeedExifInfo(false)
    , m_bFired(false)
    , m_bInit(false)
    , m_pCore(nullptr)
    , m_pCapturer(nullptr)
    , m_pMfb(nullptr)
    , m_spNvramProvider(nullptr)
    , m_mainframe_halMetaIn(nullptr)
    , m_mainframe_halMetaOut(nullptr)
    , m_inputYuvFmt(InputYuvFmt_Nv12)
    , m_doCancelStatus(false)
{
    FUNCTION_SCOPE;

    //if( !isIspHidlProcess)
    {
        ExposureUpdateMode __3aUpdateMode = ExposureUpdateMode::CURRENT;

        if (mfll::isAisMode(m_mfbMode)) {
            // if it's AIS, update the finial capture param by AIS pipeline
            __3aUpdateMode = ExposureUpdateMode::AIS;
        }

        updateCurrent3A(__3aUpdateMode);
    }

    m_dbgLevel = mfll::MfllProperty::getDebugLevel();
}

MFNRShotInfo::~MFNRShotInfo()
{
    FUNCTION_SCOPE;
}

void MFNRShotInfo::updateMfllStrategy()
{
    FUNCTION_SCOPE;
    FUNCTION_TRACE();

    bool bIsAis = mfll::isAisMode(getMfbMode());

    MfllStrategyConfig_t strategyCfg;

#define __DEFAULT_CAPTURE_NUM__ 4
    int captureNum = __DEFAULT_CAPTURE_NUM__; // makes default 4 frame
    int blendNum = __DEFAULT_CAPTURE_NUM__;
    int dummyNum = 0;
    int delayNum = getDelayFrameNum();
#undef  __DEFAULT_CAPTURE_NUM__

    int suggestedIso = getFinalIso(); // current iso from PLINE
    int suggestedExp = getFinalShutterTime();

    sp<IMfllStrategy> pStrategy = IMfllStrategy::createInstance();
    //reload
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
        if (CC_UNLIKELY(!m_bFlashMfllSupport && getIsFlashOn())) {
            MY_LOGW("Not support MFNR-Flash feature");
            goto LB_EXIT;
        }

        if (CC_UNLIKELY(m_spNvramProvider->init(nvramCfg) != MfllErr_Ok)) {
            MY_LOGE("init IMfllNvram failed");
            goto LB_EXIT;
        }

        // NVRAM output
        setNvramIndex(nvramCfg.iQueryIndex_Bss);

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
#ifdef MFB_MAX_FRAME
        if (captureNum > MFB_MAX_FRAME) {
            MY_LOGE("Tuning Error: CaptureNum:%d is > MFB_MAX_FRAME:%d, set to %d", captureNum, MFB_MAX_FRAME, MFB_MAX_FRAME);
            captureNum = MFB_MAX_FRAME;
        }
        if (blendNum > MFB_MAX_FRAME) {
            MY_LOGE("Tuning Error: BlendNum:%d is > MFB_MAX_FRAME:%d, set to %d", blendNum, MFB_MAX_FRAME, MFB_MAX_FRAME);
            blendNum = MFB_MAX_FRAME;
        }
#endif
        suggestedExp = static_cast<int>(strategyCfg.exp);
        suggestedIso = static_cast<int>(strategyCfg.iso);
    }

    if (getIsFlashOn()) {
        //TODO: need to refine it someday.
        MY_LOGD("Flash MFNR tricky patch: force set to 3 captureNum");
        captureNum = 3;
        blendNum = 3;
        dummyNum = MFNR_FLASH_DUMMY_FRAMES_FRONT;
        delayNum = std::max(delayNum, MFNR_FLASH_DUMMY_FRAMES_BACK);
    }

LB_EXIT:
    // update again
    setFinalIso(suggestedIso);
    setFinalShutterTime(suggestedExp);
    setCaptureNum(captureNum);
    setBlendNum(blendNum);
    setDummyFrameNum(dummyNum);
    setDelayFrameNum(delayNum);
    setEnableMfnr(strategyCfg.enableMfb);
    setFullSizeMc(strategyCfg.isFullSizeMc);
    setEnableDownscale(strategyCfg.enableDownscale);
    setDownscaleRatio(strategyCfg.downscaleRatio);
    setDownscaleDividend(strategyCfg.downscaleDividend);
    setDownscaleDivisor(strategyCfg.downscaleDivisor);
    setPostrefineNr(strategyCfg.postrefine_nr);
    setPostrefineMfb(strategyCfg.postrefine_mfb);

if (m_mulitCamFeatureMode == MTK_MULTI_CAM_FEATURE_MODE_VSDOF) {
    MY_LOGD("disable downscale in multicam VSDoF feature mode");
    setEnableDownscale(0);
    setDownscaleRatio(2);
    setDownscaleDividend(16);
    setDownscaleDivisor(16);
}

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
    MY_LOGD3("dummy = %d", getDummyFrameNum());
    MY_LOGD3("delay = %d", getDelayFrameNum());
    MY_LOGD3("fmc = %d", getIsFullSizeMc() ? 1 : 0);
    MY_LOGD3("mfb = %d", getIsEnableMfnr() ? 1 : 0);
    MY_LOGD3("downscale(enabled:%d, ratio:%d, dividend:%d, divisor:%d)", getEnableDownscale(), getDownscaleRatio(), getDownscaleDividend(), getDownscaleDivisor());
    MY_LOGD3("postrefine(nr:%d, mfb:%d)", getPostrefineNr(), getPostrefineMfb());
    MY_LOGD3("nvramIdx = %d", getNvramIndex());
#endif
}

bool MFNRShotInfo::initMfnrCore()
{
    FUNCTION_SCOPE;
    FUNCTION_TRACE();

    std::lock_guard<decltype(m_pCoreMx)> _l(m_pCoreMx);

    if (m_bFired) {
        MY_LOGE("MFNR has been fired, cannot fire twice");
        return false;
    }

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
    if (m_pCore.get() == nullptr)
        m_pCore = IMfllCore::createInstance();
    m_pCapturer = IMfllCapturer::createInstance(IMfllCapturer::Type::YuvCapturer);
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
    //m_pCapturer->setMfllCore(m_pCore.get());
    m_pCapturer->setShotMode(getMfbMode());
    m_pCapturer->setPostNrType(NoiseReductionType_None);

    // mfb
    m_pMfb->init(getOpenId());
    m_pMfb->setMfllCore(m_pCore.get());
    m_pMfb->setShotMode(getMfbMode());
    m_pMfb->setPostNrType(NoiseReductionType_None);

    // core
    MfllBypassOption byPassOption;

    byPassOption.bypassCapture = 1;
#if SUPPORT_YUV_BSS
    byPassOption.bypassBss = 0;
#else
    byPassOption.bypassBss = 1;
#endif
    for (int i = 0; i < MFLL_MAX_FRAMES; i++) {
        byPassOption.bypassAllocRawBuffer[i] = 1;
        byPassOption.bypassAllocRrzoBuffer[i] = 1;
        byPassOption.bypassAllocQyuvBuffer[i] = 1;
        byPassOption.bypassEncodeQYuv[i] = 1;
    }

    if (m_mulitCamFeatureMode == MTK_MULTI_CAM_FEATURE_MODE_VSDOF) {
        MY_LOGE("TODO: please fix it. <MFNR3.0 should apply 10bit YUV>");
        m_pCore->setInputYuvFormat(getInputYuvFmt());
    }

    m_pCore->setBypassOption(byPassOption);
    if (getCaptureNum() <= 1)
        m_pCore->setBypassOptionAsSingleFrame();

    m_pCore->setDownscaleInfo(
            getEnableDownscale(), getDownscaleRatio(), getDownscaleDividend(), getDownscaleDivisor());

    m_pCore->setCaptureResolution(
            getSizeSrc().w, getSizeSrc().h);

    m_pCore->setCapturer(m_pCapturer);
    m_pCore->setMfb(m_pMfb);
    m_pCore->setNvramProvider(m_spNvramProvider);
    //m_pCore->registerEventListener(new ListenerImp4Ctrler(this)); // register event listener

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

unsigned int MFNRShotInfo::getMfnrCoreVersion(void)
{
    FUNCTION_SCOPE;
    FUNCTION_TRACE();

    std::lock_guard<decltype(m_pCoreMx)> _l(m_pCoreMx);

    if (m_pCore.get() == nullptr)
        m_pCore = IMfllCore::createInstance();

    if (m_pCore.get() == nullptr) {
        MY_LOGE("create MFNR Core Library failed");
        return 0;
    }

    return m_pCore->getVersion();
}

std::string MFNRShotInfo::getMfnrCoreVersionString(void)
{
    FUNCTION_SCOPE;
    FUNCTION_TRACE();

    std::lock_guard<decltype(m_pCoreMx)> _l(m_pCoreMx);

    if (m_pCore.get() == nullptr)
        m_pCore = IMfllCore::createInstance();

    if (m_pCore.get() == nullptr) {
        MY_LOGE("create MFNR Core Library failed");
        return 0;
    }

    return m_pCore->getVersionString();
}

bool MFNRShotInfo::execute()
{
    FUNCTION_SCOPE;
    FUNCTION_TRACE();

    std::lock_guard<decltype(m_pCoreMx)> _l(m_pCoreMx);

    if (m_bFired) {
        MY_LOGE("MFNR has been fired, cannot fire twice");
        return false;
    }

    if (m_pCore.get() == nullptr) {
        MY_LOGE("MFNR Core is NULL");
        return false;
    }

    std::lock_guard<std::mutex> __l(m_futureExeMx);

    bool bStart = false; // flag represents if it's need to start a job

    // std::shared_future::valid
    if (m_futureExe.valid()) {
        switch (m_futureExe.wait_for(std::chrono::seconds(0))) {
        case std::future_status::deferred: // Job hasn't been executed yet.
        case std::future_status::ready: // Job has finished.
            bStart = true; // It's ok to start a new job.
            break;
        case std::future_status::timeout: // Job is still in executing, return false.
            return false;
        default:;
        }
    }
    else {
        bStart = true;
    }

    if (bStart) {
        // start a new job
        auto fu = std::async(std::launch::async, [this]() mutable {
            auto r = this->doMfnr();
            return r;
        });

        m_futureExe = std::shared_future<bool>(std::move(fu));

        return true;
    }
    return false;

}

bool MFNRShotInfo::waitExecution(intptr_t* result /* = nullptr */)
{
    FUNCTION_SCOPE;
    FUNCTION_TRACE();

    std::shared_future<bool> t1;
    {
        std::lock_guard<std::mutex> __l(m_futureExeMx);
        t1 = m_futureExe;
    }

    if (t1.valid()) {
        if (result) (*result) = t1.get();
        else        t1.wait();
        return true;
    }
    else {
        return false;
    }

    return false;
}

MFNRShotInfo::ExecutionStatus
MFNRShotInfo::getExecutionStatus() const
{
    FUNCTION_SCOPE;
    FUNCTION_TRACE();

    std::lock_guard<std::mutex> __l(m_futureExeMx);
    if (CC_LIKELY(m_futureExe.valid())) {
        switch (m_futureExe.wait_for(std::chrono::seconds(0))) {
        case std::future_status::deferred:
            return ES_NOT_STARTED_YET;
        case std::future_status::timeout:
            return ES_RUNNING;
        case std::future_status::ready:
            return ES_READY;
        }
    }
    return ES_NOT_STARTED_YET;
}

void MFNRShotInfo::doCancel()
{
    FUNCTION_SCOPE;
    FUNCTION_TRACE();

    std::lock_guard<decltype(m_pCoreMx)> _l(m_pCoreMx);
    if (m_pCore.get()) {
        m_pCore->doCancel(); // async call
    }
}

void MFNRShotInfo::addDataToMfnrCore(
        sp<IMfllImageBuffer>&   sourceImg,
        sp<IMfllImageBuffer>&   quarterImg,
        MfllMotionVector        mv,
        IMetadata*              pAppMeta,
        IMetadata*              pHalMeta,
        IMetadata*              pHalMetaDynamic,
        MINT32                  requestNo)
{
    FUNCTION_SCOPE;
    FUNCTION_TRACE();

    if (requestNo == 0)
        configMfllCore(pHalMeta, pHalMetaDynamic);

    std::deque<void*> metaset;
    metaset.push_back(static_cast<void*>(pAppMeta));
    metaset.push_back(static_cast<void*>(pHalMeta));
    metaset.push_back(static_cast<void*>(nullptr));

    m_pCore->setSyncPrivateData(metaset);

    m_pCore->queueYuvCapture(requestNo, sourceImg, quarterImg, mv);
}

bool MFNRShotInfo::setOutputBufToMfnrCore(sp<IMfllImageBuffer>& ouputImg)
{
    FUNCTION_SCOPE;
    FUNCTION_TRACE();
    return (MfllErr_Ok == m_pCore->setMixingBuffer(MixYuvType_Working, ouputImg));
}

MfllMotionVector
MFNRShotInfo::calMotionVector(const IMetadata* pHalMeta, size_t index)
{
    FUNCTION_SCOPE;
    FUNCTION_TRACE();

    MfllMotionVector    mv;
    MSize               rzoSize;
    MRect               p1ScalarRgn;
    MBOOL               ret = MTRUE;

    /* to get GMV info and the working resolution */
    ret = retrieveGmvInfo(pHalMeta, mv.x, mv.y, rzoSize);
    if (ret == MTRUE) {
        ret = IMetadata::getEntry<MRect>(
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

void MFNRShotInfo::updateCurrent3A(ExposureUpdateMode mode)
{
    FUNCTION_SCOPE;
    FUNCTION_TRACE();

    std::unique_ptr <
                    IHal3A,
                    std::function<void(IHal3A*)>
                    > hal3a
            (
                MAKE_Hal3A(m_openId, __CALLERNAME__),
                [](IHal3A* p){ if (p) p->destroyInstance(__CALLERNAME__); }
            );

    if (hal3a.get() == nullptr) {
        MY_LOGE("create IHal3A instance failed");
        m_bFlashMfllSupport = false;
        return;
    }

    typedef std::mutex T_LOCKER;
    static T_LOCKER _locker;
    ExpSettingParam_T   expParam;
    CaptureParam_T      capParam;
    MUINT32 delayedFrames = 0;

    {
        std::lock_guard<T_LOCKER> _l(_locker);
        // get the current 3A
        hal3a->send3ACtrl(E3ACtrl_GetExposureInfo,  (MINTPTR)&expParam, 0);  // for update info in ZSD mode
        hal3a->send3ACtrl(E3ACtrl_GetExposureParam, (MINTPTR)&capParam, 0);
        hal3a->send3ACtrl(E3ACtrl_GetCaptureDelayFrame, reinterpret_cast<MINTPTR>(&delayedFrames), 0);
        m_bFlashMfllSupport = hal3a->send3ACtrl(E3ACtrl_ChkMFNRFlash, 0, 0);
    }

    MY_LOGD("3A capture delayed frames(%u)", delayedFrames);
    setDelayFrameNum(delayedFrames);

    MY_LOGD("default strategy config (iso,exp)=(%d,%d)", m_realIso, m_shutterTime);
    MY_LOGD("current capture params (iso,exp)=(%d,%d)", capParam.u4RealISO, capParam.u4Eposuretime);
    if (m_realIso == 0 || m_shutterTime == 0) {
        m_realIso = capParam.u4RealISO;
        m_shutterTime = capParam.u4Eposuretime;
        MY_LOGW("default strategy config is invalid, "\
                "config to current capture params (iso,exp)=(%d,%d)",
                m_realIso, m_shutterTime);
    }

    switch (mode) {
    case ExposureUpdateMode::AIS:
        {
            std::lock_guard<T_LOCKER> _l(_locker);
            // get the AIS specific 3A
            hal3a->send3ACtrl(E3ACtrl_EnableAIS, 1, 0);
            hal3a->send3ACtrl(E3ACtrl_GetExposureInfo,  (MINTPTR)&expParam, 0);  // for update info in ZSD mode
            hal3a->send3ACtrl(E3ACtrl_GetExposureParam, (MINTPTR)&capParam, 0);
            hal3a->send3ACtrl(E3ACtrl_EnableAIS, 0, 0);
        }
        m_finalRealIso = capParam.u4RealISO;
        m_finalShutterTime = capParam.u4Eposuretime;
        MY_LOGD("AIS P-line config (iso,exp)=(%d,%d)", m_finalRealIso, m_finalShutterTime);
        break;

    case ExposureUpdateMode::MFNR:
    default:
        m_finalRealIso = m_realIso;
        m_finalShutterTime = m_shutterTime;
        break;
    }
}


void MFNRShotInfo::configMfllCore(IMetadata* pHalMeta, IMetadata* pHalMetaDynamic)
{
    FUNCTION_SCOPE;
    FUNCTION_TRACE();

    // set middleware info to MFNR Core Lib for debug dump
    {
        MfllMiddlewareInfo_t __middlewareInfo =
            m_pCore->getMiddlewareInfo();

        struct T {
            MINT32 val;
            MBOOL result;
            T() : val(-1), result(MFALSE) {};
        } requestKey, frameKey, iso, dummyUniqueKey, dummyRequestKey, dummyFrameKey, magicNum;

        struct U {
            MINT64 val;
            MBOOL result;
            U() : val(-1), result(MFALSE) {};
        } exp;

        requestKey.result   = NSCam::IMetadata::getEntry<MINT32>(pHalMeta, MTK_PIPELINE_REQUEST_NUMBER, requestKey.val);
        frameKey.result     = NSCam::IMetadata::getEntry<MINT32>(pHalMeta, MTK_PIPELINE_FRAME_NUMBER, frameKey.val);
        iso.result          = NSCam::IMetadata::getEntry<MINT32>(pHalMetaDynamic, MTK_SENSOR_SENSITIVITY, iso.val);
        exp.result          = NSCam::IMetadata::getEntry<MINT64>(pHalMetaDynamic, MTK_SENSOR_EXPOSURE_TIME, exp.val);
        magicNum.result     = NSCam::IMetadata::getEntry<MINT32>(pHalMeta, MTK_P1NODE_PROCESSOR_MAGICNUM, magicNum.val);


        MY_LOGD3("%s:=========================", __FUNCTION__);
        MY_LOGD3("%s: Get first requestKey -> %d, requestKey->: %" PRIi32, __FUNCTION__, requestKey.result, requestKey.val);
        MY_LOGD3("%s: Get first frameKey -> %d, frameKey->: %" PRIi32, __FUNCTION__, frameKey.result, frameKey.val);
        MY_LOGD3("%s: Get Iso -> %d, iso->: %" PRIi32, __FUNCTION__, iso.result, iso.val);
        MY_LOGD3("%s: Get Exp -> %d, exp->: %" PRIi64, __FUNCTION__, exp.result, exp.val);
        MY_LOGD3("%s: Get magicNum -> %d, magicNum->: %" PRIi32, __FUNCTION__, magicNum.result, magicNum.val);
        MY_LOGD3("%s:=========================", __FUNCTION__);


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
        __middlewareInfo.uniqueKey  = m_uniqueKey;
        __middlewareInfo.requestKey = static_cast<int>(requestKey.val);
        __middlewareInfo.frameKey   = static_cast<int>(frameKey.val);
        __middlewareInfo.iso        = static_cast<int>(iso.val);
        __middlewareInfo.shutterUs  = static_cast<int>(exp.val);

        dummyUniqueKey.result       = NSCam::IMetadata::getEntry<MINT32>(pHalMeta, MTK_PIPELINE_DUMP_UNIQUE_KEY, dummyUniqueKey.val);
        if (dummyUniqueKey.result) {
            MY_LOGD("Update dumpinfo - uniqueKey due to MTK_PIPELINE_DUMP_UNIQUE_KEY is set.");
            __middlewareInfo.uniqueKey  = static_cast<int>(dummyUniqueKey.val);
            //
            dummyRequestKey.result  = NSCam::IMetadata::getEntry<MINT32>(pHalMeta, MTK_PIPELINE_DUMP_REQUEST_NUMBER, dummyRequestKey.val);
            if (dummyRequestKey.result) {
                __middlewareInfo.requestKey = static_cast<int>(dummyRequestKey.val);
                MY_LOGD("Update dumpinfo - requestKey due to MTK_PIPELINE_DUMP_REQUEST_NUMBER is set.");
            }
            //
            dummyFrameKey.result    = NSCam::IMetadata::getEntry<MINT32>(pHalMeta, MTK_PIPELINE_DUMP_FRAME_NUMBER, dummyFrameKey.val);
            if (dummyFrameKey.result) {
                __middlewareInfo.frameKey   = static_cast<int>(dummyFrameKey.val);
                MY_LOGD("Update dumpinfo - frameKey due to MTK_PIPELINE_DUMP_FRAME_NUMBER is set.");
            }
        }

        __middlewareInfo.postrefine_nr = getPostrefineNr();
        __middlewareInfo.postrefine_mfb = getPostrefineMfb();

        m_pCore->setMiddlewareInfo(__middlewareInfo);
        MY_LOGD3("set uniqueKey(%d) to MFNR Core Lib", m_uniqueKey);
    }

    // update request Exif
    {
        MUINT8 isRequestExif = 0;
        IMetadata::getEntry<MUINT8>(pHalMeta, MTK_HAL_REQUEST_REQUIRE_EXIF, isRequestExif);

        m_bNeedExifInfo = (isRequestExif != 0);
    }

}

void MFNRShotInfo::makesDebugInfo(const std::map<MUINT32, MUINT32>& data, IMetadata* pMetadata)
{
    FUNCTION_SCOPE;

    if (pMetadata == nullptr) {
        MY_LOGE("%s: pMetadata is NULL", __FUNCTION__);
        return;
    }

    IMetadata exifMeta;
    {
        IMetadata::IEntry entry = pMetadata->entryFor(MTK_3A_EXIF_METADATA);
        if (entry.isEmpty()) {
            MY_LOGW("%s: no MTK_3A_EXIF_METADATA can be used", __FUNCTION__);
            return;
        }

        exifMeta = entry.itemAt(0, Type2Type<IMetadata>());
    }

    // add previous debug information from BSS
    IMetadata::Memory debugInfoSet;
    if (IMetadata::getEntry<IMetadata::Memory>(&exifMeta, MTK_MF_EXIF_DBGINFO_MF_DATA, debugInfoSet)) {
        auto pTag = reinterpret_cast<debug_exif_field*>(debugInfoSet.editArray());

        for (const auto& item : data) {
            const MUINT32 index = item.first;
            if (pTag[index].u4FieldID&0x1000000)
                continue;
            pTag[index].u4FieldID    = (0x1000000 | index);
            pTag[index].u4FieldValue = item.second;
            //MY_LOGD3("edit %d, %d, %d", index, pTag[index].u4FieldID, pTag[index].u4FieldValue);
        }
        IMetadata::setEntry<IMetadata::Memory>(&exifMeta, MTK_MF_EXIF_DBGINFO_MF_DATA, debugInfoSet);

    } else {
        /* set debug information into debug Exif metadata */
        DebugExifUtils::setDebugExif(
                DebugExifUtils::DebugExifType::DEBUG_EXIF_MF,
                static_cast<MUINT32>(MTK_MF_EXIF_DBGINFO_MF_KEY),
                static_cast<MUINT32>(MTK_MF_EXIF_DBGINFO_MF_DATA),
                data,
                &exifMeta);
    }

    /* update debug Exif metadata */
    IMetadata::IEntry entry_exif(MTK_3A_EXIF_METADATA);
    entry_exif.push_back(exifMeta, Type2Type<IMetadata>());
    pMetadata->update(entry_exif.tag(), entry_exif);
}

bool MFNRShotInfo::doMfnr()
{
    FUNCTION_SCOPE;
    FUNCTION_TRACE();

    // prepare
    {
        std::lock_guard<decltype(m_pCoreMx)> _l(m_pCoreMx);

        if (m_bFired) {
            MY_LOGE("MFNR has been fired, cannot fire twice");
            return false;
        }

        if (m_pCore.get() == nullptr) {
            MY_LOGE("MFNR Core is NULL");
            return false;
        }

        m_bFired = true;
    }
    // do MFNR
    {
        MY_LOGD("doMfnr");
        m_pCore->setQueueYuvFramesCounts((uint32_t)getRealBlendNum());
        m_pCore->doMfll();
    }
    // Exif
    {
        // store framecount for clear-zoom
        if (m_mainframe_halMetaIn) {
            IMetadata::setEntry<MINT32>(
                m_mainframe_halMetaIn,
                MTK_PLUGIN_PROCESSED_FRAME_COUNT,
                m_pCore->getFrameBlendedNum() + 1);
        } else {
            MY_LOGW("ask for debug exif but mainframe hal meta in is null");
        }

        if (m_mainframe_halMetaOut) {
            IMetadata::setEntry<MINT32>(
                m_mainframe_halMetaOut,
                MTK_PLUGIN_PROCESSED_FRAME_COUNT,
                m_pCore->getFrameBlendedNum() + 1);

            // append debug exif if need
            if (m_bNeedExifInfo) {
                auto&& pExifContainer = m_pCore->getExifContainer();
                if (pExifContainer.get()) {
                    makesDebugInfo(pExifContainer->getInfoMap(), m_mainframe_halMetaOut);
                }
                else {
                    MY_LOGW("ask for debug exif but exif container is empty");
                }
            }
        } else {
            MY_LOGW("ask for debug exif but mainframe hal meta out is null");
        }
    }
    // done
    {
        std::lock_guard<decltype(m_pCoreMx)> _l(m_pCoreMx);

        m_bFired = false;

        m_pMfb = nullptr;
        m_pCapturer = nullptr;
        m_spNvramProvider = nullptr;
        m_pCore = nullptr;
    }

    return true;
}

void MFNRShotInfo::updateUniqueKey(const int& key)
{
    FUNCTION_SCOPE;
    FUNCTION_TRACE();

    MY_LOGW("Update MFNRShotInfo::m_uniqueKey from %d to %d", m_uniqueKey, key);

    m_uniqueKey = key;
}

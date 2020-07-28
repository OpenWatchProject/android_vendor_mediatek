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
//
#define LOG_TAG "MfllVendorImp"
static const char* __CALLERNAME__ = LOG_TAG;

#include "MFNRImp.h"

#include "controller/MFNRCtrler.h"
#include "controller/MFNRCtrler_v1.6.h"
#include "../utils/VendorUtils.h"
#include "../utils/RequestFrame.h"
#include "../utils/ControllerContainer.h"


// MTKCAM
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/StlUtils.h>
#include <mtkcam/aaa/IHal3A.h> // setIsp, CaptureParam_T
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/feature/mfnr/MfllProperty.h>

//zHDR
#include <mtkcam/utils/hw/HwInfoHelper.h>
#include <mtkcam/feature/utils/FeatureProfileHelper.h> //ProfileParam
#include <mtkcam/drv/IHalSensor.h>

// MFNR
#include <mtkcam/feature/mfnr/MfllProperty.h>
#include <mtkcam/feature/mfnr/MfllTypes.h>

// AOSP
#include <cutils/compiler.h>

// STL
#include <memory> // std::shared_ptr
#include <deque> // std::deque
#include <mutex> // std::mutex
#include <algorithm> // std::find
#include <thread>
#include <chrono>
#include <cassert> // assert

// ----------------------------------------------------------------------------
// definitions
// ----------------------------------------------------------------------------
#define THREAD_SLOT_SIZE                    2
#define MFNR_FLASH_DUMMY_FRAMES_FRONT       2
#define MFNR_FLASH_DUMMY_FRAMES_BACK        2

// ----------------------------------------------------------------------------
// MY_LOG
// ----------------------------------------------------------------------------
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
//
#ifdef __DEBUG
#define FUNCTION_SCOPE          auto __scope_logger__ = create_scope_logger(__FUNCTION__)
#define LOG_SCOPE(log)          auto __scope_logger__ = create_scope_logger(log)
#include <memory>
static std::shared_ptr<char> create_scope_logger(const char* functionName)
{
    bool bEnableLog = mfll::MfllProperty::getDebugLevel() >= 3;
    char* pText = const_cast<char*>(functionName);
    CAM_LOGD_IF(bEnableLog, "[%s] + ", pText);
    return std::shared_ptr<char>(pText, [bEnableLog](char* p){ CAM_LOGD_IF(bEnableLog, "[%s] -", p); });
}
#else
#define FUNCTION_SCOPE
#define LOG_SCOPE(log)
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
#endif // SCOPE_TIMER


using namespace android;
using namespace NSCam::plugin;

using NS3Av3::CaptureParam_T;
using NS3Av3::IHal3A;
using NS3Av3::E3ACtrl_EnableFlareInManualCtrl;

Mutex gCtrlersLock;

// ----------------------------------------------------------------------------
// helper
// ----------------------------------------------------------------------------
static inline std::shared_ptr<MFNRCtrler> create_mfnr_controller(
        int openId,
        int mfbMode, // mfbMode, 0: not specific, 1: MFNR, 2: AIS
        MFNRVendor::ConfigParams const& rParams) //isZSDMode, isZHDRMode
{
    // to create a MFNRCtrler to query info
    MfllMode mfllMode = MfllMode_NormalMfll;
    MUINT32 sensorMode = rParams.sensorMode;

    // 0: Not specific, 1: MFNR, 2: AIS
    switch (mfbMode) {
        case MTK_MFB_MODE_MFLL:
            mfllMode = (CC_UNLIKELY(rParams.isZHDRMode)) ?
                (CC_LIKELY(rParams.isZSDMode) ? MfllMode_ZhdrZsdMfll : MfllMode_ZhdrNormalMfll) :
                (CC_LIKELY(rParams.isZSDMode) ? MfllMode_ZsdMfll : MfllMode_NormalMfll);
            break;
        case MTK_MFB_MODE_AIS:
            mfllMode = (CC_UNLIKELY(rParams.isZHDRMode)) ?
                (CC_LIKELY(rParams.isZSDMode) ? MfllMode_ZhdrZsdAis : MfllMode_ZhdrNormalAis) :
                (CC_LIKELY(rParams.isZSDMode) ? MfllMode_ZsdAis : MfllMode_NormalAis);
            break;
        default:
            mfllMode = (CC_UNLIKELY(rParams.isZHDRMode)) ?
                (CC_LIKELY(rParams.isZSDMode) ? MfllMode_ZhdrZsdMfll : MfllMode_ZhdrNormalMfll) :
                (CC_LIKELY(rParams.isZSDMode) ? MfllMode_ZsdMfll : MfllMode_NormalMfll);
    }

    if (rParams.isAutoHDR) {
        mfllMode = static_cast<enum MfllMode>(mfllMode | (1 << MfllMode_Bit_AutoHdr));
    }

    MY_LOGD("custom hint:%d", rParams.customHint);
    switch (rParams.customHint) {
        case MTK_CUSTOM_HINT_1:
            mfllMode = static_cast<enum MfllMode>(mfllMode | (1 << MfllMode_Bit_Custom1));
            break;
        case MTK_CUSTOM_HINT_2:
            mfllMode = static_cast<enum MfllMode>(mfllMode | (1 << MfllMode_Bit_Custom2));
            break;
        case MTK_CUSTOM_HINT_3:
            mfllMode = static_cast<enum MfllMode>(mfllMode | (1 << MfllMode_Bit_Custom3));
            break;
        case MTK_CUSTOM_HINT_4:
            mfllMode = static_cast<enum MfllMode>(mfllMode | (1 << MfllMode_Bit_Custom4));
            break;
        default:;
    }

    MY_LOGD("MfllMode(0x%X), mfbMode(%d), sensorMode(%u)"\
            "isZsd(%d), isZhdr(%d), isAutoHDR(%d), "\
            "realIso(%d), exposureTime(%d), isFlashOn(%d)",
            mfllMode, mfbMode, sensorMode,
            rParams.isZSDMode, rParams.isZHDRMode, rParams.isAutoHDR,
            rParams.realIso, rParams.exposureTime, rParams.isFlashOn);

    if (SENSOR_SCENARIO_ID_UNNAMED_START == sensorMode) {
        MY_LOGW("sensorMode is unnamed for MFNRCtrler, use Capture as default");
        sensorMode = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
    }

    std::shared_ptr<MFNRCtrler> c
        (
            dynamic_cast<MFNRCtrler*>(new MFNRCtrler_v1_6(openId, mfllMode, sensorMode, rParams.realIso, rParams.exposureTime, rParams.isFlashOn))
        );

    return c;
}


static inline void store_app_setting(IMetadata* pAppMeta, MFNRCtrler* ctrler)
{
    // {{{
    // get the data and saves to MFNR controller,
    MUINT8 __aemode = 0;
    MUINT8 __eis = 0;
    MINT32 __iso = 0;
    MINT64 __exp = 0;

    // Back up the original AppSetting if it exists.
    auto b = IMetadata::getEntry<MUINT8>(pAppMeta, MTK_CONTROL_AE_MODE, __aemode);
    if (b) {
        ctrler->setAppSetting(
                MFNRCtrler::AppSetting::CONTROL_AE_MODE,
                static_cast<int64_t>(__aemode));
    }

    b = IMetadata::getEntry<MUINT8>(pAppMeta, MTK_CONTROL_VIDEO_STABILIZATION_MODE, __eis);
    if (b) {
        ctrler->setAppSetting(
                MFNRCtrler::AppSetting::CONTROL_VIDEO_STABILIZATION_MODE,
                static_cast<int64_t>(__eis));
    }

    b = IMetadata::getEntry<MINT32>(pAppMeta, MTK_SENSOR_SENSITIVITY, __iso);
    if (b) {
        ctrler->setAppSetting(
                MFNRCtrler::AppSetting::SENSOR_SENSITIVITY,
                static_cast<int64_t>(__iso));
    }

    b = IMetadata::getEntry<MINT64>(pAppMeta, MTK_SENSOR_EXPOSURE_TIME, __exp);
    if (b) {
        ctrler->setAppSetting(
                MFNRCtrler::AppSetting::SENSOR_EXPOSURE_TIME,
                static_cast<int64_t>(__exp));
    }
    // }}}
}

static inline void update_app_setting(IMetadata* pAppMeta, const MFNRCtrler* ctrler)
{
    bool bNeedManualAe = [&](){
        // if using ZHDR, cannot apply manual AE
        if (mfll::isZhdrMode(ctrler->getMfbMode()))
            return false;
        // if using FLASH, cannot apply menual AE
        if (ctrler->getIsFlashOn())
            return false;
        // if uisng MFNR (since MFNR uses ZSD buffers), we don't need manual AE
        // but if MFNR using non-ZSD flow, we need to apply manual AE
        if (mfll::isMfllMode(ctrler->getMfbMode())) {
            if (mfll::isZsdMode(ctrler->getMfbMode()))
                return false;
            else
                return true;
        }
        /// otherwise, we need it
        return true;
    }();

    if (bNeedManualAe) {
        IMetadata::setEntry<MUINT8>(pAppMeta, MTK_CONTROL_AE_MODE, MTK_CONTROL_AE_MODE_OFF);
        IMetadata::setEntry<MINT32>(pAppMeta, MTK_SENSOR_SENSITIVITY, ctrler->getFinalIso());
        IMetadata::setEntry<MINT64>(pAppMeta, MTK_SENSOR_EXPOSURE_TIME, ctrler->getFinalShutterTime() * 1000); // ms->us
    }

    IMetadata::setEntry<MUINT8>(pAppMeta, MTK_CONTROL_VIDEO_STABILIZATION_MODE, MTK_CONTROL_VIDEO_STABILIZATION_MODE_ON);
}


// ----------------------------------------------------------------------------
// static member
// ----------------------------------------------------------------------------
ControllerContainer<MFNRVendorImp::CtrlerType> MFNRVendorImp::sUnusedCtrlers;


// ----------------------------------------------------------------------------
// Implementations of IVendor
// ----------------------------------------------------------------------------
MFNRVendor::FeatureInfo MFNRVendor::queryFeatureInfo(
        int openId,
        int mfbMode,
        MFNRVendor::ConfigParams const& rParams)
{
    FUNCTION_SCOPE;
    Mutex::Autolock _l(MFNRVendor::sCtrlerAccessLock);
    FeatureInfo info;

    int bForceMfb = MfllProperty::isForceMfll();
    if (CC_UNLIKELY( bForceMfb == 0 )) {
        MY_LOGD("Force disable MFNR");
        mfbMode = MTK_MFB_MODE_OFF;
    }
    else if (CC_UNLIKELY(bForceMfb > 0)) {
        MY_LOGD("Force MFNR (bForceMfb:%d)", bForceMfb);
        mfbMode = bForceMfb;
    }

    if (mfbMode == MTK_MFB_MODE_OFF)
        return info;

    // to create a MFNRCtrler to query info
    auto ctrler = create_mfnr_controller(openId, mfbMode, rParams);
    if (CC_UNLIKELY(ctrler.get() == nullptr)) {
        MY_LOGE("fatal error: create a MFNR controller failed");
        return info;
    }

    info.doMfb = ctrler->getIsEnableMfnr();
    info.frameCapture = ctrler->getCaptureNum();
    info.frameBlend = ctrler->getBlendNum();

    if (info.doMfb != 0) {
        // push back into container
        Mutex::Autolock _l(gCtrlersLock);
        // limited unused controller size <= 2
        if (MFNRVendorImp::sUnusedCtrlers.size() >= 2)
            MFNRVendorImp::sUnusedCtrlers.take_first();

        MFNRVendorImp::sUnusedCtrlers.push_back(ctrler);
    }

    return info;
}


MFNRVendorImp::MFNRVendorImp(
        char const*  pcszName,
        MINT32 const i4OpenId,
        MINT64 const vendorMode
    ) noexcept
    : BaseVendor(i4OpenId, vendorMode, String8::format("MFNR vendor"))
    , mIsZsd(MTRUE)
    , mIsZhdr(MFALSE)
    , mIsAutoHdr(MFALSE)
    , mMfbMode(0)
    , m_dbgLevel(0)
{
    m_dbgLevel = mfll::MfllProperty::getDebugLevel();

    FUNCTION_SCOPE;
    MY_LOGD3("[%d]create vendor %" PRId64 " from %s", i4OpenId, vendorMode, pcszName);

    setThreadShotsSize(THREAD_SLOT_SIZE);

    try {
        mHal3A = std::unique_ptr< IHal3A, std::function<void(IHal3A*)> >
            (
                MAKE_Hal3A(i4OpenId, __CALLERNAME__),
                [](IHal3A* p){ if (p) p->destroyInstance(__CALLERNAME__); }
            );
    } catch (std::exception&) {
        mHal3A = nullptr;
        MY_LOGE("create IHal3A instance failed");
#if (MTKCAM_LOG_LEVEL_DEFAULT >= 3)
        // even though mHal3A is nullptr, functions still work. Hence assert while
        // log level >= 3
        assert(0);
#endif
    }
}


MFNRVendorImp::~MFNRVendorImp()
{
    FUNCTION_SCOPE;
    syncAllThread();
    mHal3A = nullptr;
    try {
        Mutex::Autolock _l(gCtrlersLock);
#if 0  // support background service, cannot clear MFNR controller container
        MFNRVendorImp::sUnusedCtrlers.clear();
#endif
    } catch (std::exception&) {
        MY_LOGE("clear unused MFNR controller throws exception");
    }
}


status_t MFNRVendorImp::get(
        MINT32           openId         __attribute__((unused)),
        const InputInfo& in             __attribute__((unused)),
        FrameInfoSet&    out            __attribute__((unused)))
{
    FUNCTION_SCOPE;
    SCOPE_TIMER(__t0, __FUNCTION__);

    std::shared_ptr<CtrlerType> ctrler;

    // Step 0: check if EM config has been configured, if yes, clear all unused
    //         controller and re-create a new one.
    MFNRVendor::EMConfig emcfg;
    if (CC_UNLIKELY(getEMConfig(&emcfg, true))) {
        MY_LOGD("Enable MFNR w/ EM config");

        getCtrlerContainer(CTRLER_UNUSED)->clear();

        // Step 1-1: create a unused MFNR controller
        ctrler = getCtrler(CTRLER_UNUSED);

        ctrler->setCaptureNum(emcfg.captureNum);
        ctrler->setBlendNum(emcfg.blendNum);
        ctrler->setFinalIso(emcfg.realIso);
        ctrler->setFinalShutterTime(emcfg.shutterTimeUs);
    }
    else {
        // Step 1-2: create (or get) the first unused MFNR controller
        ctrler = getCtrler(CTRLER_UNUSED);
    }

    // check controller
    if (CC_UNLIKELY( ctrler.get() == nullptr )) {
        MY_LOGE("unexpected error, get controller failed");
        return UNKNOWN_ERROR;
    }

#if (MTKCAM_LOG_LEVEL_DEFAULT >= 3)
    int bForceMmdvfsOff = MfllProperty::getForceMmdvfsOff();
    if (CC_UNLIKELY( bForceMmdvfsOff > 0 )) {
        MY_LOGD("MMDVFS has been force disabled by adb property");
    }
    else
#endif
    {
        // check in.sensorMode
        if (CC_UNLIKELY( in.sensorMode == SENSOR_SCENARIO_ID_UNNAMED_START )) {
            MY_LOGW("sensorMode is unnamed for IScenarioControl, use Capture as default");
            ctrler->enterCaptureScenario(openId, SENSOR_SCENARIO_ID_NORMAL_CAPTURE);
        }
        else {
            ctrler->enterCaptureScenario(openId, in.sensorMode);

        }
    }

    // Step 2: configure settings
    int frameCount = ctrler->getCaptureNum();
    int frameDelay = [&ctrler, &in]()->int
    {
        // cpature with flash
        if (ctrler->getIsFlashOn())
            return std::max(ctrler->getDelayFrameNum(), MFNR_FLASH_DUMMY_FRAMES_BACK);

        // no need delay frame in non-ZSD mode
        if (CC_UNLIKELY( in.isZsdMode == 0 ))
            return 0;

        int previewBrightness = ctrler->getRealIso() * ctrler->getShutterTime();
        int captureBrightness = ctrler->getFinalIso() * ctrler->getFinalShutterTime();

        // [2017/06/08]: workaround MTK_FOCUS_PAUSE is not frame synced problem (Rynn)
        constexpr const bool bWorkAroundAFLock = true;

        // if brightness is different between preview and capture, we
        // need delay frame for AE stable
        if ( !mfll::isAisMode(ctrler->getMfbMode()) ) { //
            MY_LOGD("it not AIS mode (use cache frame), no need to use delay frame");
            return 0;
        }
        if (bWorkAroundAFLock || (previewBrightness != captureBrightness)) {
            MY_LOGD("workaround AF lock: %d", bWorkAroundAFLock ? 1 : 0);
            MY_LOGD("brightness is not equal (preview:%d, capture:%d)",
                     previewBrightness, captureBrightness);
            return ctrler->getDelayFrameNum();
        }

        return 0;
    }();
    int frameDummy = [&ctrler, &in]()->int
    {
        // cpature with flash
        if (ctrler->getIsFlashOn())
            return MFNR_FLASH_DUMMY_FRAMES_FRONT;

        return 0;
    }();

#ifdef __DEBUG
    // {{{ show info
    MY_LOGD3("%s: iso   = %d", __FUNCTION__, ctrler->getFinalIso());
    MY_LOGD3("%s: exp   = %d", __FUNCTION__, ctrler->getFinalShutterTime());
    MY_LOGD3("%s: cap   = %d", __FUNCTION__, frameCount);
    MY_LOGD3("%s: dummy = %d", __FUNCTION__, frameDummy);
    MY_LOGD3("%s: delay = %d", __FUNCTION__, frameDelay);
    MY_LOGD3("%s: sensor mode = %u", __FUNCTION__, ctrler->getSensorMode());
    // }}}
#endif

    MUINT8 bOriFocusPause  = 0;
    if ( !IMetadata::getEntry<MUINT8>(&in.halCtrl, MTK_FOCUS_PAUSE, bOriFocusPause) ) {
        MY_LOGW("%s: cannot retrieve MTK_FOCUS_PAUSE from HAL metadata, assume "\
                "it to 0", __FUNCTION__);
    }

    // Step 2.1: configure capture setting.
    for (int i = 0; i < frameCount; i++) {
        bool bLastFrame = ((i + 1) == frameCount);

        // default, copy from InputInfo.
        MetaItem item;
        item.setting.appMeta = in.appCtrl;
        item.setting.halMeta = in.halCtrl;

        IMetadata& appSetting(item.setting.appMeta);
        if (i == 0) {
            // saves the default app setting to controller
            store_app_setting(&appSetting, ctrler.get());
        }

        // update App Setting for every request
        update_app_setting(&appSetting, ctrler.get());

        // modify hal control metadata
        IMetadata& halSetting(item.setting.halMeta);
        {
            // update ISP profile for zHDR (single frame default)
            if (mfll::isZhdrMode(ctrler->getMfbMode())) {
                bool isAutoHDR = mfll::isAutoHdr(ctrler->getMfbMode());
                MUINT sensorMode = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
                MSize sensorSize;
                NSCamHW::HwInfoHelper helper(openId);
                if (!helper.getSensorSize(sensorMode, sensorSize)) {
                    MY_LOGW("cannot get sensor size");
                }
                else {
                    // Prepare query Feature Shot ISP Profile
                    ProfileParam profileParam
                    {
                        sensorSize,
                        SENSOR_VHDR_MODE_ZVHDR, /*VHDR mode*/
                        sensorMode,
                        ProfileParam::FLAG_NONE,
                        ((isAutoHDR) ? (ProfileParam::FMASK_AUTO_HDR_ON) : (ProfileParam::FMASK_NONE)),
                    };

                    MUINT8 profile = 0;
                    if (FeatureProfileHelper::getShotProf(profile, profileParam))
                    {
                        MY_LOGD("ISP profile is set(%u)", profile);
                        // modify hal control metadata for zHDR
                        IMetadata::setEntry<MUINT8>(
                            &halSetting , MTK_3A_ISP_PROFILE , profile);
                        IMetadata::setEntry<MUINT8>(
                            &halSetting, MTK_3A_AE_CAP_SINGLE_FRAME_HDR, 1);
                    }
                    else
                    {
                        MY_LOGW("ISP profile is not set(%u)", profile);
                    }
                }
            }

            // pause AF for (N - 1) frames and resume for the last frame
            IMetadata::setEntry<MUINT8>(
                    &halSetting, MTK_FOCUS_PAUSE,
                    bLastFrame ? bOriFocusPause : 1);
            IMetadata::setEntry<MUINT8>(
                    &halSetting, MTK_HAL_REQUEST_REQUIRE_EXIF, 1);
            IMetadata::setEntry<MUINT8>(
                    &halSetting, MTK_HAL_REQUEST_DUMP_EXIF, 1);

            // need pure raw for MFNR flow
            IMetadata::setEntry<MINT32>(
                    &halSetting, MTK_P1NODE_RAW_TYPE, 1);

            MINT32 customHintInMFNR = mfll::getCustomHint(ctrler->getMfbMode());
            MINT32 customHintInHal;
            // check customHint in metadata for customize feature
            if ( !IMetadata::getEntry<MINT32>(&halSetting, MTK_PLUGIN_CUSTOM_HINT, customHintInHal) ) {
                MY_LOGW("%s: cannot retrieve MTK_PLUGIN_CUSTOM_HINT from HAL metadata, assume "\
                        "it to %d", __FUNCTION__, customHintInMFNR);
                IMetadata::setEntry<MINT32>(
                    &halSetting,
                    MTK_PLUGIN_CUSTOM_HINT,
                    customHintInMFNR);
            }
            else if (customHintInMFNR != customHintInHal) { // query and check the result
                MY_LOGW("%s: MTK_PLUGIN_CUSTOM_HINT in MFNR(%d) and Hal(%d) setting are different"
                        , __FUNCTION__, customHintInMFNR, customHintInHal);
            }
        }

        out.settings.push_back(item);
    }

    // Step 2.2: add dummy frames (for 3A stable before capture with flash)
    for (int i = 0; i < frameDummy; i++) {
        // default, copy from InputInfo.
        MetaItem item;
        item.setting.appMeta = in.appCtrl;
        item.setting.halMeta = in.halCtrl;

        // Use hal meta hint for dummy frames before capture with flash
        if (ctrler->getIsFlashOn()) {
            IMetadata& appSetting(item.setting.appMeta);
            IMetadata::setEntry<MUINT8>(&appSetting, MTK_CONTROL_AE_MODE, MTK_CONTROL_AE_MODE_OFF);
            IMetadata::setEntry<MINT64>(&appSetting, MTK_SENSOR_EXPOSURE_TIME, 33333333);
            IMetadata::setEntry<MINT32>(&appSetting, MTK_SENSOR_SENSITIVITY, 1000);
        }
        // need pure raw for MFNR flow
        IMetadata::setEntry<MINT32>(&item.setting.halMeta, MTK_P1NODE_RAW_TYPE, 1);

        out.dummySettings.push_back(item);
    }

    // Step 2.3: add delay frames (for 3A stable before resume preview)
    for (int i = 0; i < frameDelay; i++) {
        // default, copy from InputInfo.
        MetaItem item;
        item.setting.appMeta = in.appCtrl;
        item.setting.halMeta = in.halCtrl;

        // use hal meta hint to restore AE during capture delay frames
        IMetadata& halSetting(item.setting.halMeta);
        IMetadata::setEntry<MBOOL>(&halSetting, MTK_3A_DUMMY_AFTER_REQUEST_FRAME, 1);
        IMetadata::setEntry<MINT32>(&halSetting, MTK_P1NODE_RAW_TYPE, 1);

        out.delayedSettings.push_back(item);
    }

    // Step 2.3: configure it
    out.table.vendorMode    = mMode;
    out.table.inCategory    = FORMAT_RAW;
    out.table.outCategory   = FORMAT_YUV;
    out.table.bufferType    = BUFFER_NO_WORKING_OUTPUT;
    out.frameCount          = out.settings.size();
    out.dummyCount          = out.dummySettings.size();
    out.delayedCount        = out.delayedSettings.size();

    // Step 2.4: tell 3A manager to apply flare w/ manual exposure mode
    if (CC_LIKELY(mHal3A.get())) {
        MY_LOGD3("ask flare offset -> yes");
        mHal3A->send3ACtrl(E3ACtrl_EnableFlareInManualCtrl, 1, 0);
    }
    else {
        MY_LOGE("ask flare offset failed, due to no IHal3A instance");
    }


    // Step 3: create StreamId Map
    // create StreamId Map for RequestFrame
    {
        SCOPE_TIMER (__t1, "Create StreamID map");
        RequestFrame::StreamIdMap_Img map;
        if (CC_LIKELY(in.fullRaw.get())) {
            map[RequestFrame::eRequestImg_FullSrc] = in.fullRaw->getStreamId();
            ctrler->setSizeSrc(in.fullRaw->getImgSize());
            MY_LOGD3("set source size=%dx%d",
                    ctrler->getSizeSrc().w, ctrler->getSizeSrc().h);
        }
        else {
            MY_LOGE("no full size RAW");
        }

        if (in.resizedRaw.get()) {
            map[RequestFrame::eRequestImg_ResizedSrc] = in.resizedRaw->getStreamId();
            ctrler->setSizeRrzo(in.resizedRaw->getImgSize());
            MY_LOGD3("set rrzo size=%dx%d",
                    ctrler->getSizeRrzo().w, ctrler->getSizeRrzo().h);
        }
        else {
            MY_LOGD("no rrzo");
        }

        // lcso
        if (CC_LIKELY(in.lcsoRaw.get())) {
            map[RequestFrame::eRequestImg_LcsoRaw] = in.lcsoRaw->getStreamId();
        }
        else {
            MY_LOGD("no lcso");
        }

        if (CC_LIKELY(in.postview.get())) {
            map[RequestFrame::eRequestImg_PostviewOut] = in.postview->getStreamId();
        }
        else {
            MY_LOGD("no postview YUV");
        }

        if (CC_LIKELY(in.jpegYuv.get())) {
            map[RequestFrame::eRequestImg_FullOut] = in.jpegYuv->getStreamId();
        }
        else {
            MY_LOGE("no output YUV");
        }

        if (CC_LIKELY(in.thumbnailYuv.get())) {
            map[RequestFrame::eRequestImg_ThumbnailOut] = in.thumbnailYuv->getStreamId();
        }
        else {
            MY_LOGW("no thumbnail info");
        }

        if (CC_LIKELY(in.workingbuf.get())) {
            map[RequestFrame::eRequestImg_WorkingBufferOut] = in.workingbuf->getStreamId();
        }
        else {
            MY_LOGD("no working buffer info");
        }

        if ( in.vYuv.size() + RequestFrame::eRequestImg_ReservedOut1 >= RequestFrame::eRequestImg_Out_End )
            MY_LOGE("output yuv too much.");
        for(size_t i = 0; i < in.vYuv.size(); ++i) {
            if (CC_LIKELY(in.vYuv[i].get())) {
                map[RequestFrame::eRequestImg_ReservedOut1 + i] = in.vYuv[i]->getStreamId();
            }
        }

        // TODO: postview(HAL1)

        ctrler->setStreamIdMapImg(std::move(map));
    }

    return OK;
}


status_t MFNRVendorImp::set(
        MINT32              openId              __attribute__((unused)),
        const InputSetting& in                  __attribute__((unused)))
{
    FUNCTION_SCOPE;

    // Step 1: Take the first unused MFNR controller.
    auto ctrler = takeCtrler(CTRLER_UNUSED);
    if (CC_UNLIKELY(ctrler.get() == nullptr)) {
        MY_LOGE("No unused MFNR controller");
        return UNKNOWN_ERROR;
    }

    // Step 2: check capture frame number is matched or not.
    if (CC_UNLIKELY(ctrler->getCaptureNum() > static_cast<int>(in.vFrame.size()))) {
        MY_LOGE("frame amount is not enough, re-set size to %zu",
                in.vFrame.size());
        if (in.vFrame.size() <= 0) {
            MY_LOGE("frame size is 0");
            return UNKNOWN_ERROR;
        }
        ctrler->setCaptureNum(static_cast<int>(in.vFrame.size()));
        ctrler->setBlendNum(static_cast<int>(in.vFrame.size()));
    }

    // Step3:
    //  3.1: set request to this controller
    for (size_t i = 0; i < in.vFrame.size(); i++) {
        if (static_cast<int>(i) >= ctrler->getCaptureNum())
            break;

        MY_LOGD3("MFNRCtrler is belong to frame requestNo:%d", in.vFrame[i].frameNo);
        ctrler->setBelong(in.vFrame[i].frameNo);
    }

    //  3.2: set message callback
    ctrler->setMessageCallback(in.pMsgCb);


    // Step4: fire MFNR asynchronously and push this controller to
    //        used controller container
    pushCtrler(ctrler, CTRLER_USED);
    // if the number of used thread slot reaches to max,
    // this method will be blocked until there's a free thread slot.
    asyncRun([this, ctrler]() mutable {
            MFLL_THREAD_NAME("doMFNR");
            LOG_SCOPE("MfllVendorImp: asyncRun");
            ctrler->initMfnrCore(); // init MFNR controller
            ctrler->execute();
            ctrler->waitExecution();
            removeCtrler(ctrler, CTRLER_USED); // remove self from container
            ctrler = nullptr; // clear smart pointer after use, this scope may
                              // not exit until std::future::get has invoked.
        });

    return OK;
}


status_t MFNRVendorImp::queue(
        MINT32  const              openId           __attribute__((unused)),
        MUINT32 const              requestNo        __attribute__((unused)),
        wp<IVendor::IDataCallback> cb               __attribute__((unused)),
        BufferParam                bufParam         __attribute__((unused)),
        MetaParam                  metaParam        __attribute__((unused)))
{
    FUNCTION_SCOPE;

#ifdef __DEBUG
    MY_LOGI("queue frame, requestNo=%u", requestNo);
#endif

    // Step 1:
    // find the used(fired) controller by request number
    auto spCtrl = getCtrler(requestNo, CTRLER_USED);
    if (CC_UNLIKELY(spCtrl.get() == nullptr)) {
        MY_LOGW("error, this frame(requestNo:%d) is not belong to anyone.", requestNo);
        return BAD_INDEX;
    }

    // Step 1.1:
    // Check if MTK_HAL_REQUEST_ERROR_FRAME in HAL metadata is 1 or not.
    // If yes, gives an empty frame to controller as dummy frame.
    bool bIsDummyFrame = [&metaParam]()
    {
        // check MTK_HAL_REQUEST_ERROR_FRAME
        MUINT8 isError = 0;
        auto b = IMetadata::getEntry<MUINT8>(
                metaParam.halCtrl->getMetadata(),
                MTK_HAL_REQUEST_ERROR_FRAME,
                isError);
        if (b && isError) {
            return true;
        }
        return false;
    }();

    if (__builtin_expect( bIsDummyFrame, false )) {
        MY_LOGW("requestNo=%u is a dummy frame", requestNo);
    }

    // Step 2:
    // enque frame to controller.
    {
        MY_LOGD3("enque frame, requestNo=%u", requestNo);
        auto _result = [&](){
            if (bIsDummyFrame)
                return spCtrl->enqueFrame(nullptr); // enque a nullptr if it's dummy frame
            else
                return spCtrl->enqueFrame(openId, requestNo, bufParam, metaParam, cb);
        }();

        if (_result < 0) {
            switch (_result) {
            case -1:
                MY_LOGW("error, this controller is invalidated, cannot enque frame");
                break;
            default:
                MY_LOGW("error, enqueFrame returns failed");
                break;
            }
            return BAD_INDEX;
        }
    }

    // Step 3:
    // check if all frames are enqued to the controller.
    if (spCtrl->getEnquedTimes() >= spCtrl->getCaptureNum()) {
        // TODO: all frame we got, need to tell AE mgr do not enable flare offset
        if (CC_LIKELY(mHal3A.get())) {
            MY_LOGD3("ask flare offset -> no");
            mHal3A->send3ACtrl(E3ACtrl_EnableFlareInManualCtrl, 0, 0);
        }
    }

    // ignore dummy frame
    return bIsDummyFrame ? BAD_INDEX : OK;
}


status_t MFNRVendorImp::beginFlush(MINT32 /*openId*/)
{
    FUNCTION_SCOPE;

    drain(); // cancel flow, and drain.

    return OK;
}


status_t MFNRVendorImp::endFlush(MINT32 /*openId*/)
{
    FUNCTION_SCOPE;
    return OK;
}


status_t MFNRVendorImp::sendCommand(
        MINT32      cmd,
        MINT32      /* openId */,
        MetaItem&   /* meta */,
        MINT32&     /* arg1 */,
        MINT32&     /* arg2 */,
        void*       arg3
    )
{
    FUNCTION_SCOPE;
    switch (cmd) {
    case MFNRVendor::INFO_MFNR_ENG_PARAM:
        MY_LOGD("set eng param");
        setEMConfig(reinterpret_cast<MFNRVendor::EMConfig*>(arg3));
        break;
    case MFNRVendor::INFO_MFNR_CONFIG_PARAM:
        MY_LOGD("set ConfigParams");
        setConfigParms(reinterpret_cast<MFNRVendor::ConfigParams*>(arg3));
        break;
    default:;
    }

    return OK;
}


status_t MFNRVendorImp::dump(MINT32 /* openId */)
{
    FUNCTION_SCOPE;
    return OK;
}


// ----------------------------------------------------------------------------
// Implementations of MFNRVendor
// ----------------------------------------------------------------------------
MERROR MFNRVendorImp::config(
        MFNRVendor::ConfigParams const& rParams
    )
{
    FUNCTION_SCOPE;
    // no one invoke this method yet
    return OK;
}


MERROR MFNRVendorImp::query(
        MFNRVendor::FeatureInfo&
    )
{
    FUNCTION_SCOPE;

    return OK;
}


void MFNRVendorImp::drain(bool bCancel /* = true */)
{
    FUNCTION_SCOPE;
    // moves all controller from container(s)
    auto CC = std::move(mUsedCtrlers);

    // Type of ControllerContainer::iterate's argument
    typedef ControllerContainer<CtrlerType>::ContainerType Data_T;

    // check if using background service
    bool bUsingBackgroundServ = [this](){
        std::lock_guard<NSCam::SpinLock> __l(m_lockConfigParms);
        return m_pConfigParams.get()
            ? !!m_pConfigParams->usingBackgroundService
            : false;
    }();

    CC.iterate([bCancel, bUsingBackgroundServ](Data_T& data) {
        // if not using background service, we cancel MFNR flow ASAP
        if (!bUsingBackgroundServ) {
            for (auto&& itr : data)
                if (itr.get() && bCancel)
                    itr->doCancel();
        }

        for (auto&& itr : data)
            if (itr.get())
                itr->waitExecution();

    });
}


void MFNRVendorImp::setConfigParms(
        const MFNRVendor::ConfigParams* pParms)
{
    FUNCTION_SCOPE;

    if (CC_UNLIKELY(pParms == nullptr)) {
        MY_LOGD("MFNRVendor::ConfigParams is nullptr");
        return;
    }

    {
        // update vendor information for default ConfigParams
        std::lock_guard<NSCam::SpinLock> __l(m_lockConfigParms);
        if (m_pConfigParams.get() == nullptr) {
            m_pConfigParams = std::make_shared<MFNRVendor::ConfigParams>(MFNRVendor::ConfigParams());
        }
        *m_pConfigParams.get() = *pParms; // copy
    }
}

bool
MFNRVendorImp::getConfigParms(
        MFNRVendor::ConfigParams* pParms,
        bool                      bClear /* = false */ )
{
    {
        std::lock_guard<NSCam::SpinLock> __l(m_lockConfigParms);
        if (m_pConfigParams.get() == nullptr) {
            return false;
        }
        *pParms = *m_pConfigParams.get(); // copy
        if (bClear)
            m_pConfigParams = nullptr;
    }
    return true;
}


void MFNRVendorImp::setEMConfig(const MFNRVendor::EMConfig* pConfig)
{
    FUNCTION_SCOPE;

    if (CC_UNLIKELY(pConfig == nullptr)) {
        MY_LOGD("MFNRVendor::EMConfig is nullptr");
        return;
    }

    {
        std::lock_guard<NSCam::SpinLock> __l(m_lockEmConfig);
        if (m_pEmConfig.get() == nullptr) {
            m_pEmConfig = std::make_shared<MFNRVendor::EMConfig>(MFNRVendor::EMConfig());
        }
        *m_pEmConfig.get() = *pConfig;

        // update vendor information following EMConfig
        mMfbMode = MTK_MFB_MODE_AIS;  // always AIS (always need change PLINE)
        mIsZsd = MFALSE; // always false.
        mIsZhdr = m_pEmConfig->isZHDRMode;
        mIsAutoHdr = m_pEmConfig->isAutoHDR;
    }
}


bool
MFNRVendorImp::
getEMConfig(
        MFNRVendor::EMConfig*   pCfg,
        bool                    bClear /* = false */
        )
{
    {
        std::lock_guard<NSCam::SpinLock> __l(m_lockEmConfig);
        if (m_pEmConfig.get() == nullptr) {
            return false;
        }
        *pCfg = *m_pEmConfig.get(); // copy
        if (bClear)
            m_pEmConfig = nullptr;
    }
    return true;
}


ControllerContainer<MFNRVendorImp::CtrlerType>*
MFNRVendorImp::getCtrlerContainer(CTRLER u)
{
    switch (u) {
    case CTRLER_USED:
        return &mUsedCtrlers;
    case CTRLER_UNUSED:
        return &sUnusedCtrlers;
    default:
        MY_LOGE("get wrong controller container (enum=%d)", u);
        return nullptr;
    }
    return nullptr;
}


std::shared_ptr<MFNRVendorImp::CtrlerType>
MFNRVendorImp::getCtrler(CTRLER u)
{
    FUNCTION_SCOPE;
    Mutex::Autolock _l(gCtrlersLock);
    auto pCC = getCtrlerContainer(u);
    std::shared_ptr<CtrlerType> ctrler;

    // Type of ControllerContainer::iterate's argument
    typedef ControllerContainer<CtrlerType>::ContainerType Data_T;

    pCC->iterate
    (
        [this, &ctrler]
        (Data_T& data)
        {
            // if no controller in container, create a new one and push back into it
            if (data.size() <= 0) {
                auto openId  = this->getOpenId();
                MFNRVendor::ConfigParams params;
                if (CC_LIKELY(!getConfigParms(&params, true))) {
                    MY_LOGW("cannot get ConfigParams, use default setting");
                    params.mfbMode = this->mMfbMode;
                    params.isZSDMode  = this->mIsZsd;
                    params.isZHDRMode = this->mIsZhdr;
                    params.isAutoHDR  = this->mIsAutoHdr;
                }
                ctrler = create_mfnr_controller(openId, params.mfbMode, params);
                data.push_back(ctrler);
            }
            // or get the last controller
            else {
                ctrler = data.back();
            }
        }
    );

    return ctrler;
}


std::shared_ptr<MFNRVendorImp::CtrlerType>
MFNRVendorImp::getCtrler(MUINT32 requestNo, CTRLER u)
{
    FUNCTION_SCOPE;
    Mutex::Autolock _l(gCtrlersLock);
    return getCtrlerContainer(u)->belong_to(requestNo);
}


std::shared_ptr<MFNRVendorImp::CtrlerType>
MFNRVendorImp::takeCtrler(CTRLER u)
{
    FUNCTION_SCOPE;
    Mutex::Autolock _l(gCtrlersLock);
    return getCtrlerContainer(u)->take_last();
}


void MFNRVendorImp::pushCtrler(const std::shared_ptr<CtrlerType>& c, CTRLER u)
{
    FUNCTION_SCOPE;
    Mutex::Autolock _l(gCtrlersLock);
    getCtrlerContainer(u)->push_back(c);
}


bool MFNRVendorImp::removeCtrler(std::shared_ptr<CtrlerType> c, CTRLER u)
{
    FUNCTION_SCOPE;
    Mutex::Autolock _l(gCtrlersLock);
    return getCtrlerContainer(u)->remove(c);
}

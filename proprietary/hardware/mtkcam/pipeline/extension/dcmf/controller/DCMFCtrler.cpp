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
#define MTK_MFNR

#define LOG_TAG "DCMFCtrler"
static const char* __CALLERNAME__ = LOG_TAG;

#include "DCMFCtrler.h"
// log
#include <mtkcam/utils/std/Log.h>
// stereo
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#include <mtkcam/feature/DualCam/IDualCamMFStrategy.h>
// custom setting
#include <camera_custom_stereo.h>
// STL
#include <atomic>
#include <cstdint>
#include <functional> // std::function
#include <mutex>

#ifdef MTK_MFNR
// MFNR Core library
#include <mtkcam/feature/mfnr/MfllTypes.h>
#include <mtkcam/feature/mfnr/IMfllNvram.h>
#include <mtkcam/feature/mfnr/IMfllStrategy.h>
using namespace mfll;
#endif


#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
// FUNCTION_SCOPE
#ifdef __DEBUG
#define FUNCTION_SCOPE          auto __scope_logger__ = create_scope_logger(__FUNCTION__)
#include <memory>
#include <functional>
static std::shared_ptr<char> create_scope_logger(const char* functionName)
{
    char* pText = const_cast<char*>(functionName);
    CAM_LOGD("[%s] + ", pText);
    return std::shared_ptr<char>(pText, [](char* p){ CAM_LOGD("[%s] -", p); });
}
#else
#define FUNCTION_SCOPE          do{}while(0)
#endif
// SCOPE_TIME
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
using namespace NSCam;
using namespace NSCam::plugin;
using namespace NSCam::NSCamFeature::NSFeaturePipe::NSDCMF;
using namespace NS3Av3;
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
DCMFCtrler::
DCMFCtrler(
    NSCamFeature::NSFeaturePipe::NSDCMF::DualCamMFType mode
) : BaseDCCtrler(mode)
{
    FUNCTION_SCOPE;

    if(mMode == _DualCamMFType::TYPE_DCMF_3rdParty){
#ifdef MTK_MFNR
        updateMfllStrategy();                       // frame-per-capture, delay frame
        setHWSyncStableNum(getHWSyncStableCnt());   // hwsync stable count
#else
        // 3rd-party mfnr
        setCaptureNum(queryCaptureFrameCount());    // frame-per-capture, delay frame
        setDelayFrameNum(0);                        // delay frame
        setHWSyncStableNum(getHWSyncStableCnt());   // hwsync stable count
#endif
    }else if(mMode == _DualCamMFType::TYPE_DCMF_3rdParty_HDR){
        updateHdrStrategy();                        // frame-per-capture, delay frame
        setHWSyncStableNum(0);                      // hwsync stable count. Since there is specific AE setting in HDR mode hwsync stable count should be 0
    }else{
        setCaptureNum(queryCaptureFrameCount());    // frame-per-capture
        setDelayFrameNum(0);                        // delay frame
        setHWSyncStableNum(getHWSyncStableCnt());   // hwsync stable count
    }

    mpCB = new DCMFListenerImp(this);
    MY_LOGD("mode(%d) CapNum(%d) DelayNum(%d) HWSyncNum(%d) mpCB(%p)", mMode, getCaptureNum(), getDelayFrameNum(), getHWSyncStableNum(), mpCB.get());
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
DCMFCtrler::
~DCMFCtrler()
{
    FUNCTION_SCOPE;
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
bool
DCMFCtrler::
updateDCMFRequest(
    sp<PipeReq>& request __attribute((unused)),
    std::shared_ptr<RequestFrame>& main1Frame          __attribute((unused)),
    std::shared_ptr<RequestFrame>& main2Frame          __attribute((unused)),
    MBOOL isMainFrame,
    MBOOL isLastFrame
)
{
    FUNCTION_SCOPE;
    MY_LOGD("request(%p) main1Frame(%p) main2Frame(%p) isMainFrame(%d) isLastFrame(%d)",
                    request.get(),
                    main1Frame.get(),
                    main2Frame.get(),
                    isMainFrame, isLastFrame);

    if(isMainFrame){
        request->addParam(DualCamMFParamID::PID_IS_MAIN, 1);
    }

    if(isLastFrame){
        request->addParam(DualCamMFParamID::PID_IS_LAST, 1);
    }

    request->addParam(DualCamMFParamID::PID_ENABLE_MFB, (m_enableMfnr ? 1 : 0));

    request->addParam(DualCamMFParamID::PID_ENABLE_HDR, (m_enableHdr ? 1 : 0));

    request->addParam(DualCamMFParamID::PID_FRAME_NUM, getCaptureNum());

    if(main1Frame != nullptr)
    {
        if(isMainFrame){
            m_mainRequestFrame = main1Frame;
        }
        MY_LOGD("main1Frame: [%d] imgo(%p) rrzo(%p) lsco(%p) jpeg(%p) thumb(%p) resrv(%p)",
                main1Frame->getOpenId(),
                main1Frame->getImageBuffer(RequestFrame::eRequestImg_FullSrc),
                main1Frame->getImageBuffer(RequestFrame::eRequestImg_ResizedSrc),
                main1Frame->getImageBuffer(RequestFrame::eRequestImg_LcsoRaw),
                main1Frame->getImageBuffer(RequestFrame::eRequestImg_FullOut),
                main1Frame->getImageBuffer(RequestFrame::eRequestImg_ThumbnailOut),
                main1Frame->getImageBuffer(RequestFrame::eRequestImg_ReservedOut1));
    }
    if(main2Frame != nullptr)
    {
        MY_LOGD("main2Frame: [%d] imgo(%p) rrzo(%p) lsco(%p) jpeg(%p) thumb(%p)",
                main2Frame->getOpenId(),
                main2Frame->getImageBuffer(RequestFrame::eRequestImg_FullSrc),
                main2Frame->getImageBuffer(RequestFrame::eRequestImg_ResizedSrc),
                main2Frame->getImageBuffer(RequestFrame::eRequestImg_LcsoRaw),
                main2Frame->getImageBuffer(RequestFrame::eRequestImg_FullOut),
                main2Frame->getImageBuffer(RequestFrame::eRequestImg_ThumbnailOut));
    }
    // after main2 finish, mark it to error to manually
    MY_LOGD("mark to error");
    main2Frame->markError(RequestFrame::eRequestImg_All, true);
    main2Frame->markError(RequestFrame::eRequestMeta_All, true);
    return true;
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
bool
DCMFCtrler::
DCMFProcessingDone(
    PipeCB::EventId eventType,
    sp<PipeReq>& request
)
{
    FUNCTION_SCOPE;
    // do somthing you want, and then call DCMFProcessingDone
    BaseDCCtrler::DCMFProcessingDone(eventType, request);
    return true;
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
MUINT32
DCMFCtrler::
queryCaptureFrameCount(
)
{
    FUNCTION_SCOPE;

    IDualCamMFStrategy* pDCMFStrategy = IDualCamMFStrategy::getInstance();

    return pDCMFStrategy->getCaptureFrameCount();
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
bool
DCMFCtrler::
yuvDone(
    PipeCB::EventId eventType  __attribute__((unused)),
    sp<PipeReq>& request
)
{
    FUNCTION_SCOPE;
    // do somthing you want, and then call postviewDone
    BaseDCCtrler::yuvDone(eventType, request);
    return true;
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
android::status_t
DCMFCtrler::
onEvent(
    PipeCB::EventId eventType,
    sp<PipeReq>& request,
    void* param1          __attribute((unused)),
    void* param2          __attribute((unused))
)
{
    FUNCTION_SCOPE;
    auto toString = [](PipeCB::EventId eventType)
    {
        switch(eventType)
        {
            case PipeCB::eCompleted:
                return "eCompleted";
            case PipeCB::eFailed:
                return "eFailed";
            case PipeCB::eShutter:
                return "eShutter";
            case PipeCB::eP2Done:
                return "eP2Done";
            case PipeCB::eFlush:
                return "eFlush";
            case PipeCB::ePostview:
                return "ePostview";
            case PipeCB::eYuvDone:
                return "eYuvDone";
        }
        return "UNKNOWN";
    };
    MY_LOGD("cmd(%s) id(%d)", toString(eventType), eventType);
    switch(eventType)
    {
        case PipeCB::eShutter:
            onShutter();
            break;
        case PipeCB::eP2Done:
            onNextCaptureReady();
            break;
        case PipeCB::eFailed:
        case PipeCB::eFlush:
        case PipeCB::eCompleted:
            DCMFProcessingDone(eventType, request);
            break;
        case PipeCB::ePostview:
            postviewDone(eventType, request);
            break;
        case PipeCB::eYuvDone:
            yuvDone(eventType, request);
            break;
    }
    return OK;
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void
DCMFCtrler::updateMfllStrategy(
)
{
    FUNCTION_SCOPE;
    SCOPE_TIMER(__t0, __FUNCTION__);

    MfllStrategyConfig_t strategyCfg;

#define __DEFAULT_CAPTURE_NUM__ 4
    int captureNum = __DEFAULT_CAPTURE_NUM__; // makes default 4 frame
    int blendNum = __DEFAULT_CAPTURE_NUM__;
#undef  __DEFAULT_CAPTURE_NUM__

    updateCurrent3A();

    int suggestedIso = m_finalRealIso; // current iso from PLINE
    int suggestedExp = m_finalShutterTime;

    sp<IMfllStrategy> pStrategy = IMfllStrategy::createInstance();
    sp<IMfllNvram> spNvramProvider = IMfllNvram::createInstance();

    if (CC_UNLIKELY(pStrategy.get() == nullptr)) {
        MY_LOGE("IMfllStrategy create failed");
        goto LB_EXIT;
    }
    if (CC_UNLIKELY(spNvramProvider.get() == nullptr)) {
        MY_LOGE("create IMfllNvram failed");
        goto LB_EXIT;
    }

    {
        strategyCfg.original_iso = m_realIso;
        strategyCfg.original_exp = m_shutterTime;
        strategyCfg.iso = suggestedIso;
        strategyCfg.exp = suggestedExp;
        strategyCfg.isAis = false;
        if (CC_UNLIKELY(spNvramProvider->init(getMain1OpenId()) != MfllErr_Ok)) {
            MY_LOGE("init IMfllNvram failed");
            goto LB_EXIT;
        }

        if (CC_UNLIKELY(pStrategy->init(spNvramProvider) != MfllErr_Ok)) {
            MY_LOGE("init IMfllStrategy failed");
            goto LB_EXIT;
        }

        if (CC_UNLIKELY(pStrategy->queryStrategy(strategyCfg, &strategyCfg) != MfllErr_Ok)) {
            MY_LOGE("IMfllStrategy::queryStrategy returns error");
            goto LB_EXIT;
        }

#if 1
        const size_t mfllIsoThresholdCount = 2;
        std::vector<int> mfllIsoThresholds = ::getMFLLExecutedRealIsoThresholds();
        if(mfllIsoThresholds.size() == 2)
        {
            const int lowRealIso = mfllIsoThresholds[0];
            const int highRealIso = mfllIsoThresholds[1];

            MY_LOGD("mfllIsoThresholds, low:%d, high:%d", lowRealIso, highRealIso);
            // customized strategy
            if(m_realIso < lowRealIso){
                strategyCfg.frameCapture = 1;
                strategyCfg.enableMfb = false;
            }else if(m_realIso >= lowRealIso && m_realIso < highRealIso){
                strategyCfg.frameCapture = 4;
                strategyCfg.enableMfb = true;
            }else{
                strategyCfg.frameCapture = 6;
                strategyCfg.enableMfb = true;
            }

            if(strategyCfg.frameCapture > 6){ MY_LOGE("strategyCfg.frameCapture > 6");}
            if(strategyCfg.frameCapture < 2 && strategyCfg.enableMfb){ MY_LOGE("strategyCfg.frameCapture < 2 when mfb is enabled");}

            MY_LOGW("customized strategy @ iso(%d) => cap(%d) mfb(%d)",
                m_realIso,
                strategyCfg.frameCapture,
                strategyCfg.enableMfb
            );
        }
        else
        {
            MY_LOGE("invalid mfllIsoThresholdCount count, cur:%zu, desired:%zu", mfllIsoThresholds.size(), mfllIsoThresholdCount);
        }
#endif

        captureNum = static_cast<int>(strategyCfg.frameCapture);
        blendNum = static_cast<int>(strategyCfg.frameBlend);
        suggestedExp = static_cast<int>(strategyCfg.exp);
        suggestedIso = static_cast<int>(strategyCfg.iso);
    }

    captureNum = ::property_get_int32("vendor.dg.dcmf.frameCnt", captureNum);

LB_EXIT:
    // update again
    m_finalRealIso = suggestedIso;
    m_finalShutterTime = suggestedExp;
    m_enableMfnr = strategyCfg.enableMfb;

    if(::property_get_int32("vendor.di.camera.cali.dump_img", 0)){
        MY_LOGW("in calibration mode, force disable mfnr");
        m_enableMfnr = false;
    }

    if(captureNum == 1){
        MY_LOGW("1 frame-per-capture, force disable mfnr");
        m_enableMfnr = false;
    }

    if(m_enableMfnr){
        setCaptureNum(captureNum);
    }else{
        setCaptureNum(1);
    }

#ifdef __DEBUG
    MY_LOGD("-------------");
    MY_LOGD("iso = %d", m_realIso);
    MY_LOGD("exp = %d", m_finalShutterTime);
    MY_LOGD("cap = %d", getCaptureNum());
    MY_LOGD("mfb = %d", m_enableMfnr);
#endif
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void
DCMFCtrler::updateHdrStrategy(
)
{
    FUNCTION_SCOPE;
    SCOPE_TIMER(__t0, __FUNCTION__);

#define __DEFAULT_CAPTURE_NUM__ 2
    int captureNum = __DEFAULT_CAPTURE_NUM__; // makes default 4 frame
#undef  __DEFAULT_CAPTURE_NUM__

LB_EXIT:
   setCaptureNum(captureNum);
   setDelayFrameNum(3); // AE needs 3 frames to recover to stable state

   m_enableHdr = true;
#ifdef __DEBUG
    MY_LOGD("cap = %d", getCaptureNum());
    MY_LOGD("delay = %d", getDelayFrameNum());
#endif
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void
DCMFCtrler::updateCurrent3A()
{
    FUNCTION_SCOPE;
    SCOPE_TIMER(__t0, __FUNCTION__);

    std::unique_ptr <
                    IHal3A,
                    std::function<void(IHal3A*)>
                    > hal3a
            (
                MAKE_Hal3A(getMain1OpenId(), __CALLERNAME__),
                [](IHal3A* p){ if (p) p->destroyInstance(__CALLERNAME__); }
            );

    if (hal3a.get() == nullptr) {
        MY_LOGE("create IHal3A instance failed");
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
        // hal3a->send3ACtrl(E3ACtrl_GetCaptureDelayFrame, reinterpret_cast<MINTPTR>(&delayedFrames), 0);
    }

    // MY_LOGD("3A capture delayed frames(%u)", delayedFrames);
    setDelayFrameNum(delayedFrames);

    MY_LOGD("default strategy config (iso,exp)=(%d,%d)", m_realIso, m_shutterTime);
    if (m_realIso == 0 || m_shutterTime == 0) {
        m_realIso = capParam.u4RealISO;
        m_shutterTime = capParam.u4Eposuretime;
        MY_LOGD("default strategy config is invalid, "\
                "config to current capture params (iso,exp)=(%d,%d)",
                m_realIso, m_shutterTime);
    }

    m_finalRealIso = m_realIso;
    m_finalShutterTime = m_shutterTime;
}
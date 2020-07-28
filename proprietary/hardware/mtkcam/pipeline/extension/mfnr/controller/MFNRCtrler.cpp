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

#define LOG_TAG "MfllController"
static const char* __CALLERNAME__ = LOG_TAG;

#include "MFNRCtrler.h"

// MTKCAM
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/Trace.h>
#include <mtkcam/utils/exif/DebugExifUtils.h>
#include <mtkcam/feature/mfnr/MfllProperty.h>

// STL
#include <atomic>
#include <cstdint>
#include <functional> // std::function
#include <mutex>


#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)

#define MY_DBG_COND(level)          __builtin_expect( m_dbgLevel >= level, false )
#define MY_LOGD3(...)               do { if ( MY_DBG_COND(3) ) MY_LOGD(__VA_ARGS__); } while(0)
// FUNCTION_SCOPE
#ifdef __DEBUG
#define FUNCTION_SCOPE          auto __scope_logger__ = create_scope_logger(__FUNCTION__)
#include <memory>
#include <functional>
static std::shared_ptr<char> create_scope_logger(const char* functionName)
{
    bool bEnableLog = (mfll::MfllProperty::getDebugLevel() >= 3);
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
using namespace NS3Av3;


// ----------------------------------------------------------------------------
// MFNRCtrler
// ----------------------------------------------------------------------------
MFNRCtrler::MFNRCtrler(
        int sensorId,
        MfllMode mfllMode,
        MUINT32 sensorMode,
        int realIso,
        int exposureTime,
        bool isFlashOn)
    : BaseController(__CALLERNAME__)
    , m_openId(sensorId)
    , m_mfbMode(mfllMode)
    , m_realIso(realIso)
    , m_shutterTime(exposureTime)
    , m_finalRealIso(0)
    , m_finalShutterTime(0)
    , m_captureNum(0)
    , m_blendNum(0)
    , m_bDoMfb(false)
    , m_bForceMfnr(false)
    , m_bFullSizeMc(false)
    , m_bFlashOn(isFlashOn)
    , m_delayFrame(0)
    , m_dbgLevel(0)
{
    m_dbgLevel = mfll::MfllProperty::getDebugLevel();

    FUNCTION_SCOPE;
    SCOPE_TIMER(__t0, __FUNCTION__);

    ExposureUpdateMode __3aUpdateMode = ExposureUpdateMode::CURRENT;

    if (mfll::isAisMode(m_mfbMode)) {
        // if it's AIS, update the finial capture param by AIS pipeline
        __3aUpdateMode = ExposureUpdateMode::AIS;
    }

    updateCurrent3A(__3aUpdateMode);

    // -1 means not exists
    m_originalAppSetting.resize(
            static_cast<size_t>(AppSetting::SIZE),
            -1);
}


MFNRCtrler::~MFNRCtrler()
{
    FUNCTION_SCOPE;
}


intptr_t MFNRCtrler::job(
        intptr_t arg1 /* = 0 */     __attribute__((unused)),
        intptr_t arg2 /* = 0 */     __attribute__((unused)))
{
    CAM_TRACE_CALL();
    doMfnr();
    return 0;
}


void MFNRCtrler::makesDebugInfo(
        const std::map<unsigned int, uint32_t>& data,
        IMetadata*                              pMetadata)
{
    FUNCTION_SCOPE;
    SCOPE_TIMER(__t0, __FUNCTION__);

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

    /* set debug information into debug Exif metadata */
    DebugExifUtils::setDebugExif(
            DebugExifUtils::DebugExifType::DEBUG_EXIF_MF,
            static_cast<MUINT32>(MTK_MF_EXIF_DBGINFO_MF_KEY),
            static_cast<MUINT32>(MTK_MF_EXIF_DBGINFO_MF_DATA),
            data,
            &exifMeta);

    /* update debug Exif metadata */
    IMetadata::IEntry entry_exif(MTK_3A_EXIF_METADATA);
    entry_exif.push_back(exifMeta, Type2Type<IMetadata>());
    pMetadata->update(entry_exif.tag(), entry_exif);
}

void MFNRCtrler::restoreAppSetting(IMetadata* pAppMeta)
{
    if (CC_UNLIKELY(pAppMeta == nullptr)) {
        MY_LOGE("%s: pAppMeta is NULL", __FUNCTION__);
        return;
    }

    // restore the original app setting back to pAppMeta if exists (value >= 0)
#define __CONTAINER m_originalAppSetting
#define __PMETADATA pAppMeta
#define __RESTORE_APP_SETTING(TAG, TYPE)                                        \
    if (__CONTAINER[static_cast<size_t>(AppSetting::TAG)] >= 0) {               \
        IMetadata::setEntry<TYPE>(                                              \
                __PMETADATA,                                                    \
                MTK_##TAG,                                                      \
                static_cast<TYPE>(                                              \
                    __CONTAINER[static_cast<size_t>(AppSetting::TAG)]));        \
    }                                                                           \
    else {                                                                      \
        __PMETADATA->remove(MTK_##TAG);                                         \
    }
    //
    __RESTORE_APP_SETTING(CONTROL_AE_MODE,                      MUINT8);
    __RESTORE_APP_SETTING(CONTROL_AWB_LOCK,                     MUINT8);
    __RESTORE_APP_SETTING(CONTROL_VIDEO_STABILIZATION_MODE,     MUINT8);
    __RESTORE_APP_SETTING(SENSOR_SENSITIVITY,                   MINT32);
    __RESTORE_APP_SETTING(SENSOR_EXPOSURE_TIME,                 MINT64);
    //
#undef __RESTORE_APP_SETTING
#undef __PMETADATA
#undef __CONTAINER
}

void MFNRCtrler::updateMfllStrategy()
{
}


bool MFNRCtrler::initMfnrCore()
{
    return true;
}


bool MFNRCtrler::doMfnr()
{
    return true;
}


void MFNRCtrler::doCancel()
{
}


void MFNRCtrler::updateCurrent3A(ExposureUpdateMode mode)
{
    FUNCTION_SCOPE;
    SCOPE_TIMER(__t0, __FUNCTION__);

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
    }

    MY_LOGD3("3A capture delayed frames(%u)", delayedFrames);
    setDelayFrameNum(delayedFrames);

    MY_LOGD3("default strategy config (iso,exp)=(%d,%d)", m_realIso, m_shutterTime);
    MY_LOGD3("current capture params (iso,exp)=(%d,%d)", capParam.u4RealISO, capParam.u4Eposuretime);
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
        break;

    case ExposureUpdateMode::MFNR:
    default:
        m_finalRealIso = m_realIso;
        m_finalShutterTime = m_shutterTime;
        break;
    }
}

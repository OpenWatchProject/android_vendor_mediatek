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
#define LOG_TAG "AinrCtrler"
static const char* __CALLERNAME__ = LOG_TAG;

//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/aaa/IHal3A.h>
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/aaa/aaa_hal_common.h>

//
#include <stdlib.h>
#include <utils/Errors.h>
#include <utils/List.h>
#include <utils/RefBase.h>
#include <cutils/properties.h>
#include <sstream>
//
#include "AinrCtrler.h"

//
using namespace NSCam;
using namespace plugin;
using namespace android;
using namespace ainr;
using namespace std;
using namespace NS3Av3;

/******************************************************************************
 *
 ******************************************************************************/
#include <mtkcam/utils/hw/HwInfoHelper.h>
//
#include <mtkcam3/feature/ainr/AinrUlog.h>

/******************************************************************************
*
******************************************************************************/
AinrCtrler::AinrCtrler(
        int uniqueKey,
        int sensorId,
        AinrMode mfllMode,
        int realIso,
        int exposureTime,
        bool isFlashOn)
    : m_uniqueKey(uniqueKey)
    , m_openId(sensorId)
    , m_mfbMode(mfllMode)
    , m_realIso(realIso)
    , m_shutterTime(exposureTime)
    , m_finalRealIso(0)
    , m_finalShutterTime(0)
    , m_captureNum(0)
    , m_blendNum(0)
    , m_bDoAinr(false)
    , m_bForceAinr(false)
    , m_bFlashOn(isFlashOn)
    , m_bFlashAinrSupport(false)
    , m_dummyFrame(0)
    , m_delayFrame(0)
    , m_dgnGain(0)
    , m_dbgLevel(0)
    , m_droppedFrameNum(0)
    , m_bNeedExifInfo(false)
    , m_bFired(false)
    , m_bInit(false)
    , m_pCore(nullptr)
    , m_spNvramProvider(nullptr)
    , m_imgoStride(0)
    , mAlgoType(AINR_SINGLE)
{
    CAM_ULOGM_FUNCLIFE_ALWAYS();
    ExposureUpdateMode __3aUpdateMode = ExposureUpdateMode::CURRENT;

    if (ainr::isAisMode(m_mfbMode)) {
        // if it's AIS, update the finial capture param by AIS pipeline
        __3aUpdateMode = ExposureUpdateMode::AIS;
    }

    updateCurrent3A(__3aUpdateMode);


    m_dbgLevel = ::property_get_int32("vendor.debug.ainr.log", 0);
}

AinrCtrler::~AinrCtrler()
{
    CAM_ULOGM_FUNCLIFE_ALWAYS();
}

void AinrCtrler::updateAinrStrategy(NSCam::MSize size)
{
    CAM_ULOGM_APILIFE();

    #define __DEFAULT_CAPTURE_NUM__ 6
    int captureNum = __DEFAULT_CAPTURE_NUM__; // makes default 6 frame
    #undef  __DEFAULT_CAPTURE_NUM__

    std::shared_ptr<IAinrStrategy> pStrategy = IAinrStrategy::createInstance();
    //reload
    m_spNvramProvider = IAinrNvram::createInstance();

    if (CC_UNLIKELY(pStrategy.get() == nullptr)) {
        ainrLogE("IAinrStrategy create failed");
        return;
    }
    if (CC_UNLIKELY(m_spNvramProvider.get() == nullptr)) {
        ainrLogE("create IAinrNvram failed");
        return;
    }


    IAinrNvram::ConfigParams nvramCfg;
    nvramCfg.iSensorId = getOpenId();
    nvramCfg.iso       = getFinalIso();

    if (CC_UNLIKELY(m_spNvramProvider->init(nvramCfg) != AinrErr_Ok)) {
        ainrLogE("init IAinrNvram failed");
        return;
    }

    if (CC_UNLIKELY(pStrategy->init(m_spNvramProvider) != AinrErr_Ok)) {
        ainrLogE("init IAinrStrategy failed");
        return;
    }

    AinrStrategyConfig_t strategyCfg;
    strategyCfg.ainrIso = getFinalIso();
    strategyCfg.size = size;
    if (CC_UNLIKELY(pStrategy->queryStrategy(strategyCfg, &strategyCfg) != AinrErr_Ok)) {
        ainrLogE("IAinrStrategy::queryStrategy returns error");
        return;
    }

    captureNum = static_cast<int>(strategyCfg.frameCapture);

    // update again
    setCaptureNum(captureNum);
    setEnableAinr(strategyCfg.enableAinr);
}

bool AinrCtrler::initAinrCore()
{
    CAM_ULOGM_APILIFE();

    std::lock_guard<decltype(m_pCoreMx)> _l(m_pCoreMx);

    if (m_bFired) {
        ainrLogE("Ainr has been fired, cannot fire twice");
        return false;
    }

    // check blending/capture number
    if (getCaptureNum() <= 0) {
        ainrLogE("capture or blend number is wrong (0)");
        return false;
    }

    // create instance.
    if(m_pCore.get() == nullptr) {
        m_pCore = IAinrCore::createInstance();
    }

    if ( CC_UNLIKELY(m_pCore.get() == nullptr) ) {
        ainrLogE("create ainr Core Library failed");
        return false;
    }

    // TODO: Need it?
    m_pCore->setNvramProvider(m_spNvramProvider);

    // prepare AinrConfig
    AinrConfig cfg;
    cfg.sensor_id  = getOpenId();
    cfg.captureNum = getCaptureNum();
    cfg.requestNum = m_requestKey;
    cfg.frameNum   = m_frameKey;
    cfg.uniqueKey  = m_uniqueKey;
    cfg.dgnGain    = m_dgnGain;
    cfg.imgoWidth  = m_sizeImgo.w;
    cfg.imgoHeight = m_sizeImgo.h;
    cfg.rrzoWidth  = m_sizeRrzo.w;
    cfg.rrzoHeight = m_sizeRrzo.h;
    cfg.imgoStride = m_imgoStride;
    cfg.algoType     = mAlgoType;

    // Dump for debug
    ainrLogD("Ainr core config captureNum(%d)", cfg.captureNum);

    if (m_pCore->init(cfg) != AinrErr_Ok) {
        ainrLogE("Init AINR Core returns fail");
        return false;
    }

    return true;
}

bool AinrCtrler::execute()
{
    CAM_ULOGM_APILIFE();

    std::lock_guard<decltype(m_pCoreMx)> _l(m_pCoreMx);

    if (m_bFired) {
        ainrLogE("Ainr has been fired, cannot fire twice");
        return false;
    }

    if (m_pCore.get() == nullptr) {
        ainrLogE("Ainr Core is NULL");
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
            auto r = this->doAinr();
            return r;
        });

        m_futureExe = std::shared_future<bool>(std::move(fu));

        return true;
    }
    return false;

}

bool AinrCtrler::waitExecution(intptr_t* result /* = nullptr */)
{
    CAM_ULOGM_APILIFE();

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

AinrCtrler::ExecutionStatus
AinrCtrler::getExecutionStatus() const
{
    CAM_ULOGM_APILIFE();

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

void AinrCtrler::doCancel()
{
    CAM_ULOGM_APILIFE();

    std::lock_guard<decltype(m_pCoreMx)> _l(m_pCoreMx);
    if (m_pCore.get()) {
        m_pCore->doCancel(); // async call
    }
}

void AinrCtrler::updateCurrent3A(ExposureUpdateMode mode)
{
    CAM_ULOGM_APILIFE();

    std::unique_ptr <
                    IHal3A,
                    std::function<void(IHal3A*)>
                    > hal3a
            (
                MAKE_Hal3A(m_openId, __CALLERNAME__),
                [](IHal3A* p){ if (p) p->destroyInstance(__CALLERNAME__); }
            );

    if (hal3a.get() == nullptr) {
        ainrLogE("create IHal3A instance failed");
        m_bFlashAinrSupport = false;
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
        m_bFlashAinrSupport = hal3a->send3ACtrl(E3ACtrl_ChkMFNRFlash, 0, 0);
    }

    ainrLogD("3A capture delayed frames(%u)", delayedFrames);
    setDelayFrameNum(delayedFrames);

    ainrLogD("default strategy config (iso,exp)=(%d,%d)", m_realIso, m_shutterTime);
    ainrLogD("current capture params (iso,exp)=(%d,%d)", capParam.u4RealISO, capParam.u4Eposuretime);
    if (m_realIso == 0 || m_shutterTime == 0) {
        m_realIso = capParam.u4RealISO;
        m_shutterTime = capParam.u4Eposuretime;
        ainrLogW("default strategy config is invalid, "\
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
        ainrLogD("AIS P-line config (iso,exp)=(%d,%d)", m_finalRealIso, m_finalShutterTime);
        break;

    case ExposureUpdateMode::MFNR:
    default:
        m_finalRealIso = m_realIso;
        m_finalShutterTime = m_shutterTime;
        break;
    }
}


void AinrCtrler::configAinrCore(IMetadata* pHalMeta, IMetadata* pMetaDynamic)
{
    CAM_ULOGM_APILIFE();

    // set middleware info to ainr Core Lib for debug dump
    {
        if (!IMetadata::getEntry<MINT32>(pHalMeta,
                MTK_PIPELINE_REQUEST_NUMBER, m_requestKey)) {
            ainrLogW("Get MTK_PIPELINE_REQUEST_NUMBER fail");
        }

        if (!IMetadata::getEntry<MINT32>(pHalMeta,
                MTK_PIPELINE_FRAME_NUMBER, m_frameKey)) {
            ainrLogW("Get MTK_PIPELINE_FRAME_NUMBER fail");
        }

        int iso = 0;
        if (!IMetadata::getEntry<MINT32>(pMetaDynamic,
                MTK_SENSOR_SENSITIVITY, iso)) {
            ainrLogW("Get MTK_SENSOR_SENSITIVITY fail");
        }

        int64_t shutterTime = 0;  // nanoseconds (ns)
        if (!IMetadata::getEntry<MINT64>(pMetaDynamic,
                MTK_SENSOR_EXPOSURE_TIME, shutterTime)) {
            ainrLogW("Get MTK_SENSOR_EXPOSURE_TIME fail");
        }

        int magicNum = 0;
        if (!IMetadata::getEntry<MINT32>(pHalMeta,
                MTK_P1NODE_PROCESSOR_MAGICNUM, magicNum)) {
            ainrLogW("Get MTK_P1NODE_PROCESSOR_MAGICNUM fail");
        }

        // Get DGN gain
        IMetadata::Memory dgnGainInfo;
        dgnGainInfo.resize(sizeof(HAL3APerframeInfo_T));
        if (!IMetadata::getEntry<IMetadata::Memory>(pHalMeta,
                MTK_3A_PERFRAME_INFO, dgnGainInfo)) {
            ainrLogW("Get MTK_3A_PERFRAME_INFO fail");
        }
        HAL3APerframeInfo_T *r3APerframeInfo
                = reinterpret_cast<HAL3APerframeInfo_T*>(dgnGainInfo.editArray());
        m_dgnGain = r3APerframeInfo->u4P1DGNGain;

        ainrLogD("requestNum(%d) frameNum(%d) magicNum(%d)",
                m_requestKey, m_frameKey, magicNum);
        ainrLogD("dgnGain(%d) iso(%d) exp=%" PRIi64,
                m_dgnGain, iso, shutterTime);

        // Query ainr iso from P1 app result. Which may be saved in EXIF
        // Release previous nvram provider due to the previous is for preview
        m_spNvramProvider = nullptr;
        m_spNvramProvider = IAinrNvram::createInstance();

        IAinrNvram::ConfigParams nvramCfg;
        nvramCfg.iSensorId = getOpenId();
        nvramCfg.iso = iso;

        ainrLogD("Re-load nvram data");
        if (CC_UNLIKELY(m_spNvramProvider->init(nvramCfg) != AinrErr_Ok)) {
            ainrLogE("init IAinrNvram failed");
            return;
        }
    }

    // update request Exif
    {
        MUINT8 isRequestExif = 0;
        MBOOL result = MFALSE;

        result = IMetadata::getEntry<MUINT8>(pHalMeta, MTK_HAL_REQUEST_REQUIRE_EXIF, isRequestExif);
        m_bNeedExifInfo = (isRequestExif != 0);
    }

    // Inint ainrcore;
    initAinrCore();
}

bool AinrCtrler::doAinr()
{
    CAM_ULOGM_APILIFE();

    // TODO: Need to merge these three blocks????
    // prepare
    {
        std::lock_guard<decltype(m_pCoreMx)> _l(m_pCoreMx);

        if (m_bFired) {
            ainrLogE("ainr has been fired, cannot fire twice");
            return false;
        }

        if( CC_UNLIKELY(m_pCore.get() == nullptr) ) {
            ainrLogE("ainr Core is NULL");
            return false;
        }

        m_bFired = true;
    }
    // do AINR
    {
        std::lock_guard<decltype(m_pCoreMx)> _l(m_pCoreMx);
        // We add data package in the final stage. To ensure
        // we colloect all the buffers/Meta we wanted.
        m_pCore->addInput(m_vDataPack);
        m_pCore->doAinr();
    }
    // done
    {
        std::lock_guard<decltype(m_pCoreMx)> _l(m_pCoreMx);

        m_bFired = false;
        m_spNvramProvider = nullptr;
        m_pCore = nullptr;
    }

    return true;
}

void AinrCtrler::addInputData(const AinrPipelinePack & inputPack)
{
    std::lock_guard<decltype(m_pCoreMx)> _l(m_pCoreMx);
    m_vDataPack.push_back(inputPack);
}

void AinrCtrler::addOutData(IImageBuffer *outputBuffer)
{
    std::lock_guard<decltype(m_pCoreMx)> _l(m_pCoreMx);
    m_pCore->addOutput(outputBuffer);
}

void AinrCtrler::registerCB(std::function<void(int32_t, int32_t)> cb)
{
    std::lock_guard<decltype(m_pCoreMx)> _l(m_pCoreMx);
    m_pCore->registerCallback(cb);
}

AINRCORE_VERSION AinrCtrler::queryCoreVersion() const
{
    std::shared_ptr<IAinrCore> core = IAinrCore::createInstance();
    if (CC_UNLIKELY(core.get() == nullptr)) {
        ainrLogF("ainr core is NULL cannot query core version");
        return AINRCORE_VERSION_1_0;
    }
    return core->queryVersion();
}

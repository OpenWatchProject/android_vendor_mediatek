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
 * MediaTek Inc. (C) 2019. All rights reserved.
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
#define LOG_TAG "AiHdrShot"

//
#include "AiHdrShot.h"

//
#include <mtkcam/aaa/IHal3A.h>
#include <mtkcam/aaa/aaa_hal_common.h>
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/utils/hw/HwInfoHelper.h>
#include <mtkcam3/feature/ainr/AinrUlog.h>

// STD
#include <cstdlib>
#include <stdlib.h>
#include <sstream>

// AOSP
#include <utils/Errors.h>
#include <utils/List.h>
#include <utils/RefBase.h>
#include <cutils/properties.h>

// Custom tuning
#include <isp_tuning/ver1/isp_tuning_cam_info.h>  // For RAWIspCamInfo

//
using namespace ainr;
using namespace std;
using namespace NS3Av3;

/******************************************************************************
*
******************************************************************************/
namespace NSCam {
namespace NSPipelinePlugin {

AiHdrShot::AiHdrShot(
        int uniqueKey,
        int sensorId,
        int realIso,
        int exposureTime)
    : m_uniqueKey(uniqueKey)
    , m_openId(sensorId)
    , m_realIso(realIso)
    , m_shutterTime(exposureTime)
    , m_captureNum(0)
    , m_bDoAinr(false)
    , m_delayFrame(0)
    , m_requestKey(0)
    , m_frameKey(0)
    , m_dgnGain(0)
    , m_obOffset(0)
    , m_imgoStride(0)
    , m_algoType(AIHDR)
    , m_bFired(false)
    , m_spNvramProvider(nullptr)
    , mStrategyDone(false)
    , mBssEvType(exposureType::SE)
    , m_mainFrameIndex(0)
    , m_pCore(nullptr) {
    CAM_ULOGM_FUNCLIFE_ALWAYS();
}

AiHdrShot::~AiHdrShot() {
    CAM_ULOGM_FUNCLIFE_ALWAYS();
}

void AiHdrShot::updateAinrStrategy(NSCam::MSize size) {
    CAM_ULOGM_APILIFE();

    std::shared_ptr<IAinrNvram> nvramProvider = IAinrNvram::createInstance();
    if (CC_UNLIKELY(nvramProvider.get() == nullptr)) {
        ainrLogF("create IAinrNvram failed");
        return;
    }

    IAinrNvram::ConfigParams nvramCfg;
    nvramCfg.iSensorId = getOpenId();
    nvramCfg.iso = m_realIso;
    nvramCfg.featureType = AIHDR;
    if (CC_UNLIKELY(nvramProvider->init(nvramCfg) != AinrErr_Ok)) {
        ainrLogF("init IAinrNvram failed");
        return;
    }

    std::shared_ptr<IAinrStrategy> pStrategy = IAinrStrategy::createInstance();
    if (CC_UNLIKELY(pStrategy.get() == nullptr)) {
        ainrLogF("IAinrStrategy create failed");
        return;
    }

    if (CC_UNLIKELY(pStrategy->init(nvramProvider) != AinrErr_Ok)) {
        ainrLogF("init IAinrStrategy failed");
        return;
    }

    // Update strategy
    AinrStrategyConfig_t strategyCfg;
    strategyCfg.ainrIso = m_realIso;
    strategyCfg.size = size;
    if (CC_UNLIKELY(pStrategy->queryStrategy(strategyCfg, &strategyCfg) != AinrErr_Ok)) {
        ainrLogF("IAinrStrategy::queryStrategy returns error");
        return;
    }

    mBssEvType = strategyCfg.bssEvTypes;
    mEvTypes = strategyCfg.vEvTypes;

    // Release nvramProvider
    nvramProvider = nullptr;

    // update again
    m_captureNum = static_cast<int>(strategyCfg.frameCapture);
    m_bDoAinr = strategyCfg.enableAinr;

    // Strategy done
    mStrategyDone = true;
}

void AiHdrShot::updateBssToMeta(const int frameIdx,
        IMetadata* pAppMeta, IMetadata* pHalMeta) {
    if (mStrategyDone.load(std::memory_order_relaxed)
            == false) {
        ainrLogF("Please update strategy first");
        return;
    }

    if (CC_UNLIKELY(pAppMeta == nullptr)
            || CC_UNLIKELY(pHalMeta == nullptr)) {
        ainrLogF("metadata pointer is nullptr");
        return;
    }

    if (frameIdx < 0
            || mEvTypes.empty()
            || frameIdx >= mEvTypes.size()) {
        ainrLogW("frame index(%d) is not in the range(%zu)", frameIdx,
                mEvTypes.size());
        return;
    }

    // BSS initial
    IMetadata::setEntry<MINT32>(pHalMeta,
            MTK_FEATURE_BSS_SELECTED_FRAME_COUNT, m_captureNum);
    IMetadata::setEntry<MINT32>(pHalMeta, MTK_FEATURE_BSS_FORCE_DROP_NUM, 0);

    // No need to do reorder
    IMetadata::setEntry<MBOOL>(pHalMeta, MTK_FEATURE_BSS_REORDER, MFALSE);

    // Judge whether to be referenced as BSS
    MINT32 processEnable = MTK_FEATURE_BSS_PROCESS_DISABLE;
    if (mEvTypes[frameIdx] == mBssEvType) {
        processEnable = MTK_FEATURE_BSS_PROCESS_ENABLE;
        ainrLogD("Enable bss at index(%d) evtype(%d)", frameIdx, mBssEvType);
    }
    IMetadata::setEntry<MINT32>(pHalMeta,
                MTK_FEATURE_BSS_PROCESS, processEnable);
}

void AiHdrShot::updateEvToMeta(const int frameIdx,
        IMetadata* pAppMeta, IMetadata* pHalMeta) {
    if (mStrategyDone.load(std::memory_order_relaxed)
            == false) {
        ainrLogF("Please update strategy first");
        return;
    }

    if (CC_UNLIKELY(pAppMeta == nullptr)
            || CC_UNLIKELY(pHalMeta == nullptr)) {
        ainrLogF("metadata pointer is nullptr");
        return;
    }

    if (frameIdx < 0
            || mEvTypes.empty()
            || frameIdx >= mEvTypes.size()) {
        ainrLogW("frame index(%d) is not in the range(%zu)", frameIdx,
                mEvTypes.size());
        return;
    }

    MINT32 evType = MTK_3A_FEATURE_AE_EXPOSURE_LEVEL_NONE;
    switch (mEvTypes[frameIdx]) {
        case exposureType::SE:
            evType = MTK_3A_FEATURE_AE_EXPOSURE_LEVEL_SHORT;
        break;
        case exposureType::NE:
            evType = MTK_3A_FEATURE_AE_EXPOSURE_LEVEL_NORMAL;
        break;
        case exposureType::LE:
            evType = MTK_3A_FEATURE_AE_EXPOSURE_LEVEL_LONG;
        break;
        default:
            ainrLogD("not support evType(%d)", mEvTypes[frameIdx]);
        break;

    }
    IMetadata::setEntry<MINT32>(pHalMeta, MTK_3A_FEATURE_AE_EXPOSURE_LEVEL, evType);
    ainrLogD("Update frame index(%d) evType(%d)", frameIdx, evType);
}

bool AiHdrShot::initAinrCore() {
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
    if (m_pCore.get() == nullptr) {
        m_pCore = IAinrCore::createInstance();
    }

    if ( CC_UNLIKELY(m_pCore.get() == nullptr) ) {
        ainrLogE("create ainr Core Library failed");
        return false;
    }

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
    cfg.algoType   = m_algoType;
    cfg.needDRC    = true;
    if (m_pCore->init(cfg) != AinrErr_Ok) {
        ainrLogE("Init AINR Core returns fail");
        return false;
    }

    return true;
}

bool AiHdrShot::execute() {
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

    bool bStart = false;  // flag represents if it's need to start a job

    // std::shared_future::valid
    if (m_futureExe.valid()) {
        switch (m_futureExe.wait_for(std::chrono::seconds(0))) {
        case std::future_status::deferred:  // Job hasn't been executed yet.
        case std::future_status::ready:  // Job has finished.
            bStart = true;  // It's ok to start a new job.
            break;
        case std::future_status::timeout:  // Job is still in executing, return false.
            return false;
        default:{};
        }
    } else {
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

bool AiHdrShot::waitExecution(intptr_t* result /* = nullptr */) {
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
    } else {
        return false;
    }

    return false;
}

AiHdrShot::ExecutionStatus
AiHdrShot::getExecutionStatus() const {
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

void AiHdrShot::doCancel() {
    CAM_ULOGM_APILIFE();

    std::lock_guard<decltype(m_pCoreMx)> _l(m_pCoreMx);
    if (m_pCore.get()) {
        m_pCore->doCancel();  // async call
    }
}

void AiHdrShot::configAinrCore(const IMetadata* pHalMeta, const IMetadata* pMetaDynamic) {
    CAM_ULOGM_APILIFE();

    // set middleware info to ainr Core Lib for debug dump
    {
        int requestNum = 0;
        if (!IMetadata::getEntry<MINT32>(pHalMeta, MTK_PIPELINE_REQUEST_NUMBER, requestNum)) {
            ainrLogW("Get MTK_PIPELINE_REQUEST_NUMBER fail");
        }

        int frameNum = 0;
        if (!IMetadata::getEntry<MINT32>(pHalMeta, MTK_PIPELINE_FRAME_NUMBER, frameNum)) {
            ainrLogW("Get MTK_PIPELINE_FRAME_NUMBER fail");
        }

        int iso = 0;
        if (!IMetadata::getEntry<MINT32>(pMetaDynamic, MTK_SENSOR_SENSITIVITY, iso)) {
            ainrLogW("Get MTK_SENSOR_SENSITIVITY fail");
        }

        int64_t shutterTime = 0;  // nanoseconds (ns)
        if (!IMetadata::getEntry<MINT64>(pMetaDynamic, MTK_SENSOR_EXPOSURE_TIME, shutterTime)) {
            ainrLogW("Get MTK_SENSOR_EXPOSURE_TIME fail");
        }

        int magicNum = 0;
        if (!IMetadata::getEntry<MINT32>(pHalMeta, MTK_P1NODE_PROCESSOR_MAGICNUM, magicNum)) {
            ainrLogW("Get MTK_P1NODE_PROCESSOR_MAGICNUM fail");
        }

        // Get DGN gain
        IMetadata::Memory dgnGainInfo;
        dgnGainInfo.resize(sizeof(HAL3APerframeInfo_T));
        if (!IMetadata::getEntry<IMetadata::Memory>(pHalMeta, MTK_3A_PERFRAME_INFO, dgnGainInfo)) {
            ainrLogW("Get MTK_3A_PERFRAME_INFO fail");
        }
        HAL3APerframeInfo_T *r3APerframeInfo = reinterpret_cast<HAL3APerframeInfo_T *>(dgnGainInfo.editArray());

        m_requestKey = requestNum;
        m_frameKey   = frameNum;
        m_dgnGain    = r3APerframeInfo->u4P1DGNGain;
        ainrLogD("requestNum(%d) frameNum(%d) magicNum(%d) dgnGain(%d) iso(%d) exp=%" PRIi64, m_requestKey, m_frameKey, magicNum, m_dgnGain, iso, shutterTime);

        // Query ainr iso from P1 app result. Which may be saved in EXIF
        // Release previous nvram provider due to the previous is for preview
        ainrLogD("Re-load nvram data");
        m_spNvramProvider = IAinrNvram::createInstance();
        if (CC_UNLIKELY(m_spNvramProvider.get() == nullptr)) {
            ainrLogF("create m_spNvramProvider failed");
            return;
        }

        IAinrNvram::ConfigParams nvramCfg;
        nvramCfg.iSensorId = getOpenId();
        nvramCfg.iso = iso;
        nvramCfg.featureType = AIHDR;
        if (CC_UNLIKELY(m_spNvramProvider->init(nvramCfg) != AinrErr_Ok)) {
            ainrLogE("init m_spNvramProvider failed");
            return;
        }
    }

    // Inint ainrcore;
    initAinrCore();
}

bool AiHdrShot::doAinr() {
    CAM_ULOGM_APILIFE();

    // prepare
    {
        std::lock_guard<decltype(m_pCoreMx)> _l(m_pCoreMx);

        if (m_bFired) {
            ainrLogE("ainr has been fired, cannot fire twice");
            return false;
        }

        if (CC_UNLIKELY(m_pCore.get() == nullptr)) {
            ainrLogE("ainr Core is NULL");
            return false;
        }

        m_bFired = true;
    }
    // do AINR
    // We add data package in the final stage. To ensure
    // we colloect all the buffers/Meta we wanted.
    {
        std::lock_guard<decltype(m_pCoreMx)> _l(m_pCoreMx);
        m_pCore->addInput(m_vDataPack);
        m_pCore->doAinr();
        m_mainFrameIndex = m_pCore->queryMainFrameIndex();
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

void AiHdrShot::addInputData(const AinrPipelinePack & inputPack) {
    std::lock_guard<decltype(m_pCoreMx)> _l(m_pCoreMx);
    m_vDataPack.push_back(inputPack);
}

void AiHdrShot::addOutData(IImageBuffer *outputBuffer) {
    std::lock_guard<decltype(m_pCoreMx)> _l(m_pCoreMx);
    m_pCore->addOutput(outputBuffer);
}

void AiHdrShot::registerCB(std::function<void(int32_t, int32_t)> cb) {
    std::lock_guard<decltype(m_pCoreMx)> _l(m_pCoreMx);
    m_pCore->registerCallback(cb);
}

AINRCORE_VERSION AiHdrShot::queryCoreVersion() const {
    std::shared_ptr<IAinrCore> core = IAinrCore::createInstance();
    if (CC_UNLIKELY(core.get() == nullptr)) {
        ainrLogF("ainr core is NULL cannot query core version");
        return AINRCORE_VERSION_1_0;
    }
    return core->queryVersion();
}

int32_t AiHdrShot::queryMainFrameIndex() const {
    return m_mainFrameIndex;
}

}  // namespace NSPipelinePlugin
}  // namespace NSCam

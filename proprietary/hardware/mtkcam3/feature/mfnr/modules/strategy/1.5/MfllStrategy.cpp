/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2018. All rights reserved.
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
#define LOG_TAG "MfllCore/Strategy"

#include "MfllStrategy.h"

// MFNR public headers
#include <mtkcam3/feature/mfnr/MfllProperty.h>
#include <mtkcam3/feature/mfnr/MfllLog.h>

// MTKCAM
#include <mtkcam/utils/hw/GyroCollector.h>
#include <mtkcam/utils/hw/HwInfoHelper.h> // NSCamHw::HwInfoHelper
#include <mtkcam/drv/IHalSensor.h> // NSCam::SENSOR_SCENARIO_ID_NORMAL_CAPTURE
#if MTK_CAM_NEW_NVRAM_SUPPORT
#include "MfllNvram.h"
#endif
#include <camera_custom_nvram.h>
#include <custom/feature/mfnr/camera_custom_mfll.h> // CUST_MFLL_ENABLE_CONTENT_AWARE_AIS

// AOPS
#include <utils/SystemClock.h> // elapsedRealtime(), uptimeMillis()
#include <cutils/compiler.h>

// STL
#include <chrono>
#include <fstream>

using android::sp;
using namespace mfll;

//-----------------------------------------------------------------------------
IMfllStrategy* IMfllStrategy::createInstance()
{
    return (IMfllStrategy*)new MfllStrategy;
}
//-----------------------------------------------------------------------------
void IMfllStrategy::destroyInstance()
{
    decStrong((void*)this);
}
//-----------------------------------------------------------------------------
// MfllStrategy implementation
//-----------------------------------------------------------------------------
MfllStrategy::MfllStrategy()
: m_mtkAisInited(false)
, m_nvramChunkMfnr(NULL)
, m_nvramChunkMfnrTh(NULL)
, m_captureFrameNum(MFLL_CAPTURE_FRAME)
, m_blendFrameNum(MFLL_BLEND_FRAME)
{
    mfllTraceCall();

    mfllTraceBegin("create MTKAis instance");
    m_mtkAis = std::shared_ptr<MTKAis>(
            MTKAis::createInstance(DRV_AIS_OBJ_SW),
            [this](auto *obj)->void {
                mfllTraceBegin("del_MTKAis");
                if (CC_LIKELY(obj)) {
                    // check inited or not.
                    if (CC_LIKELY( this->m_mtkAisInited.load() )) {
                        obj->AisReset();
                    }
                    obj->destroyInstance();
                }
                mfllTraceEnd();
            }
    );
    mfllTraceEnd();

    if (m_mtkAis.get() == NULL) {
        mfllLogE("create MTKAis failed");
        return;
    }

    AIS_INIT_PARAM _initParam;
    _initParam.eProcID = AIS_PROC2;
    MRESULT result = S_AIS_OK;
    mfllTraceBegin("init MTKAis");

    // If AisInit return S_AIS_OK, we MUST invoke MTKAis::AisReset for avoiding
    // memory leakage, but if AisInit returns failed, we CANNOT invoke
    // MTKAis::AisReset, or an exception will happen.
    result = m_mtkAis->AisInit(&_initParam, NULL);
    mfllTraceEnd();
    if (CC_UNLIKELY( result != S_AIS_OK )) {
        mfllLogE("init MTKAis failed with code %#x", result);
        m_mtkAis = nullptr;
        return;
    }
    else {
        m_mtkAisInited.store(true);
    }
}
//-----------------------------------------------------------------------------
MfllStrategy::~MfllStrategy()
{
    // wait future done (timeout 1 second)
    if (CC_LIKELY( m_futureWorkingBuffer.valid() )) {
        try {
            auto _status = m_futureWorkingBuffer.wait_for(std::chrono::seconds(1));
            if (CC_UNLIKELY( _status != std::future_status::ready )) {
                mfllLogE("wait future status is not ready");
            }
        }
        catch (std::exception&) {
            mfllLogE("m_futureWorkingBuffer::wait_for throws exception");
        }
    }

    m_mtkAis = nullptr;
}
//-----------------------------------------------------------------------------
enum MfllErr MfllStrategy::init(sp<IMfllNvram> &nvramProvider)
{
    mfllTraceCall();

    if (nvramProvider.get() == NULL) {
        mfllLogE("%s: nvram provider is NULL", __FUNCTION__);
        return MfllErr_BadArgument;
    }

    if (m_mtkAis.get() == NULL) {
        mfllLogE("%s: MTKAis instance is NULL", __FUNCTION__);
        return MfllErr_NullPointer;
    }

    /* read NVRAM */
    size_t chunkSize = 0;
    const char *pChunkMfnr = nvramProvider->getChunkMfnr(&chunkSize);
    const char *pChunkMfnrTh = nvramProvider->getChunkMfnrTh(&chunkSize);
    if (pChunkMfnr == NULL) {
        mfllLogE("%s: read NVRAM mfnr failed", __FUNCTION__);
        return MfllErr_UnexpectedError;
    }
    if (pChunkMfnrTh == NULL) {
        mfllLogE("%s: read NVRAM mfnrTh failed", __FUNCTION__);
        return MfllErr_UnexpectedError;
    }
    m_nvramChunkMfnr = pChunkMfnr;
    m_nvramChunkMfnrTh = pChunkMfnrTh;

    char *pMutableChunkMfnr = const_cast<char*>(pChunkMfnr);

    /* reading NVRAM */
    NVRAM_CAMERA_FEATURE_MFLL_STRUCT* pNvramMfnr = reinterpret_cast<NVRAM_CAMERA_FEATURE_MFLL_STRUCT*>(pMutableChunkMfnr);
    m_captureFrameNum = pNvramMfnr->capture_M;
    m_blendFrameNum = pNvramMfnr->blend_N;
    m_isFullSizeMc =  1;

    /* ask AIS working buffer size */
    {
        mfllTraceName("create_thd_ais_buf");
        AIS_GET_PROC2_INFO _getParam = {0};
        auto _result = m_mtkAis->AisFeatureCtrl(
                AIS_FTCTRL_GET_PROC2_INFO,
                (void*)&_getParam,
                nullptr);
        if (CC_UNLIKELY( _result != S_AIS_OK )) {
            mfllLogE("%s: AisFeatureCtrl GET_PROC2_INFO failed", __FUNCTION__);
            return MfllErr_UnexpectedError;
        }

        /* async prepare working buffer */
        m_futureWorkingBuffer = std::async( std::launch::async, [this](MUINT32 __bufSize) -> int{
            MFLL_THREAD_NAME("strategy_alloc_buf");
            mfllTraceName("allocate_ais_wk_buf");

            /* create working buffer */
            m_imgWorkingBuf = IMfllImageBuffer::createInstance();
            if (CC_UNLIKELY( m_imgWorkingBuf.get() == nullptr )) {
                mfllLogE("create AIS working buffer instance failed");
                return -1;
            }

            /* align working buffer size */
            __bufSize = (__bufSize + 1) >> 1;
            m_imgWorkingBuf->setResolution(__bufSize, 2);
            m_imgWorkingBuf->setImageFormat(ImageFormat_Y8);
            mfllLogD3("init AIS working buffer with size %u", __bufSize << 1);

            auto r = m_imgWorkingBuf->initBuffer();
            if (CC_UNLIKELY( r != MfllErr_Ok )) {
                mfllLogE("init AIS working buffer failed");
                return -1;
            }
            return 0; // OK!
        }, _getParam.u4WorkingBufSize );
    }


    return MfllErr_Ok;
}
//-----------------------------------------------------------------------------
enum MfllErr MfllStrategy::queryStrategy(
        const MfllStrategyConfig_t &cfg,
        MfllStrategyConfig_t *out)
{
    mfllTraceCall();

    if (m_mtkAis.get() == NULL) {
        mfllLogE("%s: MTKAis instance doesn't exist", __FUNCTION__);
        return MfllErr_UnexpectedError;
    }
    if (m_nvramChunkMfnr == NULL) {
        mfllLogE("%s: NVRAM chunk Mfnr is NULL", __FUNCTION__);
        return MfllErr_UnexpectedError;
    }
    if (m_nvramChunkMfnrTh == NULL) {
        mfllLogE("%s: NVRAM chunk MfnrTh is NULL", __FUNCTION__);
        return MfllErr_UnexpectedError;
    }

    /* default AISConfig */
    MfllStrategyConfig_t finalCfg = cfg;

    finalCfg.frameCapture   = m_captureFrameNum;
    finalCfg.frameBlend     = m_blendFrameNum;
    finalCfg.isFullSizeMc   = m_isFullSizeMc;

    // check content-award ais
    auto adbEnableGyro = MfllProperty::readProperty(
        mfll::Property_Gyro, MFLL_CONTENT_AWARE_AIS_ENABLE);

    // check customer folder
    bool bCustomEnable = [&cfg](){
        if (CC_UNLIKELY( cfg.sensor_id < 0 )) {
            mfllLogE("sensor id < 0, something wrong, disable content-aware AIS");
            return false;
        }

        bool b = !! (CUST_MFLL_ENABLE_CONTENT_AWARE_AIS & (1 << cfg.sensor_id));
        if (CC_UNLIKELY( !b )) {
            mfllLogI("disable Content-Aware AIS by custom");
        }
        return b;
    }();

    //
    bool bEnableContentAwareAis = adbEnableGyro && (cfg.isAis != 0) && bCustomEnable;

    if (CC_LIKELY( bEnableContentAwareAis )) {
        mfllLogD("enable content-aware AIS");
    }
    else {
        mfllLogD("disabled content-aware AIS");
    }

    //-------------------------------------------------------------------------
    // query frame number
    //-------------------------------------------------------------------------
    if (bEnableContentAwareAis) { // AIS only
        mfllLogE("Not support for content-aware AIS");
    }
    else {
        /* update capture num/blend num by NVRAM only */
        /* NVRAM block */
        const NVRAM_CAMERA_FEATURE_MFLL_STRUCT* pNvramMfnr =
            reinterpret_cast<decltype(pNvramMfnr)>(m_nvramChunkMfnr);

        /* check ISO range */
        finalCfg.frameCapture = m_captureFrameNum;
        finalCfg.frameBlend   = m_blendFrameNum;
    }

    //-------------------------------------------------------------------------
    // query if it's necessary to do MFNR
    //-------------------------------------------------------------------------
    /* checks ISO only */
    {
        const NVRAM_CAMERA_FEATURE_MFLL_STRUCT* pNvramMfnr =
            reinterpret_cast<decltype(pNvramMfnr)>(m_nvramChunkMfnr);

        const NVRAM_CAMERA_FEATURE_MFNR_THRES_STRUCT* pNvramMfnThr =
            reinterpret_cast<decltype(pNvramMfnThr)>(m_nvramChunkMfnrTh);

        if (finalCfg.isAis != 0) { // AIS (MFNR with changing pline mechanism)

            bool isPlineTrigger = false;
            bool isMfnrTrigger = false;
#if 0
            // current (preview) iso threshold for pline change
            if (finalCfg.original_iso < pNvramMfnr->pline_iso_th) {
                // original iso is good enough, no need to change PLine.
                finalCfg.iso = cfg.original_iso;
                finalCfg.exp = cfg.original_exp;
            }
            else{
                isPlineTrigger = true;
            }
#else
            // Not tuning for NVRAM2.1
            isPlineTrigger = true;
#endif
            // change (capture) iso threshold for mfll trigger
            if (finalCfg.iso < pNvramMfnThr->iso_th) {
                // low capture iso, only capture single frame (mfll will be bypass).
                finalCfg.frameCapture = 1;
                finalCfg.frameBlend = 1;
            }
            else {
                isMfnrTrigger = true;
            }

            // continue to execute the flow if PLine changing or MFNR triggered
            if (isPlineTrigger || isMfnrTrigger) {
                finalCfg.enableMfb = 1;
            }
            else {
                finalCfg.enableMfb = 0;
            }

            mfllLogI("%s:[AIS] Set enableMfb to %d due to Pline(%d)/MFNR(%d) will be triggered",
                    __FUNCTION__, finalCfg.enableMfb, isPlineTrigger, isMfnrTrigger);
        }
        else { // MFNR (without changing pline mechanism), on/off by iso threshold
            // no changing Pline, capture iso = preview iso
            if (cfg.iso > pNvramMfnThr->iso_th)
                finalCfg.enableMfb = 1;
            else
                finalCfg.enableMfb = 0;

        }

        // query downscale tuning setting: iso threshold to trigger downscale yuv and downscale ratio
        mfllLogW("TODO: Please fix me <enableDownscale>");
        finalCfg.enableDownscale   = MfllProperty::readProperty(Property_ForceDownscale, pNvramMfnr->memc_dsus_mode);
        finalCfg.downscaleRatio    = 0;
        finalCfg.downscaleDividend = MfllProperty::readProperty(Property_ForceDownscaleDividend, pNvramMfnr->dsdn_ratio);
        finalCfg.downscaleDivisor  = MfllProperty::readProperty(Property_ForceDownscaleDivisor, 16);
        finalCfg.postrefine_nr     = MfllProperty::readProperty(Property_PostNrRefine, pNvramMfnr->post_refine_en);
        finalCfg.postrefine_mfb    = MfllProperty::readProperty(Property_PostMfbRefine, pNvramMfnr->post_me_refine_en);
        finalCfg.postrefine_mfb    = finalCfg.postrefine_nr?finalCfg.postrefine_mfb:0;

        mfllLogI("%s: iso:%u, origin_iso:%u, pline_iso_th: not support, mfll_iso_th:%d, downscale(enabled:%d, ratio:%d, %d/%d), finalCfg(enableMfb:%d, frameCapture:%d), postrefine(nr:%d, mfb:%d)",
                __FUNCTION__, cfg.iso, cfg.original_iso, /*pNvramMfnr->pline_iso_th,*/ pNvramMfnThr->iso_th,
                finalCfg.enableDownscale, finalCfg.downscaleRatio, finalCfg.downscaleDividend, finalCfg.downscaleDivisor, finalCfg.enableMfb, finalCfg.frameCapture,
                finalCfg.postrefine_nr, finalCfg.postrefine_mfb);
    }


    //-------------------------------------------------------------------------
    // always force on MFB
    //-------------------------------------------------------------------------
#if MFLL_MFB_ALWAYS_ON
    finalCfg.enableMfb = 1;
#endif

    //-------------------------------------------------------------------------
    // check properties.
    //-------------------------------------------------------------------------
    {
        int bForceMfb = MfllProperty::isForceMfll();

        if (CC_UNLIKELY( bForceMfb == 0 )) {
            mfllLogD("Force disable MFNR");
            finalCfg.enableMfb = 0;
        }
        else if (CC_UNLIKELY(bForceMfb > 0)) {
            mfllLogD("Force MFNR (bForceMfb:%d)", bForceMfb);
            finalCfg.enableMfb = 1;
        }

        if (finalCfg.enableMfb) {
            int forceExp        = MfllProperty::getExposure();
            int forceIso        = MfllProperty::getIso();
            int forceCaptureNum = MfllProperty::getCaptureNum();
            int forceBlendNum   = MfllProperty::getBlendNum();
            int forceFullSizeMc = MfllProperty::getFullSizeMc();

            if (CC_UNLIKELY(forceExp > 0)) finalCfg.exp = forceExp * 1000;
            if (CC_UNLIKELY(forceIso > 0)) finalCfg.iso = forceIso;
            if (CC_UNLIKELY(forceCaptureNum > 0)) finalCfg.frameCapture = forceCaptureNum;
            if (CC_UNLIKELY(forceBlendNum > 0)) finalCfg.frameBlend = forceBlendNum;
            if (CC_UNLIKELY(forceFullSizeMc >= 0)) finalCfg.isFullSizeMc = forceFullSizeMc;

            /* calculate either auto iso or auto exposure */
            if (CC_LIKELY(forceExp <= 0 && forceIso <= 0)) {
                // general case, do nothing.
            }
            else if (forceExp > 0 && forceIso > 0) {
                // manual set both exposure and ISO, do nothing.
            }
            else if (forceExp > 0) {
                /* calculate auto ISO */
                int total = cfg.exp * cfg.iso;
                finalCfg.iso = total / finalCfg.exp;
            }
            else if (forceIso > 0) {
                int total = cfg.exp * cfg.iso;
                finalCfg.exp = total / finalCfg.iso;
            }
            else;
        }
    }

    *out = finalCfg;
    return MfllErr_Ok;
}

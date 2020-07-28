/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2015. All rights reserved.
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

#include <mtkcam3/feature/mfnr/MfllProperty.h>
#include <mtkcam3/feature/mfnr/MfllLog.h>

#include <camera_custom_nvram.h>

// AOPS
#include <cutils/compiler.h>

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
: m_nvramChunk(NULL)
, m_captureFrameNum(MFLL_CAPTURE_FRAME)
, m_blendFrameNum(MFLL_BLEND_FRAME)
{
    m_mtkAis = std::shared_ptr<MTKAis>(
            MTKAis::createInstance(DRV_AIS_OBJ_SW),
            [](auto *obj)->void {
                if (obj) obj->destroyInstance();
            }
    );

    if (m_mtkAis.get() == NULL) {
        mfllLogE("create MTKAis failed");
        return;
    }

    auto result = m_mtkAis->AisInit(NULL, NULL);
    if (result != S_AIS_OK) {
        mfllLogE("init MTKAis failed with code %#x", result);
        return;
    }
}
//-----------------------------------------------------------------------------
MfllStrategy::~MfllStrategy()
{
}
//-----------------------------------------------------------------------------
enum MfllErr MfllStrategy::init(sp<IMfllNvram> &nvramProvider)
{
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
    const char *pChunk = nvramProvider->getChunk(&chunkSize);
    if (pChunk == NULL) {
        mfllLogE("%s: read NVRAM failed", __FUNCTION__);
        return MfllErr_UnexpectedError;
    }
    m_nvramChunk = pChunk;
    char *pMutableChunk = const_cast<char*>(pChunk);

    /* reading NVRAM */
    NVRAM_CAMERA_FEATURE_MFLL_STRUCT* pNvram = reinterpret_cast<NVRAM_CAMERA_FEATURE_MFLL_STRUCT*>(pMutableChunk);
    m_captureFrameNum = pNvram->capture_frame_number;
    m_blendFrameNum = pNvram->blend_frame_number;
    m_isFullSizeMc =  pNvram->full_size_mc;

    /* prepare data for MTKAis */
    AIS_SET_PROC1_IN_INFO param;
    mfllLogD("original mfll_iso_th=%d", pNvram->mfll_iso_th);

    param.u4TuningExpTH     = pNvram->ais_exp_th;
    param.u4TuningIsoTH     = pNvram->mfll_iso_th;
    param.u4TuningExpTH0    = pNvram->ais_exp_th; // TODO: no EXP_TH0 ??
    param.u4TuningIsoTH0    = pNvram->mfll_iso_th; // TODO: no ISO_TH0??
    param.u4EnMFNRIsoTH     = pNvram->mfll_iso_th;
    /* advanced tuning parameters */
    param.u4TuningAdvanceEn        = pNvram->ais_advanced_tuning_en;
    param.u4TuningAdvanceMaxExp    = pNvram->ais_advanced_max_exposure;
    param.u4TuningAdvanceMaxIso    = pNvram->ais_advanced_max_iso;

    mfllLogD("ais_exp_th = %d, mfll_iso_th = %d",
            pNvram->ais_exp_th, pNvram->mfll_iso_th);
    mfllLogD("ais_adv_en = %d, ais_adv_max_exp = %d, ais_adv_max_iso = %d",
            pNvram->ais_advanced_tuning_en,
            pNvram->ais_advanced_max_exposure,
            pNvram->ais_advanced_max_iso);

    auto result = m_mtkAis->AisFeatureCtrl(
            AIS_FTCTRL_SET_PROC1_INFO,
            (void*)&param,
            NULL);
    if (result != S_AIS_OK) {
        mfllLogE("%s: AisFeatureCtrl failed with code %#x", __FUNCTION__, result);
    }
    return MfllErr_Ok;
}
//-----------------------------------------------------------------------------
enum MfllErr MfllStrategy::queryStrategy(
        const MfllStrategyConfig_t &cfg,
        MfllStrategyConfig_t *out)
{
    if (m_mtkAis.get() == NULL) {
        mfllLogE("%s: MTKAis instance doesn't exist", __FUNCTION__);
        return MfllErr_UnexpectedError;
    }
    if (m_nvramChunk == NULL) {
        mfllLogE("%s: NVRAM chunk is NULL", __FUNCTION__);
        return MfllErr_UnexpectedError;
    }

    /* default AISConfig */
    MfllStrategyConfig_t finalCfg = cfg;

    finalCfg.frameCapture = m_captureFrameNum;
    finalCfg.frameBlend = m_blendFrameNum;
    finalCfg.isFullSizeMc = m_isFullSizeMc;

    //-------------------------------------------------------------------------
    // case: MFNR only
    //-------------------------------------------------------------------------
    /* not enable AIS, checks ISO only */
    if (cfg.isAis == 0) {
        size_t chunkSize;
        NVRAM_CAMERA_FEATURE_MFLL_STRUCT* pNvram =
            reinterpret_cast<NVRAM_CAMERA_FEATURE_MFLL_STRUCT*>(
                    const_cast<char*>(m_nvramChunk)
                    );
        if (cfg.iso > pNvram->mfll_iso_th)
            finalCfg.enableMfb = 1;
        else
            finalCfg.enableMfb = 0;
    }
    //-------------------------------------------------------------------------
    // case: AIS
    //-------------------------------------------------------------------------
    else {
        AIS_PROC1_PARA_OUT paramOut;
        AIS_PROC1_PARA_IN paramIn;
        paramIn.u4CurrentIso = cfg.iso;
        paramIn.u4CurrentExp = cfg.exp;
        auto result = m_mtkAis->AisMain(AIS_PROC1, &paramIn, &paramOut);
        if (result != S_AIS_OK) {
            mfllLogE("%s: MTKAis::AisMain returns failed(%#x)", __FUNCTION__, result);
            return MfllErr_UnexpectedError;
        }

        /* update iso/exp if need */
        if (paramOut.bUpdateExpIso != 0) {
            finalCfg.iso = paramOut.u4Iso;
            finalCfg.exp = paramOut.u4Exp;
        }
        /* check if it's necessary to do MFNR */
        if (paramOut.bDoMFNR != 0) {
            finalCfg.frameBlend = 4; // FIXME: should be queried from MTKAis
            finalCfg.enableMfb = 1;
        }
        else {
            finalCfg.enableMfb = 0;
        }
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

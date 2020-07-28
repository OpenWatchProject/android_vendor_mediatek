/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
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
#define LOG_TAG "AinrCore/Strategy"

#include "AinrStrategy.h"
#include <mtkcam3/feature/ainr/AinrUlog.h>
#include <mtkcam3/feature/ainr/IAinrCore.h>

// MTKCAM
#include <mtkcam/utils/hw/GyroCollector.h>
#include <mtkcam/utils/hw/HwInfoHelper.h> // NSCamHw::HwInfoHelper
#include <mtkcam/drv/IHalSensor.h> // NSCam::SENSOR_SCENARIO_ID_NORMAL_CAPTURE
#include <mtkcam/def/ImageFormat.h>
#include <mtkcam/utils/mapping_mgr/cam_idx_mgr.h>
// AINR Core Lib
#include <mtkcam3/feature/ainr/IAinrNvram.h>


#include <camera_custom_nvram.h>
#include <mtkcam/utils/std/Aee.h>
// AOPS
#include <utils/SystemClock.h> // elapsedRealtime(), uptimeMillis()
#include <cutils/compiler.h>
#include <cutils/properties.h>
// STL
#include <chrono>
#include <fstream>
#include <sstream>

using android::sp;
using namespace ainr;

//-----------------------------------------------------------------------------
std::shared_ptr<IAinrStrategy> IAinrStrategy::createInstance()
{
    return std::make_shared<AinrStrategy>();
}
//-----------------------------------------------------------------------------
// AinrStrategy implementation
//-----------------------------------------------------------------------------
AinrStrategy::AinrStrategy()
: m_nvramChunk(nullptr)
, m_captureFrameNum(AINR_CAPTURE_FRAME)
, m_ainrIsoTh(0)
{
}
//-----------------------------------------------------------------------------
AinrStrategy::~AinrStrategy()
{
}
//-----------------------------------------------------------------------------
enum AinrErr AinrStrategy::init(const std::shared_ptr<IAinrNvram> &nvramProvider)
{
    CAM_ULOGM_APILIFE();

    if (nvramProvider.get() == nullptr) {
        ainrLogE("%s: nvram provider is nullptr", __FUNCTION__);
        return AinrErr_BadArgument;
    }

    /* read NVRAM */
    size_t chunkSize = 0;
    const char *pChunk = nvramProvider->getChunkAinrTh(&chunkSize);
    if (pChunk == nullptr) {
        ainrLogE("%s: read NVRAM failed", __FUNCTION__);
        return AinrErr_UnexpectedError;
    }
    m_nvramChunk = pChunk;
    char *pMutableChunk = const_cast<char*>(pChunk);

    /* reading NVRAM */
    NVRAM_CAMERA_FEATURE_AINR_THRES_STRUCT* pNvram = reinterpret_cast<NVRAM_CAMERA_FEATURE_AINR_THRES_STRUCT*>(pMutableChunk);
    if (pNvram->capture_frame_number < 1) {
        ainrLogI("There is no tuning, frameCount(%d)", pNvram->capture_frame_number);
        m_captureFrameNum = 0;
        //
    } else {
        m_captureFrameNum = pNvram->capture_frame_number;
        m_ainrIsoTh       = pNvram->ainr_iso_th;
    }

    return AinrErr_Ok;
}
//-----------------------------------------------------------------------------
enum AinrErr AinrStrategy::queryStrategy(
        const AinrStrategyConfig_t &cfg,
        AinrStrategyConfig_t *out)
{
    CAM_ULOGM_APILIFE();

    if (CC_UNLIKELY(m_nvramChunk == nullptr) || CC_UNLIKELY(out == nullptr)) {
        ainrLogF("%s: NVRAM chunk is NULL", __FUNCTION__);
        return AinrErr_UnexpectedError;
    }

    AinrStrategyConfig_t finalCfg = cfg;
    finalCfg.ainrIsoTh    = m_ainrIsoTh;

    if(m_captureFrameNum) {
        if(finalCfg.ainrIso > finalCfg.ainrIsoTh) {
            // Query APU is supported or not
            std::shared_ptr<IAinrCore> core = IAinrCore::createInstance();
            if (CC_UNLIKELY(core == nullptr)) {
                ainrLogF("cannot create IAinrCore in strategy stage");
                return AinrErr_UnexpectedError;
            }

            if(core->queryAlgoSupport(finalCfg.size)) {
                finalCfg.enableAinr = true;
            } else {
                out->frameCapture = 0;
                out->enableAinr = false;
                ainrLogI("Ainr Core not support resolution");
                return AinrErr_Ok;
            }
        }
    } else {
        out->frameCapture = 0;
        out->enableAinr = false;
        ainrLogI("There is no tuning no need to execute ainr");
        return AinrErr_Ok;
    }

    ainrLogD_IF(finalCfg.enableAinr == false ,
               "AINR NOT execute, due to ISO THRESHOLD,current iso(%d) < TH(%d)",
               finalCfg.ainrIso, finalCfg.ainrIsoTh);

    /* read NVRAM */
    char *pMutableChunk = const_cast<char*>(m_nvramChunk);

    /* reading NVRAM */
    NVRAM_CAMERA_FEATURE_AINR_THRES_STRUCT* pNvram = reinterpret_cast<NVRAM_CAMERA_FEATURE_AINR_THRES_STRUCT*>(pMutableChunk);

    // iso range for adaptive frame number mechanism
    if(finalCfg.ainrIso > pNvram->iso_level8) {
        finalCfg.frameCapture = pNvram->frame_num8;
    } else if(finalCfg.ainrIso > pNvram->iso_level7) {
        finalCfg.frameCapture = pNvram->frame_num7;
    } else if(finalCfg.ainrIso > pNvram->iso_level6) {
        finalCfg.frameCapture = pNvram->frame_num6;
    } else if(finalCfg.ainrIso > pNvram->iso_level5) {
        finalCfg.frameCapture = pNvram->frame_num5;
    } else if(finalCfg.ainrIso > pNvram->iso_level4) {
        finalCfg.frameCapture = pNvram->frame_num4;
    } else if(finalCfg.ainrIso > pNvram->iso_level3) {
        finalCfg.frameCapture = pNvram->frame_num3;
    } else if(finalCfg.ainrIso > pNvram->iso_level2) {
        finalCfg.frameCapture = pNvram->frame_num2;
    } else  {
        finalCfg.frameCapture = pNvram->frame_num1;
    }

    int forceNum   = ::property_get_int32("vendor.ainr.forcedNum", 0);
    int nvramDebug = ::property_get_int32("vendor.ainr.nvdebug", 0);

    if(forceNum) {
        ainrLogD("Force ainr capture number(%d) for debug", forceNum);
        finalCfg.frameCapture = forceNum;
    }

    if(nvramDebug) {
        ainrLogD("capture_frame_number(%d), ainr_iso_th(%d)", pNvram->capture_frame_number, pNvram->ainr_iso_th);
        ainrLogD("Frame1(%d) isolevel1(%d)", pNvram->frame_num1, pNvram->iso_level1);
        ainrLogD("Frame2(%d) isolevel2(%d)", pNvram->frame_num2, pNvram->iso_level2);
        ainrLogD("Frame3(%d) isolevel3(%d)", pNvram->frame_num3, pNvram->iso_level3);
        ainrLogD("Frame4(%d) isolevel4(%d)", pNvram->frame_num4, pNvram->iso_level4);
        ainrLogD("Frame5(%d) isolevel5(%d)", pNvram->frame_num5, pNvram->iso_level5);
        ainrLogD("Frame6(%d) isolevel6(%d)", pNvram->frame_num6, pNvram->iso_level6);
        ainrLogD("Frame7(%d) isolevel7(%d)", pNvram->frame_num7, pNvram->iso_level7);
        ainrLogD("Frame8(%d) isolevel8(%d)", pNvram->frame_num8, pNvram->iso_level8);
    }

    ainrLogD("Current Iso(%d), ainrIsoTh(%d), frameCapture(%d)", finalCfg.ainrIso, finalCfg.ainrIsoTh, finalCfg.frameCapture);

    *out = finalCfg;
    return AinrErr_Ok;
}

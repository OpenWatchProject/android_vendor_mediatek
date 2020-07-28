/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2019. All rights reserved.
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

// MTKCAM
#include <mtkcam/def/ImageFormat.h>
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/utils/hw/GyroCollector.h>
#include <mtkcam/utils/hw/HwInfoHelper.h>
#include <mtkcam/utils/mapping_mgr/cam_idx_mgr.h>
#include <mtkcam/utils/std/Aee.h>

// AINR Core Lib
#include <mtkcam3/feature/ainr/IAinrCore.h>
#include <mtkcam3/feature/ainr/AinrUlog.h>

// Custom
#include <camera_custom_nvram.h>

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
: m_nvramProvider(nullptr)
, m_nvramChunk(nullptr)
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
    m_nvramProvider = nvramProvider;

    /* read NVRAM */
    size_t chunkSize = 0;
    const char *pChunk = nvramProvider->getChunkAinrTh(&chunkSize);
    if (pChunk == nullptr) {
        ainrLogF("%s: read NVRAM failed", __FUNCTION__);
        return AinrErr_UnexpectedError;
    }
    m_nvramChunk = pChunk;

    return AinrErr_Ok;
}
//-----------------------------------------------------------------------------
enum AinrErr AinrStrategy::queryStrategy(
        const AinrStrategyConfig_t &cfg,
        AinrStrategyConfig_t *out)
{
    CAM_ULOGM_APILIFE();

    if (CC_UNLIKELY(m_nvramChunk == nullptr) || CC_UNLIKELY(out == nullptr)) {
        ainrLogF("%s: NVRAM chunk is nullptr", __FUNCTION__);
        return AinrErr_UnexpectedError;
    }

    AinrStrategyConfig_t finalCfg = cfg;
    int currentIso = finalCfg.ainrIso;

    const FEATURE_NVRAM_AIISP_EV_T *nvramPtr
        = reinterpret_cast<const FEATURE_NVRAM_AIISP_EV_T*>(m_nvramChunk);
    const auto &lowThreshold = nvramPtr->part1_start;
    const auto &highThreshold = nvramPtr->part1_end;
    ainrLogD("lowThreshold(%d), highThreshold(%d), currentIso(%d)",
                lowThreshold.val, highThreshold.val, currentIso);

    // Capture decision
    if (currentIso > lowThreshold.val && currentIso < highThreshold.val) {
        // Query APU is supported or not
        std::shared_ptr<IAinrCore> core = IAinrCore::createInstance();
        if (CC_UNLIKELY(core == nullptr)) {
            ainrLogF("cannot create IAinrCore in strategy stage");
            return AinrErr_UnexpectedError;
        }

        if(core->queryAlgoSupport(finalCfg.size)) {
            finalCfg.enableAinr = true;
            finalCfg.ainrIsoTh = lowThreshold.val;
        } else {
            out->frameCapture = 0;
            out->enableAinr = false;
            ainrLogI("Ainr Core not support resolution");
            return AinrErr_Ok;
        }
    } else {
        out->frameCapture = 0;
        out->enableAinr = false;
        ainrLogI("ainr iso(%d) < Threshold(%d)", currentIso, lowThreshold.val);
        return AinrErr_Ok;
    }

    auto &vEvTypes = finalCfg.vEvTypes;

    #define BUILD_EV_TYPES(IDX)                              \
    {                                                        \
        exposureType type =                                  \
            static_cast<exposureType>(nvramPtr->IDX.val);    \
        switch (type) {                                      \
            case exposureType::SSE:                          \
            case exposureType::SE:                           \
            case exposureType::NE:                           \
            case exposureType::LE:                           \
                vEvTypes.push_back(type);                    \
                ainrLogD("Exposure type(%d)", type);         \
                break;                                       \
            case exposureType::BYPASS:                       \
            default:                                         \
                ainrLogD("Default or bypass mode");          \
                break;                                       \
        }                                                    \
    }
    BUILD_EV_TYPES(sel_seq0);
    BUILD_EV_TYPES(sel_seq1);
    BUILD_EV_TYPES(sel_seq2);
    BUILD_EV_TYPES(sel_seq3);
    BUILD_EV_TYPES(sel_seq4);
    BUILD_EV_TYPES(sel_seq5);
    BUILD_EV_TYPES(sel_seq6);
    BUILD_EV_TYPES(sel_seq7);
    #undef BUILD_EV_TYPES

    // Indicate bss ev type, which SE = 1, NE = 2, LE = 3
    finalCfg.bssEvTypes = static_cast<exposureType>(nvramPtr->sel_bss.val);
    finalCfg.frameCapture = static_cast<int>(vEvTypes.size());
    ainrLogD("Current Iso(%d), ainrIsoTh(%d), frameCapture(%d) bssEvType(%d)"
                , finalCfg.ainrIso, finalCfg.ainrIsoTh
                , finalCfg.frameCapture, nvramPtr->sel_bss.val);

    *out = finalCfg;

    return AinrErr_Ok;
}

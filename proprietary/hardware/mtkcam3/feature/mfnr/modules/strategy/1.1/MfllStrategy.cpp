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
#define LOG_TAG "MfllCore/Strategy"

#include "MfllStrategy.h"

// MFNR public headers
#include <mtkcam3/feature/mfnr/MfllProperty.h>
#include <mtkcam3/feature/mfnr/MfllLog.h>

// MTKCAM
#include <mtkcam/utils/hw/GyroCollector.h>
#include <camera_custom_nvram.h>

// AOPS
#include <utils/SystemClock.h> // elapsedRealtime(), uptimeMillis()
#include <cutils/compiler.h>

// STL
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

    param.u2IsoLvl1         = pNvram->iso_level1;
    param.u2IsoLvl2         = pNvram->iso_level2;
    param.u2IsoLvl3         = pNvram->iso_level3;
    param.u2IsoLvl4         = pNvram->iso_level4;
    param.u2IsoLvl5         = pNvram->iso_level5;
    /* u1FrmNum represents frame number for blending */
    param.u1FrmNum1         = pNvram->frame_num1;
    param.u1FrmNum2         = pNvram->frame_num2;
    param.u1FrmNum3         = pNvram->frame_num3;
    param.u1FrmNum4         = pNvram->frame_num4;
    param.u1FrmNum5         = pNvram->frame_num5;
    param.u1FrmNum6         = pNvram->frame_num6;
    /* u1SrcNum represents frame number for capture */
    param.u1SrcNum1         = pNvram->frame_num1;
    param.u1SrcNum2         = pNvram->frame_num2;
    param.u1SrcNum3         = pNvram->frame_num3;
    param.u1SrcNum4         = pNvram->frame_num4;
    param.u1SrcNum5         = pNvram->frame_num5;
    param.u1SrcNum6         = pNvram->frame_num6;

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
    // query frame number
    //-------------------------------------------------------------------------
    {
        int64_t currentTs = android::uptimeMillis() * 1000000L; // ms-->ns
        constexpr static int64_t const A_SECOND = 1000000000L;

        // retrieve gyro info
        auto gyroInfo = NSCam::Utils::GyroCollector::getData(
                currentTs - A_SECOND, // i
                currentTs
                );


        mfllLogD("gyro info size= %zu", gyroInfo.size());

        // if enable dump RAW, enable
        if (CC_UNLIKELY(MfllProperty::readProperty(Property_DumpGyro) > 0)) {
            static int serial_ = 0;
            char filename[512];
            snprintf(
                    filename,
                    sizeof(filename),
                    "%s%04d_%s.txt",
                    "/sdcard/DCIM/Camera/MFLL",
                    serial_++,
                    "gyro");
            std::ofstream wt(filename, std::ofstream::out);
            for (size_t i = 0; i < gyroInfo.size(); i++) {
                const auto& itr = gyroInfo[i];
                wt  << itr.x << ", "
                    << itr.y << ", "
                    << itr.z << ", "
                    << itr.timestamp
                    << std::endl;
            }
        }

        AIS_PROC1_PARA_IN   paramIn;
        AIS_PROC1_PARA_OUT  paramOut;

        constexpr bool bGyroNeedMemcpy =
            sizeof(AIS_GYRO_INFO) != sizeof(NSCam::Utils::GyroCollector::GyroInfo);

        std::unique_ptr<AIS_GYRO_INFO> gyroDataChunk_ = [bGyroNeedMemcpy, &gyroInfo]{
            return CC_UNLIKELY(bGyroNeedMemcpy)
                ? std::unique_ptr<AIS_GYRO_INFO>(new AIS_GYRO_INFO[gyroInfo.size()])
                : nullptr
                ;
        }();

        // gyro information
        paramIn.u4GyroNum = gyroInfo.size();
        paramIn.u4GyroIntervalMS = static_cast<MUINT32>(
                NSCam::Utils::GyroCollector::INTERVAL);

        // copy gyro info if necessary
        if (CC_UNLIKELY(bGyroNeedMemcpy)) {
            mfllLogW("%s: AIS_GYRO_INFO doesn't equals to GyroInfo, need copy. " \
                     "Please consider using the same structure w/o wasting copy " \
                     "operations which are expensive.",
                     __FUNCTION__);

            for (size_t i = 0; i < gyroInfo.size(); i++) {
                if (CC_UNLIKELY(gyroDataChunk_.get() == nullptr)) {
                    mfllLogE("%s: Gyro data chunk is null", __FUNCTION__);
                    break;
                }

                gyroDataChunk_.get()[i].fX = static_cast<MFLOAT>(gyroInfo[i].x);
                gyroDataChunk_.get()[i].fY = static_cast<MFLOAT>(gyroInfo[i].y);
                gyroDataChunk_.get()[i].fZ = static_cast<MFLOAT>(gyroInfo[i].z);
            }

            paramIn.prGyroInfo = gyroDataChunk_.get();
        }
        else {
            // data structures between middleware and algorithm are the same,
            // we can use memory chunk directly.
            paramIn.prGyroInfo = static_cast<AIS_GYRO_INFO*>(
                    static_cast<void*>(
                    const_cast<NSCam::Utils::GyroCollector::GyroInfo*>(gyroInfo.data())));
        }

        paramIn.u4CurrentIso = cfg.iso;

        // debug message
        mfllLogD("%s: AIS_PROC1_PARA_IN.u4CurrentIso     = %u", __FUNCTION__, paramIn.u4CurrentIso);
        mfllLogD("%s: AIS_PROC1_PARA_IN.u4gyroNum        = %u", __FUNCTION__, paramIn.u4GyroNum);
        mfllLogD("%s: AIS_PROC1_PARA_IN.prgyroInfo       = %p", __FUNCTION__, paramIn.prGyroInfo);
        mfllLogD("%s: AIS_PROC1_PARA_IN.u4gyroIntervalMS = %u", __FUNCTION__, paramIn.u4GyroIntervalMS);

        auto result = m_mtkAis->AisMain(AIS_PROC1, &paramIn, &paramOut);
        if (result != S_AIS_OK) {
            mfllLogE("%s: MTKAis::AisMain with AIS_PROC1 returns fail(%#x)", __FUNCTION__, result);
        }
        else {
            finalCfg.frameCapture = static_cast<int>(paramOut.u1ReqSrcNum);
            finalCfg.frameBlend = static_cast<int>(paramOut.u1ReqFrmNum);
        }
    }

    //-------------------------------------------------------------------------
    // query if it's necessary to do MFNR
    //-------------------------------------------------------------------------
    /* checks ISO only */
    {
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

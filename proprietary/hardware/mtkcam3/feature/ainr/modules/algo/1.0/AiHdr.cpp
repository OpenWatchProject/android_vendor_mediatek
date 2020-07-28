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
#define LOG_TAG "AinrCore/AiHdrAlgo"

#include "AiHdr.h"

#include <mtkcam3/feature/ainr/AinrUlog.h>
#include <mtkcam3/feature/utils/ImageBufferUtils.h>

// ISP profile
#include <tuning_mapping/cam_idx_struct_ext.h>
// For RAWIspCamInfo
#include <isp_tuning/ver1/isp_tuning_cam_info.h>

// AOSP
#include <utils/String8.h>
#include <cutils/properties.h>

// STD
#include <unordered_map>  // std::unordered_map


using namespace android;
using namespace ainr;
using namespace NSCam;

AiHdr::AiHdr() {
    ainrLogD("AiHdr ctor");
    m_coreDrv = decltype(m_coreDrv)(MTKHDRCore::createInstance(DRV_HDRCORE_OBJ_SW),
        [](MTKHDRCore* core) {
            CAM_ULOGM_TAGLIFE("NrCore destroy");
            if (CC_UNLIKELY(core == nullptr)) {
                ainrLogW("HDR core is nullptr");
            } else {
                ainrLogD("HDR core unint");
                core->destroyInstance();
            }
        });
    if (CC_UNLIKELY(m_coreDrv == nullptr)) {
        ainrLogF("m_coreDrv is null");
    }
}

AiHdr::~AiHdr() {
    ainrLogD("AiHdr desctructor");
}

enum AinrErr AiHdr::doNrCore(const AiParam &params, const AiCoreParam &coreParam) {
    CAM_ULOGM_APILIFE();
    AinrErr err = AinrErr_Ok;

    {
        CAM_ULOGM_TAGLIFE("NR core init");
        HDRCORE_INIT_IN_STRUCT rInitIn;
        HDRCORE_DUMP_INFO &debugInfo = rInitIn.dumpInfo;
        debugInfo.uniqueKey = coreParam.uniqueKey;
        debugInfo.requestNum = coreParam.requestNum;
        debugInfo.frameNum = coreParam.frameNum;
        //
        rInitIn.u4CoreNum   = 8;
        rInitIn.u4Width     = params.width;
        rInitIn.u4Height    = params.height;
        rInitIn.u4AlgoIdx   = 0;
        rInitIn.u4FrameNum  = coreParam.captureNum;
        rInitIn.u4TileMode  = coreParam.isTileMode ? 1 : 0;
        rInitIn.u4AlgoIdx   = static_cast<uint32_t>(coreParam.sampleMode);
        m_coreDrv->HDRCoreInit(&rInitIn, nullptr);
    }

    HDRCORE_GET_PROC_INFO_OUT_STRUCT rGetProcInfoOut = {};
    HDRCORE_SET_PROC_INFO_IN_STRUCT  rSetProcInfoIn = {};
    //
    HDRCORE_PROC2_IN_STRUCT rProc2In = {};
    HDRCORE_PROC2_OUT_STRUCT rProc2Out = {};

    size_t unPackSize = 0;
    auto outBuffer = coreParam.outBuffer;
    for (size_t i = 0; i < outBuffer->getPlaneCount(); i++) {
        unPackSize += outBuffer->getBufSizeInBytes(i);
    }
    rProc2In.u4ImgSize = unPackSize;
    rProc2Out.u4OutSize = unPackSize;

    // TODO(Yuan Jung) We will use const ptr when algo is ready
    using ApuNvram = FEATURE_NVRAM_AIISP_ISO_APU_Part1_T;
    using PostNvram = FEATURE_NVRAM_AIISP_ISO_PostSW_T;
    size_t apuTuningSize = 0;
    size_t postTuningSize = 0;
    const ApuNvram* apuTuning = reinterpret_cast<const ApuNvram*>
            (mNvram->getSpecificChunk(
            IAinrNvram::nvram_hint::AIISP_ISO_APU_Part1, &apuTuningSize));
    const PostNvram* postTuning = reinterpret_cast<const PostNvram*>
            (mNvram->getSpecificChunk(
            IAinrNvram::nvram_hint::AIISP_ISO_POSTSW, &postTuningSize));
    HDRCORE_NVRAM_INFO tuningBufferInfo = {};
    tuningBufferInfo.fusion = apuTuning;
    tuningBufferInfo.postSw = postTuning;
    rProc2In.tuningBuffer = &tuningBufferInfo;

    // OB and OB ofst
    for (int i = 0; i < 4; i++) {
       rProc2In.i4ObOffst[i] = params.obOfst[i];
       rProc2In.i4DgnGain[i] = params.dgnGain[i];
    }

    HDRCore_VerInfo rCoreVer;
    m_coreDrv->HDRCoreFeatureCtrl(HDRCORE_FTCTRL_GET_VERSION , nullptr, &rCoreVer);
    m_coreDrv->HDRCoreFeatureCtrl(HDRCORE_FTCTRL_GET_PROC_INFO , nullptr, &rGetProcInfoOut);
    rSetProcInfoIn.u4Num = rGetProcInfoOut.u4Num;

    // create ion working buffer
    std::vector< sp<IImageBuffer> >        vWorkingIonBuf;
    {
        CAM_ULOGM_TAGLIFE("MDLA working buffer allocation");
        for (int i = 0; i < rSetProcInfoIn.u4Num; i++) {
            rSetProcInfoIn.rBuf[i].u4Size = rGetProcInfoOut.rBuf[i].u4Size;
            rSetProcInfoIn.rBuf[i].bUseIon = rGetProcInfoOut.rBuf[i].bUseIon;
            ainrLogD("MDLA Working buffer[%d] size(%d)", i, rSetProcInfoIn.rBuf[i].u4Size);

            // Error check for working buffer size
            if (rSetProcInfoIn.rBuf[i].u4Size < 0) {
                ainrLogF("MDLA Working buffer[%d] size(%d) negative!!!", i, rSetProcInfoIn.rBuf[i].u4Size);
                return AinrErr_UnexpectedError;
            }

            sp<IImageBuffer> working = nullptr;
            auto ret = ImageBufferUtils::getInstance().allocBuffer(
            working, rSetProcInfoIn.rBuf[i].u4Size, 1, eImgFmt_STA_BYTE);
            if (CC_UNLIKELY(ret == MFALSE)) {
                // Release previous mdla input buffer
                if (!vWorkingIonBuf.empty()) {
                    for (auto && buf : vWorkingIonBuf) {
                        ImageBufferUtils::getInstance().deallocBuffer(buf.get());
                        buf = nullptr;
                    }
                    vWorkingIonBuf.clear();
                }
                ainrLogF("allocate mdla input error!!!");
                return AinrErr_UnexpectedError;
            }

            rSetProcInfoIn.rBuf[i].i4BuffFD = working->getFD(0);
            rSetProcInfoIn.rBuf[i].prBuff   = reinterpret_cast<void*>(working->getBufVA(0));
            vWorkingIonBuf.push_back(working);
        }
    }
    //
    m_coreDrv->HDRCoreFeatureCtrl(HDRCORE_FTCTRL_SET_PROC_INFO, &rSetProcInfoIn, nullptr);

    // ev container size should be capture number
    if (coreParam.expInfos.size() != coreParam.captureNum) {
        ainrLogF("expInfos(%d) size not correct", coreParam.expInfos.size());
        return AinrErr_UnexpectedError;
    }

    // sortedPairs container size should be capture number
    if (coreParam.sortedPairs.size() != coreParam.captureNum) {
        ainrLogF("sortedPairs(%d) size not correct",
                coreParam.sortedPairs.size());
        return AinrErr_UnexpectedError;
    }

    // N frame mv_unpackRaws
    for (int i = 0; i < coreParam.captureNum; i++) {
        // std::pair<index, evLevel>
        const std::pair<int, int> &idexEvPair = coreParam.sortedPairs[i];
        int index = idexEvPair.first;

        // Update ev information
        auto &evInfos = coreParam.expInfos;
        auto [shutterTime, sensorGain, ispGain]
                    = evInfos[index];
        auto &aeParam = rProc2In.ImgAeInfo[i];
        aeParam.s8Exposuretime = shutterTime;
        aeParam.u4SensorGain = sensorGain;
        aeParam.u4DGNGain = ispGain;

        // fill up buffers va
        auto inUpkBuffer = coreParam.inBuffers[index];
        rProc2In.u2Img[i] = reinterpret_cast<MUINT16*>(inUpkBuffer->getBufVA(0));
    }

    rProc2Out.u2OutImg = reinterpret_cast<MUINT16*>(outBuffer->getBufVA(0));
    ainrLogD("Execute HDR core in tile mode+");
    m_coreDrv->HDRCoreMain(HDRCORE_PROC2, &rProc2In, &rProc2Out);
    ainrLogD("Execute HDR core in tile mode-");

    {
        CAM_ULOGM_TAGLIFE("NrCore mdla buffer release");
        // Release MDLA working buffer
        for (auto && buf : vWorkingIonBuf) {
            ImageBufferUtils::getInstance().deallocBuffer(buf.get());
            buf = nullptr;
        }
        vWorkingIonBuf.clear();
    }

    return err;
}

enum AinrErr AiHdr::calToneMatrix(const AinrAeInfo &src,
        const AinrAeInfo &target, std::vector<int> *matrix) {
    if (CC_UNLIKELY(matrix == nullptr)) {
        ainrLogF("Tone matrix is nullptr");
        return AinrErr_UnexpectedError;
    }

    auto fillAeInfo = [](const AinrAeInfo &aeInfo,
            HDRCORE_IMG_AE_INFO* coreAeInfo) -> void {
        coreAeInfo->s8Exposuretime = aeInfo.exposureTime;
        coreAeInfo->u4SensorGain = aeInfo.sensorGain;
        coreAeInfo->u4DGNGain = aeInfo.ispGain;
    };

    HDRCORE_TONE_ALIGNMENT_IN_STRUCT input = {};
    fillAeInfo(src, &input.CurrentImgAeInfo);
    fillAeInfo(target, &input.TargetImgAeInfo);

    HDRCORE_TONE_ALIGNMENT_OUT_STRUCT output = {};
    output.CCM = matrix;
    m_coreDrv->HDRCoreFeatureCtrl(HDRCORE_FTCTRL_TONE_ALIGNMENT,
            &input, &output);
    return AinrErr_Ok;
}

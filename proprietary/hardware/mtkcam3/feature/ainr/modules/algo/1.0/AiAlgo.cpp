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
#define LOG_TAG "AinrCore/AiAlgo"

#include "AiAlgo.h"

#include <mtkcam/aaa/IHalISP.h>
#include <mtkcam/drv/iopipe/PostProc/INormalStream.h>
#include <mtkcam/utils/hw/IFDContainer.h>
#include <mtkcam/utils/TuningUtils/FileDumpNamingRule.h>
#include <mtkcam3/feature/utils/ImageBufferUtils.h>
#include <mtkcam3/feature/ainr/AinrUlog.h>

// STD
#include <unordered_map>  // std::unordered_map
#include <memory>

// ISP profile
#include <tuning_mapping/cam_idx_struct_ext.h>
// For RAWIspCamInfo
#include <isp_tuning/ver1/isp_tuning_cam_info.h>

// NVRAM structure
#include <camera_custom_nvram.h>

// AOSP
#include <utils/String8.h>
#include <cutils/properties.h>

#define AINR_FD_TOLERENCE        (600000000)

using NSCam::TuningUtils::FILE_DUMP_NAMING_HINT;

using namespace android;
using namespace ainr;
using namespace NSCam;
using namespace NSCam::NSIoPipe;
using namespace NSCam::NSIoPipe::NSPostProc;

AiAlgo::AiAlgo()
    : mNvram(nullptr)
    , mHlrRatio(100) {
    ainrLogD("AiAlgo ctor");
    mDumpWorking = ::property_get_int32("vendor.debug.camera.ainr.dump", 0);
}

AiAlgo::~AiAlgo() {
    ainrLogD("AiAlgo desctructor");
}

void AiAlgo::init(std::shared_ptr<IAinrNvram> nvram) {
    mNvram = nvram;
    return;
}

enum AinrErr AiAlgo::doHG(const AiHgParam &params) {
    CAM_ULOGM_FUNCLIFE_ALWAYS();

    std::unique_ptr< MTKHG, std::function<void(MTKHG*)> >
                                pHGDrv = nullptr;
    pHGDrv = decltype(pHGDrv)(
        MTKHG::createInstance(DRV_HG_OBJ_SW),
        [](MTKHG* p) {
            if (CC_UNLIKELY(p == nullptr)) {
                ainrLogW("MTKHG is nullptr");
                return;
            } else {
                p->destroyInstance();
            }
        });
    if (CC_UNLIKELY(pHGDrv == nullptr))
    {
        ainrLogF(" Error: pHGDrv createInstance() fail");
        return AinrErr_UnexpectedError;
    }

    pHGDrv->HGInit(nullptr, nullptr);

    HG_VerInfo rHgVer;
    pHGDrv->HGFeatureCtrl(HG_FTCTRL_GET_VERSION , nullptr, &rHgVer);

    HG_GET_PROC_INFO_IN_STRUCT  rGetProcInfIn;  // only record "i4MaxFP"
    HG_GET_PROC_INFO_OUT_STRUCT rGetProcInfOut;  // only record "i4ReqWMSize"
    HG_SET_PROC_INFO_IN_STRUCT  rSetProcInfIn;
    HG_PROC1_IN_STRUCT          rProc1In;
    HG_PROC1_OUT_STRUCT         rProc1Out;

    // TODO: Need to remove algo magic num 40
    auto baseFeo = params.baseFeBuf;
    auto refFeo  = params.refFeBuf;
    MSize feoSize = baseFeo->getImgSize();
    rGetProcInfIn.i4MaxFP = feoSize.w * feoSize.h /40;
    pHGDrv->HGFeatureCtrl(HG_FTCTRL_GET_PROC_INFO , &rGetProcInfIn, &rGetProcInfOut);

    // Working buffer allocation
    auto wbSize = rGetProcInfOut.i4ReqWMSize;
    auto workingBuffer = std::unique_ptr<char[]>(new char[wbSize]{0});
    rSetProcInfIn.pvWM = reinterpret_cast<void*>(workingBuffer.get());
    rSetProcInfIn.i4WMSize = rGetProcInfOut.i4ReqWMSize;
    pHGDrv->HGFeatureCtrl(HG_FTCTRL_SET_PROC_INFO , &rSetProcInfIn, NULL);

    // Set up size
    auto imgo = params.imgoBuf;
    MSize imgoSize = imgo->getImgSize();
    const MRect& cropRect = params.cropRect;
    rProc1In.i4SrcWidth = imgoSize.w;
    rProc1In.i4SrcHeight = imgoSize.h;
    rProc1In.i4RzWidth = cropRect.s.w;
    rProc1In.i4RzHeight = cropRect.s.h;
    rProc1In.i4WCP = cropRect.p.x;
    rProc1In.i4HCP = cropRect.p.y;
    rProc1In.i4Idx = 0;
    rProc1In.u4Rand = 0;
    ainrLogD("Resize size(%d, %d), p(%d, %d)",
            rProc1In.i4RzWidth, rProc1In.i4RzHeight,
            cropRect.p.x, cropRect.p.y);

    auto gridX = params.gridX;
    auto gridY = params.gridY;
    const MSize &gridSize = params.gridMapSize;
    rProc1In.MapWidth = gridSize.w;
    rProc1In.MapHeight = gridSize.h;

    size_t mapAllocSize = 0;
    for (size_t i = 0; i < gridX->getPlaneCount(); i++) {
        mapAllocSize += gridX->getBufSizeInBytes(i);
    }
    rProc1Out.MapAllocSize = static_cast<MINT32>(mapAllocSize);
    ainrLogD("HG WbSize(%d), MapSize(%d)", rGetProcInfOut.i4ReqWMSize, rProc1Out.MapAllocSize);

    auto fmoBase = params.baseFmBuf;
    auto fmoRef  = params.refFmBuf;
    rProc1In.pi2BaseFe = reinterpret_cast<MINT16*>(baseFeo->getBufVA(0));
    rProc1In.pi2NewFe  = reinterpret_cast<MINT16*>(refFeo->getBufVA(0));
    rProc1In.pi2BaseFm = reinterpret_cast<MINT16*>(fmoBase->getBufVA(0));
    rProc1In.pi2NewFm  = reinterpret_cast<MINT16*>(fmoRef->getBufVA(0));

    // Set up homography in data. FEOs and FMOs
    // Allocate genGrid buffer
    rProc1Out.pi4MapX = reinterpret_cast<MINT32*>(gridX->getBufVA(0));
    rProc1Out.pi4MapY = reinterpret_cast<MINT32*>(gridY->getBufVA(0));

    // Start homography
    pHGDrv->HGMain(HG_PROC1, &rProc1In, &rProc1Out);

    gridX->syncCache(eCACHECTRL_INVALID);
    gridY->syncCache(eCACHECTRL_INVALID);

    return AinrErr_Ok;
}

enum AinrErr AiAlgo::refBufUpk(const AiParam &params, const AiRefPackage &bufPackage) {
    // Utils allocate and initial
    CAM_ULOGM_FUNCLIFE_ALWAYS();

    std::unique_ptr< MTKNRUtils, std::function<void(MTKNRUtils*)> >
                                pNRUtilsDrv = nullptr;
    pNRUtilsDrv = decltype(pNRUtilsDrv)(
        MTKNRUtils::createInstance(DRV_NRUTILS_OBJ_SW),
        [](MTKNRUtils* p) {
            if (CC_UNLIKELY(p == nullptr)) {
                ainrLogW("UV unpack is nullptr");
                return;
            } else {
                p->destroyInstance();
            }
        });
    if (CC_UNLIKELY(pNRUtilsDrv == nullptr))
    {
        ainrLogE(" Error: MTKNRUtils createInstance() fail");
        return AinrErr_UnexpectedError;
    }

    NRUTILS_VerInfo rUtilsVer;
    pNRUtilsDrv->NRUtilsFeatureCtrl(NRUTILS_FTCTRL_GET_VERSION , nullptr, &rUtilsVer);

    NRUTILS_CONFIG_UNPACK_BAYER_UV_OUT_STRUCT rUnpackUVOut;
    NRUTILS_UNPACK_BAYER_UV_IN_STRUCT rProcIn;
    NRUTILS_UNPACK_BAYER_UV_OUT_STRUCT rProcOut;
    //
    NRUTILS_CONFIG_UNPACK_BAYER_UV_IN_STRUCT rUnpackUVIn;

    rUnpackUVIn.u4CoreNum = 8;
    rUnpackUVIn.i4Width   = params.width;
    rUnpackUVIn.i4Height  = params.height;
    rUnpackUVIn.i4Stride  = params.stride;
    rUnpackUVIn.i4OutBits = params.outBits;
    rUnpackUVIn.i4Mode = static_cast<int32_t>(AinrUpkMode::UPK_VPU);
    rUnpackUVIn.i4IsAihdr = (params.algoType == AIHDR) ? 1 : 0;

    // OB and OB ofst
    for (int i = 0; i < 4; i++) {
        rUnpackUVIn.i4ObOffst[i] = params.obOfst[i];
        rUnpackUVIn.i4DgnGain[i] = params.dgnGain[i];
    }

    // config
    pNRUtilsDrv->NRUtilsFeatureCtrl(NRUTILS_FTCTRL_CONFIG_UNPACK_BAYER_UV, &rUnpackUVIn, &rUnpackUVOut);

    // Buffers
    sp<IImageBuffer> refWorkingBuf = nullptr;

    // prepare working buffer
    if (rUnpackUVOut.i4WMSize) {
        NRUTILS_BUFFER_STRUCT &rWM = rProcIn.rWM;
        rWM.Size = rUnpackUVOut.i4WMSize;
        rWM.BufferNum = 1;
        // create working buffer
        ImageBufferUtils::getInstance().allocBuffer(
            refWorkingBuf, rWM.Size, 1, eImgFmt_STA_BYTE);
        if (CC_UNLIKELY( refWorkingBuf.get() == nullptr )) {
            ainrLogE("baseWorkingBuf is null");
            return AinrErr_UnexpectedError;
        }
        rWM.pBuff[0]  = reinterpret_cast<void*>(refWorkingBuf->getBufVA(0));
        rWM.BuffFD[0] = refWorkingBuf->getFD(0);
    } else {
        ainrLogD("No need to allocate working buffer");
    }

    // Input1 for RG buffer
    const auto rgBuf = bufPackage.rgBuf;
    size_t rgBufSize = 0;
    for (size_t i = 0; i < rgBuf->getPlaneCount(); i++) {
        rgBufSize += rgBuf->getBufSizeInBytes(i);
    }
    rProcIn.rInput1.Size = rgBufSize;
    rProcIn.rInput1.BufferNum = 1;
    rProcIn.rInput1.pBuff[0]  = reinterpret_cast<void*>(rgBuf->getBufVA(0));
    rProcIn.rInput1.BuffFD[0] = rgBuf->getFD(0);

    // Input2 for GB buffer
    const auto gbBuf = bufPackage.gbBuf;
    size_t gbBufSize = 0;
    for (size_t i = 0; i < gbBuf->getPlaneCount(); i++) {
        gbBufSize += gbBuf->getBufSizeInBytes(i);
    }
    rProcIn.rInput2.Size = gbBufSize;
    rProcIn.rInput2.BufferNum = 1;
    rProcIn.rInput2.pBuff[0] = reinterpret_cast<void*>(gbBuf->getBufVA(0));
    rProcIn.rInput2.BuffFD[0] = gbBuf->getFD(0);

    // create output buffer
    const auto outBuf = bufPackage.outBuf;
    size_t outBufSize = 0;
    for (size_t i = 0; i < outBuf->getPlaneCount(); i++) {
        outBufSize += outBuf->getBufSizeInBytes(i);
    }
    rProcOut.rOutput.Size = outBufSize;
    rProcOut.rOutput.BufferNum = 1;
    rProcOut.rOutput.pBuff[0]  = reinterpret_cast<void*>(outBuf->getBufVA(0));
    rProcOut.rOutput.BuffFD[0] = outBuf->getFD(0);
    ainrLogD("RG bufferSize(%zu), GB bufferSize(%zu)", rgBufSize, gbBufSize);

    // process
    ainrLogD("UV unpack+");
    pNRUtilsDrv->NRUtilsFeatureCtrl(NRUTILS_FTCTRL_PROC_UNPACK_BAYER_UV, &rProcIn, &rProcOut);
    ainrLogD("UV unpack-");

    //
    ImageBufferUtils::getInstance().deallocBuffer(refWorkingBuf.get());

    return AinrErr_Ok;
}

enum AinrErr AiAlgo::baseBufUpk(const AiParam &params, const AiBasePackage &bufPackage) {
    CAM_ULOGM_FUNCLIFE_ALWAYS();

    std::unique_ptr< MTKNRUtils, std::function<void(MTKNRUtils*)> >
                                pNRUtilsDrv = nullptr;
    pNRUtilsDrv = decltype(pNRUtilsDrv)(
        MTKNRUtils::createInstance(DRV_NRUTILS_OBJ_SW),
        [](MTKNRUtils* p) {
            if (CC_UNLIKELY(p == nullptr)) {
                ainrLogW("Base unpack is nullptr");
                return;
            } else {
                ainrLogD("Destroy base unpack");
                p->destroyInstance();
            }
        });
    if (CC_UNLIKELY(pNRUtilsDrv == nullptr)) {
        ainrLogF(" Error: MTKNRUtils createInstance() fail");
        return AinrErr_UnexpectedError;
    }

    NRUTILS_VerInfo rUtilsVer;
    pNRUtilsDrv->NRUtilsFeatureCtrl(NRUTILS_FTCTRL_GET_VERSION , nullptr, &rUtilsVer);

    //
    NRUTILS_CONFIG_UNPACK_BAYER_IN_STRUCT rUnpackBaseIn;
    rUnpackBaseIn.u4CoreNum = 8;
    rUnpackBaseIn.i4Width   = params.width;
    rUnpackBaseIn.i4Height  = params.height;
    rUnpackBaseIn.i4Stride  = params.stride;
    rUnpackBaseIn.i4OutBits = params.outBits;
    rUnpackBaseIn.i4Mode = static_cast<int32_t>(AinrUpkMode::UPK_VPU);
    rUnpackBaseIn.i4IsAihdr = (params.algoType == AIHDR) ? 1 : 0;

    // OB and OB ofst
    for (int i = 0; i < 4; i++) {
        rUnpackBaseIn.i4ObOffst[i] = params.obOfst[i];
        rUnpackBaseIn.i4DgnGain[i] = params.dgnGain[i];
    }

    // config
    NRUTILS_CONFIG_UNPACK_BAYER_OUT_STRUCT rUnpackBaseOut;
    pNRUtilsDrv->NRUtilsFeatureCtrl(NRUTILS_FTCTRL_CONFIG_UNPACK_BAYER, &rUnpackBaseIn, &rUnpackBaseOut);

    NRUTILS_UNPACK_BAYER_IN_STRUCT rProcIn;
    // Only CPU version need working buffer
    sp<IImageBuffer> baseWorkingBuf = nullptr;
    if (rUnpackBaseOut.i4WMSize) {
        // prepare working buffer
        NRUTILS_BUFFER_STRUCT &rWM = rProcIn.rWM;
        rWM.Size = rUnpackBaseOut.i4WMSize;
        rWM.BufferNum = 1;
        // create working buffer
        ImageBufferUtils::getInstance().allocBuffer(
            baseWorkingBuf, rWM.Size, 1, eImgFmt_STA_BYTE);
        if (CC_UNLIKELY(baseWorkingBuf.get() == nullptr)) {
            ainrLogF("baseWorkingBuf is null");;
            return AinrErr_UnexpectedError;
        }
        rWM.pBuff[0]  = reinterpret_cast<void*>(baseWorkingBuf->getBufVA(0));
        rWM.BuffFD[0] = baseWorkingBuf->getFD(0);
    } else {
        ainrLogD("No need to allocate working buffer");
    }

    // prepare Input
    const auto imgoBase = bufPackage.inBuf;
    NRUTILS_BUFFER_STRUCT &rInput = rProcIn.rInput;
    rInput.Size = rUnpackBaseIn.i4Stride*rUnpackBaseIn.i4Height;
    rInput.BufferNum = 1;
    rInput.pBuff[0]  = reinterpret_cast<void*>(imgoBase->getBufVA(0));
    rInput.BuffFD[0] = imgoBase->getFD(0);

    // prepare output
    const auto outBuf = bufPackage.outBuf;
    NRUTILS_UNPACK_BAYER_OUT_STRUCT rProcOut;
    rProcOut.rOutput.Size = rUnpackBaseIn.i4Width*rUnpackBaseIn.i4Height*sizeof(short);
    rProcOut.rOutput.BufferNum = 1;
    rProcOut.rOutput.pBuff[0]  = reinterpret_cast<void*>(outBuf->getBufVA(0));
    rProcOut.rOutput.BuffFD[0] = outBuf->getFD(0);

    // process
    ainrLogD("Base unpack+");
    pNRUtilsDrv->NRUtilsFeatureCtrl(NRUTILS_FTCTRL_PROC_UNPACK_BAYER, &rProcIn, &rProcOut);
    ainrLogD("Base unpack-");

    // Finish base frame handling. We release buffer
    // But only CPU version has working buffer
    ImageBufferUtils::getInstance().deallocBuffer(baseWorkingBuf.get());

    return AinrErr_Ok;
}

enum AinrErr AiAlgo::doNrCore(const AiParam &params, const AiCoreParam &coreParam) {
    CAM_ULOGM_APILIFE();
    AinrErr err = AinrErr_Ok;

    std::unique_ptr<MTKNRCore, std::function<void(MTKNRCore*)>> pNRCoreDrv = nullptr;
    pNRCoreDrv = decltype(pNRCoreDrv)( MTKNRCore::createInstance(DRV_NRCORE_OBJ_SW),
        [](MTKNRCore* nrCore) {
            //ainrTraceName("NrCore destroy");
            if (CC_UNLIKELY(nrCore == nullptr)) {
                ainrLogW("NRCore is nullptr");
            } else {
                ainrLogD("NR core unint");
                nrCore->destroyInstance();
            }
        }
    );
    if (CC_UNLIKELY(pNRCoreDrv == nullptr))
    {
        ainrLogF("pNRCoreDrv is null");
        return AinrErr_UnexpectedError;
    }

    using ApuNvram = FEATURE_NVRAM_AIISP_ISO_APU_Part1_T;
    using PostNvram = FEATURE_NVRAM_AIISP_ISO_PostSW_T;
    size_t apuTuningSize = 0;
    size_t postTuningSize = 0;
    const ApuNvram* apuTuning = reinterpret_cast<const ApuNvram*>
            (mNvram->getSpecificChunk(
            IAinrNvram::nvram_hint::AIISP_ISO_APU_Part1, &apuTuningSize));
    if (CC_UNLIKELY(apuTuning == nullptr)) {
        ainrLogF("apuTuning is null");
        return AinrErr_UnexpectedError;
    }

    const PostNvram* postTuning = reinterpret_cast<const PostNvram*>
            (mNvram->getSpecificChunk(
            IAinrNvram::nvram_hint::AIISP_ISO_POSTSW, &postTuningSize));
    if (CC_UNLIKELY(postTuning == nullptr)) {
        ainrLogF("postTuning is null");
        return AinrErr_UnexpectedError;
    }

    auto indexModelInfo = apuTuning->model;
    ainrLogD("Model index(%d)", indexModelInfo.val);

    {
        CAM_ULOGM_TAGLIFE("NR core init");
        NRCORE_INIT_IN_STRUCT rInitIn;
        NRCORE_DUMP_INFO &debugInfo = rInitIn.dumpInfo;
        debugInfo.uniqueKey = coreParam.uniqueKey;
        debugInfo.requestNum = coreParam.requestNum;
        debugInfo.frameNum = coreParam.frameNum;
        //
        rInitIn.u4CoreNum   = 8;
        rInitIn.u4Width     = params.width;
        rInitIn.u4Height    = params.height;
        rInitIn.u4AlgoIdx   = indexModelInfo.val;
        rInitIn.u4FrameNum  = coreParam.captureNum;
        rInitIn.u4TileMode  = coreParam.isTileMode ? 1 : 0;
        pNRCoreDrv->NRCoreInit(&rInitIn, nullptr);
    }

    NRCORE_GET_PROC_INFO_OUT_STRUCT rGetProcInfoOut;
    NRCORE_SET_PROC_INFO_IN_STRUCT  rSetProcInfoIn;
    //
    NRCORE_PROC2_IN_STRUCT rProc2In;
    NRCORE_PROC2_OUT_STRUCT rProc2Out;

    size_t unPackSize = 0;
    const auto outBuffer = coreParam.outBuffer;
    for (size_t i = 0; i < outBuffer->getPlaneCount(); i++) {
        unPackSize += outBuffer->getBufSizeInBytes(i);
    }
    rProc2In.u4ImgSize = unPackSize;

    NRCORE_NVRAM_INFO tuningBufferInfo;
    tuningBufferInfo.fusion = apuTuning;
    tuningBufferInfo.postSw = postTuning;
    rProc2In.tuningBuffer = &tuningBufferInfo;

    // OB and OB ofst
    for (int i = 0; i < 4; i++) {
       rProc2In.i4ObOffst[i] = params.obOfst[i];
       rProc2In.i4DgnGain[i] = params.dgnGain[i];
    }
    rProc2Out.u4OutSize = unPackSize;

    NRCore_VerInfo rCoreVer;
    pNRCoreDrv->NRCoreFeatureCtrl(NRCORE_FTCTRL_GET_VERSION , nullptr, &rCoreVer);
    pNRCoreDrv->NRCoreFeatureCtrl(NRCORE_FTCTRL_GET_PROC_INFO , nullptr, &rGetProcInfoOut);
    rSetProcInfoIn.u4Num = rGetProcInfoOut.u4Num;

    // create ion working buffer
    std::vector< sp<IImageBuffer> >        vWorkingIonBuf;
    {
        CAM_ULOGM_TAGLIFE("MDLA working buffer allocation");
        for (int i = 0; i < rSetProcInfoIn.u4Num; i++)
        {
            rSetProcInfoIn.rBuf[i].u4Size = rGetProcInfoOut.rBuf[i].u4Size;
            rSetProcInfoIn.rBuf[i].bUseIon = rGetProcInfoOut.rBuf[i].bUseIon;
            ainrLogD("MDLA Working buffer[%d] size(%d)", i, rSetProcInfoIn.rBuf[i].u4Size);

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
    pNRCoreDrv->NRCoreFeatureCtrl(NRCORE_FTCTRL_SET_PROC_INFO, &rSetProcInfoIn, nullptr);

    // N frame mv_unpackRaws
    for (int i = 0; i < coreParam.captureNum; i++)
    {
        auto inUpkBuffer = coreParam.inBuffers[i];
        rProc2In.u2Img[i] = reinterpret_cast<MUINT16*>(inUpkBuffer->getBufVA(0));
    }

    rProc2Out.u2OutImg = reinterpret_cast<MUINT16*>(outBuffer->getBufVA(0));
    ainrLogD("Execute NR core in tile mode+");
    pNRCoreDrv->NRCoreMain(NRCORE_PROC2, &rProc2In, &rProc2Out);
    ainrLogD("Execute NR core in tile mode-");

    {
        CAM_ULOGM_TAGLIFE("NrCore mdla buffer release");
        // Release MDLA working buffer
        for(auto && buf : vWorkingIonBuf) {
            ImageBufferUtils::getInstance().deallocBuffer(buf.get());
            buf = nullptr;
        }
        vWorkingIonBuf.clear();
    }

    return err;
}

enum AinrErr AiAlgo::doDRC(const IAiAlgo::drcParam &params) {
    AinrErr err = AinrErr_Ok;
    int32_t openId = params.openId;
    String8 strLog;

    if (params.pRawBuf == nullptr || params.pAppMeta == nullptr
        || params.pHalMeta == nullptr || params.pAppMetaDynamic == nullptr) {
        ainrLogF("doDRC fail because one of parameter is nullptr");
        return AinrErr_UnexpectedError;
    }

    auto rawBuf = params.pRawBuf;
    auto appMeta = params.pAppMeta;
    auto halMeta = params.pHalMeta;
    auto dynamicMeta = params.pAppMetaDynamic;

    // Dump Input buffer before apply DRC
    if (mDumpWorking & AinrDumpWorking) {
        char fileName[256] = "";
        FILE_DUMP_NAMING_HINT dumpNamingHint{};

        // Extract hal metadata and fill up file name;
        extract(&dumpNamingHint, halMeta);

        // Extract buffer information and fill up file name;
        extract(&dumpNamingHint, rawBuf);

        // Get bayer ordered
        extract_by_SensorOpenId(&dumpNamingHint, openId);

        // Generate file name
        genFileName_RAW(fileName, sizeof(fileName), &dumpNamingHint,
                TuningUtils::RAW_PORT_IMGO, "BeforeDRC");
        rawBuf->saveToFile(fileName);
    }

    MINT64 p1timestamp = 0;
    if (!IMetadata::getEntry<MINT64>(halMeta, MTK_P1NODE_FRAME_START_TIMESTAMP, p1timestamp)) {
        ainrLogF("cannot get timestamp MTK_P1NODE_FRAME_START_TIMESTAMP");
        return AinrErr_UnexpectedError;
    }

    // Face information
    MTKFDContainerInfo* pFdDataPtr = nullptr;
    MTKFDContainerInfo fdData;

    sp<IFDContainer> fdReader = IFDContainer::createInstance(LOG_TAG,  IFDContainer::eFDContainer_Opt_Read);
    if (fdReader.get()) {
        std::vector<MTKFDContainerInfo*> vFdData = fdReader->queryLock(p1timestamp - AINR_FD_TOLERENCE, p1timestamp);
        if (vFdData.empty()) {
            ainrLogD("There is no face at ts:%" PRId64 "", p1timestamp);
        } else {
            // The latest fd info
            pFdDataPtr = vFdData.back();
            if (pFdDataPtr) {
                pFdDataPtr->clone(fdData);
            } else {
                ainrLogW("pFdDataPtr is nullptr");
            }

            MtkCameraFaceMetadata *pDetectedFaces = &fdData.facedata;
            ainrLogD("number_of_faces (%d)", pDetectedFaces->number_of_faces);
        }

        fdReader->queryUnlock(vFdData);
    } else {
        ainrLogW("FDContainer createInstance fail");
    }

    // Get AE exposure information
    int64_t shutterTime = 0;  // nanoseconds (ns)
    if (!IMetadata::getEntry<MINT64>(dynamicMeta, MTK_SENSOR_EXPOSURE_TIME, shutterTime)) {
        ainrLogW("Get MTK_SENSOR_EXPOSURE_TIME fail");
    }

    int32_t sensorGain = 0;
    if (!IMetadata::getEntry<MINT32>(dynamicMeta, MTK_3A_FEATURE_AE_SENSOR_GAIN_VALUE, sensorGain)) {
        ainrLogF("cannot get MTK_3A_FEATURE_AE_SENSOR_GAIN_VALUE");
        return AinrErr_UnexpectedError;
    }

    int32_t ispGain = 0;
    if (!IMetadata::getEntry<MINT32>(dynamicMeta, MTK_3A_FEATURE_AE_ISP_GAIN_VALUE, ispGain)) {
        ainrLogF("cannot get MTK_3A_FEATURE_AE_ISP_GAIN_VALUE");
        return AinrErr_UnexpectedError;
    }

    int32_t lightValue = 0;
    if (!IMetadata::getEntry<MINT32>(halMeta, MTK_REAL_LV, lightValue)) {
        ainrLogF("cannot get MTK_REAL_LV");
        return AinrErr_UnexpectedError;
    }
    ainrLogD("Shutter(%" PRId64 " ns), sensorGain(%d), ispGain(%d), luxValue(%d)"
        , shutterTime, sensorGain, ispGain, lightValue);

    // Get WB gain table
    constexpr int wbSize = 3;
    MRational wbTable[wbSize] = {};  // Gain table for RGB
    for (int i = 0; i < wbSize; i++) {
        if (!IMetadata::getEntry<MRational>(dynamicMeta, MTK_SENSOR_NEUTRAL_COLOR_POINT, wbTable[i], i)) {
            ainrLogF("cannot get MTK_SENSOR_NEUTRAL_COLOR_POINT");
            return AinrErr_UnexpectedError;
        } else {
            strLog += String8::format("Gain[%d]=(%d) ", i, wbTable[i].denominator);
        }
    }

    // TODO:(Yuan Jung) remove if define if AI-Deblur is done
#if 0
    // Get Lens shading table
    IMetadata::Memory lscData;
    if (!IMetadata::getEntry<IMetadata::Memory>(halMeta, MTK_LSC_TBL_DATA, lscData)) {
        ainrLogF("cannot get MTK_LSC_TBL_DATA");
        return AinrErr_UnexpectedError;
    } else {
        strLog += String8::format("lsc table size(%lu) ", lscData.size());
    }
#endif
    // Get AE region table
    constexpr int regionTableSize = 5;
    int regionTable[regionTableSize];
    for (int i = 0; i < regionTableSize; i++) {
        if (!IMetadata::getEntry<MINT32>(appMeta, MTK_CONTROL_AE_REGIONS, regionTable[i], i)) {
            ainrLogF("cannot get MTK_SENSOR_NEUTRAL_COLOR_POINT");
            return AinrErr_UnexpectedError;
        }
    }
    strLog += String8::format("Left(%d) Top(%d) Right(%d) Bottom(%d) Weight(%d)", regionTable[0]
                                , regionTable[1], regionTable[2], regionTable[3], regionTable[4]);
    ainrLogD("DRC %s", strLog.string());

    // Start DRC
    std::unique_ptr<MTKDRC, std::function<void(MTKDRC*)>> pDRCDrv = nullptr;
    pDRCDrv = decltype(pDRCDrv)(MTKDRC::createInstance(DRV_DRC_OBJ_SW),
        [](MTKDRC* drcDrv) {
            if (CC_UNLIKELY(drcDrv == nullptr)) {
                ainrLogW("DRCDrv is nullptr");
            } else {
                ainrLogD("DRCDrv unint");
                drcDrv->destroyInstance();
            }
        });
    if (CC_UNLIKELY(pDRCDrv == nullptr))
    {
        ainrLogF("pDRCDrv is null");
        return AinrErr_UnexpectedError;
    }

    DRC_VerInfo rDRCVer;
    pDRCDrv->DRCFeatureCtrl(DRC_FTCTRL_GET_VERSION, nullptr, &rDRCVer);

    DRC_IN_STRUCT rDRCIn;  // Allocate DRC_IN_STRUCT in stack since its size is small
    DRCAEInfo &aeInfo = rDRCIn.DrcAe;
    aeInfo.i4LV = lightValue;

    DRC_DUMP_INFO &dumpInfo = rDRCIn.dumpInfo;
    dumpInfo.uniqueKey = params.uniqueKey;
    dumpInfo.requestNum = params.requestNum;
    dumpInfo.frameNum = params.frameNum;

    // DRC NVRAM
    size_t drcTuningSize = 0;
    const FEATURE_NVRAM_AIISP_ISO_DRC_T* drcTuning
            = reinterpret_cast<const FEATURE_NVRAM_AIISP_ISO_DRC_T*>
            (mNvram->getSpecificChunk(
            IAinrNvram::nvram_hint::AIISP_ISO_DRC, &drcTuningSize));

    // Prepare ISP Info
    DRCISPInfo &ispInfo = rDRCIn.DrcIsp;
    ispInfo.NVRAMTable = drcTuning;
    ispInfo.LSCTable = nullptr;
    for (int i = 0; i < 3; i++) {
        ispInfo.WBTable[i] = wbTable[i].denominator;
    }

    // DRC buffer information
    auto rawSize = rawBuf->getImgSize();
    int bayerOrder = rawBuf->getColorArrangement();

    DRCInput &InputInfo = rDRCIn.DrcIn;
    InputInfo.ImgWidth = rawSize.w;
    InputInfo.ImgHeight = rawSize.h;
    InputInfo.BayerOrder = bayerOrder;
    InputInfo.mtkCameraFaceMetadata = &fdData.facedata;
    InputInfo.RawIN = reinterpret_cast<uint16_t*>(rawBuf->getBufVA(0));
    for (int i = 0; i < 4; i++) {
        InputInfo.touch_rect[i] = static_cast<uint16_t>(regionTable[i]);
    }

    auto pDRCOut = std::make_unique<DRC_OUT_STRUCT>();  // Allocate DRC_OUT_STRUCT in heap since its size is too large
    if (CC_UNLIKELY(pDRCOut.get() == nullptr)) {
        ainrLogF("Allocate DRC_OUT_STRUCT fail");
        return AinrErr_UnexpectedError;
    }

    // Set up DRC output no matter what stage it is. It would be 16 bit raw.
    pDRCOut->RawOUT = reinterpret_cast<uint16_t*>(rawBuf->getBufVA(0));

    // DRC Statistics DRC input should be 16 bits raw
    pDRCDrv->DRCMain(DRC_PROC_STATISTICS, &rDRCIn,
            reinterpret_cast<void*>(pDRCOut.get()));

    // HRL HDR ratio
    auto drcIspReg = pDRCOut->DRCISPReg;
    mHlrRatio = drcIspReg.HLR_HDR_RATIO;
    ainrLogD("HLR ratio(%d)", mHlrRatio);

    // Make raw apply LSC/WB
    sp<IImageBuffer> highBitRawBuffer = nullptr;
    ImageBufferUtils::getInstance().allocBuffer(
        highBitRawBuffer, rawSize.w, rawSize.h, eImgFmt_BAYER22_PAK);
    if (CC_UNLIKELY(highBitRawBuffer.get() == nullptr)) {
        ainrLogF("highBitRawBuffer is null");;
        return AinrErr_UnexpectedError;
    }

    applyShadingWb(openId, rawBuf, highBitRawBuffer.get(),
            appMeta, halMeta);

    // DRC Apply
    InputInfo.RawIN = reinterpret_cast<uint16_t*>(highBitRawBuffer->getBufVA(0));
    ainrLogD("DRC+");
    pDRCDrv->DRCMain(DRC_PROC_APPLY, &rDRCIn,
            reinterpret_cast<void*>(pDRCOut.get()));
    ainrLogD("DRC-");

    // Copy result to Metadata
    IMetadata::Memory ltmTuningBuffer;
    int ltmTuningBufferSize = static_cast<int>(sizeof(HWLTMReg));
    ltmTuningBuffer.resize(ltmTuningBufferSize);
    memcpy(reinterpret_cast<void*>(ltmTuningBuffer.editArray()),
            &(pDRCOut->DRCHWLTMReg), ltmTuningBufferSize);
    IMetadata::setEntry<IMetadata::Memory>(halMeta,
            MTK_ISP_DRC_CURVE, ltmTuningBuffer);
    IMetadata::setEntry<MINT32>(halMeta,
            MTK_ISP_DRC_CURVE_SIZE, ltmTuningBufferSize);

    ImageBufferUtils::getInstance().deallocBuffer(highBitRawBuffer);

    return err;
}

enum AinrErr AiAlgo::applyShadingWb(const int openId, NSCam::IImageBuffer* inBuf, NSCam::IImageBuffer* outBuf,
        const NSCam::IMetadata* appMeta, const NSCam::IMetadata* halMeta) {
    CAM_ULOGM_APILIFE();

    // Normal stream
    using uPtrNormalStream
            = std::unique_ptr<INormalStream, std::function<void(INormalStream*)>>;
    MUINT8 ispProfile = NSIspTuning::EIspProfile_AIISP_DRC;

    uPtrNormalStream stream(
        INormalStream::createInstance(openId),
        [](INormalStream* p) {
            if (!p) return;
            p->uninit(LOG_TAG);
            p->destroyInstance();
        }
    );
    if (CC_UNLIKELY(stream.get() == nullptr)) {
        ainrLogF("Create notrmal stream fail");
        return AinrErr_UnexpectedError;
    }

    auto bResult = stream->init(LOG_TAG);
    if (CC_UNLIKELY(bResult == MFALSE)) {
        ainrLogF("init INormalStream returns MFALSE");
        return AinrErr_UnexpectedError;
    }

    // Get buffer information
    auto inputSize = inBuf->getImgSize();

    QParams enqueParams;
    FrameParams frameParams;
    frameParams.mStreamTag = NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_Vss;
    frameParams.mSecureFra = 0;  // Normal path

    auto setUpCropInfo = [](const MSize size, const int groupId,
            MCrpRsInfo *crop)
            -> void {
            if (CC_UNLIKELY(crop == nullptr)) {
                ainrLogF("crop is nullptr");
                return;
            }

            // Group information
            crop->mFrameGroup = 0;
            crop->mGroupID = groupId;

            // Resolution setting
            MCropRect &cropRect = crop->mCropRect;
            cropRect.p_fractional.x = 0;
            cropRect.p_fractional.y = 0;
            cropRect.p_integral.x = 0;
            cropRect.p_integral.y = 0;
            cropRect.s.w = size.w;
            cropRect.s.h = size.h;

            // Resize destination size
            crop->mResizeDst.w = size.w;
            crop->mResizeDst.h = size.h;
    };

    // Set up each hardware port
    MCrpRsInfo crzo;
    MCrpRsInfo wdmao;
    MCrpRsInfo wroto;
    setUpCropInfo(inputSize, 1, &crzo);
    setUpCropInfo(inputSize, 2, &wdmao);
    setUpCropInfo(inputSize, 3, &wroto);

    frameParams.mvCropRsInfo.push_back(crzo);
    frameParams.mvCropRsInfo.push_back(wdmao);
    frameParams.mvCropRsInfo.push_back(wroto);


    // IMGI in/TIMGO out
    Input src;
    src.mPortID = PORT_IMGI;
    src.mPortID.group = 0;
    src.mBuffer = inBuf;
    frameParams.mvIn.push_back(src);

    Output dst;
    dst.mPortID = PORT_TIMGO;
    dst.mPortID.group = 0;
    dst.mBuffer = outBuf;
    frameParams.mvOut.push_back(dst);

    // 18 bit sepcial select for 16bit data
    MUINT32 rawDepthType
            = static_cast<MUINT32>(NSCam::NSIoPipe::EDIPRAWDepthType_BEFORE_DGN);
    ExtraParam extraDepthType;
    extraDepthType.CmdIdx = EPIPE_RAW_DEPTHTYPE_CMD;
    extraDepthType.moduleStruct
            = reinterpret_cast<void*>(&rawDepthType);
    frameParams.mvExtraParam.push_back(extraDepthType);

    // timgo special select
    MUINT32 timgoType
            = static_cast<MUINT32>(NSCam::NSIoPipe::EDIPTimgoDump_AFTER_HLR);
    ExtraParam extra;
    extra.CmdIdx = EPIPE_TIMGO_DUMP_SEL_CMD;  // In p2 driver timgo is a kind of debug port
    extra.moduleStruct = reinterpret_cast<void*>(&timgoType);
    frameParams.mvExtraParam.push_back(extra);

    // high bit handle
    MUINT32 rawHDRType
            = static_cast<MUINT32>(NSCam::NSIoPipe::EDIPRawHDRType_ISPHDR);
    ExtraParam extraHDRType;
    extraHDRType.CmdIdx = EPIPE_RAW_HDRTYPE_CMD;
    extraHDRType.moduleStruct
            = reinterpret_cast<void*>(&rawHDRType);
    frameParams.mvExtraParam.push_back(extraHDRType);

    // Debug dump information
    if (mDumpWorking & AinrDumpPartial) {
        int requestNum = 0;
        if (!IMetadata::getEntry<MINT32>(halMeta
                , MTK_PIPELINE_REQUEST_NUMBER, requestNum)) {
            ainrLogW("Get MTK_PIPELINE_REQUEST_NUMBER fail");
        }

        int frameNum = 0;
        if (!IMetadata::getEntry<MINT32>(halMeta
                , MTK_PIPELINE_FRAME_NUMBER, frameNum)) {
            ainrLogW("Get MTK_PIPELINE_FRAME_NUMBER fail");
        }

        int processUniqueKey = 0;
        if (!IMetadata::getEntry<MINT32>(halMeta
                , MTK_PIPELINE_UNIQUE_KEY, processUniqueKey)) {
            ainrLogW("cannot get unique about ainr capture");
        }

        int64_t timestamp = 0; // set to 0 if not found.
#if MTK_CAM_DISPAY_FRAME_CONTROL_ON
        if (!IMetadata::getEntry<MINT64>(halMeta
                , MTK_P1NODE_FRAME_START_TIMESTAMP, timestamp)) {
            ainrLogW("cannot get MTK_P1NODE_FRAME_START_TIMESTAMP");
        }

#else
        if (!IMetadata::getEntry<MINT64>(halMeta
                , MTK_SENSOR_TIMESTAMP, timestamp)) {
            ainrLogW("cannot get MTK_SENSOR_TIMESTAMP");
        }
#endif

        frameParams.RequestNo = requestNum;
        frameParams.FrameNo = frameNum;
        frameParams.Timestamp = timestamp;
        frameParams.UniqueKey = processUniqueKey;
        frameParams.IspProfile = ispProfile;

        // get up sensorDev id
        IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
        if (CC_UNLIKELY(pHalSensorList == nullptr)) {
            ainrLogF("get IHalSensorList instance failed");
            return AinrErr_UnexpectedError;
        }
        auto sensorDevId = pHalSensorList->querySensorDevIdx(openId);
        frameParams.SensorDev = sensorDevId;

        // Pass2 driver use 2 to indicate capture....
        frameParams.NeedDump = 2;
    }

    // Tuning buffer
    std::unique_ptr<NS3Av3::IHalISP, std::function<void(NS3Av3::IHalISP*)>>
            pHalIsp =
            std::unique_ptr<NS3Av3::IHalISP, std::function<void(NS3Av3::IHalISP*)>>
            (
                MAKE_HalISP(openId, LOG_TAG),
                [](NS3Av3::IHalISP* p) { if(CC_LIKELY(p)) p->destroyInstance(LOG_TAG); }
            );
    if (CC_UNLIKELY(pHalIsp.get() == nullptr)) {
        ainrLogF("create IHalIsp failed");
        return AinrErr_UnexpectedError;
    }

    auto tuningSize = stream->getRegTableSize();
    auto tuningBuf = std::unique_ptr<char[]>(new char[tuningSize]{0});
    if(CC_UNLIKELY(tuningBuf.get() == nullptr)) {
        ainrLogF("allocate tuning buffer error");
        return AinrErr_UnexpectedError;
    }

    NS3Av3::MetaSet_T inMetaSet(*(appMeta), *(halMeta));
    IMetadata::setEntry<MUINT8>(&inMetaSet.halMeta, MTK_3A_ISP_PROFILE, NSIspTuning::EIspProfile_AIISP_DRC);
    IMetadata::setEntry<MUINT8>(&inMetaSet.halMeta, MTK_3A_PGN_ENABLE, 1);
    IMetadata::setEntry<MINT32>(&inMetaSet.halMeta, MTK_ISP_HLR_RATIO, mHlrRatio);

    TuningParam rTuningParam;
    rTuningParam.pRegBuf = tuningBuf.get();
    rTuningParam.pLcsBuf = nullptr;

    {
        CAM_ULOGM_TAGLIFE("Set up process raw");
        if (0 > pHalIsp->setP2Isp(0, inMetaSet, &rTuningParam, nullptr))
        {
            ainrLogF("pass2 setIsp - skip tuning pushing");
            return AinrErr_UnexpectedError;
        }
    }
    frameParams.mTuningData = tuningBuf.get();

    Input lscBuffer;
    lscBuffer.mPortID = PORT_LSCI;
    lscBuffer.mPortID.group = 0;
    lscBuffer.mBuffer = reinterpret_cast<IImageBuffer*>(rTuningParam.pLsc2Buf);
    frameParams.mvIn.push_back(lscBuffer);

    // Finish set upt frameParams
    enqueParams.mvFrameParams.push_back(frameParams);

    // Callback
    Pass2Async p2Async;
    enqueParams.mpCookie = reinterpret_cast<void*>(&p2Async);
    enqueParams.mpfnCallback = [](QParams& rParams)->MVOID
    {
        if (CC_UNLIKELY(rParams.mpCookie == nullptr)) {
            ainrLogF("Cookie is nullptr");
            return;
        }

        Pass2Async* pAsync = reinterpret_cast<Pass2Async*>(rParams.mpCookie);
        std::lock_guard<std::mutex> __l(pAsync->getLocker());
        pAsync->notifyOne();
        ainrLogD("Raw2Raw pass2 done");
    };

    // Start to enque request to pass2
    {
        CAM_ULOGM_TAGLIFE("RAW2Raw processing");
        auto __l = p2Async.uniqueLock();
        MBOOL bEnqueResult = stream->enque(enqueParams);
        if (CC_UNLIKELY(!bEnqueResult)) {
            ainrLogF("Raw2Raw stream enque fail");
            return AinrErr_UnexpectedError;
        }
        else {
            ainrLogD("Wait for Raw2Raw processing....");
            p2Async.wait(std::move(__l));
        }
    }

    if (mDumpWorking & AinrDumpWorking) {
        char fileName[256] = "";
        FILE_DUMP_NAMING_HINT dumpNamingHint{};

        // Extract hal metadata and fill up file name;
        extract(&dumpNamingHint, halMeta);
        // Extract buffer information and fill up file name;
        extract(&dumpNamingHint, outBuf);
        // Extract by sensor id
        extract_by_SensorOpenId(&dumpNamingHint, openId);

        genFileName_RAW(fileName, sizeof(fileName), &dumpNamingHint,
                TuningUtils::RAW_PORT_UNDEFINED, "HighBitRaw");
        outBuf->saveToFile(fileName);
    }

    return AinrErr_Ok;
}

enum AinrErr AiAlgo::calToneMatrix(const AinrAeInfo &src __attribute__((unused)),
        const AinrAeInfo &target __attribute__((unused)),
        std::vector<int> *matrix __attribute__((unused))) {
    ainrLogW("not support calToneMatrix in AINR");
    return AinrErr_Ok;
}

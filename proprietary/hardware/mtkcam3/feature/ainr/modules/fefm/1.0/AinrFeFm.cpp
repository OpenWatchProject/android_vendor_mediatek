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
#define LOG_TAG "AinrCore/FeFm"

#include "AinrFeFm.h"
#include <mtkcam3/feature/ainr/AinrUlog.h>
// Pass2 header
#include <mtkcam/drv/def/ispio_port_index.h> // port id

#include <unordered_map> // std::unordered_map

// platform dependent headers
// If you want to debug with dip_reg.h remember to add
// "in mk (AINR_INCLUDE_PATH += $(MTK_PATH_SOURCE)/hardware/mtkcam/drv/include/dip/isp_50/)"
//#include <dip_reg.h> // dip_x_reg_t
// ISP profile
#include <tuning_mapping/cam_idx_struct_ext.h>
// Allocate working buffer. Be aware of that we use AOSP library
#include <mtkcam3/feature/utils/ImageBufferUtils.h>
#include <mtkcam/utils/TuningUtils/FileDumpNamingRule.h> // tuning file naming

//AOSP
#include <utils/Vector.h>
#include <cutils/properties.h>

// Isp profile mapper
#include <mtkcam3/feature/utils/ISPProfileMapper.h>

using namespace ainr;
using namespace NSImageio::NSIspio;
using namespace NSCam::TuningUtils;

//-----------------------------------------------------------------------------
// IAinrFeFm methods
//-----------------------------------------------------------------------------
std::shared_ptr<IAinrFeFm> IAinrFeFm::createInstance()
{
    std::shared_ptr<IAinrFeFm> pAinrFeFm = std::make_shared<AinrFeFm>();
    return pAinrFeFm;
}

AinrFeFm::AinrFeFm()
    : m_sensorId(0)
    , m_blockSize(0)
    , m_regTableSize(0)
{
}

AinrFeFm::~AinrFeFm()
{
    m_pNormalStream = nullptr;
    m_pHalIsp = nullptr;
}

enum AinrErr AinrFeFm::init(IAinrFeFm::ConfigParams const& params,
        IAinrFeFm::OutInfo *outParam) {
    enum AinrErr err = AinrErr_Ok;
    auto sensorId = params.openId;
    m_sensorId = params.openId;

    /* RAII for INormalStream */
    m_pNormalStream = decltype(m_pNormalStream)(
        INormalStream::createInstance(sensorId),
        [](INormalStream* p) {
            if (!p) return;
            p->uninit(LOG_TAG);
            p->destroyInstance();
        }
    );

    if (CC_UNLIKELY( m_pNormalStream.get() == nullptr )) {
        ainrLogF("create INormalStream fail");
        return AinrErr_UnexpectedError;
    }
    else {
        auto bResult = m_pNormalStream->init(LOG_TAG);
        if (CC_UNLIKELY(bResult == MFALSE)) {
            ainrLogF("init INormalStream returns MFALSE");
            return AinrErr_UnexpectedError;
        }
    }

     // create IHal3A
    m_pHalIsp =
            std::unique_ptr< IHalISP, std::function<void(IHalISP*)> >
            (
                MAKE_HalISP(sensorId, "AinrFefm"),
                [](IHalISP* p) { if(CC_LIKELY(p)) p->destroyInstance("AinrFefm"); }
            );
    if (CC_UNLIKELY(m_pHalIsp.get() == nullptr)) {
        ainrLogF("create IHalIsp failed");
        return AinrErr_UnexpectedError;
    }

    // Tuning size
    m_regTableSize = m_pNormalStream->getRegTableSize();

    // Set up FEFM tuning
    setupFeFmTuning();
    //
    m_blockSize = getFeFmBlockSize(m_feInfo.mFEMODE);

    if(CC_UNLIKELY(m_blockSize == 0)) {
        ainrLogF("FEFM block size is null!!!");
        return AinrErr_UnexpectedError;
    } else {
        ainrLogD("Fe mode(%d), block size(%d)", m_feInfo.mFEMODE, m_blockSize);
    }

    // Record resolution and fefm size
    // FeFm size should be block size alignment
    m_bufferSize = params.bufferSize;

    //
    auto calculateResize = [] (const MSize& inputSize, const int aligned,
            MSize* outputSize) -> void {
        int outWidth = inputSize.w;
        int outHeight = inputSize.h;
        std::map<EDIPInfoEnum, MUINT32> pDipInfo;
        if(INormalStream::queryDIPInfo(pDipInfo)) {
            uint32_t fmMaxWidth = pDipInfo[EDIPINFO_FM_WIDTH_LIMITATION];
            if (inputSize.w >= fmMaxWidth) {
                // TODO(Yuan Jung) Next chip we will try to put
                // this ratio in NVRAM
                outWidth = fmMaxWidth;
                outHeight = (fmMaxWidth * inputSize.h)/inputSize.w;
            }
            ainrLogD("FM max width(%d)", fmMaxWidth);
        }

        #define ALIGN(v, a) (((v + (a-1)) / a) * a)
        outputSize->w = ALIGN(outWidth, aligned);
        outputSize->h = ALIGN(outHeight, aligned);
        #undef ALIGN
    };

    calculateResize(m_bufferSize, m_blockSize, &m_CropSize);
    ainrLogD("FeFm input size (%d, %d) crop size(%d, %d)"
        , m_bufferSize.w, m_bufferSize.h
        , m_CropSize.w, m_CropSize.h);

    m_feoSize.w = (m_CropSize.w/m_blockSize)*40;
    m_feoSize.h = m_CropSize.h/m_blockSize;
    ainrLogD("Feo size(%d, %d)", m_feoSize.w, m_feoSize.h);
    //
    m_fmoSize.w = (m_feoSize.w/40)*2;
    m_fmoSize.h = m_feoSize.h;
    ainrLogD("Fmo size(%d, %d)", m_fmoSize.w, m_fmoSize.h);
    // FMO tuning width/height
    m_fmInfo.mFMWIDTH = m_CropSize.w/m_blockSize;
    m_fmInfo.mFMHEIGHT = m_CropSize.h/m_blockSize;

    {
        std::lock_guard<std::mutex> __lk(m_cbMx);
        m_cbMethod = params.cbMethod;
    }

    if (CC_LIKELY(outParam)) {
        MRect &resizeCrop = outParam->resizeCrop;
        resizeCrop.p.x = 0;
        resizeCrop.p.y = 0;
        resizeCrop.s.w = m_CropSize.w;
        resizeCrop.s.h = m_CropSize.h;
        outParam->feoSize = m_feoSize;
        outParam->fmoSize = m_fmoSize;
    }

    return err;
}

enum AinrErr AinrFeFm::getFeFmSize(MSize &feoSize, MSize &fmoSize)
{
    //
    feoSize = m_feoSize;
    fmoSize = m_fmoSize;
    return AinrErr_Ok;
}

enum AinrErr AinrFeFm::prepareFE(QParams &qParams, IImageBuffer *inputBuff,
        IImageBuffer *outputFeo, IImageBuffer *outputYuv,
        std::vector<int> *matrix, MetaSet_T &metaset)
{
    // PrepareFE
    FrameParams paramsFe;
    paramsFe.mStreamTag = ENormalStreamTag_Normal;  //Normal case FE would work as well. ENormalStreamTag_FE;

    // input
    Input inputFe;
    inputFe.mPortID = PORT_IMGI;
    inputFe.mBuffer = inputBuff;
    paramsFe.mvIn.push_back(inputFe);
    // Output
    Output outFe;
    outFe.mPortID = PORT_FEO;
    outFe.mBuffer = outputFeo;
    paramsFe.mvOut.push_back(outFe);

    // Bittrue flow
    if(outputYuv) {
        // Output YUV in IMG3O for bit true
        Output outYuv;
        outYuv.mPortID       = PORT_IMG3O;
        outYuv.mBuffer       = outputYuv;
        outYuv.mPortID.group = 0;
        paramsFe.mvOut.push_back(outYuv);
    }
    // Module SRZ
    // Be aware of that out_w/out_h should be
    // the same as crop_w/crop_h
    m_srzSizeInfo.in_w = m_bufferSize.w;
    m_srzSizeInfo.in_h = m_bufferSize.h;
    m_srzSizeInfo.out_w =  m_CropSize.w;
    m_srzSizeInfo.out_h =  m_CropSize.h;
    m_srzSizeInfo.crop_w = m_CropSize.w;
    m_srzSizeInfo.crop_h = m_CropSize.h;

    std::shared_ptr<char> tuningBuffer(new char[m_regTableSize]{},
            std::default_delete<char[]>());
    if(CC_UNLIKELY(tuningBuffer.get() == nullptr)) {
        ainrLogF("FE tuning buffer is nullptr");
        return AinrErr_BadArgument;
    }

    ModuleInfo moduleInfo;
    moduleInfo.moduleTag = EDipModule_SRZ1;
    moduleInfo.moduleStruct = &m_srzSizeInfo;
    paramsFe.mvModuleData.push_back(moduleInfo);
    // ExtraParam
    ExtraParam extra;
    extra.CmdIdx = EPIPE_FE_INFO_CMD;
    extra.moduleStruct = static_cast<void*>(&m_feInfo);
    paramsFe.mvExtraParam.push_back(extra);

    // Query ispProfile
    ISPProfileMapper* ispProfileMapper = ISPProfileMapper::getInstance();
    if (CC_UNLIKELY(ispProfileMapper == nullptr)) {
        ainrLogF("ispProfileMapper is nullptr");
        return AinrErr_BadArgument;
    }

    auto mappingKey
            = ispProfileMapper->queryMappingKey(&metaset.appMeta,
                &metaset.halMeta,
                false,  // is physical or not
                eMappingScenario_Capture,
                eSensorComb_Single);

    NSIspTuning::EIspProfile_T ispProfile = {};
    auto profileStage = eStage_AIBC_FE;
    if(!ispProfileMapper->mappingProfile(mappingKey, profileStage,
            ispProfile)) {
        ainrLogF("Failed to get ISP Profile, stage(%d)", profileStage);
        return AinrErr_BadArgument;
    }
    ainrLogD("FE ispProfile(%d)", ispProfile);

    // Isp tuning. We need to do raw2yuv for feo
    // therefore we need to setIsp
    {
        CAM_ULOGM_TAGLIFE("FEFM setIsp");
        IMetadata::setEntry<MUINT8>(&metaset.halMeta, MTK_3A_ISP_PROFILE, ispProfile);
        // We dont need lsc therefore we bypass it
        IMetadata::setEntry<MINT32>(&metaset.halMeta, MTK_3A_ISP_BYPASS_LCE, 1);
        //trySetMetadata<MUINT8>(pIMetaHal, MTK_3A_PGN_ENABLE, 0);
        IMetadata::setEntry<MUINT8>(&metaset.halMeta, MTK_3A_PGN_ENABLE, 0); //Disable shading
        // Our input is raw buffer. We should indicate it as raw to ISP
        IMetadata::setEntry<MINT32>(&metaset.halMeta, MTK_ISP_P2_IN_IMG_FMT, 0);

        TuningParam rTuningParam;
        rTuningParam.pRegBuf = tuningBuffer.get();
        rTuningParam.pLcsBuf = nullptr;

        if (matrix) {
            rTuningParam.pUserCCM_Setting = reinterpret_cast<MUINT32*>(matrix->data());
            rTuningParam.u4UserCCM_Setting_Size = static_cast<MUINT32>(matrix->size());
            ainrLogD("CCM size(%d)", rTuningParam.u4UserCCM_Setting_Size);
        }

        auto rSetIsp = [&] {
            if (CC_UNLIKELY(m_pHalIsp.get() == nullptr)) {
                ainrLogE("IHal3A instance is NULL");
                return -1;
            }
            return m_pHalIsp->setP2Isp(
                    0,
                    metaset,
                    &rTuningParam,
                    &metaset);
        }();

        if (CC_UNLIKELY(rSetIsp != 0)) {
            ainrLogF("setIsp returns fail");
            return AinrErr_BadArgument;
        }
        // End
    }
    paramsFe.mTuningData = tuningBuffer.get();

    // Set up performance decision
    struct timeval current = {};
    ::gettimeofday(&current, nullptr);
    paramsFe.ExpectedEndTime = current;

    qParams.mvFrameParams.push_back(paramsFe);

    // Keep tuning buffer
    m_tuningBuffers.emplace_back(std::move(tuningBuffer));
    return AinrErr_Ok;
}

enum AinrErr AinrFeFm::buildFe(IAinrFeFm::DataPackage *package)
{
    if (CC_UNLIKELY(package->appMeta == nullptr)
        || CC_UNLIKELY(package->halMeta == nullptr)
        || CC_UNLIKELY(package->inBuf == nullptr)
        || CC_UNLIKELY(package->outBuf== nullptr)) {
        ainrLogE("One of data is null in packages!!");
        return AinrErr_BadArgument;
    }

    MetaSet_T metaset;
    metaset.appMeta = *(package->appMeta);
    metaset.halMeta = *(package->halMeta);

    prepareFE(m_qParams, package->inBuf, package->outBuf, package->outYuv,
            package->matrix, metaset);
    return AinrErr_Ok;
}

enum AinrErr AinrFeFm::doFe(const IAinrFeFm::DataPackage *package)
{
    CAM_ULOGM_FUNCLIFE_ALWAYS();

    if (CC_UNLIKELY(package->appMeta == nullptr)
        || CC_UNLIKELY(package->halMeta == nullptr)
        || CC_UNLIKELY(package->inBuf == nullptr)
        || CC_UNLIKELY(package->outBuf== nullptr)) {
        ainrLogE("One of data is null in packages!!");
        return AinrErr_BadArgument;
    }

    MetaSet_T metaset = {};
    metaset.appMeta = *(package->appMeta);
    metaset.halMeta = *(package->halMeta);

    int32_t bDumWorking
            = ::property_get_int32("vendor.debug.camera.ainr.dump", 0);
    int32_t bBittrue = ::property_get_int32("vendor.bittrue.ainr.en", 0);

    sp<IImageBuffer> yuvBuf = nullptr;
    if (bBittrue) {
        MBOOL ret = ImageBufferUtils::getInstance().allocBuffer(
            yuvBuf, m_bufferSize.w, m_bufferSize.h, eImgFmt_YV12);
        if (CC_UNLIKELY(ret == MFALSE)
                || CC_UNLIKELY(yuvBuf.get() == nullptr)) {
            ainrLogF("allocate resize YUV buffer error!!!");
            return AinrErr_UnexpectedError;
        }
    }

    //
    QParams qParams = {};
    prepareFE(qParams, package->inBuf, package->outBuf, yuvBuf.get(),
            nullptr /* color matrix*/, metaset);

    // Set up normalPipe callback
    Pass2Async p2Async = {};
    qParams.mpCookie = static_cast<void*>(&p2Async);
    qParams.mpfnCallback = [](QParams& rParams)->MVOID
    {
        if (CC_UNLIKELY(rParams.mpCookie == nullptr))
            return;

        Pass2Async* pAsync = static_cast<Pass2Async*>(rParams.mpCookie);
        std::lock_guard<std::mutex> __l(pAsync->getLocker());
        pAsync->notifyOne();
        ainrLogD("FE pass2 done");
    };

    // Start to enque request to pass2
    {
        auto __l = p2Async.uniqueLock();
        CAM_ULOGM_TAGLIFE("FE processing");
        MBOOL bEnqueResult = MTRUE;
        {
            if (CC_UNLIKELY(m_pNormalStream.get() == nullptr)) {
                ainrLogF("INormalStream instance is nullptr");
                return AinrErr_BadArgument;
            }
            ainrLogD("FE pass2 enque");
            bEnqueResult = m_pNormalStream->enque(qParams);
        }

        if (CC_UNLIKELY(!bEnqueResult)) {
            ainrLogF("FE enque fail");
            return AinrErr_BadArgument;
        }
        else {
            p2Async.wait(std::move(__l));
        }
    }

    if (bDumWorking & AinrDumpWorking) {
        // Feo
        bufferDump(package->outBuf, package->halMeta , "Singlefeo",
                static_cast<int>(RAW_PORT_UNDEFINED), formatType::BLOB);
        // Dump YV12 debug buffer
        if (bBittrue) {
            bufferDump(yuvBuf.get(), package->halMeta , "SingleYuv",
                static_cast<int>(RAW_PORT_UNDEFINED), formatType::BLOB);
        }
    }

    return AinrErr_Ok;
}

enum AinrErr AinrFeFm::buildFm(IImageBuffer *baseBuf, IImageBuffer *refBuf
                                        , IImageBuffer *fmo, int index, bool needCb)
{
    if (CC_UNLIKELY(baseBuf == nullptr)
        || CC_UNLIKELY(refBuf == nullptr)
        || CC_UNLIKELY(fmo == nullptr)) {
        ainrLogE("One of data is null in FM argument list !!");
        return AinrErr_BadArgument;
    }

    prepareFM(m_qParams, baseBuf, refBuf, fmo, index, needCb);
    return AinrErr_Ok;
}

enum AinrErr AinrFeFm::doFeFm()
{
    // Set up normalPipe callback
    Pass2Async p2Async;
    m_qParams.mpCookie = static_cast<void*>(&p2Async);
    m_qParams.mpfnCallback = [](QParams& rParams)->MVOID
    {
        if (CC_UNLIKELY(rParams.mpCookie == nullptr))
            return;

        Pass2Async* pAsync = static_cast<Pass2Async*>(rParams.mpCookie);
        std::lock_guard<std::mutex> __l(pAsync->getLocker());
        pAsync->notifyOne();
        ainrLogD("FEFM pass2 done");
    };

    // Start to enque request to pass2
    {
        auto __l = p2Async.uniqueLock();
        CAM_ULOGM_TAGLIFE("FEFM processing");
        MBOOL bEnqueResult = MTRUE;
        {
            if (CC_UNLIKELY(m_pNormalStream.get() == nullptr)) {
                ainrLogE("INormalStream instance is NULL");
                return AinrErr_BadArgument;
            }
            ainrLogD("FEFM pass2 enque");
            bEnqueResult = m_pNormalStream->enque(m_qParams);
        }

        if (CC_UNLIKELY(!bEnqueResult)) {
            ainrLogE("FEFM enque fail");
            return AinrErr_BadArgument;
        }
        else {
            p2Async.wait(std::move(__l));
        }
    }
    return AinrErr_Ok;
}

enum AinrErr AinrFeFm::prepareFM(QParams &qParams, IImageBuffer *feoBase,
        IImageBuffer *feoRef, IImageBuffer *fmo, int index, bool needCb)
{
    std::shared_ptr<char> tuningBuffer(new char[m_regTableSize]{},
            std::default_delete<char[]>());
    if(CC_UNLIKELY(tuningBuffer.get() == nullptr)) {
        ainrLogF("FM tuning buffer is nullptr");
        return AinrErr_BadArgument;
    }

    // PrepareFE
    FrameParams paramsFm;
    paramsFm.mStreamTag = ENormalStreamTag_FM;

    // input
    Input inputFeoBase;
    inputFeoBase.mPortID = PORT_LFEOI;
    inputFeoBase.mBuffer = feoBase;
    paramsFm.mvIn.push_back(inputFeoBase);
    Input inputFeoRef;
    inputFeoRef.mPortID = PORT_RFEOI;
    inputFeoRef.mBuffer = feoRef;
    paramsFm.mvIn.push_back(inputFeoRef);

    // Output
    Output outFm;
    outFm.mPortID = PORT_FMO;
    outFm.mBuffer = fmo;
    paramsFm.mvOut.push_back(outFm);

    // Tuning data
    paramsFm.mTuningData = static_cast<MVOID*>(tuningBuffer.get());

    // Extra param FM tuning
    ExtraParam extra;
    extra.CmdIdx = EPIPE_FM_INFO_CMD;
    extra.moduleStruct = static_cast<void*>(&m_fmInfo);
    paramsFm.mvExtraParam.push_back(extra);

    if(needCb) {
        paramsFm.mpCookie = reinterpret_cast<MVOID*>(this);
        paramsFm.FrameNo  = index;

        // Per-Frame callback
        paramsFm.mpfnCallback = [](const FrameParams& rFrmParams, EFrameCallBackMsg FrmCBMsg) -> bool {
            AinrFeFm* fefm = reinterpret_cast<AinrFeFm*>(rFrmParams.mpCookie);
            std::lock_guard<std::mutex> __lk(fefm->m_cbMx);

            ainrLogD("FEFM callback frame number(%d)", rFrmParams.FrameNo);
            fefm->m_cbMethod(rFrmParams.FrameNo);

            return true;
        };
    }

    // Set up performance decision
    struct timeval current = {};
    ::gettimeofday(&current, nullptr);
    paramsFm.ExpectedEndTime = current;

    qParams.mvFrameParams.push_back(paramsFm);

    // Keep tuning buffer
    m_tuningBuffers.emplace_back(std::move(tuningBuffer));
    return AinrErr_Ok;
}

enum AinrErr AinrFeFm::checkPackage(IAinrFeFm::BufferPackage &bufferSet, IAinrFeFm::MetaDataPackage &metaSet)
{
    if(CC_UNLIKELY(!bufferSet.inBase
        && !bufferSet.inRef
        && !bufferSet.outFeoBase
        && !bufferSet.outFeoRef
        && !bufferSet.outFmoBase
        && !bufferSet.outFmoRef
        && !metaSet.appMetaBase
        && !metaSet.halMetaBase
        && !metaSet.dynamicMetaBase
        && !metaSet.appMetaRef
        && !metaSet.halMetaRef
        && !metaSet.dynamicMetaRef)) {
        ainrLogE("Check pacakge fail because of null pointer");
        return AinrErr_NullPointer;
    }
    return AinrErr_Ok;
}

enum AinrErr AinrFeFm::setupFeFmTuning()
{
    // TODO: We need to load tuning data by Json file instead of hard code
    // FE info
    m_feInfo.mFEDSCR_SBIT = 3;
    m_feInfo.mFETH_C = 2;
    m_feInfo.mFETH_G = 0;
    m_feInfo.mFEFLT_EN = 1;
    m_feInfo.mFEPARAM = 8;
    m_feInfo.mFEMODE = 1;
    // FM info
    m_fmInfo.mFMSR_TYPE = 0;
    m_fmInfo.mFMOFFSET_X = 16;
    m_fmInfo.mFMOFFSET_Y = 16;
    m_fmInfo.mFMRES_TH = 2;
    m_fmInfo.mFMSAD_TH = 1023;
    m_fmInfo.mFMMIN_RATIO = 90;

    return AinrErr_Ok;
}

int AinrFeFm::getFeFmBlockSize(const int FE_MODE)
{
    int blockSize = 16;
    switch(FE_MODE)
    {
        case 0:
            blockSize = 32;
            break;
        case 1:
            blockSize = 16;
            break;
        case 2:
            blockSize = 8;
            break;
        default:
            break;
    }
    return blockSize;
}

void AinrFeFm::bufferDump(IImageBuffer* buff, IMetadata* halMeta
    , const char *pUserString, int port, formatType type) {
    FILE_DUMP_NAMING_HINT dumpHint = {};
    // dump input buffer
    char fileName[256] = "";

    if (buff == nullptr
        || halMeta == nullptr
        || pUserString == nullptr) {
        ainrLogE("HalMeta or buff is nullptr, dump fail");
        return;
    }

    // Extract Unique key/Request numbuer/Frame number.. from halMeta
    extract(&dumpHint, halMeta);

    // Extract buffer information and fill up file name;
    extract(&dumpHint, buff);

    // Extract by sensor id
    extract_by_SensorOpenId(&dumpHint, m_sensorId);

    // Generate file name
    switch (type) {
        case formatType::RAW:
        case formatType::BLOB:
            genFileName_RAW(fileName, sizeof(fileName), &dumpHint, static_cast<RAW_PORT>(port), pUserString);
            break;
        case formatType::YUV:
             genFileName_YUV(fileName, sizeof(fileName), &dumpHint, static_cast<YUV_PORT>(port), pUserString);
            break;
        default:
            ainrLogW("formatType not define");
            break;
    }

    buff->saveToFile(fileName);

    return;
}

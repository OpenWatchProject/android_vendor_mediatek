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
#define LOG_TAG "AinrCore"

#include "AinrCore.h"
#include <mtkcam3/feature/ainr/AinrUlog.h>
// mtkcam
#include <mtkcam/utils/metadata/IMetadata.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/drv/iopipe/SImager/ISImager.h>

// JobQueue and memory utils
#include <mtkcam/utils/std/JobQueue.h>
#include <mtkcam/utils/sys/MemoryInfo.h>
//
#include <mtkcam/drv/IHalSensor.h> // sensor type

// ISP profile
#include <tuning_mapping/cam_idx_struct_ext.h>
// For RAWIspCamInfo
#include <isp_tuning/ver1/isp_tuning_cam_info.h>

// Debug exif
#include <mtkcam/utils/exif/DebugExifUtils.h>
#include <debug_exif/dbg_id_param.h>
#include <debug_exif/cam/dbg_cam_param.h>

// NVRAM
/*To-Do: remove when cam_idx_mgr.h reorder camear_custom_nvram.h order before isp_tuning_cam_info.h */
#include <camera_custom_nvram.h>
#include <mtkcam/utils/mapping_mgr/cam_idx_mgr.h>
#include <mtkcam/utils/std/StlUtils.h> // NSCam::SpinLock
#include <mtkcam/utils/sys/ThermalInfo.h>
#include <mtkcam/utils/sys/MemoryInfo.h>

// Allocate working buffer. Be aware of that we use AOSP library
#include <mtkcam3/feature/utils/ImageBufferUtils.h>
// AOSP
#include <cutils/compiler.h>
#include <cutils/properties.h>
#include <utils/String8.h>

// Online device tuning
#include <mtkcam/utils/TuningUtils/FileReadRule.h>

// STL
#include <vector> // std::vector
#include <deque> // std::deque
#include <map> // std::map
#include <string> // std::string
#include <future> // std::async, std::launch
#include <fstream> // std::ifstream
#include <regex> // std::regex, std::sregex_token_iterator
#include <iterator> // std::back_inserter
#include <cfenv>
#include <stdio.h>

// Buffer dump
#define AINR_DUMP_PATH              "/data/vendor/camera_dump/"
#define AINR_DUMP_TUNING_FILENAME    "ainr-tuning.bin"

using std::vector;
using std::map;
using std::string;
using NSCam::TuningUtils::FILE_DUMP_NAMING_HINT;
using namespace ainr;
using namespace NSIspTuning;

#define CHECK_AINR(ret, msg)     do { if (ret != AinrErr_Ok) { printf("Failed: %s\n", msg); assert(false); } }while(0)

// Static lock to protect NRCore. Only one AinrCore allowerd to do MDLA
static std::mutex sNrCoreMx;

/******************************************************************************
 * A I N R
 *****************************************************************************/

/**
 *  AinrCore
 */
AinrCore::AinrCore(void)
    : m_sensorId(0)
    , m_imgoWidth(0)
    , m_imgoHeight(0)
    , m_imgoStride(0)
    , m_rrzoWidth(0)
    , m_rrzoHeight(0)
    , m_uniqueKey(0)
    , m_fefmIspProfile(EIspProfile_AINR_MainYUV)
    , m_mainPathProfile(EIspProfile_AINR_Main)
    , m_captureNum(0)
    , m_requestNum(0)
    , m_frameNum(0)
    , m_dgnGain(0)
    , m_obFst(256)
    , mbNeedTileMode(true)
    , m_bIsLowMem(false)
    , m_outBuffer(nullptr)
    , m_NvramPtr(nullptr)
    , mb_AllocfefmDone(false)
    , mb_AllocUpDone(false)
    , mb_upBaseDone(false)
    , m_algoType(AINR_ONLY)
{
    m_DumpWorking  = ::property_get_int32("vendor.debug.camera.ainr.dump", 0);
    m_bittrueEn    = ::property_get_int32("vendor.bittrue.ainr.en", 0);
    m_onlineTuning = ::property_get_int32("vendor.debug.camera.dumpin.en", 0);
    m_autoTest     = ::property_get_int32("vendor.debug.ainr.test", 0);
}

AinrCore::~AinrCore(void)
{
    // wait Fefm future done (timeout 1 second)
    ainrLogD("~AinrCore");
    // Release fefm buffer
    CHECK_AINR( releaseFeFmBuffer(), "Release fefm buffer fail" );
    CHECK_AINR( releaseGridBuffer(), "Release grid buffer fail" );
    CHECK_AINR( releaseWarpingBuffer(), "Release warping buffer fail" );
    CHECK_AINR( releaseUnpackBuffer(), "Release unpack buffer fail");

    // NVRAM release
    {

        std::lock_guard<decltype(m_nvMx)> _l(m_nvMx);

        m_NvramPtr = nullptr;
    }

    if(m_DumpWorking) {
        mvMetaPacks.clear();
    }
}

/*
 * We initialize NRAlgo and P2 related modules (FEFM, Warping)
 * We ask jobQueue to do memory allocation.
*/
enum AinrErr AinrCore::init(const AinrConfig_t &cfg)
{
    AinrErr err = AinrErr_Ok;

    // Parsing cfg
    {
        std::lock_guard<std::mutex> _l(m_cfgMx);
        m_sensorId = cfg.sensor_id; // opened sensor id
        m_captureNum = cfg.captureNum;
        m_imgoWidth = cfg.imgoWidth;
        m_imgoHeight = cfg.imgoHeight;
        m_imgoStride = cfg.imgoStride;
        m_rrzoWidth = cfg.rrzoWidth;
        m_rrzoHeight = cfg.rrzoHeight;
        m_requestNum = cfg.requestNum;
        m_frameNum = cfg.frameNum;
        m_uniqueKey = cfg.uniqueKey;
        m_dgnGain   = cfg.dgnGain;
        mbNeedTileMode = true;
        m_algoType     = cfg.algoType;
    }

    // On device tuning for DGN gain and OB ofst
    if(m_onlineTuning == 2) {
        m_dgnGain = ::property_get_int32("vendor.ainr.dgnGain", 4928);
    }

    ainrLogD("Imgo (w,h,s) = (%d, %d, %lu), DGN gain(%d)", m_imgoWidth, m_imgoHeight, m_imgoStride
                                                       , m_dgnGain);
    // Fefm
    m_fefm = IAinrFeFm::createInstance();

    if(CC_UNLIKELY(m_fefm.get() == nullptr)) {
        ainrLogF("Get fefm error");
        return AinrErr_UnexpectedError;
    }

    // FEFM callback function to trigger
    // warping and unpack buffers.
    m_AsyncLauncher = [this] (int index) -> void {

        ainrLogD("Launch matching reference frame task(%d)", index);
        std::lock_guard<std::mutex> __lk(m_jobsMx);
        m_jobs.push_back(
                std::async(std::launch::async,
                    m_matchingMethod, index)
                );
    };

    // Init FEFM stage
    {
        std::lock_guard<std::mutex> _l(m_cfgMx);

        IAinrFeFm::OutInfo outParam = {};
        IAinrFeFm::ConfigParams fefmcfg = {};
        fefmcfg.openId = m_sensorId;
        // We use rrzo to do FEFM
        fefmcfg.bufferSize.w = m_rrzoWidth;
        fefmcfg.bufferSize.h = m_rrzoHeight;
        fefmcfg.cbMethod   = m_AsyncLauncher;
        m_fefm->init(fefmcfg, &outParam);

        m_vFeo.resize(m_captureNum);
        m_vFmoBase.resize(m_captureNum);
        m_vFmoRef.resize(m_captureNum);
        ainrLogD("Ainr capture number(%d)", m_captureNum);
    }

    // Allocate working buffer
    // We would like to use jobQueue to do buffer allocation
    // Get jobQueue
    if (__builtin_expect( mMinorJobQueue.get() == nullptr, false )) {
            acquireJobQueue();
    }

    // Allocate Fefm memory
    {
        MSize feoSize;
        MSize fmoSize;

        m_fefm->getFeFmSize(feoSize, fmoSize);

        auto _fefmAllocJob = [this](MSize pfeoSize, MSize pfmoSize) ->int {
            CAM_ULOGM_TAGLIFE("Allocate fefm buffers");
            std::lock_guard<std::mutex> _lk(m_fefmMx);

            MBOOL ret = MTRUE;
            // base feo
            for(auto && buf : m_vFeo) {
                ret = ImageBufferUtils::getInstance().allocBuffer(
                    buf, pfeoSize.w, pfeoSize.h, eImgFmt_STA_BYTE);
                if(CC_UNLIKELY(ret == MFALSE)) {
                    ainrLogF("allocate fmo buffer error!!!");
                    return AinrErr_UnexpectedError;
                }
            }
            // FmoBase
            for(auto && buf : m_vFmoBase) {
                ret = ImageBufferUtils::getInstance().allocBuffer(
                    buf, pfmoSize.w, pfmoSize.h, eImgFmt_STA_BYTE);
                if(CC_UNLIKELY(ret == MFALSE)) {
                    ainrLogF("allocate fmo buffer error!!!");
                    return AinrErr_UnexpectedError;
                }
            }
            // FmoRef
            for(auto && buf : m_vFmoRef) {
                ret = ImageBufferUtils::getInstance().allocBuffer(
                    buf, pfmoSize.w, pfmoSize.h, eImgFmt_STA_BYTE);
                if(CC_UNLIKELY(ret == MFALSE)) {
                    ainrLogF("allocate fmo buffer error!!!");
                    return AinrErr_UnexpectedError;
                }
            }

            mb_AllocfefmDone = true;
            ainrLogD("Allocate fefm buffer done");
            m_fefmCond.notify_one();
            return AinrErr_Ok;
        };

        mMinorJobQueue->addJob( std::bind( _fefmAllocJob, feoSize, fmoSize ));
    }

    // Allocate unpack
    auto _alloc_unpack = [this] () -> void {
        CAM_ULOGM_TAGLIFE("Allocate working(Warping+Unpack) buffers");
        std::lock_guard<std::mutex> _lk(m_AllocUpMx);

        // Allocate warping buffers
        // Output buffer allocation
        m_vRg.resize(m_captureNum);
        m_vGb.resize(m_captureNum);

        // WPE outout buffer should be 64 aligned
        MUINT32 aligned = 64;
        MSize outSize = MSize(m_imgoWidth, m_imgoHeight/2); // This is because one run for half of buffer

        for(int i = 1; i < m_captureNum; i++) {
            auto ret = ImageBufferUtils::getInstance().allocBuffer(
                m_vRg[i], outSize.w, outSize.h, eImgFmt_BAYER10, MTRUE, aligned);
            if(CC_UNLIKELY(ret == MFALSE)) {
                ainrLogE("allocate RG buffer error!!!");
                return;
            }
            ret = ImageBufferUtils::getInstance().allocBuffer(
                m_vGb[i], outSize.w, outSize.h, eImgFmt_BAYER10, MTRUE, aligned);
            if(CC_UNLIKELY(ret == MFALSE)) {
                ainrLogE("allocate GB buffer error!!!");
                return;
            }
        }

        // Allocate unpack buffers
        unsigned int size = m_imgoWidth*m_imgoHeight*sizeof(short);

        for(int i = 0; i < m_captureNum; i++) {
            sp<IImageBuffer> unpackRawBuff = nullptr;

            ImageBufferUtils::getInstance().allocBuffer(
            unpackRawBuff, size, 1, eImgFmt_STA_BYTE);
            if (CC_UNLIKELY( unpackRawBuff.get() == nullptr )) {
                ainrLogE("unpackRawBuff is null");;
                return;
            }

            mv_unpackRaws.push_back(unpackRawBuff);
        }

        mb_AllocUpDone = true;
        ainrLogD("Allocate warping and unpack buffer done");
        m_AllocUpCond.notify_one();
        return;
    };

    mMinorJobQueue->addJob( _alloc_unpack );

    // Init warping
    m_warping = IAinrWarping::createInstance();

    if(CC_UNLIKELY(m_warping.get() == nullptr)) {
        ainrLogF("Get warping error");
        return AinrErr_UnexpectedError;
    }

    // Init stage
    {

        IAinrWarping::ConfigParams warpingCfg;
        warpingCfg.openId = m_sensorId;
        warpingCfg.bufferSize.w = m_imgoWidth;
        warpingCfg.bufferSize.h = m_imgoHeight;
        m_warping->init(warpingCfg);
    }

    // Judge memory situation
    // Choose algo type
    int64_t memRestriction = NSCam::NSMemoryInfo::getSystemTotalMemorySize();
    std::fesetround(FE_TONEAREST);
    #define RESTRICT_MEM_TH (4.3)
    if (std::nearbyint(memRestriction/1024/1024/1024) < RESTRICT_MEM_TH)
    {
        ainrLogD("It is low memory. System Memory is (%" PRId64 ") <= 4.3G Mb, use down sampe "
                "nearbyint Memory is (%.1f)", memRestriction,
                nearbyint(memRestriction/1024/1024/1024));
        m_bIsLowMem = true;
    }
    #undef RESTRICT_MEM_TH

lbExit:

    return err;
}

enum AinrErr AinrCore::doAinr()
{
    CAM_ULOGM_APILIFE();
    AinrErr err = AinrErr_Ok;

    // Set Prformance Mode to Thermal Side
    if (NSCam::NSThermalInfo::setThermalMode(1) < 0) {
        ainrLogW("Write Thermal Hinet Falied");
    }

    parsingMeta();

    if(m_autoTest) {
        bufferReload();
    }

    if (__builtin_expect( mMinorJobQueue.get() == nullptr, false )) {
            acquireJobQueue();
    }

    // Unpack Base frame
    // Async Unpack golden frame and gen noise
    // Handle base frame
    auto _jobBase = [this]() -> void {
        CAM_ULOGM_TAGLIFE("Unpack base frame");
        NRUTILS_VerInfo rUtilsVer;
        MINT32 outBits = 12;

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
            }
        );

        if(CC_UNLIKELY(pNRUtilsDrv == nullptr))
        {
            ainrLogE(" Error: MTKNRUtils createInstance() fail");
            return;
        }

        pNRUtilsDrv->NRUtilsFeatureCtrl(NRUTILS_FTCTRL_GET_VERSION , NULL, &rUtilsVer);

        // Buffers
        sp<IImageBuffer> unpackRawBuff      = nullptr;
        sp<IImageBuffer> unpackNoiseRawBuff = nullptr;
        //
        NRUTILS_CONFIG_UNPACK_BAYER_IN_STRUCT rUnpackBaseIn;
        NRUTILS_UNPACK_BAYER_IN_STRUCT rProcIn;
        NRUTILS_UNPACK_BAYER_OUT_STRUCT rProcOut;
        IImageBuffer* imgoBase = m_vDataPackage[0].imgoBuffer;

        if(m_onlineTuning == 2  && !m_autoTest) {
            FileReadRule rule;
            const char* profileName = "EIspProfile_AINR_Single";
            rule.getFile_RAW(0, profileName, imgoBase, "P2Node", m_sensorId);
        }

        rUnpackBaseIn.u4CoreNum = 8;
        rUnpackBaseIn.i4Width   = m_imgoWidth;
        rUnpackBaseIn.i4Height  = m_imgoHeight;
        rUnpackBaseIn.i4Stride  = imgoBase->getBufStridesInBytes(0);
        rUnpackBaseIn.i4OutBits = outBits;
        //rUnpackBaseIn.i4Mode = 1; // Used for CPU version of unpack

        // OB and OB ofst
        {
            for(int i = 0; i < 4; i++) {
                rUnpackBaseIn.i4ObOffst[i] = 256;
                rUnpackBaseIn.i4DgnGain[i] = m_dgnGain;
            }
        }

        NRUTILS_CONFIG_UNPACK_BAYER_OUT_STRUCT rUnpackBaseOut;

        // config
        pNRUtilsDrv->NRUtilsFeatureCtrl(NRUTILS_FTCTRL_CONFIG_UNPACK_BAYER, &rUnpackBaseIn, &rUnpackBaseOut);

        // Only CPU version need working buffer
        sp<IImageBuffer> baseWorkingBuf = nullptr;

        if(rUnpackBaseOut.i4WMSize) {
            // prepare working buffer
            NRUTILS_BUFFER_STRUCT &rWM = rProcIn.rWM;
            rWM.Size = rUnpackBaseOut.i4WMSize;
            rWM.BufferNum = 1;
            // create working buffer
            ImageBufferUtils::getInstance().allocBuffer(
                baseWorkingBuf, rWM.Size, 1, eImgFmt_STA_BYTE);
            if (CC_UNLIKELY( baseWorkingBuf.get() == nullptr )) {
                ainrLogE("baseWorkingBuf is null");;
                return;
            }
            rWM.pBuff[0]  = reinterpret_cast<void*>(baseWorkingBuf->getBufVA(0));
            rWM.BuffFD[0] = baseWorkingBuf->getFD(0);
        } else {
            ainrLogD("No need to allocate working buffer");
        }

        // prepare Input
        NRUTILS_BUFFER_STRUCT &rInput = rProcIn.rInput;
        rInput.Size = rUnpackBaseIn.i4Stride*rUnpackBaseIn.i4Height;
        rInput.BufferNum = 1;
        rInput.pBuff[0]  = reinterpret_cast<void*>(imgoBase->getBufVA(0));
        rInput.BuffFD[0] = imgoBase->getFD(0);

        // Because we use jobQue to do unpack buffer allocation
        // and unpack golden frame task works sequentially after buffer
        // allocation. No need to protect "mv_unpackRaws[0] and m_noiseRaws".

        // prepare output
        rProcOut.rOutput.Size = rUnpackBaseIn.i4Width*rUnpackBaseIn.i4Height*sizeof(short);

        rProcOut.rOutput.BufferNum = 1;
        rProcOut.rOutput.pBuff[0]  = reinterpret_cast<void*>(mv_unpackRaws[0]->getBufVA(0));
        rProcOut.rOutput.BuffFD[0] = mv_unpackRaws[0]->getFD(0);

        // process
        pNRUtilsDrv->NRUtilsFeatureCtrl(NRUTILS_FTCTRL_PROC_UNPACK_BAYER, &rProcIn, &rProcOut);

        if(m_DumpWorking & AinrDumpWorking) {
            bufferDumpBlob(m_vDataPackage[0].halMeta, mv_unpackRaws[0].get(), "BaseFrameUnpack" , m_mainPathProfile);
        }

        // Finish base frame handling. We release buffer
        // But only CPU version has working buffer
        ImageBufferUtils::getInstance().deallocBuffer(baseWorkingBuf.get());

        {
            std::lock_guard<std::mutex> _lk(m_upBaseMx);
            mb_upBaseDone = true;
            ainrLogD("Unpack base done");
            m_upBaseCond.notify_one();
        }
    };

    mMinorJobQueue->addJob( _jobBase );

    if ( CC_UNLIKELY(doMatching() != AinrErr_Ok) ) {
        ainrLogW("[Matching fail!] Do error handling by unpack golden pack10");
        CHECK_AINR( releaseFeFmBuffer(), "Release fefm buffer fail" );
        CHECK_AINR( releaseGridBuffer(), "Release grid buffer fail" );
        CHECK_AINR( releaseWarpingBuffer(), "Release warping buffer fail" );
        m_algoType = AINR_SINGLE;
        unpackRawBuffer(m_vDataPackage[0].imgoBuffer, m_outBuffer);
        goto lbExit;
    }

    if ( CC_UNLIKELY(doNrCore() != AinrErr_Ok) ) {
        ainrLogW("[NR core fail!] Do error handling by unpack golden pack10");
        m_algoType = AINR_SINGLE;
        unpackRawBuffer(m_vDataPackage[0].imgoBuffer, m_outBuffer);
        goto lbExit;
    }

lbExit:
    // Release unpack raw buffers if needed
    CHECK_AINR( releaseUnpackBuffer(), "Release unpack buffer fail");

    // Setup final raw bayer format
    setUpBayerOrder(m_outBuffer);

    // Set up debug exif
    setDebugExif(m_vDataPackage[0].outHalMeta, m_algoType);

    if (NSCam::NSThermalInfo::setThermalMode(0) < 0) {
        ainrLogW("Write Thermal Hinet Falied");
    }

    return err;
}

enum AinrErr AinrCore::doCancel()
{
    return AinrErr_Ok;
}

enum AinrErr AinrCore::setNvramProvider(const std::shared_ptr<IAinrNvram> &nvramProvider)
{
    enum AinrErr err = AinrErr_Ok;
    std::lock_guard<decltype(m_nvMx)> _l(m_nvMx);

    m_NvramPtr = nvramProvider;

lbExit:
    return err;
}

enum AinrErr AinrCore::addInput(const std::vector<AinrPipelinePack>& inputPack)
{
    std::lock_guard<std::mutex> _l(m_DataLock);

    m_vDataPackage = inputPack;
    return AinrErr_Ok;
}

enum AinrErr AinrCore::addOutput(IImageBuffer *outBuffer)
{
    std::lock_guard<std::mutex> _l(m_DataLock);
    m_outBuffer = outBuffer;
    return AinrErr_Ok;
}

void AinrCore::prepareJobs()
{
    // Homography job

    IMetadata* halMeta = nullptr;
    halMeta = m_vDataPackage[0].halMeta;
    MRect rrzoCrop;

    if(CC_LIKELY(halMeta)) {
       NSCam::IMetadata::getEntry<MRect>(halMeta, MTK_P1NODE_SENSOR_CROP_REGION, rrzoCrop);
       ainrLogD("Ainr crop(%d, %d, %d, %d)", rrzoCrop.p.x, rrzoCrop.p.y, rrzoCrop.s.w, rrzoCrop.s.h);

    } else {
       ainrLogW("Golden halMeta is null!!!");
    }

    if(m_autoTest) {
        rrzoCrop.p.x = ::property_get_int32("vendor.ainr.cropx", 0);
        rrzoCrop.p.y = ::property_get_int32("vendor.ainr.cropy", 0);
    }

    m_rrzCrop = rrzoCrop;

    // Method prepare
    // Homography gridMap allocation
    m_vGridX.resize(m_captureNum);
    m_vGridY.resize(m_captureNum);

    m_homoMethod = [this](int index) -> enum AinrErr {
        CAM_ULOGM_TAGLIFE("Homogrpahy");
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
            }
        );

        if(CC_UNLIKELY(pHGDrv == nullptr))
        {
            ainrLogF(" Error: pHGDrv createInstance() fail");
            return AinrErr_UnexpectedError;
        }
        HG_GET_PROC_INFO_IN_STRUCT  rGetProcInfIn; // only record "i4MaxFP"
        HG_GET_PROC_INFO_OUT_STRUCT rGetProcInfOut; // only record "i4ReqWMSize"
        HG_SET_PROC_INFO_IN_STRUCT  rSetProcInfIn;
        HG_PROC1_IN_STRUCT          rProc1In; // Record FEO/FMO and grid??
        HG_PROC1_OUT_STRUCT         rProc1Out;
        HG_VerInfo                  rHgVer;

        pHGDrv->HGInit(NULL, NULL);
        pHGDrv->HGFeatureCtrl(HG_FTCTRL_GET_VERSION , NULL, &rHgVer);

        // TODO: Need to confirm
        MSize feoSize = m_vFeo[0]->getImgSize();
        rGetProcInfIn.i4MaxFP = feoSize.w * feoSize.h /40;
        pHGDrv->HGFeatureCtrl(HG_FTCTRL_GET_PROC_INFO , &rGetProcInfIn, &rGetProcInfOut);
        ainrLogD("Homography working memory size %d", rGetProcInfOut.i4ReqWMSize);

        // Set up size
        auto imgo = m_vDataPackage[0].imgoBuffer;
        auto rrzo = m_vDataPackage[0].rrzoBuffer;
        MSize imgoSize = imgo->getImgSize();
        MSize rrzoSize = rrzo->getImgSize();
        rProc1In.i4SrcWidth = imgoSize.w;
        rProc1In.i4SrcHeight = imgoSize.h;
        rProc1In.i4RzWidth = rrzoSize.w;
        rProc1In.i4RzHeight = rrzoSize.h;
        rProc1In.i4WCP = m_rrzCrop.p.x;//0;
        rProc1In.i4HCP = m_rrzCrop.p.y;//0;
        rProc1In.i4Idx = 0;
        rProc1In.u4Rand = 0;
        rProc1In.MapWidth = 582;
        rProc1In.MapHeight = 437;
        rProc1Out.MapAllocSize = 1017336;

        // Working buffer allocation
        auto wbSize = rGetProcInfOut.i4ReqWMSize;
        auto workingBuffer = std::unique_ptr<char[]>(new char[wbSize]{0});

        auto gridSize = rProc1Out.MapAllocSize;

        auto ret = ImageBufferUtils::getInstance().allocBuffer(
            m_vGridX[index], gridSize, 1, eImgFmt_STA_BYTE);
        if(CC_UNLIKELY(ret == MFALSE)) {
            ainrLogF("allocate gridX error!!!");
            return AinrErr_UnexpectedError;
        }
        ret = ImageBufferUtils::getInstance().allocBuffer(
            m_vGridY[index], gridSize, 1, eImgFmt_STA_BYTE);
        if(CC_UNLIKELY(ret == MFALSE)) {
            ainrLogF("allocate gridY error!!!");
            return AinrErr_UnexpectedError;
        }


        rProc1In.pi2BaseFe = reinterpret_cast<MINT16*>(m_vFeo[0]->getBufVA(0));
        rProc1In.pi2NewFe  = reinterpret_cast<MINT16*>(m_vFeo[index]->getBufVA(0));
        rProc1In.pi2BaseFm = reinterpret_cast<MINT16*>(m_vFmoBase[index]->getBufVA(0));
        rProc1In.pi2NewFm  = reinterpret_cast<MINT16*>(m_vFmoRef[index]->getBufVA(0));

        // Set homography proc info. Set upt working buffer?
        rSetProcInfIn.pvWM = reinterpret_cast<void*>(workingBuffer.get());
        rSetProcInfIn.i4WMSize = rGetProcInfOut.i4ReqWMSize;
        pHGDrv->HGFeatureCtrl(HG_FTCTRL_SET_PROC_INFO , &rSetProcInfIn, NULL);

        //Set up homography in data. FEOs and FMOs
        // Allocate genGrid buffer
        rProc1Out.pi4MapX = reinterpret_cast<MINT32*>(m_vGridX[index]->getBufVA(0));
        rProc1Out.pi4MapY = reinterpret_cast<MINT32*>(m_vGridY[index]->getBufVA(0));

        // Start homography
        pHGDrv->HGMain(HG_PROC1, &rProc1In, &rProc1Out);

        m_vGridX[index]->syncCache(eCACHECTRL_INVALID);
        m_vGridY[index]->syncCache(eCACHECTRL_INVALID);

        if(m_DumpWorking & AinrDumpWorking) {
            std::string gridXStr = "GridX" + std::to_string(index);
            std::string gridYStr = "GridY" + std::to_string(index);

            bufferDumpBlob(m_vDataPackage[index].halMeta, m_vGridX[index].get(), gridXStr.c_str() , m_fefmIspProfile);
            bufferDumpBlob(m_vDataPackage[index].halMeta, m_vGridY[index].get(), gridYStr.c_str() , m_fefmIspProfile);
        }

        // Early release reference feo/fmo buffers if no dump buffers needed
        if(m_DumpWorking == AinrDumpClose) {
            ImageBufferUtils::getInstance().deallocBuffer(m_vFeo[index].get());
            ImageBufferUtils::getInstance().deallocBuffer(m_vFmoBase[index].get());
            ImageBufferUtils::getInstance().deallocBuffer(m_vFmoRef[index].get());

            m_vFeo[index]     = nullptr;
            m_vFmoBase[index] = nullptr;
            m_vFmoRef[index]  = nullptr;
        }
        return AinrErr_Ok;
    };


    m_warpingMethod = [this](int index) -> enum AinrErr {
        // Wait Warping and Unpack buffer
        {
            // Because we use conditional wait which changes mutex
            // state need to use unique_lock
            std::unique_lock<std::mutex> _lk(m_AllocUpMx);
            if(!mb_AllocUpDone) {
                ainrLogD("Wait warping and unpack buffer allocation....");
                m_AllocUpCond.wait(_lk);
            }
        }

        IAinrWarping::WarpingPackage infoPack;
        infoPack.buffer = m_vDataPackage[index].imgoBuffer;
        infoPack.gridX  = m_vGridX[index].get();
        infoPack.gridY  = m_vGridY[index].get();
        infoPack.outRg  = m_vRg[index].get();
        infoPack.outGb  = m_vGb[index].get();
        auto ret = m_warping->doWarping(infoPack);

        if(m_DumpWorking & AinrDumpPartial) {
            std::string warpingRgStr = "WarpingRg" + std::to_string(index);
            std::string warpingGbStr = "WarpingGb" + std::to_string(index);

            bufferDumpRaw(m_vDataPackage[index].halMeta, m_vRg[index].get(), TuningUtils::RAW_PORT_IMGO, warpingRgStr.c_str(), m_mainPathProfile);
            bufferDumpRaw(m_vDataPackage[index].halMeta, m_vGb[index].get(), TuningUtils::RAW_PORT_IMGO, warpingGbStr.c_str(), m_mainPathProfile);
        }

        // Release GridMap
        ImageBufferUtils::getInstance().deallocBuffer(m_vGridX[index].get());
        ImageBufferUtils::getInstance().deallocBuffer(m_vGridY[index].get());
        m_vGridX[index] = nullptr;
        m_vGridY[index] = nullptr;

        return ret;
    };

    m_unpackMethod = [this](int index) -> enum AinrErr {
        CAM_ULOGM_TAGLIFE("UnpackUV");

        if(CC_UNLIKELY(m_vRg[index].get() == nullptr)
                || CC_UNLIKELY(m_vGb[index].get() == nullptr)) {
            ainrLogF("Warping packraw is nullptr");
            return AinrErr_UnexpectedError;
        }

        // Utils allocate and initial
        NRUTILS_VerInfo rUtilsVer;
        MINT32 outBits = 12;
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
            }
        );

        if(CC_UNLIKELY(pNRUtilsDrv == nullptr))
        {
            ainrLogF(" Error: MTKNRUtils createInstance() fail");
            return AinrErr_UnexpectedError;
        }

        pNRUtilsDrv->NRUtilsFeatureCtrl(NRUTILS_FTCTRL_GET_VERSION , NULL, &rUtilsVer);

        NRUTILS_CONFIG_UNPACK_BAYER_UV_OUT_STRUCT rUnpackUVOut;
        NRUTILS_CONFIG_UNPACK_BAYER_UV_IN_STRUCT rUnpackUVIn;
        NRUTILS_UNPACK_BAYER_UV_IN_STRUCT rProcIn;
        NRUTILS_UNPACK_BAYER_UV_OUT_STRUCT rProcOut;

        size_t warpingStride = m_vRg[index]->getBufStridesInBytes(0);
        rUnpackUVIn.u4CoreNum = 8;
        rUnpackUVIn.i4Width   = m_imgoWidth;
        rUnpackUVIn.i4Height  = m_imgoHeight;
        rUnpackUVIn.i4Stride  = warpingStride;
        rUnpackUVIn.i4OutBits = outBits;
        // rUnpackUVIn.i4Mode = 1; // Used for CPU version of unpack

        // OB and OB ofst
        {
            for(int i = 0; i < 4; i++) {
                rUnpackUVIn.i4ObOffst[i] = 256;
                rUnpackUVIn.i4DgnGain[i] = m_dgnGain;
            }
        }

        // config
        pNRUtilsDrv->NRUtilsFeatureCtrl(NRUTILS_FTCTRL_CONFIG_UNPACK_BAYER_UV, &rUnpackUVIn, &rUnpackUVOut);

        // Buffers
        sp<IImageBuffer> refWorkingBuf = nullptr;
        sp<IImageBuffer> outRgGbBuffer = nullptr;

        // prepare working buffer
        if(rUnpackUVOut.i4WMSize) {
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
        //

        // Check online device tuning
        if(m_onlineTuning == 2 && !m_autoTest) {
            FileReadRule rule;
            const char* bufStr = "MtkCam/IBUS";
            std::string warpingRgStr = "WarpingRg" + std::to_string(index);
            std::string warpingGbStr = "WarpingGb" + std::to_string(index);

            rule.getFile_WarpingRaw(index, "EIspProfile_AINR_Main", m_vRg[index].get(), warpingRgStr.c_str(), bufStr, m_sensorId);
            rule.getFile_WarpingRaw(index, "EIspProfile_AINR_Main", m_vGb[index].get(), warpingGbStr.c_str(), bufStr, m_sensorId);
        }

        // Input1 for RG buffer
        // TODO: I think we need to check size is from same source to avoid information not sync
        unsigned int rgRbSize = rUnpackUVIn.i4Stride*rUnpackUVIn.i4Height/2;
        rProcIn.rInput1.Size = rgRbSize;
        rProcIn.rInput1.BufferNum = 1;
        rProcIn.rInput1.pBuff[0]  = reinterpret_cast<void*>(m_vRg[index]->getBufVA(0));
        rProcIn.rInput1.BuffFD[0] = m_vRg[index]->getFD(0);

        // Input2 for GB buffer
        rProcIn.rInput2.Size = rgRbSize;
        rProcIn.rInput2.BufferNum = 1;
        rProcIn.rInput2.pBuff[0] = reinterpret_cast<void*>(m_vGb[index]->getBufVA(0));
        rProcIn.rInput2.BuffFD[0] = m_vGb[index]->getFD(0);

        // create output buffer
        rProcOut.rOutput.Size = rUnpackUVIn.i4Width*rUnpackUVIn.i4Height*sizeof(short);
        rProcOut.rOutput.BufferNum = 1;
        rProcOut.rOutput.pBuff[0]  = reinterpret_cast<void*>(mv_unpackRaws[index]->getBufVA(0));
        rProcOut.rOutput.BuffFD[0] = mv_unpackRaws[index]->getFD(0);

        // process
        pNRUtilsDrv->NRUtilsFeatureCtrl(NRUTILS_FTCTRL_PROC_UNPACK_BAYER_UV, &rProcIn, &rProcOut);
        //
        ImageBufferUtils::getInstance().deallocBuffer(refWorkingBuf.get());

        //
        if(m_DumpWorking & AinrDumpWorking) {
            std::string uvStr = "RgRbUnpack" + std::to_string(index);
            bufferDumpBlob(m_vDataPackage[index].halMeta, mv_unpackRaws[index].get(), uvStr.c_str(), m_mainPathProfile);
        }

        // Release wapring buffers
        ImageBufferUtils::getInstance().deallocBuffer(m_vRg[index].get());
        ImageBufferUtils::getInstance().deallocBuffer(m_vGb[index].get());
        m_vRg[index] = nullptr;
        m_vGb[index] = nullptr;

        return AinrErr_Ok;
    };

    m_matchingMethod = [this](int index) {
        ::prctl(PR_SET_NAME, "AinrMatchingThread", 0, 0, 0);
        CHECK_AINR(m_homoMethod(index), "homography error");
        CHECK_AINR(m_warpingMethod(index), "warping error");
        CHECK_AINR(m_unpackMethod(index), "Unpack error");
        return;
    };

    return;
}

enum AinrErr AinrCore::doMatching()
{
    AinrErr err = AinrErr_Ok;
    CAM_ULOGM_APILIFE();

    // Wait FEFM buffers
    {
        // Because we use conditional wait which changes mutex
        // state need to use unique_lock
        CAM_ULOGM_TAGLIFE("wait_fefm_wk_buf");
        std::unique_lock<std::mutex> _lk(m_fefmMx);
        if(!mb_AllocfefmDone) {
            ainrLogD("Wait FEFM allocation....");
            m_fefmCond.wait(_lk);
        }
    }

    // Check fefm working buffers
    {
        for(auto && buf : m_vFeo) {
            if (CC_UNLIKELY( buf.get() == nullptr )) {
                ainrLogE("feo working buffers are nullptr");
                releaseFeFmBuffer();
                return AinrErr_UnexpectedError;
            }
        }

        for(auto && buf : m_vFmoBase) {
            if (CC_UNLIKELY( buf.get() == nullptr )) {
                ainrLogE("fmoBase working buffers are nullptr");
                releaseFeFmBuffer();
                return AinrErr_UnexpectedError;
            }
        }

        for(auto && buf : m_vFmoRef) {
            if (CC_UNLIKELY( buf.get() == nullptr )) {
                ainrLogE("fmoRef working buffers are nullptr");
                releaseFeFmBuffer();
                return AinrErr_UnexpectedError;
            }
        }
    }

    // Check capture number is sync with inputBuffer package
    if(CC_UNLIKELY( m_captureNum != static_cast<int>(m_vDataPackage.size())
        && m_vDataPackage.size() != 0) ) {
            ainrLogE("capture number(%d) is not sync with input buffer(%zu)", m_captureNum, m_vDataPackage.size());
            releaseFeFmBuffer();
            return AinrErr_UnexpectedError;
    }

    // Start to do FEFM
    if(CC_UNLIKELY(m_fefm.get() == nullptr)) {
        ainrLogF("Get fefm error");
        return AinrErr_UnexpectedError;
    }

    // Prepare Homography/Warping/Unpack jobs
    prepareJobs();

    {
        std::lock_guard<std::mutex> _l(m_DataLock);

        if(m_bittrueEn) {
            m_vRrzoYuv.resize(m_captureNum);
            // For yuv dump
            // Because it is bittrue flow. No need to take
            // care performance
            for(auto && buf : m_vRrzoYuv) {
                MBOOL ret = MTRUE;
                ret = ImageBufferUtils::getInstance().allocBuffer(
                    buf, m_rrzoWidth, m_rrzoHeight, eImgFmt_YV12);
                if(CC_UNLIKELY(ret == MFALSE)) {
                    ainrLogE("allocate rrzo YUV buffer error!!!");
                    return AinrErr_UnexpectedError;
                }
            }
        }

        for(int i = 0; i < m_captureNum; i++) {
            IAinrFeFm::DataPackage pack;

            pack.appMeta = m_vDataPackage[i].appMeta;
            pack.halMeta = m_vDataPackage[i].halMeta;
            pack.inBuf   = m_vDataPackage[i].rrzoBuffer;
            pack.outBuf  = m_vFeo[i].get();

            if(m_bittrueEn) {
                pack.outYuv = m_vRrzoYuv[i].get();
            }

            // Do re-install for bitrue
            if(m_autoTest) {
                IMetadata::setEntry<MINT32>(pack.halMeta, MTK_HAL_REQUEST_INDEX_BSS, i);
            }
            //

            m_fefm->buildFe(&pack);

            if(i != 0) {
                m_fefm->buildFm(m_vFeo[0].get(), m_vFeo[i].get(), m_vFmoRef[i].get(), i, false);
                m_fefm->buildFm(m_vFeo[i].get(), m_vFeo[0].get(), m_vFmoBase[i].get(), i, true);
            }
        }

        err = m_fefm->doFeFm();

        if(err == AinrErr_Ok && ( m_DumpWorking & AinrDumpWorking )) {
            // Dump feo
            for(int i = 0; i < m_captureNum; i++) {
                // Feo
                std::string feoStr = "feo" + std::to_string(i);
                bufferDumpBlob(m_vDataPackage[i].halMeta, m_vFeo[i].get(), feoStr.c_str() , m_fefmIspProfile);

                //IMG3O YUV
                if(m_bittrueEn) {
                    bufferDumpYuv(m_vDataPackage[i].halMeta,  m_vRrzoYuv[i].get(), YUV_PORT_IMG3O, "YuvRrz", m_fefmIspProfile);
                    // RRZO yuv is no used. We release it.
                    ImageBufferUtils::getInstance().deallocBuffer(m_vRrzoYuv[i].get());
                    m_vRrzoYuv[i] = nullptr;
                }
            }
            //
            for(int i = 1; i < m_captureNum; i++) {
                std::string fmoBaseStr = "fmoBase" + std::to_string(i);
                std::string fmoRefStr  = "fmoRef"  + std::to_string(i);

                bufferDumpBlob(m_vDataPackage[i].halMeta, m_vFmoBase[i].get(), fmoBaseStr.c_str() , m_fefmIspProfile);
                bufferDumpBlob(m_vDataPackage[i].halMeta, m_vFmoRef[i].get(),  fmoRefStr.c_str()  , m_fefmIspProfile);
            }
        }
    }

    // Start to wait previous stage (UV unpack done)
    {
        CAM_ULOGM_TAGLIFE("Wait UV upack done");
        // Wait all threads done
        for( auto &fut : m_jobs ) {
            if (CC_LIKELY( fut.valid() )) {
                try {
                    auto _status = fut.wait_for(std::chrono::seconds(5));
                    if (CC_UNLIKELY( _status != std::future_status::ready )) {
                        ainrLogE("wait future status is not ready");
                        return AinrErr_UnexpectedError;
                    }
                }
                catch (std::exception&) {
                    ainrLogE("matching thread::wait_for throws exception");
                    return AinrErr_UnexpectedError;
                }
            } else {
                ainrLogE("matching thread is not in the valid state error!!!");
                return AinrErr_UnexpectedError;
            }
        }
    }

    // Wait base unpack done
    {
        std::unique_lock<std::mutex> __lk(m_upBaseMx);
        if(!mb_upBaseDone) {
            ainrLogD("Wait Unpack base done....");
            m_upBaseCond.wait(__lk);
        } else {
            ainrLogD("No need to wait unpack base done");
        }
    }

    if(mbNeedTileMode && !m_bIsLowMem) {
        nextCapture();
    }

    // Ensure all buffers are released
    CHECK_AINR( releaseFeFmBuffer(), "Release fefm buffer fail" );
    CHECK_AINR( releaseGridBuffer(), "Release grid buffer fail" );
    CHECK_AINR( releaseWarpingBuffer(), "Release warping buffer fail" );
lbExit:
    return err;
}

enum AinrErr AinrCore::setUpTuning(NRCORE_PROC1_IN_STRUCT *unPackBayerCfg)
{
    AinrErr err = AinrErr_Ok;

    if(CC_UNLIKELY(unPackBayerCfg == nullptr)) {
        ainrLogE("unPackBayerCfg is null");
        return AinrErr_UnexpectedError;
    }

    MUINT32 defaultChi = 500;
    MUINT32 defaultStd = 5000;
    MUINT32 chi0 = defaultChi;
    MUINT32 chi1 = defaultChi;
    MUINT32 chi2 = defaultChi;
    MUINT32 chi3 = defaultChi;

    MUINT32 std0 = defaultStd;
    MUINT32 std1 = defaultStd;
    MUINT32 std2 = defaultStd;
    MUINT32 std3 = defaultStd;

    MUINT32 lambda = 0;
    MUINT32 rsv[7]={0};

    if(CC_UNLIKELY(m_NvramPtr.get() == nullptr)) {
        ainrLogW("Nvram is null, we use default");
    } else {
        // Dump
        size_t bufferSize = 0;
        char *pMutableChunk = const_cast<char*>(m_NvramPtr->getChunk(&bufferSize));
        FEATURE_NVRAM_AINR_T *data = reinterpret_cast<FEATURE_NVRAM_AINR_T*>(pMutableChunk);

        if(CC_LIKELY(data)) {
            chi0 = data->chi_R;
            chi1 = data->chi_GR;
            chi2 = data->chi_GB;
            chi3 = data->chi_B;
            std0 = data->std_R;
            std1 = data->std_GR;
            std2 = data->std_GB;
            std3 = data->std_B;
            //
            lambda  = data->Lambda;
            rsv[0] = data->rsv1;
            rsv[1] = data->rsv2;
            rsv[2] = data->rsv3;
            rsv[3] = data->rsv4;
            rsv[4] = data->rsv5;
            rsv[5] = data->rsv6;
            rsv[6] = data->rsv7;

            mCoreTuning.blend_R  = data->AINR_blend_R;
            mCoreTuning.blend_Gr = data->AINR_blend_GR;
            mCoreTuning.blend_Gb = data->AINR_blend_GB;
            mCoreTuning.blend_B  = data->AINR_blend_B;
        }

        // On device tuning
        if((m_onlineTuning == 2) || m_autoTest) {
            data->chi_R  = chi0 = ::property_get_int32("vendor.ainr.chi0", 500);
            data->chi_GR = chi1 = ::property_get_int32("vendor.ainr.chi1", 500);
            data->chi_GB = chi2 = ::property_get_int32("vendor.ainr.chi2", 500);
            data->chi_B  = chi3 = ::property_get_int32("vendor.ainr.chi3", 500);

            data->std_R  = std0 = ::property_get_int32("vendor.ainr.std0", 5000);
            data->std_GR = std1 = ::property_get_int32("vendor.ainr.std1", 5000);
            data->std_GB = std2 = ::property_get_int32("vendor.ainr.std2", 5000);
            data->std_B  = std3 = ::property_get_int32("vendor.ainr.std3", 5000);
            //
            data->Lambda = lambda = ::property_get_int32("vendor.ainr.lambda", 0);
            data->rsv1   = rsv[0] = ::property_get_int32("vendor.ainr.rsv1", 0);
            data->rsv2   = rsv[1] = ::property_get_int32("vendor.ainr.rsv2", 0);
            data->rsv3   = rsv[2] = ::property_get_int32("vendor.ainr.rsv3", 0);
            data->rsv4   = rsv[3] = ::property_get_int32("vendor.ainr.rsv4", 0);
            data->rsv5   = rsv[4] = ::property_get_int32("vendor.ainr.rsv5", 0);
            data->rsv6   = rsv[5] = ::property_get_int32("vendor.ainr.rsv6", 0);
            data->rsv7   = rsv[6] = ::property_get_int32("vendor.ainr.rsv7", 0);
            //
            mCoreTuning.blend_R  = ::property_get_int32("vendor.ainr.blend0", 1000000);
            mCoreTuning.blend_Gr = ::property_get_int32("vendor.ainr.blend1", 1000000);
            mCoreTuning.blend_Gb = ::property_get_int32("vendor.ainr.blend2", 1000000);
            mCoreTuning.blend_B  = ::property_get_int32("vendor.ainr.blend3", 1000000);
        }

        if((m_DumpWorking & AinrDumpPartial) && data) {
            dumpAinrTuning(data);
        }
    }

    unPackBayerCfg->i4ChParam[0] = chi0;
    unPackBayerCfg->i4ChParam[1] = std0;
    unPackBayerCfg->i4ChParam[2] = lambda;
    unPackBayerCfg->i4ChParam[3] = rsv[0];
    unPackBayerCfg->i4ChParam[4] = rsv[1];
    unPackBayerCfg->i4ChParam[5] = rsv[2];
    unPackBayerCfg->i4ChParam[6] = rsv[3];
    unPackBayerCfg->i4ChParam[7] = rsv[4];

   std::string strlog;

    for (int i = 0; i < 8; i++)
        strlog += " i4chParma[" + std::to_string(i) + "]=" + std::to_string(unPackBayerCfg->i4ChParam[i]);

    strlog += " Alpha blend:" + std::to_string(mCoreTuning.blend_R) + ", " + std::to_string(mCoreTuning.blend_Gr)
               + "," + std::to_string(mCoreTuning.blend_Gb) + ", " + std::to_string(mCoreTuning.blend_B);

    ainrLogD("Tuning data:%s", strlog.c_str());

    return err;
}

enum AinrErr AinrCore::setUpTileModeTuning(NRCORE_PROC2_IN_STRUCT *unPackBayerCfg)
{
    AinrErr err = AinrErr_Ok;

    if(CC_UNLIKELY(unPackBayerCfg == nullptr)) {
        ainrLogE("unPackBayerCfg is null");
        return AinrErr_UnexpectedError;
    }

    MUINT32 defaultChi = 500;
    MUINT32 defaultStd = 5000;
    MUINT32 chi0 = defaultChi;
    MUINT32 chi1 = defaultChi;
    MUINT32 chi2 = defaultChi;
    MUINT32 chi3 = defaultChi;

    MUINT32 std0 = defaultStd;
    MUINT32 std1 = defaultStd;
    MUINT32 std2 = defaultStd;
    MUINT32 std3 = defaultStd;

    MUINT32 lambda = 0;
    MUINT32 rsv[7]={0};

    if(CC_UNLIKELY(m_NvramPtr.get() == nullptr)) {
        ainrLogW("Nvram is null, we use default");
    } else {
        // Dump
        size_t bufferSize = 0;
        char *pMutableChunk = const_cast<char*>(m_NvramPtr->getChunk(&bufferSize));
        FEATURE_NVRAM_AINR_T *data = reinterpret_cast<FEATURE_NVRAM_AINR_T*>(pMutableChunk);

        if(CC_LIKELY(data)) {
            chi0 = data->chi_R;
            chi1 = data->chi_GR;
            chi2 = data->chi_GB;
            chi3 = data->chi_B;
            std0 = data->std_R;
            std1 = data->std_GR;
            std2 = data->std_GB;
            std3 = data->std_B;
            //
            lambda  = data->Lambda;
            rsv[0] = data->rsv1;
            rsv[1] = data->rsv2;
            rsv[2] = data->rsv3;
            rsv[3] = data->rsv4;
            rsv[4] = data->rsv5;
            rsv[5] = data->rsv6;
            rsv[6] = data->rsv7;

            mCoreTuning.blend_R  = data->AINR_blend_R;
            mCoreTuning.blend_Gr = data->AINR_blend_GR;
            mCoreTuning.blend_Gb = data->AINR_blend_GB;
            mCoreTuning.blend_B  = data->AINR_blend_B;
        }

        // On device tuning
        if((m_onlineTuning == 2) || m_autoTest) {
            data->chi_R  = chi0 = ::property_get_int32("vendor.ainr.chi0", 500);
            data->chi_GR = chi1 = ::property_get_int32("vendor.ainr.chi1", 500);
            data->chi_GB = chi2 = ::property_get_int32("vendor.ainr.chi2", 500);
            data->chi_B  = chi3 = ::property_get_int32("vendor.ainr.chi3", 500);

            data->std_R  = std0 = ::property_get_int32("vendor.ainr.std0", 5000);
            data->std_GR = std1 = ::property_get_int32("vendor.ainr.std1", 5000);
            data->std_GB = std2 = ::property_get_int32("vendor.ainr.std2", 5000);
            data->std_B  = std3 = ::property_get_int32("vendor.ainr.std3", 5000);
            //
            data->Lambda = lambda = ::property_get_int32("vendor.ainr.lambda", 0);
            data->rsv1   = rsv[0] = ::property_get_int32("vendor.ainr.rsv1", 0);
            data->rsv2   = rsv[1] = ::property_get_int32("vendor.ainr.rsv2", 0);
            data->rsv3   = rsv[2] = ::property_get_int32("vendor.ainr.rsv3", 0);
            data->rsv4   = rsv[3] = ::property_get_int32("vendor.ainr.rsv4", 0);
            data->rsv5   = rsv[4] = ::property_get_int32("vendor.ainr.rsv5", 0);
            data->rsv6   = rsv[5] = ::property_get_int32("vendor.ainr.rsv6", 0);
            data->rsv7   = rsv[6] = ::property_get_int32("vendor.ainr.rsv7", 0);
            //
            mCoreTuning.blend_R  = ::property_get_int32("vendor.ainr.blend0", 1000000);
            mCoreTuning.blend_Gr = ::property_get_int32("vendor.ainr.blend1", 1000000);
            mCoreTuning.blend_Gb = ::property_get_int32("vendor.ainr.blend2", 1000000);
            mCoreTuning.blend_B  = ::property_get_int32("vendor.ainr.blend3", 1000000);
        }

        if((m_DumpWorking & AinrDumpPartial) && data) {
            dumpAinrTuning(data);
        }
    }

    unPackBayerCfg->i4ChParam[0] = chi0;
    unPackBayerCfg->i4ChParam[1] = std0;
    unPackBayerCfg->i4ChParam[2] = lambda;
    unPackBayerCfg->i4ChParam[3] = rsv[0];
    unPackBayerCfg->i4ChParam[4] = rsv[1];
    unPackBayerCfg->i4ChParam[5] = rsv[2];
    unPackBayerCfg->i4ChParam[6] = rsv[3];
    unPackBayerCfg->i4ChParam[7] = rsv[4];

    std::string strlog;

    for (int i = 0; i < 8; i++)
        strlog += " i4chParma[" + std::to_string(i) + "]=" + std::to_string(unPackBayerCfg->i4ChParam[i]);

    strlog += " Alpha blend:" + std::to_string(mCoreTuning.blend_R) + ", " + std::to_string(mCoreTuning.blend_Gr)
               + "," + std::to_string(mCoreTuning.blend_Gb) + ", " + std::to_string(mCoreTuning.blend_B);

    ainrLogD("Tuning data:%s", strlog.c_str());

    return err;
}

void AinrCore::dumpAinrTuning(FEATURE_NVRAM_AINR_T *data)
{
    //dump binary
    auto dump2Binary = [this](MINT32 uniqueKey, MINT32 requestNum, MINT32 frameNum, const char* buf, size_t size, string fn) -> bool {
        char filepath[256] = {0};
        snprintf(filepath, sizeof(filepath)-1, "%s/%09d-%04d-%04d-""%s", AINR_DUMP_PATH, uniqueKey, requestNum, frameNum, fn.c_str());
        std::ofstream ofs (filepath, std::ofstream::binary);
        if (!ofs.is_open()) {
            ainrLogW("dump2Binary: open file(%s) fail", filepath);
            return false;
        }
        //ofs.write(buf, size);
        char *pMutableChunk = const_cast<char*>(buf);
        FEATURE_NVRAM_AINR_T *p = reinterpret_cast<FEATURE_NVRAM_AINR_T*>(pMutableChunk);

        ofs << "RRzo: crop_x:" << m_rrzCrop.p.x << ", crop_y:" << m_rrzCrop.p.y << endl;
        ofs << "Chi[" << 0 <<"]="<<p->chi_R<<endl;
        ofs << "Std[" << 0 <<"]="<<p->std_R<<endl;

        ofs << "Chi[" << 1 <<"]="<<p->chi_GR<<endl;
        ofs << "Std[" << 1 <<"]="<<p->std_GR<<endl;

        ofs << "Chi[" << 2 <<"]="<<p->chi_GB<<endl;
        ofs << "Std[" << 2 <<"]="<<p->std_GB<<endl;

        ofs << "Chi[" << 3 <<"]="<<p->chi_B<<endl;
        ofs << "Std[" << 3 <<"]="<<p->std_B<<endl;

        ofs << "u4Alpha[" << 0 <<"]="<<mCoreTuning.blend_R<<endl;
        ofs << "u4Alpha[" << 1 <<"]="<<mCoreTuning.blend_Gr<<endl;
        ofs << "u4Alpha[" << 2 <<"]="<<mCoreTuning.blend_Gb<<endl;
        ofs << "u4Alpha[" << 3 <<"]="<<mCoreTuning.blend_B<<endl;

        ofs << "DGN gain:" << m_dgnGain << endl;
        ofs << "OB ofst:" << m_obFst << endl;

        ofs << "Lambda=" << p->Lambda << endl;
        ofs << "rsv1="  << p->rsv1   << endl;
        ofs << "rsv2="  << p->rsv2   << endl;
        ofs << "rsv3="  << p->rsv3   << endl;
        ofs << "rsv4="  << p->rsv4   << endl;
        ofs << "rsv5="  << p->rsv5   << endl;
        ofs << "rsv6="  << p->rsv6   << endl;
        ofs << "rsv7="  << p->rsv7   << endl;

        ofs << "BSS order: ";
        for ( auto && i : mv_bssOrder) {
            ofs << i <<" ";
        }
        ofs << endl;

        ofs.close();
        return true;
    };

    dump2Binary(m_uniqueKey, m_requestNum, m_frameNum, (const char*)(data), sizeof(FEATURE_NVRAM_AINR_T), AINR_DUMP_TUNING_FILENAME);
}

enum AinrErr AinrCore::doNrCore()
{
    CAM_ULOGM_APILIFE();
    std::lock_guard<decltype(sNrCoreMx)> _gk(sNrCoreMx);
    AinrErr err = AinrErr_Ok;

    ::prctl(PR_SET_NAME, "AinrCore", 0, 0, 0);

    if (CC_UNLIKELY( m_outBuffer == nullptr )) {
        ainrLogE("outRgGbBuffer is null");
        return AinrErr_UnexpectedError;
    }

    // Choose algo type
    MUINT32 algoIdx = 0;
    if (m_bIsLowMem)
    {
        ainrLogD("Low memory we use down sample ");
        algoIdx = 1;
    }

    std::unique_ptr<MTKNRCore, std::function<void(MTKNRCore*)>> pNRCoreDrv = nullptr;
    pNRCoreDrv = decltype(pNRCoreDrv)( MTKNRCore::createInstance(DRV_NRCORE_OBJ_SW),
        [](MTKNRCore* nrCore) {
            CAM_ULOGM_TAGLIFE("NrCore destroy");
            if (CC_UNLIKELY( nrCore == nullptr )) {
                ainrLogW("NRCore is nullptr");
            } else {
                ainrLogD("NR core unint");
                nrCore->destroyInstance();
            }
        }
    );

    if (CC_UNLIKELY( pNRCoreDrv == nullptr ))
    {
        ainrLogE("pNRCoreDrv is null");
        return AinrErr_UnexpectedError;
    }

    {
        CAM_ULOGM_TAGLIFE("NR core init");

        NRCORE_INIT_IN_STRUCT rInitIn;
        // TODO: Need to query from Nvram
        rInitIn.u4CoreNum   = 8;
        rInitIn.u4Width     = m_imgoWidth;
        rInitIn.u4Height    = m_imgoHeight;
        rInitIn.u4AlgoIdx   = 0;
        rInitIn.u4FrameNum  = m_captureNum;
        rInitIn.u4TileMode  = mbNeedTileMode ? 1 : 0;
        rInitIn.u4AlgoIdx   = algoIdx;

        pNRCoreDrv->NRCoreInit(&rInitIn, NULL);
    }

    NRCORE_GET_PROC_INFO_OUT_STRUCT rGetProcInfoOut;
    NRCORE_SET_PROC_INFO_IN_STRUCT  rSetProcInfoIn;
    //
    NRCore_VerInfo rCoreVer;
    NRCORE_PROC1_IN_STRUCT rProc1In;
    NRCORE_PROC2_IN_STRUCT rProc2In;
    NRCORE_PROC1_OUT_STRUCT rProc1Out;
    NRCORE_PROC2_OUT_STRUCT rProc2Out;

    unsigned int alpha           = 1000000;//1000000;
    size_t       unPackSize      = 0;
    size_t       noiseBufferSize = 0;

    for(size_t i = 0; i < m_outBuffer->getPlaneCount(); i++) {
        unPackSize += m_outBuffer->getBufSizeInBytes(i);
    }

    if(mbNeedTileMode) {
        setUpTileModeTuning(&rProc2In);
        rProc2In.u4Alpha[0] = mCoreTuning.blend_R; // alpha = 0.5
        rProc2In.u4Alpha[1] = mCoreTuning.blend_Gr;
        rProc2In.u4Alpha[2] = mCoreTuning.blend_Gb;
        rProc2In.u4Alpha[3] = mCoreTuning.blend_B;
        rProc2In.u4ImgSize  = unPackSize;
    } else {
        setUpTuning(&rProc1In);
        rProc1In.u4Alpha[0] = mCoreTuning.blend_R; // alpha = 0.5
        rProc1In.u4Alpha[1] = mCoreTuning.blend_Gr;
        rProc1In.u4Alpha[2] = mCoreTuning.blend_Gb;
        rProc1In.u4Alpha[3] = mCoreTuning.blend_B;
        rProc1In.u4ImgSize  = unPackSize;
        rProc1In.pvCookie   = reinterpret_cast<void*>(this);
        rProc1In.fpStageCallback = [](DRVNRCoreStage_e stage, void *cookie) -> int {
            ainrLogD("Ainr pre-processing done cb");

            AinrCore* core = reinterpret_cast<AinrCore*>(cookie);

            if(core == nullptr) {
                ainrLogE("Ainr cookie is nullptr");
                return -1;
            }

            ainrLogD("Release UV unpack");
            for(auto i = 1; i < core->mv_unpackRaws.size(); i++) {
                ImageBufferUtils::getInstance().deallocBuffer(core->mv_unpackRaws[i].get());
                core->mv_unpackRaws[i] = nullptr;
            }
            core->nextCapture();

            return 0;
        };
    }

    // OB and OB ofst
    {
        for(int i = 0; i < 4; i++) {
           rProc2In.i4ObOffst[i] = rProc1In.i4ObOffst[i] = m_obFst;
           rProc2In.i4DgnGain[i] = rProc1In.i4DgnGain[i] = m_dgnGain;
        }
    }
    rProc1Out.u4OutSize = unPackSize;
    rProc2Out.u4OutSize = unPackSize;

    pNRCoreDrv->NRCoreFeatureCtrl(NRCORE_FTCTRL_GET_VERSION , NULL, &rCoreVer);
    pNRCoreDrv->NRCoreFeatureCtrl(NRCORE_FTCTRL_GET_PROC_INFO , NULL, &rGetProcInfoOut);

    // auto workBufSize   = rGetProcInfoOut.u4ReqWMSize;
    // auto workingBuffer = std::unique_ptr<char[]>(new char[workBufSize]{0});
    rSetProcInfoIn.u4Num = rGetProcInfoOut.u4Num;

    std::vector< sp<IImageBuffer> >        vWorkingIonBuf;

    // create ion working buffer
    for (int i = 0; i < rSetProcInfoIn.u4Num; i++)
    {
        rSetProcInfoIn.rBuf[i].u4Size = rGetProcInfoOut.rBuf[i].u4Size;
        rSetProcInfoIn.rBuf[i].bUseIon = rGetProcInfoOut.rBuf[i].bUseIon;
        ainrLogD("MDLA Working buffer[%d] size(%d)", i, rSetProcInfoIn.rBuf[i].u4Size);

        sp<IImageBuffer> working = nullptr;
        auto ret = ImageBufferUtils::getInstance().allocBuffer(
        working, rSetProcInfoIn.rBuf[i].u4Size, 1, eImgFmt_STA_BYTE);
        if(CC_UNLIKELY(ret == MFALSE)) {
            // Release previous mdla input buffer
            if(!vWorkingIonBuf.empty()) {
                for(auto && buf : vWorkingIonBuf) {
                    ImageBufferUtils::getInstance().deallocBuffer(buf.get());
                    buf = nullptr;
                }
                vWorkingIonBuf.clear();
            }
            ainrLogE("allocate mdla input error!!!");
            return AinrErr_UnexpectedError;
        }

        rSetProcInfoIn.rBuf[i].i4BuffFD = working->getFD(0);
        rSetProcInfoIn.rBuf[i].prBuff   = reinterpret_cast<void*>(working->getBufVA(0));
        vWorkingIonBuf.push_back(working);
    }
    //
    pNRCoreDrv->NRCoreFeatureCtrl(NRCORE_FTCTRL_SET_PROC_INFO, &rSetProcInfoIn, NULL);

    // N frame mv_unpackRaws
    for (int i = 0; i < m_captureNum; i++)
    {
        rProc1In.u2Img[i] = reinterpret_cast<MUINT16*>(mv_unpackRaws[i]->getBufVA(0));
        rProc2In.u2Img[i] = reinterpret_cast<MUINT16*>(mv_unpackRaws[i]->getBufVA(0));
    }

    rProc1Out.u2OutImg = reinterpret_cast<MUINT16*>(m_outBuffer->getBufVA(0));
    rProc2Out.u2OutImg = reinterpret_cast<MUINT16*>(m_outBuffer->getBufVA(0));

    if(mbNeedTileMode) {
        ainrLogD("Execute NR core in tile mode");
        pNRCoreDrv->NRCoreMain(NRCORE_PROC2, &rProc2In, &rProc2Out);
    } else {
        ainrLogD("Execute NR core in frame mode");
        pNRCoreDrv->NRCoreMain(NRCORE_PROC1, &rProc1In, &rProc1Out);
    }

    if(m_DumpWorking & AinrDumpPartial) {
        bufferDumpRaw(m_vDataPackage[0].halMeta, m_outBuffer, TuningUtils::RAW_PORT_IMGO, "RawOutput", m_mainPathProfile);
    }

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

enum AinrErr AinrCore::releaseFeFmBuffer()
{
    // Because deallocBuffer check buffer pointer
    // no need to check again here
    // Release feoBase feoRef

    // Release feos
    ainrLogD("release feo buffers");
    for(auto && buf : m_vFeo) {
        ImageBufferUtils::getInstance().deallocBuffer(buf.get());
        buf = nullptr;
    }
    m_vFeo.clear();

    // Release fmoBase
    for(auto && buf : m_vFmoBase) {
        ImageBufferUtils::getInstance().deallocBuffer(buf.get());
        buf = nullptr;
    }
    m_vFmoBase.clear();

    // Release fmoRef
    for(auto && buf : m_vFmoRef) {
        ImageBufferUtils::getInstance().deallocBuffer(buf.get());
        buf = nullptr;
    }
    m_vFmoRef.clear();

    return AinrErr_Ok;
}

enum AinrErr AinrCore::releaseWarpingBuffer()
{
    for(int i = 1; i < m_captureNum; i++) {
        ImageBufferUtils::getInstance().deallocBuffer(m_vRg[i].get());
        ImageBufferUtils::getInstance().deallocBuffer(m_vGb[i].get());
        m_vRg[i] = nullptr;
        m_vGb[i] = nullptr;
    }

    m_vRg.clear();
    m_vGb.clear();

    return AinrErr_Ok;
}

enum AinrErr AinrCore::releaseGridBuffer()
{
    for(int i = 1; i < m_captureNum; i++) {
        ImageBufferUtils::getInstance().deallocBuffer(m_vGridX[i].get());
        ImageBufferUtils::getInstance().deallocBuffer(m_vGridY[i].get());
        m_vGridX[i] = nullptr;
        m_vGridY[i] = nullptr;
    }

    m_vGridX.clear();
    m_vGridY.clear();

    return AinrErr_Ok;
}

enum AinrErr AinrCore::releaseUnpackBuffer()
{
    // Release unpack buffers mv_unpackRaws
    for(auto && buf : mv_unpackRaws) {
        ImageBufferUtils::getInstance().deallocBuffer(buf.get());
        buf = nullptr;
    }
    mv_unpackRaws.clear();

    return AinrErr_Ok;
}

void AinrCore::bufferDumpBlob(IMetadata *halMeta, IImageBuffer* buff, const char *pUserString, MUINT8 ispProfile)
{
    // dump input buffer
    char                      fileName[512];
    FILE_DUMP_NAMING_HINT     dumpNamingHint;
    //

    if(!halMeta || !buff) {
        ainrLogE("HalMeta or buff is nullptr, dump fail");
        return;
    }

    // Extract hal metadata and fill up file name;
    extract(&dumpNamingHint, halMeta);
    // Extract buffer information and fill up file name;
    extract(&dumpNamingHint, buff);
    // Extract by sensor id
    extract_by_SensorOpenId(&dumpNamingHint, m_sensorId);
    // IspProfile
    dumpNamingHint.IspProfile = ispProfile; //EIspProfile_Capture;

    genFileName_RAW(fileName, sizeof(fileName), &dumpNamingHint, RAW_PORT_UNDEFINED, pUserString);
    buff->saveToFile(fileName);
    return;
}

void AinrCore::bufferDumpRaw(IMetadata *halMeta, IImageBuffer* buff, RAW_PORT type
    , const char *pUserString, MUINT8 ispProfile) {
    // dump input buffer
    char                      fileName[512];
    FILE_DUMP_NAMING_HINT     dumpNamingHint;
    //

    if(!halMeta || !buff) {
        ainrLogE("HalMeta or buff is nullptr, dump fail");
        return;
    }

    // Extract hal metadata and fill up file name;
    extract(&dumpNamingHint, halMeta);
    // Extract buffer information and fill up file name;
    extract(&dumpNamingHint, buff);
    // Extract by sensor id
    extract_by_SensorOpenId(&dumpNamingHint, m_sensorId);
    // IspProfile
    dumpNamingHint.IspProfile = ispProfile; //EIspProfile_Capture;

    genFileName_RAW(fileName, sizeof(fileName), &dumpNamingHint, type, pUserString);
    buff->saveToFile(fileName);
}

void AinrCore::bufferDumpYuv(IMetadata *halMeta, IImageBuffer* buff, YUV_PORT type
    , const char *pUserString, MUINT8 ispProfile) {
    // dump input buffer
    char                      fileName[512];
    FILE_DUMP_NAMING_HINT     dumpNamingHint;
    //

    if(!halMeta || !buff) {
        ainrLogE("HalMeta or buff is nullptr, dump fail");
        return;
    }

    // Extract hal metadata and fill up file name;
    extract(&dumpNamingHint, halMeta);
    // Extract buffer information and fill up file name;
    extract(&dumpNamingHint, buff);
    // Extract by sensor id
    extract_by_SensorOpenId(&dumpNamingHint, m_sensorId);
    // IspProfile
    dumpNamingHint.IspProfile = ispProfile; //EIspProfile_Capture;

    genFileName_YUV(fileName, sizeof(fileName), &dumpNamingHint, type, pUserString);
    buff->saveToFile(fileName);
}

void AinrCore::acquireJobQueue()
{
    static std::mutex       sResLock;
    static std::weak_ptr<
        NSCam::JobQueue<void()>
    >                       sResJobQ;

    // acquire resource from weak_ptr
    std::lock_guard<std::mutex> lk(sResLock);
    mMinorJobQueue = sResJobQ.lock();
    if (mMinorJobQueue.get() == nullptr) {
        mMinorJobQueue = std::shared_ptr< NSCam::JobQueue<void()> >(
                new NSCam::JobQueue<void()>("AinrCoreJob")
                );
        sResJobQ = mMinorJobQueue;
    }
    return;
}

void AinrCore::parsingMeta()
{
    // Parsing BSS
    {
        if(m_DumpWorking) {
            // Resize dubugMeta
            mvMetaPacks.resize(m_vDataPackage.size());
            //
            for(auto i = 0; i < m_vDataPackage.size(); i++) {
                // Because fast shot 2 shot may early release buffers including metadata
                // therefore we need to back up metadata before we use it.
                mvMetaPacks[i].appMeta = *(m_vDataPackage[i].appMeta);
                mvMetaPacks[i].halMeta = *(m_vDataPackage[i].halMeta);
                m_vDataPackage[i].appMeta = &mvMetaPacks[i].appMeta;
                m_vDataPackage[i].halMeta = &mvMetaPacks[i].halMeta;

                IMetadata *pHalMeta = m_vDataPackage[i].halMeta;

                NSCam::IMetadata::setEntry<MINT32>(pHalMeta, MTK_PIPELINE_FRAME_NUMBER, i);
                mv_bssOrder.push_back(i);

            }
        }
    }

    // Fetch OB offset from Golden metadata (RAWIspCamInfo)
    {
        IMetadata* halMeta = nullptr;
        halMeta = m_vDataPackage[0].halMeta;

        if(CC_LIKELY(halMeta)) {
            struct M {
                IMetadata::Memory val;
                MBOOL result;
                M() : result(MFALSE) {};
            } obOfst;

            //
            obOfst.val.resize(sizeof(RAWIspCamInfo));
            obOfst.result = NSCam::IMetadata::getEntry<IMetadata::Memory>(halMeta, MTK_PROCESSOR_CAMINFO, obOfst.val);
            RAWIspCamInfo *camInfo = reinterpret_cast<RAWIspCamInfo *>(obOfst.val.editArray());

            // Algo request obofset should be positive value
            if(camInfo->rOBC_OFST[0] < 0)
                m_obFst = camInfo->rOBC_OFST[0] * -1;
            else
                m_obFst = camInfo->rOBC_OFST[0];
        } else {
            ainrLogW("Golden halMeta is null!!!");
        }

        ainrLogD("OB offset(%d)", m_obFst);
    }

    return;
}

void AinrCore::bufferReload()
{
    for(auto i = 0; i < m_vDataPackage.size(); i++) {
        FileReadRule rule;
        const char* bufStr = "P2Node";
        std::string imgoStr = "InputImgo" + std::to_string(i);
        std::string rrzoStr = "InputRrzo" + std::to_string(i);

        rule.getFile_WarpingRaw(i, "EIspProfile_AINR_Main", m_vDataPackage[i].imgoBuffer, imgoStr.c_str(), bufStr, m_sensorId);
        rule.getFile_WarpingRaw(i, "EIspProfile_AINR_Main", m_vDataPackage[i].rrzoBuffer, rrzoStr.c_str(), bufStr, m_sensorId);

        std::string reImgoStr = "RedumpInputImgo" + std::to_string(i);
        std::string reRrzoStr = "RedumpInputRrzo" + std::to_string(i);

        bufferDumpRaw(m_vDataPackage[i].halMeta, m_vDataPackage[i].imgoBuffer, TuningUtils::RAW_PORT_IMGO, reImgoStr.c_str(), m_mainPathProfile);
        bufferDumpRaw(m_vDataPackage[i].halMeta, m_vDataPackage[i].rrzoBuffer, TuningUtils::RAW_PORT_IMGO, reRrzoStr.c_str(), m_mainPathProfile);
    }

    return;
}

void AinrCore::nextCapture()
{
    // Release RRZO and IMGO
    {
        std::lock_guard<decltype(m_cbMx)> _k(m_cbMx);
        MINT32 processUniqueKey = 0;
        if (!IMetadata::getEntry<MINT32>(m_vDataPackage[0].halMeta, MTK_PLUGIN_UNIQUEKEY, processUniqueKey)) {
            ainrLogE("cannot get unique about ainr capture");
            return;
        } else {
            constexpr int mainFrameIndex = 0;
            m_cb(processUniqueKey, mainFrameIndex);
        }
    }
}

void AinrCore::registerCallback(std::function<void(int32_t, int32_t)> cb)
{
    std::lock_guard<decltype(m_cbMx)> _k(m_cbMx);
    m_cb = cb;
    return;
}

enum AinrErr AinrCore::unpackRawBuffer(IImageBuffer* pInPackBuf, IImageBuffer* pOutUnpackBuf)
{
    DngOpResultInfo dngResult;
    DngOpImageInfo dngImage;

    int i4ImgWidth  = pInPackBuf->getImgSize().w;
    int i4ImgHeight = pInPackBuf->getImgSize().h;
    int i4BufSize   = pInPackBuf->getBufSizeInBytes(0);
    int i4ImgStride = pInPackBuf->getBufStridesInBytes(0);
    auto u4BufSize = DNGOP_BUFFER_SIZE(i4ImgWidth * 2, i4ImgHeight);

    std::unique_ptr< MTKDngOp, std::function<void(MTKDngOp*)> >
                                    pDngOp = nullptr;

    pDngOp = decltype(pDngOp)(
        MTKDngOp::createInstance(DRV_DNGOP_UNPACK_OBJ_SW),
        [](MTKDngOp* p) {
            if (!p) return;
            p->destroyInstance(p);
        }
    );

    // unpack algorithm
    ainrLogD("Unpack +");
    dngImage.Width = i4ImgWidth;
    dngImage.Height = i4ImgHeight;
    dngImage.Stride_src = i4ImgStride;
    dngImage.Stride_dst = pOutUnpackBuf->getBufStridesInBytes(0);
    dngImage.BIT_NUM = 10;
    dngImage.BIT_NUM_DST = 12;
    dngImage.Buff_size = u4BufSize;
    dngImage.srcAddr     = (void *) (pInPackBuf->getBufVA(0));
    dngResult.ResultAddr = (void *) (pOutUnpackBuf->getBufVA(0));
    pDngOp->DngOpMain((void*)&dngImage, (void*)&dngResult);
    ainrLogD("Unpack -");
    ainrLogD("unpack processing. va[in]:%p, va[out]:%p", dngImage.srcAddr, dngResult.ResultAddr);
    ainrLogD("img size(%dx%d) src stride(%d) bufSize(%d) -> dst stride(%d) bufSize(%zu)", i4ImgWidth, i4ImgHeight,
             dngImage.Stride_src,i4BufSize, dngImage.Stride_dst , pOutUnpackBuf->getBufSizeInBytes(0));

    return AinrErr_Ok;
}

enum AinrErr AinrCore::setUpBayerOrder(IImageBuffer *rawBuffer)
{
    // Get sensor format
    IHalSensorList *const pIHalSensorList = MAKE_HalSensorList(); // singleton, no need to release
    if (pIHalSensorList) {
        MUINT32 sensorDev = (MUINT32) pIHalSensorList->querySensorDevIdx(m_sensorId);
        NSCam::SensorStaticInfo sensorStaticInfo;
        memset(&sensorStaticInfo, 0, sizeof(NSCam::SensorStaticInfo));
        pIHalSensorList->querySensorStaticInfo(sensorDev, &sensorStaticInfo);
        rawBuffer->setColorArrangement(static_cast<MINT32>(sensorStaticInfo.sensorFormatOrder));
    }
    return AinrErr_Ok;
}

void AinrCore::setDebugExif(IMetadata* metadata, int algoType)
{
    if (CC_UNLIKELY( metadata == nullptr )) {
        ainrLogW("Out hal meta is nullptr cannot setup exif");
        return;
    }

    bool haveExif = false;
    {
        IMetadata::IEntry entry = metadata->entryFor(MTK_HAL_REQUEST_REQUIRE_EXIF);
        if (! entry.isEmpty()  && entry.itemAt(0, Type2Type<MUINT8>()) )
                haveExif = true;
    }

    // Get 3A exifMeta from metadata
    IMetadata exifMeta;
    IMetadata::IEntry entry = metadata->entryFor(MTK_3A_EXIF_METADATA);
    if (entry.isEmpty()) {
        ainrLogW("No MTK_3A_EXIF_METADATA can be used");
        return;
    }
    exifMeta = entry.itemAt(0, Type2Type<IMetadata>());

    // add previous debug information from BSS
    IMetadata::Memory debugInfoSet;
    const MUINT32 idx = MF_TAG_AINR_EN;
    std::map<MUINT32, MUINT32> data;
    data.emplace(std::make_pair(idx, algoType));

    if(IMetadata::getEntry<IMetadata::Memory>(&exifMeta, MTK_MF_EXIF_DBGINFO_MF_DATA, debugInfoSet)) {
        auto pTag = reinterpret_cast<debug_exif_field*>(debugInfoSet.editArray());

        for (const auto& item : data) {
            const MUINT32 index = item.first;
            if (pTag[index].u4FieldID&0x1000000)
                continue;
            pTag[index].u4FieldID    = (0x1000000 | index);
            pTag[index].u4FieldValue = item.second;
        }
        IMetadata::setEntry<IMetadata::Memory>(&exifMeta, MTK_MF_EXIF_DBGINFO_MF_DATA, debugInfoSet);
    } else {
        /* set debug information into debug Exif metadata */
        DebugExifUtils::setDebugExif(
                DebugExifUtils::DebugExifType::DEBUG_EXIF_MF,
                static_cast<MUINT32>(MTK_MF_EXIF_DBGINFO_MF_KEY),
                static_cast<MUINT32>(MTK_MF_EXIF_DBGINFO_MF_DATA),
                data,
                &exifMeta);
    }

    /* update debug Exif metadata */
    IMetadata::IEntry entry_exif(MTK_3A_EXIF_METADATA);
    entry_exif.push_back(exifMeta, Type2Type<IMetadata>());
    metadata->update(entry_exif.tag(), entry_exif);
    //
    return;
}

bool AinrCore::queryAlgoSupport(NSCam::MSize size)
{
    bool isSupport = true;

    NRCORE_QUERY_CAPABILITY_IN_STRUCT rQueryCapabilityIn = {};
    MRESULT RetCode = S_NRCORE_OK;
    std::unique_ptr<MTKNRCore, std::function<void(MTKNRCore*)>> pNRCoreDrv = nullptr;
    pNRCoreDrv = decltype(pNRCoreDrv)( MTKNRCore::createInstance(DRV_NRCORE_OBJ_SW),
        [](MTKNRCore* nrCore) {
            CAM_ULOGM_TAGLIFE("NrCore destroy");
            if (CC_UNLIKELY( nrCore == nullptr )) {
                ainrLogW("NRCore is nullptr");
            } else {
                ainrLogD("NR core unint");
                nrCore->destroyInstance();
            }
        }
    );
    if (CC_UNLIKELY(pNRCoreDrv == nullptr))
    {
        ainrLogE("Create NRCore fail");
        return false;
    }

    ainrLogD("Input resolution (w, h)=(%d, %d)", size.w, size.h);

    rQueryCapabilityIn.u4Width = size.w;
    rQueryCapabilityIn.u4Height = size.h;
    RetCode = pNRCoreDrv->NRCoreFeatureCtrl(NRCORE_FTCTRL_QUERY_CAPABILITY, (void *)&rQueryCapabilityIn, nullptr);
    if (RetCode == E_NRCORE_INVALID_RESOLUTION) {
        ainrLogD("unsupported resolution (w, h)=(%d, %d)", rQueryCapabilityIn.u4Width, rQueryCapabilityIn.u4Height);
        return false;
    }

    return isSupport;
}

AINRCORE_VERSION AinrCore::queryVersion() const
{
    return AINRCORE_VERSION_1_0;
}

int32_t AinrCore::queryMainFrameIndex() const {
    // AINR 1.0 no need to take care of main frame
    // Main frame is always the first frame
    return 0;
}

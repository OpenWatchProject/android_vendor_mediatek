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
#define LOG_TAG "AinrCore"

#include "AinrCore.h"
#include <mtkcam3/feature/ainr/AinrUlog.h>
// mtkcam
#include <mtkcam/utils/metadata/IMetadata.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>

// JobQueue and memory utils
#include <mtkcam/utils/std/JobQueue.h>
#include <mtkcam/utils/sys/MemoryInfo.h>
//
#include <mtkcam/drv/IHalSensor.h> // sensor type
#include <mtkcam/drv/iopipe/SImager/ISImager.h>

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
#include <vector>  // std::vector
#include <deque>  // std::deque
#include <map>  // std::map
#include <string>  // std::string
#include <future>  // std::async, std::launch
#include <fstream>  // std::ifstream
#include <regex>  // std::regex, std::sregex_token_iterator
#include <iterator>  // std::back_inserter
#include <cfenv>
#include <algorithm>  // std::sort

// Buffer dump
#define AINR_DUMP_PATH              "/data/vendor/camera_dump"
#define AINR_DUMP_TUNING_FILENAME    "ainr-tuning.bin"

using std::vector;
using std::map;
using std::string;
using NSCam::TuningUtils::FILE_DUMP_NAMING_HINT;
using namespace ainr;
using namespace NSIspTuning;

#define CHECK_AINR(ret, msg)     do { if (ret != AinrErr_Ok) { printf("Failed: %s\n", msg); assert(false); } }while(0)

// Warping Map
#define MAP_WIDTH 582
#define MAP_HEIGHT 437
#define MAP_PIXEL_SIZE 4
#define UNPACK_OUTBITS 12

// Static lock to protect NRCore. Only one AinrCore allowerd to do MDLA
static std::mutex sNrCoreMx;

/******************************************************************************
 * A I N R
 *****************************************************************************/

/**
 *  AinrCore
 */
AinrCore::AinrCore(void)
    : m_DumpWorking(0)
    , m_bittrueEn(0)
    , m_onlineTuning(0)
    , m_autoTest(0)
    , m_sensorId(0)
    , m_imgoWidth(0)
    , m_imgoHeight(0)
    , m_imgoStride(0)
    , m_rrzoWidth(0)
    , m_rrzoHeight(0)
    , m_uniqueKey(0)
    , m_captureNum(0)
    , m_requestNum(0)
    , m_frameNum(0)
    , m_dgnGain(4928)
    , m_obFst(256)
    , m_bNeedTileMode(true)
    , m_bIsLowMem(false)
    , m_algoType(AINR_ONLY)
    , mb_needDrc(false)
    , m_bssIndex(0)
    , m_mainFrameIndex(0)
    , m_NvramPtr(nullptr)
    , mb_AllocfefmDone(false)
    , m_fefm(nullptr)
    , m_warping(nullptr)
    , mb_AllocUpDone(false)
    , m_outBuffer(nullptr)
    , mMinorJobQueue(nullptr)
    , mb_upBaseDone(false)
    , m_aiAlgo(nullptr)
    , m_bFeoReady(false) {
    m_DumpWorking  = ::property_get_int32("vendor.debug.camera.ainr.dump", 0);
    m_bittrueEn    = ::property_get_int32("vendor.bittrue.ainr.en", 0);
    m_onlineTuning = ::property_get_int32("vendor.debug.camera.dumpin.en", 0);
    m_autoTest     = ::property_get_int32("vendor.debug.ainr.test", 0);
}

AinrCore::~AinrCore(void) {
    // wait Fefm future done (timeout 1 second)
    ainrLogD("~AinrCore");
    // Release fefm buffer
    CHECK_AINR(releaseFeFmBuffer(), "Release fefm buffer fail");
    CHECK_AINR(releaseGridBuffer(), "Release grid buffer fail");
    CHECK_AINR(releaseWarpingBuffer(), "Release warping buffer fail");
    CHECK_AINR(releaseUnpackBuffer(), "Release unpack buffer fail");

    // NVRAM release
    {
        std::lock_guard<decltype(m_nvMx)> _l(m_nvMx);
        m_NvramPtr = nullptr;
    }
}

/*
 * We initialize NRAlgo and P2 related modules (FEFM, Warping)
 * We ask jobQueue to do memory allocation.
*/
enum AinrErr AinrCore::init(const AinrConfig_t &cfg) {
    AinrErr err = AinrErr_Ok;

    // Parsing cfg
    {
        std::lock_guard<std::mutex> _l(m_cfgMx);
        m_sensorId      = cfg.sensor_id;
        m_captureNum    = cfg.captureNum;
        m_imgoWidth     = cfg.imgoWidth;
        m_imgoHeight    = cfg.imgoHeight;
        m_imgoStride    = cfg.imgoStride;
        m_rrzoWidth     = cfg.rrzoWidth;
        m_rrzoHeight    = cfg.rrzoHeight;
        m_requestNum    = cfg.requestNum;
        m_frameNum      = cfg.frameNum;
        m_uniqueKey     = cfg.uniqueKey;
        m_dgnGain       = cfg.dgnGain;
        m_bNeedTileMode = true;
        m_algoType      = cfg.algoType;
        mb_needDrc      = cfg.needDRC;
    }

    ainrLogD("Imgo (w,h,s) = (%u, %u, %zu), bssIndex(%d)",
            m_imgoWidth, m_imgoHeight, m_imgoStride, m_bssIndex);

    // Fefm
    m_fefm = IAinrFeFm::createInstance();
    if (CC_UNLIKELY(m_fefm.get() == nullptr)) {
        ainrLogE("Get fefm error");
        return AinrErr_UnexpectedError;
    }

    // FEFM callback function to trigger
    // warping and unpack buffers.
    m_AsyncLauncher = [this] (int index) -> void {
        ainrLogD("Launch matching reference frame task(%d)", index);
        std::lock_guard<std::mutex> __lk(m_jobsMx);
        m_jobs.push_back(
                std::async(std::launch::async,
                    m_matchingMethod, index));
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

        // crop information
        m_rrzCrop = outParam.resizeCrop;

        m_vFeo.resize(m_captureNum);
        m_vFmoBase.resize(m_captureNum);
        m_vFmoRef.resize(m_captureNum);
        ainrLogD("Ainr capture number(%d)", m_captureNum);
    }

    // Allocate working buffer
    // We would like to use jobQueue to do buffer allocation
    // Get jobQueue
    if (CC_UNLIKELY(mMinorJobQueue.get() == nullptr)) {
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
            ainrLogD("Allocate FEO");
            for (auto && buf : m_vFeo) {
                ret = ImageBufferUtils::getInstance().allocBuffer(
                    buf, pfeoSize.w, pfeoSize.h, eImgFmt_STA_BYTE);
                if (CC_UNLIKELY(ret == MFALSE)) {
                    ainrLogF("allocate fmo buffer error!!!");
                    return AinrErr_UnexpectedError;
                }
            }
            // FmoBase
            ainrLogD("Allocate FMO");
            for (auto && buf : m_vFmoBase) {
                ret = ImageBufferUtils::getInstance().allocBuffer(
                    buf, pfmoSize.w, pfmoSize.h, eImgFmt_STA_BYTE);
                if (CC_UNLIKELY(ret == MFALSE)) {
                    ainrLogF("allocate fmo buffer error!!!");
                    return AinrErr_UnexpectedError;
                }
            }
            // FmoRef
            for (auto && buf : m_vFmoRef) {
                ret = ImageBufferUtils::getInstance().allocBuffer(
                    buf, pfmoSize.w, pfmoSize.h, eImgFmt_STA_BYTE);
                if (CC_UNLIKELY(ret == MFALSE)) {
                    ainrLogF("allocate fmo buffer error!!!");
                    return AinrErr_UnexpectedError;
                }
            }

            mb_AllocfefmDone = true;
            ainrLogD("Allocate fefm buffer done");
            m_fefmCond.notify_one();
            return AinrErr_Ok;
        };

        mMinorJobQueue->addJob(std::bind(_fefmAllocJob, feoSize, fmoSize));
    }

    // Allocate unpack
    auto _alloc_unpack = [this] () -> void {
        std::lock_guard<std::mutex> _lk(m_AllocUpMx);
        CAM_ULOGM_TAGLIFE("Allocate working(Warping+Unpack) buffers");
        // Allocate warping buffers
        // Output buffer allocation
        m_vRg.resize(m_captureNum);
        m_vGb.resize(m_captureNum);

        // WPE outout buffer should be 64 aligned
        MUINT32 aligned = 64;
        MSize outSize = MSize(m_imgoWidth, m_imgoHeight/2);  // This is because one run for half of buffer
        ainrLogD("Allocate Warping");
        for (int i = 0; i < m_captureNum; i++) {
            auto ret = ImageBufferUtils::getInstance().allocBuffer(
                m_vRg[i], outSize.w, outSize.h, eImgFmt_BAYER10, MTRUE, aligned);
            if (CC_UNLIKELY(ret == MFALSE)) {
                ainrLogF("allocate RG buffer error!!!");
                return;
            }
            ret = ImageBufferUtils::getInstance().allocBuffer(
                m_vGb[i], outSize.w, outSize.h, eImgFmt_BAYER10, MTRUE, aligned);
            if (CC_UNLIKELY(ret == MFALSE)) {
                ainrLogF("allocate GB buffer error!!!");
                return;
            }
        }

        // Allocate unpack buffers
        unsigned int size = m_imgoWidth*m_imgoHeight*sizeof(short);
        for (int i = 0; i < m_captureNum; i++) {
            sp<IImageBuffer> unpackRawBuff = nullptr;
            ImageBufferUtils::getInstance().allocBuffer(
            unpackRawBuff, size, 1, eImgFmt_STA_BYTE);
            if (CC_UNLIKELY(unpackRawBuff.get() == nullptr)) {
                ainrLogF("unpackRawBuff is null");;
                return;
            }

            mv_unpackRaws.push_back(unpackRawBuff);
        }

        mb_AllocUpDone = true;
        ainrLogD("Allocate warping and unpack buffer done");
        m_AllocUpCond.notify_one();
        return;
    };

    mMinorJobQueue->addJob(_alloc_unpack);

    // Init warping
    m_warping = IAinrWarping::createInstance();
    if (CC_UNLIKELY(m_warping.get() == nullptr)) {
        ainrLogE("Get warping error");
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
    int64_t memRestriction = NSCam::NSMemoryInfo::getSystemTotalMemorySize();
    std::fesetround(FE_TONEAREST);
    #define RESTRICT_MEM_TH (4.3)
    if (std::nearbyint(memRestriction/1024/1024/1024) < RESTRICT_MEM_TH) {
        ainrLogD("It is low memory. System Memory is (%" PRId64 ") <= 4.3G Mb, use down sampe "
                "nearbyint Memory is (%.1f)", memRestriction,
                std::nearbyint(memRestriction/1024/1024/1024));
        m_bIsLowMem = true;
    }
    #undef RESTRICT_MEM_TH

    m_aiAlgo = IAiAlgo::createInstance(m_algoType);
    if (CC_UNLIKELY(m_aiAlgo.get() == nullptr)) {
        ainrLogF("Can not get AiAlgo instance!");
        return AinrErr_UnexpectedError;
    }

    return err;
}

enum AinrErr AinrCore::doAinr() {
    AinrErr err = AinrErr_Ok;
    CAM_ULOGM_FUNCLIFE_ALWAYS();

    // Set Prformance Mode to Thermal Side
    if (NSCam::NSThermalInfo::setThermalMode(1) < 0) {
        ainrLogW("Write Thermal Hinet Falied");
    }

    // Check capture number is sync with inputBuffer package
    if (CC_UNLIKELY(m_captureNum != static_cast<int>(m_vDataPackage.size())
        && m_vDataPackage.size() != 0)) {
            ainrLogF("capture number(%d) is not sync with input buffer(%zu)",
                    m_captureNum, m_vDataPackage.size());
            return AinrErr_UnexpectedError;
    }

    // Parsing metadata before postproc
    parsingMeta();

    if (m_autoTest) {
        bufferReload();
    }

    if (CC_UNLIKELY(mMinorJobQueue.get() == nullptr)) {
        acquireJobQueue();
    }

    // Unpack Base frame
    // Async Unpack golden frame and gen noise
    // Handle base frame
    auto _jobBase = [this]() -> void {
        AiParam params;
        params.width = m_imgoWidth;
        params.height = m_imgoHeight;
        params.stride = m_imgoStride;
        params.outBits = UNPACK_OUTBITS;
        params.algoType = m_algoType;
        String8 logString;
        logString += String8::format("AiParam BuffInfo(%d, %d, %d) ",
                params.width, params.height, params.stride);

        for (int i = 0; i < 4; i++) {
           params.obOfst[i] = m_obFst;
           params.dgnGain[i] = m_dgnGain;
           logString += String8::format("AiParam obInfo[%d](%d, %d) ",
                i, params.dgnGain[i], params.obOfst[i]);
        }
        ainrLogD("Base Unpack Param:(%s)", logString.string());

        IImageBuffer* imgoBase = m_vDataPackage[m_bssIndex].imgoBuffer;
        if (CC_UNLIKELY(imgoBase == nullptr)) {
            ainrLogF("BSS frame is nullptr");
            return;
        }

        if(m_onlineTuning == ONLINE_DEVICE_MODE_ENABLE  && !m_autoTest) {
            FileReadRule rule;
            std::string ispProfileName
                    = "EIspProfile_AIISP_Capture_Before_Blend";
            if (m_algoType == AIHDR) {
                ispProfileName = "EIspProfile_AIISP_Capture_Before_Blend_16b";
            }
            rule.getFile_RAW(0, ispProfileName.c_str(), imgoBase,
                    "P2Node", m_sensorId);
        }

        AiBasePackage bufPackage;
        bufPackage.inBuf = imgoBase;
        bufPackage.outBuf = mv_unpackRaws[m_bssIndex].get();
        m_aiAlgo->baseBufUpk(params, bufPackage);

        if (m_DumpWorking & AinrDumpWorking) {
            FILE_DUMP_NAMING_HINT &hint = mv_dumpNamingHints[m_bssIndex];
            bufferDump(hint, mv_unpackRaws[m_bssIndex].get(), "BaseFrameUnpack",
                    static_cast<int>(RAW_PORT_UNDEFINED), formatType::BLOB);
        }

        {
            std::lock_guard<std::mutex> _lk(m_upBaseMx);
            mb_upBaseDone = true;
            m_upBaseCond.notify_one();
            ainrLogD("Unpack base done");
        }
    };

    mMinorJobQueue->addJob(_jobBase);

    if (CC_UNLIKELY(doMatching() != AinrErr_Ok)) {
        ainrLogW("[Matching fail!] Do error handling by unpack golden pack10");
        CHECK_AINR(releaseFeFmBuffer(), "Release fefm buffer fail");
        CHECK_AINR(releaseGridBuffer(), "Release grid buffer fail");
        CHECK_AINR(releaseWarpingBuffer(), "Release warping buffer fail");
        m_algoType = AINR_SINGLE;
        unpackRawBuffer(m_vDataPackage[m_bssIndex].imgoBuffer, m_outBuffer);
        goto lbExit;
    }

    if (CC_UNLIKELY(doNrCore() != AinrErr_Ok)) {
        ainrLogW("[NR core fail!] Do error handling by unpack golden pack10");
        m_algoType = AINR_SINGLE;
        unpackRawBuffer(m_vDataPackage[m_bssIndex].imgoBuffer, m_outBuffer);
        goto lbExit;
    }

lbExit:
    // Release unpack raw buffers if needed
    CHECK_AINR(releaseUnpackBuffer(), "Release unpack buffer fail");

    // Setup final raw bayer format
    setUpBayerOrder(m_outBuffer);

    // Do DRC if needed
    if (mb_needDrc) {
        IAiAlgo::drcParam param;
        param.pRawBuf = m_outBuffer;
        param.pAppMeta = m_vDataPackage[m_bssIndex].appMeta;
        param.pHalMeta = m_vDataPackage[m_bssIndex].halMeta;
        param.pAppMetaDynamic = m_vDataPackage[m_bssIndex].appMetaDynamic;
        param.openId = m_sensorId;
        param.uniqueKey = m_uniqueKey;
        param.requestNum = m_requestNum;
        param.frameNum = m_frameNum;
        m_aiAlgo->doDRC(param);
    }

    if (m_DumpWorking & AinrDumpPartial) {
        // Dump tuning buffer
        dumpAinrTuning(AINR_DUMP_TUNING_FILENAME);

        // Dump final rawout
        FILE_DUMP_NAMING_HINT &hint = mv_dumpNamingHints[m_bssIndex];
        bufferDump(hint, m_outBuffer, "RawOutput",
                static_cast<int>(TuningUtils::RAW_PORT_IMGO), formatType::RAW);
    }

    // Set up debug exif
    setDebugExif(m_vDataPackage[m_bssIndex].outHalMeta, m_algoType);

    if (NSCam::NSThermalInfo::setThermalMode(0) < 0) {
        ainrLogW("Write Thermal Hinet Falied");
    }

    return err;
}

enum AinrErr AinrCore::doCancel() {
    return AinrErr_Ok;
}

enum AinrErr AinrCore::setNvramProvider(const std::shared_ptr<IAinrNvram> &nvramProvider) {
    enum AinrErr err = AinrErr_Ok;
    std::lock_guard<decltype(m_nvMx)> _l(m_nvMx);
    m_NvramPtr = nvramProvider;

    return err;
}

enum AinrErr AinrCore::addInput(const std::vector<AinrPipelinePack>& inputPack) {
    std::lock_guard<std::mutex> _l(m_DataLock);
    m_vDataPackage = inputPack;
    return AinrErr_Ok;
}

enum AinrErr AinrCore::addOutput(IImageBuffer *outBuffer) {
    std::lock_guard<std::mutex> _l(m_DataLock);
    m_outBuffer = outBuffer;
    return AinrErr_Ok;
}

void AinrCore::prepareJobs() {
    // Homography job
    IMetadata* halMeta = nullptr;
    halMeta = m_vDataPackage[m_bssIndex].halMeta;

    // Method prepare
    // Homography gridMap allocation
    m_vGridX.resize(m_captureNum);
    m_vGridY.resize(m_captureNum);

    m_homoMethod = [this](int index) -> enum AinrErr {
        int32_t gridWidth = MAP_WIDTH;
        int32_t gridHeight = MAP_HEIGHT;
        int32_t gridMapTotalSize = MAP_WIDTH * MAP_HEIGHT * MAP_PIXEL_SIZE;
        ainrLogD("GridMap bufferSize(%d)", gridMapTotalSize);

        auto ret = ImageBufferUtils::getInstance().allocBuffer(
            m_vGridX[index], gridMapTotalSize, 1, eImgFmt_STA_BYTE);
        if (CC_UNLIKELY(ret == MFALSE)) {
            ainrLogF("allocate gridX error!!!");
            return AinrErr_UnexpectedError;
        }
        ret = ImageBufferUtils::getInstance().allocBuffer(
            m_vGridY[index], gridMapTotalSize, 1, eImgFmt_STA_BYTE);
        if (CC_UNLIKELY(ret == MFALSE)) {
            ainrLogF("allocate gridY error!!!");
            return AinrErr_UnexpectedError;
        }

        if (CC_UNLIKELY(m_vDataPackage[m_bssIndex].imgoBuffer
                == nullptr)) {
            ainrLogF("IMGO is nullptr");
            return AinrErr_UnexpectedError;
        }

        AiHgParam hdParm;
        hdParm.imgoBuf = m_vDataPackage[m_bssIndex].imgoBuffer;
        hdParm.rrzoBuf = m_vDataPackage[m_bssIndex].rrzoBuffer;
        hdParm.baseFeBuf = m_vFeo[m_bssIndex].get();
        hdParm.refFeBuf = m_vFeo[index].get();
        hdParm.baseFmBuf = m_vFmoBase[index].get();
        hdParm.refFmBuf =  m_vFmoRef[index].get();
        hdParm.gridX = m_vGridX[index].get();
        hdParm.gridY = m_vGridY[index].get();
        hdParm.cropRect = m_rrzCrop;
        hdParm.gridMapSize = MSize(gridWidth, gridHeight);
        m_aiAlgo->doHG(hdParm);

        m_vGridX[index]->syncCache(eCACHECTRL_INVALID);
        m_vGridY[index]->syncCache(eCACHECTRL_INVALID);

        if (m_DumpWorking & AinrDumpWorking) {
            std::string gridXStr = "GridX" + std::to_string(index);
            std::string gridYStr = "GridY" + std::to_string(index);
            FILE_DUMP_NAMING_HINT &hintGrid = mv_dumpNamingHints[index];
            bufferDump(hintGrid, m_vGridX[index].get() , gridXStr.c_str(),
                    static_cast<int>(RAW_PORT_UNDEFINED), formatType::BLOB);
            bufferDump(hintGrid, m_vGridY[index].get() , gridYStr.c_str(),
                    static_cast<int>(RAW_PORT_UNDEFINED), formatType::BLOB);
        }

        // Early release reference feo/fmo buffers if no dump buffers needed
        if (m_DumpWorking == AinrDumpClose) {
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
            if (!mb_AllocUpDone) {
                ainrLogD("Wait warping and unpack buffer allocation....");
                m_AllocUpCond.wait(_lk);
            }
        }

        // Warping Buffer Check
        if (CC_UNLIKELY(m_vRg[index].get() == nullptr)
                || CC_UNLIKELY(m_vGb[index].get() == nullptr)) {
            ainrLogF("Warping packed raw is nullptr");
            return AinrErr_UnexpectedError;
        }

        if (CC_UNLIKELY(m_vDataPackage[index].imgoBuffer == nullptr)) {
            ainrLogF("Warping input imgo is nullptr");
            return AinrErr_UnexpectedError;
        }

        IAinrWarping::WarpingPackage infoPack;
        infoPack.buffer = m_vDataPackage[index].imgoBuffer;
        infoPack.gridX  = m_vGridX[index].get();
        infoPack.gridY  = m_vGridY[index].get();
        infoPack.outRg  = m_vRg[index].get();
        infoPack.outGb  = m_vGb[index].get();
        auto ret = m_warping->doWarping(infoPack);

        if (m_DumpWorking & AinrDumpPartial) {
            IMetadata *halMeta = m_vDataPackage[index].halMeta;
            std::string warpingRgStr = "WarpingRg" + std::to_string(index);
            std::string warpingGbStr = "WarpingGb" + std::to_string(index);
            FILE_DUMP_NAMING_HINT &hintWarp = mv_dumpNamingHints[index];
            bufferDump(hintWarp, m_vRg[index].get(), warpingRgStr.c_str(),
                    static_cast<int>(TuningUtils::RAW_PORT_IMGO), formatType::RAW);
            bufferDump(hintWarp, m_vGb[index].get(), warpingGbStr.c_str(),
                    static_cast<int>(TuningUtils::RAW_PORT_IMGO), formatType::RAW);
        }

        // Release GridMap
        ImageBufferUtils::getInstance().deallocBuffer(m_vGridX[index].get());
        ImageBufferUtils::getInstance().deallocBuffer(m_vGridY[index].get());
        m_vGridX[index] = nullptr;
        m_vGridY[index] = nullptr;

        return ret;
    };

    m_unpackMethod = [this](int index) -> enum AinrErr {
        // Warping Buffer Check
        if (CC_UNLIKELY(m_vRg[index].get() == nullptr)
                || CC_UNLIKELY(m_vGb[index].get() == nullptr)) {
            ainrLogF("Warping packed raw is nullptr");
            return AinrErr_UnexpectedError;
        }

        // Check online device tuning
        if (m_onlineTuning == ONLINE_DEVICE_MODE_ENABLE && !m_autoTest) {
            std::string ispProfileName = "EIspProfile_AIISP_Capture_Before_Blend";
            if (m_algoType == AIHDR) {
                ispProfileName = "EIspProfile_AIISP_Capture_Before_Blend_16b";
            }

            FileReadRule rule;
            const char* bufStr = "MtkCam/IBUS";
            std::string warpingRgStr = "WarpingRg" + std::to_string(index);
            std::string warpingGbStr = "WarpingGb" + std::to_string(index);
            rule.getFile_WarpingRaw(index, ispProfileName.c_str(),
                    m_vRg[index].get(), warpingRgStr.c_str(), bufStr, m_sensorId);
            rule.getFile_WarpingRaw(index, ispProfileName.c_str(),
                    m_vGb[index].get(), warpingGbStr.c_str(), bufStr, m_sensorId);
        }

        const MSize &warpSize = m_vRg[index]->getImgSize();
        size_t warpingStride = m_vRg[index]->getBufStridesInBytes(0);

        AiParam params;
        params.width = m_imgoWidth;
        params.height = m_imgoHeight;
        params.stride = warpingStride;
        params.outBits = UNPACK_OUTBITS;
        params.algoType = m_algoType;
        String8 logString;
        logString += String8::format("AiParam BuffInfo(%d, %d, %d) ", params.width, params.height, params.stride);

        for (int i = 0; i < 4; i++) {
           params.obOfst[i] = m_obFst;
           params.dgnGain[i] = m_dgnGain;
           logString += String8::format("AiParam obInfo[%d](%d, %d) ", i, params.dgnGain[i], params.obOfst[i]);
        }

        ainrLogD("Reference frame Unpack Param:(%s)", logString.string());

        AiRefPackage bufPackage;
        bufPackage.rgBuf = m_vRg[index].get();
        bufPackage.gbBuf = m_vGb[index].get();
        bufPackage.outBuf = mv_unpackRaws[index].get();
        m_aiAlgo->refBufUpk(params, bufPackage);

        //
        if (m_DumpWorking & AinrDumpWorking) {
            FILE_DUMP_NAMING_HINT &hintUpk = mv_dumpNamingHints[index];
            std::string uvStr = "RgRbUnpack" + std::to_string(index);
            bufferDump(hintUpk, mv_unpackRaws[index].get() , uvStr.c_str(),
                    static_cast<int>(RAW_PORT_UNDEFINED), formatType::BLOB);
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

enum AinrErr AinrCore::doMatching() {
    AinrErr err = AinrErr_Ok;
    CAM_ULOGM_APILIFE();

    int bssIndex = m_bssIndex;
    int captureNum = m_captureNum;

    // Wait FEFM buffers
    {
        // Because we use conditional wait which changes mutex
        // state need to use unique_lock
        CAM_ULOGM_TAGLIFE("wait_fefm_wk_buf");
        std::unique_lock<std::mutex> _lk(m_fefmMx);
        if (!mb_AllocfefmDone) {
            ainrLogD("Wait FEFM allocation....");
            m_fefmCond.wait(_lk);
        }
    }

    // Check fefm working buffers
    {
        for (auto && buf : m_vFeo) {
            if (CC_UNLIKELY(buf.get() == nullptr)) {
                ainrLogE("feo working buffers are nullptr");
                releaseFeFmBuffer();
                return AinrErr_UnexpectedError;
            }
        }

        for (auto && buf : m_vFmoBase) {
            if (CC_UNLIKELY(buf.get() == nullptr)) {
                ainrLogE("fmoBase working buffers are nullptr");
                releaseFeFmBuffer();
                return AinrErr_UnexpectedError;
            }
        }

        for (auto && buf : m_vFmoRef) {
            if (CC_UNLIKELY(buf.get() == nullptr)) {
                ainrLogE("fmoRef working buffers are nullptr");
                releaseFeFmBuffer();
                return AinrErr_UnexpectedError;
            }
        }
    }

    // Start to do FEFM
    if (CC_UNLIKELY(m_fefm.get() == nullptr)) {
        ainrLogE("Get fefm error");
        return AinrErr_UnexpectedError;
    }

    // Prepare Homography/Warping/Unpack jobs
    prepareJobs();

    {
        std::lock_guard<std::mutex> _l(m_DataLock);

        int idx = bssIndex;
        do {
            if (!m_bFeoReady) {
                IAinrFeFm::DataPackage pack;
                pack.appMeta = m_vDataPackage[idx].appMeta;
                pack.halMeta = m_vDataPackage[idx].halMeta;
                pack.inBuf   = m_vDataPackage[idx].rrzoBuffer;
                pack.outBuf  = m_vFeo[idx].get();

                // check if need tone aligned
                if (mv_toneAlignedMatrixs.size() == captureNum) {
                    pack.matrix = &mv_toneAlignedMatrixs[idx];
                }

                if (m_bittrueEn) {
                    sp<IImageBuffer> yuvBuf = nullptr;
                    MBOOL ret = ImageBufferUtils::getInstance().allocBuffer(
                        yuvBuf, m_rrzoWidth, m_rrzoHeight, eImgFmt_YV12);
                    if (CC_UNLIKELY(ret == MFALSE)
                            || CC_UNLIKELY(yuvBuf.get() == nullptr)) {
                        ainrLogF("allocate resize YUV buffer error!!!");
                        return AinrErr_UnexpectedError;
                    }
                    pack.outYuv = yuvBuf.get();
                    m_vRrzoYuv.push_back(yuvBuf);
                }

                //
                m_fefm->buildFe(&pack);
            }

            if (idx != m_bssIndex) {
                m_fefm->buildFm(m_vFeo[m_bssIndex].get(), m_vFeo[idx].get(),
                        m_vFmoRef[idx].get(), idx, false);
                m_fefm->buildFm(m_vFeo[idx].get(), m_vFeo[m_bssIndex].get(),
                        m_vFmoBase[idx].get(), idx, true);
            }

            // Next run
            ainrLogD("Process FEFM frame index(%d)", idx);
            idx = (idx + 1) % captureNum;
        } while (idx != m_bssIndex && captureNum > 1);

        err = m_fefm->doFeFm();

        if (err == AinrErr_Ok && (m_DumpWorking & AinrDumpWorking)) {
            // Dump feo
            for (int i = 0; i < m_captureNum; i++) {
                FILE_DUMP_NAMING_HINT &hintFeo = mv_dumpNamingHints[i];
                // Feo
                std::string feoStr = "feo" + std::to_string(i);
                bufferDump(hintFeo, m_vFeo[i].get() , feoStr.c_str(),
                        static_cast<int>(RAW_PORT_UNDEFINED), formatType::BLOB);

                // If feo is done before doAinr we do not have img3o YUV
                // therefore no need to dump it
                if (m_bittrueEn && !m_bFeoReady
                    && m_vRrzoYuv.size() == m_captureNum) {
                    FILE_DUMP_NAMING_HINT &hintImg3o = mv_dumpNamingHints[i];
                    bufferDump(hintImg3o, m_vRrzoYuv[i].get(), "YuvRrz",
                            static_cast<int>(YUV_PORT_IMG3O), formatType::YUV);
                    // RRZO yuv is no used. We release it.
                    ImageBufferUtils::getInstance().deallocBuffer(m_vRrzoYuv[i].get());
                    m_vRrzoYuv[i] = nullptr;
                }
            }

            // Only reference frames have fmo
            int i = (bssIndex + 1) % captureNum;
            do {
                FILE_DUMP_NAMING_HINT &hintFmo = mv_dumpNamingHints[i];
                std::string fmoBaseStr = "fmoBase" + std::to_string(i);
                std::string fmoRefStr  = "fmoRef"  + std::to_string(i);
                bufferDump(hintFmo, m_vFmoBase[i].get(), fmoBaseStr.c_str(),
                        static_cast<int>(RAW_PORT_UNDEFINED), formatType::BLOB);
                bufferDump(hintFmo, m_vFmoRef[i].get(), fmoRefStr.c_str(),
                        static_cast<int>(RAW_PORT_UNDEFINED), formatType::BLOB);

                i = (i + 1) % captureNum;
            } while (i != m_bssIndex && captureNum > 1);
        }
    }

    // Start to wait previous stage (UV unpack done)
    {
        CAM_ULOGM_TAGLIFE("Wait UV upack done");
        // Wait all threads done
        for (auto &fut : m_jobs) {
            if (CC_LIKELY(fut.valid())) {
                fut.wait();
            } else {
                ainrLogF("matching thread is not in the valid state error!!!");
                return AinrErr_UnexpectedError;
            }
        }
    }

    // Wait base unpack done
    {
        std::unique_lock<std::mutex> __lk(m_upBaseMx);
        if (!mb_upBaseDone) {
            ainrLogD("Wait Unpack base done....");
            m_upBaseCond.wait(__lk);
        } else {
            ainrLogD("No need to wait unpack base done");
        }
    }

    if (m_bNeedTileMode && !m_bIsLowMem) {
        nextCapture();
    }

    // Ensure all buffers are released
    CHECK_AINR(releaseFeFmBuffer(), "Release fefm buffer fail");
    CHECK_AINR(releaseGridBuffer(), "Release grid buffer fail");
    CHECK_AINR(releaseWarpingBuffer(), "Release warping buffer fail");

    return err;
}

enum AinrErr AinrCore::dumpAinrTuning(const string &fileName) {
    // dump binary
    char filepath[256] = {0};
    snprintf(filepath, sizeof(filepath)-1, "%s/%09d-%04d-%04d-""%s", AINR_DUMP_PATH,
            m_uniqueKey, m_requestNum, m_bssIndex, fileName.c_str());
    std::ofstream ofs(filepath, std::ofstream::binary);
    if (!ofs.is_open()) {
        ainrLogW("dump2Binary: open file(%s) fail", filepath);
        return AinrErr_UnexpectedError;
    }

    ofs << "RRzo: crop_x=" << m_rrzCrop.p.x << ", crop_y=" << m_rrzCrop.p.y << endl;
    ofs << "DGN gain=" << m_dgnGain << endl;
    ofs << "OB ofst=" << m_obFst << endl;
    ofs << "BSS golden index=" << m_bssIndex << endl;

    // dump exposure information if needed
    if (!mv_ExpInfos.empty()) {
        for (int i = 0; i < mv_ExpInfos.size(); i++) {
            String8 dumpString;
            auto [shutterTime, sensorGain, ispGain]
                    = mv_ExpInfos[i];
            dumpString += String8::format("index=%d, sensorGain=%d,"
                    "ispGain=%d exp=%" PRIi64, i, sensorGain,
                    ispGain, shutterTime);
            ofs << dumpString.string() << endl;
        }
    }

    // dump ccm matrix if needed
    if (!mv_toneAlignedMatrixs.empty()) {
        std::vector<ToneAlignedMatrix> &vMatrix = mv_toneAlignedMatrixs;
        for (int i = 0; i < vMatrix.size(); i++) {
            ofs << "Matix id" << i << endl;

            ToneAlignedMatrix &matrix = vMatrix[i];
            for (int j = 0; j < matrix.size(); j++) {
                String8 dumpString;
                dumpString += String8::format("calToneMatrix[%d]=(%d)", j, matrix[j]);
                ofs << dumpString.string() << endl;
            }
        }
   }

    ofs.close();

    return AinrErr_Ok;
}

enum AinrErr AinrCore::doNrCore() {
    ::prctl(PR_SET_NAME, "AinrCore", 0, 0, 0);

    std::lock_guard<decltype(sNrCoreMx)> _gk(sNrCoreMx);
    AinrErr err = AinrErr_Ok;

    if (CC_UNLIKELY(m_outBuffer == nullptr)) {
        ainrLogF("m_outBuffer is null");
        return AinrErr_UnexpectedError;
    }

    // Choose algo type
    AinrCoreMode sampleMode = AinrCoreMode::Full_SAMPLE;
    if (m_bIsLowMem) {
        ainrLogD("Low memory we use down sample ");
        sampleMode = AinrCoreMode::Down_SAMPLE;
    }

    {
        std::lock_guard<decltype(m_nvMx)> _l(m_nvMx);
        m_aiAlgo->init(m_NvramPtr);
    }

    String8 logString;
    AiParam params;
    params.width = m_imgoWidth;
    params.height = m_imgoHeight;
    params.stride = m_imgoStride;
    logString += String8::format("AiParam BuffInfo(%d, %d, %d)", params.width, params.height, params.stride);

    for (int i = 0; i < 4; i++) {
       params.obOfst[i] = m_obFst;
       params.dgnGain[i] = m_dgnGain;
       logString += String8::format("AiParam obInfo[%d](%d, %d) ", i, params.dgnGain[i], params.obOfst[i]);
    }

    AiCoreParam coreParam;
    std::vector<NSCam::IImageBuffer*> inBuffers;
    for (const auto & buffer : mv_unpackRaws) {
        coreParam.inBuffers.push_back(buffer.get());
    }

    if (m_algoType == AIHDR) {
        coreParam.expInfos = mv_ExpInfos;
        coreParam.sortedPairs = mv_IndexEvPairs;
    }

    coreParam.captureNum = m_captureNum;
    coreParam.isTileMode = true;
    coreParam.sampleMode = sampleMode;
    coreParam.outBuffer  = m_outBuffer;
    coreParam.uniqueKey = m_uniqueKey;
    coreParam.requestNum = m_requestNum;
    coreParam.frameNum = m_frameNum;
    logString += String8::format("AiCoreParam capNum(%d) inBufferSize(%zu), sampleMode(%d) reqNum(%d)",
            coreParam.captureNum , coreParam.inBuffers.size(), coreParam.sampleMode, m_requestNum);
    ainrLogD("Core config dump(%s)", logString.string());
    m_aiAlgo->doNrCore(params, coreParam);

    return err;
}

enum AinrErr AinrCore::releaseFeFmBuffer() {
    // Because deallocBuffer check buffer pointer
    // no need to check again here
    // Release feoBase feoRef

    // Release feos
    ainrLogD("release feo buffers");
    for (auto && buf : m_vFeo) {
        ImageBufferUtils::getInstance().deallocBuffer(buf.get());
        buf = nullptr;
    }
    m_vFeo.clear();

    // Release fmoBase
    for (auto && buf : m_vFmoBase) {
        ImageBufferUtils::getInstance().deallocBuffer(buf.get());
        buf = nullptr;
    }
    m_vFmoBase.clear();

    // Release fmoRef
    for (auto && buf : m_vFmoRef) {
        ImageBufferUtils::getInstance().deallocBuffer(buf.get());
        buf = nullptr;
    }
    m_vFmoRef.clear();

    return AinrErr_Ok;
}

enum AinrErr AinrCore::releaseWarpingBuffer() {
    for (auto && buf : m_vRg) {
        ImageBufferUtils::getInstance().deallocBuffer(buf.get());
        buf = nullptr;
    }
    for (auto && buf : m_vGb) {
        ImageBufferUtils::getInstance().deallocBuffer(buf.get());
        buf = nullptr;
    }
    m_vRg.clear();
    m_vGb.clear();

    return AinrErr_Ok;
}

enum AinrErr AinrCore::releaseGridBuffer() {
    for (auto && buf : m_vGridX) {
        ImageBufferUtils::getInstance().deallocBuffer(buf.get());
        buf = nullptr;
    }
    for (auto && buf : m_vGridY) {
        ImageBufferUtils::getInstance().deallocBuffer(buf.get());
        buf = nullptr;
    }
    m_vGridX.clear();
    m_vGridY.clear();

    return AinrErr_Ok;
}

enum AinrErr AinrCore::releaseUnpackBuffer() {
    // Release unpack buffers mv_unpackRaws
    for (auto && buf : mv_unpackRaws) {
        ImageBufferUtils::getInstance().deallocBuffer(buf.get());
        buf = nullptr;
    }
    mv_unpackRaws.clear();

    return AinrErr_Ok;
}

void AinrCore::bufferDump(FILE_DUMP_NAMING_HINT dumpHint, IImageBuffer* buff
    , const char *pUserString, int port, formatType type) {
    // dump input buffer
    char fileName[256] = "";

    if (buff == nullptr || pUserString == nullptr) {
        ainrLogE("HalMeta or buff is nullptr, dump fail");
        return;
    }

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

void AinrCore::acquireJobQueue() {
    static std::mutex       sResLock;
    static std::weak_ptr<
        NSCam::JobQueue<void()>
    >                       sResJobQ;

    // acquire resource from weak_ptr
    std::lock_guard<std::mutex> lk(sResLock);
    mMinorJobQueue = sResJobQ.lock();
    if (mMinorJobQueue.get() == nullptr) {
        mMinorJobQueue = std::shared_ptr< NSCam::JobQueue<void()> >(
                new NSCam::JobQueue<void()>("AinrCoreJob"));
        sResJobQ = mMinorJobQueue;
    }
    return;
}

void AinrCore::bufferReload() {
    for (auto i = 0; i < m_vDataPackage.size(); i++) {
        FileReadRule rule;
        const char* bufStr = "P2Node";
        std::string imgoStr = "InputImgo" + std::to_string(i);
        std::string rrzoStr = "InputRrzo" + std::to_string(i);
        rule.getFile_WarpingRaw(i, "EIspProfile_AINR_Main", m_vDataPackage[i].imgoBuffer, imgoStr.c_str(), bufStr, m_sensorId);
        rule.getFile_WarpingRaw(i, "EIspProfile_AINR_Main", m_vDataPackage[i].rrzoBuffer, rrzoStr.c_str(), bufStr, m_sensorId);

        std::string reImgoStr = "RedumpInputImgo" + std::to_string(i);
        std::string reRrzoStr = "RedumpInputRrzo" + std::to_string(i);
        FILE_DUMP_NAMING_HINT &hint = mv_dumpNamingHints[i];
        bufferDump(hint, m_vDataPackage[i].imgoBuffer, reImgoStr.c_str(),
                static_cast<int>(TuningUtils::RAW_PORT_IMGO), formatType::RAW);
        bufferDump(hint, m_vDataPackage[i].rrzoBuffer, reRrzoStr.c_str(),
                static_cast<int>(TuningUtils::RAW_PORT_IMGO), formatType::RAW);
    }

    return;
}

void AinrCore::nextCapture() {
    // Release RRZO and IMGO
    std::lock_guard<decltype(m_cbMx)> _k(m_cbMx);
    MINT32 processUniqueKey = 0;
    if (!IMetadata::getEntry<MINT32>(m_vDataPackage[0].halMeta,
            MTK_PIPELINE_UNIQUE_KEY, processUniqueKey)) {
        ainrLogE("cannot get unique about ainr capture");
        return;
    }

    m_cb(processUniqueKey, m_mainFrameIndex);
}

void AinrCore::registerCallback(std::function<void(int32_t, int32_t)> cb) {
    std::lock_guard<decltype(m_cbMx)> _k(m_cbMx);
    m_cb = std::move(cb);
    return;
}

enum AinrErr AinrCore::unpackRawBuffer(IImageBuffer* pInPackBuf, IImageBuffer* pOutUnpackBuf) {
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
        });

    // unpack algorithm
    ainrLogD("Unpack +");
    dngImage.Width = i4ImgWidth;
    dngImage.Height = i4ImgHeight;
    dngImage.Stride_src = i4ImgStride;
    dngImage.Stride_dst = pOutUnpackBuf->getBufStridesInBytes(0);
    dngImage.BIT_NUM = 10;
    dngImage.BIT_NUM_DST = 12;
    dngImage.Buff_size = u4BufSize;
    dngImage.srcAddr = reinterpret_cast<void *>(pInPackBuf->getBufVA(0));
    dngResult.ResultAddr = reinterpret_cast<void *>(pOutUnpackBuf->getBufVA(0));
    pDngOp->DngOpMain(reinterpret_cast<void *>(&dngImage), reinterpret_cast<void *>(&dngResult));
    ainrLogD("Unpack -");
    ainrLogD("unpack processing. va[in]:%p, va[out]:%p", dngImage.srcAddr, dngResult.ResultAddr);
    ainrLogD("img size(%dx%d) src stride(%d) bufSize(%d) -> dst stride(%d) bufSize(%zu)", i4ImgWidth, i4ImgHeight,
             dngImage.Stride_src, i4BufSize, dngImage.Stride_dst , pOutUnpackBuf->getBufSizeInBytes(0));

    return AinrErr_Ok;
}

enum AinrErr AinrCore::setUpBayerOrder(IImageBuffer *rawBuffer) {
    // Get sensor format
    IHalSensorList *const pIHalSensorList = MAKE_HalSensorList();  // singleton, no need to release
    if (pIHalSensorList) {
        MUINT32 sensorDev = (MUINT32) pIHalSensorList->querySensorDevIdx(m_sensorId);
        NSCam::SensorStaticInfo sensorStaticInfo;
        memset(&sensorStaticInfo, 0, sizeof(NSCam::SensorStaticInfo));
        pIHalSensorList->querySensorStaticInfo(sensorDev, &sensorStaticInfo);
        rawBuffer->setColorArrangement(static_cast<MINT32>(sensorStaticInfo.sensorFormatOrder));
    }
    return AinrErr_Ok;
}

void AinrCore::setDebugExif(IMetadata* metadata, int algoType) {
    if (CC_UNLIKELY(metadata == nullptr)) {
        ainrLogW("Out hal meta is nullptr cannot setup exif");
        return;
    }

    // Get 3A exifMeta from metadata
    IMetadata exifMeta;
    if (!IMetadata::getEntry<IMetadata>(metadata, MTK_3A_EXIF_METADATA, exifMeta)) {
        ainrLogW("No MTK_3A_EXIF_METADATA can be used");
        return;
    }

    // add previous debug information from BSS
    IMetadata::Memory debugInfoSet;
    const MUINT32 idx = MF_TAG_AINR_EN;
    std::map<MUINT32, MUINT32> data;
    data.emplace(std::make_pair(idx, algoType));

    if (IMetadata::getEntry<IMetadata::Memory>(&exifMeta, MTK_MF_EXIF_DBGINFO_MF_DATA, debugInfoSet)) {
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
    IMetadata::setEntry<IMetadata>(metadata, MTK_3A_EXIF_METADATA, exifMeta);

    return;
}

bool AinrCore::queryAlgoSupport(NSCam::MSize size __attribute__((unused))) {
    // TODO: Wait for algo support API
    return true;
}

void AinrCore::recordExpInfo()
{
    int index = 0;
    for (const auto &info : m_vDataPackage) {
        IMetadata* dynamicMeta = info.appMetaDynamic;
        IMetadata* pHalMeta = info.halMeta;
        // Get AE exposure information
        int64_t shutterTime = 0;  // nanoseconds (ns)
        if (!IMetadata::getEntry<MINT64>(dynamicMeta,
                MTK_SENSOR_EXPOSURE_TIME, shutterTime)) {
            ainrLogW("Get MTK_SENSOR_EXPOSURE_TIME fail");
        }

        int32_t sensorGain = 0;
        if (!IMetadata::getEntry<MINT32>(dynamicMeta,
                MTK_3A_FEATURE_AE_SENSOR_GAIN_VALUE, sensorGain)) {
            ainrLogW("cannot get MTK_3A_FEATURE_AE_SENSOR_GAIN_VALUE");
        }

        int32_t ispGain = 0;
        if (!IMetadata::getEntry<MINT32>(dynamicMeta,
                MTK_3A_FEATURE_AE_ISP_GAIN_VALUE, ispGain)) {
            ainrLogW("cannot get MTK_3A_FEATURE_AE_ISP_GAIN_VALUE");
        }

        int32_t requestNum = 0;
        if (!IMetadata::getEntry<MINT32>(pHalMeta,
                MTK_PIPELINE_REQUEST_NUMBER, requestNum)) {
            ainrLogW("Get MTK_PIPELINE_REQUEST_NUMBER fail");
        }

        int32_t magicNum = 0;
        if (!IMetadata::getEntry<MINT32>(pHalMeta,
                MTK_P1NODE_PROCESSOR_MAGICNUM, magicNum)) {
            ainrLogW("Get MTK_P1NODE_PROCESSOR_MAGICNUM fail");
        }

        ainrLogD("M(%d), R(%d), Shutter(%" PRId64 " ns), sensorGain(%d), dgnGain(%d)",
                magicNum, requestNum, shutterTime, sensorGain, ispGain);
        mv_ExpInfos.push_back(std::make_tuple(shutterTime, sensorGain, ispGain));

        int32_t evLevel = MTK_3A_FEATURE_AE_EXPOSURE_LEVEL_NONE;
        if (index != m_bssIndex) {
            if (!IMetadata::getEntry<MINT32>(pHalMeta,
                    MTK_3A_FEATURE_AE_EXPOSURE_LEVEL, evLevel)) {
                ainrLogW("Get MTK_P1NODE_PROCESSOR_MAGICNUM fail");
            }
        }
        mv_IndexEvPairs.push_back(std::make_pair(index, evLevel));

        // Update index to next run
        ++index;
    }
    std::sort(mv_IndexEvPairs.begin(), mv_IndexEvPairs.end(),
                [](const std::pair<int, int> &a,
                const std::pair<int, int> &b) -> bool {
                    // compare ev level
                    if (a.second != b.second) {
                        return a.second < b.second;
                    }

                    // compare index if ev level is equal
                    return a.first < b.first;
                });

    for (const std::pair<int, int> &item : mv_IndexEvPairs) {
        ainrLogD("Index(%d), evValue(%d)", item.first, item.second);
    }
}

void AinrCore::calFramesToneAligned()
{
    auto getNEIndex = [this]() -> int32_t {
        int index = 0;
        for (const auto &info : m_vDataPackage) {
            IMetadata* pHalMeta = info.halMeta;
            int32_t evType = 0;
            if (!IMetadata::getEntry<MINT32>(pHalMeta,
                    MTK_3A_FEATURE_AE_EXPOSURE_LEVEL, evType)) {
                ainrLogW("Get MTK_3A_FEATURE_AE_EXPOSURE_LEVEL fail");
            }

            if (evType == MTK_3A_FEATURE_AE_EXPOSURE_LEVEL_NORMAL) {
                return index;
            }

            index++;
        }

        // not found NE frame
        return -1;
    };

    int32_t targetIndex = getNEIndex();
    if (targetIndex == -1) {
        ainrLogW("Get targetIndex fail");
        return;
    }
    ainrLogD("Aligned target index(%d)", targetIndex);

    auto [targetShutterTime, targetSensorGain, targetIspGain]
            = mv_ExpInfos[targetIndex];
    AinrAeInfo targetAE = {};
    targetAE.exposureTime = targetShutterTime;
    targetAE.sensorGain = targetSensorGain;
    targetAE.ispGain = targetIspGain;

    for (auto &expInfo : mv_ExpInfos) {
        auto [shutterTime, sensorGain, ispGain]
            = expInfo;
        AinrAeInfo currentAE = {};
        currentAE.exposureTime = shutterTime;
        currentAE.sensorGain = sensorGain;
        currentAE.ispGain = ispGain;

        ToneAlignedMatrix matrix;
        m_aiAlgo->calToneMatrix(currentAE, targetAE, &matrix);
        mv_toneAlignedMatrixs.push_back(matrix);
    }
    ainrLogD("toneMartrix container size(%d)", mv_toneAlignedMatrixs.size());

}

void AinrCore::parsingMeta()
{
    m_bssIndex = bssFrameDecision();
    m_mainFrameIndex = mainFrameDecision(m_NvramPtr);
    ainrLogD("bssIndex(%d) mainFrameIndex(%d)", m_bssIndex, m_mainFrameIndex);

    if (m_algoType == AIHDR) {
        recordExpInfo();
        calFramesToneAligned();
    }

    // On device tuning for DGN gain and OB ofst
    if (m_onlineTuning == ONLINE_DEVICE_MODE_ENABLE) {
        m_dgnGain = ::property_get_int32("vendor.ainr.dgnGain", 4928);
        m_obFst = ::property_get_int32("vendor.ainr.obofst", 256);
    }
    ainrLogD("DGN gain(%d) obOffset(%d)", m_dgnGain, m_obFst);

    // Wait FEFM buffers
    {
        // Because we use conditional wait which changes mutex
        // state need to use unique_lock
        CAM_ULOGM_TAGLIFE("wait_fefm_wk_buf");
        std::unique_lock<std::mutex> _lk(m_fefmMx);
        if (!mb_AllocfefmDone) {
            ainrLogD("Wait FEFM allocation....");
            m_fefmCond.wait(_lk);
        }
    }

    // Parsing feo if needed
    auto size = m_vDataPackage.size();
    for (int i = 0; i < size; ++i) {
        IMetadata::Memory feoInfo = {};
        auto pHalMeta = m_vDataPackage[i].halMeta;
        if (IMetadata::getEntry<IMetadata::Memory>(pHalMeta,
                MTK_ISP_FEO_INFO, feoInfo)) {
            ainrLogD("We already has feo, copy it!");
            auto feoBuf = m_vFeo[i];
            size_t feoTotalSize = 0, feoBufSize = 0;
            for (size_t i = 0; i < feoBuf->getPlaneCount(); ++i) {
                feoTotalSize += feoBuf->getBufSizeInBytes(i);
            }

            // structure check
            constexpr auto feoInfoSize = sizeof(IAinrFeFm::FeoInfo);
            if (CC_UNLIKELY(feoInfoSize != feoInfo.size())) {
                ainrLogF("FeoInfo size not sync(%zu, %zu)",
                        feoInfoSize, feoInfo.size());
                return;
            }

            // FEO size check
            IAinrFeFm::FeoInfo *infoPtr
                            = reinterpret_cast<IAinrFeFm::FeoInfo*>
                            (feoInfo.editArray());
            if (CC_UNLIKELY(feoTotalSize != infoPtr->feoTotalSize)) {
                ainrLogF("Feo size not sync(%d, %d)",
                        feoTotalSize, infoPtr->feoTotalSize);
                return;
            }

            IMetadata::Memory feoData = {};
            if (!IMetadata::getEntry<IMetadata::Memory>(pHalMeta,
                    MTK_ISP_FEO_DATA, feoData)) {
                ainrLogF("cannot get MTK_ISP_FEO_DATA");
                return;
            }

            // Check IMemory data
            if (CC_UNLIKELY(feoTotalSize != feoData.size())) {
                ainrLogF("Feo size not sync with meta(%d, %d)",
                        feoTotalSize, feoData.size());
                return;
            }

            memcpy(reinterpret_cast<void*>(feoBuf->getBufVA(0)),
                    reinterpret_cast<void*>(feoData.editArray()),
                    feoTotalSize);
            feoBuf->syncCache();
            m_bFeoReady = true;
            ainrLogD("Feo copy done");
        } else {
            ainrLogD("there is no feo information");
        }
    }

    // need to set up dumpHint
    if (m_DumpWorking) {
        mv_dumpNamingHints.resize(size);
        for (auto i = 0; i < m_vDataPackage.size(); i++) {
            IMetadata* pHalMeta = m_vDataPackage[i].halMeta;
            if (pHalMeta == nullptr) {
                ainrLogF("pHalMeta is nullptr in dump stage");
                return;
            }

            FILE_DUMP_NAMING_HINT* pHint
                    = &mv_dumpNamingHints[i];
            // Extract hal metadata and fill up file name;
            extract(pHint, pHalMeta);
            pHint->FrameNo = i;
        }
    }
}

int32_t AinrCore::bssFrameDecision() {
    int32_t bssFrame = 0;
    // AINR bss is in frame 0, because BSS has already reorded frame
    if (m_algoType == AIHDR) {
        for (int i = 0; i < m_vDataPackage.size(); i++) {
            auto halMeta = m_vDataPackage[i].halMeta;
            if (CC_LIKELY(halMeta)) {
                MBOOL isGolden = MFALSE;
                if (IMetadata::getEntry<MBOOL>(halMeta,
                        MTK_FEATURE_BSS_ISGOLDEN, isGolden)) {
                    if (isGolden) {
                        ainrLogD("Golden, index(%d)", i);
                        bssFrame = i;
                    } else {
                        ainrLogD("Not golden, index(%d)", i);
                    }
                } else {
                    ainrLogW("There is no MTK_FEATURE_BSS_ISGOLDEN");
                } // Query metadata "MTK_FEATURE_BSS_ISGOLDEN"
            }
        }
    }

    ainrLogD("BSS golden(%d)", bssFrame);
    return bssFrame;
}

int32_t AinrCore::mainFrameDecision(std::shared_ptr<IAinrNvram> spNvramPtr) {
    if (CC_UNLIKELY(spNvramPtr.get() == nullptr)) {
        ainrLogD("cannot do mainFrame decision use bssGolden");
        return m_bssIndex;
    }

    size_t lvNvramSize = 0;
    using nvramType = FEATURE_NVRAM_AIISP_LV_Pre_T;
    auto hint = IAinrNvram::nvram_hint::AIISP_LV_PRE;
    const nvramType* lvNvram
            = reinterpret_cast<const nvramType*>(
            spNvramPtr->getSpecificChunk(hint, &lvNvramSize));
    if (lvNvram == nullptr) {
        ainrLogW("lvNvram is nullptr use golden");
        return m_bssIndex;
    }

    auto postIndex = lvNvram->iso_atms_post_idx.val;
    if (postIndex < m_captureNum) {
        return postIndex;
    }

    ainrLogD("Main frame should use bss");
    return m_bssIndex;
}

AINRCORE_VERSION AinrCore::queryVersion() const {
    return AINRCORE_VERSION_2_0;
}

int32_t AinrCore::queryMainFrameIndex() const {
    return m_mainFrameIndex;
}
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
#define LOG_TAG "AinrCore/Nvram"

#include "AinrNvram.h"
#include <mtkcam3/feature/ainr/AinrUlog.h>

#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/aaa/INvBufUtil.h>
#include <mtkcam/aaa/IHalISP.h>

#include <unordered_map>  // std::unordered_map
#include <deque>  // std::deque

#include <utils/String8.h>


using android::sp;
using NS3Av3::IHalISP;
using NSCam::IHalSensorList;
using NSCam::SENSOR_DEV_NONE;

using namespace ainr;
using namespace NSIspTuning;

using android::String8;

//
#define NVRAM_AIISP_EV_SIZE sizeof(FEATURE_NVRAM_AIISP_EV_T)
#define NVRAM_AIISP_LV_SIZE sizeof(FEATURE_NVRAM_AIISP_LV_Pre_T)
#define NVRAM_AIISP_ISO_FEFM_T_SIZE sizeof(FEATURE_NVRAM_AIISP_ISO_FEFM_T)
#define NVRAM_AIISP_ISO_SWME_T_SIZE sizeof(FEATURE_NVRAM_AIISP_ISO_SWME_T)
#define NVRAM_AIISP_ISO_APU_Part1_SIZE sizeof(FEATURE_NVRAM_AIISP_ISO_APU_Part1_T)
#define NVRAM_AIISP_ISO_APU_Part2_SIZE sizeof(FEATURE_NVRAM_AIISP_ISO_APU_Part2_T)
#define NVRAM_AIISP_ISO_APU_Part3_SIZE sizeof(FEATURE_NVRAM_AIISP_ISO_APU_Part3_T)
#define NVRAM_AIISP_ISO_POSTSW_SIZE sizeof(FEATURE_NVRAM_AIISP_ISO_PostSW_T)
#define NVRAM_AIISP_ISO_DRC_SIZE sizeof(FEATURE_NVRAM_AIISP_ISO_DRC_T)


//-----------------------------------------------------------------------------
// IAinrNvram methods
//-----------------------------------------------------------------------------
std::shared_ptr<IAinrNvram> IAinrNvram::createInstance()
{
    std::shared_ptr<IAinrNvram> pAinrNvram = std::make_shared<AinrNvram>();
    return pAinrNvram;
}
//-----------------------------------------------------------------------------
// we save NVRAM chunk memory to a std::unordered_map
// for the next time we're using, looking up sensor ID and return the memory
// chunk as soon as possible. Using memory trade with performance (maybe lol)

//-----------------------------------------------------------------------------

AinrNvram::AinrNvram()
    : m_ispProfile(EIspProfile_AIISP_Capture_Before_Blend)
{
    CAM_ULOGM_FUNCLIFE_ALWAYS();
}

AinrNvram::~AinrNvram()
{
    CAM_ULOGM_FUNCLIFE_ALWAYS();
}

enum AinrErr AinrNvram::init(int sensorId)
{
    IAinrNvram::ConfigParams params;
    params.iSensorId = sensorId;
    params.bFlashOn  = false;

    ainrLogW("It is not suggested to init NVRAM 2.0 using only sensorId");

    return init(params);
}
//-----------------------------------------------------------------------------
enum AinrErr AinrNvram::init(IAinrNvram::ConfigParams& params)
{
    enum AinrErr err = AinrErr_Ok;
    String8 strLog;
    int openId = params.iSensorId;

    if (openId < 0) {
        ainrLogF("%s: sensor ID must >= 0", __FUNCTION__);
        return AinrErr_UnexpectedError;
    }

    uint32_t sensorDev = SENSOR_DEV_NONE;
    IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
    if (CC_UNLIKELY(pHalSensorList == nullptr)) {
        ainrLogF("%s: get IHalSensorList instance failed", __FUNCTION__);
        return AinrErr_UnexpectedError;
    }
    sensorDev = pHalSensorList->querySensorDevIdx(openId);

    if (params.featureType == AIHDR) {
        m_ispProfile = EIspProfile_AIISP_Capture_Before_Blend_16b;
    } else {
        m_ispProfile = EIspProfile_AIISP_Capture_Before_Blend;
    }

    auto pNvBufUtil = MAKE_NvBufUtil();
    if (pNvBufUtil == nullptr) {
        ainrLogF("pNvBufUtil is nullptr");
        return AinrErr_UnexpectedError;
    }

    // Get Main NVRAM Handle
    NVRAM_CAMERA_FEATURE_STRUCT *pNvram = nullptr;
    auto result = pNvBufUtil->getBufAndRead(
        CAMERA_NVRAM_DATA_FEATURE,
        sensorDev, (void*&)pNvram);
    if (result != 0 || pNvram == nullptr) {
        ainrLogF("read buffer chunk fail");
        return AinrErr_UnexpectedError;
    }

    #define LOAD_NVRAM_CHUNK(MODULE_ID, SIZE, MAX_NUM, MEM_CHUNK)           \
        {                                                                   \
            std::shared_ptr<char> chunk(new char[SIZE]                      \
                , std::default_delete<char[]>());                           \
            if (CC_UNLIKELY(chunk.get() == nullptr)) {                     \
                ainrLogF("Allocate NVRAM CHUNK fail(%d)", MODULE_ID);       \
                return AinrErr_UnexpectedError;                             \
            }                                                               \
                                                                            \
            int index = doQueryIndex(sensorDev, MODULE_ID);                 \
            if (index < 0 || index >= MAX_NUM) {                            \
                ainrLogF("index(%d) is not legal. MAX_NUM(%d)"              \
                            , index, MAX_NUM);                              \
            }                                                               \
            strLog += String8::format("%s tuning index(%d) "                \
                            , #MEM_CHUNK, index);                           \
                                                                            \
            void *nvChunk                                                   \
                = reinterpret_cast<void*>(&pNvram->MEM_CHUNK[index]);       \
                                                                            \
           memcpy(reinterpret_cast<void*>(chunk.get())                      \
                    , nvChunk, SIZE);                                       \
            m_tuning_map.emplace(MODULE_ID, chunk);                         \
        }

    LOAD_NVRAM_CHUNK(NSIspTuning::EModule_AIISP_EV
                        , NVRAM_AIISP_EV_SIZE
                        , NVRAM_AIISP_EV_TBL_NUM
                        , AIISP_EV);
    LOAD_NVRAM_CHUNK(NSIspTuning::EModule_AIISP_LV_Pre
                        , NVRAM_AIISP_LV_SIZE
                        , NVRAM_AIISP_LV_Pre_TBL_NUM
                        , AIISP_LV_Pre);
    #undef LOAD_NVRAM_CHUNK
    ainrLogD("Dump: %s", strLog.string());

    int32_t iso = params.iso;

    #define LOAD_INTERPOLATION_CHUNK(OPEN_ID, MODULE_ID, SIZE, ISO)         \
        {                                                                   \
            std::shared_ptr<char> chunk =                                   \
                    loadInterpolationChunk(OPEN_ID, MODULE_ID, SIZE, ISO);  \
            if (CC_UNLIKELY(chunk.get() == nullptr)) {                     \
                ainrLogF("Allocate NVRAM CHUNK fail(%d)", MODULE_ID);       \
                return AinrErr_UnexpectedError;                             \
            }                                                               \
            m_tuning_map.emplace(MODULE_ID, chunk);                         \
        }

    LOAD_INTERPOLATION_CHUNK(openId, NSIspTuning::EModule_AIISP_ISO_FEFM,
            NVRAM_AIISP_ISO_FEFM_T_SIZE, iso);
    LOAD_INTERPOLATION_CHUNK(openId, NSIspTuning::EModule_AIISP_ISO_APU_Part1,
            NVRAM_AIISP_ISO_APU_Part1_SIZE, iso);
    LOAD_INTERPOLATION_CHUNK(openId, NSIspTuning::EModule_AIISP_ISO_PostSW,
            NVRAM_AIISP_ISO_POSTSW_SIZE, iso);
    LOAD_INTERPOLATION_CHUNK(openId, NSIspTuning::EModule_AIISP_ISO_DRC,
            NVRAM_AIISP_ISO_DRC_SIZE, iso);
    #undef LOAD_INTERPOLATION_CHUNK

    return err;
}

int32_t AinrNvram::doQueryIndex(uint32_t sensorDev, EModule_T moduleId) {
    // IdxMgr is singleton pattern, no need to invoke destroyInstance
    // actually, IdxMgr has no destroyInstance API indeed...
    IdxMgr* pMgr = IdxMgr::createInstance(static_cast<ESensorDev_T>(sensorDev));
    if (CC_UNLIKELY(pMgr == nullptr)) {
        ainrLogF("%s: create IdxMgr instance failed", __FUNCTION__);
        return -1;
    }

    CAM_IDX_QRY_COMB rMapping_Info;
    UINT32 magicNo =  UINT_MAX;  // Indicate the latest preview mapping info
    pMgr->getMappingInfo(static_cast<ESensorDev_T>(sensorDev), rMapping_Info, magicNo);

    rMapping_Info.eFlash = EFlash_No;
    rMapping_Info.eIspProfile = m_ispProfile;
    int32_t tuningIndex = -1;
    tuningIndex = pMgr->query(static_cast<ESensorDev_T>(sensorDev)
                                , moduleId, rMapping_Info, __FUNCTION__);
    return tuningIndex;
}

std::shared_ptr<char> AinrNvram::loadInterpolationChunk(int32_t openId,
        NSIspTuning::EModule_T moduleId, size_t size, int32_t iso) {
    // C++17 supports pointer array like "std::shared_ptr<char[]> ptr(new char[size]);"
    // But we prefer C++11 coding style here.
    std::shared_ptr<char> chunk(new char[size], std::default_delete<char[]>());
    if (CC_UNLIKELY(chunk.get() == nullptr)) {
        ainrLogF("Allocate NVRAM CHUNK fail(%d)", moduleId);
        return nullptr;
    }

    // RAII for IHalISP instance
    std::unique_ptr<IHalISP, std::function<void(IHalISP*)>>
            pHalISP = decltype(pHalISP)(
            MAKE_HalISP(openId, LOG_TAG),
            [](IHalISP* p){ if (p) p->destroyInstance(LOG_TAG);});
    if (CC_UNLIKELY(pHalISP.get() == nullptr)) {
        ainrLogF("pHalISP is nullptr");
        return nullptr;
    }

    CAM_IDX_QRY_COMB_WITH_ISO interpoInfo = {};
    interpoInfo.i4ISO = iso;
    interpoInfo.module = moduleId;
    interpoInfo.module_struct_size = size;

    uint32_t sensorDev = SENSOR_DEV_NONE;
    IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
    if (CC_UNLIKELY(pHalSensorList == nullptr)) {
        ainrLogF("%s: get IHalSensorList instance failed", __FUNCTION__);
        return nullptr;
    }
    sensorDev = pHalSensorList->querySensorDevIdx(openId);

    // Query mapping information
    IdxMgr* pMgr = IdxMgr::createInstance(static_cast<ESensorDev_T>(sensorDev));
    if (CC_UNLIKELY(pMgr == nullptr)) {
        ainrLogF("%s: create IdxMgr instance failed", __FUNCTION__);
        return nullptr;
    }

    UINT32 magicNo =  UINT_MAX;  // Indicate the latest preview mapping info
    CAM_IDX_QRY_COMB &mappingInfo = interpoInfo.mapping_info;
    pMgr->getMappingInfo(static_cast<ESensorDev_T>(sensorDev), mappingInfo, magicNo);

    // Setup mappingInfo
    mappingInfo.eFlash = EFlash_No;
    mappingInfo.eIspProfile = m_ispProfile;

    pHalISP->sendIspCtrl(NS3Av3::EISPCtrl_GetAINRParam,
            reinterpret_cast<MINTPTR>(&interpoInfo),
            reinterpret_cast<MINTPTR>(chunk.get()));


    return chunk;
}

const std::shared_ptr<char> AinrNvram::chunk(size_t *bufferSize) const {
    auto find = m_tuning_map.find(NSIspTuning::EModule_AIISP_ISO_APU_Part1);
    if(find == m_tuning_map.end()) {
        ainrLogF("Cannot find AIISP_ISO_APU_Part1 in hashTable");
        if (bufferSize) {
            *bufferSize = 0;
        }

        return nullptr;
    }

    if (bufferSize) {
        *bufferSize = NVRAM_AIISP_ISO_APU_Part1_SIZE;
    }

    return find->second;
}

const std::shared_ptr<char> AinrNvram::chunkAinrTh(size_t *bufferSize) const {
    auto find = m_tuning_map.find(NSIspTuning::EModule_AIISP_EV);
    if(find == m_tuning_map.end()) {
        ainrLogF("Cannot find EModule_AIISP_EV in hashTable");
        if (bufferSize) {
            *bufferSize = 0;
        }

        return nullptr;
    }

    if (bufferSize) {
        *bufferSize = NVRAM_AIISP_EV_SIZE;
    }

    return find->second;
}

const char* AinrNvram::getChunk(size_t *bufferSize) const {
    auto find = m_tuning_map.find(NSIspTuning::EModule_AIISP_ISO_APU_Part1);
    if(find == m_tuning_map.end()) {
        ainrLogF("Cannot find AIISP_ISO_APU_Part1 in hashTable");
        if (bufferSize) {
            *bufferSize = 0;
        }

        return nullptr;
    }

    if (bufferSize) {
        *bufferSize = NVRAM_AIISP_ISO_APU_Part1_SIZE;
    }

    auto chunk = find->second;
    return static_cast<const char*>(chunk.get());
}

const char* AinrNvram::getChunkAinrTh(size_t *bufferSize) const {
    auto find = m_tuning_map.find(NSIspTuning::EModule_AIISP_EV);
    if(find == m_tuning_map.end()) {
        ainrLogF("Cannot find AIISP_EV in hashTable");
        if (bufferSize) {
            *bufferSize = 0;
        }

        return nullptr;
    }

    if (bufferSize) {
        *bufferSize = NVRAM_AIISP_EV_SIZE;
    }

    auto chunk = find->second;
    return static_cast<const char*>(chunk.get());
}

const char* AinrNvram::getSpecificChunk(const nvram_hint hint
    , size_t* bufferSize) const {

    size_t size = 0;
    EModule_T moduleId = NSIspTuning::EModule_AIISP_ISO_APU_Part1;
    switch (hint) {
        case nvram_hint::AIISP_THRE:
            moduleId = NSIspTuning::EModule_AIISP_EV;
            size = NVRAM_AIISP_EV_SIZE;
            break;
        case nvram_hint::AIISP_LV_PRE:
            moduleId = NSIspTuning::EModule_AIISP_LV_Pre;
            size = NVRAM_AIISP_LV_SIZE;
            break;
        case nvram_hint::AIISP_ISO_FEFM:
            moduleId = NSIspTuning::EModule_AIISP_ISO_FEFM;
            size = NVRAM_AIISP_ISO_FEFM_T_SIZE;
            break;
        case nvram_hint::AIISP_ISO_APU_Part1:
            moduleId = NSIspTuning::EModule_AIISP_ISO_APU_Part1;
            size = NVRAM_AIISP_ISO_APU_Part1_SIZE;
            break;
        case nvram_hint::AIISP_ISO_POSTSW:
            moduleId = NSIspTuning::EModule_AIISP_ISO_PostSW;
            size = NVRAM_AIISP_ISO_POSTSW_SIZE;
            break;
        case nvram_hint::AIISP_ISO_DRC:
            moduleId = NSIspTuning::EModule_AIISP_ISO_DRC;
            size = NVRAM_AIISP_ISO_DRC_SIZE;
            break;
        case nvram_hint::AIISP_ISO_APU_Part2:
        case nvram_hint::AIISP_ISO_APU_Part3:
        case nvram_hint::AIISP_ISO_SWME:
        default:
            ainrLogF("Undetermined AI algo type(%d)"
                , static_cast<int32_t>(hint));
            break;
    }

    auto find = m_tuning_map.find(moduleId);
    if(find == m_tuning_map.end()) {
        ainrLogF("Cannot find AIISP_EV in hashTable");
        if (bufferSize) {
            *bufferSize = 0;
        }

        return nullptr;
    }

    if (bufferSize) {
        *bufferSize = size;
    }

    auto chunk = find->second;
    return static_cast<const char*>(chunk.get());
}

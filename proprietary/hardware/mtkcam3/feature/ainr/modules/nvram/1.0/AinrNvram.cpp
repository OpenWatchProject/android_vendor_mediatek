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
#define LOG_TAG "AinrCore/Nvram"

#include "AinrNvram.h"
#include <mtkcam3/feature/ainr/AinrUlog.h>

#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/aaa/INvBufUtil.h>
#include <mtkcam/aaa/IHalISP.h>
#include <mtkcam/utils/std/StlUtils.h> // NSCam::SpinLock

// ISP profile
#include <tuning_mapping/cam_idx_struct_ext.h>

#include <unordered_map> // std::unordered_map
#include <deque> // std::deque

using android::sp;
using android::Mutex;

using NSCam::IHalSensorList;
using NSCam::SENSOR_DEV_NONE;
using NS3Av3::IHalISP;

using namespace ainr;
using namespace NSIspTuning;

#define NVRAM_SIZE sizeof(FEATURE_NVRAM_AINR_T)
#define NVRAM_ENCODE_KEY(id, idx) (id*NVRAM_AINR_TBL_NUM + idx)
//
#define NVRAM_AINRTH_SIZE sizeof(NVRAM_CAMERA_FEATURE_AINR_THRES_STRUCT)
#define NVRAM_AINRTH_ENCODE_KEY(id, idx) (id*NVRAM_AINR_THRES_TBL_NUM + idx)

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
// TO-DO:
//   For NVRAM2.0, ths map size may rise to 20~40 (maybe more).
//   if need be, please consider to apply better mechanism for memory saving.
static NSCam::SpinLock s_nvram_chunks_mx;
static std::unordered_map< int, std::shared_ptr<char> > s_nvram_chunks;
// Threshold
static std::unordered_map< int, std::shared_ptr<char> > s_nvram_chunkAinrTh;

//-----------------------------------------------------------------------------
static bool doQueryIndex(IAinrNvram::ConfigParams& params)
{
    params.queryIndexAinr   = -1;
    params.queryIndexAinrTh = -1;

    MUINT sensorDev = SENSOR_DEV_NONE;
    {
        IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
        if (CC_UNLIKELY(pHalSensorList == NULL)) {
            ainrLogE("%s: get IHalSensorList instance failed", __FUNCTION__);
            return false;
        }
        sensorDev = pHalSensorList->querySensorDevIdx(params.iSensorId);
    }

    // IdxMgr is singleton pattern, no need to invoke destroyInstance
    // actually, IdxMgr has no destroyInstance API indeed...
    IdxMgr* pMgr = IdxMgr::createInstance(static_cast<ESensorDev_T>(sensorDev));
    if (CC_UNLIKELY(pMgr == NULL)) {
        ainrLogE("%s: create IdxMgr instance failed", __FUNCTION__);
        return false;
    }
    {
        CAM_IDX_QRY_COMB rMapping_Info;
        UINT32 magicNo =  UINT_MAX; // if the magic number cannot be found, MappingInfoMgr
                                    // will use the latest result to query.
        pMgr->getMappingInfo(static_cast<ESensorDev_T>(sensorDev), rMapping_Info, magicNo);

        // force adjust flash information because we knew it (we don't trush MaggingInfoMgr, sometimes).
        rMapping_Info.eFlash        = EFlash_No;
        rMapping_Info.eIspProfile   = EIspProfile_AINR_Main;

        params.queryIndexAinr   = pMgr->query(static_cast<ESensorDev_T>(sensorDev), NSIspTuning::EModule_AINR, rMapping_Info, __FUNCTION__);
        params.queryIndexAinrTh = pMgr->query(static_cast<ESensorDev_T>(sensorDev), NSIspTuning::EModule_AINR_THRES, rMapping_Info, __FUNCTION__);
    }

    ainrLogD("mappingInfo sensorId(%d), flashOn(%d) to ainrIndex: %d and ainrTh: %d", params.iSensorId, params.bFlashOn
                                                                                    , params.queryIndexAinr, params.queryIndexAinrTh);
    return true;
}
//-----------------------------------------------------------------------------
static std::shared_ptr<char> get_nvram_chunk(IAinrNvram::ConfigParams const& params)
{

    if (params.iSensorId< 0) {
        ainrLogE("%s: sensorId < 0", __FUNCTION__);
        return nullptr;
    }

    if (CC_UNLIKELY(params.queryIndexAinr < 0 || params.queryIndexAinr >= NVRAM_AINR_TBL_NUM)) {
        ainrLogE("%s: nvramIndex invalid(%d)", __FUNCTION__, params.queryIndexAinr);
        return nullptr;
    }

    int key = NVRAM_ENCODE_KEY(params.iSensorId, params.queryIndexAinr);

    std::deque<const char*> errMsgs;

    { // SpinLock locked
        std::lock_guard<NSCam::SpinLock> __l(s_nvram_chunks_mx);
        {
        //if (s_nvram_chunks[key].get() == nullptr) {
            size_t chunkSize = NVRAM_SIZE;
            std::shared_ptr<char> chunk(new char[chunkSize]);
            NVRAM_CAMERA_FEATURE_STRUCT *pNvram;
            MUINT sensorDev = SENSOR_DEV_NONE;
            {
                IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
                if (pHalSensorList == NULL) {
                    errMsgs.push_back("get IHalSensorList instance failed");
                    goto lbErr;
                }
                sensorDev = pHalSensorList->querySensorDevIdx(params.iSensorId);
            }

            auto pNvBufUtil = MAKE_NvBufUtil();
            if (pNvBufUtil == NULL) {
                errMsgs.push_back("pNvBufUtil==0");
                goto lbErr;
            }
            auto result = pNvBufUtil->getBufAndRead(
                    CAMERA_NVRAM_DATA_FEATURE,
                    sensorDev, (void*&)pNvram);
            if (result != 0) {
                errMsgs.push_back("read buffer chunk fail");
                goto lbErr;
            }

            // IdxMgr is singleton pattern, no need to invoke destroyInstance
            // actually, IdxMgr has no destroyInstance API indeed...
            UINT32 magicNo =  UINT_MAX;

            IdxMgr* pMgr = IdxMgr::createInstance(static_cast<ESensorDev_T>(sensorDev));

            if (CC_UNLIKELY(pMgr == NULL)) {
                ainrLogE("%s: create IdxMgr instance failed", __FUNCTION__);
                goto lbErr;
            }

            CAM_IDX_QRY_COMB_WITH_ISO interpoInfo;
            pMgr->getMappingInfo(static_cast<ESensorDev_T>(sensorDev), interpoInfo.mapping_info, magicNo);

            interpoInfo.mapping_info.eFlash        = EFlash_No;
            interpoInfo.mapping_info.eIspProfile   = EIspProfile_AINR_Main;
            interpoInfo.i4ISO                      = params.iso;
            ainrLogD("Current iso(%d)", interpoInfo.i4ISO);

            /* RAII for IHalISP instance */
            std::unique_ptr< IHalISP, std::function<void(IHalISP*)> > pHalISP = decltype(pHalISP)(
                    MAKE_HalISP(params.iSensorId, LOG_TAG),
                    [](IHalISP* p){ if (p) p->destroyInstance(LOG_TAG); }
            );

            pHalISP->sendIspCtrl(NS3Av3::EISPCtrl_GetAINRParam, reinterpret_cast<MINTPTR>(&interpoInfo), reinterpret_cast<MINTPTR>(chunk.get()));
            ainrLogD("Read FEATURE_NVRAM_AINR_T, size=%zu(byte)", chunkSize);
            //s_nvram_chunks[key] = chunk;
            //return s_nvram_chunks[key];
            return chunk;
        }
    #if 0
        else {
            return s_nvram_chunks[key];
        }
    #endif
    }

lbErr:
    for (auto itr : errMsgs)
        ainrLogE("%s: %s", __FUNCTION__, itr);
    return nullptr;
}
//-----------------------------------------------------------------------------

static std::shared_ptr<char> get_nvram_chunkAinrTh(IAinrNvram::ConfigParams& params)
{
    if (params.iSensorId< 0) {
        ainrLogE("%s: sensorId < 0", __FUNCTION__);
        return nullptr;
    }

    if (CC_UNLIKELY(params.queryIndexAinrTh < 0 || params.queryIndexAinrTh >= NVRAM_AINR_THRES_TBL_NUM)) {
        ainrLogE("%s: nvramIndex invalid: AinrTh(%d)", __FUNCTION__, params.queryIndexAinrTh);
        params.queryIndexAinrTh = -1;
        return nullptr;
    }

    int key = NVRAM_AINRTH_ENCODE_KEY(params.iSensorId, params.queryIndexAinrTh);

    std::deque<const char*> errMsgs;

    { // SpinLock locked
        std::lock_guard<NSCam::SpinLock> __l(s_nvram_chunks_mx);
        if (s_nvram_chunkAinrTh[key].get() == nullptr) {
            size_t chunkSize = NVRAM_AINRTH_SIZE;
            std::shared_ptr<char> chunk(new char[chunkSize]);
            NVRAM_CAMERA_FEATURE_STRUCT *pNvram;
            MUINT sensorDev = SENSOR_DEV_NONE;
            {
                IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
                if (pHalSensorList == NULL) {
                    errMsgs.push_back("get IHalSensorList instance failed");
                    goto lbErr;
                }
                sensorDev = pHalSensorList->querySensorDevIdx(params.iSensorId);
            }

            auto pNvBufUtil = MAKE_NvBufUtil();
            if (pNvBufUtil == NULL) {
                errMsgs.push_back("pNvBufUtil==0");
                goto lbErr;
            }
            auto result = pNvBufUtil->getBufAndRead(
                    CAMERA_NVRAM_DATA_FEATURE,
                    sensorDev, (void*&)pNvram);
            if (result != 0) {
                errMsgs.push_back("read buffer chunk fail");
                goto lbErr;
            }

            memcpy((void*)chunk.get(), (void*)&pNvram->AINR_THRES[params.queryIndexAinrTh], chunkSize);
            ainrLogD("Read NVRAM_CAMERA_FEATURE_MFNR_THRES_STRUCT, size=%zu(byte)", chunkSize);
            s_nvram_chunkAinrTh[key] = chunk;
            return s_nvram_chunkAinrTh[key];
        }
        else {
            return s_nvram_chunkAinrTh[key];
        }
    }

lbErr:
    for (auto itr : errMsgs)
        ainrLogE("%s: %s", __FUNCTION__, itr);
    return nullptr;
}

//-----------------------------------------------------------------------------
static void clear_nvram_chunks()
{
    std::unordered_map< int, std::shared_ptr<char> > st;
    std::unordered_map< int, std::shared_ptr<char> > stTh;
    {
        std::lock_guard<NSCam::SpinLock> __l(s_nvram_chunks_mx);
        st   = std::move(s_nvram_chunks);
        stTh = std::move(s_nvram_chunkAinrTh);
    }
}
//-----------------------------------------------------------------------------
// AinrNvram implementation
//-----------------------------------------------------------------------------
AinrNvram::AinrNvram()
{
    CAM_ULOGM_FUNCLIFE_ALWAYS();
}
//-----------------------------------------------------------------------------
AinrNvram::~AinrNvram()
{
    CAM_ULOGM_FUNCLIFE_ALWAYS();
}
//-----------------------------------------------------------------------------
enum AinrErr AinrNvram::init(int sensorId)
{
    ConfigParams params;
    params.iSensorId = sensorId;
    params.bFlashOn  = false;

    ainrLogW("%s: It is not suggested to init NVRAM 2.0 using only sensorId", __FUNCTION__);

    return init(params);
}
//-----------------------------------------------------------------------------
enum AinrErr AinrNvram::init(IAinrNvram::ConfigParams& params)
{
    enum AinrErr err = AinrErr_Ok;

    Mutex::Autolock _l(&m_mutex);

    if (params.iSensorId < 0) {
        ainrLogE("%s: sensor ID must >= 0", __FUNCTION__);
        return AinrErr_UnexpectedError;
    }

    if (!doQueryIndex(params)) {
        return AinrErr_UnexpectedError;
    }

    if ((m_nvramChunk.get() == nullptr)) {
        m_nvramChunk = get_nvram_chunk(params);
    }

    if ((m_nvramChunkAinrTh.get() == nullptr)) {
        m_nvramChunkAinrTh = get_nvram_chunkAinrTh(params);
    }

lbExit:
    return err;
}
//-----------------------------------------------------------------------------
const std::shared_ptr<char> AinrNvram::chunk(size_t *bufferSize) const
{
    if (getChunk(bufferSize) != NULL) {
        size_t size = NVRAM_SIZE;
        std::shared_ptr<char> __chunk(new char[size]);
        memcpy((void*)__chunk.get(), (void*)m_nvramChunk.get(), size);
        return __chunk;
    }
    return NULL;
}
//-----------------------------------------------------------------------------
const std::shared_ptr<char> AinrNvram::chunkAinrTh(size_t *bufferSize) const
{
    if (getChunk(bufferSize) != NULL) {
        size_t size = NVRAM_SIZE;
        std::shared_ptr<char> __chunk(new char[size]);
        memcpy((void*)__chunk.get(), (void*)m_nvramChunkAinrTh.get(), size);
        return __chunk;
    }
    return NULL;
}
//-----------------------------------------------------------------------------
const char* AinrNvram::getChunk(size_t *bufferSize) const
{
    if (m_nvramChunk.get() == NULL) {
        ainrLogE("%s: please init IAinrNvram first", __FUNCTION__);
        if (bufferSize)
            *bufferSize = 0;
        return NULL;
    }
    else {
        if (bufferSize)
            *bufferSize = NVRAM_SIZE;
    }

    return static_cast<const char*>(m_nvramChunk.get());
}
//-----------------------------------------------------------------------------
const char* AinrNvram::getChunkAinrTh(size_t *bufferSize) const
{
    if (m_nvramChunkAinrTh.get() == NULL) {
        ainrLogE("%s: please init IAinrNvram first", __FUNCTION__);
        if (bufferSize)
            *bufferSize = 0;
        return NULL;
    }
    else {
        if (bufferSize)
            *bufferSize = NVRAM_SIZE;
    }

    return static_cast<const char*>(m_nvramChunkAinrTh.get());
}

const char* AinrNvram::getSpecificChunk(const nvram_hint hint
    , size_t* bufferSize) const {
    if (hint == nvram_hint::AIISP_ISO_APU_Part1) {
        return getChunk(bufferSize);
    } else if (hint == nvram_hint::AIISP_THRE) {
        return getChunkAinrTh(bufferSize);
    }

    return nullptr;
}

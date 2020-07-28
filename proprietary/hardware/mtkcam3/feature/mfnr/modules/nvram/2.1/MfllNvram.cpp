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
#define LOG_TAG "MfllCore/Nvram"

#include "MfllNvram.h"
#include <mtkcam3/feature/mfnr/MfllLog.h>

#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/aaa/INvBufUtil.h>
#include <mtkcam/utils/std/StlUtils.h> // NSCam::SpinLock

#include <unordered_map> // std::unordered_map
#include <deque> // std::deque

using android::sp;
using android::Mutex;

using NSCam::IHalSensorList;
using NSCam::SENSOR_DEV_NONE;
using namespace mfll;

#define NVRAM_MFNR_SIZE sizeof(NVRAM_CAMERA_FEATURE_MFLL_STRUCT)
#define NVRAM_MFNRTH_SIZE sizeof(NVRAM_CAMERA_FEATURE_MFNR_THRES_STRUCT)
#define NVRAM_MFNR_ENCODE_KEY(id, idx) (id*NVRAM_MFNR_TBL_NUM + idx)
#define NVRAM_MFNRTH_ENCODE_KEY(id, idx) (id*NVRAM_MFNR_THRES_TBL_NUM + idx)

//-----------------------------------------------------------------------------
// IMfllNvram methods
//-----------------------------------------------------------------------------
IMfllNvram* IMfllNvram::createInstance()
{
    return (IMfllNvram*)new MfllNvram;
}
//-----------------------------------------------------------------------------
void IMfllNvram::destroyInstance()
{
    decStrong((void*)this);
}
//-----------------------------------------------------------------------------
// we save NVRAM chunk memory to a std::unordered_map
// for the next time we're using, looking up sensor ID and return the memory
// chunk as soon as possible. Using memory trade with performance (maybe lol)
// TO-DO:
//   For NVRAM2.0, ths map size may rise to 20~40 (maybe more).
//   if need be, please consider to apply better mechanism for memory saving.
static NSCam::SpinLock s_nvram_chunks_mx;
static std::unordered_map< int, std::shared_ptr<char> > s_nvram_chunkMfnr;
static std::unordered_map< int, std::shared_ptr<char> > s_nvram_chunkMfnrTh;

//-----------------------------------------------------------------------------
static bool doQueryIndex(IMfllNvram::ConfigParams& params)
{
    params.iQueryIndex_Bss = -1;
    params.iQueryIndex_Mfnr = -1;
    params.iQueryIndex_MfnrTh = -1;

    MUINT sensorDev = SENSOR_DEV_NONE;
    {
        IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
        if (CC_UNLIKELY(pHalSensorList == NULL)) {
            mfllLogE("%s: get IHalSensorList instance failed", __FUNCTION__);
            return false;
        }
        sensorDev = pHalSensorList->querySensorDevIdx(params.iSensorId);
    }

    // IdxMgr is singleton pattern, no need to invoke destroyInstance
    // actually, IdxMgr has no destroyInstance API indeed...
    IdxMgr* pMgr = IdxMgr::createInstance(static_cast<ESensorDev_T>(sensorDev));
    if (CC_UNLIKELY(pMgr == NULL)) {
        mfllLogE("%s: create IdxMgr instance failed", __FUNCTION__);
        return false;
    }
    {
        CAM_IDX_QRY_COMB rMapping_Info;
        UINT32 magicNo =  UINT_MAX; // if the magic number cannot be found, MappingInfoMgr
                                    // will use the latest result to query.
        pMgr->getMappingInfo(static_cast<ESensorDev_T>(sensorDev), rMapping_Info, magicNo);

        // force adjust flash information because we knew it (we don't trush MaggingInfoMgr, sometimes).
        rMapping_Info.eFlash = (params.bFlashOn)?EFlash_MainFlash:EFlash_No;
        rMapping_Info.eIspProfile   = EIspProfile_MFNR_Before_Blend;
        params.iQueryIndex_Bss      = pMgr->query(static_cast<ESensorDev_T>(sensorDev), NSIspTuning::EModule_BSS, rMapping_Info, __FUNCTION__);
        params.iQueryIndex_Mfnr     = pMgr->query(static_cast<ESensorDev_T>(sensorDev), NSIspTuning::EModule_MFNR, rMapping_Info, __FUNCTION__);
        params.iQueryIndex_MfnrTh   = pMgr->query(static_cast<ESensorDev_T>(sensorDev), NSIspTuning::EModule_MFNR_THRES, rMapping_Info, __FUNCTION__);
    }

    mfllLogD3("mappingInfo sensorId(%d), flashOn(%d) to index BSS(%d), Mfnr(%d), MfnrTh(%d)"
        , params.iSensorId, params.bFlashOn, params.iQueryIndex_Bss, params.iQueryIndex_Mfnr, params.iQueryIndex_MfnrTh);

    return true;
}
//-----------------------------------------------------------------------------
static std::shared_ptr<char> get_nvram_chunkMfnr(IMfllNvram::ConfigParams& params)
{
    if (params.iSensorId< 0) {
        mfllLogE("%s: sensorId < 0", __FUNCTION__);
        return nullptr;
    }

    if (CC_UNLIKELY(params.iQueryIndex_Mfnr < 0 || params.iQueryIndex_Mfnr >= NVRAM_MFNR_TBL_NUM)) {
        mfllLogE("%s: nvramIndex invalid: Mfnr(%d)", __FUNCTION__, params.iQueryIndex_Mfnr);
        params.iQueryIndex_Mfnr = -1;
        return nullptr;
    }

    int key = NVRAM_MFNR_ENCODE_KEY(params.iSensorId, params.iQueryIndex_Mfnr);

    std::deque<const char*> errMsgs;

    { // SpinLock locked
        std::lock_guard<NSCam::SpinLock> __l(s_nvram_chunks_mx);
        if (s_nvram_chunkMfnr[key].get() == nullptr) {
            size_t chunkSize = NVRAM_MFNR_SIZE;
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

            memcpy((void*)chunk.get(), (void*)&pNvram->MFNR[params.iQueryIndex_Mfnr], chunkSize);
            mfllLogD3("%s: read NVRAM_CAMERA_FEATURE_MFLL_STRUCT, size=%zu(byte)",
                    __FUNCTION__, chunkSize);
            s_nvram_chunkMfnr[key] = chunk;
            return s_nvram_chunkMfnr[key];
        }
        else {
            return s_nvram_chunkMfnr[key];
        }
    }

lbErr:
    for (auto itr : errMsgs)
        mfllLogE("%s: %s", __FUNCTION__, itr);
    return nullptr;
}
//-----------------------------------------------------------------------------
static std::shared_ptr<char> get_nvram_chunkMfnrTh(IMfllNvram::ConfigParams& params)
{
    if (params.iSensorId< 0) {
        mfllLogE("%s: sensorId < 0", __FUNCTION__);
        return nullptr;
    }

    if (CC_UNLIKELY(params.iQueryIndex_MfnrTh < 0 || params.iQueryIndex_MfnrTh >= NVRAM_MFNR_THRES_TBL_NUM)) {
        mfllLogE("%s: nvramIndex invalid: MfnrTh(%d)", __FUNCTION__, params.iQueryIndex_MfnrTh);
        params.iQueryIndex_MfnrTh = -1;
        return nullptr;
    }

    int key = NVRAM_MFNRTH_ENCODE_KEY(params.iSensorId, params.iQueryIndex_MfnrTh);

    std::deque<const char*> errMsgs;

    { // SpinLock locked
        std::lock_guard<NSCam::SpinLock> __l(s_nvram_chunks_mx);
        if (s_nvram_chunkMfnrTh[key].get() == nullptr) {
            size_t chunkSize = NVRAM_MFNRTH_SIZE;
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

            memcpy((void*)chunk.get(), (void*)&pNvram->MFNR_THRES[params.iQueryIndex_MfnrTh], chunkSize);
            mfllLogD3("%s: read NVRAM_CAMERA_FEATURE_MFNR_THRES_STRUCT, size=%zu(byte)",
                    __FUNCTION__, chunkSize);
            s_nvram_chunkMfnrTh[key] = chunk;
            return s_nvram_chunkMfnrTh[key];
        }
        else {
            return s_nvram_chunkMfnrTh[key];
        }
    }

lbErr:
    for (auto itr : errMsgs)
        mfllLogE("%s: %s", __FUNCTION__, itr);
    return nullptr;
}
//-----------------------------------------------------------------------------
static void clear_nvram_chunks()
{
    std::unordered_map< int, std::shared_ptr<char> > st1;
    std::unordered_map< int, std::shared_ptr<char> > st2;
    {
        std::lock_guard<NSCam::SpinLock> __l(s_nvram_chunks_mx);
        st1 = std::move(s_nvram_chunkMfnr);
        st2 = std::move(s_nvram_chunkMfnrTh);
    }
}
//-----------------------------------------------------------------------------
// MfllNvram implementation
//-----------------------------------------------------------------------------
MfllNvram::MfllNvram()
{
}
//-----------------------------------------------------------------------------
MfllNvram::~MfllNvram()
{
}
//-----------------------------------------------------------------------------
enum MfllErr MfllNvram::init(int sensorId)
{
    ConfigParams params;
    params.iSensorId = sensorId;
    params.bFlashOn  = false;

    mfllLogW("%s: It is not suggested to init NVRAM 2.0 using only sensorId", __FUNCTION__);

    return init(params);
}
//-----------------------------------------------------------------------------
enum MfllErr MfllNvram::init(IMfllNvram::ConfigParams& params)
{
    enum MfllErr err = MfllErr_Ok;

    Mutex::Autolock _l(&m_mutex);

    if (params.iSensorId < 0) {
        mfllLogE("%s: sensor ID must >= 0", __FUNCTION__);
        return MfllErr_UnexpectedError;
    }

    if (!doQueryIndex(params)) {
        return MfllErr_UnexpectedError;
    }

    if ((m_nvramChunkMfnr.get() == nullptr)) {
        m_nvramChunkMfnr = get_nvram_chunkMfnr(params);
    }

    if ((m_nvramChunkMfnrTh.get() == nullptr)) {
        m_nvramChunkMfnrTh = get_nvram_chunkMfnrTh(params);
    }

lbExit:
    return err;
}
//-----------------------------------------------------------------------------
std::shared_ptr<char> MfllNvram::chunk(size_t *bufferSize)
{
    mfllLogD3("Not support chunk, pass to chunkMfnr");
    return chunkMfnr(bufferSize);
}
//-----------------------------------------------------------------------------
std::shared_ptr<char> MfllNvram::chunkMfnr(size_t *bufferSize)
{
    if (getChunk(bufferSize) != NULL) {
        size_t size = NVRAM_MFNR_SIZE;
        std::shared_ptr<char> __chunk(new char[size]);
        memcpy((void*)__chunk.get(), (void*)m_nvramChunkMfnr.get(), size);
        return __chunk;
    }
    return NULL;
}
//-----------------------------------------------------------------------------
std::shared_ptr<char> MfllNvram::chunkMfnrTh(size_t *bufferSize)
{
    if (getChunk(bufferSize) != NULL) {
        size_t size = NVRAM_MFNRTH_SIZE;
        std::shared_ptr<char> __chunk(new char[size]);
        memcpy((void*)__chunk.get(), (void*)m_nvramChunkMfnrTh.get(), size);
        return __chunk;
    }
    return NULL;
}
//-----------------------------------------------------------------------------
const char* MfllNvram::getChunk(size_t *bufferSize)
{
    mfllLogD3("Not support getChunk, pass to getChunkMfnr");
    return getChunkMfnr(bufferSize);
}
//-----------------------------------------------------------------------------
const char* MfllNvram::getChunkMfnr(size_t *bufferSize)
{
    if (m_nvramChunkMfnr.get() == NULL) {
        mfllLogE("%s: please init IMfllNvram first", __FUNCTION__);
        if (bufferSize)
            *bufferSize = 0;
        return NULL;
    }
    else {
        if (bufferSize)
            *bufferSize = NVRAM_MFNR_SIZE;
    }

    return static_cast<const char*>(m_nvramChunkMfnr.get());
}
//-----------------------------------------------------------------------------
const char* MfllNvram::getChunkMfnrTh(size_t *bufferSize)
{
    if (m_nvramChunkMfnrTh.get() == NULL) {
        mfllLogE("%s: please init IMfllNvram first", __FUNCTION__);
        if (bufferSize)
            *bufferSize = 0;
        return NULL;
    }
    else {
        if (bufferSize)
            *bufferSize = NVRAM_MFNRTH_SIZE;
    }

    return static_cast<const char*>(m_nvramChunkMfnrTh.get());
}

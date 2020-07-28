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

#define NVRAM_SIZE sizeof(NVRAM_CAMERA_FEATURE_MFLL_STRUCT)

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
static NSCam::SpinLock s_nvram_chunks_mx;
static std::unordered_map< int, std::shared_ptr<char> > s_nvram_chunks;

//-----------------------------------------------------------------------------
static std::shared_ptr<char> get_nvram_chunk(int sensorId)
{
    if (sensorId < 0) {
        mfllLogE("%s: sensorId < 0", __FUNCTION__);
        return nullptr;
    }

    std::deque<const char*> errMsgs;

    { // SpinLock locked
        std::lock_guard<NSCam::SpinLock> __l(s_nvram_chunks_mx);
        if (s_nvram_chunks[sensorId].get() == nullptr) {
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
                sensorDev = pHalSensorList->querySensorDevIdx(sensorId);
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

            memcpy((void*)chunk.get(), (void*)&pNvram->mfll, chunkSize);
            mfllLogD("%s: read NVRAM_CAMERA_FEATURE_MFLL_STRUCT, size=%zu(byte)",
                    __FUNCTION__, chunkSize);
            s_nvram_chunks[sensorId] = chunk;
            return s_nvram_chunks[sensorId];
        }
        else {
            return s_nvram_chunks[sensorId];
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
    std::unordered_map< int, std::shared_ptr<char> > st;
    {
        std::lock_guard<NSCam::SpinLock> __l(s_nvram_chunks_mx);
        st = std::move(s_nvram_chunks);
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
    enum MfllErr err = MfllErr_Ok;

    Mutex::Autolock _l(&m_mutex);

    if (sensorId < 0) {
        mfllLogE("%s: sensor ID must >= 0", __FUNCTION__);
        return MfllErr_UnexpectedError;
    }

    if ((m_nvramChunk.get() == nullptr)) {
        m_nvramChunk = get_nvram_chunk(sensorId);
    }

lbExit:
    return err;
}
//-----------------------------------------------------------------------------
enum MfllErr MfllNvram::init(IMfllNvram::ConfigParams& params)
{
    return init(params.iSensorId);
}
//-----------------------------------------------------------------------------
std::shared_ptr<char> MfllNvram::chunk(size_t *bufferSize)
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
std::shared_ptr<char> MfllNvram::chunkMfnr(size_t *bufferSize)
{
    mfllLogW("Not support chunkMfnr, pass to chunk");
    return chunk(bufferSize);
}
//-----------------------------------------------------------------------------
std::shared_ptr<char> MfllNvram::chunkMfnrTh(size_t *bufferSize)
{
    mfllLogW("Not support chunkMfnrTh");
    return NULL;
}
//-----------------------------------------------------------------------------
const char* MfllNvram::getChunk(size_t *bufferSize)
{
    if (m_nvramChunk.get() == NULL) {
        mfllLogE("%s: please init IMfllNvram first", __FUNCTION__);
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
const char* MfllNvram::getChunkMfnr(size_t *bufferSize)
{
    mfllLogW("Not support getChunkMfnr, pass to chunk");
    return getChunk(bufferSize);
}
//-----------------------------------------------------------------------------
const char* MfllNvram::getChunkMfnrTh(size_t *bufferSize)
{
    mfllLogW("Not support getChunkMfnrTh");
    return NULL;
}


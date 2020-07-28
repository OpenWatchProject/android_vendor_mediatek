/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "aao_buf_mgr"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <mtkcam/utils/std/Trace.h>
#include <sys/stat.h>
#include <cutils/properties.h>
#include <string.h>

#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
#include <debug/DebugUtil.h>
#include <mtkcam/drv/IHalSensor.h>

#include "IBufMgr.h"
#include "IResultPool.h"

#if CAM3_FAKE_SENSOR_DRV
#include "fake_sensor_drv/IStatisticPipe.h"
#else
#include <mtkcam/drv/iopipe/CamIO/IStatisticPipe.h>
#endif

#include <mtkcam/drv/iopipe/CamIO/IHalCamIO.h>
#include <private/IopipeUtils.h>

#include "SttBufQ.h"
#include "string.h"

using namespace NS3Av3;
using namespace NSCam;
using namespace NSCam::NSIoPipe;
using namespace NSCam::NSIoPipe::NSCamIOPipe;

#define FILE_NAME_SIZE 64
#define P1_HW_BUF_MAX_NUM 4
#define P1_HW_BUF_MASK    3 // (P1_HW_BUF_MAX_NUM - 1)
#define SEM_TIME_LIMIT_NS 500000000L // (500 ms)

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class AAOBufMgrImp : public AAOBufMgr
{
public:
    virtual MBOOL destroyInstance();
                  AAOBufMgrImp(MINT32 i4SensorDev, MINT32 i4SensorIdx, const ConfigInfo_T& rConfigInfo);
    virtual      ~AAOBufMgrImp();

    virtual MBOOL dequeueHwBuf();
    virtual MBOOL enqueueHwBuf();
    virtual StatisticBufInfo* dequeueSwBuf();
    virtual MVOID abortDequeue();
    virtual MINT32 waitDequeue(){return 0;};
    virtual MVOID notifyPreStop();
    virtual MVOID reset();

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data member
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    MINT32           const m_i4SensorDev;
    MINT32           const m_i4SensorIdx;
    MBOOL                  m_bDebugEnable;
    IStatisticPipe*        m_pSttPipe;
    MUINT32                m_rBufIndex;  // the index of write buffers.
    PortID                 m_rPort;
    MBOOL                  m_bAbort;
    MUINT32                m_u4FrameCount;
    INormalPipe*           m_pCamIO;//NSCam::NSIoPipe::NSCamIOPipe::INormalPipe
    StatisticBufInfo       m_rHwBuf;
    QBufInfo               m_HW_QBuf[P1_HW_BUF_MAX_NUM];
    MUINT32                m_HW_DeQBufIdx;
    MUINT32                m_HW_EnQBufIdx;
    std::mutex             m_QBufMtx;
    mutable std::mutex     m_Lock;
    mutable std::mutex     m_LockDeqIdx;
    std::condition_variable m_Cond;
    MBOOL                  m_bForceReturn;
    MBOOL                  m_bPreStop;
};

#if (CAM3_3ATESTLVL > CAM3_3AUT)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AAOBufMgr*
AAOBufMgr::
createInstance(MINT32 const i4SensorDev, MINT32 const i4SensorIdx, const ConfigInfo_T& rConfigInfo, STT_CFG_INFO_T const sttInfo __unused)
{
    AAOBufMgrImp* pObj = new AAOBufMgrImp(i4SensorDev,i4SensorIdx,rConfigInfo);
    return pObj;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
AAOBufMgrImp::
destroyInstance()
{
    delete this;
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AAOBufMgrImp::
AAOBufMgrImp(MINT32 i4SensorDev, MINT32 i4SensorIdx, const ConfigInfo_T& rConfigInfo)
    : m_i4SensorDev(i4SensorDev)
    , m_i4SensorIdx(i4SensorIdx)
    , m_bDebugEnable(MFALSE)
    , m_pSttPipe(NULL)
    , m_rBufIndex(0)
    , m_rPort(PORT_AAO)
    , m_bAbort(MFALSE)
    , m_u4FrameCount(0)
    , m_pCamIO(NULL)
    , m_bPreStop(MFALSE)
{
    // NormalIOPipe create instance
    if (m_pCamIO == NULL)
    {
        m_pCamIO = (INormalPipe*)INormalPipeUtils::get()->createDefaultNormalPipe(m_i4SensorIdx, LOG_TAG);
        if (m_pCamIO == NULL)
        {
            CAM_LOGE("Fail to create NormalPipe");
        }
    }

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.aao_mgr.enable", value, "0");
    m_bDebugEnable = atoi(value);

    m_pSttPipe = IStatisticPipe::createInstance(m_i4SensorIdx, LOG_TAG);
    m_HW_DeQBufIdx = 0;
    m_HW_EnQBufIdx = 0;
    m_bForceReturn = MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AAOBufMgrImp::
~AAOBufMgrImp()
{
    if(m_pSttPipe != NULL)
    {
        m_pSttPipe->destroyInstance(LOG_TAG);
        m_pSttPipe = NULL;
    }

    // NormalIOPipe destroy instance
    if (m_pCamIO != NULL)
    {
        m_pCamIO->destroyInstance(LOG_TAG);
        m_pCamIO = NULL;
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static MBOOL dumpHwBuf(char* fileName, char* folderName, BufInfo& rLastBuf, MBOOL isHwBuf = 0)
{
    FILE *fp = fopen(fileName, "w");
    if (NULL == fp)
    {
        CAM_LOGE("fail to open file to save img: %s", fileName);
        MINT32 err = mkdir(folderName, S_IRWXU | S_IRWXG | S_IRWXO);
        CAM_LOGD("err = %d", err);
        return MFALSE;
    } else
    {
        if(isHwBuf){
            fwrite(&(rLastBuf.mMetaData.mMagicNum_hal), sizeof(MUINT32), 1, fp);
            fwrite(&(rLastBuf.mStride), sizeof(MUINT32), 1, fp);
            fwrite(&(rLastBuf.mSize), sizeof(MUINT32), 1, fp);
        }
        fwrite(reinterpret_cast<void *>(rLastBuf.mVa), 1, rLastBuf.mSize, fp);
        fclose(fp);
        return MTRUE;
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static MBOOL dumpBuf(char* fileName, char* folderName, void* pBuf, MUINT32 u4Size)
{
    FILE *fp = fopen(fileName, "w");
    if (NULL == fp)
    {
        CAM_LOGE("fail to open file to save img: %s", fileName);
        MINT32 err = mkdir(folderName, S_IRWXU | S_IRWXG | S_IRWXO);
        CAM_LOGD("err = %d", err);
        return MFALSE;
    } else
    {
        fwrite(reinterpret_cast<void *>(pBuf), 1, u4Size, fp);
        fclose(fp);
        return MTRUE;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
AAOBufMgrImp::
dequeueHwBuf()
{
    CAM_LOGD_IF(m_bDebugEnable,"[%s] m_i4SensorDev(%d) + \n", __FUNCTION__, m_i4SensorDev);

    if(m_bAbort || m_bPreStop)
        return -1;

    if(m_pSttPipe != NULL)
    {
        DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "dequeueHwBuf", 0);
        // deque HW buffer from driver.
        MUINT32 QBufIdx = m_HW_DeQBufIdx & P1_HW_BUF_MASK;

        m_bForceReturn = MFALSE;
        if (m_HW_DeQBufIdx - m_HW_EnQBufIdx >= P1_HW_BUF_MASK)
        {
            CAM_LOGE("[%s] dequeue HW buffer fail, force return", __FUNCTION__);
            m_bForceReturn = MTRUE;
            m_Cond.notify_all();
        }

        CAM_TRACE_BEGIN("AAO STT deque");
        MBOOL ret = m_pSttPipe->deque(m_rPort, m_HW_QBuf[QBufIdx]);
        CAM_TRACE_END();

        int size = m_HW_QBuf[QBufIdx].mvOut.size();
        DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "dequeueHwBuf", 1);
        if(m_bAbort || m_bPreStop || (size == 0 && ret == MTRUE)){
            DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "dequeueHwBuf", -1);
            return -1;
        }

        if(ret == MFALSE)
        {
            if (m_HW_DeQBufIdx - m_HW_EnQBufIdx >= P1_HW_BUF_MASK)
            {
                CAM_LOGE("[%s] wait enqueue", __FUNCTION__);
            }

            CAM_LOGE("[%s] m_pSttPipe deque fail", __FUNCTION__);
            DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "dequeueHwBuf", -1);
            return MFALSE;
        }

        BufInfo rLastBuf = m_HW_QBuf[QBufIdx].mvOut.at( size-1 );

        CAM_LOGD_IF(m_bDebugEnable, "Deque HW Buffer Idx : %d (%d) - pa[0x%p]\n", QBufIdx, m_HW_DeQBufIdx, (void*)(rLastBuf.mPa));

        {
            std::lock_guard<std::mutex> lock(m_LockDeqIdx);
            m_HW_DeQBufIdx++;
        }
        m_u4FrameCount++;
        m_Cond.notify_all();
        DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "dequeueHwBuf", -1);

        do {
            std::shared_ptr<ISttBufQ> pSttBufQ;
            std::shared_ptr<ISttBufQ::DATA> pData;
            int BufSize;

            pSttBufQ = NSCam::ISttBufQ::getInstance(m_i4SensorDev);
            if (pSttBufQ == NULL) break;

            pData = pSttBufQ->deque();
            if (pData == NULL) break;

            BufSize = rLastBuf.mSize;
            pData->AAO.resize(BufSize);
            pData->MagicNumberStt     = rLastBuf.mMetaData.mMagicNum_hal;
            pData->MagicNumberRequest = 0;
            ::memcpy(pData->AAO.data(), (const void*)rLastBuf.mVa, BufSize);

            pSttBufQ->enque(pData);
        } while(0);
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
AAOBufMgrImp::
enqueueHwBuf()
{
    std::lock_guard<std::mutex> autoLock(m_QBufMtx);

    if(m_pSttPipe != NULL)
    {
        if(m_HW_EnQBufIdx >= m_HW_DeQBufIdx)
        {
            CAM_LOGD("[%s] AAO deque fail - EnQBufIdx(%d) -> DeQBufIdx(%d)\n", __FUNCTION__, m_HW_EnQBufIdx, m_HW_DeQBufIdx);
            return -1;
        }

        // enque HW buffer back driver
        CAM_LOGD_IF(m_bDebugEnable, "enque Hw buffer back driver.\n");
        CAM_TRACE_BEGIN("AAO STT enque");
        CAM_LOGD_IF(m_bDebugEnable, "EnQBufIdx(%d) -> DeQBufIdx(%d)\n", m_HW_EnQBufIdx, m_HW_DeQBufIdx);
        DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "enqueueHwBuf", 0);
        for (MUINT32 i = 0; i < P1_HW_BUF_MAX_NUM; i++)
        {
            MUINT32 QBufIdx = m_HW_EnQBufIdx & P1_HW_BUF_MASK;
            int size = m_HW_QBuf[QBufIdx].mvOut.size();
            if(size <= 0) {
                CAM_LOGD("without deque buffer, skip enque");
                break;
            }
            BufInfo rLastBuf = m_HW_QBuf[QBufIdx].mvOut.at( size-1 );

            CAM_LOGD_IF(m_bDebugEnable, "Enque HW Buffer Idx : %d (%d) - pa[0x%p]\n", QBufIdx, m_HW_EnQBufIdx, (void*)(rLastBuf.mPa));
            MINT32 i4Ret = m_pSttPipe->enque(m_HW_QBuf[QBufIdx]);
            if(!i4Ret)
            {
                CAM_LOGE("[%s] m_pSttPipe enque fail", __FUNCTION__);
                CAM_TRACE_END();
                return -1;
            }

            m_HW_EnQBufIdx++;

            if(m_HW_EnQBufIdx >= m_HW_DeQBufIdx || m_HW_EnQBufIdx >= m_rHwBuf.mFrameCount) {
                CAM_LOGD_IF(m_bDebugEnable, "[%s] enque break, m_HW_EnQBufIdx: %d, m_HW_DeQBufIdx: %d, m_rHwBuf.mFrameCount: %d\n", __FUNCTION__, m_HW_EnQBufIdx, m_HW_DeQBufIdx, m_rHwBuf.mFrameCount);
                break;
            }
        }
        CAM_TRACE_END();
        DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "enqueueHwBuf", -1);
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
StatisticBufInfo*
AAOBufMgrImp::
dequeueSwBuf()
{
    std::lock_guard<std::mutex> autoLock(m_QBufMtx);

    MBOOL bEnable = property_get_int32("vendor.aao.dump.enable", 0);
    MBOOL bEnableHW = property_get_int32("vendor.aaohw.dump.enable", 0);

    if(m_pSttPipe != NULL)
    {
        MBOOL bWaitDeqHW = MFALSE;
        {
            std::lock_guard<std::mutex> lock(m_LockDeqIdx);
            if(m_HW_EnQBufIdx >= m_HW_DeQBufIdx) {
                bWaitDeqHW = MTRUE;
                CAM_LOGD("[%s] AAO deque fail - EnQBufIdx(%d) -> DeQBufIdx(%d)\n", __FUNCTION__, m_HW_EnQBufIdx, m_HW_DeQBufIdx);
            }
        }
        if(bWaitDeqHW)
        {
            std::unique_lock<std::mutex> lk(m_Lock);
            // add long timeout to avoid being stuck with abortDequeue
            std::cv_status i4Ret = m_Cond.wait_for(lk, std::chrono::nanoseconds(SEM_TIME_LIMIT_NS));
            if (i4Ret == std::cv_status::timeout) {
                CAM_LOGD("[%s] i4Ret(%d), TimeOut(%d ms)\n", __FUNCTION__, i4Ret, SEM_TIME_LIMIT_NS / 1000000);
                m_bForceReturn = MFALSE;
                return NULL;
            }
            if(m_bAbort || m_bPreStop || m_bForceReturn) {
                CAM_LOGD("[%s] AAO buffer return NULL: EnQBufIdx(%d)/DeQBufIdx(%d)\n", __FUNCTION__, m_HW_EnQBufIdx, m_HW_DeQBufIdx);
                m_bForceReturn = MFALSE;
                return NULL;
            }
            CAM_LOGD("[%s] AAO deque success - EnQBufIdx(%d) -> DeQBufIdx(%d)\n", __FUNCTION__, m_HW_EnQBufIdx, m_HW_DeQBufIdx);
        }

        // get the last HW buffer to SW Buffer.
        MUINT32 DeQBufIdx = (MUINT32)(m_HW_DeQBufIdx - 1);
        MUINT32 QBufIdx   = DeQBufIdx & P1_HW_BUF_MASK;
        CAM_LOGD_IF(m_bDebugEnable, "Get HW Buffer Idx : %d (%d)\n", QBufIdx, DeQBufIdx);
        DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "dequeueSwBuf", 0);
        int size = m_HW_QBuf[QBufIdx].mvOut.size();
        BufInfo rLastBuf = m_HW_QBuf[QBufIdx].mvOut.at( size-1 );

        // copy the last HW buffer to SW Buffer.
        m_rHwBuf.mMagicNumber = rLastBuf.mMetaData.mMagicNum_hal;
        m_rHwBuf.mSize = rLastBuf.mSize;
        m_rHwBuf.mVa = rLastBuf.mVa;
        m_rHwBuf.mStride = rLastBuf.mStride;
        m_rHwBuf.mFrameCount = m_u4FrameCount;
        m_rHwBuf.mTimeStamp = rLastBuf.mMetaData.mTimeStamp;

        // update flash state to statistic information.
        IResultPool* pResultPoolObj = IResultPool::getInstance(m_i4SensorDev);
        AllResult_T *pAllResult = pResultPoolObj->getResultByReqNum(m_rHwBuf.mMagicNumber, __FUNCTION__);
        if(pAllResult)
        {
            FLASHResultToMeta_T* pFLASHResult = (FLASHResultToMeta_T*)(pAllResult->ModuleResult[E_FLASH_RESULTTOMETA]->read());
            if(pFLASHResult != NULL)
                m_rHwBuf.mFlashState = pFLASHResult->u1FlashState;
        }
        pResultPoolObj->returnResult(pAllResult, __FUNCTION__);

        DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "dequeueSwBuf", 1);

        CAM_LOGD_IF(m_bDebugEnable, "port(%d), va[0x%p]/pa[0x%p]/#(%d), Size(%d), timeStamp(%" PRId64 ")",
                  m_rPort.index, (void*)(m_rHwBuf.mVa), (void*)(rLastBuf.mPa), m_rHwBuf.mMagicNumber, m_rHwBuf.mSize, m_rHwBuf.mTimeStamp);

        // dump aao data for debug
        if (bEnable) {
            static MUINT32 count = 0;
            char fileName[FILE_NAME_SIZE];
            char folderName[FILE_NAME_SIZE];
            strncpy(folderName, "/data/vendor/aao", FILE_NAME_SIZE);
            sprintf(fileName, "/data/vendor/aao/aao_%d_%d.raw", rLastBuf.mMetaData.mMagicNum_hal, count++);
            if(dumpHwBuf(fileName, folderName, rLastBuf))
            {
                CAM_LOGD("%s\n", fileName);
            }
        }
        // dump aao hw data for ut
        if(bEnableHW){
            char fileName[FILE_NAME_SIZE];
            char folderName[FILE_NAME_SIZE];
            strncpy(folderName, "/data/vendor/aaoHwBuf", FILE_NAME_SIZE);
            sprintf(fileName, "/data/vendor/aaoHwBuf/aao_%d.hwbuf", rLastBuf.mMetaData.mMagicNum_hal);
            if(dumpHwBuf(fileName, folderName, rLastBuf, bEnableHW))
            {
                CAM_LOGD("%s\n", fileName);
            }
        }

        DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "dequeueSwBuf", -1);

        if(m_bAbort || m_bPreStop)
            return NULL;
    }

    return &m_rHwBuf;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
AAOBufMgrImp::
notifyPreStop()
{
    CAM_LOGD("[%s] m_rBufIndex(%d) +\n", __FUNCTION__, m_rBufIndex);
    m_bPreStop = MTRUE;
    CAM_LOGD("[%s] m_rBufIndex(%d) -\n", __FUNCTION__, m_rBufIndex);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
AAOBufMgrImp::
abortDequeue()
{
    CAM_LOGD("[%s] m_rBufIndex(%d) +\n", __FUNCTION__, m_rBufIndex);
    m_bAbort = MTRUE;
    m_u4FrameCount = 0;
    if(m_pSttPipe != NULL)
        m_pSttPipe->abortDma(m_rPort,LOG_TAG);
    m_Cond.notify_all();
    CAM_LOGD("[%s] abortDma done\n", __FUNCTION__);
    CAM_LOGD("[%s] m_rBufIndex(%d) -\n", __FUNCTION__, m_rBufIndex);
}

MVOID
AAOBufMgrImp::
reset()
{
    CAM_LOGD("[%s] m_i4SensorDev:%d\n", __FUNCTION__, m_i4SensorDev);
    /* W+T susprned/resume flow , skip enque after deque done when reset() is called */
    m_HW_EnQBufIdx = m_HW_DeQBufIdx;
}

#endif

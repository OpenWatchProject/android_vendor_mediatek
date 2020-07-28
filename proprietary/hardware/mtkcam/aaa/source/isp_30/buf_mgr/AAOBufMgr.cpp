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
#include <aao_buf_mgr.h>

#include "SttBufQ.h"
#include "string.h"

using namespace android;
using namespace NS3Av3;
using namespace NSCam;

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
                  AAOBufMgrImp(MINT32 i4SensorDev, MINT32 i4SensorIdx, const ConfigInfo_T& /*rConfigInfo*/, STT_CFG_INFO_T const sttInfo);
    virtual      ~AAOBufMgrImp();

    virtual MBOOL dequeueHwBuf();
    virtual MBOOL enqueueHwBuf();
    virtual StatisticBufInfo* dequeueSwBuf();
    virtual MVOID abortDequeue();
    virtual MINT32 waitDequeue(){return 0;};
    virtual MVOID notifyPreStop(){return ;}
    virtual MVOID setRequestNum(MUINT32 u4RequestNum) { m_u4LastRequestNum = u4RequestNum; };
    virtual MVOID reset() {};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data member
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    MINT32           const m_i4SensorDev;
    MINT32           const m_i4SensorIdx;
    MBOOL                  m_bDebugEnable;
    MUINT32                m_rBufIndex;  // the index of write buffers.
    MBOOL                  m_bAbort;
    MUINT32                m_u4FrameCount;
    StatisticBufInfo       m_rHwBuf;
    BufInfo_T              m_HW_QBuf[P1_HW_BUF_MAX_NUM];
    MUINT32                m_HW_DeQBufIdx;
    MUINT32                m_HW_EnQBufIdx;
    Mutex                  m_QBufMtx;
    mutable std::mutex     m_Lock;
    mutable Mutex          m_LockDeqIdx;
    std::condition_variable m_Cond;
    MBOOL                  m_bForceReturn;
    MUINT32                m_u4MagicNum[P1_HW_BUF_MAX_NUM];
    MUINT32                m_u4LastRequestNum;
};

#if (CAM3_3ATESTLVL > CAM3_3AUT)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AAOBufMgr*
AAOBufMgr::
createInstance(MINT32 const i4SensorDev, MINT32 const i4SensorIdx, const ConfigInfo_T& rConfigInfo, STT_CFG_INFO_T const sttInfo)
{
    AAOBufMgrImp* pObj = new AAOBufMgrImp(i4SensorDev,i4SensorIdx,rConfigInfo, sttInfo);
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
AAOBufMgrImp(MINT32 i4SensorDev, MINT32 i4SensorIdx, const ConfigInfo_T& /*rConfigInfo*/, STT_CFG_INFO_T const sttInfo)
    : m_i4SensorDev(i4SensorDev)
    , m_i4SensorIdx(i4SensorIdx)
    , m_bDebugEnable(MFALSE)
    , m_rBufIndex(0)
    , m_bAbort(MFALSE)
    , m_u4FrameCount(0)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.aao_buf_mgr.enable", value, "0");
    m_bDebugEnable = atoi(value);

    IAAOBufMgrWrapper::getInstance().initPipe(m_i4SensorDev, m_i4SensorIdx, sttInfo.i4TgInfo);
    IAAOBufMgrWrapper::getInstance().startPipe(m_i4SensorDev);

    m_HW_DeQBufIdx = 0;
    m_HW_EnQBufIdx = 0;
    m_bForceReturn = MFALSE;
    memset(m_u4MagicNum, 0, P1_HW_BUF_MAX_NUM * sizeof(MUINT32));
    m_u4LastRequestNum = 0;

    CAM_LOGD("[%s] SensorDev(%d) -", __FUNCTION__, m_i4SensorDev);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AAOBufMgrImp::
~AAOBufMgrImp()
{
    IAAOBufMgrWrapper::getInstance().stopPipe(m_i4SensorDev);
    IAAOBufMgrWrapper::getInstance().uninitPipe(m_i4SensorDev);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
AAOBufMgrImp::
dequeueHwBuf()
{
    CAM_LOGD_IF(m_bDebugEnable, "[%s +] m_i4SensorDev(%d) EnQBufIdx(%d) DeQBufIdx(%d)\n", __FUNCTION__, m_i4SensorDev, m_HW_EnQBufIdx, m_HW_DeQBufIdx);

    if (m_bAbort)
        return -1;

    // deque HW buffer from driver.
    MUINT32 QBufIdx = m_HW_DeQBufIdx & P1_HW_BUF_MASK;

    if (m_HW_DeQBufIdx - m_HW_EnQBufIdx >= P1_HW_BUF_MASK)
    {
        CAM_LOGE("[%s] buffer full wait enque, force return, EnQBufIdx(%d) DeQBufIdx(%d)\n", __FUNCTION__, m_HW_EnQBufIdx, m_HW_DeQBufIdx);
        m_bForceReturn = MTRUE;
        m_Cond.notify_all();
    }

    CAM_TRACE_BEGIN("AAO STT deque");
    MBOOL ret = IAAOBufMgrWrapper::getInstance().dequePipe(m_i4SensorDev, m_HW_QBuf[QBufIdx]);
    CAM_TRACE_END();

    if (m_bAbort || (m_HW_QBuf[QBufIdx].size == 0 && ret == MTRUE))
    {
        return -1;
    }

    if (ret == MFALSE)
    {
        if (m_HW_DeQBufIdx - m_HW_EnQBufIdx >= P1_HW_BUF_MASK)
        {
            CAM_LOGE("[%s] buffer full wait enque\n", __FUNCTION__);
        }

        CAM_LOGE("[%s] AAO deque fail, EnQBufIdx(%d) DeQBufIdx(%d)\n", __FUNCTION__, m_HW_EnQBufIdx, m_HW_DeQBufIdx);
        return MFALSE;
    }

    CAM_LOGD_IF(m_bDebugEnable, "[%s] Deque HW Buffer Idx : %d (%d) - virtAddr:[0x%x]/phyAddr:[0x%x]\n", __FUNCTION__, QBufIdx, m_HW_DeQBufIdx, m_HW_QBuf[QBufIdx].virtAddr, m_HW_QBuf[QBufIdx].phyAddr);

    {
        Mutex::Autolock lock(m_LockDeqIdx);
        m_HW_DeQBufIdx++;
    }
    m_u4FrameCount++;
    m_Cond.notify_all();

    CAM_LOGD_IF(m_bDebugEnable, "[%s -] m_i4SensorDev(%d) EnQBufIdx(%d) DeQBufIdx(%d)\n", __FUNCTION__, m_i4SensorDev, m_HW_EnQBufIdx, m_HW_DeQBufIdx);
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
AAOBufMgrImp::
enqueueHwBuf()
{
    CAM_LOGD_IF(m_bDebugEnable, "[%s +] m_i4SensorDev(%d) EnQBufIdx(%d) DeQBufIdx(%d)\n", __FUNCTION__, m_i4SensorDev, m_HW_EnQBufIdx, m_HW_DeQBufIdx);
    Mutex::Autolock autoLock(m_QBufMtx);

    if(m_HW_EnQBufIdx >= m_HW_DeQBufIdx)
    {
        CAM_LOGD("[%s] no need to enque, EnQBufIdx(%d) DeQBufIdx(%d)\n", __FUNCTION__, m_HW_EnQBufIdx, m_HW_DeQBufIdx);
        return -1;
    }

    // enque HW buffer back driver
    CAM_TRACE_BEGIN("AAO STT enque");
    for (MUINT32 i = 0; i < P1_HW_BUF_MAX_NUM; i++)
    {
        MUINT32 QBufIdx = m_HW_EnQBufIdx & P1_HW_BUF_MASK;
        CAM_LOGD_IF(m_bDebugEnable, "[%s] Enque HW Buffer Idx : %d (%d) - rBufInfo.virtAddr:[0x%x]/phyAddr:[0x%x]\n", __FUNCTION__, QBufIdx, m_HW_EnQBufIdx, m_HW_QBuf[QBufIdx].virtAddr, m_HW_QBuf[QBufIdx].phyAddr);

        MBOOL ret = IAAOBufMgrWrapper::getInstance().enquePipe(m_i4SensorDev, m_HW_QBuf[QBufIdx]);
        if (!ret)
        {
            CAM_LOGE("[%s] AAO enque fail, EnQBufIdx(%d) DeQBufIdx(%d)\n", __FUNCTION__, m_HW_EnQBufIdx, m_HW_DeQBufIdx);
            CAM_TRACE_END();
            return -1;
        }

        m_HW_EnQBufIdx++;

        if (m_HW_EnQBufIdx >= m_HW_DeQBufIdx || m_HW_EnQBufIdx >= m_rHwBuf.mFrameCount)
        {
            CAM_LOGD_IF(m_bDebugEnable, "[%s] enque break, m_HW_EnQBufIdx:%d, m_HW_DeQBufIdx:%d, m_rHwBuf.mFrameCount:%d\n", __FUNCTION__, m_HW_EnQBufIdx, m_HW_DeQBufIdx, m_rHwBuf.mFrameCount);
            break;
        }
    }
    CAM_TRACE_END();

    CAM_LOGD_IF(m_bDebugEnable, "[%s -] m_i4SensorDev(%d) EnQBufIdx(%d) DeQBufIdx(%d)\n", __FUNCTION__, m_i4SensorDev, m_HW_EnQBufIdx, m_HW_DeQBufIdx);
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
StatisticBufInfo*
AAOBufMgrImp::
dequeueSwBuf()
{
    CAM_LOGD_IF(m_bDebugEnable, "[%s +] m_i4SensorDev(%d) EnQBufIdx(%d) DeQBufIdx(%d)\n", __FUNCTION__, m_i4SensorDev, m_HW_EnQBufIdx, m_HW_DeQBufIdx);
    Mutex::Autolock autoLock(m_QBufMtx);

    MBOOL bWaitDeqHW = MFALSE;
    {
        Mutex::Autolock lock(m_LockDeqIdx);
        if (m_HW_EnQBufIdx >= m_HW_DeQBufIdx || m_HW_DeQBufIdx < 2) // (IP-based) ISP 3.X needs to wait before second dequeued buffer
        {
            bWaitDeqHW = MTRUE;
            CAM_LOGD("[%s] Not get buffer wait deque, EnQBufIdx(%d) DeQBufIdx(%d)\n", __FUNCTION__, m_HW_EnQBufIdx, m_HW_DeQBufIdx);
            if (m_HW_DeQBufIdx == 0)
            {
                // when m_HW_DeQBufIdx == 0, it doesn't need to wait, because first buffer can't be used
                CAM_LOGD("[%s] AAO buffer return NULL, DeQBufIdx:%d\n", __FUNCTION__, m_HW_DeQBufIdx);
                return NULL;
            }
        }
    }
    if (bWaitDeqHW)
    {
        //m_Cond.wait(m_Lock);

        // add long timeout to avoid being stuck with abortDequeue
        std::unique_lock<std::mutex> lk(m_Lock);
        std::cv_status i4Ret = m_Cond.wait_for(lk, std::chrono::nanoseconds(SEM_TIME_LIMIT_NS));
        if (i4Ret == std::cv_status::timeout)
            CAM_LOGD("[%s] i4Ret(%d), TimeOut(%ld ms)\n", __FUNCTION__, i4Ret, SEM_TIME_LIMIT_NS / 1000000);

        if (m_bAbort || m_bForceReturn)
        {
            CAM_LOGD("[%s] AAO buffer return NULL, EnQBufIdx(%d) DeQBufIdx(%d)\n", __FUNCTION__, m_HW_EnQBufIdx, m_HW_DeQBufIdx);
            m_bForceReturn = MFALSE;
            return NULL;
        }
        CAM_LOGD("[%s] AAO deque success, EnQBufIdx(%d) DeQBufIdx(%d)\n", __FUNCTION__, m_HW_EnQBufIdx, m_HW_DeQBufIdx);
    }

    // set magic number to next two frames, 1: certainly mapping, 2: avoid losing magic number when cal time too much
    MUINT32 TargetDeqIdx = (m_HW_DeQBufIdx + 1) & P1_HW_BUF_MASK;
    m_u4MagicNum[TargetDeqIdx] = m_u4LastRequestNum;
    CAM_LOGD_IF(m_bDebugEnable, "[%s] DeQBufIdx:%d -> set magic number:%d\n", __FUNCTION__, TargetDeqIdx, m_u4MagicNum[TargetDeqIdx]);
    TargetDeqIdx = (m_HW_DeQBufIdx + 2) & P1_HW_BUF_MASK;
    m_u4MagicNum[TargetDeqIdx] = m_u4LastRequestNum;
    CAM_LOGD_IF(m_bDebugEnable, "[%s] DeQBufIdx:%d -> set magic number:%d\n", __FUNCTION__, TargetDeqIdx, m_u4MagicNum[TargetDeqIdx]);

    // get the last HW buffer to SW Buffer.
    MUINT32 DeQBufIdx = (MUINT32)(m_HW_DeQBufIdx - 1);
    MUINT32 QBufIdx   = DeQBufIdx & P1_HW_BUF_MASK;
    CAM_LOGD_IF(m_bDebugEnable, "[%s] Get HW Buffer Idx : %d (%d)\n", __FUNCTION__, QBufIdx, DeQBufIdx);

    // copy the last HW buffer to SW Buffer.
    BufInfo_T rLastBuf = m_HW_QBuf[QBufIdx];
    m_rHwBuf.mMagicNumber = m_u4MagicNum[QBufIdx];
    CAM_LOGD_IF(m_bDebugEnable, "[%s] DeQBufIdx:%d -> get magic number:%d\n", __FUNCTION__, QBufIdx, m_u4MagicNum[QBufIdx]);
    m_rHwBuf.mSize = rLastBuf.size;
    m_rHwBuf.mVa = rLastBuf.virtAddr;
    // m_rHwBuf.mStride = rLastBuf.mStride;
    m_rHwBuf.mFrameCount = m_u4FrameCount;
    m_rHwBuf.mTimeStamp = 0;

    // update flash state to statistic information.
    IResultPool* pResultPoolObj = IResultPool::getInstance(m_i4SensorDev);
    FLASHResultToMeta_T* pFLASHResult = (FLASHResultToMeta_T*)pResultPoolObj->getResult(m_rHwBuf.mMagicNumber,E_FLASH_RESULTTOMETA);
    if(pFLASHResult != NULL)
        m_rHwBuf.mFlashState = pFLASHResult->u1FlashState;
    CAM_LOGD_IF(m_bDebugEnable, "[%s] FlashState %d, magic number:%d\n", __FUNCTION__, m_rHwBuf.mFlashState, m_u4MagicNum[QBufIdx]);
    CAM_LOGD_IF(m_bDebugEnable, "[%s] virtAddr[0x%x]/phyAddr[0x%x]/#(%d), Size(%d), timeStamp(%" PRId64 ")", __FUNCTION__,
              m_HW_QBuf[QBufIdx].virtAddr, m_HW_QBuf[QBufIdx].phyAddr, m_rHwBuf.mMagicNumber, m_rHwBuf.mSize, m_rHwBuf.mTimeStamp);

    do {
        android::sp<ISttBufQ> pSttBufQ;
        android::sp<ISttBufQ::DATA> pData;
        int BufSize;

        pSttBufQ = NSCam::ISttBufQ::getInstance(m_i4SensorDev);
        if (pSttBufQ == NULL) break;

        pData = pSttBufQ->deque();
        if (pData == NULL) break;

        BufSize = rLastBuf.size;
        pData->AAO.resize(BufSize);
        pData->MagicNumberStt     = m_u4MagicNum[QBufIdx];
        pData->MagicNumberRequest = 0;
        ::memcpy(pData->AAO.editArray(), (const void*)rLastBuf.virtAddr, BufSize);

        pSttBufQ->enque(pData);
    } while(0);

    // dump aao data for debug
    MBOOL bEnable = property_get_int32("vendor.aao.dump.enable", 0);
    static MINT32 frameCnt = 0;
    if (bEnable)
    {
        char fileName[FILE_NAME_SIZE];
        sprintf(fileName, "/sdcard/aao/aao_%d.raw", frameCnt++);
        FILE *fp = fopen(fileName, "w");
        if (NULL == fp)
        {
            CAM_LOGE("fail to open file to save img: %s", fileName);
            MINT32 err = mkdir("/sdcard/aao", S_IRWXU | S_IRWXG | S_IRWXO);
            CAM_LOGD("err = %d", err);
            return MFALSE;
        }
        CAM_LOGD_IF(m_bDebugEnable, "%s\n", fileName);
        fwrite(reinterpret_cast<void *>(rLastBuf.virtAddr), 1, rLastBuf.size, fp);
        fclose(fp);
    }

    if (m_bAbort)
        return NULL;

    CAM_LOGD_IF(m_bDebugEnable, "[%s -] m_i4SensorDev(%d) EnQBufIdx(%d) DeQBufIdx(%d)\n", __FUNCTION__, m_i4SensorDev, m_HW_EnQBufIdx, m_HW_DeQBufIdx);
    return &m_rHwBuf;
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
    IAAOBufMgrWrapper::getInstance().abortDeque(m_i4SensorDev);
    m_Cond.notify_all();
    CAM_LOGD("[%s] m_rBufIndex(%d) -\n", __FUNCTION__, m_rBufIndex);
}

#endif

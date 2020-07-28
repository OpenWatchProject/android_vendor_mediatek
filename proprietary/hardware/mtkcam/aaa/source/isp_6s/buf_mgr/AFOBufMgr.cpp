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
#define LOG_TAG "afo_buf_mgr"

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
#include <drv/isp_reg.h>
#include <private/IopipeUtils.h>

#include <dbg_aaa_param.h>
#include <dbg_af_param.h>
#include <camera_custom_nvram.h>
#include <af_param.h>
#include <af_feature.h>
extern "C" {
#include <af_algo_if.h>
}
#include <af_mgr_if.h>
//#include <isp_mgr_af_stat.h>

#include <ae_mgr_if.h>

using namespace NS3Av3;
using namespace NSCam::NSIoPipe;
using namespace NSCam::NSIoPipe::NSCamIOPipe;

#define P1_HW_BUF_MAX_NUM 4
#define P1_HW_BUF_MASK    3 // (P1_HW_BUF_MAX_NUM - 1)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class AFOBufMgrImp : public AFOBufMgr
{
public:
    virtual MBOOL destroyInstance();
    AFOBufMgrImp(MINT32 i4SensorDev, MINT32 i4SensorIdx);
    virtual      ~AFOBufMgrImp();

    virtual MBOOL dequeueHwBuf();
    virtual MBOOL enqueueHwBuf();
    virtual StatisticBufInfo* dequeueSwBuf();
    virtual MVOID abortDequeue();
    virtual MINT32 waitDequeue();
    virtual MVOID notifyPreStop();
    virtual MVOID reset();

    virtual MBOOL sttDeque();
    virtual MBOOL sttEnque();

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data member
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    MINT32           const m_i4SensorDev;
    MINT32           const m_i4SensorIdx;
    MBOOL                  m_bDebugEnable;
    IStatisticPipe*        m_pSttPipe;
    PortID                 m_rPort;
    MBOOL                  m_bAbort;
    MUINT32                m_u4FrameCount;
    StatisticBufInfo       m_rHwBuf;
    QBufInfo               m_HW_QBuf[P1_HW_BUF_MAX_NUM];
    MUINT32                m_HW_DeQBufIdx;
    MUINT32                m_HW_EnQBufIdx;

    MUINT64                m_u8TimeDeque;
    MUINT64                m_u8TimeDequeDone;
    MUINT64                m_u8TimeEnque;

    MUINT32                m_u4ConfigLatency;
    MINT32                 m_i4IsCCUAF;
    MINT32                 m_i4UserKey;
    MINT32                 m_i4IsFirstTime;
    MINT32                 m_isAfoEnable;
    MBOOL                  m_bPreStop;
};

/* (unit:us) */
#define GETTIMESTAMP(time) {      \
        struct timespec t;        \
        MUINT64 timestamp;        \
                                  \
        t.tv_sec = t.tv_nsec = 0; \
        timestamp = 0;            \
        clock_gettime(CLOCK_MONOTONIC, &t); \
        timestamp = (MUINT64)((t.tv_sec) * 1000000000LL + t.tv_nsec)/1000; \
        time = timestamp;         \
        }

#if (CAM3_3ATESTLVL > CAM3_3AUT)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AFOBufMgr*
AFOBufMgr::
createInstance(MINT32 const i4SensorDev, MINT32 const i4SensorIdx, STT_CFG_INFO_T const sttInfo __unused)
{
    AFOBufMgrImp* pObj = new AFOBufMgrImp(i4SensorDev, i4SensorIdx);
    return pObj;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
AFOBufMgrImp::
destroyInstance()
{
    delete this;
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AFOBufMgrImp::
AFOBufMgrImp(MINT32 i4SensorDev, MINT32 i4SensorIdx)
    : m_i4SensorDev(i4SensorDev)
    , m_i4SensorIdx(i4SensorIdx)
    , m_bDebugEnable(MFALSE)
    , m_pSttPipe(NULL)
    , m_rPort(PORT_AFO)
    , m_bAbort(MFALSE)
    , m_u4FrameCount(0)
    , m_u8TimeDeque(0)
    , m_u8TimeDequeDone(0)
    , m_u8TimeEnque(0)
    , m_i4IsCCUAF(0)
    , m_i4UserKey(0)
    , m_i4IsFirstTime(1)
    , m_isAfoEnable(0)
    , m_bPreStop(MFALSE)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.afo_mgr.enable", value, "0");
    m_bDebugEnable = atoi(value);

    m_pSttPipe = IStatisticPipe::createInstance(m_i4SensorIdx, LOG_TAG);
    if(!m_pSttPipe)
    {
        CAM_LOGE("[%s] m_pSttPipe NULL", __FUNCTION__);
    }

    AFStaticInfo_T staticInfo;
    IAfMgr::getInstance(m_i4SensorDev).getStaticInfo(staticInfo, LOG_TAG);
    m_isAfoEnable = staticInfo.isAfHwEnableByP1;
    if (m_isAfoEnable)
    {
        m_HW_DeQBufIdx = 0;
        m_HW_EnQBufIdx = 0;
        m_u4ConfigLatency = 3; // CPU : T+3
    }
    else
    {
        CAM_LOGW("[%s] disable AF", __FUNCTION__);
    }

    CAM_LOGD("[%s] SensorDev(%d) -", __FUNCTION__, m_i4SensorDev);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AFOBufMgrImp::
~AFOBufMgrImp()
{
    if (m_pSttPipe != NULL)
    {
        m_pSttPipe->destroyInstance(LOG_TAG);
        m_pSttPipe = NULL;
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static MBOOL dumpHwBuf(char* fileName, char* folderName, StatisticBufInfo& rHwBuf, MBOOL isHwBuf = 0)
{
    FILE *fp = fopen(fileName, "w");
    if (NULL == fp)
    {
        CAM_LOGE("fail to open file to save img: %s", fileName);
        MINT32 err = mkdir(folderName, S_IRWXU | S_IRWXG | S_IRWXO);
        CAM_LOGD("err = %d", err);
        return MFALSE;
    }
    else
    {
        if (isHwBuf)
        {
            fwrite(&(rHwBuf.mMagicNumber), sizeof(MUINT32), 1, fp);
            fwrite(&(rHwBuf.mStride), sizeof(MUINT32), 1, fp);
            fwrite(&(rHwBuf.mSize), sizeof(MUINT32), 1, fp);
        }
        fwrite(reinterpret_cast<void *>(rHwBuf.mVa), 1, rHwBuf.mSize, fp);
        fclose(fp);
        return MTRUE;
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
AFOBufMgrImp::
dequeueHwBuf()
{
    CAM_LOGD_IF(m_bDebugEnable, "[%s] m_i4SensorDev(%d) + \n", __FUNCTION__, m_i4SensorDev);

    if(m_bAbort || m_bPreStop)
        return -1;

    /*-----------------------------------------------------------------------------------------------------
     *
     *                                     P1 - ISP Tuning Mgr
     *
     *-----------------------------------------------------------------------------------------------------*/

    if (m_i4IsFirstTime == 1)
    {
        AFStaticInfo_T staticInfo;
        IAfMgr::getInstance(m_i4SensorDev).getStaticInfo(staticInfo, LOG_TAG);
        m_i4IsCCUAF = staticInfo.isCCUAF;
        if (m_pSttPipe && m_i4IsCCUAF)
        {
            m_i4UserKey = m_pSttPipe->attach(LOG_TAG);
            m_i4IsFirstTime = 0;
        }
    }

    if (m_i4IsCCUAF)
    {
        if (m_i4UserKey != 0)
        {
            if (m_pSttPipe)
            {
                m_pSttPipe->wait(EPipeSignal_VSYNC, EPipeSignal_ClearNone, m_i4UserKey, 200);
                MUINT64 timeVsync;
                GETTIMESTAMP(timeVsync);
                CAM_LOGD_IF(m_bDebugEnable, "CCUAF Vsync - %ld", timeVsync);
            }
        }
        else
        {
            CAM_LOGE("%s, UserKey==0", __FUNCTION__);
        }
    }
    else
    {
        if (m_pSttPipe != NULL && m_isAfoEnable == 1)
        {
            CAM_TRACE_BEGIN("AFO STT deque");
            MBOOL ret = sttDeque();
            CAM_TRACE_END();
            if (ret == MFALSE)
                return MFALSE;

            // get the latest AFO buffer
            while (1)
            {
#define TIME_3MS 3000
                if ((m_u8TimeDequeDone - m_u8TimeDeque < TIME_3MS) && (m_u8TimeDequeDone > m_u8TimeDeque))
                {
                    ret = sttEnque();

                    if (ret != MFALSE)
                        ret = sttDeque();
                    else
                        return MFALSE;
                }
                else
                {
                    break;
                }
            }

            m_u4FrameCount++;

            // To read PL_STT from registers
            IspP1RegIo Regs[4];
            Regs[0].Addr = ((MUINT32) offsetof(cam_reg_t, AF_R1_AF_PL_STAT_0));
            Regs[1].Addr = ((MUINT32) offsetof(cam_reg_t, AF_R1_AF_PL_STAT_1));
            Regs[2].Addr = ((MUINT32) offsetof(cam_reg_t, AF_R1_AF_PL_STAT_0));
            Regs[3].Addr = ((MUINT32) offsetof(cam_reg_t, AF_R1_AF_PL_STAT_1));
            MUINT32 getRaw = 0x1;
            INormalPipe* pPipe = (INormalPipe*)INormalPipeUtils::get()->createDefaultNormalPipe(m_i4SensorIdx, LOG_TAG);
            if (pPipe != NULL)
            {
                pPipe->getIspReg(Regs, 4, getRaw);
                pPipe->destroyInstance(LOG_TAG);
                MUINT64 PD_STT = 0;
                PD_STT +=  ((MUINT64)(Regs[1].Data) << 32) + Regs[0].Data;
                PD_STT +=  ((MUINT64)(Regs[3].Data) << 32) + Regs[2].Data;
                CAM_LOGD_IF(m_bDebugEnable, "PL_STT %" PRIu64 " from register : (%d, %d, %d, %d)", PD_STT, Regs[0].Data, Regs[1].Data, Regs[2].Data, Regs[3].Data);
            }
            else
            {
                CAM_LOGE("PL_STT ERROR: normalPipe open fail");
            }
        }
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
AFOBufMgrImp::
enqueueHwBuf()
{
    if(m_bAbort || m_bPreStop)
        return -1;

    /*-----------------------------------------------------------------------------------------------------
     *
     *                                     P1 - ISP Tuning Mgr
     *
     *-----------------------------------------------------------------------------------------------------*/
    if (m_pSttPipe != NULL && m_isAfoEnable == 1)
    {
        if (m_HW_EnQBufIdx >= m_HW_DeQBufIdx)
        {
            CAM_LOGD("[%s] AFO deque fail - EnQBufIdx(%d) -> DeQBufIdx(%d)\n", __FUNCTION__, m_HW_EnQBufIdx, m_HW_DeQBufIdx);
            return -1;
        }

        CAM_TRACE_BEGIN("AFO STT enque");
        CAM_LOGD_IF(m_bDebugEnable, "EnQBufIdx(%d) -> DeQBufIdx(%d)\n", m_HW_EnQBufIdx, m_HW_DeQBufIdx);
        for (MINT32 i = 0; i < P1_HW_BUF_MAX_NUM; i++)
        {
            MBOOL ret = sttEnque();
            if (ret == MFALSE)
                break;
        }
        CAM_TRACE_END();
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
StatisticBufInfo*
AFOBufMgrImp::
dequeueSwBuf()
{
    MBOOL bEnable = property_get_int32("vendor.afo.dump.enable", 0);
    MBOOL bEnableHW = property_get_int32("vendor.afohw.dump.enable", 0);

    if(m_bAbort || m_bPreStop)
        return NULL;

    /*-----------------------------------------------------------------------------------------------------
     *
     *                                     P1 - ISP Tuning Mgr
     *
     *-----------------------------------------------------------------------------------------------------*/
    if (m_pSttPipe != NULL  && m_isAfoEnable == 1)
    {
        if (m_HW_EnQBufIdx >= m_HW_DeQBufIdx)
        {
            CAM_LOGD("[%s] AFO deque fail - EnQBufIdx(%d) -> DeQBufIdx(%d)\n", __FUNCTION__, m_HW_EnQBufIdx, m_HW_DeQBufIdx);
            return NULL;
        }

        // get the last HW buffer to SW Buffer.
        MUINT32 DeQBufIdx = (MUINT32)(m_HW_DeQBufIdx - 1);
        MUINT32 QBufIdx   = DeQBufIdx & P1_HW_BUF_MASK;
        CAM_LOGD_IF(m_bDebugEnable, "Get HW Buffer Idx : %d (%d)\n", QBufIdx, DeQBufIdx);
        int size = m_HW_QBuf[QBufIdx].mvOut.size();
        BufInfo rLastBuf = m_HW_QBuf[QBufIdx].mvOut.at( size - 1 );

        // copy the last HW buffer to SW Buffer.
        m_rHwBuf.mMagicNumber   = rLastBuf.mMetaData.mMagicNum_hal;
        m_rHwBuf.mSize          = rLastBuf.mSize;
        m_rHwBuf.mVa            = rLastBuf.mVa;
        m_rHwBuf.mStride        = rLastBuf.mStride;
        m_rHwBuf.mFrameCount    = m_u4FrameCount;
        m_rHwBuf.mTimeStamp  = rLastBuf.mMetaData.mTimeStamp;
        m_rHwBuf.mConfigNum = 0;//NSIspTuning::ISP_MGR_AF_STAT_CONFIG_T::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev)).getConfigNum( m_rHwBuf.mMagicNumber);
        m_rHwBuf.mConfigLatency = m_u4ConfigLatency;

        // update flash state to statistic information.
        IResultPool* pResultPoolObj = IResultPool::getInstance(m_i4SensorDev);
        AllResult_T *pAllResult = pResultPoolObj->getResultByReqNum(m_rHwBuf.mMagicNumber, __FUNCTION__);
        if(pAllResult)
        {
            FLASHResultToMeta_T* pFLASHResult = (FLASHResultToMeta_T*)(pAllResult->ModuleResult[E_FLASH_RESULTTOMETA]->read());
            if(pFLASHResult != NULL)
                m_rHwBuf.mFlashState = pFLASHResult->u1FlashState;

            AFResultConfig_T* pAFResultConfig = (AFResultConfig_T*)(pAllResult->ModuleResult[E_AF_CONFIGRESULTTOISP]->read());
            if (pAFResultConfig != NULL)
                m_rHwBuf.mConfigNum = pAFResultConfig->afIspRegInfo.configNum;
        }
        pResultPoolObj->returnResult(pAllResult, __FUNCTION__);

        CAM_LOGD_IF(m_bDebugEnable, "port(%d), va[0x%p]/pa[0x%p]/#(%d), Size(%d), Stride(%d), timeStamp(%" PRId64 ")",
                    m_rPort.index, (void*)(m_rHwBuf.mVa), (void*)(rLastBuf.mPa), m_rHwBuf.mMagicNumber, m_rHwBuf.mSize, m_rHwBuf.mStride, m_rHwBuf.mTimeStamp);

        if (m_bAbort || m_bPreStop)
            return NULL;
    }

    // dump afo data for debug
    if (bEnable)
    {
        static MUINT32 count = 0;
        char fileName[64];
        char folderName[64];
        MINT32 slen;
#define AFO_DIR "/sdcard/afo"
        slen = strlen(AFO_DIR) + 1;
        strncpy(folderName, AFO_DIR, slen);
        sprintf(fileName, "/data/vendor/AFObitTrue/afo_%d.raw", m_rHwBuf.mMagicNumber);
        if (dumpHwBuf(fileName, folderName, m_rHwBuf))
        {
            CAM_LOGD_IF(m_bDebugEnable, "%s\n", fileName);
        }
    }

    // dump afo hw data for ut
    if (bEnableHW)
    {
        char fileName[64];
        char folderName[64];
        MINT32 slen;
#define AFOHW_DIR "/sdcard/afoHwBuf"
        slen = strlen(AFOHW_DIR) + 1;
        strncpy(folderName, AFOHW_DIR, slen);
        sprintf(fileName, "/data/vendor/AFObitTrue/afo_%d.hwbuf", m_rHwBuf.mMagicNumber);
        if (dumpHwBuf(fileName, folderName, m_rHwBuf, bEnableHW))
        {
            CAM_LOGD_IF(m_bDebugEnable, "%s\n", fileName);
        }
    }

    if(m_bAbort || m_bPreStop)
        return NULL;

    return &m_rHwBuf;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
AFOBufMgrImp::
abortDequeue()
{
    if (m_pSttPipe != NULL  && m_isAfoEnable == 1)
        m_pSttPipe->abortDma(m_rPort, LOG_TAG);

    CAM_LOGD("[%s] abortDma done\n", __FUNCTION__);

    CAM_LOGD("[%s] +\n", __FUNCTION__);
    m_bAbort = MTRUE;
    m_u4FrameCount = 0;
    CAM_LOGD("[%s] -\n", __FUNCTION__);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
AFOBufMgrImp::
notifyPreStop()
{
    CAM_LOGD("[%s] \n", __FUNCTION__);
    m_bPreStop = MTRUE;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
AFOBufMgrImp::
reset()
{
    CAM_LOGD("[%s] Dev(%d)\n", __FUNCTION__, m_i4SensorDev);
    /* W+T susprned/resume flow , skip enque after deque done when reset() is called */
    m_HW_EnQBufIdx = m_HW_DeQBufIdx;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
AFOBufMgrImp::
waitDequeue()
{
#define TIME_25MS 25000
    if ((m_u8TimeEnque - m_u8TimeDequeDone > TIME_25MS) && (m_u8TimeEnque > m_u8TimeDequeDone))
    {
        CAM_TRACE_BEGIN("waitDequeue");

        AE_PERFRAME_INFO_T AEPerframeInfo;
        IAeMgr::getInstance().getAEInfo(m_i4SensorDev, AEPerframeInfo);
        MUINT32 i, DequeCnt;
        MUINT64 Temp = (m_u8TimeEnque - m_u8TimeDequeDone) * 1000;

        DequeCnt =  Temp / AEPerframeInfo.rAEUpdateInfo.u8FrameDuration_ns;
        if (DequeCnt > 0)
        {
            if (((Temp - AEPerframeInfo.rAEUpdateInfo.u8FrameDuration_ns * DequeCnt) * 2) > (MUINT64)AEPerframeInfo.rAEUpdateInfo.u8FrameDuration_ns)
                DequeCnt += 1;
            if (DequeCnt > P1_HW_BUF_MAX_NUM)
                DequeCnt = P1_HW_BUF_MAX_NUM;
        }

        CAM_LOGD("[%s] DequeCnt = %d, FrameDuration = %lld", __FUNCTION__, DequeCnt, AEPerframeInfo.rAEUpdateInfo.u8FrameDuration_ns);
        CAM_LOGD("[%s] TimeEnque = %lld, TimeDequeDone = %lld", __FUNCTION__, m_u8TimeEnque, m_u8TimeDequeDone);

        /* In order to get the latest AFO buffer */
        for (i = 0; i < DequeCnt; i++)
        {
            CAM_LOGD("[%s] Cnt = %d\n", __FUNCTION__, i);

            /* Deque */
            CAM_TRACE_BEGIN("STT deque");
            MUINT32 QBufIdx = m_HW_DeQBufIdx & P1_HW_BUF_MASK;
            MBOOL ret = m_pSttPipe->deque(m_rPort, m_HW_QBuf[QBufIdx]);
            CAM_TRACE_END();
            int size = m_HW_QBuf[QBufIdx].mvOut.size();
            if (m_bAbort || m_bPreStop || (size == 0 && ret == MTRUE) || (ret == MFALSE))
            {
                CAM_LOGE("[%s] m_pSttPipe deque fail", __FUNCTION__);
                return -1;
            }
            CAM_LOGD_IF(m_bDebugEnable, "Deque HW Buffer Idx : %d (%d)\n", QBufIdx, m_HW_DeQBufIdx);
            m_HW_DeQBufIdx++;

            /* Enque */
            for (MINT32 j = 0; j < P1_HW_BUF_MAX_NUM; j++)
            {
                MUINT32 QBufIdx = m_HW_EnQBufIdx & P1_HW_BUF_MASK;
                CAM_LOGD_IF(m_bDebugEnable, "Enque HW Buffer Idx : %d (%d)\n", QBufIdx, m_HW_EnQBufIdx);
                CAM_TRACE_BEGIN("STT enque");
                MINT32 i4Ret = m_pSttPipe->enque(m_HW_QBuf[QBufIdx]);
                CAM_TRACE_END();
                if (!i4Ret)
                {
                    CAM_LOGE("[%s] m_pSttPipe enque fail", __FUNCTION__);
                    return -1;
                }

                m_HW_EnQBufIdx++;

                if (m_HW_EnQBufIdx >= m_HW_DeQBufIdx)
                    break;
            }
        }
        CAM_LOGD("[%s] -\n", __FUNCTION__);
        CAM_TRACE_END();
    }

    return 0;
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
AFOBufMgrImp::
sttDeque()
{
    if (m_pSttPipe != NULL  && m_isAfoEnable == 1)
    {
        MUINT32 QBufIdx = m_HW_DeQBufIdx & P1_HW_BUF_MASK;
        GETTIMESTAMP(m_u8TimeDeque);
        CAM_TRACE_BEGIN("sttpipe deque");
        MBOOL ret = m_pSttPipe->deque(m_rPort, m_HW_QBuf[QBufIdx]);
        CAM_TRACE_END();
        int size = m_HW_QBuf[QBufIdx].mvOut.size(); // number of buffers from m_HW_QBuf[QBufIdx].mvOut

        if(m_bAbort || m_bPreStop || (size == 0 && ret == MTRUE))
            return MFALSE;

        if (ret == MFALSE)
        {
            CAM_LOGE("[%s] m_pSttPipe deque fail", __FUNCTION__);
            return MFALSE;
        }
        GETTIMESTAMP(m_u8TimeDequeDone);
        CAM_LOGD_IF(m_bDebugEnable, "Deque HW Buffer Idx : %d (%d)\n", QBufIdx, m_HW_DeQBufIdx);
        m_HW_DeQBufIdx++;
        return MTRUE;
    }

    return MFALSE;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
AFOBufMgrImp::
sttEnque()
{
    if (m_pSttPipe != NULL  && m_isAfoEnable == 1)
    {
        if (m_HW_EnQBufIdx >= m_HW_DeQBufIdx)
        {
            return MFALSE;
        }

        MUINT32 QBufIdx = m_HW_EnQBufIdx & P1_HW_BUF_MASK;
        CAM_LOGD_IF(m_bDebugEnable, "Enque HW Buffer Idx : %d (%d)\n", QBufIdx, m_HW_EnQBufIdx);
        CAM_TRACE_BEGIN("sttpipe enque");
        MINT32 i4Ret = m_pSttPipe->enque(m_HW_QBuf[QBufIdx]);
        CAM_TRACE_END();

        if (!i4Ret)
        {
            CAM_LOGE("[%s] m_pSttPipe enque fail", __FUNCTION__);
            return MFALSE;
        }

        GETTIMESTAMP(m_u8TimeEnque);
        m_HW_EnQBufIdx++;
        return MTRUE;
    }

    return MFALSE;
}

#endif

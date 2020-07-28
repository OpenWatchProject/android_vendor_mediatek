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
#define LOG_TAG "pso_buf_mgr"

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

#if (CAM3_3A_ISP_40_EN)
#include "../common/hal3a/v1.0/buf_mgr/IBufMgr.h"
#else
#include "IBufMgr.h"
#endif

#include <mtkcam/drv/iopipe/CamIO/IStatisticPipe.h>
#include <mtkcam/drv/iopipe/CamIO/IHalCamIO.h>
#include <private/IopipeUtils.h>

//PSOSeparation
#include <utils/Condition.h>
#include <ae_param.h>
#include <ae_tuning_custom.h>

using namespace android;
using namespace NS3Av3;
using namespace NSCam::NSIoPipe;
using namespace NSCam::NSIoPipe::NSCamIOPipe;

typedef struct
{
    MINT32                  m_i4AwbLineSize;//aaoNoAWBSz
    MINT32                  m_i4AeSize     ;// in byte
    MINT32                  m_i4HdrSize    ;// in byte
    MINT32                  m_i4TsfSize    ;// in byte
    MINT32                  m_i4LineSize   ;// lineByte
    MINT32                  m_i4SkipSize   ;
    MINT32                  m_i4BlkNumX;//m_rTsfEnvInfo.ImgWidth;
    MINT32                  m_i4BlkNumY;//m_rTsfEnvInfo.ImgHeight;
    MBOOL                   m_bEnableHDRYConfig;
    MBOOL                   m_bEnableOverCntConfig;
    MBOOL                   m_bEnableTSFConfig;
    MBOOL                   m_bDisablePixelHistConfig;
    MUINT32                 m_u4AEOverExpCntShift;
} PSO_BUF_CONFIG_T;

typedef struct
{
    MVOID*                  m_pPSOSepAWBBuf;
    MVOID*                  m_pPSOSepAEBuf;
    MVOID*                  m_pPSOSepHistBuf;
    MVOID*                  m_pPSOSepOverCntBuf;
    MVOID*                  m_pPSOSepLSCBuf;
} PSO_BUF_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//PSOSeparation
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
struct PSOStatisticBufInfo : public StatisticBufInfo
{
    MUINT32                 m_u4PSOSepAWBsize;
    MUINT32                 m_u4PSOSepAEsize;
    MUINT32                 m_u4PSOSepsize;
    MVOID*                  m_pPSOSepBuf;
    PSO_BUF_CONFIG_T        m_rCfg;
    PSO_BUF_T               m_rBufs;
    virtual MVOID*          getPart(MUINT32 u4ParId) const;
    virtual MUINT32         getPartSize(MUINT32 u4ParId) const;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//PSOSeparation
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class PSOStatisticBuf
{
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data member
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    mutable Mutex           m_Lock;
    MBOOL                   m_fgLog;
    MBOOL                   m_fgCmd;
    MBOOL                   m_fgFlush;
    Condition               m_Cond;
    PSOStatisticBufInfo     m_rBufInfo;
    MUINT8                  m_ufgOpt;
    MUINT32                 m_u4width ;
    MUINT32                 m_u4height;
    MUINT32                 m_u4BlocksizeX;
    MUINT32                 m_u4BlocksizeY;
    ConfigInfo_T            m_rConfigInfo;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor/Dtor.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:    ////    Disallowed.
    //  Copy constructor is disallowed.
    PSOStatisticBuf(PSOStatisticBuf const&);
    //  Copy-assignment operator is disallowed.
    PSOStatisticBuf& operator=(PSOStatisticBuf const&);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    PSOStatisticBuf();
    virtual ~PSOStatisticBuf();

    virtual MBOOL allocateBuf(MUINT8 const ufgStatOpt, MBOOL bEnablePixelBaseHist,
                            MUINT32 const u4AAWidth,MUINT32 const u4AAHight, MUINT32 const i4WindowNumX, MUINT32 const i4WindowNumY,
                            const ConfigInfo_T& rConfigInfo);
    virtual MBOOL freeBuf();
    virtual MBOOL write(const BufInfo& rBufInfo, const MUINT32 u4FrameCount);
    virtual StatisticBufInfo* read();
    virtual MBOOL flush();
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class PSOBufMgrImp : public PSOBufMgr
{
public:
    virtual MBOOL destroyInstance();
                  PSOBufMgrImp(MINT32 i4SensorDev, MINT32 i4SensorIdx, const ConfigInfo_T& rConfigInfo);
    virtual      ~PSOBufMgrImp();

    virtual MBOOL dequeueHwBuf();
    virtual MBOOL enqueueHwBuf();
    virtual StatisticBufInfo* dequeueSwBuf();
    virtual MVOID abortDequeue();
    virtual MINT32 waitDequeue(){return 0;};
    virtual MVOID notifyPreStop();
    virtual MVOID reset() {};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data member
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    MINT32           const m_i4SensorDev;
    MINT32           const m_i4SensorIdx;
    MBOOL                  m_bDebugEnable;
    MBOOL                  m_bPSODumpEn;
    IStatisticPipe*        m_pSttPipe;
    MUINT32                m_rBufIndex;  // the index of write buffers.
    PortID                 m_rPort;
    MBOOL                  m_bAbort;
    PSOStatisticBuf        m_rBufInfo[MAX_STATISTIC_BUFFER_CNT];
    // MUINT8                 m_ufgStatOpt;
    MUINT32                m_u4WindowNumX;
    MUINT32                m_u4WindowNumY;
    MUINT32                m_u4AAWidth;
    MUINT32                m_u4AAHight;
    MUINT32                m_u4FrameCount;
    INormalPipe*           m_pCamIO;//NSCam::NSIoPipe::NSCamIOPipe::INormalPipe
    ConfigInfo_T           m_rConfigInfo;
    MBOOL                  m_bPreStop;
};

#if (CAM3_3ATESTLVL > CAM3_3AUT)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#if (CAM3_3A_ISP_50_EN)
PSOBufMgr*
PSOBufMgr::
createInstance(MINT32 const i4SensorDev, MINT32 const i4SensorIdx, const ConfigInfo_T& rConfigInfo, STT_CFG_INFO_T const sttInfo)
{
    PSOBufMgrImp* pObj = new PSOBufMgrImp(i4SensorDev,i4SensorIdx, rConfigInfo);
    return pObj;
}
#else
PSOBufMgr*
PSOBufMgr::
createInstance(MINT32 const i4SensorDev, MINT32 const i4SensorIdx, const ConfigInfo_T& rConfigInfo)
{
    PSOBufMgrImp* pObj = new PSOBufMgrImp(i4SensorDev,i4SensorIdx, rConfigInfo);
    return pObj;
}
#endif
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++r+++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
PSOBufMgrImp::
destroyInstance()
{
    delete this;
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
PSOBufMgrImp::
PSOBufMgrImp(MINT32 i4SensorDev, MINT32 i4SensorIdx, const ConfigInfo_T& rConfigInfo)
    : m_i4SensorDev(i4SensorDev)
    , m_i4SensorIdx(i4SensorIdx)
    , m_bDebugEnable(MFALSE)
    , m_bPSODumpEn(MFALSE)
    , m_pSttPipe(NULL)
    , m_rBufIndex(0)
    , m_rPort(PORT_PSO)
    , m_bAbort(MFALSE)
    , m_u4WindowNumX(0)
    , m_u4WindowNumY(0)
    , m_u4AAWidth(0)
    , m_u4AAHight(0)
    , m_u4FrameCount(0)
    , m_pCamIO(NULL)
    , m_bPreStop(MFALSE)
{
    m_rConfigInfo = rConfigInfo;

    const AE_PARAM_T* pAeParam;
    switch (m_i4SensorDev)
    {
        default:
        case ESensorDev_Main:
            pAeParam = &getAEParam<ESensorDev_Main>();
            break;
        case ESensorDev_Sub:
            pAeParam = &getAEParam<ESensorDev_Sub>();
            break;
        case ESensorDev_MainSecond:
            pAeParam = &getAEParam<ESensorDev_MainSecond>();
            break;
        case ESensorDev_SubSecond:
            pAeParam = &getAEParam<ESensorDev_SubSecond>();
            break;
    }

    m_u4WindowNumX = pAeParam->strAEParasetting.u4AEWinodwNumX; //rAwbStatParma.i4WindowNumX;
    m_u4WindowNumY = pAeParam->strAEParasetting.u4AEWinodwNumY; //rAwbStatParma.i4WindowNumY;

    // NormalIOPipe create instance
    if (m_pCamIO == NULL)
    {
        m_pCamIO = (INormalPipe*)INormalPipeUtils::get()->createDefaultNormalPipe(m_i4SensorIdx, LOG_TAG);
        if (m_pCamIO == NULL)
        {
            CAM_LOGE("Fail to create NormalPipe");
        }
    }

    if (m_pCamIO != NULL)
        m_pCamIO->sendCommand(NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_GET_HBIN_INFO,
                            (MINTPTR)&m_u4AAWidth, (MINTPTR)&m_u4AAHight, 0);

    if(m_u4AAWidth <= 0 || m_u4AAHight <= 0)
        CAM_LOGE("AAWidth and AAHight val error");

    for(int i = 0; i < MAX_STATISTIC_BUFFER_CNT; i++)
    {
        CAM_LOGD("[%s]  i(%d), BitMode(%d) \n", __FUNCTION__, i, m_rConfigInfo.i4BitMode);
        m_rBufInfo[i].allocateBuf(0, MFALSE, m_u4AAWidth, m_u4AAHight, m_u4WindowNumX, m_u4WindowNumY, m_rConfigInfo);
    }

    m_bDebugEnable = ::property_get_int32("vendor.debug.pso_mgr.enable", 0);
    m_bPSODumpEn = ::property_get_int32("vendor.pso.dump.enable", 0);

    m_pSttPipe = IStatisticPipe::createInstance(m_i4SensorIdx, LOG_TAG);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
PSOBufMgrImp::
~PSOBufMgrImp()
{
    for(int i = 0; i < MAX_STATISTIC_BUFFER_CNT; i++)
    {
        CAM_LOGD("[%s]  i = %d \n", __FUNCTION__, i);
        m_rBufInfo[i].freeBuf();
    }

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
MINT32
PSOBufMgrImp::
dequeueHwBuf()
{
    CAM_LOGD_IF(m_bDebugEnable,"[%s] m_i4SensorDev(%d) + \n", __FUNCTION__, m_i4SensorDev);

    if(m_bAbort || m_bPreStop)
        return -1;

    if(m_pSttPipe != NULL)
    {
        DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "dequeueHwBuf", 0);
        // deque HW buffer from driver.
        QBufInfo    rDQBuf;
        CAM_TRACE_BEGIN("PSO STT deque");
        MBOOL ret = m_pSttPipe->deque(m_rPort, rDQBuf);
        CAM_TRACE_END();
        int size = rDQBuf.mvOut.size();
        DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "dequeueHwBuf", 1);
        if(m_bAbort || m_bPreStop || (size == 0 && ret == MTRUE)){
            DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "dequeueHwBuf", -1);
            return -1;
        }

        if(ret == MFALSE)
        {
            CAM_LOGE("[%s] m_pSttPipe deque fail", __FUNCTION__);
            DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "dequeueHwBuf", -1);
            return MFALSE;
        }
        // get the last HW buffer to SW Buffer.
        int index = m_rBufIndex;
        m_u4FrameCount++;
        BufInfo rLastBuf = rDQBuf.mvOut.at( size-1 );

        // copy the last HW buffer to SW Buffer.

        DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "dequeueHwBuf", 2);
        CAM_TRACE_BEGIN("PSO STT clone");
        m_rBufInfo[index].write(rLastBuf, m_u4FrameCount);
        CAM_TRACE_END();
        DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "dequeueHwBuf", 3);

        CAM_LOGD_IF(m_bDebugEnable, "port(%d), index(%d), va[%d]/pa[0x%08x]/#(%d), Size(%d), rawType(%d)",
                  (MINT32)m_rPort.index, index, (MINT32)rLastBuf.mVa, (MINT32)rLastBuf.mPa, rLastBuf.mMetaData.mMagicNum_hal, rLastBuf.mSize, rLastBuf.mMetaData.mRawType);

        // dump pso data for debug
        if (m_bPSODumpEn) {
            char fileName[64];
            static MUINT32 count;
            sprintf(fileName, "/sdcard/pso/pso_%d_%d.raw", rLastBuf.mMetaData.mMagicNum_hal, count++);

            FILE *fp = fopen(fileName, "w");
            if (NULL == fp)
            {
                CAM_LOGE("fail to open file to save img: %s", fileName);
                MINT32 err = mkdir("/sdcard/pso", S_IRWXU | S_IRWXG | S_IRWXO);
                CAM_LOGD("err = %d", err);
            } else
            {
                CAM_LOGD_IF(m_bDebugEnable,"%s\n", fileName);
                fwrite(reinterpret_cast<void *>(rLastBuf.mVa), 1, rLastBuf.mSize, fp);
                fclose(fp);
            }
        }

        if(m_bAbort || m_bPreStop)
            return -1;
        // enque HW buffer back driver
        CAM_LOGD_IF(m_bDebugEnable, "enque Hw buffer back driver.\n");
        CAM_TRACE_BEGIN("PSO STT enque");
        m_pSttPipe->enque(rDQBuf);
        CAM_TRACE_END();
        DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "dequeueHwBuf", -1);
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
PSOBufMgrImp::
enqueueHwBuf()
{
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
StatisticBufInfo*
PSOBufMgrImp::
dequeueSwBuf()
{
    CAM_LOGD_IF(m_bDebugEnable,"[%s] index(%d) +\n", __FUNCTION__, m_rBufIndex);
    if(m_bAbort || m_bPreStop)
        return NULL;
    DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "dequeueSwBuf", 0);
    int r_index = m_rBufIndex;
    StatisticBufInfo* pBuf = m_rBufInfo[r_index].read();
    DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "dequeueSwBuf", 1);
    // switch buffer
    m_rBufIndex = ( r_index + 1 ) % MAX_STATISTIC_BUFFER_CNT;
    CAM_LOGD_IF(m_bDebugEnable,"[%s] m_i4SensorDev(%d), port(%d), r_index(%d), w_index(%d)\n", __FUNCTION__, m_i4SensorDev, (MINT32)m_rPort, r_index, (MINT32)m_rBufIndex);
    DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "dequeueSwBuf", -1);
    return pBuf;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
PSOBufMgrImp::
abortDequeue()
{
    CAM_LOGD("[%s] m_rBufIndex(%d) +\n", __FUNCTION__, m_rBufIndex);
    m_bAbort = MTRUE;
    m_u4FrameCount = 0;
    if(m_pSttPipe != NULL)
        m_pSttPipe->abortDma(m_rPort,LOG_TAG);
    CAM_LOGD("[%s] abortDma done\n", __FUNCTION__);
    for(int i = 0; i < MAX_STATISTIC_BUFFER_CNT; i++)
        m_rBufInfo[i].flush();
    CAM_LOGD("[%s] m_rBufIndex(%d) -\n", __FUNCTION__, m_rBufIndex);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
PSOBufMgrImp::
notifyPreStop()
{
    CAM_LOGD("[%s] m_rBufIndex(%d) +\n", __FUNCTION__, m_rBufIndex);
    m_bPreStop = MTRUE;
    CAM_LOGD("[%s] m_rBufIndex(%d) -\n", __FUNCTION__, m_rBufIndex);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//PSOSeparation
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID*
PSOStatisticBufInfo::
getPart(MUINT32 u4ParId) const
{
    switch (u4ParId)
    {
        case PSOSepAWBBuf:
            return (m_rBufs.m_pPSOSepAWBBuf);
            break;
        case PSOSepAEBuf:
            return (m_rBufs.m_pPSOSepAEBuf);
            break;
        case PSOSepBuf:
            return (m_pPSOSepBuf);
            break;
        default:
            return NULL;
            break;
    }
}

MUINT32
PSOStatisticBufInfo::
getPartSize(MUINT32 u4ParId) const
{
    switch (u4ParId)
    {
        case PSOSepAWBBuf:
            return m_u4PSOSepAWBsize;
            break;
        case PSOSepAEBuf:
            return m_u4PSOSepAEsize;
            break;
        case PSOSepBuf:
            return m_u4PSOSepsize;
            break;
        default:
            return MFALSE;
            break;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Function.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
PSOStatisticBuf::
PSOStatisticBuf()
    : m_fgLog(MFALSE)
    , m_fgCmd(MFALSE)
    , m_fgFlush(MFALSE)
    , m_ufgOpt(0)
    , m_u4width(0)
    , m_u4height(0)
    , m_u4BlocksizeX(0)
    , m_u4BlocksizeY(0)
{
    memset(&m_rBufInfo.m_rBufs, 0, sizeof(PSO_BUF_T));
    memset(&m_rBufInfo.m_rCfg, 0, sizeof(PSO_BUF_CONFIG_T));
    m_rBufInfo.mVa = (MUINTPTR)NULL;
    m_rBufInfo.mSize = 0;
    m_rBufInfo.m_pPSOSepBuf = NULL;
    m_rBufInfo.m_u4PSOSepAEsize = 0;
    m_rBufInfo.m_u4PSOSepAWBsize = 0;
    m_rBufInfo.m_u4PSOSepsize = 0;

}

PSOStatisticBuf::
~PSOStatisticBuf()
{
    freeBuf();
}

MBOOL
PSOStatisticBuf::
allocateBuf(MUINT8 const /*ufgStatOpt*/, MBOOL /*bEnablePixelBaseHist*/,
            MUINT32 const u4AAWidth, MUINT32 const u4AAHight, MUINT32 const u4WindowNumX, MUINT32 const u4WindowNumY,
            const ConfigInfo_T& rConfigInfo)
{
    Mutex::Autolock lock(m_Lock);

    m_fgLog = property_get_int32("vendor.debug.statistic_buf.enable", 0);

    m_rConfigInfo = rConfigInfo;
    CAM_LOGD_IF(m_fgLog, "[%s] BitMode(%d) \n", __FUNCTION__, m_rConfigInfo.i4BitMode);

    m_rBufInfo.m_rCfg.m_i4BlkNumX = u4WindowNumX;//m_rTsfEnvInfo.ImgWidth;(120)
    m_rBufInfo.m_rCfg.m_i4BlkNumY = u4WindowNumY;//m_rTsfEnvInfo.ImgHeight;(90)

    m_u4width = u4AAWidth;
    m_u4height = u4AAHight;

    m_rBufInfo.m_rCfg.m_i4AwbLineSize = m_rBufInfo.m_rCfg.m_i4BlkNumX * 4;
    m_rBufInfo.m_rCfg.m_i4AeSize      = ((m_rBufInfo.m_rCfg.m_i4BlkNumX + 3)/4) * 4;    // in byte

    if(m_rConfigInfo.i4BitMode == EBitMode_12Bit)
        m_rBufInfo.m_rCfg.m_i4HdrSize     = ((m_rBufInfo.m_rCfg.m_i4BlkNumX*4 + 31)/32)*4;  // in byte
    else if(m_rConfigInfo.i4BitMode == EBitMode_14Bit)
        m_rBufInfo.m_rCfg.m_i4HdrSize     = ((m_rBufInfo.m_rCfg.m_i4BlkNumX*8 + 31)/32)*4;  // in byte

    m_rBufInfo.m_rCfg.m_i4TsfSize     = ((m_rBufInfo.m_rCfg.m_i4BlkNumX*32 + 31)/32)*4; // in byte
    m_rBufInfo.m_rCfg.m_i4LineSize    = m_rBufInfo.m_rCfg.m_i4AwbLineSize + m_rBufInfo.m_rCfg.m_i4AeSize + m_rBufInfo.m_rCfg.m_i4HdrSize + m_rBufInfo.m_rCfg.m_i4TsfSize;//lineByte

    m_rBufInfo.m_u4PSOSepAWBsize = PSO_SEP_AWB_SIZE_COEF * u4WindowNumX * u4WindowNumY;
    m_rBufInfo.m_u4PSOSepAEsize = PSO_SEP_AE_SIZE_COEF * u4WindowNumX*u4WindowNumY;
    m_rBufInfo.m_u4PSOSepsize = m_rBufInfo.m_u4PSOSepAWBsize+m_rBufInfo.m_u4PSOSepAEsize;

    m_rBufInfo.m_pPSOSepBuf = (MVOID*)malloc(m_rBufInfo.m_u4PSOSepsize);
    m_rBufInfo.m_rBufs.m_pPSOSepAWBBuf = m_rBufInfo.m_pPSOSepBuf;
    m_rBufInfo.m_rBufs.m_pPSOSepAEBuf = (MUINT8 *)m_rBufInfo.m_pPSOSepBuf + m_rBufInfo.m_u4PSOSepAWBsize;

    if(m_rBufInfo.m_pPSOSepBuf != NULL && m_rBufInfo.m_u4PSOSepsize != 0){
        CAM_LOGD_IF(m_fgLog, "[%s] m_rBufInfo->m_pPSOSepBuf = 0x%p, \n", __FUNCTION__, m_rBufInfo.m_pPSOSepBuf);
        CAM_LOGD_IF(m_fgLog, "[%s] m_rBufInfo->m_u4PSOSepsize = 0x%d, \n", __FUNCTION__, (MINT32)m_rBufInfo.m_u4PSOSepsize);
    }

    if(m_rBufInfo.m_pPSOSepBuf == NULL)
    {
        CAM_LOGE("m_rBufInfo.m_pPSOSepBuf == NULL");
        return MFALSE;
    }

    return MTRUE;
}

MBOOL
PSOStatisticBuf::
freeBuf()
{
    Mutex::Autolock lock(m_Lock);
    if(m_rBufInfo.m_pPSOSepBuf != NULL && m_rBufInfo.m_u4PSOSepsize != 0){
        CAM_LOGD_IF(m_fgLog, "[%s] m_rBufInfo->m_pPSOSepBuf = 0x%p, \n", __FUNCTION__, m_rBufInfo.m_pPSOSepBuf);
        free(m_rBufInfo.m_pPSOSepBuf);
    }

    memset(&m_rBufInfo.m_rBufs, 0, sizeof(PSO_BUF_T));
    memset(&m_rBufInfo.m_rCfg, 0, sizeof(PSO_BUF_CONFIG_T));
    m_rBufInfo.m_pPSOSepBuf = NULL;
    m_rBufInfo.m_u4PSOSepAEsize = 0;
    m_rBufInfo.m_u4PSOSepAWBsize = 0;
    m_rBufInfo.m_u4PSOSepsize = 0;

    return MTRUE;
}

MBOOL
PSOStatisticBuf::
write(const BufInfo& rBufInfo, const MUINT32 u4FrameCount)
{
    Mutex::Autolock lock(m_Lock);

    if(m_rBufInfo.m_pPSOSepBuf == NULL || m_rBufInfo.m_u4PSOSepsize ==0){
        CAM_LOGE("m_rBufInfo.m_pPSOSepBuf == NULL");
        CAM_LOGE("m_rBufInfo.m_u4PSOSepsize == 0");
        return MFALSE;
    }

    CAM_LOGD_IF(m_fgLog, "[%s] + m_fgCmd(%d) Magic(%d)\n", __FUNCTION__, m_fgCmd, rBufInfo.mMetaData.mMagicNum_hal);

    // copy buffer member
    m_rBufInfo.mMagicNumber = rBufInfo.mMetaData.mMagicNum_hal;
    m_rBufInfo.mSize = rBufInfo.mSize;
    m_rBufInfo.mStride = rBufInfo.mStride;
    m_rBufInfo.mFrameCount = u4FrameCount;
    m_rBufInfo.mTimeStamp = rBufInfo.mMetaData.mTimeStamp;
    m_rBufInfo.mRawType = ((EPipeRawFmt)rBufInfo.mMetaData.mRawType == EPipe_BEFORE_LSC) ? PSO_OUTPUT_AFTER_OB : PSO_OUTPUT_BEFORE_OB;
    // separate buffer
    {
        const MUINT8* pSrc = reinterpret_cast<const MUINT8*>(rBufInfo.mVa);
        const PSO_BUF_CONFIG_T& rCfg = m_rBufInfo.m_rCfg;
        PSO_BUF_T& rBuf = m_rBufInfo.m_rBufs;
        MUINT8* pawb = (MUINT8*)rBuf.m_pPSOSepAWBBuf;
        MUINT8* pae = (MUINT8*)rBuf.m_pPSOSepAEBuf;

        for(int j=0; j<rCfg.m_i4BlkNumY; j++)
        {
            if(pawb!=0)//awb
            {
                memcpy(pawb, pSrc, rCfg.m_i4BlkNumX*4);
                pawb+=rCfg.m_i4BlkNumX*4;
            }//awb end

            if(pae!=0)//ae
            {
                memcpy(pae, (pSrc + rCfg.m_i4AwbLineSize), rCfg.m_i4BlkNumX);
                pae+=rCfg.m_i4BlkNumX;
            }//ae end

            pSrc += rCfg.m_i4LineSize;
        }
    }
    CAM_LOGD_IF(m_fgLog,"[%s] psoSeparation success!", __FUNCTION__);

    m_fgCmd = MTRUE;
    // broadcast for writing buffer done.
    m_Cond.broadcast();
    CAM_LOGD_IF(m_fgLog, "[%s] - m_fgCmd(%d) Magic(%d)\n", __FUNCTION__, m_fgCmd, rBufInfo.mMetaData.mMagicNum_hal);
    return MTRUE;
}

StatisticBufInfo*
PSOStatisticBuf::
read()
{
    Mutex::Autolock lock(m_Lock);

    if(m_rBufInfo.m_pPSOSepBuf == NULL){
        CAM_LOGE("m_rBufInfo.m_pPSOSepBuf == NULL");
        return NULL;
    }

    if(m_fgFlush)
        return NULL;

    CAM_LOGD_IF(m_fgLog, "[%s] + m_fgCmd(%d)", __FUNCTION__, m_fgCmd);
    // wait for writing buffer done.
    if(!m_fgCmd)
    {
        CAM_LOGD_IF(m_fgLog, "[%s] wait\n", __FUNCTION__);
        m_Cond.wait(m_Lock);
    }
    m_fgCmd = MFALSE;
    CAM_LOGD_IF(m_fgLog, "[%s] - m_fgCmd(%d) Magic(%d)\n", __FUNCTION__, m_fgCmd, m_rBufInfo.mMagicNumber);
    if(m_fgFlush)
    {
        return NULL;
    }
    else
        return &m_rBufInfo;
}

MBOOL
PSOStatisticBuf::
flush()
{
    Mutex::Autolock lock(m_Lock);
    CAM_LOGD_IF(m_fgLog, "[%s] + m_fgCmd(%d)\n", __FUNCTION__, m_fgCmd);
    m_fgFlush = MTRUE;
    if(!m_fgCmd)
        m_Cond.broadcast();
    CAM_LOGD_IF(m_fgLog, "[%s] -\n", __FUNCTION__);
    return MTRUE;
}

#endif

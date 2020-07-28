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
#define LOG_TAG "aeo_buf_mgr"

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

#if CAM3_FAKE_SENSOR_DRV
#include "fake_sensor_drv/IStatisticPipe.h"
#else
#include <mtkcam/drv/iopipe/CamIO/IStatisticPipe.h>
#endif

#include <mtkcam/drv/iopipe/CamIO/IHalCamIO.h>
#include <private/IopipeUtils.h>
#include <unistd.h>

#include <ae_mgr_if.h>

using namespace NS3Av3;
using namespace NSCam::NSIoPipe;
using namespace NSCam::NSIoPipe::NSCamIOPipe;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  AEO buffer
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define AEO_STT_DATA_SIZE (10)                    // byte
#define AEO_STT_PLINE_NUM (90)
#define AEO_STT_BLOCK_NUM (120)
#define AEO_STT_HIST_BIN_NUM (128)
#define AEO_STT_HIST_DATA_SIZE (3)                // byte
#define AEO_STT_HIST_NUM (4)
#define AEO_BUF_SIZE (AEO_STT_DATA_SIZE * AEO_STT_PLINE_NUM * AEO_STT_BLOCK_NUM + AEO_STT_HIST_BIN_NUM * AEO_STT_HIST_DATA_SIZE * AEO_STT_HIST_NUM)

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class AEOBufMgrImp : public AEOBufMgr
{
public:
    virtual MBOOL destroyInstance();
                  AEOBufMgrImp(MINT32 i4SensorDev, MINT32 i4SensorIdx);
    virtual      ~AEOBufMgrImp();

    virtual MBOOL dequeueHwBuf();
    virtual MBOOL enqueueHwBuf();
    virtual StatisticBufInfo* dequeueSwBuf();
    virtual MVOID abortDequeue();
    virtual MINT32 waitDequeue(){return 0;};
    virtual MVOID notifyPreStop();

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data member
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    MINT32                 m_i4Tglineno;

private:
    MINT32           const m_i4SensorDev;
    MINT32           const m_i4SensorIdx;
    MBOOL                  m_bDebugEnable;
    IStatisticPipe*        m_pSttPipe;
    MUINT32                m_rBufIndex;  // the index of write buffers.
    StatisticBuf           m_rBufInfo[MAX_STATISTIC_BUFFER_CNT];
    PortID                 m_rPort;
    MBOOL                  m_bAbort;
    MINT32                 m_i4UserKey;
    MBOOL                  m_bPreStop;
};

#if (CAM3_3ATESTLVL > CAM3_3AUT)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#if (CAM3_3A_ISP_50_EN)
AEOBufMgr*
AEOBufMgr::
createInstance(MINT32 const i4SensorDev, MINT32 const i4SensorIdx, MINT32 const i4Tglineno, STT_CFG_INFO_T const sttInfo)
{
    AEOBufMgrImp* pObj = new AEOBufMgrImp(i4SensorDev,i4SensorIdx);
    pObj->m_i4Tglineno = i4Tglineno;
    return pObj;
}
#else
AEOBufMgr*
AEOBufMgr::
createInstance(MINT32 const i4SensorDev, MINT32 const i4SensorIdx, MINT32 const i4Tglineno)
{
    AEOBufMgrImp* pObj = new AEOBufMgrImp(i4SensorDev,i4SensorIdx);
    pObj->m_i4Tglineno = i4Tglineno;
    return pObj;
}
#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
AEOBufMgrImp::
destroyInstance()
{
    delete this;
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AEOBufMgrImp::
AEOBufMgrImp(MINT32 i4SensorDev, MINT32 i4SensorIdx)
    : m_i4Tglineno(0)
    , m_i4SensorDev(i4SensorDev)
    , m_i4SensorIdx(i4SensorIdx)
    , m_bDebugEnable(MFALSE)
    , m_pSttPipe(NULL)
    , m_rBufIndex(0)
    , m_rPort(PORT_AAO)
    , m_bAbort(MFALSE)
    , m_i4UserKey(0)
    , m_bPreStop(MFALSE)
{
    for(int i = 0; i < MAX_STATISTIC_BUFFER_CNT; i++)
    {
        CAM_LOGD("[%s]  i = %d \n", __FUNCTION__, i);
        m_rBufInfo[i].allocateBuf(AEO_BUF_SIZE);
    }

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.aeo_mgr.enable", value, "0");
    m_bDebugEnable = atoi(value);

    m_pSttPipe = IStatisticPipe::createInstance(m_i4SensorIdx, LOG_TAG);
    if(m_pSttPipe != NULL)
    {
        m_i4UserKey = m_pSttPipe->attach(LOG_TAG);
        CAM_LOGD("[%s]  m_i4UserKey(%d) \n", __FUNCTION__, m_i4UserKey);
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AEOBufMgrImp::
~AEOBufMgrImp()
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
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
AEOBufMgrImp::
dequeueHwBuf()
{
    MUINT32  timeout  = 200; /*ms*/
    MUINTPTR rBufAddr = (MUINTPTR)NULL;

    if(m_bAbort || m_bPreStop)
        return -1;

    if(m_pSttPipe != NULL)
    {
        CAM_LOGD_IF(m_bDebugEnable,"[%s] wait TG done, m_i4SensorDev(%d) m_i4Tglineno(%d)\n", __FUNCTION__, m_i4SensorDev, m_i4Tglineno);
        DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "dequeueHwBuf", 0);
        // deque HW buffer from driver.
        CAM_TRACE_BEGIN("AEO STT deque");
        m_pSttPipe->sendCommand(ESPipeCmd_SET_TG_INT_LINE, (MINTPTR)m_i4Tglineno, 0, 0);
        MUINT32 nRet = m_pSttPipe->wait(EPipeSignal_TG_INT, EPipeSignal_ClearWait, m_i4UserKey, timeout);
        if(nRet == MFALSE) {
            CAM_LOGD("[%s] wait TG int fail", __FUNCTION__);
            return -1;
        }
        CAM_LOGD("[%s] TG done\n", __FUNCTION__);

        m_pSttPipe->sendCommand(ESPipeCmd_GET_STT_CUR_BUF, m_rPort, (MINTPTR)&rBufAddr,0);
        CAM_LOGD_IF(m_bDebugEnable,"[%s] S_%d: dma(x%d)cur_addr(0x%d)\n", __FUNCTION__, m_i4SensorDev, (MINT32)m_rPort.index, (MINT32)rBufAddr);
        CAM_TRACE_END();
        DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "dequeueHwBuf", 1);
        if(m_bAbort || m_bPreStop){
            DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "dequeueHwBuf", -1);
            return -1;
        }

        if(rBufAddr != (MUINTPTR)NULL)
            IAeMgr::getInstance().doPvAE_TG_INT(m_i4SensorDev, reinterpret_cast<MVOID *>(rBufAddr));
        CAM_LOGD("[%s] doPvAE_TG_INT done\n", __FUNCTION__);
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
AEOBufMgrImp::
enqueueHwBuf()
{
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
StatisticBufInfo*
AEOBufMgrImp::
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
    CAM_LOGD_IF(m_bDebugEnable,"[%s] m_i4SensorDev(%d), port(%d), r_index(%d), w_index(%d)\n", __FUNCTION__, m_i4SensorDev, (MINT32)m_rPort, r_index, m_rBufIndex);
    DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "dequeueSwBuf", -1);
    return pBuf;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
AEOBufMgrImp::
abortDequeue()
{
    CAM_LOGD("[%s] m_rBufIndex(%d) +\n", __FUNCTION__, m_rBufIndex);
    m_bAbort = MTRUE;
    if(m_pSttPipe != NULL)
        m_pSttPipe->signal(EPipeSignal_TG_INT,m_i4UserKey);
    CAM_LOGD("[%s] abortDma done\n", __FUNCTION__);
    for(int i = 0; i < MAX_STATISTIC_BUFFER_CNT; i++)
        m_rBufInfo[i].flush();
    CAM_LOGD("[%s] m_rBufIndex(%d) -\n", __FUNCTION__, m_rBufIndex);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
AEOBufMgrImp::
notifyPreStop()
{
    CAM_LOGD("[%s] m_rBufIndex(%d) +\n", __FUNCTION__, m_rBufIndex);
    m_bPreStop = MTRUE;
    CAM_LOGD("[%s] m_rBufIndex(%d) -\n", __FUNCTION__, m_rBufIndex);
}

#endif

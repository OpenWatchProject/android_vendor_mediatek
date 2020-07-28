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

#include <dbg_aaa_param.h>
#include <dbg_af_param.h>
#include <camera_custom_nvram.h>
#include <aaa/af_param.h>
#include <af_feature.h>
#include <af_algo_if.h>
#include <af_mgr_if.h>
#include <isp_mgr_af_stat.h>
#include <utils/List.h>

#define MTKCAM_CCU_AF_SUPPORT
#if defined(MTKCAM_CCU_AF_SUPPORT)
#include <drv/isp_reg.h>
#include "ccu_ext_interface/ccu_af_reg.h"
#include "iccu_ctrl_af.h"
#include "iccu_mgr.h"
using namespace NSCcuIf;
#endif
using namespace NS3Av3;
using namespace NSCam::NSIoPipe;
using namespace NSCam::NSIoPipe::NSCamIOPipe;
using namespace android;

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
    virtual MVOID reset() {};
    virtual MVOID notifyPreStop();

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
    Mutex                  m_DequeueMtx;
    Condition              m_DequeueCond;
    #if defined(MTKCAM_CCU_AF_SUPPORT)
    NSCcuIf::ICcuCtrlAf*           m_pICcuCtrlAf;
    AFAcquireBuf_Reg_INFO_OUT_AP_T m_pAfoBufferPtr;
    #endif
    QBufInfo               m_HW_QBuf[P1_HW_BUF_MAX_NUM];
    MUINT32                m_HW_DeQBufIdx;
    MUINT32                m_HW_EnQBufIdx;
    MBOOL                  m_bPreStop;
    MUINT32                m_bIsSupportCCU_AF;
    MBOOL                  m_bIsStartDequeueSW;
    Mutex                  m_QBufMtx;
};

/*-------------------------------------------------------------------------------------------------------
 *
 *                             P1 - ISP Tuning Mgr
 *
 *  AFO Thread:  dequeueHwBuf(P1 deque) ->
 *  AF  Thread:                            waitDequeue -> dequeueSwBuf -> doAF -> enqueueHwBuf(P1 enque)
 *-------------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------------
 *
 *                             CCU for AF control
 *
 *  AF  Thread:  waitDequeue(CCU deque) -> dequeueSwBuf -> doAF -> enqueueHwBuf(CCU enque)
 *-------------------------------------------------------------------------------------------------------*/


#if (CAM3_3ATESTLVL > CAM3_3AUT)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#if (CAM3_3A_ISP_50_EN)
AFOBufMgr*
AFOBufMgr::
createInstance(MINT32 const i4SensorDev, MINT32 const i4SensorIdx, STT_CFG_INFO_T const sttInfo)
{
    AFOBufMgrImp* pObj = new AFOBufMgrImp(i4SensorDev,i4SensorIdx);
    return pObj;
}
#else
AFOBufMgr*
AFOBufMgr::
createInstance(MINT32 const i4SensorDev, MINT32 const i4SensorIdx)
{
    AFOBufMgrImp* pObj = new AFOBufMgrImp(i4SensorDev,i4SensorIdx);
    return pObj;
}
#endif
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
    #if defined(MTKCAM_CCU_AF_SUPPORT)
    , m_pICcuCtrlAf(NULL)
    #endif
    , m_bIsStartDequeueSW(MFALSE)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.afo_mgr.enable", value, "0");
    m_bDebugEnable = atoi(value);

    NormalPipe_QueryInfo qry;
    MBOOL bIsTwinON = MFALSE;
    INormalPipeModule::get()->query(0, ENPipeQueryCmd_D_Twin,0,0, qry);
    INormalPipe* pPipe = (INormalPipe*)INormalPipeUtils::get()->createDefaultNormalPipe(i4SensorIdx, "CCU_AF");
    pPipe->sendCommand( NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_GET_DTwin_INFO, (MINTPTR)(&bIsTwinON), 0, 0);
    pPipe->destroyInstance("CCU_AF");

    m_bIsSupportCCU_AF = 0;

    CAM_LOGD("[%s] IsSupportCCU_AF(%d), D_TWIN(%d), bIsTwinON(%d) +", __FUNCTION__, m_bIsSupportCCU_AF, qry.D_TWIN, bIsTwinON);

    if ((property_get_int32("vendor.debug.af_ccu.enable", 0) > 0) ||
        ((qry.D_TWIN > 0) && (bIsTwinON > 0)))
    {
        m_bIsSupportCCU_AF = 1;
        CAM_LOGD("[%s] Change CCU_AF(%d)", __FUNCTION__, m_bIsSupportCCU_AF);

        if (property_get_int32("vendor.debug.af_ccu.disable", 0) > 0)
        {
            m_bIsSupportCCU_AF = 0;
            CAM_LOGD("[%s] [ADB] Change CCU_AF(%d)", __FUNCTION__, m_bIsSupportCCU_AF);
        }
    }

    if (m_bIsSupportCCU_AF != 0)
    {
        #if defined(MTKCAM_CCU_AF_SUPPORT)
        memset(&m_pAfoBufferPtr, 0, sizeof(AFAcquireBuf_Reg_INFO_OUT_AP_T));
        if (m_pICcuCtrlAf == NULL)
        {
            m_pICcuCtrlAf = NSCcuIf::ICcuCtrlAf::createInstance((const uint8_t *)"AFOBufMgr",(ESensorDev_T)m_i4SensorDev);
            m_pICcuCtrlAf->init((MUINT32)m_i4SensorIdx, (ESensorDev_T)m_i4SensorDev);
        }
        #endif
    }
    else
    {
        m_pSttPipe = IStatisticPipe::createInstance(m_i4SensorIdx, LOG_TAG);
        m_HW_DeQBufIdx = 0;
        m_HW_EnQBufIdx = 0;
    }

    CAM_LOGD("[%s] SensorDev(%d) -", __FUNCTION__, m_i4SensorDev);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AFOBufMgrImp::
~AFOBufMgrImp()
{
    m_DequeueCond.broadcast();
    #if defined(MTKCAM_CCU_AF_SUPPORT)
    if (m_pICcuCtrlAf != NULL)
    {
        m_pICcuCtrlAf->uninit();
        m_pICcuCtrlAf->destroyInstance();
    }
    #endif

    if(m_pSttPipe != NULL)
    {
        m_pSttPipe->destroyInstance(LOG_TAG);
        m_pSttPipe = NULL;
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
MINT32
AFOBufMgrImp::
dequeueHwBuf()
{
    CAM_LOGD_IF(m_bDebugEnable,"[%s] m_i4SensorDev(%d) + \n", __FUNCTION__, m_i4SensorDev);

    if(m_bAbort)
        return -1;

    /*-----------------------------------------------------------------------------------------------------
     *
     *                             Control Flow 2 : P1 - ISP Tuning Mgr
     *
     *-----------------------------------------------------------------------------------------------------*/
    if(m_pSttPipe != NULL)
    {
        DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "dequeueHwBuf", 0);
        // deque HW buffer from driver.
        MUINT32 QBufIdx = m_HW_DeQBufIdx & P1_HW_BUF_MASK;

        CAM_TRACE_BEGIN("AFO STT deque");
        MBOOL ret = m_pSttPipe->deque(m_rPort, m_HW_QBuf[QBufIdx]);
        CAM_TRACE_END();

        // Apaptive Compensation
        struct timespec t;
        t.tv_sec = t.tv_nsec = 0;
        clock_gettime(CLOCK_MONOTONIC, &t);
        MUINT64 timestamp =(MUINT64)(((t.tv_sec) * 1000000000LL + t.tv_nsec)/1000);
        DebugUtil::getInstance(m_i4SensorDev)->TS_AFDone = timestamp;   // (unit:us)

        int size = m_HW_QBuf[QBufIdx].mvOut.size();
        DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "dequeueHwBuf", 1);
        if(m_bAbort || (size == 0 && ret == MTRUE)){
            DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "dequeueHwBuf", -1);
            m_DequeueCond.broadcast();
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
            m_DequeueCond.broadcast();
            return MFALSE;
        }

        {
            Mutex::Autolock autoLock(m_QBufMtx);

            int size = m_HW_QBuf[QBufIdx].mvOut.size();
            BufInfo rLastBuf = m_HW_QBuf[QBufIdx].mvOut.at( size-1 );

            CAM_LOGD_IF(m_bDebugEnable, "Deque HW Buffer Idx : %d (%d) - pa[0x%p]\n", QBufIdx, m_HW_DeQBufIdx, (void*)(rLastBuf.mPa));

            m_HW_DeQBufIdx++;
            m_u4FrameCount++;
            DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "dequeueHwBuf", -1);

            // Performance : keep dequeue and enqueue until AF thread start dequeueSwBuf
            MBOOL ChkDeque = (m_HW_DeQBufIdx - m_HW_EnQBufIdx >= P1_HW_BUF_MASK) ? MTRUE : MFALSE;
            CAM_LOGD_IF(ChkDeque, " To execute AF is too long\n");
            if ((m_bIsStartDequeueSW == MFALSE) || (ChkDeque == MTRUE))
            {
                if(m_HW_EnQBufIdx < m_HW_DeQBufIdx)
                {
                    CAM_LOGD_IF(m_bDebugEnable, "EnQBufIdx(%d) -> DeQBufIdx(%d)\n", m_HW_EnQBufIdx, m_HW_DeQBufIdx);
                    for (MUINT32 i = 0; i < P1_HW_BUF_MAX_NUM; i++)
                    {
                        MUINT32 QBufIdx = m_HW_EnQBufIdx & P1_HW_BUF_MASK;
                        int size = m_HW_QBuf[QBufIdx].mvOut.size();
                        BufInfo rLastBuf = m_HW_QBuf[QBufIdx].mvOut.at( size-1 );

                        CAM_LOGD_IF(m_bDebugEnable, "Enque HW Buffer Idx : %d (%d) - pa[0x%p]\n", QBufIdx, m_HW_EnQBufIdx, (void*)(rLastBuf.mPa));
                        MINT32 i4Ret = m_pSttPipe->enque(m_HW_QBuf[QBufIdx]);
                        if(!i4Ret)
                        {
                            CAM_LOGE("[%s] m_pSttPipe enque fail", __FUNCTION__);
                            break;
                        }

                        m_HW_EnQBufIdx++;

                        if(m_HW_EnQBufIdx >= m_HW_DeQBufIdx)
                            break;
                    }
                }
            }
        }

        m_DequeueCond.broadcast();
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
    Mutex::Autolock autoLock(m_QBufMtx);

    #if defined(MTKCAM_CCU_AF_SUPPORT)
    /*-----------------------------------------------------------------------------------------------------
     *
     *                             Control Flow 1 : CCU for AF control
     *
     *-----------------------------------------------------------------------------------------------------*/
    if (m_pICcuCtrlAf != NULL)
    {
        DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "enqueueHwBuf", 0);
        // enque HW buffer back driver
        CAM_LOGD_IF(m_bDebugEnable, "enque Hw buffer back CCU.\n");
        CAM_TRACE_BEGIN("AFO CCU enque");
        m_pICcuCtrlAf->ccu_af_enque_afo_buffer(m_pAfoBufferPtr.AFOBufAddr);
        CAM_TRACE_END();
        DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "enqueueHwBuf", -1);
    }
    #endif

    /*-----------------------------------------------------------------------------------------------------
     *
     *                             Control Flow 2 : P1 - ISP Tuning Mgr
     *
     *-----------------------------------------------------------------------------------------------------*/
    if(m_pSttPipe != NULL)
    {
        if(m_HW_EnQBufIdx >= m_HW_DeQBufIdx)
        {
            CAM_LOGD("[%s] AFO deque fail - EnQBufIdx(%d) -> DeQBufIdx(%d)\n", __FUNCTION__, m_HW_EnQBufIdx, m_HW_DeQBufIdx);
            return -1;
        }

        // enque HW buffer back driver
        CAM_LOGD_IF(m_bDebugEnable, "enque Hw buffer back driver.\n");
        CAM_TRACE_BEGIN("AFO STT enque");
        CAM_LOGD_IF(m_bDebugEnable, "EnQBufIdx(%d) -> DeQBufIdx(%d)\n", m_HW_EnQBufIdx, m_HW_DeQBufIdx);
        DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "enqueueHwBuf", 0);
        for (MUINT32 i = 0; i < P1_HW_BUF_MAX_NUM; i++)
        {
            MUINT32 QBufIdx = m_HW_EnQBufIdx & P1_HW_BUF_MASK;

            int size = m_HW_QBuf[QBufIdx].mvOut.size();
            BufInfo rLastBuf = m_HW_QBuf[QBufIdx].mvOut.at( size-1 );

            CAM_LOGD_IF(m_bDebugEnable, "Enque HW Buffer Idx : %d (%d) - pa[0x%p]\n", QBufIdx, m_HW_EnQBufIdx, (void*)(rLastBuf.mPa));
            MINT32 i4Ret = m_pSttPipe->enque(m_HW_QBuf[QBufIdx]);
            if(!i4Ret)
            {
                CAM_LOGE("[%s] m_pSttPipe enque fail", __FUNCTION__);
                return -1;
            }

            m_HW_EnQBufIdx++;

            if(m_HW_EnQBufIdx >= m_HW_DeQBufIdx)
                break;
        }
        CAM_TRACE_END();
        DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "dequeueHwBuf", -1);
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
    Mutex::Autolock autoLock(m_QBufMtx);

    MBOOL bEnable = property_get_int32("vendor.afo.dump.enable", 0);
    MBOOL bEnableHW = property_get_int32("vendor.afohw.dump.enable", 0);

    #if defined(MTKCAM_CCU_AF_SUPPORT)
    /*-----------------------------------------------------------------------------------------------------
     *
     *                             Control Flow 1 : CCU for AF control
     *
     *-----------------------------------------------------------------------------------------------------*/
    if (m_pICcuCtrlAf != NULL)
    {
       // get the last HW buffer to SW Buffer.
        /*********** TODO ***************/
        BufInfo rLastBuf;// = ???;

        // copy the last HW buffer to SW Buffer.
        m_rHwBuf.mMagicNumber = m_pAfoBufferPtr.AFOBufMagic_num;//rLastBuf.mMetaData.mMagicNum_hal;
        m_rHwBuf.mSize        = 0x81000;//rLastBuf.mSize;//0x81000;//0x186A0
        m_rHwBuf.mVa          = (MUINTPTR)m_pAfoBufferPtr.AFOBufAddr;//rLastBuf.mVa;
        m_rHwBuf.mStride      = m_pAfoBufferPtr.AFOBufStride;//rLastBuf.mStride;//0xF20;//0x7A0
        m_rHwBuf.mFrameCount  = m_u4FrameCount;
        m_rHwBuf.mTimeStamp   = 0;//rLastBuf.mMetaData.mTimeStamp;
        m_rHwBuf.mConfigNum   = m_pAfoBufferPtr.AFOBufConfigNum;
        /*********** TODO ***************/

        DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "dequeueHwBuf", 2);

        CAM_TRACE_BEGIN("AFO CCU parse");
        // Execute AF function
        IAfMgr::getInstance().passAFBuffer(m_i4SensorDev, reinterpret_cast<MVOID *>(&m_rHwBuf));
        CAM_TRACE_END();

        DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "dequeueHwBuf", 3);

        CAM_LOGD_IF(m_bDebugEnable, "port(CCU), va[0x%p]/#(%d), Size(%d), Stride(%d), timeStamp(%" PRId64 ")",
                  (void*)(m_rHwBuf.mVa), m_rHwBuf.mMagicNumber, m_rHwBuf.mSize, m_rHwBuf.mStride, m_rHwBuf.mTimeStamp);

        // dump afo data for debug
        if (bEnable) {
            static MUINT32 count = 0;
            char fileName[64];
            char folderName[64];
            MINT32 slen;
            #define AFO_DIR "/sdcard/afo"
            slen = strlen(AFO_DIR) + 1;
            strncpy(folderName, AFO_DIR, slen);
            sprintf(fileName, "/sdcard/afo/afo_%d_%d.raw", rLastBuf.mMetaData.mMagicNum_hal, count++);
            if(dumpHwBuf(fileName, folderName, rLastBuf))
            {
                CAM_LOGD_IF(m_bDebugEnable,"%s\n", fileName);
            }
        }
        // dump afo hw data for ut
        if(bEnableHW){
            char fileName[64];
            char folderName[64];
            MINT32 slen;
            #define AFOHW_DIR "/sdcard/afoHwBuf"
            slen = strlen(AFOHW_DIR) + 1;
            strncpy(folderName, AFOHW_DIR, slen);
            sprintf(fileName, "/sdcard/afoHwBuf/afo_%d.hwbuf", rLastBuf.mMetaData.mMagicNum_hal);
            if(dumpHwBuf(fileName, folderName, rLastBuf, bEnableHW))
            {
                CAM_LOGD_IF(m_bDebugEnable,"%s\n", fileName);
            }
        }

        DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "dequeueHwBuf", -1);

        if(m_bAbort)
            return NULL;
    }
    #endif

    /*-----------------------------------------------------------------------------------------------------
     *
     *                             Control Flow 2 : P1 - ISP Tuning Mgr
     *
     *-----------------------------------------------------------------------------------------------------*/
    if(m_pSttPipe != NULL)
    {
        m_bIsStartDequeueSW = MTRUE;

        if(m_HW_EnQBufIdx >= m_HW_DeQBufIdx)
        {
            CAM_LOGD("[%s] AFO deque fail - EnQBufIdx(%d) -> DeQBufIdx(%d)\n", __FUNCTION__, m_HW_EnQBufIdx, m_HW_DeQBufIdx);
            return NULL;
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
        m_rHwBuf.mConfigNum = NSIspTuningv3::ISP_MGR_AF_STAT_CONFIG_T::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev)).getConfigNum( m_rHwBuf.mMagicNumber);

        DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "dequeueSwBuf", 1);

        CAM_TRACE_BEGIN("AFO STT parse");
        // Execute AF function
        IAfMgr::getInstance().passAFBuffer(m_i4SensorDev, reinterpret_cast<MVOID *>(&m_rHwBuf));
        CAM_TRACE_END();

        DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "dequeueSwBuf", 2);

        CAM_LOGD_IF(m_bDebugEnable, "port(%d), va[0x%p]/pa[0x%p]/#(%d), Size(%d), timeStamp(%" PRId64 ")",
                  m_rPort.index, (void*)(m_rHwBuf.mVa), (void*)(rLastBuf.mPa), m_rHwBuf.mMagicNumber, m_rHwBuf.mSize, m_rHwBuf.mTimeStamp);

        // dump afo data for debug
        if (bEnable) {
            static MUINT32 count = 0;
            char fileName[64];
            char folderName[64];
            MINT32 slen;
            #define AFO_DIR "/sdcard/afo"
            slen = strlen(AFO_DIR) + 1;
            strncpy(folderName, AFO_DIR, slen);
            sprintf(fileName, "/sdcard/afo/afo_%d_%d.raw", rLastBuf.mMetaData.mMagicNum_hal, count++);
            if(dumpHwBuf(fileName, folderName, rLastBuf))
            {
                CAM_LOGD_IF(m_bDebugEnable,"%s\n", fileName);
            }
        }
        // dump afo hw data for ut
        if(bEnableHW){
            char fileName[64];
            char folderName[64];
            MINT32 slen;
            #define AFOHW_DIR "/sdcard/afoHwBuf"
            slen = strlen(AFOHW_DIR) + 1;
            strncpy(folderName, AFOHW_DIR, slen);
            sprintf(fileName, "/sdcard/afoHwBuf/afo_%d.hwbuf", rLastBuf.mMetaData.mMagicNum_hal);
            if(dumpHwBuf(fileName, folderName, rLastBuf, bEnableHW))
            {
                CAM_LOGD_IF(m_bDebugEnable,"%s\n", fileName);
            }
        }

        DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "dequeueSwBuf", -1);

        if(m_bAbort)
            return NULL;
    }

    return &m_rHwBuf;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
AFOBufMgrImp::
abortDequeue()
{
    if(m_pSttPipe != NULL)
        m_pSttPipe->abortDma(m_rPort,LOG_TAG);
    CAM_LOGD("[%s] abortDma done\n", __FUNCTION__);

    Mutex::Autolock autoLock(m_DequeueMtx);
    CAM_LOGD("[%s] +\n", __FUNCTION__);
    m_bAbort = MTRUE;
    m_u4FrameCount = 0;
    m_DequeueCond.broadcast();
    CAM_LOGD("[%s] -\n", __FUNCTION__);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
AFOBufMgrImp::
notifyPreStop()
{
    CAM_LOGI("[%s] \n", __FUNCTION__);
    m_bPreStop = MTRUE;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
AFOBufMgrImp::
waitDequeue()
{
    Mutex::Autolock autoLock(m_DequeueMtx);
    if(m_bAbort)
        return 0;

    #if defined(MTKCAM_CCU_AF_SUPPORT)
    /*-----------------------------------------------------------------------------------------------------
     *
     *                             Control Flow 1 : CCU for AF control
     *
     *-----------------------------------------------------------------------------------------------------*/
    if (m_pICcuCtrlAf != NULL)
    {
        DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "dequeueHwBuf", 0);

        CAM_TRACE_BEGIN("AFO CCU deque");
        m_pICcuCtrlAf->ccu_af_deque_afo_buffer(&m_pAfoBufferPtr);
        CAM_TRACE_END();
        CAM_LOGD_IF(m_bDebugEnable, "Dev(%d) deque Hw buffer from CCU. #(%d) - #(%d)\n",
                    m_i4SensorDev, m_pAfoBufferPtr.AFOBufMagic_num, m_pAfoBufferPtr.AFOBufConfigNum);

        // Apaptive Compensation
        struct timespec t;
        t.tv_sec = t.tv_nsec = 0;
        clock_gettime(CLOCK_MONOTONIC, &t);
        MUINT64 timestamp =(MUINT64)(((t.tv_sec) * 1000000000LL + t.tv_nsec)/1000);
        DebugUtil::getInstance(m_i4SensorDev)->TS_AFDone = timestamp;   // (unit:us)

        //int size = rDQBuf.mvOut.size();
        DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "dequeueHwBuf", 1);

        m_u4FrameCount++;
        DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "dequeueHwBuf", -1);
        return 0;
    }
    #endif

    m_DequeueCond.wait(m_DequeueMtx);
    CAM_LOGD_IF(m_bDebugEnable, "[%s] done\n", __FUNCTION__);
    return 0;
}
#endif

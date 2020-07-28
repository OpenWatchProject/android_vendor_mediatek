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
#define LOG_TAG "pdo_buf_mgr"

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
#include <mtkcam/drv/IHalSensor.h>

#if (CAM3_3A_ISP_40_EN)
#include "../common/hal3a/v1.0/buf_mgr/IBufMgr.h"
#else
#include "IBufMgr.h"
#endif

#include <mtkcam/drv/iopipe/CamIO/IStatisticPipe.h>
#include <mtkcam/drv/iopipe/CamIO/ICamsvStatisticPipe.h>
#include <mtkcam/drv/iopipe/CamIO/IHalCamIO.h>
#include <private/IopipeUtils.h>

#include <dbg_aaa_param.h>
#include <dbg_af_param.h>
#include <camera_custom_nvram.h>
#include <af_param.h>
#include <af_feature.h>
#include <af_algo_if.h>
#include <af_mgr_if.h>
#include <aaa_hal_sttCtrl.h>

using namespace NS3Av3;
using namespace NSCam::NSIoPipe;
using namespace NSCam::NSIoPipe::NSCamIOPipe;

#define PDO_BUF_SIZE (65535)

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class PDOBufMgrImp : public PDOBufMgr
{
public:
    virtual MBOOL destroyInstance();
                  PDOBufMgrImp(MINT32 i4SensorDev, MINT32 i4SensorIdx);
    virtual      ~PDOBufMgrImp();

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
    IStatisticPipe*        m_pSttPipe;
    ICamsvStatisticPipe*   m_pCamsvSttPipe;
    MUINT32                m_rBufIndex;  // the index of write buffers.
    StatisticBuf           m_rBufInfo[MAX_STATISTIC_BUFFER_CNT];
    PortID                 m_rPort;
    MBOOL                  m_bIsPDO;
    MBOOL                  m_bAbort;
    Hal3ASttCtrl*          m_p3ASttCtrl;
    MBOOL                  m_bPreStop;
};

#if (CAM3_3ATESTLVL > CAM3_3AUT)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#if (CAM3_3A_ISP_50_EN)
PDOBufMgr*
PDOBufMgr::
createInstance(MINT32 const i4SensorDev, MINT32 const i4SensorIdx, STT_CFG_INFO_T const sttInfo)
{
    PDOBufMgrImp* pObj = new PDOBufMgrImp(i4SensorDev,i4SensorIdx);
    return pObj;
}
#else
PDOBufMgr*
PDOBufMgr::
createInstance(MINT32 const i4SensorDev, MINT32 const i4SensorIdx)
{
    PDOBufMgrImp* pObj = new PDOBufMgrImp(i4SensorDev,i4SensorIdx);
    return pObj;
}
#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
PDOBufMgrImp::
destroyInstance()
{
    delete this;
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
PDOBufMgrImp::
PDOBufMgrImp(MINT32 i4SensorDev, MINT32 i4SensorIdx)
    : m_i4SensorDev(i4SensorDev)
    , m_i4SensorIdx(i4SensorIdx)
    , m_rPort(PORT_PDO)
    , m_rBufIndex(0)
    , m_bDebugEnable(MFALSE)
    , m_pSttPipe(NULL)
    , m_pCamsvSttPipe(NULL)
    , m_bIsPDO(MTRUE)
    , m_bAbort(MFALSE)
{
    m_bDebugEnable = property_get_int32("vendor.debug.pdo_mgr.enable", 0);

    m_p3ASttCtrl = Hal3ASttCtrl::getInstance(m_i4SensorDev);

    if( (m_p3ASttCtrl->quertPDAFStatus() == 1/*Hal3ASttCtrl::FEATURE_PDAF_SUPPORT_BNR_PDO*/)||
        (m_p3ASttCtrl->quertPDAFStatus() == 4/*Hal3ASttCtrl::FEATURE_PDAF_SUPPORT_PBN_PDO*/))
    {
        m_bIsPDO = MTRUE;
    }
    else
    {
        m_bIsPDO = MFALSE;
    }

    if(m_bIsPDO)
    {
        for(int i = 0; i < MAX_STATISTIC_BUFFER_CNT; i++)
        {
            CAM_LOGD("[%s]  i = %d \n", __FUNCTION__, i);
            m_rBufInfo[i].allocateBuf(PDO_BUF_SIZE);
        }
        m_pSttPipe = IStatisticPipe::createInstance(m_i4SensorIdx, LOG_TAG);
    }
    else
        m_pCamsvSttPipe = ICamsvStatisticPipe::createInstance(m_i4SensorIdx, LOG_TAG, EPIPE_CAMSV_FEATURE_PDAF);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
PDOBufMgrImp::
~PDOBufMgrImp()
{
    if(m_bIsPDO)
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
    else
    {
        if(m_pCamsvSttPipe != NULL)
    	  {
            m_pCamsvSttPipe->destroyInstance(LOG_TAG);
            m_pCamsvSttPipe = NULL;
        }
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
PDOBufMgrImp::
dequeueHwBuf()
{
    CAM_LOGD_IF(m_bDebugEnable,"[%s] m_i4SensorDev(%d) + \n", __FUNCTION__, m_i4SensorDev);

    MBOOL bEnable = property_get_int32("vendor.pdo.dump.enable", 0);

    if(m_bAbort)
        return -1;

    // deque HW buffer from driver.
    QBufInfo    rDQBuf;
    MBOOL ret = MTRUE;

    CAM_TRACE_BEGIN("PDO STT deque");
    if(m_bIsPDO && m_pSttPipe != NULL)
        //ret = m_pSttPipe->deque(m_rPort, rDQBuf, ESTT_CacheInvalidAll);
        ret = m_pSttPipe->deque(m_rPort, rDQBuf, ESTT_CacheInvalidByRange);
    else if(m_pCamsvSttPipe != NULL)
    {
        PortID _dq_portID;
        //ret = m_pCamsvSttPipe->deque(_dq_portID, rDQBuf, ESTT_CacheInvalidAll);
        ret = m_pCamsvSttPipe->deque(_dq_portID, rDQBuf, ESTT_CacheInvalidByRange);
    }
    CAM_TRACE_END();
    int size = rDQBuf.mvOut.size();
    if(m_bAbort || (size == 0 && ret == MTRUE))
        return -1;

    if(ret == MFALSE)
    {
        CAM_LOGE("[%s] m_pSttPipe deque fail", __FUNCTION__);
        return MFALSE;
    }
    // get the last HW buffer to SW Buffer.
    int index = m_rBufIndex;
    BufInfo rLastBuf = rDQBuf.mvOut.at( size-1 );

    // copy the last HW buffer to SW Buffer.
    StatisticBufInfo rHwBuf;
    rHwBuf.mMagicNumber = rLastBuf.mMetaData.mMagicNum_hal;
    rHwBuf.mSize = rLastBuf.mSize;
    rHwBuf.mVa = rLastBuf.mVa;
    rHwBuf.mPa_offset = rLastBuf.mPa_offset;
    rHwBuf.mPrivateData = rLastBuf.mMetaData.mPrivateData;
    rHwBuf.mPrivateDataSize = rLastBuf.mMetaData.mPrivateDataSize;
    rHwBuf.mStride = rLastBuf.mStride;
    rHwBuf.mTimeStamp = rLastBuf.mMetaData.mTimeStamp;

    // Execute AF function
    IAfMgr::getInstance().passPDBuffer(m_i4SensorDev, reinterpret_cast<MVOID *>(&rHwBuf));

    CAM_LOGD_IF(m_bDebugEnable, "port(%d), index(%d), va[0x%x]/pa[0x%08x]/#(%d), Size(%d), Stride(%d), timeStamp(%" PRId64 ")",
              m_rPort.index, index, rHwBuf.mVa, rLastBuf.mPa, rHwBuf.mMagicNumber, rHwBuf.mSize, rHwBuf.mStride, rHwBuf.mTimeStamp);

    // dump pdo data for debug
    if (bEnable) {
        char fileName[64];
        static MUINT32 count;
        sprintf(fileName, "/sdcard/pdo/pdo_%d_%d.raw", rLastBuf.mMetaData.mMagicNum_hal, count++);

        FILE *fp = fopen(fileName, "w");
        if (NULL == fp)
        {
            CAM_LOGE("fail to open file to save img: %s", fileName);
            MINT32 err = mkdir("/sdcard/pdo", S_IRWXU | S_IRWXG | S_IRWXO);
            CAM_LOGD("err = %d", err);
        } else
        {
            CAM_LOGD_IF(m_bDebugEnable,"%s\n", fileName);
            fwrite(reinterpret_cast<void *>(rLastBuf.mVa), 1, rLastBuf.mSize, fp);
            fclose(fp);
        }
    }

    if(m_bAbort)
        return -1;
    // enque HW buffer back driver
    CAM_LOGD_IF(m_bDebugEnable, "enque Hw buffer back driver.\n");
    CAM_TRACE_BEGIN("PDO STT enque");
    if(m_bIsPDO && m_pSttPipe != NULL)
        m_pSttPipe->enque(rDQBuf);
    else if(m_pCamsvSttPipe != NULL)
        m_pCamsvSttPipe->enque(rDQBuf);
    CAM_TRACE_END();

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
PDOBufMgrImp::
enqueueHwBuf()
{
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
StatisticBufInfo*
PDOBufMgrImp::
dequeueSwBuf()
{
    // Unused code, beacuse PDAF function executed when deque and enque statistic data.
    // No need to dequeueSwBuf for executing PDAF function on AF thread.
    if(m_bIsPDO)
    {
        CAM_LOGD_IF(m_bDebugEnable,"[%s] index(%d) +\n", __FUNCTION__, m_rBufIndex);
        if(m_bAbort)
            return NULL;
        int r_index = m_rBufIndex;
        StatisticBufInfo* pBuf = m_rBufInfo[r_index].read();
        // switch buffer
        m_rBufIndex = ( r_index + 1 ) % MAX_STATISTIC_BUFFER_CNT;
        CAM_LOGD_IF(m_bDebugEnable,"[%s] m_i4SensorDev(%d), port(%d), r_index(%d), w_index(%d)\n", __FUNCTION__, m_i4SensorDev, m_rPort.index, r_index, m_rBufIndex);
        return pBuf;
    } else
    {
        return NULL;
    }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
PDOBufMgrImp::
abortDequeue()
{
    CAM_LOGD("[%s] m_rBufIndex(%d)\n", __FUNCTION__, m_rBufIndex);
    m_bAbort = MTRUE;
    if(m_pCamsvSttPipe != NULL && !m_bIsPDO)
    {
        PortID _dq_portID = NSImageio::NSIspio::EPortIndex_CAMSV_IMGO;
        m_pCamsvSttPipe->abortDma(_dq_portID,LOG_TAG);
    }
    if(m_pSttPipe != NULL && m_bIsPDO)
        m_pSttPipe->abortDma(m_rPort,LOG_TAG);
    for(int i = 0; i < MAX_STATISTIC_BUFFER_CNT; i++)
        m_rBufInfo[i].flush();
}


MVOID
PDOBufMgrImp::
notifyPreStop()
{
    CAM_LOGD("[%s] +", __FUNCTION__);
    m_bPreStop = MTRUE;
    CAM_LOGD("[%s] -", __FUNCTION__);
}


#endif

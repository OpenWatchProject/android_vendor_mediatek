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
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

#define LOG_TAG "MVHDR3ExpoBufMgr"

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

#include "IBufMgr.h"

#if CAM3_FAKE_SENSOR_DRV
#include "fake_sensor_drv/ICamsvStatisticPipe.h"
#else
#include <mtkcam/drv/iopipe/CamIO/ICamsvStatisticPipe.h>
#endif

#include <mtkcam/drv/iopipe/CamIO/IHalCamIO.h>
#include <private/IopipeUtils.h>
#include <mtkcam/drv/iopipe/PortMap.h>
#include <utils/Condition.h>

#include "camera_custom_mvhdr3expo.h"

using namespace android;
using namespace NS3Av3;
using namespace NSCam::NSIoPipe;
using namespace NSCam::NSIoPipe::NSCamIOPipe;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  MVHDRStatisticBuf
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class MVHDRStatisticBuf
{
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data member
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    mutable Mutex           m_Lock;
    MBOOL                   m_fgLog;
    MBOOL                   m_fgCmd;
    MBOOL                   m_fgFlush;
    MUINT32                 m_i4TotalBufSize;
    Condition               m_Cond;
    StatisticBufInfo        m_rBufInfo;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor/Dtor.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    MVHDRStatisticBuf(MVHDRStatisticBuf const&);
    MVHDRStatisticBuf& operator=(MVHDRStatisticBuf const&);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    MVHDRStatisticBuf();
    virtual ~MVHDRStatisticBuf();

    virtual MBOOL allocateBuf(MUINT32 const i4BufSize);
    virtual MBOOL freeBuf();
    virtual MBOOL write(MINT32 i4SensorDev,
                        const BufInfo& rAEYBufInfo,        // AE Y
                        const BufInfo& rAEHistBufInfo,     // AE hist
                        const BufInfo& rEmbDataBufInfo);   // embedded data
    virtual StatisticBufInfo* read();
    virtual MBOOL flush();
};


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class MVHDR3ExpoBufMgrImp : public MVHDR3ExpoBufMgr
{
public:
    virtual MBOOL destroyInstance();
                  MVHDR3ExpoBufMgrImp(MINT32 i4SensorDev, MINT32 i4SensorIdx);
    virtual      ~MVHDR3ExpoBufMgrImp();

    virtual MBOOL dequeueHwBuf();
    virtual MBOOL enqueueHwBuf();
    virtual StatisticBufInfo* dequeueSwBuf();
    virtual MVOID abortDequeue();
    virtual MINT32 waitDequeue(){return 0;};
    virtual MVOID notifyPreStop();

    MINT32 dequeueHw(const char* tag, ICamsvStatisticPipe* m_pCamsvSttPipe, QBufInfo &rDQBuf, BufInfo &rLastBuf);

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data member
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    MINT32           const m_i4SensorDev;
    MINT32           const m_i4SensorIdx;
    MBOOL                  m_bDebugEnable;
    ICamsvStatisticPipe*   m_pCamsvSttPipeAEY;
    ICamsvStatisticPipe*   m_pCamsvSttPipeAEHist;
    ICamsvStatisticPipe*   m_pCamsvSttPipeAEEmbData;
    MUINT32                m_rBufIndex;  // the index of write buffers.
    MVHDRStatisticBuf      m_rBufInfo[MAX_STATISTIC_BUFFER_CNT];
    PortID                 m_rPort;
    MBOOL                  m_bAbort;
    MBOOL                  m_bPreStop;
};

#if (CAM3_3ATESTLVL > CAM3_3AUT)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVHDR3ExpoBufMgr*
MVHDR3ExpoBufMgr::
createInstance(MINT32 const i4SensorDev, MINT32 const i4SensorIdx, STT_CFG_INFO_T const sttInfo)
{
    MVHDR3ExpoBufMgrImp* pObj = new MVHDR3ExpoBufMgrImp(i4SensorDev,i4SensorIdx);
    return pObj;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
MVHDR3ExpoBufMgrImp::
destroyInstance()
{
    delete this;
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVHDR3ExpoBufMgrImp::
MVHDR3ExpoBufMgrImp(MINT32 i4SensorDev, MINT32 i4SensorIdx)
    : m_i4SensorDev(i4SensorDev)
    , m_i4SensorIdx(i4SensorIdx)
    , m_bDebugEnable(MFALSE)
    , m_pCamsvSttPipeAEY(NULL)
    , m_pCamsvSttPipeAEHist(NULL)
    , m_pCamsvSttPipeAEEmbData(NULL)
    , m_rBufIndex(0)
    , m_bAbort(MFALSE)
    , m_bPreStop(MFALSE)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.mvhdr_3expo_mgr.enable", value, "1"); // temp for test
    m_bDebugEnable = atoi(value);

    MUINT32 u4MVHDRBufSize = getMVHDR3ExpoBufSize();
    for(int i = 0; i < MAX_STATISTIC_BUFFER_CNT; i++)
    {
        CAM_LOGD("[%s]  i = %d u4MVHDRBufSize = %d\n", __FUNCTION__, i, u4MVHDRBufSize);
        m_rBufInfo[i].allocateBuf(u4MVHDRBufSize);
    }

    m_pCamsvSttPipeAEY = ICamsvStatisticPipe::createInstance(m_i4SensorIdx, LOG_TAG, EPIPE_CAMSV_FEATURE_4CELL_MVHDR_Y);
    m_pCamsvSttPipeAEHist = ICamsvStatisticPipe::createInstance(m_i4SensorIdx, LOG_TAG, EPIPE_CAMSV_FEATURE_4CELL_MVHDR_AE);
    m_pCamsvSttPipeAEEmbData = ICamsvStatisticPipe::createInstance(m_i4SensorIdx, LOG_TAG, EPIPE_CAMSV_FEATURE_4CELL_MVHDR_EMB);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVHDR3ExpoBufMgrImp::
~MVHDR3ExpoBufMgrImp()
{
    for(int i = 0; i < MAX_STATISTIC_BUFFER_CNT; i++)
    {
        CAM_LOGD("[%s]  i = %d \n", __FUNCTION__, i);
        m_rBufInfo[i].freeBuf();
    }

    if(m_pCamsvSttPipeAEY != NULL)
    {
        m_pCamsvSttPipeAEY->destroyInstance(LOG_TAG);
        m_pCamsvSttPipeAEY = NULL;
    }

    if(m_pCamsvSttPipeAEHist != NULL)
    {
        m_pCamsvSttPipeAEHist->destroyInstance(LOG_TAG);
        m_pCamsvSttPipeAEHist = NULL;
    }

    if(m_pCamsvSttPipeAEEmbData != NULL)
    {
        m_pCamsvSttPipeAEEmbData->destroyInstance(LOG_TAG);
        m_pCamsvSttPipeAEEmbData = NULL;
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
MVHDR3ExpoBufMgrImp::
dequeueHwBuf()
{
    CAM_LOGD_IF(m_bDebugEnable,"[%s] m_i4SensorDev(%d) + \n", __FUNCTION__, m_i4SensorDev);

    if(m_bAbort || m_bPreStop)
        return -1;

    MINT32 ret;
    QBufInfo rDQBufAEY, rDQBufAEHist, rDQBufAEEmbData;
    BufInfo  rLastBufAEY, rLastBufAEHist, rLastBufAEEmbData;

    if(m_pCamsvSttPipeAEY != NULL) {
        ret = dequeueHw( "CamsvSttPipeAEY", m_pCamsvSttPipeAEY, rDQBufAEY, rLastBufAEY);
        if(ret != MTRUE) return ret;
    } else
        CAM_LOGD("m_pCamsvSttPipeAEY is NULL");
    if(m_pCamsvSttPipeAEHist != NULL) {
        ret = dequeueHw( "CamsvSttPipeAEHist", m_pCamsvSttPipeAEHist, rDQBufAEHist, rLastBufAEHist);
        if(ret != MTRUE) return ret;
    } else
        CAM_LOGD("m_pCamsvSttPipeAEHist is NULL");
    if(m_pCamsvSttPipeAEEmbData != NULL) {
        ret = dequeueHw( "CamsvSttPipeAEEmbData", m_pCamsvSttPipeAEEmbData, rDQBufAEEmbData, rLastBufAEEmbData);
        if(ret != MTRUE) return ret;
    } else
        CAM_LOGD("m_pCamsvSttPipeAEEmbData is NULL");

    if(m_pCamsvSttPipeAEY != NULL && m_pCamsvSttPipeAEHist != NULL && m_pCamsvSttPipeAEEmbData != NULL) {
        CAM_TRACE_BEGIN("MVHDR3Expo STT clone");
        int index = m_rBufIndex;
        m_rBufInfo[index].write(m_i4SensorDev, rLastBufAEY, rLastBufAEHist, rLastBufAEEmbData);
        CAM_TRACE_END();
    }

    if(m_pCamsvSttPipeAEY != NULL) {
        if(m_bAbort || m_bPreStop)
            return -1;
        // enque HW buffer back driver
        CAM_LOGD_IF(m_bDebugEnable, "enque CamsvSttPipeAEY Hw buffer back driver.\n");
        CAM_TRACE_BEGIN("CamsvSttPipeAEY STT enque");
        m_pCamsvSttPipeAEY->enque(rDQBufAEY);
        CAM_TRACE_END();
    }
    if(m_pCamsvSttPipeAEHist != NULL) {
        if(m_bAbort || m_bPreStop)
            return -1;
        // enque HW buffer back driver
        CAM_LOGD_IF(m_bDebugEnable, "enque CamsvSttPipeAEHist Hw buffer back driver.\n");
        CAM_TRACE_BEGIN("CamsvSttPipeAEHist STT enque");
        m_pCamsvSttPipeAEHist->enque(rDQBufAEHist);
        CAM_TRACE_END();
    }
    if(m_pCamsvSttPipeAEEmbData != NULL) {
        if(m_bAbort || m_bPreStop)
            return -1;
        // enque HW buffer back driver
        CAM_LOGD_IF(m_bDebugEnable, "enque CamsvSttPipeAEEmbData Hw buffer back driver.\n");
        CAM_TRACE_BEGIN("CamsvSttPipeAEEmbData STT enque");
        m_pCamsvSttPipeAEEmbData->enque(rDQBufAEEmbData);
        CAM_TRACE_END();
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
MVHDR3ExpoBufMgrImp::
dequeueHw(const char* tag, ICamsvStatisticPipe* m_pCamsvSttPipe, QBufInfo &rDQBuf, BufInfo &rLastBuf)
{
    MBOOL bEnable = property_get_int32("vendor.mvhdr3expo.dump.enable", 0);
    MBOOL bEnableHW = property_get_int32("vendor.mvhdr3expohw.dump.enable", 0);

    // deque HW buffer from driver.
    CAM_TRACE_BEGIN("MVHDR3Expo STT deque");
    PortID _dq_portID;
    MBOOL ret = m_pCamsvSttPipe->deque(_dq_portID, rDQBuf);
    CAM_TRACE_END();
    int size = rDQBuf.mvOut.size();
    if(m_bAbort || m_bPreStop || (size == 0 && ret == MTRUE) )
        return -1;

    if(ret == MFALSE)
    {
        CAM_LOGE("[%s] MVHDR3Expo %s deque fail", __FUNCTION__, tag);
        return MFALSE;
    }
    // get the last HW buffer to SW Buffer.
    int index = m_rBufIndex;
    rLastBuf = rDQBuf.mvOut.at( size-1 );

    CAM_LOGD_IF(m_bDebugEnable, "[%s()- %s] port(%d), index(%d), va[%d]/pa[0x%d]/#(%d), Size(%d), timeStamp(%" PRId64 ")", __FUNCTION__, tag,
              m_rPort.index, index, (MINT32)rLastBuf.mVa, (MINT32)rLastBuf.mPa, rLastBuf.mMetaData.mMagicNum_hal, rLastBuf.mSize, rLastBuf.mMetaData.mTimeStamp);

    // dump mvhdr3expo data for debug
    if (bEnable) {
        static MUINT32 count = 0;
        char fileName[64];
        char folderName[64];
        strncpy(folderName, "/sdcard/mvhdr3expo", sizeof(folderName));
        folderName[sizeof(folderName) - 1] = '\0';
        snprintf(fileName, sizeof(fileName), "/sdcard/mvhdr3expo/mvhdr3expo_%s_%d_%d.raw", tag, rLastBuf.mMetaData.mMagicNum_hal, count++);
        if(dumpHwBuf(fileName, folderName, rLastBuf))
        {
            CAM_LOGD_IF(m_bDebugEnable,"%s\n", fileName);
        }
    }
    // dump mvhdr3expo hw data for ut
    if(bEnableHW){
        char fileName[64];
        char folderName[64];
        strncpy(folderName, "/sdcard/mvhdr3expoHwBuf", sizeof(folderName));
        folderName[sizeof(folderName)-1] = '\0';
        snprintf(fileName, sizeof(fileName), "/sdcard/mvhdr3expoHwBuf/mvhdr3expo_%s_%d.hwbuf", tag, rLastBuf.mMetaData.mMagicNum_hal);
        if(dumpHwBuf(fileName, folderName, rLastBuf, bEnableHW))
        {
            CAM_LOGD_IF(m_bDebugEnable,"%s\n", fileName);
        }
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
MVHDR3ExpoBufMgrImp::
enqueueHwBuf()
{
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
StatisticBufInfo*
MVHDR3ExpoBufMgrImp::
dequeueSwBuf()
{
    CAM_LOGD_IF(m_bDebugEnable,"[%s] index(%d) +\n", __FUNCTION__, m_rBufIndex);
    if(m_bAbort || m_bPreStop)
        return NULL;
    int r_index = m_rBufIndex;
    StatisticBufInfo* pBuf = m_rBufInfo[r_index].read();
    // switch buffer
    m_rBufIndex = ( r_index + 1 ) % MAX_STATISTIC_BUFFER_CNT;
    CAM_LOGD_IF(m_bDebugEnable,"[%s] m_i4SensorDev(%d), port(%d), r_index(%d), w_index(%d)\n", __FUNCTION__, m_i4SensorDev, m_rPort.index, r_index, m_rBufIndex);
    return pBuf;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
MVHDR3ExpoBufMgrImp::
abortDequeue()
{
    CAM_LOGD("[%s]\n", __FUNCTION__);
    m_bAbort = MTRUE;
    if(m_pCamsvSttPipeAEY != NULL)
    {
        m_pCamsvSttPipeAEY->abortDma(PORT_CAMSV_IMGO,LOG_TAG);
    }
    if(m_pCamsvSttPipeAEHist != NULL)
    {
        m_pCamsvSttPipeAEHist->abortDma(PORT_CAMSV_IMGO,LOG_TAG);
    }
    if(m_pCamsvSttPipeAEEmbData != NULL)
    {
        m_pCamsvSttPipeAEEmbData->abortDma(PORT_CAMSV_IMGO,LOG_TAG);
    }
    for(int i = 0; i < MAX_STATISTIC_BUFFER_CNT; i++)
        m_rBufInfo[i].flush();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
MVHDR3ExpoBufMgrImp::
notifyPreStop()
{
    CAM_LOGD("[%s] m_rBufIndex(%d) +\n", __FUNCTION__, m_rBufIndex);
    m_bPreStop = MTRUE;
    CAM_LOGD("[%s] m_rBufIndex(%d) -\n", __FUNCTION__, m_rBufIndex);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  MVHDRStatisticBuf
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVHDRStatisticBuf::
MVHDRStatisticBuf()
    : m_fgLog(MFALSE)
    , m_fgCmd(MFALSE)
    , m_fgFlush(MFALSE)
    , m_i4TotalBufSize(0)

{
    m_rBufInfo.mSize = 0;
    m_rBufInfo.mVa = (MUINTPTR)NULL;
}

MVHDRStatisticBuf::
~MVHDRStatisticBuf()
{
    freeBuf();
}

MBOOL
MVHDRStatisticBuf::
allocateBuf(MUINT32 const i4BufSize)
{
    Mutex::Autolock lock(m_Lock);
    m_fgLog = property_get_int32("vendor.debug.mvhdr_stat_buf.enable", 1); // temp for test

    if (!m_rBufInfo.mVa || !m_i4TotalBufSize) {
        m_i4TotalBufSize = i4BufSize;
        m_rBufInfo.mVa = (MUINTPTR) new MUINT32[m_i4TotalBufSize + 1];
        CAM_LOGD_IF(m_fgLog, "allocateBuf(): mVa(%p).", (void *)m_rBufInfo.mVa);
    }

    return MTRUE;
}

MBOOL
MVHDRStatisticBuf::
freeBuf()
{
    Mutex::Autolock lock(m_Lock);

    if (m_rBufInfo.mVa && m_i4TotalBufSize) {
        CAM_LOGD_IF(m_fgLog, "freeBuf(): mVa(%p).", (void *)m_rBufInfo.mVa);
        delete [] (MUINT32 *)m_rBufInfo.mVa;
        m_i4TotalBufSize = 0;
    }

    return MTRUE;
}

MBOOL
MVHDRStatisticBuf::
write(MINT32 i4SensorDev, const BufInfo& rAEYBufInfo, const BufInfo& rAEHistBufInfo, const BufInfo& rEmbDataBufInfo)
{
    Mutex::Autolock lock(m_Lock);

    if (!m_rBufInfo.mVa) {
        CAM_LOGE("write(): mVa is NULL.");
        return MFALSE;
    }

    /* copy buffer info */
    m_rBufInfo.mMagicNumber = rAEYBufInfo.mMetaData.mMagicNum_hal;
    m_rBufInfo.mSize = rAEYBufInfo.mSize;
    m_rBufInfo.mStride = rAEYBufInfo.mStride;
    m_rBufInfo.mTimeStamp = rAEYBufInfo.mMetaData.mTimeStamp;
    MUINT32 u4MVHDRRatio_x100 = 0;
    decodeMVHDR3ExpoStatistic(i4SensorDev,
                              reinterpret_cast<MVOID *>(rAEYBufInfo.mVa),
                              reinterpret_cast<MVOID *>(rAEHistBufInfo.mVa),
                              reinterpret_cast<MVOID *>(rEmbDataBufInfo.mVa),
                              reinterpret_cast<MVOID *>(m_rBufInfo.mVa),
                              u4MVHDRRatio_x100);
    m_rBufInfo.mMVHDRRatio_x100 = u4MVHDRRatio_x100;

    /* update cmd and broadcast */
    m_fgCmd = MTRUE;
    m_Cond.broadcast();

    CAM_LOGD_IF(m_fgLog, "[%s] - m_fgCmd(%d) Magic(%d) size(%d) MVHDRRatio(%d)\n", __FUNCTION__, m_fgCmd, rAEYBufInfo.mMetaData.mMagicNum_hal, rAEYBufInfo.mSize, u4MVHDRRatio_x100);

    return MTRUE;
}

StatisticBufInfo*
MVHDRStatisticBuf::
read()
{
    Mutex::Autolock lock(m_Lock);

    if(m_rBufInfo.mVa == NULL){
        CAM_LOGE("m_rBufInfo.mVa == NULL");
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
MVHDRStatisticBuf::
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


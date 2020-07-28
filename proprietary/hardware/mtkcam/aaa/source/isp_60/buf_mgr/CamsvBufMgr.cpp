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

#define LOG_TAG "CamsvBufMgr"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <mtkcam/utils/std/Trace.h>
#include <sys/stat.h>
#include <cutils/properties.h>
#include <string.h>
#include <unordered_map>

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

#include "camera_custom_msdk.h"

#include <iccu_ctrl_ae.h>

//flicker
#include "flicker_hal_if.h"
#include <ae_mgr/ae_mgr_if.h>


using namespace android;
using namespace NS3Av3;
using namespace NSCam;
using namespace NSCam::NSIoPipe;
using namespace NSCam::NSIoPipe::NSCamIOPipe;
using namespace NSIspTuning;
using namespace NSCcuIf;

#define MAX_MVHDR_STAT_BUFFER_CNT (2)

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  CamsvStatisticBuf
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CamsvStatisticBuf
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
    MUINT32                 m_i4TotalFlkBufSize;
    Condition               m_Cond;
    StatisticBufInfo        m_rBufInfo;
    StatisticBufInfo        m_rBufFlkInfo;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor/Dtor.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    CamsvStatisticBuf(CamsvStatisticBuf const&);
    CamsvStatisticBuf& operator=(CamsvStatisticBuf const&);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    CamsvStatisticBuf();
    virtual ~CamsvStatisticBuf();

    virtual MBOOL allocateBuf(MUINT32 const i4BufSize);
    virtual MBOOL allocateFlkBuf(MUINT32 const i4BufSize);
    virtual MBOOL configBuf(MUINTPTR pVa);
    virtual MBOOL freeBuf();
    virtual MBOOL freeFlkBuf();
    virtual MBOOL write(MINT32 i4SensorDev, unordered_map<MUINT32, BufInfo>& mapLastBuf, MUINT32& u4MVHDRRatio_x100);   // all camsv channels
    virtual StatisticBufInfo* read();
    virtual void setAeBufInfo(CAMSV_CUSTOM_BUFFER_INFO bufInfo);
    virtual CAMSV_CUSTOM_BUFFER_INFO getAeBufInfo();
    virtual void setFlkBufInfo(CAMSV_CUSTOM_BUFFER_INFO bufInfo);
    virtual CAMSV_CUSTOM_BUFFER_INFO getFlkBufInfo() {return {0};}
    virtual MBOOL flush();
};


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CamsvBufMgrImp : public CamsvBufMgr
{
public:
    virtual MBOOL destroyInstance();
                  CamsvBufMgrImp(MINT32 i4SensorDev, MINT32 i4SensorIdx, STT_CFG_INFO_T const sttInfo);
    virtual      ~CamsvBufMgrImp();

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
    MUINT32                m_rBufIndex;  // the index of write buffers.
    CamsvStatisticBuf      m_rBufInfo[MAX_MVHDR_STAT_BUFFER_CNT];
    PortID                 m_rPort;
    MBOOL                  m_bAbort;
    MBOOL                  m_bPreStop;
    vector<MUINT32>        m_vecCamsvIndex;
    unordered_map<MUINT32, ICamsvStatisticPipe*> m_mapCamsvSttPipe;
    ICcuCtrlAe*            m_pICcuAe;
    MBOOL                  m_bCcuEnable;
    struct CcuAeHdrBufferSet m_rBufferSets[MAX_MVHDR_STAT_BUFFER_CNT];
};

MUINT32 getSensorDevID(ESensorDev_T eSensorDev);

#if (CAM3_3ATESTLVL > CAM3_3AUT)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
CamsvBufMgr*
CamsvBufMgr::
createInstance(MINT32 const i4SensorDev, MINT32 const i4SensorIdx, STT_CFG_INFO_T const sttInfo)
{
    CamsvBufMgrImp* pObj = new CamsvBufMgrImp(i4SensorDev,i4SensorIdx,sttInfo);
    return pObj;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
CamsvBufMgrImp::
destroyInstance()
{
    delete this;
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
CamsvBufMgrImp::
CamsvBufMgrImp(MINT32 i4SensorDev, MINT32 i4SensorIdx, STT_CFG_INFO_T sttInfo)
    : m_i4SensorDev(i4SensorDev)
    , m_i4SensorIdx(i4SensorIdx)
    , m_bDebugEnable(MFALSE)
    , m_vecCamsvIndex(sttInfo.vecCAMSVIndexOf3EXPO)
    , m_rBufIndex(0)
    , m_bAbort(MFALSE)
    , m_bPreStop(MFALSE)
    , m_pICcuAe(nullptr)
    , m_bCcuEnable(MTRUE)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.camsv_buf_mgr.enable", value, "1"); // temp for test
    m_bDebugEnable = atoi(value);

    MUINT32 u4SensorID = getSensorDevID((ESensorDev_T)i4SensorDev);
    CAMSV_CUSTOM_BUFFER_INFO u4MVHDRBufInfo = GetCamsvBufSize(CAMSV_CUSTOM_DECODE_AE, u4SensorID);
    CAMSV_CUSTOM_BUFFER_INFO u4MVHDRFlkBufInfo = GetCamsvBufSize(CAMSV_CUSTOM_DECODE_FLK, u4SensorID);
    m_bCcuEnable = m_pICcuAe->getHdrDataBuffer(i4SensorIdx, (NSIspTuning::ESensorDev_T)i4SensorDev, m_rBufferSets);
    if (!m_bCcuEnable)
    {

        for(int i = 0; i < MAX_MVHDR_STAT_BUFFER_CNT; i++)
        {
            CAM_LOGD("[%s]  i = %d u4MVHDRBufSize = %d u4MVHDRFlkBufSize = %d\n", __FUNCTION__, i, u4MVHDRBufInfo.size, u4MVHDRFlkBufInfo.size);

            // AE
            m_rBufInfo[i].allocateBuf(u4MVHDRBufInfo.size);
            m_rBufInfo[i].setAeBufInfo(u4MVHDRBufInfo);

            // FLK
            m_rBufInfo[i].allocateFlkBuf(u4MVHDRFlkBufInfo.size);
            m_rBufInfo[i].setFlkBufInfo(u4MVHDRFlkBufInfo);
        }
    }
    else
    {
        if (MAX_MVHDR_STAT_BUFFER_CNT != CCU_HDR_DATA_BUF_CNT)
        {
            CAM_LOGE("[%s] MAX_MVHDR_STAT_BUFFER_CNT(%d), CCU_HDR_DATA_BUF_CNT(%d) \n", __FUNCTION__, MAX_MVHDR_STAT_BUFFER_CNT, CCU_HDR_DATA_BUF_CNT);
        }

        m_pICcuAe = ICcuCtrlAe::getInstance(i4SensorIdx, (NSIspTuning::ESensorDev_T)i4SensorDev);
        for(int i = 0; i < MAX_MVHDR_STAT_BUFFER_CNT; i++)
        {
            // AE
            m_rBufInfo[i].configBuf((MUINTPTR) m_rBufferSets[i].va);
            m_rBufInfo[i].setAeBufInfo(u4MVHDRBufInfo);

            // FLK
            m_rBufInfo[i].allocateFlkBuf(u4MVHDRFlkBufInfo.size);
            m_rBufInfo[i].setFlkBufInfo(u4MVHDRFlkBufInfo);
        }
    }

    for (MUINT32 u4CamsvIndex: m_vecCamsvIndex)
    {
        m_mapCamsvSttPipe[u4CamsvIndex] = ICamsvStatisticPipe::createInstance(m_i4SensorIdx, LOG_TAG, u4CamsvIndex);
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
CamsvBufMgrImp::
~CamsvBufMgrImp()
{
    if (!m_bCcuEnable)
    {
        for(int i = 0; i < MAX_MVHDR_STAT_BUFFER_CNT; i++)
        {
            CAM_LOGD("[%s]  i = %d \n", __FUNCTION__, i);
            m_rBufInfo[i].freeBuf();
            m_rBufInfo[i].freeFlkBuf();
        }
    }

    for (MUINT32 u4CamsvIndex: m_vecCamsvIndex)
    {
        if(m_mapCamsvSttPipe[u4CamsvIndex] != nullptr)
        {
            m_mapCamsvSttPipe[u4CamsvIndex]->destroyInstance(LOG_TAG);
            m_mapCamsvSttPipe[u4CamsvIndex] = nullptr;
        }
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
CamsvBufMgrImp::
dequeueHwBuf()
{
    CAM_LOGD_IF(m_bDebugEnable,"[%s] m_i4SensorDev(%d) + \n", __FUNCTION__, m_i4SensorDev);

    if(m_bAbort || m_bPreStop)
        return -1;

    MINT32 ret;
    unordered_map<MUINT32, QBufInfo> mapDQBuf;
    unordered_map<MUINT32, BufInfo> mapLastBuf;

    MBOOL bCamsvEnable = MTRUE;
    for (MUINT32 u4CamsvIndex: m_vecCamsvIndex)
    {
        string strCamsvSttPipe = "CamsvSttPipe(" + to_string(u4CamsvIndex) + ")";
        if(m_mapCamsvSttPipe[u4CamsvIndex] != nullptr)
        {
            ret = dequeueHw( strCamsvSttPipe.c_str(), m_mapCamsvSttPipe[u4CamsvIndex], mapDQBuf[u4CamsvIndex], mapLastBuf[u4CamsvIndex]);
            if(ret != MTRUE) return ret;
        } else
            CAM_LOGD("%s is NULL", strCamsvSttPipe.c_str());

        bCamsvEnable = bCamsvEnable && (m_mapCamsvSttPipe[u4CamsvIndex] != nullptr);
    }

    if(bCamsvEnable)
    {
         if (m_bCcuEnable) { // switch buffer for CCU ON
            m_rBufIndex = ( m_rBufIndex + 1 ) % MAX_MVHDR_STAT_BUFFER_CNT;
            CAM_LOGD_IF(m_bDebugEnable,"[%s] switch buffer for CCU ON, m_i4SensorDev(%d), port(%d), m_rBufIndex(%d), w_index(%d)\n", __FUNCTION__, m_i4SensorDev, m_rPort.index, m_rBufIndex, m_rBufIndex);
        }

        MUINT32 u4MVHDRRatio_x100 = 0;
        CAM_TRACE_BEGIN("MVHDR3Expo STT clone");
        m_rBufInfo[m_rBufIndex].write(m_i4SensorDev, mapLastBuf, u4MVHDRRatio_x100);
        CAM_TRACE_END();

        if (m_bCcuEnable) // send vhdr buffer to CCU
        {
            struct ccu_hdr_ae_input_data rInputData;
            CAMSV_CUSTOM_BUFFER_INFO aeBufInfo = m_rBufInfo[m_rBufIndex].getAeBufInfo();
            rInputData.hdr_data_buffer_mva = m_rBufferSets[m_rBufIndex].mva;
            rInputData.u4BufSizeX = aeBufInfo.width;
            rInputData.u4BufSizeY = aeBufInfo.height;
            rInputData.u4CurHDRRatio = u4MVHDRRatio_x100;
            m_pICcuAe->ccuControl(MSG_TO_CCU_HDR_AE_INPUT_READY, (void *)&rInputData, nullptr);
            CAM_LOGD_IF(m_bDebugEnable, "[%s] send buffer to CCU, m_rBufferSets.mva(%d), u4MVHDRRatio_x100(%d), u4BufSizeX(%d), u4BufSizeY(%d), r_index(%d)", __FUNCTION__,
                        (MUINT32)m_rBufferSets[m_rBufIndex].mva, u4MVHDRRatio_x100, rInputData.u4BufSizeX, rInputData.u4BufSizeY, m_rBufIndex);
        }
    }

    for (MUINT32 u4CamsvIndex: m_vecCamsvIndex)
    {
        if(m_mapCamsvSttPipe[u4CamsvIndex] != nullptr) {
            if(m_bAbort || m_bPreStop)
                return -1;
            // enque HW buffer back driver
            string strCamsvSttPipe = "CamsvSttPipe(" + to_string(u4CamsvIndex) + ")";
            CAM_LOGD_IF(m_bDebugEnable, "enque %s Hw buffer back driver.\n", strCamsvSttPipe.c_str());
            strCamsvSttPipe += " STT enque";
            CAM_TRACE_BEGIN(strCamsvSttPipe.c_str());
            m_mapCamsvSttPipe[u4CamsvIndex]->enque(mapDQBuf[u4CamsvIndex]);
            CAM_TRACE_END();
        }
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
CamsvBufMgrImp::
dequeueHw(const char* tag, ICamsvStatisticPipe* m_pCamsvSttPipe, QBufInfo &rDQBuf, BufInfo &rLastBuf)
{
    MBOOL bEnable = property_get_int32("vendor.camsv.dump.enable", 0);
    MBOOL bEnableHW = property_get_int32("vendor.camsvhw.dump.enable", 0);

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
        strncpy(folderName, "/data/vendor/camsvBuf", sizeof(folderName));
        folderName[sizeof(folderName) - 1] = '\0';
        snprintf(fileName, sizeof(fileName), "/data/vendor/camsvBuf/camsvBuf_%s_%d_%d.raw", tag, rLastBuf.mMetaData.mMagicNum_hal, count++);
        if(dumpHwBuf(fileName, folderName, rLastBuf))
        {
            CAM_LOGD_IF(m_bDebugEnable,"%s\n", fileName);
        }
    }
    // dump mvhdr3expo hw data for ut
    if(bEnableHW){
        char fileName[64];
        char folderName[64];
        strncpy(folderName, "/data/vendor/camsvHwBuf", sizeof(folderName));
        folderName[sizeof(folderName)-1] = '\0';
        snprintf(fileName, sizeof(fileName), "/data/vendor/camsvHwBuf/camsvHwBuf_%s_%d.hwbuf", tag, rLastBuf.mMetaData.mMagicNum_hal);
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
CamsvBufMgrImp::
enqueueHwBuf()
{
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
StatisticBufInfo*
CamsvBufMgrImp::
dequeueSwBuf()
{
    CAM_LOGD_IF(m_bDebugEnable,"[%s] index(%d) +\n", __FUNCTION__, m_rBufIndex);
    if(m_bAbort || m_bPreStop)
        return NULL;
    int r_index = m_rBufIndex;
    StatisticBufInfo* pBuf = m_rBufInfo[r_index].read();

    // switch buffer for CCU OFF
    if (!m_bCcuEnable) {
        m_rBufIndex = ( r_index + 1 ) % MAX_MVHDR_STAT_BUFFER_CNT;
        CAM_LOGD_IF(m_bDebugEnable,"[%s] switch buffer for CCU OFF, m_i4SensorDev(%d), port(%d), r_index(%d), w_index(%d)\n", __FUNCTION__, m_i4SensorDev, m_rPort.index, r_index, m_rBufIndex);
    }
    return pBuf;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
CamsvBufMgrImp::
abortDequeue()
{
    CAM_LOGD("[%s]\n", __FUNCTION__);
    m_bAbort = MTRUE;
    for (MUINT32 u4CamsvIndex: m_vecCamsvIndex)
    {
        if(m_mapCamsvSttPipe[u4CamsvIndex] != NULL)
        {
            m_mapCamsvSttPipe[u4CamsvIndex]->abortDma(PORT_CAMSV_IMGO,LOG_TAG);
        }
    }

    for(int i = 0; i < MAX_MVHDR_STAT_BUFFER_CNT; i++)
        m_rBufInfo[i].flush();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
CamsvBufMgrImp::
notifyPreStop()
{
    CAM_LOGD("[%s] m_rBufIndex(%d) +\n", __FUNCTION__, m_rBufIndex);
    m_bPreStop = MTRUE;
    CAM_LOGD("[%s] m_rBufIndex(%d) -\n", __FUNCTION__, m_rBufIndex);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  CamsvStatisticBuf
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
CamsvStatisticBuf::
CamsvStatisticBuf()
    : m_fgLog(MFALSE)
    , m_fgCmd(MFALSE)
    , m_fgFlush(MFALSE)
    , m_i4TotalBufSize(0)
    , m_i4TotalFlkBufSize(0)

{
    m_rBufInfo.mSize = 0;
    m_rBufInfo.mVa = (MUINTPTR)NULL;
    m_rBufFlkInfo.mVa = (MUINTPTR)NULL;
    m_fgLog = property_get_int32("vendor.debug.mvhdr_stat_buf.enable", 1); // temp for test
}

CamsvStatisticBuf::
~CamsvStatisticBuf()
{
    freeBuf();
    freeFlkBuf();
}

MBOOL
CamsvStatisticBuf::
allocateBuf(MUINT32 const i4BufSize)
{
    Mutex::Autolock lock(m_Lock);

    if (!m_rBufInfo.mVa || !m_i4TotalBufSize) {
        m_i4TotalBufSize = i4BufSize;
        m_rBufInfo.mVa = (MUINTPTR) new MUINT32[m_i4TotalBufSize + 1];
        CAM_LOGD_IF(m_fgLog, "allocateBuf(): mVa(%p).", (void *)m_rBufInfo.mVa);
    }

    return MTRUE;
}

MBOOL
CamsvStatisticBuf::
allocateFlkBuf(MUINT32 const i4BufSize)
{
    Mutex::Autolock lock(m_Lock);
    m_fgLog = property_get_int32("vendor.debug.mvhdr_stat_buf.enable", 1); // temp for test

    if (!m_rBufFlkInfo.mVa || !m_i4TotalFlkBufSize) {
        m_i4TotalFlkBufSize = i4BufSize;
        m_rBufFlkInfo.mVa = (MUINTPTR) new MUINT32[m_i4TotalFlkBufSize + 1];
        CAM_LOGD_IF(m_fgLog, "allocateFlkBuf(): mVa(%p).", (void *)m_rBufFlkInfo.mVa);
    }

    return MTRUE;
}


MBOOL
CamsvStatisticBuf::
configBuf(MUINTPTR pVa)
{
    Mutex::Autolock lock(m_Lock);
    if (!m_rBufInfo.mVa)
    {
        m_rBufInfo.mVa = pVa;
        CAM_LOGD_IF(m_fgLog, "configBuf(): mVa(%p).", (void *)m_rBufInfo.mVa);
    }
    return MTRUE;
}

MBOOL
CamsvStatisticBuf::
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
CamsvStatisticBuf::
freeFlkBuf()
{
    Mutex::Autolock lock(m_Lock);

    if (m_rBufFlkInfo.mVa && m_i4TotalFlkBufSize) {
        CAM_LOGD_IF(m_fgLog, "freeFlkBuf(): mVa(%p).", (void *)m_rBufFlkInfo.mVa);
        delete [] (MUINT32 *)m_rBufFlkInfo.mVa;
        m_i4TotalFlkBufSize = 0;
    }

    return MTRUE;
}


MBOOL
CamsvStatisticBuf::
write(MINT32 i4SensorDev, unordered_map<MUINT32, BufInfo>& mapLastBuf, MUINT32& u4MVHDRRatio_x100)
{
    Mutex::Autolock lock(m_Lock);

    if (!m_rBufInfo.mVa) {
        CAM_LOGE("write(): mVa is NULL.");
        return MFALSE;
    }

    /* copy AE buffer info */
    m_rBufInfo.mMagicNumber = mapLastBuf[VC_3HDR_Y].mMetaData.mMagicNum_hal;
    m_rBufInfo.mSize = mapLastBuf[VC_3HDR_Y].mSize;
    m_rBufInfo.mStride = mapLastBuf[VC_3HDR_Y].mStride;
    m_rBufInfo.mTimeStamp = mapLastBuf[VC_3HDR_Y].mMetaData.mTimeStamp;
    MUINT32 u4SensorID = getSensorDevID((ESensorDev_T)i4SensorDev);
    CAMSV_STAT_T data;
    data.i4SensorDev = i4SensorDev;
    for (auto it: mapLastBuf)
    {
        MUINT32 VC_FEATURE_INDEX = it.first;
        BufInfo* buf = &it.second;
        data.mapCamsvDataPointer[VC_FEATURE_INDEX] = reinterpret_cast<MVOID *>(buf->mVa);
    }
    data.pAeOutputDataPointer = reinterpret_cast<MVOID *>(m_rBufInfo.mVa);
    GetCamsvCustomDecode(CAMSV_CUSTOM_DECODE_AE, u4SensorID, reinterpret_cast<MVOID *>(&data));
    m_rBufInfo.mMVHDRRatio_x100 = data.u4MVHDRRatio_x100;
    u4MVHDRRatio_x100 = data.u4MVHDRRatio_x100;

    /* ============== FLK + ============== */
    if (!m_rBufFlkInfo.mVa) {
        CAM_LOGE("write(): m_rBufFlkInfo.mVa is NULL.");
    }
    else
    {
        /* copy Flicker buffer info */
        m_rBufFlkInfo.mMagicNumber = mapLastBuf[VC_3HDR_FLICKER].mMetaData.mMagicNum_hal;
        m_rBufFlkInfo.mSize = mapLastBuf[VC_3HDR_FLICKER].mSize;
        m_rBufFlkInfo.mStride = mapLastBuf[VC_3HDR_FLICKER].mStride;
        m_rBufFlkInfo.mTimeStamp = mapLastBuf[VC_3HDR_FLICKER].mMetaData.mTimeStamp;
        data.pFlkOutputDataPointer = reinterpret_cast<MVOID *>(m_rBufFlkInfo.mVa);
        GetCamsvCustomDecode(CAMSV_CUSTOM_DECODE_FLK, u4SensorID, reinterpret_cast<MVOID *>(&data));

        AE_PERFRAME_INFO_T AEPerframeInfo;
        IAeMgr::getInstance().getAEInfo(i4SensorDev, AEPerframeInfo);
        MINT32 i4AeMode = AEPerframeInfo.rAEUpdateInfo.i4AEMode;
        if (i4AeMode)
        {
            // update flicker state
            FlickerInput flkIn;
            FlickerOutput flkOut;
            memset(&flkOut, 0, sizeof(flkOut));
            flkIn.aeExpTime = (int)(AEPerframeInfo.rAEISPInfo.u8P1Exposuretime_ns/1000);
            flkIn.pBuf = reinterpret_cast<MVOID*>(m_rBufFlkInfo.mVa);
            IFlickerHal::getInstance(i4SensorDev)->update(FLK_SENSOR_TYPE_3EXPO, &flkIn, &flkOut);
        }

        // Set AE flicker mode
        int flkResult;
        IFlickerHal::getInstance(i4SensorDev)->getFlickerState(flkResult);

        CAM_LOGD_IF(m_fgLog, "dequeueHwBuf(): set AE flicker state(%d).", flkResult);
        if (flkResult == HAL_FLICKER_AUTO_60HZ)
        {
            MUINT32 u4FlickerMode = LIB3A_AE_FLICKER_AUTO_MODE_60HZ;
            IAeMgr::getInstance().sendAECtrl(i4SensorDev, EAECtrl_SetAutoFlickerMode, u4FlickerMode, NULL, NULL, NULL);
        }
        else
        {
            MUINT32 u4FlickerMode = LIB3A_AE_FLICKER_AUTO_MODE_50HZ;
            IAeMgr::getInstance().sendAECtrl(i4SensorDev, EAECtrl_SetAutoFlickerMode, u4FlickerMode, NULL, NULL, NULL);
        }

        CAM_LOGD_IF(m_fgLog, "[%s]writeFlk done. Magic(%d) Size(%d) Stride(%d) TimeStamp(%d)\n",
            __FUNCTION__, mapLastBuf[VC_3HDR_FLICKER].mMetaData.mMagicNum_hal, mapLastBuf[VC_3HDR_FLICKER].mSize, mapLastBuf[VC_3HDR_FLICKER].mStride, mapLastBuf[VC_3HDR_FLICKER].mMetaData.mTimeStamp);
    }
    /* ============== FLK - ============== */

    /* update cmd and broadcast */
    m_fgCmd = MTRUE;
    m_Cond.broadcast();

    CAM_LOGD_IF(m_fgLog, "[%s] - m_fgCmd(%d) Magic(%d) size(%d) MVHDRRatio(%d)\n", __FUNCTION__, m_fgCmd, mapLastBuf[VC_3HDR_Y].mMetaData.mMagicNum_hal, mapLastBuf[VC_3HDR_Y].mSize, data.u4MVHDRRatio_x100);

    return MTRUE;
}

StatisticBufInfo*
CamsvStatisticBuf::
read()
{
    Mutex::Autolock lock(m_Lock);

    if(m_rBufInfo.mVa == 0){
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

void CamsvStatisticBuf::setAeBufInfo(CAMSV_CUSTOM_BUFFER_INFO bufInfo)
{
    m_rBufInfo.mMVHDRStatWidth = bufInfo.width;
    m_rBufInfo.mMVHDRStatHeight = bufInfo.height;
}

CAMSV_CUSTOM_BUFFER_INFO CamsvStatisticBuf::getAeBufInfo(void)
{
    CAMSV_CUSTOM_BUFFER_INFO bufInfo;
    bufInfo.width = m_rBufInfo.mMVHDRStatWidth;
    bufInfo.height = m_rBufInfo.mMVHDRStatHeight;
    return bufInfo;
}


void CamsvStatisticBuf::setFlkBufInfo(CAMSV_CUSTOM_BUFFER_INFO bufInfo)
{
    m_rBufFlkInfo.mMVHDRStatWidth = bufInfo.width;
    m_rBufFlkInfo.mMVHDRStatHeight = bufInfo.height;
}



MBOOL
CamsvStatisticBuf::
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

MUINT32 getSensorDevID(ESensorDev_T eSensorDev)
{
    MINT32 i4DebugEnable = property_get_int32("vendor.debug.camsv_stat_buf.enable", 1);
    SensorStaticInfo sInfo;
    MINT32 i4SensorIdx= mapSensorDevToIdx((MUINT32)eSensorDev);
    MINT32 i4SensorDevID;
    i4SensorDevID = NS3Av3::mapSensorIdxToDev(i4SensorIdx);
    // Get Sensor ID
    IHalSensorList* const pIHalSensorList = MAKE_HalSensorList();
    if(!pIHalSensorList) {
        CAM_LOGE("[%s()] Can not create pIHalSensorList\n", __FUNCTION__);
        return E_AE_NULL_AE_TABLE;
    }
    pIHalSensorList->querySensorStaticInfo(i4SensorDevID, &sInfo);
    CAM_LOGD_IF(i4DebugEnable, "[%s()] m_i4SensorDev:%d sensorID:%d\n", __FUNCTION__, i4SensorDevID, sInfo.sensorDevID);

    return sInfo.sensorDevID;
}



#endif

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
#define LOG_TAG "AFOBufMgr"

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

#include <afo_buf_mgr.h>

#include <dbg_aaa_param.h>
#include <dbg_af_param.h>
#include <camera_custom_nvram.h>
#include <aaa/af_param.h>
#include <af_feature.h>
#include <af_algo_if.h>
#include <af_mgr_if.h>
#include <isp_mgr_af_stat.h>
#include <af_define.h>

using namespace NS3Av3;
//using namespace NSCam::NSIoPipe;
//using namespace NSCam::NSIoPipe::NSCamIOPipe;
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
                  AFOBufMgrImp(MINT32 i4SensorDev, MINT32 i4SensorIdx, STT_CFG_INFO_T const sttInfo);
    virtual      ~AFOBufMgrImp();

    virtual MBOOL dequeueHwBuf();
    virtual MBOOL enqueueHwBuf();
    virtual StatisticBufInfo* dequeueSwBuf();
    virtual MVOID abortDequeue();
    virtual MINT32 waitDequeue();
    virtual MVOID notifyPreStop();
    virtual MVOID reset() {};

    virtual MBOOL sttDeque();
    virtual MBOOL sttEnque();

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data member
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    MINT32           const m_i4SensorDev;
    MINT32           const m_i4SensorIdx;
    MBOOL                  m_bDebugEnable;
    MBOOL                  m_bAbort;
    MUINT32                m_u4FrameCount;
    StatisticBufInfo       m_rHwBuf;
    MBOOL                  m_bPreStop;
    MBOOL                  m_bDequeStatus;
    MINT32                 m_IsAFSupport;
    MUINT32                m_u4ConfigLatency;
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
createInstance(MINT32 const i4SensorDev, MINT32 const i4SensorIdx, STT_CFG_INFO_T const sttInfo)
{
    AFOBufMgrImp* pObj = new AFOBufMgrImp(i4SensorDev, i4SensorIdx, sttInfo);
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
AFOBufMgrImp(MINT32 i4SensorDev, MINT32 i4SensorIdx, STT_CFG_INFO_T const sttInfo)
    : m_i4SensorDev(i4SensorDev)
    , m_i4SensorIdx(i4SensorIdx)
    , m_bDebugEnable(MFALSE)
    , m_bAbort(MFALSE)
    , m_u4FrameCount(0)
    , m_bDequeStatus(MFALSE)
    , m_u4ConfigLatency(2) // CPU : T+2 (sendcommand)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.afo_mgr.enable", value, "0");
    m_bDebugEnable = atoi(value);

    m_IsAFSupport = IAfMgr::getInstance().getAFMaxAreaNum(m_i4SensorDev);

    if (m_IsAFSupport)
    {
        IAFOBufMgrWrapper::getInstance().initPipe(m_i4SensorDev, m_i4SensorIdx, sttInfo.i4TgInfo);
        IAFOBufMgrWrapper::getInstance().startPipe(m_i4SensorDev);
    }
    else
    {
        CAM_LOGW("[%s] disable AF", __FUNCTION__);
    }

    m_bPreStop = MFALSE;

    CAM_LOGD("[%s] SensorDev(%d) TG(%d) -", __FUNCTION__, m_i4SensorDev, sttInfo.i4TgInfo);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AFOBufMgrImp::
~AFOBufMgrImp()
{
    CAM_LOGD("[%s] SensorDev(%d)", __FUNCTION__, m_i4SensorDev);
    if (m_IsAFSupport)
    {
        IAFOBufMgrWrapper::getInstance().stopPipe(m_i4SensorDev);
        IAFOBufMgrWrapper::getInstance().uninitPipe(m_i4SensorDev);
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
    } else
    {
        if(isHwBuf){
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
    CAM_LOGD_IF(m_bDebugEnable,"[%s] m_i4SensorDev(%d) deque \n", __FUNCTION__, m_i4SensorDev);

    if(m_bAbort || !m_IsAFSupport)
        return -1;

    m_bDequeStatus = IAFOBufMgrWrapper::getInstance().dequePipe(m_i4SensorDev); /* wait AF done*/

    if (m_bDequeStatus == MFALSE)
    {
        CAM_LOGW("[%s] Fail\n", __FUNCTION__);
        return MFALSE;
    }

    m_u4FrameCount++;

    CAM_LOGD_IF(m_bDebugEnable,"[%s] m_i4SensorDev(%d) done \n", __FUNCTION__, m_i4SensorDev);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
AFOBufMgrImp::
enqueueHwBuf()
{
    if(m_bAbort || !m_IsAFSupport)
        return -1;

    if (m_bDequeStatus != MFALSE)
    {
        IAFOBufMgrWrapper::getInstance().enquePipe(m_i4SensorDev);
    }
    else
    {
        CAM_LOGW("[%s] m_bDequeStatus(%d)\n", __FUNCTION__, m_bDequeStatus);
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
    MUINT32 i4Magic = 0;

    if(m_bAbort || !m_IsAFSupport || !m_bDequeStatus || m_u4FrameCount < 4)
        return NULL;

    CAM_LOGD_IF(m_bDebugEnable,"[%s] m_i4SensorDev(%d)\n", __FUNCTION__, m_i4SensorDev);

    BufInfo_T *rLastBuf = IAFOBufMgrWrapper::getInstance().dequeSwPipe(m_i4SensorDev);

    // copy the last HW buffer to SW Buffer.
    m_rHwBuf.mMagicNumber   = m_u4FrameCount;
    m_rHwBuf.mSize          = rLastBuf->size;
    m_rHwBuf.mVa            = rLastBuf->virtAddr;
    m_rHwBuf.mStride        = NSIspTuningv3::ISP_MGR_AF_STAT_CONFIG_T::getInstance( static_cast<ESensorDev_T>(m_i4SensorDev)).getAFStride(); // from isp_mgr_af
    m_rHwBuf.mFrameCount    = m_u4FrameCount;
    m_rHwBuf.mTimeStamp     = 0;
    m_rHwBuf.mConfigNum     = NSIspTuningv3::ISP_MGR_AF_STAT_CONFIG_T::getInstance( static_cast<ESensorDev_T>(m_i4SensorDev)).getConfigNum(i4Magic);
    m_rHwBuf.mConfigLatency = m_u4ConfigLatency;

    CAM_TRACE_BEGIN("AFO CCU parse");
    // Execute AF function
    IAfMgr::getInstance().passAFBuffer(m_i4SensorDev, reinterpret_cast<MVOID *>(&m_rHwBuf));
    CAM_TRACE_END();

    // dump afo data for debug
    if (bEnable) {
        static MUINT32 count = 0;
        char fileName[64];
        char folderName[64];
        MINT32 slen;
        #define AFO_DIR "/sdcard/afo"
        slen = strlen(AFO_DIR) + 1;
        strncpy(folderName, AFO_DIR, slen);
        sprintf(fileName, "/sdcard/afo/afo_%d_%d.raw", m_rHwBuf.mMagicNumber, count++);
        if(dumpHwBuf(fileName, folderName, m_rHwBuf))
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
        sprintf(fileName, "/sdcard/afoHwBuf/afo_%d.hwbuf", m_rHwBuf.mMagicNumber);
        if(dumpHwBuf(fileName, folderName, m_rHwBuf, bEnableHW))
        {
            CAM_LOGD_IF(m_bDebugEnable,"%s\n", fileName);
        }
    }

    if(m_bAbort)
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
    CAM_LOGD("[%s] abortDma done\n", __FUNCTION__);

    CAM_LOGD("[%s] +\n", __FUNCTION__);
    m_bAbort = MTRUE;
    m_u4FrameCount = 0;
    if (m_IsAFSupport)
    {
        IAFOBufMgrWrapper::getInstance().abortDeque(m_i4SensorDev);
    }
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
MINT32
AFOBufMgrImp::
waitDequeue()
{
    return 0;
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
AFOBufMgrImp::
sttDeque()
{
    return MFALSE;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
AFOBufMgrImp::
sttEnque()
{
    return MFALSE;
}

#endif

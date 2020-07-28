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
#define LOG_TAG "flko_buf_mgr"

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

#if CAM3_FAKE_SENSOR_DRV
#include "fake_sensor_drv/IStatisticPipe.h"
#else
#include <mtkcam/drv/iopipe/CamIO/IStatisticPipe.h>
#endif

#include <mtkcam/drv/iopipe/CamIO/IHalCamIO.h>
#include <private/IopipeUtils.h>

#include <ae_mgr/ae_mgr_if.h>
#include <flicker/flicker_hal_base.h>
#include <aaa_hal_sttCtrl.h>


using namespace NS3Av3;
using namespace NSCam::NSIoPipe;
using namespace NSCam::NSIoPipe::NSCamIOPipe;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  FLKO buffer
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define FLKO_MAX_LENG (6000)
#define FLKO_BUF_SIZE (FLKO_MAX_LENG * 4 * 3)

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class FLKOBufMgrImp : public FLKOBufMgr
{
public:
    virtual MBOOL destroyInstance();
                  FLKOBufMgrImp(MINT32 i4SensorDev, MINT32 i4SensorIdx);
    virtual      ~FLKOBufMgrImp();

    virtual MBOOL dequeueHwBuf();
    virtual MBOOL enqueueHwBuf();
    virtual StatisticBufInfo* dequeueSwBuf();
    virtual MVOID abortDequeue();
    virtual MINT32 waitDequeue(){return 0;};
    virtual MVOID notifyPreStop();
    virtual MVOID reset(){};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data member
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    MINT32           const m_i4SensorDev;
    MINT32           const m_i4SensorIdx;
    MBOOL                  m_bDebugEnable;
    MBOOL                  m_bFLKODumpEn;
    MBOOL                  m_bFLKOHWDumpEn;
    IStatisticPipe*        m_pSttPipe;
    MUINT32                m_rBufIndex;  // the index of write buffers.
    StatisticBuf           m_rBufInfo[MAX_STATISTIC_BUFFER_CNT];
    PortID                 m_rPort;
    MBOOL                  m_bAbort;
    MBOOL                  m_bEnableFlko;
    MBOOL                  m_bPreStop;
};

#if (CAM3_3ATESTLVL > CAM3_3AUT)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FLKOBufMgr*
FLKOBufMgr::
createInstance(MINT32 const i4SensorDev, MINT32 const i4SensorIdx)
{
    FLKOBufMgrImp* pObj = new FLKOBufMgrImp(i4SensorDev,i4SensorIdx);
    return pObj;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
FLKOBufMgrImp::
destroyInstance()
{
    delete this;
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FLKOBufMgrImp::
FLKOBufMgrImp(MINT32 i4SensorDev, MINT32 i4SensorIdx)
    : m_i4SensorDev(i4SensorDev)
    , m_i4SensorIdx(i4SensorIdx)
    , m_bDebugEnable(MFALSE)
    , m_bFLKODumpEn(MFALSE)
    , m_bFLKOHWDumpEn(MFALSE)
    , m_pSttPipe(NULL)
    , m_rBufIndex(0)
    , m_rPort(PORT_FLKO)
    , m_bAbort(MFALSE)
    , m_bPreStop(MFALSE)
{
    for(int i = 0; i < MAX_STATISTIC_BUFFER_CNT; i++)
    {
        CAM_LOGD("[%s]  i = %d \n", __FUNCTION__, i);
        m_rBufInfo[i].allocateBuf(FLKO_BUF_SIZE);
    }

    m_bDebugEnable = ::property_get_int32("vendor.debug.flko_mgr.enable", 0);
    m_bFLKODumpEn = ::property_get_int32("vendor.flko.dump.enable", 0);
    m_bFLKOHWDumpEn = ::property_get_int32("vendor.flkohw.dump.enable", 0);

    m_pSttPipe = IStatisticPipe::createInstance(m_i4SensorIdx, LOG_TAG);
    m_bEnableFlko = Hal3ASttCtrl::getInstance(m_i4SensorDev)->isFlkEnable();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FLKOBufMgrImp::
~FLKOBufMgrImp()
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
FLKOBufMgrImp::
dequeueHwBuf()
{
    CAM_LOGD_IF(m_bDebugEnable,"[%s] m_i4SensorDev(%d) + \n", __FUNCTION__, m_i4SensorDev);

    if(m_bAbort || m_bPreStop)
       return -1;

    if(m_pSttPipe != NULL)
    {
        if (m_bEnableFlko)
        {
            DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "dequeueHwBuf", 0);
            // deque HW buffer from driver.
            QBufInfo    rDQBuf;
            CAM_TRACE_BEGIN("FLKO STT deque");
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
            BufInfo rLastBuf = rDQBuf.mvOut.at( size-1 );

            // copy the last HW buffer to SW Buffer.
            StatisticBufInfo rHwBuf;
            rHwBuf.mMagicNumber = rLastBuf.mMetaData.mMagicNum_hal;
            rHwBuf.mSize = rLastBuf.mSize;
            rHwBuf.mVa = rLastBuf.mVa;
            rHwBuf.mStride = rLastBuf.mStride;
            rHwBuf.mTimeStamp = rLastBuf.mMetaData.mTimeStamp;

            DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "dequeueHwBuf", 2);
            CAM_TRACE_BEGIN("FLKO STT clone");
            m_rBufInfo[index].write(rHwBuf);
            CAM_TRACE_END();
            DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "dequeueHwBuf", 3);

            CAM_LOGD_IF(m_bDebugEnable, "port(%d), index(%d), va[%d]/pa[0x%d]/#(%d), Size(%d), timeStamp(%" PRId64 ")",
                      m_rPort.index, index, (MINT32)rHwBuf.mVa, (MINT32)rLastBuf.mPa, rHwBuf.mMagicNumber, rHwBuf.mSize, rHwBuf.mTimeStamp);

            // dump flko data for debug
            if (m_bFLKODumpEn) {
                static MUINT32 count = 0;
                char fileName[64];
                char folderName[64];
                strncpy(folderName, "/sdcard/flko", 13);
                sprintf(fileName, "/sdcard/flko/flko%d_%d.raw", rLastBuf.mMetaData.mMagicNum_hal, count++);
                if(dumpHwBuf(fileName, folderName, rLastBuf))
                {
                    CAM_LOGD_IF(m_bDebugEnable,"%s\n", fileName);
                }
            }
            // dump flko hw data for ut
            if(m_bFLKOHWDumpEn){
                char fileName[64];
                char folderName[64];
                strncpy(folderName, "/sdcard/flkoHwBuf", 18);
                sprintf(fileName, "/sdcard/flkoHwBuf/flko_%d.hwbuf", rLastBuf.mMetaData.mMagicNum_hal);
                if(dumpHwBuf(fileName, folderName, rLastBuf, m_bFLKOHWDumpEn))
                {
                    CAM_LOGD_IF(m_bDebugEnable,"%s\n", fileName);
                }
            }

            if(m_bAbort || m_bPreStop){
                DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "dequeueHwBuf", -1);
                return -1;
            }
            // enque HW buffer back driver
            CAM_LOGD_IF(m_bDebugEnable, "enque Hw buffer back driver.\n");
            CAM_TRACE_BEGIN("FLKO STT enque");
            m_pSttPipe->enque(rDQBuf);
            CAM_TRACE_END();
            DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "dequeueHwBuf", 4);

#if CAM3_FLICKER_FEATURE_EN
            if(m_bAbort || m_bPreStop){
                DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "dequeueHwBuf", -1);
                return -1;
            }
            CAM_TRACE_FMT_BEGIN("Flk");
            FlickerInput flkIn;
            FlickerOutput flkOut;
            flkOut.flickerResult = 0;
            AE_MODE_CFG_T previewInfo;

            MINT32 i4AeMode = IAeMgr::getInstance().getAEMode(m_i4SensorDev);
            if( (i4AeMode != 0) ){
                // Dequeue FLKO buffer from SW buffer, if flicker is enabled
                StatisticBufInfo* rBufInfo = dequeueSwBuf();
                if(rBufInfo == NULL) return S_3A_OK;

                IAeMgr::getInstance().getPreviewParams(m_i4SensorDev, previewInfo);
                flkIn.aeExpTime = previewInfo.u4Eposuretime;
                flkIn.afFullStat.bValid = MTRUE;
                flkIn.pBuf = reinterpret_cast<MVOID*>(rBufInfo->mVa);
                FlickerHalBase::getInstance().update(m_i4SensorDev, &flkIn, &flkOut);
            }

            int flkResult;
            FlickerHalBase::getInstance().getFlickerResult(flkResult);
            flkOut.flickerResult = flkResult;

            // Set flicker result to AE
            if(flkOut.flickerResult == HAL_FLICKER_AUTO_60HZ)
            {
                CAM_LOGD_IF(m_bDebugEnable, "setaeflicker 60hz");
                IAeMgr::getInstance().setAEAutoFlickerMode(m_i4SensorDev, 1);
            }
            else
            {
                CAM_LOGD_IF(m_bDebugEnable, "setaeflicker 50hz");
                IAeMgr::getInstance().setAEAutoFlickerMode(m_i4SensorDev, 0);
            }
            CAM_TRACE_FMT_END();
#endif
        }
        else
        {
#if CAM3_FLICKER_FEATURE_EN
            int flkResult;
            FlickerHalBase::getInstance().getFlickerResult(flkResult);
            CAM_LOGD_IF(m_bDebugEnable, "Preview, FLK not first open");

            // Set flicker result to AE
            if(flkResult == HAL_FLICKER_AUTO_60HZ)
            {
                CAM_LOGD_IF(m_bDebugEnable, "setaeflicker 60hz");
                IAeMgr::getInstance().setAEAutoFlickerMode(m_i4SensorDev, 1);
            }
            else
            {
                CAM_LOGD_IF(m_bDebugEnable, "setaeflicker 50hz");
                IAeMgr::getInstance().setAEAutoFlickerMode(m_i4SensorDev, 0);
            }
#endif
        }
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
FLKOBufMgrImp::
enqueueHwBuf()
{
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
StatisticBufInfo*
FLKOBufMgrImp::
dequeueSwBuf()
{
    CAM_LOGD_IF(m_bDebugEnable,"[%s] index(%d) +\n", __FUNCTION__, m_rBufIndex);
    if(m_bAbort || m_bPreStop)
        return NULL;
    DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "dequeueSwBuf", 0);
    int r_index = m_rBufIndex;
    CAM_LOGD_IF(m_bDebugEnable,"[%s] m_rBufInfo[%d] = %p +\n", __FUNCTION__, m_rBufIndex, &(m_rBufInfo[m_rBufIndex]));
    StatisticBufInfo* pBuf = m_rBufInfo[r_index].read();
    DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "dequeueSwBuf", 1);
    // switch buffer
    m_rBufIndex = ( r_index + 1 ) % MAX_STATISTIC_BUFFER_CNT;
    CAM_LOGD_IF(m_bDebugEnable,"[%s] m_i4SensorDev(%d), port(%d), r_index(%d), w_index(%d)\n", __FUNCTION__, m_i4SensorDev, m_rPort.index, r_index, m_rBufIndex);
    DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "dequeueSwBuf", -1);
    return pBuf;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
FLKOBufMgrImp::
abortDequeue()
{
    CAM_LOGD("[%s] m_rBufIndex(%d)\n", __FUNCTION__, m_rBufIndex);
    m_bAbort = MTRUE;
    if(m_pSttPipe != NULL && m_bEnableFlko)
        m_pSttPipe->abortDma(m_rPort,LOG_TAG);
    for(int i = 0; i < MAX_STATISTIC_BUFFER_CNT; i++)
    {
        CAM_LOGD_IF(m_bDebugEnable,"[%s] m_rBufInfo[%d] = %p +\n", __FUNCTION__, i, &(m_rBufInfo[i]));
        m_rBufInfo[i].flush();
    }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
FLKOBufMgrImp::
notifyPreStop()
{
    CAM_LOGD("[%s] m_rBufIndex(%d) +\n", __FUNCTION__, m_rBufIndex);
    m_bPreStop = MTRUE;
    CAM_LOGD("[%s] m_rBufIndex(%d) -\n", __FUNCTION__, m_rBufIndex);
}

#endif

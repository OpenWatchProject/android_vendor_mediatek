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

#define LOG_TAG "FLKOBufMgr"

#include <sys/stat.h>
#include <string.h>
#include <cutils/properties.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/Trace.h>
#if CAM3_FAKE_SENSOR_DRV
#include "fake_sensor_drv/IStatisticPipe.h"
#else
#include <mtkcam/drv/iopipe/CamIO/IStatisticPipe.h>
#endif
#include <mtkcam/drv/iopipe/Port.h>
#include <mtkcam/drv/iopipe/PortMap.h>

#include <aaa_types.h>
#include <aaa_error_code.h>
#include <flicker_hal_if.h>
#include <ae_mgr_if.h>
#include "IBufMgr.h"
#include "FLKOBufMgr.h"

#define IS_3EXPO_EN (0)

using namespace NS3Av3;
using namespace NSCam::NSIoPipe;
using namespace NSCam::NSIoPipe::NSCamIOPipe;

#define FLKO_MAX_LENG (6000)
#define FLKO_BUF_SIZE (FLKO_MAX_LENG * 4 * 3)
#define DEBUG_FLKO_MGR_ENABLE "vendor.debug.flko_mgr.enable"

static MBOOL dumpHwBuf(int sensorType, BufInfo& rLastBuf, MBOOL isHwBuf = 0)
{
    //char fileName[64];
    //char folderName[64];
    //char typeName[10] = "Normal";

    //if (sensorType == FLK_SENSOR_TYPE_3EXPO)
   // {
   //     snprintf(typeName, sizeof(typeName), "3expo");
   // }

    //if (isHwBuf) {
    //    snprintf(folderName, sizeof(folderName), "/sdcard/flkoHwBuf_%s", typeName);
    //    snprintf(fileName, sizeof(fileName), "/sdcard/flkoHwBuf_%s/flko_%d.hwbuf",
    //            typeName, rLastBuf.mMetaData.mMagicNum_hal);
    //} else {
    //    static MUINT32 count = 0;
    //    snprintf(folderName, sizeof(folderName), "/sdcard/flko_%s", typeName);
    //    snprintf(fileName, sizeof(fileName), "/sdcard/flko_%s/flko%d_%d.raw",
    //            typeName, rLastBuf.mMetaData.mMagicNum_hal, count++);
   // }

    //MINT32 err = mkdir(folderName, S_IRWXU | S_IRWXG | S_IRWXO);
    //FILE *fp = fopen(fileName, "w");
    //if (!fp) {
    //    CAM_LOGE("fail to open file to save img: %s", fileName);
    //    return MFALSE;
    //}

    //if (isHwBuf) {
    //    fwrite(&(rLastBuf.mMetaData.mMagicNum_hal), sizeof(MUINT32), 1, fp);
    //    fwrite(&(rLastBuf.mStride), sizeof(MUINT32), 1, fp);
    //    fwrite(&(rLastBuf.mSize), sizeof(MUINT32), 1, fp);
   // }
   // fwrite(reinterpret_cast<void *>(rLastBuf.mVa), 1, rLastBuf.mSize, fp);
   // fclose(fp);
    (void)sensorType;
    (void)rLastBuf;
    (void)isHwBuf;
    return MTRUE;
}

#if (CAM3_3ATESTLVL > CAM3_3AUT)
FLKOBufMgr* FLKOBufMgr::createInstance(
        MINT32 const i4SensorDev, MINT32 const i4SensorIdx, STT_CFG_INFO_T const sttInfo)
{
    (void)sttInfo;
    return new FLKOBufMgr(i4SensorDev, i4SensorIdx);
}

MBOOL FLKOBufMgr::destroyInstance()
{
    delete this;
    return MTRUE;
}

FLKOBufMgr::FLKOBufMgr(MINT32 i4SensorDev, MINT32 i4SensorIdx)
    : m_i4SensorDev(i4SensorDev)
    , m_i4SensorIdx(i4SensorIdx)
    , m_bDebugEnable(MFALSE)
    , m_pSttPipe(NULL)
    , m_rPort(PORT_FLKO)
    , m_bAbort(MFALSE)
    , m_sensorType(FLK_SENSOR_TYPE_NORMAL)
    , m_pCamsvSttPipeFlickerData(NULL)
    , m_p3ASttCtrl(NULL)
{
    m_bDebugEnable = property_get_int32(DEBUG_FLKO_MGR_ENABLE, 0);
    m_pSttPipe = IStatisticPipe::createInstance(m_i4SensorIdx, LOG_TAG);
    m_p3ASttCtrl = Hal3ASttCtrl::getInstance(m_i4SensorDev);
    //int type = m_p3ASttCtrl->isMvHDREnable();
    //if(type == FEATURE_MVHDR_SUPPORT_3EXPO_VIRTUAL_CHANNEL)
    //{
    //    CAM_LOGI("FLKOBufMgr(): Expo type = FLK_SENSOR_TYPE_3EXPO");
    //    m_sensorType = FLK_SENSOR_TYPE_3EXPO;
    //    m_pCamsvSttPipeFlickerData = ICamsvStatisticPipe::createInstance(m_i4SensorIdx, LOG_TAG, EPIPE_CAMSV_FEATURE_4CELL_MVHDR_FLK);
   // }
   // else
    //{
        CAM_LOGI("FLKOBufMgr(): Expo type = FLK_SENSOR_TYPE_NORMAL");
        m_sensorType = FLK_SENSOR_TYPE_NORMAL;
    //}
}

FLKOBufMgr::~FLKOBufMgr()
{
    if (m_pSttPipe)
    {
        m_pSttPipe->destroyInstance(LOG_TAG);
        m_pSttPipe = NULL;
    }

    if(m_pCamsvSttPipeFlickerData)
    {
        m_pCamsvSttPipeFlickerData->destroyInstance(LOG_TAG);
        m_pCamsvSttPipeFlickerData = NULL;
    }
}

MINT32 FLKOBufMgr::dequeueHwBuf()
{
    CAM_LOGD_IF(m_bDebugEnable, "dequeueHwBuf(): m_i4SensorDev(%d).", m_i4SensorDev);
    QBufInfo rDQBuf;
    BufInfo rLastBuf;
    //MUINT32 bufSize = 0;
    //MINT32 *buffer = NULL;

    if (m_bAbort)
        return -1;

    // Deque

    int res = dequeueHw(m_sensorType, rDQBuf, rLastBuf);
    if (res != MTRUE)
    {
        return res;
    }

#if CAM3_FLICKER_FEATURE_EN
    if (m_bAbort) // TODO
        return -1;

    //if (m_sensorType == FLK_SENSOR_TYPE_3EXPO)
   // {
        // Decode data
//#if IS_3EXPO_EN
//        bufSize = getMVHDR3ExpoFLKBufSize();
//        buffer = (MINT32 *)malloc(bufSize*4);
//        MUINT32 mode = 0;
//        decodeMVHDR3ExpoFLKStatistic((MVOID *)rLastBuf.mVa, (MVOID *)buffer, mode);
//#endif
//    }

    // CONTROL_AE_ANTIBANDING_MODE has no effect when
    // CONTROL_MODE is off
    // CONTROL_AE_MODE is off

    CAM_TRACE_BEGIN("FLK algo");
    MINT32 i4AeMode = IAeMgr::getInstance().getAEMode(m_i4SensorDev);
    if (i4AeMode)
    {
        AE_MODE_CFG_T previewInfo;
        IAeMgr::getInstance().getPreviewParams(m_i4SensorDev, previewInfo);

        // update flicker state
        FlickerInput flkIn;
        FlickerOutput flkOut;
        memset(&flkOut, 0, sizeof(flkOut));
        flkIn.aeExpTime = previewInfo.u4Eposuretime;
        //if (m_sensorType == FLK_SENSOR_TYPE_3EXPO)
       // {
       //     flkIn.pBuf = reinterpret_cast<MVOID*>(buffer);
       // }
       // else
       // {
            flkIn.pBuf = reinterpret_cast<MVOID*>(rLastBuf.mVa);
        //}
        IFlickerHal::getInstance(m_i4SensorDev)->update(m_sensorType, &flkIn, &flkOut);
    }
    CAM_TRACE_END();

//#if IS_3EXPO_EN
//    if (buffer)
//    {
//        free(buffer);
//        buffer = NULL;
//    }
//#endif

    if (m_bAbort) // TODO: why m_bAbort everywhere?
        return -1;

    // Enque
    enqueueHw(m_sensorType, rDQBuf);

#endif


#if CAM3_FLICKER_FEATURE_EN
    // Set AE flicker mode
    int flkResult;
    IFlickerHal::getInstance(m_i4SensorDev)->getFlickerState(flkResult);

    CAM_LOGD_IF(m_bDebugEnable, "dequeueHwBuf(): set AE flicker state(%d).",
            flkResult);
    if (flkResult == HAL_FLICKER_AUTO_60HZ)
        IAeMgr::getInstance().setAEAutoFlickerMode(
                m_i4SensorDev, LIB3A_AE_FLICKER_AUTO_MODE_60HZ);
    else
        IAeMgr::getInstance().setAEAutoFlickerMode(
                m_i4SensorDev, LIB3A_AE_FLICKER_AUTO_MODE_50HZ);
#endif

    return MTRUE;
}

MINT32 FLKOBufMgr::dequeueHw(int sensorType, QBufInfo &rDQBuf, BufInfo &rLastBuf)
{
    CAM_LOGD_IF(m_bDebugEnable, "dequeueHw(): m_i4SensorDev(%d).sensorType(%d)", m_i4SensorDev, sensorType);
    MBOOL bEnable = property_get_int32("vendor.flko.dump.enable", 0);
    MBOOL bEnableHW = property_get_int32("vendor.flkohw.dump.enable", 0);

    if (sensorType == FLK_SENSOR_TYPE_NORMAL)
    {
        if (!m_pSttPipe)
        {
            CAM_LOGE("dequeueHw(): Stt pipe is null.");
            return MFALSE;
        }

        if (IFlickerHal::getInstance(m_i4SensorDev)->isAttach()) {
            // deque HW buffer by blocking method
            CAM_LOGD_IF(m_bDebugEnable, "dequeueHw(): deque hw buffer.");
            CAM_TRACE_BEGIN("FLKO STT deque");
            if (m_pSttPipe->deque(m_rPort, rDQBuf) == MFALSE) {
                CAM_LOGE("dequeueHw(): m_pSttPipe deque fail.");
                CAM_TRACE_END();
                return MFALSE;
            }
            CAM_TRACE_END();
        }
        else
        {
            CAM_LOGD_IF(m_bDebugEnable, "dequeueHwBuf(): flicker is NOT attached.");
            return MFALSE;
        }
    }
    else    // 3 expo
    {
        if (!m_pCamsvSttPipeFlickerData)
        {
            CAM_LOGE("dequeueHw(): Camsv stt pipe is null.");
            return MFALSE;
        }
        // Deque HW buffer from driver.
        CAM_TRACE_BEGIN("FLKO 3 expo STT deque");
        PortID _dq_portID;
        if (!m_pCamsvSttPipeFlickerData->deque(_dq_portID, rDQBuf))
        {
            CAM_LOGE("dequeueHw(): m_pCamsvSttPipeFlickerData deque fail.");
            CAM_TRACE_END();
            return MFALSE;
        }
        CAM_TRACE_END();
    }

    // Get buffer size
    int size = rDQBuf.mvOut.size();
    if (!size)
    {
        CAM_LOGE("dequeueHw(): rDQBuf.mvOut.size is 0");
        return MFALSE;
    }
    if (m_bAbort)
        return -1;

    // Get the last HW buffer to SW Buffer
    rLastBuf = rDQBuf.mvOut.at(size - 1);
    CAM_LOGD_IF(m_bDebugEnable, "dequeueHw(): port(%d), va[%p]/pa[%p]/#(%d), Size(%d), timeStamp(%" PRId64 ")",
            m_rPort.index, (void *)rLastBuf.mVa, (void *)rLastBuf.mPa,
            rLastBuf.mMetaData.mMagicNum_hal, rLastBuf.mSize, rLastBuf.mMetaData.mTimeStamp);

    // Dump buffer
    if (bEnable)
    {
        dumpHwBuf(sensorType, rLastBuf);
    }
    if (bEnableHW)
    {
        dumpHwBuf(sensorType, rLastBuf, bEnableHW);
    }

    return MTRUE;
}

MBOOL FLKOBufMgr::enqueueHwBuf()
{
    // We enque HW buffer once we copy to SW buffer
    return MTRUE;
}

MINT32 FLKOBufMgr::enqueueHw(int sensorType, QBufInfo &rDQBuf)
{
    CAM_LOGD_IF(m_bDebugEnable, "enqueueHw(): m_i4SensorDev(%d).", m_i4SensorDev);

    if (sensorType == FLK_SENSOR_TYPE_NORMAL)
    {
        if(!m_pSttPipe)
        {
            CAM_LOGE("enqueueHw(): m_pSttPipe is null, can not enque.");
            return MFALSE;
        }

        CAM_LOGD_IF(m_bDebugEnable, "enqueueHw(): enque hw buffer.");
        CAM_TRACE_BEGIN("FLKO STT enque");
        m_pSttPipe->enque(rDQBuf);
        CAM_TRACE_END();
    }
    else    // 3 expo
    {
        if(!m_pCamsvSttPipeFlickerData)
        {
            CAM_LOGE("enqueueHw(): m_pCamsvSttPipeFlickerData is null, can not enque.");
            return MFALSE;
        }

        // Enque HW buffer back driver
        CAM_LOGD_IF(m_bDebugEnable, "enqueueHw(): enque 3 expo hw buffer.");
        CAM_TRACE_BEGIN("FLKO 3expo STT enque");
        m_pCamsvSttPipeFlickerData->enque(rDQBuf);
        CAM_TRACE_END();
    }
    return MTRUE;
}


StatisticBufInfo* FLKOBufMgr::dequeueSwBuf()
{
    return NULL;
}

MVOID FLKOBufMgr::abortDequeue()
{
    CAM_LOGD("abortDequeue().");

    m_bAbort = MTRUE;

    if (m_pSttPipe && IFlickerHal::getInstance(m_i4SensorDev)->isAttach())
        m_pSttPipe->abortDma(m_rPort, LOG_TAG);

    if(m_pCamsvSttPipeFlickerData)
    {
        m_pCamsvSttPipeFlickerData->abortDma(PORT_CAMSV_IMGO,LOG_TAG);
    }
}

MINT32 FLKOBufMgr::waitDequeue()
{
    return 0;
}

#endif


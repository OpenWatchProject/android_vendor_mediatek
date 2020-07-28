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
#include <ae_mgr/ae_mgr_if.h>
#include "IBufMgr.h"
#include "flko_buf_mgr.h"
#include "FLKOBufMgr.h"

// FIX-ME, for IP-Base build pass +
// #include "camera_custom_mvhdr3expo_flicker.h"
// FIX-ME, for IP-Base build pass -


using namespace NS3Av3;
using namespace NSCam::NSIoPipe;
using namespace NSCam::NSIoPipe::NSCamIOPipe;

#define FLKO_MAX_LENG (6000)
#define FLKO_BUF_SIZE (FLKO_MAX_LENG * 4 * 3)
#define DEBUG_FLKO_MGR_ENABLE "vendor.debug.flko_mgr.enable"
#define FLK_ENABLE MTRUE


static MBOOL dumpHwBuf(BufInfo_T& rLastBuf, MBOOL isHwBuf = 0)
{
    // TODO:
    // char fileName[64];
    // char folderName[64];
    // char typeName[10] = "Normal";

    // if (sensorType == TYPE_3EXPO)
    // {
    //     snprintf(typeName, sizeof(typeName), "3expo");
    // }

    // if (isHwBuf) {
    //     snprintf(folderName, sizeof(folderName), "/sdcard/flkoHwBuf_%s", typeName);
    //     snprintf(fileName, sizeof(fileName), "/sdcard/flkoHwBuf_%s/flko_%d.hwbuf",
    //             typeName, rLastBuf.mMetaData.mMagicNum_hal);
    // } else {
    //     static MUINT32 count = 0;
    //     snprintf(folderName, sizeof(folderName), "/sdcard/flko_%s", typeName);
    //     snprintf(fileName, sizeof(fileName), "/sdcard/flko_%s/flko%d_%d.raw",
    //             typeName, rLastBuf.mMetaData.mMagicNum_hal, count++);
    // }

    // MINT32 err = mkdir(folderName, S_IRWXU | S_IRWXG | S_IRWXO);
    // FILE *fp = fopen(fileName, "w");
    // if (!fp) {
    //     CAM_LOGE("fail to open file to save img: %s", fileName);
    //     return MFALSE;
    // }

    // if (isHwBuf) {
    //     fwrite(&(rLastBuf.mMetaData.mMagicNum_hal), sizeof(MUINT32), 1, fp);
    //     fwrite(&(rLastBuf.mStride), sizeof(MUINT32), 1, fp);
    //     fwrite(&(rLastBuf.mSize), sizeof(MUINT32), 1, fp);
    // }
    // fwrite(reinterpret_cast<void *>(rLastBuf.mVa), 1, rLastBuf.mSize, fp);
    // fclose(fp);
    (void)rLastBuf;
    (void)isHwBuf;

    return MTRUE;
}

#if (CAM3_3ATESTLVL > CAM3_3AUT)
FLKOBufMgr* FLKOBufMgr::createInstance(
        MINT32 const i4SensorDev, MINT32 const i4SensorIdx, STT_CFG_INFO_T const sttInfo)
{
    return new FLKOBufMgr(i4SensorDev, i4SensorIdx, sttInfo);
}

MBOOL FLKOBufMgr::destroyInstance()
{
    delete this;
    return MTRUE;
}

FLKOBufMgr::FLKOBufMgr(MINT32 i4SensorDev, MINT32 i4SensorIdx, STT_CFG_INFO_T const sttInfo)
    : m_i4SensorDev(i4SensorDev)
    , m_i4SensorIdx(i4SensorIdx)
    , m_bDebugEnable(MFALSE)
    , m_pSttPipe(NULL)
    , m_rPort(PORT_FLKO)
    , m_bAbort(MFALSE)
    , m_pCamsvSttPipeFlickerData(NULL)
    , m_p3ASttCtrl(NULL)
{
    m_bDebugEnable = property_get_int32(DEBUG_FLKO_MGR_ENABLE, 0);
    if (FLK_ENABLE == MTRUE)
    {
        IFLKOBufMgrWrapper::getInstance().initPipe(m_i4SensorDev, m_i4SensorIdx, sttInfo.i4TgInfo);
        IFLKOBufMgrWrapper::getInstance().startPipe(m_i4SensorDev);
    }
}

FLKOBufMgr::~FLKOBufMgr()
{
    IFLKOBufMgrWrapper::getInstance().stopPipe(m_i4SensorDev);
    IFLKOBufMgrWrapper::getInstance().uninitPipe(m_i4SensorDev);
}

MINT32 FLKOBufMgr::dequeueHwBuf()
{
    CAM_LOGD_IF(m_bDebugEnable, "dequeueHwBuf(): m_i4SensorDev(%d).", m_i4SensorDev);
    BufInfo_T rDQBuf;
    BufInfo_T rLastBuf;
    //MUINT32 bufSize = 0;

    if (FLK_ENABLE != MTRUE)
        return MTRUE;

    if (m_bAbort)
        return -1;

    // Deque

    int res = dequeueHw(rDQBuf, rLastBuf);
    if (res != MTRUE)
    {
        return res;
    }

#if CAM3_FLICKER_FEATURE_EN
    if (m_bAbort)
        return -1;

    // CONTROL_AE_ANTIBANDING_MODE has no effect when
    // CONTROL_MODE is off
    // CONTROL_AE_MODE is off

//    CAM_TRACE_BEGIN("FLK algo");
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
        flkIn.pBuf = reinterpret_cast<MVOID*>(rLastBuf.virtAddr);
        IFlickerHal::getInstance(m_i4SensorDev)->update(0, &flkIn, &flkOut);
    }
//    CAM_TRACE_END();

    if (m_bAbort)
        return -1;

    // Enque
    enqueueHw(rDQBuf);

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

MINT32 FLKOBufMgr::dequeueHw(BufInfo_T &rDQBuf, BufInfo_T &rLastBuf)
{
    CAM_LOGD_IF(m_bDebugEnable, "dequeueHw(): m_i4SensorDev(%d).", m_i4SensorDev);
    MBOOL bEnable = property_get_int32("vendor.flko.dump.enable", 0);
    MBOOL bEnableHW = property_get_int32("vendor.flkohw.dump.enable", 0);

    CAM_TRACE_BEGIN("FLKO deque");
    MBOOL ret = IFLKOBufMgrWrapper::getInstance().dequePipe(m_i4SensorDev, rDQBuf);
    CAM_TRACE_END();

    // Get buffer size
    int size = rDQBuf.size;
    if (!size)
    {
        CAM_LOGE("dequeueHw(): rDQBuf.size is 0");
        return MFALSE;
    }
    if (m_bAbort)
        return -1;

    // Get the last HW buffer to SW Buffer
    rLastBuf = rDQBuf;
//    CAM_LOGD_IF(m_bDebugEnable, "dequeueHw(): port(%d), va[%p]/pa[%p]/#(%d), Size(%d), timeStamp(%" PRId64 ")",
//            m_rPort.index, (void *)rLastBuf.virtAddr, (void *)rLastBuf.mPa,
//            rLastBuf.mMetaData.mMagicNum_hal, rLastBuf.mSize, rLastBuf.mMetaData.mTimeStamp);

    // Dump buffer
    if (bEnable)
    {
        dumpHwBuf(rLastBuf);
    }
    if (bEnableHW)
    {
        dumpHwBuf(rLastBuf, bEnableHW);
    }

    return MTRUE;
}

MBOOL FLKOBufMgr::enqueueHwBuf()
{
    // We enque HW buffer once we copy to SW buffer
    return MTRUE;
}

MINT32 FLKOBufMgr::enqueueHw(BufInfo_T &rDQBuf)
{
    CAM_LOGD_IF(m_bDebugEnable, "enqueueHw(): m_i4SensorDev(%d).", m_i4SensorDev);

    CAM_TRACE_BEGIN("FLKO enque");
    MBOOL ret = IFLKOBufMgrWrapper::getInstance().enquePipe(m_i4SensorDev, rDQBuf);
    CAM_TRACE_END();

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
}

MINT32 FLKOBufMgr::waitDequeue()
{
    return 0;
}

#endif


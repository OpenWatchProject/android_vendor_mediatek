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
*      TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

/**
* @file eis_hal.cpp
*
* EIS Hal Source File
*
*/

#include <cstdio>
#include <queue>
#include <android/sensor.h>
#include <utils/threads.h>
#include <cutils/atomic.h>
#include <utils/SystemClock.h>
#include <utils/Trace.h>
#include <cutils/log.h>
#include <cutils/properties.h>
#include <cutils/atomic.h>

#include <mtkcam/utils/imgbuf/IIonImageBufferHeap.h>
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/drv/iopipe/CamIO/IHalCamIO.h>
#include <mtkcam/utils/sys/SensorProvider.h>

using namespace std;
using namespace android;
using namespace NSCam;
using namespace NSCam::NSIoPipe;
using namespace NSCam::NSIoPipe::NSCamIOPipe;
using namespace NSCam::Utils;

#include "gis_calibration_imp.h"

#include <mtkcam3/feature/lmv/lmv_hal.h>
#include <mtkcam/aaa/INvBufUtil.h>
#include <camera_custom_nvram.h>
#include <camera_custom_eis.h>


/*******************************************************************************
*
********************************************************************************/
#define GIS_CALIBRATION_HAL_DEBUG

#ifdef GIS_CALIBRATION_HAL_DEBUG

#undef __func__
#define __func__ __FUNCTION__

#undef  LOG_TAG
#define LOG_TAG "GISCalibration"
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_EIS_HAL);

#undef MY_LOGD
#undef MY_LOGD
#undef MY_LOGI
#undef MY_LOGW
#undef MY_LOGE
#undef MY_TRACE_API_LIFE
#undef MY_TRACE_FUNC_LIFE
#undef MY_TRACE_TAG_LIFE

#define MY_LOGD(fmt, arg...)       CAM_ULOGMD("[%s]" fmt, __func__, ##arg)
#define MY_LOGI(fmt, arg...)       CAM_ULOGMI("[%s]" fmt, __func__, ##arg)
#define MY_LOGW(fmt, arg...)       CAM_ULOGMW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define MY_LOGE(fmt, arg...)       CAM_ULOGME("[%s] %s ERROR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)
#define MY_TRACE_API_LIFE()        CAM_ULOGM_APILIFE()
#define MY_TRACE_FUNC_LIFE()       CAM_ULOGM_FUNCLIFE()
#define MY_TRACE_TAG_LIFE(name)    CAM_ULOGM_TAGLIFE(name)
#endif


#define EIS_HAL_NAME "GISCalibration"
#define intPartShift 8
#define floatPartShift (31 - intPartShift)
#define DEBUG_DUMP_FRAMW_NUM 10


const MUINT32 GyroInterval_ms = 5;

MINT32 GisCalibrationImp::mEISInterval = GyroInterval_ms;
MINT32 GisCalibrationImp::mEMEnabled = 0;
MINT32 GisCalibrationImp::mDebugDump = 0;


typedef struct EIS_TSRecord_t
{
    MUINT64 id;
    MUINT64 ts;
}EIS_TSRecord;



#define UNREASONABLE_GYRO_VALUE (10.0f)

/*******************************************************************************
* Debug property
********************************************************************************/

/*******************************************************************************
* User defined
********************************************************************************/
GyroItem GisCalibrationImp::mGyroData[MAX_SENSOR_ID];
/*******************************************************************************
*
********************************************************************************/
GisCalibration *GisCalibration::CreateInstance(char const *userName,const MUINT32 &aSensorIdx,
    const MUINT32 eisFactor)
{
    MY_LOGD("%s - sensor ID:%d", userName, aSensorIdx);
    return new GisCalibrationImp(aSensorIdx, eisFactor);
}

/*******************************************************************************
*
********************************************************************************/
MVOID GisCalibrationImp::DestroyInstance(char const *userName)
{
    MY_LOGD("%s - sensor ID:%d is destroyed", userName, mSensorIdx);
    delete this;
}


/*******************************************************************************
*
********************************************************************************/
GisCalibrationImp::GisCalibrationImp(const MUINT32 &aSensorIdx, const MUINT32 eisFactor)
    : GisCalibration()
    , mSensorIdx(aSensorIdx)
    , mEisFactor(eisFactor)
{
    mUsers = 0;

    //> member variable
    mDoEisCount = 0;    //Vent@20140427: Add for EIS GMV Sync Check.
    mFrameCnt = 0;
    mIsEisPlusConfig = 0;
    mCurSensorIdx = 0;
    mEisSupport = MTRUE;
    mGyroEnable = MFALSE;
    mAccEnable  = MFALSE;

    mTsForAlgoDebug = 0;
    m_pNvram = NULL;
    mGisInputW = 0;
    mGisInputH = 0;
    mSkipWaitGyro = MFALSE;
    mbEMSaveFlag = MFALSE;

    mEISInterval = GyroInterval_ms;
    mEMEnabled = 0;
    mDebugDump = 0;
    m_pLMVHal = NULL;
    m_pGisAlg = NULL;

    mChangedInCalibration = 0;
    mNVRAMRead = MFALSE;
    mSleepTime = 0;
    mbLastCalibration = MTRUE;
    m_pNVRAM_defParameter = NULL;
    m_pGisWorkBuf = NULL;

    // Sensor Provider
    mpSensorProvider = NULL;

    // Sensor
    m_pHalSensorList = NULL;
    m_pHalSensor = NULL;


}

/*******************************************************************************
*
********************************************************************************/
MINT32 GisCalibrationImp::Init()
{
    MY_TRACE_API_LIFE();

    //====== Check Reference Count ======
    Mutex::Autolock lock(mLock);

    if(mUsers > 0)
    {
        android_atomic_inc(&mUsers);
        MY_LOGD("snesorIdx(%u) has %d users",mSensorIdx,mUsers);
        return LMV_RETURN_NO_ERROR;
    }

    //====== Dynamic Debug ======
    mEISInterval = GyroInterval_ms;
    mEMEnabled = 0;
    mDebugDump = 0;

    mDebugDump = property_get_int32("vendor.debug.eis.giscalibration", mDebugDump);
    mEMEnabled = property_get_int32("vendor.debug.eis.EMEnabled", mEMEnabled);
    mEISInterval = property_get_int32("vendor.eisrecord.setinterval", GyroInterval_ms);

    while(!mGyroData[mSensorIdx].mGyroDataQueue.empty())
    {
        mGyroData[mSensorIdx].mGyroDataQueue.pop();
    }
    mGyroData[mSensorIdx].mGyroCount = 0;
    mGyroData[mSensorIdx].mGyroReverse = 0;
    mGyroData[mSensorIdx].mLastGyroTimestamp = 0;

    mCurSensorIdx = mSensorIdx;
    m_pLMVHal = NULL;
    //Init GIS member data
    m_pNvram = NULL;
    mChangedInCalibration = 0;
    mNVRAMRead = MFALSE;
    mSleepTime = 0;

    mbLastCalibration = MTRUE;
    mbEMSaveFlag = MFALSE;
    m_pNVRAM_defParameter = NULL;
    m_pNVRAM_defParameter = new NVRAM_CAMERA_FEATURE_GIS_STRUCT;
    memset(m_pNVRAM_defParameter, 0, sizeof(NVRAM_CAMERA_FEATURE_GIS_STRUCT));
    memset(mSensorInfo, 0, sizeof(mSensorInfo));

    MY_LOGD("(%p)  mSensorIdx(%u) init", this, mSensorIdx);

    //====== Create Sensor Object ======

    m_pHalSensorList = MAKE_HalSensorList();
    if(m_pHalSensorList == NULL)
    {
        MY_LOGE("IHalSensorList::get fail");
        goto create_fail_exit;
    }

    //====== Create EIS Algorithm Object ======

    m_pGisAlg = MTKGyro::createInstance();

    if (m_pGisAlg == NULL)
    {
        MY_LOGE("MTKGyro::createInstance fail");
        goto create_fail_exit;
    }

    //====== Create Sensor Listener Object ======
    mpSensorProvider = NULL;

    //====== Increase User Count ======

    android_atomic_inc(&mUsers);

    MY_LOGD("-");
    return LMV_RETURN_NO_ERROR;

create_fail_exit:

    if (m_pHalSensorList != NULL)
    {
        m_pHalSensorList = NULL;
    }

    MY_LOGD("-");
    return LMV_RETURN_NULL_OBJ;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 GisCalibrationImp::Uninit()
{
    MY_TRACE_API_LIFE();

    Mutex::Autolock lock(mLock);

    //====== Check Reference Count ======

    if(mUsers <= 0)
    {
        MY_LOGD("mSensorIdx(%u) has 0 user",mSensorIdx);
        return LMV_RETURN_NO_ERROR;
    }

    //====== Uninitialize ======

    android_atomic_dec(&mUsers);    //decrease referebce count

    if(mUsers == 0)    // there is no user
    {
        MINT32 err = LMV_RETURN_NO_ERROR;

        MY_LOGD("mSensorIdx(%u) uninit", mSensorIdx);

        //======  Release EIS Algo Object ======

        if (UNLIKELY((mDebugDump >= 2) || (mEMEnabled == 1)))
        {
            MY_LOGD("mIsEisPlusConfig(%d)",mIsEisPlusConfig);
            if(mIsEisPlusConfig == 1)
            {
                MY_TRACE_TAG_LIFE("GISCalibration::SaveLog");
                err = m_pGisAlg->GyroFeatureCtrl(GYRO_FEATURE_SAVE_LOG, (MVOID *)&mTsForAlgoDebug, NULL);
                if(err != S_GYRO_OK)
                {
                    MY_LOGE("GyroFeatureCtrl(GYRO_FEATURE_SAVE_LOG) fail(0x%x)",err);
                }
            }
        }

        //Writeback to NVRAM
        if (mNVRAMRead)
        {
            //Force update the NVRAM tmp buffer
            if (m_pNvram && m_pNVRAM_defParameter)
            {
                memcpy(&(m_pNvram->gis), m_pNVRAM_defParameter, sizeof(NVRAM_CAMERA_FEATURE_GIS_STRUCT));
            }
            if ( auto pNvBufUtil = MAKE_NvBufUtil() ) {
                err = pNvBufUtil->write(CAMERA_NVRAM_DATA_FEATURE, mSensorInfo[mSensorIdx].mSensorDev);
            }
        }

        if (m_pGisAlg != NULL)
        {
            MY_LOGD("m_pGisAlg uninit");
            m_pGisAlg->GyroReset();
            m_pGisAlg->destroyInstance(m_pGisAlg);
            m_pGisAlg = NULL;
        }

        // Next-Gen EIS
        if (mpSensorProvider != NULL)
        {
            MY_TRACE_TAG_LIFE("SensorProvider::disableSensor");
            MY_LOGD("mpSensorProvider uninit");
            //mpSensorProvider->disableSensor(SensorType_Acc);
            mpSensorProvider->disableSensor(SENSOR_TYPE_GYRO);
            mpSensorProvider = NULL;
        }


        //====== Destroy Sensor Object ======

        if(m_pHalSensorList != NULL)
        {
            m_pHalSensorList = NULL;
        }

        //======  Release Memory and IMem Object ======
        //>  free GIS working buffer

        if (m_pGisWorkBuf != NULL)
        {
            m_pGisWorkBuf->unlockBuf("GisWorkBuf");
            DestroyMemBuf(m_pGisWorkBuf);
        }

        //======  Reset Member Variable ======
        if (m_pLMVHal != NULL)
        {
            m_pLMVHal->DestroyInstance(LOG_TAG);
            m_pLMVHal = NULL;
        }

        mFrameCnt = 0; // first frmae
        mIsEisPlusConfig = 0;
        mDoEisCount = 0;    //Vent@20140427: Add for EIS GMV Sync Check.
        mGyroEnable = MFALSE;
        mAccEnable  = MFALSE;

        mChangedInCalibration = 0;
        mGisInputW = 0;
        mGisInputH = 0;
        mNVRAMRead = MFALSE;
        m_pNvram = NULL;
        mSleepTime = 0;
        mbLastCalibration = MTRUE;

        while(!mGyroData[mSensorIdx].mGyroDataQueue.empty())
        {
            mGyroData[mSensorIdx].mGyroDataQueue.pop();
        }
        mGyroData[mSensorIdx].mGyroCount = 0;
        mGyroData[mSensorIdx].mGyroReverse = 0;
        mGyroData[mSensorIdx].mLastGyroTimestamp = 0;

        mbEMSaveFlag = MFALSE;

        if (NULL != m_pNVRAM_defParameter)
        {
            delete m_pNVRAM_defParameter;
            m_pNVRAM_defParameter = NULL;
        }

        mEISInterval = GyroInterval_ms;
        mEMEnabled = 0;
        mDebugDump = 0;

    }
    else
    {
        MY_LOGD("mSensorIdx(%u) has %d users",mSensorIdx,mUsers);
    }


    return LMV_RETURN_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 GisCalibrationImp::CreateMemBuf(MUINT32 memSize, android::sp<IImageBuffer>& spImageBuf)
{
    MY_TRACE_FUNC_LIFE();

    MINT32 err = LMV_RETURN_NO_ERROR;
    IImageBufferAllocator* pImageBufferAlloc = IImageBufferAllocator::getInstance();

    MY_LOGD("Size(%u)",memSize);
    IImageBufferAllocator::ImgParam bufParam((size_t)memSize, 0);
    spImageBuf = pImageBufferAlloc->alloc("EIS_HAL", bufParam);
    MY_LOGD("X");
    return err;
}

/******************************************************************************
*
*******************************************************************************/
MINT32 GisCalibrationImp::DestroyMemBuf(android::sp<IImageBuffer>& spImageBuf)
{
    MY_TRACE_FUNC_LIFE();

    MINT32 err = LMV_RETURN_NO_ERROR;
    IImageBufferAllocator* pImageBufferAlloc = IImageBufferAllocator::getInstance();
    MY_LOGD("DestroyMemBuf");


    if (spImageBuf != NULL)
    {
        pImageBufferAlloc->free(spImageBuf.get());
        spImageBuf = NULL;
    }

    MY_LOGD("X");
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 GisCalibrationImp::GetSensorInfo(MUINT32 sensorID)
{
    MY_TRACE_FUNC_LIFE();

    MINT32 err = LMV_RETURN_NO_ERROR;

    mSensorInfo[sensorID].mSensorDev = m_pHalSensorList->querySensorDevIdx(sensorID);
    MY_LOGD("[mSensorInfo] sensorIdx(%u), mSensorDev(%u)", sensorID, mSensorInfo[sensorID].mSensorDev);

    m_pHalSensor = m_pHalSensorList->createSensor(EIS_HAL_NAME,1,&sensorID);

    if (LIKELY(m_pHalSensor != NULL))
    {
        err = m_pHalSensor->sendCommand( mSensorInfo[sensorID].mSensorDev,SENSOR_CMD_GET_PIXEL_CLOCK_FREQ,
                                         (MUINTPTR)&mSensorInfo[sensorID].mSensorPixelClock, 0, 0);
        if (err != LMV_RETURN_NO_ERROR)
        {
            MY_LOGE("SENSOR_CMD_GET_PIXEL_CLOCK_FREQ is fail(0x%x)",err);
            return LMV_RETURN_API_FAIL;
        }

        err = m_pHalSensor->sendCommand( mSensorInfo[sensorID].mSensorDev,SENSOR_CMD_GET_FRAME_SYNC_PIXEL_LINE_NUM,
                                         (MUINTPTR)&mSensorInfo[sensorID].mSensorLinePixel, 0, 0);
        if (err != LMV_RETURN_NO_ERROR)
        {
            MY_LOGE("SENSOR_CMD_GET_PIXEL_CLOCK_FREQ is fail(0x%x)",err);
            return LMV_RETURN_API_FAIL;
        }

        m_pHalSensor->destroyInstance(EIS_HAL_NAME);
        m_pHalSensor = NULL;

    }else
    {
        MY_LOGE("m_pHalSensorList->createSensor fail, m_pHalSensor == NULL");
    }

    MY_LOGD("mSensorDev(%u), pixelclock (%d), pixelline(%x)", mSensorInfo[sensorID].mSensorDev,
                                                              mSensorInfo[sensorID].mSensorPixelClock,
                                                              mSensorInfo[sensorID].mSensorLinePixel);

    return LMV_RETURN_NO_ERROR;
}

MINT32 GisCalibrationImp::EnableSensor()
{
    {
        MY_TRACE_TAG_LIFE("SensorProvider::createInstance");
        mpSensorProvider = SensorProvider::createInstance(EIS_HAL_NAME);
    }

    {
        MY_TRACE_TAG_LIFE("SensorProvider::enableSensor");
        // No use for accelerameter
        //mAccEnable  = mpSensorProvider->enableSensor(SensorType_Acc,mEISInterval);
        mAccEnable  = MFALSE;
        mGyroEnable = mpSensorProvider->enableSensor(SENSOR_TYPE_GYRO, mEISInterval);
        MY_LOGD("EN:(Acc,Gyro)=(%d,%d)",mAccEnable,mGyroEnable);
    }

    if (m_pLMVHal == NULL)
    {
        m_pLMVHal = LMVHal::CreateInstance(LOG_TAG, mSensorIdx);
    }

    return LMV_RETURN_NO_ERROR;
}

MINT32 GisCalibrationImp::CalcSensorTrs(MUINT32 sensorID, MUINT32 sensorHeight)
{
    MDOUBLE tRS = 0.0f,numLine;
    numLine = mSensorInfo[sensorID].mSensorLinePixel&0xFFFF;
    if (mSensorInfo[sensorID].mSensorPixelClock != 0)
    {
        tRS = numLine / mSensorInfo[sensorID].mSensorPixelClock;
        tRS = tRS * (float)(sensorHeight - 1);
    }else
    {
        MY_LOGW("mSensorPixelClock is 0, so can NOT get tRS");
    }

    MY_LOGD("tRS in table: %f, calculated tRS: %f", m_pNVRAM_defParameter->gis_deftRS[0], tRS);

    //Replace the tRS from the table by current sensor mode
    mSensorInfo[sensorID].mRecordParameter[0] = tRS;
    tRS += mSensorInfo[sensorID].mRecordParameter[5];

    //Check 30 fps maxmum
    if (tRS > 0.042f)
    {
        MY_LOGW("30 fps tRS+tOffset: %f should be small than 0.042 ms", tRS);
    }
    mSensorInfo[sensorID].mtRSTime = (MINT64)((MDOUBLE)tRS*1000000.0f);
    MY_LOGD("waiting gyro time: %" PRIi64 " ", mSensorInfo[sensorID].mtRSTime);
    return LMV_RETURN_NO_ERROR;
}


/*******************************************************************************
*
********************************************************************************/
MINT32 GisCalibrationImp::ConfigCalibration(const LMV_HAL_CONFIG_DATA &aLMVConfig)
{
    MY_TRACE_API_LIFE();

    if(mEisSupport == MFALSE)
    {
        MY_LOGD("mSensorIdx(%u) not support EIS",mSensorIdx);
        return LMV_RETURN_NO_ERROR;
    }

    MINT32 err = LMV_RETURN_NO_ERROR;

    if(mIsEisPlusConfig == 0)
    {
        EnableSensor();
        MY_LOGD("GIS first config for calibration (%u)", aLMVConfig.sensorType);

        GYRO_INIT_INFO_STRUCT  gyroAlgoInitData;
        GYRO_GET_PROC_INFO_STRUCT gyroGetProcData;
        GYRO_SET_WORKING_BUFFER_STRUCT gyroSetworkingbuffer;

        //====== Read NVRAM calibration data ======
        MUINT32 sensorDev;
        sensorDev = m_pHalSensorList->querySensorDevIdx(mSensorIdx);
        if ( auto pNvBufUtil = MAKE_NvBufUtil() ) {
            err = pNvBufUtil->getBufAndRead(CAMERA_NVRAM_DATA_FEATURE, sensorDev, (void*&)m_pNvram);
        }
        if (m_pNVRAM_defParameter && m_pNvram)
        {
            memcpy(m_pNVRAM_defParameter, &(m_pNvram->gis), sizeof(NVRAM_CAMERA_FEATURE_GIS_STRUCT));
        }else
        {
            MY_LOGE("m_pNVRAM_defParameter OR m_pNVRAM_defParameter is NULL\n");
            return  LMV_RETURN_NULL_OBJ;
        }
        mNVRAMRead = MTRUE;

        //> Init GIS
        ////////////////////////////////////////////////////////////////////////////////////////////////////////
        //> prepare eisPlusAlgoInitData
        memset(&gyroAlgoInitData, 0, sizeof(gyroAlgoInitData));
        MUINT64 timewithSleep = elapsedRealtime();
        MUINT64 timewithoutSleep = uptimeMillis();

        if (m_pLMVHal->GetLMVStatus())
        {
            m_pLMVHal->GetRegSetting(&gyroAlgoInitData);
            m_pLMVHal->GetLMVInputSize(&mGisInputW, &mGisInputH);
        }else
        { //Need pass from metadata!! TBD

        }

        mSensorInfo[mSensorIdx].mUsed = MTRUE; //Default sensor must be enabled.
        if(LMV_RETURN_NO_ERROR != GetSensorInfo(mSensorIdx))
        {
            MY_LOGE("GetSensorInfo fail");
            return LMV_RETURN_INVALID_DRIVER;
        }

        //====== Turn on Eis Configure One Time Flag ======
        if ( (mSensorInfo[mSensorIdx].mSensorDev == SENSOR_DEV_SUB) ||
             (mSensorInfo[mSensorIdx].mSensorDev == SENSOR_DEV_SUB_2))
        {
            mGyroData[mSensorIdx].mGyroReverse = 1;
            MY_LOGI("mSensorDev(%u), GYRO data reversed",mSensorInfo[mSensorIdx].mSensorDev);
        }else
        {
            mGyroData[mSensorIdx].mGyroReverse = 0;
            MY_LOGD("mSensorDev(%u), GYRO data normal",mSensorInfo[mSensorIdx].mSensorDev);
        }

        CalcSensorTrs(mSensorIdx, gyroAlgoInitData.param_Height);

        m_pNVRAM_defParameter->gis_defParameter1[0] = mSensorInfo[mSensorIdx].mRecordParameter[0];
        m_pNVRAM_defParameter->gis_defParameter2[0] = mSensorInfo[mSensorIdx].mRecordParameter[0];
        m_pNVRAM_defParameter->gis_defParameter3[0] = mSensorInfo[mSensorIdx].mRecordParameter[0];

        MY_LOGD("def data 0: %f    %f    %f    %f    %f    %f", m_pNVRAM_defParameter->gis_defParameter1[0], m_pNVRAM_defParameter->gis_defParameter1[1],
                                                                m_pNVRAM_defParameter->gis_defParameter1[2], m_pNVRAM_defParameter->gis_defParameter1[3],
                                                                m_pNVRAM_defParameter->gis_defParameter1[4], m_pNVRAM_defParameter->gis_defParameter1[5]);

        MY_LOGD("def data 1: %f    %f    %f    %f    %f    %f", m_pNVRAM_defParameter->gis_defParameter2[0], m_pNVRAM_defParameter->gis_defParameter2[1],
                                                                m_pNVRAM_defParameter->gis_defParameter2[2], m_pNVRAM_defParameter->gis_defParameter2[3],
                                                                m_pNVRAM_defParameter->gis_defParameter2[4], m_pNVRAM_defParameter->gis_defParameter2[5]);

        MY_LOGD("def data 2: %f    %f    %f    %f    %f    %f", m_pNVRAM_defParameter->gis_defParameter3[0], m_pNVRAM_defParameter->gis_defParameter3[1],
                                                                m_pNVRAM_defParameter->gis_defParameter3[2], m_pNVRAM_defParameter->gis_defParameter3[3],
                                                                m_pNVRAM_defParameter->gis_defParameter3[4], m_pNVRAM_defParameter->gis_defParameter3[5]);

        mSensorInfo[mSensorIdx].mRecordParameter[0] = m_pNVRAM_defParameter->gis_defParameter3[0];
        mSensorInfo[mSensorIdx].mRecordParameter[1] = m_pNVRAM_defParameter->gis_defParameter3[1];
        mSensorInfo[mSensorIdx].mRecordParameter[2] = m_pNVRAM_defParameter->gis_defParameter3[2];
        mSensorInfo[mSensorIdx].mRecordParameter[3] = m_pNVRAM_defParameter->gis_defParameter3[3];
        mSensorInfo[mSensorIdx].mRecordParameter[4] = m_pNVRAM_defParameter->gis_defParameter3[4];
        mSensorInfo[mSensorIdx].mRecordParameter[5] = m_pNVRAM_defParameter->gis_defParameter3[5];

        gyroAlgoInitData.param_Width = m_pNVRAM_defParameter->gis_defWidth;
        gyroAlgoInitData.param_Height= m_pNVRAM_defParameter->gis_defHeight;
        gyroAlgoInitData.param_crop_Y= m_pNVRAM_defParameter->gis_defCrop;

        //Deprecated. No use now for calibration
        gyroAlgoInitData.crz_crop_X = 0;
        gyroAlgoInitData.crz_crop_Y = 0;
        gyroAlgoInitData.crz_crop_Width = 0;
        gyroAlgoInitData.crz_crop_Height = 0;

        gyroAlgoInitData.ProcMode = GYRO_PROC_MODE_CAL;
        gyroAlgoInitData.param = m_pNVRAM_defParameter->gis_defParameter1;
        gyroAlgoInitData.sleep_t =  timewithSleep - timewithoutSleep;
        gyroAlgoInitData.GyroCalInfo.GYRO_sample_rate = 1000 / mEISInterval;
        MY_LOGD("sleep_t is (%" PRIu64 ")", gyroAlgoInitData.sleep_t);
        MY_LOGD("rrz crop w(%d),h(%d)", gyroAlgoInitData.rrz_crop_Width, gyroAlgoInitData.rrz_crop_Height);
        MY_LOGD("rrzOut w(%d),h(%d)", gyroAlgoInitData.rrz_scale_Width, gyroAlgoInitData.rrz_scale_Height);
        MY_LOGD("crz offset x(%d),y(%d)", gyroAlgoInitData.crz_crop_X, gyroAlgoInitData.crz_crop_Y);
        MY_LOGD("crzOut w(%d),h(%d)", gyroAlgoInitData.crz_crop_Width, gyroAlgoInitData.crz_crop_Height);
        MY_LOGD("aEisConfig  PIXEL_MODE(%d),op h(%d), opv(%d), rp h(%d), rp v(%d)", gyroAlgoInitData.GyroCalInfo.PIXEL_MODE,
                                                                        gyroAlgoInitData.GyroCalInfo.EIS_OP_H_step, gyroAlgoInitData.GyroCalInfo.EIS_OP_V_step,
                                                                        gyroAlgoInitData.GyroCalInfo.EIS_RP_H_num, gyroAlgoInitData.GyroCalInfo.EIS_RP_V_num);
        if (UNLIKELY(mEMEnabled == 1))
        {
            gyroAlgoInitData.debug = MTRUE;
            gyroAlgoInitData.EMmode = MTRUE;
            mbEMSaveFlag = MFALSE;
        }

        if (UNLIKELY(mDebugDump >= 2))
        {
            gyroAlgoInitData.debug = MTRUE;
        }

        err = m_pGisAlg->GyroInit(&gyroAlgoInitData);
        if(err != S_GYRO_OK)
        {
            MY_LOGE("GyroInit fail(0x%x)",err);
            return LMV_RETURN_API_FAIL;
        }

        err = m_pGisAlg->GyroFeatureCtrl(GYRO_FEATURE_GET_PROC_INFO, NULL, &gyroGetProcData);
        if(err != S_GYRO_OK)
        {
            MY_LOGE("get Gyro proc info fail(0x%x)",err);
            return LMV_RETURN_API_FAIL;
        }

        CreateMemBuf(gyroGetProcData.ext_mem_size, m_pGisWorkBuf);
        m_pGisWorkBuf->lockBuf("GisWorkBuf", eBUFFER_USAGE_SW_MASK);
        if (!m_pGisWorkBuf->getBufVA(0))
        {
            MY_LOGE("m_pGisWorkBuf create ImageBuffer fail");
            return LMV_RETURN_MEMORY_ERROR;
        }

        gyroSetworkingbuffer.extMemStartAddr = (MVOID *)m_pGisWorkBuf->getBufVA(0);
        gyroSetworkingbuffer.extMemSize = gyroGetProcData.ext_mem_size;

        err = m_pGisAlg->GyroFeatureCtrl(GYRO_FEATURE_SET_WORK_BUF_INFO, &gyroSetworkingbuffer, NULL);
        if(err != S_GYRO_OK)
        {

            MY_LOGE("mGisWorkBuf GYRO_FEATURE_SET_WORK_BUF_INFO fail");
            return LMV_RETURN_API_FAIL;
        }
        mIsEisPlusConfig = 1;
    }

    return LMV_RETURN_NO_ERROR;
}

MINT32 GisCalibrationImp::DoCalibration(LMV_HAL_CONFIG_DATA *apLMVConfig, MINT64 aTimeStamp, MINT64 aExpTime)
{
    MY_TRACE_API_LIFE();

    MINT32 err = LMV_RETURN_NO_ERROR;
    MUINT32 aWidth = 0,aHeight = 0;

    if (UNLIKELY(mTsForAlgoDebug == 0))
    {
        mTsForAlgoDebug = aTimeStamp;
    }

    if (mEisSupport == MFALSE)
    {
        MY_LOGD("mSensorIdx(%u) not support EIS",mSensorIdx);
        return LMV_RETURN_NO_ERROR;
    }

    //====== Check Config Data ======

    if(apLMVConfig == NULL)
    {
        MY_LOGE("apEisConfig is NULL");
        err = LMV_RETURN_NULL_OBJ;
        return err;
    }

    GYRO_INIT_INFO_STRUCT  gyroAlgoInitData;
    memset(&gyroAlgoInitData, 0, sizeof(gyroAlgoInitData));

    //Pass from P1Node maybe better
    if (m_pLMVHal->GetLMVStatus())
    {
        m_pLMVHal->GetRegSetting(&gyroAlgoInitData);
        m_pLMVHal->GetLMVInputSize(&aWidth, &aHeight);
    }else
    { //Need pass from metadata!! TBD

    }

    if ((aWidth != mGisInputW) || (aHeight != mGisInputH))
    {
        mChangedInCalibration++;
    }


    GYRO_SET_PROC_INFO_STRUCT gyroSetProcData;
    memset(&gyroSetProcData,0,sizeof(gyroSetProcData));

    gyroSetProcData.frame_t= aTimeStamp;
    gyroSetProcData.frame_AE = (MINT32)(aExpTime / 1000);

    gyroSetProcData.rrz_crop_X = gyroAlgoInitData.rrz_crop_X;
    gyroSetProcData.rrz_crop_Y = gyroAlgoInitData.rrz_crop_Y;

    gyroSetProcData.rrz_crop_Width = gyroAlgoInitData.rrz_crop_Width;
    gyroSetProcData.rrz_crop_Height = gyroAlgoInitData.rrz_crop_Height;

    gyroSetProcData.rrz_scale_Width = gyroAlgoInitData.rrz_scale_Width;
    gyroSetProcData.rrz_scale_Height = gyroAlgoInitData.rrz_scale_Height;

    //CRZ relative information is deprecated later!!!
    gyroSetProcData.crz_crop_X = 0;
    gyroSetProcData.crz_crop_Y = 0;

    gyroSetProcData.crz_crop_Width = 0;
    gyroSetProcData.crz_crop_Height = 0;

    //MY_LOGD("crz offset x(%d),y(%d)", gyroSetProcData.crz_crop_X, gyroSetProcData.crz_crop_Y);
    //MY_LOGD("crzOut w(%d),h(%d)", gyroSetProcData.crz_crop_Width, gyroSetProcData.crz_crop_Height);

    MY_LOGD("frame_AE(%d), rrz_crop_X(%d), rrz_crop_Y(%d), rrz_crop_Width(%d), rrz_crop_Height(%d), rrz_out_Width(%d), rrz_out_Height(%d)",
             gyroSetProcData.frame_AE, gyroSetProcData.rrz_crop_X, gyroSetProcData.rrz_crop_Y,
             gyroSetProcData.rrz_crop_Width, gyroSetProcData.rrz_crop_Height,
             gyroSetProcData.rrz_scale_Width, gyroSetProcData.rrz_scale_Height);

    //Get sensor data from SensorProvider
    GetSensorData(gyroSetProcData);

    GyroEISStatistics   currentEisResult;
    //EIS statistics
    for (MINT32 i=0; i<EIS_WIN_NUM; i++)
    {
        currentEisResult.eis_data[4*i + 0] = apLMVConfig->lmvData.i4LMV_X[i];
        currentEisResult.eis_data[4*i + 1] = apLMVConfig->lmvData.i4LMV_Y[i];
        currentEisResult.eis_data[4*i + 2] = apLMVConfig->lmvData.NewTrust_X[i];
        currentEisResult.eis_data[4*i + 3] = apLMVConfig->lmvData.NewTrust_Y[i];
    }

    //====== GIS Algorithm ======
    if (1)
    {
        gyroSetProcData.val_LMV = mbLastCalibration;
        mbLastCalibration = MTRUE;
        gyroSetProcData.EIS_LMV = currentEisResult.eis_data;
        err = m_pGisAlg->GyroFeatureCtrl(GYRO_FEATURE_SET_PROC_INFO,&gyroSetProcData, NULL);
        if(err != S_GYRO_OK)
        {
            MY_LOGE("GIS:GYRO_FEATURE_SET_PROC_INFO fail(0x%x)",err);
            err = LMV_RETURN_API_FAIL;
            return err;
        }

        err = m_pGisAlg->GyroMain();
        if(err != S_GYRO_OK)
        {
            MY_LOGE("GIS:GyroMain fail(0x%x)",err);
            err = LMV_RETURN_API_FAIL;
            return err;
        }
    }else
    {
        mbLastCalibration = MFALSE;
        MY_LOGD("Bypass calibration");
    }

    GYRO_CAL_RESULT_INFO_STRUCT gyroCal;
    memset(&gyroCal,0,sizeof(gyroCal));
    err = m_pGisAlg->GyroFeatureCtrl(GYRO_FEATURE_GET_CAL_RESULT_INFO, NULL,&gyroCal);
    if(err != S_GYRO_OK)
    {
        MY_LOGE("GIS:GYRO_FEATURE_SET_PROC_INFO fail(0x%x)",err);
        err = LMV_RETURN_API_FAIL;
        return err;
    }

    {
        MY_LOGD("Cal:frame(%d) valid (%d) , %f    %f    %f    %f    %f    %f", mDoEisCount, gyroCal.dataValid, gyroCal.paramFinal[0], gyroCal.paramFinal[1], gyroCal.paramFinal[2],
                                                                                                       gyroCal.paramFinal[3], gyroCal.paramFinal[4], gyroCal.paramFinal[5] );
    }

    if (mChangedInCalibration > 0)
    {
        gyroCal.dataValid = 3; //Means size is changed in the calibration
    }

    if (mChangedInCalibration == 1)
    {
        MY_LOGD("Cal:frame(%d), Initial W/H (%d/%d) , Current W/H    (%d/%d)", mDoEisCount, mGisInputW, mGisInputH, aWidth, aHeight);
    }

    if (gyroCal.dataValid == 0) //Calibration pass and write to the nvram
    {
        m_pNVRAM_defParameter->gis_defParameter3[0] = gyroCal.paramFinal[0];
        m_pNVRAM_defParameter->gis_defParameter3[1] = gyroCal.paramFinal[1];
        m_pNVRAM_defParameter->gis_defParameter3[2] = gyroCal.paramFinal[2];
        m_pNVRAM_defParameter->gis_defParameter3[3] = gyroCal.paramFinal[3];
        m_pNVRAM_defParameter->gis_defParameter3[4] = gyroCal.paramFinal[4];
        m_pNVRAM_defParameter->gis_defParameter3[5] = gyroCal.paramFinal[5];

        mSensorInfo[mSensorIdx].mRecordParameter[0] = gyroCal.paramFinal[0];
        mSensorInfo[mSensorIdx].mRecordParameter[1] = gyroCal.paramFinal[1];
        mSensorInfo[mSensorIdx].mRecordParameter[2] = gyroCal.paramFinal[2];
        mSensorInfo[mSensorIdx].mRecordParameter[3] = gyroCal.paramFinal[3];
        mSensorInfo[mSensorIdx].mRecordParameter[4] = gyroCal.paramFinal[4];
        mSensorInfo[mSensorIdx].mRecordParameter[5] = gyroCal.paramFinal[5];

    }else if ( (gyroCal.dataValid == 1) || (gyroCal.dataValid == 2) || (gyroCal.dataValid == 4)) //Calibration failed and only used in recording
    {
        mSensorInfo[mSensorIdx].mRecordParameter[0] = gyroCal.paramFinal[0];
        mSensorInfo[mSensorIdx].mRecordParameter[1] = gyroCal.paramFinal[1];
        mSensorInfo[mSensorIdx].mRecordParameter[2] = gyroCal.paramFinal[2];
        mSensorInfo[mSensorIdx].mRecordParameter[3] = gyroCal.paramFinal[3];
        mSensorInfo[mSensorIdx].mRecordParameter[4] = gyroCal.paramFinal[4];
        mSensorInfo[mSensorIdx].mRecordParameter[5] = gyroCal.paramFinal[5];
    }else
    {
        mSensorInfo[mSensorIdx].mRecordParameter[0] = m_pNVRAM_defParameter->gis_defParameter3[0];
        mSensorInfo[mSensorIdx].mRecordParameter[1] = m_pNVRAM_defParameter->gis_defParameter3[1];
        mSensorInfo[mSensorIdx].mRecordParameter[2] = m_pNVRAM_defParameter->gis_defParameter3[2];
        mSensorInfo[mSensorIdx].mRecordParameter[3] = m_pNVRAM_defParameter->gis_defParameter3[3];
        mSensorInfo[mSensorIdx].mRecordParameter[4] = m_pNVRAM_defParameter->gis_defParameter3[4];
        mSensorInfo[mSensorIdx].mRecordParameter[5] = m_pNVRAM_defParameter->gis_defParameter3[5];
    }

    if (UNLIKELY(mEMEnabled == 1))
    {
        if ((gyroCal.dataValid == 0) || (gyroCal.dataValid == 2) || (gyroCal.dataValid == 5))
        {
            if (mbEMSaveFlag == MFALSE)
            {
                err = m_pGisAlg->GyroFeatureCtrl(GYRO_FEATURE_SAVE_EM_INFO, (MVOID *)&mTsForAlgoDebug, NULL);
                if(err != S_GYRO_OK)
                {
                    MY_LOGE("GyroFeatureCtrl(GYRO_FEATURE_SAVE_EM_INFO) fail(0x%x)",err);
                }
                mbEMSaveFlag = MTRUE;
            }
        }
    }

    if (UNLIKELY(mDebugDump >= 1))
    {
        MY_LOGD("-");
    }

    return LMV_RETURN_NO_ERROR;
}

MVOID GisCalibrationImp::GetSensorData(GYRO_SET_PROC_INFO_STRUCT& gyroSetProcData)
{
    vector<SensorData> sensorData;
    memset(gyro_t_frame_array, 0, sizeof(gyro_t_frame_array));
    memset(gyro_xyz_frame_array, 0, sizeof(gyro_xyz_frame_array));

    {
        MY_TRACE_TAG_LIFE("SensorProvider::getAllSensorData");
        mpSensorProvider->getAllSensorData(SENSOR_TYPE_GYRO, sensorData);
    }

    mGyroData[mSensorIdx].mGyroQueueLock.lock();

    for (MUINT32 i = 0; i < sensorData.size(); i++)
    {
        if (gyroSetProcData.gyro_num >= GYRO_DATA_PER_FRAME)
        {
            gyroSetProcData.gyro_num = 0;
        }

        //place with ascending timestamp
        int index = (sensorData.front().timestamp < sensorData.back().timestamp) ?
            i :
            sensorData.size() - (i + 1);

        if ( LIKELY((sensorData[index].gyro[0] < UNREASONABLE_GYRO_VALUE) && (sensorData[index].gyro[0] > -UNREASONABLE_GYRO_VALUE) &&
             (sensorData[index].gyro[1] < UNREASONABLE_GYRO_VALUE) && (sensorData[index].gyro[1] > -UNREASONABLE_GYRO_VALUE) &&
             (sensorData[index].gyro[2] < UNREASONABLE_GYRO_VALUE) && (sensorData[index].gyro[2] > -UNREASONABLE_GYRO_VALUE)))
        {
            gyro_t_frame_array[gyroSetProcData.gyro_num] = sensorData[index].timestamp;
            gyro_xyz_frame_array[3*(gyroSetProcData.gyro_num) + 0] = sensorData[index].gyro[0];
            if (UNLIKELY(GisCalibrationImp::mGyroData[mSensorIdx].mGyroReverse == 1))
            {
                gyro_xyz_frame_array[3*(gyroSetProcData.gyro_num) + 1] = -sensorData[index].gyro[1];
                gyro_xyz_frame_array[3*(gyroSetProcData.gyro_num) + 2] = -sensorData[index].gyro[2];
            }
            else
            {
                gyro_xyz_frame_array[3*(gyroSetProcData.gyro_num) + 1] = sensorData[index].gyro[1];
                gyro_xyz_frame_array[3*(gyroSetProcData.gyro_num) + 2] = sensorData[index].gyro[2];
            }

            if (UNLIKELY(mDebugDump >= 1))
            {
                MY_LOGD("Gyro(%f,%f,%f,%" PRIi64 ")",sensorData[index].gyro[0], sensorData[index].gyro[1],
                    sensorData[index].gyro[2], sensorData[index].timestamp);
            }
            gyroSetProcData.gyro_num++;

#if     RECORD_WITHOUT_EIS_ENABLE
            if (g_AIMDump == 1)
            {
                gGyroRecord[gGyroRecordWriteID].id = EIS_GYROSCOPE;
                gGyroRecord[gGyroRecordWriteID].x = sensorData[index].gyro[0];
                if (UNLIKELY(GisCalibrationImp::mGyroData[mSensorIdx].mGyroReverse == 1))
                {
                    gGyroRecord[gGyroRecordWriteID].y = -sensorData[index].gyro[1];
                    gGyroRecord[gGyroRecordWriteID].z = -sensorData[index].gyro[2];
                }else
                {
                    gGyroRecord[gGyroRecordWriteID].y = sensorData[index].gyro[1];
                    gGyroRecord[gGyroRecordWriteID].z = sensorData[index].gyro[2];
                }
                gGyroRecord[gGyroRecordWriteID].ts = sensorData[index].timestamp;
                gGyroRecordWriteID++;
                if (gGyroRecordWriteID >= TSRECORD_MAXSIZE)
                    gGyroRecordWriteID = 0;
                if (UNLIKELY(GisCalibrationImp::mDebugDump >= 2))
                {
                    MY_LOGD("[%d] Gyro(%f,%f,%f,%" PRIi64 ")",
                                gGyroRecordWriteID,
                                sensorData[index].gyro[0],
                                sensorData[index].gyro[1],
                                sensorData[index].gyro[2],
                                sensorData[index].timestamp);
                }
            }
#endif
        }
        else
        {
            MY_LOGE("Unreasonable gyro data(%f,%f,%f,%" PRIi64 ")", sensorData[index].gyro[0], sensorData[index].gyro[1],
                sensorData[index].gyro[2], sensorData[index].timestamp);
            GisCalibrationImp::mGyroData[mSensorIdx].mLastGyroTimestamp = sensorData[index].timestamp;
        }
    }

    mGyroData[mSensorIdx].mGyroQueueLock.unlock();

    if (UNLIKELY(mDebugDump >= 1))
    {
        MY_LOGD("Gyro data num: %d, video ts: %" PRIi64 " ", gyroSetProcData.gyro_num, gyroSetProcData.frame_t);
    }

    gyroSetProcData.gyro_t_frame = gyro_t_frame_array;
    gyroSetProcData.gyro_xyz_frame = gyro_xyz_frame_array;
}


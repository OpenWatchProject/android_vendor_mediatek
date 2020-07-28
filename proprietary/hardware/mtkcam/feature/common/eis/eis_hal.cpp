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
#include <sys/stat.h>

#include <mtkcam/utils/imgbuf/IIonImageBufferHeap.h>
#include <mtkcam/utils/std/common.h>
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/drv/iopipe/CamIO/IHalCamIO.h>

using namespace std;
using namespace android;
using namespace NSCam;
using namespace NSCam::Utils;
using namespace NSCam::NSIoPipe;
using namespace NSCam::NSIoPipe::NSCamIOPipe;

#include "eis_hal_imp.h"
#include "eis_macro.h"

#include <mtkcam/feature/eis/lmv_hal.h>
#include <mtkcam/utils/sys/SensorProvider.h>
#include <mtkcam/aaa/INvBufUtil.h>
#include <camera_custom_nvram.h>
#include <camera_custom_eis.h>

using namespace NSCam::Utils;
using namespace NSCam::EIS;
#if 1
#undef  ATRACE_TAG
#define ATRACE_TAG ATRACE_TAG_ALWAYS
#define DP_TRACE_CALL()                 ATRACE_CALL()
#define DP_TRACE_BEGIN(name)            ATRACE_BEGIN(name)
#define DP_TRACE_END()                  ATRACE_END()

#else

#define DP_TRACE_CALL()
#define DP_TRACE_BEGIN(name)
#define DP_TRACE_END()

#endif  // CONFIG_FOR_SYSTRACE


/*******************************************************************************
*
********************************************************************************/
#define EIS_HAL_DEBUG

#ifdef EIS_HAL_DEBUG

#undef __func__
#define __func__ __FUNCTION__

#undef  LOG_TAG
#define LOG_TAG "EisHal"
#include <mtkcam/utils/std/Log.h>

#define EIS_LOG(fmt, arg...)    CAM_LOGD("[%s]" fmt, __func__, ##arg)
#define EIS_INF(fmt, arg...)    CAM_LOGI("[%s]" fmt, __func__, ##arg)
#define EIS_WRN(fmt, arg...)    CAM_LOGW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define EIS_ERR(fmt, arg...)    CAM_LOGE("[%s] %s ERROR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)

#else
#define EIS_LOG(a,...)
#define EIS_INF(a,...)
#define EIS_WRN(a,...)
#define EIS_ERR(a,...)
#endif

#define EIS_HAL_NAME "EisHal"
#define intPartShift 8
#define floatPartShift (31 - intPartShift)
#define DEBUG_DUMP_FRAMW_NUM 10
queue<EIS_GyroRecord> emptyQueue;

MAKE_EISHAL_OBJ(0);
MAKE_EISHAL_OBJ(1);
MAKE_EISHAL_OBJ(2);
MAKE_EISHAL_OBJ(3);


MINT32 EisHalImp::mEMEnabled = 0;
MINT32 EisHalImp::mDebugDump = 0;
MINT32 EisHalImp::mDisableGyroData = 0;

#define EISO_BUFFER_NUM (30)

/*******************************************************************************
* GYRO Data
********************************************************************************/
static MFLOAT gAccInfo[3] = {0};
static MFLOAT gGyroInfo[3] = {0};


typedef struct EIS_TSRecord_t
{
    MUINT64 id;
    MUINT64 ts;
}EIS_TSRecord;


#define UNREASONABLE_GYRO_VALUE (10.0f)

/*******************************************************************************
* Debug property
********************************************************************************/
#define RECORD_WITHOUT_EIS_ENABLE   (0) //Only used in AIM debug, Must be disable in normal case
#if RECORD_WITHOUT_EIS_ENABLE
static EIS_GyroRecord gGyroRecord[TSRECORD_MAXSIZE];
static EIS_TSRecord gTSRecord[TSRECORD_MAXSIZE];
static EIS_TSRecord gvHDRRecord[TSRECORD_MAXSIZE];
static MUINT32 gvHDRRecordWriteID = 0;
static MUINT32 gTSRecordWriteID = 0;
static MUINT32 gGyroRecordWriteID = 0;
static MINT32 g_AIMDump = 0;
static MINT32 g_GyroOnly = 0;
static MINT32 g_ImageOnly = 0;
static MINT32 g_forecDejello = 1;
#endif
#define FSC_DUMP_FOLDER_PATH "storage/sdcard0/fsc"

/*******************************************************************************
*
********************************************************************************/
static MUINT32 DumpToFile(
    char *filename,
    unsigned char *buffer,
    unsigned int size
)
{
    uint32_t writeCount = 0;
    int fd = ::open(filename, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
    if( fd < 0 )
    {
        EIS_ERR("Cannot create file [%s]", filename);
    }
    else
    {
        for( int cnt = 0, nw = 0; writeCount < size; ++cnt )
        {
            nw = ::write(fd, buffer + writeCount, size - writeCount);
            if( nw < 0 )
            {
                EIS_ERR("Cannot write to file [%s] buffer(%p) size(%d) writeCount(%d) nw(%d)", filename, buffer, size, writeCount, size);
                break;
            }
            writeCount += nw;
        }
        if( ::close(fd)<0 )
        {
            EIS_ERR("Close file failed [%s]", filename);
        }
    }
    return writeCount;
}

/*******************************************************************************
*
********************************************************************************/

MVOID EisHalImp::GetSensorData()
{
    MBOOL bReversed = MFALSE;
    vector<SensorData> sensorData;
    mpSensorProvider->getAllSensorData(SENSOR_TYPE_GYRO, sensorData);

    EISHAL_SwitchIsGyroReverseNotZero(mSensorIdx, bReversed);

    for (MUINT32 i = 0; i < sensorData.size(); i++)
    {
        //place with ascending timestamp
        int index = (sensorData.front().timestamp < sensorData.back().timestamp) ?
            i :
            sensorData.size() - (i + 1);

        if ( LIKELY((sensorData[index].gyro[0] < UNREASONABLE_GYRO_VALUE) && (sensorData[index].gyro[0] > -UNREASONABLE_GYRO_VALUE) &&
             (sensorData[index].gyro[1] < UNREASONABLE_GYRO_VALUE) && (sensorData[index].gyro[1] > -UNREASONABLE_GYRO_VALUE) &&
             (sensorData[index].gyro[2] < UNREASONABLE_GYRO_VALUE) && (sensorData[index].gyro[2] > -UNREASONABLE_GYRO_VALUE)))
        {
            EIS_GyroRecord tmp;
            tmp.x = sensorData[index].gyro[0];
            if (UNLIKELY(bReversed))
            {
                tmp.y = -sensorData[index].gyro[1];
                tmp.z = -sensorData[index].gyro[2];
            }
            else
            {
                tmp.y = sensorData[index].gyro[1];
                tmp.z = sensorData[index].gyro[2];
            }
            tmp.ts = sensorData[index].timestamp;
            EISHAL_SwitchPushGyroQueue(mSensorIdx, tmp);
            DP_TRACE_CALL();
#if     RECORD_WITHOUT_EIS_ENABLE
            if (g_AIMDump == 1)
            {
                gGyroRecord[gGyroRecordWriteID].id = EIS_GYROSCOPE;
                gGyroRecord[gGyroRecordWriteID].x = sensorData[index].gyro[0];
                if (UNLIKELY(bReversed))
                {
                    gGyroRecord[gGyroRecordWriteID].y = -sensorData[index].gyro[1];
                    gGyroRecord[gGyroRecordWriteID].z = -sensorData[index].gyro[2];
                }
                else
                {
                    gGyroRecord[gGyroRecordWriteID].y = sensorData[index].gyro[1];
                    gGyroRecord[gGyroRecordWriteID].z = sensorData[index].gyro[2];
                }
                gGyroRecord[gGyroRecordWriteID].ts = sensorData[index].timestamp;
                gGyroRecordWriteID++;
                if (gGyroRecordWriteID >= TSRECORD_MAXSIZE)
                        gGyroRecordWriteID = 0;

                if (UNLIKELY(EisHalImp::mDebugDump >= 2))
                {
                    EIS_LOG("[%d] Gyro(%f,%f,%f,%" PRIi64 ")",
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
            EIS_ERR("Unreasonable gyro data(%f,%f,%f,(%" PRIi64 ")", sensorData[index].gyro[0], sensorData[index].gyro[1],
                sensorData[index].gyro[2], sensorData[index].timestamp);
            EISHAL_SwitchSetLastGyroTimestampValue(mSensorIdx, sensorData[index].timestamp);
            EISHAL_SWitchWaitGyroCond_Signal(mSensorIdx);
        }
    }
}

/*******************************************************************************
*
********************************************************************************/
EisHal *EisHal::CreateInstance(char const *userName, const EisInfo& eisInfo, const MUINT32 &aSensorIdx,
    MUINT32 MultiSensor)
{
    EIS_LOG("%s",userName);
    return EisHalImp::GetInstance(eisInfo, aSensorIdx, MultiSensor);
}

/*******************************************************************************
*
********************************************************************************/
EisHal *EisHalImp::GetInstance(const EisInfo& eisInfo, const MUINT32 &aSensorIdx, MUINT32 MultiSensor)
{
    EIS_LOG("sensorIdx(%u)",aSensorIdx);

    switch(aSensorIdx)
    {
        case 0 : return EisHalObj<0>::GetInstance(eisInfo, MultiSensor);
        case 1 : return EisHalObj<1>::GetInstance(eisInfo, MultiSensor);
        case 2 : return EisHalObj<2>::GetInstance(eisInfo, MultiSensor);
        case 3 : return EisHalObj<3>::GetInstance(eisInfo, MultiSensor);
        default :
            EIS_WRN("Current limit is 4 sensors, use 0");
            return EisHalObj<0>::GetInstance(eisInfo, MultiSensor);
    }
}

/*******************************************************************************
*
********************************************************************************/
MVOID EisHalImp::DestroyInstance(char const *userName)
{
    Mutex::Autolock lock(mLock);
    EIS_LOG("%s",userName);

    //====== Check Reference Count ======
    if(mUsers <= 0)
    {
        EIS_LOG("mSensorIdx(%u) has 0 user",mSensorIdx);
        MUINT32 sensorIndex = mSensorIdx;

        switch(mSensorIdx)
        {
             case 0 :
                EisHalObj<0>::destroyInstance();
                break;
             case 1 :
                EisHalObj<1>::destroyInstance();
                break;
             case 2 :
                EisHalObj<2>::destroyInstance();
                break;
             case 3 :
                EisHalObj<3>::destroyInstance();
                break;
             default :
                 EIS_WRN(" %d is not exist", mSensorIdx);
        }
        EIS_LOG("EISHal mSensorIdx(%u) is destroyed", sensorIndex);
    }
}

/*******************************************************************************
*
********************************************************************************/
EisHalImp::EisHalImp(const EisInfo& eisInfo, const MUINT32 &aSensorIdx, MUINT32 MultiSensor)
    : EisHal()
    , mEisInfo(eisInfo)
    , mSensorIdx(aSensorIdx)
    , mMultiSensor(MultiSensor)
{
    mUsers = 0;

    //> member variable
    mSrzOutW = 0;
    mSrzOutH = 0;
    mGpuGridW = 0;
    mGpuGridH = 0;
    mMVWidth = 0;
    mMVHeight = 0;
    mDoEisCount = 0;    //Vent@20140427: Add for EIS GMV Sync Check.
    mFrameCnt = 0;
    mIsEisConfig = 0;
    mIsEisPlusConfig = 0;
    mEisPlusCropRatio = 0;
    mCurSensorIdx = 0;
    mEisSupport = MTRUE;
    mEisMode = 0;
    mEISInterval = 0;
    mGyroEnable = MFALSE;
    mAccEnable  = MFALSE;
    mEisPlusResult.GridX = 0;
    mEisPlusResult.GridY = 0;
    mEisPlusResult.ClipX = 0;
    mEisPlusResult.ClipY = 0;

    mTsForAlgoDebug = 0;
    m_pNvram = NULL;
    mGisInputW = 0;
    mGisInputH = 0;
    mSkipWaitGyro = MFALSE;
    mbEMSaveFlag = MFALSE;

    mEMEnabled = 0;
    mDebugDump = 0;
    mBitTrueDump = 0;
    mDisableGyroData = 0;
    m_pLMVHal = NULL;
#if EIS_ALGO_READY
    m_pEisPlusAlg = NULL;
    m_pEisAlg = NULL;
    m_pGisAlg = NULL;

    memset(&mEisAlgoProcData, 0, sizeof(mEisAlgoProcData));
#endif

    mChangedInCalibration = 0;
    mNVRAMRead = MFALSE;
    mSleepTime = 0;
    mbLastCalibration = MTRUE;
    m_pNVRAM_defParameter = NULL;
    m_pEisDbgBuf = NULL;
    m_pEisPlusWorkBuf = NULL;
    m_pGisWorkBuf = NULL;

    // SensorProvider
    mpSensorProvider = NULL;

    // Sensor
    m_pHalSensorList = NULL;
    m_pHalSensor = NULL;


}

/*******************************************************************************
*
********************************************************************************/
MINT32 EisHalImp::Init()
{
    DP_TRACE_CALL();

    //====== Check Reference Count ======
    Mutex::Autolock lock(mLock);
    MUINT32 index;
    if(mUsers > 0)
    {
        android_atomic_inc(&mUsers);
        EIS_LOG("snesorIdx(%u) has %d users",mSensorIdx,mUsers);
        return EIS_RETURN_NO_ERROR;
    }

    MINT32 err = EIS_RETURN_NO_ERROR;

    //====== Dynamic Debug ======
    mEISInterval = EISCustom::getGyroIntervalMs();
    mEMEnabled = 0;
    mDebugDump = 0;
    mDisableGyroData = 0;

    mDebugDump = property_get_int32("vendor.debug.eis.dump", mDebugDump);
    mEMEnabled = property_get_int32("vendor.debug.eis.EMEnabled", mEMEnabled);
    mEISInterval = property_get_int32("vendor.eisrecord.setinterval", mEISInterval);
    mDisableGyroData = property_get_int32("vendor.debug.eis.disableGyroData", mDisableGyroData);
    mBitTrueDump = property_get_int32("vendor.debug.eis.bittruedump", mBitTrueDump);

#if RECORD_WITHOUT_EIS_ENABLE
    g_AIMDump = 0;
    g_GyroOnly = 0;
    g_ImageOnly = 0;
    g_forecDejello = 1;

    g_AIMDump = property_get_int32("vendor.debug.eis.AIMDump", g_AIMDump);
    g_GyroOnly = property_get_int32("vendor.debug.eis.GyroOnly", g_GyroOnly);
    g_ImageOnly = property_get_int32("vendor.debug.eis.ImageOnly", g_ImageOnly);
    g_forecDejello = property_get_int32("vendor.debug.eis.forceDejello", g_forecDejello);
#endif
    if (mBitTrueDump)
    {
        mkdir(FSC_DUMP_FOLDER_PATH,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        mkdir("storage/sdcard0/bittrue",S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }

    EISHAL_SwitchClearGyroQueue(mSensorIdx);
    EISHAL_SwitchSetGyroCountZero(mSensorIdx);

    mCurSensorIdx = mSensorIdx;
    m_pLMVHal = NULL;
    //Init GIS member data
    m_pNvram = NULL;
    mChangedInCalibration = 0;
    mNVRAMRead = MFALSE;
    mSleepTime = 0;

    EISHAL_SwitchSetGyroReverseValue(mSensorIdx, 0);
    EISHAL_SwitchSetLastGyroTimestampValue(mSensorIdx, 0);
    mbLastCalibration = MTRUE;
    mbEMSaveFlag = MFALSE;
    m_pNVRAM_defParameter = NULL;
    m_pNVRAM_defParameter = new NVRAM_CAMERA_FEATURE_GIS_STRUCT;
    memset(m_pNVRAM_defParameter, 0, sizeof(NVRAM_CAMERA_FEATURE_GIS_STRUCT));
    memset(mSensorInfo, 0, sizeof(mSensorInfo));

#if RECORD_WITHOUT_EIS_ENABLE
    gTSRecordWriteID = 0;
    gGyroRecordWriteID = 0;
    gvHDRRecordWriteID = 0;
    memset(&gTSRecord, 0, sizeof(gTSRecord));
    memset(&gvHDRRecord, 0, sizeof(gvHDRRecord));
    memset(&gGyroRecord, 0, sizeof(gTSRecord));
#endif

    EIS_LOG("(%p)  mSensorIdx(%u) init", this, mSensorIdx);

    //====== Create Sensor Object ======

    m_pHalSensorList = MAKE_HalSensorList();
    if(m_pHalSensorList == NULL)
    {
        EIS_ERR("IHalSensorList::get fail");
        goto create_fail_exit;
    }

    //====== Create EIS Algorithm Object ======

#if EIS_ALGO_READY
    m_pEisPlusAlg = MTKEisPlus::createInstance();

    if(m_pEisPlusAlg == NULL)
    {
        EIS_ERR("MTKEisPlus::createInstance fail");
        goto create_fail_exit;
    }

    m_pGisAlg = MTKGyro::createInstance();

    if (m_pGisAlg == NULL)
    {
        EIS_ERR("MTKGyro::createInstance fail");
        goto create_fail_exit;
    }

#endif

    //====== Create Sensor Provider Object ======
    mpSensorProvider = NULL;

    //====== Increase User Count ======

    android_atomic_inc(&mUsers);

    EIS_LOG("-");
    return EIS_RETURN_NO_ERROR;

create_fail_exit:

    if (m_pHalSensorList != NULL)
    {
        m_pHalSensorList = NULL;
    }

#if EIS_ALGO_READY
    if (m_pEisPlusAlg != NULL)
    {
        m_pEisPlusAlg->EisPlusReset();
        m_pEisPlusAlg->destroyInstance(m_pEisPlusAlg);
        m_pEisPlusAlg = NULL;
    }

#endif

    EIS_LOG("-");
    return EIS_RETURN_NULL_OBJ;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 EisHalImp::Uninit()
{
    Mutex::Autolock lock(mLock);

    //====== Check Reference Count ======

    if(mUsers <= 0)
    {
        EIS_LOG("mSensorIdx(%u) has 0 user",mSensorIdx);
        return EIS_RETURN_NO_ERROR;
    }

    //====== Uninitialize ======

    android_atomic_dec(&mUsers);    //decrease referebce count

    if(mUsers == 0)    // there is no user
    {
        MINT32 err = EIS_RETURN_NO_ERROR;

        EIS_LOG("mSensorIdx(%u) uninit", mSensorIdx);

        //======  Release EIS Algo Object ======

#if EIS_ALGO_READY

        if (UNLIKELY(mDebugDump >= 2))
        {
            EIS_LOG("mIsEisPlusConfig(%d)",mIsEisPlusConfig);
            if(mIsEisPlusConfig == 1)
            {
                err = m_pEisPlusAlg->EisPlusFeatureCtrl(EIS_PLUS_FEATURE_SAVE_LOG,(MVOID *)&mTsForAlgoDebug,NULL);
                if(err != S_EIS_PLUS_OK)
                {
                    EIS_ERR("EisPlusFeatureCtrl(EIS_PLUS_FEATURE_SAVE_LOG) fail(0x%x)",err);
                }
            }
        }

#if !RECORD_WITHOUT_EIS_ENABLE
        if (UNLIKELY((mDebugDump >= 2) || (mEMEnabled == 1)))
#endif
        {
            EIS_LOG("mIsEisPlusConfig(%d)",mIsEisPlusConfig);
            if(mIsEisPlusConfig == 1)
            {
                err = m_pGisAlg->GyroFeatureCtrl(GYRO_FEATURE_SAVE_LOG, (MVOID *)&mTsForAlgoDebug, NULL);
                if(err != S_GYRO_OK)
                {
                    EIS_ERR("GyroFeatureCtrl(GYRO_FEATURE_SAVE_LOG) fail(0x%x)",err);
                }
            }
        }

#if RECORD_WITHOUT_EIS_ENABLE
        if (g_AIMDump == 1)
        {
            if (gTSRecordWriteID != 0)
            {
                MUINT32 LogCount,WriteCnt;
                char LogFileName[100];
                FILE * pLogFp ;

                LogCount = (mTsForAlgoDebug/(MUINT64)1000000000LL);

                EIS_LOG("RecordWriteID, mTsForAlgoDebug (%" PRIi64 "), LogCount(%d)",  mTsForAlgoDebug, LogCount);
                mkdir("storage/sdcard0/gyro",S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
                sprintf(LogFileName, "storage/sdcard0/gyro/EIS_Gyro_LOG_%d.bin", LogCount);
                pLogFp = fopen(LogFileName, "wb");
                if (NULL == pLogFp )
                {
                    EIS_ERR("Can't open file to save EIS HAL Log\n");
                }else
                {
                    WriteCnt = fwrite((void*)(&gGyroRecordWriteID),sizeof(gGyroRecordWriteID),1,pLogFp);
                    WriteCnt = fwrite(gGyroRecord,sizeof(gGyroRecord),1,pLogFp);
                    fflush(pLogFp);
                    fclose(pLogFp);
                }
                EIS_LOG("%d, gGyroRecordWriteID(%d)", LogCount, gGyroRecordWriteID);

                sprintf(LogFileName, "storage/sdcard0/gyro/EIS_TS_LOG_%d.bin", LogCount);
                pLogFp = fopen(LogFileName, "wb");
                if (NULL == pLogFp )
                {
                    EIS_ERR("Can't open file to save EIS HAL Log\n");
                }else
                {
                    MUINT64 timewithSleep = elapsedRealtime();
                    MUINT64 timewithoutSleep = uptimeMillis();
                    MUINT64 Diff =  timewithSleep - timewithoutSleep;
                    WriteCnt = fwrite((void*)(&gTSRecordWriteID),sizeof(gTSRecordWriteID),1,pLogFp);
                    WriteCnt = fwrite((void*)(&Diff),sizeof(Diff),1,pLogFp);
                    WriteCnt = fwrite(gTSRecord,sizeof(gTSRecord),1,pLogFp);
                    fflush(pLogFp);
                    fclose(pLogFp);
                }
                EIS_LOG("%d, gTSRecordWriteID(%d)", LogCount, gTSRecordWriteID);
#if 0
                sprintf(LogFileName, "storage/sdcard0/gyro/EIS_vHDR_LOG_%d.bin", LogCount);
                pLogFp = fopen(LogFileName, "wb");
                if (NULL == pLogFp )
                {
                    EIS_ERR("Can't open file to save EIS HAL Log\n");
                }else
                {
                    MUINT32 IsFirstLE;
                    IsFirstLE = mSensorStaticInfo.iHDR_First_IS_LE;
                    WriteCnt = fwrite((void*)(&gvHDRRecordWriteID),sizeof(gvHDRRecordWriteID),1,pLogFp);
                    WriteCnt = fwrite((void*)(&IsFirstLE),sizeof(IsFirstLE),1,pLogFp);
                    WriteCnt = fwrite(gvHDRRecord,sizeof(gvHDRRecord),1,pLogFp);
                    fflush(pLogFp);
                    fclose(pLogFp);
                }
                EIS_LOG("%d, gvHDRRecordWriteID(%d)", LogCount, gvHDRRecordWriteID);
#endif
            }
            gTSRecordWriteID = 0;
            gGyroRecordWriteID = 0;
            gvHDRRecordWriteID = 0;
            memset(&gTSRecord, 0, sizeof(gTSRecord));
            memset(&gvHDRRecord, 0, sizeof(gvHDRRecord));
            memset(&gGyroRecord, 0, sizeof(gGyroRecord));
        }
#endif
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
            EIS_LOG("m_pGisAlg uninit");
            m_pGisAlg->GyroReset();
            m_pGisAlg->destroyInstance(m_pGisAlg);
            m_pGisAlg = NULL;
        }

        if (m_pEisPlusAlg != NULL)
        {
            EIS_LOG("m_pEisPlusAlg uninit");
            m_pEisPlusAlg->EisPlusReset();
            m_pEisPlusAlg->destroyInstance(m_pEisPlusAlg);
            m_pEisPlusAlg = NULL;
        }
#endif

        // Next-Gen EIS
        if (mpSensorProvider != NULL)
        {
            EIS_LOG("mpSensorProvider uninit");
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
        //>  free EIS Plus working buffer
        if (m_pEisPlusWorkBuf != NULL)
        {
            m_pEisPlusWorkBuf->unlockBuf("EISPlusWorkBuf");
            DestroyMemBuf(m_pEisPlusWorkBuf);
        }

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

        mSrzOutW = 0;
        mSrzOutH = 0;
        mFrameCnt = 0; // first frmae
        mIsEisConfig = 0;
        mIsEisPlusConfig = 0;
        mDoEisCount = 0;    //Vent@20140427: Add for EIS GMV Sync Check.
        mGpuGridW = 0;
        mGpuGridH = 0;
        mMVWidth = 0;
        mMVHeight = 0;
        mGyroEnable = MFALSE;
        mAccEnable  = MFALSE;

        mChangedInCalibration = 0;
        mGisInputW = 0;
        mGisInputH = 0;
        mNVRAMRead = MFALSE;
        m_pNvram = NULL;
        mSleepTime = 0;
        mbLastCalibration = MTRUE;
        EISHAL_SwitchSetGyroCountZero(mSensorIdx);
        EISHAL_SwitchSetLastGyroTimestampValue(mSensorIdx, 0);

        mbEMSaveFlag = MFALSE;

        if (NULL != m_pNVRAM_defParameter)
        {
            delete m_pNVRAM_defParameter;
            m_pNVRAM_defParameter = NULL;
        }

        mEISInterval = 0;
        mEMEnabled = 0;
        mDebugDump = 0;
        mDisableGyroData = 0;
    }
    else
    {
        EIS_LOG("mSensorIdx(%u) has %d users",mSensorIdx,mUsers);
    }

    EISHAL_SwitchClearGyroQueue(mSensorIdx);

    return EIS_RETURN_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 EisHalImp::CreateMemBuf(MUINT32 memSize, android::sp<IImageBuffer>& spImageBuf)
{
    MINT32 err = EIS_RETURN_NO_ERROR;
    IImageBufferAllocator* pImageBufferAlloc = IImageBufferAllocator::getInstance();

    EIS_LOG("Size(%u)",memSize);
    IImageBufferAllocator::ImgParam bufParam((size_t)memSize, 0);
    spImageBuf = pImageBufferAlloc->alloc("EIS_HAL", bufParam);
    EIS_LOG("X");
    return err;
}

/******************************************************************************
*
*******************************************************************************/
MINT32 EisHalImp::DestroyMemBuf(android::sp<IImageBuffer>& spImageBuf)
{
    MINT32 err = EIS_RETURN_NO_ERROR;
    IImageBufferAllocator* pImageBufferAlloc = IImageBufferAllocator::getInstance();
    EIS_LOG("DestroyMemBuf");


    if (spImageBuf != NULL)
    {
        pImageBufferAlloc->free(spImageBuf.get());
        spImageBuf = NULL;
    }

    EIS_LOG("X");
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 EisHalImp::GetSensorInfo(MUINT32 sensorID)
{
    MINT32 err = EIS_RETURN_NO_ERROR;

    mSensorInfo[sensorID].mSensorDev = m_pHalSensorList->querySensorDevIdx(sensorID);
    EIS_LOG("[mSensorInfo] sensorIdx(%u), mSensorDev(%u)", sensorID, mSensorInfo[sensorID].mSensorDev);

    m_pHalSensor = m_pHalSensorList->createSensor(EIS_HAL_NAME,1,&sensorID);

    if (LIKELY(m_pHalSensor != NULL))
    {
        err = m_pHalSensor->sendCommand( mSensorInfo[sensorID].mSensorDev,SENSOR_CMD_GET_PIXEL_CLOCK_FREQ,
                                         (MUINTPTR)&mSensorInfo[sensorID].mSensorPixelClock, 0, 0);
        if (err != EIS_RETURN_NO_ERROR)
        {
            EIS_ERR("SENSOR_CMD_GET_PIXEL_CLOCK_FREQ is fail(0x%x)",err);
            return EIS_RETURN_API_FAIL;
        }

        err = m_pHalSensor->sendCommand( mSensorInfo[sensorID].mSensorDev,SENSOR_CMD_GET_FRAME_SYNC_PIXEL_LINE_NUM,
                                         (MUINTPTR)&mSensorInfo[sensorID].mSensorLinePixel, 0, 0);
        if (err != EIS_RETURN_NO_ERROR)
        {
            EIS_ERR("SENSOR_CMD_GET_PIXEL_CLOCK_FREQ is fail(0x%x)",err);
            return EIS_RETURN_API_FAIL;
        }

        m_pHalSensor->destroyInstance(EIS_HAL_NAME);
        m_pHalSensor = NULL;

    }else
    {
        EIS_ERR("m_pHalSensorList->createSensor fail, m_pHalSensor == NULL");
    }

    EIS_LOG("mSensorDev(%u), pixelclock (%d), pixelline(%x)", mSensorInfo[sensorID].mSensorDev,
                                                              mSensorInfo[sensorID].mSensorPixelClock,
                                                              mSensorInfo[sensorID].mSensorLinePixel);

    return EIS_RETURN_NO_ERROR;
}


MINT32 EisHalImp::EnableSensor()
{
    DP_TRACE_BEGIN("SensorProvider");
    mpSensorProvider = SensorProvider::createInstance(EIS_HAL_NAME);
    DP_TRACE_END();

    // No use for accelerameter
    //mAccEnable  = mpSensorProvider->enableSensor(SensorType_Acc,mEISInterval);
    mAccEnable  = MFALSE;
    mGyroEnable = mpSensorProvider->enableSensor(SENSOR_TYPE_GYRO, mEISInterval);
    EIS_LOG("EN:(Acc,Gyro)=(%d,%d)",mAccEnable,mGyroEnable);


    if (m_pLMVHal == NULL)
    {
        m_pLMVHal = LMVHal::CreateInstance(LOG_TAG, mSensorIdx);
    }

    return EIS_RETURN_NO_ERROR;
}

MINT32 EisHalImp::CalcSensorTrs(MUINT32 sensorID, MUINT32 sensorHeight)
{
    MDOUBLE tRS = 0.0f,numLine;
    numLine = mSensorInfo[sensorID].mSensorLinePixel&0xFFFF;
    if (mSensorInfo[sensorID].mSensorPixelClock != 0)
    {
        tRS = numLine / mSensorInfo[sensorID].mSensorPixelClock;
        tRS = tRS * (float)(sensorHeight - 1);
    }else
    {
        EIS_WRN("mSensorPixelClock is 0, so can NOT get tRS");
    }

    EIS_LOG("tRS in table: %f, calculated tRS: %f", m_pNVRAM_defParameter->gis_deftRS[0], tRS);

    //Replace the tRS from the table by current sensor mode
    mSensorInfo[sensorID].mRecordParameter[0] = tRS;
    tRS += mSensorInfo[sensorID].mRecordParameter[5];

    //Check 30 fps maxmum
    if (tRS > 0.042f)
    {
        EIS_WRN("30 fps tRS+tOffset: %f should be small than 0.042 ms", tRS);
    }
    mSensorInfo[sensorID].mtRSTime = (MINT64)((MDOUBLE)tRS*1000000.0f);
    EIS_LOG("waiting gyro time: %" PRIi64 " ", mSensorInfo[sensorID].mtRSTime);
    return EIS_RETURN_NO_ERROR;
}

MVOID EisHalImp::AddMultiSensors(MUINT32 sensorID)
{
    if( sensorID >= 0 && sensorID < MAX_SENSOR_ID )
    {
        if( mSensorInfo[sensorID].mUsed == MTRUE )
        {
            EIS_WRN("mSensorInfo[%d] is used",sensorID);
        }
        else
        {
            mSensorInfo[sensorID].mUsed = MTRUE;
            if( UNLIKELY( mDebugDump > 0 ) )
            {
                EIS_LOG("Add sensor=%d", sensorID);
            }
            if( EIS_RETURN_NO_ERROR != GetSensorInfo(sensorID) )
            {
                EIS_ERR("GetSensorInfo fail");
            }
        }
    }
}

MVOID EisHalImp::configMVNumber(const EIS_HAL_CONFIG_DATA &apEisConfig, MINT32 *mvWidth, MINT32 *mvHeight)
{
    if (apEisConfig.srzOutW == 0 || apEisConfig.srzOutH == 0)
    {
        EIS_ERR("configMVNumber failed, width=%d, height=%d", apEisConfig.srzOutW, apEisConfig.srzOutH);
        return;
    }

    EISCustom::getMVNumber(apEisConfig.srzOutW, apEisConfig.srzOutH, mvWidth, mvHeight);
}

MINT32 EisHalImp::ConfigGyroAlgo(const EIS_HAL_CONFIG_DATA &aEisConfig,
    const EIS_PLUS_SET_ENV_INFO_STRUCT &eisInitData)
{
    MINT32 err = EIS_RETURN_NO_ERROR;

    GYRO_INIT_INFO_STRUCT  gyroAlgoInitData;
    GYRO_GET_PROC_INFO_STRUCT gyroGetProcData;
    GYRO_SET_WORKING_BUFFER_STRUCT gyroSetworkingbuffer;

    memset(&gyroAlgoInitData, 0, sizeof(gyroAlgoInitData));
    MUINT64 timewithSleep = elapsedRealtime();
    MUINT64 timewithoutSleep = uptimeMillis();

    if (m_pLMVHal->GetLMVStatus())
    {
        m_pLMVHal->GetRegSetting(&gyroAlgoInitData);
    }
    gyroAlgoInitData.GyroCalInfo.GYRO_sample_rate = 1000 / mEISInterval;
    gyroAlgoInitData.eis_mode = eisInitData.EIS_mode;

    // Enable available sensor
    mSensorInfo[mSensorIdx].mUsed = MTRUE; //Default sensor must be enabled.
    if( aEisConfig.is_multiSensor )
    {
        AddMultiSensors(aEisConfig.fov_wide_idx);
        AddMultiSensors(aEisConfig.fov_tele_idx);
    }

    if(EIS_RETURN_NO_ERROR != GetSensorInfo(mSensorIdx))
    {
        EIS_ERR("GetSensorInfo fail");
        return EIS_RETURN_INVALID_DRIVER;
    }

    gyroAlgoInitData.MVWidth = mMVWidth;
    gyroAlgoInitData.MVHeight = mMVHeight;

    gyroAlgoInitData.sensor_Width = aEisConfig.sensor_Width;
    gyroAlgoInitData.sensor_Height = aEisConfig.sensor_Height;

    gyroAlgoInitData.rrz_crop_X = aEisConfig.rrz_crop_X;
    gyroAlgoInitData.rrz_crop_Y = aEisConfig.rrz_crop_Y;

    gyroAlgoInitData.rrz_crop_Width = aEisConfig.rrz_crop_Width;
    gyroAlgoInitData.rrz_crop_Height = aEisConfig.rrz_crop_Height;

    gyroAlgoInitData.rrz_scale_Width = aEisConfig.rrz_scale_Width;
    gyroAlgoInitData.rrz_scale_Height = aEisConfig.rrz_scale_Height;

    gyroAlgoInitData.fov_align_Width = aEisConfig.fov_align_Width;
    gyroAlgoInitData.fov_align_Height = aEisConfig.fov_align_Height;

    gyroAlgoInitData.FSCSlices = mFSCInfo.numSlices;
    gyroAlgoInitData.FSC_en    = mFSCInfo.isEnabled;

    //====== Turn on Eis Configure One Time Flag ======
    if( (mSensorInfo[mSensorIdx].mSensorDev == SENSOR_DEV_SUB) ||
        (mSensorInfo[mSensorIdx].mSensorDev == SENSOR_DEV_SUB_2) )
    {
        EISHAL_SwitchSetGyroReverseValue(mSensorIdx, 1);
        EIS_LOG("mSensorDev(%u), GYRO data reversed",mSensorInfo[mSensorIdx].mSensorDev);
    }
    else
    {
        EISHAL_SwitchSetGyroReverseValue(mSensorIdx, 0);
        EIS_LOG("mSensorDev(%u), GYRO data normal",mSensorInfo[mSensorIdx].mSensorDev);
    }

    if (UNLIKELY(mMultiSensor == 1)) //Multi Sensors
    {
        MUINT32 i;
        for (i = 0; i <MAX_SENSOR_ID; i++)
        {
            if (mSensorInfo[i].mUsed == MTRUE)
            {
                //====== Read NVRAM calibration data ======
                auto pNvBufUtil = MAKE_NvBufUtil();
                err = (!pNvBufUtil)
                    ? EIS_RETURN_NULL_OBJ
                    : pNvBufUtil->getBufAndRead(CAMERA_NVRAM_DATA_FEATURE, mSensorInfo[i].mSensorDev, (void*&)m_pNvram);
                if (err == 0)
                {
                    if (i == mSensorIdx)
                    {
                        memcpy(m_pNVRAM_defParameter, &(m_pNvram->gis), sizeof(NVRAM_CAMERA_FEATURE_GIS_STRUCT));
                    }
                    memcpy(mSensorInfo[i].mRecordParameter, m_pNvram->gis.gis_defParameter3, sizeof(mSensorInfo[i].mRecordParameter));
                    if( UNLIKELY( mDebugDump > 0 ) )
                    {
                        EIS_LOG("SensorIdx=%d, SensorDev=%d, param[0]=%f, param[1]=%f, param[2]=%f, param[3]=%f, param[4]=%f, param[5]=%f",
                                 i, mSensorInfo[i],
                                 mSensorInfo[i].mRecordParameter[0],
                                 mSensorInfo[i].mRecordParameter[1],
                                 mSensorInfo[i].mRecordParameter[2],
                                 mSensorInfo[i].mRecordParameter[3],
                                 mSensorInfo[i].mRecordParameter[4],
                                 mSensorInfo[i].mRecordParameter[5]);
                    }
                    mSensorInfo[i].mDefWidth =  m_pNvram->gis.gis_defWidth;
                    mSensorInfo[i].mDefHeight =  m_pNvram->gis.gis_defHeight;
                    mSensorInfo[i].mDefCrop=  m_pNvram->gis.gis_defCrop;
                }else
                {
                    EIS_ERR("INvBufUtil::getBufAndRead is failed! SensorIdx(%d), SensorDev(%d)", i, mSensorInfo[i].mSensorDev);
                    return  EIS_RETURN_NULL_OBJ;
                }

                EISCustom::getGISParameter(i, mSensorInfo[i].mRecordParameter,
                    &(mSensorInfo[i].mDefWidth),
                    &(mSensorInfo[i].mDefHeight),
                    &(mSensorInfo[i].mDefCrop));
            }
        }
    }else //Single sensor
    {
        //====== Read NVRAM calibration data ======
        if ( auto pNvBufUtil = MAKE_NvBufUtil() ) {
            err = pNvBufUtil->getBufAndRead(CAMERA_NVRAM_DATA_FEATURE, mSensorInfo[mSensorIdx].mSensorDev, (void*&)m_pNvram);
        }
        if (m_pNVRAM_defParameter && m_pNvram)
        {
            memcpy(m_pNVRAM_defParameter, &(m_pNvram->gis), sizeof(NVRAM_CAMERA_FEATURE_GIS_STRUCT));
            memcpy(mSensorInfo[mSensorIdx].mRecordParameter, m_pNvram->gis.gis_defParameter3, sizeof(mSensorInfo[mSensorIdx].mRecordParameter));
            mSensorInfo[mSensorIdx].mDefWidth = m_pNvram->gis.gis_defWidth;
            mSensorInfo[mSensorIdx].mDefHeight = m_pNvram->gis.gis_defHeight;
            mSensorInfo[mSensorIdx].mDefCrop = m_pNvram->gis.gis_defCrop;

        }else
        {
            EIS_ERR("m_pNVRAM_defParameter OR m_pNVRAM_defParameter is NULL\n");
            return  EIS_RETURN_NULL_OBJ;
        }

        EISCustom::getGISParameter(mSensorIdx, mSensorInfo[mSensorIdx].mRecordParameter,
            &(mSensorInfo[mSensorIdx].mDefWidth),
            &(mSensorInfo[mSensorIdx].mDefHeight),
            &(mSensorInfo[mSensorIdx].mDefCrop));
    }

    mNVRAMRead = MFALSE; //No write back

    //> prepare eisPlusAlgoInitData
    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    CalcSensorTrs(mSensorIdx, aEisConfig.sensor_Height);

    gyroAlgoInitData.param_Width = mSensorInfo[mSensorIdx].mDefWidth;
    gyroAlgoInitData.param_Height= mSensorInfo[mSensorIdx].mDefHeight;
    gyroAlgoInitData.param_crop_Y= mSensorInfo[mSensorIdx].mDefCrop;
    gyroAlgoInitData.ProcMode = GYRO_PROC_MODE_MV;
    gyroAlgoInitData.param = mSensorInfo[mSensorIdx].mRecordParameter;
    gyroAlgoInitData.sleep_t =  timewithSleep - timewithoutSleep;
    mSleepTime = gyroAlgoInitData.sleep_t;

    EIS_LOG("def data Rec:(%d, %d) %f    %f    %f    %f    %f    %f", mSensorInfo[mSensorIdx].mDefWidth, mSensorInfo[mSensorIdx].mDefHeight,
                                                              mSensorInfo[mSensorIdx].mRecordParameter[0], mSensorInfo[mSensorIdx].mRecordParameter[1],
                                                              mSensorInfo[mSensorIdx].mRecordParameter[2], mSensorInfo[mSensorIdx].mRecordParameter[3],
                                                              mSensorInfo[mSensorIdx].mRecordParameter[4], mSensorInfo[mSensorIdx].mRecordParameter[5]);

    gyroAlgoInitData.crz_crop_X = aEisConfig.cropX;
    gyroAlgoInitData.crz_crop_Y = aEisConfig.cropY;

    gyroAlgoInitData.crz_crop_Width = aEisConfig.crzOutW;
    gyroAlgoInitData.crz_crop_Height = aEisConfig.crzOutH;

    MBOOL isEIS30Opt = EIS_MODE_IS_EIS_30_ENABLED(mEisMode) ? MTRUE : MFALSE;

    if ((aEisConfig.srzOutW <= D1_WIDTH) && (aEisConfig.srzOutH <= D1_HEIGHT))
    {
        gyroAlgoInitData.block_size = 8;
    }
    else if ((aEisConfig.srzOutW <= ALGOPT_FHD_THR_W) && (aEisConfig.srzOutH <= ALGOPT_FHD_THR_H))
    {
        gyroAlgoInitData.block_size = isEIS30Opt ? 48 : 16;
    }
    else
    {
        gyroAlgoInitData.block_size = 96;
    }

#if !RECORD_WITHOUT_EIS_ENABLE
    if (UNLIKELY(mDebugDump >= 2))
#endif
    {
        gyroAlgoInitData.debug = MTRUE;
    }
    EIS_LOG("sleep_t is (%" PRIu64 ")", gyroAlgoInitData.sleep_t);
    EIS_LOG("aEisConfig  IMG w(%d),h(%d)", aEisConfig.imgiW, aEisConfig.imgiH);

    EIS_LOG("crz offset x(%d),y(%d)", gyroAlgoInitData.crz_crop_X, gyroAlgoInitData.crz_crop_Y);
    EIS_LOG("crzOut w(%d),h(%d)", gyroAlgoInitData.crz_crop_Width, gyroAlgoInitData.crz_crop_Height);
    EIS_LOG("gyroAlgoInitData block_size(%d)", gyroAlgoInitData.block_size);

    if( UNLIKELY(mDebugDump > 0) &&
        mFSCInfo.isEnabled )
    {
        EIS_LOG("FSC_en=%d(slice=%d)", gyroAlgoInitData.FSC_en, gyroAlgoInitData.FSCSlices);
    }

    err = m_pGisAlg->GyroInit(&gyroAlgoInitData);
    if (err != S_GYRO_OK)
    {
        EIS_ERR("GyroInit fail(0x%x)",err);
        return EIS_RETURN_API_FAIL;
    }

    err = m_pGisAlg->GyroFeatureCtrl(GYRO_FEATURE_GET_PROC_INFO, NULL, &gyroGetProcData);
    if (err != S_GYRO_OK)
    {
        EIS_ERR("get Gyro proc info fail(0x%x)",err);
        return EIS_RETURN_API_FAIL;
    }

    CreateMemBuf(gyroGetProcData.ext_mem_size, m_pGisWorkBuf);
    m_pGisWorkBuf->lockBuf("GisWorkBuf", eBUFFER_USAGE_SW_MASK);
    if (!m_pGisWorkBuf->getBufVA(0))
    {
        EIS_ERR("m_pGisWorkBuf create ImageBuffer fail");
        return EIS_RETURN_MEMORY_ERROR;
    }

    gyroSetworkingbuffer.extMemStartAddr = (MVOID *)m_pGisWorkBuf->getBufVA(0);
    gyroSetworkingbuffer.extMemSize = gyroGetProcData.ext_mem_size;

    err = m_pGisAlg->GyroFeatureCtrl(GYRO_FEATURE_SET_WORK_BUF_INFO, &gyroSetworkingbuffer, NULL);
    if(err != S_GYRO_OK)
    {

        EIS_ERR("mGisWorkBuf create IMem fail");
        return EIS_RETURN_API_FAIL;
    }

    return EIS_RETURN_NO_ERROR;

}

MINT32 EisHalImp::CreateEISPlusAlgoBuf()
{
    MINT32 err = EIS_RETURN_NO_ERROR;
    EIS_PLUS_GET_PROC_INFO_STRUCT eisPlusGetProcData;
    EIS_PLUS_SET_WORKING_BUFFER_STRUCT eisPlusWorkBufData;

    eisPlusGetProcData.ext_mem_size = 0;
    eisPlusGetProcData.Grid_H = 0;
    eisPlusGetProcData.Grid_W = 0;

    eisPlusWorkBufData.extMemSize = 0;
    eisPlusWorkBufData.extMemStartAddr = NULL;

    //> Preapre EIS Plus Working Buffer

    err = m_pEisPlusAlg->EisPlusFeatureCtrl(EIS_PLUS_FEATURE_GET_PROC_INFO, NULL, &eisPlusGetProcData);
    if (err != S_EIS_PLUS_OK)
    {
        EIS_ERR("EisPlus: EIS_PLUS_FEATURE_GET_PROC_INFO fail(0x%x)",err);
        return EIS_RETURN_API_FAIL;
    }

    EIS_LOG("ext_mem_size(%u)",eisPlusGetProcData.ext_mem_size);

    CreateMemBuf(eisPlusGetProcData.ext_mem_size, m_pEisPlusWorkBuf);
    m_pEisPlusWorkBuf->lockBuf("EISPlusWorkBuf", eBUFFER_USAGE_SW_MASK);
    if (!m_pEisPlusWorkBuf->getBufVA(0))
    {
        EIS_ERR("m_pEisPlusWorkBuf create ImageBuffer fail");
        return EIS_RETURN_MEMORY_ERROR;
    }

    EIS_LOG("m_pEisPlusWorkBuf : size(%u),virAdd(0x%p)", eisPlusGetProcData.ext_mem_size, (MVOID*)m_pEisPlusWorkBuf->getBufVA(0));

    eisPlusWorkBufData.extMemSize = eisPlusGetProcData.ext_mem_size;
    eisPlusWorkBufData.extMemStartAddr = (MVOID *)m_pEisPlusWorkBuf->getBufVA(0);

    err = m_pEisPlusAlg->EisPlusFeatureCtrl(EIS_PLUS_FEATURE_SET_WORK_BUF_INFO, &eisPlusWorkBufData, NULL);
    if (err != S_EIS_PLUS_OK)
    {
        EIS_ERR("EisPlus: EIS_PLUS_FEATURE_SET_WORK_BUF_INFO fail(0x%x)",err);
        return EIS_RETURN_API_FAIL;
    }
    return EIS_RETURN_NO_ERROR;

}


/*******************************************************************************
*
********************************************************************************/
MINT32 EisHalImp::ConfigGis(const EIS_HAL_CONFIG_DATA &aEisConfig, const MUINT32 eisMode)
{
    if(mEisSupport == MFALSE)
    {
        EIS_LOG("mSensorIdx(%u) not support EIS",mSensorIdx);
        return EIS_RETURN_NO_ERROR;
    }

    MINT32 err = EIS_RETURN_NO_ERROR;

#if EIS_ALGO_READY
    static EIS_SET_ENV_INFO_STRUCT eisAlgoInitData;

    if (mIsEisPlusConfig == 0)
    {
        EnableSensor();
        EIS_LOG("EIS Plus first config");

        mEisMode = eisMode;

        /* Configure matrix dimension
         */
        mGpuGridW = 2;
        mGpuGridH = 2;

        EIS_LOG("EIS Warp Matrix = Grid W(%d), H(%d)", mGpuGridW, mGpuGridH);

        EIS_PLUS_SET_ENV_INFO_STRUCT eisPlusAlgoInitData;
        //> prepare eisPlusAlgoInitData
        memset(&eisPlusAlgoInitData, 0, sizeof(eisPlusAlgoInitData));

        // get EIS Plus customized data
        GetEisPlusCustomize(&eisPlusAlgoInitData.eis_plus_tuning_data, &aEisConfig);

        if (UNLIKELY(mDebugDump >= 2))
        {
            eisPlusAlgoInitData.debug = MTRUE;
            EIS_LOG("eisPlus debug(%d)",eisPlusAlgoInitData.debug);
        }

        EIS_LOG("EIS Plus tuning_data");
        EIS_LOG("warping_mode(%d),effort(%d)",eisPlusAlgoInitData.eis_plus_tuning_data.warping_mode,eisPlusAlgoInitData.eis_plus_tuning_data.effort);
        EIS_LOG("search_range(%d,%d)",eisPlusAlgoInitData.eis_plus_tuning_data.search_range_x,eisPlusAlgoInitData.eis_plus_tuning_data.search_range_y);
        EIS_LOG("crop_ratio(%d),stabilization_strength(%f)",eisPlusAlgoInitData.eis_plus_tuning_data.crop_ratio,eisPlusAlgoInitData.eis_plus_tuning_data.stabilization_strength);

        //> Init Eis plus

        eisPlusAlgoInitData.MVWidth = mMVWidth;
        eisPlusAlgoInitData.MVHeight = mMVHeight;

        eisPlusAlgoInitData.GyroValid= mGyroEnable;
        eisPlusAlgoInitData.Gvalid= mAccEnable;
        eisPlusAlgoInitData.EIS_mode = 2;

        err = m_pEisPlusAlg->EisPlusInit(&eisPlusAlgoInitData);
        if(err != S_EIS_PLUS_OK)
        {
            EIS_ERR("EisPlusInit fail(0x%x)",err);
            return EIS_RETURN_API_FAIL;
        }

        CreateEISPlusAlgoBuf();
        ConfigGyroAlgo(aEisConfig, eisPlusAlgoInitData);
        mIsEisPlusConfig = 1;
    }
#endif
    return EIS_RETURN_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 EisHalImp::ConfigFEFMEis(const EIS_HAL_CONFIG_DATA & aEisConfig, const MUINT32 eisMode, const MULTISCALE_INFO* apMultiScaleInfo)
{
    if(mEisSupport == MFALSE)
    {
        EIS_LOG("mSensorIdx(%u) not support EIS",mSensorIdx);
        return EIS_RETURN_NO_ERROR;
    }

    MINT32 err = EIS_RETURN_NO_ERROR;

#if EIS_ALGO_READY
    static EIS_SET_ENV_INFO_STRUCT eisAlgoInitData;
#endif

#if EIS_ALGO_READY
    if (mIsEisPlusConfig == 0)
    {
        EnableSensor();
        EIS_LOG("EIS Plus first config");

        mEisMode = eisMode;

        /* Configure matrix dimension
         */
        if( EIS_MODE_IS_EIS_22_ENABLED(mEisMode) ||
            (EIS_MODE_IS_EIS_25_ENABLED(mEisMode) && !EIS_MODE_IS_EIS_DEJELLO_ENABLED(mEisMode)) )
        {
            mGpuGridW = 2;
            mGpuGridH = 2;
        }
        else
        {
            mGpuGridW = 31;
            mGpuGridH = 18;
        }

        EIS_LOG("EISMode = (0x%x), EIS Warp Matrix = Grid W(%d), H(%d)",
                eisMode, mGpuGridW, mGpuGridH);

        EIS_PLUS_SET_ENV_INFO_STRUCT eisPlusAlgoInitData;

        //> prepare eisPlusAlgoInitData
        memset(&eisPlusAlgoInitData, 0, sizeof(eisPlusAlgoInitData));

        // get EIS Plus customized data
        GetEisPlusCustomize(&eisPlusAlgoInitData.eis_plus_tuning_data, &aEisConfig);

        if (UNLIKELY(mDebugDump >= 2))
        {
            eisPlusAlgoInitData.debug = MTRUE;
            EIS_LOG("eisPlus debug(%d)",eisPlusAlgoInitData.debug);
        }

        EIS_LOG("EIS Plus tuning_data");
        EIS_LOG("warping_mode(%d),effort(%d)",eisPlusAlgoInitData.eis_plus_tuning_data.warping_mode,eisPlusAlgoInitData.eis_plus_tuning_data.effort);
        EIS_LOG("search_range(%d,%d)",eisPlusAlgoInitData.eis_plus_tuning_data.search_range_x,eisPlusAlgoInitData.eis_plus_tuning_data.search_range_y);
        EIS_LOG("crop_ratio(%d),stabilization_strength(%f)",eisPlusAlgoInitData.eis_plus_tuning_data.crop_ratio,eisPlusAlgoInitData.eis_plus_tuning_data.stabilization_strength);

        //> Init Eis plus

        //EIS 2.5
        GetEis25Customize(&eisPlusAlgoInitData.eis25_tuning_data, &aEisConfig);
#if RECORD_WITHOUT_EIS_ENABLE
        if (UNLIKELY(eisPlusAlgoInitData.eis25_tuning_data.en_dejello != g_forecDejello))
        {
            eisPlusAlgoInitData.eis25_tuning_data.en_dejello = g_forecDejello;
            EIS_LOG("Force dejello: %d", eisPlusAlgoInitData.eis25_tuning_data.en_dejello);
        }
#endif

        eisPlusAlgoInitData.Gvalid    = mAccEnable;
        eisPlusAlgoInitData.GyroValid = mGyroEnable;

        /* Set up EIS algo mode
         */
        if( EIS_MODE_IS_EIS_GYRO_ENABLED(mEisMode) && EIS_MODE_IS_EIS_IMAGE_ENABLED(mEisMode) )
        {
            // Fusion
            eisPlusAlgoInitData.EIS_mode = 3;
            eisPlusAlgoInitData.en_gyro_fusion = 1;
            if (UNLIKELY(mDebugDump > 0))
            {
                EIS_LOG("Run EIS 2.5 Fusion");
            }
        }
        else if( EIS_MODE_IS_EIS_GYRO_ENABLED(mEisMode) && !EIS_MODE_IS_EIS_IMAGE_ENABLED(mEisMode) )
        {
            // Gyro-based Only
            eisPlusAlgoInitData.EIS_mode = 4;
            eisPlusAlgoInitData.en_gyro_fusion = 1;
            if (UNLIKELY(mDebugDump > 0))
            {
                EIS_LOG("Run EIS 2.5 Gyro-based only");
            }
        }
        else if( EIS_MODE_IS_EIS_IMAGE_ENABLED(mEisMode) && !EIS_MODE_IS_EIS_GYRO_ENABLED(mEisMode) )
        {
            // Image-based Only
            eisPlusAlgoInitData.EIS_mode = 3;
            eisPlusAlgoInitData.en_gyro_fusion = 0;
            eisPlusAlgoInitData.GyroValid = 0;
            eisPlusAlgoInitData.eis25_tuning_data.en_dejello = 0;
            if (UNLIKELY(mDebugDump > 0))
            {
                EIS_LOG("Run EIS 2.5 Image-based only");
            }
        }
        else
        {
            // fallthrough
            eisPlusAlgoInitData.EIS_mode = 0;
            eisPlusAlgoInitData.en_gyro_fusion = 0;
            EIS_ERR("Unknown EIS Mode!");
        }


#if RECORD_WITHOUT_EIS_ENABLE
        if (UNLIKELY(g_GyroOnly == 1))
        {
            eisPlusAlgoInitData.EIS_mode = 4;
            EIS_LOG("Force Gyro-only mode: %d",eisPlusAlgoInitData.EIS_mode);
        }
#endif

#if RECORD_WITHOUT_EIS_ENABLE
        if (UNLIKELY(g_ImageOnly == 1))
        {
            g_GyroOnly = 0;
            eisPlusAlgoInitData.EIS_mode = 3;
            eisPlusAlgoInitData.en_gyro_fusion = 0; //DEBUG MODE: Image-only
            EIS_LOG("Force Image-only mode: %d ,en_gyro_fusion: %d", eisPlusAlgoInitData.EIS_mode, eisPlusAlgoInitData.en_gyro_fusion);
        }
#endif

        for (MUINT32 i = 0; (i<MULTISCALE_FEFM) && apMultiScaleInfo; i++)
        {
            eisPlusAlgoInitData.MultiScale_width[i] = apMultiScaleInfo->MultiScale_width[i];
            eisPlusAlgoInitData.MultiScale_height[i] = apMultiScaleInfo->MultiScale_height[i];
            eisPlusAlgoInitData.MultiScale_blocksize[i] = apMultiScaleInfo->MultiScale_blocksize[i];

            EIS_LOG("FE level(%d) w/h (%d/%d) , block (%d)", i, apMultiScaleInfo->MultiScale_width[i],
                                                                apMultiScaleInfo->MultiScale_height[i],
                                                                apMultiScaleInfo->MultiScale_blocksize[i]);
        }

        switch( aEisConfig.warp_precision )
        {
        case 4:
            eisPlusAlgoInitData.warpPrecision = EIS_PLUS_WARP_PRECISION_4_BITS;
            break;
        case 5:
            eisPlusAlgoInitData.warpPrecision = EIS_PLUS_WARP_PRECISION_5_BITS;
            break;
        default:
            eisPlusAlgoInitData.warpPrecision = EIS_PLUS_WARP_PRECISION_5_BITS;
            EIS_WRN("Unsupported warp precision=%d, use default precision=5(idx=%d)",
                    aEisConfig.warp_precision, eisPlusAlgoInitData.warpPrecision);
            break;
        }

        /*
                                                 EIS_MODE   GYRO_VALID  en_gyro_fusion
        EIS 2.5 Image-based Only                 3           0           N/A         No gyroscope exist
        EIS 2.5 Gyro-based Only                  4           1            1          4Kx2K
        EIS 2.5 Fusion                           3           1            1          FHD
        EIS 2.5 Image-based Only + Gyro still    3           1            0          vHDR
        EIS 1.2                                  0           0            0          4Kx2K with vHDR
        */
        if (UNLIKELY(mDebugDump > 0))
        {
            EIS_LOG("EIS_MODE=%d, GYRO_VALID=%d, en_gyro_fusion=%d, warpPrecision=%d",
                    eisPlusAlgoInitData.EIS_mode,
                    eisPlusAlgoInitData.GyroValid,
                    eisPlusAlgoInitData.en_gyro_fusion,
                    eisPlusAlgoInitData.warpPrecision);
        }

        err = m_pEisPlusAlg->EisPlusInit(&eisPlusAlgoInitData);
        if(err != S_EIS_PLUS_OK)
        {
            EIS_ERR("EisPlusInit fail(0x%x)",err);
            return EIS_RETURN_API_FAIL;
        }

        CreateEISPlusAlgoBuf();
        ConfigGyroAlgo(aEisConfig, eisPlusAlgoInitData);

        mIsEisPlusConfig = 1;
    }
#endif

    return EIS_RETURN_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 EisHalImp::ConfigRSCMEEis(const EIS_HAL_CONFIG_DATA & aEisConfig, const MUINT32 eisMode, const FSC_INFO_T* fscInfo)
{
    if(mEisSupport == MFALSE)
    {
        EIS_LOG("mSensorIdx(%u) not support EIS",mSensorIdx);
        return EIS_RETURN_NO_ERROR;
    }

    MINT32 err = EIS_RETURN_NO_ERROR;

#if EIS_ALGO_READY
    static EIS_SET_ENV_INFO_STRUCT eisAlgoInitData;
#endif

#if EIS_ALGO_READY
    if (mIsEisPlusConfig == 0)
    {
        EnableSensor();
        EIS_LOG("EIS Plus first config");

        mEisMode = eisMode;

        /* Configure matrix dimension
         */
        if( EIS_MODE_IS_EIS_22_ENABLED(mEisMode) ||
            (EIS_MODE_IS_EIS_25_ENABLED(mEisMode) && !EIS_MODE_IS_EIS_DEJELLO_ENABLED(mEisMode)) )
        {
            mGpuGridW = 2;
            mGpuGridH = 2;
        }
        else
        {
            mGpuGridW = 31;
            mGpuGridH = 18;
        }

        EIS_LOG("EISMode = (0x%x), EIS Warp Matrix = Grid W(%d), H(%d)",
                eisMode, mGpuGridW, mGpuGridH);

        EIS_PLUS_SET_ENV_INFO_STRUCT eisPlusAlgoInitData;

        //> prepare eisPlusAlgoInitData
        memset(&eisPlusAlgoInitData, 0, sizeof(eisPlusAlgoInitData));

        // get EIS Plus customized data
        GetEisPlusCustomize(&eisPlusAlgoInitData.eis_plus_tuning_data, &aEisConfig);

        if (UNLIKELY(mDebugDump >= 2))
        {
            eisPlusAlgoInitData.debug = MTRUE;
            EIS_LOG("eisPlus debug(%d)",eisPlusAlgoInitData.debug);
        }

        EIS_LOG("EIS Plus tuning_data");
        EIS_LOG("warping_mode(%d),effort(%d)",eisPlusAlgoInitData.eis_plus_tuning_data.warping_mode,eisPlusAlgoInitData.eis_plus_tuning_data.effort);
        EIS_LOG("search_range(%d,%d)",eisPlusAlgoInitData.eis_plus_tuning_data.search_range_x,eisPlusAlgoInitData.eis_plus_tuning_data.search_range_y);
        EIS_LOG("crop_ratio(%d),stabilization_strength(%f)",eisPlusAlgoInitData.eis_plus_tuning_data.crop_ratio,eisPlusAlgoInitData.eis_plus_tuning_data.stabilization_strength);

        //> Init Eis plus

        //EIS 3.0
        GetEis30Customize(&eisPlusAlgoInitData.eis30_tuning_data, &aEisConfig);

        eisPlusAlgoInitData.MVWidth = mMVWidth;
        eisPlusAlgoInitData.MVHeight = mMVHeight;

        eisPlusAlgoInitData.Gvalid    = mAccEnable;
        eisPlusAlgoInitData.GyroValid = mGyroEnable;

        /* Set up EIS algo mode
         */
        if( EIS_MODE_IS_EIS_GYRO_ENABLED(mEisMode) && EIS_MODE_IS_EIS_IMAGE_ENABLED(mEisMode) )
        {
            // Fusion
            eisPlusAlgoInitData.EIS_mode = 5;
            eisPlusAlgoInitData.en_gyro_fusion = 1;
            if (UNLIKELY(mDebugDump > 0))
            {
                EIS_LOG("Run EIS 3.0 Fusion");
            }
        }
        else if( EIS_MODE_IS_EIS_GYRO_ENABLED(mEisMode) && !EIS_MODE_IS_EIS_IMAGE_ENABLED(mEisMode) )
        {
            // Gyro-based Only
            eisPlusAlgoInitData.EIS_mode = 6;
            eisPlusAlgoInitData.en_gyro_fusion = 1;
            if (UNLIKELY(mDebugDump > 0))
            {
                EIS_LOG("Run EIS 3.0 Gyro-based only");
            }
        }
        else if( EIS_MODE_IS_EIS_IMAGE_ENABLED(mEisMode) && !EIS_MODE_IS_EIS_GYRO_ENABLED(mEisMode) )
        {
            // Image-based Only
            eisPlusAlgoInitData.EIS_mode = 5;
            eisPlusAlgoInitData.en_gyro_fusion = 0;
            eisPlusAlgoInitData.GyroValid = 0;
            if (UNLIKELY(mDebugDump > 0))
            {
                EIS_LOG("Run EIS 3.0 Image-based only");
            }
        }
        else
        {
            // fallthrough
            eisPlusAlgoInitData.EIS_mode = 0;
            eisPlusAlgoInitData.en_gyro_fusion = 0;
            EIS_ERR("Unknown EIS Mode!");
        }

#if RECORD_WITHOUT_EIS_ENABLE
        if (UNLIKELY(g_GyroOnly == 1))
        {
            eisPlusAlgoInitData.EIS_mode = 6;
            eisPlusAlgoInitData.en_gyro_fusion = 1;
            EIS_LOG("Force Gyro-only mode: %d, en_gyro_fusion: %d",eisPlusAlgoInitData.EIS_mode, eisPlusAlgoInitData.en_gyro_fusion);
        }
#endif

#if RECORD_WITHOUT_EIS_ENABLE
        if (UNLIKELY(g_ImageOnly == 1))
        {
            g_GyroOnly = 0;
            eisPlusAlgoInitData.EIS_mode = 5;
            eisPlusAlgoInitData.en_gyro_fusion = 0;
            EIS_LOG("Force Image-only mode: %d , en_gyro_fusion: %d", eisPlusAlgoInitData.EIS_mode, eisPlusAlgoInitData.en_gyro_fusion);
        }
#endif
        switch( aEisConfig.warp_precision )
        {
        case 4:
            eisPlusAlgoInitData.warpPrecision = EIS_PLUS_WARP_PRECISION_4_BITS;
            break;
        case 5:
            eisPlusAlgoInitData.warpPrecision = EIS_PLUS_WARP_PRECISION_5_BITS;
            break;
        default:
            eisPlusAlgoInitData.warpPrecision = EIS_PLUS_WARP_PRECISION_5_BITS;
            EIS_WRN("Unsupported warp precision=%d, use default precision=5(idx=%d)",
                    aEisConfig.warp_precision, eisPlusAlgoInitData.warpPrecision);
            break;
        }

        if( fscInfo != NULL )
        {
            mFSCInfo.numSlices = fscInfo->numSlices;
            mFSCInfo.isEnabled = fscInfo->isEnabled;

            eisPlusAlgoInitData.FSCSlices =  mFSCInfo.numSlices;
            eisPlusAlgoInitData.FSC_en    =  mFSCInfo.isEnabled;
        }

        if (UNLIKELY(mDebugDump > 0))
        {
            EIS_LOG("EIS_MODE=%d,GYRO_VALID=%d,en_gyro_fusion=%d,warpPrecision=%d,FSC_en=%d(slice=%d)",
                    eisPlusAlgoInitData.EIS_mode,
                    eisPlusAlgoInitData.GyroValid,
                    eisPlusAlgoInitData.en_gyro_fusion,
                    eisPlusAlgoInitData.warpPrecision,
                    eisPlusAlgoInitData.FSC_en,
                    eisPlusAlgoInitData.FSCSlices
                    );
        }

        err = m_pEisPlusAlg->EisPlusInit(&eisPlusAlgoInitData);
        if(err != S_EIS_PLUS_OK)
        {
            EIS_ERR("EisPlusInit fail(0x%x)",err);
            return EIS_RETURN_API_FAIL;
        }

        CreateEISPlusAlgoBuf();
        ConfigGyroAlgo(aEisConfig, eisPlusAlgoInitData);

        mIsEisPlusConfig = 1;
    }
#endif

    return EIS_RETURN_NO_ERROR;
}


MINT32 EisHalImp::ForcedDoEisPass2()
{
    DP_TRACE_CALL();
    EISHAL_SwitchGyroQueueLock(mSensorIdx);
    mSkipWaitGyro = MTRUE;
    EISHAL_SWitchWaitGyroCond_Signal(mSensorIdx);
    EISHAL_SwitchGyroQueueUnlock(mSensorIdx);
    return EIS_RETURN_NO_ERROR;
}

MINT32 EisHalImp::ExecuteGyroAlgo(const EIS_HAL_CONFIG_DATA* apEisConfig,const MINT64 aTimeStamp, const MUINT32 aExpTime, MUINT32 aLExpTime, GYRO_MV_RESULT_INFO_STRUCT& gyroMVresult, IMAGE_BASED_DATA *imgBaseData)
{
    MINT32 err = EIS_RETURN_NO_ERROR;
    GYRO_SET_PROC_INFO_STRUCT gyroSetProcData;
    MUINT64 gyro_t_frame_array[GYRO_DATA_PER_FRAME];
    MDOUBLE gyro_xyz_frame_array[GYRO_DATA_PER_FRAME*3];

    memset(&gyroSetProcData,0,sizeof(gyroSetProcData));

    if (UNLIKELY(mTsForAlgoDebug == 0))
    {
        mTsForAlgoDebug = aTimeStamp;
    }

    if (UNLIKELY(mMultiSensor != 0)) //Support Multiple sensor mode
    {
        if (UNLIKELY(mCurSensorIdx != apEisConfig->sensorIdx))
        {
            //Get the new sensor info
            mCurSensorIdx = apEisConfig->sensorIdx;
            if (mSensorInfo[mCurSensorIdx].mUsed == MTRUE)
            {
                CalcSensorTrs(mCurSensorIdx, apEisConfig->sensor_Height);
            }else
            {
                EIS_WRN("mSensorInfo[%d] is not used", mCurSensorIdx);
                AddMultiSensors(mCurSensorIdx);
                CalcSensorTrs(mCurSensorIdx, apEisConfig->sensor_Height);
            }
        }
    }

    MBOOL gyroEnabled = mpSensorProvider->isEnabled(SENSOR_TYPE_GYRO);

    if ((aTimeStamp != 0) && gyroEnabled)
    {
        MUINT32 waitTime = 0;
        EIS_GyroRecord lastGyro;
        lastGyro.ts = 0;
        const MINT64 currentTarget = aTimeStamp + (mSleepTime*1000L) + (mSensorInfo[mCurSensorIdx].mtRSTime*1000L);
        do
        {
            if (waitTime >= (mEISInterval * 2))
            {
                EIS_WRN("Wait gyro timedout! currentTarget= %" PRId64 " ms, lastGyro= %" PRId64 " ms, systemTime= %" PRId64 " ms",
                    currentTarget/1000000, lastGyro.ts/1000000, getTimeInMs());
                EIS_WRN("Detect no gyro data coming timeout(%d ms), now force skip waiting", waitTime);
                break;
            }

            //Get sensor data from SensorProvider and push to GyroQueue
            GetSensorData();

            MBOOL bGTTarget = MFALSE;
            EISHAL_SwitchGyroQueueLock(mSensorIdx);
            mSkipWaitGyro = MFALSE;
            EISHAL_SwitchGyroQueueBack(mSensorIdx, lastGyro);
            EISHAL_SwitchIsLastGyroTimestampGT(mSensorIdx, currentTarget, bGTTarget);

            if (bGTTarget)
            {
                EIS_LOG("video (%" PRIi64 ") < global Gyro timestamp => go GIS ", currentTarget);
                EISHAL_SwitchGyroQueueUnlock(mSensorIdx);
                break;
            }

            if (mSkipWaitGyro == MTRUE)
            {
                EIS_LOG("skip wait Gyro: %d by next video trigger",waitTime);
                EISHAL_SwitchGyroQueueUnlock(mSensorIdx);
                break;
            }

            EISHAL_SwitchGyroQueueUnlock(mSensorIdx);
            if (lastGyro.ts >= currentTarget)
            {
                //Get gyro data successfully
                break;
            }

            waitTime++;

            if (UNLIKELY(mDebugDump >= 1))
            {
                if (UNLIKELY(waitTime > 1))
                {
                    EIS_LOG("wait Gyro time: %d",waitTime);
                }
            }
            usleep(1000); //1ms
        }while(1);
    }

#if RECORD_WITHOUT_EIS_ENABLE
    if (g_AIMDump == 1)
    {
#if 0
        if (UNLIKELY(mDebugDump >= 1))
        {
            EIS_LOG("vHDR LE(%d), SE(%d)",OutputParam.u4videoHDRLE_us, OutputParam.u4videoHDRSE_us);
        }

        if(gvHDRRecordWriteID >= TSRECORD_MAXSIZE)
        {
                gvHDRRecordWriteID = 0;
        }
        gvHDRRecord[gvHDRRecordWriteID].id = 5;
        gvHDRRecord[gvHDRRecordWriteID].ts = ((MUINT64)OutputParam.u4videoHDRLE_us<<32);
        gvHDRRecord[gvHDRRecordWriteID].ts += (MUINT64)OutputParam.u4videoHDRSE_us;
        EIS_LOG("gTSRecord[gvHDRRecordWriteID].ts(%d)",gvHDRRecord[gvHDRRecordWriteID].ts);
        gvHDRRecordWriteID++;
#endif
        if(gTSRecordWriteID >= TSRECORD_MAXSIZE)
        {
            gTSRecordWriteID = 0;
        }
        gTSRecord[gTSRecordWriteID].id = 3;
        gTSRecord[gTSRecordWriteID].ts = aExpTime;
        gTSRecordWriteID++;

        if ( gGyroRecordWriteID > 1)
        {
            if(gTSRecordWriteID >= TSRECORD_MAXSIZE)
            {
                gTSRecordWriteID = 0;
            }
            gTSRecord[gTSRecordWriteID].id = 2;
            gTSRecord[gTSRecordWriteID].ts = 0; //No finding gyro timestamp

            for (int i=gGyroRecordWriteID-1;i>0;i--)
            {
                if (gGyroRecord[i].id == EIS_GYROSCOPE)
                {
                    gTSRecord[gTSRecordWriteID].ts = gGyroRecord[i].ts;
                    break;
                }
            }
            gTSRecordWriteID++;
        }else
        {
            if(gTSRecordWriteID >= TSRECORD_MAXSIZE)
            {
                gTSRecordWriteID = 0;
            }

            gTSRecord[gTSRecordWriteID].id = 2;
            gTSRecord[gTSRecordWriteID].ts = 0; //No gyro data exist
            gTSRecordWriteID++;
        }

        if(gTSRecordWriteID >= TSRECORD_MAXSIZE)
        {
            gTSRecordWriteID = 0;
        }
        gTSRecord[gTSRecordWriteID].id = 1;
        gTSRecord[gTSRecordWriteID].ts = aTimeStamp;
        gTSRecordWriteID++;

    }
#endif

    EISHAL_SwitchGyroQueueLock(mSensorIdx);
    EISHAL_SwitchPopGyroQueue(mSensorIdx, gyro_t_frame_array, gyro_xyz_frame_array, gyroSetProcData.gyro_num);
    EISHAL_SwitchGyroQueueUnlock(mSensorIdx);

    if (UNLIKELY(mDebugDump >= 1))
    {
        EIS_LOG("Gyro data num: %d, video ts: %" PRIi64 " ", gyroSetProcData.gyro_num, aTimeStamp);
    }

    gyroSetProcData.MVWidth = mMVWidth;
    gyroSetProcData.MVHeight = mMVHeight;

    gyroSetProcData.frame_t = aTimeStamp;
    gyroSetProcData.frame_AE = aExpTime;

    // New for EIS 3.0
    gyroSetProcData.cam_idx = mCurSensorIdx;
    gyroSetProcData.sensor_Width = apEisConfig->sensor_Width;
    gyroSetProcData.sensor_Height = apEisConfig->sensor_Height;
    gyroSetProcData.param = mSensorInfo[mCurSensorIdx].mRecordParameter;
    gyroSetProcData.vHDR_idx = apEisConfig->vHDREnabled;
    gyroSetProcData.frame_LE = aLExpTime;
    gyroSetProcData.frame_SE = aExpTime;

    gyroSetProcData.gyro_t_frame = gyro_t_frame_array;
    gyroSetProcData.gyro_xyz_frame = gyro_xyz_frame_array;

    gyroSetProcData.rrz_crop_X = apEisConfig->rrz_crop_X;
    gyroSetProcData.rrz_crop_Y = apEisConfig->rrz_crop_Y;

    gyroSetProcData.rrz_crop_Width = apEisConfig->rrz_crop_Width;
    gyroSetProcData.rrz_crop_Height = apEisConfig->rrz_crop_Height;

    gyroSetProcData.rrz_scale_Width = apEisConfig->rrz_scale_Width;
    gyroSetProcData.rrz_scale_Height = apEisConfig->rrz_scale_Height;

    gyroSetProcData.crz_crop_X = apEisConfig->cropX;
    gyroSetProcData.crz_crop_Y = apEisConfig->cropY;

    gyroSetProcData.crz_crop_Width = apEisConfig->crzOutW;
    gyroSetProcData.crz_crop_Height = apEisConfig->crzOutH;

    gyroSetProcData.fov_align_Width = apEisConfig->fov_align_Width;
    gyroSetProcData.fov_align_Height = apEisConfig->fov_align_Height;
    gyroSetProcData.warp_grid[0].x = apEisConfig->warp_grid[0].x;
    gyroSetProcData.warp_grid[1].x = apEisConfig->warp_grid[1].x;
    gyroSetProcData.warp_grid[2].x = apEisConfig->warp_grid[2].x;
    gyroSetProcData.warp_grid[3].x = apEisConfig->warp_grid[3].x;
    gyroSetProcData.warp_grid[0].y = apEisConfig->warp_grid[0].y;
    gyroSetProcData.warp_grid[1].y = apEisConfig->warp_grid[1].y;
    gyroSetProcData.warp_grid[2].y = apEisConfig->warp_grid[2].y;
    gyroSetProcData.warp_grid[3].y = apEisConfig->warp_grid[3].y;

    if( mFSCInfo.isEnabled )
    {
        gyroSetProcData.FSCScalingFactor = imgBaseData->fscData->scalingFactor;
    }

    MINT32  eis_data[EIS_WIN_NUM*4];
    gyroSetProcData.val_LMV = apEisConfig->lmvDataEnabled;
    if (gyroSetProcData.val_LMV)
    {
        for (MINT32 i=0; i<EIS_WIN_NUM; i++)
        {
            eis_data[4*i + 0] = apEisConfig->lmv_data->i4LMV_X[i];
            eis_data[4*i + 1] = apEisConfig->lmv_data->i4LMV_Y[i];
            eis_data[4*i + 2] = apEisConfig->lmv_data->NewTrust_X[i];
            eis_data[4*i + 3] = apEisConfig->lmv_data->NewTrust_Y[i];
        }
        gyroSetProcData.EIS_LMV = eis_data;
    }

    if (UNLIKELY(mDebugDump >= 1))
    {
        EIS_LOG("crz offset x(%d),y(%d)", gyroSetProcData.crz_crop_X, gyroSetProcData.crz_crop_Y);
        EIS_LOG("crzOut w(%d),h(%d)", gyroSetProcData.crz_crop_Width, gyroSetProcData.crz_crop_Height);
        EIS_LOG("cam_idx(%d), sensor_Width(%d), sensor_Height(%d), vHDR_idx(%d), SE(%d), LE(%d)",
                gyroSetProcData.cam_idx, gyroSetProcData.sensor_Width, gyroSetProcData.sensor_Height,
                gyroSetProcData.vHDR_idx, gyroSetProcData.frame_SE, gyroSetProcData.frame_LE);
        EIS_LOG("fov_align_Width(%d), fov_align_Height(%d)", gyroSetProcData.fov_align_Width, gyroSetProcData.fov_align_Height);
        EIS_LOG("warp_gridX[0]=%f, warp_gridX[1]=%f, warp_gridX[2]=%f, warp_gridX[3]=%f",
                gyroSetProcData.warp_grid[0].x, gyroSetProcData.warp_grid[1].x,
                gyroSetProcData.warp_grid[2].x, gyroSetProcData.warp_grid[3].x);
        EIS_LOG("warp_gridY[0]=%f, warp_gridY[1]=%f, warp_gridY[2]=%f, warp_gridY[3]=%f",
                gyroSetProcData.warp_grid[0].y, gyroSetProcData.warp_grid[1].y,
                gyroSetProcData.warp_grid[2].y, gyroSetProcData.warp_grid[3].y);
        EIS_LOG("MVWidth=%d, MVHeight=%d", gyroSetProcData.MVWidth, gyroSetProcData.MVHeight);
    }
    if( UNLIKELY(mDebugDump >= 2) )
    {
        EIS_LOG("gyroSetProcData.param[0]=%f, gyroSetProcData.param[1]=%f, gyroSetProcData.param[2]=%f",
                mSensorInfo[mCurSensorIdx].mRecordParameter[0],
                mSensorInfo[mCurSensorIdx].mRecordParameter[1],
                mSensorInfo[mCurSensorIdx].mRecordParameter[2]);
        EIS_LOG("gyroSetProcData.param[3]=%f, gyroSetProcData.param[4]=%f, gyroSetProcData.param[5]=%f",
                mSensorInfo[mCurSensorIdx].mRecordParameter[3],
                mSensorInfo[mCurSensorIdx].mRecordParameter[4],
                mSensorInfo[mCurSensorIdx].mRecordParameter[5]);
        if( mFSCInfo.isEnabled )
        {
            MUINT32 *ptr = (MUINT32*) gyroSetProcData.FSCScalingFactor;
            EIS_LOG("FSCFactor(%p)", gyroSetProcData.FSCScalingFactor);
            for( MUINT32 i = 0; i < mFSCInfo.numSlices; ++i )
            {
                EIS_LOG("FSC[%u]=%d", i, ptr[i]);
            }
        }
    }
    if (mDisableGyroData)
    {
        memset(gyroSetProcData.gyro_xyz_frame, 0, sizeof(gyro_xyz_frame_array));
    }
    //====== GIS Algorithm ======

    err = m_pGisAlg->GyroFeatureCtrl(GYRO_FEATURE_SET_PROC_INFO,&gyroSetProcData, NULL);
    if(err != S_GYRO_OK)
    {
            EIS_ERR("GIS:GYRO_FEATURE_SET_PROC_INFO fail(0x%x)",err);
            err = EIS_RETURN_API_FAIL;
            return err;
    }

    err = m_pGisAlg->GyroMain();
    if(err != S_GYRO_OK)
    {
            EIS_ERR("GIS:GyroMain fail(0x%x)",err);
            err = EIS_RETURN_API_FAIL;
            return err;
    }


    err = m_pGisAlg->GyroFeatureCtrl(GYRO_FEATURE_GET_MV_RESULT_INFO, NULL, &gyroMVresult);
    if(err != S_GYRO_OK)
    {
        EIS_ERR("GIS:GYRO_FEATURE_SET_PROC_INFO fail(0x%x)",err);
        err = EIS_RETURN_API_FAIL;
        return err;
    }

    if (mBitTrueDump)
    {
        dumpBitTrue(NULL, NULL, NULL, &gyroSetProcData);
    }

    return EIS_RETURN_NO_ERROR;
}


MINT32 EisHalImp::DoGis(EIS_HAL_CONFIG_DATA *apEisConfig, MINT64 aTimeStamp, MUINT32 aExpTime)
{
    MINT32 err = EIS_RETURN_NO_ERROR;

    if(mEisSupport == MFALSE)
    {
        EIS_LOG("mSensorIdx(%u) not support EIS",mSensorIdx);
        return EIS_RETURN_NO_ERROR;
    }
    DP_TRACE_CALL();

    MUINT32 i;

#if EIS_ALGO_READY
    //====== Check Config Data ======
    if(apEisConfig == NULL)
    {
        EIS_ERR("apEisConfig is NULL");
        err = EIS_RETURN_NULL_OBJ;
        return err;
    }

    //====== Setting EIS Plus Algo Process Data ======

    EIS_PLUS_SET_PROC_INFO_STRUCT eisPlusProcData;
    memset(&eisPlusProcData, 0, sizeof(eisPlusProcData));

    if (UNLIKELY(mDebugDump >= 1))
    {
        EIS_LOG("GMVx(%d)", apEisConfig->gmv_X);
        EIS_LOG("GMVy(%d)", apEisConfig->gmv_Y);
        EIS_LOG("ConfX(%d)", apEisConfig->confX);
        EIS_LOG("ConfY(%d)", apEisConfig->confY);
    }

    //> Set EisPlusProcData
    if (m_pLMVHal->GetLMVStatus())
    {
        eisPlusProcData.eis_info.eis_gmv_conf[0] = apEisConfig->confX;
        eisPlusProcData.eis_info.eis_gmv_conf[1] = apEisConfig->confY;
        eisPlusProcData.eis_info.eis_gmv[0]      = apEisConfig->gmv_X;
        eisPlusProcData.eis_info.eis_gmv[1]      = apEisConfig->gmv_Y;
    }
    else
    {
        //Set EIS algo to NEVER use GMV
    }

    //> get FE block number

    MUINT32 feBlockNum = 0;

    if(apEisConfig->srzOutW <= D1_WIDTH && apEisConfig->srzOutH <= D1_HEIGHT)
    {
        feBlockNum = 8;
    }
    else if(apEisConfig->srzOutW <= EIS_FE_MAX_INPUT_W && apEisConfig->srzOutH <= EIS_FE_MAX_INPUT_H)
    {
        feBlockNum = 16;
    }
    else
    {
        feBlockNum = 32;
    }

    eisPlusProcData.block_size   = feBlockNum;
    eisPlusProcData.imgiWidth    = apEisConfig->imgiW;
    eisPlusProcData.imgiHeight   = apEisConfig->imgiH;
    eisPlusProcData.CRZoWidth    = apEisConfig->crzOutW;
    eisPlusProcData.CRZoHeight   = apEisConfig->crzOutH;
    eisPlusProcData.SRZoWidth    = apEisConfig->srzOutW;
    eisPlusProcData.SRZoHeight   = apEisConfig->srzOutH;
    eisPlusProcData.oWidth       = apEisConfig->feTargetW;
    eisPlusProcData.oHeight      = apEisConfig->feTargetH;
    eisPlusProcData.TargetWidth  = apEisConfig->gpuTargetW;
    eisPlusProcData.TargetHeight = apEisConfig->gpuTargetH;
    eisPlusProcData.cropX        = apEisConfig->cropX;
    eisPlusProcData.cropY        = apEisConfig->cropY;

    //> config EIS Plus data

    mSrzOutW = apEisConfig->srzOutW;
    mSrzOutH = apEisConfig->srzOutH;

    if (UNLIKELY(mDebugDump >= 1))
    {
        EIS_LOG("eisPlusProcData");
        EIS_LOG("eis_gmv_conf[0](%d)",eisPlusProcData.eis_info.eis_gmv_conf[0]);
        EIS_LOG("eis_gmv_conf[1](%d)",eisPlusProcData.eis_info.eis_gmv_conf[1]);
        EIS_LOG("eis_gmv[0](%f)",eisPlusProcData.eis_info.eis_gmv[0]);
        EIS_LOG("eis_gmv[1](%f)",eisPlusProcData.eis_info.eis_gmv[1]);
        EIS_LOG("block_size(%u)",(MUINT32)eisPlusProcData.block_size);
        EIS_LOG("imgi(%d,%d)",eisPlusProcData.imgiWidth,eisPlusProcData.imgiHeight);
        EIS_LOG("CRZ(%d,%d)",eisPlusProcData.CRZoWidth,eisPlusProcData.CRZoHeight);
        EIS_LOG("SRZ(%d,%d)",eisPlusProcData.SRZoWidth,eisPlusProcData.SRZoHeight);
        EIS_LOG("FeTarget(%u,%u)",eisPlusProcData.oWidth,eisPlusProcData.oHeight);
        EIS_LOG("target(%d,%d)",eisPlusProcData.TargetWidth,eisPlusProcData.TargetHeight);
        EIS_LOG("crop(%d,%d)",eisPlusProcData.cropX,eisPlusProcData.cropY);
    }

    GYRO_MV_RESULT_INFO_STRUCT gyroMVresult;
    ExecuteGyroAlgo(apEisConfig, aTimeStamp, aExpTime, 0, gyroMVresult);
/////////////////////////////////////////////////////////////////////////////
    SensorData sensorData;
    if (mpSensorProvider->getLatestSensorData(SENSOR_TYPE_GYRO, sensorData))
    {
        for(i = 0; i < 3; i++)
        {
            //eisPlusProcData.sensor_info.AcceInfo[i] = sensorData.acceleration[i];
            eisPlusProcData.sensor_info.GyroInfo[i] = sensorData.gyro[i];
        }
    }

    eisPlusProcData.sensor_info.gyro_in_mv = gyroMVresult.mv;
    eisPlusProcData.sensor_info.valid_gyro_num  = gyroMVresult.valid_gyro_num;

    if (UNLIKELY(mDebugDump >= 1))
    {
        EIS_LOG("EN:(Acc,Gyro)=(%d,%d)",mAccEnable,mGyroEnable);
        EIS_LOG("EISPlus:Acc(%f,%f,%f)",eisPlusProcData.sensor_info.AcceInfo[0],eisPlusProcData.sensor_info.AcceInfo[1],eisPlusProcData.sensor_info.AcceInfo[2]);
        EIS_LOG("EISPlus:Gyro(%f,%f,%f)",eisPlusProcData.sensor_info.GyroInfo[0],eisPlusProcData.sensor_info.GyroInfo[1],eisPlusProcData.sensor_info.GyroInfo[2]);
    }

    //====== EIS Plus Algorithm ======

    err = m_pEisPlusAlg->EisPlusFeatureCtrl(EIS_PLUS_FEATURE_SET_PROC_INFO,&eisPlusProcData, NULL);
    if(err != S_EIS_PLUS_OK)
    {
        EIS_ERR("EisPlus:EIS_PLUS_FEATURE_SET_PROC_INFO fail(0x%x)",err);
        err = EIS_RETURN_API_FAIL;
        return err;
    }

    err = m_pEisPlusAlg->EisPlusMain(&mEisPlusResult);
    if(err != S_EIS_PLUS_OK)
    {
        EIS_ERR("EisPlus:EisMain fail(0x%x)",err);
        err = EIS_RETURN_API_FAIL;
        return err;
    }

    if (UNLIKELY(mDebugDump >= 1))
    {
        EIS_LOG("EisPlusMain- X: %d  Y: %d\n", mEisPlusResult.ClipX, mEisPlusResult.ClipY);
    }

    //====== Dynamic Debug ======
#if RECORD_WITHOUT_EIS_ENABLE
    if (g_AIMDump == 1)
    {
        mEisPlusResult.GridX[0] = 0;
        mEisPlusResult.GridX[1] = (apEisConfig->imgiW-1)*16;
        mEisPlusResult.GridX[2] = 0;
        mEisPlusResult.GridX[3] = (apEisConfig->imgiW-1)*16;

        mEisPlusResult.GridY[0] = 0;
        mEisPlusResult.GridY[1] = 0;
        mEisPlusResult.GridY[2] = (apEisConfig->imgiH-1)*16;
        mEisPlusResult.GridY[3] = (apEisConfig->imgiH-1)*16;
    }
#endif


    if (UNLIKELY(mDebugDump >= 2))
    {
        EIS_INF("EIS WARP MAP");
        for(MUINT32  i = 0; i < mGpuGridW*mGpuGridH; ++i)
        {
            EIS_LOG("X[%u]=%d",i,mEisPlusResult.GridX[i]);
            EIS_LOG("Y[%u]=%d",i,mEisPlusResult.GridY[i]);
        }
    }
 #endif

    if(mDebugDump >= 1)
    {
        EIS_LOG("-");
    }

    return EIS_RETURN_NO_ERROR;
}


MINT32 EisHalImp::DoFEFMEis(EIS_HAL_CONFIG_DATA *apEisConfig, IMAGE_BASED_DATA *imgBaseData, MINT64 aTimeStamp, MUINT32 aExpTime)
{
    MINT32 err = EIS_RETURN_NO_ERROR;
    MUINT32 i;

    if (UNLIKELY(mEisSupport == MFALSE))
    {
        EIS_LOG("mSensorIdx(%u) not support EIS",mSensorIdx);
        return EIS_RETURN_NO_ERROR;
    }

    DP_TRACE_CALL();

#if EIS_ALGO_READY
    //====== Check Config Data ======
    if(apEisConfig == NULL)
    {
        EIS_ERR("apEisConfig is NULL");
        err = EIS_RETURN_NULL_OBJ;
        return err;
    }

    //====== Setting EIS Plus Algo Process Data ======

    EIS_PLUS_SET_PROC_INFO_STRUCT eisPlusProcData;
    memset(&eisPlusProcData, 0, sizeof(eisPlusProcData));

    if (UNLIKELY(mDebugDump >= 1))
    {
        EIS_LOG("GMVx(%d)", apEisConfig->gmv_X);
        EIS_LOG("GMVy(%d)", apEisConfig->gmv_Y);
        EIS_LOG("ConfX(%d)", apEisConfig->confX);
        EIS_LOG("ConfY(%d)", apEisConfig->confY);
    }

    //> Set EisPlusProcData
    if (m_pLMVHal->GetLMVStatus())
    {
        eisPlusProcData.eis_info.eis_gmv_conf[0] = apEisConfig->confX;
        eisPlusProcData.eis_info.eis_gmv_conf[1] = apEisConfig->confY;
        eisPlusProcData.eis_info.eis_gmv[0]      = apEisConfig->gmv_X;
        eisPlusProcData.eis_info.eis_gmv[1]      = apEisConfig->gmv_Y;
    }
    else
    {
        //Set EIS algo to NEVER use GMV
    }

    //> get FE block number

    MUINT32 feBlockNum = 0;

    if(apEisConfig->srzOutW <= D1_WIDTH && apEisConfig->srzOutH <= D1_HEIGHT)
    {
        feBlockNum = 8;
    }
    else if(apEisConfig->srzOutW <= EIS_FE_MAX_INPUT_W && apEisConfig->srzOutH <= EIS_FE_MAX_INPUT_H)
    {
        feBlockNum = 16;
    }
    else
    {
        feBlockNum = 32;
    }

    eisPlusProcData.block_size   = feBlockNum;
    eisPlusProcData.imgiWidth    = apEisConfig->imgiW;
    eisPlusProcData.imgiHeight   = apEisConfig->imgiH;
    eisPlusProcData.CRZoWidth    = apEisConfig->crzOutW;
    eisPlusProcData.CRZoHeight   = apEisConfig->crzOutH;
    eisPlusProcData.SRZoWidth    = apEisConfig->srzOutW;
    eisPlusProcData.SRZoHeight   = apEisConfig->srzOutH;
    eisPlusProcData.oWidth       = apEisConfig->feTargetW;
    eisPlusProcData.oHeight      = apEisConfig->feTargetH;
    eisPlusProcData.TargetWidth  = apEisConfig->gpuTargetW;
    eisPlusProcData.TargetHeight = apEisConfig->gpuTargetH;
    eisPlusProcData.cropX        = apEisConfig->cropX;
    eisPlusProcData.cropY        = apEisConfig->cropY;
    eisPlusProcData.MVWidth      = mMVWidth;
    eisPlusProcData.MVHeight     = mMVHeight;

    //> config EIS Plus data

    mSrzOutW = apEisConfig->srzOutW;
    mSrzOutH = apEisConfig->srzOutH;

    eisPlusProcData.frame_t = (MUINT64) aTimeStamp;
    eisPlusProcData.ShutterTime = aExpTime;
    eisPlusProcData.process_idx = apEisConfig->process_idx;
    eisPlusProcData.process_mode = apEisConfig->process_mode;
    //> get FEO statistic
    eisPlusProcData.fe_result.fe_cur_info[0] = imgBaseData->fefmData->FE[0];
    eisPlusProcData.fe_result.fe_cur_info[1] = imgBaseData->fefmData->FE[1];
    eisPlusProcData.fe_result.fe_cur_info[2] = imgBaseData->fefmData->FE[2];

    eisPlusProcData.fe_result.fe_pre_info[0] = imgBaseData->fefmData->LastFE[0];
    eisPlusProcData.fe_result.fe_pre_info[1] = imgBaseData->fefmData->LastFE[1];
    eisPlusProcData.fe_result.fe_pre_info[2] = imgBaseData->fefmData->LastFE[2];

    eisPlusProcData.fm_result.fm_fw_idx[0] = imgBaseData->fefmData->ForwardFM[0];
    eisPlusProcData.fm_result.fm_fw_idx[1] = imgBaseData->fefmData->ForwardFM[1];
    eisPlusProcData.fm_result.fm_fw_idx[2] = imgBaseData->fefmData->ForwardFM[2];

    eisPlusProcData.fm_result.fm_bw_idx[0] = imgBaseData->fefmData->BackwardFM[0];
    eisPlusProcData.fm_result.fm_bw_idx[1] = imgBaseData->fefmData->BackwardFM[1];
    eisPlusProcData.fm_result.fm_bw_idx[2] = imgBaseData->fefmData->BackwardFM[2];

    eisPlusProcData.fm_result.fm_pre_res[0] = imgBaseData->fefmData->ForwardFMREG[0];
    eisPlusProcData.fm_result.fm_pre_res[1] = imgBaseData->fefmData->ForwardFMREG[1];
    eisPlusProcData.fm_result.fm_pre_res[2] = imgBaseData->fefmData->ForwardFMREG[2];

    eisPlusProcData.fm_result.fm_cur_res[0] = imgBaseData->fefmData->BackwardFMREG[0];
    eisPlusProcData.fm_result.fm_cur_res[1] = imgBaseData->fefmData->BackwardFMREG[1];
    eisPlusProcData.fm_result.fm_cur_res[2] = imgBaseData->fefmData->BackwardFMREG[2];

    if (UNLIKELY(mDebugDump >= 1))
    {
        EIS_LOG("eisPlusProcData");
        EIS_LOG("algMode(%d), algCounter(%d)",eisPlusProcData.process_mode, eisPlusProcData.process_idx);
        EIS_LOG("eis_gmv_conf[0](%d)",eisPlusProcData.eis_info.eis_gmv_conf[0]);
        EIS_LOG("eis_gmv_conf[1](%d)",eisPlusProcData.eis_info.eis_gmv_conf[1]);
        EIS_LOG("eis_gmv[0](%f)",eisPlusProcData.eis_info.eis_gmv[0]);
        EIS_LOG("eis_gmv[1](%f)",eisPlusProcData.eis_info.eis_gmv[1]);
        EIS_LOG("block_size(%u)",(MUINT32)eisPlusProcData.block_size);
        EIS_LOG("imgi(%d,%d)",eisPlusProcData.imgiWidth,eisPlusProcData.imgiHeight);
        EIS_LOG("CRZ(%d,%d)",eisPlusProcData.CRZoWidth,eisPlusProcData.CRZoHeight);
        EIS_LOG("SRZ(%d,%d)",eisPlusProcData.SRZoWidth,eisPlusProcData.SRZoHeight);
        EIS_LOG("FeTarget(%u,%u)",eisPlusProcData.oWidth,eisPlusProcData.oHeight);
        EIS_LOG("target(%d,%d)",eisPlusProcData.TargetWidth,eisPlusProcData.TargetHeight);
        EIS_LOG("crop(%d,%d)",eisPlusProcData.cropX,eisPlusProcData.cropY);
    }

    GYRO_MV_RESULT_INFO_STRUCT gyroMVresult;
    ExecuteGyroAlgo(apEisConfig, aTimeStamp, aExpTime, 0, gyroMVresult);

    SensorData sensorData;
    if (mpSensorProvider->getLatestSensorData(SENSOR_TYPE_GYRO, sensorData))
    {
        for(i = 0; i < 3; i++)
        {
            //eisPlusProcData.sensor_info.AcceInfo[i] = sensorData.acceleration[i];
            eisPlusProcData.sensor_info.GyroInfo[i] = sensorData.gyro[i];
        }
    }

    eisPlusProcData.sensor_info.gyro_in_mv = gyroMVresult.mv;
    eisPlusProcData.sensor_info.valid_gyro_num  = gyroMVresult.valid_gyro_num;

    if (UNLIKELY(mDebugDump >= 1))
    {
        EIS_LOG("EN:(Acc,Gyro)=(%d,%d)",mAccEnable,mGyroEnable);
        EIS_LOG("EISPlus:Acc(%f,%f,%f)",eisPlusProcData.sensor_info.AcceInfo[0],eisPlusProcData.sensor_info.AcceInfo[1],eisPlusProcData.sensor_info.AcceInfo[2]);
        EIS_LOG("EISPlus:Gyro(%f,%f,%f)",eisPlusProcData.sensor_info.GyroInfo[0],eisPlusProcData.sensor_info.GyroInfo[1],eisPlusProcData.sensor_info.GyroInfo[2]);
    }

    //====== EIS Plus Algorithm ======

    err = m_pEisPlusAlg->EisPlusFeatureCtrl(EIS_PLUS_FEATURE_SET_PROC_INFO,&eisPlusProcData, NULL);
    if(err != S_EIS_PLUS_OK)
    {
        EIS_ERR("EisPlus:EIS_PLUS_FEATURE_SET_PROC_INFO fail(0x%x)",err);
        err = EIS_RETURN_API_FAIL;
        return err;
    }

    err = m_pEisPlusAlg->EisPlusMain(&mEisPlusResult);
    if(err != S_EIS_PLUS_OK)
    {
        EIS_ERR("EisPlus:EisMain fail(0x%x)",err);
        err = EIS_RETURN_API_FAIL;
        return err;
    }

    if (UNLIKELY(mDebugDump >= 1))
    {
        EIS_LOG("EisPlusMain- X: %d  Y: %d\n", mEisPlusResult.ClipX, mEisPlusResult.ClipY);
    }

    //====== Dynamic Debug ======
#if RECORD_WITHOUT_EIS_ENABLE
    if (g_AIMDump == 1)
    {
        mEisPlusResult.GridX[0] = 0;
        mEisPlusResult.GridX[1] = (apEisConfig->imgiW-1)*16;
        mEisPlusResult.GridX[2] = 0;
        mEisPlusResult.GridX[3] = (apEisConfig->imgiW-1)*16;

        mEisPlusResult.GridY[0] = 0;
        mEisPlusResult.GridY[1] = 0;
        mEisPlusResult.GridY[2] = (apEisConfig->imgiH-1)*16;
        mEisPlusResult.GridY[3] = (apEisConfig->imgiH-1)*16;

        if(g_forecDejello == 1)
        {
            MUINT32 t,s;
            MUINT32 oWidth = apEisConfig->imgiW;
            MUINT32 oHeight = apEisConfig->imgiH;


            for(t=0;t<mGpuGridH;t++)
            {
                for(s=0;s<mGpuGridW;s++)
                {
                    float indx_x = (float)(oWidth-1)*s/(mGpuGridW-1);
                    float indx_y = (float)(oHeight-1)*t/(mGpuGridH-1);
                    mEisPlusResult.GridX[t*mGpuGridW+s] = (int)(indx_x*16);
                    mEisPlusResult.GridY[t*mGpuGridW+s] = (int)(indx_y*16);
                }
            }
        }
    }
#endif


    if (UNLIKELY(mDebugDump >= 2))
    {
        EIS_INF("EIS WARP MAP");
        for(MUINT32  i = 0; i < mGpuGridW*mGpuGridH; ++i)
        {
            EIS_LOG("X[%u]=%d",i,mEisPlusResult.GridX[i]);
            EIS_LOG("Y[%u]=%d",i,mEisPlusResult.GridY[i]);
        }
    }
 #endif


    if(mDebugDump >= 1)
    {
        EIS_LOG("-");
    }

    return EIS_RETURN_NO_ERROR;
}

MINT32 EisHalImp::DoRSCMEEis(EIS_HAL_CONFIG_DATA *apEisConfig, IMAGE_BASED_DATA *imgBaseData, MINT64 aTimeStamp, MUINT32 aExpTime , MUINT32 aLExpTime)
{
    MINT32 err = EIS_RETURN_NO_ERROR;
    MUINT32 i;

    if (UNLIKELY(mEisSupport == MFALSE))
    {
        EIS_LOG("mSensorIdx(%u) not support EIS",mSensorIdx);
        return EIS_RETURN_NO_ERROR;
    }

    DP_TRACE_CALL();

#if EIS_ALGO_READY
    //====== Check Config Data ======
    if(apEisConfig == NULL)
    {
        EIS_ERR("apEisConfig is NULL");
        err = EIS_RETURN_NULL_OBJ;
        return err;
    }

    //====== Setting EIS Plus Algo Process Data ======

    EIS_PLUS_SET_PROC_INFO_STRUCT eisPlusProcData;
    memset(&eisPlusProcData, 0, sizeof(eisPlusProcData));

    apEisConfig->lmvDataEnabled = imgBaseData->lmvData->enabled;
    if (apEisConfig->lmvDataEnabled)
    {
        apEisConfig->lmv_data = &imgBaseData->lmvData->data;
    }

    //> Set EisPlusProcData
    if (m_pLMVHal->GetLMVStatus()) // calibration
    {
        eisPlusProcData.eis_info.eis_gmv_conf[0] = apEisConfig->confX;
        eisPlusProcData.eis_info.eis_gmv_conf[1] = apEisConfig->confY;
        eisPlusProcData.eis_info.eis_gmv[0]      = apEisConfig->gmv_X;
        eisPlusProcData.eis_info.eis_gmv[1]      = apEisConfig->gmv_Y;
    }
    else
    {
        //Set EIS algo to NEVER use GMV
    }

    eisPlusProcData.imgiWidth    = apEisConfig->imgiW;
    eisPlusProcData.imgiHeight   = apEisConfig->imgiH;
    eisPlusProcData.CRZoWidth    = apEisConfig->crzOutW;
    eisPlusProcData.CRZoHeight   = apEisConfig->crzOutH;
    eisPlusProcData.SRZoWidth    = apEisConfig->srzOutW;
    eisPlusProcData.SRZoHeight   = apEisConfig->srzOutH;
    eisPlusProcData.oWidth       = apEisConfig->feTargetW;
    eisPlusProcData.oHeight      = apEisConfig->feTargetH;
    eisPlusProcData.TargetWidth  = apEisConfig->gpuTargetW;
    eisPlusProcData.TargetHeight = apEisConfig->gpuTargetH;
    eisPlusProcData.cropX        = apEisConfig->cropX;
    eisPlusProcData.cropY        = apEisConfig->cropY;
    eisPlusProcData.MVWidth      = mMVWidth;
    eisPlusProcData.MVHeight     = mMVHeight;

    // New for EIS 3.0
    eisPlusProcData.RSSoWidth       = apEisConfig->rssoWidth;
    eisPlusProcData.RSSoHeight      = apEisConfig->rssoHeight;
    eisPlusProcData.RSCLevel        = 6;
    eisPlusProcData.mv_idx          = mCurSensorIdx;
    eisPlusProcData.Trs             = mSensorInfo[mCurSensorIdx].mRecordParameter[0];
    eisPlusProcData.FovAlignWidth   = apEisConfig->fov_align_Width;
    eisPlusProcData.FovAlignHeight  = apEisConfig->fov_align_Height;
    eisPlusProcData.WarpGrid[0].x   = apEisConfig->warp_grid[0].x;
    eisPlusProcData.WarpGrid[1].x   = apEisConfig->warp_grid[1].x;
    eisPlusProcData.WarpGrid[2].x   = apEisConfig->warp_grid[2].x;
    eisPlusProcData.WarpGrid[3].x   = apEisConfig->warp_grid[3].x;
    eisPlusProcData.WarpGrid[0].y   = apEisConfig->warp_grid[0].y;
    eisPlusProcData.WarpGrid[1].y   = apEisConfig->warp_grid[1].y;
    eisPlusProcData.WarpGrid[2].y   = apEisConfig->warp_grid[2].y;
    eisPlusProcData.WarpGrid[3].y   = apEisConfig->warp_grid[3].y;

    if( mFSCInfo.isEnabled &&
        imgBaseData->fscData != NULL )
    {
        eisPlusProcData.FSCProcWidth     = imgBaseData->fscData->procWidth;
        eisPlusProcData.FSCProcHeight    = imgBaseData->fscData->procHeight;
        eisPlusProcData.FSCScalingFactor = imgBaseData->fscData->scalingFactor;
    }

    //> config EIS Plus data

    mSrzOutW = apEisConfig->srzOutW;
    mSrzOutH = apEisConfig->srzOutH;

    eisPlusProcData.frame_t = (MUINT64) aTimeStamp;
    eisPlusProcData.ShutterTime = aExpTime;
    eisPlusProcData.process_idx = apEisConfig->process_idx;
    eisPlusProcData.process_mode = apEisConfig->process_mode;

    if (UNLIKELY(mDebugDump >= 1))
    {
        EIS_LOG("eisPlusProcData");
        EIS_LOG("algMode(%d), algCounter(%d)",eisPlusProcData.process_mode, eisPlusProcData.process_idx);
        EIS_LOG("eis_gmv_conf[0](%d)",eisPlusProcData.eis_info.eis_gmv_conf[0]);
        EIS_LOG("eis_gmv_conf[1](%d)",eisPlusProcData.eis_info.eis_gmv_conf[1]);
        EIS_LOG("eis_gmv[0](%f)",eisPlusProcData.eis_info.eis_gmv[0]);
        EIS_LOG("eis_gmv[1](%f)",eisPlusProcData.eis_info.eis_gmv[1]);
        EIS_LOG("block_size(%u)",(MUINT32)eisPlusProcData.block_size);
        EIS_LOG("imgi(%d,%d)",eisPlusProcData.imgiWidth,eisPlusProcData.imgiHeight);
        EIS_LOG("CRZ(%d,%d)",eisPlusProcData.CRZoWidth,eisPlusProcData.CRZoHeight);
        EIS_LOG("SRZ(%d,%d)",eisPlusProcData.SRZoWidth,eisPlusProcData.SRZoHeight);
        EIS_LOG("FeTarget(%u,%u)",eisPlusProcData.oWidth,eisPlusProcData.oHeight);
        EIS_LOG("target(%d,%d)",eisPlusProcData.TargetWidth,eisPlusProcData.TargetHeight);
        EIS_LOG("crop(%d,%d)",eisPlusProcData.cropX,eisPlusProcData.cropY);
        EIS_LOG("RSSoWidth(%d), RSSoHeight(%d), mv_idx(%d), Trs(%f)",
                eisPlusProcData.RSSoWidth, eisPlusProcData.RSSoHeight,
                eisPlusProcData.mv_idx, eisPlusProcData.Trs);
        EIS_LOG("FovAlignWidth(%d), FovAlignHeight(%d)", eisPlusProcData.FovAlignWidth, eisPlusProcData.FovAlignHeight);
        EIS_LOG("WarpGridX[0]=%f, WarpGridX[1]=%f, WarpGridX[2]=%f, WarpGridX[3]=%f",
                eisPlusProcData.WarpGrid[0].x, eisPlusProcData.WarpGrid[1].x,
                eisPlusProcData.WarpGrid[2].x, eisPlusProcData.WarpGrid[3].x);
        EIS_LOG("WarpGridY[0]=%f, WarpGridY[1]=%f, WarpGridY[2]=%f, WarpGridY[3]=%f",
                eisPlusProcData.WarpGrid[0].y, eisPlusProcData.WarpGrid[1].y,
                eisPlusProcData.WarpGrid[2].y, eisPlusProcData.WarpGrid[3].y);
        EIS_LOG("FSCProc(%dx%d)Factor@(%p)",
                eisPlusProcData.FSCProcWidth,
                eisPlusProcData.FSCProcHeight,
                eisPlusProcData.FSCScalingFactor);
    }

    GYRO_MV_RESULT_INFO_STRUCT gyroMVresult;
    ExecuteGyroAlgo(apEisConfig, aTimeStamp, aExpTime, aLExpTime, gyroMVresult, imgBaseData);

    SensorData sensorData;
    if (mpSensorProvider->getLatestSensorData(SENSOR_TYPE_GYRO, sensorData))
    {
        for(i = 0; i < 3; i++)
        {
            //eisPlusProcData.sensor_info.AcceInfo[i] = sensorData.acceleration[i];
            eisPlusProcData.sensor_info.GyroInfo[i] = sensorData.gyro[i];
        }
    }

    eisPlusProcData.sensor_info.gyro_in_mv = gyroMVresult.mv;
    eisPlusProcData.sensor_info.valid_gyro_num  = gyroMVresult.valid_gyro_num;

    //> config EIS 3.0 RSCME relative configurations
    eisPlusProcData.sensor_info.fbuf_in_rsc_mv  = imgBaseData->rscData->RSCME_mv;
    eisPlusProcData.sensor_info.fbuf_in_rsc_var = imgBaseData->rscData->RSCME_var;

    if (UNLIKELY(mDebugDump >= 1))
    {
        EIS_LOG("EN:(Acc,Gyro)=(%d,%d)",mAccEnable,mGyroEnable);
        EIS_LOG("EISPlus:Acc(%f,%f,%f)",eisPlusProcData.sensor_info.AcceInfo[0],eisPlusProcData.sensor_info.AcceInfo[1],eisPlusProcData.sensor_info.AcceInfo[2]);
        EIS_LOG("EISPlus:Gyro(%f,%f,%f)",eisPlusProcData.sensor_info.GyroInfo[0],eisPlusProcData.sensor_info.GyroInfo[1],eisPlusProcData.sensor_info.GyroInfo[2]);
    }

    //====== EIS Plus Algorithm ======

    err = m_pEisPlusAlg->EisPlusFeatureCtrl(EIS_PLUS_FEATURE_SET_PROC_INFO,&eisPlusProcData, NULL);
    if(err != S_EIS_PLUS_OK)
    {
        EIS_ERR("EisPlus:EIS_PLUS_FEATURE_SET_PROC_INFO fail(0x%x)",err);
        err = EIS_RETURN_API_FAIL;
        return err;
    }

    err = m_pEisPlusAlg->EisPlusMain(&mEisPlusResult);
    if(err != S_EIS_PLUS_OK)
    {
        EIS_ERR("EisPlus:EisMain fail(0x%x)",err);
        err = EIS_RETURN_API_FAIL;
        return err;
    }

    // if (UNLIKELY(mDebugDump >= 1))
    {
        EIS_LOG("EisPlusMain- X: %d  Y: %d\n", mEisPlusResult.ClipX, mEisPlusResult.ClipY);
    }

    //====== Dynamic Debug ======
#if RECORD_WITHOUT_EIS_ENABLE
    if (g_AIMDump == 1)
    {
        MUINT32 t,s;
        MUINT32 oWidth = apEisConfig->imgiW;
        MUINT32 oHeight = apEisConfig->imgiH;

        for(t=0;t<mGpuGridH;t++)
        {
            for(s=0;s<mGpuGridW;s++)
            {
                float indx_x = (float)(oWidth-1)*s/(mGpuGridW-1);
                float indx_y = (float)(oHeight-1)*t/(mGpuGridH-1);
                mEisPlusResult.GridX[t*mGpuGridW+s] = (int)(indx_x*32);
                mEisPlusResult.GridY[t*mGpuGridW+s] = (int)(indx_y*32);
            }
        }
    }
#endif

    if (mBitTrueDump)
    {
        if( mFSCInfo.isEnabled )
        {
            //--dump fsc warp data
            char LogFileName[100];
            MUINT32 fscsize = sizeof(MINT32)*mFSCInfo.numSlices;
            sprintf(LogFileName, FSC_DUMP_FOLDER_PATH"/fsc_warp_data_t%" PRIi64 "_%d.bin", aTimeStamp, fscsize);
            DumpToFile(LogFileName, (unsigned char *)imgBaseData->fscData->scalingFactor, fscsize);
        }
        dumpBitTrue(&mEisPlusResult, &eisPlusProcData, imgBaseData->rscData, NULL);
    }

    if (UNLIKELY(mDebugDump >= 2))
    {
        EIS_INF("EIS WARP MAP");
        for(MUINT32  i = 0; i < mGpuGridW*mGpuGridH; ++i)
        {
            EIS_LOG("X[%u]=%d",i,mEisPlusResult.GridX[i]);
            EIS_LOG("Y[%u]=%d",i,mEisPlusResult.GridY[i]);
        }
        if( mFSCInfo.isEnabled )
        {
            MUINT32 *ptr = (MUINT32*) eisPlusProcData.FSCScalingFactor;
            for( MUINT32 i = 0; i < mFSCInfo.numSlices; ++i )
            {
                EIS_LOG("FSC[%u]=%d", i, ptr[i]);
            }
        }
    }
 #endif


    if(mDebugDump >= 1)
    {
        EIS_LOG("-");
    }

    return EIS_RETURN_NO_ERROR;
}



/*******************************************************************************
*
********************************************************************************/
MVOID EisHalImp::SetEisPlusWarpInfo(MINT32 * const aGridX, MINT32 * const aGridY)
{
    mEisPlusResult.GridX = aGridX;
    mEisPlusResult.GridY = aGridY;

    if(mDebugDump >= 1)
    {
        EIS_LOG("[IN]grid VA(0x%p,0x%p)", (MVOID*)aGridX, (MVOID*)aGridY);
        EIS_LOG("[MEMBER]grid VA(0x%p,0x%p)",(MVOID*)mEisPlusResult.GridX, (MVOID*)mEisPlusResult.GridY);
    }
}


/*******************************************************************************
*
********************************************************************************/
MVOID EisHalImp::GetEisPlusResult(P_EIS_PLUS_RESULT_INFO_STRUCT apEisPlusResult)
{
    if(mEisSupport == MFALSE)
    {
        EIS_LOG("mSensorIdx(%u) not support EIS",mSensorIdx);
        return;
    }

    apEisPlusResult->ClipX = mEisPlusResult.ClipX;
    apEisPlusResult->ClipY = mEisPlusResult.ClipY;

    if(mDebugDump >= 1)
    {
        EIS_LOG("Clip(%u,%u)",apEisPlusResult->ClipX,apEisPlusResult->ClipY);
    }
}


#if EIS_ALGO_READY
/*******************************************************************************
*
********************************************************************************/
MVOID EisHalImp::GetEisPlusCustomize(EIS_PLUS_TUNING_PARA_STRUCT *a_pTuningData, const EIS_HAL_CONFIG_DATA *apEisConfig)
{
    if (mDebugDump >= 1)
    {
        EIS_LOG("+");
    }

    configMVNumber(*apEisConfig, &mMVWidth, &mMVHeight);

    EIS_PLUS_Customize_Para_t customSetting;
    EISCustom::getEISPlusData(&customSetting, mEisInfo.videoConfig);

    a_pTuningData->warping_mode           = static_cast<MINT32>(customSetting.warping_mode);
    a_pTuningData->effort                 = 2;  // limit to 400 points
    a_pTuningData->search_range_x         = customSetting.search_range_x;
    a_pTuningData->search_range_y         = customSetting.search_range_y;
    a_pTuningData->crop_ratio             = customSetting.crop_ratio;
    a_pTuningData->gyro_still_time_th = customSetting.gyro_still_time_th;
    a_pTuningData->gyro_max_time_th = customSetting.gyro_max_time_th;
    a_pTuningData->gyro_similar_th = customSetting.gyro_similar_th;
    a_pTuningData->stabilization_strength = customSetting.stabilization_strength;

    if (mDebugDump >= 1)
    {
        EIS_LOG("-");
    }
}

/*******************************************************************************
*
********************************************************************************/
MVOID EisHalImp::GetEis25Customize(EIS25_TUNING_PARA_STRUCT *a_pTuningData, const EIS_HAL_CONFIG_DATA *apEisConfig)
{
    (void) apEisConfig;

    if (mDebugDump >= 1)
    {
        EIS_LOG("+");
    }

    EIS25_Customize_Tuning_Para_t customSetting;

    EISCustom::getEIS25Data(&customSetting);

    a_pTuningData->en_dejello             = customSetting.en_dejello;
    a_pTuningData->stabilization_strength = customSetting.stabilization_strength;
    a_pTuningData->stabilization_level    = customSetting.stabilization_level;
    a_pTuningData->gyro_still_mv_th       = customSetting.gyro_still_mv_th;
    a_pTuningData->gyro_still_mv_diff_th  = customSetting.gyro_still_mv_diff_th;

    if (mDebugDump >= 1)
    {
        EIS_LOG("-");
    }
}

/*******************************************************************************
*
********************************************************************************/
MVOID EisHalImp::GetEis30Customize(EIS30_TUNING_PARA_STRUCT *a_pTuningData, const EIS_HAL_CONFIG_DATA *apEisConfig)
{
    (void) apEisConfig;

    if (mDebugDump >= 1)
    {
        EIS_LOG("+");
    }

    EIS30_Customize_Tuning_Para_t customSetting;

    EISCustom::getEIS30Data(&customSetting);

    a_pTuningData->stabilization_strength = customSetting.stabilization_strength;
    a_pTuningData->stabilization_level    = customSetting.stabilization_level;
    a_pTuningData->gyro_still_mv_th       = customSetting.gyro_still_mv_th;
    a_pTuningData->gyro_still_mv_diff_th  = customSetting.gyro_still_mv_diff_th;

    if (mDebugDump >= 1)
    {
        EIS_LOG("-");
    }
}
#endif

MVOID EisHalImp::dumpBitTrue(EIS_PLUS_RESULT_INFO_STRUCT* eisResult, EIS_PLUS_SET_PROC_INFO_STRUCT* eisProcInfo,
    RSCME_PACKAGE* rscData, GYRO_SET_PROC_INFO_STRUCT* gyroProcData)
{
    MUINT32 WriteCnt = 0;

    //dump warpmap
    if (eisResult != NULL)
    {
        FILE* pWarpmapFp = fopen("storage/sdcard0/bittrue/WARPMAP.bin", "ab");
        if (pWarpmapFp != NULL)
        {
            WriteCnt = fwrite(mEisPlusResult.GridX, sizeof(MINT32)*mGpuGridH*mGpuGridW, 1, pWarpmapFp);
            WriteCnt = fwrite(mEisPlusResult.GridY, sizeof(MINT32)*mGpuGridH*mGpuGridW, 1, pWarpmapFp);
            fflush(pWarpmapFp);
            fclose(pWarpmapFp);
        }
        else
        {
            EIS_ERR("pWarpmapFp == NULL!");
        }
    }

    //dump eisPlusProcInfo
    if (eisProcInfo != NULL)
    {
        FILE* pEisProcFp = fopen("storage/sdcard0/bittrue/EIS_PLUS_PROC_DATA.bin", "ab");
        if (pEisProcFp != NULL)
        {
            WriteCnt = fwrite(eisProcInfo, sizeof(EIS_PLUS_SET_PROC_INFO_STRUCT), 1, pEisProcFp);
            fflush(pEisProcFp);
            fclose(pEisProcFp);
        }
        else
        {
            EIS_ERR("pEisProcFp == NULL!");
        }
    }

    //dump RSCME buffer
    if (rscData != NULL && eisProcInfo != NULL)
    {
        FILE* pRSCFp = fopen("storage/sdcard0/bittrue/RSCME_BUFFER.bin", "ab");
        if (pRSCFp != NULL)
        {
            WriteCnt = fwrite(rscData->RSCME_mv, sizeof(MUINT8)*eisProcInfo->RSSoWidth*eisProcInfo->RSSoHeight, 1, pRSCFp);
            WriteCnt = fwrite(rscData->RSCME_var, sizeof(MUINT8)*eisProcInfo->RSSoWidth*eisProcInfo->RSSoHeight, 1, pRSCFp);
            fflush(pRSCFp);
            fclose(pRSCFp);
        }
        else
        {
            EIS_ERR("pRSCFp == NULL!");
        }
    }

    //dump gyroProcData
    if (gyroProcData != NULL)
    {
        FILE* pGyroSetFp = fopen("storage/sdcard0/bittrue/GYRO_SET_PROC_DATA.bin", "ab");
        if (pGyroSetFp != NULL)
        {
            WriteCnt = fwrite(gyroProcData, sizeof(GYRO_SET_PROC_INFO_STRUCT), 1, pGyroSetFp);
            WriteCnt = fwrite(gyroProcData->param, sizeof(mSensorInfo[mCurSensorIdx].mRecordParameter), 1, pGyroSetFp);
            WriteCnt = fwrite(gyroProcData->gyro_t_frame, sizeof(MUINT64)*GYRO_DATA_PER_FRAME, 1, pGyroSetFp);
            WriteCnt = fwrite(gyroProcData->gyro_xyz_frame, sizeof(MDOUBLE)*GYRO_DATA_PER_FRAME, 1, pGyroSetFp);
            fflush(pGyroSetFp);
            fclose(pGyroSetFp);
        }
        else
        {
            EIS_ERR("pGyroSetFp == NULL!");
        }
    }
}



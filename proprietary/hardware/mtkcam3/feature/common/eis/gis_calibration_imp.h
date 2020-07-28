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

/**
* @file eis_hal_imp.h
*
* EIS Hal Implementation Header File
*
*/


#ifndef _GIS_CALIBRATION_IMP_H_
#define _GIS_CALIBRATION_IMP_H_

#include <Mutex.h>
#include <Condition.h>

#include <mtkcam3/feature/eis/gis_calibration.h>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <MTKGyro.h>

#define LIKELY(x)       __builtin_expect(!!(x), 1)
#define UNLIKELY(x)     __builtin_expect(!!(x), 0)

#define MAX_MEMORY_SIZE         (40)
#define TSRECORD_MAXSIZE        (108000)
#define EIS_ACCELEROMETER       (1)
#define EIS_GYROSCOPE           (2)
#define GYRO_DATA_PER_FRAME     (100)
#define MAX_SENSOR_ID           (10)

typedef struct EIS_GyroRecord_t
{
    MINT32  id;
    MFLOAT x;
    MFLOAT y;
    MFLOAT z;
    MINT64 ts;
}EIS_GyroRecord;


class EIS_SensorInfo
{
    public:
    MUINT32 mUsed;
    MUINT32 mSensorDev;
    MINT64  mtRSTime;
    MUINT32 mSensorPixelClock;
    MUINT32 mSensorLinePixel;
    MUINT32 mDefWidth;
    MUINT32 mDefHeight;
    MUINT32 mDefCrop;
    MDOUBLE mRecordParameter[6];

    EIS_SensorInfo()
    {
        MUINT32 i;
        mUsed = 0;
        mSensorDev = 0;
        mtRSTime = 0;
        mSensorPixelClock = 0;
        mSensorLinePixel = 0;
        mDefWidth = 0;
        mDefHeight = 0;
        mDefCrop = 0;
        for (i = 0;i<6;i++)
        {
            mRecordParameter[i] = 0;
        }
    }
};


typedef struct GyroEISStatistics_t
{
    MINT32  eis_data[EIS_WIN_NUM*4];
    MUINT64 ts;
}GyroEISStatistics;


class GyroItem
{
public:
    Mutex mGyroQueueLock;
    Condition mWaitGyroCond;
    queue<EIS_GyroRecord> mGyroDataQueue;
    MUINT32 mGyroCount;
    MUINT32 mGyroReverse;
    MINT64  mLastGyroTimestamp;
    GyroItem(): mGyroQueueLock(::Mutex::PRIVATE), mWaitGyroCond(::Condition::PRIVATE),
                mGyroCount(0), mGyroReverse(0), mLastGyroTimestamp(0)
    {}
};


struct NVRAM_CAMERA_FEATURE_STRUCT_t;
struct NVRAM_CAMERA_FEATURE_GIS_STRUCT_t;

class LMVHal;
namespace NSCam{
namespace Utils{
class SensorProvider;
}
}

/**
  *@class GisCalibrationImp
  *@brief Implementation of GisCalibration class
*/
class GisCalibrationImp : public GisCalibration
{
private:
    GisCalibrationImp(const GisCalibrationImp&);
    GisCalibrationImp();
    GisCalibrationImp &operator = (const GisCalibrationImp&);

public:
    /**
         *@brief Destroy EisHal object
         *@param[in] userName : user name,i.e. who destroy EIS HAL object
       */
    virtual MVOID DestroyInstance(char const *userName);


    /**
         *@brief EisHalImp constructor
       */
    GisCalibrationImp(const MUINT32 &aSensorIdx, const MUINT32 eisFactor);

    /**
         *@brief EisHalImp destructor
       */

    /**
         *@brief Initialization function
         *@param[in] aSensorIdx : sensor index
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 Init();

    /**
         *@brief Unitialization function
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 Uninit();

    /**
         *@brief Configure EIS
         *@details Use this API after pass1/pass2 config and before pass1/pass2 start
         *@param[in] aEisPass : indicate pass1 or pass2
         *@param[in] aEisConfig : EIS config data
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
     virtual MINT32 ConfigCalibration(const LMV_HAL_CONFIG_DATA &aLMVConfig);

    /**
         *@brief Execute EIS
         *@param[in] aEisPass : indicate pass1 or pass2
         *@param[in] apEisConfig : EIS config data, mainly for pass2
         *@param[in] aTimeStamp : time stamp of pass1 image
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 DoCalibration(LMV_HAL_CONFIG_DATA *apLMVConfig = NULL, MINT64 aTimeStamp = -1, MINT64 aExpTime = 0);

private:

    /**
         *@brief Get sensor info
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    MINT32 GetSensorInfo(MUINT32 sensorID);
    MINT32 CalcSensorTrs(MUINT32 sensorID, MUINT32 sensorHeight);

    /**
         *@brief Create IMem buffer
         *@param[in,out] memSize : memory size, will align to L1 cache
         *@param[in] bufCnt : how many buffer
         *@param[in,out] bufInfo : IMem object
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    MINT32 CreateMemBuf(MUINT32 memSize, android::sp<IImageBuffer>& spImageBuf);
    /**
         *@brief Destroy IMem buffer
         *@param[in] bufCnt : how many buffer
         *@param[in,out] bufInfo : IMem object
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    MINT32 DestroyMemBuf(android::sp<IImageBuffer>& spImageBuf);
    MINT32 EnableSensor();
    MVOID  GetSensorData(GYRO_SET_PROC_INFO_STRUCT& gyroSetProcData);

    /***************************************************************************************/

    volatile MINT32 mUsers;
    mutable Mutex mLock;
    mutable Mutex mP1Lock;

    MTKGyro* m_pGisAlg;
    android::sp<IImageBuffer> m_pGisWorkBuf;

    // EIS result
    MUINT32 mDoEisCount;    //Vent@20140427: Add for EIS GMV Sync Check.

    // member variable
    MUINT32 mFrameCnt;
    MUINT32 mIsEisPlusConfig;
    MBOOL   mEisSupport;

    // Gyroscope SensorProvider
    android::sp<SensorProvider> mpSensorProvider;

    // Sensor Hal
    IHalSensorList  *m_pHalSensorList;
    IHalSensor      *m_pHalSensor;

    const MUINT32 mSensorIdx;
    const MUINT32 mEisFactor;
    MBOOL mGyroEnable;
    MBOOL mAccEnable;
    MUINT64 mTsForAlgoDebug;
    NVRAM_CAMERA_FEATURE_STRUCT_t* m_pNvram;
    MUINT32 mChangedInCalibration;
    MUINT32 mGisInputW;
    MUINT32 mGisInputH;
    MBOOL   mNVRAMRead;
    MINT64  mSleepTime;
    MBOOL   mSkipWaitGyro;
    MBOOL   mbLastCalibration;

    MUINT32 mCurSensorIdx;
    EIS_SensorInfo mSensorInfo[MAX_SENSOR_ID];

    MBOOL   mbEMSaveFlag;
    NVRAM_CAMERA_FEATURE_GIS_STRUCT_t* m_pNVRAM_defParameter;

    LMVHal *m_pLMVHal;
    static GyroItem mGyroData[MAX_SENSOR_ID];
    MUINT64 gyro_t_frame_array[GYRO_DATA_PER_FRAME];
    MDOUBLE gyro_xyz_frame_array[GYRO_DATA_PER_FRAME*3];

public:
    static MINT32 mEISInterval;
    static MINT32 mEMEnabled;
    static MINT32 mDebugDump;
};




#endif


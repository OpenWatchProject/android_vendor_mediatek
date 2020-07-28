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
#define MTK_LOG_ENABLE 1
#include "camera_custom_zvhdr.h"
#include <math.h>
#include <string.h>
#include <cstdio>
#include <cstdlib>
#include <cutils/properties.h>
#include <cutils/log.h> // For XLOG?().
#include <utils/Errors.h>

#define MY_LOGD(fmt, arg...)  ALOGD(fmt, ##arg)
#define MY_LOGE(fmt, arg...)  ALOGE(fmt, ##arg)

#define MY_LOG_IF(cond, ...)      do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)

#ifndef CLAMP
#define CLAMP(x,min,max)                        (((x) > (max)) ? (max) : (((x) < (min)) ? (min) : (x)))
#endif

#ifndef SIGN
#define SIGN(a)             ((MINT64)(a)>0?1:-1)
#endif


/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/


/**************************************************************************
 *     E N U M / S T R U C T / T Y P E D E F    D E C L A R A T I O N     *
 **************************************************************************/
#define CUST_ZVHDR_ANTIBANDING_BASE_50HZ        (9995)
#define CUST_ZVHDR_ANTIBANDING_BASE_60HZ        (8335)
#define CUST_ZVHDR_ANTIBANDING_BASE_TOLERRANCE  (10)        // %

#define CUST_ZVHDR_ISPGAIN_BASE                 (1024)      // 1x = 1023
#define CUST_ZVHDR_LV_INDEX_UNIT                (10)        // 1.0 LV
#define CUST_ZVHDR_LV_INDEX_MIN                 (0)         // LV 0
#define CUST_ZVHDR_LV_INDEX_MAX                 (18)        // LV 18
#define CUST_ZVHDR_LV_INDEX_NUM                 ((CUST_ZVHDR_LV_INDEX_MAX - CUST_ZVHDR_LV_INDEX_MIN) + 1)
#define CUST_ZVHDR_RATIO_TOLERRANCE             (3)

/*
*   Notice, we do not sugget use ratio = 8, since not all sensors support
*/
                                                    //LV 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18
//static MINT32 i4DynamicRatio[CUST_ZVHDR_LV_INDEX_NUM] = {1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 4, 4, 4, 4, 4, 4, 4};
                                                   //LV 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18
//static MINT32 i4DeFlickerLV[CUST_ZVHDR_LV_INDEX_NUM] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0};


/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static MBOOL bVHDRResetFlag = MFALSE;
static MBOOL m_bDebugEnable = MFALSE;
static MINT32 i4PvOnOff = 0;
static MINT32 i4PvMotionOn = 0;
static MINT32 i4PvRatio = 0;
static MINT32 i4PvRMGt = 0;
static MINT32 i4HdrAlgoRMGt = 0;
static MINT32 i4FlickerExpoBase[2][13] =
{
    /*
    *   50Hz
    */
    {
        9995, 20002, 29997, 40004, 49998, 60005, 69998, 80005, 89995, 100005,
        109995,120002, 12997
    }
    ,
    /*
    *   60Hz
    */
    {
        8328, 16666, 24994, 33332, 41659, 49998, 58325, 66664, 74997, 83330,
        91663,99996, 108329
    }
};

#if 0
static MBOOL m_bRMGTHSmoothEnable = MTRUE;
#endif

/**************************************************************************
 *       P R I V A T E    F U N C T I O N    D E C L A R A T I O N        *
 **************************************************************************/

static MVOID Ratio2AEInfo(const ZVHDRExpSettingInputParam_T& rInput, ZVHDRExpSettingOutputParam_T& rOutput, ZVHDRSmoothInfo_T& m_HdrSmoothInfo);



MVOID getZVHDRExpSetting(const ZVHDRExpSettingInputParam_T& rInput, ZVHDRExpSettingOutputParam_T& rOutput)
{

#if 0
    ZVHDRSmoothInfo_T m_HdrSmoothInfo;
    MINT32 i4Ratio = 0, i4DebugRatio = 0;

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.zvhdr-pline.enable", value, "1");
    m_bDebugEnable = (MBOOL)atoi(value);

    property_get("vendor.debug.ratio.enable", value, "0");
    i4DebugRatio = (MINT32)atoi(value);


    if (bVHDRResetFlag)
    {
        i4PvOnOff = 0;
        i4PvMotionOn = 0;
        i4PvRatio = 0;
        i4PvRMGt = 0;
        i4HdrAlgoRMGt = 0;
    }

    rOutput.bEnableWorkaround = MFALSE;
    rOutput.HdrEvSetting = rInput.HdrEvSetting;

    if (rInput.HdrEvSetting.i4HdrFrom)
    {
        i4HdrAlgoRMGt = rInput.HdrEvSetting.i4RMGt;
        rOutput.HdrEvSetting.i4HdrFrom = 0;
        MY_LOG_IF(m_bDebugEnable,"[%s] Update from HDR AE ALGO i4HdrAlgoRMGt: %d\n", __FUNCTION__, i4HdrAlgoRMGt);
    }

    memset(&m_HdrSmoothInfo, 0, sizeof(m_HdrSmoothInfo));
    m_HdrSmoothInfo.i4FinalRMGt = rInput.HdrEvSetting.i4RMGt;
    m_HdrSmoothInfo.i4HdrOnOff = i4PvOnOff;

    smoothHdrOnOff(rInput, rOutput, m_HdrSmoothInfo);

    calIsoRatio(rInput, rOutput, m_HdrSmoothInfo);

    calMotionRatio(rInput, rOutput, m_HdrSmoothInfo);

    smoothRatioRMGt(rInput, rOutput, m_HdrSmoothInfo);

    Ratio2AEInfo(rInput, rOutput, m_HdrSmoothInfo);

    //calAntiFlickerRatio(rInput, rOutput, m_HdrSmoothInfo);

    if (i4DebugRatio != 0) {

        m_HdrSmoothInfo.i4FinalRatio = i4DebugRatio;
        rOutput.u4SEExpTimeInUS = rOutput.u4LEExpTimeInUS * 100 / m_HdrSmoothInfo.i4FinalRatio;
        MY_LOG_IF(m_bDebugEnable, "[%s] FinalRatio = DebugRatio = %d !!!!!!!!!!\n",  __FUNCTION__, i4DebugRatio);
    }

#if 0
    property_get("vendor.debug.zvhdr-rmgthsmooth.enable", value, "1");
    m_bRMGTHSmoothEnable = (MBOOL)atoi(value);

    if (!m_bRMGTHSmoothEnable) {
        if(i4IsoRatio == 1)
            i4IsoRatio = 2;
    }

    if (!m_bRMGTHSmoothEnable) {
        if (rOutput.u4SEExpTimeInUS != 0)
        {
            i4RatioCheck = (rOutput.u4LEExpTimeInUS * 100) / rOutput.u4SEExpTimeInUS;
            i4RatioCheck = (i4RatioCheck + 50) / 100;
            /*
             *   If Ratio equals 1, change ratio to 2 to prevent RMG hunting
             */
            if(i4RatioCheck == 1)
            {
                rOutput.u4SEExpTimeInUS = rOutput.u4LEExpTimeInUS / 2;
            }
        }
    }

    if (m_bRMGTHSmoothEnable && (rOutput.HdrEvSetting.i4RMGk != 4095)) {
        RMGTHSmooth(i4FinalRatio, rOutput, u4AnitBandingBase);
    }
#endif

#if 0
    rOutput.u4LE_SERatio_x100 = rOutput.u4LEExpTimeInUS *100 / rOutput.u4SEExpTimeInUS;
    if(rOutput.u4LE_SERatio_x100 < 100) {
        rOutput.u4LE_SERatio_x100 = 100;
        rOutput.u4SEExpTimeInUS = rOutput.u4LEExpTimeInUS;
    }
#endif

    rOutput.HdrEvSetting.i4HdrExposure[0] = rOutput.u4LEExpTimeInUS;
    rOutput.HdrEvSetting.i4HdrExposure[1] = rOutput.u4SEExpTimeInUS;
    rOutput.HdrEvSetting.i4HdrIspGain[0] = rOutput.u4LEISPGain;
    rOutput.HdrEvSetting.i4HdrIspGain[1] = rOutput.u4SEISPGain;
    rOutput.HdrEvSetting.i4HdrAfeGain[0] = rOutput.u4LESensorGain;
    rOutput.HdrEvSetting.i4HdrAfeGain[1] = rOutput.u4SESensorGain;

    i4PvOnOff = rOutput.HdrEvSetting.i4HdrOnOff;
    i4PvRatio = rOutput.u4LE_SERatio_x100;
    i4PvRMGt = rOutput.HdrEvSetting.i4RMGt;
    i4PvMotionOn = m_HdrSmoothInfo.i4MotionOn;

    MY_LOG_IF(m_bDebugEnable, "[%s] bVHDRResetFlag: %d, HdrOnOff: %d, RMG_K: %d, RMG_T: %d, u4LE_SERatio_x100: %d, LE:(%d,%d,%d) SE:(%d,%d,%d)\n", __FUNCTION__, bVHDRResetFlag, rOutput.HdrEvSetting.i4HdrOnOff, rOutput.HdrEvSetting.i4RMGk, rOutput.HdrEvSetting.i4RMGt,
                            rOutput.u4LE_SERatio_x100,rOutput.u4LEExpTimeInUS,rOutput.u4LESensorGain, rOutput.u4LEISPGain,
                                                      rOutput.u4SEExpTimeInUS,rOutput.u4SESensorGain, rOutput.u4SEISPGain);
    MY_LOG_IF(m_bDebugEnable, "[%s] Current final HdrOnOff:%d \n", __FUNCTION__, rOutput.HdrEvSetting.i4HdrOnOff);

    if (bVHDRResetFlag)
    {
        bVHDRResetFlag = MFALSE;
    }
#endif

}


MINT32 getDataInterpolation(MINT32 i4X, MINT32 i4X0, MINT32 i4X1, MINT32 i4Y0, MINT32 i4Y1)
{
    MINT32 i4Y;
    MINT32 i4Denoinator = (i4X1 - i4X0);
    MINT32 i4Numerator = (i4Y1 - i4Y0) * (i4X - i4X0);

    if(i4X >= i4X1)
        i4Y = i4Y1;
    else if(i4X <= i4X0)
        i4Y = i4Y0;
    else
        i4Y = i4Y0 + ((i4Numerator + (SIGN(i4Numerator * i4Denoinator)*(i4Denoinator >> 1)))/i4Denoinator);

    return i4Y;
}

MVOID calIsoRatio(const ZVHDRExpSettingInputParam_T& rInput, ZVHDRExpSettingOutputParam_T& rOutput, ZVHDRSmoothInfo_T& m_HdrSmoothInfo)
{
#if 0
    MINT32 i4Ratio = 0;
    MUINT32 u4ISOThresValue = 0;

    AUTO_FLICKER_RATIO_T rFlickerParam = rInput.rAutoFlicker;

    i4Ratio = (rInput.HdrEvSetting.i4HdrRatio[0] == 0) ? 1 : rInput.HdrEvSetting.i4HdrRatio[0];
    u4ISOThresValue = (rInput.u4ISPGain * rInput.u4SensorGain + (1<<9)) >> 10;
    m_HdrSmoothInfo.i4IsoRatio = getDataInterpolation((MINT32)u4ISOThresValue, (MINT32)rFlickerParam.rIsoThr.u4X2, (MINT32)rFlickerParam.rIsoThr.u4X1, i4Ratio, (MINT32)rFlickerParam.rIsoThr.u4Y1);

    MY_LOG_IF(m_bDebugEnable, "[%s] Iso = %d, HdrRatio = %d, IsoRatio = %d, HdrFrom = %d\n", __FUNCTION__, u4ISOThresValue, rInput.HdrEvSetting.i4HdrRatio[0], m_HdrSmoothInfo.i4IsoRatio, rInput.HdrEvSetting.i4HdrFrom);
#endif
}

MVOID calMotionRatio(const ZVHDRExpSettingInputParam_T& rInput, ZVHDRExpSettingOutputParam_T& rOutput, ZVHDRSmoothInfo_T& m_HdrSmoothInfo)
{
#if 0
    /*
     *  G/Gyro Info for ratio control
     */

    MINT64 i8AcceValue = 0, i8GyroValue = 0;
    MINT32 i4AcceProb = 0, i4GyroProb = 0;
    MINT32 i4MotionProb = 0;
    MINT32 i4TargetMotion = 0;
    static MINT32 i4MotionOnCount = 0, i4PvTargetMotion = 0;

    HDR_G_GYRO_SENSOR_T  rGGyroSensor = rInput.rGGyroSensor;
    HDR_G_GYRO_INFO_T rGGyroInfo = rInput.rGGyroInfo;

    if (bVHDRResetFlag)
    {
        i4MotionOnCount = 0;
        i4PvTargetMotion = 0;
    }

    i8AcceValue = sqrt((MINT64)rGGyroInfo.i4Acce[0]*rGGyroInfo.i4Acce[0]+(MINT64)rGGyroInfo.i4Acce[1]*rGGyroInfo.i4Acce[1]+(MINT64)rGGyroInfo.i4Acce[2]*rGGyroInfo.i4Acce[2]);
    i8GyroValue = sqrt((MINT64)rGGyroInfo.i4Gyro[0]*rGGyroInfo.i4Gyro[0]+(MINT64)rGGyroInfo.i4Gyro[1]*rGGyroInfo.i4Gyro[1]+(MINT64)rGGyroInfo.i4Gyro[2]*rGGyroInfo.i4Gyro[2]);
    i4AcceProb = getDataInterpolation((MINT32)i8AcceValue, rGGyroSensor.rAcceProb.u4X1,rGGyroSensor.rAcceProb.u4X2,rGGyroSensor.rAcceProb.u4Y1,rGGyroSensor.rAcceProb.u4Y2);
    i4GyroProb = getDataInterpolation((MINT32)i8GyroValue, rGGyroSensor.rGyroProb.u4X1,rGGyroSensor.rGyroProb.u4X2,rGGyroSensor.rGyroProb.u4Y1,rGGyroSensor.rGyroProb.u4Y2);
    i4MotionProb = i4AcceProb + i4GyroProb;
    m_HdrSmoothInfo.i4MotionRatio = getDataInterpolation(i4MotionProb, rGGyroSensor.rMotionRatio.u4X1, rGGyroSensor.rMotionRatio.u4X2, m_HdrSmoothInfo.i4IsoRatio, rGGyroSensor.rMotionRatio.u4Y2);
    m_HdrSmoothInfo.i4MotionOn = (i4MotionProb > rGGyroSensor.i4MotionOnThr) ? 1 : 0;

    /*
     * Motion On Stable Check
     */

    if (i4PvTargetMotion != m_HdrSmoothInfo.i4MotionOn)
    {
        i4PvTargetMotion = m_HdrSmoothInfo.i4MotionOn;
        if (i4MotionOnCount != 0) {
            m_HdrSmoothInfo.i4MotionOn = i4PvMotionOn;
        }
        i4MotionOnCount = 1;

    }
    else {

        if (i4MotionOnCount < rGGyroSensor.i4MotionOnCntThr)
        {
           i4MotionOnCount++;
           m_HdrSmoothInfo.i4MotionOn = i4PvMotionOn;

        } else if (i4MotionOnCount == rGGyroSensor.i4MotionOnCntThr)
        {
           i4MotionOnCount++;
        }

    }

    MY_LOG_IF(m_bDebugEnable, "[%s] MotionRatio:%d, MotionOn:%d, MotionOnCount:%d, MotionOnCntThr:%d, MotionOnThr:%d, MotionProb:%d, AcceProb:%d (%lld), GyroProb:%d (%lld)\n",
                                __FUNCTION__, m_HdrSmoothInfo.i4MotionRatio, m_HdrSmoothInfo.i4MotionOn, i4MotionOnCount, rInput.rGGyroSensor.i4MotionOnCntThr, rInput.rGGyroSensor.i4MotionOnThr, i4MotionProb, i4AcceProb, i8AcceValue, i4GyroProb, i8GyroValue);
#endif
}

MVOID calAntiFlickerRatio(const ZVHDRExpSettingInputParam_T& rInput, ZVHDRExpSettingOutputParam_T& rOutput, ZVHDRSmoothInfo_T& m_HdrSmoothInfo)
{
#if 0
    MUINT64 u8TotalGain = 0;
    MINT32 i4LEShutter = 0, i4FlickerTolerance = 0, i4FlickerMultiple = 0;
    MUINT32 u4AnitBandingBase = 0;

    AUTO_FLICKER_RATIO_T rFlickerParam = rInput.rAutoFlicker;

    m_HdrSmoothInfo.i4DeflickerEn1 = (rInput.i4LV < rFlickerParam.i4Flkr1LVThr[rInput.bIs60HZ]);
    m_HdrSmoothInfo.i4DeflickerEn2 = (rInput.i4LV < rFlickerParam.i4Flkr2LVThr[rInput.bIs60HZ]);

    u4AnitBandingBase = (rInput.bIs60HZ) ? (CUST_ZVHDR_ANTIBANDING_BASE_60HZ) : (CUST_ZVHDR_ANTIBANDING_BASE_50HZ);

    i4FlickerTolerance = ((MINT32)u4AnitBandingBase * CUST_ZVHDR_ANTIBANDING_BASE_TOLERRANCE + 50) / 100;


    i4LEShutter = rInput.u4ShutterTime;

    u8TotalGain = (MUINT64)i4LEShutter * (MUINT64)rInput.u4SensorGain * (MUINT64)rInput.u4ISPGain ;

    rOutput.u4LESensorGain = rInput.u4SensorGain;

    MY_LOG_IF(m_bDebugEnable, "[%s] Freq:%d, Lv:%d, x1ISO:%d, SatGain:%d, SensorGain:%d, ISPGain:%d, InputShut:%d, LEShut:%d\n",
                                __FUNCTION__, rInput.bIs60HZ, rInput.i4LV, rInput.u41xGainISO, rInput.u4SaturationGain, rInput.u4SensorGain, rInput.u4ISPGain, rInput.u4ShutterTime, i4LEShutter);
    MY_LOG_IF(m_bDebugEnable, "[%s] MaxExp:%d, MinExp:%d, AntiBandingBase:%d\n",  __FUNCTION__, rFlickerParam.i4MaxExp[rInput.bIs60HZ], rFlickerParam.i4MinExp[rInput.bIs60HZ], u4AnitBandingBase);
    MY_LOG_IF(m_bDebugEnable, "[%s] TotalGain:%lld\n",  __FUNCTION__, u8TotalGain);


    if(i4LEShutter > rFlickerParam.i4MaxExp[rInput.bIs60HZ])
    {
        rOutput.u4LEExpTimeInUS = rFlickerParam.i4MaxExp[rInput.bIs60HZ];
        u8TotalGain = (u8TotalGain + ((MUINT64)rOutput.u4LEExpTimeInUS >> 1)) / (MUINT64)rOutput.u4LEExpTimeInUS;
        rOutput.u4LEISPGain = (u8TotalGain + ((MUINT64)rOutput.u4LESensorGain >> 1)) / (MUINT64)rOutput.u4LESensorGain;

    }
    else {
        rOutput.u4LEExpTimeInUS = i4LEShutter;
        rOutput.u4LEISPGain = rInput.u4ISPGain;
    }

    rOutput.u4SESensorGain = rOutput.u4LESensorGain;
    rOutput.u4SEISPGain = rOutput.u4LEISPGain;
    rOutput.u4SEExpTimeInUS = rOutput.u4LEExpTimeInUS / m_HdrSmoothInfo.i4MotionRatio;
    if(rOutput.u4SEExpTimeInUS < rFlickerParam.i4MinExp[rInput.bIs60HZ])
    {
        rOutput.u4SEExpTimeInUS = rFlickerParam.i4MinExp[rInput.bIs60HZ];
    }

    m_HdrSmoothInfo.i4FinalRatio = m_HdrSmoothInfo.i4MotionRatio * 100;

     /*
      *   If LE < BandingBase, no need to follow flicker rule
      */
    if(rOutput.u4LEExpTimeInUS >= (u4AnitBandingBase - i4FlickerTolerance))
    {
        /*
         * Flicker Type 1: SE != N * BandingBase
         */
        if (m_HdrSmoothInfo.i4DeflickerEn1) {

            if(rOutput.u4SEExpTimeInUS >= u4AnitBandingBase) {

                i4FlickerMultiple = (rOutput.u4SEExpTimeInUS / u4AnitBandingBase) + (rOutput.u4SEExpTimeInUS % u4AnitBandingBase != 0);
                rOutput.u4SEExpTimeInUS = i4FlickerMultiple * u4AnitBandingBase;
                m_HdrSmoothInfo.i4FinalRatio = (rOutput.u4LEExpTimeInUS * 100) / rOutput.u4SEExpTimeInUS;
                MY_LOG_IF(m_bDebugEnable, "[%s] i4DeflickerEn1 = 1, FlkrMulti:%d, SE:%d, FinalRatio:%d !!!!!\n", __FUNCTION__, i4FlickerMultiple, rOutput.u4SEExpTimeInUS, m_HdrSmoothInfo.i4FinalRatio);
            }
        }

        /*
         * Flicker Type 2: SE < BandingBase
         */
        if (m_HdrSmoothInfo.i4DeflickerEn2) {

            if(rOutput.u4SEExpTimeInUS < u4AnitBandingBase) {

                rOutput.u4SEExpTimeInUS = u4AnitBandingBase;
                m_HdrSmoothInfo.i4FinalRatio = (rOutput.u4LEExpTimeInUS * 100) / rOutput.u4SEExpTimeInUS;
                MY_LOG_IF(m_bDebugEnable, "[%s] i4DeflickerEn2 = 1, SE:%d, FinalRatio:%d !!!!!\n", __FUNCTION__, rOutput.u4SEExpTimeInUS, m_HdrSmoothInfo.i4FinalRatio);
            }
        }
    }

    /*
     * Ratio Bound Protection
     */
    if(m_HdrSmoothInfo.i4FinalRatio < 100) {
        m_HdrSmoothInfo.i4FinalRatio = 100;
        rOutput.u4SEExpTimeInUS = rOutput.u4LEExpTimeInUS;
    }

    MY_LOG_IF(m_bDebugEnable, "[%s] IsoRatio: %d, FinalRatio:%d, FlickerEn1:%d, FlickerEn2:%d, i4LV:%d, LE:(%d, %d, %d), SE:(%d, %d, %d)\n", __FUNCTION__,
                                                     m_HdrSmoothInfo.i4IsoRatio, m_HdrSmoothInfo.i4FinalRatio, m_HdrSmoothInfo.i4DeflickerEn1, m_HdrSmoothInfo.i4DeflickerEn2, rInput.i4LV,
                                                     rOutput.u4LEExpTimeInUS, rOutput.u4LESensorGain, rOutput.u4LEISPGain, rOutput.u4SEExpTimeInUS, rOutput.u4SESensorGain, rOutput.u4SEISPGain);
#endif

}


MVOID Ratio2AEInfo(const ZVHDRExpSettingInputParam_T& rInput, ZVHDRExpSettingOutputParam_T& rOutput, ZVHDRSmoothInfo_T& m_HdrSmoothInfo)
{
#if 0
    MUINT32 u4LeTotalGain, u4SEExpo, u4SEExpoMult, u4CurrRatio;
    AUTO_FLICKER_RATIO_T rFlickerParam = rInput.rAutoFlicker;

    /*
    *   Init
    */

    rOutput.u4LEExpTimeInUS = rInput.u4ShutterTime;
    rOutput.u4LESensorGain = rInput.u4SensorGain;
    rOutput.u4LEISPGain = rInput.u4ISPGain;

    rOutput.u4SEExpTimeInUS = rInput.u4ShutterTime;
    rOutput.u4SESensorGain = rInput.u4SensorGain;
    rOutput.u4SEISPGain = rInput.u4ISPGain;
    u4CurrRatio = m_HdrSmoothInfo.i4FinalRatio;

    m_HdrSmoothInfo.i4DeflickerEn1 = (rInput.i4LV < rFlickerParam.i4Flkr1LVThr[rInput.bIs60HZ]);        // Flicker multiple
    m_HdrSmoothInfo.i4DeflickerEn2 = (rInput.i4LV < rFlickerParam.i4Flkr2LVThr[rInput.bIs60HZ]);        // Flicker base

    /*
    *   Uninit
    */

    u4LeTotalGain = rOutput.u4LEExpTimeInUS * rOutput.u4LESensorGain;

    MY_LOG_IF(m_bDebugEnable, "[%s] LEInfo(%d,%d,%d) Ratio:%d, bIs60Hz:%d, SatGain:%d\n", __FUNCTION__, rOutput.u4LEExpTimeInUS, rOutput.u4LESensorGain, u4LeTotalGain, u4CurrRatio, rInput.bIs60HZ, rInput.u4SaturationGain);

    if(u4CurrRatio == 100) {
        rOutput.u4SEExpTimeInUS = rOutput.u4LEExpTimeInUS;
        rOutput.u4SESensorGain = rOutput.u4LESensorGain;
        MY_LOG_IF(m_bDebugEnable, "[%s] Ratio:1\n", __FUNCTION__);
    }
    else {
        if(rOutput.u4LEExpTimeInUS < i4FlickerExpoBase[(MINT32)rInput.bIs60HZ][0])
        {
            rOutput.u4SESensorGain = rInput.u4SaturationGain;
            rOutput.u4SEExpTimeInUS = ((MUINT64)u4LeTotalGain * 100 + ((rOutput.u4SESensorGain * u4CurrRatio) >> 1)) / (rOutput.u4SESensorGain * u4CurrRatio);

            MY_LOG_IF(m_bDebugEnable, "[%s] LEExp < FlickerBase(%d) => SEInfo(%d,%d) Ratio:%d\n", __FUNCTION__, i4FlickerExpoBase[(MINT32)rInput.bIs60HZ][0],
                                                                                              rOutput.u4SEExpTimeInUS, rOutput.u4SESensorGain, u4CurrRatio);
        }
        else {
            u4SEExpo = ((MUINT64)rOutput.u4LEExpTimeInUS * rOutput.u4LESensorGain * 100) / (rInput.u4SaturationGain * u4CurrRatio);

            for(int i = 0; i < 13; i++)
            {
                if(i4FlickerExpoBase[(MINT32)rInput.bIs60HZ][i] >= u4SEExpo)
                {
                    u4SEExpoMult = i;
                    break;
                }
            }

            MY_LOG_IF(m_bDebugEnable, "[%s] Ratio:%d, SEExpo:%d, SEExpoMult:%d\n", __FUNCTION__, u4CurrRatio, u4SEExpo, u4SEExpoMult);

            if(u4SEExpoMult == 0) {
                if(m_HdrSmoothInfo.i4DeflickerEn2)
                    rOutput.u4SEExpTimeInUS = i4FlickerExpoBase[(MINT32)rInput.bIs60HZ][0];
                else
                    rOutput.u4SEExpTimeInUS = u4SEExpo;
                rOutput.u4SESensorGain = rInput.u4SaturationGain;

                MY_LOG_IF(m_bDebugEnable, "[%s] DeflickerEn2:%d => SEExp:%d, SEGain:%d, FlickerBase:%d\n", __FUNCTION__, m_HdrSmoothInfo.i4DeflickerEn2 , rOutput.u4SEExpTimeInUS,
                                                                                                                         rOutput.u4SESensorGain, i4FlickerExpoBase[(MINT32)rInput.bIs60HZ][0]);
            }
            else {
                if(m_HdrSmoothInfo.i4DeflickerEn1)
                    rOutput.u4SEExpTimeInUS = i4FlickerExpoBase[(MINT32)rInput.bIs60HZ][u4SEExpoMult];
                else
                    rOutput.u4SEExpTimeInUS = u4SEExpo;
                rOutput.u4SESensorGain = ((MUINT64)u4LeTotalGain * 100 + ((rOutput.u4SEExpTimeInUS * u4CurrRatio) >> 1)) / (rOutput.u4SEExpTimeInUS * u4CurrRatio);

                MY_LOG_IF(m_bDebugEnable, "[%s] DeflickerEn1:%d => SEExp:%d, SEGain:%d, FlickerBase:%d\n", __FUNCTION__, m_HdrSmoothInfo.i4DeflickerEn1 , rOutput.u4SEExpTimeInUS,
                                                                                                                         rOutput.u4SESensorGain, i4FlickerExpoBase[(MINT32)rInput.bIs60HZ][u4SEExpoMult]);

                if(rOutput.u4SESensorGain <= rInput.u4SaturationGain)
                {
                    MY_LOG_IF(m_bDebugEnable, "[%s] SEGain(%d) < SatGain(%d)\n", __FUNCTION__, rOutput.u4SESensorGain , rInput.u4SaturationGain);
                    rOutput.u4SEExpTimeInUS = i4FlickerExpoBase[(MINT32)rInput.bIs60HZ][u4SEExpoMult - 1];
                    rOutput.u4SESensorGain = ((MUINT64)u4LeTotalGain * 100 + ((rOutput.u4SEExpTimeInUS * u4CurrRatio) >> 1)) / (rOutput.u4SEExpTimeInUS * u4CurrRatio);
                }

                if(rOutput.u4SEExpTimeInUS > rOutput.u4LEExpTimeInUS)
                {
                    MY_LOG_IF(m_bDebugEnable, "[%s] SEExp(%d) > LEExp(%d)\n", __FUNCTION__, rOutput.u4SEExpTimeInUS , rOutput.u4LEExpTimeInUS);
                    rOutput.u4SEExpTimeInUS = rOutput.u4LEExpTimeInUS;
                    rOutput.u4SESensorGain = ((MUINT64)u4LeTotalGain * 100 + ((rOutput.u4SEExpTimeInUS * u4CurrRatio) >> 1)) / (rOutput.u4SEExpTimeInUS * u4CurrRatio);
                }
            }

        }

    }

    rOutput.u4LE_SERatio_x100 = (MUINT32)(((MUINT64)rOutput.u4LESensorGain * rOutput.u4LEExpTimeInUS * 100)/(rOutput.u4SESensorGain * rOutput.u4SEExpTimeInUS));

    if(rOutput.u4LE_SERatio_x100 < 100) {
        rOutput.u4LE_SERatio_x100 = 100;
        rOutput.u4SEExpTimeInUS = rOutput.u4LEExpTimeInUS;
        rOutput.u4SESensorGain = rOutput.u4LESensorGain;
    }
#endif
}


MVOID smoothRatioRMGt(const ZVHDRExpSettingInputParam_T& rInput, ZVHDRExpSettingOutputParam_T& rOutput, ZVHDRSmoothInfo_T& m_HdrSmoothInfo)
{
#if 0
    MINT32 i4SmoothCntThr = 0, i4RatioUnit = 0, i4RMGtUnit = 0, i4MultiCheck = 0, i4CalcPvRatio = 0;
    static MINT32 i4SmoothCount = 0, i4PvTargetRatio = 0;
    static MINT32 i4RMGtTarget = 0, i4RMGtDown = 0;
    static MINT32 i4RMGtStep = 0, i4RatioStep = 0; // index to temp array

    AUTO_SMOOTH_T  rRatioSmooth = rInput.rAutoSmooth;

    if (bVHDRResetFlag)
    {
        i4SmoothCount = 0;
        i4PvTargetRatio = 0;
        i4RMGtTarget = 0;
        i4RMGtDown = 0;
        i4RMGtStep = 0;
        i4RatioStep = 0;
    }

    //MINT32 i4RMGtArr[17] = {0,64,128,192,256,320,384,448,512,576,640,704,768,832,896,960,1024}; //temp RMGt array
    //MINT32 i4RatioArr[9] = {0,50,100,150,200,250,300,350,400}; //temp ratio array

    i4SmoothCntThr = rRatioSmooth.i4RatioSmoothThr[m_HdrSmoothInfo.i4MotionOn];
    i4RatioUnit = rRatioSmooth.i4RatioUnit[m_HdrSmoothInfo.i4MotionOn];
    i4RMGtUnit = rRatioSmooth.i4RMGtUnit[m_HdrSmoothInfo.i4MotionOn];

    /*
     * Ratio Stable Check
     */

    m_HdrSmoothInfo.i4FinalRatio = m_HdrSmoothInfo.i4MotionRatio * 100;

    if (rRatioSmooth.bRatioSmoothEnable == MTRUE) {

     //if (i4PvTargetRatio != m_HdrSmoothInfo.i4FinalRatio)
     if (abs(i4PvTargetRatio-m_HdrSmoothInfo.i4FinalRatio) > CUST_ZVHDR_RATIO_TOLERRANCE)
     {
         i4PvTargetRatio = m_HdrSmoothInfo.i4FinalRatio;
         if (i4SmoothCount != 0) {
             m_HdrSmoothInfo.i4FinalRatio = i4PvRatio;
         }
         i4SmoothCount = 1;

     }
     else {

         if (i4SmoothCount < i4SmoothCntThr)
         {
            i4SmoothCount++;
            m_HdrSmoothInfo.i4FinalRatio = i4PvRatio;

         } else if (i4SmoothCount == i4SmoothCntThr)
         {
            i4SmoothCount++;
         }

     }
     //rOutput.u4SEExpTimeInUS = (rOutput.u4LEExpTimeInUS*100) / m_HdrSmoothInfo.i4FinalRatio;

     MY_LOG_IF(m_bDebugEnable, "[%s] STABLE MotionOn:%d, SmoothCntThr:%d, SmoothCount:%d, PvRatio:%d, PvTargetRatio:%d, FinalRatio:%d\n", __FUNCTION__, m_HdrSmoothInfo.i4MotionOn, i4SmoothCntThr, i4SmoothCount, i4PvRatio, i4PvTargetRatio, m_HdrSmoothInfo.i4FinalRatio);

    }


#if 0
        /*MINT32 i4RatioRound[10] = {400,100,100,200,200,200,200,200,200,200};
        static MINT32 i4RoundCount = 0;
        i4FinalRatio = i4RatioRound[i4RoundCount];
        if (i4RoundCount < 9) {
            i4RoundCount++;
        } else {
            i4RoundCount = 0;
        }*/

        MY_LOG_IF(m_bDebugEnable, "[RatioSmooth] Old FinalRatio: %d, Old RMGt: %d \n", i4FinalRatio, rOutput.HdrEvSetting.i4RMGt);
#endif


    /*
     *  Ratio/RMGt Step Change
     */
    if ((i4PvRatio != m_HdrSmoothInfo.i4FinalRatio) && (i4PvRatio != 0)) {

        if (abs(m_HdrSmoothInfo.i4FinalRatio-i4PvRatio) > i4RatioUnit) {

            i4MultiCheck = (i4PvRatio + (i4RatioUnit >> 1)) / i4RatioUnit;
            i4CalcPvRatio = i4MultiCheck * i4RatioUnit;
            if (abs(i4CalcPvRatio-i4PvRatio) > CUST_ZVHDR_RATIO_TOLERRANCE) {
                i4CalcPvRatio = i4PvRatio;
            }

            MY_LOG_IF(m_bDebugEnable, "[%s] i4CalcPvRatio: %d, i4MultiCheck: %d \n", __FUNCTION__, i4CalcPvRatio, i4MultiCheck);

            if (m_HdrSmoothInfo.i4FinalRatio > i4PvRatio) {

                i4RatioStep = i4CalcPvRatio / i4RatioUnit + 1;
                m_HdrSmoothInfo.i4FinalRatio = i4RatioStep * i4RatioUnit;
            }
            else {
                i4RatioStep = i4CalcPvRatio / i4RatioUnit - (i4CalcPvRatio % i4RatioUnit == 0);
                m_HdrSmoothInfo.i4FinalRatio = i4RatioStep * i4RatioUnit;
            }
        }
    }

#if 0
    if ((i4PvRatio != m_HdrSmoothInfo.i4FinalRatio) && (i4PvRatio != 0)) {

        if ((i4PvRatio != 100) && (m_HdrSmoothInfo.i4FinalRatio != 100)) {

            if (abs(m_HdrSmoothInfo.i4FinalRatio-i4PvRatio) > i4RatioUnit) {

                if (m_HdrSmoothInfo.i4FinalRatio > i4PvRatio) {

                    i4RatioStep = i4PvRatio / i4RatioUnit + 1;
                    m_HdrSmoothInfo.i4FinalRatio = i4RatioStep * i4RatioUnit;
                }
                else {
                    i4RatioStep = i4PvRatio / i4RatioUnit - (i4PvRatio % i4RatioUnit == 0);
                    m_HdrSmoothInfo.i4FinalRatio = i4RatioStep * i4RatioUnit;
                }
            }

            m_HdrSmoothInfo.i4FinalRMGt = i4PvRMGt;

        } else if (m_HdrSmoothInfo.i4FinalRatio == 100) {

            if (i4PvRMGt == 1024) {

                i4RMGtTarget = i4HdrAlgoRMGt;
                m_HdrSmoothInfo.i4FinalRMGt = i4RMGtTarget;

            }
            else {

                i4RMGtTarget = 1024;
                m_HdrSmoothInfo.i4FinalRatio = i4PvRatio;
                if (abs(i4RMGtTarget-i4PvRMGt) > i4RMGtUnit) {

                    i4RMGtStep = i4PvRMGt / i4RMGtUnit + 1;
                    m_HdrSmoothInfo.i4FinalRMGt = i4RMGtStep * i4RMGtUnit;

                }
                else {

                    m_HdrSmoothInfo.i4FinalRMGt = i4RMGtTarget;
                }
            }
        }
        else { // i4PvRatio == 100

            i4RMGtTarget = 1024;
            m_HdrSmoothInfo.i4FinalRMGt = i4RMGtTarget;
            i4RMGtDown = 1;
        }
    }
    else {
        // Might be in the middle of 1x->Nx
        if (i4RMGtDown) {

            i4RMGtTarget = i4HdrAlgoRMGt;
            m_HdrSmoothInfo.i4FinalRatio = i4PvRatio;
            if (abs(i4RMGtTarget-i4PvRMGt) > i4RMGtUnit) {

                i4RMGtStep = i4PvRMGt / i4RMGtUnit - (i4PvRMGt % i4RMGtUnit == 0);
                m_HdrSmoothInfo.i4FinalRMGt = i4RMGtStep * i4RMGtUnit;

            }
            else {

                i4RMGtDown = 0;
            }
        }
    }
#endif

    rOutput.HdrEvSetting.i4RMGt = m_HdrSmoothInfo.i4FinalRMGt;
    //rOutput.u4SEExpTimeInUS = rOutput.u4LEExpTimeInUS * 100 / m_HdrSmoothInfo.i4FinalRatio;

    MY_LOG_IF(m_bDebugEnable, "[%s] SMOOTH MotionOn:%d, RatioUnit:%d, PvRatio:%d, FinalRatio:%d, RatioStep:%d, RMGtUnit:%d, RMGt:%d, RMGtTarget:%d, RMGtDown:%d, RMGtStep:%d\n", __FUNCTION__,
                            m_HdrSmoothInfo.i4MotionOn, i4RatioUnit, i4PvRatio, m_HdrSmoothInfo.i4FinalRatio, i4RatioStep, i4RMGtUnit, m_HdrSmoothInfo.i4FinalRMGt, i4RMGtTarget, i4RMGtDown, i4RMGtStep);
#endif

}

MVOID smoothHdrOnOff(const ZVHDRExpSettingInputParam_T& rInput, ZVHDRExpSettingOutputParam_T& rOutput, ZVHDRSmoothInfo_T& m_HdrSmoothInfo)
{
#if 0
    /*
     * HdrOnOff Stable Check
     */
    AUTO_SMOOTH_T  rHdrSmooth = rInput.rAutoSmooth;
    static MINT32 i4PvTargetOnOff = 0, i4SmoothCount = 0;

    if (bVHDRResetFlag)
    {
        i4SmoothCount = 0;
        i4PvTargetOnOff = 0;
        m_HdrSmoothInfo.i4HdrOnOff = rInput.HdrEvSetting.i4HdrOnOff;
    }

    if(!rHdrSmooth.bHdrWaitAEStable || rInput.bAEStable)
    {

        m_HdrSmoothInfo.i4HdrOnOff = rInput.HdrEvSetting.i4HdrOnOff;

        if (i4PvTargetOnOff != m_HdrSmoothInfo.i4HdrOnOff)
        {
            i4PvTargetOnOff = m_HdrSmoothInfo.i4HdrOnOff;
            if (i4SmoothCount != 0) {
                m_HdrSmoothInfo.i4HdrOnOff = i4PvOnOff;
            }
            i4SmoothCount = 1;

        }
        else {

            if (i4SmoothCount < rHdrSmooth.i4HdrInfoCounterThr)
            {
               i4SmoothCount++;
               m_HdrSmoothInfo.i4HdrOnOff = i4PvOnOff;

            } else if (i4SmoothCount == rHdrSmooth.i4HdrInfoCounterThr)
            {
               i4SmoothCount++;
            }

        }

    }


    rOutput.HdrEvSetting.i4HdrOnOff = m_HdrSmoothInfo.i4HdrOnOff;

    MY_LOG_IF(m_bDebugEnable, "[%s] HdrOnOff:%d, WaitAEStable:%d, AEStable:%d, HdrInfoCounterThr:%d, SmoothCount:%d, PvOnOff:%d, PvTargetOnOff:%d, rInputHdrOnOff:%d\n", __FUNCTION__, m_HdrSmoothInfo.i4HdrOnOff, rHdrSmooth.bHdrWaitAEStable, rInput.bAEStable, rHdrSmooth.i4HdrInfoCounterThr, i4SmoothCount, i4PvOnOff, i4PvTargetOnOff, rInput.HdrEvSetting.i4HdrOnOff);
#endif

}

MVOID resetZVHDRFlag(){
    bVHDRResetFlag = MTRUE;
}

#if 0
/**********************************************************************************
 * Dynamic ratio 2x <-> 1x with RMG_TH change                                     *
 *                                                                                *
 * Luminance level changes when LE/SE ratio change from 2x to 1x and vice versa   *
 *                                                                                *
 * (1) When change from 2x to 1x, slowly increase non-linear RMG k value to 4090  *
 *     then change setting to 1x.                                                 *
 * (2) When change from 1x to 2x, change setting to 2x
with non-linear RMG_TH     *
 *     value 4090 then slowly decrease non-linear RMG_TH value from 4090 to target*
 *     value then change setting to 1x.                                           *
 **********************************************************************************/
MVOID RMGTHSmooth(MINT32 i4FinalRatio, ZVHDRExpSettingOutputParam_T& rOutput, MINT32 u4AnitBandingBase) {


    MINT32 u4RMGIdxRange;
    const MINT32 u4SegUnit = 128;
    static MINT32 u4PvLSRatiox100 = 0;
    static MINT32 u4count = 0;
    MINT32 u4TargetSEExpTimeInUS = 0;

    u4RMGIdxRange = (4096-i4NVRAMRMGSeg)/u4SegUnit;

    if (bVHDRResetFlag == MTRUE) {
        u4PvLSRatiox100 = 0;
        u4count = 0;
        bVHDRResetFlag = MFALSE;
    }

    MY_LOG_IF(m_bDebugEnable, "[%s] init u4PvLSRatiox100: %d, u4count: %d\n", __FUNCTION__, u4PvLSRatiox100,u4count);
    if ((u4PvLSRatiox100 != 0) && (i4FinalRatio == 100)) {
        if (u4count < u4RMGIdxRange) {
            u4TargetSEExpTimeInUS = rOutput.u4LEExpTimeInUS*100/u4PvLSRatiox100;

            if (u4TargetSEExpTimeInUS < u4AnitBandingBase) {
                rOutput.HdrEvSetting.i4RMGk = i4NVRAMRMGSeg;
                u4PvLSRatiox100 = i4FinalRatio;
                u4count = u4RMGIdxRange;

            } else {
                rOutput.u4SEExpTimeInUS = u4TargetSEExpTimeInUS;
                if (u4count == u4RMGIdxRange - 1) {
                    rOutput.HdrEvSetting.i4RMGk = 4090;
                } else {
                    rOutput.HdrEvSetting.i4RMGk = i4NVRAMRMGSeg + (u4count+1)*u4SegUnit;
                }
                u4count++;
            }
        } else {
            rOutput.HdrEvSetting.i4RMGk = i4NVRAMRMGSeg;
            u4PvLSRatiox100 = i4FinalRatio;
        }
    } else if (i4FinalRatio != 100){
        if (u4count > 0) {
            rOutput.u4SEExpTimeInUS = rOutput.u4LEExpTimeInUS*100/i4FinalRatio;
            u4PvLSRatiox100 = i4FinalRatio;
            if (u4count == u4RMGIdxRange) {
                rOutput.HdrEvSetting.i4RMGk = 4090;
            } else {
                rOutput.HdrEvSetting.i4RMGk = i4NVRAMRMGSeg + u4count*u4SegUnit;
            }
            u4count--;
        } else {
            rOutput.HdrEvSetting.i4RMGk = i4NVRAMRMGSeg;
            u4PvLSRatiox100 = i4FinalRatio;
        }
    } else {
        rOutput.HdrEvSetting.i4RMGk = i4NVRAMRMGSeg;
        u4PvLSRatiox100 = i4FinalRatio;
        u4count = u4RMGIdxRange;
    }

    MY_LOG_IF(m_bDebugEnable, "[%s] u4PvLSRatiox100: %d, i4RMGk: %d, u4count: %d, LE: %d, SE: %d\n", __FUNCTION__, u4PvLSRatiox100, rOutput.HdrEvSetting.i4RMGk, u4count, rOutput.u4LEExpTimeInUS, rOutput.u4SEExpTimeInUS);
}
#endif


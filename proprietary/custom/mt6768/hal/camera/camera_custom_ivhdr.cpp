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
#include "camera_custom_ivhdr.h"
#include <math.h>
#include <cstdio>
#include <cstdlib>
#include <cutils/properties.h>
#include <cutils/log.h> // For XLOG?().
#include <utils/Errors.h>



/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/
 // customize parameters
#define CUST_IVHDR_8X_EXPTIME_50HZ    30000
#define CUST_IVHDR_8X_EXPTIME_60HZ    33333
#define CUST_IVHDR_8X_ISOTHRES            200   // ISO 200
#define CUST_IVHDR_8X_RATIO                  4      // 8x

#define CUST_IVHDR_4X_EXPTIME_50HZ    30000
#define CUST_IVHDR_4X_EXPTIME_60HZ    33333
#define CUST_IVHDR_4X_ISOTHRES            200   // ISO 200
#define CUST_IVHDR_4X_RATIO                  4      // 4x

#define CUST_IVHDR_2X_EXPTIME_50HZ    30000
#define CUST_IVHDR_2X_EXPTIME_60HZ    33333
#define CUST_IVHDR_2X_ISOTHRES            200   // ISO 200
#define CUST_IVHDR_2X_RATIO                  2      // 2x

#define CUST_IVHDR_1X_EXPTIME_50HZ    30000
#define CUST_IVHDR_1X_EXPTIME_60HZ    33333
#define CUST_IVHDR_1X_ISOTHRES            400   // ISO 400

#define CUST_IVHDR_MAX_EXPTIME_50HZ  60000
#define CUST_IVHDR_MAX_EXPTIME_60HZ  66666
#define CUST_IVHDR_MAX_ISOTHRES          800   // ISO 800

#define CUST_IVHDR_CUSTOM_MAX_SENSOR_GAINTHRES        (8704)   //
#define CUST_IVHDR_CUSTOM_MAX_ISP_GAINTHRES           (6144)   //


#define CUST_IVHDR_ENABLE_WORKAROUND_SOLUTION 0
#define CUST_IVHDR_WORKAROUND_ISOTHRES            400
/**************************************************************************
 *     E N U M / S T R U C T / T Y P E D E F    D E C L A R A T I O N     *
 **************************************************************************/
#define CUST_IVHDR_ANTIBANDING_BASE_50HZ   10000
#define CUST_IVHDR_ANTIBANDING_BASE_60HZ    8333

#define CUST_IVHDR_ISPGAIN_BASE 1024   // 1x = 1023
/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/

/**************************************************************************
 *       P R I V A T E    F U N C T I O N    D E C L A R A T I O N        *
 **************************************************************************/
MVOID getIVHDRExpSetting(const IVHDRExpSettingInputParam_T& rInput, IVHDRExpSettingOutputParam_T& rOutput)
{
    MUINT32 u4TotalGain = rInput.u4SensorGain * rInput.u4IspGain;

    rOutput.bEnableWorkaround = 0;
    rOutput.u4SEExpTimeInUS =  rInput.u4ShutterTime ;
    rOutput.u4LEExpTimeInUS =  rInput.u4ShutterTime ;


    if(rInput.u4SensorGain >= CUST_IVHDR_CUSTOM_MAX_SENSOR_GAINTHRES) {
        rOutput.u4SESensorGain = CUST_IVHDR_CUSTOM_MAX_SENSOR_GAINTHRES;
        rOutput.u4LESensorGain = CUST_IVHDR_CUSTOM_MAX_SENSOR_GAINTHRES;
    }
    else {
        rOutput.u4SESensorGain = rInput.u4SensorGain;
        rOutput.u4LESensorGain = rInput.u4SensorGain;
    }

    rOutput.u4SEISPGain = (u4TotalGain + (rOutput.u4SESensorGain >> 1)) / rOutput.u4SESensorGain;
    rOutput.u4LEISPGain = (u4TotalGain + (rOutput.u4LESensorGain >> 1)) / rOutput.u4LESensorGain;

    if(rOutput.u4LEISPGain >= CUST_IVHDR_CUSTOM_MAX_ISP_GAINTHRES)
    {
        rOutput.u4LEISPGain = CUST_IVHDR_CUSTOM_MAX_ISP_GAINTHRES;
    }

    if(rOutput.u4SEISPGain >= CUST_IVHDR_CUSTOM_MAX_ISP_GAINTHRES)
    {
        rOutput.u4SEISPGain = CUST_IVHDR_CUSTOM_MAX_ISP_GAINTHRES;
    }

#if 0
    if(u4TotalGain >= CUST_IVHDR_CUSTOM_MAX_SENSOR_GAINTHRES) {
        rOutput.u4SESensorGain = CUST_IVHDR_CUSTOM_MAX_SENSOR_GAINTHRES;
        rOutput.u4LESensorGain = CUST_IVHDR_CUSTOM_MAX_SENSOR_GAINTHRES;
        rOutput.u4SEISPGain = (u4TotalGain * CUST_IVHDR_ISPGAIN_BASE + (rOutput.u4SESensorGain >> 1)) / rOutput.u4SESensorGain;
        rOutput.u4LEISPGain = (u4TotalGain * CUST_IVHDR_ISPGAIN_BASE + (rOutput.u4LESensorGain >> 1)) / rOutput.u4LESensorGain;

        if(rOutput.u4LEISPGain >= CUST_IVHDR_CUSTOM_MAX_ISP_GAINTHRES) {
            rOutput.u4SEISPGain = CUST_IVHDR_CUSTOM_MAX_ISP_GAINTHRES;
            rOutput.u4LEISPGain = CUST_IVHDR_CUSTOM_MAX_ISP_GAINTHRES;
        }
    }
    else {
        rOutput.u4SESensorGain = u4TotalGain;
        rOutput.u4LESensorGain = u4TotalGain;
        rOutput.u4SEISPGain = CUST_IVHDR_ISPGAIN_BASE;
        rOutput.u4LEISPGain = CUST_IVHDR_ISPGAIN_BASE;
    }

#endif
#if 0
MUINT64 u4LE1xExpTimeUs;
MUINT64 u4LE2xExpTimeUs;
MUINT64 u4LE4xExpTimeUs;
MUINT64 u4LE8xExpTimeUs;
MUINT64 u4LEMaxExpTimeUs;
MUINT64 u4ISO1xThresValue;
MUINT64 u4ISO2xThresValue;
MUINT64 u4ISO4xThresValue;
MUINT64 u4ISO8xThresValue;
MUINT64 u4ISOMaxThresValue;
MUINT64 u4AnitBandingBase;
MUINT32 u4Ratio_x100;
MUINT64 u4InputSum;
MUINT32 u4CountIdx = 0;
MUINT32 u4NewExpTimeUs;
MUINT32 u4ISOWorkaroundValue;

    if(rInput.bIs60HZ == MTRUE) {
        u4LE1xExpTimeUs = CUST_IVHDR_1X_EXPTIME_60HZ;
        u4LE2xExpTimeUs = CUST_IVHDR_2X_EXPTIME_60HZ;
        u4LE4xExpTimeUs = CUST_IVHDR_4X_EXPTIME_60HZ;
        u4LE8xExpTimeUs = CUST_IVHDR_8X_EXPTIME_60HZ;        
        u4LEMaxExpTimeUs = CUST_IVHDR_MAX_EXPTIME_60HZ; // 66ms    
        u4AnitBandingBase = CUST_IVHDR_ANTIBANDING_BASE_60HZ;   // 8.333ms
    } else {
        u4LE1xExpTimeUs = CUST_IVHDR_1X_EXPTIME_50HZ;
        u4LE2xExpTimeUs = CUST_IVHDR_2X_EXPTIME_50HZ;
        u4LE4xExpTimeUs = CUST_IVHDR_4X_EXPTIME_50HZ;
        u4LE8xExpTimeUs = CUST_IVHDR_8X_EXPTIME_50HZ;        
        u4LEMaxExpTimeUs = CUST_IVHDR_MAX_EXPTIME_50HZ; // 80ms
        u4AnitBandingBase = CUST_IVHDR_ANTIBANDING_BASE_50HZ;   // 10ms
    }

    u4ISO1xThresValue = 1024*CUST_IVHDR_1X_ISOTHRES / rInput.u41xGainISO;
    u4ISO2xThresValue = 1024*CUST_IVHDR_2X_ISOTHRES / rInput.u41xGainISO;
    u4ISO4xThresValue = 1024*CUST_IVHDR_4X_ISOTHRES / rInput.u41xGainISO;
    u4ISO8xThresValue = 1024*CUST_IVHDR_8X_ISOTHRES / rInput.u41xGainISO;
    u4ISOMaxThresValue = 1024*CUST_IVHDR_MAX_ISOTHRES / rInput.u41xGainISO;
    u4ISOWorkaroundValue = CUST_IVHDR_CUSTOM_MAX_GAINTHRES; //1024*CUST_IVHDR_WORKAROUND_ISOTHRES / rInput.u41xGainISO;
    u4InputSum = rInput.u4ShutterTime * rInput.u4SensorGain;

    rOutput.bEnableWorkaround = MFALSE;
    rOutput.u4SEISPGain = CUST_IVHDR_ISPGAIN_BASE;
    rOutput.u4LEISPGain = CUST_IVHDR_ISPGAIN_BASE;   

//    XLOGE("[getIVHDRExpSetting] time:%d %d %d %d %d ISO:%d %d %d %d %d u4InputSum:%d\n", u4LE1xExpTimeUs, u4LE2xExpTimeUs, u4LE4xExpTimeUs, u4LE8xExpTimeUs, u4LEMaxExpTimeUs, u4ISO1xThresValue, u4ISO2xThresValue, u4ISO4xThresValue, u4ISO8xThresValue, u4ISOMaxThresValue, u4InputSum);

    if((u4LE8xExpTimeUs*u4ISO8xThresValue) >= CUST_IVHDR_8X_RATIO*u4InputSum) {    // 8x
        for(u4CountIdx=0; (MINT32)(u4LE8xExpTimeUs - u4CountIdx*u4AnitBandingBase) >= u4AnitBandingBase; u4CountIdx++) {
            u4NewExpTimeUs = u4LE8xExpTimeUs - u4CountIdx*u4AnitBandingBase;
//            XLOGE("[getIVHDRExpSetting] u4CountIdx:%d u4NewExpTimeUs:%d %d %d", u4CountIdx, u4NewExpTimeUs, (u4NewExpTimeUs >>3)*rInput.u4SaturationGain, rInput.u4ShutterTime*rInput.u4SensorGain);
            if((u4NewExpTimeUs*rInput.u4SaturationGain) <= CUST_IVHDR_8X_RATIO*u4InputSum) {
                rOutput.u4SEExpTimeInUS = u4NewExpTimeUs / CUST_IVHDR_8X_RATIO;
                rOutput.u4SESensorGain = u4InputSum / rOutput.u4SEExpTimeInUS;
                rOutput.u4LEExpTimeInUS = u4NewExpTimeUs;
                rOutput.u4LESensorGain = rOutput.u4SESensorGain;                
 //           XLOGE("[getIVHDRExpSetting] Shutter:%d %d Gain:%d %d", rOutput.u4SEExpTimeInUS, rOutput.u4LEExpTimeInUS, rOutput.u4SESensorGain, rOutput.u4LESensorGain);
                break;
            }
        }

        if((MINT32)(u4LE8xExpTimeUs - u4CountIdx*u4AnitBandingBase) < u4AnitBandingBase) {
                rOutput.u4SEExpTimeInUS = u4InputSum / rInput.u4SaturationGain;
                rOutput.u4SESensorGain = rInput.u4SaturationGain;
                rOutput.u4LEExpTimeInUS = CUST_IVHDR_8X_RATIO*rOutput.u4SEExpTimeInUS;
                rOutput.u4LESensorGain = rOutput.u4SESensorGain;
        }
//         XLOGE("[getIVHDRExpSetting] Shutter:%d %d Gain:%d %d", rOutput.u4SEExpTimeInUS, rOutput.u4LEExpTimeInUS, rOutput.u4SESensorGain, rOutput.u4LESensorGain);        
    } else if((u4LE4xExpTimeUs*u4ISO4xThresValue) >= CUST_IVHDR_4X_RATIO*u4InputSum) {   // 4x
        rOutput.u4SEExpTimeInUS = u4LE4xExpTimeUs / CUST_IVHDR_4X_RATIO;
        rOutput.u4SESensorGain = u4InputSum / rOutput.u4SEExpTimeInUS;
        rOutput.u4LEExpTimeInUS = u4LE4xExpTimeUs;
        rOutput.u4LESensorGain = rOutput.u4SESensorGain;       
    } else if((u4LE2xExpTimeUs*u4ISO2xThresValue) >= CUST_IVHDR_2X_RATIO*u4InputSum) {   // 2x
        rOutput.u4SEExpTimeInUS = u4LE4xExpTimeUs / CUST_IVHDR_2X_RATIO;    
        rOutput.u4SESensorGain = u4InputSum / rOutput.u4SEExpTimeInUS;
        rOutput.u4LEExpTimeInUS = u4LE2xExpTimeUs;
        rOutput.u4LESensorGain = rOutput.u4SESensorGain;        
    } else {   // 1x
        if((u4LE1xExpTimeUs*u4ISO1xThresValue) > u4InputSum) {
            rOutput.u4SEExpTimeInUS = u4LE1xExpTimeUs;
            rOutput.u4SESensorGain = u4InputSum / rOutput.u4SEExpTimeInUS;
            rOutput.u4LEExpTimeInUS = u4LE1xExpTimeUs;
            rOutput.u4LESensorGain = rOutput.u4SESensorGain;                
        } else{
            for(u4CountIdx=0; (u4LE1xExpTimeUs + u4CountIdx*u4AnitBandingBase) <= u4LEMaxExpTimeUs; u4CountIdx++) {
                u4NewExpTimeUs = u4LEMaxExpTimeUs + u4CountIdx*u4AnitBandingBase;
//                XLOGE("[getIVHDRExpSetting] u4CountIdx:%d u4NewExpTimeUs:%d %d %d", u4CountIdx, u4NewExpTimeUs, (u4NewExpTimeUs >>3)*rInput.u4SaturationGain, rInput.u4ShutterTime*rInput.u4SensorGain);
                if((u4NewExpTimeUs*u4ISO1xThresValue) >= u4InputSum) {
                    rOutput.u4SEExpTimeInUS = u4NewExpTimeUs;
                    rOutput.u4SESensorGain = u4InputSum / rOutput.u4SEExpTimeInUS;
                    rOutput.u4LEExpTimeInUS = u4NewExpTimeUs;
                    rOutput.u4LESensorGain = rOutput.u4SESensorGain;                
//                    XLOGE("[getIVHDRExpSetting] Shutter:%d %d Gain:%d %d", rOutput.u4SEExpTimeInUS, rOutput.u4LEExpTimeInUS, rOutput.u4SESensorGain, rOutput.u4LESensorGain);
                    break;
               }
            }

            if((u4LE1xExpTimeUs + u4CountIdx*u4AnitBandingBase) > u4LEMaxExpTimeUs) {
                    rOutput.u4SEExpTimeInUS = u4LEMaxExpTimeUs;
                    rOutput.u4SESensorGain = u4InputSum / rOutput.u4SEExpTimeInUS;
                    rOutput.u4LEExpTimeInUS = u4LEMaxExpTimeUs;
                    rOutput.u4LESensorGain = rOutput.u4SESensorGain;
            }
//         XLOGE("[getIVHDRExpSetting] Shutter:%d %d Gain:%d %d", rOutput.u4SEExpTimeInUS, rOutput.u4LEExpTimeInUS, rOutput.u4SESensorGain, rOutput.u4LESensorGain);        
        }
    }
    
   // workaround solution if need
    if((rOutput.u4SESensorGain > u4ISOWorkaroundValue) && (CUST_IVHDR_ENABLE_WORKAROUND_SOLUTION == 1)) {
        rOutput.bEnableWorkaround = MTRUE;
        rOutput.u4SESensorGain = u4ISOWorkaroundValue;
        rOutput.u4LESensorGain = rOutput.u4SESensorGain;
    }

    if((rOutput.u4SEISPGain > u4ISOWorkaroundValue) && (CUST_IVHDR_ENABLE_WORKAROUND_SOLUTION == 1)) {
        rOutput.u4SEISPGain = u4ISOWorkaroundValue;
        rOutput.u4LEISPGain = rOutput.u4SEISPGain;
    }

    ALOGD("[%s] x1ISO:%d, MaxGain:%d, SEAfeGain:%d, SEIspGain:%d, LEAfeGain:%d, LEIspGain:%d\n", __FUNCTION__, rInput.u41xGainISO, u4ISOWorkaroundValue,
                                                        rOutput.u4SESensorGain, rOutput.u4SEISPGain,
                                                        rOutput.u4LESensorGain, rOutput.u4LEISPGain);

    if(rOutput.u4LESensorGain >= CUST_IVHDR_CUSTOM_MAX_GAINTHRES)
        rOutput.u4LESensorGain = CUST_IVHDR_CUSTOM_MAX_GAINTHRES;

    if(rOutput.u4SESensorGain >= CUST_IVHDR_CUSTOM_MAX_GAINTHRES)
        rOutput.u4SESensorGain = CUST_IVHDR_CUSTOM_MAX_GAINTHRES;
#endif

}


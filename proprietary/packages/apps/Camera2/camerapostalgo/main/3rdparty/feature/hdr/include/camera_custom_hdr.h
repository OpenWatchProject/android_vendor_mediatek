/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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
#ifndef _CAMERA_CUSTOM_HDR_H_
#define _CAMERA_CUSTOM_HDR_H_

#include <sys/cdefs.h>
#include <utils/def/BuiltinTypes.h>
//#include "AEPlinetable.h"
__BEGIN_DECLS

// 0: static bracketing for normal AE
// 1: adaptive bracketing for iVHDR, mVHDR AE
// 2: adaptive bracketing for normal AE
#define BRACKETING_TYPE 0

// enable this will dump HDR Debug Information into SDCARD
#define CUST_HDR_DEBUG  0

// debug dump path
#define HDR_DEBUG_OUTPUT_FOLDER "/storage/sdcard0/"

/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/
// For HDR Customer Parameters

// [capture policy]
//     - When CUST_HDR_CAPTURE_POLICY==0,
//          The capture size of each frame will be as same as the HDR output image size.
//          If no special reason (ex. memory issues), this one will be a good choise.
//     - When CUST_HDR_CAPTURE_POLICY==1,
//          Use the sensor size as the maximum frame size.
//          The HDR result will be scaled up to fit JPEG size later.
//          This option can save memory when capture size is larger than sensor size.
//          But the cost is a HDR with worse quality.
#define CUST_HDR_CAPTURE_POLICY 0

// [Core Number]
//     - Value Range: 1(For single-core)/2 or 4 or 10(For multi-core)
#define CUST_HDR_CORE_NUMBER    8

//
/*        [Capture Algorithm]
u4AOEMode==1 : Dynamic EV Bracket
     - Use histogram 0~40 Avg to calculate LE.
        And by DeltaEV + SE_OFFSET to calculate SE.
        if LE-SE > than EVDIFFTH
           use LE, (LE+SE)/2, SE
        else
           use LE + SE
u4AOEMode==0 : Static EV Bracket

     - When CUST_HDR_CAPTURE_ALGORITHM==1,
          If original Image (0EV) needs to be saved. we use 1.
          If there is less than HDR_NEOverExp_Percent/1000 pixels
          over saturation in 0EV, we capture +1.5/0 EV 2  frames, else we take +1.5/0/-2 EV 3 frames.
     - When CUST_HDR_CAPTURE_ALGORITHM==2,
          If original Image (0EV) needs not to be saved. we use 2.
          If there is less than HDR_NEOverExp_Percent/1000 pixels
          over saturation in 0EV, we capture +1.5/-1 EV 2  frames, else we take +1.5/0/-2 EV 3 frames.
*/

//u4AOEMode==0 : Static EV Bracket Parameters
#define CUST_HDR_CAPTURE_ALGORITHM   1
#define CUST_HDR_NEOverExp_Percent   10

//u4AOEMode==1 : Dynamic EV Bracket Parameters
#define SE_HIST_BIN_NUM 20
#define INTENSITY_TARGET 1.2f
#define NE_AOE_GAIN 0.84f // -0.25EV
#define SE_OFFSET 0.5f
#define EVDIFFTH 2.95f
#define MAX_LE_EV 3.33f
#define MIN_LE_EV 0.34f
#define HIGHLIGHT_TH 32

// [Prolonged VD]
//     - Value Range: 1(default)~ (depend on sensor characteristics).
#define CUST_HDR_PROLONGED_VD   1

// [BRatio]
//     - Higher value:
//         - Decrease the degree of artifact (halo-like around object boundary).
//         - Avoid non-continued edge in fusion result.
//         - Decrease little dynamic range.
//         - Decrease sharpness.
//     - Value Range: 1 (non-blur) ~ 160. (Default: 40).
#define CUST_HDR_BRATIO     40

// [Gain]
//     - Higher value increase sharpness, but also increase noise.
//     - Value Range: 256 ~ 512. (Default: 256 (1x gain)) (384/256 = 1.5x gain)
#define CUST_HDR_GAIN_00    256
#define CUST_HDR_GAIN_01    256
#define CUST_HDR_GAIN_02    256
#define CUST_HDR_GAIN_03    256
#define CUST_HDR_GAIN_04    256
#define CUST_HDR_GAIN_05    256
#define CUST_HDR_GAIN_06    256
#define CUST_HDR_GAIN_07    256
#define CUST_HDR_GAIN_08    256
#define CUST_HDR_GAIN_09    256
#define CUST_HDR_GAIN_10    256
#define CUST_HDR_GAIN_11    256
#define CUST_HDR_GAIN_12    256
#define CUST_HDR_GAIN_13    256
#define CUST_HDR_GAIN_14    256
#define CUST_HDR_GAIN_15    256


// [F Control]
//     -Higher value decreases the degree of flare, but also decrease parts of the image details.
//     - Value Range: 0 ~ 50. (Default: 10)
#define CUST_HDR_BOTTOM_FRATIO      10
//     - Value Range: 0 ~ 50. (Default: 10)
#define CUST_HDR_TOP_FRATIO     10
//     - Value Range: 0 ~ 24. (Default: 16)
#define CUST_HDR_BOTTOM_FBOUND      16
//     - Value Range: 0 ~ 24. (Default: 16)
#define CUST_HDR_TOP_FBOUND     16

// [De-halo Control]
//     - Higher value reduce more halo for sky, but also reduce more dynamic range in some parts of the image.
//     - Value Range: 0 (off) ~ 255. (Default: 245)
#define CUST_HDR_TH_HIGH                245

// [Noise Control]
//     - Higher value reduce more noise, but also reduce dynamic range in low light region.
//     - Value Range: 0 (off) ~ 255. (Default: 25)
#define CUST_HDR_TH_LOW                 0

// [Level Subtract]
//     - Value Range: 0 (less low-frequency halo, less dynamic range) or 1 (more low-frequency halo, more dynamic range). (Default: 0)
#define CUST_HDR_TARGET_LEVEL_SUB       0

// [ISP Gamma Selection]
//     - Value Range: 0 (create gamma/degamma curve base on fixed curve) or 1 (create gamma/degamma curve base on ISP Gamma). (Default: 0)
#define CUST_HDR_USE_ISP_GAMMA      0

// [Feature Match point threshold]
//     - Value Range: (Default: 20)
#define CUST_HDR_MATCH_POINT_THRESHOLD   20

/**************************************************************************
 *     E N U M / S T R U C T / T Y P E D E F    D E C L A R A T I O N     *
 **************************************************************************/

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *        P U B L I C    F U N C T I O N    D E C L A R A T I O N         *
 **************************************************************************/
MUINT32 CustomHdrCoreNumberGet(void);
MUINT32 CustomHdrProlongedVdGet(void);
MUINT32 CustomHdrBRatioGet(void);
MUINT32 CustomHdrGainArrayGet(MUINT32 u4ArrayIndex);
double CustomHdrBottomFRatioGet(void);
double CustomHdrTopFRatioGet(void);
MUINT32 CustomHdrBottomFBoundGet(void);
MUINT32 CustomHdrTopFBoundGet(void);
MINT32 CustomHdrThHighGet(void);
MINT32 CustomHdrThLowGet(void);
MUINT32 CustomHdrTargetLevelSubGet(void);
MBOOL CustomHdrUseIspGamma(void);
MBOOL CustomHdrMatchPointThreshold(void);

/*******************************************************************************
* HDR exposure setting
*******************************************************************************/
/*
 * NOTE: this structure should be aligned with struct ExpSettingParam_T in
 * aaa_hal_common.h
 */
typedef struct HDRExpSettingInputParam_S
{
    MINT32 u4AOEMode;
    MUINT32 u4MaxSensorAnalogGain; // 1x=1024
    MUINT32 u4MaxAEExpTimeInUS;    // unit: us
    MUINT32 u4MinAEExpTimeInUS;     // unit: us
    MUINT32 u4ShutterLineTime;    // unit: 1/1000 us
    MUINT32 u4MaxAESensorGain;     // 1x=1024
    MUINT32 u4MinAESensorGain;     // 1x=1024
    MUINT32 u4ExpTimeInUS0EV;      // unit: us
    MUINT32 u4SensorGain0EV;       // 1x=1024
    MUINT32 u4ISOValue;            // real ISO value
    MUINT8  u1FlareOffset0EV;
    MINT32  i4GainBase0EV;      // AOE application for LE calculation
    MINT32  i4LE_LowAvg;        // AOE application for LE calculation, def: 0 ~ 39 avg
    MINT32  i4SEDeltaEVx100;    // AOE application for SE calculation
    MBOOL   bDetectFace; // detect face or not
    MUINT32 u4Histogram[128];
    MUINT32 u4FlareHistogram[128];
    //strAETable PLineAETable;
    MINT32 i4aeTableCurrentIndex;

#ifdef __cplusplus
    HDRExpSettingInputParam_S()
    : u4AOEMode(0),
      u4MaxSensorAnalogGain(0),
      u4MaxAEExpTimeInUS(0),
      u4MinAEExpTimeInUS(0),
      u4ShutterLineTime(0),
      u4MaxAESensorGain(0),
      u4MinAESensorGain(0),
      u4ExpTimeInUS0EV(0),
      u4SensorGain0EV(0),
      u4ISOValue(0),
      u1FlareOffset0EV(0),
      i4GainBase0EV(0),
      i4LE_LowAvg(0),
      i4SEDeltaEVx100(0),
      bDetectFace(MFALSE),
      u4Histogram(),
      u4FlareHistogram(),
      //PLineAETable(),
      i4aeTableCurrentIndex(-1)
    {}
#endif
} HDRExpSettingInputParam_T;

typedef struct HDRExpSettingOutputParam_S
{
    MUINT32 u4OutputFrameNum;     // Output frame number (2 or 3)
    MUINT32 u4ExpTimeInUS[3];     // unit: us; [0]-> short exposure; [1]: 0EV; [2]: long exposure
    MUINT32 u4SensorGain[3];      // 1x=1204; [0]-> short exposure; [1]: 0EV; [2]: long exposure
    MUINT32 u4BypassLTM[3];
    MUINT8  u1FlareOffset[3];     // [0]-> short exposure; [1]: 0EV; [2]: long exposure
    MUINT32 u4FinalGainDiff[2];   // 1x=1024; [0]: Between short exposure and 0EV; [1]: Between 0EV and long exposure
    MUINT32 u4TargetTone;         // Decide the curve to decide target tone
    MINT32  i4RunningNumber;      // 4-digit running number (range: 1 ~ 9999) for debug purpose

#ifdef __cplusplus
    HDRExpSettingOutputParam_S()
    : u4OutputFrameNum(0),
      u4ExpTimeInUS(),
      u4SensorGain(),
      u4BypassLTM(),
      u1FlareOffset(),
      u4FinalGainDiff(),
      u4TargetTone(0),
      i4RunningNumber(0)
    {}
#endif
} HDRExpSettingOutputParam_T;

MVOID getHDRExpSetting(const HDRExpSettingInputParam_T& rInput, HDRExpSettingOutputParam_T& rOutput);

__END_DECLS

#endif  // _CAMERA_CUSTOM_HDR_H_


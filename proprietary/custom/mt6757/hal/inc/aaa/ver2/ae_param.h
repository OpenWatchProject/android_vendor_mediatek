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

#ifndef _AE_PARAM_H
#define _AE_PARAM_H

#include "../../../../../common/hal/inc/custom/aaa/ae_param.h"

#include <ae_feature.h>
#include <camera_custom_nvram.h>
#include <awb_param.h>
#include <camera_custom_AEPlinetable.h>

#define AE_PARAM_REVISION    7264006
#define AE_BLOCK_NO  5
#define AE_V4P0_BLOCK_NO  15
#define AE_TG_BLOCK_NO_X 12
#define AE_TG_BLOCK_NO_Y 9
#define AE_TG_BLOCK_TOTAL_SIZE (AE_TG_BLOCK_NO_X*AE_TG_BLOCK_NO_Y)
#define FLARE_SCALE_UNIT (512) // 1.0 = 512
#define FLARE_OFFSET_DOMAIN (4095) // 12bit domain
#define AE_STABLE_THRES 1 //3   //0.3 ev
#define AE_WIN_OFFSET          1000   // for android window define
#define MAX_ISP_GAIN   (10*1024)
#define AE_HISTOGRAM_BIN (128)
#define FLARE_HISTOGRAM_BIN 40
#define SENSTIVITY_UINT 1024
#define AE_VGAIN_UNIT         ((MUINT32)128)              //gain unit in  virtual-gain  , when calculate best CW Target
#define AE_RATIO_SCALE   ((MUINT32)64)   // scale to calculate traget CW vakye  and current CW value
#define AE_BV_TARGET ((MUINT32)47)
#define DELTA_INDEX_SCALE ((MINT32)8)
#define AE_SENSOR_MAX_QUEUE 4
#define MAX_AE_PRE_EVSETTING 3
#define AE_RATIOTBL_MAXSIZE 16
#define AE_METER_GAIN_BASE 1024
#define AE_PARAM_PS_RMBN_SEL 3

typedef enum
{
	PSO_OUTPUT_AFTER_OB = 0,
	PSO_OUTPUT_BEFORE_OB,
}PSO_PATH_SEL_ENUM;

typedef struct
{
    PSO_PATH_SEL_ENUM Pso_Output_Path; //Before OB or after OB
    MUINT32 OB_Offset[4];  //0: R, 1:Gr, 2:Gb, 3:B
    MUINT32 OB_Gain[4];    //0: R, 1:Gr, 2:Gb, 3:B
    MUINT32 OB_Gain_Unit;

    AWB_GAIN_T PreGain1;
    MUINT32 PreGain1_Unit;

    AWB_GAIN_T awb_gain;
    MUINT32 awb_gain_Unit;

    MUINT16 LSC_Gain_Grid_R [AE_V4P0_BLOCK_NO*AE_V4P0_BLOCK_NO];
    MUINT16 LSC_Gain_Grid_G [AE_V4P0_BLOCK_NO*AE_V4P0_BLOCK_NO];
    MUINT16 LSC_Gain_Grid_B [AE_V4P0_BLOCK_NO*AE_V4P0_BLOCK_NO];
    MUINT32 LSC_Gain_Unit;

    MINT32 CCM[9];
    MUINT32 CCM_Gain_Unit;

    MUINT32 RGB2Y_Coef[3]; //RGB2Y Coef
    MINT64  i8AAOTimeStamp;
}AAO_PROC_INFO_T;


// PSO window statistics
typedef struct
{
    MUINT8 ucR;
    MUINT8 ucGr;
    MUINT8 ucGb;
    MUINT8 ucB;
    //MUINT8 ucY;
} AE_PSO_MAIN_STAT_T;




// AAO separation size coefficients
#define AAO_SEP_AWB_SIZE_COEF  4 * 2       // AAO AWB size coefficient awb AAO 1byte to 2byte
#define AAO_SEP_AE_SIZE_COEF   2       // AAO AE size coefficient
#define AAO_SEP_HIST_SIZE_COEF (4 * 4) // AAO HIST size coefficient
#define AAO_SEP_OC_SIZE_COEF   2       // AAO OverCnt size coefficient
#define AAO_SEP_LSC_SIZE_COEF  (4 * 2) // AAO LSC size coefficient
// PSO separation size coefficients
#define PSO_SEP_AWB_SIZE_COEF  4       // PSO AWB size coefficient
#define PSO_SEP_AE_SIZE_COEF   1       // PSO AE size coefficient

typedef enum
{
    AE_WEIGHTING_CENTRALWEIGHT=0,
    AE_WEIGHTING_SPOT,
    AE_WEIGHTING_AVERAGE
}eWeightingID;

typedef struct
{
   eWeightingID eID;  //weighting table ID
   MUINT32 W[5][5];    //AE weighting table
}strWeightTable;

typedef struct {
    MBOOL   bFaceLocSizeCheck;
    MUINT32 u4Size_X[2];
    MUINT32 u4Size_Y[2];
    MUINT32 u4Loc_X[2];
    MUINT32 u4Loc_Y[2];
} strFaceLocSize;

typedef struct {
    MINT32  i4DeltaEVDiff;
    MUINT32 u4WoAccRatio;// 65
    MUINT32 u4BoAccRatio;// 80
    MUINT32 u4PredY_H;// 240
    MUINT32 u4PredY_L;// 4
    MUINT32 u4AccY_Max;//254
    MUINT32 u4AccCWV_Max;// 510
    MUINT32 u4AccY_Min;// 1
    MBOOL   bRecycleEnable;
} strPerframeCFG;
typedef struct {
    MUINT32 u4InStableThd;  // 0.08EV
    MUINT32 u4OutStableThd; // 0.08EV

    MBOOL   bEnableAEModeStableTHD;         // enable video ae stable threshold setting
    MUINT32 u4VideoInStableThd;               // video mode each index 0.08EV
    MUINT32 u4VideoOutStableThd;              // video mode each index 0.08EV
    MUINT32 u4FaceInStableThd;               // Face mode each index 0.08EV
    MUINT32 u4FaceOutStableThd;              // Face mode each index 0.08EV
    MUINT32 u4TouchInStableThd;               // Touch mode each index 0.08EV
    MUINT32 u4TouchOutStableThd;              // Touch mode each index 0.08EV
    MBOOL   bEnableFaceAeLock;               // Face Ae lock option
    MBOOL   bEnableZeroStableThd;            // enable zero stable thd
} strAEStableThd;
typedef struct {
    MBOOL   bEnableAEHsTS;                   // enable AE HS Target temp smooth
    MBOOL   bEnableAEAblTS;                  // enable AE ABL Target temp smooth
    MBOOL   bEnableAEAoeTS;                  // enable AE AOE Target temp smooth
    MBOOL   bEnableAENsTS;                   // enable AE NS Target temp smooth
    MBOOL   bEnableAEFaceTS;                 // enable AE Face Target temp smooth
    MBOOL   bEnableAEVideoTS;                // enable AE Video Target temp smooth
    MBOOL   bEnableAEMainTargetTS;                // enable AE Main Target temp smooth
    MBOOL   bEnableAEHsV4p0TargetTS;                // enable AE HSV4p0 Target temp smooth
} strAECWRTempSmooth;
typedef struct {
    MBOOL  bEnableFlatSkyEnhance;                                // enable HS adaptive THD by realBV
    MUINT8  uSkyDetInThd;
    MUINT8  uSkyDetOutThd;
    MUINT32 u4FlatSkyTHD;
    MUINT32 u4FlatSkyEVD;
    MINT32 u4BVRatio_X[2];
    MINT32 u4BVRatio_Y[2];
} strHSFlatSkyCFG;

typedef struct
{
    MBOOL   bEnable;                        //enable adaptive AOE THD
    MUINT32 u4TblLength;
    MINT32 i4BV[16];
    MUINT32 u4THD[16];
} strAOEBVRef;

typedef struct {
    MBOOL bEnableABLabsDiff;                                // enable enhance ABL(absDiff)
    MINT32  u4EVDiffRatio_X[2];
    MINT32  u4EVDiffRatio_Y[2];
    MINT32  u4BVRatio_X[2];
    MINT32  u4BVRatio_Y[2];
} strABL_absDiff;
typedef struct {
    MBOOL u4BVAccEnable;
    MINT32 i4DeltaBVRatio;
    MUINT32 u4B2T_Target;
    strAEMovingRatio  pAEBVAccRatio;           // Preview ACC ratio
} strBVAccRatio;
typedef struct {
	MBOOL u4PsoAccEnable;
    MINT32 i4PsoEscRatioWO;  //PSO speedup ratio discount, 100 base (Wash-Out case)
    MINT32 i4PsoEscRatioNonLinear; //PSO speedup ratio discount, 100 base (Non-Linear Case)
	MINT32 ConvergeDeltaPosBound;
    MINT32 ConvergeDeltaNegBound;
}strPSOConverge;
typedef struct {
    MBOOL   bNonCWRAccEnable;
    MUINT32 u4HS_FHY_HBound;
    MUINT32 u4CWV_HBound;
    MUINT32 u4HS_TargetHBound;
    MUINT32 u4HS_ACCTarget;
} strNonCWRAcc;
typedef struct {
    MUINT32 u4MinFDYLowBound;               // face boundary min Y value
    MUINT32 u4MaxFDYHighBound;              // face boundary max Y value
    MINT32  i4FaceBVLowBound;               // face boundary min BV value
    MINT32  i4FaceBVHighBound;              // face boundary max BV value
    MUINT32 u4TmpFDY_HBound;                // tempFDY HighBound
    MUINT32 u4TmpFDY_LBound;                // tempFDY LowBound
    MUINT32 u4FD_Lost_MaxCnt;               // face lost max count
    MUINT32 u4FD_Scale_Ratio;               // face window scale ratio
    MUINT32 u4FaceNSLowBound;       //NS face target Y
}strFaceSmooth;
typedef struct {
    MBOOL   bLandmarkCtrlFlag;              // face landmark ctrol flag
    MUINT32 u4LandmarkCV_Thd;               // face landmark info cv thd
    MUINT32 u4LandmarkWeight;               // face landmark weight
}strFaceLandMarkCtrl;

typedef struct
{
    MBOOL   bEnable;            //enable NS CDF
    UINT32  u4Pcent;            // 1=0.1%, 0~1000
    MINT32 i4NS_CDFRatio_X[2];
    MUINT32 u4NS_CDFRatio_Y[2];
} strNS_CDF;
typedef struct
{
    MBOOL   bEnableMultiStepHS;         //Enable HS enhance method : MultiStep HS(Conflict with ori HS)
    MUINT32 u4TblLength;                                //Max size : 16
    MUINT32 u4MS_EVDiff[AE_RATIOTBL_MAXSIZE];            //size must large than u4TblLength
    MUINT32 u4MS_OutdoorTHD[AE_RATIOTBL_MAXSIZE];        //size must large than u4TblLength
    MUINT32 u4MS_IndoorTHD[AE_RATIOTBL_MAXSIZE];         //size must large than u4TblLength
    MINT32    i4MS_BVRatio[2];                      //MS_HS In/Out door BV threshold
} strHSMultiStep;
typedef struct {
    MBOOL bEnableStablebyHist;
    MUINT32 u4HistEVDiff;
    MUINT32 u4OverexpoTHD;
    MUINT32 u4UnderexpoTHD;
    MUINT32 u4HistStableTHD;
} strHistStableCFG;
typedef struct {
    MBOOL bEnableOverExpoAOE;
    MUINT32 u4OElevel;
    MUINT32 u4OERatio_LowBnd;
    MUINT32 u4OERatio_HighBnd;
    MINT32 i4OE_BVRatio_X[2];
    MUINT32 u4OE_BVRatio_Y[2];
    MUINT32 u4OEWeight[AE_V4P0_BLOCK_NO][AE_V4P0_BLOCK_NO];
} strOverExpoAOECFG;
typedef struct {
    MBOOL bEnableAEMainTarget;
    MUINT32 u4MainTargetWeight;     //weight 1024 base
    MUINT32 u4MainTargetWeightTbl[AE_V4P0_BLOCK_NO][AE_V4P0_BLOCK_NO];  //15x15 Gau weight table
    AE_TARGET_PROB_T TargetBVRatioTbl;
    MBOOL bEnableColorWTRatio;
    AE_TARGET_PROB_T ColorWTRatioTbl;
    MBOOL bEnableColorSuppressR;
    MBOOL bEnableColorSuppressG;
    MBOOL bEnableColorSuppressB;
} strMainTargetCFG;
typedef struct {
    MBOOL bEnableHSv4p0;
    MUINT32 u4HSv4p0Weight;     //weight 1024 base
    MUINT32 u4BVSize;      //max size 16
    MUINT32 u4EVDSize;   //max size 16
    MINT32 i4HS_BVRatio[AE_RATIOTBL_MAXSIZE];            //Ratio[uBVSize]
    MUINT32 u4HS_PcntRatio[AE_RATIOTBL_MAXSIZE];     //Ratio[uBVSize]
    MUINT32 u4HS_EVDRatio[AE_RATIOTBL_MAXSIZE];     //Ratio[u4EVDSize]
    MUINT32 u4HSTHDRatioTbl[AE_RATIOTBL_MAXSIZE][AE_RATIOTBL_MAXSIZE]; //RatioTbl[uBVSize][uEVDSize]
    MBOOL bEnableDynWTRatio;
    AE_TARGET_PROB_T DynWTRatioTbl;
} strHSv4p0CFG;
typedef struct {
    UINT32 u4FlareSmallDeltaIdxStep;
    UINT32 u4CWVChangeStep;
    UINT32 u4AllStableStep;
} strDynamicFlare;

typedef struct{
    MBOOL   bEnableNSBVCFG;
    MUINT32 u4TblLength;
    MINT32  i4BV[AE_RATIOTBL_MAXSIZE];
    MUINT32 u4BTTHD[AE_RATIOTBL_MAXSIZE];
} strNSBVCFG;


/*
* TG AE speedup tuning parameters
*/
typedef struct
{

//B2T
	MUINT32 OverExpoThr; //245
	MUINT32 OverExpoRatio; //70%
	MUINT32 OverExpoResetAccEVTh; //200

	MUINT32 OverExpoSTD_Th[3];
	MUINT32 OverExpoSTD_Ratio[4];

	MUINT32 OverExpoAccDeltaEV_Th[3];
	MINT32 OverExpoAccDeltaEV_DeltaEV[4];

	MUINT32 OverExpoCountSTD_Th;

    MUINT32 OverExpoLowBound;
    MUINT32 OverExpLightAcc1;
    MUINT32 OverExpLightAcc2;
    MUINT32 OverExpLightAcc3;
    MUINT32 OverExpLightAcc4;
//D2T
	MUINT32 UnderExpoThr; //10
	MUINT32 UnderExpoRatio; //95%
	MUINT32 UnderExpoResetAccEVTh; //20

	MUINT32 UnderExpoSTD_Th[3];
	MUINT32 UnderExpoSTD_Ratio[4];

	MUINT32 UnderExpoAccDeltaEV_Th[3];
	MINT32 UnderExpoAccDeltaEV_DeltaEV[4];

	MUINT32 UnderExpoCountSTD_Th;

} strAETgTuningPara;

/*
*   HDR Application
*/
#define HDR_NIGHT_CONTRAST_WEIGHTING_TBL_NUM      (11)
#define HDR_NIGHT_LV_WEIGHTING_TBL_NUM            (20)

typedef struct {
    MINT32 i4HistgramRatioL;
    MINT32 i4HistgramRatioH;
    MINT32 i4NightTarget;
    MINT32 i4HistSeg;
    MINT32 i4ContrastWtTbl[HDR_NIGHT_CONTRAST_WEIGHTING_TBL_NUM];
    MINT32 i4LVWtTbl[HDR_NIGHT_LV_WEIGHTING_TBL_NUM];
} strHdrNightCFG;



typedef struct
{
    MBOOL   bEnableSaturationCheck;        //if toward high saturation scene , then reduce AE target
    MBOOL   bEnablePreIndex;                    // decide the re-initial index after come back to camera
    MBOOL   bEnableRotateWeighting;        // AE rotate the weighting automatically or not
    MBOOL   bEV0TriggerStrobe;
    MBOOL   bLockCamPreMeteringWin;
    MBOOL   bLockVideoPreMeteringWin;
    MBOOL   bLockVideoRecMeteringWin;
    MBOOL   bSkipAEinBirghtRange;            // To skip the AE in some brightness range for meter AE
    MBOOL   bPreAFLockAE;                        // Decide the do AE in the pre-AF or post-AF
    MBOOL   bStrobeFlarebyCapture;          // to Decide the strobe flare by capture image or precapture image
    MBOOL   bEnableFaceAE;                        // Enable the Face AE or not
    MBOOL   bEnableMeterAE;                      // Enable the Meter AE or not
    MBOOL   bFlarMaxStepGapLimitEnable;   //enable max step gap for low light
    MBOOL   bEnableAESmoothGain;             // Enable AE smooth gain
    MBOOL   bEnableLongCaptureCtrl;          // enable the log capture control sequence
    MBOOL   bEnableTouchSmoothRatio;         // enable Touch mode smooth converge
    MBOOL   bEnableTouchSmooth;              // enable Touch mode smooth with perframe
    MBOOL   bEnablePixelBaseHist;            // enable pixel based histogram
    MBOOL   bEnableHDRSTATconfig;            // enable HDR Y LSB config
    MBOOL   bEnableAEOVERCNTconfig;          // enable AE over-exposure count config
    MBOOL   bEnableTSFSTATconfig;            // enable TSF RGB STAT config
    MUINT8   bEnableHDRLSB;                     //HW HDR enable flag 0:8bit(default) 1:12bit 2:14bit
    MBOOL   bEnableFlareFastConverge;        // enable fast flare
    MBOOL   bEnableRAFastConverge;        // enable fast flare
    MUINT32 u4BackLightStrength;              // strength of backlight condtion
    MUINT32 u4OverExpStrength;               // strength of anti over exposure
    MUINT32 u4HistStretchStrength;           //strength of  histogram stretch
    MUINT32 u4SmoothLevel;                      // time LPF smooth level , internal use
    MUINT32 u4TimeLPFLevel;                     //time LOW pass filter level
    MUINT32 u4AEWinodwNumX;                   // AE statistic winodw number X
    MUINT32 u4AEWinodwNumY;                   // AE statistic winodw number Y
    MUINT8 uBlockNumX;                         //AE X block number
    MUINT8 uBlockNumY;                         //AE Yblock number
    MUINT8 uTgBlockNumX;                         //AE X block number
    MUINT8 uTgBlockNumY;                         //AE Yblock number
    MUINT8 uSatBlockCheckLow;             //saturation block check , low thres
    MUINT8 uSatBlockCheckHigh;            //sturation  block check , hight thres
    MUINT8 uSatBlockAdjustFactor;        // adjust factore , to adjust central weighting target value
    MUINT8 uMeteringYLowSkipRatio;     // metering area min Y value to skip AE
    MUINT8 uMeteringYHighSkipRatio;    // metering area max Y value to skip AE
    MUINT32 u4MinYLowBound;                 // metering boundary min Y value
    MUINT32 u4MaxYHighBound;                // metering boundary max Y value
    MUINT32 u4MeterWeight;              // metering weight betwenn CWR and MeterTarget
    MUINT32 u4MinCWRecommend;           // mini target value
    MUINT32 u4MaxCWRecommend;          // max target value
    MINT8   iMiniBVValue;                          // mini BV value.
    MINT8   uAEShutterDelayCycle;         // for AE smooth used.
    MINT8   uAESensorGainDelayCycleWShutter;
    MINT8   uAESensorGainDelayCycleWOShutter;
    MINT8   uAEIspGainDelayCycle;

    // custom Y coefficients
    MUINT32   u4AEYCoefR;        // for AE_YCOEF_R
    MUINT32   u4AEYCoefG;        // for AE_YCOEF_G
    MUINT32   u4AEYCoefB;        // for AE_YCOEF_B

    MUINT32   u4LongCaptureThres;          // enable the log capture control sequence
    MUINT32   u4CenterFaceExtraWeighting;  // give center face an extra weighting
    MUINT16 u2AEStatThrs;
    MUINT8  uCycleNumAESkipAfterExit;
    MUINT8  uOverExpoTHD;                  //overexposure cnt threshold
    MUINT32 u4HSSmoothTHD;
    MUINT32 u4FinerEvIndexBase;            // 1:0.1EV 2:0.05EV 3:0.033EV  10:0.01
}strAEParamCFG;

typedef struct
{
    MINT32 Diff_EV;     //  delta EVx10 ,different between Yavg and Ytarget     Diff_EV=    log(  Yarg/Ytarget,2)
    MINT32  Ration;        //  Yarg/Ytarget  *100
    MINT32  move_index;   // move index
}strAEMOVE;


typedef struct
{
   MBOOL bAFPlineEnable;
   MINT16 i2FrameRate[5][2];
}strAFPlineInfo;

typedef struct
{
    MINT8 iLEVEL1_GAIN;
    MINT8 iLEVEL2_GAIN;
    MINT8 iLEVEL3_GAIN;
    MINT8 iLEVEL4_GAIN;
    MINT8 iLEVEL5_GAIN;
    MINT8 iLEVEL6_GAIN;
    MINT8 iLEVEL1_TARGET_DIFFERENCE;
    MINT8 iLEVEL2_TARGET_DIFFERENCE;
    MINT8 iLEVEL3_TARGET_DIFFERENCE;
    MINT8 iLEVEL4_TARGET_DIFFERENCE;
    MINT8 iLEVEL5_TARGET_DIFFERENCE;
    MINT8 iLEVEL6_TARGET_DIFFERENCE;
    MINT8 iLEVEL1_GAINH;
    MINT8 iLEVEL1_GAINL;
    MINT8 iLEVEL2_GAINH;
    MINT8 iLEVEL2_GAINL;
    MINT8 iLEVEL3_GAINH;
    MINT8 iLEVEL3_GAINL;
    MINT8 iLEVEL4_GAINH;
    MINT8 iLEVEL4_GAINL;
    MINT8 iLEVEL5_GAINH;
    MINT8 iLEVEL5_GAINL;
    MINT8 iLEVEL6_GAINH;
    MINT8 iLEVEL6_GAINL;
    MINT8 iGAIN_DIFFERENCE_LIMITER;
}strAELimiterTable;


typedef struct
{
    MUINT8 uHist0StartBlockXRatio;       //Histogram 0 window config start block X ratio (0~100)
    MUINT8 uHist0EndBlockXRatio;         //Histogram 0 window config end block X ratio (0~100)
    MUINT8 uHist0StartBlockYRatio;       //Histogram 0 window config start block Y ratio (0~100)
    MUINT8 uHist0EndBlockYRatio;         //Histogram 0 window config end block Y ratio (0~100)
    MUINT8 uHist0OutputMode;               //Histogram 0 output source mode
    MUINT8 uHist0BinMode;                    //Histogram 0 bin mode range
    MUINT8 uHist1StartBlockXRatio;       //Histogram 1 window config start block X ratio (0~100)
    MUINT8 uHist1EndBlockXRatio;         //Histogram 1 window config end block X ratio (0~100)
    MUINT8 uHist1StartBlockYRatio;       //Histogram 1 window config start block Y ratio (0~100)
    MUINT8 uHist1EndBlockYRatio;         //Histogram 1 window config end block Y ratio (0~100)
    MUINT8 uHist1OutputMode;               //Histogram 1 output source mode
    MUINT8 uHist1BinMode;                    //Histogram 1 bin mode range
    MUINT8 uHist2StartBlockXRatio;       //Histogram 2 window config start block X ratio (0~100)
    MUINT8 uHist2EndBlockXRatio;         //Histogram 2 window config end block X ratio (0~100)
    MUINT8 uHist2StartBlockYRatio;       //Histogram 2 window config start block Y ratio (0~100)
    MUINT8 uHist2EndBlockYRatio;         //Histogram 2 window config end block Y ratio (0~100)
    MUINT8 uHist2OutputMode;               //Histogram 2 output source mode
    MUINT8 uHist2BinMode;                    //Histogram 2 bin mode range
    MUINT8 uHist3StartBlockXRatio;       //Histogram 3 window config start block X ratio (0~100)
    MUINT8 uHist3EndBlockXRatio;         //Histogram 3 window config end block X ratio (0~100)
    MUINT8 uHist3StartBlockYRatio;       //Histogram 3 window config start block Y ratio (0~100)
    MUINT8 uHist3EndBlockYRatio;         //Histogram 3 window config end block Y ratio (0~100)
    MUINT8 uHist3OutputMode;               //Histogram 3 output source mode
    MUINT8 uHist3BinMode;                      //Histogram 3 bin mode range
} AE_HIST_WIN_CFG_T;

/*
*   HDR Application
*/
#define SUPPORTED_MAX_HDR_EXPNUM            (4)
#define HDR_AE_LV_TBL_NUM                   (19)


// HDR Application - Information

typedef struct
{
    unsigned long long  u8TotalHistCount;
    unsigned long long  u8EqvHdrProb;
    unsigned long long  u8DarkHdrProb;
    unsigned long long  u8BrightHdrProb;
    MINT32              i4BrightEnhanceProb;
    MINT32              i4FDBrightToneProb;
    MINT32              i4HdrContrastL;
    MINT32              i4HdrContrastH;
    MINT32              i4HdrContrastEVDiff;
    MINT32              i4NightEVDark;
    MINT32              i4NightEVBright;
    MINT32              i4NightEVDiff;
    MINT32              i4NightEVDiffProb;
    MINT32              i4NightHistY;
    MINT32              i4NightHistProb;
    MINT32              i4NightLVProb;
    MINT32              i4NightProb;
    MINT32              i4COEDiffProb;
    MINT32              i4COEDiffWeightProb;
    MINT32              i4COEOuterProb;
    MINT32              i4COEOuterWeightProb;
    MINT32              i4ABLDiffProb;
    MINT32              i4LVProb;
    MINT32              i4TotalProb;
    MINT32              i4FinalProb;
    MINT32              i4HdrOnOff;
    MINT32              i4MotionProb;
    MINT32              i4MotionOnOff;
    MINT32              i4RMGtCurrY;
    MINT32              i4cHdrLECurrY;
    MINT32              i4cHdrSECurrY;
} HDR_DETECTOR_INFO_T;

typedef struct
{
    MINT32                      i4Acce[3];
    MINT32                      i4Gyro[3];
} HDR_GGYRO_INFO_T;


typedef struct
{
    MINT32  i4AutoRMGk;
    MINT32  i4AutoRMGt;
    MINT32  i4AutoLVRatio;
    MINT32  i4AutoContrastRatio;
    MINT32  i4AutoBrightRatio;
    MINT32  i4AutoNSRatio;
    MINT32  i4AutoISORatio;
    MINT32  i4AutoGyroRatio;
    MINT32  i4AutoAlgRatio;
    MINT32  i4AutoPvAlgoRatio;
    HDR_GGYRO_INFO_T rGGyroInfo;
} HDR_AUTO_ENHANCE_INFO_T;

typedef struct {
    MINT32 i4NormalAETarget;
    MINT32 i4HdrNormalAETarget;
    MINT32 i4AutoEVWeight;
    MINT32 i4AutoBrightWeight;
    MINT32 i4AutoABLWeight;
    MINT32 i4AutoLVWeightL;
    MINT32 i4AutoLVWeightH;
    MINT32 i4AutoNightWeight;
    MINT32 i4AutoWeight;
    MINT32 i4CurrAoeY;
    MINT32 i4TargetAoeY;
    MINT32 i4TargetAoeGain;
    MINT32 i4CurrNSY;
    MINT32 i4TargetNSY;
    MINT32 i4TargetNSGain;
    MINT32 i4TargetGain;
    MINT32 i4HdrTargetY;
    MINT32 i4MinTargetY;
    MINT32 i4MaxTargetY;
    MUINT32 u4AutoTarget;
    MINT32  i4AEBlendingTarget;
    MINT32  i4FDTarget;
    MINT32  i4FDBlendingTarget;
    MINT32  i4HdrFinalTarget;
} HDR_AE_TARGET_INFO_T;

typedef struct {
    MINT32  i4GainBase;     /*  Video HDR 0 EV info.  */
    MINT32  i4SEInfo;       /*  Delta EV compared with i4GaniBase  */
    MINT32  i4LEInfo;       /*  Low averege data  */
} HDR_CHDR_INFO_T;

typedef struct {
    /*
    *   HDR detector smooth info
    */
    MINT32  i4PvHdrOnOff;
    MINT32  i4PvHdrStableOnOff;
    MINT32  i4HdrOnOffCounter;
    MINT32  i4HdrStableOnOff;
    /*
    *   Motion smooth info
    */
    MINT32  i4MotionOnCount;
    MINT32  i4PvMotionOnOff;
    MINT32  i4PvMotionStableOnOff;
    MINT32  i4MotionStableOnOff;
    /*
    *   Ratio smooth info
    */
    MINT32  i4SmoothTrigger;
    MINT32  i4PvHdrRatio;
    MINT32  i4PvHdrStableRatio;
    MINT32  i4HdrRatioCount;
    MINT32  i4MeanHdrRatio;
    MINT32  i4HdrStableRatio;
    MINT32  i4HdrRatioStep;
    MINT32  i4PvHdrSmoothRatio;
    MINT32  i4HdrSmoothRatio;
    MINT32  i4MediumHdrRatio;
} HDR_SMOOTH_INFO_T;

typedef struct {
    MINT32  i4LEExpo;
    MINT32  i4LEAfeGain;
    MINT32  i4LEIspGain;
    MINT32  i4SEExpo;
    MINT32  i4SEAfeGain;
    MINT32  i4SEIspGain;
    MINT32  i4HdrRatio;
} HDR_EV_INFO_T;

typedef struct
{
    MINT32                      i4AEMode;
    MINT32                      i4LV;
    HDR_DETECTOR_INFO_T         rDetectorInfo;
    HDR_AUTO_ENHANCE_INFO_T     rAutoEnhanceInfo;
    HDR_AE_TARGET_INFO_T        rHdrAETargetInfo;
    HDR_CHDR_INFO_T             rcHdrInfo;
    HDR_SMOOTH_INFO_T           rHdrSmoothInfo;
    HDR_EV_INFO_T               rHdrEvInfo;
} HDR_AE_INFO_T;

// HDR Application - Parameters

typedef struct {
    MINT32              i4HdrHistP0, i4HdrHistP1;
    MINT32              i4HdrHistW0, i4HdrHistW1;
} HDR_WEIGHTING_WIN_T;

typedef struct
{
    MBOOL               bHistDetectEnable;
    HDR_WEIGHTING_WIN_T rHdrDetectDarkWtWin;
    HDR_WEIGHTING_WIN_T rHdrDetectBrightWtWin;
    HDR_WEIGHTING_WIN_T rHdrEnhanceBrightWtWin;
    HDR_WEIGHTING_WIN_T rHdrFDBrightWtWin;
    MINT32              i4HdrContrastPercentH;
    MINT32              i4HdrContrastPercentL;
    MINT32              i4HdrNightEVDiffPercentH;
    MINT32              i4HdrNightEVDiffPercentL;
    MINT32              i4HdrNightDarkPercent;
    AE_TARGET_PROB_T    rHdrNightEVDiffProb;
    AE_TARGET_PROB_T    rHdrNightHistProb;
    AE_TARGET_PROB_T    rHdrNightLVProb;
} HDR_DETECT_HIST_T;

typedef struct
{
    MBOOL               bGeometryCOEEnable;
    MINT32              i4COEWeight;                // x1 = 1024
    AE_TARGET_PROB_T    rHdrCOEOuterRatio;          // the outer y ratio
    AE_TARGET_PROB_T    rHdrCOEDiffRatio;           // inner/outer y difference ratio
} HDR_GEOMETRY_COE_T;

typedef struct
{
    BOOL                bGeometryABLEnable;
    MINT32              i4ABLWeight;                // x1 = 1024
    AE_TARGET_PROB_T    rCenterBrightDiffRatio;     // C-type / U-type y difference ratio
    AE_TARGET_PROB_T    rCenterDarkDiffRatio;       // C-type / U-type y difference ratio
} HDR_GEOMETRY_ABL_T;

typedef struct
{
    HDR_GEOMETRY_COE_T rGeometryCOEProp;
    HDR_GEOMETRY_ABL_T rGeometryABLProp;
} HDR_DETECT_GEOMETRY_T;

typedef struct
{
    BOOL    bLVDetectEnable;
    MINT32  i4LVProbTbl[HDR_AE_LV_TBL_NUM];
} HDR_DETECT_LV_T;

typedef struct
{
    BOOL                    bHdrDetectorEnable;
    MINT32                  i4HdrConfidenceThr;
    HDR_DETECT_HIST_T       rHdrHistInfo;
    HDR_DETECT_GEOMETRY_T   rHdrGeometryInfo;
    HDR_DETECT_LV_T         rHdrLVInfo;
} HDR_DETECTOR_T;

typedef struct
{
    MINT32                  i4NightPercent;
    AE_TARGET_PROB_T        rNightTarget;
} HDR_NIGHT_TARGET_T;

typedef struct
{
    MINT32                  i4AoePercent;
    MINT32                  i4AoeLV[3];
    MINT32                  i4AoeTarget[3];
} HDR_AOE_TARGET_T;

typedef struct {
    MINT32                  i4LVLimit;
    AE_TARGET_PROB_T        rTargetMinLimit;
    AE_TARGET_PROB_T        rTargetMaxLimit;
} HDR_TARGET_LIMIT_T;

typedef struct
{
    MINT32                  i4DefTarget;
    MINT32                  i4DefWeight;
    AE_TARGET_PROB_T        rAutoEVWeight;
    AE_TARGET_PROB_T        rAutoBrightWeight;
    AE_TARGET_PROB_T        rAutoABLWeight;
    AE_TARGET_PROB_T		rAutoLVWeightL;
    AE_TARGET_PROB_T		rAutoLVWeightH;
    AE_TARGET_PROB_T		rAutoNightWeight;
    HDR_AOE_TARGET_T        rHdrAoeTarget;
    HDR_NIGHT_TARGET_T      rHdrNightTarget;
    HDR_TARGET_LIMIT_T      rTargetLimitL;
    HDR_TARGET_LIMIT_T      rTargetLimitM;
    HDR_TARGET_LIMIT_T      rTargetLimitH;
} HDR_AE_TARGET_T;

typedef struct {
    AE_TARGET_PROB_T        rFaceHdrProb;
} HDR_FACE_T;

typedef struct {
    // TBD
} HDR_TOUCH_T;

typedef struct {
    HDR_FACE_T              rFaceAEProp;
    HDR_TOUCH_T             rTouchAEProp;
} HDR_MISC_PROP_T;

typedef struct
{
    AE_TARGET_PROB_T        rAcceProb;
    AE_TARGET_PROB_T        rGyroProb;
    AE_TARGET_PROB_T        rMotionRatio;
    MINT32                  i4MotionOnThr;
    MINT32                  i4MotionOnCntThr;

} HDR_G_GYRO_SENSOR_T;


typedef struct
{
    MINT32              i4LVRatio50[HDR_AE_LV_TBL_NUM];
    MINT32              i4LVRatio60[HDR_AE_LV_TBL_NUM];
    AE_TARGET_PROB_T    rContrastRatio;
    AE_TARGET_PROB_T    rBrightRatio;
    AE_TARGET_PROB_T    rNightProbRatio;
    HDR_G_GYRO_SENSOR_T rGGyroSensor;
} AUTO_BRIGHT_ENHANCE_T;

typedef struct
{
    // TBD
} AUTO_DARK_ENHANCE_T;

typedef struct
{
    MINT32  i4RmmExpoDiffThr;           //  %
    MINT32  i4LscRatio;                 //
} AUTO_TUNING_T;


typedef struct
{
    BOOL    bHdrWaitAEStable;
    MINT32  i4HdrInfoCounterThr;
    BOOL    bRatioSmoothEnable;
    MINT32  i4RatioSmoothThr[2];        //  [0]:normal, [1]:motion
    MINT32  i4RoughRatioUnit[2];        //  [0]:normal, [1]:motion
    MINT32  i4FinerRatioUnit[2];        //  [0]:normal, [1]:motion
    MINT32  i4RMGtUnit[2];              //  [0]:normal, [1]:motion
} AUTO_SMOOTH_T;

typedef struct
{
    MINT32 i4Flkr1LVThr[2];         // [0]: 50 Hz,  [1]: 60 Hz
    MINT32 i4MinRatioAntiFlk;
    MINT32 i4MaxExp[2];
    MINT32 i4MinExp[2];
    AE_TARGET_PROB_T rIsoThr;
} AUTO_FLICKER_RATIO_T;

typedef struct
{
    BOOL                    bAutoEnhanceEnable;
    MINT32                  i4RMGk;
    MINT32                  i4RMGtRatio;
    AE_TARGET_PROB_T        rRMGt;
    MINT32                  i4SupportHdrNum;
    MINT32                  i4AutoEnhanceProb;
    MINT32                  i4HdrWeight;
    AUTO_BRIGHT_ENHANCE_T   rAutoBright;
    AUTO_FLICKER_RATIO_T    rAutoFlicker;
    AUTO_SMOOTH_T           rAutoSmooth;
    AUTO_DARK_ENHANCE_T     rAutoDark;
    AUTO_TUNING_T           rAutoTuning;
} HDR_AUTO_ENHANCE_T;

typedef struct
{
    MINT32                  i4LEThr;
    MINT32                  i4SERatio;
    MINT32                  i4SETarget;
    MINT32                  i4BaseGain;
} HDR_CHDR_T;

typedef enum {
    eMainHDR_OFF = 0,
    eMainHDR_AUTO,
    eMainHDR_ON,
    eMainHDR_VT,
    eMain2HDR_OFF,
    eMain2HDR_AUTO,
    eMain2HDR_ON,
    eMain2HDR_VT,
    eSubHDR_OFF,
    eSubHDR_AUTO,
    eSubHDR_ON,
    eSubHDR_VT,
    eSub2HDR_OFF,
    eSub2HDR_AUTO,
    eSub2HDR_ON,
    eSub2HDR_VT
}eHDRPARAM_ID;

typedef struct
{
    eHDRPARAM_ID            eHdrParamID;
    HDR_DETECTOR_T          rHdrDetector;
    HDR_AUTO_ENHANCE_T      rHdrAutoEnhance;
    HDR_AE_TARGET_T         rHdrAETarget;
    HDR_MISC_PROP_T         rHdrMiscProp;
    HDR_CHDR_T              rHdrcHdr;
} HDR_AE_PARAM_T;


/*******************************************************************************
* Dynamic Frame Rate for Video
******************************************************************************/
typedef struct VdoDynamicFrameRate_S
{
    MBOOL   isEnableDFps;
    MUINT32 EVThresNormal;
    MUINT32 EVThresNight;
} VdoDynamicFrameRate_T;

//////////////////////////////////////////
//
//  AE Parameter structure
//  Define AE algorithm initialize parameter here
//
////////////////////////////////////////

#define MAX_WEIGHT_TABLE 4
#define MAX_MAPPING_PLINE_TABLE 30

struct AE_PARAMETER
{
    strAEParamCFG strAEParasetting;
    strWeightTable *pWeighting[MAX_WEIGHT_TABLE];   //AE WEIGHTING TABLE
    strAFPlineInfo strAFPLine;
    strAFPlineInfo strAFZSDPLine;
    strAFPlineInfo strStrobePLine;
    strAFPlineInfo strStrobeZSDPLine;
    MUINT32 *pEVValueArray;
    strAEMOVE *pAEMovingTable;
    strAEMOVE *pAEVideoMovingTable;
    strAEMOVE *pAEFaceMovingTable;
    strAEMOVE *pAETrackingMovingTable;
    strAELimiterTable strAELimiterData;
    VdoDynamicFrameRate_T strVdoDFps;

    AE_HIST_WIN_CFG_T *pAEBlockWINCFG;
    AE_HIST_WIN_CFG_T *pAEPixelWINCFG;
    // v1.2
    MBOOL   bOldAESmooth;                          // Select the new or old AE smooth control
    MBOOL   bEnableSubPreIndex;                // decide the sub camera re-initial index after come back to camera
    MUINT32 u4VideoLPFWeight; // 0~24

    strFaceLocSize *pFaceLocSizecheck;
    strPerframeCFG *pPerframeCFG;
    MBOOL   bPerFrameAESmooth;                    // Perframe AE smooth option
    MBOOL   bPerFrameHDRAESmooth;                    // Perframe AE smooth option

    // TG interrupt
    MBOOL   bTgIntAEEn;                    // TG interrupt option
    MBOOL   bSpeedupEscOverExp;
    MBOOL   bSpeedupEscUnderExp;
    strAETgTuningPara *pAETgTuningParam;
    MINT32 *pTgEVCompJumpRatioOverExp;
    MINT32 *pTgEVCompJumpRatioUnderExp;
    MUINT32 pTgWeightTbl[9][12];

    //Open AE
    strAEStableThd *pAEStableThd;
    strBVAccRatio   *pBVAccRatio;              // Preview ACC ratio
    strPSOConverge *pPSOConverge;
    strNonCWRAcc    *pNonCWRAcc;
    strFaceSmooth  *pFaceSmooth;
    strAECWRTempSmooth  *pAETempSmooth;
    strFaceLandMarkCtrl *pFaceLandmark;
    strAOEBVRef     *pAOERefBV;
    strNS_CDF       *pNsCdfRatio;

    //v2.5
    strAEMovingRatio *pAETouchMovingRatio;   //(temp) Touch smooth ratio
    strHSFlatSkyCFG     *pHSFlatSkyEnhance; //HS adaptive Prob Threshold by real BV
    strABL_absDiff      *pABLabsDiff;
    //v3.0
    strHSMultiStep    *pHSMultiStep;
    strHistStableCFG  *pHistStableCFG;
    strOverExpoAOECFG *pOverExpoAOECFG;
    //v4.0
    MBOOL bAEv4p0MeterEnable;
    strMainTargetCFG *pMainTargetCFG;
    strHSv4p0CFG *pHSv4p0CFG;
    strNSBVCFG *pNSBVCFG;

    // HDR
    HDR_AE_PARAM_T    *pHdrAECFG;

    // Stereo Pline mapping
    strAESceneMapping strStereoPlineMapping;
    // HDR Pline mapping
    strAESceneMapping strHDRPlineMapping;

    //Dynamic Flare
    strDynamicFlare *pDynamicFlareCFG;

};

typedef struct AE_PARAMETER AE_PARAM_T;

/***********************
    Exposure time value , use in AE TV mode
***********************/
typedef enum
{
    TV_1_2      =0x00000002,    //!<: TV= 1/2 sec
    TV_1_3      =0x00000003,    //!<: TV= 1/3 sec
    TV_1_4      =0x00000004,    //!<: TV= 1/4 sec
    TV_1_5      =0x00000005,    //!<: TV= 1/5 sec
    TV_1_6      =0x00000006,    //!<: TV= 1/6 sec
    TV_1_7      =0x00000007,    //!<: TV= 1/7 sec
    TV_1_8      =0x00000008,    //!<: TV= 1/8 sec
    TV_1_10     =0x0000000A,    //!<: TV= 1/10 sec
    TV_1_13     =0x0000000D,    //!<: TV= 1/13 sec
    TV_1_15     =0x0000000F,    //!<: TV= 1/15 sec
    TV_1_20     =0x00000014,    //!<: TV= 1/20 sec
    TV_1_25     =0x00000019,    //!<: TV= 1/25 sec
    TV_1_30     =0x0000001E,    //!<: TV= 1/30 sec
    TV_1_40     =0x00000028,    //!<: TV= 1/40 sec
    TV_1_50     =0x00000032,    //!<: TV= 1/50 sec
    TV_1_60     =0x0000003C,    //!<: TV= 1/60 sec
    TV_1_80     =0x00000050,    //!<: TV= 1/80 sec
    TV_1_100    =0x00000064,    //!<: TV= 1/100 sec
    TV_1_125    =0x0000007D,    //!<: TV= 1/125 sec
    TV_1_160    =0x000000A0,    //!<: TV= 1/160  sec
    TV_1_200    =0x000000C8,    //!<: TV= 1/200 sec
    TV_1_250    =0x000000FA,    //!<: TV= 1/250 sec
    TV_1_320    =0x00000140,    //!<: TV= 1/320 sec
    TV_1_400    =0x00000190,    //!<: TV= 1/400 sec
    TV_1_500    =0x000001F4,    //!<: TV= 1/500 sec
    TV_1_640    =0x00000280,    //!<: TV= 1/640 sec
    TV_1_800    =0x00000320,    //!<: TV= 1/800 sec
    TV_1_1000   =0x000003E8,    //!<: TV= 1/1000 sec
    TV_1_1250   =0x000004E2,    //!<: TV= 1/1250 sec
    TV_1_1600   =0x00000640,    //!<: TV= 1/1600 sec

    TV_1_1      =0xFFFF0001,    //!<: TV= 1sec
    TV_2_1      =0xFFFF0002,    //!<: TV= 2sec
    TV_3_1      =0xFFFF0003,    //!<: TV= 3sec
    TV_4_1      =0xFFFF0004,    //!<: TV= 4sec
    TV_8_1      =0xFFFF0008,    //!<: TV= 8sec
    TV_16_1     =0xFFFF0016    //!<: TV= 16 sec
}eTimeValue;

/***********************
    Apertur time value , use in AE AV mode
    It's impossible list all Fno in enum
    So choose most close Fno.in enum
    and set real value in  structure "strAV.AvValue"
***********************/
typedef enum
{
    Fno_2,       //!<: Fno 2.0
    Fno_2_3,     //!<: Fno  2.3
    Fno_2_8,     //!<: Fno 2.8
    Fno_3_2,     //!<: Fno 3.2
    Fno_3_5,     //!<: Fno 3.5
    Fno_4_0,     //!<: Fno 4.0
    Fno_5_0,     //!<: Fno 5.0
    Fno_5_6,     //!<: Fno 5.6
    Fno_6_2,     //!<: Fno 6.2
    Fno_8_0,     //!<: Fno 8.0

    Fno_MAx
}eApetureValue ;


// AE Input/Output Structure
typedef enum
{
    AE_STATE_CREATE,            // 0  CREATE , JUST CREATE
    AE_STATE_INIT,              // 1  INIT
    AE_STATE_NORMAL_PREVIEW,    // 2  normal AE
    AE_STATE_AFASSIST,          // 3  aF assist mode, limit exposuret time
    AE_STATE_AELOCK,            // 4  LOCK ae
    AE_STATE_CAPTURE,           // 5  capture
    AE_STATE_ONE_SHOT,          // 6  one shot AE
    AE_STATE_SLOW_MOTION,       // 7  slow motion AE
    AE_STATE_PANORAMA_MODE,     // 8  Panorama mode
    AE_STATE_BACKUP_PREVIEW,    // 9  backup preview
    AE_STATE_RESTORE_PREVIEW,   //10 backup preview
    AE_STATE_POST_CAPTURE,      //11 backup preview
    AE_STATE_TOUCH_PERFRAME,    //12 Touch ae peframe
    AE_STATE_MAX
}eAESTATE;

typedef struct
{
    MINT32 m_i4CycleVDNum;
    MINT32 m_i4ShutterDelayFrames;
    MINT32 m_i4GainDelayFrames;
    MINT32 m_i4IspGainDelayFrames;
    MINT32 m_i4FrameCnt;
    MINT32 m_i4FrameTgCnt; // doPvAE_TG_INT update
    MUINT32 m_u4HwMagicNum; // doPvAE update
}strAECycleInfo;

typedef struct
{
    eAESTATE            eAeState;   //ae state
    void*               pAESatisticBuffer;
    HDR_GGYRO_INFO_T    rHdrGyroInfo;
    eAETargetMODE       eAeTargetMode;
    strEvSetting        PreEvSetting[MAX_AE_PRE_EVSETTING];
    strAECycleInfo      CycleInfo;
    MUINT32             u4PreAEidx[MAX_AE_PRE_EVSETTING];      // AE index (normal preview)
    MUINT32             u4PreTgIntAEidx[MAX_AE_PRE_EVSETTING]; // AE index (TG interrupt AE)
    MINT64              i8TimeStamp; // FD time stamp
    MBOOL               bUpdateAEidxFromCCU;
    MINT32              i4CCUAEidxNext;
} strAEInput;

typedef struct
{
    MUINT32        u4GR;
    MUINT32        u4R;
    MUINT32        u4B;
    MUINT32        u4GB;
} strSensorAWBGain;

typedef struct {
	MINT32 i4RMGt;
	MINT32 i4RMGk;
    HDR_EV_INFO_T rHdrEVInfo;
    MINT32 i4HdrOnOff;
    MINT32 i4HdrConfidence;
    MINT32 i4BrightConfidence;
    MINT32 i4DarkConfidence;
    MINT32 i4cHdrGainBase;                                  /*  Video HDR 0 EV info.  */
    MINT32 i4cHdrSEInfo;                                    /*  Delta EV compared with i4GaniBase  */
    MINT32 i4cHdrLEInfo;                                    /*  Low averege data  */
	MINT32 i4SwitchHLR;
} strEvHdrSetting;
typedef enum
{
    SYNC_AE_DUAL_CAM_DENOISE_BMDN = 0,
    SYNC_AE_DUAL_CAM_DENOISE_MFNR,
    SYNC_AE_DUAL_CAM_DENOISE_MAX
} SYNC_AE_DUAL_CAM_DENOISE_ENUM;

typedef struct
{
    MBOOL           bAEStable;      // Only used in Preview/Movie
    MBOOL           bGammaEnable;   // gamma enable
    strEvSetting    EvSetting;
	strEvHdrSetting HdrEvSetting;
    MUINT32        u4CWValue;
    MUINT32        u4FracGain;    // stepless gain, lower than 0.1EV
    MINT32         Bv;
    MINT32         AoeCompBv;
    MINT32         i4EV;
    MUINT32        u4AECondition;
    MINT32         i4DeltaBV;
    MINT32         i4PreDeltaBV;
    MUINT32        u4DeltaBVRatio[SYNC_AE_DUAL_CAM_DENOISE_MAX];
    MUINT32        u4ISO;          //correspoing ISO , only use in capture
    MUINT16        u2FrameRate;     // Calculate the frame
    MINT16        i2FlareOffset;
    MINT16        i2FlareGain;   // in 512 domain
    MINT16        i2FaceDiffIndex;
    MINT32        i4AEidxCurrent;  // current AE idx
    MINT32        i4AEidxCurrentF;  // current AE idx
    MINT32        i4AEidxNext;   // next AE idx
    MINT32        i4AEidxNextF;   // next AE idx
    MINT32        i4gammaidx;   // next gamma idx
    MINT32        i4LESE_Ratio;    // LE/SE ratio
    MINT32        i4cHdrSEDeltaEV;      //for sw HDR SE ,  -(x*100) EV , compare with converge AE
} strAEOutput;


typedef struct {
    MBOOL bEnableWorkaround;    // MTRUE : enable, MFALSE : disable
    MUINT32 u4SEExpTimeInUS;     // unit: us short exposure
    MUINT32 u4SESensorGain;        // 1x=1204 sensor gain
    MUINT32 u4SEISPGain;              // 1x=1204 isp gain
    MUINT32 u4LEExpTimeInUS;     // unit: us long exposure
    MUINT32 u4LESensorGain;        // 1x=1204 sensor gain
    MUINT32 u4LEISPGain;              // 1x=1204 isp gain
    MUINT32 u4LE_SERatio_x100;   // 100x
    strEvHdrSetting  HdrEvSetting;
} rVHDRExpSettingInfo_T;


typedef struct
{
   MUINT32 u4HighY;
   MUINT32 u4LowY;
   MUINT32 u4Maxbin;
   MUINT32 u4Brightest;
   MUINT32 u4Darkest;
   MUINT32 u4BrightHalf;
   MUINT32 u4DarkHalf;
   MUINT32 u4DownSideBrightest;
   MUINT32 u4FullBrightest;
   MUINT32 u4MaxLumiBin;
   MUINT32 u4MaxLumiBin2;       // MVHDR Application
   MUINT32 u4LowPercentBin;
   MUINT32 u4LE_LowAvg;         // 0 ~ 39 Average
   MUINT32 u4AOE_Seg_Count[4];
   MUINT32 u4AOE_Seg_Avg[4];
}strHistInfo;//histogram information

typedef struct
{
    MUINT32  u4CWValue;
    MUINT32  u4Dir;
    MUINT32  u4GreenCount;
    MUINT32  u4FaceMean;
    MUINT32*  pu4Hist1;
    MUINT32*  pu4Hist2;
    MUINT32*  pu4Hist3;
    strHistInfo sHistInfo;
}strAEInterInfo;

//Low Pass filer filter
#define G_FILTER_TAPIZE  8
#define  LPF_BUFFER_SIZE  G_FILTER_TAPIZE

typedef struct
{
    MUINT32 u4Idx ;          //index of ring buffer
    MUINT32 u4valid;         //valid data in ring buffer
    MUINT32 pu4LPFBuffer[LPF_BUFFER_SIZE]; //ring low pass buffer
    MUINT32 u4LPFLevel;                   //low pass filter level ;

}strTimeLPF;


typedef struct
{
    MUINT32 u4LpfMin;          // min lpf that be used
    MUINT32 u4LpfMax;         /// max lpf that be used

//
//
//  max LPF  level~~~~~~|       ------------
//                      |      //:          :\\,
//                      |     // :          : \\,
//                      |    //  :          :  \\,
//  min LPF level~~~~~~~|---//------------------\\---
//                          p1  p2         p3 p4    delta_idx

    MUINT32 u4p1;              // p1 please reference chart , 8X delta index
    MUINT32 u4p2;
    MUINT32 u4p3;
    MUINT32 u4p4;
}strLpfConfig;

//////////////////////////////
//  enum of AE condition
//
//////////////////////////////
enum
{
    AE_CONDITION_NORMAL=0x00,
    AE_CONDITION_BACKLIGHT=0x01,
    AE_CONDITION_OVEREXPOSURE=0x02,
    AE_CONDITION_HIST_STRETCH=0x04,
    AE_CONDITION_SATURATION_CHECK=0x08,
    AE_CONDITION_FACEAE=0x10,
    AE_CONDITION_MAINTARGET=0x20,
    AE_CONDITION_HS_V4P0=0x40
};

// AE algorithm parameter
//typedef struct
//{
//    struct_AE_Para   strAEPara;      //AE algorithm parameter
//    struct_AE           strAEStatConfig;   //AE statistic configuration
//} AE_PARAM_T;

typedef struct
{
    MBOOL bZoomChange;
    MUINT32 u4XOffset;
    MUINT32 u4YOffset;
    MUINT32 u4XWidth;
    MUINT32 u4YHeight;
} EZOOM_WINDOW_T;

typedef struct
{
    MUINT32 u4XLow;
    MUINT32 u4XHi;
    MUINT32 u4YLow;
    MUINT32 u4YHi;
    MUINT32 u4Weight;
} AE_BLOCK_WINDOW_T;

typedef struct
{
    MUINT32 u4XLow;
    MUINT32 u4XHi;
    MUINT32 u4YLow;
    MUINT32 u4YHi;
    MUINT32 u4Weight;
    MINT32  i4Id;
    MINT32  i4Type; // 0:GFD, 1:LFD, 2:OT
    MINT32  i4Rop;
    MINT32  i4Motion[2];
    MINT32  i4Landmark_XLow[3]; // index 0: left eye, index 1: right eye, index 2:mouth
    MINT32  i4Landmark_XHi[3];
    MINT32  i4Landmark_YLow[3];
    MINT32  i4Landmark_YHi[3];
    MINT32  i4LandmarkCV;
    MBOOL   bLandMarkBullsEye;
} AE_FD_BLOCK_WINDOW_T;

typedef enum
{
    AE_SENSOR_MAIN = 0,
    AE_SENSOR_SUB,
    AE_SENSOR_MAIN2,
    AE_SENSOR_SUB2,
    AE_SENSOR_MAX
} AE_SENSOR_DEV_T;

//AE Sensor Config information
typedef struct
{
    AE_NVRAM_T rAENVRAM;         // AE NVRAM param
    AE_PARAM_T rAEPARAM;
    AE_PLINETABLE_T *rAEPlineTable;
    AE_PLINEMAPPINGTABLE_T rAEPlineMapTable[30];
    EZOOM_WINDOW_T rEZoomWin;
    MINT32 i4AEMaxBlockWidth;  // AE max block width
    MINT32 i4AEMaxBlockHeight; // AE max block height
    MINT32 i4AAOLineByte;
    LIB3A_AE_METERING_MODE_T eAEMeteringMode;
    LIB3A_AE_SCENE_T eAEScene;
    LIB3A_AECAM_MODE_T eAECamMode;
    LIB3A_AE_FLICKER_MODE_T eAEFlickerMode;
    LIB3A_AE_FLICKER_AUTO_MODE_T eAEAutoFlickerMode;
    LIB3A_AE_EVCOMP_T eAEEVcomp;
    MUINT32 u4AEISOSpeed;
    MINT32    i4AEMaxFps;
    MINT32    i4AEMinFps;
    MINT32    i4SensorMode;
    AE_SENSOR_DEV_T eSensorDev;
    eAETargetMODE eAETargetMode;
    CAM_SCENARIO_T eAEScenario;
    //AAO Buffer Size
    MUINT32 u4MGRSepAWBsize;      // awb: int8[4*w*h]
    MUINT32 u4MGRSepAEsize;       // ae : int16[w*h]
    MUINT32 u4MGRSepHistsize;     // hist : int32[128*4]
    MUINT32 u4MGRSepOverCntsize;  // overexpcnt : int16[w*h]
    MUINT32 u4MGRSepPSORGBsize;
    MUINT32 u4MGRSepPSOYsize;
} AE_INITIAL_INPUT_T;

#if 0
//Handle AE input/output
typedef struct
{
    MUINT32 u4AEWindowInfo[25];
    MUINT32 u4AEHistogram[64];
    MUINT32 u4FlareHistogram[10];
    MUINT32 u4AEBlockCnt;
    FD_AE_STAT_T rFDAEStat;
} AE_STAT_T;
#endif

//AAA_OUTPUT_PARAM_T use strAEOutput
typedef struct
{
    MUINT32 u4ExposureMode;  // 0: exposure time, 1: exposure line
    MUINT32 u4Eposuretime;   //!<: Exposure time in ms
    MUINT32 u4AfeGain;       //!<: sensor gain
    MUINT32 u4IspGain;       //!<: raw gain
    MUINT16 u2FrameRate;
    MUINT16 u4CWValue;      // for N3D sync
    MUINT32 u4RealISO;      //!<: ISO speed
    MINT16   i2FlareOffset;
    MINT16   i2FlareGain;   // 512 is 1x
}AE_MODE_CFG_T;

typedef struct
{
    AE_MODE_CFG_T rPreviewMode;
    AE_MODE_CFG_T rAFMode;
    AE_MODE_CFG_T rCaptureMode[3];
}AE_OUTPUT_T;

typedef struct
{
    MBOOL bAEHistEn;
    MUINT8 uAEHistOpt;    // output source
    MUINT8 uAEHistBin;    // bin mode
    MUINT16 uAEHistYHi;
    MUINT16 uAEHistYLow;
    MUINT16 uAEHistXHi;
    MUINT16 uAEHistXLow;
} AE_HIST_WIN_T;

//AE Statistic window config
typedef struct
{
    AE_HIST_WIN_T rAEHistWinCFG[4];
    AE_HIST_WIN_T rAEPixelHistWinCFG[4];
} AE_STAT_PARAM_T;

typedef struct
{
    MUINT32 u4SensorExpTime;
    MUINT32 u4SensorGain;
    MUINT32 u4IspGain;
    MUINT32 u4ISOSpeed;
}AE_EXP_GAIN_MODIFY_T;

typedef struct AEMeterArea {
    MINT32 i4Left;
    MINT32 i4Top;
    MINT32 i4Right;
    MINT32 i4Bottom;
    MINT32 i4Weight;
    MINT32 i4Id;
    MINT32 i4Type; // 0:GFD, 1:LFD, 2:OT
    MINT32 i4Motion[2];
    MINT32 i4Landmark[3][4]; // index 0: left eye, index 1: right eye, index 2:mouth
    MINT32 i4ROP;
    MINT32 i4LandmarkCV;
} AEMeterArea_T;


#define MAX_AE_METER_AREAS  9

typedef struct AEMeteringArea {
    AEMeterArea_T rAreas[MAX_AE_METER_AREAS];
    MUINT32 u4Count;
    MINT64 i8TimeStamp; // FD time stamp
} AEMeteringArea_T;

// AE info for ISP tuning
typedef struct
{
    MUINT32 u4AETarget;
    MUINT32 u4AECurrentTarget;
    MUINT32 u4Eposuretime;   //!<: Exposure time in ms
    MUINT32 u4AfeGain;           //!<: raw gain
    MUINT32 u4IspGain;           //!<: sensor gain
    MUINT32 u4RealISOValue;
    MINT32  i4LightValue_x10;
    MINT32  i4RealLightValue_x10;   //real LV
    MUINT32 u4AECondition;
    LIB3A_AE_METERING_MODE_T eAEMeterMode;
    MINT16  i2FlareOffset;
    MINT32  i4GammaIdx;   // next gamma idx
    MINT32  i4LESE_Ratio;    // LE/SE ratio
    MINT32  i4HdrSeg;
    MINT32  i4HdrTurn;
    MUINT32 u4SWHDR_SE;      //for sw HDR SE ,  -x EV , compare with converge AE
    MUINT32 u4MaxISO;
    MUINT32 u4AEStableCnt;
    MUINT32 u4OrgExposuretime;   //!<: Exposure time in ms
    MUINT32 u4OrgRealISOValue;
    MUINT32 u4Histogrm[AE_HISTOGRAM_BIN];
    MBOOL   bGammaEnable;
    MUINT32 u4EVRatio;
    MBOOL   bAEStable;
    MBOOL   bAELock;
    MBOOL   bAELimiter;
    MBOOL   bDisableOBC;
    strEvHdrSetting    rHdrAEInfo;
    MINT32    i4HdrExpoDiffThr;
    MINT32  u4EISExpRatio;
    MUINT32 u4AEidxCurrent;
    MUINT32 u4AEidxCurrentF;
    MUINT32 u4AEidxNext;
    MUINT32 u4AEidxNextF;
    MINT32  i4DeltaIdx;
    // for shading smooth start
    MBOOL   bEnableRAFastConverge;
    MUINT32 pu4CWVAEBlock[AE_BLOCK_NO][AE_BLOCK_NO];
    MUINT32 pu4AEBlock[AE_BLOCK_NO][AE_BLOCK_NO];
    MUINT32 pu4Weighting[AE_BLOCK_NO][AE_BLOCK_NO];
    MUINT32 u4AvgWValue;
    MUINT32 u4MgrCWValue;
    MINT32  i4deltaIndex;
    MINT32  i4Cycle0FullDeltaIdx;
    MINT32  i4FrameCnt;
    MUINT32 u4WeightingSum;
    MBOOL  TgCtrlRight;
    MINT32  i4EVRatio;
    MINT32  u4AEFinerEVIdxBase;
    MBOOL   bAEScenarioChange;
    // for shading smooth end
} AE_INFO_T;

// AE info for ISP tuning
typedef struct
{
    MBOOL bEnable;
    MBOOL bEquivalent;                            // equivalent with orginal AE Pline table
    MUINT32 u4IncreaseISO_x100;          // increase ISO ratio 100 = 1x
    MUINT32 u4IncreaseShutter_x100;    // increase shutter ratio 100 = 1x
} AE_PLINE_LIMITATION_T;

// Sensor Input params for Camer 3
typedef struct
{
    MINT64 u8FrameDuration;   // naro sec
    MINT64 u8ExposureTime;   // naro sec
    MINT32 u4Sensitivity;          //ISO value
} AE_SENSOR_PARAM_T;

typedef struct
{
    MINT32 u4FrameDuration;   // micro sec
    MINT32 u4ExposureTime;    // micro sec
    MINT32 u4Sensitivity;          //ISO value
} AE_SENSOR_PARAM_QUEUE_T;

typedef struct
{
    MUINT8 uInputIndex;
    MUINT8 uOutputIndex;
    AE_SENSOR_PARAM_QUEUE_T rSensorParamQueue[AE_SENSOR_MAX_QUEUE];
} AE_SENSOR_QUEUE_CTRL_T;



typedef enum
{
	EVEN_COUNT_FRAME,
	ODD_COUNT_FRAME
}eAE_INTERFRAME_COUNT;



#endif


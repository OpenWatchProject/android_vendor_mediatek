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

#include <ae_feature.h>
#include <camera_custom_nvram.h>
#include <awb_param.h>
#include <camera_custom_AEPlinetable.h>

#define AE_PARAM_REVISION 	6131004
#define AE_BLOCK_NO  5
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
    MBOOL   bEnable;
    MUINT32 u4IdxNum;
    MUINT32 u4X_BV[16];
    MUINT32 u4Y_GmaIdx[16];
    MUINT32 u4MidPointThd[16];
} strGammaCFG;

typedef struct {
    MBOOL   bFlatSceneCheck;
    MUINT32 u4B1Pcent;
    MUINT32 u4B2Pcent;
    MUINT32 u4BP_X[2];
    MUINT32 u4BP_Y[2];
} strFlatScene;
typedef struct {
    MBOOL   bFaceLocSizeCheck;
    MUINT32 u4Size_X[2];
    MUINT32 u4Size_Y[2];
    MUINT32 u4Loc_X[2];
    MUINT32 u4Loc_Y[2];
} strFaceLocSize;

typedef struct {
    MUINT32 u4EqUpRatio;
    MUINT32 u4EqDownRatio;
    MINT32  i4DeltaEVDiff;
    MUINT32 u4EqRatio_Range_U;// 1080
    MUINT32 u4EqRatio_Range_D;// 920
    MINT32  i4EqRatio_Range_Diff;// 200
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
} strAEStableThd;
typedef struct
{
    MBOOL   bEnable;
    MUINT32 u4TblLength;
    MUINT32 u4RatioTbl_X[256];
    MUINT32 u4WghtTbl_Y[256];
} strFaceBgYWeight;

typedef struct {
    MBOOL u4BVAccEnable;
    MINT32 i4DeltaBVRatio;
    MUINT32 u4B2T_Target;
    strAEMovingRatio  pAEBVAccRatio;           // Preview ACC ratio
} strBVAccRatio;
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
}strFaceSmooth;
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
    MBOOL   bEnableVideoAEStableTHD;         // enable video ae stable threshold setting
    MUINT32 u4BackLightStrength;              // strength of backlight condtion
    MUINT32 u4OverExpStrength;               // strength of anti over exposure
    MUINT32 u4HistStretchStrength;           //strength of  histogram stretch
    MUINT32 u4SmoothLevel;                      // time LPF smooth level , internal use
    MUINT32 u4TimeLPFLevel;                     //time LOW pass filter level
    MUINT32 u4AEWinodwNumX;                   // AE statistic winodw number X
    MUINT32 u4AEWinodwNumY;                   // AE statistic winodw number Y
    MUINT8 uBlockNumX;                         //AE X block number
    MUINT8 uBlockNumY;                         //AE Yblock number
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
    MUINT8 uSatBlockCheckLow;             //saturation block check , low thres
    MUINT8 uSatBlockCheckHigh;            //sturation  block check , hight thres
    MUINT8 uSatBlockAdjustFactor;        // adjust factore , to adjust central weighting target value
    MUINT8 uMeteringYLowSkipRatio;     // metering area min Y value to skip AE
    MUINT8 uMeteringYHighSkipRatio;    // metering area max Y value to skip AE
    MUINT32 u4MinYLowBound;                 // metering boundary min Y value
    MUINT32 u4MaxYHighBound;                // metering boundary max Y value
    MUINT32 u4MinCWRecommend;           // mini target value
    MUINT32 u4MaxCWRecommend;          // max target value
    MINT8   iMiniBVValue;                          // mini BV value.
    MINT8   uAEShutterDelayCycle;         // for AE smooth used.
    MINT8   uAESensorGainDelayCycleWShutter;
    MINT8   uAESensorGainDelayCycleWOShutter;
    MINT8   uAEIspGainDelayCycle;

    // v1.2
    MUINT32   u4LongCaptureThres;          // enable the log capture control sequence
    MUINT32   u4CenterFaceExtraWeighting;  // give center face an extra weighting
    MUINT16 u2AEStatThrs;
    MUINT8  uCycleNumAESkipAfterExit;
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

    // v1.2
    MBOOL   bOldAESmooth;                          // Select the new or old AE smooth control
    MBOOL   bEnableSubPreIndex;                // decide the sub camera re-initial index after come back to camera
    MUINT32 u4VideoLPFWeight; // 0~24

    strGammaCFG   *pDGamma;
    strFlatScene  *pFlatSceneCheck;
    strFaceLocSize *pFaceLocSizecheck;
    strPerframeCFG *pPerframeCFG;
    MBOOL   bPerFrameAESmooth;                  // Perframe AE smooth option

    //Open AE
    strAEStableThd *pAEStableThd;
    strBVAccRatio   *pBVAccRatio;               // Preview ACC ratio
    strNonCWRAcc    *pNonCWRAcc;
    strFaceSmooth  *pFaceSmooth;
    strFaceBgYWeight *pFaceBgYWeightingTbl;
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
    AE_STATE_CREATE,         // CREATE , JUST CREATE
    AE_STATE_INIT,           //  INIT
    AE_STATE_NORMAL_PREVIEW, // normal AE
    AE_STATE_AFASSIST,       // aF assist mode, limit exposuret time
    AE_STATE_AELOCK,         // LOCK ae
    AE_STATE_CAPTURE,        //capture
    AE_STATE_ONE_SHOT, // one shot AE
    AE_STATE_SLOW_MOTION,    // slow motion AE
    AE_STATE_PANORAMA_MODE,  // Panorama mode
    AE_STATE_BACKUP_PREVIEW,
    AE_STATE_RESTORE_PREVIEW,
    AE_STATE_POST_CAPTURE,
    AE_STATE_MAX
}eAESTATE;

typedef enum
{
    AE_MODE_NORMAL,
    AE_MODE_AOE_TARGET,
    AE_MODE_MVHDR_TARGET,
	AE_MODE_LE_FIX_TARGET,
	AE_MODE_SE_FIX_TARGET,
}eAETARGETMODE;

typedef struct
{
    MINT32 m_i4CycleVDNum;
    MINT32 m_i4ShutterDelayFrames;
    MINT32 m_i4GainDelayFrames;
    MINT32 m_i4IspGainDelayFrames;
    MINT32 m_i4FrameCnt;
}strAECycleInfo;

typedef struct
{
    eAESTATE  eAeState;   //ae state
    void*     pAESatisticBuffer;
    eAETARGETMODE eAeTargetMode;
    strEvSetting   PreEvSetting[MAX_AE_PRE_EVSETTING];
    strAECycleInfo CycleInfo;
} strAEInput;

typedef struct
{
    MUINT32        u4GR;
    MUINT32        u4R;
    MUINT32        u4B;
    MUINT32        u4GB;
} strSensorAWBGain;

typedef enum
{
    SYNC_AE_DUAL_CAM_DENOISE_BMDN = 0,
    SYNC_AE_DUAL_CAM_DENOISE_MFNR,
    SYNC_AE_DUAL_CAM_DENOISE_MAX
} SYNC_AE_DUAL_CAM_DENOISE_ENUM;
typedef struct
{
    MBOOL          bAEStable;      // Only used in Preview/Movie
    MBOOL          bGammaEnable;   // gamma enable
    strEvSetting   EvSetting;
    MUINT32        u4CWValue;
    MUINT32        u4FracGain;    // stepless gain, lower than 0.1EV
    MINT32         Bv;
    MINT32         AoeCompBv;
    MINT32         i4EV;
    MUINT32        u4AECondition;
    MINT32         i4DeltaBV;
    MUINT32        u4ISO;          //correspoing ISO , only use in capture
    MUINT16        u2FrameRate;     // Calculate the frame
    MINT16        i2FlareOffset;
    MINT16        i2FlareGain;   // in 512 domain
    MINT16        i2FaceDiffIndex;
    MINT32        i4AEidxCurrent;  // current AE idx
    MINT32        i4AEidxNext;   // next AE idx
    MINT32        i4gammaidx;   // next gamma idx
    MINT32        i4LESE_Ratio;    // LE/SE ratio
    MINT32        i4_swHDR_SEx100;      //for sw HDR SE ,  -(x*100) EV , compare with converge AE
} strAEOutput;

typedef struct {
	eAETARGETMODE eAeTargetMode;
	MINT32 i4GainBase;
	MINT32 i4SE_DeltaEV_x100;
	MINT32 i4LE_LowAvg;
}SW_HDR_AE_INFO_T;


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
   MUINT32 u4LE_LowAvg;			// 0 ~ 39 Average
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
    AE_CONDITION_FACEAE=0x10
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

typedef enum
{
    AE_SENSOR_MAIN = 0,
    AE_SENSOR_SUB,
    AE_SENSOR_MAIN2
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
    eAETARGETMODE eAETargetMode;
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
	  MUINT32 u4ExposureMode;	  // 0: exposure time, 1: exposure line
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
    MUINT8 uAEHistYHi;
    MUINT8 uAEHistYLow;
    MUINT8 uAEHistXHi;
    MUINT8 uAEHistXLow;
} AE_HIST_WIN_T;

//AE Statistic window config
typedef struct
{
    AE_HIST_WIN_T rAEHistWinCFG[4];
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
} AEMeterArea_T;


#define MAX_AE_METER_AREAS  9

typedef struct AEMeteringArea {
	AEMeterArea_T rAreas[MAX_AE_METER_AREAS];
	MUINT32 u4Count;
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
    MINT32   i4LightValue_x10;
    MUINT32 u4AECondition;
    LIB3A_AE_METERING_MODE_T eAEMeterMode;
    MINT16   i2FlareOffset;
    MINT32        i4GammaIdx;   // next gamma idx
    MINT32  i4LESE_Ratio;    // LE/SE ratio
    MUINT32 u4SWHDR_SE;      //for sw HDR SE ,  -x EV , compare with converge AE
    MUINT32 u4MaxISO;
    MUINT32 u4AEStableCnt;
    MUINT32 u4OrgExposuretime;   //!<: Exposure time in ms
    MUINT32 u4OrgRealISOValue;
    MUINT16 u2Histogrm[AE_HISTOGRAM_BIN];
    MBOOL bGammaEnable;
    MUINT32 u4EVRatio;
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
#endif


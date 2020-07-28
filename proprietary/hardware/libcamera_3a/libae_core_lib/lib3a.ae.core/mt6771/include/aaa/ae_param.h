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

#include "common/hal/inc/custom/aaa/ae_param.h"

#include "ae_feature.h"
#include "../camera_custom_ae_nvram.h"
#include "camera_custom_3a_nvram.h" //For CAM_SCENARIO_T
#include "aaa_types.h"

#include "awb_param_for_ae.h"
// #include "awb_param.h"
// #include <camera_custom_AEPlinetable.h>
// #include "../AEPlinetable.h"
#define AE_PARAM_REVISION    7482001
#define AE_ALGO_TOOL_REVISION    5000000       // ex: 4 01 => formal release 00=>internal change 00=>customer only
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
#define AE_METER_GAIN_BASE 1024
#define MAX_AE_METER_AREAS  9

#define AE_PARAM_AE_HST_DMA_OUT_EN 1
#define AE_PARAM_AE_LINEAR_STAT_EN 1
#define AE_PARAM_PS_RMBN_SEL 3
#define AE_PARAM_PS_EXP_MODE 0
#define AE_PARAM_PS_HST_SEP_G 0
#define AE_PARAM_PS_CCU_HST_END 81




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
    MINT64 i8AAOTimeStamp;
    MINT32 i4IsAFDone;
    MINT32 i4AfDac;
    MINT32 i4IsSceneStable;

    MUINT8 *AAOBuffer;
    MUINT8 *PSOBuffer;


}AAO_PROC_INFO_T;

typedef struct
{
    MUINT32 u4sizewidth;
    MUINT32 u4sizeheight;
    MUINT32 u4ispgain;
    MUINT32 u4HDRratio;
} AE_3EXPO_PROC_INFO_T;


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
#define AAO_SEP_AWB_SIZE_COEF  4       // AAO AWB size coefficient awb AAO 1byte to 2byte
#define AAO_SEP_AE_SIZE_COEF   2       // AAO AE size coefficient
#define AAO_SEP_HIST_SIZE_COEF (4 * 4) // AAO HIST size coefficient
#define AAO_SEP_OC_SIZE_COEF   2       // AAO OverCnt size coefficient
#define AAO_SEP_LSC_SIZE_COEF  (4 * 2) // AAO LSC size coefficient
// PSO separation size coefficients
#define PSO_SEP_AWB_SIZE_COEF  4       // PSO AWB size coefficient
#define PSO_SEP_AE_SIZE_COEF   1       // PSO AE size coefficient


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


/*
*   HDR Application
*/
#define SUPPORTED_MAX_HDR_EXPNUM            (4)
#define HDR_AE_LV_TBL_NUM                   (19)






//////////////////////////////////////////
//
//  AE Parameter structure
//  Define AE algorithm initialize parameter here
//
////////////////////////////////////////

#define MAX_MAPPING_PLINE_TABLE 30


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
    MINT32              i4MagicNum;
    MINT32              i4IsAFDone;
    MINT32              i4AfDac;   
    MINT32              i4IsSceneStable;
} strAEInput;

typedef struct
{
    MUINT32        u4GR;
    MUINT32        u4R;
    MUINT32        u4B;
    MUINT32        u4GB;
} strSensorAWBGain;



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
    MUINT32 u4LandMarkXLow;
    MUINT32 u4LandMarkXHi;
    MUINT32 u4LandMarkYLow;
    MUINT32 u4LandMarkYHi;
    MINT32  i4LandMarkICSWeighting;
    MINT32  i4LandMarkRip;
    MINT32  i4LandMarkRop;
} AE_FD_BLOCK_WINDOW_T;

typedef enum
{
    AE_SENSOR_MAIN = 0,
    AE_SENSOR_SUB,
    AE_SENSOR_MAIN2,
    AE_SENSOR_SUB2,
    AE_SENSOR_MAIN3,
    AE_SENSOR_MAX
} AE_SENSOR_DEV_T;

//AE Sensor Config information
typedef struct
{
    AE_NVRAM_T *rAENVRAM;         // AE NVRAM param
    AE_PARAM_TEMP_T rAEPARAM;
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

    // AAO/PSO window size for isp5.0 
    MUINT32 u4AAO_AWBValueWinSize;            // bytes of AWB Value window
    MUINT32 u4AAO_AWBSumWinSize;              // bytes of AWB Sum window 
    MUINT32 u4AAO_AEYWinSize;                 // bytes of AEY window
    MUINT32 u4AAO_AEOverWinSize;              // bytes of AEOverCnt window
    MUINT32 u4AAO_HistSize;                   // bytes of each Hist
    MUINT32 u4PSO_SE_AWBWinSize;              // bytes of AWB window
    MUINT32 u4PSO_LE_AWBWinSize;              // bytes of AWB window
    MUINT32 u4PSO_SE_AEYWinSize;              // bytes of AEY window  
    MUINT32 u4PSO_LE_AEYWinSize;              // bytes of AEY window
    MUINT32 u4PSO_SE_HistSize;                // bytes of each Hist
    MUINT32 u4PSO_LE_HistSize;                // bytes of each Hist
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
	MUINT16 u4AvgY;         // for N3D sync    
    MUINT32 u4RealISO;      //!<: ISO speed
    MINT16   i2FlareOffset;
    MINT16   i2FlareGain;   // 512 is 1x
	MINT32  u4AEFinerEVIdxBase;//finer ev idx
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

typedef struct
{
    MBOOL bPSHistEn;
    MUINT16 uPSHistYHi;
    MUINT16 uPSHistYLow;
    MUINT16 uPSHistXHi;
    MUINT16 uPSHistXLow;
} PS_HIST_WIN_T;

//AE Statistic window config
typedef struct
{
    AE_HIST_WIN_T rAEHistWinCFG[6];
    AE_HIST_WIN_T rAEPixelHistWinCFG[6];
    PS_HIST_WIN_T rPSPixelHistWinCFG;
    AE_BLOCK_WINDOW_T m_AEWinBlock;
    MUINT32 u4EndBankIdx;
    MUINT32 u4UseDefaultHight;
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
    MINT32 i4LandmarkCV;
    MINT32 i4ROP;
    MINT32 i4LandMarkRip;
    MINT32 i4LandMarkRop;
} AEMeterArea_T;




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
    // for shading smooth end
    // for LCE smooth start
    MUINT32 u4FaceAEStable;
    MINT32  i4Crnt_FDY;
    MUINT32 u4MeterFDTarget;
    MUINT32 u4FDProb;
    MUINT32 u4FaceNum;
    MINT32  i4FDY_ArrayOri[MAX_AE_METER_AREAS];
    MINT32  i4FDY_Array[MAX_AE_METER_AREAS];
    AEMeterArea LandMarkFDArea;
    AEMeterArea LandMarkFDSmoothArea;
    AEMeterArea FDArea[MAX_AE_METER_AREAS];
    MBOOL   bAETouchEnable;
    MBOOL   bAEScenarioChange;
    // for LCE smooth end

    // for face LCE
    MBOOL bFaceAELCELinkEnable;
    MUINT32 u4MaxGain;

    HDR_TONE_INFO_T rHdrToneInfo;
    MBOOL bIsMaxIndexStable;
    MBOOL bIsndexSmallOffset;
    MUINT32 u4MaxIndexStableCount;
    MINT32 i4AEComp;
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

typedef enum
{
	EAESensorDevId_0   = 0x0,  //ESensorDev_None
	EAESensorDevId_1   = 0x1,  //ESensorDev_Main
	EAESensorDevId_2   = 0x2,  //ESensorDev_Sub
	EAESensorDevId_3   = 0x3,
	EAESensorDevId_4   = 0x4,  //ESensorDev_MainSecond
	EAESensorDevId_5   = 0x5,  //ESensorDev_Main3D
	EAESensorDevId_6   = 0x6,
	EAESensorDevId_7   = 0x7,
	EAESensorDevId_8   = 0x8,  //ESensorDev_SubSecond
	EAESensorDevId_9   = 0x9,
	EAESensorDevId_10   = 0xA,
	EAESensorDevId_11   = 0xB,
	EAESensorDevId_12   = 0xC,
	EAESensorDevId_13   = 0xD,
	EAESensorDevId_14   = 0xE,
	EAESensorDevId_15   = 0xF,
	EAESensorDevId_16   = 0x10, //ESensorDev_MainThird
	EAESensorDevId_17   = 0x11,
	EAESensorDevId_18   = 0x12,
	EAESensorDevId_19   = 0x13,
	EAESensorDevId_20   = 0x14,
	EAESensorDevId_Max  = 0x21
} EAeSensorDevId_T;

#endif


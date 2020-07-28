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
#ifndef _AF_PARAM_H

#define _AF_PARAM_H

#define AF_PARAM_REVISION 8134007

#define AF_WIN_NUM_SPOT  1

#define MAX_MULTI_ZONE_WIN_NUM 16*16

#define MIN_AF_HW_WIN_X 1

#define MIN_AF_HW_WIN_Y 1

#define MAX_AF_HW_WIN_X 128

#define MAX_AF_HW_WIN_Y 128

#define MAX_AF_HW_WIN MAX_AF_HW_WIN_X*MAX_AF_HW_WIN_Y

#define FD_WIN_NUM     15

#define PATH_LENGTH    100

#define AF_PSUBWIN_NUM 64

//#define AFEXTENDCOEF
typedef enum
{
    EAFSensorDevId_0   = 0x0,  //ESensorDev_None
    EAFSensorDevId_1   = 0x1,  //ESensorDev_Main
    EAFSensorDevId_2   = 0x2,  //ESensorDev_Sub
    EAFSensorDevId_3   = 0x3,
    EAFSensorDevId_4   = 0x4,  //ESensorDev_MainSecond
    EAFSensorDevId_5   = 0x5,  //ESensorDev_Main3D
    EAFSensorDevId_6   = 0x6,
    EAFSensorDevId_7   = 0x7,
    EAFSensorDevId_8   = 0x8,  //ESensorDev_SubSecond
    EAFSensorDevId_9   = 0x9,
    EAFSensorDevId_10   = 0xA,
    EAFSensorDevId_11   = 0xB,
    EAFSensorDevId_12   = 0xC,
    EAFSensorDevId_13   = 0xD,
    EAFSensorDevId_14   = 0xE,
    EAFSensorDevId_15   = 0xF,
    EAFSensorDevId_16   = 0x10, //ESensorDev_MainThird
    EAFSensorDevId_17   = 0x11,
    EAFSensorDevId_18   = 0x12,
    EAFSensorDevId_19   = 0x13,
    EAFSensorDevId_20   = 0x14,
    EAFSensorDevId_Max  = 0x21
} EAFSensorDevId_T;

typedef enum
{
    AF_MARK_NONE = 0,
    AF_MARK_NORMAL,
    AF_MARK_OK,
    AF_MARK_FAIL,

} AF_MARK_T;

// AF mode definition (depart form af_feature.h)
typedef enum
{
    LIB3A_AF_MODE_OFF = 0,           // Disable AF
    LIB3A_AF_MODE_AFS,               // AF-Single Shot Mode
    LIB3A_AF_MODE_AFC,               // AF-Continuous Mode
    LIB3A_AF_MODE_AFC_VIDEO,         // AF-Continuous Mode (Video)
    LIB3A_AF_MODE_MACRO,               // AF Macro Mode
    LIB3A_AF_MODE_INFINITY,          // Infinity Focus Mode
    LIB3A_AF_MODE_MF,                // Manual Focus Mode
    LIB3A_AF_MODE_CALIBRATION,       // AF Calibration Mode
    LIB3A_AF_MODE_FULLSCAN,           // AF Full Scan Mode
    LIB3A_AF_MODE_AFC_MONITOR,
    LIB3A_AF_MODE_ADVFULLSCAN,	     // AF Advanced FullScan Mode
    LIB3A_AF_MODE_TEMP_CALI,		 // AF Temperature Calibration


    LIB3A_AF_MODE_NUM,               // AF mode number
    LIB3A_AF_MODE_MIN = LIB3A_AF_MODE_OFF,
    LIB3A_AF_MODE_MAX = LIB3A_AF_MODE_FULLSCAN

} LIB3A_AF_MODE_T;

/* AF ROI description*/
typedef struct AREA_t
{
    MINT32 i4X;
    MINT32 i4Y;
    MINT32 i4W;
    MINT32 i4H;
    MINT32 i4Info; /* output area status. ref AF_MARK_T*/

    AREA_t()
    {
        i4X=0;
        i4Y=0;
        i4W=0;
        i4H=0;
        i4Info=AF_MARK_NONE;
    }
    AREA_t( MINT32 i4X_, MINT32 i4Y_, MINT32 i4W_, MINT32 i4H_, MINT32 i4Info_)
    {
        i4X=i4X_;
        i4Y=i4Y_;
        i4W=i4W_;
        i4H=i4H_;
        i4Info=i4Info_;
    }
    AREA_t operator =(const AREA_t &in)
    {
        i4X=in.i4X;
        i4Y=in.i4Y;
        i4W=in.i4W;
        i4H=in.i4H;
        i4Info=in.i4Info;
        return (*this);
    }
} AREA_T;

/* Intput Area interface for 3A HAL. */
typedef struct
{
    MINT32  i4Count;
    MINT32  i4Score;
    AREA_T  sRect[AF_WIN_NUM_SPOT];
    MINT32  i4Id[AF_WIN_NUM_SPOT];
    MINT32  i4Type[AF_WIN_NUM_SPOT]; // 0:GFD, 1:LFD, 2:OT
    MINT32  i4Motion[AF_WIN_NUM_SPOT][2];
    MINT32  i4Landmark[AF_WIN_NUM_SPOT][3][4]; // index 0: left eye, index 1: right eye, index 2:mouth
    MINT32  i4LandmarkCV[AF_WIN_NUM_SPOT];
    MINT32  i4LandmarkROP[AF_WIN_NUM_SPOT];
    MINT32  i4LandmarkRIP[AF_WIN_NUM_SPOT];
    MINT32  i4PortEnable;
    MINT32  i4IsTrueFace;
    MFLOAT  f4CnnResult0;
    MFLOAT  f4CnnResult1;
} AF_AREA_T;

/* Multi Zone status*/
typedef struct
{
    MINT32  i4TotalNum;
    AREA_T  sROI[MAX_MULTI_ZONE_WIN_NUM];
} AF_MULTI_ZONE_T;

/* Coordinate.*/
typedef struct AF_COORDINATE_t
{
    MINT32  i4W;
    MINT32  i4H;

    AF_COORDINATE_t()
    {
        i4W=0;
        i4H=0;
    }
    AF_COORDINATE_t( MINT32 i4W_, MINT32 i4H_)
    {
        i4W=i4W_;
        i4H=i4H_;
    }
    AF_COORDINATE_t operator =(const AF_COORDINATE_t &in)
    {
        i4W=in.i4W;
        i4H=in.i4H;
        return (*this);
    }
} AF_COORDINATE_T;

/* AF HW configure*/
typedef struct
{
    // ===============
    //    AF HAL control flow
    // ===============
    /*TG coordinate*/
    AF_COORDINATE_T sTG_SZ;
    /*HW coordinate*/
    AF_COORDINATE_T sBIN_SZ;
    // ===============
    //      ISP HW setting
    // ===============
    /*AF ROI : TG coordinate*/
    AREA_T sRoi;
    /*AF_BLK_1*/
    MINT32 AF_BLK_XNUM;
    MINT32 AF_BLK_YNUM;
    /*SGG PGN*/
    MINT32 i4SGG_GAIN;
    /*SGG GMRC 1*/
    MINT32 i4SGG_GMR1;
    MINT32 i4SGG_GMR2;
    MINT32 i4SGG_GMR3;
    MINT32 i4SGG_GMR4;
    /*SGG GMRC 2*/
    MINT32 i4SGG_GMR5;
    MINT32 i4SGG_GMR6;
    MINT32 i4SGG_GMR7;
    /*AF_CON*/
    MINT32 AF_H_GONLY;
    MINT32 AF_V_GONLY;
    MINT32 AF_V_AVG_LVL;
    MINT32 AF_BLF[4]; /*[0]AF_BLF_EN ; [1]AF_BLF_D_LVL ; [2]AF_BLF_R_LVL ; [3] AF_BLF_VFIR_MUX*/
    /*AF_TH_0*/
    MINT32 AF_TH_H[2]; /*[0]AF_H_TH_0 ; [1]AF_H_TH_1 */
    /*AF_TH_1*/
    MINT32 AF_TH_V;
    MINT32 AF_TH_G_SAT;
    /*AF_FLT_1 ~ AF_FLT_3*/
    MINT32 AF_FIL_H0[12];
    /*AF_FLT_4 ~ AF_FLT_6*/
    MINT32 AF_FIL_H1[12];
    /*AF_FLT_7 ~ AF_FLT_8*/
    MINT32 AF_FIL_V[12]; /*[0]AF_VFLT_X0 ; [1]AF_VFLT_X1 ; [2]AF_VFLT_X2 ; [3] AF_VFLT_X3*/

    MUINT32 u4ConfigNum;

    // EXT mode 3.5
    MINT32 AF_EXT_ENABLE;
    MINT32 AF_TH_R_SAT;
    MINT32 AF_TH_B_SAT;
    MINT32 AF_TH_H2[4];
    MINT32 i4SGG5_GAIN;
    MINT32 i4SGG5_GMR[7];

    //ISP 5.0
    MINT32 AF_V_FLT_MODE; // FIR or IIR
    MINT32 AF_LUT_MODE[3]; // [0]th0 ;[1]th1; [2]th2(V)
    MINT32 AF_LUT_GAIN[3]; // [0]th0 ;[1]th1; [2]th2(V)
    MINT32 AF_LUT_TH0_Dn[16];
    MINT32 AF_LUT_TH1_Dn[16];
    MINT32 AF_LUT_TH2_Dn[16];
    MINT32 AF_DS_En;
    MINT32 AF_FV_H0_ABS;
    MINT32 AF_FV_H1_ABS;
    MINT32 AF_FV_V_ABS;
    MINT32 AF_FV_MAX_ABS;
    MINT32 AF_FV_WEIGHT_ABS;
    MINT32 AF_FV_H1_En; // for low power mode
} AF_CONFIG_T;

#define ISO_MAX_NUM     (8)
typedef struct
{
    MINT32 i4ISONum;
    MINT32 i4ISO[ISO_MAX_NUM];

    MINT32 i4GMR[7][ISO_MAX_NUM];

    MINT32 i4MIN_TH[ISO_MAX_NUM];
    MINT32 i4HW_TH[ISO_MAX_NUM];

    MINT32 i4MIN_TH1[ISO_MAX_NUM];
    MINT32 i4HW_TH1[ISO_MAX_NUM];

    MINT32 i4MIN_TH2[ISO_MAX_NUM];
    MINT32 i4HW_TH2[ISO_MAX_NUM];
} CustAF_THRES_T;

typedef struct
{
    MINT32 i4ImageWidth;
    MINT32 i4ImageHeight;
    MINT32 i4SensorID;
    CustAF_THRES_T Coef;
} AF_Extend_Coef_T;

typedef struct
{
    MINT32 AF_WINX[6];
    MINT32 AF_WINY[6];
    MINT32 AF_XSIZE;
    MINT32 AF_YSIZE;
    MINT32 AF_WINXE;
    MINT32 AF_WINYE;
    MINT32 AF_SIZE_XE;
    MINT32 AF_SIZE_YE;

} AF_WIN_CONFIG_T;


/* This struct is AF statistic data */
typedef struct // DO NOT change member sequence
{
    MUINT32 u4FILV;
    MUINT32 u4FILH0;
    MUINT32 u4FILH1;
    MUINT32 u4GSum;
    MUINT32 u4FILH2;
    MUINT16 u4FILH0Max;
    MUINT16 u4FILH2Cnt;
    MUINT32 u4RSum;
    MUINT32 u4BSum;
    MUINT16 u4RSCnt;
    MUINT16 u4BSCnt;
    MUINT32 u4SCnt;

} AF_STAT_T;


/* This struct is for hybrid AF input */
typedef struct
{
    MUINT32 u4NumBlkX;
    MUINT32 u4NumBlkY;
    MUINT32 u4NumStride;
    MUINT32 u4SizeBlkX;
    MUINT32 u4SizeBlkY;

    AF_STAT_T *ptrStat;

    // EXT mode 3.5
    MUINT32 u4AfExtValid;

    MUINT32 u4ConfigNum;
    MINT32  i4AFPos;
} AF_STAT_PROFILE_T;


typedef struct
{
    MINT64 i8StatH[MAX_AF_HW_WIN-1]; // no use
    MINT64 i8StatV[MAX_AF_HW_WIN-1]; // no use
    MBOOL  bValid;
} AF_FULL_STAT_T;

typedef struct LENS_INFO_t
{
    MINT32 i4CurrentPos;        //current position
    MINT32 i4MacroPos;          //macro position
    MINT32 i4InfPos;            //Infiniti position
    MINT32 bIsMotorMoving;      //Motor Status
    MINT32 bIsMotorOpen;        //Motor Open?
    MINT32 bIsSupportSR;

    LENS_INFO_t()
    {
        i4CurrentPos   = 0;
        i4MacroPos     = 0;
        i4InfPos       = 0;
        bIsMotorMoving = 0;
        bIsMotorOpen   = 0;
        bIsSupportSR   = 0;
    }
    LENS_INFO_t( MINT32 i4CurrentPos_, MINT32 i4MacroPos_, MINT32 i4InfPos_, MINT32 bIsMotorMoving_, MINT32 bIsMotorOpen_, MINT32 bIsSupportSR_)
    {
        i4CurrentPos   = i4CurrentPos_;
        i4MacroPos     = i4MacroPos_;
        i4InfPos       = i4InfPos_;
        bIsMotorMoving = bIsMotorMoving_;
        bIsMotorOpen   = bIsMotorOpen_;
        bIsSupportSR   = bIsSupportSR_;
    }

    LENS_INFO_t operator =(const LENS_INFO_t &in)
    {
        i4CurrentPos   = in.i4CurrentPos;
        i4MacroPos     = in.i4MacroPos;
        i4InfPos       = in.i4InfPos;
        bIsMotorMoving = in.bIsMotorMoving;
        bIsMotorOpen   = in.bIsMotorOpen;
        bIsSupportSR   = in.bIsSupportSR;
        return (*this);
    }

} LENS_INFO_T;

typedef struct
{
    MINT32 i4Confidence;
    MINT32 i4CurPosDAC;
    MINT32 i4CurPosDist;
    MINT32 i4AfWinPosX;
    MINT32 i4AfWinPosY;
    MINT32 i4AfWinPosCnt;
} LASER_INFO_T;

typedef enum
{
    ROI_TYPE_NONE = 0,
    ROI_TYPE_AP,
    ROI_TYPE_OT,
    ROI_TYPE_FD,
    ROI_TYPE_CENTER,
    ROI_TYPE_DEFAULT,
} ROI_TYPE_T;

typedef struct AFPD_BLOCK_ROI_t
{
    ROI_TYPE_T    sType;
    AREA_T        sPDROI;

    AFPD_BLOCK_ROI_t():
        sType(ROI_TYPE_NONE),
        sPDROI(AREA_T())
    {}
    AFPD_BLOCK_ROI_t( ROI_TYPE_T _sType, AREA_T _sPDROI)
    {
        sType=_sType;
        sPDROI=_sPDROI;
    }
    AFPD_BLOCK_ROI_t operator =(const AFPD_BLOCK_ROI_t &in)
    {
        sType=in.sType;
        sPDROI=in.sPDROI;
        return (*this);
    }

} AFPD_BLOCK_ROI_T;

typedef struct
{
    AFPD_BLOCK_ROI_T sBlockROI;
    MUINT16          i4PDafDacIndex;
    MUINT16          i4PDafConfidence;
    MINT32           i4PDafConverge;
    MINT32           i4SatPercent;

} AFPD_BLOCK_T;

typedef struct
{
    MINT32        i4Valid;
    MUINT32       i4FrmNum;
    MINT32        i4PDPureRawfrm;
    MINT32        i4PDBlockInfoNum;
    AFPD_BLOCK_T  sPDBlockInfo[AF_PSUBWIN_NUM];

} AFPD_INFO_T;

typedef struct
{
    MINT32      i4IsEnterCam;       // 1: re-enter camera ; 0: only switch camera mode
    MINT32      i4IsVDO;
    MINT32      i4IsZSD;
    MINT32      i4IsMZ;
    MINT32      i4IsAEStable;
    MINT32      i4DeltaBV;
    MINT32      i4IsAEidxReset;
    MUINT32     u4AEFinerEVIdxBase;
    MUINT32     u4AEidxCurrentF;
    MINT64      i8GSum;
    MINT32      i4ISO;
    MINT32      i4SceneLV;
    MINT32      i4ShutterValue;
    MINT32      i4FullScanStep;
    MINT32      i4IsFlashFrm;           // PL detect
    MINT32      i4AEBlockAreaYCnt;      // PL detect
    MUINT8      *pAEBlockAreaYvalue;    // PL detect
    LENS_INFO_T  sLensInfo;
    LASER_INFO_T sLaserInfo;
    AREA_T      sEZoom;
    MUINT32     i4HybridAFMode;
    MUINT32     i4CurrP1FrmNum;   // stereo af
    MUINT16     i4DafDacIndex;    // stereo af
    MUINT16     i4DafConfidence;  // stereo af
    MINT32      i4DafConverge;    // stereo af
    AFPD_INFO_T sPDInfo;
    AF_AREA_T   sAFArea;
    AF_COORDINATE_T    sTGSz; /*TG coordinate*/
    AF_COORDINATE_T    sHWSz; /*HW coordinate*/
    AF_STAT_PROFILE_T  sStatProfile;
    MUINT32     u4CurTemperature;
    MUINT16     i2AdvFSRepeat;
    MINT32      i4IsSupportN2Frame; // support HW effective at N+2 frame

    // EXT mode 3.5
    MINT32      i4IsExtStatMode;

    // Adaptive Compensation
    MUINT64 TS_AFDone;           // (unit: us)    Time stamp of AFDone. (from Hal3A)
    MUINT64 TS_MLStart;          // (unit: us)    Time stamp of start of move lens.
    MINT32 PixelInLine;         // (unit: pixel) pixel number per line
    MINT32 PixelClk;            // (unit: Hz)    pixel readout per second
} AF_INPUT_T;

typedef struct
{
    MINT32  i4SyncMode;
    MINT32  i4Minf;
    MINT32  i4Mmacro;
    MINT32  i4Sinf;
    MINT32  i4Smacro;
    MUINT32 u4MTemp;
    MUINT32 u4STemp;
    MINT32  i4MPos;
    MINT64  i8MFV;
    MINT32  i4SPos;
    MINT64  i8SFV;

    MINT32  i4IsSAFRun;
    MINT32  i4IsSMAFTrig;
    MINT32  i4IsSCAFTrig;

    AF_AREA_T s_MAFArea;
    AF_AREA_T s_MFDInput;
    MINT32  i4SAFRunPos;
    MINT32  i4SyncStatus;
    MINT32  i4MAFCStatus;
    MINT32  i4SAFCStatus;
    MINT32  i4MovDir;
    MINT32 i4AEStable;
    MINT32 i4FVStable;
} AF_SyncInfo_T;

typedef struct
{
    unsigned short Close_Loop_AF_Min_Position;
    unsigned short Close_Loop_AF_Max_Position;
    unsigned char  Close_Loop_AF_Hall_AMP_Offset;
    unsigned char  Close_Loop_AF_Hall_AMP_Gain;
    unsigned short AF_infinite_pattern_distance;
    unsigned short AF_Macro_pattern_distance;
    unsigned char  AF_infinite_calibration_temperature;
    unsigned char  AF_macro_calibration_temperature;
    unsigned short Warm_AF_infinite_calibration;
    unsigned short Warm_AF_macro_calibration;
    unsigned char  Warm_AF_infinite_calibration_temperature;
    unsigned char  Warm_AF_macro_calibration_temperature;
    unsigned short AF_Middle_calibration;
    unsigned char  AF_Middle_calibration_temperature;
    unsigned short Warm_AF_Middle_calibration;
    unsigned char  Warm_AF_Middle_calibration_temperature;

    unsigned char  Reserved[40];
} AF_OTPInfo_T;


typedef enum
{
    // AF sync v3.0
    SYNC_STATUS_MONITORnDONE = 0,
    SYNC_STATUS_INITLENS     = 1,
    SYNC_STATUS_FINESEARCH     = 2,
    SYNC_STATUS_CONTRASTAF     = 3,
    SYNC_STATUS_LENSFOLLOW     = 4
} SYNC_STATUS_T;


typedef enum
{
    AF_ROI_SEL_NONE = 0, /* for preview start only*/
    AF_ROI_SEL_AP,
    AF_ROI_SEL_OT,
    AF_ROI_SEL_FD,
    AF_ROI_SEL_CENTER,
    AF_ROI_SEL_DEFAULT,

} AF_ROI_SEL_T;

typedef enum
{
    AF_SEARCH_DONE = 0,
    AF_SEARCH_TARGET_MOVE,
    AF_SEARCH_TARGET_FS,
    AF_SEARCH_CONTRAST,
    AF_SEARCH_READY,
} AF_SEARCH_TYPE_T;

typedef struct
{
    // af status to mgr
    MINT32       i4IsAfSearch;
    MINT32       i4IsFocused;

    // af hw setting
    MINT32       i4ROISel;
    MINT32       i4IsSelHWROI;

    MINT32       i4AFBestPos;
    MINT32       i4AFPos;
    MINT32       i4FDDetect;
    MINT64       i8AFValue;

    // af status to stereo
    MUINT8       i4IsLearning;
    MUINT8       i4IsQuerying;
    MUINT8       i4AfValid;
    MUINT16      i4AfDacIndex;
    MUINT16      i4AfConfidence;
    MUINT32      i4QueryFrmNum;

    // af pd info
    AREA_T       sPDWin;
    MUINT32      i4IsPDReliable;
    MUINT32      i4PdTrigUiConf;

    // af hw config data
    AF_CONFIG_T  sAFStatConfig;

    // af multi-zone status
    AF_MULTI_ZONE_T sROIStatus;

    // af info1
    MINT32       i4IsSceneChange;
    MINT32       i4IsSceneStable;
    MUINT32      i4IsPDTracking;
    MUINT32      i4SceneState;

    // af compensation info
    MINT32       i4ThermalLensPos; //Lens Thermal Compensate result
    MINT32       i4PostureDac;
    MINT32       i4ZECChg;         //ZEC setting to mgr

    // af info2
    AREA_T       sLastFocusedArea;
    MINT32       i4IsTargetAssitMove;
    MINT32       i4AdpAlarm;

    // af search range
    MINT32       i4SearchRangeInf;
    MINT32       i4SearchRangeMac;

    //af FV
    MINT64       i8FV;

} AF_OUTPUT_T;

#define DistTBLLength 15
#define DafMultiSourceLength 3
typedef struct
{
    MINT32  i4AFS_MODE;    //0 : singleAF, 1:smoothAF
    MINT32  i4AFC_MODE;    //0 : singleAF, 1:smoothAF
    MINT32  i4VAFC_MODE;   //0 : singleAF, 1:smoothAF

    MINT32 i4TBLL;
    MINT32 i4Dist[DistTBLLength];
    MINT32 i4Dacv[DistTBLLength];
    MINT32 i4FocusRange[DistTBLLength];
} AF_PARAM_T;

typedef struct
{
    MINT32  i4Num;
    MINT32  i4Pos[PATH_LENGTH];

} AF_STEP_T;


// AF info for ISP tuning
typedef struct
{
    MINT32 i4AFPos; // AF position

} AF_INFO_T;

// for af_flow_custom
typedef struct COMPARESET_t
{
    MINT32 Value;
    MINT32 Target;
} CompareSet_T;

typedef struct CHECK_AF_CALLBACK_INFO_t
{
    MINT32 isAfterAutoMode;
    MINT32 isSearching;        // 0: idle, 1: pd, 2: fs, 3: contrast
    CompareSet_T CompSet_PDCL; // PD confidence level
    CompareSet_T CompSet_ISO;  // ISO
    CompareSet_T CompSet_FPS;  // FPS
} CHECK_AF_CALLBACK_INFO_T;

#endif

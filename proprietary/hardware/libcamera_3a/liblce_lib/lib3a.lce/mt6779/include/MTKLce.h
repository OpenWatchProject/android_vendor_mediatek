//MTK_SWIP_PROJECT_START
#ifndef __MTK_LCE_H__
#define __MTK_LCE_H__

#define LCE_ALGO_CHIP_VERSION       (6779)
#define LCE_ALGO_MAIN_VERSION       (40)
#define LCE_ALGO_SUB_VERSION        (1)
#define LCE_ALGO_SYSTEM_VERSION     (1)

#include "MTKLceType.h"
#include "MTKLceErrCode.h"


/*****************************************************************************
    Feature Control Enum and Structure
******************************************************************************/

typedef enum
{
    MTKLCE_FEATURE_BEGIN,               // minimum of feature id
    MTKLCE_FEATURE_SET_ENV_INFO,        // feature id to setup environment information
    MTKLCE_FEATURE_SET_PROC_INFO,       // feature id to setup processing information
    MTKLCE_FEATURE_GET_RESULT,          // feature id to get result
    MTKLCE_FEATURE_GET_EXIF,            // feature id to get EXIF
    MTKLCE_FEATURE_CONFIG_SMOOTH,
    MTKLCE_FEATURE_CONFIG_FLARE,
    MTKLCE_FEATURE_CONFIG_ENHANCE_FACTOR,
    MTKLCE_FEATURE_CONFIG_MODE,
    MTKLCE_FEATURE_GET_DUALCAM_SYNC_INFO,    
    MTKLCE_FEATURE_MAX                  // maximum of feature id
} MTKLCE_FEATURE_ENUM;

typedef enum
{
    MTKLCE_TUNING_SET_Preview = 0,
    MTKLCE_TUNING_SET_Video,
    MTKLCE_TUNING_SET_Capture,
    MTKLCE_TUNING_SET_HDR_Preview,
    MTKLCE_TUNING_SET_HDR_Video,
    MTKLCE_TUNING_SET_HDR_Capture,
    MTKLCE_TUNING_SET_AutoHDR_Preview,
    MTKLCE_TUNING_SET_AutoHDR_Video,
    MTKLCE_TUNING_SET_AutoHDR_Capture,
    MTKLCE_TUNING_SET_RSV0,
    MTKLCE_TUNING_SET_RSV1,
    MTKLCE_TUNING_SET_RSV2,
    MTKLCE_TUNING_SET_MAX         // maximum of set enum
} eMTKLCE_TUNING_SET_T;

typedef enum
{
    eLCESensorDev_None         = 0x00,
    eLCESensorDev_Main         = 0x01,
    eLCESensorDev_Sub          = 0x02,
    eLCESensorDev_MainSecond   = 0x04,
    eLCESensorDev_Main3D       = 0x05,
    eLCESensorDev_Sub2         = 0x08,
    eLCESensorDev_MainThird    = 0x10,
}   eLCESensorDev_T;

typedef enum
{
    MTKLCE_FACE_STATE_FDIN,
    MTKLCE_FACE_STATE_FDOUT,
    MTKLCE_FACE_STATE_FDCONTINUE,
    MTKLCE_FACE_STATE_FDDROP,
    MTKLCE_FACE_STATE_END
} eMTKLCE_FACE_STATE_T;

typedef enum {
    LCE_FACE_PROTECT_KEEP_LIGHT_MODE,
    LCE_FACE_PROTECT_KEEP_DARK_MODE,
    LCE_FACE_PROTECT_MODE_OFF
} eMTKLCE_FACE_PROTECT_MODE;

#define LCE_PARA_TBL_NUM                                (19)
#define LCE_LV_TBL_NUM                                  (19)
#define LCE_CONTRAST_TBL_NUM                            (11)
#define LCE_CONTRAST_INDEX_UNIT                         (10)
#define LCE_LV_INDEX_UNIT                               (10)
#define LCE_CONTRAST_INDEX_MIN                          (0)
#define LCE_CONTRAST_INDEX_MAX                          (10)
#define LCE_CONTRAST_INDEX_UNIT                         (100)
#define LCE_CONTRAST_INDEX_BASE                         (700)
#define LCE_LV_INDEX_MIN                                (0)
#define LCE_LV_INDEX_MAX                                (18)
#define LCE_DIFFRANGE_INDEX_MIN                         (0)
#define LCE_DIFFRANGE_INDEX_MAX                         (11)
#define LCE_BASIC_TBL_NUM                               (4)
#define LCE_BRIGHT_STD_TBL_NUM                          (5)
#define LCE_DARK_STD_TBL_NUM                            (19)

#define LCE_Y_BIN_NUM                                   (128)
#define LCE_DIFF_RANGE_BIN_NUM                          (12)
#define LCE_Y_MULTI                                     (32)
#define LCE_HIST_SHIFT_BIT                              (5)
#define LCE_FIX_POINT_NUM                               (10)

#define LCE_MAX_LOG10                                   (3700)
#define TP_NUM                                          (8)
#define LCE_P0_MAX_VALUE                                (LCE_MAX_LOG10 - TP_NUM)

#define GMA_TBL_SIZE                                    (193)
#define GMA_TBL_NUM                                     (4)
#define GMA_CP_NUM                                      (145)

#define LCES_SIZE_MAXIMUM                               (510)
#define LCES_SIZE_MINIMUM                               (32)


// GMA NVRAM structure
typedef struct {
    MINT32 i4EncNVRAMGMA[GMA_TBL_NUM][GMA_TBL_SIZE];
} LCE_GMA_INFO_T;

typedef struct {
    MINT32 i4BaseStrLV[LCE_BASIC_TBL_NUM];
    MINT32 i4DarkStrRatio[LCE_BASIC_TBL_NUM];
    MINT32 i4BrightStrRatio[LCE_BASIC_TBL_NUM];
    MINT32 i4LVTarget[LCE_LV_TBL_NUM];
    MINT32 i4MaxLCEGain[LCE_LV_TBL_NUM];
    MINT32 i4MaxFinalTarget[LCE_LV_TBL_NUM];
}  MTK_LCE_TUNING_BASIC_T;

typedef struct {
    MINT32 i4DarkSTDTbl[LCE_CONTRAST_TBL_NUM][LCE_DARK_STD_TBL_NUM];
    MINT32 i4BrightSTDTbl[LCE_CONTRAST_TBL_NUM][LCE_BRIGHT_STD_TBL_NUM];
    MINT32 i4CenSlopeEnhance[LCE_DIFF_RANGE_BIN_NUM];
} MTK_LCE_TUNING_LUT_T;

typedef struct {
    MINT32 u4FlatAreaPercen;
    MINT32 u4FlatRatioTH;
    MBOOL bDarkSkyProtectEnable;
    MINT32 i4DarkFlatPercen;
    MINT32 i4DarkFlatY_L;
    MINT32 i4DarkFlatY_H;
    MINT32 i4FlatLV_L;
    MINT32 i4FlatLV_H;

    MINT32 i4RsvPara0;
    MINT32 i4RsvPara1;
    MINT32 i4RsvPara2;
    MINT32 i4RsvPara3;
    MINT32 i4RsvPara4;
    MINT32 i4RsvPara5;
    MINT32 i4RsvPara6;
    MINT32 i4RsvPara7;
    MINT32 i4RsvPara8;
    MINT32 i4RsvPara9;

    MINT32 i4LVBriRatio[LCE_LV_TBL_NUM];
    MINT32 i4LVBriLimit[LCE_LV_TBL_NUM];
    MINT32 i4FlatLumaLoBound[LCE_LV_TBL_NUM];
    MINT32 i4FlatLumaHiBound[LCE_LV_TBL_NUM];
    MINT32 i4LCEParaTbl0[LCE_LV_TBL_NUM];
    MINT32 i4LCEParaTbl1[LCE_LV_TBL_NUM];
    MINT32 i4LCEParaTbl2[LCE_LV_TBL_NUM];
    MINT32 i4LCEParaTbl3[LCE_LV_TBL_NUM];

}  MTK_LCE_TUNING_PARA_T;

typedef struct {
    MBOOL  bFLceEnable;
    MINT32 i4LoBoundRatio;
    MINT32 i4HiBoundRatio;
    MINT32 i4BrightTH;
    MINT32 i4DarkFaceLimit;
    MINT32 i4FrontLightTH_L;
    MINT32 i4FrontLightTH_H;
    MINT32 i4DarkCurveBlendRatio;
    MBOOL bMultiSlopeEnable;
    MINT32 i4SlopeHigh0;
    MINT32 i4SlopeHigh1;
    MINT32 i4SlopeHigh2;
    MINT32 i4SlopeHigh3;
    MINT32 i4SlopeHigh4;
    MINT32 i4SlopeLow;
    MINT32 i4DarkFaceRatio;
    MINT32 i4FDWidthCropRat;
    MINT32 i4FDHeightCropRat;
    MINT32 i4RsvFace0;
    MINT32 i4RsvFace1;
    MINT32 i4RsvFace2;
    MINT32 i4RsvFace3;
    MINT32 i4RsvFace4;
    MINT32 i4RsvFace5;
    MINT32 i4RsvFace6;
    MINT32 i4RsvFace7;
    MINT32 i4RsvFace8;
    MINT32 i4RsvFace9;
    MINT32 i4LCEFaceTbl0[LCE_LV_TBL_NUM];
    MINT32 i4LCEFaceTbl1[LCE_LV_TBL_NUM];
    MINT32 i4CenSlopeEnhance[LCE_DIFF_RANGE_BIN_NUM];
    eMTKLCE_FACE_PROTECT_MODE eFaceLCEProtectMode;
    MBOOL bFaceLCESmoothLinkEnable;
    MUINT32 u4RobustFaceSlowSpeedCnt;
    MINT32 i4RobustFaceSmoothSpeed;
    MINT32 i4RobustFaceSmoothSlowSpeed;
    MINT32 bNonRobustFaceLockLCE;
    MUINT32 u4FaceLCEStableMaxCnt;
    MINT32 i4FaceLCEInStableThd;
    MINT32 i4FaceLCEOutStableThd;
}  MTK_LCE_TUNING_FACE_T;

typedef struct {
    MBOOL bSmoothEnable;
    MINT32 i4LCEPosSpeed;
    MINT32 i4LCENegSpeed;
    MINT32 i4LCECrosSpeed;     // 0 ~ 10
    MINT32 i4LCESpeed4AEStable;
    MINT32 i4RsvSmooth0;
    MINT32 i4RsvSmooth1;
    MINT32 i4RsvSmooth2;
    MINT32 i4RsvSmooth3;
} MTK_LCE_TUNING_SMOOTH_T;

typedef struct {
    MBOOL bLCEcurrGainEnable;
    MINT32 i4LCECurrGainRatio;
    MINT32 i4RsvCustom0;
    MINT32 i4RsvCustom1;
    MINT32 i4RsvCustom2;
    MINT32 i4RsvCustom3;
    MINT32 i4RsvCustom4;
    MINT32 i4RsvCustom5;
    MINT32 i4RsvCustom6;
    MINT32 i4RsvCustom7;
    MINT32 i4RsvCustom8;
    MINT32 i4RsvCustom9;
    MINT32 i4LCECustomTbl0[LCE_LV_TBL_NUM];
    MINT32 i4LCECustomTbl1[LCE_LV_TBL_NUM];
    MINT32 i4LCECustomTbl2[LCE_LV_TBL_NUM];
    MINT32 i4LCECustomTbl3[LCE_LV_TBL_NUM];
} MTK_LCE_TUNING_CUSTOM_T;

typedef struct {
    MINT32 i4HistCCLB;
    MINT32 i4HistCCUB;
    MINT32 i4GCEratio;
    MINT32 i4TCPLB;
    MINT32 i4TCPUB;
    MINT32 i4SlopeTop;
    MINT32 i4DetailRangeRatio;
    MINT32 i4CenSlopeMin;

    MINT32 i4LowYRatio;
    MINT32 i4HighYRatio;
    MBOOL bDRCompensateEnable;
    MINT32 i4CompLowYRatio;
    MINT32 i4DFRatio_max;
    MINT32 i4DFRatio_min;
    MINT32 i4FlatRatioBound;

    MBOOL bMultiSlopeEnable;
    MINT32 i4SlopPoint0;
    MINT32 i4SlopPoint1;
    MINT32 i4SlopPoint2;
    MINT32 i4SlopPoint3;
    MINT32 i4SlopPoint4;
    MINT32 i4SlopeLow;

    MINT32 i4SlopeHigh0;
    MINT32 i4SlopeHigh1;
    MINT32 i4SlopeHigh2;
    MINT32 i4SlopeHigh3;
    MINT32 i4SlopeHigh4;

    MINT32 i4RsvStats0;
    MINT32 i4RsvStats1;
    MINT32 i4RsvStats2;
    MINT32 i4RsvStats3;
    MINT32 i4RsvStats4;
}  MTK_LCE_STATS_T;

typedef struct {
    MTK_LCE_TUNING_BASIC_T rLCEBasic;
    MTK_LCE_TUNING_LUT_T rLCELUTs;
    MTK_LCE_TUNING_PARA_T rLCEPara;
    MTK_LCE_TUNING_FACE_T rFaceLCE;
    MTK_LCE_TUNING_SMOOTH_T rLCESmooth;
    MTK_LCE_TUNING_CUSTOM_T rLCECustom;
    MTK_LCE_STATS_T rLCEStats;
} MTK_AUTO_LCE_TUNING_T;

typedef struct {
    MINT32  i4LightValue_x10;

    //for LCE smooth
    MBOOL bAETouchEnable;
    MBOOL bIsPlineMaxIndex;
    MBOOL bAEStable;
    MINT32 i4DeltaIdx;
    MINT32 u4AEFinerEVIdxBase;

    //For face lce
    MUINT32 u4FaceAEStable;
    MUINT32 u4MeterFDTarget;
    MUINT32 u4MeterFDLinkTarget;
    MUINT32 u4FaceNum;
    MINT32 i4FDY_ArrayOri;
    MINT32 i4FDY_ArraySort;
    MINT32 i4Crnt_FDY;
    MINT32 i4FDXLow;
    MINT32 i4FDXHi;
    MINT32 i4FDYLow;
    MINT32 i4FDYHi;
    MINT32 i4LMXLow;
    MINT32 i4LMXHi;
    MINT32 i4LMYLow;
    MINT32 i4LMYHi;
    MBOOL  bFaceAELCELinkEnable;
    MUINT32 u4MaxGain;

    // For face AE-LCE link smooth
    MBOOL bEnableFaceAE;
    MUINT32 u4FaceState;
    MUINT32 u4FaceRobustCnt;
    MUINT32 u4FaceRobustTrustCnt;
    MUINT32 u4FD_Lock_MaxCnt;
    MUINT32 u4FDDropTempSmooth;
    MUINT32 u4OTFaceCnt;
    MBOOL bOTFaceTimeOutLockAE;

    MINT32 i4AEMagicNum;
    MINT32 i4AERequestNum;
}LCE_AE_INFO_T;

typedef struct {
    MINT32 FLC_OFST_R;
    MINT32 FLC_OFST_B;
    MINT32 FLC_OFST_G;
    MINT32 FLC_GAIN_R;
    MINT32 FLC_GAIN_B;
    MINT32 FLC_GAIN_G;
}LCS_FLC_INFO_T;

typedef struct {
    MINT32 ACC;
    MINT32 CNV_00;
    MINT32 CNV_01;
    MINT32 CNV_02;
    MINT32 CNV_10;
    MINT32 CNV_11;
    MINT32 CNV_12;
    MINT32 CNV_20;
    MINT32 CNV_21;
    MINT32 CNV_22;
}LCS_CCM_INFO_T;

typedef struct {
    MUINT32 u4Width;
    MUINT32 u4Height;
    MUINT32 u4FrameWidth;
    MUINT16* u4LightMap;
    LCS_FLC_INFO_T rP1FLCInfo;
    LCS_FLC_INFO_T rP2FLCInfo;
    LCS_CCM_INFO_T rP1CCMInfo;
    LCS_CCM_INFO_T rP2CCMInfo;
}LCE_LCS_INFO_T;

typedef struct {
    MBOOL bIsValid;   // is data valid flag

    //LCE Result
    MINT32 LCE_TC_P[TP_NUM];
    MINT32 LCE_TC_O[TP_NUM];
    
    MINT32 LCE_CEN_MAX_SLOPE;
    MINT32 LCE_CEN_MIN_SLOPE;          
}MTK_LCE_DUALCAM_SYNC_INFO_T, *P_MTK_LCE_DUALCAM_SYNC_INFO_T;

#ifndef MTKGMADUALCAMSYNCINFO
#define MTKGMADUALCAMSYNCINFO
typedef struct {
    MBOOL bIsValid;   // is data valid flag

    //GGMD1 Result
    MBOOL bIsFaceGMA;
    MINT32 i4EncGMA[GMA_TBL_SIZE];
}MTK_GMA_DUALCAM_SYNC_INFO_T, *P_MTK_GMA_DUALCAM_SYNC_INFO_T;
#endif //MTKGMADUALCAMSYNCINFO

typedef struct
{
    MTK_AUTO_LCE_TUNING_T rAutoLCEParam;
    LCE_GMA_INFO_T rNVRAMGmaParam;
} MTK_LCE_ENV_INFO_STRUCT, *P_MTK_LCE_ENV_INFO_STRUCT;

typedef struct
{
    MINT32 i4ChipVersion;
    MINT32 i4MainVersion;
    MINT32 i4SubVersion;
    MINT32 i4SystemVersion;
    MINT32 i4PatchVersion;

    MINT32 i4LV;
    MINT32 i4LVIdx_L;
    MINT32 i4LVIdx_H;
    MINT32 i4CurrDR;
    MINT32 i4DRIdx_L;
    MINT32 i4DRIdx_H;
    MINT32 i4DiffRangeIdx_L;
    MINT32 i4DiffRangeIdx_H;

    MINT32 i4LumaTarget;
    MINT32 i4FlatTarget;
    MINT32 i4LumaProb;
    MINT32 i4FlatProb;
    MINT32 i4LumaFlatProb;
    MINT32 i4LumaFlatTarget;
    MINT32 i4LVTarget;
    MINT32 i4BrightAvg;
    MINT32 i4LVProb;
    MINT32 i4DStrengthLevel;
    MINT32 i4BStrengthLevel;
    MINT32 i4FinalTarget;
    MINT32 i4FinalDStrength;
    MINT32 i4FinalBStrength;

    MINT32 i4P0;
    MINT32 i4P1;
    MINT32 i4P50;
    MINT32 i4P250;
    MINT32 i4P500;
    MINT32 i4P750;
    MINT32 i4P950;
    MINT32 i4P999;
    MINT32 i4O0;
    MINT32 i4O1;
    MINT32 i4O50;
    MINT32 i4O250;
    MINT32 i4O500;
    MINT32 i4O750;
    MINT32 i4O950;
    MINT32 i4O999;
    MINT32 i4CenMaxSlope;
    MINT32 i4CenMinSlope;

    MINT32 i4LCE_FD_Enable;
    MINT32 i4FaceNum;
    MINT32 i4FDXLow;
    MINT32 i4FDXHi;
    MINT32 i4FDYLow;
    MINT32 i4FDYHi;
    MINT32 i4LCSXLow;
    MINT32 i4LCSXHi;
    MINT32 i4LCSYLow;
    MINT32 i4LCSYHi;
    MINT32 i4FaceLoBound;
    MINT32 i4FaceHiBound;
    MINT32 i4ctrlPoint_f0;
    MINT32 i4ctrlPoint_f1;
    MINT32 i4FDY;
    MINT32 i4MeterFDTarget;
    MINT32 i4MeterFDLinkTarget;
    MINT32 i4AEGain;
    MINT32 i4FaceAELCELinkEnable;
    MINT32 i4MaxAEGain;
    MBOOL bIsFrontLight;
    MBOOL bAETouchEnable;
    MBOOL bFaceAEEnable;
    MINT32 i4OriP0;
    MINT32 i4OriP1;
    MINT32 i4OriP50;
    MINT32 i4OriP250;
    MINT32 i4OriP500;
    MINT32 i4OriP750;
    MINT32 i4OriP950;
    MINT32 i4OriP999;
    MINT32 i4OriO0;
    MINT32 i4OriO1;
    MINT32 i4OriO50;
    MINT32 i4OriO250;
    MINT32 i4OriO500;
    MINT32 i4OriO750;
    MINT32 i4OriO950;
    MINT32 i4OriO999;
    MINT32 i4FaceP0;
    MINT32 i4FaceP1;
    MINT32 i4FaceP50;
    MINT32 i4FaceP250;
    MINT32 i4FaceP500;
    MINT32 i4FaceP750;
    MINT32 i4FaceP950;
    MINT32 i4FaceP999;
    MINT32 i4FaceO0;
    MINT32 i4FaceO1;
    MINT32 i4FaceO50;
    MINT32 i4FaceO250;
    MINT32 i4FaceO500;
    MINT32 i4FaceO750;
    MINT32 i4FaceO950;
    MINT32 i4FaceO999;
    MBOOL bFaceSmoothLinkEnable;
    MUINT32 u4FaceState;
    MUINT32 u4FaceRobustCnt;
    MUINT32 u4FaceRobustTrustCnt;
    MUINT32 u4NonRobustFaceCnt;
    MBOOL bFaceLCEStable;
    MUINT32 u4FaceStableCnt;
    MUINT32 u4FaceInStableThd;
    MUINT32 u4FaceOutStableThd;
    MUINT32 u4NeedFaceProtection;
    MUINT32 u4NeedFaceGain;
    MUINT32 u4FaceProtectMode;
    MINT32 i4SmoothEnable;
    MINT32 i4LCEPosSpeed;
    MINT32 i4LCENegSpeed;
    MINT32 i4LCECrosSpeed;
    MINT32 i4LCESpeed4AEStable;

    MINT32 i4DStrengthRatio;
    MINT32 i4BStrengthRatio;
    MINT32 i4CurrMaxLceGain;
    MINT32 i4MaxFinalTarget;
    MINT32 i4LumaMean;
    MINT32 i4FlatMean;
    MINT32 i4DSTDIndex;
    MBOOL  bDarkSkyProtectEnable;
    MINT32 i4DarkFlatY;
    MINT32 i4DarkFlatRatio;
    MINT32 i4FlatLVRatio;
    MBOOL  bBypassLCE;
    MBOOL  bIncorrectLCSO;
    MINT32 i4GCEP0;
    MINT32 i4GCEP1;
    MINT32 i4GCEP50;
    MINT32 i4GCEP250;
    MINT32 i4GCEP500;
    MINT32 i4GCEP750;
    MINT32 i4GCEP950;
    MINT32 i4GCEP999;
    MINT32 i4GCEO0;
    MINT32 i4GCEO1;
    MINT32 i4GCEO50;
    MINT32 i4GCEO250;
    MINT32 i4GCEO500;
    MINT32 i4GCEO750;
    MINT32 i4GCEO950;
    MINT32 i4GCEO999;
    MINT32 i4AEMagicNum;
} MTK_LCE_EXIF_INFO_STRUCT, *P_MTK_LCE_EXIF_INFO_STRUCT;

typedef struct
{
    LCE_AE_INFO_T rLCEAEInfo;   // Get current AE related information
    LCE_LCS_INFO_T rLCELCSInfo; // Get current LCS information

    MBOOL bIsDualcamSlave;  // Dualcam Master: False, Slave: True
    MTK_LCE_DUALCAM_SYNC_INFO_T rLCEDualcamSyncInfo;
    MTK_GMA_DUALCAM_SYNC_INFO_T rGMADualcamSyncInfo;		
} MTK_LCE_PROC_INFO_STRUCT, *P_MTK_LCE_PROC_INFO_STRUCT;

typedef struct
{
    MINT32 LCE_TC_P[TP_NUM];
    MINT32 LCE_TC_O[TP_NUM];

    MINT32 LCE_CEN_MAX_SLOPE;
    MINT32 LCE_CEN_MIN_SLOPE;
    MINT32 LCE_CEN_SLOPE_DEP_EN;
    MINT32 LCE_MAX_SLOPE_N;
    MINT32 LCE_MIN_SLOPE_N;
    MINT32 LCE_CORING_TH;

    MINT32 LCE_TCHL_EN;
    MINT32 LCE_TCHL_MTH;
    MINT32 LCE_TCHL_MGAIN;
    MINT32 LCE_TCHL_DTH1;
    MINT32 LCE_TCHL_DTH2;
    MINT32 LCE_TCHL_BW1;
    MINT32 LCE_TCHL_BW2;

    MINT32 LCE_BIL_TH1;
    MINT32 LCE_BIL_TH2;
    MINT32 LCE_BIL_TH3;
    MINT32 LCE_BIL_TH4;

    MINT32 GCE_TC_P[TP_NUM];
    MINT32 GCE_TC_O[TP_NUM];

    MINT32 LCE_DRINDEX_L;
    MINT32 LCE_DRINDEX_H;
    MINT32 LCE_CURRENT_MAX_GAIN;
    MINT32 LCE_MAX_GAIN_RATIO;
} MTK_LCE_RESULT_INFO_STRUCT, *P_MTK_LCE_RESULT_INFO_STRUCT;


class MTKLce
{
public:
    static MTKLce* createInstance(eLCESensorDev_T const eSensorDev);
    virtual MVOID   destroyInstance(MTKLce* obj) = 0;

    virtual ~MTKLce(){}

    // Process Control
    virtual MRESULT LceInit(MVOID *InitInData, MVOID *InitOutData) = 0;
    virtual MRESULT LceMain(void) = 0;
    virtual MRESULT LceExit(void) = 0;
    virtual MRESULT LceReset(void) = 0;                 // RESET for each image

    // Feature Control
    virtual MRESULT LceFeatureCtrl(MUINT32 FeatureID, MVOID* pParaIn, MVOID* pParaOut) = 0;
private:

};



#endif  //__MTK_Lce_H__
//MTK_SWIP_PROJECT_END

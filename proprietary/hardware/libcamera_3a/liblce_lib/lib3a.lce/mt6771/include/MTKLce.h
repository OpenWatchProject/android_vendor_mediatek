//MTK_SWIP_PROJECT_START
#ifndef __MTK_LCE_H__
#define __MTK_LCE_H__

#define LCE_ALGO_CHIP_VERSION       (6771)
#define LCE_ALGO_MAIN_VERSION       (30)
#define LCE_ALGO_SUB_VERSION        (4)
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
    eLCESensorDev_MainThird    = 0x10
}   eLCESensorDev_T;


#define LCE_PARA_TBL_NUM                                (19)
#define LCE_LV_TBL_NUM                                  (19)
#define LCE_CONTRAST_TBL_NUM                            (11)
#define LCE_CONTRAST_INDEX_UNIT                         (10)
#define LCE_LV_INDEX_UNIT                               (10)
#define LCE_CONTRAST_INDEX_MIN                          (0)
#define LCE_CONTRAST_INDEX_MAX                          (10)
#define LCE_LV_INDEX_MIN                                (0)         // 0 LV
#define LCE_LV_INDEX_MAX                                (18)        // LV 18
#define LCE_PRECISION_UNIT                              (10)        //2^10 = 1024 ~ 1000
#define LCE_CENTER_RADIUS                               (5)
#define LCE_DETAIL_INDEX_UNIT                           (10)

#define LCE_DETAIL_INDEX_MIN                            (0)
#define LCE_DETAIL_INDEX_MAX                            (10)
#define LCE_DELTA_TBL_UNIT                              (32)
#define LCE_DIFFRANGE_INDEX_MIN                         (0)
#define LCE_DIFFRANGE_INDEX_MAX                         (11)

#define LCE_Y_BIN_NUM                                   (128)
#define LCE_DIFF_RANGE_BIN_NUM                          (12)
#define LCE_Y_MULTI                                     (32)

#define LCE_MAX_LOG10                                   (3700)
#define LCE_HLR_LOG4X                                   (617)

#define DCE_LV_NUM                                      (19)
#define DCE_PARA_NUM                                    (11)
#define TP_NUM                                          (8)
#define DCE_Y_MULTI                                     (32)
#define GMA_CP_NUM                                      (145)
#define DCE_HIST_BITSHIT								(5)
#define GMA_TBL_SIZE                                    (193)
#define GMA_TBL_NUM                                     (4)


// GMA NVRAM structure
typedef struct
{
    MINT32 i4EncNVRAMGMA[GMA_TBL_NUM][GMA_TBL_SIZE];
} LCE_GMA_INFO_T;

typedef struct {
    MINT32 i4LCETbl[LCE_CONTRAST_TBL_NUM][LCE_LV_TBL_NUM];
    MINT32 i4CenSlopeEnhance[LCE_DIFF_RANGE_BIN_NUM];
} MTK_LCE_TUNING_LUT_T;

typedef struct {
    MINT32 i4LCEPosSpeed;
    MINT32 i4LCENegSpeed;
    MINT32 i4LCESpeed4AEStable;     // 0 ~ 10
} MTK_LCE_TUNING_SMOOTH_T;

typedef struct {
    MINT32 i4Enable;
} MTK_LCE_TUNING_FLARE_T;

typedef struct {
    MINT32 i4HistCCLB;
    MINT32 i4HistCCUB;
    MINT32 i4LCESeg;
    MINT32 i4LCEContrastRatio;
    MINT32 i4LCEHistYThr;
    MINT32 i4SlopeLow;
    MINT32 i4SlopeHigh;
    MINT32 i4SlopeTop;
    MINT32 i4DetailRangeRatio;
    MINT32 i4CenSlopeMin;
    MINT32 i4vBlacKeep;
    MINT32 i4vFixedPt1;
    MINT32 i4vFixedPt2;
    MINT32 i4vFixedPt3;
    MINT32 i4vFixedPt4;
    MINT32 i4TCPLB;
    MINT32 i4TCPUB;

    MINT32 i4Stats0;
    MINT32 i4Stats1;
    MINT32 i4Stats2;
    MINT32 i4Stats3;
    MINT32 i4Stats4;

}  MTK_LCE_STATS_T;

typedef struct {
    MINT32 i4LCEPara0[LCE_PARA_TBL_NUM];
    MINT32 i4LCEPara1[LCE_PARA_TBL_NUM];
    MINT32 i4LCEPara2[LCE_PARA_TBL_NUM];
    MINT32 i4LCEPara3[LCE_PARA_TBL_NUM];
    MINT32 i4LCEPara4[LCE_PARA_TBL_NUM];
    MINT32 i4LCEPara5[LCE_PARA_TBL_NUM];
    MINT32 i4LCEPara6[LCE_PARA_TBL_NUM];
    MINT32 i4LCEPara7[LCE_PARA_TBL_NUM];
    MINT32 i4LCEPara8[LCE_PARA_TBL_NUM];
    MINT32 i4LCEPara9[LCE_PARA_TBL_NUM];
    MINT32 i4LCEPara10[LCE_PARA_TBL_NUM];
}  MTK_LCE_TUNING_PARA_T;

typedef struct {
    MBOOL  bFLceEnable;
    MBOOL  bKeepBrightEnable;
    MINT32 i4KeepBrightSlope;
    MINT32 i4LoBoundRatio;
    MINT32 i4HiBoundRatio;
    MINT32 i4BrightTH;
    MINT32 i4MaxLCEGain_L;
    MINT32 i4MaxLCEGain_H;
    MINT32 i4FaceLV_L;
    MINT32 i4FaceLV_H;
    MINT32 i4FDProb_ratio;
    MINT32 i4FacePara0;
    MINT32 i4FacePara1;
    MINT32 i4FacePara2;
    MINT32 i4FacePara3;
    MINT32 i4FacePara4;
    MINT32 i4FacePara5;
    MINT32 i4FacePara6;
    MINT32 i4FacePara7;
    MINT32 i4FacePara8;
    MINT32 i4FacePara9;
    MINT32 i4FacePara10;

    MINT32 i4LCEFaceTbl0[LCE_LV_TBL_NUM];
    MINT32 i4LCEFaceTbl1[LCE_LV_TBL_NUM];
    MINT32 i4CenSlopeEnhance[LCE_DIFF_RANGE_BIN_NUM];

}  MTK_LCE_TUNING_FACE_T;

typedef struct {
    MTK_LCE_STATS_T rLCEStats;
    MTK_LCE_TUNING_LUT_T rLCELUTs;
    MTK_LCE_TUNING_PARA_T rLCEPara;
    MTK_LCE_TUNING_FACE_T rFaceLCE;
    MTK_LCE_TUNING_SMOOTH_T rLCESmooth;
    MTK_LCE_TUNING_FLARE_T rLCEFlare;
} MTK_AUTO_LCE_TUNING_T;

typedef struct {
    MUINT32 u4AETarget;
    MUINT32 u4AECurrentTarget;
    MUINT32 u4Eposuretime;   //!<: Exposure time in ms
    MUINT32 u4AfeGain;           //!<: raw gain
    MUINT32 u4IspGain;           //!<: sensor gain
    MUINT32 u4RealISOValue;
    MINT32  i4LightValue_x10;
    MUINT32 u4AECondition;
    MINT16  i2FlareOffset;
    MINT32  i4GammaIdx;   // next gamma idx
    MINT32  i4LESE_Ratio;    // LE/SE ratio
    MUINT32 u4SWHDR_SE;      //for sw HDR SE ,  -x EV , compare with converge AE
    MUINT32 u4MaxISO;
    MUINT32 u4AEStableCnt;
    MUINT32 u4OrgExposuretime;   //!<: Exposure time in ms
    MUINT32 u4OrgRealISOValue;
    MUINT32 u4Histogrm[LCE_Y_BIN_NUM];
    MBOOL bGammaEnable;

    MINT32 i4AEStable;
    MINT32 i4EVRatio;
    MINT32 i4DeltaIdx;
    MINT32 u4AEFinerEVIdxBase;

    //For touch lce
    MBOOL bAETouchEnable;

    //For face lce
    MUINT32 u4FaceAEStable;
    MUINT32 u4MeterFDTarget;
    MUINT32 u4FDProb;
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

    //for LCE lock
    MBOOL bIsMaxIndexStable;
    MUINT32 u4MaxIndexStableCount;
}LCE_AE_INFO_T;

typedef struct {
    MINT32 GAIN_R;
    MINT32 GAIN_GR;
    MINT32 GAIN_B;
    MINT32 GAIN_GB;
    MINT32 OFST_R;
    MINT32 OFST_GR;
    MINT32 OFST_B;
    MINT32 OFST_GB;
    MINT32 SATU_R;
    MINT32 SATU_GR;
    MINT32 SATU_B;
    MINT32 SATU_GB;
}LCS_AWB_INFO_T;

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
    LCS_AWB_INFO_T rP1AWBInfo;
    LCS_AWB_INFO_T rP2AWBInfo;
    LCS_CCM_INFO_T rP1CCMInfo;
    LCS_CCM_INFO_T rP2CCMInfo;
}LCE_LCS_INFO_T;

typedef struct
{
    MINT32 i4DCETbl1[DCE_PARA_NUM][DCE_LV_NUM];
    MINT32 i4DCETbl2[DCE_PARA_NUM][DCE_LV_NUM];
} MTK_DCE_TUNING_LUT_T;

typedef struct
{
    MINT32 i4SmoothDCESpeed;
    MTK_DCE_TUNING_LUT_T rDCELuts;
} MTK_AUTO_DCE_TUNING_T;

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
    MTK_AUTO_DCE_TUNING_T rAutoDCEParam;
    LCE_GMA_INFO_T rNVRAMGmaParam;
} MTK_LCE_ENV_INFO_STRUCT, *P_MTK_LCE_ENV_INFO_STRUCT;

typedef struct
{
    MINT32 i4ChipVersion;
    MINT32 i4MainVersion;
    MINT32 i4SubVersion;
    MINT32 i4SystemVersion;

    MINT32 i4LV;
    MINT32 i4LVIdx_L;
    MINT32 i4LVIdx_H;
    MINT32 i4DiffRangeIdx_L;
    MINT32 i4DiffRangeIdx_H;

    MINT32 i4LCEScene;
    MINT32 i4LVTarget;
    MINT32 i4LumaTarget;
    MINT32 i4BrightAvg;
    MINT32 i4DarkLumaProb;
    MINT32 i4NewLumaTarget;
    MINT32 i4LumaProb;
    MINT32 i4FlatTarget;
    MINT32 i4FlatProb;
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
    MINT32 i4FDProb;
    MINT32 i4AEGain;
    MINT32 i4KeepBrightEnable;
    MINT32 i4currBrightSlope;
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
    MINT32 i4SmoothEnable;
    MINT32 i4SmoothSpeed;
    MINT32 i4SmoothWaitAE;
    MINT32 i4FlareEnable;
    MINT32 i4FlareOffset;

    MINT32 i4HalfLVIdx_L;
    MINT32 i4HalfLVIdx_H;
    MINT32 i4CurrDR;
    MINT32 i4DRIdx_L;
    MINT32 i4DRIdx_H;
    MBOOL  bFaceDCEFlag;
    MINT32 i4DarkHEWt;
    MINT32 i4BrightHEWt;
    MINT32 i4SkyThr;
    MINT32 i4DceSkyLimitThr;
    MINT32 i4SkyOn;
    MINT32 i4SkyOff;
    MINT32 i4DCESlopeMax;
    MINT32 i4DCESmoothSpeed;
    MBOOL  bIncorrectLCSO;
    MINT32 i4DCEP0;
    MINT32 i4DCEP1;
    MINT32 i4DCEP50;
    MINT32 i4DCEP250;
    MINT32 i4DCEP500;
    MINT32 i4DCEP750;
    MINT32 i4DCEP950;
    MINT32 i4DCEP999;
    MINT32 i4DCEO0;
    MINT32 i4DCEO1;
    MINT32 i4DCEO50;
    MINT32 i4DCEO250;
    MINT32 i4DCEO500;
    MINT32 i4DCEO750;
    MINT32 i4DCEO950;
    MINT32 i4DCEO999;

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
    MINT32 LCE_A_TC_P0;
    MINT32 LCE_A_TC_P1;
    MINT32 LCE_A_TC_P50;
    MINT32 LCE_A_TC_P250;
    MINT32 LCE_A_TC_P500;
    MINT32 LCE_A_TC_P750;
    MINT32 LCE_A_TC_P950;
    MINT32 LCE_A_TC_P999;
    MINT32 LCE_A_TC_O0;
    MINT32 LCE_A_TC_O1;
    MINT32 LCE_A_TC_O50;
    MINT32 LCE_A_TC_O250;
    MINT32 LCE_A_TC_O500;
    MINT32 LCE_A_TC_O750;
    MINT32 LCE_A_TC_O950;
    MINT32 LCE_A_TC_O999;

    MINT32 LCE_A_CEN_MAX_SLOPE;
    MINT32 LCE_A_CEN_MIN_SLOPE;
    MINT32 LCE_A_CEN_SLOPE_DEP_EN;
    MINT32 LCE_A_MAX_SLOPE_N;
    MINT32 LCE_A_MIN_SLOPE_N;

    MINT32 LCE_TCHL_EN;
    MINT32 LCE_TCHL_MTH;
    MINT32 LCE_TCHL_MGIAN;
    MINT32 LCE_TCHL_DTH1;
    MINT32 LCE_TCHL_DTH2;
    MINT32 LCE_TCHL_BW1;
    MINT32 LCE_TCHL_BW2;

    MINT32 LCE_BIL_TH1;
    MINT32 LCE_BIL_TH2;
    MINT32 LCE_BIL_TH3;
    MINT32 LCE_BIL_TH4;

    MINT32 DCE_TC_P0;
    MINT32 DCE_TC_P1;
    MINT32 DCE_TC_P50;
    MINT32 DCE_TC_P250;
    MINT32 DCE_TC_P500;
    MINT32 DCE_TC_P750;
    MINT32 DCE_TC_P950;
    MINT32 DCE_TC_P999;
    MINT32 DCE_TC_O0;
    MINT32 DCE_TC_O1;
    MINT32 DCE_TC_O50;
    MINT32 DCE_TC_O250;
    MINT32 DCE_TC_O500;
    MINT32 DCE_TC_O750;
    MINT32 DCE_TC_O950;
    MINT32 DCE_TC_O999;

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

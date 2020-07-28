//MTK_SWIP_PROJECT_START
#ifndef __MTK_LCE_H__
#define __MTK_LCE_H__

#define LCE_ALGO_CHIP_VERSION       (6763)
#define LCE_ALGO_MAIN_VERSION       (30)
#define LCE_ALGO_SUB_VERSION        (2)
#define LCE_ALGO_SYSTEM_VERSION     (1)

#include "MTKLceType.h"
#include "MTKLceErrCode.h"


/*****************************************************************************
    Feature Control Enum and Structure
******************************************************************************/

typedef enum
{
	MTKLCE_FEATURE_BEGIN,				// minimum of feature id
	MTKLCE_FEATURE_SET_ENV_INFO,		// feature id to setup environment information
    MTKLCE_FEATURE_SET_PROC_INFO,		// feature id to setup processing information
    MTKLCE_FEATURE_GET_RESULT,			// feature id to get result
    MTKLCE_FEATURE_GET_EXIF,				// feature id to get EXIF
	MTKLCE_FEATURE_CONFIG_SMOOTH,
	MTKLCE_FEATURE_CONFIG_FLARE,
	MTKLCE_FEATURE_CONFIG_ENHANCE_FACTOR,
	MTKLCE_FEATURE_CONFIG_MODE,
    MTKLCE_FEATURE_MAX					// maximum of feature id
}	MTKLCE_FEATURE_ENUM;

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
    eLCESensorDev_Sub2         = 0x08
}   eLCESensorDev_T;


#define ISP_NVRAM_LCE_LV_TBL_NUM                        (19)
#define ISP_NVRAM_LCE_CONTRAST_TBL_NUM                  (11)
#define LCE_CONTRAST_INDEX_UNIT                         (10)
#define LCE_LV_INDEX_UNIT                               (10)
#define LCE_CONTRAST_INDEX_MIN                          (0)
#define LCE_CONTRAST_INDEX_MAX                          (10)
#define LCE_LV_INDEX_MIN                                (0)         // 0 LV
#define LCE_LV_INDEX_MAX                                (18)        // LV 18
#define LCE_PRECISION_UNIT                              (10)        //2^10 = 1024 ~ 1000
#define LCE_CENTER_RADIUS                               (5)
#define ISP_NVRAM_LCE_DETAIL_TBL_NUM                    (10)
#define LCE_DETAIL_INDEX_UNIT                           (10)
#define ISP_NVRAM_LCE_DETAIL_TBL_NUM                    (11)
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


typedef struct {
    MINT32 i4vTbl1[ISP_NVRAM_LCE_CONTRAST_TBL_NUM][ISP_NVRAM_LCE_LV_TBL_NUM];
    MINT32 i4vTbl2[ISP_NVRAM_LCE_CONTRAST_TBL_NUM][ISP_NVRAM_LCE_LV_TBL_NUM];
    MINT32 i4vTbl3[ISP_NVRAM_LCE_CONTRAST_TBL_NUM][ISP_NVRAM_LCE_LV_TBL_NUM];
    MINT32 i4vTbl4[ISP_NVRAM_LCE_CONTRAST_TBL_NUM][ISP_NVRAM_LCE_LV_TBL_NUM];
    MINT32 i4DetailWtTbl[ISP_NVRAM_LCE_DETAIL_TBL_NUM];
    MINT32 i4CenSlopeEnhance[LCE_DIFF_RANGE_BIN_NUM];
} MTK_LCE_TUNING_LUT_T;

typedef struct {
    MINT32 i4Enable;
    MINT32 i4WaitAEStable;
    MINT32 i4Speed;     // 0 ~ 10
} MTK_LCE_TUNING_SMOOTH_T;

typedef struct {
    MINT32 i4Enable;
} MTK_LCE_TUNING_FLARE_T;



typedef enum {
    eLCE_NORMAL_MODE = 0,
    eLCE_HDR_MODE,
    eLCE_MODE_NUM
} LCE_MODE_T;

typedef enum {
    eLceProfile_Preview = 0,
    eLceProfile_Video,
    eLceProfile_Capture,
    eLceProfile_iHDR_Preview,
    eLceProfile_zHDR_Preview,
    eLceProfile_mHDR_Preview,
    eLceProfile_iHDR_Video,
    eLceProfile_zHDR_Video,
    eLceProfile_mHDR_Video,
    eLceProfile_iHDR_Preview_VSS,
    eLceProfile_zHDR_Preview_VSS,
    eLceProfile_mHDR_Preview_VSS,
    eLceProfile_iHDR_Video_VSS,
    eLceProfile_zHDR_Video_VSS,
    eLceProfile_mHDR_Video_VSS,
    eLceProfile_zHDR_Capture,
    eLceProfile_mHDR_Capture,
    eLceProfile_Auto_iHDR_Preview,
    eLceProfile_Auto_zHDR_Preview,
    eLceProfile_Auto_mHDR_Preview,
    eLceProfile_Auto_iHDR_Video,
    eLceProfile_Auto_zHDR_Video,
    eLceProfile_Auto_mHDR_Video,
    eLceProfile_Auto_iHDR_Preview_VSS,
    eLceProfile_Auto_zHDR_Preview_VSS,
    eLceProfile_Auto_mHDR_Preview_VSS,
    eLceProfile_Auto_iHDR_Video_VSS,
    eLceProfile_Auto_zHDR_Video_VSS,
    eLceProfile_Auto_mHDR_Video_VSS,
    eLceProfile_Auto_zHDR_Capture,
    eLceProfile_Auto_mHDR_Capture,
    eLceProfile_MFNR_Before_Blend,
    eLceProfile_MFNR_Single,
    eLceProfile_MFNR_MFB,
    eLceProfile_MFNR_After_Blend,
    eLceProfile_zHDR_Capture_MFNR_Before_Blend,
    eLceProfile_zHDR_Capture_MFNR_Single,
    eLceProfile_zHDR_Capture_MFNR_MFB,
    eLceProfile_zHDR_Capture_MFNR_After_Blend,
    eLceProfile_EIS_Preview,
    eLceProfile_EIS_Video,
    eLceProfile_EIS_iHDR_Preview,
    eLceProfile_EIS_zHDR_Preview,
    eLceProfile_EIS_mHDR_Preview,
    eLceProfile_EIS_iHDR_Video,
    eLceProfile_EIS_zHDR_Video,
    eLceProfile_EIS_mHDR_Video,
    eLceProfile_EIS_Auto_iHDR_Preview,
    eLceProfile_EIS_Auto_zHDR_Preview,
    eLceProfile_EIS_Auto_mHDR_Preview,
    eLceProfile_EIS_Auto_iHDR_Video,
    eLceProfile_EIS_Auto_zHDR_Video,
    eLceProfile_EIS_Auto_mHDR_Video,
    eLceProfile_Capture_MultiPass_HWNR,
    eLceProfile_YUV_Reprocess,
    eLceProfile_Flash_Capture,
    eLceProfile_NUM
} eLCE_PROFILE_T;

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

    MINT32 i4Reserved0;
    MINT32 i4Reserved1;
    MINT32 i4Reserved2;
    MINT32 i4Reserved3;
    MINT32 i4Reserved4;

    MTK_LCE_TUNING_LUT_T rLCELUTs;
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
    eMTKLCE_TUNING_SET_T eLCECamMode;
    MINT32 i4AutoHDREnable;
    MTK_AUTO_LCE_TUNING_T rAutoLCEParam[2];
} MTK_LCE_ENV_INFO_STRUCT, *P_MTK_LCE_ENV_INFO_STRUCT;

typedef struct
{
    MINT32 i4LceProfile;
    MINT32 i4LCECamMode;
    MINT32 i4AutoHDREnable;
    MINT32 i4ChipVersion;
    MINT32 i4MainVersion;
    MINT32 i4SubVersion;
    MINT32 i4SystemVersion;

    MINT32 i4LV;
    MINT32 i4ContrastY10;
    MINT32 i4EVRatio;
    MINT32 i4EVContrastY10;
    MINT32 i4SegDiv;
    MINT32 i4ContrastIdx_L;
    MINT32 i4ContrastIdx_H;
    MINT32 i4LVIdx_L;
    MINT32 i4LVIdx_H;
    MINT32 i4DetailRatio1;
    MINT32 i4DetailRatio50;
    MINT32 i4DetailRatio500;
    MINT32 i4DetailRatio950;
    MINT32 i4DetailRatio999;
    MINT32 i4IntpDiffRange128;
    MINT32 i4DiffRangeIdx_L;
    MINT32 i4DiffRangeIdx_H;

    MINT32 i4P1;
    MINT32 i4P50;
    MINT32 i4P500;
    MINT32 i4P950;
    MINT32 i4P999;
    MINT32 i4O1;
    MINT32 i4O50;
    MINT32 i4O500;
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
    MINT32 i4OriP1;
    MINT32 i4OriP50;
    MINT32 i4OriP500;
    MINT32 i4OriP950;
    MINT32 i4OriP999;
    MINT32 i4OriO1;
    MINT32 i4OriO50;
    MINT32 i4OriO500;
    MINT32 i4OriO950;
    MINT32 i4OriO999;
    MINT32 i4FaceP1;
    MINT32 i4FaceP50;
    MINT32 i4FaceP500;
    MINT32 i4FaceP950;
    MINT32 i4FaceP999;
    MINT32 i4FaceO1;
    MINT32 i4FaceO50;
    MINT32 i4FaceO500;
    MINT32 i4FaceO950;
    MINT32 i4FaceO999;

    MINT32 i4SmoothEnable;
    MINT32 i4SmoothSpeed;
    MINT32 i4SmoothWaitAE;
    MINT32 i4FlareEnable;
    MINT32 i4FlareOffset;

} MTK_LCE_EXIF_INFO_STRUCT, *P_MTK_LCE_EXIF_INFO_STRUCT;

typedef struct
{
    eLCE_PROFILE_T eLceProfile;
	LCE_AE_INFO_T rLCEAEInfo;   // Get current AE related information
	LCE_LCS_INFO_T rLCELCSInfo; // Get current LCS information
} MTK_LCE_PROC_INFO_STRUCT, *P_MTK_LCE_PROC_INFO_STRUCT;

typedef struct
{
    MINT32 LCE_A_TC_P1;
    MINT32 LCE_A_TC_P50;
    MINT32 LCE_A_TC_P500;
    MINT32 LCE_A_TC_P950;
    MINT32 LCE_A_TC_P999;
    MINT32 LCE_A_TC_O1;
    MINT32 LCE_A_TC_O50;
    MINT32 LCE_A_TC_O500;
    MINT32 LCE_A_TC_O950;
    MINT32 LCE_A_TC_O999;
    MINT32 LCE_A_CEN_MAX_SLOPE;
    MINT32 LCE_A_CEN_MIN_SLOPE;

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

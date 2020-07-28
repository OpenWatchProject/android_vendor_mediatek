#ifndef __CCU_AE_ALGO_DATA_H__
#define __CCU_AE_ALGO_DATA_H__

#include "ccu_ae_utility.h"
#include "ccu_ae_param.h"
#include "ccu_awb_param.h"

/*Delete*/
typedef unsigned int FIELD;
// CAM_OBC_OFFST0 CAM+0500H: OB for B channel
typedef struct {
        FIELD OBC_OFST_B                : 13;
        FIELD rsv_13                    : 19;
} CCU_ISP_CAM_OBC_OFFST0_T;

typedef union {
    //enum { MASK     = 0x00001FFF };
    //typedef ISP_CAM_OBC_OFFST0_T reg_t;
    CCU_ISP_CAM_OBC_OFFST0_T   bits;
    MUINT32 val;
} CCU_ISP_NVRAM_OBC_OFFST0_T;

// CAM_OBC_OFFST1 CAM+0504H: OB for Gr channel
typedef struct {
        FIELD OBC_OFST_GR               : 13;
        FIELD rsv_13                    : 19;
} CCU_ISP_CAM_OBC_OFFST1_T;

typedef union {
    //enum { MASK     = 0x00001FFF };
    //typedef ISP_CAM_OBC_OFFST1_T reg_t;
    CCU_ISP_CAM_OBC_OFFST1_T   bits;
    MUINT32 val;
} CCU_ISP_NVRAM_OBC_OFFST1_T;

// CAM_OBC_OFFST2 CAM+0508H: OB for Gb channel
typedef struct {
        FIELD OBC_OFST_GB               : 13;
        FIELD rsv_13                    : 19;
} CCU_ISP_CAM_OBC_OFFST2_T;

typedef union {
    //enum { MASK     = 0x00001FFF };
    //typedef ISP_CAM_OBC_OFFST2_T reg_t;
    CCU_ISP_CAM_OBC_OFFST2_T   bits;
    MUINT32 val;
} CCU_ISP_NVRAM_OBC_OFFST2_T;

// CAM_OBC_OFFST3 CAM+050CH: OB for R channel
typedef struct {
        FIELD OBC_OFST_R                : 13;
        FIELD rsv_13                    : 19;
} CCU_ISP_CAM_OBC_OFFST3_T;

typedef union {
    //enum { MASK     = 0x00001FFF };
    //typedef ISP_CAM_OBC_OFFST3_T reg_t;
    CCU_ISP_CAM_OBC_OFFST3_T   bits;
    MUINT32 val;
} CCU_ISP_NVRAM_OBC_OFFST3_T;

// CAM_OBC_GAIN0 CAM+0510H: OB gain for B channel
typedef struct {
        FIELD OBC_GAIN_B                : 13;
        FIELD rsv_13                    : 19;
} CCU_ISP_CAM_OBC_GAIN0_T;

typedef union {
    //enum { MASK     = 0x00001FFF };
    //typedef ISP_CAM_OBC_GAIN0_T reg_t;
    CCU_ISP_CAM_OBC_GAIN0_T   bits;
    MUINT32 val;
} CCU_ISP_NVRAM_OBC_GAIN0_T;

// CAM_OBC_GAIN1 CAM+0514H: OB gain for Gr channel
typedef struct {
        FIELD OBC_GAIN_GR               : 13;
        FIELD rsv_13                    : 19;
} CCU_ISP_CAM_OBC_GAIN1_T;

typedef union {
    //enum { MASK     = 0x00001FFF };
    //typedef ISP_CAM_OBC_GAIN1_T reg_t;
    CCU_ISP_CAM_OBC_GAIN1_T   bits;
    MUINT32 val;
} CCU_ISP_NVRAM_OBC_GAIN1_T;

// CAM_OBC_GAIN2 CAM+0518H: OB gain for Gb channel
typedef struct {
        FIELD OBC_GAIN_GB               : 13;
        FIELD rsv_13                    : 19;
} CCU_ISP_CAM_OBC_GAIN2_T;

typedef union {
    //enum { MASK     = 0x00001FFF };
    //typedef ISP_CAM_OBC_GAIN2_T reg_t;
    CCU_ISP_CAM_OBC_GAIN2_T   bits;
    MUINT32 val;
} CCU_ISP_NVRAM_OBC_GAIN2_T;

// CAM_OBC_GAIN3 CAM+051CH: OB gain for R channel
typedef struct {
        FIELD OBC_GAIN_R                : 13;
        FIELD rsv_13                    : 19;
} CCU_ISP_CAM_OBC_GAIN3_T;

typedef union {
    //enum { MASK     = 0x00001FFF };
    //typedef ISP_CAM_OBC_GAIN3_T reg_t;
    CCU_ISP_CAM_OBC_GAIN3_T   bits;
    MUINT32 val;
} CCU_ISP_NVRAM_OBC_GAIN3_T;

//
typedef union {
    //enum { COUNT = 8 };
    MUINT32 set[8];
    struct {
        CCU_ISP_NVRAM_OBC_OFFST0_T  offst0; // Q.1.12
        CCU_ISP_NVRAM_OBC_OFFST1_T  offst1; // Q.1.12
        CCU_ISP_NVRAM_OBC_OFFST2_T  offst2; // Q.1.12
        CCU_ISP_NVRAM_OBC_OFFST3_T  offst3; // Q.1.12
        CCU_ISP_NVRAM_OBC_GAIN0_T   gain0; // Q.0.4.9
        CCU_ISP_NVRAM_OBC_GAIN1_T   gain1; // Q.0.4.9
        CCU_ISP_NVRAM_OBC_GAIN2_T   gain2; // Q.0.4.9
        CCU_ISP_NVRAM_OBC_GAIN3_T   gain3; // Q.0.4.9
    };
} CCU_ISP_NVRAM_OBC_T;
#define NVRAM_ISP_REGS_ISO_SUPPORT_NUM 20
/*Delete*/

#define LANDMARK_MAX_FACE_NUM 1
#define LANDMARK_SMOOTH_MAX_LENGTH 3
#define AE_LTM_FD_HIST_BIN 256

enum {
    CCU_AE_DBG_OPTION_ALL_OFF = 0,
    CCU_AE_DBG_OPTION_DAMPING = 1,
    CCU_AE_DBG_OPTION_FRAME_RATE = 2,
    CCU_AE_DBG_OPTION_EXP_GAIN = 3,
    CCU_AE_DBG_OPTION_FLARE = 4,
    CCU_AE_DBG_OPTION_MODE = 5,
    CCU_AE_DBG_OPTION_AE_STAT = 6,
    CCU_AE_DBG_OPTION_AE_CONDITION = 7,
    CCU_AE_DBG_OPTION_AE_WIN_INFO = 8,
    CCU_AE_DBG_OPTION_ALL_ON = 9
}; // useful check ok

typedef enum {
    CCU_HS_LIMIT_NONE,
    CCU_HS_LIMIT_AETARGET,
} CCU_AE_HS_LIMITEDBY; // useful check ok

typedef enum {
    AOE_LIMIT_NONE,
    AOE_LIMIT_COEP,
    AOE_LIMIT_MIXED,
} CCU_AE_AOE_LIMITEDBY; // useful check ok

typedef enum {
    CCU_ABL_LIMIT_NONE,
    CCU_ABL_LIMIT_CENTER_HIGHBOUND,
    CCU_ABL_LIMIT_TARGET_STRENGTH,
} CCU_AE_ABL_LIMITEDBY; // useful check ok

typedef enum {
    CCU_NS_LIMIT_NONE,
    CCU_NS_LIMIT_BTL,
    CCU_NS_LIMIT_DTL_UP,
    CCU_NS_LIMIT_DTL
} CCU_AE_NS_LIMITEDBY; // useful check ok

typedef enum {
    CCU_TARGET_COE_OUTERY,
    CCU_TARGET_COE_DIFF,
    CCU_TARGET_ABL_DIFF,
    CCU_TARGET_ABL_ABSDIFF,
    CCU_TARGET_HS_FLAT,
    CCU_TARGET_HS_COLOR,
    CCU_TARGET_NS_FLAT,
    CCU_TARGET_HDR_AUTO_EV_WEIGHT,
    CCU_TARGET_HDR_AUTO_ABL_WEIGHT,
    CCU_TARGET_HDR_AUTO_LV_WEIGHT,
    CCU_TARGET_HDR_AUTO_RMGT,
    CCU_TARGET_HDR_COEP_OUTERY,
    CCU_TARGET_HDR_COEP_DIFF,
    CCU_TARGET_HDR_ABL_CENTER_BRIGHTER_DIFF,
    CCU_TARGET_HDR_ABL_CENTER_DARKER_DIFF,
    CCU_TARGET_HDR_NS_EV_DIFF,
    CCU_TARGET_HDR_NS_HIST_Y,
    CCU_TARGET_HDR_NS_LV,
    CCU_TARGET_HDR_NS_BRIGHTY,
    CCU_TARGET_HDR_MIN_LIMITY_L,
    CCU_TARGET_HDR_MAX_LIMITY_L,
    CCU_TARGET_HDR_MIN_LIMITY_M,
    CCU_TARGET_HDR_MAX_LIMITY_M,
    CCU_TARGET_HDR_MIN_LIMITY_H,
    CCU_TARGET_HDR_MAX_LIMITY_H
    //            TARGET_NS_LB,
} CCU_AE_TARGET; // useful check ok

typedef enum {
    CCU_AE_Y_8BITS,
    CCU_AE_Y_12BITS,
    CCU_AE_Y_14BITS
} CCU_AE_HDR_LSB; // useful check ok

typedef struct {
    //////           Center OE      ///////
    MINT32 COEYMean_outer;
    MINT32 COEYMean_inner;
    MINT32 COEEVDiff_InOut;

    //////           Backlit         ///////
    MINT32 MaxEVDiff;

    //Open ABL enhance
    MINT32 CType_YMean_Dark[4];
    MINT32 CType_YMean_Bright[4];
    MINT32 Corner_YMean_Dark[4];
    MINT32 Corner_YMean_Bright[4];

    MINT32 CBL_EVDiff[8];
    MINT32 CBL_MaxEVDiff;
    MINT32 CBL_absMaxEVDiff;
} CCU_AE_SCENE_STS; // useful check ok

typedef struct {

    MUINT32 Target_ABL;
    MUINT32 Target_AOE;
    MINT32 Target_HS;
    MUINT32 Target_NS;
    MUINT32 Target_Main; //v4p0
    MUINT32 Target_HSv4p0; //v4p0

    MUINT32 Target_PrevABL;
    MUINT32 Target_PrevAOE;
    MINT32 Target_PrevHS;
    MUINT32 Target_PrevMVHDRHS;
    MUINT32 Target_PrevNS;
    MUINT32 Target_PrevFinal;
    MUINT32 Target_PrevMain; //v4p0
    MUINT32 Target_PrevHSv4p0; //v4p0

    // ABL
    MUINT32 Hist_ABL_THD;
    MUINT32 Hist_ABL_Target;

    // AOE
    MUINT32 Hist_AOE_THD;
    MUINT32 Hist_AOE_TargetGain;
    MUINT32 Hist_AOE_Target;
    MUINT32 AOE_Y;
    MUINT32 OE_Ratio;
    MUINT32 OE_Percent;
    MUINT32 OE_DeltaY;
    MUINT32 OE_DeltaY_SE;
    MUINT32 OE_DeltaY_Ori_SE;
    MUINT32 OE_Ori_SEY;
    MUINT32 Prob_OE_P;
    MUINT32 Prob_OE_Ratio_P;
    MUINT32 Final_Prob_OE_P;
    MUINT32 Final_Ltm_Weight;    
    
    MUINT32 Prob_OE_Value_Hbound;
    MUINT32 Pre_LTM_CT;
    float   Pre_LTM_CAP;
    float   Final_LTM_CAP;
    // HS
    MUINT32 Hist_HS_LimitY;
    MUINT32 Hist_HS_LimitTHD;
    MINT32 Hist_HS_LimitTHDNormal;
    MINT32 Hist_HS_LimitTHDFlat;
    MUINT32 Hist_HS_TargetGain;
    MUINT32 Hist_HS_Target;

    // CCU_HDR_AE_SUPPORT +++
    // MVHDR HS
    MINT32 MVHDR_Hist_HS_LimitTHD;
    MUINT32 MVHDR_Orig_HS_FLAT;
    MUINT32 MVHDR_Prob_HS_FLAT;
    MUINT32 MVHDR_HS_FlatLowY;
    MUINT32 MVHDR_HS_FlatHighY;
    MUINT32 MVHDR_Hist_HS_TargetGain;
    MUINT32 MVHDR_Hist_HS_Target;
    // CCU_HDR_AE_SUPPORT ---

    MINT32 Wet_ABL;
    MINT32 Wet_AOE;
    MINT32 Wet_HS;
    MINT32 Wet_Main; //v4p0
    MINT32 Wet_HSv4p0; //v4p0

    MUINT32 Prob_COE_OuterY;
    MUINT32 Orig_COE_OuterY;
    MUINT32 Prob_COE_Diff;
    MINT32 Orig_COE_Diff;
    MUINT32 Prob_COE_P;
    MUINT32 AOE_COE_Hist_Y;
    MUINT32 AOE_COE_THD;
    MUINT32 AOE_COE_Target;
    CCU_AE_AOE_LIMITEDBY AOE_LIMITED_BY;

    MUINT32 Prob_ABL_P;
    MUINT32 Orig_ABL_P;
    MUINT32 Prob_ABL_BV;
    MUINT32 Orig_ABL_BV;
    MUINT32 Prob_ABL_DIFF;
    MUINT32 Orig_ABL_DIFF;
    MUINT32 ABL_Y;
    CCU_AE_ABL_LIMITEDBY ABL_LIMITED_BY;

    MUINT32 Orig_HS_FLAT;
    MUINT32 Prob_HS_FLAT;
    MUINT32 Orig_HS_COLOR;
    MUINT32 Prob_HS_COLOR;
    MUINT32 Orig_HS_BV;
    MUINT32 Prob_HS_BV;
    MUINT32 HS_FlatLowY;
    MUINT32 HS_FlatHighY;
    CCU_AE_HS_LIMITEDBY HS_LIMITED_BY;

    MUINT32 Prob_MAIN_COLOR;

    ////////////////////////////////
    MUINT32 Orig_NS_BV;
    MUINT32 Prob_NS_BV;
    MUINT32 NS_CDF_Y;
    MINT32    NS_CDF_BV;
    MUINT32 Prob_NS_CDF;
    MUINT32 Prob_NS;

    MUINT32 Orig_NS_FLAT;
    MUINT32 Prob_NS_FLAT;
    MUINT32 NS_OE_THD1;
    MUINT32 NS_OE_THDFLAT;
    MUINT32 NS_OE_THD;

    MUINT32 NS_BrightTone_Y;
    MUINT32 NS_BrightTone_THDS;
    MUINT32 NS_BrightTone_Diff;
    MUINT32 NS_BrightTone_THD;
    MUINT32 NS_BrightTone_Target;

    MUINT32 Prob_NS_Sky;

    MUINT32 NS_BT_Limit;
    MUINT32 NS_BT_THD;
    MUINT32 NS_BT_Target;
    MUINT32 NS_Target;
    MUINT32 NS_FlatLowY;
    MUINT32 NS_FlatHighY;
    MUINT32 NS_Y;
    MUINT32 NS_BT_Y;
    CCU_AE_NS_LIMITEDBY NS_LIMITED_BY;

    // color info
    MUINT32 m_u4SceneSumR;
    MUINT32 m_u4SceneSumG;
    MUINT32 m_u4SceneSumB;
    MUINT32 m_u4ColorDistance;
    MUINT32 m_u4PrevColorDistance;

    // Hist Counting
    MUINT32 u4FlareRGBHistCount;

    MUINT32 u4FullRGBHistCount;
    MUINT32 u4CentralYHistCount;
    MUINT32 u4FullYHistCount;
    MUINT32 u4FullRHistCount;
    MUINT32 u4FullGHistCount;
    MUINT32 u4FullBHistCount;

    MUINT32 u4FullRGBHistSECount;
    MUINT32 u4CentralYHistSECount;
    MUINT32 u4FullYHistSECount;
    MUINT32 u4FullRHistSECount;
    MUINT32 u4FullGHistSECount;
    MUINT32 u4FullBHistSECount;

    //Color Suppression
    MUINT32 u4Orig_Cs_Gs_G_Ratio;
    MUINT32 u4Prob_Cs_Gs_GC;
    MUINT32 u4Prob_Cs_Gs_BV;
    MUINT32 u4Prob_Cs_Gs;
    MUINT32 u4Cs_Gs_HS_OEhit;
    MUINT32 u4Orig_Cs_Gs_HS_LimtY;
    MUINT32 u4Pure_Cs_Gs_HS_LimtY;
    MUINT32 u4Cs_Gs_HS_LImitY;
    MUINT32 u4Orig_Cs_Gs_MT_CWV;
    MUINT32 u4Cs_Gs_MT_CWV;

    MUINT32 Orig_DR;
} CCU_AE_TARGET_STS; // useful check ok

typedef enum {
    CCU_FT_ABL = 0,
    CCU_FT_ABL_ABS,
    //          FT_AOE,
    CCU_FT_HS,
    CCU_FT_NS_BV,
    CCU_FT_NS_CDF,
    CCU_FT_NS_SKY,
    CCU_FT_NUM
} CCU_AE_FEATURE_TARGET; // useful check ok

typedef enum {
    CCU_HIST_DARK,
    CCU_HIST_BRIGHT,
} CCU_AE_HIST_END; // useful check ok

typedef enum {
    CCU_HIST_PCENT,
    CCU_HIST_THD,
    CCU_HIST_INTV,
    CCU_HIST_MAXBIN,
} CCU_AE_HIST_MEASUREMETHOD; // useful check ok

typedef struct {
    CCU_AE_HIST_END             eEndPoint;
    CCU_AE_HIST_MEASUREMETHOD   eMethod;
    UINT32                  u4Start;
    UINT32                  u4End;
    UINT32                  u4Percentage;
    UINT32                  u4Thd;
    UINT32                  u4Feather;
    UINT32                  u4CurY;
    UINT32                  u4Bin;
    UINT32                  u4Count;
} CCU_AE_HIST_SPEC; // useful check ok

typedef struct
{
    MBOOL bAELock;
    MBOOL bEnableAE;
    MBOOL bIsoSpeedReal;
    MBOOL bVideoDynamic;
    MBOOL bVideoRecording;
    MBOOL bAElimitor;
    MBOOL bStrobeOn;
    MBOOL bRotateWeighting;
    MBOOL bAEObjectTracking;
    CCU_LIB3A_AE_SCENE_T    eAEScene;     // change AE Pline
    CCU_LIB3A_AE_EVCOMP_T    eAEComp;
    CCU_LIB3A_AE_METERING_MODE_T    eAEMeterMode;
    MUINT32    u4AEISOSpeed;   // change AE Pline
    CCU_LIB3A_AE_FLICKER_MODE_T    eAEFlickerMode;    // change AE Pline
    MINT32    i4AEMaxFps;
    MINT32    i4AEMinFps;
    CCU_LIB3A_AE_FLICKER_AUTO_MODE_T    eAEAutoFlickerMode;   // change AE Pline
    CCU_LIB3A_AECAM_MODE_T eAECamMode;
    CCU_AE_SENSOR_DEV_T eSensorDev;
    MINT32    i4SensorMode;
} CCU_AE_CMD_SET_T;

typedef struct {
    MBOOL   bMeteringEnable;
    MBOOL   bFDEnable;
    MUINT32 u4FaceFailCnt;
    MUINT32 u4FaceFoundCnt;
    MUINT32 u4MeterFacePreTarget;
    MUINT32 m_u4FDY;
    MUINT32 m_u4FDProb;
    MUINT32 m_u4HdrFDProb;
    MUINT32 m_u4FaceFrameCnt;
    MUINT32 m_u4MeteringCnt;
    MINT32 m_i4FaceXDR;
    MUINT32 m_u4FaceDis;
    MUINT32 m_u4FaceProbLoc;
    MUINT32 m_u4FaceProbSize;
    MUINT32 m_u4FaceSizeLocFinalProb;
    MUINT32 m_u4IsRobustFace;
    MUINT32 m_u4IsSideFace;
    MUINT32 m_u4IsOTFace;
    MINT32  m_i4FaceId;
    MUINT32 m_u4FaceStableCount;
    MUINT32 m_u4FaceDelayCnt;
    MUINT32 u4PrevFD;                    // Still Use
    MUINT32 m_u4MaxWeighting;            // Still Use
    MUINT32 m_u4Weighting[CCU_MAX_AE_METER_AREAS];
    MUINT32 m_i4FDMotionHist[FD_Motion_FrameBuffer][FD_Motion_Direction]; //motion using
    MUINT32 m_i4FDSizeHist[FD_SizeVar_FrameBuffer][FD_SizeVar_Direction]; //size record using
    CCU_AE_FD_BLOCK_WINDOW_T m_AEFDWinBlock[CCU_MAX_AE_METER_AREAS];
    CCU_AE_BLOCK_WINDOW_T m_AEFDWinBlock_Scale[CCU_MAX_AE_METER_AREAS];
    CCU_AE_BLOCK_WINDOW_T m_LTMFDWinBlock_Scale[CCU_MAX_AE_METER_AREAS];
    CCU_AE_BLOCK_WINDOW_T m_AEMeteringBlock[CCU_MAX_AE_METER_AREAS];
    MUINT32 m_u4FaceCentralYValue[CCU_MAX_AE_METER_AREAS];
    MUINT8  m_u4FaceNum;
    MUINT32 m_u4MaxFaceIdx;
    MINT32  m_i4IsTrueFaceCNN;
    MUINT32 m_u4TotalWeight;
    MUINT8  m_u4FaceNumHist; //sorting using
    MINT32  m_i4FD_Y_SortIdx_Array_Hist[CCU_MAX_AE_METER_AREAS]; //sorting using
    MINT32  i4FD_Size_temp_origHist[CCU_MAX_AE_METER_AREAS]; //sorting using
    MINT32  i4FD_Size_temp_sortHist[CCU_MAX_AE_METER_AREAS]; //sorting using
    MINT32  i4FDIdHist[CCU_MAX_AE_METER_AREAS]; //sorting using
    MUINT32 m_u4FaceStableCnt; //sorting using
    MUINT32 m_u4LCETarget; //LCE link using
    MUINT32 m_u4LCEHalfTarget; //LCE link using
    MUINT32 m_u4FaceRobustCnt;//LCE link using
    MUINT32 m_u4FaceRobustTaget;//LCE link using
    MINT32  m_i4EVCompDiff;//LCE link using for EC comp
    MUINT8  m_u4FaceOTCnt;//LCE link using
    MUINT8  m_u4FaceState;//LCE link using
    MUINT8  bEnableFaceAE;//LCE link using for NVRAM
    MUINT8  m_u4FaceRobustTrustCnt;//LCE link using for NVRAM
    MUINT32 m_u4FD_Lock_MaxCnt;//LCE link using for NVRAM
    MUINT8  m_u4FDDropTempSmoothCnt;//LCE link using for NVRAM
    MUINT8  m_u4OTFaceTrustCnt;//LCE link using for NVRAM
    MBOOL   bOTFaceTimeOutLockAE;//LCE link using for NVRAM
    MBOOL   bEnableContrastLink;//LCE link using for NVRAM
} CCU_AE_FD_STS;

typedef enum {
    CCU_FACE_STATE_FDIN,
    CCU_FACE_STATE_FDOUT,
    CCU_FACE_STATE_FDCONTINUE,
    CCU_FACE_STATE_FDDROP,
    CCU_FACE_STATE_END
} CCU_AE_FACE_STATE;

typedef enum {
    CCU_APP_MODE_PV,
    CCU_APP_MODE_VDO,
    CCU_APP_MODE_FD,
    CCU_APP_MODE_TK,
    CCU_APP_MODE_END
} CCU_AE_APP_MODE;

typedef enum  {
    CCU_CONV_BRIGHT,
    CCU_CONV_DARK,
    CCU_CONV_END
} AE_CONV_COND;

typedef struct {
    MBOOL   bPerFrameETFlag;    //AE Perframe Equivalent Translation flag
    MUINT32 u4CurActExp;        //AE Perframe Current frame Active ExpsoureTime
    MUINT32 u4NextActExp;       //AE Perframe Next frame Active ExpsoureTime
    MUINT32 u4CurActAfe;        //AE Perframe Current frame Active AFE Gain
    MUINT32 u4NextActAfe;       //AE Perframe Next frame Active AFE Gain
    MUINT32 u4CurAplAfe;        //AE Perframe Current frame Apply AFE Gain
    MUINT32 u4PreExp;           //AE Perframe Previous frame ExpsoureTime
    MUINT32 u4PreAfe;           //AE Perframe Previous frame AFE Gain
    MUINT32 u4PreIsp;           //AE Perframe Previous frame ISP Gain
    MUINT32 u4CurIsp;           //AE Perframe Current frame ISP Gain
    MUINT32 u4MinGainDelay;     //AE Perframe Minimal Gain Delay Frame
    MUINT32 u4ISPGainDelay;     //AE Perframe Isp Gain Delay Frame
    MUINT32 u4EqRatio;          //AE Perframe Final EquivalentTranslation Ratio
    MUINT32 u4EqOriRatio;       //AE Perframe Ori EquivalentTranslation Ratio
    MUINT32 u4OriCWV;           //AE Perframe Ori CWV
    MUINT32 u4NextCWV;          //AE Perframe Next Frame CWV
} CCU_AE_PERFRAME_T; // useful check ok

typedef struct {
    MUINT32 u4PreY;
    MUINT32 u4CurY;
    MINT32  i4PredDeltaEV;
    MINT32  i4TrueDeltaEV;
    MBOOL   bB2DNonLinear;
    MBOOL   bD2BNonLinear;
    MINT32 i4AccRatio;
    MINT32 i4AccBVRatio;
    MINT32 i4EVRatio;
    MINT32 i4EVDiffx1000;
    MINT32 i4PrevEvDiffFrac;
    MUINT32 u4B2DLinearThd;
    MUINT32 u4D2BLinearThd;
} CCU_AE_LINEAR_RESP;

typedef struct {
    MUINT16 PreAEidx;//=0;
    MUINT16 Pre_PreAEidx;//=0;
    MUINT16 nAEStableCount;//=0;
    MUINT16 FlareArr[16];//={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    MUINT16 FlareArrCount;//=0;
} CCU_AE_FLARE_STS;

typedef struct
{
     MINT32     face_num;
     // Face candidate position
     MINT32     x0[LANDMARK_MAX_FACE_NUM];
     MINT32     y0[LANDMARK_MAX_FACE_NUM];
     MINT32     x1[LANDMARK_MAX_FACE_NUM];
     MINT32     y1[LANDMARK_MAX_FACE_NUM];
     // original position
     MINT32     ox0[LANDMARK_MAX_FACE_NUM][LANDMARK_SMOOTH_MAX_LENGTH];
     MINT32     oy0[LANDMARK_MAX_FACE_NUM][LANDMARK_SMOOTH_MAX_LENGTH];
     MINT32     ox1[LANDMARK_MAX_FACE_NUM][LANDMARK_SMOOTH_MAX_LENGTH];
     MINT32     oy1[LANDMARK_MAX_FACE_NUM][LANDMARK_SMOOTH_MAX_LENGTH];
     MINT32     motion[LANDMARK_MAX_FACE_NUM][4];
 } CCU_FACE_SMOOTH_INFO;

typedef enum
{
		CCU_AE_SMOOTH_STATUS_HDR,
        CCU_AE_SMOOTH_STATUS_FACE,
        CCU_AE_SMOOTH_STATUS_SMALL_RANGE,
        CCU_AE_SMOOTH_STATUS_EXTREME_OVER,
        CCU_AE_SMOOTH_STATUS_EXTREME_UNDER,
        CCU_AE_SMOOTH_STATUS_OVER,
        CCU_AE_SMOOTH_STATUS_OVER_POINTLIGHT,
        CCU_AE_SMOOTH_STATUS_POINTLIGHT,
        CCU_AE_SMOOTH_STATUS_NORMAL,
        CCU_AE_SMOOTH_STATUS_STABLE,
        CCU_AE_SMOOTH_STATUS_EXTREME_OVER_TOUCH,
        CCU_AE_SMOOTH_STATUS_EXTREME_UNDER_TOUCH,
        CCU_AE_SMOOTH_STATUS_NORMAL_TOUCH,
        CCU_AE_SMOOTH_STATUS_ONE_STEP
}CCU_AE_SMOOTH_STATUS;


typedef enum
{
        CCU_AE_STABLE_CASE_NOT_STABLE = 0,
        CCU_AE_STABLE_CASE_FACE_NOT_STABLE = 1,
        CCU_AE_STABLE_CASE_NORMAL_STABLE = 2,
        CCU_AE_STABLE_CASE_HIST_STABLE = 3,
        CCU_AE_STABLE_CASE_NORMAL_FIRST_STABLE = 4,
        CCU_AE_STABLE_CASE_NORMAL_CONTINUE_STABLE = 5,
        CCU_AE_STABLE_CASE_FACE_STABLE = 6,
        CCU_AE_STABLE_CASE_FACE_FIRST_STABLE = 7,
        CCU_AE_STABLE_CASE_FACE_CONTINUE_STABLE = 8,
        CCU_AE_STABLE_CASE_OTHER_STABLE = 9,
        CCU_AE_STABLE_CASE_BOUNDARY_FIRST_STABLE = 10,
        CCU_AE_STABLE_CASE_BOUNDARY_COUNTUE_STABLE = 11,
}CCU_AE_STABLE_CASE;


typedef struct
{
    MUINT32 YStart;
    MUINT32 YEnd;
    MUINT8 *BufferInRoi;
} CCU_BankInfo;

/*
*    LTM Related
*/
typedef struct
{
    MUINT32 u4ClipThd;
    MUINT32 u4TurnPoint;
    MUINT32 u4MiddlePoint;
    MFLOAT fCurveSlopK1;
    MFLOAT fCurveSlopK2;
} AE_LTM_INFO_T;

typedef struct
{
    MINT32 hist_after_ltm[AE_SW_HISTOGRAM_BIN];
    MFLOAT brightness_mapping_array[AE_SW_HISTOGRAM_BIN];
    MFLOAT ltm_cap;
    MUINT32 u4ClipThd;
    MUINT32 u4TurnPoint;
} ltm_to_ae_info;

/*
 *    AWB & AAO config
 */
typedef struct
{
    // Main stat window size
    UINT32 u4WindowSizeX;           //!<: Main stat window horizontal size
                                    //!<: Total window size must be smaller
                                    //!<: than 256K pixels. Min 4 pixels. Must be even

    UINT32 u4WindowSizeY;           //!<: Main stat window vertical size

                                    // Main stat window pitch
    UINT32 u4WindowPitchX;          //!<: Main stat window horizontal pitch
    UINT32 u4WindowPitchY;          //!<: Main stat window horizontal pitch

                                    // First AWB window original (most upper left)
    UINT32 u4WindowOriginX;         //!<: Horizontal origin of the 1st AWB window
    UINT32 u4WindowOriginY;         //!<: Vertical origin of the 1st AWB window

                                    // Number of AWB windows
    UINT32 u4WindowNumX;            //!<: Number of AWB windows horizontally. Max - 128
    UINT32 u4WindowNumY;            //!<: Number of AWB windows vertically. Max - 128

                                    // Thresholds
    UINT32 u4LowThresholdR;         //!<: Low threshold for R AWB stat range
    UINT32 u4LowThresholdG;         //!<: Low threshold for G AWB stat range
    UINT32 u4LowThresholdB;         //!<: Low threshold for G AWB stat range

    UINT32 u4HighThresholdR;        //!<: High threshold for R AWB stat range
    UINT32 u4HighThresholdG;        //!<: High threshold for G AWB stat range
    UINT32 u4HighThresholdB;        //!<: High threshold for G AWB stat range

                                    // Pixel counters
    UINT32 u4PixelCountR;           //!<: ROUND((1<<24)/number of red pixels in a window)
    UINT32 u4PixelCountG;           //!<: ROUND((1<<24)/number of green pixels in a window)
    UINT32 u4PixelCountB;           //!<: ROUND((1<<24)/number of blue pixels in a window)

                                    // Pre-gain maximum limit clipping
    UINT32 u4GainLimitR;           //!<: Maximum limit clipping for R color
    UINT32 u4GainLimitG;           //!<: Maximum limit clipping for G color
    UINT32 u4GainLimitB;           //!<: Maximum limit clipping for B color

                                   // Pre-gain values
    UINT32 u4GainValueR;           //!<: Pre-gain Value for R color (unsigned fix-point with 9 bit fraction and 4 bit integer)
    UINT32 u4GainValueG;           //!<: Pre-gain Value for G color (unsigned fix-point with 9 bit fraction and 4 bit integer)
    UINT32 u4GainValueB;           //!<: Pre-gain Value for B color (unsigned fix-point with 9 bit fraction and 4 bit integer)

                                   // AWB error threshold
    UINT32 u4ErrorThreshold;        //!<: Programmable threshold for the allowed total
                                    //!<: over-exposured and under-exposered pixels in the RGB average
                                    //!<: for AWB purposes.
                                    // AWB digital Zoom Window area for AWB Statistics
    UINT32 u4MinXWinNumber;            //!<: Minimum X window number
    UINT32 u4MinYWinNumber;            //!<: Minimum Y window number
    UINT32 u4MaxXWinNumber;            //!<: Maximum X window number
    UINT32 u4MaxYWinNumber;            //!<: Minimum Y window number
                                    // AWB stat DMA buffer pointer
    UINT32 u4AWBDmaPointer;         //!<: DMA address pointer in DDR, Must be 64 bits aligned

    UINT32 u4AWBDmaReset;           //!<: DMA Soft Reset, when asserted, DMA will stop without violating RAMBUF protocol. Will clear by itself after DMA is stopped.
    UINT32 u4AWBDmaFifoThreshold;   //!<: Programmable FIFO Threshold, req will assert when at least threshold entries reached.
                                    //!<: Range 0 to 63. Default is 16.
    UINT32 u4AWBDmaFifoDepth;       //!<: Programmable FIFO Depth/2, min 4, max 64 .
                                    //!<: Range 0 to 63. Default is 32.
} CCU_AWBSTAT_CONFIG_BLK;

#define AE_HST_CONFIG_MAX 6
#define AE_HST_BINMODE_128 0
#define AE_HST_BINMODE_256 1
#define AE_HST_COLORMODE_R 0
#define AE_HST_COLORMODE_G 1
#define AE_HST_COLORMODE_B 2
#define AE_HST_COLORMODE_RGB 3
#define AE_HST_COLORMODE_Y 4
#define AE_HST_COLORMODE_FLARE 5

typedef struct
{
    MUINT8 enable;
    int BIN_MODE;                               /* Bin mode 0: 128bin        */
                                                /*          1: 256bin        */
                                                /*          2: 256bin Low    */
                                                /*          3: 256bin Hi     */
                                                /*                           */
    int COLOR_MODE;                             /* Input source color,BW     */
                                                /*          0:R              */
                                                /*          1:G              */
                                                /*          2:B              */
                                                /*          3:BW R,G,B       */
                                                /*          4:BW use AE_SUM  */
                                                /*                           */
    int X_RatioLOW;                              /* Ratio Range of interest   */
    int X_RatioHI;                              /* defined by window location*/
    int Y_RatioLOW;                              /*   0-100                   */
    int Y_RatioHI;                              /*                           */
                                                /*                           */
}CCU_HSTConfig_T;


typedef struct {
    MUINT32 m_u4SkipPcent;
    MUINT32 m_u4OEPcent;
    MUINT32 m_u410PcentY;
    MUINT32 m_u4OEPcentY;
    MUINT32 m_u4FDY;
    MINT32 m_i4DR;
    MUINT32 m_u4OeTh;
    MUINT32 m_u4FdTh;
    MUINT32 m_u4FdMinTh;
    MUINT32 m_u4OeTarget;
    MUINT32 u4FaceBalancedTarget;
} CCU_AE_FBTReport;


typedef struct
{
    MUINT8 AE_SE_STAT_EN;
    MUINT8 AE_OVERCNT_EN;
    MUINT8 AE_HST_DMA_OUT_EN;
    MUINT8 AE_HST_CCU_EN;
    MUINT8 AE_HSTSE_EN;
    CCU_HSTConfig_T AE_HST_Config[AE_HST_CONFIG_MAX];
}CCU_AAOConfig_T;

//****** members for AeAlgo ******
typedef struct
{
    /*Flow*/
    MUINT32 m_u4ReqNumberHead;
    MUINT32 m_u4ReqNumberTail;
    MUINT32 m_u4CamId;
    MBOOL m_bAEOneShot;
    MBOOL m_bIsFirstTime;
    MBOOL m_bTouchAEAreaChange;
    MBOOL m_bFaceAEAreaChange;
    MUINT32 m_u4LockIndexF;
    MUINT32 m_u4LockIndex;
    MUINT32 m_i4EVLockIndexF;
    MUINT32 m_i4EVLockIndex;
    MUINT32 m_u4PrevEVComp;
    MINT32 m_i4AEDebugCtrl;
    MUINT32 m_u4PlineChange;

    CCU_AE_CTRL m_AECtrl;
    CCU_CYCLE_CTRL m_CycleCtrl;
    CCU_FEATURE_CTRL m_FeatureCtrl;
    CCU_EV_QUEUE_CTRL m_EVCtrl;
    CCU_ALGO_CTRL m_AlgoCtrl;
    CCU_AE_TargetMODE m_AETargetMode;

    MINT32 m_i4PreSensorMode;
    MBOOL m_bPerFrameAESmooth; 
    MUINT32 m_u4InStableThd;
    MUINT32 m_u4OutStableThd;
    MBOOL m_bEnableZeroStableThd;
    MUINT32 m_u4B2TStart;
    MUINT32 m_u4B2TEnd;
    MUINT32 m_u4D2TStart;
    MUINT32 m_u4D2TEnd;
    MBOOL m_bPreCalibEnable;
    MBOOL m_adb_bEableManualTarget;
    MUINT32 m_adb_u4ManualTarget;
    MBOOL m_adb_bEableCalib;

    /*Statistic*/
    CCU_AE_BLOCK_WINDOW_T m_AEWinBlock;
    MUINT32 m_u4AAO_AWBValueWinSize8;
    MUINT32 m_u4AAO_AWBValueSepSize8;
    MUINT32 m_u4AAO_AWBSumWinSize8;
    MUINT32 m_u4AAO_AWBSumSepSize8;
    MUINT32 m_u4AAO_AEYWinSize8;
    MUINT32 m_u4AAO_AEYSepSize8;
    MUINT32 m_u4AAO_AEYWinSizeSE8;
    MUINT32 m_u4AAO_AEYSepSizeSE8;
    MUINT32 m_u4AAO_AEOverWinSize8;
    MUINT32 m_u4AAO_AEOverSepSize8;

    MUINT32 m_u4HIST_LEWinSize8;
    MUINT32 m_u4HIST_LESepSize8;
    MUINT32 m_u4HIST_SEWinSize8;
    MUINT32 m_u4HIST_SESepSize8;
    MUINT32 m_u4AAO_LineSize8;

    MUINT32 m_u4AWB_Rounding;
    MUINT32 m_u4AWB_ShiftBits;
    MUINT32 m_u4Y_Rounding;
    MUINT32 m_u4Y_ShiftBits;
    //AE Block
    MUINT32 m_u4AEOverExpRatio;    //Global OE Ratio
    MUINT32 m_u4AEOverExpRatioR;   //Global R OE Ratio
    MUINT32 m_u4AEOverExpRatioG;   //Global G OE Ratio
    MUINT32 m_u4AEOverExpRatioB;   //Global B OE Ratio
    MUINT32 m_pu4AEOverExpBlockCnt[AE_V4P0_BLOCK_NO][AE_V4P0_BLOCK_NO];
    MUINT32 m_pu4AEOverExpBlockCntR[AE_V4P0_BLOCK_NO][AE_V4P0_BLOCK_NO];
    MUINT32 m_pu4AEOverExpBlockCntG[AE_V4P0_BLOCK_NO][AE_V4P0_BLOCK_NO];
    MUINT32 m_pu4AEOverExpBlockCntB[AE_V4P0_BLOCK_NO][AE_V4P0_BLOCK_NO];

    MUINT32 m_u4BlockValidHeight5x5; // AE valid 5x5 data region
    MUINT32 m_u4BlockValidHeight15x15; // AE valid 15x15 data region
    MUINT32 m_pu4AEBlock[AE_BLOCK_NO][AE_BLOCK_NO]; // AE block value in Algorithm level
    MUINT32 m_pu4AEBlockSE[AE_BLOCK_NO][AE_BLOCK_NO]; // AE SE block value in Algorithm level
    MUINT32 m_pu4AEv4p0Block[AE_V4P0_BLOCK_NO][AE_V4P0_BLOCK_NO]; // AE v4p0 block value in Algorithm level
    MUINT32 m_pu4AEv4p0BlockSE[AE_V4P0_BLOCK_NO][AE_V4P0_BLOCK_NO]; // AE SE v4p0 block value in Algorithm level
    MUINT32 m_pu4AEBlockR[AE_V4P0_BLOCK_NO][AE_V4P0_BLOCK_NO];
    MUINT32 m_pu4AEBlockG[AE_V4P0_BLOCK_NO][AE_V4P0_BLOCK_NO];
    MUINT32 m_pu4AEBlockB[AE_V4P0_BLOCK_NO][AE_V4P0_BLOCK_NO];

    MUINT32 m_u4AEOverExpSE;
    MUINT32 m_u4AEOverExpCntSE;
    MUINT32 m_u4AETotalCntSE;

    // Linear Block
    MUINT32 m_pu4AELieanrSortedY[AE_V4P0_BLOCK_NO * AE_V4P0_BLOCK_NO];
    MUINT32 m_pu4AELieanrSortedSEY[AE_V4P0_BLOCK_NO * AE_V4P0_BLOCK_NO];

    //AE Hist
    MUINT32 m_pu4CentralYHist[AE_SW_HISTOGRAM_BIN];
    MUINT32 m_pu4FullYHist[AE_SW_HISTOGRAM_BIN];
    MUINT32 m_pu4FullRGBHist[AE_SW_HISTOGRAM_BIN];
    MUINT32 m_pu4FullRHist[AE_SW_HISTOGRAM_BIN];
    MUINT32 m_pu4FullGHist[AE_SW_HISTOGRAM_BIN];
    MUINT32 m_pu4FullBHist[AE_SW_HISTOGRAM_BIN];

    MUINT32 m_pu4CentralYHistSE[AE_SW_HISTOGRAM_BIN];
    MUINT32 m_pu4FullYHistSE[AE_SW_HISTOGRAM_BIN];
    MUINT32 m_pu4FullRGBHistSE[AE_SW_HISTOGRAM_BIN];
    MUINT32 m_pu4FullRHistSE[AE_SW_HISTOGRAM_BIN];
    MUINT32 m_pu4FullGHistSE[AE_SW_HISTOGRAM_BIN];
    MUINT32 m_pu4FullBHistSE[AE_SW_HISTOGRAM_BIN];

    /*General*/
    CCU_eAETableID  eID;
    CCU_eAETableID  eCapID;
    MUINT32 u4TotalIndex;
    MUINT32 u4CapTotalIndex;
    MINT32 m_i4MaxBV;
    MINT32 m_i4PreMaxBV;
    MINT32 m_i4MinBV;
    MINT32 m_i4CapMaxBV;
    MINT32 m_i4CapMinBV;
    MUINT32 m_u4Index;       // current AE index
    MUINT32 m_u4IndexF;
    MUINT32 m_u4IndexFMax;
    MUINT32 m_u4IndexFMin;
    MUINT32 m_u4IndexMax;
    MUINT32 m_u4IndexMin;
    MINT32 m_i4RealBV;
    MINT32 m_i4RealBVx1000;
    MUINT32 m_u4AETarget;
    MINT32 m_i4EV;
    MINT32 m_i4BV;
    MINT32 m_i4AoeCompBV;
    MINT32 m_i4IspGain;
    MUINT32 m_u4EffectiveIndex;
    MINT32 m_i4AEIndexF[CCU_AE_SENSOR_MAX];
    MINT32 m_i4AEIndex[CCU_AE_SENSOR_MAX];
    MUINT32 m_u4NvramIdx;   
    CCU_AE_ZOOM_WIN_INFO m_ZoomWinInfo;
    CCU_strAEOutput m_CurAEOutput;
    CCU_strEvSetting m_CurEvSetting;

    /*Metering*/
    MUINT32 m_u4GauCWValue;     // center weighting value
    MUINT32 m_pWtTbl_W[5][5];    //AE weighting table
    MUINT32 m_u4WeightingSum;
    MUINT32 m_u4CWValue;     // center weighting value
    MUINT32 m_u4CWSEValue;    // center weighting value
    MUINT32 m_u4AvgWValue;     // average weighting value
    MUINT32 m_u4AvgWSEValue;     // average weighting value
    MUINT32 m_u4MeterY;
    MUINT32 m_u4CentralY;            //Y value of central block
    MUINT32 m_u4LinearCWValue; //center weighting value of linear AAO
    MUINT32 m_u4MgrCWValue;    // center weighting value for mgr use
    MUINT32 m_u4CWRecommendPureAE; // AE recommend cw target (after Contrast Link, after backlight, anti-overxposure...)
    MUINT32 m_u4CWRecommend; // recommend cw target (after Contrast Link)
    MUINT32 m_u4CWRecommendPureAE_Stable; //recommend cw target(before Contrast Link), pass to LCE
    MUINT32 m_u4CWRecommendAfterPureAE_Stable;
    MUINT32 m_u4CWRecommend_Stable; // recommend cw target (after Contrast Link), pass to LCE
    MUINT32 m_u4AECondition;         // record backlight, hist stretch. over exp... condition , hit or not
    MBOOL m_bIsBlueSkyDet;
    MINT32 m_i4BlueStableCount;
    CCU_AE_SCENE_STS m_SceneCls;
    CCU_AWB_GAIN_T m_AWBGains;
    CCU_AE_TARGET_STS m_TargetSts;
    MUINT32 m_u4BacklightCWM;
    MUINT32 m_u4AntiOverExpCWM;
    MUINT32 m_u4HistoStretchCWM;
    MBOOL m_bAEMeteringEnable;
    MBOOL m_bAEMeteringStable;
    MUINT8 m_u4MeteringCnt;
    MUINT32 m_u4MeterStableValue;
    MUINT32 m_pu4MeteringY[CCU_MAX_AE_METER_AREAS];
    CCU_AE_TOUCH_BLOCK_WINDOW_T m_AETOUCHWinBlock[CCU_MAX_AE_METER_AREAS];
    CCU_AE_TOUCH_BLOCK_WINDOW_T m_PreAETOUCHWinBlock[CCU_MAX_AE_METER_AREAS]; //-1000~1000 domain
    MUINT32 m_i4VdoPrevTarget;
    MUINT32 m_u4PrevVdoTarget;

    /*Face*/
    MINT32 m_i4FaceDiffIndex;
    CCU_AE_FD_STS m_MeterFDSts;
    MINT32  m_i4Prev_FDY;
    MINT32  m_i4Crnt_FDY;
    MINT32  m_i4PrevAFDone;
    MINT32  m_i4CrntAFDone;
    MINT32  m_i4LastRobust_FDY;
    MINT32  m_i4PrevFaceId;
    MINT32  m_i4PrevFaceNum;
    MINT32  m_i4PrevFaceStableWidth;
    MINT32  m_i4PrevFaceStableHeight;
    MINT32  m_i4PrevFaceStableCenterX;
    MINT32  m_i4PrevFaceStableCenterY;
    MINT32  m_i4PrevFaceStableRealBVx1000;
    MUINT32 m_u4PrevIsOTFace;
    MINT32  m_i4HugeYDiffCnt;
    MUINT32 m_i4PrevFaceFrameCnt;
    MUINT32 m_u4MeterFDTarget;
    CCU_AE_FBTReport m_FBTRep;
    MUINT32 m_u4FCWR_Orgin;
    MUINT32 m_u4FaceCaseNum;
    MUINT32 m_u4NormalTarget;
    MUINT32 m_u4FaceFinalTarget;
    MUINT32 m_u4FaceAEStable;
    MUINT32 m_u4FaceAEStableIndex;
    MUINT32 m_u4FaceAEStableCWV;
    MUINT32 m_u4FaceAEStableCWR;
    MUINT32 m_u4FaceAEStableConvergeY; //smooth Face use, record stable FDY 
    CCU_AE_BLOCK_WINDOW_T m_FaceAEStableBlock;
    MINT32 m_i4FDY_ArrayFaceWTBHist[FD_Hist_FrameBuffer][CCU_MAX_AE_METER_AREAS];
    MINT32 m_i4FDY_ArrayFace[CCU_MAX_AE_METER_AREAS];
    MINT32 m_i4FDY_ArrayFace_CCUFrameDone[CCU_MAX_AE_METER_AREAS];
    MINT32 m_i4FDY_ArrayLM[CCU_MAX_AE_METER_AREAS];
    MINT32 m_i4FDY_ArrayLM_CCUFrameDone[CCU_MAX_AE_METER_AREAS];
    MINT32 m_i4FDY_ArrayOri[CCU_MAX_AE_METER_AREAS];
    MINT32 m_i4FDY_ArrayOri_CCUFrameDone[CCU_MAX_AE_METER_AREAS];
    MINT32 m_i4FDY_Array[CCU_MAX_AE_METER_AREAS];
    MINT32 m_i4FDY_Array_CCUFrameDone[CCU_MAX_AE_METER_AREAS];
    MINT32 m_i4FD_Y_SortIdx_Array[CCU_MAX_AE_METER_AREAS];
    CCU_AEMeterArea_T m_LandMarkFDArea;
    CCU_AEMeterArea_T m_LandMarkFDSmoothArea;
    CCU_AEMeterArea_T m_FDArea[CCU_MAX_AE_METER_AREAS];
    CCU_FACE_SMOOTH_INFO m_PrevFaceSmoothResult;
    CCU_FACE_SMOOTH_INFO m_FaceSmoothResult;
    MINT8 m_FaceSmoothMatchIdxCurrent2Last[19];
    MINT32 m_i4DebugFrameCount;
    MINT32 m_i4DebugAAODump;
    MUINT32 m_i4PrevFaceProcessAEFrameCnt;
    MUINT32 m_i4FaceProcessAEFrameCnt;
    MUINT32 m_u4FaceAEStableInThd;
    MUINT32 m_u4FaceAEStableOutThd;
    MBOOL m_bFaceAELock;
    MINT32 m_i4CNNStablize;
    MINT32 m_i4CNNFaceFrameCnt;
    MINT32 m_i4GyroAcc[3];
    MINT32 m_i4GyroRot[3];
    MINT32 m_i4PrevGyroAcc[3];
    MUINT32 m_bFaceAELCELinkEnable;
    MUINT32 m_u4MaxGain;
    MUINT32 m_u4SERatio;
    MUINT32 m_u4preFDProbOE;
    MINT32 m_i4FDProbSize;
    MINT32 m_i4FDProbLoc;

	/*Folivora Effect*/
    MBOOL m_bFolivoraEffectEnable;
    MBOOL m_bInFolivoraEffect;
	MUINT32 m_u4FolivoraBValue;
    MUINT32 m_u4FolivoraCurValue;
	MUINT32 m_u4FolivoraFMatrix[AE_V4P0_BLOCK_NO][AE_V4P0_BLOCK_NO];
	MUINT32 m_u4FolivoraBTicket;
	MUINT32 m_u4FolivoraFTicket;
	MUINT32 m_u4FolivoraCurTicket;
	MBOOL m_bIsFolivoraMoving;
	MBOOL m_bIsFolivoraBMoving;
	MBOOL m_bIsFolivoraFMoving;
	MUINT32 m_u4FolivoraBMovingTH;
	MUINT32 m_u4FolivoraFMovingTH;
    MUINT32 m_u4FolivoraFMovingQuantity;
	MUINT32 m_u4FolivoraFMovingQuantityTH;
	MUINT32 m_u4FolivoraDarkMotionTH;
	MUINT32 m_u4FolivoraDarkMinMotion;
    MUINT32 m_bIsFolivoraInTracking;
	MUINT32 m_u4FolivoraTrackingTH;
	MUINT32 m_u4FolivoraHesitate;
	MUINT32 m_u4FolivoraHesitateTH;
    
    /* Butterfly Effect*/
    MBOOL m_bBtEffectEnable;
    MBOOL m_bBtInEffect;
    MBOOL m_bBtOver;
    MUINT32 m_bBtOrgStable;
    MUINT32 m_bBtStable;
    MUINT32 m_bBtPreDevStable;
    MUINT32 m_u4BtPreDevIndex;
    MUINT32 m_u4BtBValue;
    MUINT32 m_u4BtCurValue;
    MUINT32 m_u4BtBTicket;
    MUINT32 m_u4BtCTicket;
    MUINT32 m_bBtRightAfterSync;
    MUINT32 m_bBtSync;
    MUINT32 m_bBtMapping;
    MUINT32 m_u4BtTh;
    MUINT32 m_u4BtSpeed;
    MUINT32 m_u4BtChangeCnt;
    MUINT32 m_u4BtUpBound;
    MUINT32 m_u4BtLowBound;

    /*For Face LTM*/
    MINT32 m_i4LTMFaceBrightToneValue_12bit[CCU_MAX_AE_METER_AREAS];
    MUINT32 m_u4LTMCroppedFDHist[CCU_MAX_AE_METER_AREAS][AE_LTM_FD_HIST_BIN];
    MINT32 m_i4LTMFaceProtectPercent;
    MINT32 m_i4LTMFaceProtectBrightTonePercent;

    /*Smooth*/
    MBOOL m_bAEStable;
	CCU_AE_STABLE_CASE m_u4StableCase;
    CCU_AE_STABLE_CASE m_u4StableCaseCur;
    MINT32 m_i4EvDiffFrac;
    CCU_AE_CMD_SET_T m_AECmdSet;
    CCU_AE_LIMITOR_T m_AELimitor;
    MUINT32 m_u4StablePreOECnt;
    MUINT32 m_u4StablePreUECnt;
    MUINT32 m_u4StableOECnt;
    MUINT32 m_u4StableUECnt;
    MUINT32 m_u4AEFinerEVIdxBase;
    MINT32 m_i4AEFinerEVBase;
    MBOOL m_bFracGainLimited;
    MUINT32 m_u4FracGain;
    MINT32 m_i4DiffEV;
    MINT32 m_i4DeltaIndex; //current delta Index
    MINT32 m_i4DeltaCycleIndex;
    CCU_AE_LINEAR_RESP m_LinearResp;
    MUINT32 m_u4CWRecommendStable; // recommend cw target (after backlight, anti-overxposure...)
    CCU_AE_SMOOTH_STATUS e_prev_smooth_status;
    CCU_AE_SMOOTH_STATUS e_prev_smooth_status_pre;
    MINT32 prev_delta_idx;
    MINT32 prev2_delta_idx;
    MINT32 prev3_delta_idx;
    MINT32 prev_delta_sum;
    MINT32 delta_index_full;
    MUINT32 extreme_over_prob;
    MUINT32 extreme_under_prob;
    MUINT32 over_prob;
    MUINT32 over_point_light_prob;
    MUINT32 point_light_prob;
    MUINT32 non_linear_cwv_ratio;
    MUINT32 non_linear_avg_ratio;
    MINT32 next_not_compensation;
    MINT32 prev_delta_idx_for_frac;
    MINT32 gyro;
    MINT32 acc;

    /*AE sync to Single*/
    CCU_SYNC2SINGLE_REPORT m_sync_report;

    /*HDR*/
    MBOOL m_bIsMHDRMode;
    MUINT32 m_u4DeltaWeight;
    CCU_HDR_AE_INFO_T m_HdrAEInfo;
    MINT32 m_i4DeltaIdxHist[2];  // Record Delta_Idx for HDR Smooth Algo.
    MINT32 m_i4FlkrExpoBase[2];  // Transfer Ratio to Exposure/Gain
    MUINT32 m_u4ExpUnit;          // Transfer Ratio to Exposure/Gain

    /*LTM*/
    ltm_to_ae_info m_ltm_to_ae_info;
    AE_LTM_NVRAM_PARAM m_LtmNvramParam;

    /*Reversed*/
    MUINT32 m_u4Reserved[40];

    /*!!!!!!!!!!!!!!! PONITER MUST BE PUT AT BOTTOM MOST (due to CCU driver constraint) !!!!!!!!!!!!!!!!*/
    CCU_strAEMovingRatio *m_pCurMovingRatio;
    CCU_strEvPline *m_pCurrentTable;
    CCU_strFinerEvPline *m_pCurrentTableF;
    CCU_strAEPLineGainList *pCurrentGainList;
    CCU_AE_NVRAM_T* pAeNVRAM;

#if 0
    CCU_strEvSetting m_PreEvSettingQueue[MAX_AE_PRE_EVSETTING];
    MUINT32 u4FullBrightest;    //histogram information
    MINT32  m_i4GammaIdx;   // next gamma idx
    MBOOL   m_bNolinearAccEn;
    MUINT32 m_i4PrvDeltaIndex;
    MINT32 m_i4PreAEDeltaIndex;
    MBOOL m_ResetFullFrameCycle;
    MBOOL m_bAENotPerFrameMode;
    MINT32 m_i4ExtremeCond;
    MINT32 m_i4Cycle0FullDeltaIdx;
    CCU_AE_PERFRAME_T m_PerFrameInfo;
    MINT32  m_i4RealStableBV;
    CCU_strAEOutput m_OrigPrvAESetting;
    CCU_strAEOutput m_PrvStateAESetting;
    MBOOL m_bTouchIsReferencedFlag;
    MUINT32 m_u4PreACWValue;
    MUINT32 m_u4PreAVGValue;
    unsigned int totalCnt;
    CCU_strAEOutput m_CaptureAESetting;
    //SYNC
    MUINT32 m_u4RGBY[4], m_u4RGB[3]; //BAYER
    MUINT32 m_u4W[3], m_u4AvgW;//MONO
                               //FLOW
    MBOOL m_bCalculateAE;
    MBOOL m_bCalculateAEAP;
    MBOOL m_bSkipAE;
    MUINT32 m_u4AEScheduleCnt;
    MUINT32 m_u4AEExitStableCnt;
    MBOOL m_bPerFrameAEWorking;
    CCU_eAESTATE m_preAeState;
    MUINT8 m_uoutstable;
    MINT32 m_cnt;
    MUINT32 m_u4CaptureIdx;
    // AFAssist
    CCU_strAEOutput m_AFStateAESetting;
    MUINT32 m_u4AccCWValue;     // center weighting value
                                // v1.2
    MUINT32 m_u4DeltaWeight;
    MBOOL  m_bAENoChange;
    // disable flare related
    MUINT32 m_pu4FlareRGBHist[AE_SW_HISTOGRAM_BIN];
    CCU_AE_FLARE_STS    m_FlareSts;
    MUINT32 m_u4Prvflare;
    MUINT32 m_u4Capflare;
    MUINT32 m_u4SW_FlareValue;
    MINT32 m_i4FlareSTD;
    MUINT32 m_u4PreSumR;
    MUINT32 m_u4PreSumG;
    MUINT32 m_u4PreSumB;
    MUINT32 m_u4SumR;
    MUINT32 m_u4SumG;
    MUINT32 m_u4SumB;
    MUINT32 m_u4PreTotalCount[3];
    MINT32 m_i4FlareCountR;
    MINT32 m_i4FlareCountG;
    MINT32 m_i4FlareCountB;
    MINT32 m_i4FlareFlarePixelValue;
    MUINT32 m_u4STD_R;
    MUINT32 m_u4STD_G;
    MUINT32 m_u4STD_B;
    MUINT32 m_u4TotalCount[3];
    MUINT32 m_u4FlarePercent;
    MBOOL flare_disable;
    MUINT32 m_SmoothFlare ;
    MUINT32 m_PreCWV ;
    MUINT32 m_PreSW_FlareValue ;
    MUINT32 m_Cycle1CWV;
    MUINT32 m_Cycle2CWV;
    MUINT32 m_Cycle3CWV;
    MUINT32 m_Cycle1Ratio;
    MUINT32 m_Cycle2Ratio;
    MUINT32 m_CWVDiff ;
    MUINT32 m_SlowFlare ;
    MUINT32 m_Cyele0_FlareOffset ;
    MUINT32 m_Cyele0_Base ;
    MUINT32 m_PreFlare;
#endif
#ifdef WIN32
    int AAOY_INDEX;
    int AAORGB_INDEX;
    MUINT32 AAOY_LE[120 * 90];
    MUINT32 AAOY_SE[120 * 90];
    MUINT32 AAO_RGB[3][120 * 90];
    MUINT32 AAO_RecRGB[3][120 * 90];
#endif
} CCU_AeAlgo;
#endif

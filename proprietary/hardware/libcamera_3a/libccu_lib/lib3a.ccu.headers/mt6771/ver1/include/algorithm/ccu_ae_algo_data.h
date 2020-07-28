#ifndef __CCU_AE_ALGO_DATA_H__
#define __CCU_AE_ALGO_DATA_H__
//#include "ae_algo_ctrl.h"
#include "ccu_ae_param.h"
#include "ccu_awb_param.h"

#define AE_ALGO_REVISION     7474002
#define AE_ALGO_IF_REVISION   7471001

#define LANDMARK_MAX_FACE_NUM 1
#define LANDMARK_SMOOTH_MAX_LENGTH 3
#define NVRAM_ISP_REGS_ISO_SUPPORT_NUM 20
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
    MUINT32 OE_DeltaY;
    MUINT32 Prob_OE_P;

    // HS
    MUINT32 Hist_HS_LimitY;
    MUINT32 Hist_HS_LimitTHD;
    MINT32 Hist_HS_LimitTHDNormal;
    MINT32 Hist_HS_LimitTHDFlat;
    MUINT32 Hist_HS_TargetGain;
    MUINT32 Hist_HS_Target;

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

    MUINT32 Orig_HS_G_Ratio;
    MUINT32 Prob_HS_GC_Ratio;


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



////5 * 4 20bytes
//typedef enum
//{
//    CCU_AE_SENSOR_MAIN = 0,
//    CCU_AE_SENSOR_SUB,
//    CCU_AE_SENSOR_MAIN2,
//    CCU_AE_SENSOR_SUB2,
//    CCU_AE_SENSOR_MAX
//} CCU_AE_SENSOR_DEV_T;

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
    MUINT32 m_u4FaceFrameCnt;
    MUINT32 m_u4MeteringCnt;
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
    CCU_AE_FD_BLOCK_WINDOW_T m_AEFDWinBlock[CCU_MAX_AE_METER_AREAS];
    CCU_AE_BLOCK_WINDOW_T m_AEFDWinBlock_Scale[CCU_MAX_AE_METER_AREAS];
    MUINT32 m_u4FaceCentralYValue[CCU_MAX_AE_METER_AREAS];
    MUINT8  m_u4FaceNum;
    MUINT32 m_u4MaxFaceIdx;
    MINT32  m_i4IsTrueFaceCNN;
    MUINT32 m_u4TotalWeight;
} CCU_AE_FD_STS;

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

//#ifdef FLARE

typedef struct {
    MUINT16 PreAEidx;//=0;
    MUINT16 Pre_PreAEidx;//=0;
    MUINT16 nAEStableCount;//=0;
    MUINT16 FlareArr[16];//={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    MUINT16 FlareArrCount;//=0;
} CCU_AE_FLARE_STS;
//#endif

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
//****** members for AeAlgo ******
typedef struct
{

    MBOOL   m_bAEFinerEVComp;
    MUINT32 m_u4GauCWValue;     // center weighting value
    MUINT32 m_u4AccCWValue;     // center weighting value

    MUINT32 u4FullBrightest;    //histogram infomation

    MINT32  m_i4EvDiffFrac;
    MINT32  i4MaxBV;
    MINT32  i4MinBV;


    MUINT32 m_pWtTbl_W[5][5];    //AE weighting table
    // v1.2
    MUINT32 m_u4DeltaWeight;


    //MBOOL   m_AECmdSet_bRotateWeighting;
    //CCU_LIB3A_AE_SCENE_T    m_AECmdSet_eAEScene;     // change AE Pline
    //CCU_LIB3A_AE_EVCOMP_T   m_AECmdSet_eAEComp;
    // 128*4*3+5*5*4+15*15*4*5+14*4 = 6192bytes
    //MINT32  m_AECmdSet_i4SensorMode;
    

    //finerEV
    MUINT32 m_u4IndexF;
    MINT32  m_i4RealBV;

    MUINT32 m_pu4CentralYHist[AE_HISTOGRAM_BIN];
    MUINT32 m_pu4FullYHist[AE_HISTOGRAM_BIN];
    MUINT32 m_pu4FullRGBHist[AE_HISTOGRAM_BIN];
    MUINT32 m_pu4FlareRGBHist[AE_HISTOGRAM_BIN];
  
    MUINT32 m_u4CWValue;     // center weighting value
    MINT32  m_i4GammaIdx;   // next gamma idx
    MUINT32 m_pu4AEBlock[AE_BLOCK_NO][AE_BLOCK_NO];   // AE block value in Algorithm level
    MBOOL   m_bNolinearAccEn;



    //PSO Buffer Size
    MUINT32 m_u4MGRSepPSORGBsize;

    MINT32  m_i4AEDebugCtrl;

    //V4.0
    MBOOL m_bIsBlueSkyDet;
    MBOOL m_isBlueSky;
    MINT32 m_i4BlueStableCount;

    MUINT32 m_pu4AEOverExpoBlockCnt[AE_V4P0_BLOCK_NO][AE_V4P0_BLOCK_NO];
    MUINT32 m_pu4AEv4p0Block[AE_V4P0_BLOCK_NO][AE_V4P0_BLOCK_NO];   // AE v4p0 block value in Algorithm level
    MUINT32 m_pu4AEBlockR[AE_V4P0_BLOCK_NO][AE_V4P0_BLOCK_NO];
    MUINT32 m_pu4AEBlockG[AE_V4P0_BLOCK_NO][AE_V4P0_BLOCK_NO];
    MUINT32 m_pu4AEBlockB[AE_V4P0_BLOCK_NO][AE_V4P0_BLOCK_NO];



    MUINT32 m_pu4AELinearYBlock[AE_V4P0_BLOCK_NO][AE_V4P0_BLOCK_NO];
    MUINT32 m_pu4AELinearYMax;
    MUINT32 m_pu4AELinearGrBlock[AE_V4P0_BLOCK_NO][AE_V4P0_BLOCK_NO];
    MUINT32 m_pu4AELieanrSortedG[AE_V4P0_BLOCK_NO * AE_V4P0_BLOCK_NO];

    MBOOL  m_bAENoChange;
    MUINT32 m_u4WeightingSum;

    // v2.0
    CCU_AE_SCENE_STS    m_SceneCls;
    CCU_AWB_GAIN_T      m_AWBGains;
    CCU_AE_TARGET_STS   m_TargetSts;

    MUINT32 m_u4BacklightCWM;
    MUINT32 m_u4AntiOverExpCWM;
    MUINT32 m_u4HistoStretchCWM;


    MUINT32 m_u4LinearCWValue; //center weighting value of linear AAO
    MUINT32 m_u4AvgWValue;     // average weighting value
    MUINT32 m_u4MgrCWValue;    // center weighting value for mgr use
    MUINT32 m_u4CWRecommend; // recommend cw target (after backlight, anti-overxposure...)
    MUINT32 m_u4CentralY;            //Y value of central block
    MUINT32 m_u4AECondition;         // record backlight, hist stretch. over exp... condition , hit or not
    MINT32  m_i4RealBVx1000;

    
    MUINT32 m_u4MeterY;
    MINT32 m_i4FaceDiffIndex;
    MBOOL  m_bAEMeteringEnable;
    MBOOL m_bAEStable;

    

//#if defined(TOUCH) || defined(FACE)
    CCU_AE_FD_STS m_MeterFDSts;
//#endif
//#if defined(TOUCH)

    MBOOL m_bAEMeteringStable;
    MUINT32 m_u4MeterStableValue;
    MUINT32 m_u4MeteringYValue[CCU_MAX_AE_METER_AREAS];
    CCU_AE_TOUCH_BLOCK_WINDOW_T m_AETOUCHWinBlock[CCU_MAX_AE_METER_AREAS];
    CCU_AE_TOUCH_BLOCK_WINDOW_T m_PreAETOUCHWinBlock[CCU_MAX_AE_METER_AREAS]; //-1000~1000 domain
    MUINT8 m_u4MeteringCnt;
    MBOOL m_bisFirstTime;
//#endif
    MBOOL m_bTouchAEAreaChange;
//#if defined(FACE)
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
    MUINT32 m_u4FaceCaseNum;
    MUINT32 m_u4NormalTarget;
    MUINT32 m_u4FaceFinalTarget;
    MUINT32 m_u4FaceAEStable;
    MUINT32 m_u4FaceAEStableIndex;
    MUINT32 m_u4FaceAEStableCWV;
    MUINT32 m_u4FaceAEStableCWR;
    CCU_AE_BLOCK_WINDOW_T m_FaceAEStableBlock;
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
    MUINT32 m_i4DigitalZoomTGDomain_XOffset;
    MUINT32 m_i4DigitalZoomTGDomain_YOffset;
    MUINT32 m_i4DigitalZoomTGDomain_Width;
    MUINT32 m_i4DigitalZoomTGDomain_Height;
    MBOOL m_bFaceAELock;
    MINT32 m_i4CNNStablize;
    MINT32 m_i4CNNFaceFrameCnt;
    MINT32 m_i4GyroAcc[3];
    MINT32 m_i4GyroRot[3];
    MINT32 m_i4PrevGyroAcc[3];

//#endif
    MUINT32 m_bFaceAEAreaChange;

    /**
     * SMOOTH
     */
    MUINT32 m_i4PrvDeltaIndex;



    MINT32 m_i4PreAEDeltaIndex;
    CCU_AE_CMD_SET_T m_AECmdSet;
    MUINT32 m_u4StablePreOECnt;
    MUINT32 m_u4StablePreUECnt;
    MUINT32 m_u4StableOECnt;
    MUINT32 m_u4StableUECnt;
	MBOOL m_ResetFullFrameCycle;
	MBOOL m_bAENotPerFrameMode;
	MUINT32 m_u4AEFinerEVIdxBase;
	MINT32 m_i4AEFinerEVBase;
    MBOOL   m_bFracGainLimited;
	MINT32 m_i4FrameCnt;
    MINT32 m_i4ExtremeCond;
    MINT32 m_i4Cycle0FullDeltaIdx;
	CCU_AE_LINEAR_RESP  m_LinearResp;
    CCU_AE_PERFRAME_T   m_PerFrameInfo;
    MBOOL  m_bAEOneShot;
    MUINT32 m_u4CWRecommendStable; // recommend cw target (after backlight, anti-overxposure...)
	MUINT32 m_u4IndexFMax;
    MUINT32 m_u4IndexFMin;
    MUINT32 m_u4IndexMax;
    MUINT32 m_u4IndexMin;
    
    
    MINT32  m_i4RealStableBV;
    MINT32  m_i4EV;
    MINT32  m_i4BV;
    MINT32  m_i4AoeCompBV;
    CCU_strAEOutput m_OrigPrvAESetting;
    CCU_strAEOutput m_PrvStateAESetting;
    MINT32 m_i4IspGain;
    //Smooth
    MBOOL m_bTouchIsReferencedFlag;
    MUINT32 m_u4PreACWValue;
    MUINT32 m_u4PreAVGValue;
    MUINT32 m_u4EffectiveIndex;
    MUINT32 m_u4FracGain;
    
    MINT32  m_i4DiffEV;
    MINT32  m_i4deltaIndex; //current delta Index
    MINT32 m_i4AEIndexF[CCU_AE_SENSOR_MAX];
    MINT32 m_i4AEIndex[CCU_AE_SENSOR_MAX];
    MUINT32 m_u4Index;       // current AE index
    unsigned int totalCnt;
    CCU_strEvSetting        m_PreEvSettingQueue[MAX_AE_PRE_EVSETTING];
    //#ifdef FLARE
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
    //#endif
    MUINT32 m_i4VdoPrevTarget;
    MUINT32 m_u4PrevVdoTarget;
    MUINT32 m_u4AETarget;


    //----------------------------------
    //SYNC
    MUINT32 m_u4RGBY[4], m_u4RGB[3]; //BAYER
    MUINT32 m_u4W[3], m_u4AvgW;//MONO
    //----------------------------------

    MBOOL m_bCalculateAE;
    MBOOL m_bCalculateAEAP;
    MUINT32 m_u4AEScheduleCnt;
    MUINT32 m_u4AEExitStableCnt;
    MBOOL m_bPerFrameAEWorking;

    MUINT32 m_u4LockIndexF;
    MUINT32 m_u4LockIndex;
    MUINT32 m_i4EVLockIndexF;
    MUINT32 m_i4EVLockIndex;
    CCU_eAESTATE m_preAeState;
    MUINT32 m_u4CycleCnt;

    MUINT8 m_uoutstable;
    MINT32 m_i4DeltaCycleIndex;
    MINT32 m_cnt;

    MUINT32 m_pu4AEBlock_Exif[AE_BLOCK_NO][AE_BLOCK_NO];
    MUINT32 m_pu4AEv4p0Block_Exif[AE_V4P0_BLOCK_NO][AE_V4P0_BLOCK_NO];

    MUINT32 m_u4AAO_AWBValueWinSize8;  
    MUINT32 m_u4AAO_AWBValueSepSize8;  
    MUINT32 m_u4AAO_AWBValueWinSize16; 
    MUINT32 m_u4AAO_AWBValueSepSize16; 
    MUINT32 m_u4AAO_AWBSumWinSize8;    
    MUINT32 m_u4AAO_AWBSumSepSize8;    
    MUINT32 m_u4AAO_AWBSumWinSize16;   
    MUINT32 m_u4AAO_AWBSumSepSize16;   
    MUINT32 m_u4AAO_AEYWinSize8;     
    MUINT32 m_u4AAO_AEYSepSize8;     
    MUINT32 m_u4AAO_AEYWinSize16;    
    MUINT32 m_u4AAO_AEYSepSize16;    
    MUINT32 m_u4AAO_AEOverWinSize8;  
    MUINT32 m_u4AAO_AEOverSepSize8;  
    MUINT32 m_u4AAO_AEOverSepSize16; 
    MUINT32 m_u4PSO_SE_AWBWinSize16; 
    MUINT32 m_u4PSO_SE_AWBSepSize16; 
    MUINT32 m_u4PSO_LE_AWBWinSize16; 
    MUINT32 m_u4PSO_LE_AWBSepSize16; 
    MUINT32 m_u4PSO_SE_AEYWinSize16; 
    MUINT32 m_u4PSO_SE_AEYSepSize16; 
    MUINT32 m_u4PSO_LE_AEYWinSize16; 
    MUINT32 m_u4PSO_LE_AEYSepSize16; 
    MUINT32 m_u4AAO_LineSize8 ;
    MUINT32 m_u4AAO_LineSize16 ;
    MUINT32 m_u4PSO_LineSize16;
    MUINT32 m_u4PSO_ShiftBits;
    MUINT32 m_u4PSO_Rounding;
    MUINT32 m_u4AWB_Rounding;
    MUINT32 m_u4AWB_ShiftBits;
    MUINT32 m_u4Y_Rounding;
    MUINT32 m_u4Y_ShiftBits;

    MUINT32 m_u4PrevEVComp;

    MUINT16 u2LengthV2;
    MINT16 IDX_PartitionV2[NVRAM_ISP_REGS_ISO_SUPPORT_NUM];
    CCU_ISP_NVRAM_OBC_T OBC_TableV2[NVRAM_ISP_REGS_ISO_SUPPORT_NUM];
    CCU_ISP_NVRAM_OBC_T OBC_Table[4];

    

    MUINT32 m_u4CamId;


    MUINT32  m_bFaceAELCELinkEnable;
    MUINT32 m_u4RealFaceCWR;
    MUINT32 m_u4MaxGain;
    MUINT32 m_u4SERatio;
    MUINT32 m_u4preFDProbOE;


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

    MINT32 m_i4FDProbSize;
    MINT32 m_i4FDProbLoc;

    //put pointer members to the bottom of struct
    //avoiding 32/64 bit system offset difference
    CCU_strEvPline *m_pCurrentTable;
    CCU_strFinerEvPline *m_pCurrentTableF;
    CCU_strAEMovingRatio    *m_pCurMovingRatio;
    CCU_AE_NVRAM_T* pAeNVRAM;

} CCU_AeAlgo; // useful check ok

typedef struct 
{
    MUINT32 YStart, YEnd;
    MUINT8 *BufferInRoi;
    MUINT8 *PSOBufferInRoi;
}CCU_BankInfo;
#endif

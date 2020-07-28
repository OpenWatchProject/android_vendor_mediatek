#ifndef _CCU_AE_PARAM_H__
#define _CCU_AE_PARAM_H__

#include "ccu_ext_interface/ccu_types.h"
#include "algorithm/ccu_awb_param.h"
#include "algorithm/ccu_ae_nvram.h"
#include "algorithm/ccu_ae_feature.h"

#define MAX_AE_PRE_EVSETTING (3)
#define AE_ADB_RTT_TOTAL_CNT (5)


#define AE_SW_HISTOGRAM_BIN (256)
#define AE_SW_HISTOGRAM_BIN_BITS (8)
#define AE_BRIGHTNESS_STEP_OF_EACH_BIN_BITS (16-AE_SW_HISTOGRAM_BIN_BITS)
#define AE_HW_HISTOGRAM_BIN (256)
#define AE_VGAIN_UNIT         ((MUINT32)128)              //gain unit in  virtual-gain  , when calculate best CW Target
#define AE_METER_GAIN_BASE 1024
#define AE_BV_TARGET ((MUINT32)47)
#define AE_WIN_OFFSET          1000   // for android window define
#define AE_EV_COMP_MAX 81
#define FLARE_HISTOGRAM_BIN 40
#define FLARE_SCALE_UNIT (512) // 1.0 = 512
#define FLARE_OFFSET_DOMAIN (4095) // 12bit domain
#define AE_BV_TARGET ((MUINT32)47)

#define MAX_ISP_GAIN   (10*1024)
#define SENSTIVITY_UINT 1024
#define FD_Motion_FrameBuffer 10
#define FD_Motion_Direction 2
#define FD_SizeVar_FrameBuffer 10
#define FD_SizeVar_Direction 4
#define FD_Hist_FrameBuffer 5
#define AE_GAIN_BASE_ISP 4096
#define AE_GAIN_BASE_AFE 1024
#define FD_DR_MINY 15
#define HDR_RATIO_UNIT (1000)
//////////////////////////////
//  enum of AE condition
//////////////////////////////

enum
{
    CCU_AE_CONDITION_NORMAL           = 0x00,
    CCU_AE_CONDITION_BACKLIGHT        = 0x01,
    CCU_AE_CONDITION_OVEREXPOSURE     = 0x02,
    CCU_AE_CONDITION_HIST_STRETCH     = 0x04,
    CCU_AE_CONDITION_SATURATION_CHECK = 0x08,
    CCU_AE_CONDITION_FACEAE           = 0x10,
    CCU_AE_CONDITION_MAINTARGET       = 0x20,
    CCU_AE_CONDITION_HS_V4P0          = 0x40
};

typedef enum
{
    CCU_AE_LOG_NONE     = 0x00000000,
    CCU_AE_LOG_STAT     = 1 << 0,
    CCU_AE_LOG_FLOW     = 1 << 1,
    CCU_AE_LOG_METER    = 1 << 2,
    CCU_AE_LOG_FACE     = 1 << 3,
    CCU_AE_LOG_SMOOTH   = 1 << 4,
    CCU_AE_LOG_FULL     = 0xFFFFFFFF
} CCU_AE_LOG_LEVEL;

typedef enum
{
    CCU_AE_MODULE_NONE      = 0x00000000,
    CCU_AE_MODULE_LCE       = 1 << 0,
    CCU_AE_MODULE_LTM       = 1 << 0,
    CCU_AE_MODULE_CON_LINK  = 1 << 0,
    CCU_AE_MODULE_FULL      = 0xFFFFFFFF
} CCU_AE_MODULE_LEVEL;

typedef enum
{
    CCU_AE_RTT_NONE      = 0x00,
    CCU_AE_RTT_SET       = 0x01,
    CCU_AE_RTT_PRINT     = 0x02,
} CCU_AE_RTT_CMD;

typedef enum
{
    CCU_AE_RTT_ALGO_DATA    = 0x00,
    CCU_AE_RTT_INIT_DATA    = 0x01,
    CCU_AE_RTT_VSYNC_DATA   = 0x02,
} CCU_AE_RTT_DATA;

typedef enum
{
    CCU_AE_MODE_NORMAL,
    CCU_AE_MODE_IVHDR_TARGET,
    CCU_AE_MODE_MVHDR_TARGET,
    CCU_AE_MODE_ZVHDR_TARGET,
    CCU_AE_MODE_LE_FIX_TARGET,
    CCU_AE_MODE_SE_FIX_TARGET,
    CCU_AE_MODE_4CELL_MVHDR_TARGET
}CCU_AE_TargetMODE;

typedef enum
{
    CCU_ONCHANGE_NONE = 0,
    CCU_ONCHANGE_TRIGGER,
    CCU_ONCHANGE_COMPLETE
} CCU_ONCHANGE_STATE;

typedef enum
{
    CCU_POSTCAP_NONE = 0,
    CCU_POSTCAP_TRIGGER,
    CCU_POSTCAP_READY,
    CCU_POSTCAP_CALCULATE,
    CCU_POSTCAP_COMPLETE
} CCU_POSTCAP_STATE;


/* HDR-AE Sensor EXP Setting
   
*/
typedef struct {
    MUINT32 u4CurHdrRatio;  // HDR Ratio @ Current Frame
    MUINT32 u4AECHdrRatio;  // HDR Ratio calculated by AE algo.
    MINT32  i4LEExpo;
    MINT32  i4LEAfeGain;
    MINT32  i4LEIspGain;
    MINT32  i4MEExpo;
    MINT32  i4MEAfeGain;
    MINT32  i4MEIspGain;
    MINT32  i4SEExpo;
    MINT32  i4SEAfeGain;
    MINT32  i4SEIspGain;
    MINT32  i4VSEExpo;
    MINT32  i4VSEAfeGain;
    MINT32  i4VSEIspGain;
} CCU_strHdrEvSetting;

/* HDR-AE Sensor ATR Setting
   remain 16 MUINT32s as reserved output data.
*/
typedef struct {
    MUINT32  u4HdrATRReserved00;
    MUINT32  u4HdrATRReserved01;
    MUINT32  u4HdrATRReserved02;
    MUINT32  u4HdrATRReserved03;
    MUINT32  u4HdrATRReserved04;
    MUINT32  u4HdrATRReserved05;
    MUINT32  u4HdrATRReserved06;
    MUINT32  u4HdrATRReserved07;
    MUINT32  u4HdrATRReserved08;
    MUINT32  u4HdrATRReserved09;
    MUINT32  u4HdrATRReserved10;
    MUINT32  u4HdrATRReserved11;
    MUINT32  u4HdrATRReserved12;
    MUINT32  u4HdrATRReserved13;
    MUINT32  u4HdrATRReserved14;
    MUINT32  u4HdrATRReserved15;
} CCU_strHdrATRSetting;

typedef enum
{
    CCU_AE_SENSOR_MAIN = 0,
    CCU_AE_SENSOR_SUB,
    CCU_AE_SENSOR_MAIN2,
    CCU_AE_SENSOR_SUB2,
    CCU_AE_SENSOR_MAX
} CCU_AE_SENSOR_DEV_T;

typedef struct
{
    MUINT32 u4XLow;
    MUINT32 u4XHi;
    MUINT32 u4YLow;
    MUINT32 u4YHi;
    MUINT32 u4Weight;
} CCU_AE_BLOCK_WINDOW_T;

typedef struct
{
    MINT32 i4XLow;
    MINT32 i4XHi;
    MINT32 i4YLow;
    MINT32 i4YHi;
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
    MINT32  i4BefExtXLow;
    MINT32  i4BefExtXHi;
    MINT32  i4BefExtYLow;
    MINT32  i4BefExtYHi;
} CCU_AE_FD_BLOCK_WINDOW_T;

typedef struct
{
    MINT32 i4Left;
    MINT32 i4Right;
    MINT32 i4Top;
    MINT32 i4Bottom;
    MUINT32 u4Weight;
    MINT32  i4Id;
    MINT32  i4Type; // 0:GFD, 1:LFD, 2:OT
    MINT32  i4Rop;
    MINT32  i4Motion[2];
    MINT32 i4Landmark[3][4]; // index 0: left eye, index 1: right eye, index 2:mouth
    MINT32 i4LandmarkCV;
    MBOOL   bLandMarkBullsEye;
    MUINT32 u4LandMarkLeft;
    MUINT32 u4LandMarkRight;
    MUINT32 u4LandMarkTop;
    MUINT32 u4LandMarkBottom;
    MINT32  i4LandMarkICSWeighting;
    MINT32  i4LandMarkRip;
    MINT32  i4LandMarkRop;
} CCU_AE_FD_BLOCK_WINDOW_ICS_T;

typedef struct
{
    MINT32 u4XLow;
    MINT32 u4XHi;
    MINT32 u4YLow;
    MINT32 u4YHi;
    MINT32 u4Weight;
} CCU_AE_TOUCH_BLOCK_WINDOW_T;

typedef struct  
{
    MINT32 i4Input;
    MINT32 i4Output;
    MUINT32 u4PreOrgIndex;
    MUINT32 u4PreIndex;
    MUINT32 u4OrgIndex;
}CCU_AE_LIMITOR_T;

typedef struct
{
    MINT32 i4R; // R gain
    MINT32 i4G; // G gain
    MINT32 i4B; // B gain
}CCU_PRE_GAIN1_T;

typedef enum
{
    CCU_PSO_OUTPUT_AFTER_OB = 0,
    CCU_PSO_OUTPUT_BEFORE_OB,
}CCU_PSO_PATH_SEL_ENUM;

typedef struct {
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
} CCU_AEMeterArea_T;

typedef struct 
{
    CCU_AEMeterArea_T rAreas[CCU_MAX_AE_METER_AREAS];
    MUINT32 u4Count;
} CCU_AEMeteringArea_T;

typedef struct
{
    CCU_AWB_GAIN_T awb_gain;
    MUINT32 awb_gain_Unit;
}CCU_AAO_PROC_INFO_T;

typedef struct
{
    MINT32 m_i4CycleVDNum;
    MINT32 m_i4ShutterDelayFrames;
    MINT32 m_i4GainDelayFrames;
    MINT32 m_i4IspGainDelayFrames;
    MINT32 m_i4FrameCnt;
    MINT32 m_i4FrameTgCnt; // doPvAE_TG_INT update
    MUINT32 m_u4HwMagicNum; // doPvAE update
}CCU_strAECycleInfo;

// AE Input/Output Structure
typedef enum
{
    AE_STATE_INITIAL = 0,
    AE_STATE_PREVIEW,
    AE_STATE_PRECAP,
    AE_STATE_AFAE,
    AE_STATE_POSTCAP,
    AE_STATE_MAXIMUM
} AE_STATE_ENUM;

typedef enum
{
    ALGO_MAIN_STATE_INIT = 0,
    ALGO_MAIN_STATE_PERFRAME,
    ALGO_MAIN_STATE_NON_PERFRAME,
    ALGO_MAIN_STATE_CAL_INFO,
    ALGO_MAIN_STATE_MAXIMUM
}CCU_ENUM_ALGO_MAIN_STATE;

typedef enum
{
    ALGO_SUB_STATE_INIT = 0,
    ALGO_SUB_STATE_ACTIVE,
    ALGO_SUB_STATE_SKIP,
    ALGO_SUB_STATE_STABLE,
    ALGO_SUB_STATE_MONITOR,
    ALGO_SUB_STATE_LOCK,
    ALGO_SUB_STATE_MAXIMUM
}CCU_ENUM_ALGO_SUB_STATE;

typedef struct
{
    MUINT32 u4CycleCnt;
    MUINT32 u4MagicNo;
    MUINT32 u4DelayCnt;
}CCU_CYCLE_CTRL;

typedef struct  
{
    AE_STATE_ENUM eAEState;
    MBOOL bIsAFLock;
    MBOOL bIsAPLock;
    MBOOL bIsCalculate;
}CCU_AE_CTRL;

typedef struct
{
    MBOOL bIsSlowMotion;
    MBOOL bIsLimitMode;
}CCU_FEATURE_CTRL;

typedef struct
{
    MUINT32 u4FrameIdx;
    CCU_strEvSetting xSetting;
    MVOID *pNext;
}CCU_EV_CTRL;

typedef struct
{
    CCU_EV_CTRL *pHead;
    CCU_EV_CTRL *pTail;
    CCU_EV_CTRL xEvQueue[MAX_AE_PRE_EVSETTING];
}CCU_EV_QUEUE_CTRL;

typedef struct
{
    CCU_AE_CTRL xAECtrlPre;
    CCU_ENUM_ALGO_MAIN_STATE eMainState;
    CCU_ENUM_ALGO_SUB_STATE eSubState;
    MUINT32 u4CurrFrameCnt;
    MUINT32 u4WaitFrameCnt;
    MUINT32 u4SkipFrameList[MAX_AE_PRE_EVSETTING];
    MUINT32 u4RTTRestoreCnt;
    MUINT32 u4RTTRestoreCmd[AE_ADB_RTT_TOTAL_CNT];
    MUINT32 u4RTTRestoreValue[AE_ADB_RTT_TOTAL_CNT];
    MUINT32 u4LogLevel;
    MUINT32 u4ModuleCtrl;
}CCU_ALGO_CTRL;

typedef enum
{
    AE_TICKCNT_INITIAL = 0,
    AE_TICKCNT_STATISTIC,
    AE_TICKCNT_METERING,
    AE_TICKCNT_SOMMTH,
    AE_TICKCNT_MAX
}CCU_ENUM_AE_TICKCNT;

typedef struct
{
    void*               pAESatisticBuffer;
    //CCU_strEvSetting        PreEvSetting[MAX_AE_PRE_EVSETTING];
    CCU_strAECycleInfo      CycleInfo;
    MINT32 pCycleInfo_i4ShutterDelayFrames;
    MINT32 pCycleInfo_i4GainDelayFrames;
    MINT32 pCycleInfo_i4IspGainDelayFrames;
    MINT32 pCycleInfo_i4FrameCnt;
    CCU_ENUM_ALGO_MAIN_STATE            eAeState;   //ae state
} CCU_strAEInput;

typedef enum
{
    CCU_SYNC_AE_DUAL_CAM_DENOISE_BMDN = 0,
    CCU_SYNC_AE_DUAL_CAM_DENOISE_MFNR,
    CCU_SYNC_AE_DUAL_CAM_DENOISE_MAX
} CCU_SYNC_AE_DUAL_CAM_DENOISE_ENUM;

typedef struct
{

    MINT32 i4SyncType;
    MBOOL bIsMapping;
    MBOOL bIsAfterAeSync;
    MUINT32 u4MasterID;
    MUINT32 u4MasterIndex;
    MUINT32 u4MasterPureAeCWR;
    MUINT32 u4MasterLinkedCWR;
    MUINT32 u4MasterSpeedRatio;
    MBOOL bIsMasterStable;
    MBOOL bIsMasterAeLock;
    MUINT32 bEnButterflyEffect;
    MUINT32 u4ButterflyTH;

} CCU_SYNC2SINGLE_REPORT;


typedef struct
{
    CCU_strEvSetting     EvSetting;
    CCU_strHdrEvSetting  HdrEvSetting;
    CCU_strHdrATRSetting HdrATRSetting;
    MINT32               Bv;
    MINT32               AoeCompBv;
    MINT32               i4EV;
    MUINT32              u4ISO;          //correspoing ISO , only use in capture
    MINT16               i2FaceDiffIndex;
    MUINT32              u4Index;
    MUINT32              u4IndexF;
    MUINT32              u4FracGain;    // stepless gain, lower than 0.1EV
    MBOOL                bAEStable;      // Only used in Preview/Movie
    MUINT32              u4CWValue;
    MUINT32              u4AECondition;
    MINT32               i4DeltaBV;
    MINT32               i4PreDeltaBV;
    MUINT32              u4DeltaBVRatio[CCU_SYNC_AE_DUAL_CAM_DENOISE_MAX];
    MUINT16              u2FrameRate;     // Calculate the frame
    MUINT32              u4BayerY;
    MUINT32              u4MonoY;
    MINT32               i4RealBVx1000;
    MINT32               i4RealBV;
    MUINT32              u4CWRecommendStable;
    MUINT32              u4AEFinerEVIdxBase;
    MUINT32              u4FaceFailCnt;
    MUINT32              u4FaceFoundCnt;
    MUINT32              u4FaceAEStable;
    MINT32               i4cHdrSEDeltaEV;
    MINT32               i4AEidxNext;
} CCU_strAEOutput;

typedef struct
{
    CCU_AE_TargetMODE m_AETargetMode;
    MUINT32 u4ExposureMode; // 0: exposure time, 1: exposure line
    CCU_strEvSetting EvSetting;
    CCU_strHdrEvSetting HdrEvSetting;
    CCU_strHdrATRSetting HdrATRSetting;
    MUINT32 u4ISO;
    MUINT16 u2FrameRate;
    MUINT32 u4Index;
    MUINT32 u4IndexF;
    MUINT32 u4AEFinerEVIdxBase;
    MBOOL bResetIndex;
} strAERealSetting;

typedef struct
{
    MUINT32 u4BinningSumRatio;
    strAERealSetting *pRealSetting;
    CCU_AE_NVRAM_T *pAeNVRAM;
} strBinningSumTrans;


//AAA_OUTPUT_PARAM_T use strAEOutput
typedef struct
{
    MUINT32 u4ExposureMode;  // 0: exposure time, 1: exposure line
    MUINT32 u4Eposuretime;   //!<: Exposure time inc ms
    MUINT32 u4AfeGain;       //!<: sensor gain
    MUINT32 u4IspGain;       //!<: raw gain
    MUINT16 u2FrameRate;
    MUINT16 u4CWValue;      // for N3D sync
    MUINT16 u4AvgY;             // for N3D sync
    MUINT32 u4RealISO;      //!<: ISO speed
    MINT16   i2FlareOffset;
    MINT16   i2FlareGain;   // 512 is 1x
    MINT32  u4AEFinerEVIdxBase; //finer ev idx
}CCU_AE_MODE_CFG_T;

typedef struct
{
    MUINT32 u4AEIndex;
    MUINT32 u4FRameRate_x10;            // 10 base frame rate
    MUINT32 u4PreviewShutterSpeed_us;   // micro second
    MUINT32 u4PreviewSensorGain_x1024;  // 1024 base
    MUINT32 u4PreviewISPGain_x1024;     // 1024 base
    MUINT32 u4RealISOValue;
    MUINT32 u4CapShutterSpeed_us;       // micro second
    MUINT32 u4CapSensorGain_x1024;      // 1024 base
    MUINT32 u4CapISPGain_x1024;         // 1024 base
    MINT32  i4BrightValue_x10;          // 10 base brightness value
    MINT32  i4ExposureValue_x10;        // 10 base exposure value
    MINT32  i4LightValue_x10;           // 10 base lumince value
    MINT16  i2FlareOffset;              // 12 bit domain
    MINT16  i2FlareGain;                // 9 base gain
}CCU_FrameOutputParam_T;

typedef struct
{
    MUINT32 u4Id;
    MBOOL   bGainAlign;
    MUINT32 u4TargetGain_x1024;
    MUINT32 u4EV0ISPGain_x1024;
    MUINT32 u4EV0SensorGain_x1024;
    MUINT32 u4EV0Exptime_us;
    MUINT32 u4ExptimeLimit_us;
    MUINT32 u4GainLimit_x1024;
    MUINT32 u4FrameIndex;
    MUINT32 u4EndFrameIndex;
    MUINT32 uHintForAE;
    MFLOAT uCurrRatio;
}CCU_AE_CUST_Super_Night_Param_T;

/* HDR-AE Related Structure Start
 *
 */
/* HDR-AE MHDR Sensor Stats Data Structure[ALGO. INPUT]
   bSensorStatsExist : Identify the status of sensor AE-Stats
   u1MHDRStatsConfig : Identify every Stat exist or not
   a_pMHDRSensorStatsBuffer : Pointer to sensor AE-Stats Buffer
*/
typedef struct {
    MBOOL        bSensorStatsExist;
    MUINT8       u1MHDRStatsConfig;
    MUINT32      *a_pMHDRSensorStatsBuffer;
} CCU_AE_MHDR_STATS;

/* HDR-AE AEAlgo. Data Start
 *
 */

/* HDR-AE Params for 2EXPs Start
 *
 */
typedef struct {
    MBOOL  bAOECurrState;
    MBOOL  bAOEPrvState;
    MINT32 i4PrvAOECW;
    MINT32 i4SmoothStep;
} CCU_strAoeSmoothInfo;

typedef struct {
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
} CCU_strHistInfo;
/* HDR-AE Params for 2EXPs End
 *
 */
 
typedef enum {
    CCU_MVHDR_STATE_MOVING = 0,
    CCU_MVHDR_STATE_STABLE
}CCU_eMVHDR_STATE;

typedef struct {
    MUINT32 u4MHDRTarget;
    MUINT32 u4MHDRBVTarget;
} CCU_HDR_TARGET_INFO_T;

typedef struct {
    MUINT32 u4MHDRLEBlock[AE_BLOCK_NO][AE_BLOCK_NO];
    MUINT32 u4MHDRSEBlock[AE_BLOCK_NO][AE_BLOCK_NO];
    MUINT32 u4MHDRLEYHist[256];
    MUINT32 u4MHDRSEYHist[256];
    MUINT32 u4MHDRLEYHistTotalCount;
    MUINT32 u4MHDRSEYHistTotalCount;
    MINT32  i4MHDRBV;
    MUINT32 u4MHDRCWValue;
    MUINT32 u4MHDRFDY;
    MUINT32 u4MHDROECWValue;
    MINT32  i4AOEBvCompRatio;
    CCU_HDR_TARGET_INFO_T sHdrTargetInfo;
    CCU_strAoeSmoothInfo sAOESmoothInfo;
    CCU_strHistInfo      sHistInfo;
} CCU_MHDR_AE_INFO_T;

typedef struct {
    MUINT32 u4targetHDRRatio;
    MUINT32 u4curHDRRatio;
    MUINT32 u4preHDRRatio;
    MUINT32 u4realStatHDRRatio;
    MUINT32 u4manualHDRRatio;
    MUINT32 u4maxHDRRatio;
    MINT32 i4OE1BinRatio;
    MINT32 i4OE2BinRatio;
} CCU_HDR_RATIO_INFO_T;

typedef struct {
    MUINT32 eHDRState;
    MINT32  i4HdrRatioStableCount;
} CCU_HDR_RATIO_SMOOTH_INFO_T;

typedef struct {
    MINT32 i4MultiFacePrior;
    MUINT32 uFaceYLowBound;
    MUINT32 uFaceYHighBound;
    MINT32 i4FaceBVLowBound;
    MINT32 i4FaceBVHighBound;
    MINT32 i4BV4HDRFDTarget[4];
    MUINT32 u4HDRFDTarget[4];
    MINT32 i4OE1Pct[5];
    MINT32 i4RatioMovOE1[5];
    MINT32 i4OE2Pct[5];
    MINT32 i4RatioMovOE2[5];
    MINT32 i4HDRRatioInTolerance;
    MINT32 i4HDRRatioOutTolerance;
} CCU_HDR_PARA_INFO_T;

typedef struct {
    CCU_MHDR_AE_INFO_T          sMHDRAEInfo;
    CCU_HDR_RATIO_INFO_T        sHDRRatioInfo;
    CCU_HDR_RATIO_SMOOTH_INFO_T sHDRRatioSmoothInfo;
    CCU_strHdrEvSetting         sCurHDREvSetting;
    CCU_strHdrEvSetting         sPreHDREvSettingQueue[MAX_AE_PRE_EVSETTING]; // [Cur | 0 | 1 | 2 | ...]
    CCU_HDR_PARA_INFO_T         sHDRParaInfo;
} CCU_HDR_AE_INFO_T;
/* HDR-AE AEAlgo. Data End
 *
 */

// Camera Scenario
typedef enum
{
    CCU_CAM_SCENARIO_PREVIEW = 0,  // PREVIEW
    CCU_CAM_SCENARIO_VIDEO,        // VIDEO
    CCU_CAM_SCENARIO_CAPTURE,      // CAPTURE
    CCU_CAM_SCENARIO_CUSTOM1,      // HDR
    CCU_CAM_SCENARIO_CUSTOM2,      // AUTO HDR
    CCU_CAM_SCENARIO_CUSTOM3,      // VT
    CCU_CAM_SCENARIO_CUSTOM4,      // STEREO
    CCU_CAM_SCENARIO_NUM
} CCU_CAM_SCENARIO_T;


typedef struct
{
    uint32_t size;
    uint32_t x[10];
}AE_LTM_MID_PERC_LUT;

typedef struct
{
    uint32_t x[2];
    uint32_t ratio[2];
    uint32_t tp_ratio[14];
}AE_LTM_LOCAL_GLOBAL_BLENDING;

typedef struct
{
    MUINT32 u4ClipThdPerc;
    MUINT32 HighBoundPerc;
    AE_LTM_MID_PERC_LUT MidPercLut;
    MUINT32 u4StrengthLtpMeanIdx[2];
    MUINT32 u4StrengthLtp[2][14];
    AE_LTM_LOCAL_GLOBAL_BLENDING LocalGlobalBlending;
}AE_LTM_NVRAM_PARAM;

typedef struct
{
    MUINT32 u4XOffset;
    MUINT32 u4XWidth;
    MUINT32 u4YOffset;
    MUINT32 u4YHeight;
} CCU_AE_ZOOM_WIN_INFO;

typedef enum
{
    CCU_AE_PRIORITY_OFF = 0,
    CCU_AE_PRIORITY_ISO,
    CCU_AE_PRIORITY_SHUTTER
} CCU_AE_PRIORITY_MODE;

typedef struct
{
    CCU_AE_PRIORITY_MODE eMode;
    MUINT32 u4FixShutter;
    MUINT32 u4FixISO;
    MUINT32 u4MinShutter;
    MUINT32 u4MaxShutter;
    MUINT32 u4MaxISO;
    MUINT32 u4MinISO;
} CCU_AE_PRIORITY_INFO;

#endif


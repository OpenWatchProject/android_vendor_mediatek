#ifndef _CCU_AE_PARAM_H__
#define _CCU_AE_PARAM_H__

#include "ccu_ext_interface/ccu_types.h"
#include "algorithm/ccu_awb_param.h"
#include "algorithm/ccu_ae_nvram.h"
#include "algorithm/ccu_ae_feature.h"
#define AE_V4P0_BLOCK_NO  15
#define MAX_AE_PRE_EVSETTING 3
#define CCU_MAX_AE_METER_AREAS  9
#define MAX_WEIGHT_TABLE 4
#define AE_BLOCK_NO  5
#define AE_RATIOTBL_MAXSIZE 8 
#define AE_HISTOGRAM_BIN (128)
#define AE_HW_HISTOGRAM_BIN 256
#define AE_VGAIN_UNIT         ((MUINT32)128)              //gain unit in  virtual-gain  , when calculate best CW Target
#define AE_METER_GAIN_BASE 1024
#define AE_BV_TARGET ((MUINT32)47)
#define AE_WIN_OFFSET          1000   // for android window define
#define AE_EV_COMP_MAX 81
#define FLARE_HISTOGRAM_BIN 40
#define FLARE_SCALE_UNIT (512) // 1.0 = 512
#define FLARE_OFFSET_DOMAIN (4095) // 12bit domain
#define AE_BV_TARGET ((MUINT32)47)
#define MAX_PLINE_MAP_TABLE 30
#define MOVING_TABLE_SIZE 24
//////////////////////////////
//  enum of AE condition
//////////////////////////////
// useful check ok
// 8*4 = 32bytes
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
//27*4 = 108bytes
typedef enum
{
    CCU_LIB3A_AE_SCENE_UNSUPPORTED    = -1,
    CCU_LIB3A_AE_SCENE_OFF            = 0,
    CCU_LIB3A_AE_SCENE_AUTO           = 1,
    CCU_LIB3A_AE_SCENE_NIGHT          = 2,
    CCU_LIB3A_AE_SCENE_ACTION         = 3,
    CCU_LIB3A_AE_SCENE_BEACH          = 4,
    CCU_LIB3A_AE_SCENE_CANDLELIGHT    = 5,
    CCU_LIB3A_AE_SCENE_FIREWORKS      = 6,
    CCU_LIB3A_AE_SCENE_LANDSCAPE      = 7,
    CCU_LIB3A_AE_SCENE_PORTRAIT       = 8,
    CCU_LIB3A_AE_SCENE_NIGHT_PORTRAIT = 9,
    CCU_LIB3A_AE_SCENE_PARTY          = 10,
    CCU_LIB3A_AE_SCENE_SNOW           = 11,
    CCU_LIB3A_AE_SCENE_SPORTS         = 12,
    CCU_LIB3A_AE_SCENE_STEADYPHOTO    = 13,
    CCU_LIB3A_AE_SCENE_SUNSET         = 14,
    CCU_LIB3A_AE_SCENE_THEATRE        = 15,
    CCU_LIB3A_AE_SCENE_ISO_ANTI_SHAKE = 16,
    CCU_LIB3A_AE_SCENE_BACKLIGHT      = 17,
    CCU_LIB3A_AE_SCENE_ISO100         = 100,
    CCU_LIB3A_AE_SCENE_ISO200         = 101,
    CCU_LIB3A_AE_SCENE_ISO400         = 102,
    CCU_LIB3A_AE_SCENE_ISO800         = 103,
    CCU_LIB3A_AE_SCENE_ISO1600        = 104,
    CCU_LIB3A_AE_SCENE_ISO3200        = 105,
    CCU_LIB3A_AE_SCENE_ISO6400        = 106,
    CCU_LIB3A_AE_SCENE_ISO12800       = 107,
} CCU_LIB3A_AE_SCENE_T;




//3*4+3 = 15
typedef struct

{
    MUINT32 u4Eposuretime;
    MUINT32 u4AfeGain;
    MUINT32 u4IspGain;
    MUINT8  uIris;
    MUINT8  uSensorMode;
    MUINT8  uFlag;
}CCU_strEvSetting;

typedef struct
{
    CCU_strEvSetting sPlineTable[200];   // Pline structure table
} CCU_strEvPline;
typedef struct
{
   CCU_strEvSetting sPlineTable[2000];   // Finer EV Pline structure table
} CCU_strFinerEvPline;

// AE ISO speed
// 17*4 = 68bytes
typedef enum
{
    CCU_LIB3A_AE_ISO_SPEED_UNSUPPORTED          =     -1,
    CCU_LIB3A_AE_ISO_SPEED_AUTO                 =      0,
    CCU_LIB3A_AE_ISO_SPEED_50                   =     50,
    CCU_LIB3A_AE_ISO_SPEED_100                  =    100,
    CCU_LIB3A_AE_ISO_SPEED_150                  =    150,
    CCU_LIB3A_AE_ISO_SPEED_200                  =    200,
    CCU_LIB3A_AE_ISO_SPEED_300                  =    300,
    CCU_LIB3A_AE_ISO_SPEED_400                  =    400,
    CCU_LIB3A_AE_ISO_SPEED_600                  =    600,
    CCU_LIB3A_AE_ISO_SPEED_800                  =    800,
    CCU_LIB3A_AE_ISO_SPEED_1200                 =   1200,
    CCU_LIB3A_AE_ISO_SPEED_1600                 =   1600,
    CCU_LIB3A_AE_ISO_SPEED_2400                 =   2400,
    CCU_LIB3A_AE_ISO_SPEED_3200                 =   3200,
    CCU_LIB3A_AE_ISO_SPEED_6400                 =   6400,
    CCU_LIB3A_AE_ISO_SPEED_12800                =   12800,
    CCU_LIB3A_AE_ISO_SPEED_MAX = CCU_LIB3A_AE_ISO_SPEED_12800,
}CCU_LIB3A_AE_ISO_SPEED_T;


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
    MBOOL   bEnableFlareFastConverge;        // enable fast flare converge
    MBOOL   bEnableRAFastConverge;           // enable fast RA converge
    MBOOL   bEnableFaceFastConverge;         // enable fast face converge
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
    MBOOL bNewConstraintForMeteringEnable; // Temp solution, it will be removed @ ISP5
}CCU_strAEParamCFG;


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
    MUINT32 u4XLow; //change type
    MUINT32 u4XHi; //change type
    MUINT32 u4YLow; //change type
    MUINT32 u4YHi; //change type
    MUINT32 u4Weight; //change type
} CCU_AE_BLOCK_WINDOW_T; // useful check ok 

//24*4 = 96bytes
typedef struct
{
    MINT32 i4XLow; 
    MINT32 i4XHi; 
    MINT32 i4YLow; 
    MINT32 i4YHi; 
    MUINT32 u4Weight; 
    MINT32  i4Id; //change type
    MINT32  i4Type; // 0:GFD, 1:LFD, 2:OT //change type
    MINT32  i4Rop; //change type
    MINT32  i4Motion[2]; //change type
    MINT32  i4Landmark_XLow[3]; // index 0: left eye, index 1: right eye, index 2:mouth //change type
    MINT32  i4Landmark_XHi[3]; // change type
    MINT32  i4Landmark_YLow[3]; // change type
    MINT32  i4Landmark_YHi[3]; // change type
    MINT32  i4LandmarkCV;
    MBOOL   bLandMarkBullsEye;
    MUINT32 u4LandMarkXLow;
    MUINT32 u4LandMarkXHi;
    MUINT32 u4LandMarkYLow;
    MUINT32 u4LandMarkYHi;
    MINT32  i4LandMarkICSWeighting;
    MINT32  i4LandMarkRip;
    MINT32  i4LandMarkRop;
} CCU_AE_FD_BLOCK_WINDOW_T;

//24*4 = 96bytes
typedef struct
{
    MINT32 i4Left; 
    MINT32 i4Right; 
    MINT32 i4Top; 
    MINT32 i4Bottom; 
    MUINT32 u4Weight; 
    MINT32  i4Id; //change type
    MINT32  i4Type; // 0:GFD, 1:LFD, 2:OT //change type
    MINT32  i4Rop; //change type
    MINT32  i4Motion[2]; //change type
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
      // pinput->mFrameData.m_AEFDWinBlock[i].i4Landmark_Left[0] = (MUINT8)m_eAEFDArea.rAreas[i].i4Landmark[0][0];
      // pinput->mFrameData.m_AEFDWinBlock[i].i4Landmark_Left[1] = (MUINT8)m_eAEFDArea.rAreas[i].i4Landmark[1][0];
      // pinput->mFrameData.m_AEFDWinBlock[i].i4Landmark_Left[2] = (MUINT8)m_eAEFDArea.rAreas[i].i4Landmark[2][0];
      // pinput->mFrameData.m_AEFDWinBlock[i].i4Landmark_Right[0] = (MUINT8)m_eAEFDArea.rAreas[i].i4Landmark[0][1];
      // pinput->mFrameData.m_AEFDWinBlock[i].i4Landmark_Right[1] = (MUINT8)m_eAEFDArea.rAreas[i].i4Landmark[1][1];
      // pinput->mFrameData.m_AEFDWinBlock[i].i4Landmark_Right[2] = (MUINT8)m_eAEFDArea.rAreas[i].i4Landmark[2][1];
      // pinput->mFrameData.m_AEFDWinBlock[i].i4Landmark_Top[0] = (MUINT8)m_eAEFDArea.rAreas[i].i4Landmark[0][2];
      // pinput->mFrameData.m_AEFDWinBlock[i].i4Landmark_Top[1] = (MUINT8)m_eAEFDArea.rAreas[i].i4Landmark[1][2];
      // pinput->mFrameData.m_AEFDWinBlock[i].i4Landmark_Top[2] = (MUINT8)m_eAEFDArea.rAreas[i].i4Landmark[2][2];
      // pinput->mFrameData.m_AEFDWinBlock[i].i4Landmark_Bottom[0] = (MUINT8)m_eAEFDArea.rAreas[i].i4Landmark[0][3];
      // pinput->mFrameData.m_AEFDWinBlock[i].i4Landmark_Bottom[1] = (MUINT8)m_eAEFDArea.rAreas[i].i4Landmark[1][3];
      // pinput->mFrameData.m_AEFDWinBlock[i].i4Landmark_Bottom[2] = (MUINT8)m_eAEFDArea.rAreas[i].i4Landmark[2][3];
} CCU_AE_FD_BLOCK_WINDOW_ICS_T;
//5*4 = 20bytes
typedef struct
{
    MINT32 u4XLow;   
    MINT32 u4XHi;    
    MINT32 u4YLow;   
    MINT32 u4YHi;     
    MINT32 u4Weight; 
} CCU_AE_TOUCH_BLOCK_WINDOW_T;

typedef struct {
    MBOOL bEnableABLabsDiff;                                // enable enhance ABL(absDiff)
    MINT32  u4EVDiffRatio_X[2];
    MINT32  u4EVDiffRatio_Y[2];
    MINT32  u4BVRatio_X[2]; //Ask type
    MINT32  u4BVRatio_Y[2]; //Ask type
} CCU_strABL_absDiff; // useful check ok

typedef struct {
    MBOOL   bEnableFlatSkyEnhance;                                // enable HS adaptive THD by realBV
    MUINT8  uSkyDetInThd;
    MUINT8  uSkyDetOutThd;
    MUINT32 u4FlatSkyTHD; //
    MUINT32 u4FlatSkyEVD; //change type
    MINT32  u4BVRatio_X[2];//Ask type
    MINT32  u4BVRatio_Y[2];//Ask type
} CCU_strHSFlatSkyCFG; // useful check ok

typedef struct
{
    MBOOL   bEnableMultiStepHS;         //Enable HS enhance method : MultiStep HS(Conflict with ori HS)
    MUINT32 u4TblLength;                                //Max size : 16 //change type
    MUINT32 u4MS_EVDiff[AE_RATIOTBL_MAXSIZE];            //size must large than u4TblLength
    MUINT32 u4MS_OutdoorTHD[AE_RATIOTBL_MAXSIZE];        //size must large than u4TblLength 
    MUINT32 u4MS_IndoorTHD[AE_RATIOTBL_MAXSIZE];         //size must large than u4TblLength 
    MINT32  i4MS_BVRatio[2];                      //MS_HS In/Out door BV threshold
} CCU_strHSMultiStep; // useful check ok

typedef struct {
    MBOOL    bNonCWRAccEnable; 
    MUINT32 u4HS_FHY_HBound; //change type
    MUINT32 u4CWV_HBound; //change type
    MUINT32 u4HS_TargetHBound; //change type
    MUINT32 u4HS_ACCTarget; //change type
} CCU_strNonCWRAcc; //useful check ok

typedef struct {
    MBOOL bEnableOverExpoAOE;
    MBOOL bEnableHistOverExpoCnt;
    MUINT32 u4OElevel;
    MUINT32 u4OERatio_LowBnd;
    MUINT32 u4OERatio_HighBnd;
    MINT32 i4OE_BVRatio_X[2];
    MUINT32 u4OE_BVRatio_Y[2];
    MUINT8 u4OEWeight[AE_V4P0_BLOCK_NO][AE_V4P0_BLOCK_NO];
} CCU_strOverExpoAOECFG; // useful check ok

typedef struct {
    MBOOL bEnableAEMainTarget;
    MUINT32 u4MainTargetWeight;     //weight 1024 base
    MUINT8 u4MainTargetWeightTbl[AE_V4P0_BLOCK_NO][AE_V4P0_BLOCK_NO];  //15x15 Gau weight table
    CCU_AE_TARGET_PROB_T TargetBVRatioTbl;
    MBOOL bEnableColorWTRatio;
    CCU_AE_TARGET_PROB_T ColorWTRatioTbl;
    MUINT8 uColorSupRatioR;
    MUINT8 uColorSupRatioG;
    MUINT8 uColorSupRatioB;
    CCU_AE_TARGET_PROB_T ColorSupBVRatioTbl;
} CCU_strMainTargetCFG; // useful check ok

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
    CCU_AE_TARGET_PROB_T DynWTRatioTbl;

    
    MBOOL bEnableHsGreenSupress;    // Enable flag  for HS green supress
    MUINT32 u4HsGreenGRRatio;       //  HS green G/R Ratio
    MUINT32 u4HsGreenGBRatio;       //  HS green G/B Ratio
    MUINT32 u4HsSupRatio;           // HS green surpess stregth ratio
    MUINT32 u4HS_GCountRatio_X[2];  // HS green count weighting TBL X
    MUINT32 u4HS_GCountRatio_Y[2];  // HS green count weighting TBL Y
    
} CCU_strHSv4p0CFG; // useful check ok

typedef struct{
    MBOOL   bEnableNSBVCFG;
    MUINT32 u4TblLength;
    MINT32  i4BV[AE_RATIOTBL_MAXSIZE];
    MUINT32 u4BTTHD[AE_RATIOTBL_MAXSIZE];
    MUINT32 u4NSTHD[AE_RATIOTBL_MAXSIZE];
} CCU_strNSBVCFG; // useful check ok

typedef struct {
    MBOOL   bEnableAEHsTS;                   // enable AE HS Target temp smooth
    MBOOL   bEnableAEAblTS;                  // enable AE ABL Target temp smooth
    MBOOL   bEnableAEAoeTS;                  // enable AE AOE Target temp smooth
    MBOOL   bEnableAENsTS;                   // enable AE NS Target temp smooth
    MBOOL   bEnableAEFaceTS;                 // enable AE Face Target temp smooth
    MBOOL   bEnableAEVideoTS;                // enable AE Video Target temp smooth
    MBOOL   bEnableAEMainTargetTS;                // enable AE Main Target temp smooth
    MBOOL   bEnableAEHsV4p0TargetTS;                // enable AE HSV4p0 Target temp smooth
} CCU_strAECWRTempSmooth; // useful ckeck ok

typedef struct {
    MUINT32 u4MinFDYLowBound;               // face boundary min Y value //change type
    MUINT32 u4MaxFDYHighBound;              // face boundary max Y value //change type
    MINT32  i4FaceBVLowBound;               // face boundary min BV value
    MINT32  i4FaceBVHighBound;              // face boundary max BV value
    MUINT32 u4TmpFDY_HBound;                // tempFDY HighBound
    MUINT32 u4TmpFDY_LBound;                // tempFDY LowBound
    MUINT32 u4FD_Lost_MaxCnt;               // face lost max count //change type
    MUINT32 u4FD_Scale_Ratio;               // face window scale ratio
    MUINT32 u4FaceNSLowBound;       //NS face target Y //change type
    MUINT32 u4FD_Lock_MaxCnt;               //change type
    MUINT32 u4FD_TemporalSmooth;               //change type
    MUINT32 u4FD_FaceTopCropRat;
    MUINT32 u4FD_FaceBottomCropRat;
    MUINT32 u4FD_FaceLeftCropRat;
    MUINT32 u4FD_FaceRightCropRat;
    MUINT32 u4FD_InToOutThdMaxCnt;               //change type
    MUINT32 u4FD_OTLockUpperBnd;
    MUINT32 u4FD_OTLockLowerBnd;
    MUINT32 u4FD_ContinueTrustCnt;               //change type
    MUINT32 u4FD_PerframeAntiOverFlag;               //change type
    MUINT32 u4FD_SideFaceLock;               //change type
    MUINT32 u4FD_LimitStableThdLowBnd;               //change type
    MUINT32 u4FD_LimitStableThdLowBndNum;               //change type
    MUINT32 u4FD_ReConvergeWhenFaceChange;               //change type
    MUINT32 u4FD_FaceMotionLockRat;
    MUINT32 u4FD_ImpulseLockCnt;
    MUINT32 u4FD_ImpulseUpperThd;
    MUINT32 u4FD_ImpulseLowerThd;
    MUINT32 u4FD_ROPTrustCnt;
    MUINT32 u4FD_ReCovergeWhenSizeChangeRat;
    MUINT32 u4FD_ReCovergeWhenPosChangeDist;
    MUINT32 u4FD_ReCovergeWhenRealBVx1000DiffLarge;
    MUINT32 u4FD_ReCovergeWhenAFDone;
    MUINT32 u4FD_OTFaceLock;
    MBOOL   bCnnFaceEnable;
    MBOOL   bReConvergeWhenCNNFaceBack;
    MINT32  i4PortraitTolerance;
    MINT32  i4FD_CNN_Lock_MaxCnt;
    MBOOL   bEVCompExceptionEnable;
    MUINT32 u4EVCompTouchMeteringStableMax;
    MUINT32 u4EVCompTouchMeteringStableMin;
    MUINT32 u4EVCompTouchOutStableThd;
    MUINT32 u4EVCompTouchInStableThd;
    MBOOL   bFaceAccLockEnable;
    MUINT32 u4FaceAccLockThd;
    MUINT32 u4FaceAccLock_MaxCnt;
    MBOOL   bFaceGyroDiffLockEnable;
    MUINT32 u4FaceGyroDiffLockThd;
    MUINT32 u4FaceGyroDiffLock_MaxCnt;
}CCU_strFaceSmooth;

typedef enum
{
    CCU_FACE_PRIOR_TIME,
    CCU_FACE_PRIOR_SIZE,
    CCU_FACE_PRIOR_LIGHT,
    CCU_FACE_PRIOR_DARK,
    CCU_FACE_PRIOR_MAX
}CCU_FACE_PRIOR_ENUM;

typedef struct {
    MBOOL bEnableMultiFaceWeight;
    CCU_FACE_PRIOR_ENUM Prior;
    MUINT32 u4FaceWeight[CCU_MAX_AE_METER_AREAS];

}CCU_strFaceWeight;

typedef struct {
    MBOOL   bLandmarkCtrlFlag;              // face landmark ctrol flag
    MUINT32 u4LandmarkCV_Thd;               // face landmark info cv thd
    MUINT32 u4LandmarkWeight;               // face landmark weight
    MINT32  i4LandmarkExtRatW;
    MINT32  i4LandmarkExtRatH;
    MINT32  i4LandmarkTrustRopDegree;
    MINT32  i4RoundXYPercent;
    MINT32  i4RoundSZPercent;
    MINT32  i4THOverlap;
    MINT32  i4BUFLEN; //must not exceed #LANDMARK_SMOOTH_MAX_LENGTH
    MINT32  i4SMOOTH_LEVEL;
    MINT32  i4MOMENTUM;
    MINT32  u4LandmarkWeightPercent;
}CCU_strFaceLandMarkCtrl;

typedef struct
{
    MBOOL   bEnable;            //enable NS CDF
    UINT32   u4Pcent;            // 1=0.1%, 0~1000 //change type
    MINT32  i4NS_CDFRatio_X[2];
    MUINT32 u4NS_CDFRatio_Y[2]; //change type
} CCU_strNS_CDF;

typedef struct {
    MBOOL   bFaceLocSizeCheck;
    MUINT32 u4Size_X[2];
    MUINT32 u4Size_Y[2];
    MUINT32 u4Loc_X[2];
    MUINT32 u4Loc_Y[2];

    MBOOL   bFaceOECheck;
    MUINT32 u4OE_X[2];
    MUINT32 u4OE_Y[2];
    MINT32  i4OE_BV_X[2];
    MUINT32 u4OE_BV_Y[2];
    MUINT32 u4FCY_DX[2];
    MUINT32 u4FCY_INDOOR_DX[2];
    MUINT32 u4FCY_BX[2];
    MUINT32 u4FCY_Y[2];

    MBOOL   bLandmarkSize;
} CCU_strFaceLocSize;

typedef struct {
    MUINT32 u4InStableThd;  // 0.08EV
    MUINT32 u4OutStableThd; // 0.08EV

    MBOOL   bEnableAEModeStableTHD;         // enable video ae stable threshold setting
    MUINT32 u4VideoInStableThd;               // video mode each index 0.08EV
    MUINT32 u4VideoOutStableThd;              // video mode each index 0.08EV
    MUINT32 u4FaceInStableThd;               // Face mode each index 0.08EV
    MUINT32 u4FaceOutStableThd;              // Face mode each index 0.08EV
    MUINT32 u4FaceOutB2TStableThd;              // Face mode each index 0.08EV
    MUINT32 u4FaceOutD2TStableThd;              // Face mode each index 0.08EV
    MUINT32 u4TouchInStableThd;               // Touch mode each index 0.08EV
    MUINT32 u4TouchOutStableThd;              // Touch mode each index 0.08EV
    MBOOL   bEnableFaceAeLock;               // Face Ae lock option
    MBOOL   bEnableZeroStableThd;            // enable zero stable thd
} CCU_strAEStableThd;

typedef struct
{
    MBOOL   bEnable;                        //enable adaptive AOE THD
    MUINT32 u4TblLength;
    MINT32 i4BV[AE_RATIOTBL_MAXSIZE];
    MUINT32 u4THD[AE_RATIOTBL_MAXSIZE];
} CCU_strAOEBVRef;


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
} CCU_strPerframeCFG;

typedef struct {
    MBOOL bEnableStablebyHist;
    MUINT32 u4HistEVDiff;
    MUINT32 u4OverexpoTHD;
    MUINT32 u4UnderexpoTHD;
    MUINT32 u4HistStableTHD;
} CCU_strHistStableCFG;

typedef struct {
    MBOOL u4PsoAccEnable;
    MINT32 i4PsoEscRatioWO;  //PSO speedup ratio discount, 100 base (Wash-Out case)
    MINT32 i4PsoEscRatioNonLinear; //PSO speedup ratio discount, 100 base (Non-Linear Case)
    MINT32 ConvergeDeltaPosBound;
    MINT32 ConvergeDeltaNegBound;
}CCU_strPSOConverge;

typedef struct {
    UINT32 u4FlareSmallDeltaIdxStep;
    UINT32 u4CWVChangeStep;
    UINT32 u4AllStableStep;
    MBOOL  bOutdoorFlareEnable;
    MUINT32 u4OutdoorMaxFlareThres;
    MINT32  i4BVRatio[2];
} CCU_strDynamicFlare;


typedef struct {
    MBOOL u4BVAccEnable;
    MINT32 i4DeltaBVRatio;
    MUINT32 u4B2T_Target;
    CCU_strAEMovingRatio  pAEBVAccRatio;           // Preview ACC ratio
} CCU_strBVAccRatio;


typedef enum
{
    CCU_AE_WEIGHTING_CENTRALWEIGHT=0,
    CCU_AE_WEIGHTING_SPOT,
    CCU_AE_WEIGHTING_AVERAGE
}CCU_eWeightingID;

typedef struct
{
   CCU_eWeightingID eID;  //weighting table ID
   MUINT32 W[5][5];    //AE weighting table
}CCU_strWeightTable;


typedef enum
{
    CCU_AETABLE_RPEVIEW_AUTO = 0,     // default 60Hz
    CCU_AETABLE_CAPTURE_AUTO,
    CCU_AETABLE_VIDEO_AUTO,
    CCU_AETABLE_VIDEO1_AUTO,
    CCU_AETABLE_VIDEO2_AUTO,
    CCU_AETABLE_CUSTOM1_AUTO,
    CCU_AETABLE_CUSTOM2_AUTO,
    CCU_AETABLE_CUSTOM3_AUTO,
    CCU_AETABLE_CUSTOM4_AUTO,
    CCU_AETABLE_CUSTOM5_AUTO,
    CCU_AETABLE_VIDEO_NIGHT,
    CCU_AETABLE_CAPTURE_ISO50,
    CCU_AETABLE_CAPTURE_ISO100,
    CCU_AETABLE_CAPTURE_ISO200,
    CCU_AETABLE_CAPTURE_ISO400,
    CCU_AETABLE_CAPTURE_ISO800,
    CCU_AETABLE_CAPTURE_ISO1600,
    CCU_AETABLE_CAPTURE_ISO3200,
    CCU_AETABLE_STROBE,
    CCU_AETABLE_SCENE_INDEX1,                           // for mode used of capture
    CCU_AETABLE_SCENE_INDEX2,
    CCU_AETABLE_SCENE_INDEX3,
    CCU_AETABLE_SCENE_INDEX4,
    CCU_AETABLE_SCENE_INDEX5,
    CCU_AETABLE_SCENE_INDEX6,
    CCU_AETABLE_SCENE_INDEX7,
    CCU_AETABLE_SCENE_INDEX8,
    CCU_AETABLE_SCENE_INDEX9,
    CCU_AETABLE_SCENE_INDEX10,
    CCU_AETABLE_SCENE_INDEX11,
    CCU_AETABLE_SCENE_INDEX12,
    CCU_AETABLE_SCENE_INDEX13,
    CCU_AETABLE_SCENE_INDEX14,
    CCU_AETABLE_SCENE_INDEX15,
    CCU_AETABLE_SCENE_INDEX16,
    CCU_AETABLE_SCENE_INDEX17,
    CCU_AETABLE_SCENE_INDEX18,
    CCU_AETABLE_SCENE_INDEX19,
    CCU_AETABLE_SCENE_INDEX20,
    CCU_AETABLE_SCENE_INDEX21,
    CCU_AETABLE_SCENE_INDEX22,
    CCU_AETABLE_SCENE_INDEX23,
    CCU_AETABLE_SCENE_INDEX24,
    CCU_AETABLE_SCENE_INDEX25,
    CCU_AETABLE_SCENE_INDEX26,
    CCU_AETABLE_SCENE_INDEX27,
    CCU_AETABLE_SCENE_INDEX28,
    CCU_AETABLE_SCENE_INDEX29,
    CCU_AETABLE_SCENE_INDEX30,
    CCU_AETABLE_SCENE_INDEX31,
    CCU_AETABLE_SCENE_MAX
}CCU_eAETableID;
typedef struct
{
    CCU_LIB3A_AE_SCENE_T eAEScene;
    CCU_eAETableID ePLineID[11];
} CCU_strAEPLineMapping;
typedef struct
{
    CCU_strAEPLineMapping    sAESceneMapping[MAX_PLINE_MAP_TABLE];   // Get PLine ID for different AE mode
}CCU_strAESceneMapping;
#define SUPPORTED_MAX_HDR_EXPNUM            (4)
#define HDR_AE_LV_TBL_NUM                   (19)
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
} CCU_HDR_DETECTOR_INFO_T;

typedef struct
{
    MINT32                      i4Acce[3];
    MINT32                      i4Gyro[3];
} CCU_HDR_GGYRO_INFO_T;


typedef struct
{
    MINT32  i4AutoRMGk;
    MINT32  i4PvRMGt;
    MINT32  i4CurrRMGt;
    MINT32  i4MeanRMGt;
    MINT32  i4AutoRMGt;
    MINT32  i4AutoLVRatio;
    MINT32  i4AutoContrastRatio;
    MINT32  i4PvBrightRatio;
    MINT32  i4CurrBrightRatio;
    MINT32  i4MeanBrightRatio;
    MINT32  i4AutoBrightRatio;
    MINT32  i4AutoNSRatio;
    MINT32  i4AutoISORatio;
    MINT32  i4AutoGyroRatio;
    MINT32  i4AutoAlgRatio;
    MINT32  i4AutoPvAlgoRatio;
    CCU_HDR_GGYRO_INFO_T rGGyroInfo;
} CCU_HDR_AUTO_ENHANCE_INFO_T;

typedef struct {
    MINT32 GTM_X0;          // 12b
    MINT32 GTM_Y0;          // 16b
    MINT32 GTM_S0;          // 16.4b
    MINT32 GTM_S1;          // 16.4b
} CCU_HDR_DCPN_INFO_T;

typedef struct {
    MINT32 GTM_X[7];        // 12b
    MINT32 GTM_Y[7];        // 12b
    MINT32 GTM_S[8];        // 4.8b
} CCU_HDR_CPN_INFO_T;


typedef struct {
    CCU_HDR_DCPN_INFO_T             rHdrDCPNProp;
    CCU_HDR_CPN_INFO_T              rHdrCPNProp;
} CCU_HDR_TONE_INFO_T;

typedef struct {
    MINT32 i4NormalAETarget;
    MINT32 i4HdrNormalAETarget;
    MINT32 i4AutoEVWeight;
    MINT32 i4PvBrightWeight;
    MINT32 i4CurrBrightWeight;
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
} CCU_HDR_AE_TARGET_INFO_T;

typedef struct {
    MINT32  i4GainBase;     /*  Video HDR 0 EV info.  */
    MINT32  i4SEInfo;       /*  Delta EV compared with i4GaniBase  */
    MINT32  i4LEInfo;       /*  Low averege data  */
} CCU_HDR_CHDR_INFO_T;

typedef struct {
    /*
    *   HDR detector smooth info
    */
    MINT32  i4PvHdrOnOff;
    MINT32  i4PvHdrStableOnOff;
    MINT32  i4HdrOnOffCounter;
    MINT32  i4HdrStableOnOff;
    /*
    *   HDR weighting smooth info
    */
    MINT32  i4AutoWeightStableCount;
    /*
    *   Motion smooth info
    */
    MINT32  i4MotionOnCount;
    MINT32  i4PvMotionOnOff;
    MINT32  i4PvMotionStableOnOff;
    MINT32  i4MotionStableOnOff;

    /*
    *   RMG smooth info
    */
    MINT32 i4RMGtStableCount;

    /*
    *   Ratio smooth info
    */
    MINT32  i4SmoothTrigger;
    MINT32  i4RatioStableCount;
    MINT32  i4PvHdrRatio;
    MINT32  i4PvHdrStableRatio;
    MINT32  i4HdrRatioCount;
    MINT32  i4MeanHdrRatio;
    MINT32  i4HdrStableRatio;
    MINT32  i4HdrRatioStep;
    MINT32  i4PvHdrSmoothRatio;
    MINT32  i4HdrSmoothRatio;
    MINT32  i4MediumHdrRatio;
} CCU_HDR_SMOOTH_INFO_T;

typedef struct {
    MINT32  i4LEExpo;
    MINT32  i4LEAfeGain;
    MINT32  i4LEIspGain;
    MINT32  i4SEExpo;
    MINT32  i4SEAfeGain;
    MINT32  i4SEIspGain;
    MINT32  i4HdrRatio;
} CCU_HDR_EV_INFO_T;

typedef struct
{
    MINT32                      i4AEMode;
    MINT32                      i4LV;
    CCU_HDR_DETECTOR_INFO_T         rDetectorInfo;
    CCU_HDR_AUTO_ENHANCE_INFO_T     rAutoEnhanceInfo;
    CCU_HDR_AE_TARGET_INFO_T        rHdrAETargetInfo;
    CCU_HDR_CHDR_INFO_T             rcHdrInfo;
    CCU_HDR_SMOOTH_INFO_T           rHdrSmoothInfo;
    CCU_HDR_EV_INFO_T               rHdrEvInfo;
    CCU_HDR_TONE_INFO_T             rHdrToneInfo;
} CCU_HDR_AE_INFO_T;

// HDR Application - Parameters

typedef struct {
    MINT32              i4HdrHistP0, i4HdrHistP1;
    MINT32              i4HdrHistW0, i4HdrHistW1;
} CCU_HDR_WEIGHTING_WIN_T;

typedef struct
{
    MBOOL               bHistDetectEnable;
    CCU_HDR_WEIGHTING_WIN_T rHdrDetectDarkWtWin;
    CCU_HDR_WEIGHTING_WIN_T rHdrDetectBrightWtWin;
    CCU_HDR_WEIGHTING_WIN_T rHdrEnhanceBrightWtWin;
    CCU_HDR_WEIGHTING_WIN_T rHdrFDBrightWtWin;
    MINT32              i4HdrContrastPercentH;
    MINT32              i4HdrContrastPercentL;
    MINT32              i4HdrNightEVDiffPercentH;
    MINT32              i4HdrNightEVDiffPercentL;
    MINT32              i4HdrNightDarkPercent;
    CCU_AE_TARGET_PROB_T    rHdrNightEVDiffProb;
    CCU_AE_TARGET_PROB_T    rHdrNightHistProb;
    CCU_AE_TARGET_PROB_T    rHdrNightLVProb;
} CCU_HDR_DETECT_HIST_T;

typedef struct
{
    MBOOL               bGeometryCOEEnable;
    MINT32              i4COEWeight;                // x1 = 1024
    CCU_AE_TARGET_PROB_T    rHdrCOEOuterRatio;          // the outer y ratio
    CCU_AE_TARGET_PROB_T    rHdrCOEDiffRatio;           // inner/outer y difference ratio
} CCU_HDR_GEOMETRY_COE_T;

typedef struct
{
    MBOOL                bGeometryABLEnable;
    MINT32              i4ABLWeight;                // x1 = 1024
    CCU_AE_TARGET_PROB_T    rCenterBrightDiffRatio;     // C-type / U-type y difference ratio
    CCU_AE_TARGET_PROB_T    rCenterDarkDiffRatio;       // C-type / U-type y difference ratio
} CCU_HDR_GEOMETRY_ABL_T;

typedef struct
{
    CCU_HDR_GEOMETRY_COE_T rGeometryCOEProp;
    CCU_HDR_GEOMETRY_ABL_T rGeometryABLProp;
} CCU_HDR_DETECT_GEOMETRY_T;

typedef struct
{
    MBOOL    bLVDetectEnable;
    MINT32  i4LVProbTbl[HDR_AE_LV_TBL_NUM];
} CCU_HDR_DETECT_LV_T;

typedef struct
{
    MBOOL                    bHdrDetectorEnable;
    MINT32                  i4HdrConfidenceThr;
    CCU_HDR_DETECT_HIST_T       rHdrHistInfo;
    CCU_HDR_DETECT_GEOMETRY_T   rHdrGeometryInfo;
    CCU_HDR_DETECT_LV_T         rHdrLVInfo;
} CCU_HDR_DETECTOR_T;

typedef struct
{
    MINT32                  i4NightPercent;
    CCU_AE_TARGET_PROB_T        rNightTarget;
} CCU_HDR_NIGHT_TARGET_T;

typedef struct
{
    MINT32                  i4AoePercent;
    MINT32                  i4AoeLV[3];
    MINT32                  i4AoeTarget[3];
} CCU_HDR_AOE_TARGET_T;

typedef struct {
    MINT32                  i4LVLimit;
    CCU_AE_TARGET_PROB_T        rTargetMinLimit;
    CCU_AE_TARGET_PROB_T        rTargetMaxLimit;
} CCU_HDR_TARGET_LIMIT_T;

typedef struct
{
    MINT32                  i4DefTarget;
    MINT32                  i4DefWeight;
    CCU_AE_TARGET_PROB_T        rAutoEVWeight;
    CCU_AE_TARGET_PROB_T        rAutoBrightWeight;
    CCU_AE_TARGET_PROB_T        rAutoABLWeight;
    CCU_AE_TARGET_PROB_T        rAutoLVWeightL;
    CCU_AE_TARGET_PROB_T        rAutoLVWeightH;
    CCU_AE_TARGET_PROB_T        rAutoNightWeight;
    CCU_HDR_AOE_TARGET_T        rHdrAoeTarget;
    CCU_HDR_NIGHT_TARGET_T      rHdrNightTarget;
    CCU_HDR_TARGET_LIMIT_T      rTargetLimitL;
    CCU_HDR_TARGET_LIMIT_T      rTargetLimitM;
    CCU_HDR_TARGET_LIMIT_T      rTargetLimitH;
} CCU_HDR_AE_TARGET_T;

typedef struct {
    CCU_AE_TARGET_PROB_T        rFaceEVProb;
    CCU_AE_TARGET_PROB_T        rFaceHdrProb;
} CCU_HDR_FACE_T;


typedef struct {
    CCU_HDR_FACE_T              rFaceAEProp;
} CCU_HDR_MISC_PROP_T;

typedef struct
{
    CCU_AE_TARGET_PROB_T        rAcceProb;
    CCU_AE_TARGET_PROB_T        rGyroProb;
    CCU_AE_TARGET_PROB_T        rMotionRatio;
    MINT32                  i4MotionOnThr;
    MINT32                  i4MotionOnCntThr;

} CCU_HDR_G_GYRO_SENSOR_T;


typedef struct
{
    MINT32              i4LVRatio50[HDR_AE_LV_TBL_NUM];
    MINT32              i4LVRatio60[HDR_AE_LV_TBL_NUM];
    CCU_AE_TARGET_PROB_T    rContrastRatio;
    CCU_AE_TARGET_PROB_T    rBrightRatio;
    CCU_AE_TARGET_PROB_T    rNightProbRatio;
    CCU_HDR_G_GYRO_SENSOR_T rGGyroSensor;
} CCU_AUTO_BRIGHT_ENHANCE_T;


typedef struct
{
    MINT32  i4RmmExpoDiffThr;           //  %
    MINT32  i4LscRatio;                 //
} CCU_AUTO_TUNING_T;


typedef struct
{
    MBOOL    bHdrWaitAEStable;
    MINT32  i4HdrInfoCounterThr;
    MBOOL    bRatioSmoothEnable;
    MINT32  i4AutoWeightMode;           // 0: Max, 1:Min, 2:Mean
    MINT32  i4AutoWeightSmooth;
    MINT32  i4AutoWeightStableThr;
    MINT32  i4RMGtStableMode;           // 0: Max, 1:Min, 2:Mean
    MINT32  i4RMGtStableThr;
    MINT32  i4HdrRatioStableMode;       // 0: Max, 1:Min, 2:Mean
    MINT32  i4HdrRatioStableThr;
    MINT32  i4RatioSmoothThr[2];        //  [0]:normal, [1]:motion
    MINT32  i4RoughRatioUnit[2];        //  [0]:normal, [1]:motion
    MINT32  i4FinerRatioUnit[2];        //  [0]:normal, [1]:motion
    MINT32  i4RMGtUnit[2];              //  [0]:normal, [1]:motion
} CCU_AUTO_SMOOTH_T;

typedef struct
{
    MINT32 i4Flkr1LVThr[2];         // [0]: 50 Hz,  [1]: 60 Hz
    MINT32 i4MinRatioAntiFlk;
    MINT32 i4MaxExp[2];
    MINT32 i4MinExp[2];
    CCU_AE_TARGET_PROB_T rIsoThr;
} CCU_AUTO_FLICKER_RATIO_T;

typedef struct
{
    MBOOL                    bAutoEnhanceEnable;
    MINT32                  i4RMGk;
    MINT32                  i4RMGtRatio;
    CCU_AE_TARGET_PROB_T        rRMGt;
    MINT32                  i4SupportHdrNum;
    MINT32                  i4AutoEnhanceProb;
    MINT32                  i4HdrWeight;
    CCU_AUTO_BRIGHT_ENHANCE_T   rAutoBright;
    CCU_AUTO_FLICKER_RATIO_T    rAutoFlicker;
    CCU_AUTO_SMOOTH_T           rAutoSmooth;
    CCU_AUTO_TUNING_T           rAutoTuning;
    MINT32                  i4FlashRatio;
} CCU_HDR_AUTO_ENHANCE_T;

typedef struct
{
    MINT32                  i4LEThr;
    MINT32                  i4SERatio;
    MINT32                  i4SETarget;
    MINT32                  i4BaseGain;
} CCU_HDR_CHDR_T;

typedef enum {
    CCU_eMainHDR_OFF = 0,
    CCU_eMainHDR_AUTO,
    CCU_eMainHDR_ON,
    CCU_eMainHDR_VT,
    CCU_eMain2HDR_OFF,
    CCU_eMain2HDR_AUTO,
    CCU_eMain2HDR_ON,
    CCU_eMain2HDR_VT,
    CCU_eSubHDR_OFF,
    CCU_eSubHDR_AUTO,
    CCU_eSubHDR_ON,
    CCU_eSubHDR_VT,
    CCU_eSub2HDR_OFF,
    CCU_eSub2HDR_AUTO,
    CCU_eSub2HDR_ON,
    CCU_eSub2HDR_VT
}CCU_eHDRPARAM_ID;

typedef struct
{
    CCU_eHDRPARAM_ID            eHdrParamID;
    CCU_HDR_DETECTOR_T          rHdrDetector;
    CCU_HDR_AUTO_ENHANCE_T      rHdrAutoEnhance;
    CCU_HDR_AE_TARGET_T         rHdrAETarget;
    CCU_HDR_MISC_PROP_T         rHdrMiscProp;
    CCU_HDR_CHDR_T              rHdrcHdr;
} CCU_HDR_AE_PARAM_T;
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

} CCU_strAETgTuningPara;

typedef struct 
{
    MUINT32 EVValue[81];
} CCU_strEVC;

typedef struct
{
    MINT32 Diff_EV;     //  delta EVx10 ,different between Yavg and Ytarget     Diff_EV=    log(  Yarg/Ytarget,2)
    MINT32  Ration;        //  Yarg/Ytarget  *100
    MINT32  move_index;   // move index
} CCU_strAEMOVE;

typedef struct
{
    CCU_strAEMOVE table[MOVING_TABLE_SIZE];
} CCU_strAEMoveTable;
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
    MUINT8 uHist4StartBlockXRatio;       //Histogram 4 window config start block X ratio (0~100)
    MUINT8 uHist4EndBlockXRatio;         //Histogram 4 window config end block X ratio (0~100)
    MUINT8 uHist4StartBlockYRatio;       //Histogram 4 window config start block Y ratio (0~100)
    MUINT8 uHist4EndBlockYRatio;         //Histogram 4 window config end block Y ratio (0~100)
    MUINT8 uHist4OutputMode;               //Histogram 4 output source mode
    MUINT8 uHist4BinMode;                      //Histogram 4 bin mode range
    MUINT8 uHist5StartBlockXRatio;       //Histogram 5 window config start block X ratio (0~100)
    MUINT8 uHist5EndBlockXRatio;         //Histogram 5 window config end block X ratio (0~100)
    MUINT8 uHist5StartBlockYRatio;       //Histogram 5 window config start block Y ratio (0~100)
    MUINT8 uHist5EndBlockYRatio;         //Histogram 5 window config end block Y ratio (0~100)
    MUINT8 uHist5OutputMode;               //Histogram 5 output source mode
    MUINT8 uHist5BinMode;                      //Histogram 5 bin mode range
} CCU_AE_HIST_WIN_CFG_T;

typedef struct
{
    MUINT8 uHistStartBlockXRatio;       //Histogram 3 window config start block X ratio (0~100)
    MUINT8 uHistEndBlockXRatio;         //Histogram 3 window config end block X ratio (0~100)
    MUINT8 uHistStartBlockYRatio;       //Histogram 3 window config start block Y ratio (0~100)
    MUINT8 uHistEndBlockYRatio;         //Histogram 3 window config end block Y ratio (0~100)
} CCU_PS_HIST_WIN_CFG_T;

typedef  struct 
{    
    MBOOL   isEnableDFps;
    MUINT32 EVThresNormal;
    MUINT32 EVThresNight;
} CCU_VdoDynamicFrameRate_T;
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
}CCU_strAELimiterTable;




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

typedef struct {
    CCU_AEMeterArea_T rAreas[CCU_MAX_AE_METER_AREAS];
    MUINT32 u4Count;
    MINT32 GyroAcc[3];
    MINT32 GyroRot[3];
} CCU_AEMeteringArea_T;

typedef struct
{
    CCU_PSO_PATH_SEL_ENUM Pso_Output_Path; //Before OB or after OB
    CCU_AWB_GAIN_T awb_gain;
    MUINT32 awb_gain_Unit;

    MUINT32 OB_Gain_Unit;
    MUINT32 OB_Gain[4];
    MUINT32 OB_Offset[4];

    //MUINT32 CCM_Gain_Unit;
    //MUINT32 CCM[9];
    
    MUINT32 LSC_Gain_Unit;
    //MUINT32 LSC_Gain_Grid_R[AE_V4P0_BLOCK_NO*AE_V4P0_BLOCK_NO];
    MUINT16 LSC_Gain_Grid_G[AE_V4P0_BLOCK_NO*AE_V4P0_BLOCK_NO];
    //MUINT32 LSC_Gain_Grid_B[AE_V4P0_BLOCK_NO*AE_V4P0_BLOCK_NO];

    MUINT32 PreGain1_Unit;

    CCU_PRE_GAIN1_T PreGain1;
    MUINT32 RGB2Y_Coef[3]; //RGB2Y Coef
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
    CCU_AE_STATE_CREATE,            // 0  CREATE , JUST CREATE
    CCU_AE_STATE_INIT,              // 1  INIT
    CCU_AE_STATE_NORMAL_PREVIEW,    // 2  normal AE
    CCU_AE_STATE_AFASSIST,          // 3  aF assist mode, limit exposuret time
    CCU_AE_STATE_AELOCK,            // 4  LOCK ae
    CCU_AE_STATE_CAPTURE,           // 5  capture
    CCU_AE_STATE_ONE_SHOT,          // 6  one shot AE
    CCU_AE_STATE_SLOW_MOTION,       // 7  slow motion AE
    CCU_AE_STATE_PANORAMA_MODE,     // 8  Panorama mode
    CCU_AE_STATE_BACKUP_PREVIEW,    // 9  backup preview
    CCU_AE_STATE_RESTORE_PREVIEW,   //10 backup preview
    CCU_AE_STATE_POST_CAPTURE,      //11 backup preview
    CCU_AE_STATE_TOUCH_PERFRAME,    //12 Touch ae peframe
    CCU_AE_STATE_MAX
}CCU_eAESTATE;

typedef struct
{
    void*               pAESatisticBuffer;
    //CCU_strEvSetting        PreEvSetting[MAX_AE_PRE_EVSETTING];
    CCU_strAECycleInfo      CycleInfo;
    MINT32 pCycleInfo_i4ShutterDelayFrames;
    MINT32 pCycleInfo_i4GainDelayFrames;
    MINT32 pCycleInfo_i4IspGainDelayFrames;
    MINT32 pCycleInfo_i4FrameCnt;
    CCU_eAESTATE            eAeState;   //ae state
    ///#^
} CCU_strAEInput; // useful check ok

typedef enum
{
    CCU_SYNC_AE_DUAL_CAM_DENOISE_BMDN = 0,
    CCU_SYNC_AE_DUAL_CAM_DENOISE_MFNR,
    CCU_SYNC_AE_DUAL_CAM_DENOISE_MAX
} CCU_SYNC_AE_DUAL_CAM_DENOISE_ENUM;


typedef struct {
    MINT32 i4RMGt;
    MINT32 i4RMGk;
    CCU_HDR_EV_INFO_T rHdrEVInfo;
    MINT32 i4HdrOnOff;
    MINT32 i4HdrConfidence;
    MINT32 i4BrightConfidence;
    MINT32 i4DarkConfidence;
    MINT32 i4cHdrGainBase;                                  /*  Video HDR 0 EV info.  */
    MINT32 i4cHdrSEInfo;                                    /*  Delta EV compared with i4GaniBase  */
    MINT32 i4cHdrLEInfo;                                    /*  Low averege data  */
    MINT32 i4SwitchHLR;
} CCU_strEvHdrSetting;

typedef struct
{
    CCU_strEvSetting  EvSetting;
    MINT32        Bv;
    MINT32        AoeCompBv;
    MINT32        i4EV;
    MUINT32       u4ISO;          //correspoing ISO , only use in capture
    MINT16        i2FaceDiffIndex;
    // MINT32        i4AEidxCurrent;  // current AE idx
    // MINT32        i4AEidxNext;  // next AE idx
    MUINT32       u4Index;
    MUINT32       u4IndexF;        
    MINT32        i4Cycle0FullDeltaIdx;
    MINT32        i4FrameCnt;
    MUINT32           u4FracGain;    // stepless gain, lower than 0.1EV
    MUINT32       u4Prvflare;
    MBOOL             bAEStable;      // Only used in Preview/Movie
    MBOOL             bGammaEnable;   // gamma enable
    MUINT32           u4CWValue;
    MUINT32           u4AECondition;
    MINT32            i4DeltaBV;
    MINT32            i4PreDeltaBV;    
    MUINT32           u4DeltaBVRatio[CCU_SYNC_AE_DUAL_CAM_DENOISE_MAX];
    MUINT16           u2FrameRate;     // Calculate the frame
    MINT16           i2FlareOffset;
    MINT16           i2FlareGain;   // in 512 domain
    // MINT32           i4AEidxCurrentF;  // current AE idx
    // MINT32           i4AEidxNextF;   // next AE idx
    MINT32           i4gammaidx;   // next gamma idx
    MINT32           i4LESE_Ratio;    // LE/SE ratio
    MUINT32     u4BayerY;
    MUINT32     u4MonoY;
    MINT32  i4RealBVx1000;
    MINT32  i4RealBV;
    MUINT32 u4CWRecommendStable;
    MUINT32 u4AEFinerEVIdxBase;
    MUINT32 u4FaceFailCnt;
    MUINT32 u4FaceFoundCnt;
    MUINT32 u4FaceAEStable;
} CCU_strAEOutput;


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
   MBOOL bAFPlineEnable;
   MINT16 i2FrameRate[5][2];
}CCU_strAFPlineInfo;
typedef struct 
{
    CCU_strAEParamCFG strAEParasetting;
    CCU_strWeightTable Weighting[MAX_WEIGHT_TABLE];   //AE WEIGHTING TABLE
    CCU_strAFPlineInfo strAFPLine;
    CCU_strAFPlineInfo strAFZSDPLine;
    CCU_strAFPlineInfo strStrobePLine;
    CCU_strAFPlineInfo strStrobeZSDPLine;
    CCU_strEVC EVValueArray;    
    CCU_strAEMoveTable AEMovingTable;
    CCU_strAEMoveTable AEVideoMovingTable;
    CCU_strAEMoveTable AEFaceMovingTable;
    CCU_strAEMoveTable AETrackingMovingTable;

    CCU_strAELimiterTable strAELimiterData;
    CCU_VdoDynamicFrameRate_T strVdoDFps;

    CCU_AE_HIST_WIN_CFG_T AEBlockWINCFG;
    CCU_AE_HIST_WIN_CFG_T AEPixelWINCFG;
    CCU_PS_HIST_WIN_CFG_T PSPixelWINCFG;
    // v1.2
    MBOOL   bOldAESmooth;                          // Select the new or old AE smooth control
    MBOOL   bEnableSubPreIndex;                // decide the sub camera re-initial index after come back to camera
    MUINT32 u4VideoLPFWeight; // 0~24

    CCU_strFaceLocSize FaceLocSizecheck;
    CCU_strPerframeCFG PerframeCFG;
    MBOOL   bPerFrameAESmooth;                    // Perframe AE smooth option
    MBOOL   bPerFrameHDRAESmooth;                    // Perframe AE smooth option

    // TG interrupt
    MBOOL   bTgIntAEEn;                    // TG interrupt option
    MBOOL   bSpeedupEscOverExp;
    MBOOL   bSpeedupEscUnderExp;
    CCU_strAETgTuningPara AETgTuningParam;
    CCU_strEVC TgEVCompJumpRatioOverExp;
    CCU_strEVC TgEVCompJumpRatioUnderExp;
    MUINT32 pTgWeightTbl[9][12];

    //Open AE
    CCU_strAEStableThd AEStableThd;
    CCU_strBVAccRatio   BVAccRatio;              // Preview ACC ratio
    CCU_strPSOConverge PSOConverge;
    CCU_strNonCWRAcc    NonCWRAcc;
    CCU_strFaceSmooth  FaceSmooth;
    CCU_strFaceWeight  FaceWeight;
    CCU_strAECWRTempSmooth  AETempSmooth;
    CCU_strFaceLandMarkCtrl FaceLandmark;
    CCU_strAOEBVRef     AOERefBV;
    CCU_strNS_CDF       NsCdfRatio;

    //v2.5
    CCU_strAEMovingRatio AETouchMovingRatio;   //(temp) Touch smooth ratio
    CCU_strHSFlatSkyCFG     HSFlatSkyEnhance; //HS adaptive Prob Threshold by real BV
    CCU_strABL_absDiff      ABLabsDiff;
    //v3.0
    CCU_strHSMultiStep    HSMultiStep;
    CCU_strHistStableCFG  HistStableCFG;
    CCU_strOverExpoAOECFG OverExpoAOECFG;
    //v4.0
    MBOOL bAEv4p0MeterEnable;
    CCU_strMainTargetCFG MainTargetCFG;
    CCU_strHSv4p0CFG HSv4p0CFG;
    CCU_strNSBVCFG NSBVCFG;

    //v4.1
    CCU_strAEMovingRatio AEACCMovingRatio;   //(temp) acc smooth ratio
    // HDR
    CCU_HDR_AE_PARAM_T    HdrAECFG;

    // Stereo Pline mapping
    CCU_strAESceneMapping strStereoPlineMapping;
    // HDR Pline mapping
    CCU_strAESceneMapping strHDRPlineMapping;

    //Dynamic Flare
    CCU_strDynamicFlare DynamicFlareCFG;

}CCU_AE_PARAM_T;

typedef struct
{
    MUINT32 u4TopProtect;
    CCU_AE_DEVICES_INFO_T rDevicesInfo;
    CCU_AE_HIST_CFG_T rHistConfig;
    CCU_AE_CCT_CFG_T rCCTConfig;
    CCU_HDR_AE_CFG_T rHDRAEConfig;
    CCU_AE_PARAM_T rAeParam;
    MUINT32 u4BottomProtect;
} CCU_AE_NVRAM_T;
#endif


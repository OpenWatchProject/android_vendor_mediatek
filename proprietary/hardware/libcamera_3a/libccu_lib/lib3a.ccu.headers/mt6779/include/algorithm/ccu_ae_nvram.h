#ifndef _CCU_AE_NVRAM_H_
#define _CCU_AE_NVRAM_H_

#include "ccu_ext_interface/ccu_types.h"
#include "ccu_ae_feature.h"
#include "ccu_AEPlinetable.h"

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
#define AE_BLOCK_NO  (5)
#define AE_V4P0_BLOCK_NO  (15)
#define AE_CCT_STRENGTH_NUM (5)
#define AE_ABL_STRENGTH_NUM (8)
#define CCU_AE_AOE_STRENGTH_NUM (3)
#define MAX_WEIGHT_TABLE (4)
#define MOVING_TABLE_SIZE (24)
#define CCU_MAX_AE_METER_AREAS  (9)
#define AE_RATIOTBL_MAXSIZE (8)
#define AE_BVDR_MAXSIZE (16)
#define AE_STABLE_BV_MAXSIZE (4)
#define AE_AISBV_MAXSIZE (5)
//--------------------------------------------------------------------------------------------------------------------------------------------------------------


typedef enum
{
    CCU_AE_WEIGHTING_CENTRALWEIGHT = 0,
    CCU_AE_WEIGHTING_SPOT,
    CCU_AE_WEIGHTING_AVERAGE
} CCU_eWeightingID;

typedef enum
{
    CCU_FACE_PRIOR_TIME,
    CCU_FACE_PRIOR_SIZE,
    CCU_FACE_PRIOR_LIGHT,
    CCU_FACE_PRIOR_DARK,
    CCU_FACE_PRIOR_MAX
} CCU_FACE_PRIOR_ENUM;

typedef struct
{
    MUINT32 u4MinGain;
    MUINT32 u4MaxGain;
    MUINT32 u4MiniISOGain;
    MUINT32 u4GainStepUnit;
    MUINT32 u4PreExpUnit;
    MUINT32 u4PreMaxFrameRate;
    MUINT32 u4VideoExpUnit;
    MUINT32 u4VideoMaxFrameRate;
    MUINT32 u4Video2PreRatio;    // 1x = 1024
    MUINT32 u4CapExpUnit;
    MUINT32 u4CapMaxFrameRate;
    MUINT32 u4Cap2PreRatio;     // 1x = 1024
    MUINT32 u4Video1ExpUnit;
    MUINT32 u4Video1MaxFrameRate;
    MUINT32 u4Video12PreRatio;    // 1x = 1024
    MUINT32 u4Video2ExpUnit;
    MUINT32 u4Video2MaxFrameRate;
    MUINT32 u4Video22PreRatio;    // 1x = 1024
    MUINT32 u4Custom1ExpUnit;
    MUINT32 u4Custom1MaxFrameRate;
    MUINT32 u4Custom12PreRatio;    // 1x = 1024
    MUINT32 u4Custom2ExpUnit;
    MUINT32 u4Custom2MaxFrameRate;
    MUINT32 u4Custom22PreRatio;    // 1x = 1024
    MUINT32 u4Custom3ExpUnit;
    MUINT32 u4Custom3MaxFrameRate;
    MUINT32 u4Custom32PreRatio;    // 1x = 1024
    MUINT32 u4Custom4ExpUnit;
    MUINT32 u4Custom4MaxFrameRate;
    MUINT32 u4Custom42PreRatio;    // 1x = 1024
    MUINT32 u4Custom5ExpUnit;
    MUINT32 u4Custom5MaxFrameRate;
    MUINT32 u4Custom52PreRatio;    // 1x = 1024
    MUINT32 u4LensFno;           // 10 Base
    MUINT32 u4FocusLength_100x;           // 100 Base
} CCU_AE_DEVICES_INFO_T;

typedef struct
{
    //histogram info
    MUINT32 u4HistHighThres;                         // central histogram high threshold
    MUINT32 u4HistLowThres;                          // central histogram low threshold
    MUINT32 u4MostBrightRatio;                       // full histogram high threshold
    MUINT32 u4MostDarkRatio;                         // full histogram low threshold
    MUINT32 u4CentralHighBound;                      // central block high boundary
    MUINT32 u4CentralLowBound;                       // central block low bounary
    MUINT32 u4OverExpThres[AE_CCT_STRENGTH_NUM];     // over exposure threshold
    MUINT32 u4HistStretchThres[AE_CCT_STRENGTH_NUM]; // histogram stretch trheshold
    MUINT32 u4BlackLightThres[AE_CCT_STRENGTH_NUM];  // backlight threshold
} CCU_AE_HIST_CFG_T;

typedef struct {
    MINT32 u4X1;
    MINT32 u4Y1;
    MINT32 u4X2;
    MINT32 u4Y2;
} CCU_AE_TARGET_PROB_T;

typedef struct {
    MBOOL   bEnableHistStretch;          // enable histogram stretch
    MUINT32 u4HistStretchWeight;         // Histogram weighting value
    MUINT32 u4Pcent;                     // 1%=10, 0~1000
    MUINT32 u4Thd;                       // 0~255
    MUINT32 u4FlatThd;                   // 0~255
    MUINT32 u4FlatBrightPcent;
    MUINT32 u4FlatDarkPcent;
    CCU_AE_TARGET_PROB_T    sFlatRatio;     //TARGET_HS_FLAT

    MBOOL  bEnableGreyTextEnhance;
    MUINT32 u4GreyTextFlatStart;
    CCU_AE_TARGET_PROB_T  sGreyTextRatio;   //TARGET_HS_COLOR
} CCU_AE_HS_SPEC_T;

typedef struct {
    MBOOL   bEnableAntiOverExposure;     // enable anti over exposure
    MUINT32 u4AntiOverExpWeight;         // Anti over exposure weighting value
    MUINT32 u4Pcent;                     // 1%=10, 0~1000
    MUINT32  u4Thd;                       // 0~255

    MBOOL bEnableCOEP;                   // enable COEP
    MUINT32 u4COEPcent;                  // center over-exposure prevention
    MUINT32 u4COEThd;                    // center y threshold
    MUINT32 u4BVCompRatio;               // Compensate BV in nonlinear
    CCU_AE_TARGET_PROB_T    sCOEYRatio;     // the outer y ratio
    CCU_AE_TARGET_PROB_T    sCOEDiffRatio;  // inner/outer y difference ratio
} CCU_AE_AOE_SPEC_T;

typedef struct {
    MBOOL   bEnableBlackLight;           // enable back light detector
    MUINT32 u4BackLightWeight;           // Back light weighting value
    MUINT32 u4Pcent;                     // 1%=10, 0~1000
    MUINT32 u4Thd;                       // 0~255

    MUINT32 u4CenterHighBnd;             // center luminance
    MUINT32 u4TargetStrength;            // final target limitation
    CCU_AE_TARGET_PROB_T    sFgBgEVRatio;   //TARGET_ABL_DIFF
    CCU_AE_TARGET_PROB_T    sBVRatio;       //FT_ABL

    MBOOL   bEnableTargetStrengthByBV;         //Enable TargetStrength change along BV
    MUINT32 u4TblLength;                                //Max size : 8
    MUINT32 i4Bv[AE_ABL_STRENGTH_NUM];         //size must large than u4TblLength
    MINT32  u4TargetStrengthByBV[AE_ABL_STRENGTH_NUM];     //TargetStrength TBL along BV
} CCU_AE_ABL_SPEC_T;

typedef struct {
    MBOOL   bEnableNightScene;       // enable night scene
    MUINT32 u4Pcent;                 // 1=0.1%, 0~1000
    MUINT32 u4Thd;                   // 0~255
    MUINT32 u4FlatThd;               // ev difference between darkest and brightest

    MUINT32 u4BrightTonePcent;       // 1=0.1%bright tone percentage
    MUINT32 u4BrightToneThd;         // < 255, bright tone THD

    MUINT32 u4LowBndPcent;           // darkest percent, 1%=10, 0~1000
    MUINT32 u4LowBndThd;             // <255, lower bound target
    MUINT32 u4LowBndThdLimit;        // <255, upper bound of lower bound

    MUINT32 u4FlatBrightPcent;       // the percentage of the brightest part used to cal flatness
    MUINT32 u4FlatDarkPcent;         // the percentage of the darkest part used to cal flatness
    CCU_AE_TARGET_PROB_T    sFlatRatio; //TARGET_NS_FLAT
    CCU_AE_TARGET_PROB_T    sBVRatio;   //FT_NS

    MBOOL bEnableNightSkySuppresion;
    CCU_AE_TARGET_PROB_T    sSkyBVRatio;   //FT_NS_SKY
} CCU_AE_NS_SPEC_T;

typedef struct {
    MUINT32 u4CwrLowBound;                 // metering min CWR value
    MUINT32 u4CwrHighBound;                // metering max CWR value
    MUINT32 u4MeterWeight;              // metering weight betwenn CWR and MeterTarget
    MUINT32 uMeteringYLowBound;
    MUINT32 uMeteringYHighBound;
    MUINT32 u4MeteringStableMax;
    MUINT32 u4MeteringStableMin;
} CCU_AE_TOUCH_FD_SPEC_T;

typedef struct {
    CCU_AE_HS_SPEC_T        rHS_Spec;
    CCU_AE_AOE_SPEC_T       rAOE_Spec;
    CCU_AE_ABL_SPEC_T       rABL_Spec;
    CCU_AE_NS_SPEC_T        rNS_Spec;
    CCU_AE_TOUCH_FD_SPEC_T  rTOUCHFD_Spec;
} CCU_AE_METER_SPEC_T;

typedef struct {
    MUINT32 uPrvFlareWeightArr[16];
    MUINT32 uVideoFlareWeightArr[16];
    MUINT32 u4FlareStdThrHigh;
    MUINT32 u4FlareStdThrLow;
    MUINT32 u4PrvCapFlareDiff;
    MUINT32 u4FlareMaxStepGap_Fast;
    MUINT32 u4FlareMaxStepGap_Slow;
    MUINT32 u4FlarMaxStepGapLimitBV;
    MUINT32 u4FlareAEStableCount;
} CCU_AE_FLARE_T;

typedef struct
{
    MUINT32 u4SpeedUpRatio;
    MUINT32 u4GlobalRatio;
    MUINT32 u4Bright2TargetEnd;
    MUINT32 u4Dark2TargetStart;
    MUINT32 u4B2TEnd;
    MUINT32 u4B2TStart;
    MUINT32 u4D2TEnd;
    MUINT32 u4D2TStart;
} CCU_strAEMovingRatio;

typedef struct {
    MUINT32   u4Y_Target;                     // for AOE target , LE target , SE target -> 47
    MUINT32   u4AOE_OE_percent;         // high light percentage  x / 1000 -> 1%
    MUINT32   u4AOE_OEBound;             // for Over expsosure boud -> 184
    MUINT32   u4AOE_DarkBound;          // for Min luminance bound -> 20
    MUINT32   u4AOE_LowlightPrecent;  // for Lowlight bound percentage / 1000 ,95%
    MUINT32   u4AOE_LowlightBound;     // for Lowlight bound , 95% -> 10
    MUINT32   u4AOESceneLV_L;             // low LV start to reduce AOE -> 100
    MUINT32   u4AOESceneLV_H;             // High LV start to reduce AOE -> 150
    MUINT32   u4AOE_SWHdrLE_Bound;  // LE Condition for SW HDR -> 40
} CCU_strAEAOEAlgParam;

typedef struct
{
    MINT32    i4AOEStrengthIdx;              // AOE strength index: 0 / 1 / 2
    MUINT32    u4BVCompRatio;               // Compensate BV in nonlinear
    CCU_strAEAOEAlgParam rAEAOEAlgParam[CCU_AE_AOE_STRENGTH_NUM];
} CCU_strAEAOEInputParm;

typedef struct
{
    MBOOL   bEnableBlackLight;           // enable back light detector
    MBOOL   bEnableHistStretch;          // enable histogram stretch
    MBOOL   bEnableAntiOverExposure;     // enable anti over exposure
    MBOOL   bEnableTimeLPF;              // enable time domain LPF for smooth converge
    MBOOL   bEnableCaptureThres;         // enable capture threshold or fix flare offset
    MBOOL   bEnableVideoThres;             // enable video threshold or fix flare offset
    MBOOL   bEnableVideo1Thres;       // enable video1 threshold or fix flare offset
    MBOOL   bEnableVideo2Thres;       // enable video2 threshold or fix flare offset
    MBOOL   bEnableCustom1Thres;    // enable custom1 threshold or fix flare offset
    MBOOL   bEnableCustom2Thres;    // enable custom2 threshold or fix flare offset
    MBOOL   bEnableCustom3Thres;    // enable custom3 threshold or fix flare offset
    MBOOL   bEnableCustom4Thres;    // enable custom4 threshold or fix flare offset
    MBOOL   bEnableCustom5Thres;    // enable custom5 threshold or fix flare offset
    MBOOL   bEnableStrobeThres;           // enable strobe threshold or fix flare offset

    MUINT32 u4AETarget;                  // central weighting target
    MUINT32 u4StrobeAETarget;            // central weighting target
    MUINT32 u4InitIndex;                 // AE initiail index

    MUINT32 u4BackLightWeight;           // Back light weighting value
    MUINT32 u4HistStretchWeight;         // Histogram weighting value
    MUINT32 u4AntiOverExpWeight;         // Anti over exposure weighting value

    MUINT32 u4BlackLightStrengthIndex;   // Black light threshold strength index
    MUINT32 u4HistStretchStrengthIndex;  // Histogram stretch threshold strength index
    MUINT32 u4AntiOverExpStrengthIndex;  // Anti over exposure threshold strength index
    MUINT32 u4TimeLPFStrengthIndex;      // Smooth converge threshold strength index
    MUINT32 u4LPFConvergeLevel[AE_CCT_STRENGTH_NUM];  //LPF converge support level

    MUINT32 u4InDoorEV;                  // check the environment indoor/outdoor
    MINT32   i4BVOffset;                  // Calibrate BV offset
    MUINT32 u4PreviewFlareOffset;        // Fix preview flare offset
    MUINT32 u4CaptureFlareOffset;        // Fix capture flare offset
    MUINT32 u4CaptureFlareThres;         // Capture flare threshold
    MUINT32 u4VideoFlareOffset;        // Fix video flare offset
    MUINT32 u4VideoFlareThres;         // video flare threshold
    MUINT32 u4CustomFlareOffset;        // Fix custom flare offset
    MUINT32 u4CustomFlareThres;         // custom flare threshold
    MUINT32 u4StrobeFlareOffset;        // Fix strobe flare offset
    MUINT32 u4StrobeFlareThres;         // strobe flare threshold
    MUINT32 u4PrvMaxFlareThres;        // for max preview flare thres used
    MUINT32 u4PrvMinFlareThres;         // for min preview flare thres used
    MUINT32 u4VideoMaxFlareThres;        // for video max flare thres used
    MUINT32 u4VideoMinFlareThres;         // for video min flare thres used
    MUINT32 u4FlatnessThres;              // 10 base for flatness condition.
    MUINT32 u4FlatnessStrength;

    // v2.0
    CCU_AE_METER_SPEC_T rMeteringSpec;
    CCU_AE_FLARE_T      rFlareSpec;
    CCU_strAEMovingRatio  rAEMovingRatio;         // Preview
    CCU_strAEMovingRatio  rAEVideoMovingRatio;    // Video
    CCU_strAEMovingRatio  rAEVideo1MovingRatio; // Video1 Tracking
    CCU_strAEMovingRatio  rAEVideo2MovingRatio; // Video2 Tracking
    CCU_strAEMovingRatio  rAECustom1MovingRatio; // Custom1 Tracking
    CCU_strAEMovingRatio  rAECustom2MovingRatio; // Custom2 Tracking
    CCU_strAEMovingRatio  rAECustom3MovingRatio; // Custom3 Tracking
    CCU_strAEMovingRatio  rAECustom4MovingRatio; // Custom4 Tracking
    CCU_strAEMovingRatio  rAECustom5MovingRatio; // Custom5 Tracking
    CCU_strAEMovingRatio  rAEFaceMovingRatio;     // Face AE
    CCU_strAEMovingRatio  rAETrackingMovingRatio; // Object Tracking
    CCU_strAEAOEInputParm rAEAOENVRAMParam;
} CCU_AE_CCT_CFG_T;


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
} CCU_strAEParamCFG;

typedef struct
{
    CCU_eWeightingID eID;  //weighting table ID
    MUINT32 W[5][5];    //AE weighting table
} CCU_strWeightTable;

typedef struct
{
    MBOOL bAFPlineEnable;
    MINT16 i2FrameRate[5][2];
} CCU_strAFPlineInfo;

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
} CCU_strAELimiterTable;

// Dynamic frame rate for video
typedef struct
{
    MBOOL   isEnableDFps;
    MUINT32 EVThresNormal;
    MUINT32 EVThresNight;
} CCU_VdoDynamicFrameRate_T;

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

// TG AE speedup tuning parameters
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

    MBOOL   bEnableFaceStableTolerance;     //enable CWV/FDY tolerance mode
    MUINT32 u4FaceStableTolCnt;             //keep converge stage for stable CWV & small FDY variation
    MINT32  u4FaceStableBVThd[4];           //keep converge stage for stable CWV & small FDY variation
    MUINT32 u4FaceStableCWVDiffThd[4];      //keep converge stage for stable CWV & small FDY variation
    MUINT32 u4FaceStableFDYDiffThd[4];      //keep converge stage for stable CWV & small FDY variation
    MUINT32 u4FaceStableOutThd;             //keep converge stage for stable CWV & small FDY variation
} CCU_strAEStableThd;

typedef struct {
    MBOOL u4BVAccEnable;
    MINT32 i4DeltaBVRatio;
    MUINT32 u4B2T_Target;
    CCU_strAEMovingRatio  pAEBVAccRatio;           // Preview ACC ratio
} CCU_strBVAccRatio;

typedef struct {
    MBOOL u4PsoAccEnable;
    MINT32 i4PsoEscRatioWO;  //PSO speedup ratio discount, 100 base (Wash-Out case)
    MINT32 i4PsoEscRatioNonLinear; //PSO speedup ratio discount, 100 base (Non-Linear Case)
    MINT32 ConvergeDeltaPosBound;
    MINT32 ConvergeDeltaNegBound;
} CCU_strPSOConverge;

typedef struct {
    MBOOL    bNonCWRAccEnable;
    MUINT32 u4HS_FHY_HBound;
    MUINT32 u4CWV_HBound;
    MUINT32 u4HS_TargetHBound;
    MUINT32 u4HS_ACCTarget;
} CCU_strNonCWRAcc;

typedef struct {

    MUINT32 u4TmpFDY_HBound;                // tempFDY HighBound
    MUINT32 u4TmpFDY_LBound;                // tempFDY LowBound
    MUINT32 u4FD_Lost_MaxCnt;               // face lost max count
    MUINT32 u4FD_Scale_Ratio;               // face window scale ratio
    MUINT32 u4FD_Lock_MaxCnt;
    MUINT32 u4FD_TemporalSmooth;
    MBOOL   bFD_FaceSizeExt;
    MINT32  i4FD_FaceXMiniBnd;
    MINT32  i4FD_FaceYMiniBnd;
////        HTLU:FACECROP_START       ////
    MUINT32 u4FD_FaceTopCropRat;
    MUINT32 u4FD_FaceBottomCropRat;
    MUINT32 u4FD_FaceLeftCropRat;
    MUINT32 u4FD_FaceRightCropRat;
////        HTLU:FACECROP_END       ////
    MUINT32 u4FD_InToOutThdMaxCnt;
    MUINT32 u4FD_OTLockUpperBnd;
    MUINT32 u4FD_OTLockLowerBnd;
    MUINT32 u4FD_ContinueTrustCnt;
    MUINT32 u4FD_PerframeAntiOverFlag;
    MUINT32 u4FD_SideFaceLock;
    MUINT32 u4FD_LimitStableThdLowBnd;
    MUINT32 u4FD_LimitStableThdLowBndNum;
    MUINT32 u4FD_ReConvergeWhenFaceChange;
    MUINT32 u4FD_FaceMotionLockRat;
    MUINT32 u4FD_ImpulseLockCnt;
    MUINT32 u4FD_ImpulseUpperThd;
    MUINT32 u4FD_ImpulseLowerThd;
    MUINT32 u4FD_ROPTrustCnt;
    MUINT32 u4FD_ReCovergeWhenSizeChangeRat;
    MUINT32 u4FD_ReCovergeWhenPosChangeDist;
////        HTLU:BVDIFF_START       ////
    MUINT32 u4FD_ReCovergeWhenRealBVx1000DiffLarge;
////        HTLU:BVDIFF_END       ////
    MUINT32 u4FD_ReCovergeWhenAFDone;
    MUINT32 u4FD_OTFaceLock;
////        HTLU:PORTRAIT_START       ////
    MBOOL   bCnnFaceEnable;
    MBOOL   bReConvergeWhenCNNFaceBack;
    MINT32  i4PortraitTolerance;
    MINT32  i4FD_CNN_Lock_MaxCnt;
////        HTLU:PORTRAIT_END       ////
////        HTLU:TOUCH_START       ////
    MBOOL   bEVCompExceptionEnable;
    MUINT32 u4EVCompTouchMeteringStableMax;
    MUINT32 u4EVCompTouchMeteringStableMin;
    MUINT32 u4EVCompTouchOutStableThd;
    MUINT32 u4EVCompTouchInStableThd;
////        HTLU:TOUCH_END       ////
////        HTLU:ACCGYRO_START      ////
    MBOOL   bFaceAccLockEnable;
    MUINT32 u4FaceAccLockThd;
    MUINT32 u4FaceAccLock_MaxCnt;
    MBOOL   bFaceGyroDiffLockEnable;
    MUINT32 u4FaceGyroDiffLockThd;
    MUINT32 u4FaceGyroDiffLock_MaxCnt;
////        HTLU:ACCGYRO_END       ////
} CCU_strFaceSmooth;

typedef struct {
    MBOOL bEnableMultiFaceWeight;
    CCU_FACE_PRIOR_ENUM Prior;
    MUINT32 u4FaceWeight[CCU_MAX_AE_METER_AREAS];
} CCU_strFaceWeight;

typedef struct {
    MBOOL   bEnableAEHsTS;                   // enable AE HS Target temp smooth
    MBOOL   bEnableAEAblTS;                  // enable AE ABL Target temp smooth
    MBOOL   bEnableAEAoeTS;                  // enable AE AOE Target temp smooth
    MBOOL   bEnableAENsTS;                   // enable AE NS Target temp smooth
    MBOOL   bEnableAEFaceTS;                 // enable AE Face Target temp smooth
    MBOOL   bEnableAEVideoTS;                // enable AE Video Target temp smooth
    MBOOL   bEnableAEMainTargetTS;                // enable AE Main Target temp smooth
    MBOOL   bEnableAEHsV4p0TargetTS;                // enable AE HSV4p0 Target temp smooth
} CCU_strAECWRTempSmooth;

typedef struct {
    MBOOL   bLandmarkCtrlFlag;              // face landmark ctrol flag
    MUINT32 u4LandmarkCV_Thd;               // face landmark info cv thd
    MINT32  i4LandmarkCV_BV;               // BV thd for face landmark cv 
    MUINT32 u4LandmarkROP_Thd;              // face landmark info rop thd
    MINT32  i4LandmarkROP_BV;               // BV thd for face landmark ROP
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
    MBOOL   u4LandmarkBKMode;
    MUINT32 u4LandmarkBK_CVthd;
    MUINT32 u4LandmarkBK_FYthd;
    MUINT32 u4LandmarkBK_Weight;
    MINT32  i4LandmarkBK_BV;
} CCU_strFaceLandMarkCtrl;

typedef struct
{
    MBOOL   bEnable;                        //enable adaptive AOE THD
    MUINT32 u4TblLength;
    MINT32 i4BV[AE_RATIOTBL_MAXSIZE];
    MUINT32 u4THD[AE_RATIOTBL_MAXSIZE];
} CCU_strAOEBVRef;

typedef struct
{
    MBOOL   bEnable;            //enable NS CDF
    MUINT32 u4Pcent;            // 1=0.1%, 0~1000
    MINT32  i4NS_CDFRatio_X[2];
    MUINT32 u4NS_CDFRatio_Y[2];
} CCU_strNS_CDF;

typedef struct {
    MBOOL   bEnableFlatSkyEnhance;                                // enable HS adaptive THD by realBV
    MUINT8  uSkyDetInThd;
    MUINT8  uSkyDetOutThd;
    MUINT32 u4FlatSkyTHD;
    MUINT32 u4FlatSkyEVD;
    MINT32  u4BVRatio_X[2];
    MINT32  u4BVRatio_Y[2];
} CCU_strHSFlatSkyCFG;

typedef struct {
    MBOOL bEnableABLabsDiff;                                // enable enhance ABL(absDiff)
    MINT32  u4EVDiffRatio_X[2];
    MINT32  u4EVDiffRatio_Y[2];
    MINT32  u4BVRatio_X[2];
    MINT32  u4BVRatio_Y[2];
} CCU_strABL_absDiff;

typedef struct {
    MBOOL bEnableStablebyHist;
    MUINT32 u4HistEVDiff;
    MUINT32 u4OverexpoTHD;
    MUINT32 u4UnderexpoTHD;
    MUINT32 u4HistStableTHD;
} CCU_strHistStableCFG;

typedef struct {
    MBOOL bEnableOverExpoAOE;
    MBOOL bEnableHistOverExpoCnt;
    MBOOL bEnableLTMOE_Link;
    MBOOL bEnableLTMOE_Decrease;
    float fLtm_Cap_Weight;
    MUINT32 u4OElevel;
    MUINT32 u4OEHBoundTblLength;                          //Max size : 8
    MUINT32 u4OE_Ratio_HBound_X[AE_RATIOTBL_MAXSIZE];            //size must large than u4OETblLength
    MUINT32 u4OE_Ratio_HBound_Y[AE_RATIOTBL_MAXSIZE];            //size must large than u4OETblLength
    MUINT32 u4OERatio_LowBnd;
    MUINT32 u4OERatio_HighBnd;

    MINT32 i4OE_BVRatio_X[2];
    MUINT32 u4OE_BVRatio_Y[2];
    MUINT32 u4OETblLength;                                //Max size : 8
    MUINT32 u4OE_Ratio_X[AE_RATIOTBL_MAXSIZE];            //size must large than u4OETblLength
    MUINT32 u4OE_Ratio_Y[AE_RATIOTBL_MAXSIZE];            //size must large than u4OETblLength
    MUINT8 u4OEWeight[AE_V4P0_BLOCK_NO][AE_V4P0_BLOCK_NO];
} CCU_strOverExpoAOECFG;

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
} CCU_strMainTargetCFG;

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
} CCU_strHSv4p0CFG;

typedef struct {
    MBOOL   bEnableGreenSupress;
    MUINT32 u4GreenDetectGRRatio;       //  Green Detect G/R Ratio
    MUINT32 u4GreenDetectGBRatio;       //  Green Detect G/B Ratio
    MUINT32 u4GreenRatio[2];           // Green Detect count weighting TBL X
    MUINT32 u4GreenProb[2];            // Green Detect count weighting TBL Y
    MINT32  i4GreenBv[2];              // Green Detect BV range
    MUINT32 u4GreenBvProb[2];          // Green Detect BV Prob
    MUINT32 u4GreenDetectMin;         // Min brightness of green detect
    MUINT32 u4Hs4p0SupRatio;           // HS4p0 green supress stregth ratio
    MUINT32 u4MainTargetSupRatio;      // MainTarget green supress stregth ratio
    MUINT32 u4TH[6];                  // Reserve for HWTH
} CCU_strGreenSuppression;

typedef struct {
    CCU_strGreenSuppression GreenSup; // Reduce Green scene brightness
} CCU_strColorSuppression;

typedef struct {
    MBOOL   bEnableNSBVCFG;
    MUINT32 u4TblLength;
    MINT32  i4BV[AE_RATIOTBL_MAXSIZE];
    MUINT32 u4BTTHD[AE_RATIOTBL_MAXSIZE];
    MUINT32 u4NSTHD[AE_RATIOTBL_MAXSIZE];
} CCU_strNSBVCFG;


typedef struct {
    MBOOL   bFaceLocSizeCheck;
    MUINT32 u4SizeSmall2Big[2];
    MUINT32 u4ProbSizeSmall2Big[2];
    MUINT32 u4Loc_Near2Far[2];
    MUINT32 u4ProbLocNear2Far[2];
    MBOOL   bLandmarkSize;
    MBOOL   bFaceLocSizeByBv; // Enable LocSize prob change along BV
    MUINT32 u4TblLength; //Max size 16
    MINT32  i4Bv[AE_BVDR_MAXSIZE]; //Bv table, Bv less to More
    MUINT32 u4ProbSizeSmall[AE_BVDR_MAXSIZE]; //Size Prob change along BV
    MUINT32 u4ProbSizeBig[AE_BVDR_MAXSIZE];
    MUINT32 u4ProbLocNear[AE_BVDR_MAXSIZE]; //Loc Prob change along BV
    MUINT32 u4ProbLocFar[AE_BVDR_MAXSIZE];

    //Face Stable by PureCWR
    MBOOL bEnFaceStableByPureCWR; // enable PureCWR ; disable LinkedCWR
    //Face Out stable by BV/DR
    MBOOL bEnFaceStableOutBvDr;
    MUINT32 u4BvDr_BvSize;
    MUINT32 u4BvDr_DrSize;
    MINT32 i4BvDr_BV[6];
    MINT32 i4BvDr_DR[6];
    MUINT32 u4BvDr_FD_OutStable[6][6];
    // MINT32 i4Reverve[AE_BVDR_MAXSIZE * 4]; //Reserved usage
    MINT32 i4Reverve[12]; //Reserved usage
} CCU_strFaceLocSize;

typedef struct {
    MUINT32 u4FdCwrLowBound;               // face boundary min CWR value
    MUINT32 u4FdCwrHighBound;              // face boundary max CWR value
    MINT32  i4FaceBVLowBound;               // face boundary min BV value
    MINT32  i4FaceBVHighBound;              // face boundary max BV value
    MUINT32 uFaceCentralWeight;
    MUINT32 uFaceYLowBound;
    MUINT32 uFaceYHighBound;
    MUINT32 u4FaceNSLowBound;       //NS face target Y
} CCU_strFaceTarget;

typedef struct {
    MUINT32 u4BVSize;      //max size AE_RATIOTBL_MAXSIZE
    MUINT32 u4DRSize;      //max size AE_RATIOTBL_MAXSIZE
    MINT32  i4BV[AE_BVDR_MAXSIZE];  // Parameter index BV
    MUINT32 u4_OE_SkipPcent[AE_BVDR_MAXSIZE]; // Tolerance for overexposure
    MUINT32 u4_OE_Pcent[AE_BVDR_MAXSIZE]; // OE region behind tolerance region
    MINT32  i4DR[AE_BVDR_MAXSIZE];  // Parameter index DR
    MUINT32 u4_OE_TH_Tbl[AE_BVDR_MAXSIZE][AE_BVDR_MAXSIZE]; // OE brightness target
    MUINT32 u4_FD_TH_Tbl[AE_BVDR_MAXSIZE][AE_BVDR_MAXSIZE]; // Face brightness target
    MUINT32 u4_FD_MIN_TH_Tbl[AE_BVDR_MAXSIZE][AE_BVDR_MAXSIZE]; // Minimum face brightness target
} CCU_strFaceBalancedTarget;

typedef struct {
    MBOOL bEnableFBT;
    MBOOL bFloatingPcentDR;
    CCU_strFaceBalancedTarget Normal;
    CCU_strFaceBalancedTarget Night;
} CCU_strFaceBalancedTargetCFG;

typedef struct {
    MUINT32 u4FaceSortRa; // FD change ration for size sorting
    MUINT32 u4FaceSortStbCnt;// FD size sorting stable counting
}CCU_strFaceSorting;

typedef struct {
    MBOOL   bEnableFaceMotionDetect; //enable FD motion detection
    MUINT32 u4FaceMotionRa_FDIN; //FD motion ratio for FDIN
    MUINT32 u4FaceMotionRa_FDCONTINUE;//FD motion ratio for FDCONTINUE
    MUINT8  u4FDMotionFrame_FDIN;//FD motion frame number for FDIN
    MUINT8  u4FDMotionFrame_FDCONTINUE;//FD motion frame number for FDCONTINUE
}CCU_strFaceMotion;

typedef struct {
    MBOOL   bEnableFaceSizeDetect; //enable FD size variation detection
    MUINT32 u4FaceSizeDiffTh_W_FDIN; //Face width size diff for FDIN
    MUINT32 u4FaceSizeDiffTh_H_FDIN; //Face height size diff for FDIN
    MUINT32 u4FaceSizeDiffTh_W_FDCONTINUE;//Face width size diff for FDCONTINUE
    MUINT32 u4FaceSizeDiffTh_H_FDCONTINUE;//Face height size diff for FDCONTINUE
    MUINT8  u4FDSizeDiffFrame_FDIN;//FD size record frame number for FDIN
    MUINT8  u4FDSizeDiffFrame_FDCONTINUE;//FD size record  frame number for FDCONTINUE
    MBOOL   bEnableFaceBndLock;//FD ROI protection
    MUINT32 u4EndBnkUpperBound;//boundary threshold
}CCU_strFaceSizeVar;

typedef struct {
    MUINT32  u4FaceRbTrustCnt;//Face robust trust cnt for face LCE link
    MUINT32  u4FaceDropTempSmoothCnt;//Face drop smooth cnt for face LCE link
    MUINT32  u4OTFaceTrustCnt_FDIN;//Face OT trust cnt and converge last robust face target
    MUINT32  u4OTFaceTrustCnt_FDCONTINUE;//Face OT trust cnt and converge last robust face target
    MBOOL   bOTFaceTimeOutLockAE;//OT face time out lock ae
    MUINT32 u4Reserve1;
    MUINT32 u4Reserve2;
    MUINT32 u4Reserve3;
    MUINT32 u4Reserve4;
    MUINT32 u4Reserve5;
}CCU_strFaceLCELink;

typedef struct {
    MBOOL   bEnableContrastLink;
    CCU_strFaceLCELink FDLCELinkParam; //Face LCE link param

    MINT32 i4LTMFaceProtectPercent; // Face LTM - face protection percent (Range: 0~100)
    MINT32 i4LTMFaceBrightTonePercent; // Face LTM - face bright tone avg percent (Range: 0~100)
    MINT32 i4LTMTopCropRatio; // Face LTM - FD top crop ratio (Range: 0~100)
    MINT32 i4LTMBottomCropRatio; // Face LTM - FD bottom crop ratio (Range: 0~100)
    MINT32 i4LTMLeftCropRatio; // Face LTM - FD left crop ratio (Range: 0~100)
    MINT32 i4LTMRightCropRatio; // Face LTM - FD right crop ratio (Range: 0~100)
    MUINT32 m_u4LCEMaxGain[5]; //Face LCE link maximum gain
    //ltm weight dynamic control   
    MBOOL   bEnableDynmicLtmWeight;                                                 //Enable dynamic ltm weight
    MUINT32 u4OELTM_Weight_TblLength;                                               //Max size :8
    MUINT32 u4OE_LTM_Weight_BV_Tbl[AE_RATIOTBL_MAXSIZE];                            //size must large than u4OELTM_Weight_BVTbable
    MUINT32 u4OE_LTM_Weight_OE_Ratio_Tbl[AE_RATIOTBL_MAXSIZE];                      //size must large than u4OELTM_Weight_OE_TblLength
    MUINT32 u4OE_LTM_Weight_OE_Prob_Tbl[AE_RATIOTBL_MAXSIZE][AE_RATIOTBL_MAXSIZE];  //ProbTbl[u4OELTM_Weight_TblLength][u4OELTM_Weight_TblLength]    
    MBOOL bBTEnable;   // Butterfly effect enable
    MUINT32 u4BT_TH;     //Butterfly effect Threshold
    MUINT32 u4BT_SPEED; //Butterfly effect Speed
    MUINT32 u4BtChangeCnt; //Butterfly change tolence
    MUINT32 u4BtUpBound;   //Butterfly Out upper bound
    MUINT32 u4BtLowBound;  //Butterfly  Out low bound
    MUINT32 u4ResContrastLink[40]; //Reserve for other contrast Link param
} CCU_strContrastLink;

typedef struct {
    MBOOL  bEnableFolivoraEffect;
	MINT32 i4BvTable[AE_STABLE_BV_MAXSIZE];
	MINT32 i4FolivoraBMovingTH[AE_STABLE_BV_MAXSIZE];
	MINT32 i4FolivoraFMovingTH[AE_STABLE_BV_MAXSIZE];
	MINT32 i4FolivoraFMovingQuantityTH[AE_STABLE_BV_MAXSIZE];
	MINT32 i4FolivoraDarkMotionTH[AE_STABLE_BV_MAXSIZE];
	MINT32 i4FolivoraDarkMinMotionTH[AE_STABLE_BV_MAXSIZE];
	MINT32 i4FolivoraTrackingTH[AE_STABLE_BV_MAXSIZE];
	MINT32 i4FolivoraHesitateTH[AE_STABLE_BV_MAXSIZE];
} CCU_strFolivoraEffectPara;

/* HDR-AE Params for MHDR 3EXPs Start
*
*/

typedef struct {
    // BV vs Target Table
    MUINT32 u4TblLength;                         // max is 8
    MINT32  i4BV4HDRTarget[AE_RATIOTBL_MAXSIZE]; // | -1000 | 1000 | 3500 | 8000 |
    MUINT32 u4HDRTarget[AE_RATIOTBL_MAXSIZE];    // |    50 |   55 |   58 |   60 |
} CCU_strMvhdrTargetCFG;

typedef struct {
    // BV vs FDTarget Table
    MUINT32 u4TblLength;                           // max is 8
    MINT32  i4BV4HDRFDTarget[AE_RATIOTBL_MAXSIZE];   // | -2000 |    0 |  400 |
    MUINT32 u4HDRFDTarget[AE_RATIOTBL_MAXSIZE];    // |    40 |   35 |   30 |
} CCU_strMvhdrFDTargetCFG;

typedef struct{
    MUINT32 u4B2DLinearThd;
    MINT32 i4B2DBVACCExtreamBound;
    MINT32 i4B2DBVACCNonlinearBound;
    MINT32 i4B2DNonlinearEVDBound;
    MINT32 i4B2DLinearEVDBound;

    MUINT32 u4D2BLinearThd;
    MINT32 i4D2BNonlinearEVDBound;
    MINT32 i4D2BLinearEVDBound;
} CCU_strHdrSmoothCFG;

typedef struct{
    MBOOL   bEnable;
    MUINT32 u4TblLength;
    MUINT32 u4RatioPcent_X[AE_RATIOTBL_MAXSIZE];
    MUINT32 u4RatioPcent_Y[AE_RATIOTBL_MAXSIZE];
} CCU_str3ExpomVHdrRatioSmoothTbl;

typedef struct{
    MBOOL   bEnable;
    MUINT32 u4TblLength;
    MUINT32 u4RatioLimit_X[AE_RATIOTBL_MAXSIZE];
    MUINT32 u4RatioLimit_Y[AE_RATIOTBL_MAXSIZE];
} CCU_str3ExpomVHdrRatioLimitTbl;

typedef struct {
    CCU_str3ExpomVHdrRatioSmoothTbl  SmoothRatioTbl;
    CCU_str3ExpomVHdrRatioLimitTbl LimitRatioTbl;
} CCU_str3ExpomVHdrCFG;

typedef struct {
    // Conditions for HDR Maximum Ratio Control
    MINT32  i4MaxHDRRatio;        // 6400 stands for 64x [LE/SE]
    MINT32  i4MaxHDRRatio_Face;   // 6400 stands for 64x [LE/SE]
    MUINT32 u4TblLength;         // max is 8
    MINT32  i4BV4MaxHDRRatio[AE_RATIOTBL_MAXSIZE]; // | -1000 | 1000 | 3500 | 8000 |
    MUINT32 u4MaxHDRRatio_BV[AE_RATIOTBL_MAXSIZE]; // |  100x | 200x | 400x | 800x |
} CCU_strMaxHdrRatioCFG;

typedef struct {
    // for HDR Ratio Converge
    MINT32 i4HDRRatioInTolerance;  // 0.15*100
    MINT32 i4HDRRatioOutTolerance;  // 0.30*100
    MINT32 i4HDRStableCountTHD;
} CCU_strHdrRatioConvergeCFG;

typedef struct {
    // Method1 : HL Avg Control
    MBOOL bEnableHLAvgRatioControl;   // Enable HLAvg Control
    MUINT32 u4HDRHLRatio;             // 20 means 20%
    MUINT32 u4HDRHLTarget;            // 100 for 256

    // Method2 : OE Hist Control
    MBOOL bEnableHistOERatioControl;                // Enable HistOE Control
    MUINT32 u4TblLengthOE1;                         // max is 8
    MINT32  i4OE1Pct[AE_RATIOTBL_MAXSIZE];          // |    0  |    5  |   10  |   20  |   30  |
    MINT32  i4RatioMovOE1[AE_RATIOTBL_MAXSIZE];     // |    0  |    0  |    0  |  100  |  150  |
    MUINT32 u4TblLengthOE2;                          // max is 8
    MINT32  i4OE2Pct[AE_RATIOTBL_MAXSIZE];          // |    0  |    5  |   10  |   20  |   30  |
    MINT32  i4RatioMovOE2[AE_RATIOTBL_MAXSIZE];     // | -100  |    0  |    0  |    0  |    0  |
    MUINT32 u4OEBin[2];                             // Last 2Bin Position[1 | 129]

} CCU_strHdrRatioTargetCFG;

typedef struct {
    // Conditions for Ratio Decrease : BV & OE Ratio
    MBOOL bEnableBVDecrease;                              // Enable BV Decrease
    MUINT32 u4BVTblLength;                                // max is 8
    MINT32  i4BV4HDRRatioDecrease[AE_RATIOTBL_MAXSIZE];   // | -1000 | 1000 | 3500 | 8000 |
    MUINT32 u4RatioDecreaseRate4BV[AE_RATIOTBL_MAXSIZE];  // |  50%  | 80%  | 100% | 100% |

    // Calculate (OE_10bins/ALL_256bins) Ratio
    MBOOL bEnableOEDecrease;                              // Enable OE Decrease
    MUINT32 u4OEBinN;                                     // OE : 255~(255-OEBinN)
    MUINT32 u4OETblLength;                                // max is 8
    MUINT32 u4OERate4HDRDecrease[AE_RATIOTBL_MAXSIZE];    // | 10% | 20% | 30% | 40% |
    MUINT32 u4RatioDecreaseRate4OE[AE_RATIOTBL_MAXSIZE];  // | 80% | 80% | 100%| 100%|
} CCU_strHdrRatioDecCFG;

typedef struct {
    CCU_strMaxHdrRatioCFG HdrMaxRatioInfo;

    // for Ratio Moving Start
    CCU_strHdrRatioConvergeCFG HdrRatioConvergeInfo;

    // HDR Ratio Control
    CCU_strHdrRatioTargetCFG HdrRatioTargetInfo;
    CCU_strHdrRatioDecCFG HdrRatioDecreaseInfo;
    MINT32 i4HdrExpTransMethod;         //0: exp first. 1: gain first
} CCU_strHdrRatioCFG;

typedef struct
{
    MBOOL   bEnableMultiStepHS;         //Enable HS enhance method : MultiStep HS(Conflict with ori HS)
    MUINT32 u4TblLength;                                //Max size : 16
    MUINT32 u4MS_EVDiff[AE_RATIOTBL_MAXSIZE];            //size must large than u4TblLength
    MUINT32 u4MS_OutdoorTHD[AE_RATIOTBL_MAXSIZE];        //size must large than u4TblLength
    MUINT32 u4MS_IndoorTHD[AE_RATIOTBL_MAXSIZE];         //size must large than u4TblLength
    MINT32  i4MS_BVRatio[2];                      //MS_HS In/Out door BV threshold
} CCU_strMHDRHSMultiStep;

typedef struct {
    MUINT32 u4FDAvgRatio_X[2];
    MUINT32 u4FDAvgRatio_Y[2];
} CCU_strHdrFDAvgRatioTbl;

typedef struct {
    MBOOL   bHdrPre2CapComp;
    MUINT32 u4HdrPre2CapCompMethod;     //0:None 1:Offset 2:FDRatio 3:Offset+FDRatio
    CCU_strHdrFDAvgRatioTbl HdrFDAvgRatioTbl;
} CCU_strHdrPre2CapComp;

typedef struct {
    MUINT32 u4FlareSmallDeltaIdxStep;
    MUINT32 u4CWVChangeStep;
    MUINT32 u4AllStableStep;
    MBOOL  bOutdoorFlareEnable;
    MUINT32 u4OutdoorMaxFlareThres;
    MINT32  i4BVRatio[2];
} CCU_strDynamicFlare;

typedef struct
{
    MINT32 ae_cycle;
    MINT32 small_range_idx;
    MINT32 face_small_range_idx;
    MINT32 inverse_protect_ratio;

    MINT32 extreme_over_perframe_ratio;
    MINT32 extreme_over_non_linear_cwv_ratio_x[2];
    MINT32 extreme_over_non_linear_cwv_ratio_y[2];

    MINT32 extreme_under_perframe_ratio;
    MINT32 extreme_under_non_linear_cwv_ratio_x[2];
    MINT32 extreme_under_non_linear_cwv_ratio_y[2];

    MINT32 over_bv_th_enable;
    MINT32 over_tbl_length;
    MINT32 over_bv_th[8];
    MINT32 over_perframe_ratio[8];
    MINT32 over_perframe_ratio_default;

    MINT32 over_limit_delta_index;
    MINT32 over_prob_ratio_x[2];
    MINT32 over_prob_ratio_y[2];
    MINT32 over_non_linear_cwv_ratio_x[2];
    MINT32 over_non_linear_cwv_ratio_y[2];
    MINT32 over_non_linear_avg_ratio_x[2];
    MINT32 over_non_linear_avg_ratio_y[2];

    MINT32 over_pointlight_perframe_ratio;
    MINT32 over_pointlight_limit_delta_index;
    MINT32 over_pointlight_prob_ratio_x[2];
    MINT32 over_pointlight_prob_ratio_y[2];
    MINT32 over_pointlight_non_linear_cwv_ratio_x[2];
    MINT32 over_pointlight_non_linear_cwv_ratio_y[2];
    MINT32 over_pointlight_non_linear_avg_ratio_x[2];
    MINT32 over_pointlight_non_linear_avg_ratio_y[2];

    MINT32 pointlight_enable_delta_idx;
    MINT32 pointlight_perframe_ratio;
    MINT32 pointlight_limit_delta_index;
    MINT32 pointlight_prob_ratio_x[2];
    MINT32 pointlight_prob_ratio_y[2];
    MINT32 pointlight_non_linear_cwv_ratio_x[2];
    MINT32 pointlight_non_linear_cwv_ratio_y[2];
    MINT32 pointlight_non_linear_cwv_stop_ratio_x[2];
    MINT32 pointlight_non_linear_cwv_stop_ratio_y[2];

    MINT32 normal_bv_th_enable;
    MINT32 normal_tbl_length;
    MINT32 normal_bv_th_b2t[8];
    MINT32 normal_perframe_ratio_b2t[8];
    MINT32 normal_bv_th_d2t[8];
    MINT32 normal_perframe_ratio_d2t[8];
    MINT32 normal_perframe_ratio_b2t_default;
    MINT32 normal_perframe_ratio_d2t_default;

    MINT32 normal_non_linear_cwv_ratio_x[2];
    MINT32 normal_non_linear_cwv_ratio_y[2];
    MINT32 normal_non_linear_avg_ratio_x[2];
    MINT32 normal_non_linear_avg_ratio_y[2];
    MINT32 normal_delta_idx_ratio_x[2];
    MINT32 normal_delta_idx_ratio_y[2];
    MINT32 normal_enable_delta_idx_ratio_thd;

    MINT32 extreme_overexp_perc;
    MINT32 extreme_underexp_perc;
    MINT32 overexp_prob_high_bound_perc;
    MINT32 overexp_prob_non_linear_ratio_th;
    MINT32 overexp_prob_light_pixel_th;
    MINT32 overexp_prob_cwv_th;
    MINT32 overexp_pl_prob_high_bound_perc;
    MINT32 overexp_pl_prob_low_bound_perc;
    MINT32 overexp_pl_prob_bv_th;
    MINT32 overexp_pl_prob_non_linear_ratio_th;
    MINT32 overexp_pl_prob_linear_cwv_th;
    MINT32 overexp_pl_prob_over_perc;
    MINT32 overexp_pl_prob_light_pixel_perc;
    MINT32 overexp_pl_prob_dark_pixel_perc;
    MINT32 pl_prob_low_bound_perc;
    MINT32 pl_prob_high_bound_perc;
    MINT32 pl_prob_black_bound_perc;
    MINT32 pl_prob_EVD_th;
    MINT32 pl_prob_bv_th;
    MINT32 pl_prob_mid_pixel_perc;
    MINT32 pl_prob_dark_pixel_perc;
    MINT32 pl_prob_light_pixel_dw_perc;
    MINT32 pl_prob_light_pixel_up_perc;
    MINT32 pl_prob_black_pixel_perc;
    MINT32 face_comp_ratio;
    MINT32 face_perframe_ratio_b2t_x[2];
    MINT32 face_perframe_ratio_b2t_y[2];
    MINT32 face_perframe_ratio_d2t_x[2];
    MINT32 face_perframe_ratio_d2t_y[2];
    MINT32 smooth_gyro_enable;
    MINT32 gyro_on_th;
    MINT32 gyro_x[2];
    MINT32 gyro_y[2];

    MINT32 touch_extreme_over_perframe_ratio;
    MINT32 touch_extreme_over_non_linear_cwv_ratio_x[2];
    MINT32 touch_extreme_over_non_linear_cwv_ratio_y[2];

    MINT32 touch_extreme_under_perframe_ratio;
    MINT32 touch_extreme_under_non_linear_cwv_ratio_x[2];
    MINT32 touch_extreme_under_non_linear_cwv_ratio_y[2];

    MINT32 touch_normal_bv_th_enable;
    MINT32 touch_normal_tbl_length;
    MINT32 touch_normal_bv_th_b2t[8];
    MINT32 touch_normal_perframe_ratio_b2t[8];
    MINT32 touch_normal_bv_th_d2t[8];
    MINT32 touch_normal_perframe_ratio_d2t[8];
    MINT32 touch_normal_perframe_ratio_b2t_default;
    MINT32 touch_normal_perframe_ratio_d2t_default;

    MINT32 touch_normal_non_linear_cwv_ratio_x[2];
    MINT32 touch_normal_non_linear_cwv_ratio_y[2];
    MINT32 touch_normal_non_linear_avg_ratio_x[2];
    MINT32 touch_normal_non_linear_avg_ratio_y[2];
    MINT32 touch_normal_delta_idx_ratio_x[2];
    MINT32 touch_normal_delta_idx_ratio_y[2];
    MINT32 touch_normal_enable_delta_idx_ratio_thd;

    MINT32 temp_smooth_small_perc;
    MINT32 temp_smooth_face_perc;
    MINT32 temp_smooth_perc;

    MINT32 normal_detal_idx_en;
    MINT32 normal_detal_idx_b2t_x[2];
    MINT32 normal_detal_idx_b2t_y[2];
    MINT32 normal_detal_idx_d2t_x[2];
    MINT32 normal_detal_idx_d2t_y[2];

    MINT32 face_max_step_enable;
    MINT32 face_step_bv_th[8];
    MINT32 face_max_step[8];
    MINT32 face_tbl_length;

    MINT32 one_step_idx_enable;
    MINT32 one_step_idx_in_th;
    MINT32 one_step_idx_out_th;
    MINT32 one_step_linear_avg_th;
    MINT32 one_step_angle_th;

    MINT32 temp_smooth_one_step_perc;
    MINT32 temp_smooth_cnt_perc;
    MINT32 temp_smooth_cnt;
    MINT32 i4Revs[50];
} CCU_AE_SMOOTH_V6P0_CFG;

typedef struct
{
    MBOOL bEnableLS;
    MUINT32 u4TblLength;
    MINT32 i4LS_Bv[AE_RATIOTBL_MAXSIZE];
    MINT32 i4LS_Offset[AE_RATIOTBL_MAXSIZE];
} CCU_strLightSensor;

typedef struct
{
    MBOOL bEnableAisControl;
    MUINT32 u4Mode;
    MUINT32 u4Bv_TblLength;
    MUINT32 u4Motion_TblLength;
    MINT32 i4Bv_Tbl[AE_AISBV_MAXSIZE];
    MINT32 i4Motion_Tbl[AE_RATIOTBL_MAXSIZE];
    MUINT32 u4Shutter_Thd[AE_AISBV_MAXSIZE][AE_RATIOTBL_MAXSIZE];
    MUINT32 u4Iso_Thd[AE_AISBV_MAXSIZE][AE_RATIOTBL_MAXSIZE];
} CCU_strAisControl;

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
    CCU_strHistStableCFG  HistStableCFG;
    CCU_strOverExpoAOECFG OverExpoAOECFG;
    //v4.0
    MBOOL bAEv4p0MeterEnable;
    CCU_strMainTargetCFG MainTargetCFG;
    CCU_strHSv4p0CFG HSv4p0CFG;
    CCU_strColorSuppression ColorSup;
    CCU_strNSBVCFG NSBVCFG;

    //v4.1
    CCU_strAEMovingRatio AEACCMovingRatio;   //(temp) acc smooth ratio

    //v6.0
    CCU_strFaceLocSize FaceLocSizecheck;
    CCU_strFaceTarget FdTarget;
    CCU_strFaceBalancedTargetCFG FBTParam; // Face balanced target parameter
    CCU_strFaceSorting FDSortParam;
    CCU_strFaceMotion  FDMotionParam;
    CCU_strFaceSizeVar FDSizeVarParam;
    CCU_strContrastLink ContrastLinkParam; //For Contrast link control
	CCU_strFolivoraEffectPara FolivoraEffectPara;

    // HDR
    // CCU_HDR_AE_SUPPORT +++
    CCU_strMvhdrTargetCFG HdrTargetInfo;
    CCU_strMvhdrFDTargetCFG HdrFDTargetInfo;
    CCU_strHdrSmoothCFG HdrSmoothCFG;
    CCU_str3ExpomVHdrCFG ThreeExpomVHdrCFG;
    CCU_strHdrRatioCFG HdrRatioControl;
    CCU_strMHDRHSMultiStep HdrHsMultiStep;
    CCU_strHdrPre2CapComp HdrPre2CapCompCFG;
    MINT32 HdrReserve[256];
    // CCU_HDR_AE_SUPPORT ---

    // Stereo Pline mapping
    CCU_strAESceneMapping strStereoPlineMapping;
    // HDR Pline mapping
    CCU_strAESceneMapping strHDRPlineMapping;

    //Dynamic Flare
    CCU_strDynamicFlare DynamicFlareCFG;

    //AE smooth
    CCU_AE_SMOOTH_V6P0_CFG smooth_cfg_v6p0;

    // Light Sensor
    CCU_strLightSensor strLightSensor;

    // Motion aware shutter control
    CCU_strAisControl strAisControl;

    // Tuning version
    MUINT32 u4TuningVersion;
}CCU_AE_PARAM_T;

typedef struct
{
    MUINT32 u4TopProtect;
    CCU_AE_DEVICES_INFO_T rDevicesInfo;
    CCU_AE_HIST_CFG_T rHistConfig;
    CCU_AE_CCT_CFG_T rCCTConfig;
    CCU_AE_PARAM_T rAeParam;
    MUINT32 u4BottomProtect;
} CCU_AE_NVRAM_T;

#endif

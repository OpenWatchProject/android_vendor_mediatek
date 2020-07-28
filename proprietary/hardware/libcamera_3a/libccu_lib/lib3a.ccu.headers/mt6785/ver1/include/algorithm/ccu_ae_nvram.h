#ifndef _CCU_AE_NVRAM_H_
#define _CCU_AE_NVRAM_H_

#include "ccu_ext_interface/ccu_types.h"

//histogram control information
#define AE_CCT_STRENGTH_NUM (5)
#define AE_AOE_STRENGTH_NUM (3)

typedef struct
{
   //histogram info
    UINT32 u4HistHighThres;                         // central histogram high threshold
    UINT32 u4HistLowThres;                          // central histogram low threshold
    UINT32 u4MostBrightRatio;                       // full histogram high threshold
    UINT32 u4MostDarkRatio;                         // full histogram low threshold
    UINT32 u4CentralHighBound;                      // central block high boundary
    UINT32 u4CentralLowBound;                       // central block low bounary
    UINT32 u4OverExpThres[AE_CCT_STRENGTH_NUM];     // over exposure threshold
    UINT32 u4HistStretchThres[AE_CCT_STRENGTH_NUM]; // histogram stretch trheshold
    UINT32 u4BlackLightThres[AE_CCT_STRENGTH_NUM];  // backlight threshold
} CCU_AE_HIST_CFG_T;


typedef struct {
    MINT32 u4X1;
    MINT32 u4Y1;
    MINT32 u4X2;
    MINT32 u4Y2;
} CCU_AE_TARGET_PROB_T; // useful check ok

typedef struct {
    MBOOL   bEnableHistStretch;          // enable histogram stretch
    UINT32 u4HistStretchWeight;         // Histogram weighting value
    UINT32 u4Pcent;                     // 1%=10, 0~1000 //change type
    UINT32 u4Thd;                       // 0~255 //change type
    UINT32 u4FlatThd;                   // 0~255 //change type

    UINT32 u4FlatBrightPcent;          //change type
    UINT32 u4FlatDarkPcent;            //change type
    CCU_AE_TARGET_PROB_T    sFlatRatio;     //TARGET_HS_FLAT

    MBOOL  bEnableGreyTextEnhance;
    MUINT32 u4GreyTextFlatStart;
    CCU_AE_TARGET_PROB_T  sGreyTextRatio;   //TARGET_HS_COLOR
} CCU_AE_HS_SPEC_T; // useful check ok

typedef struct {
    MBOOL   bEnableAntiOverExposure;     // enable anti over exposure
    UINT32 u4AntiOverExpWeight;         // Anti over exposure weighting value
    UINT32 u4Pcent;                     // 1%=10, 0~1000 //change type
    UINT32  u4Thd;                       // 0~255         //change type

    MBOOL bEnableCOEP;                   // enable COEP
    UINT32 u4COEPcent;                  // center over-exposure prevention
    UINT32 u4COEThd;                    // center y threshold
    UINT32 u4BVCompRatio;               // Compensate BV in nonlinear
    CCU_AE_TARGET_PROB_T    sCOEYRatio;     // the outer y ratio
    CCU_AE_TARGET_PROB_T    sCOEDiffRatio;  // inner/outer y difference ratio
} CCU_AE_AOE_SPEC_T; // useful check ok

typedef struct {
    MBOOL   bEnableBlackLight;           // enable back light detector
    UINT32 u4BackLightWeight;           // Back light weighting value
    UINT32 u4Pcent;                     // 1%=10, 0~1000 //change type
    UINT32 u4Thd;                       // 0~255          //change type

    UINT32 u4CenterHighBnd;             // center luminance
    UINT32 u4TargetStrength;            // final target limitation
    CCU_AE_TARGET_PROB_T    sFgBgEVRatio;   //TARGET_ABL_DIFF
    CCU_AE_TARGET_PROB_T    sBVRatio;       //FT_ABL
} CCU_AE_ABL_SPEC_T; // useful check ok

typedef struct {
    MBOOL   bEnableNightScene;       // enable night scene
    UINT32 u4Pcent;                 // 1=0.1%, 0~1000 //change type
    UINT32 u4Thd;                   // 0~255 //change type
    UINT32 u4FlatThd;               // ev difference between darkest and brightest //change type

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
} CCU_AE_NS_SPEC_T; // useful check ok
typedef struct {
    UINT32 uMeteringYLowBound;
    UINT32 uMeteringYHighBound;
    UINT16 uFaceYLowBound;
    UINT32 uFaceYHighBound;
    UINT32 uFaceCentralWeight;
    UINT32 u4MeteringStableMax;
    UINT32 u4MeteringStableMin;
} CCU_AE_TOUCH_FD_SPEC_T;

typedef struct {
    CCU_AE_HS_SPEC_T        rHS_Spec;
    CCU_AE_AOE_SPEC_T       rAOE_Spec;
    CCU_AE_ABL_SPEC_T       rABL_Spec;
    CCU_AE_NS_SPEC_T        rNS_Spec;
    CCU_AE_TOUCH_FD_SPEC_T  rTOUCHFD_Spec;
    //MUINT8 uFaceCentralWeight;
    ///#^
} CCU_AE_METER_SPEC_T; // useful check ok
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
} CCU_strAEMovingRatio;//Scenario pline runtime info

typedef struct
{
    UINT32 u4MinGain;
    UINT32 u4MaxGain;
    UINT32 u4MiniISOGain;
    UINT32 u4GainStepUnit;
    UINT32 u4PreExpUnit;
    UINT32 u4PreMaxFrameRate;
    UINT32 u4VideoExpUnit;
    UINT32 u4VideoMaxFrameRate;
    UINT32 u4Video2PreRatio;    // 1x = 1024
    UINT32 u4CapExpUnit;
    UINT32 u4CapMaxFrameRate;
    UINT32 u4Cap2PreRatio;     // 1x = 1024
    UINT32 u4Video1ExpUnit;
    UINT32 u4Video1MaxFrameRate;
    UINT32 u4Video12PreRatio;    // 1x = 1024
    UINT32 u4Video2ExpUnit;
    UINT32 u4Video2MaxFrameRate;
    UINT32 u4Video22PreRatio;    // 1x = 1024
    UINT32 u4Custom1ExpUnit;
    UINT32 u4Custom1MaxFrameRate;
    UINT32 u4Custom12PreRatio;    // 1x = 1024
    UINT32 u4Custom2ExpUnit;
    UINT32 u4Custom2MaxFrameRate;
    UINT32 u4Custom22PreRatio;    // 1x = 1024
    UINT32 u4Custom3ExpUnit;
    UINT32 u4Custom3MaxFrameRate;
    UINT32 u4Custom32PreRatio;    // 1x = 1024
    UINT32 u4Custom4ExpUnit;
    UINT32 u4Custom4MaxFrameRate;
    UINT32 u4Custom42PreRatio;    // 1x = 1024
    UINT32 u4Custom5ExpUnit;
    UINT32 u4Custom5MaxFrameRate;
    UINT32 u4Custom52PreRatio;    // 1x = 1024
    UINT32 u4LensFno;           // 10 Base
    UINT32 u4FocusLength_100x;           // 100 Base
} CCU_AE_DEVICES_INFO_T;

typedef struct {
    UINT32 uPrvFlareWeightArr[16];
    UINT32 uVideoFlareWeightArr[16];
    UINT32 u4FlareStdThrHigh;
    UINT32 u4FlareStdThrLow;
    UINT32 u4PrvCapFlareDiff;
    UINT32 u4FlareMaxStepGap_Fast;
    UINT32 u4FlareMaxStepGap_Slow;
    UINT32 u4FlarMaxStepGapLimitBV;
    UINT32 u4FlareAEStableCount;
} CCU_AE_FLARE_T;

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

#define CCU_AE_AOE_STRENGTH_NUM (3)

typedef struct
{
    MINT32    i4AOEStrengthIdx;              // AOE strength index: 0 / 1 / 2
    UINT32    u4BVCompRatio;               // Compensate BV in nonlinear
    CCU_strAEAOEAlgParam rAEAOEAlgParam[CCU_AE_AOE_STRENGTH_NUM];
} CCU_strAEAOEInputParm;

//strAETable AE table Setting
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

    UINT32 u4AETarget;                  // central weighting target
    UINT32 u4StrobeAETarget;            // central weighting target
    UINT32 u4InitIndex;                 // AE initiail index

    UINT32 u4BackLightWeight;           // Back light weighting value
    UINT32 u4HistStretchWeight;         // Histogram weighting value
    UINT32 u4AntiOverExpWeight;         // Anti over exposure weighting value

    UINT32 u4BlackLightStrengthIndex;   // Black light threshold strength index
    UINT32 u4HistStretchStrengthIndex;  // Histogram stretch threshold strength index
    UINT32 u4AntiOverExpStrengthIndex;  // Anti over exposure threshold strength index
    UINT32 u4TimeLPFStrengthIndex;      // Smooth converge threshold strength index
    UINT32 u4LPFConvergeLevel[AE_CCT_STRENGTH_NUM];  //LPF converge support level

    UINT32 u4InDoorEV;                  // check the environment indoor/outdoor
    MINT32   i4BVOffset;                  // Calibrate BV offset
    UINT32 u4PreviewFlareOffset;        // Fix preview flare offset
    UINT32 u4CaptureFlareOffset;        // Fix capture flare offset
    UINT32 u4CaptureFlareThres;         // Capture flare threshold
    UINT32 u4VideoFlareOffset;        // Fix video flare offset
    UINT32 u4VideoFlareThres;         // video flare threshold
    UINT32 u4CustomFlareOffset;        // Fix custom flare offset
    UINT32 u4CustomFlareThres;         // custom flare threshold
    UINT32 u4StrobeFlareOffset;        // Fix strobe flare offset
    UINT32 u4StrobeFlareThres;         // strobe flare threshold
    UINT32 u4PrvMaxFlareThres;        // for max preview flare thres used
    UINT32 u4PrvMinFlareThres;         // for min preview flare thres used
    UINT32 u4VideoMaxFlareThres;        // for video max flare thres used
    UINT32 u4VideoMinFlareThres;         // for video min flare thres used
    UINT32 u4FlatnessThres;              // 10 base for flatness condition.
    UINT32 u4FlatnessStrength;

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
} CCU_AE_CCT_CFG_T;  // useful check ok

#define CCU_AE_HDR_UNDEREXPO_CONTRAST_TARGET_TBL_NUM        (11)

typedef struct {
    MINT32 i4RMGSeg;
    MINT32 i4RMGTurn;
    MINT32 i4HDRTarget_L;
    MINT32 i4HDRTarget_H;
    MINT32 i4HDRTargetLV_L;
    MINT32 i4HDRTargetLV_H;
    MINT32 i4OverExpoRatio;
    MINT32 i4OverExpoTarget;
    MINT32 i4OverExpoLV_L;
    MINT32 i4OverExpoLV_H;
    MINT32 i4UnderExpoContrastThr;
    MINT32 i4UnderExpoTargetTbl[CCU_AE_HDR_UNDEREXPO_CONTRAST_TARGET_TBL_NUM];
    MINT32 i4UnderExpoRatio;
    MINT32 i4AvgExpoRatio;
    MINT32 i4AvgExpoTarget;
    MINT32 i4HDRAESpeed;
    MINT32 i4HDRConvergeThr;
    /*
    *   SW HDR
    */
    MINT32 i4SWHdrLEThr;
    MINT32 i4SWHdrSERatio;
    MINT32 i4SWHdrSETarget;
    MINT32 i4SWHdrBaseGain;
} CCU_HDR_AE_CFG_T;



#endif

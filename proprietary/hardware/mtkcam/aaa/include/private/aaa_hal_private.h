/* Copyright Statement:
*
* This software/firmware and related documentation ("MediaTek Software") are
* protected under relevant copyright laws. The information contained herein is
* confidential and proprietary to MediaTek Inc. and/or its licensors. Without
* the prior written permission of MediaTek inc. and/or its licensors, any
* reproduction, modification, use or disclosure of MediaTek Software, and
* information contained herein, in whole or in part, shall be strictly
* prohibited.
*
* MediaTek Inc. (C) 2010. All rights reserved.
*
* BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
* THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
* RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
* ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
* WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
* WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
* NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
* RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
* INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
* TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
* RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
* OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
* SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
* RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
* STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
* ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
* RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
* MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
* CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
* The following software/firmware and/or related documentation ("MediaTek
* Software") have been modified by MediaTek Inc. All revisions are subject to
* any receiver's applicable license agreements with MediaTek Inc.
*/

#ifndef _MTK_HARDWARE_MTKCAM_AAA_INCLUDE_PRIVATE_AAA_HAL_PRIVATE_H_
#define _MTK_HARDWARE_MTKCAM_AAA_INCLUDE_PRIVATE_AAA_HAL_PRIVATE_H_

#include <utils/Vector.h>
#include <vector>

#include <mtkcam/aaa/aaa_hal_common.h>
#include <isp_tuning.h>
#include "../../../common/hal/inc/camera_custom_ae.h"

using namespace NSIspTuning;

/******************************************************************************
 *
 ******************************************************************************/
#define HAL3A_SETBIT(a, mask, value) (a) = (((a) & ~(mask)) | ((value) & (mask)))

#define HAL_FLG_DUMMY   (1<<0)
#define HAL_FLG_DUMMY2  (1<<2)

/******************************************************************************
 *
 ******************************************************************************/
namespace NS3Av3
{
typedef enum
{
    ESensorDevId_Main         = 0x01,
    ESensorDevId_Sub          = 0x02,
    ESensorDevId_MainSecond   = 0x04,
    ESensorDevId_Main3D       = 0x05,
    ESensorDevId_Atv          = 0x08,
}   ESensorDevId_T;

#define SENSOR_IDX_MAX (20)
template <typename T>
struct INST_T{
    std::once_flag onceFlag;
    std::unique_ptr<T> instance;
};

enum ESensorType_T {
    ESensorType_RAW = 0,
    ESensorType_YUV = 1
};


/**
 * @brief 3A commands
 */
enum ECmd_T {
     ECmd_CameraPreviewStart,
     ECmd_CameraPreviewEnd,
     ECmd_PrecaptureStart,
     ECmd_PrecaptureEnd,
     ECmd_CaptureStart,
     ECmd_CaptureEnd,
     ECmd_RecordingStart,
     ECmd_RecordingEnd,
     ECmd_Update,
     // internal command
     ECmd_Init,
     ECmd_Uninit,
     ECmd_AFUpdate, // sync with AF done
     ECmd_TouchAEStart,
     ECmd_TouchAEEnd,
     ECmd_AFStart,
     ECmd_AFEnd,
     ECmd_AFTrigger,
     ECmd_Sync3AStart,
     ECmd_Sync3AEnd

};


enum EQueryType_T{
    EQueryType_Init,
    EQueryType_Effect,
    EQueryType_AWB,
    EQueryType_AF,
    EQueryType_Ev,
    EQueryType_Sat,
    EQueryType_Bright,
    EQueryType_Contrast
};


enum E3ACaptureMode_T
{
    ECapMode_P2_Cal     = (1<<0),
    ECapMode_P2_Set     = (1<<1)
};


enum E3APreviewMode_T
{
    EPv_Normal    = 0,
    EPv_Video
};


enum ECaptureType_T
{
    ECapType_SingleCapture = 0,
    ECapType_MultiCapture
};


enum ESubFrameCount_T
{
    ESubFrameCnt_Normal = 1,
    ESubFrameCnt_SMVRx4_fps = 4,
    ESubFrameCnt_SMVRx8_fps = 8,
};


typedef struct
{
    MINT64 iYvalue;
    MINT32 i4ISO;
    MINT32 i4IsAEStable;
    MINT32 i4SceneLV;
    MINT32 ishutterValue;
    MINT32 i4DeltaBV;
    MUINT8 aeBlockV[25];
    MINT32 i4IsFlashFrm;
    MINT32 i4AEBlockAreaYCnt;
    MUINT8 *pAEBlockAreaYvalue;
    MINT32 i4IsAELocked;
} AE2AFInfo_T;

typedef struct
{
    MUINT64 TS_AFDone;           // Time stamp of AFDone of the current frame
} AdptCompTimeData_T;
struct CameraArea_T
{
    MINT32 i4Left;
    MINT32 i4Top;
    MINT32 i4Right;
    MINT32 i4Bottom;
    MINT32 i4Weight;
    CameraArea_T()
        : i4Left(0)
        , i4Top(0)
        , i4Right(0)
        , i4Bottom(0)
        , i4Weight(0)
    {}
};

#define MAX_FOCUS_AREAS  9

struct CameraFocusArea_T
{
    CameraArea_T rAreas[MAX_FOCUS_AREAS];
    MUINT32 u4Count;
    CameraFocusArea_T()
        : u4Count(0)
    {}
};


struct ISP_SENSOR_INFO_T
{
    MBOOL bHLREnable;
    MBOOL bAEStable;
    MBOOL bAELock;
    MBOOL bAEScenarioChange;
    MINT32 i4FrameId;
    MINT32 i4deltaIndex;
    MUINT32 u4AfeGain;
    MUINT32 u4IspGain;
    MUINT32 u4Eposuretime;   //!<: Exposure time in ms
    MUINT32 u4RealISOValue;
    MUINT32 u4MaxISO;
    MUINT32 u4OrgExposuretime;   //!<: Exposure time in ms
    MUINT32 u4OrgRealISOValue;
    MUINT32 u4AEStableCnt;
    MUINT32 u4AEFinerEVIdxBase;
    MUINT32 u4AEidxCurrentF;
    MINT32  i4LightValue_x10; // sceneLV
    MUINT32 u4MgrCWValue;     // central weighting Y value
    MINT32  i4AeBlockV[25];
    ISP_SENSOR_INFO_T()
        : bHLREnable(0)
        , bAEStable(0)
        , bAELock(0)
        , bAEScenarioChange(0)
        , i4FrameId(0)
        , i4deltaIndex(0)
        , u4AfeGain(0)
        , u4IspGain(0)
        , u4Eposuretime(0)
        , u4RealISOValue(0)
        , u4MaxISO(0)
        , u4OrgExposuretime(0)
        , u4OrgRealISOValue(0)
        , u4AEStableCnt(0)
        , u4AEFinerEVIdxBase(0)
        , u4AEidxCurrentF(0)
        , i4LightValue_x10(0)
        , u4MgrCWValue(0)
    {
        memset(i4AeBlockV, 0, sizeof(MINT32)*25);
    }
};

struct AF2AEInfo_T
{
    MINT32 i4MagicNum;
    MINT32 i4IsAFDone;
    MINT32 i4AfDac;
    MINT32 i4IsSceneStable;
    AF2AEInfo_T()
        : i4MagicNum(0)
        , i4IsAFDone(0)
        , i4AfDac(0)
        , i4IsSceneStable(0)
    {}
};


struct Hal3A_HDROutputParam_T
{
    MUINT32 u4OutputFrameNum;     // Output frame number (2 or 3)
    MUINT32 u4FinalGainDiff[2];   // 1x=1024; [0]: Between short exposure and 0EV; [1]: Between 0EV and long exposure
    MUINT32 u4TargetTone; //Decide the curve to decide target tone
};

struct ScaleCropedArea_T {
    MINT32 i4Xoffset;
    MINT32 i4Yoffset;
    MINT32 i4Xwidth;
    MINT32 i4Yheight;
    ScaleCropedArea_T()
        : i4Xoffset(0)
        , i4Yoffset(0)
        , i4Xwidth(0)
        , i4Yheight(0)
    {}
};

#define MAX_METERING_AREAS 9

struct CameraMeteringArea_T {
    CameraArea_T rAreas[MAX_METERING_AREAS];
    MUINT32 u4Count;
    CameraMeteringArea_T()
        : u4Count(0)
    {}
};


// LCE Info
struct LCEInfo_T {
    MINT32 i4NormalAEidx;    // gain >= 4x AE Pline table index at 30fps
    MINT32 i4LowlightAEidx;  // gain max AE Pline table index at 30fps
    MINT32 i4AEidxCur;          // AE current frame Pline table index
    MINT32 i4AEidxNext;        // AE next frame Pline table index
    MINT32 i4AEidxNextF;        // AE next frame Pline table indexF
};


typedef enum
{
    E_AE_PRECAPTURE_IDLE,
    E_AE_PRECAPTURE_START
} EAePreCapture_T;


struct SMVR_GAIN_DELAY_T {
    MUINT32 u4SensorGain_Delay;
    MUINT32 u4SutterGain_Delay;
    MUINT32 u4ISPGain_Delay;

    SMVR_GAIN_DELAY_T()
        : u4SensorGain_Delay(0)
        , u4SutterGain_Delay(0)
        , u4ISPGain_Delay(0)
        {}
};


/**
 * @brief 3A parameters
 */
// 3A parameters
struct Param_T {
    // DEFAULT DEFINITION CATEGORY ( ordered by SDK )
    MINT32  i4MagicNum;
    MINT32  i4MagicNumCur;
    MINT32  i4PresetKey;
    MINT32  i4FrameNum;
    MUINT32 u4HalFlag;
    MUINT8  u1ControlMode;
    MUINT8  u1PrecapTrig;
    MINT32  i4MinFps;
    MINT32  i4MaxFps;
    MUINT32 u4AeMode;
    MUINT8  u1HdrMode;
    MUINT32 u4AwbMode;
    MINT32 i4MWBColorTemperature; //ex. 3000, 3200, 4000, 6500... 5000 means 5000k.
    MUINT32 u4EffectMode;
    MUINT32 u4AntiBandingMode;
    MUINT32 u4SceneMode;
    MUINT32 u4StrobeMode;
    MINT32  i4ExpIndex;
    MFLOAT  fExpCompStep;

    // NEWLY-ADDED CATEGORY
    MUINT32 u4CamMode;   //Factory, ENG, normal
    MUINT32 u4ShotMode;
    MUINT32 u4CapType;  //refer to ECaptureType_T

    // MTK DEFINITION CATEGORY
    MINT32 i4IsoSpeedMode;
    MINT32 i4BrightnessMode;
    MINT32 i4HueMode;
    MINT32 i4SaturationMode;
    MINT32 i4ContrastMode;
    MINT32 i4EdgeMode;
    MUINT32 u4AeMeterMode;
    MINT32  i4RotateDegree;

    //EAePreCapture_T eAePreCapture;
    //MBOOL   bHistogramMode;
    MBOOL   bIsAELock;
    MBOOL   bIsAWBLock;

    // flash for engineer mode
    MINT32  i4PreFlashDuty;
    MINT32  i4PreFlashStep;
    MINT32  i4MainFlashDuty;
    MINT32  i4MainFlashStep;

    // shading
    MUINT8  u1ShadingMode;
    MUINT8  u1ShadingMapMode;
    MUINT8  u1ShadingMapXGrid;
    MUINT8  u1ShadingMapYGrid;

    // manual sensor
    MINT64  i8FrameDuration;  // naro sec
    MINT64  i8ExposureTime;   // naro sec
    MINT32  i4Sensitivity;    // ISO value
    MUINT8  u1BlackLvlLock;
    MUINT8  u1RollingShutterSkew;

    // color correction
    MUINT8  u1ColorCorrectMode;
    MFLOAT  fColorCorrectGain[4];
    MFLOAT  fColorCorrectMat[9];

    // edge & NR
    MUINT8  u1EdgeMode;
    MUINT8  u1NRMode;

    // tonemap
    MUINT8  u1TonemapMode;
    std::vector<MFLOAT> vecTonemapCurveBlue;
    std::vector<MFLOAT> vecTonemapCurveGreen;
    std::vector<MFLOAT> vecTonemapCurveRed;

    MINT32  i4RawType;
    MUINT8  u1CaptureIntent;
    MINT32  i4Cshot;
    MUINT8  u1ZSDCaptureIntent;
    MUINT8  u1IsGetExif;
    MBOOL   u1HQCapEnable;
    MBOOL   i4DisableP1;    // Store Request.fgDisableP1
    MUINT8  u1IsSingleFrameHDR;
    MUINT8  u1IsStartCapture;
    MBOOL   u1AeFakePreCap;
    NSIspTuning::EIspProfile_T eIspProfile;
    MINT32  i4DenoiseMode;
    NSCam::IMetadata::Memory rcapParams;
    CameraMeteringArea_T    rMeteringAreas;
    ScaleCropedArea_T       rScaleCropRect;
    MINT32 i4ZoomRatio;
    MINT32  i4ForceFace3A;
    MUINT8  u1FaceDetectMode;
    MBOOL   bDummyBeforeCapture;
    MBOOL   bDummyAfterCapture;
    MUINT8  u1PreCapStart;
    MUINT32 u4AFNVRAMIndex;
    MINT32  i4AwbValue;
    MUINT8  u1RemosaicEn;
    MBOOL   bIsDualCamWithCamSv;
    MINT32  i4FlashCalEn;    // flash calibration enable
    MINT32  i4DynamicSubsampleCount;
    AE_Cust_Param_T rAeCustParam;
    MINT64  i8ExposureTimeDummy;
    MINT64  i8FrameDurationDummy;
    MINT32  i4SensitivityDummy;
    MBOOL   bIsDummyFrame;
    MBOOL   bIsFDReady;
    NSCam::MSize targetSize;
    MBOOL   u1SubFlashCustomization;
    MUINT8  u1TorchDuty;
    MBOOL   bByPassStt;
    MUINT8  u1Dummy;
    MINT32  i4ManualPlineIdx;
    MINT32  i4CustomPlineMode;

    Param_T()
        : i4MagicNum(0)
        , i4MagicNumCur(0)
        , i4PresetKey(0)
        , i4FrameNum(0)
        , u4HalFlag(0)
        , u1ControlMode(MTK_CONTROL_MODE_AUTO)
        , u1PrecapTrig(MTK_CONTROL_AE_PRECAPTURE_TRIGGER_IDLE)
        , i4MinFps(5000)
        , i4MaxFps(30000)
        , u4AeMode(1)
        , u1HdrMode(0)
        , u4AwbMode(1)
        , i4MWBColorTemperature(0)
        , u4EffectMode(0)
        , u4AntiBandingMode(0)
        , u4SceneMode(0)
        , u4StrobeMode(0)
        , i4ExpIndex(0)
        , fExpCompStep(5)
        //
        , u4CamMode(5) //eAppMode_PhotoMode
        , u4ShotMode(0)
        , u4CapType(ECapType_SingleCapture)
        //
        , i4IsoSpeedMode(0)
        , i4BrightnessMode(1)
        , i4HueMode(1)
        , i4SaturationMode(1)
        , i4ContrastMode(1)
        , i4EdgeMode(1)
        , u4AeMeterMode (0)
        , i4RotateDegree(0)
        //
        , bIsAELock(MFALSE)
        , bIsAWBLock(MFALSE)
        //
        , i4PreFlashDuty(-1)
        , i4PreFlashStep(-1)
        , i4MainFlashDuty(-1)
        , i4MainFlashStep(-1)
        //
        , u1ShadingMode(MTK_SHADING_MODE_FAST)
        , u1ShadingMapMode(MTK_STATISTICS_LENS_SHADING_MAP_MODE_OFF)
        , u1ShadingMapXGrid(1)
        , u1ShadingMapYGrid(1)
        //
        , i8FrameDuration(0)
        , i8ExposureTime(0)
        , i4Sensitivity(0)
        , u1BlackLvlLock(MTK_BLACK_LEVEL_LOCK_OFF)
        , u1RollingShutterSkew(0)
        , u1ColorCorrectMode(MTK_COLOR_CORRECTION_MODE_FAST)
        //
        , u1EdgeMode(MTK_EDGE_MODE_FAST)
        , u1NRMode(MTK_NOISE_REDUCTION_MODE_FAST)
        , u1TonemapMode(MTK_TONEMAP_MODE_FAST)
        //
        , i4RawType(NSIspTuning::ERawType_Proc)
        , u1CaptureIntent(MTK_CONTROL_CAPTURE_INTENT_PREVIEW)
        , i4Cshot(MTK_CSHOT_FEATURE_CAPTURE_OFF)
        , u1ZSDCaptureIntent(MTK_CONTROL_CAPTURE_INTENT_PREVIEW)
        , u1IsGetExif(0)
        , u1HQCapEnable(MFALSE)
        , i4DisableP1(MFALSE)
        , u1IsSingleFrameHDR(0)
        , u1IsStartCapture(MFALSE)
        , u1AeFakePreCap(MFALSE)
        , eIspProfile(NSIspTuning::EIspProfile_Preview)
        , i4DenoiseMode(0)
        , i4ZoomRatio(100)
        , i4ForceFace3A(0)
        , u1FaceDetectMode(0)
        , bDummyBeforeCapture(MFALSE)
        , bDummyAfterCapture(MFALSE)
        , u1PreCapStart(0)
        , u4AFNVRAMIndex(0)
        , i4AwbValue(0)
        , u1RemosaicEn(0)
        , bIsDualCamWithCamSv(0)
        , i4FlashCalEn(0)
        , i4DynamicSubsampleCount(1)
        , i8ExposureTimeDummy(0)
        , i8FrameDurationDummy(0)
        , i4SensitivityDummy(0)
        , bIsDummyFrame(0)
        , bIsFDReady(MTRUE)
        , targetSize(0,0)
        , u1SubFlashCustomization(0)
        , u1TorchDuty(1)  // Torch default level
        , bByPassStt(0)
        , u1Dummy(0)
        , i4ManualPlineIdx(0)
        , i4CustomPlineMode(0)
    {}
};


/**
 * @brief 3A parameters
 */
// 3A P2 parameters
struct P2Param_T {
    MUINT8 u1PGN;
    MUINT8 u1Exif;
    MUINT8 u1DumpExif;
    MUINT8 u1RepeatResult;
    MUINT8 u1IspProfile;
    MUINT8 u1CapIntent;
    MUINT8 u1appEdgeMode;  //Android ISP Spec
    MUINT8 u1NrMode;
    MBOOL  bBypassLCE;
    MBOOL  bBypassNR;
    MBOOL  bEnableNDD;
    MINT32 i4P2InImgFmt;
    MUINT8 u1P2TuningUpdate;
    MINT32 i4UniqueKey;
    MINT32 i4RawType;
    MINT32 i4FrmNo;
    MINT32 i4ReqNo;
    MINT32 i4ISO;
    MINT32 NR3D_Data[14];
    MINT32 ResizeYUV;
    const NSCam::MRect *rpSclCropRect;
    const NSCam::MRect *rpP1Crop;
    const NSCam::MRect *rpP2Crop;
    const NSCam::MSize *rpRzInSize;
    const NSCam::MSize *rpRzSize;
    const NSCam::MSize *rpP2OriginSize;
    const NSCam::MSize *rpP2RzSize;
    NSCam::IMetadata rexifMeta;                // Need to update & write back to ResultP2, cannot be declared as pointer to indicate constanct control Metadata
    const NSCam::IMetadata::Memory *rpdbgIsp;
    const NSCam::IMetadata::Memory *rpLscData;
    const NSCam::IMetadata::Memory *rpTsfData;
    const NSCam::IMetadata::Memory *rpTsfDumpNo;

    MINT32 i4MagicNum;
    MINT32 i4EdgeMode;
    MINT32 i4halEdgeMode; //MTK Internal APP User Select Level before ISP 6.0
    MUINT8 u1TonemapMode;
    const MFLOAT * pTonemapCurveRed;
    const MFLOAT * pTonemapCurveBlue;
    const MFLOAT * pTonemapCurveGreen;
    MUINT32 u4TonemapCurveRedSize;
    MUINT32 u4TonemapCurveGreenSize;
    MUINT32 u4TonemapCurveBlueSize;

    MINT32 i4BrightnessMode;
    MINT32 i4ContrastMode;
    MINT32 i4HueMode;
    MINT32 i4SaturationMode;
    MINT32 i4DenoiseMode;
    ScaleCropedArea_T rScaleCropRect;
    MUINT8 u1ControlMode;
    NSCam::MSize targetSize;

    P2Param_T()
        : u1PGN(0)
        , u1Exif(0)
        , u1DumpExif(0)
        , u1RepeatResult(0)
        , u1IspProfile(255)
        , u1CapIntent(MTK_CONTROL_CAPTURE_INTENT_PREVIEW)
        , u1appEdgeMode(1)
        , u1NrMode(0)
        , bBypassLCE(0)
        , bBypassNR(0)
        , bEnableNDD(0)
        , i4P2InImgFmt(0)
        , u1P2TuningUpdate(0)
        , i4UniqueKey(0)
        , i4RawType(NSIspTuning::ERawType_Proc)
        , i4FrmNo(0)
        , i4ReqNo(0)
        , i4ISO(0)
        , NR3D_Data()
        , ResizeYUV(0)
        , rpSclCropRect(NULL)
        , rpP1Crop(NULL)
        , rpP2Crop(NULL)
        , rpRzInSize(NULL)
        , rpRzSize(NULL)
        , rpP2OriginSize(NULL)
        , rpP2RzSize(NULL)
        , rexifMeta()
        , rpdbgIsp(NULL)
        , rpLscData(NULL)
        , rpTsfData(NULL)
        , rpTsfDumpNo(NULL)
        , i4MagicNum(0)
        , i4EdgeMode(1)
        , i4halEdgeMode(0)
        , u1TonemapMode(MTK_TONEMAP_MODE_FAST)
        , pTonemapCurveRed(NULL)
        , pTonemapCurveBlue(NULL)
        , pTonemapCurveGreen(NULL)
        , u4TonemapCurveRedSize(1)
        , u4TonemapCurveGreenSize(1)
        , u4TonemapCurveBlueSize(1)
        , i4BrightnessMode(1)
        , i4ContrastMode(1)
        , i4HueMode(1)
        , i4SaturationMode(1)
        , i4DenoiseMode(0)
        , u1ControlMode(MTK_CONTROL_MODE_AUTO)
        , targetSize(0,0)
    {}
};


/**
 * @brief AF parameters
 */
// AF parameters
struct AF_Param_T {

    MINT32                  i4MagicNum;    /* request magic number */
    MINT32                  i4MagicNumCur; /* statistic magic number */
    MUINT32                 u4AfMode;
    MFLOAT                  fFocusDistance;
    MBOOL                   bEnable3ASetParams;
    MUINT8                  u1PrecapTrig;
    MUINT8                  u1AfTrig;
    MUINT8                  u1AfPause;
    MUINT8                  u1MZOn;
    MUINT8                  u1CaptureIntent;
    MUINT8                  u1ZSDCaptureIntent;
    CameraFocusArea_T       rFocusAreas;
    CameraArea_T            rScaleCropArea;
    MUINT8                  u1AfTrigStart;

    AF_Param_T()
        : i4MagicNum(0)
        , i4MagicNumCur(0)
        , u4AfMode(5)
        , fFocusDistance(-1)
        , bEnable3ASetParams(MTRUE)
        , u1PrecapTrig(MTK_CONTROL_AE_PRECAPTURE_TRIGGER_IDLE)
        , u1AfTrig(0)
        , u1AfPause(0)
        , u1MZOn(255)
        , u1CaptureIntent(0)
        , u1ZSDCaptureIntent(0)
        , rFocusAreas()
        , rScaleCropArea()
        , u1AfTrigStart(0)
    {}
};


struct ParamIspProfile_T
{
    EIspProfile_T eIspProfile;
    MINT32        i4MagicNum;
    MINT32        i4MagicNumCur;
    MINT32        iEnableRPG;
    MINT32        iValidateOpt;
    MBOOL         bMainFlash;
    RequestSet_T  rRequestSet;
    enum
    {
        EParamValidate_None     = 0,
        EParamValidate_All      = 1,
        EParamValidate_P2Only   = 2
    };

    ParamIspProfile_T()
        : eIspProfile(EIspProfile_Preview)
        , i4MagicNum(0)
        , i4MagicNumCur(0)
        , iEnableRPG(0)
        , iValidateOpt(1)
        , bMainFlash(MFALSE)
        , rRequestSet()
    {}

    ParamIspProfile_T(
        EIspProfile_T eIspProfile_,
        MINT32        i4MagicNum_,
        MINT32        i4MagicNumCur_,
        MINT32        iEnableRPG_,
        MINT32        iValidateOpt_,
        RequestSet_T  rRequestSet_)
        : eIspProfile(eIspProfile_)
        , i4MagicNum(i4MagicNum_)
        , i4MagicNumCur(i4MagicNumCur_)
        , iEnableRPG(iEnableRPG_)
        , iValidateOpt(iValidateOpt_)
        , bMainFlash(MFALSE)
        , rRequestSet(rRequestSet_)
    {}
};


enum E_Repeat_Result {
    E_Repeat_OFF = 0,
    E_Repeat_ON = 1
};


struct E3ACtrl_ConvertToIspGamma_ARG_T
{
    MUINT32       u4NumPts;
    const MFLOAT* pPtPairs;
    MINT32*       pOutGMA;

    E3ACtrl_ConvertToIspGamma_ARG_T()
        : u4NumPts(0)
        , pPtPairs(NULL)
        , pOutGMA(NULL)
    {}

    E3ACtrl_ConvertToIspGamma_ARG_T(const MFLOAT* _pPtPairs, MUINT32 _u4NumPts, MINT32* _pOutGMA)
        : u4NumPts(_u4NumPts)
        , pPtPairs(_pPtPairs)
        , pOutGMA(_pOutGMA)
    {}
};

struct GyroSensor_Param_T {
    MINT32  i4AcceInfo[3];
    MINT32  i4GyroInfo[3];
    MUINT64 u8AcceTS;
    MUINT64 u8GyroTS;

    GyroSensor_Param_T()
        : u8AcceTS(0)
        , u8GyroTS(0)
    {
        i4AcceInfo[0] = i4AcceInfo[1] = i4AcceInfo[2] = 0;
        i4GyroInfo[0] = i4GyroInfo[1] = i4GyroInfo[2] = 0;
    }
};

struct AWB_AAO_CONFIG_Param_T
{
    MUINT32 width;
    MUINT32 height;
    MUINT32 offset_x;
    MUINT32 offset_y;
    MUINT32 size_x;
    MUINT32 size_y;
    MUINT32 num_x;
    MUINT32 num_y;

    AWB_AAO_CONFIG_Param_T()
        : width(0)
        , height(0)
        , offset_x(0)
        , offset_y(0)
        , size_x(0)
        , size_y(0)
        , num_x(0)
        , num_y(0)
    {}
};

struct Query_3A_Index_T
{
    MUINT32 u4AENVRAMIndex;
    MUINT32 u4AWBNVRAMIndex;
    MUINT32 u4AFNVRAMIndex;
    MUINT32 u4FlashAENVRAMIndex;
    MUINT32 u4FlashAWBNVRAMIndex;
    MUINT32 u4FlashCaliNVRAMIndex;
    MUINT32 u4OBCNVRAMIndex;
    MUINT32 u4LTMNVRAMIndex;

    Query_3A_Index_T()
        : u4AENVRAMIndex(0)
        , u4AWBNVRAMIndex(0)
        , u4AFNVRAMIndex(0)
        , u4FlashAENVRAMIndex(0)
        , u4FlashAWBNVRAMIndex(0)
        , u4FlashCaliNVRAMIndex(0)
        , u4OBCNVRAMIndex(0)
        , u4LTMNVRAMIndex(0)
    {}
};

struct STT_CFG_INFO_T
{
    MINT32  i4TgInfo;
    std::vector<MUINT32> vecCAMSVIndexOf3EXPO; //This is for 3EXPO flow

    STT_CFG_INFO_T()
        : i4TgInfo(0)
    {
        vecCAMSVIndexOf3EXPO.clear();
    }
};

struct AE_PARAM_SET_INFO {
    MBOOL   bIsAELock;
    MBOOL   bBlackLvlLock;
    MINT32  i4MinFps;
    MINT32  i4MaxFps;
    MUINT32 u4AeMeterMode;
    MINT32  i4RotateDegree;
    MINT32  i4IsoSpeedMode;
    MINT32  i4ExpIndex;
    MFLOAT  fExpCompStep;
    MUINT32 u4AeMode;
    MINT32  i4DenoiseMode;
    MUINT32 u4AntiBandingMode;
    MUINT32 u4CamMode;
    MUINT32 u4ShotMode;
    MUINT32 u4SceneMode;
    MUINT8 u1HdrMode;
    MINT32  i4ZoomRatio;
    CameraMeteringArea_T rMeteringAreas;
    MUINT32 u4ZoomXOffset;
    MUINT32 u4ZoomYOffset;
    MUINT32 u4ZoomWidth;
    MUINT32 u4ZoomHeight;
    MUINT8 u1IsDummyFrame;
    MINT32  i4ManualPlineIdx;
    MINT32  i4CustomPlineMode;

    AE_PARAM_SET_INFO()
        : bIsAELock(MFALSE)
        , bBlackLvlLock(MTK_BLACK_LEVEL_LOCK_OFF)
        , i4MinFps(5000)
        , i4MaxFps(30000)
        , u4AeMeterMode (0)
        , i4RotateDegree(0)
        , i4IsoSpeedMode(0)
        , i4ExpIndex(0)
        , fExpCompStep(5)
        , u4AeMode(1)
        , i4DenoiseMode(0)
        , u4AntiBandingMode(0)
        , u4CamMode(5) //eAppMode_PhotoMode
        , u4ShotMode(0)
        , u4SceneMode(0)
        , u1HdrMode(0)
        , i4ZoomRatio(100)
        , u4ZoomXOffset(0)
        , u4ZoomYOffset(0)
        , u4ZoomWidth(0)
        , u4ZoomHeight(0)
        , u1IsDummyFrame(0)
        , i4ManualPlineIdx(0)
        , i4CustomPlineMode(0)
    {}
};


}   //namespace NS3Av3
/******************************************************************************
 *
 ******************************************************************************/
#endif //_MTK_HARDWARE_MTKCAM_AAA_INCLUDE_PRIVATE_AAA_HAL_PRIVATE_H_

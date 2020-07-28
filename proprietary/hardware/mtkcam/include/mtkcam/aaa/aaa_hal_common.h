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

/**
* @file aaa_hal_common.h
* @brief Declarations of Abstraction of 3A Hal Class and Top Data Structures
*/

#ifndef __AAA_HAL_COMMON_V3_H__
#define __AAA_HAL_COMMON_V3_H__

#include <vector>

#include <mtkcam/def/common.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/IMetadata.h>
#include <mtkcam/utils/module/module.h>
#include <mtkcam/drv/IHalSensor.h>


namespace NS3Av3
{
struct TuningParam
{
    void* pRegBuf;
    void* pMfbBuf;
    void* pMsfTbl;
    void* pMssBuf;
    void* pLsc2Buf;
    void* pLcsBuf;
    MBOOL bYNR_LCES_In;
    void* pBpc2Buf;
    void* pDcsBuf;
    MINT32 i4DcsMagicNo;
    MBOOL bDCES_Enalbe;
    void* pFaceAlphaBuf;
    MBOOL bSlave;
    void* pDualSynInfo;
    void* pLceshoBuf;
    void* pLce4CALTM;
    void* pUserLTM_Setting;
    MUINT32 u4UserLTM_Setting_Size;
    MUINT32 *pUserCCM_Setting;
    MUINT32 u4UserCCM_Setting_Size;

    /* constructor */
    TuningParam(
            void* pRegBuf  = nullptr,
            void* pMfbBuf  = nullptr,
            void* pMsfTbl  = nullptr,
            void* pMssBuf  = nullptr,
            void* pLsc2Buf = nullptr,
            void* pLcsBuf  = nullptr,
            MBOOL bYNR_LCES_In = MFALSE,
            void* pBpc2Buf = nullptr,
            void* pDcsBuf  = nullptr,
            MINT32 i4DcsMagicNo = -1,
            MBOOL bDCES_Enalbe = MFALSE,
            void* pFaceAlphaBuf = nullptr,
            MBOOL bSlave = MFALSE,
            void* pDualSynInfo = nullptr,
            void* pLceshoBuf = nullptr,
            void* pLce4CALTM = nullptr,
            void* pUserLTM_Setting = nullptr,
            MUINT32 u4UserLTM_Setting_Size = 0,
            MUINT32 *pUserCCM_Setting = nullptr,
            MUINT32 u4UserCCM_Setting_Size = 0)
        : pRegBuf(pRegBuf)
        , pMfbBuf(pMfbBuf)
        , pMsfTbl(pMsfTbl)
        , pMssBuf(pMssBuf)
        , pLsc2Buf(pLsc2Buf)
        , pLcsBuf(pLcsBuf)
        , bYNR_LCES_In(bYNR_LCES_In)
        , pBpc2Buf(pBpc2Buf)
        , pDcsBuf(pDcsBuf)
        , i4DcsMagicNo(i4DcsMagicNo)
        , bDCES_Enalbe(bDCES_Enalbe)
        , pFaceAlphaBuf(pFaceAlphaBuf)
        , bSlave(bSlave)
        , pDualSynInfo(pDualSynInfo)
        , pLceshoBuf(pLceshoBuf)
        , pLce4CALTM(pLce4CALTM)
        , pUserLTM_Setting(pUserLTM_Setting)
        , u4UserLTM_Setting_Size(u4UserLTM_Setting_Size)
        , pUserCCM_Setting(pUserCCM_Setting)
        , u4UserCCM_Setting_Size(u4UserCCM_Setting_Size)
    {}
};

struct Buffer_Param
{
    MBOOL   bSupport;
    NSCam::MSize   size;
    MINT    format;
    size_t  stride;
    MUINT32 bitDepth;

    Buffer_Param()
        : bSupport(MFALSE)
        , size(NSCam::MSize(0,0))
        , format(0)
        , stride(0)
        , bitDepth(0)
        {};
};

struct Buffer_Info
{
    Buffer_Param LCESO_Param;
    Buffer_Param DCESO_Param;
    MUINT32 u4DualSyncInfoSize = 0;
    Buffer_Param LCESHO_Param;
};

typedef enum {
    EP1_FD_Path = 0x01,
    EP1_Depth_Path = 0x02,
}eISP_P1_YUV_PATH;

struct FrameOutputParam_T
{
    MUINT32 u4AEIndex;
    MUINT32 u4AEIndexF;
    MUINT32 u4FinerEVIdxBase;
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
    MUINT32 u4AvgY;                     // preview average brightness (0 ~ 255)
    MUINT32 i4LuxValue_x10000;          // lux value (0.0001 ~ 100000)*10000
    MUINT32 u4AwbGain[3];
    FrameOutputParam_T()
        : u4AEIndex(0)
        , u4AEIndexF(0)
        , u4FinerEVIdxBase(0)
        , u4FRameRate_x10(0)
        , u4PreviewShutterSpeed_us(0)
        , u4PreviewSensorGain_x1024(0)
        , u4PreviewISPGain_x1024(0)
        , u4RealISOValue(0)
        , u4CapShutterSpeed_us(0)
        , u4CapSensorGain_x1024(0)
        , u4CapISPGain_x1024(0)
        , i4BrightValue_x10(0)
        , i4ExposureValue_x10(0)
        , i4LightValue_x10(0)
        , i2FlareOffset(0)
        , i2FlareGain(0)
        , u4AvgY(0)
        , i4LuxValue_x10000(0)
    {}
};


/**
 * @brief 3A parameters for capture
 */
struct CaptureParam_T
{
    MUINT32 u4ExposureMode;             //0: exp. time, 1: exp. line
    MUINT32 u4Eposuretime;              //!<: Exposure time in us
    MUINT32 u4AfeGain;                  //!<: sensor gain
    MUINT32 u4IspGain;                  //!<: raw gain
    MUINT32 u4RealISO;                  //!<: Real ISO speed
    MUINT32 u4FlareOffset;
    MUINT32 u4FlareGain;                // 512 is 1x
    MINT32  i4LightValue_x10;           // 10 base LV value
    MINT32  i4YuvEvIdx;
    CaptureParam_T()
        : u4ExposureMode(0)
        , u4Eposuretime(0)
        , u4AfeGain(0)
        , u4IspGain(0)
        , u4RealISO(0)
        , u4FlareOffset(0)
        , u4FlareGain(0)
        , i4LightValue_x10(0)
        , i4YuvEvIdx(0)
    {}
};

/**
 * @brief 3A parameters for capture
 */
struct DualZoomInfo_T
{
    MINT32 i4AELv_x10;          // AE Lv
    MINT32 i4AEIso;             // AE ISO
    MBOOL  bIsAEBvTrigger;      // AE isBvTrigger
    MBOOL  bIsAEAPLock;         // AE Lock or not by AP
    MINT32 i4AFDAC;             // AF DAC value
    MBOOL  bAFDone;             // AF done or not
    MBOOL  bSyncAFDone;         // Sync AF done or not
    MBOOL  bSync2ADone;         // Sync 2A done or not
    MBOOL  bOISIsValid;         // OIS Info is valid
    MFLOAT fOISHallPosXum;      // OIS Hall Pos X (um)
    MFLOAT fOISHallPosYum;      // OIS Hall Pos Y (um)
    MBOOL  bIsFrameSyncDone;

    DualZoomInfo_T()
        : i4AELv_x10(0)
        , i4AEIso(0)
        , bIsAEBvTrigger(0)
        , bIsAEAPLock(0)
        , i4AFDAC(0)
        , bAFDone(0)
        , bSyncAFDone(0)
        , bSync2ADone(0)
        , bOISIsValid(0)
        , fOISHallPosXum(0)
        , fOISHallPosYum(0)
        , bIsFrameSyncDone(1)
    {}
};


struct RequestSet_T
{
    RequestSet_T()
        : fgKeep(0)
        , fgDisableP1(0)
    {}
    std::vector<MINT32> vNumberSet;
    MBOOL fgKeep;
    MBOOL fgDisableP1;
};

struct FeatureParam_T {
    MBOOL   bExposureLockSupported;
    MBOOL   bAutoWhiteBalanceLockSupported;
    MUINT32 u4MaxFocusAreaNum;
    MUINT32 u4MaxMeterAreaNum;
    MBOOL   bEnableDynamicFrameRate;
    MINT32  i4MaxLensPos;
    MINT32  i4MinLensPos;
    MINT32  i4AFBestPos;
    MUINT32 u4FocusLength_100x;
    MINT32  u4PureRawInterval;
    MBOOL   bCcuIsSupportSecurity;
    FeatureParam_T()
        : bExposureLockSupported(0)
        , bAutoWhiteBalanceLockSupported(0)
        , u4MaxFocusAreaNum(0)
        , u4MaxMeterAreaNum(0)
        , bEnableDynamicFrameRate(0)
        , i4MaxLensPos(0)
        , i4MinLensPos(0)
        , i4AFBestPos(0)
        , u4FocusLength_100x(0)
        , u4PureRawInterval(0)
        , bCcuIsSupportSecurity(0)
    {}
};


struct ExpSettingParam_T {
    MINT32  u4AOEMode;
    MUINT32 u4MaxSensorAnalogGain;  // 1x=1024
    MUINT32 u4MaxAEExpTimeInUS;     // unit: us
    MUINT32 u4MinAEExpTimeInUS;     // unit: us
    MUINT32 u4ShutterLineTime;      // unit: 1/1000 us
    MUINT32 u4MaxAESensorGain;      // 1x=1024
    MUINT32 u4MinAESensorGain;      // 1x=1024
    MUINT32 u4ExpTimeInUS0EV;       // unit: us
    MUINT32 u4SensorGain0EV;        // 1x=1024
    MUINT32 u4ISOValue;             // real ISO value
    MUINT8  u1FlareOffset0EV;
    MINT32  i4GainBase0EV;          // AOE application for LE calculation
    MINT32  i4LE_LowAvg;            // AOE application for LE calculation, def: 0 ~ 39 avg
    MINT32  i4SEDeltaEVx100;        // AOE application for SE calculation
    MBOOL   bDetectFace;            // detect face or not
    MUINT32 u4Histogram[128];
    MUINT32 u4FlareHistogram[128];
    MVOID*  pPLineAETable;
    MINT32  i4aeTableCurrentIndex;
    MUINT32 u4LE_SERatio_x100;      // vhdr ratio (x100)
    MUINT32 u4ExpRatio;
};


// 3A ASD info
struct ASDInfo_T {
    MINT32 i4AELv_x10;          // AE Lv
    MBOOL  bAEBacklit;          // AE backlit condition
    MBOOL  bAEStable;           // AE stable
    MINT16 i2AEFaceDiffIndex;   // Face AE difference index with central weighting
    MINT32 i4AWBRgain_X128;     // AWB Rgain
    MINT32 i4AWBBgain_X128;     // AWB Bgain
    MINT32 i4AWBRgain_D65_X128; // AWB Rgain (D65; golden sample)
    MINT32 i4AWBBgain_D65_X128; // AWB Bgain (D65; golden sample)
    MINT32 i4AWBRgain_CWF_X128; // AWB Rgain (CWF; golden sample)
    MINT32 i4AWBBgain_CWF_X128; // AWB Bgain (CWF; golden sample)
    MBOOL  bAWBStable;          // AWB stable
    MINT32 i4AFPos;             // AF position
    MVOID* pAFTable;            // Pointer to AF table
    MINT32 i4AFTableOffset;     // AF table offset
    MINT32 i4AFTableMacroIdx;   // AF table macro index
    MINT32 i4AFTableIdxNum;     // AF table total index number
    MBOOL  bAFStable;           // AF stable
};


enum E_CAPTURE_TYPE
{
    E_CAPTURE_NORMAL = 0,
    E_CAPTURE_HIGH_QUALITY_CAPTURE
} ;


struct CapParam_T
{
    MUINT32             u4CapType;
    MINT64              i8ExposureTime;
    NSCam::IMetadata    metadata;
    CapParam_T()
        :u4CapType(E_CAPTURE_NORMAL)
        ,i8ExposureTime(0){}
};

struct AE_Pline_Limitation_T {
    MBOOL   bEnable;
    MBOOL   bEquivalent;
    MUINT32 u4IncreaseISO_x100;
    MUINT32 u4IncreaseShutter_x100;
};

struct AE_EXP_SETTING_T {
    MINT32 u4ExposureTime;   // naro sec
    MINT32 u4Sensitivity;    //ISO value
};

/* frame information */
typedef struct AF_FRAME_INFO_t
{
    MINT64 i8FocusValue;    // focusValue
    MINT32 i4LensPos;       // lens position
    MINT32 GyroValue[3];    // gyro value
    MINT32 AFROI[5];        // X,Y,W,H,Type
} AF_FRAME_INFO_T;

enum E_DAF_MODE {
    E_DAF_OFF               = 0,
    E_DAF_RUN_STEREO        = 1, /* run stereo hal */
    E_DAF_RUN_DEPTH_ENGINE  = 2  /* run depth engine */
};

// max frames to queue DAF information
#define DAF_TBL_QLEN 32
#define DIST_TBL_QLEN 16
typedef struct DAF_VEC_STRUCT_t
{
    MUINT32 frm_mun;
    MINT32  af_mode;
    MINT32  af_roi_sel;
    MUINT8  is_learning;
    MUINT8  is_querying;
    MUINT8  af_valid;
    MUINT8  is_af_stable;
    MUINT16 af_dac_pos;
    MUINT16 af_dac_index;
    MUINT16 af_confidence;
    MUINT16 af_win_start_x;
    MUINT16 af_win_start_y;
    MUINT16 af_win_end_x;
    MUINT16 af_win_end_y;
    MUINT16 daf_dac_index;
    MUINT16 daf_confidence;
    MUINT16 daf_distance;
    MUINT16 thermal_lens_pos;
    MUINT16 posture_dac;
    MINT32  is_scene_stable;
} DAF_VEC_STRUCT;


typedef struct
{
    MUINT8   is_daf_run;
    MUINT32  is_query_happen;
    MUINT32  curr_p1_frm_num;
    MUINT32  curr_p2_frm_num;
    MUINT16  af_dac_min;
    MUINT16  af_dac_max;
    MUINT16  af_dac_inf;
    MUINT16  af_dac_marco;
    MUINT16  af_distance_inf;
    MUINT16  af_distance_marco;
    MUINT16  af_dac_start;
    MUINT32  dac[DIST_TBL_QLEN];
    MUINT32  dist[DIST_TBL_QLEN];

    DAF_VEC_STRUCT daf_vec[DAF_TBL_QLEN];
}DAF_TBL_STRUCT;

typedef struct FSC_DAC_INIT_INFO_t
{
    MINT32 macro_To_Inf_Ratio;
    MINT32 dac_Inf;
    MINT32 dac_Macro;
    MINT32 damping_Time;
    MINT32 readout_Time_us;
    MINT32 init_DAC;
    MINT32 af_Table_Start;
    MINT32 af_Table_End;
    FSC_DAC_INIT_INFO_t()
    {
        macro_To_Inf_Ratio = 0;
        dac_Inf = 0;
        dac_Macro = 0;
        damping_Time = 0;
        readout_Time_us = 0;
        init_DAC = 0;
        af_Table_Start = 0;
        af_Table_End = 0;
    }
} FSC_DAC_INIT_INFO_T;

typedef struct FSC_DAC_INFO_t
{
    MINT32 DAC_From;
    MINT32 DAC_To;
    MINT32 Percent;
    FSC_DAC_INFO_t()
    {
        DAC_From = 0;
        DAC_To = 0;
        Percent = 0;
    }
} FSC_DAC_INFO_T;

typedef struct FSC_FRM_INFO_t
{
    MINT32 SttNum;
    MINT32 SetCount; // 0~2
    FSC_DAC_INFO_T DACInfo[2];
    FSC_FRM_INFO_t()
    {
        SttNum = 0;
        SetCount = 0;
    }
    FSC_FRM_INFO_t& operator =(const FSC_FRM_INFO_t &in)
    {
        SttNum = in.SttNum;
        SetCount = in.SetCount;
        memcpy(DACInfo, in.DACInfo, sizeof(FSC_DAC_INFO_t)*2);
        return (*this);
    }
} FSC_FRM_INFO_T;


#define OIS_DATA_NUM 8
struct OISInfo_T
{
    int64_t TimeStamp[OIS_DATA_NUM];
    int i4OISHallPosX[OIS_DATA_NUM];
    int i4OISHallPosY[OIS_DATA_NUM];
};

enum ESTART_CAP_TYPE_T
{
    ESTART_CAP_NORMAL = 0,
    ESTART_CAP_MANUAL,
    ESTART_CAP_SPECIAL
};


struct S3ACtrl_GetIspGamma
{
    MUINT32*    gamma_lut;          // in: pointer to a user-allocating buffer
    MUINT32     gamma_lut_size;     // out
    MBOOL       enabled;            // out
};


enum E3ACtrl_T
{
    E3ACtrl_Begin = 0,
    //shading
    E3ACtrl_SetShadingSdblkCfg      = 0x0001,
    E3ACtrl_SetShadingEngMode,
    E3ACtrl_SetShadingByp123,
    E3ACtrl_SetShadingOnOff,
    E3ACtrl_SetShadingTSFOnOff,
    E3ACtrl_SetShadingDynamic,
    E3ACtrl_SetShadingColorTemp,
//    E3ACtrl_SetShadingStrength,

    //AWB
    E3ACtrl_SetAwbBypCalibration    = 0x0100,
    E3ACtrl_SetAwbMode,

    //AE set
//    E3ACtrl_SetExposureParam        = 0x0200,
    E3ACtrl_SetHalHdr               = 0x0200,
    E3ACtrl_SetAETargetMode,
    E3ACtrl_SetAEIsoSpeedMode,
    E3ACtrl_SetAELimiterMode,
    E3ACtrl_SetAECamMode,
    E3ACtrl_SetAEEISRecording,
    E3ACtrl_SetAEPlineLimitation,
    E3ACtrl_EnableDisableAE,
    E3ACtrl_SetAEVHDRratio,
    E3ACtrl_EnableAIS,
    E3ACtrl_EnableFlareInManualCtrl,
    E3ACtrl_SetMinMaxFps,
    E3ACtrl_SetCCUCB,
    E3ACtrl_EnableTgInt,
    E3ACtrl_EnableBMDN,
    E3ACtrl_EnableMFHR,
    E3ACtrl_GetIsStrobeBVTrigger,
    E3ACtrl_SetStereoDualAELock,
    E3ACtrl_SetStereoAFLock,
    E3ACtrl_SetStereoAfStop,
    E3ACtrl_ResetMvhdrRatio,

    //AE get
    E3ACtrl_GetCurrentEV            = 0x0280,
    E3ACtrl_GetBVOffset,
    E3ACtrl_GetNVRAMParam,
    E3ACtrl_GetAEPLineTable,
    E3ACtrl_GetExposureInfo,
    E3ACtrl_GetExposureParam,
    E3ACtrl_GetInitExposureTime,
    E3ACtrl_GetAECapPLineTable,
    E3ACtrl_GetIsAEStable,
    E3ACtrl_GetRTParamsInfo,
    E3ACtrl_GetEvCapture,
    E3ACtrl_GetEvSetting,
    E3ACtrl_GetCaptureDelayFrame,
    E3ACtrl_GetSensorSyncInfo,
    E3ACtrl_GetSensorPreviewDelay,
    E3ACtrl_GetSensorDelayInfo,
    E3ACtrl_GetIsoSpeed,
    E3ACtrl_GetAEStereoDenoiseInfo,
    E3ACtrl_GetAEInitExpoSetting,
    E3ACtrl_GetAF2AEInfo,
    E3ACtrl_GetPrioritySetting,
    E3ACtrl_GetIsAEPlineIndexMaxStable,
    E3ACtrl_GetISOThresStatus,
    E3ACtrl_ResetGetISOThresStatus,
    E3ACtrl_EnableAEStereoManualPline,
    //ISP
//    E3ACtrl_ConvertToIspGamma
    E3ACtrl_GetIspGamma             = 0x0300,
    E3ACtrl_GetRwbInfo              = 0x0301,
    E3ACtrl_SetLcsoParam            = 0x0302,
    E3ACtrl_ValidatePass1           = 0x0303,
    E3ACtrl_SetIspProfile,
    E3ACtrl_GetOBOffset,
    E3ACtrl_GetLCEGain,

    //Flash
    E3ACtrl_GetQuickCalibration     = 0x0400,
    E3ACtrl_EnableFlashQuickCalibration,
    E3ACtrl_SetIsFlashOnCapture,
    E3ACtrl_GetIsFlashOnCapture,
    E3ACtrl_ChkMainFlashOnCond4StartCapture,
    E3ACtrl_ChkMFNRFlash,
    E3ACtrl_GetFlashCapDummyCnt,
    E3ACtrl_GetIsFlashLowPower,

    //3A misc set
    //3A misc get
    E3ACtrl_GetAsdInfo              = 0x0480,
    E3ACtrl_GetExifInfo,
    E3ACtrl_GetSupportedInfo,
    E3ACtrl_GetDualZoomInfo,
    E3ACtrl_GetCCUFrameSyncInfo,
    E3ACtrl_GetCurrResult,
    E3ACtrl_GetAAOIsReady,
    E3ACtrl_SetSync3ADevDoSwitch,
    E3ACtrl_SetCCUTGDynamicSwitch,

    //AF set
    E3ACtrl_SetAFMode               = 0x0500,
    E3ACtrl_SetAFMFPos,
    E3ACtrl_SetAFFullScanStep,
    E3ACtrl_SetAFCamMode,
    E3ACtrl_SetEnableOIS,
    E3ACtrl_SetEnablePBin,
    E3ACtrl_SetPureRawData,
    E3ACtrl_SetAF2AEInfo,

    //AF get
    E3ACtrl_GetAFDAFTable           = 0x0600,
    E3ACtrl_GetOISPos,
    E3ACtrl_Get_AF_FSC_INIT_INFO,
    E3ACtrl_NOTIFY_AF_FSC_INFO,

    //Flow control set
    E3ACtrl_Enable3ASetParams       = 0x1000,
    E3ACtrl_SetOperMode             = 0x1001,
    E3ACtrl_SetStereoParams         = 0x1002,
    E3ACtrl_EnableGyroSensor        = 0x1003,
    E3ACtrl_SetIsZsdCapture         = 0x1004,
    E3ACtrl_SwitchTo2D              = 0x1005,
    E3ACtrl_SwitchTo3D              = 0x1006,
    E3ACtrl_SetParamAEMode          = 0x1007,
    //Flow control get
    E3ACtrl_GetOperMode             = 0x2001,
    //online-tuning get size of mfb
    E3ACtrl_GetMfbSize              = 0x2002,
    E3ACtrl_SetCaptureMaxFPS,

    //Sync3A control
    E3ACtrl_Sync3A_IsActive,
    E3ACtrl_Sync3A_Sync2ASetting,
    E3ACtrl_Num
};

enum EBitMode_T
{
    EBitMode_10Bit = 0,
    EBitMode_12Bit,
    EBitMode_14Bit,
    EBitMode_16Bit
};

enum EHlrOption_T
{
    EHlrOption_Auto = 0,
    EHlrOption_ForceOff
};

enum EAFCtrl_T
{
    EAFCtrl_GetPostureDAC           = 0,
    EAFCtrl_GetCalibratedDistance   = 1,
    EAFCtrl_Num
};

struct ConfigInfo_T
{
    MINT32 i4SubsampleCount;    // for example : 120 fps => i4SubsampleCount = 4
    MINT32 i4RequestCount;      // The request count of set function(MW called)
    MBOOL bIsSecureCam;
    EBitMode_T i4BitMode;
    EHlrOption_T i4HlrOption;
    NSCam::IMetadata CfgHalMeta;
    NSCam::IMetadata CfgAppMeta;
    MUINT32 u4P1DirectYUV_Port;
    MINT32  i4CamFeatureMode;

    ConfigInfo_T()
        : i4SubsampleCount(1)
        , i4RequestCount(1)
        , bIsSecureCam(0)
        , i4BitMode(EBitMode_12Bit)
        , i4HlrOption(EHlrOption_ForceOff)
        , CfgHalMeta()
        , CfgAppMeta()
        , u4P1DirectYUV_Port(0)
        , i4CamFeatureMode(-1)
    {};

    ~ConfigInfo_T() {};
};

// AE init exposure setting for camera launch
struct AEInitExpoSetting_T
{
    MUINT32 u4SensorMode;       // input: sensor mode
    MUINT32 u4AETargetMode;     // input: AE target mode
    MUINT32 u4Eposuretime;      // output: AE sensor shutter (if HDR: long exposure)
    MUINT32 u4AfeGain;          // output: AE sensor gain (if HDR: long exposure)
    MUINT32 u4Eposuretime_se;   // output: AE short sensor shutter for HDR
    MUINT32 u4AfeGain_se;       // output: AE short sensor gain for HDR
    MUINT32 u4Eposuretime_me;   // output: AE middle sensor shutter for HDR
    MUINT32 u4AfeGain_me;       // output: AE middle sensor gain for HDR
    MUINT32 u4Eposuretime_vse;  // output: AE very short sensor shutter for HDR
    MUINT32 u4AfeGain_vse;      // output: AE very short sensor gain for HDR
    NSCam::IHalSensor::ConfigParam sensorCfg; // pass to ae_mgr for getting binning ratio (only support after isp_60)
};

struct shadingConfig_T
{
    MUINT32 AAOstrideSize;
    MUINT32 AAOBlockW;
    MUINT32 AAOBlockH;

    MUINT32 u4HBinWidth;
    MUINT32 u4HBinHeight;

    MINT32 i4BinWidth;
    MINT32 i4BinHight;

    shadingConfig_T()
        : AAOstrideSize(0)
        , AAOBlockW(0)
        , AAOBlockH(0)
        , u4HBinWidth(0)
        , u4HBinHeight(0)
        , i4BinWidth(0)
        , i4BinHight(0)
    {};
};

struct HAL3APerframeInfo_T
{
    MUINT32 u4P1DGNGain;
    MUINT64 u8P1Exposuretime_ns;

    HAL3APerframeInfo_T()
        : u4P1DGNGain(0)
        , u8P1Exposuretime_ns(0)
    {};

    ~HAL3APerframeInfo_T() {};
};


/******************************************************************************
 *  Sync3A Enum
 ******************************************************************************/
enum E_SYNC2A_MODE
{
    E_SYNC2A_MODE_IDLE               = 0,
    E_SYNC2A_MODE_NONE               = 1,
    E_SYNC2A_MODE_VSDOF              = 2,
    E_SYNC2A_MODE_DENOISE            = 3,
    E_SYNC2A_MODE_DUAL_ZOOM          = 4,
    E_SYNC2A_MODE_VSDOF_BY_FRAME     = 5,
    E_SYNC2A_MODE_DENOISE_BY_FRAME   = 6,
    E_SYNC2A_MODE_DUAL_ZOOM_BY_FRAME = 7
};

enum E_SYNCAF_MODE
{
    E_SYNCAF_MODE_IDLE = 0,
    E_SYNCAF_MODE_ON   = 1,
    E_SYNCAF_MODE_OFF  = 2,
};

enum E_HW_FRM_SYNC_MODE
{
    E_HW_FRM_SYNC_MODE_IDLE = 0,
    E_HW_FRM_SYNC_MODE_ON   = 1,
    E_HW_FRM_SYNC_MODE_OFF  = 2,
};

enum E_SYNC3A_NOTIFY
{
    E_SYNC3A_NOTIFY_NONE        = 0,
    E_SYNC3A_NOTIFY_SWITCH_ON   = 1,
};

struct Stereo_Param_T {
    MINT32  i4Sync2AMode;
    MINT32  i4SyncAFMode;
    MINT32  i4HwSyncMode;
    MINT32  i4MasterIdx;
    MINT32  i4SlaveIdx;
    MBOOL   bIsByFrame;
    MBOOL   bIsDummyFrame;
    MINT32  i4Sync3ASwitchOn;
    MINT32  i4ZoomRatio;
    MINT32  i4FrameNum;
    MBOOL   bIsChange4Master;
    MBOOL   bIsChange4Slave;

    Stereo_Param_T()
        : i4Sync2AMode(0)
        , i4SyncAFMode(0)
        , i4HwSyncMode(0)
        , i4MasterIdx(0)
        , i4SlaveIdx(0)
        , bIsByFrame(0)
        , bIsDummyFrame(0)
        , i4Sync3ASwitchOn(E_SYNC3A_NOTIFY_NONE)
        , i4ZoomRatio(100)
        , i4FrameNum(0)
        , bIsChange4Master(0)
        , bIsChange4Slave(0)
    {}
};


//E3ACtrl_GetFlashCapDummyCnt input
struct rHAL3AFlashCapDummyInfo_T
{
    MUINT32 u4AeMode;
    MUINT32 u4StrobeMode;
    MUINT32 u4CaptureIntent;

    rHAL3AFlashCapDummyInfo_T()
        : u4AeMode(1)
        , u4StrobeMode(0)
        , u4CaptureIntent(1)
    {};

    ~rHAL3AFlashCapDummyInfo_T() {};
};

//E3ACtrl_GetFlashCapDummyCnt output
struct rHAL3AFlashCapDummyCnt_T
{
    MUINT32 u4CntBefore;
    MUINT32 u4CntAfter;

    rHAL3AFlashCapDummyCnt_T()
        : u4CntBefore(0)
        , u4CntAfter(0)
    {};

    ~rHAL3AFlashCapDummyCnt_T() {};
};

}   //namespace NS3Av3
#endif //__AAA_HAL_COMMON_V3_H__

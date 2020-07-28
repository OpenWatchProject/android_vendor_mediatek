/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifndef _CCUIF_COMPAT_H_
#define _CCUIF_COMPAT_H_

//#include "ccu_sensor_if.h"
#include "ccu_ext_interface/ccu_ext_interface.h"
#include "algorithm/ae_algo_ctrl.h"


/*******************************************************************************
* ARM linux kernel 64 -> CCU 32 compatible
* Make all pointers as 32bit MVA,
* avoiding pointer type compatible problem in kernel driver
********************************************************************************/
#define CCU_COMPAT_PTR_T MUINT32

namespace NSCcuIf {

typedef struct COMPAT_SENSOR_INFO_IN
{
    //U32 u32SensorId;
    U16 u16FPS;                     // Current FPS used by set_mode()
    U32 eScenario;            // Preview, Video, HS Video, and etc.
    U32 sensorI2cSlaveAddr;
    U32 dma_buf_va_h;
    U32 dma_buf_va_l;
    U32 u32I2CId;
    CCU_COMPAT_PTR_T dma_buf_mva;  // virtual address of I2C DMA buffer
    U32 sensor_idx;
    U32 sensor_slot;
    bool isSpSensor;
} COMPAT_SENSOR_INFO_IN_T;

typedef struct {
    MUINT16 u2Length;
} COMPAT_CCU_ISP_NVRAM_ISO_INTERVAL_STRUCT;

typedef struct
{
    MUINT32 u4FrameIdx;
    CCU_strEvSetting xSetting;
    CCU_COMPAT_PTR_T pNext;
} COMPAT_CCU_EV_CTRL;

typedef struct
{
    CCU_COMPAT_PTR_T pHead;
    CCU_COMPAT_PTR_T pTail;
    COMPAT_CCU_EV_CTRL xEvQueue[MAX_AE_PRE_EVSETTING];
} COMPAT_CCU_EV_QUEUE_CTRL;


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
    COMPAT_CCU_EV_QUEUE_CTRL m_EVCtrl;
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
	CCU_AE_STABLE_CASE m_StableCase;
    CCU_AE_STABLE_CASE m_StableCaseCur;
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
    CCU_COMPAT_PTR_T m_pCurMovingRatio;
    CCU_COMPAT_PTR_T m_pCurrentTable;
    CCU_COMPAT_PTR_T m_pCurrentTableF;
    CCU_COMPAT_PTR_T pCurrentGainList;
    CCU_COMPAT_PTR_T pAeNVRAM;

    /*Delete*/
    /*
    MUINT32 ReqNumberHead;
    MUINT16 u2LengthV2;
    MINT16 IDX_PartitionV2[NVRAM_ISP_REGS_ISO_SUPPORT_NUM];
    CCU_ISP_NVRAM_OBC_T OBC_TableV2[NVRAM_ISP_REGS_ISO_SUPPORT_NUM];
    CCU_ISP_NVRAM_OBC_T OBC_Table[4];
    */
    /*Delete*/
} COMPAT_CCU_AeAlgo;



typedef struct
{
    MBOOL Call_AE_Core_init;
    strAERealSetting InitAESetting;
    MUINT32 u4LastIdx;
    MUINT32 EndBankIdx;
    MUINT32 u4InitIndex;

    MINT32  m_i4AEMaxBlockWidth;
    MINT32  m_i4AEMaxBlockHeight;
    MUINT32 m_u4AETarget;

    MUINT32 m_LineWidth;
    MUINT32 m_u4IndexFMax;
    MUINT32 m_u4IndexFMin;
    MUINT32 m_u4IndexMax;
    MUINT32 m_u4IndexMin;
    MUINT32 m_u4FinerEVIdxBase; // Real index base
    MUINT32 m_u4AAO_AWBValueWinSize;            // bytes of AWB Value window
    MUINT32 m_u4AAO_AWBSumWinSize;              // bytes of AWB Sum window
    MUINT32 m_u4AAO_AEYWinSize;                 // bytes of AEY window
    MUINT32 m_u4AAO_AEYWinSizeSE;                 // bytes of AEY window
    MUINT32 m_u4AAO_AEOverWinSize;              // bytes of AEOverCnt window
    MUINT32 m_u4AAO_HistSize;                   // bytes of each Hist
    MUINT32 m_u4AAO_HistSizeSE;                   // bytes of each Hist

    MINT32 i4MaxBV;
    MINT32 i4MinBV;
    MINT32 i4BVOffset;
    MUINT16 u2Length;

    CCU_LIB3A_AE_METERING_MODE_T eAEMeteringMode;
    CCU_LIB3A_AE_SCENE_T eAEScene;
    CCU_LIB3A_AECAM_MODE_T eAECamMode;
    CCU_LIB3A_AE_FLICKER_MODE_T eAEFlickerMode;
    CCU_LIB3A_AE_FLICKER_AUTO_MODE_T eAEAutoFlickerMode;
    CCU_LIB3A_AE_EVCOMP_T eAEEVcomp;
    MUINT32   u4AEISOSpeed;
    MINT32    i4AEMaxFps;
    MINT32    i4AEMinFps;
    MINT32    i4SensorMode;
    MUINT32   u4ExpUnit;
    CCU_AE_SENSOR_DEV_T eSensorDev;
    CCU_CAM_SCENARIO_T eAEScenario;
    CCU_AE_TargetMODE m_InitAETargetMode;

    /*ISP 6.0*/
    CCU_AWBSTAT_CONFIG_BLK AWBStatConfig;
    CCU_AAOConfig_T AAOStatConfig;


    /*!!!!!!!!!!!!!!! PONITER MUST BE PUT AT BOTTOM MOST (due to CCU driver constraint) !!!!!!!!!!!!!!!!*/
    CCU_COMPAT_PTR_T m_LumLog2x1000;
    CCU_COMPAT_PTR_T pCurrentTableF;
    CCU_COMPAT_PTR_T pCurrentTable;
    CCU_COMPAT_PTR_T pCurrentGainList;
    CCU_COMPAT_PTR_T pEVValueArray;
    CCU_COMPAT_PTR_T pAETouchMovingRatio;
    CCU_COMPAT_PTR_T pAeNVRAM;
    CCU_COMPAT_PTR_T prAeSyncNvram;
    CCU_COMPAT_PTR_T pCusAEParam;
}COMPAT_AE_CORE_INIT;

typedef struct
{
    COMPAT_AE_CORE_INIT algo_init_param;
    CCU_COMPAT_PTR_T sync_algo_init_param;
} COMPAT_CCU_AE_ALGO_INITI_PARAM_T;

// Note: need to ensure the structure define are same as a_algo_ctrl.h - AE_CORE_CTRL_RUN_TIME_INFO
typedef struct
{
    MINT32 i4MaxBV;
    MINT32 i4MinBV;
    MINT32 i4CapMaxBV;
    MINT32 i4CapMinBV;
    MUINT32 u4TotalIndex;
    MUINT32 u4CapTotalIndex;
    CCU_eAETableID  eID;
    CCU_eAETableID  eCapID;
    MUINT32 m_u4FinerEVIdxBase;
    MUINT32 u4UpdateLockIndex;
    MUINT32 u4IndexMax;
    MUINT32 u4IndexMin;
    MUINT32 u4NvramIdx;
    CCU_AE_PRIORITY_INFO PriorityInfo;
    AE_LTM_NVRAM_PARAM LtmNvramParam;

    CCU_COMPAT_PTR_T pCurrentTable;
    CCU_COMPAT_PTR_T pCurrentTableF;
    CCU_COMPAT_PTR_T pAeNVRAM;
    CCU_COMPAT_PTR_T pCusAEParam;
} COMPAT_AE_CORE_CTRL_RUN_TIME_INFO;

typedef struct
{
    CCU_SYNCAE_CAMERA_TYPE_ENUM eCamType;
    CCU_LIB3A_AE_FLICKER_MODE_T eAeFlickerMode;
    CCU_LIB3A_AE_FLICKER_AUTO_MODE_T eAeAutoFlickerMode;

    MINT32 i4SensorMode;
    MUINT32 u4BaseIdxF;
    MUINT32 u4BaseISPGain;
    MUINT32 u4AvgY;

    MUINT32 u4ExpUnit;
    MUINT32 u4ShutterBase;

    CCU_SYNCAE_U4RANGE_T rAeIdxFRange;
    CCU_SYNCAE_U4RANGE_T rExpRange;
    CCU_SYNCAE_U4RANGE_T rAfeGainRange;
    CCU_SYNCAE_U4RANGE_T rIspGainRange;
    CCU_SYNCAE_I4RANGE_T rBvFRange;

    CCU_strAEOutput rInput;
    CCU_strAEOutput rPrevInput;

    CCU_COMPAT_PTR_T prPlineTableF;
    CCU_COMPAT_PTR_T prAeNvram;
    CCU_COMPAT_PTR_T prSensorGainTable;
    CCU_COMPAT_PTR_T prAeSyncNvram;
} COMPAT_CCU_SYNC_AE_CAM_INPUT_T;

typedef struct
{
    MUINT16 u2SyncNum;
    MUINT16 u2MasterId;
    CCU_SYNC_AE_SYNC_TYPE_ENUM eSyncType;
    COMPAT_CCU_SYNC_AE_CAM_INPUT_T rCamInput[CCU_SYNC_AE_SUPPORT_SENSORS];
} COMPAT_CCU_SYNC_AE_INPUT_T;

typedef struct COMPAT_LENS_INFO_IN
{
    char szLensName[32];
    U32 dma_buf_va_h;
    U32 dma_buf_va_l;
    U32 u32I2CId;
    U32 sensor_slot;
    CCU_COMPAT_PTR_T dma_buf_mva;  // virtual address of I2C DMA buffer
} COMPAT_LENS_INFO_IN_T;

typedef struct AFOINIT_INFO_IN
{
    U32 AFOBufsAddr[Half_AFO_BUFFER_COUNT];
    U32 AFORegAddr[Half_AF_REG_COUNT];
    /*
    U32 AFOBufsAddr2[3];
    U32 AFORegAddr2[3];
    */
    U32 magic_reg;
    U32 twin_reg;
    COMPAT_LENS_INFO_IN_T lens_info;

} COMPAT_AFOINIT_INFO_IN_T;

struct COMPAT_ccu_ltm_init_data_in
{
    //Overwrite by CCU drv.
    uint32_t ltmso_ring_buf_addr[CCU_LTMSO_RING_SIZE];  //REG_LTMSO_R1_LTMSO_BASE_ADDR

    uint32_t ltm_en;
    uint32_t hlr_en;
    uint32_t ltm_nvram_size;
    uint32_t hlr_nvram_size;
    uint32_t ltm_nvram_num;
    uint32_t hlr_nvram_num;
    CCU_COMPAT_PTR_T ltm_nvram_addr;
    CCU_COMPAT_PTR_T hlr_nvram_addr;
};


};  //namespace NSCcuIf
#endif  //  _CCUIF_H_


/* Copyright Statement:
*
* This software/firmware and related documentation ("MediaTek Software") are
* protected under relevant copyright laws. The information contained herein
* is confidential and proprietary to MediaTek Inc. and/or its licensors.
* Without the prior written permission of MediaTek inc. and/or its licensors,
* any reproduction, modification, use or disclosure of MediaTek Software,
* and information contained herein, in whole or in part, shall be strictly prohibited.
*/
/* MediaTek Inc. (C) 2010. All rights reserved.
*
* BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
* THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
* RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
* AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
* NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
* SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
* SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
* THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
* THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
* CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
* SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
* STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
* CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
* AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
* OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
* MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
* The following software/firmware and/or related documentation ("MediaTek Software")
* have been modified by MediaTek Inc. All revisions are subject to any receiver's
* applicable license agreements with MediaTek Inc.
*/
#ifndef _AWB_PARAM_IF_H
#define _AWB_PARAM_IF_H

// Light source probability
typedef struct
{
    MINT32 i4P0[AWB_LIGHT_NUM]; // Probability 0
    MINT32 i4P1[AWB_LIGHT_NUM]; // Probability 1
    MINT32 i4P2[AWB_LIGHT_NUM]; // Probability 2
    MINT32 i4P[AWB_LIGHT_NUM];  // Probability P0 x P1 x P2 & Normalized
    MINT32 i4P3[AWB_LIGHT_NUM]; // Probability 3 - FacePredictor
    MINT32 i4FinalGainProb[AWB_LIGHT_NUM];      // Final Probability after Post-process (FacePredictor, ...)
    MINT32 i4FaceAstGainProb[AWB_LIGHT_NUM];    // Probability of Face Predictor, PfaceAst = (100 - (i4P * i4P3))
} AWB_LIGHT_PROBABILITY_N3D_T;

typedef struct
{
	MINT32 i4SumR; // R summation of specified light source of specified parent block
	MINT32 i4SumG; // G summation of specified light source of specified parent block
	MINT32 i4SumB; // B summation of specified light source of specified parent block
	MINT32 i4ChildBlkNum; // Child block number of specified light source of specified parent block
	MINT32 i4Light; // Light source of specified parent block
	XY_COORDINATE_T rXY;    // XrYr of specified parent block
} AWB_PARENT_BLK_STAT_N3D_T;

// Light source statistics
typedef struct
{
    MINT32 i4WeightedSumR[AWB_LIGHT_NUM];         // Weighted R summation of specified light source
    MINT32 i4WeightedSumG[AWB_LIGHT_NUM];         // Weighted G summation of specified light source
    MINT32 i4WeightedSumB[AWB_LIGHT_NUM];         // Weighted B summation of specified light source
    MINT32 i4WeightedWindowNum[AWB_LIGHT_NUM];    // Weighted window number of specified light source
    MINT32 i4ParentBlkNum[AWB_LIGHT_NUM];         // Parent block number of specified light source
    MINT32 i4WeightedParentBlkNum[AWB_LIGHT_NUM]; // Weighted parent block number of specified light source
    XY_COORDINATE_T rRotatedXY[AWB_LIGHT_NUM];    // Rotated XY coordinate (for debug purpose)
    MUINT32 u4CalibratedWeightedSumR[AWB_LIGHT_NUM]; // Calibrated weighted R summation of specified light source
    MUINT32 u4CalibratedWeightedSumG[AWB_LIGHT_NUM]; // Calibrated weighted G summation of specified light source
    MUINT32 u4CalibratedWeightedSumB[AWB_LIGHT_NUM]; // Calibrated weighted B summation of specified light source
    MINT32 i4LimitedWeightedSumR[AWB_LIGHT_NUM];         // Weighted R summation of specified light source
    MINT32 i4LimitedWeightedSumG[AWB_LIGHT_NUM];         // Weighted G summation of specified light source
    MINT32 i4LimitedWeightedSumB[AWB_LIGHT_NUM];         // Weighted B summation of specified light source
    XY_COORDINATE_T rLimitedRotatedXY[AWB_LIGHT_NUM];    // Rotated XY coordinate (for debug purpose)
} AWB_LIGHT_STAT_N3D_T;

// AWB info for ISP tuning
typedef struct
{
    AWB_LIGHT_PROBABILITY_N3D_T rProb; // Light source probability
    AWB_LIGHT_STAT_N3D_T rLightStat; // Light source statistics
    AWB_GAIN_T rLightAWBGain[AWB_CAL_LIGHT_SRC_NUM]; // Golden sample's AWB gain for multi-CCM
    AWB_GAIN_T rAlgGain;
    AWB_GAIN_T rCurrentAWBGain; // Current AWB gain
    AWB_GAIN_T rCurrentFullAWBGain; // Current AWB gain without preference
    MINT32 i4NeutralParentBlkNum; // Neutral parent block number
    MINT32 i4CCT; // CCT
    MINT32 i4FluorescentIndex; // Fluorescent index
    MINT32 i4DaylightFluorescentIndex; // Daylight fluorescent index
    MINT32 i4SceneLV; // Scene LV
    MINT32 i4AWBMode; // AWB mode
    MBOOL  bAWBStable; // AWB stable
    AWB_GAIN_T rPregain1; // AA pregain1
    MBOOL  bCscEnable; // AA CCM enable
    MINT32 rCscCCM[9]; // AA CCM
    MBOOL bAWBLock;
    AWB_GAIN_T rAwbGainNoPref; //AWB gain without preference
    MINT32 i4PanelXYDiff;
	  MINT32 i4PanelProb;

	//-----Not Related to MTK AWB algo and won't maintain this part-----//
    AWB_GAIN_T rRPG;
	AWB_GAIN_T rPGN;
    MBOOL bCustCCMEnable; //Customized CCM Enable
    MINT32 rCustCCM[9]; // Customized CCM
    //------------------------------------------------------------------//
} AWB_INFO_N3D_T;

typedef struct
{
    AWB_GAIN_T rAlgGain;
    AWB_GAIN_T rCurrentGain;
    AWB_GAIN_T rTargetGain;
    MINT32 i4CCT;
    MINT32 i4LightMode;
    MINT32 i4SceneLV;
    AWB_LIGHT_PROBABILITY_N3D_T rLightProb;
    AWB_GAIN_T rAwbGainNoPref; //AWB gain without preference
    AWB_PARENT_BLK_STAT_N3D_T rAwbParentStatBlk[AWB_PARENT_BLK_NUM_MAX_Y][AWB_PARENT_BLK_NUM_MAX_X]; //New add
    MINT32 ParentBlkNumX; //New add
    MINT32 ParentBlkNumY; //New add

    // ====== AWB Standby Sync ===========
    MINT32  i4XR;
    MINT32  i4YR;
    MINT32  LockXR;
    MINT32  LockYR;
    MINT32  ZoomRatio;
    MINT32  i4SyncMode;
    MINT32  i4SyncOpt;
    MINT32  i4MasterDev;
    MBOOL   i4Smoothflag;
    MBOOL   i4SyncEn;
    AWB_GAIN_T rStandbySyncGain;
    AWB_GAIN_T rSynclockAWBGain;
    AWB_GAIN_T rPreviousAWBGain;
    
} AWB_SYNC_INPUT_N3D_IF_T;

typedef struct
{
    AWB_GAIN_T rAWBGain;
    MINT32 i4CCT;
    MINT32 i4XR;
    MINT32 i4YR;
    AWB_GAIN_T rSyncAWBGain;
    MINT32 i4PreviewCam;
} AWB_SYNC_OUTPUT_N3D_IF_T;

typedef struct
{
    AWB_GAIN_T rPreviewAWBGain;              // AWB gain for DIP
    AWB_GAIN_T rPreviewStrobeAWBGain;        // AWB gain for DIP
    AWB_GAIN_T rPreviewFullWBGain;           // AWB gain without preference
    AWB_GAIN_T rCaptureAWBGain;              // AWB gain for DIP
    AWB_GAIN_T rCaptureFullWBGain;           // AWB gain without preference
    AWB_GAIN_T rPreviewRAWPreGain2;          // AWB gain for AE statistics
    AWB_GAIN_T rPreviewStrobeRAWPreGain2;    // AWB gain for AE statistics
    AWB_INFO_N3D_T rAWBInfo;                     // AWB info
    AWB_SYNC_INPUT_N3D_IF_T rAWBSyncInput_N3D;  // N3D AWB info
    AWB_SYNC_OUTPUT_N3D_IF_T rAWBSyncOutput_N3D; // N3D AWB info
} AWB_OUTPUT_N3D_T;


#endif
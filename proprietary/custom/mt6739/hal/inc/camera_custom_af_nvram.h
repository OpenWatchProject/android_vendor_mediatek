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
#ifndef _CAMERA_CUSTOM_AF_NVRAM_H_
#define _CAMERA_CUSTOM_AF_NVRAM_H_

#include <stddef.h>
#include "MediaTypes.h"

#define NVRAM_CUSTOM_REVISION   7453001

/*******************************************************************************
* AF
********************************************************************************/

//____AF NVRAM____

#define AF_TABLE_NUM (30)
#define ISO_MAX_NUM   (8)
#define GMEAN_MAX_NUM (6)

#define JUMP_NUM        (5)
#define STEPSIZE_NUM    (15)
#define PRERUN_NUM      (2)
#define FPS_THRES_NUM   (2)


typedef struct
{
    MINT32 i4InfPos;
    MINT32 i4MacroPos;

} FOCUS_RANGE_T;

typedef struct
{
    MINT32 i4Enable;

    MINT32 i4ChgType;          // 1 is &&, 0 is ||
    MINT32 i4ChgOffset;        // value -> 0 more sensitive
    MINT32 i4ChgThr[3];        // percentage -> 0 more sensitive; level 1~3 is stable~sensitive
    MINT32 i4ChgCnt[3];        // level 1~3 is stable~sensitive

    MINT32 i4StbType;          // 1 is &&, 0 is ||
    MINT32 i4StbOffset;        // value -> 0 more stable
    MINT32 i4StbThr[3];        // percentage -> 0 more stable; level 1~3 is stable~sensitive
    MINT32 i4StbCnt[3];        // level 1~3 is stable~sensitive
} SCENE_MONITOR_T;


#define PD_CALI_DATA_SIZE   (2048)
#define PD_CONF_IDX_SIZE    (5)

typedef struct
{
    MINT32  i4FocusPDSizeX;
    MINT32  i4FocusPDSizeY;
    MINT32  i4ConfIdx1[PD_CONF_IDX_SIZE];
    MINT32  i4ConfIdx2[PD_CONF_IDX_SIZE];
    MINT32  i4ConfTbl[PD_CONF_IDX_SIZE+1][PD_CONF_IDX_SIZE+1];
    MINT32  i4SaturateLevel;
    MINT32  i4SaturateThr;
    MINT32  i4ConfThr;
    MINT32  i4Reserved[20];
    MINT32  i4V5Config[64];
} PD_ALGO_TUNING_T;

typedef struct
{
    MUINT8  uData[PD_CALI_DATA_SIZE];
    MINT32  i4Size;
} PD_CALIBRATION_DATA_T;

typedef struct
{
    PD_CALIBRATION_DATA_T rCaliData; //this field is only for function parseCaliData(), pd calibration data in nvram is located at NVRAM_AF_PARA_STRUCT
    PD_ALGO_TUNING_T rTuningData;
} PD_NVRAM_T;

typedef struct
{
    MUINT32 i4BufferState;
    MUINT32 i4PosStatCnt;
    MUINT32 i4PosData[1024];
} DUALCAM_NVRAM_T;

typedef struct
{
    MINT32  i4Offset;
    MINT32  i4NormalNum;
    MINT32  i4MacroNum;
    MINT32  i4InfIdxOffset;
    MINT32  i4MacroIdxOffset;
    MINT32  i4Pos[AF_TABLE_NUM];

} NVRAM_AF_TABLE_T;

typedef struct
{
    MINT32 i4ISONum;
    MINT32 i4ISO[ISO_MAX_NUM];

    MINT32 i4GMR[7][ISO_MAX_NUM];

    MINT32 i4MIN_TH[ISO_MAX_NUM];
    MINT32 i4HW_TH[ISO_MAX_NUM];

    MINT32 i4MIN_TH1[ISO_MAX_NUM];
    MINT32 i4HW_TH1[ISO_MAX_NUM];

    MINT32 i4MIN_TH2[ISO_MAX_NUM];
    MINT32 i4HW_TH2[ISO_MAX_NUM];

} NVRAM_AF_THRES_T;

typedef struct
{
    NVRAM_AF_TABLE_T sTABLE;
    MINT32 i4THRES_MAIN;
    MINT32 i4THRES_SUB;
    MINT32 i4AFC_FAIL_CNT;
    MINT32 i4FAIL_POS;

    MINT32 i4INIT_WAIT;
    MINT32 i4FRAME_WAIT[JUMP_NUM];
    MINT32 i4DONE_WAIT;
} NVRAM_AF_COEF;

typedef struct
{
    // --- AF SW coef ---
    NVRAM_AF_COEF sAF_Coef;         // for normal

    // --- AF HW thres ---
    NVRAM_AF_THRES_T sAF_TH;        // sensor preview mode


    // --- Common use ---
    MINT32 i4ReadOTP;               // 0 : disable, 1:enable
    MINT32 i4StatGain;
    MINT32 i4LV_THRES;
    MINT32 i4InfPos;
    MINT32 i4FRAME_TIME;
    MINT32 i4BackJump[JUMP_NUM];
    MINT32 i4BackJumpPos;
    // AFC & VAFC
    MINT32 i4AFC_STEP_SIZE;
    MINT32 i4SPOT_PERCENT_W;        // AFC window location
    MINT32 i4SPOT_PERCENT_H;        // AFC window location
    MINT32 i4CHANGE_CNT_DELTA;

    // AFS
    MINT32 i4AFS_STEP_MIN_ENABLE;
    MINT32 i4AFS_STEP_MIN_NORMAL;
    MINT32 i4AFS_STEP_MIN_MACRO;

    // FV Monitor
    MINT32 i4FIRST_FV_WAIT;
    MINT32 i4FV_1ST_STABLE_THRES;
    MINT32 i4FV_1ST_STABLE_OFFSET;
    MINT32 i4FV_1ST_STABLE_NUM;
    MINT32 i4FV_1ST_STABLE_CNT;
    MINT32 i4FV_SHOCK_THRES;
    MINT32 i4FV_SHOCK_OFFSET;
    MINT32 i4FV_VALID_CNT;
    MINT32 i4FV_SHOCK_FRM_CNT;
    MINT32 i4FV_SHOCK_CNT;

    // --- FaceAF ---
    MINT32 i4FDWinPercent;
    MINT32 i4FDSizeDiff;
    MINT32 i4FD_DETECT_CNT;
    MINT32 i4FD_NONE_CNT;

    // --- AFv1.1/AFv1.2 ---
    MINT32 i4LeftSearchEnable;      //[0] enable left peak search if i4Coef[0] != 0
    MINT32 i4LeftJumpStep;          //[1] disable left peak search, left step= 3 + i4Coef[1]
    MINT32 i4Curve5ptFit;           //[2] enable 5 point curve fitting if i4Coef[2] != 0
    MINT32 i4AfDoneDelay;           //[3] AF done happen delay count
    MINT32 i4ZoomInWinChg;          //[4] enable AF window change with Zoom-in
    MINT32 i4SensorEnable;          //[5] AF use sensor lister => 0:disable, 1:enable
    MINT32 i4PostureComp;           //[6] post comp max offset => 0:disable, others:enable
    MINT32 i4SceneMonitorLevel;     //[7] scenechange enhancement level => 0:original, 1~3:;level stable to sensitive, 9:use coef

    // Scene Monitor
    SCENE_MONITOR_T sFV;
    SCENE_MONITOR_T sGS;
    SCENE_MONITOR_T sAEB;
    SCENE_MONITOR_T sGYRO;
    SCENE_MONITOR_T sACCE;


    // --- AFv2.0 ---
    // Black faceAF
    MINT32 i4FvExtractEnable;               // 0 is disable, > 0 is enable
    MINT32 i4FvExtractThr;                  // percentage, fix
    // Damping compensation
    MINT32 i4DampingCompEnable;             // 0 is disable, 1 is enable
    MINT32 i4DampingStep[STEPSIZE_NUM];     //{ step1, step2, ... }
    // Tunable lens search direction
    MINT32 i4DirSelectEnable;               // 0 is disable, 1 is enable
    MINT32 i4InfDir;                        // 1: right search
    MINT32 i4MidDir;                        // 2: reverse search ; 0: keep search
    MINT32 i4MacDir;                        // -1: left search
    MINT32 i4RatioInf;                      // below % of path belongs to INF
    MINT32 i4RatioMac;                      // above % of path belongs to MAC
    // Tunable lens search startPos
    MINT32 i4StartBoundEnable;              // 0 is disable, 1 is enable
    MINT32 i4StartCamCAF;                   // -1: force start from idx 0 (INF)
    MINT32 i4StartCamTAF;                   // X: if close to boundary in X steps, start from boundary

    // FPS-adjusted AF table
    MINT32 i4FpsRemapTblEnable;             // 0 is disable, 1 is enable
    MINT32 i4FpsThres[FPS_THRES_NUM];       // fps = {12, 15}
    MINT32 i4TableClipPt[FPS_THRES_NUM];    // = {2, 1};
    MINT32 i4TableRemapPt[FPS_THRES_NUM];   // = {7, 8};

    // --- Easytuning ---
    MINT32 i4EasyTuning[100];               // AF easy tuning paremeters

    // --- Damping ---
    MINT32 i4DampingTbls[500];

    // --- AFv2.5 ---
    MINT32 i4Coefs[100];
    // [0] inclinePeak rejection; 0: is disable, 1: is enable
    // [1] fdOffMaxCnt, fd turnOff maxCnt for exiting faceAF; 0: use default value 10
    // [2] firstCaf force trigger cnt
    // [3] -> moved to i4HybridAFCoefs[]
    // [4] -> moved to i4LDAFCoefs[]
    // [5] -> moved to i4LDAFCoefs[]
    // [6] fd seek retrigger ratio
    // [7] ot size diff trigger
    // [8~10] no used
    // [11~20] -> moved to i4SDAFCoefs[]
    // [21~30] -> moved to i4PDAFCoefs[]
    // [31]~[40] -> moved to i4HybridAFCoefs[]
    // [41]~[50] -> moved to i4PLAFCoefs[]
    // [51]~[60] reserved
    // [61]~[70] -> moved to i4LDAFCoefs[]
    // [71]~[85] -> moved to i4Revs[]
    // [86]~[87] -> moved to i4Revs[]

    // --- AFv3.0 ---
    MINT32 i4AFv3Config[64];
    MINT32 i4PLAFCoefs[64];
    MINT32 i4HybridAFCoefs[128];
    MINT32 i4PDAFCoefs[64];
    MINT32 i4LDAFCoefs[64];
    MINT32 i4SDAFCoefs[64];
    MINT32 i4FaceAFCoefs[64];
    MINT32 i43DAFCoefs[64];
    MINT32 i4MultiAFCoefs[64];
    MINT32 i4TempErr[64];
    MINT32 i4ZoEffect[64];
    MINT32 i4DualAFCoefs[512];

    // --- Reserved ---
    MINT32 i4Revs[256];
} AF_NVRAM_T;

#endif // _CAMERA_CUSTOM_3A_NVRAM_H_


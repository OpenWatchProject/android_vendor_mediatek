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
/**
 * @file af_mgr.h
 * @brief AF manager, do focusing for raw sensor.
 */
#ifndef _IAfCxUPARAM_H_
#define _IAfCxUPARAM_H_

#ifdef __cplusplus
extern "C" {
#endif //#ifdef __cplusplus
#include <af_algo_if.h>
#include <af_param.h>
#ifdef __cplusplus
} // extern "C" {
#endif //#ifdef __cplusplus

#include <af_define.h>

//=============== from isp_af_config.h - Start
#ifndef __PLATFORM__// CPU
#define ERegInfo_AF(FLD) ERegInfo_AF_R1_##FLD
#define ERegInfo_AFO(FLD) ERegInfo_AFO_R1_##FLD
#define ERegInfo_CAMCTL(FLD) ERegInfo_CAMCTL_R1_##FLD
#define ERegInfo_CRP(FLD) ERegInfo_CRP_R1_##FLD
#else               // CCU
#define ERegInfo_AF(FLD) ERegInfo_AF_R1A_##FLD
#define ERegInfo_AFO(FLD) ERegInfo_AFO_R1A_##FLD
#define ERegInfo_CAMCTL(FLD) ERegInfo_CAMCTL_R1A_##FLD
#define ERegInfo_CRP(FLD) ERegInfo_CRP_R1A_##FLD
#endif

#define FORCE_CPUAF_ENABLE 0

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

enum
{
    ERegInfo_AFO(AFO_BASE_ADDR),
    ERegInfo_CAMCTL(CAMCTL_EN2),
    ERegInfo_CAMCTL(CAMCTL_DMA_EN),
    ERegInfo_CAMCTL(CAMCTL_SEL),
    ERegInfo_CRP(CRP_X_POS),
    ERegInfo_AF(AF_SIZE),
    ERegInfo_AF(AF_VLD),
    ERegInfo_AF(AF_BLK_1),
    ERegInfo_AFO(AFO_OFST_ADDR),
    ERegInfo_AFO(AFO_XSIZE),
    ERegInfo_AFO(AFO_YSIZE),
    ERegInfo_AFO(AFO_STRIDE),
    ERegInfo_AF(AF_CON),
    ERegInfo_AF(AF_CON2),
    ERegInfo_AF(AF_BLK_PROT),
    ERegInfo_AF(AF_BLK_0),
    ERegInfo_AF(AF_HFLT0_1),
    ERegInfo_AF(AF_HFLT0_2),
    ERegInfo_AF(AF_HFLT0_3),
    ERegInfo_AF(AF_HFLT1_1),
    ERegInfo_AF(AF_HFLT1_2),
    ERegInfo_AF(AF_HFLT1_3),
    ERegInfo_AF(AF_HFLT2_1),
    ERegInfo_AF(AF_HFLT2_2),
    ERegInfo_AF(AF_HFLT2_3),
    ERegInfo_AF(AF_VFLT_1),
    ERegInfo_AF(AF_VFLT_2),
    ERegInfo_AF(AF_VFLT_3),
    ERegInfo_AF(AF_PL_HFLT_1),
    ERegInfo_AF(AF_PL_HFLT_2),
    ERegInfo_AF(AF_PL_HFLT_3),
    ERegInfo_AF(AF_PL_VFLT_1),
    ERegInfo_AF(AF_PL_VFLT_2),
    ERegInfo_AF(AF_PL_VFLT_3),
    ERegInfo_AF(AF_TH_0),
    ERegInfo_AF(AF_TH_1),
    ERegInfo_AF(AF_TH_2),
    ERegInfo_AF(AF_TH_3),
    ERegInfo_AF(AF_TH_4),
    ERegInfo_AF(AF_LUT_H0_0),
    ERegInfo_AF(AF_LUT_H0_1),
    ERegInfo_AF(AF_LUT_H0_2),
    ERegInfo_AF(AF_LUT_H0_3),
    ERegInfo_AF(AF_LUT_H0_4),
    ERegInfo_AF(AF_LUT_H1_0),
    ERegInfo_AF(AF_LUT_H1_1),
    ERegInfo_AF(AF_LUT_H1_2),
    ERegInfo_AF(AF_LUT_H1_3),
    ERegInfo_AF(AF_LUT_H1_4),
    ERegInfo_AF(AF_LUT_H2_0),
    ERegInfo_AF(AF_LUT_H2_1),
    ERegInfo_AF(AF_LUT_H2_2),
    ERegInfo_AF(AF_LUT_H2_3),
    ERegInfo_AF(AF_LUT_H2_4),
    ERegInfo_AF(AF_LUT_V_0),
    ERegInfo_AF(AF_LUT_V_1),
    ERegInfo_AF(AF_LUT_V_2),
    ERegInfo_AF(AF_LUT_V_3),
    ERegInfo_AF(AF_LUT_V_4),
    ERegInfo_AF(AF_SGG1_0),
    ERegInfo_AF(AF_SGG1_1),
    ERegInfo_AF(AF_SGG1_2),
    ERegInfo_AF(AF_SGG1_3),
    ERegInfo_AF(AF_SGG1_4),
    ERegInfo_AF(AF_SGG1_5),
    ERegInfo_AF(AF_SGG5_0),
    ERegInfo_AF(AF_SGG5_1),
    ERegInfo_AF(AF_SGG5_2),
    ERegInfo_AF(AF_SGG5_3),
    ERegInfo_AF(AF_SGG5_4),
    ERegInfo_AF(AF_SGG5_5),
    EAFRegInfo_NUM
};

enum
{
    EHWCONSTRAINT_1 = 1,
    EHWCONSTRAINT_2,
    EHWCONSTRAINT_3,
    EHWCONSTRAINT_4,
    EHWCONSTRAINT_5,
    EHWCONSTRAINT_6,
    EHWCONSTRAINT_7,
    EHWCONSTRAINT_8,
    EHWCONSTRAINT_9,
    EHWCONSTRAINT_10,
    EHWCONSTRAINT_11,
    EHWCONSTRAINT_12,
    EHWCONSTRAINT_13,
};

typedef struct AFISPREG_INFO
{
    MUINT32     addr; ///< address
    MUINT32     val;  ///< value
} AFISPREG_INFO_T;

typedef AFISPREG_INFO_T   AFRegInfo_T;

typedef struct AFRESULT_ISPREG_t
{
    MBOOL      enableAFHw;
    MBOOL      isApplied;
    MINT32     configNum;
    AFRegInfo_T rAFRegInfo[EAFRegInfo_NUM];
} AFRESULT_ISPREG_T;
//=============== from isp_af_config.h - End


typedef struct AF_CONFIG_INFO_t
{
    AREA_T hwArea;
    MINT32 hwBlkNumX;
    MINT32 hwBlkNumY;
    MINT32 hwBlkSizeX;
    MINT32 hwBlkSizeY;
    MINT32 hwEnExtMode;
    MINT32 hwConstrainErr;
} AF_CONFIG_INFO_T;

// structure for afmgr::config()
typedef struct ConfigAFInput_t
{
    AF_HW_INIT_INPUT_T  initHWConfigInput;  // defined in af_param.h
} ConfigAFInput_T;
typedef struct ConfigAFOutput_t
{
    AF_HW_INIT_OUTPUT_T initHWConfigOutput; // AFConfig + crpR1Sel
    AF_CONFIG_INFO_T    hwConfigInfo;       // configInfo
    AFRESULT_ISPREG_T*  resultConfig;       // registermap
} ConfigAFOutput_T;

// structure for afmgr::process()
typedef struct AlgoCommand_t
{
    MUINT32         requestNum;
    LIB3A_AF_MODE_T afLibMode;
    MINT32          mfPos;
    eAFControl      eLockAlgo; // idle/lock/unlock
    MBOOL           bTrigger;
    MBOOL           bCancel;
    MBOOL           bTargetAssistMove;
    MINT32          isSkipAf;
} AlgoCommand_T;

#define COMMAND_CLEAR(command) \
    command.requestNum=-1;\
    command.afLibMode=LIB3A_AF_MODE_NUM;\
    command.mfPos=-1;\
    command.eLockAlgo=AfCommand_Idle;\
    command.bTrigger=MFALSE;\
    command.bCancel=MFALSE;\
    command.bTargetAssistMove=MFALSE;\
    command.isSkipAf=-1;

typedef struct CCUStartAFInput_t
{
    AlgoCommand_T   initAlgoCommand;
    AF_INIT_INPUT_T initAlgoInput;
} CCUStartAFInput_T;

// structure for afmgr::start()
typedef struct StartAFInput_t
{
    AlgoCommand_T   initAlgoCommand;
    AF_INIT_INPUT_T initAlgoInput;
    MVOID*          ptrMCUDrv;      //MCUDrv*
} StartAFInput_T;

typedef struct StartAFOutput_t
{
    AF_INIT_OUTPUT_T initAlgoOutput;
} StartAFOutput_T;

typedef struct DoAFInput_t
{
    AlgoCommand_T algoCommand;  // algo command
    AF_INPUT_T    afInput;      // algo input
} DoAFInput_T;

typedef struct DoAFOutput_t
{
    AF_OUTPUT_T      afOutput;    // algo output
    ConfigAFOutput_T ispOutput;   // isp output
    LENS_INFO_T      curLensInfo; // vcm output
    MUINT64          mvLensTS;    // vcm output
} DoAFOutput_T;

typedef struct GetPdBlockInput_t
{
    MINT32            inWinBufSz;
    MINT32            inAfRoiNum; // number of inAfRoi
    AFPD_BLOCK_ROI_T  inAfRoi[3];
    FD_INFO_T         inFdInfo;
} GetPdBlockInput_T;

typedef struct GetPdBlockOutput_t
{
    MINT32            outWinNum; // number of outWin
    AFPD_BLOCK_ROI_T  outWins[36];
} GetPdBlockOutput_T;
#endif

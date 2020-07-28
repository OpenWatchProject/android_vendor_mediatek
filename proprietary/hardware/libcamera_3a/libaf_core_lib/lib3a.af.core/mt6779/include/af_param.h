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
#ifndef _AF_PARAM_H

#define _AF_PARAM_H

#define AF_PARAM_REVISION 8502001

#include "af_common.h"
#include "camera_custom_af_nvram.h"
#include "dbg_af_param.h"


//------------------ DEFINE START ------------------
#define AF_NONE_PROPERTY (7533967)

#define AF_WIN_NUM_SPOT  1

#define MIN_AF_HW_WIN_X 1

#define MIN_AF_HW_WIN_Y 1

#define MAX_AF_HW_WIN_X 128

#define MAX_AF_HW_WIN_Y 128

#define MAX_AF_HW_WIN MAX_AF_HW_WIN_X*MAX_AF_HW_WIN_Y

#define HISTOGRAM_BIN_NUM 256

#define AF_PROT_BLOCK_X_SIZE 16

#define AF_PROT_BLOCK_Y_SIZE 16

#define FD_WIN_NUM     15

#define PATH_LENGTH    30

#define AF_PSUBWIN_NUM 36

//#define AFEXTENDCOEF

#define ISO_MAX_NUM     (8)

#define DistTBLLength 15

#define DafMultiSourceLength 3
//------------------ DEFINE END ------------------



//------------------ ENUM START ------------------
typedef enum
{
    AF_MARK_NONE = 0,
    AF_MARK_NORMAL,
    AF_MARK_OK,
    AF_MARK_FAIL,

} AF_MARK_T;

// AF mode definition (depart form af_feature.h)
typedef enum
{
    LIB3A_AF_MODE_OFF = 0,      // Disable AF
    LIB3A_AF_MODE_AFS,          // AF-Single Shot Mode
    LIB3A_AF_MODE_AFC,          // AF-Continuous Mode
    LIB3A_AF_MODE_AFC_VIDEO,    // AF-Continuous Mode (Video)
    LIB3A_AF_MODE_MACRO,        // AF Macro Mode
    LIB3A_AF_MODE_INFINITY,     // Infinity Focus Mode
    LIB3A_AF_MODE_MF,           // Manual Focus Mode
    LIB3A_AF_MODE_CALIBRATION,  // AF Calibration Mode
    LIB3A_AF_MODE_FULLSCAN,     // AF Full Scan Mode
    LIB3A_AF_MODE_AFC_MONITOR,  // Special AFC monitor (check usage)
    LIB3A_AF_MODE_TEMP_CALI,    // AF Temperature Calibration

    LIB3A_AF_MODE_NUM,          // AF mode number
    LIB3A_AF_MODE_MIN = LIB3A_AF_MODE_OFF,
    LIB3A_AF_MODE_MAX = LIB3A_AF_MODE_TEMP_CALI

} LIB3A_AF_MODE_T;

typedef enum
{
    // AF sync v3.0
    SYNC_STATUS_MONITORnDONE = 0,
    SYNC_STATUS_INITLENS     = 1,
    SYNC_STATUS_FINESEARCH     = 2,
    SYNC_STATUS_CONTRASTAF     = 3,
    SYNC_STATUS_LENSFOLLOW     = 4
} SYNC_STATUS_T;

typedef enum
{
    AF_ROI_SEL_NONE = 0, /* for preview start only*/
    AF_ROI_SEL_AP,
    AF_ROI_SEL_OT,
    AF_ROI_SEL_FD,
    AF_ROI_SEL_CENTER,
    AF_ROI_SEL_DEFAULT,

} AF_ROI_SEL_T;

typedef enum
{
    AF_SEARCH_DONE = 0,
    AF_SEARCH_TARGET_MOVE,
    AF_SEARCH_TARGET_FS,
    AF_SEARCH_CONTRAST,
    AF_SEARCH_READY,
} AF_SEARCH_TYPE_T;

typedef enum
{
    AFO_MODE_H = 0,
    AFO_MODE_M,
    AFO_MODE_L,
    AFO_MODE_DEFAULT,

} AFO_STAT_MODE_T;
//------------------ ENUM END ------------------



//------------------ STRUCT START ------------------

//will_seperate_to_new_header ?
/////////////////////////////////////////////////////////////
/* AF ROI description*/
typedef struct AREA_t
{
    MINT32 i4X;
    MINT32 i4Y;
    MINT32 i4W;
    MINT32 i4H;
    MINT32 i4Info; /* output area status */

} AREA_T;

/* Coordinate.*/
typedef struct AF_COORDINATE_t
{
    MINT32  i4W;
    MINT32  i4H;

} AF_COORDINATE_T;

typedef struct
{
    // ===============
    //      ISP HW setting
    // ===============
    MINT32 AF_BLF_EN;       //AF_R1_AF_CON
    MINT32 AF_BLF_D_LVL;
    MINT32 AF_BLF_R_LVL;
    MINT32 AF_BLF_VFIR_MUX;
    MINT32 AF_H_GONLY;
    MINT32 AF_V_GONLY;
    MINT32 AF_V_AVG_LVL;
    MINT32 AF_VFLT_MODE;

    MINT32 AF_DS_EN;        //AF_R1_AF_CON2
    MINT32 AF_H_FV0_ABS;
    MINT32 AF_H_FV1_ABS;
    MINT32 AF_H_FV2_ABS;
    MINT32 AF_V_FV0_ABS;
    MINT32 AF_PL_H_FV_ABS;
    MINT32 AF_PL_V_FV_ABS;
    MINT32 AF_H_FV0_EN;
    MINT32 AF_H_FV1_EN;
    MINT32 AF_H_FV2_EN;
    MINT32 AF_V_FV0_EN;
    MINT32 AF_PL_FV_EN;
    MINT32 AF_8BIT_LOWPOWER_EN;
    MINT32 AF_PL_BITSEL;

    MINT32 AF_IMAGE_WD;     //AF_R1_AF_SIZE

    MINT32 AF_VLD_XSTART;   //AF_R1_AF_VLD
    MINT32 AF_VLD_YSTART;

    MINT32 AF_PROT_BLK_XSIZE;   //AF_R1_AF_BLK_PROT
    MINT32 AF_PROT_BLK_YSIZE;

    MINT32 AF_BLK_XSIZE;    //AF_R1_AF_BLK_0
    MINT32 AF_BLK_YSIZE;

    MINT32 AF_BLK_XNUM;     //AF_R1_AF_BLK_1
    MINT32 AF_BLK_YNUM;

    MINT32 AF_HFLT0_P1;     //AF_R1_AF_HFLT0_1
    MINT32 AF_HFLT0_P2;
    MINT32 AF_HFLT0_P3;
    MINT32 AF_HFLT0_P4;
    MINT32 AF_HFLT0_P5;     //AF_R1_AF_HFLT0_2
    MINT32 AF_HFLT0_P6;
    MINT32 AF_HFLT0_P7;
    MINT32 AF_HFLT0_P8;
    MINT32 AF_HFLT0_P9;     //AF_R1_AF_HFLT0_3
    MINT32 AF_HFLT0_P10;
    MINT32 AF_HFLT0_P11;
    MINT32 AF_HFLT0_P12;

    MINT32 AF_HFLT1_P1;     //AF_R1_AF_HFLT1_1
    MINT32 AF_HFLT1_P2;
    MINT32 AF_HFLT1_P3;
    MINT32 AF_HFLT1_P4;
    MINT32 AF_HFLT1_P5;     //AF_R1_AF_HFLT1_2
    MINT32 AF_HFLT1_P6;
    MINT32 AF_HFLT1_P7;
    MINT32 AF_HFLT1_P8;
    MINT32 AF_HFLT1_P9;     //AF_R1_AF_HFLT1_3
    MINT32 AF_HFLT1_P10;
    MINT32 AF_HFLT1_P11;
    MINT32 AF_HFLT1_P12;

    MINT32 AF_HFLT2_P1;     //AF_R1_AF_HFLT2_1
    MINT32 AF_HFLT2_P2;
    MINT32 AF_HFLT2_P3;
    MINT32 AF_HFLT2_P4;
    MINT32 AF_HFLT2_P5;     //AF_R1_AF_HFLT2_2
    MINT32 AF_HFLT2_P6;
    MINT32 AF_HFLT2_P7;
    MINT32 AF_HFLT2_P8;
    MINT32 AF_HFLT2_P9;     //AF_R1_AF_HFLT2_3
    MINT32 AF_HFLT2_P10;
    MINT32 AF_HFLT2_P11;
    MINT32 AF_HFLT2_P12;

    MINT32 AF_VFLT_X1;      //AF_R1_AF_VFLT_1
    MINT32 AF_VFLT_X2;
    MINT32 AF_VFLT_X3;
    MINT32 AF_VFLT_X4;
    MINT32 AF_VFLT_X5;      //AF_R1_AF_VFLT_2
    MINT32 AF_VFLT_X6;
    MINT32 AF_VFLT_X7;
    MINT32 AF_VFLT_X8;
    MINT32 AF_VFLT_X9;      //AF_R1_AF_VFLT_3
    MINT32 AF_VFLT_X10;
    MINT32 AF_VFLT_X11;
    MINT32 AF_VFLT_X12;

    MINT32 AF_PL_HFLT_P1;   //AF_R1_AF_PL_HFLT_1
    MINT32 AF_PL_HFLT_P2;
    MINT32 AF_PL_HFLT_P3;
    MINT32 AF_PL_HFLT_P4;
    MINT32 AF_PL_HFLT_P5;   //AF_R1_AF_PL_HFLT_2
    MINT32 AF_PL_HFLT_P6;
    MINT32 AF_PL_HFLT_P7;
    MINT32 AF_PL_HFLT_P8;
    MINT32 AF_PL_HFLT_P9;   //AF_R1_AF_PL_HFLT_3
    MINT32 AF_PL_HFLT_P10;
    MINT32 AF_PL_HFLT_P11;
    MINT32 AF_PL_HFLT_P12;

    MINT32 AF_PL_VFLT_X1;   //AF_R1_AF_PL_VFLT_1
    MINT32 AF_PL_VFLT_X2;
    MINT32 AF_PL_VFLT_X3;
    MINT32 AF_PL_VFLT_X4;
    MINT32 AF_PL_VFLT_X5;   //AF_R1_AF_PL_VFLT_2
    MINT32 AF_PL_VFLT_X6;
    MINT32 AF_PL_VFLT_X7;
    MINT32 AF_PL_VFLT_X8;
    MINT32 AF_PL_VFLT_X9;   //AF_R1_AF_PL_VFLT_3
    MINT32 AF_PL_VFLT_X10;
    MINT32 AF_PL_VFLT_X11;
    MINT32 AF_PL_VFLT_X12;

    MINT32 AF_H_TH_0;       //AF_R1_AF_TH_0
    MINT32 AF_H_TH_1;
    MINT32 AF_H_TH_2;       //AF_R1_AF_TH_1
    MINT32 AF_V_TH;
    MINT32 AF_PL_H_TH;      //AF_R1_AF_TH_2

    MINT32 AF_G_SAT_TH;     //AF_R1_AF_TH_3
    MINT32 AF_B_SAT_TH;
    MINT32 AF_R_SAT_TH;     //AF_R1_AF_TH_4

    MINT32 AF_H_TH_0_LUT_MODE;  //AF_R1_AF_LUT_H0_0
    MINT32 AF_H_TH_0_GAIN;
    MINT32 AF_H_TH_0_D1;    //AF_R1_AF_LUT_H0_1
    MINT32 AF_H_TH_0_D2;
    MINT32 AF_H_TH_0_D3;
    MINT32 AF_H_TH_0_D4;
    MINT32 AF_H_TH_0_D5;    //AF_R1_AF_LUT_H0_2
    MINT32 AF_H_TH_0_D6;
    MINT32 AF_H_TH_0_D7;
    MINT32 AF_H_TH_0_D8;
    MINT32 AF_H_TH_0_D9;    //AF_R1_AF_LUT_H0_3
    MINT32 AF_H_TH_0_D10;
    MINT32 AF_H_TH_0_D11;
    MINT32 AF_H_TH_0_D12;
    MINT32 AF_H_TH_0_D13;   //AF_R1_AF_LUT_H0_4
    MINT32 AF_H_TH_0_D14;
    MINT32 AF_H_TH_0_D15;
    MINT32 AF_H_TH_0_D16;

    MINT32 AF_H_TH_1_LUT_MODE;  //AF_R1_AF_LUT_H1_0
    MINT32 AF_H_TH_1_GAIN;
    MINT32 AF_H_TH_1_D1;    //AF_R1_AF_LUT_H1_1
    MINT32 AF_H_TH_1_D2;
    MINT32 AF_H_TH_1_D3;
    MINT32 AF_H_TH_1_D4;
    MINT32 AF_H_TH_1_D5;    //AF_R1_AF_LUT_H1_2
    MINT32 AF_H_TH_1_D6;
    MINT32 AF_H_TH_1_D7;
    MINT32 AF_H_TH_1_D8;
    MINT32 AF_H_TH_1_D9;    //AF_R1_AF_LUT_H1_3
    MINT32 AF_H_TH_1_D10;
    MINT32 AF_H_TH_1_D11;
    MINT32 AF_H_TH_1_D12;
    MINT32 AF_H_TH_1_D13;   //AF_R1_AF_LUT_H1_4
    MINT32 AF_H_TH_1_D14;
    MINT32 AF_H_TH_1_D15;
    MINT32 AF_H_TH_1_D16;

    MINT32 AF_H_TH_2_LUT_MODE;  //AF_R1_AF_LUT_H2_0
    MINT32 AF_H_TH_2_GAIN;
    MINT32 AF_H_TH_2_D1;    //AF_R1_AF_LUT_H2_1
    MINT32 AF_H_TH_2_D2;
    MINT32 AF_H_TH_2_D3;
    MINT32 AF_H_TH_2_D4;
    MINT32 AF_H_TH_2_D5;    //AF_R1_AF_LUT_H2_2
    MINT32 AF_H_TH_2_D6;
    MINT32 AF_H_TH_2_D7;
    MINT32 AF_H_TH_2_D8;
    MINT32 AF_H_TH_2_D9;    //AF_R1_AF_LUT_H2_3
    MINT32 AF_H_TH_2_D10;
    MINT32 AF_H_TH_2_D11;
    MINT32 AF_H_TH_2_D12;
    MINT32 AF_H_TH_2_D13;   //AF_R1_AF_LUT_H2_4
    MINT32 AF_H_TH_2_D14;
    MINT32 AF_H_TH_2_D15;
    MINT32 AF_H_TH_2_D16;

    MINT32 AF_V_TH_LUT_MODE;    //AF_R1_AF_LUT_V_0
    MINT32 AF_V_TH_GAIN;
    MINT32 AF_V_TH_D1;      //AF_R1_AF_LUT_V_1
    MINT32 AF_V_TH_D2;
    MINT32 AF_V_TH_D3;
    MINT32 AF_V_TH_D4;
    MINT32 AF_V_TH_D5;      //AF_R1_AF_LUT_V_2
    MINT32 AF_V_TH_D6;
    MINT32 AF_V_TH_D7;
    MINT32 AF_V_TH_D8;
    MINT32 AF_V_TH_D9;      //AF_R1_AF_LUT_V_3
    MINT32 AF_V_TH_D10;
    MINT32 AF_V_TH_D11;
    MINT32 AF_V_TH_D12;
    MINT32 AF_V_TH_D13;     //AF_R1_AF_LUT_V_4
    MINT32 AF_V_TH_D14;
    MINT32 AF_V_TH_D15;
    MINT32 AF_V_TH_D16;

    MINT32 AF_SGG1_GAIN;    //AF_R1_AF_SGG1_0
    MINT32 AF_SGG1_GMR_1;
    MINT32 AF_SGG1_GMR_2;   //AF_R1_AF_SGG1_1
    MINT32 AF_SGG1_GMR_3;
    MINT32 AF_SGG1_GMR_4;   //AF_R1_AF_SGG1_2
    MINT32 AF_SGG1_GMR_5;
    MINT32 AF_SGG1_GMR_6;   //AF_R1_AF_SGG1_3
    MINT32 AF_SGG1_GMR_7;
    MINT32 AF_SGG1_GMR_8;   //AF_R1_AF_SGG1_4
    MINT32 AF_SGG1_GMR_9;
    MINT32 AF_SGG1_GMR_10;  //AF_R1_AF_SGG1_5
    MINT32 AF_SGG1_GMR_11;

    MINT32 AF_SGG5_GAIN;    //AF_R1_AF_SGG5_0
    MINT32 AF_SGG5_GMR_1;
    MINT32 AF_SGG5_GMR_2;   //AF_R1_AF_SGG5_1
    MINT32 AF_SGG5_GMR_3;
    MINT32 AF_SGG5_GMR_4;   //AF_R1_AF_SGG5_2
    MINT32 AF_SGG5_GMR_5;
    MINT32 AF_SGG5_GMR_6;   //AF_R1_AF_SGG5_3
    MINT32 AF_SGG5_GMR_7;
    MINT32 AF_SGG5_GMR_8;   //AF_R1_AF_SGG5_4
    MINT32 AF_SGG5_GMR_9;
    MINT32 AF_SGG5_GMR_10;  //AF_R1_AF_SGG5_5
    MINT32 AF_SGG5_GMR_11;

    //MINT32 AF_PL_STAT_0;    //AF_R1_AF_PL_STAT_0 ; read only, no config
    //MINT32 AF_PL_STAT_1;    //AF_R1_AF_PL_STAT_1 ; read only, no config
} AF_CONFIG_REG_T;

/* AF HW configure*/
typedef struct
{
    // ===============
    //    AF HAL control flow
    // ===============
    /*TG coordinate*/
    AF_COORDINATE_T sTG_SZ;
    /*HW coordinate*/
    AF_COORDINATE_T sBIN_SZ;

    /*AF ROI : TG coordinate*/
    AREA_T sRoi;

    /*Stats info*/
    MUINT32 u4ConfigNum;
    MUINT32 u4AfoStatMode;      // afo foot-print type ; (bit-depth) x (high/low)
    MUINT32 u4AfoBlkSzByte;     // afo foot-print block size

    MUINT32 u4IsConfigChanged;  // if any setting change, set this flag true


    // ===============
    //     ALGO setting
    // ===============
    // Control 1
    MINT32 AF_BLF[4];
    MINT32 AF_H_GONLY;
    MINT32 AF_V_GONLY;
    MINT32 AF_V_AVG_LVL;
    MINT32 AF_V_FLT_MODE;

    // Control 2
    MINT32 AF_DS_En;
    MINT32 AF_FV_H_ABS[3];
    MINT32 AF_FV_V_ABS;
    MINT32 AF_FV_PL_H_ABS;
    MINT32 AF_FV_PL_V_ABS;
    MINT32 AF_FV_H_En[3];
    MINT32 AF_FV_V_En;
    MINT32 AF_FV_PL_En;
    MINT32 AF_8BIT_LOWPOWER_En;
    MINT32 AF_PL_BITSEL;

    // ROI
    MINT32 AF_BLK_XNUM;
    MINT32 AF_BLK_YNUM;
    MINT32 AF_BLK_XSIZE;
    MINT32 AF_BLK_YSIZE;

    // HW filter (H0~2, V, PL_H, PL_V)
    MINT32 AF_FIL_H0[12];
    MINT32 AF_FIL_H1[12];
    MINT32 AF_FIL_H2[12];
    MINT32 AF_FIL_V[12];
    MINT32 AF_FIL_PL_H[12];
    MINT32 AF_FIL_PL_V[12];

    // HW thr (H0~2, V, PL, G, B, R)
    MINT32 AF_TH_H[3];
    MINT32 AF_TH_V;
    MINT32 AF_TH_H_PL;
    MINT32 AF_TH_G_SAT;
    MINT32 AF_TH_B_SAT;
    MINT32 AF_TH_R_SAT;

    // LUT (H0~H2, V)
    MINT32 AF_LUT_H_MODE[3];
    MINT32 AF_LUT_H_GAIN[3];
    MINT32 AF_LUT_H_TH0_Dn[16];
    MINT32 AF_LUT_H_TH1_Dn[16];
    MINT32 AF_LUT_H_TH2_Dn[16];
    MINT32 AF_LUT_V_MODE;
    MINT32 AF_LUT_V_GAIN;
    MINT32 AF_LUT_V_TH_Dn[16];

    // SGG (1&5)
    MINT32 AF_SGG1_GAIN;
    MINT32 AF_SGG1_GMR[11];
    MINT32 AF_SGG5_GAIN;
    MINT32 AF_SGG5_GMR[11];


    // ===============
    //     AF REG setting
    // ===============
    AF_CONFIG_REG_T sConfigReg;

} AF_CONFIG_T;
/////////////////////////////////////////////////////////////


/* AF statistic data */
typedef struct
{   // DO NOT change member sequence
    //AFO_MODE_M: 48 bytes
    MUINT32 u4FILV;
    MUINT32 u4FILH0;
    MUINT32 u4FILH1;
    MUINT32 u4FILH2;
    MUINT32 u4GSum;
    MUINT32 u4RSum;
    MUINT32 u4BSum;
    MUINT16 u2RSCnt;
    MUINT16 u2BSCnt;
    MUINT16 u2GSCnt;
    MUINT16 u2FILVCnt;
    MUINT16 u2FILH0Cnt;
    MUINT16 u2FILH1Cnt;
    MUINT16 u2FILH2Cnt;
    MUINT16 u2Dummy;
    MUINT32 u4Dummy;
} AF_STAT_T;

typedef struct
{   // DO NOT change member sequence
    //AFO_MODE_L: 40 bytes
    MUINT32 u4FILV;
    MUINT32 u4FILH0;
    MUINT32 u4FILH1;
    MUINT32 u4FILH2;
    MUINT32 u4GSum;
    MUINT32 u4RSum;
    MUINT32 u4BSum;
    //MUINT16 u2RSCnt;
    //MUINT16 u2BSCnt;
    MUINT16 u2GSCnt;
    MUINT16 u2FILVCnt;
    MUINT16 u2FILH0Cnt;
    MUINT16 u2FILH1Cnt;
    MUINT16 u2FILH2Cnt;
    MUINT16 u2Dummy;
    //MUINT32 u4Dummy;
} AF_STAT_L_T;

typedef struct
{   // DO NOT change member sequence
    //AFO_MODE_H: 48 bytes
    MUINT32 u4FILV;
    MUINT32 u4FILH0;
    MUINT32 u4FILH1;
    MUINT32 u4FILH2;
    MUINT32 u4GSum;
    MUINT32 u4RSum;
    MUINT32 u4BSum;
    MUINT16 u2RSCnt;
    MUINT16 u2BSCnt;
    MUINT16 u2GSCnt;
    MUINT16 u2FILMSB;   // v[1]H0[2]H1[2]H2[2]
    MUINT16 u2FILVCnt;
    MUINT16 u2FILH0Cnt;
    MUINT16 u2FILH1Cnt;
    MUINT16 u2FILH2Cnt;
    //MUINT16 u2Dummy;
    MUINT32 u4Dummy;
} AF_STAT_H_T;


/* AF stats. profile info */
typedef struct
{
    MUINT32 u4NumBlkX;
    MUINT32 u4NumBlkY;
    MUINT32 u4NumStride;
    MUINT32 u4SizeBlkX;
    MUINT32 u4SizeBlkY;

    MUINT32 u4AfoStatMode;  // New, afo foot-print type ; (bit-depth) x (high/low)
    MUINT32 u4AfoBlkSzByte; // New, afo foot-print block size

    MUINT32 u4ConfigNum;
    MINT32  i4AFPos;
} AF_STAT_PROFILE_T;


/* Intput Area */
typedef struct
{
    MINT32  i4Count;
    AREA_T  sRect[AF_WIN_NUM_SPOT];

} AF_AREA_T;

/* New, for FD data(ROI + FDinfo) */
typedef struct
{
    MINT32  i4IsFdValid;    // fd updated by new frame or not
    MINT32  i4Count;
    MINT32  i4Score;
    AREA_T  sRect[AF_WIN_NUM_SPOT];
    MINT32  i4Id[AF_WIN_NUM_SPOT];
    MINT32  i4Type[AF_WIN_NUM_SPOT];            // 0:GFD, 1:LFD, 2:OT
    MINT32  i4Motion[AF_WIN_NUM_SPOT][2];
    MINT32  i4Landmark[AF_WIN_NUM_SPOT][3][4];  // index 0: left eye, index 1: right eye, index 2:mouth
    MINT32  i4LandmarkCV[AF_WIN_NUM_SPOT];
    MINT32  i4LandmarkROP[AF_WIN_NUM_SPOT];
    MINT32  i4LandmarkRIP[AF_WIN_NUM_SPOT];
    MINT32  i4PortEnable;
    MINT32  i4IsTrueFace;
    MFLOAT  f4CnnResult0;
    MFLOAT  f4CnnResult1;
} FD_INFO_T;

/* New, for static get property */
typedef struct
{
    // debug on/off for all properties
    MINT32 enable;

    // log on/off
    MINT32 log_enable;

    // tuning para
    MINT32 v3nvram;
    MINT32 hbnvram;
    MINT32 hbnvram2;
    MINT32 fdnvram;

    MINT32 tuning_enable;
    MINT32 tuning_TH0;
    MINT32 tuning_TH1;
    MINT32 tuning_TH2;
    MINT32 tuning_minth;
    MINT32 tuning_gsthr;
    MINT32 tuning_fvthr;

    MINT32 set_infinite;
    MINT32 lensmovetime;

    // test mode
    MINT32 fullscan_step;
    MINT32 localfullscan;
    MINT32 thermalcali_ctrl;

    // zoom effect
    MINT32 zoomext;
    MINT32 zefp;
    MINT32 zefsel;
    MINT32 zeccalib;

    // hybrid af
    MINT32 hybrid_mode;
    MINT32 pdafSingleWin;
    MINT32 pdsubidx;
    MINT32 exifpdsubwin;

    // plaf
    MINT32 pldetect;
    MINT32 pltuning_Rtop;
    MINT32 pltuning_Sa;
    MINT32 pltuning_Sr;
    MINT32 pltuning_Ba;
    MINT32 pltuning_Br;
    MINT32 pltuning_Amin1;
    MINT32 pltuning_Rmin1;
    MINT32 pltuning_Amax;
    MINT32 pltuning_Rmax;
    MINT32 pltuning_CtrlBit;
    MINT32 pl_SCNT;
    MINT32 pl_convThrInf;
    MINT32 pl_convThrMac;

    // thermal
    MINT32 TempCaliTimeOut;
    MINT32 TempDbgLog;
    MINT32 thermalclean;
    MINT32 LTCOFF;
    MINT32 LTCMode;
    MINT32 LTCBias;

    // fsc
    MINT32 fscFrmDelay;

    // dump data
    MINT32 dumpfdaf_enable;
    MINT32 PLWFile_enable;
    MINT32 prvdump_step;
    MINT32 prvdump_start;
    MINT32 prvdump_num;
    MINT32 prvdump_frameidle;
    MINT32 dumpdata;

} AF_DBG_S_PROPERTY_T;

/* New, for dynamic get property */
typedef struct
{
    // test mode
    MINT32 trigger;
    MINT32 frame;
    MINT32 scan;
    MINT32 inf;

    // af motor control
    MINT32 motor_disable;
    MINT32 motor_position;
    MINT32 motor_control;
    MINT32 motor_dac;
    MINT32 motor_slavedac;

    // general debug reserved
    MINT32 db_rsv1;
    MINT32 db_rsv2;
    MINT32 db_rsv3;
    MINT32 db_rsv4;
    MINT32 db_rsv5;

} AF_DBG_D_PROPERTY_T;

typedef struct LENS_INFO_t
{
    MINT32 i4CurrentPos;        //current position
    MINT32 i4MacroPos;          //macro position
    MINT32 i4InfPos;            //Infiniti position
    MINT32 bIsMotorMoving;      //Motor Status
    MINT32 bIsMotorOpen;        //Motor Open?
    MINT32 bIsSupportSR;
} LENS_INFO_T;

typedef struct
{
    MINT32 i4Confidence;
    MINT32 i4CurPosDAC;
    MINT32 i4CurPosDist;
    MINT32 i4AfWinPosX;
    MINT32 i4AfWinPosY;
    MINT32 i4AfWinPosCnt;
} LASER_INFO_T;

typedef struct AFPD_BLOCK_ROI_t
{
    eAF_ROI_TYPE_T    sType;
    AREA_T            sPDROI;
} AFPD_BLOCK_ROI_T;

typedef struct
{
    AFPD_BLOCK_ROI_T sBlockROI;
    MUINT16          i4PDafDacIndex;
    MUINT16          i4PDafConfidence;
    MINT32           i4PDafConverge;
    MINT32           i4SatPercent;
    MINT32           i4ConfidenceExt;

} AFPD_BLOCK_T;

typedef struct
{
    MINT32        i4Valid;
    MUINT32       i4FrmNum;
    MINT32        i4PDBlockInfoNum;
    MINT32        i4PDSourceID;
    AFPD_BLOCK_T  sPDBlockInfo[AF_PSUBWIN_NUM];

} AFPD_INFO_T;

typedef struct
{
    MINT64  i8MFV;
    MINT64  i8SFV;

    MINT32  i4SyncMode;
    MINT32  i4Minf;
    MINT32  i4Mmacro;
    MINT32  i4Sinf;
    MINT32  i4Smacro;
    MINT32  i4MTemp;
    MINT32  i4STemp;
    MINT32  i4MPos;

    MINT32  i4SPos;

    MINT32  i4IsSAFRun;
    MINT32  i4IsSMAFTrig;
    MINT32  i4IsSCAFTrig;

    AF_AREA_T s_MAFArea;
    FD_INFO_T s_MFDInput;
    MINT32  i4SAFRunPos;
    MINT32  i4SyncStatus;
    MINT32  i4MAFCStatus;
    MINT32  i4SAFCStatus;
    MINT32  i4MovDir;
    MINT32 i4AEStable;
    MINT32 i4FVStable;

} AF_SyncInfo_T;


/* OTP info */
typedef struct
{
    unsigned short Close_Loop_AF_Min_Position;
    unsigned short Close_Loop_AF_Max_Position;
    unsigned char  Close_Loop_AF_Hall_AMP_Offset;
    unsigned char  Close_Loop_AF_Hall_AMP_Gain;
    unsigned short AF_infinite_pattern_distance;
    unsigned short AF_Macro_pattern_distance;
    unsigned char  AF_infinite_calibration_temperature;
    unsigned char  AF_macro_calibration_temperature;

    unsigned short Posture_AF_infinite_calibration;
    unsigned short Posture_AF_macro_calibration;
    unsigned char  Posture_AF_infinite_calibration_temperature;
    unsigned char  Posture_AF_macro_calibration_temperature;
    unsigned short AF_Middle_calibration;
    unsigned char  AF_Middle_calibration_temperature;
    unsigned char  ModuleConfig;
    unsigned char  CalibDataFlag;

    unsigned char  Reserved[40];
} AF_OTPInfo_T;

/* AF info for ISP tuning */
typedef struct
{
    MINT32 i4AFPos; // AF position

} AF_INFO_T;


/////////////////////////////////////////////////////////////
/* afc_initHwConfig input */
typedef struct
{
    MINT32          enableLPB;   // low power bit ; by query
    MINT32          enableH3;    // default set 1
    MINT32          enableRBSAT; // default set 1
    AF_CONFIG_T     sDefaultHWCfg;

}AF_HW_INIT_INPUT_T;

/* afc_initHwConfig output */
typedef struct
{
    AF_CONFIG_T     sInitHWCfg;
    MINT32          crpR1Sel;   // setting to LTM

}AF_HW_INIT_OUTPUT_T;

/* afc_initAF input */
typedef struct
{
    MINT32          otpInfPos;          // to update af table
    MINT32          otpMacroPos;        // to update af table
    MINT32          otpMiddlePos;       // to update af table

    MINT32          i4Acce[3];
    MINT32          i4AcceScale;
    MINT32          i4IsEnterCam;
    MINT32          i4ISO;

    LIB3A_AF_MODE_T eAFMode;
    AF_COORDINATE_T sTGSz;
    AF_COORDINATE_T sHWSz;
    AF_AREA_T       sAFArea;
    AREA_T          sEZoom;
    LENS_INFO_T     sLensInfo;
    AF_CONFIG_T     sInitHwConfig;

    AF_NVRAM_T      sAFNvram;

    AF_DBG_S_PROPERTY_T sProperty;

    MINT32          i4RealSensorDev;

} AF_INIT_INPUT_T;

/* afc_initAF output */
typedef struct
{
    MINT32 posturedInitPos;

    MINT32 otpAfTableStr;
    MINT32 otpAfTableEnd;

} AF_INIT_OUTPUT_T;

/* afc_handleAF input */
typedef struct
{
    // Keep all 64 bits variables here !!!!!!!!
    MUINT64         TS_SOF;                 // (unit: us)    Time stamp of VSYNC. (from Hal3A)
    MUINT64         TS_MLStart;             // (unit: us)    Time stamp of start of move lens.
    MUINT64         PL_2PhaseFV;             // (unit: us)    Time stamp of start of move lens.

    // Flow information
    MUINT32         i4FrameNum;             // current AFO stats. belonging frame number ; synced with mgr flow
    MUINT32         i4IsCaptureIntent;
    MUINT32         i4HybridAFMode;
    MINT32          i4IsEnterCam;           // 1: re-enter camera ; 0: only switch camera mode
    MINT32          i4IsFlashFrm;           // PL detect
    MINT32          i4IsSupportN2Frame;     // support HW effective at N+2 frame
    MINT32          i4FullScanStep;
    MUINT32         u4CurTemperature;

    // Adaptive Compensation

    MINT32          PixelInLine;            // (unit: pixel) pixel number per line
    MINT32          PixelClk;               // (unit: Hz)    pixel readout per second

    // AF information
    AF_COORDINATE_T sTGSz;                  // TG coordinate
    AF_COORDINATE_T sHWSz;                  // HW coordinate
    AREA_T          sEZoom;
    AF_AREA_T       sAFArea;
    LENS_INFO_T     sLensInfo;

    // AE information
    MINT32          i4AEBlockY[25];         // aeBlockV[25]
    MUINT32         u4AECmv;                // u4MgrCWValue ; AE center weighting Yvalue
    MINT32          i4ISO;                  // u4RealISOValue
    MINT32          i4SceneLV;              // i4LightValue_x10
    MINT32          i4ShutterValue;         // u4Eposuretime
    MINT32          i4IsAEStable;           // bAEStable
    MINT32          i4IsAELocked;           // bAELock
    MINT32          i4IsLVDiffLarge;        // LV change large and need converge AE
    MINT32          i4DeltaBV;              // i4deltaIndex
    MINT32          i4IsAEidxReset;         // bAEScenarioChange
    MUINT32         u4AEFinerEVIdxBase;     // u4AEFinerEVIdxBase
    MUINT32         u4AEidxCurrentF;        // u4AEidxCurrentF

    // G/Gyro information
    MINT32          i4Acce[3];
    MUINT32         u4AcceScale;
    MINT32          i4Gyro[3];
    MUINT32         u4GyroScale;

    // OIS information
    MINT32          i4IsOISStable;          // check ois status keep stable

    // Global motion vector information
    MINT32          i4GmvX;
    MINT32          i4GmvY;
    MINT32          i4GmvConfX;
    MINT32          i4GmvConfY;
    MINT32          i4GmvMax;

    AFPD_INFO_T     sPDInfo;
    LASER_INFO_T    sLaserInfo;
    FD_INFO_T       sFDInfo;                //

    // AF stat. info


    AF_DBG_D_PROPERTY_T dProperty;

    AF_STAT_PROFILE_T  sStatProfile;        // to be modified (replace by AFO Bank preprocess results)

    // Keep all 64 bits variables at TOP !!!!!!!!

} AF_INPUT_T;

/* afc_handleAF output */
typedef struct
{
    MINT64       i8AFValue;

    // af status to mgr
    MINT32       i4IsAfSearch;
    MINT32       i4IsFocused;

    // af hw setting
    MINT32       i4ROISel;
    MINT32       i4IsSelHWROI;

    MINT32       i4AFBestPos;
    MINT32       i4AFPos;
    MINT32       i4FDDetect;

    // af pd info
    AREA_T       sPDWin;
    MUINT32      i4PdTrigUiConf;

    AF_AREA_T    sExtFdArea;                // New, move from api   extendFDWin

    // af hw config data
    AF_CONFIG_T  sAFStatConfig;

    // af info1
    MINT32       i4IsSceneChange;
    MINT32       i4IsSceneStable;
    MUINT32      i4SceneState;
    MINT32       i4IsLockAEReq;             // lock AE during contrast search

    // af compensation info
    MINT32       i4ThermalLensPos;          //Lens Thermal Compensate result
    MINT32       i4PostureDac;
    MINT32       i4ZECChg;                  //ZEC setting to mgr

    // af info2
    AREA_T       sLastFocusedArea;
    MINT32       i4IsTargetAssitMove;
    MINT32       i4AdpAlarm;
    MINT32       i4DbgRtfData[8];           // reserved RealTimeViewer fields for usage

    // af search range
    MINT32       i4SearchRangeInf;
    MINT32       i4SearchRangeMac;

} AF_OUTPUT_T;
/////////////////////////////////////////////////////////////



// remove_ to_ip.h
typedef struct
{
    MINT32  i4Num;
    MINT32  i4Pos[PATH_LENGTH];

} AF_STEP_T;

//remove_to_mgr
typedef struct
{
    MINT32  i4AFS_MODE;    //0 : singleAF, 1:smoothAF
    MINT32  i4AFC_MODE;    //0 : singleAF, 1:smoothAF
    MINT32  i4VAFC_MODE;   //0 : singleAF, 1:smoothAF

    MINT32 i4TBLL;
    MINT32 i4Dist[DistTBLLength];
    MINT32 i4Dacv[DistTBLLength];
    MINT32 i4FocusRange[DistTBLLength];
} AF_PARAM_T;

// remove_to_mgr for af_flow_custom
typedef struct COMPARESET_t
{
    MINT32 Value;
    MINT32 Target;
} CompareSet_T;

// remove_to_mgr for af_flow_custom
typedef struct CHECK_AF_CALLBACK_INFO_t
{
    MINT32 isAfterAutoMode;
    MINT32 isSearching;        // 0: idle, 1: pd, 2: fs, 3: contrast
    CompareSet_T CompSet_PDCL; // PD confidence level
    CompareSet_T CompSet_ISO;  // ISO
    CompareSet_T CompSet_FPS;  // FPS
} CHECK_AF_CALLBACK_INFO_T;

//------------------ STRUCT END ------------------



#endif

/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

#ifndef _ISPIF_
#define _ISPIF_

namespace NSIspTuning
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ISP module Top Control Structure (by Feature)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct{
    MINT32 GMVX;
    MINT32 GMVY;
    MINT32 confX;
    MINT32 confY;
    MINT32 MAX_GMV;
    MINT32 frameReset;
    MINT32 GMV_Status;
    MINT32 ISO_cutoff;
    MINT32 isGyroValid;
    MINT32 gyroXAccelX1000;
    MINT32 gyroYAccelX1000;
    MINT32 gyroZAccelX1000;
    MINT32 gyroTimeStampHigh;
    MINT32 gyroTimeStampLow;
} ISP_NR3D_META_INFO_T;

typedef struct{
    MINT32 GMV_level_th_def;
    MINT32 GMV_level_th;
    MINT32 GMV_sm_th;
    MINT32 GMV_sm_th_up;
    MINT32 GMV_sm_th_lo;
    MINT32 GMV_lv_th_up;
    MINT32 GMV_lv_th_lo;
    MINT32 GMV_min_str;
    MINT32 ISO_upper;
    MINT32 Conf_th;
} ISP_NVRAM_NR3D_PARAS_T;

typedef struct{
    ISP_NR3D_META_INFO_T M_Info;
    ISP_NVRAM_NR3D_PARAS_T C_Paras;
    MINT32 Device_ID;
} ISP_NR3D_SMOOTH_INFO_T;


typedef MUINT32 FIELD;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ISP Enable (Pass1@TG1)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CAM_CTL_EN CAM+0004H: ISP Enable (Pass1)
typedef struct {
        FIELD  TG_EN                                 :  1;      /*  0.. 0, 0x00000001 */
        FIELD  DMX_EN                                :  1;      /*  1.. 1, 0x00000002 */
        FIELD  SGM_EN                                :  1;      /*  2.. 2, 0x00000004 */
        FIELD  RMG_EN                                :  1;      /*  3.. 3, 0x00000008 */
        FIELD  RMM_EN                                :  1;      /*  4.. 4, 0x00000010 */
        FIELD  OBC_EN                                :  1;      /*  5.. 5, 0x00000020 */
        FIELD  BNR_EN                                :  1;      /*  6.. 6, 0x00000040 */
        FIELD  LSC_EN                                :  1;      /*  7.. 7, 0x00000080 */
        FIELD  CAC_EN                                :  1;      /*  8.. 8, 0x00000100 */
        FIELD  RPG_EN                                :  1;      /*  9.. 9, 0x00000200 */
        FIELD  RRZ_EN                                :  1;      /* 10..10, 0x00000400 */
        FIELD  RMX_EN                                :  1;      /* 11..11, 0x00000800 */
        FIELD  PAKG_EN                               :  1;      /* 12..12, 0x00001000 */
        FIELD  BMX_EN                                :  1;      /* 13..13, 0x00002000 */
        FIELD  CPG_EN                                :  1;      /* 14..14, 0x00004000 */
        FIELD  PAK_EN                                :  1;      /* 15..15, 0x00008000 */
        FIELD  UFE_EN                                :  1;      /* 16..16, 0x00010000 */
        FIELD  AF_EN                                 :  1;      /* 17..17, 0x00020000 */
        FIELD  SGG1_EN                               :  1;      /* 18..18, 0x00040000 */
        FIELD  AA_EN                                 :  1;      /* 19..19, 0x00080000 */
        FIELD  QBIN1_EN                              :  1;      /* 20..20, 0x00100000 */
        FIELD  LCS_EN                                :  1;      /* 21..21, 0x00200000 */
        FIELD  QBIN2_EN                              :  1;      /* 22..22, 0x00400000 */
        FIELD  RCP_EN                                :  1;      /* 23..23, 0x00800000 */
        FIELD  RCP3_EN                               :  1;      /* 24..24, 0x01000000 */
        FIELD  rsv_25                                :  2;      /* 25..26, 0x06000000 */
        FIELD  BIN_EN                                :  1;      /* 27..27, 0x08000000 */
        FIELD  DBS_EN                                :  1;      /* 28..28, 0x10000000 */
        FIELD  DBN_EN                                :  1;      /* 29..29, 0x20000000 */
        FIELD  PBN_EN                                :  1;      /* 30..30, 0x40000000 */
        FIELD  UFEG_EN                               :  1;      /* 31..31, 0x80000000 */
} ISP_CAM_CTL_EN_T;

typedef union {
    typedef ISP_CAM_CTL_EN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CTL_EN_T;

typedef struct
{
        FIELD  IMGO_EN                               :  1;      /*  0.. 0, 0x00000001 */
        FIELD  UFEO_EN                               :  1;      /*  1.. 1, 0x00000002 */
        FIELD  RRZO_EN                               :  1;      /*  2.. 2, 0x00000004 */
        FIELD  AFO_EN                                :  1;      /*  3.. 3, 0x00000008 */
        FIELD  LCSO_EN                               :  1;      /*  4.. 4, 0x00000010 */
        FIELD  AAO_EN                                :  1;      /*  5.. 5, 0x00000020 */
        FIELD  PSO_EN                                :  1;      /*  6.. 6, 0x00000040 */
        FIELD  BPCI_EN                               :  1;      /*  7.. 7, 0x00000080 */
        FIELD  LSCI_EN                               :  1;      /*  8.. 8, 0x00000100 */
        FIELD  CACI_EN                               :  1;      /*  9.. 9, 0x00000200 */
        FIELD  PDO_EN                                :  1;      /* 10..10, 0x00000400 */
        FIELD  LSC3I_EN                              :  1;      /* 11..11, 0x00000800 */
        FIELD  PDI_EN                                :  1;      /* 12..12, 0x00001000 */
        FIELD  FLKO_EN                               :  1;      /* 13..13, 0x00002000 */
        FIELD  LMVO_EN                               :  1;      /* 14..14, 0x00004000 */
        FIELD  RSSO_EN                               :  1;      /* 15..15, 0x00008000 */
        FIELD  UFGO_EN                               :  1;      /* 16..16, 0x00010000 */
        FIELD  rsv_17                                : 15;      /* 17..31, 0xFFFE0000 */
}ISP_CAM_CTL_DMA_EN_T;

typedef union {
    typedef ISP_CAM_CTL_DMA_EN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CTL_DMA_EN_T;


typedef struct {
        FIELD  SGG3_EN                               :  1;      /*  0.. 0, 0x00000001 */
        FIELD  FLK_EN                                :  1;      /*  1.. 1, 0x00000002 */
        FIELD  HDS_EN                                :  1;      /*  2.. 2, 0x00000004 */
        FIELD  LMV_EN                                :  1;      /*  3.. 3, 0x00000008 */
        FIELD  RSS_EN                                :  1;      /*  4.. 4, 0x00000010 */
        FIELD  rsv_5                                 :  8;      /*  5..12, 0x00001FE0 */
        FIELD  CPN_EN                                :  1;      /* 13..13, 0x00002000 */
        FIELD  DCPN_EN                               :  1;      /* 14..14, 0x00004000 */
        FIELD  ADBS_EN                               :  1;      /* 15..15, 0x00008000 */
        FIELD  BMX2_EN                               :  1;      /* 16..16, 0x00010000 */
        FIELD  PSB_EN                                :  1;      /* 17..17, 0x00020000 */
        FIELD  SGG2_EN                               :  1;      /* 18..18, 0x00040000 */
        FIELD  PDE_EN                                :  1;      /* 19..19, 0x00080000 */
        FIELD  GSE_EN                                :  1;      /* 20..20, 0x00100000 */
        FIELD  PCP_EN                                :  1;      /* 21..21, 0x00200000 */
        FIELD  RMB_EN                                :  1;      /* 22..22, 0x00400000 */
        FIELD  PS_EN                                 :  1;      /* 23..23, 0x00800000 */
        FIELD  HLR_EN                                :  1;      /* 24..24, 0x01000000 */
        FIELD  AMX_EN                                :  1;      /* 25..25, 0x02000000 */
        FIELD  SL2F_EN                               :  1;      /* 26..26, 0x04000000 */
        FIELD  VBN_EN                                :  1;      /* 27..27, 0x08000000 */
        FIELD  SL2J_EN                               :  1;      /* 28..28, 0x10000000 */
        FIELD  STM_EN                                :  1;      /* 29..29, 0x20000000 */
        FIELD  SCM_EN                                :  1;      /* 30..30, 0x40000000 */
        FIELD  SGG5_EN                               :  1;      /* 31..31, 0x80000000 */
} ISP_CAM_CTL_EN2_T; // /* 0x1A004018 */

typedef union {
    typedef ISP_CAM_CTL_EN2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CTL_EN2_T;

//
typedef union {
    enum { COUNT = 2 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_CTL_EN_T  en_p1;
        ISP_NVRAM_CTL_EN2_T  en_2;
        ISP_NVRAM_CTL_DMA_EN_T en_dma_p1;
    };
} ISP_NVRAM_CTL_EN_P1_T;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ISP Enable (Pass2)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// DIP_X_CTL_EN_P2 CAM+0018H: ISP Enable (Pass2)
typedef struct {
		FIELD  UNP_EN                                :  1;		/*  0.. 0, 0x00000001 */
		FIELD  UFD_EN                                :  1;		/*  1.. 1, 0x00000002 */
		FIELD  PGN_EN                                :  1;		/*  2.. 2, 0x00000004 */
		FIELD  SL2_EN                                :  1;		/*  3.. 3, 0x00000008 */
		FIELD  UDM_EN                                :  1;		/*  4.. 4, 0x00000010 */
		FIELD  LCE_EN                                :  1;		/*  5.. 5, 0x00000020 */
		FIELD  G2G_EN                                :  1;		/*  6.. 6, 0x00000040 */
		FIELD  GGM_EN                                :  1;		/*  7.. 7, 0x00000080 */
		FIELD  GDR1_EN                               :  1;		/*  8.. 8, 0x00000100 */
		FIELD  GDR2_EN                               :  1;		/*  9.. 9, 0x00000200 */
		FIELD  DBS2_EN                               :  1;		/* 10..10, 0x00000400 */
		FIELD  OBC2_EN                               :  1;		/* 11..11, 0x00000800 */
		FIELD  RMG2_EN                               :  1;		/* 12..12, 0x00001000 */
		FIELD  BNR2_EN                               :  1;		/* 13..13, 0x00002000 */
		FIELD  RMM2_EN                               :  1;		/* 14..14, 0x00004000 */
		FIELD  LSC2_EN                               :  1;		/* 15..15, 0x00008000 */
		FIELD  RCP2_EN                               :  1;		/* 16..16, 0x00010000 */
		FIELD  PAK2_EN                               :  1;		/* 17..17, 0x00020000 */
		FIELD  RNR_EN                                :  1;		/* 18..18, 0x00040000 */
		FIELD  SL2G_EN                               :  1;		/* 19..19, 0x00080000 */
		FIELD  SL2H_EN                               :  1;		/* 20..20, 0x00100000 */
		FIELD  reserve_bit                           :  1;		/* 21..21, 0x00200000 */
		FIELD  SL2K_EN                               :  1;		/* 22..22, 0x00400000 */
		FIELD  FLC_EN                                :  1;		/* 23..23, 0x00800000 */
		FIELD  FLC2_EN                               :  1;		/* 24..24, 0x01000000 */
		FIELD  SMX1_EN                               :  1;		/* 25..25, 0x02000000 */
		FIELD  PAKG2_EN                              :  1;		/* 26..26, 0x04000000 */
		FIELD  G2G2_EN                               :  1;		/* 27..27, 0x08000000 */
		FIELD  GGM2_EN                               :  1;		/* 28..28, 0x10000000 */
		FIELD  WSHIFT_EN                             :  1;		/* 29..29, 0x20000000 */
		FIELD  WSYNC_EN                              :  1;		/* 30..30, 0x40000000 */
		FIELD  MDPCROP2_EN                           :  1;		/* 31..31, 0x80000000 */
} ISP_DIP_X_CTL_RGB_EN_T;

typedef union {
    typedef ISP_DIP_X_CTL_RGB_EN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CTL_RGB_EN_T;

typedef struct {
		FIELD  SMX4_EN                               :  1;		/*  0.. 0, 0x00000001 */
		FIELD  ADBS2_EN                              :  1;		/*  1.. 1, 0x00000002 */
		FIELD  DCPN2_EN                              :  1;		/*  2.. 2, 0x00000004 */
		FIELD  CPN2_EN                               :  1;		/*  3.. 3, 0x00000008 */
		FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
} ISP_DIP_X_CTL_RGB2_EN_T;

typedef union {
    typedef ISP_DIP_X_CTL_RGB2_EN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CTL_RGB2_EN_T;

typedef struct {
		FIELD  MFB_EN                                :  1;		/*  0.. 0, 0x00000001 */
		FIELD  C02B_EN                               :  1;		/*  1.. 1, 0x00000002 */
		FIELD  C24_EN                                :  1;		/*  2.. 2, 0x00000004 */
		FIELD  G2C_EN                                :  1;		/*  3.. 3, 0x00000008 */
		FIELD  C42_EN                                :  1;		/*  4.. 4, 0x00000010 */
		FIELD  NBC_EN                                :  1;		/*  5.. 5, 0x00000020 */
		FIELD  NBC2_EN                               :  1;		/*  6.. 6, 0x00000040 */
		FIELD  PCA_EN                                :  1;		/*  7.. 7, 0x00000080 */
		FIELD  SEEE_EN                               :  1;		/*  8.. 8, 0x00000100 */
		FIELD  COLOR_EN                              :  1;		/*  9.. 9, 0x00000200 */
		FIELD  NR3D_EN                               :  1;		/* 10..10, 0x00000400 */
		FIELD  SL2B_EN                               :  1;		/* 11..11, 0x00000800 */
		FIELD  SL2C_EN                               :  1;		/* 12..12, 0x00001000 */
		FIELD  SL2D_EN                               :  1;		/* 13..13, 0x00002000 */
		FIELD  SL2E_EN                               :  1;		/* 14..14, 0x00004000 */
		FIELD  SRZ1_EN                               :  1;		/* 15..15, 0x00008000 */
		FIELD  SRZ2_EN                               :  1;		/* 16..16, 0x00010000 */
		FIELD  CRZ_EN                                :  1;		/* 17..17, 0x00020000 */
		FIELD  MIX1_EN                               :  1;		/* 18..18, 0x00040000 */
		FIELD  MIX2_EN                               :  1;		/* 19..19, 0x00080000 */
		FIELD  MIX3_EN                               :  1;		/* 20..20, 0x00100000 */
		FIELD  CRSP_EN                               :  1;		/* 21..21, 0x00200000 */
		FIELD  C24B_EN                               :  1;		/* 22..22, 0x00400000 */
		FIELD  MDPCROP_EN                            :  1;		/* 23..23, 0x00800000 */
		FIELD  C02_EN                                :  1;		/* 24..24, 0x01000000 */
		FIELD  FE_EN                                 :  1;		/* 25..25, 0x02000000 */
		FIELD  rsv_26                                :  2;		/* 26..27, 0x0C000000 */
		FIELD  PLNW1_EN                              :  1;		/* 28..28, 0x10000000 */
		FIELD  PLNR1_EN                              :  1;		/* 29..29, 0x20000000 */
		FIELD  PLNW2_EN                              :  1;		/* 30..30, 0x40000000 */
		FIELD  PLNR2_EN                              :  1;		/* 31..31, 0x80000000 */
		}ISP_DIP_X_CTL_YUV_EN_T;

typedef union {
    typedef ISP_DIP_X_CTL_YUV_EN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CTL_YUV_EN_T;

typedef struct {
		FIELD  FM_EN                                 :  1;		/*  0.. 0, 0x00000001 */
		FIELD  SRZ3_EN                               :  1;		/*  1.. 1, 0x00000002 */
		FIELD  SRZ4_EN                               :  1;		/*  2.. 2, 0x00000004 */
		FIELD  SRZ5_EN                               :  1;		/*  3.. 3, 0x00000008 */
		FIELD  HFG_EN                                :  1;		/*  4.. 4, 0x00000010 */
		FIELD  SL2I_EN                               :  1;		/*  5.. 5, 0x00000020 */
		FIELD  NDG_EN                                :  1;		/*  6.. 6, 0x00000040 */
		FIELD  NDG2_EN                               :  1;		/*  7.. 7, 0x00000080 */
		FIELD  SMX2_EN                               :  1;		/*  8.. 8, 0x00000100 */
		FIELD  SMX3_EN                               :  1;		/*  9.. 9, 0x00000200 */
		FIELD  MIX4_EN                               :  1;		/* 10..10, 0x00000400 */
		FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
}ISP_DIP_X_CTL_YUV2_EN_T;

typedef union {
    typedef ISP_DIP_X_CTL_YUV2_EN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CTL_YUV2_EN_T;


//
typedef union {
    enum { COUNT = 4 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_CTL_RGB_EN_T  en_rgb;
        ISP_NVRAM_CTL_RGB2_EN_T  en_rgb2;
        ISP_NVRAM_CTL_YUV_EN_T  en_yuv;
        ISP_NVRAM_CTL_YUV2_EN_T en_yuv2;
    };
} ISP_NVRAM_CTL_EN_P2_T;
typedef struct {
    FIELD  ADBS_EDGE                             :  4;      /*  0.. 3, 0x0000000F */
    FIELD  ADBS_LE_INV_CTL                       :  4;      /*  4.. 7, 0x000000F0 */
    FIELD  rsv_8                                 : 24;      /*  8..31, 0xFFFFFF00 */
} ISP_CAM_ADBS_CTL_T; // /* 0x1A004D50 */

typedef union {
    typedef ISP_CAM_ADBS_CTL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ADBS_CTL_T;

typedef struct {
    FIELD  ADBS_LUMA_MODE                        :  1;      /*  0.. 0, 0x00000001 */
    FIELD  rsv_1                                 : 15;      /*  1..15, 0x0000FFFE */
    FIELD  ADBS_BLD_MXRT                         :  8;      /* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                :  8;      /* 24..31, 0xFF000000 */
} ISP_CAM_ADBS_GRAY_BLD_0_T; // /* 0x1A004D54 */

typedef union {
    typedef ISP_CAM_ADBS_GRAY_BLD_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ADBS_GRAY_BLD_0_T;

typedef struct {
    FIELD  ADBS_BLD_LOW                          : 12;      /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
    FIELD  ADBS_BLD_SLP                          : 12;      /* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
} ISP_CAM_ADBS_GRAY_BLD_1_T; // /* 0x1A004D58 */

typedef union {
    typedef ISP_CAM_ADBS_GRAY_BLD_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ADBS_GRAY_BLD_1_T;

typedef struct {
    FIELD  ADBS_BIAS_R0                          :  8;      /*  0.. 7, 0x000000FF */
    FIELD  ADBS_BIAS_R1                          :  8;      /*  8..15, 0x0000FF00 */
    FIELD  ADBS_BIAS_R2                          :  8;      /* 16..23, 0x00FF0000 */
    FIELD  ADBS_BIAS_R3                          :  8;      /* 24..31, 0xFF000000 */
} ISP_CAM_ADBS_BIAS_LUT_R0_T; // /* 0x1A004D5C */

typedef union {
    typedef ISP_CAM_ADBS_BIAS_LUT_R0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ADBS_BIAS_LUT_R0_T;

typedef struct {
    FIELD  ADBS_BIAS_R4                          :  8;      /*  0.. 7, 0x000000FF */
    FIELD  ADBS_BIAS_R5                          :  8;      /*  8..15, 0x0000FF00 */
    FIELD  ADBS_BIAS_R6                          :  8;      /* 16..23, 0x00FF0000 */
    FIELD  ADBS_BIAS_R7                          :  8;      /* 24..31, 0xFF000000 */
} ISP_CAM_ADBS_BIAS_LUT_R1_T; // /* 0x1A004D60 */

typedef union {
    typedef ISP_CAM_ADBS_BIAS_LUT_R1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ADBS_BIAS_LUT_R1_T;

typedef struct {
    FIELD  ADBS_BIAS_R8                          :  8;      /*  0.. 7, 0x000000FF */
    FIELD  ADBS_BIAS_R9                          :  8;      /*  8..15, 0x0000FF00 */
    FIELD  ADBS_BIAS_R10                         :  8;      /* 16..23, 0x00FF0000 */
    FIELD  ADBS_BIAS_R11                         :  8;      /* 24..31, 0xFF000000 */
} ISP_CAM_ADBS_BIAS_LUT_R2_T; // /* 0x1A004D64 */

typedef union {
    typedef ISP_CAM_ADBS_BIAS_LUT_R2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ADBS_BIAS_LUT_R2_T;

typedef struct {
    FIELD  ADBS_BIAS_R12                         :  8;      /*  0.. 7, 0x000000FF */
    FIELD  ADBS_BIAS_R13                         :  8;      /*  8..15, 0x0000FF00 */
    FIELD  ADBS_BIAS_R14                         :  8;      /* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                :  8;      /* 24..31, 0xFF000000 */
} ISP_CAM_ADBS_BIAS_LUT_R3_T; // /* 0x1A004D68 */

typedef union {
    typedef ISP_CAM_ADBS_BIAS_LUT_R3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ADBS_BIAS_LUT_R3_T;

typedef struct {
    FIELD  ADBS_BIAS_G0                          :  8;      /*  0.. 7, 0x000000FF */
    FIELD  ADBS_BIAS_G1                          :  8;      /*  8..15, 0x0000FF00 */
    FIELD  ADBS_BIAS_G2                          :  8;      /* 16..23, 0x00FF0000 */
    FIELD  ADBS_BIAS_G3                          :  8;      /* 24..31, 0xFF000000 */
} ISP_CAM_ADBS_BIAS_LUT_G0_T; // /* 0x1A004D6C */

typedef union {
    typedef ISP_CAM_ADBS_BIAS_LUT_G0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ADBS_BIAS_LUT_G0_T;

typedef struct {
    FIELD  ADBS_BIAS_G4                          :  8;      /*  0.. 7, 0x000000FF */
    FIELD  ADBS_BIAS_G5                          :  8;      /*  8..15, 0x0000FF00 */
    FIELD  ADBS_BIAS_G6                          :  8;      /* 16..23, 0x00FF0000 */
    FIELD  ADBS_BIAS_G7                          :  8;      /* 24..31, 0xFF000000 */
} ISP_CAM_ADBS_BIAS_LUT_G1_T; // /* 0x1A004D70 */

typedef union {
    typedef ISP_CAM_ADBS_BIAS_LUT_G1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ADBS_BIAS_LUT_G1_T;

typedef struct {
    FIELD  ADBS_BIAS_G8                          :  8;      /*  0.. 7, 0x000000FF */
    FIELD  ADBS_BIAS_G9                          :  8;      /*  8..15, 0x0000FF00 */
    FIELD  ADBS_BIAS_G10                         :  8;      /* 16..23, 0x00FF0000 */
    FIELD  ADBS_BIAS_G11                         :  8;      /* 24..31, 0xFF000000 */
} ISP_CAM_ADBS_BIAS_LUT_G2_T; // /* 0x1A004D74 */

typedef union {
    typedef ISP_CAM_ADBS_BIAS_LUT_G2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ADBS_BIAS_LUT_G2_T;

typedef struct {
    FIELD  ADBS_BIAS_G12                         :  8;      /*  0.. 7, 0x000000FF */
    FIELD  ADBS_BIAS_G13                         :  8;      /*  8..15, 0x0000FF00 */
    FIELD  ADBS_BIAS_G14                         :  8;      /* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                :  8;      /* 24..31, 0xFF000000 */
} ISP_CAM_ADBS_BIAS_LUT_G3_T; // /* 0x1A004D78 */

typedef union {
    typedef ISP_CAM_ADBS_BIAS_LUT_G3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ADBS_BIAS_LUT_G3_T;

typedef struct {
    FIELD  ADBS_BIAS_B0                          :  8;      /*  0.. 7, 0x000000FF */
    FIELD  ADBS_BIAS_B1                          :  8;      /*  8..15, 0x0000FF00 */
    FIELD  ADBS_BIAS_B2                          :  8;      /* 16..23, 0x00FF0000 */
    FIELD  ADBS_BIAS_B3                          :  8;      /* 24..31, 0xFF000000 */
} ISP_CAM_ADBS_BIAS_LUT_B0_T; // /* 0x1A004D7C */

typedef union {
    typedef ISP_CAM_ADBS_BIAS_LUT_B0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ADBS_BIAS_LUT_B0_T;

typedef struct {
    FIELD  ADBS_BIAS_B4                          :  8;      /*  0.. 7, 0x000000FF */
    FIELD  ADBS_BIAS_B5                          :  8;      /*  8..15, 0x0000FF00 */
    FIELD  ADBS_BIAS_B6                          :  8;      /* 16..23, 0x00FF0000 */
    FIELD  ADBS_BIAS_B7                          :  8;      /* 24..31, 0xFF000000 */
} ISP_CAM_ADBS_BIAS_LUT_B1_T; // /* 0x1A004D80 */

typedef union {
    typedef ISP_CAM_ADBS_BIAS_LUT_B1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ADBS_BIAS_LUT_B1_T;

typedef struct {
    FIELD  ADBS_BIAS_B8                          :  8;      /*  0.. 7, 0x000000FF */
    FIELD  ADBS_BIAS_B9                          :  8;      /*  8..15, 0x0000FF00 */
    FIELD  ADBS_BIAS_B10                         :  8;      /* 16..23, 0x00FF0000 */
    FIELD  ADBS_BIAS_B11                         :  8;      /* 24..31, 0xFF000000 */
} ISP_CAM_ADBS_BIAS_LUT_B2_T; // /* 0x1A004D84 */

typedef union {
    typedef ISP_CAM_ADBS_BIAS_LUT_B2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ADBS_BIAS_LUT_B2_T;

typedef struct {
    FIELD  ADBS_BIAS_B12                         :  8;      /*  0.. 7, 0x000000FF */
    FIELD  ADBS_BIAS_B13                         :  8;      /*  8..15, 0x0000FF00 */
    FIELD  ADBS_BIAS_B14                         :  8;      /* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                :  8;      /* 24..31, 0xFF000000 */
} ISP_CAM_ADBS_BIAS_LUT_B3_T; // /* 0x1A004D88 */

typedef union {
    typedef ISP_CAM_ADBS_BIAS_LUT_B3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ADBS_BIAS_LUT_B3_T;

typedef struct {
    FIELD  ADBS_GAIN_R                           : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                :  3;      /* 13..15, 0x0000E000 */
    FIELD  ADBS_GAIN_B                           : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                :  3;      /* 29..31, 0xE0000000 */
} ISP_CAM_ADBS_GAIN_0_T; // /* 0x1A004D8C */

typedef union {
    typedef ISP_CAM_ADBS_GAIN_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ADBS_GAIN_0_T;

typedef struct {
    FIELD  ADBS_GAIN_G                           : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                : 19;      /* 13..31, 0xFFFFE000 */
} ISP_CAM_ADBS_GAIN_1_T; // /* 0x1A004D90 */

typedef union {
    typedef ISP_CAM_ADBS_GAIN_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ADBS_GAIN_1_T;

typedef struct {
    FIELD  ADBS_IVGN_R                           : 10;      /*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                :  6;      /* 10..15, 0x0000FC00 */
    FIELD  ADBS_IVGN_B                           : 10;      /* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                :  6;      /* 26..31, 0xFC000000 */
} ISP_CAM_ADBS_IVGN_0_T; // /* 0x1A004D94 */

typedef union {
    typedef ISP_CAM_ADBS_IVGN_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ADBS_IVGN_0_T;

typedef struct {
    FIELD  ADBS_IVGN_G                           : 10;      /*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                : 22;      /* 10..31, 0xFFFFFC00 */
} ISP_CAM_ADBS_IVGN_1_T; // /* 0x1A004D98 */

typedef union {
    typedef ISP_CAM_ADBS_IVGN_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ADBS_IVGN_1_T;

typedef struct {
    FIELD  ADBS_HDR_GN                           :  8;      /*  0.. 7, 0x000000FF */
    FIELD  ADBS_HDR_GN2                          :  8;      /*  8..15, 0x0000FF00 */
    FIELD  ADBS_HDR_GNP                          :  3;      /* 16..18, 0x00070000 */
    FIELD  rsv_19                                :  1;      /* 19..19, 0x00080000 */
    FIELD  ADBS_HDR_OSCTH                        : 12;      /* 20..31, 0xFFF00000 */
} ISP_CAM_ADBS_HDR_T; // /* 0x1A004D9C */

typedef union {
    typedef ISP_CAM_ADBS_HDR_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ADBS_HDR_T;

typedef struct {
    FIELD  rsv_0                                 : 29;      /*  0..28, 0x1FFFFFFF */
    FIELD  ADBS_EIGER_EN                         :  1;      /* 29..29, 0x20000000 */
    FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
} ISP_CAM_ADBS_CMDL_ONLY_1_T; // /* 0x1A004DA0 */

typedef union {
    typedef ISP_CAM_ADBS_CMDL_ONLY_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ADBS_CMDL_ONLY_1_T;

typedef union {
    enum { COUNT = 14 };
    struct {
        //ISP_NVRAM_ADBS_CTL_T  ctl;
        ISP_NVRAM_ADBS_GRAY_BLD_0_T  gray_bld_0;
        ISP_NVRAM_ADBS_GRAY_BLD_1_T  gray_bld_1;
        ISP_NVRAM_ADBS_BIAS_LUT_R0_T  bias_lut_r0;
        ISP_NVRAM_ADBS_BIAS_LUT_R1_T  bias_lut_r1;
        ISP_NVRAM_ADBS_BIAS_LUT_R2_T  bias_lut_r2;
        ISP_NVRAM_ADBS_BIAS_LUT_R3_T  bias_lut_r3;
        ISP_NVRAM_ADBS_BIAS_LUT_G0_T  bias_lut_g0;
        ISP_NVRAM_ADBS_BIAS_LUT_G1_T  bias_lut_g1;
        ISP_NVRAM_ADBS_BIAS_LUT_G2_T  bias_lut_g2;
        ISP_NVRAM_ADBS_BIAS_LUT_G3_T  bias_lut_g3;
        ISP_NVRAM_ADBS_BIAS_LUT_B0_T  bias_lut_b0;
        ISP_NVRAM_ADBS_BIAS_LUT_B1_T  bias_lut_b1;
        ISP_NVRAM_ADBS_BIAS_LUT_B2_T  bias_lut_b2;
        ISP_NVRAM_ADBS_BIAS_LUT_B3_T  bias_lut_b3;
        //ISP_NVRAM_ADBS_GAIN_0_T  gain_0;
        //ISP_NVRAM_ADBS_GAIN_1_T  gain_1;
        //ISP_NVRAM_ADBS_IVGN_0_T  ivgn_0;
        //ISP_NVRAM_ADBS_IVGN_1_T  ivgn_1;
        //ISP_NVRAM_ADBS_HDR_T  hdr;
        //ISP_NVRAM_ADBS_CMDL_ONLY_1_T  cmdl_only_1;
    };
    MUINT32 set[COUNT];
} ISP_NVRAM_ADBS_T;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// DBS
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef struct {
    FIELD  DBS_OFST                              : 12;    /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                :  4;    /* 12..15, 0x0000F000 */
    FIELD  DBS_SL                                :  8;    /* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                :  8;    /* 24..31, 0xFF000000 */
} ISP_CAM_DBS_SIGMA_T; // /* 0x1A004C00 */

typedef union {
    typedef ISP_CAM_DBS_SIGMA_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DBS_SIGMA_T;

typedef struct {
    FIELD  DBS_BIAS_Y0                           :  8;    /*  0.. 7, 0x000000FF */
    FIELD  DBS_BIAS_Y1                           :  8;    /*  8..15, 0x0000FF00 */
    FIELD  DBS_BIAS_Y2                           :  8;    /* 16..23, 0x00FF0000 */
    FIELD  DBS_BIAS_Y3                           :  8;    /* 24..31, 0xFF000000 */
} ISP_CAM_DBS_BSTBL_0_T; // /* 0x1A004C04 */

typedef union {
    typedef ISP_CAM_DBS_BSTBL_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DBS_BSTBL_0_T;

typedef struct {
    FIELD  DBS_BIAS_Y4                           :  8;    /*  0.. 7, 0x000000FF */
    FIELD  DBS_BIAS_Y5                           :  8;    /*  8..15, 0x0000FF00 */
    FIELD  DBS_BIAS_Y6                           :  8;    /* 16..23, 0x00FF0000 */
    FIELD  DBS_BIAS_Y7                           :  8;    /* 24..31, 0xFF000000 */
} ISP_CAM_DBS_BSTBL_1_T; // /* 0x1A004C08 */

typedef union {
    typedef ISP_CAM_DBS_BSTBL_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DBS_BSTBL_1_T;

typedef struct {
    FIELD  DBS_BIAS_Y8                           :  8;    /*  0.. 7, 0x000000FF */
    FIELD  DBS_BIAS_Y9                           :  8;    /*  8..15, 0x0000FF00 */
    FIELD  DBS_BIAS_Y10                          :  8;    /* 16..23, 0x00FF0000 */
    FIELD  DBS_BIAS_Y11                          :  8;    /* 24..31, 0xFF000000 */
} ISP_CAM_DBS_BSTBL_2_T; // /* 0x1A004C0C */

typedef union {
    typedef ISP_CAM_DBS_BSTBL_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DBS_BSTBL_2_T;

typedef struct {
    FIELD  DBS_BIAS_Y12                          :  8;    /*  0.. 7, 0x000000FF */
    FIELD  DBS_BIAS_Y13                          :  8;    /*  8..15, 0x0000FF00 */
    FIELD  DBS_BIAS_Y14                          :  8;    /* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                :  8;    /* 24..31, 0xFF000000 */
} ISP_CAM_DBS_BSTBL_3_T; // /* 0x1A004C10 */

typedef union {
    typedef ISP_CAM_DBS_BSTBL_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DBS_BSTBL_3_T;

typedef struct {
    FIELD  DBS_HDR_GNP                           :  3;    /*  0.. 2, 0x00000007 */
    FIELD  rsv_3                                 :  1;    /*  3.. 3, 0x00000008 */
    FIELD  DBS_SL_EN                             :  1;    /*  4.. 4, 0x00000010 */
    FIELD  rsv_5                                 :  3;    /*  5.. 7, 0x000000E0 */
    FIELD  DBS_LE_INV_CTL                        :  4;    /*  8..11, 0x00000F00 */
    FIELD  DBS_EDGE                              :  4;    /* 12..15, 0x0000F000 */
    FIELD  DBS_HDR_GAIN                          :  8;    /* 16..23, 0x00FF0000 */
    FIELD  DBS_HDR_GAIN2                         :  8;    /* 24..31, 0xFF000000 */
} ISP_CAM_DBS_CTL_T; // /* 0x1A004C14 */

typedef union {
    typedef ISP_CAM_DBS_CTL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DBS_CTL_T;

typedef struct {
    FIELD  DBS_HDR_OSCTH                         : 12;    /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                : 20;    /* 12..31, 0xFFFFF000 */
} ISP_CAM_DBS_CTL_2_T; // /* 0x1A004C18 */

typedef union {
    typedef ISP_CAM_DBS_CTL_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DBS_CTL_2_T;

typedef struct {
    FIELD  DBS_MUL_B                             :  8;    /*  0.. 7, 0x000000FF */
    FIELD  DBS_MUL_GB                            :  8;    /*  8..15, 0x0000FF00 */
    FIELD  DBS_MUL_GR                            :  8;    /* 16..23, 0x00FF0000 */
    FIELD  DBS_MUL_R                             :  8;    /* 24..31, 0xFF000000 */
} ISP_CAM_DBS_SIGMA_2_T; // /* 0x1A004C1C */

typedef union {
    typedef ISP_CAM_DBS_SIGMA_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DBS_SIGMA_2_T;

typedef struct {
    FIELD  DBS_YGN_B                             :  6;    /*  0.. 5, 0x0000003F */
    FIELD  rsv_6                                 :  2;    /*  6.. 7, 0x000000C0 */
    FIELD  DBS_YGN_GB                            :  6;    /*  8..13, 0x00003F00 */
    FIELD  rsv_14                                :  2;    /* 14..15, 0x0000C000 */
    FIELD  DBS_YGN_GR                            :  6;    /* 16..21, 0x003F0000 */
    FIELD  rsv_22                                :  2;    /* 22..23, 0x00C00000 */
    FIELD  DBS_YGN_R                             :  6;    /* 24..29, 0x3F000000 */
    FIELD  rsv_30                                :  2;    /* 30..31, 0xC0000000 */
} ISP_CAM_DBS_YGN_T; // /* 0x1A004C20 */

typedef union {
    typedef ISP_CAM_DBS_YGN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DBS_YGN_T;

typedef struct {
    FIELD  DBS_SL_Y1                             : 10;    /*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                :  6;    /* 10..15, 0x0000FC00 */
    FIELD  DBS_SL_Y2                             : 10;    /* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                :  6;    /* 26..31, 0xFC000000 */
} ISP_CAM_DBS_SL_Y12_T; //  /* 0x1A004C24 */

typedef union {
    typedef ISP_CAM_DBS_SL_Y12_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DBS_SL_Y12_T;

typedef struct {
    FIELD  DBS_SL_Y3                             : 10;    /*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                :  6;    /* 10..15, 0x0000FC00 */
    FIELD  DBS_SL_Y4                             : 10;    /* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                :  6;    /* 26..31, 0xFC000000 */
} ISP_CAM_DBS_SL_Y34_T; //  /* 0x1A004C28 */

typedef union {
    typedef ISP_CAM_DBS_SL_Y34_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DBS_SL_Y34_T;

typedef struct {
    FIELD  DBS_SL_G1                             : 10;    /*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                :  6;    /* 10..15, 0x0000FC00 */
    FIELD  DBS_SL_G2                             : 10;    /* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                :  6;    /* 26..31, 0xFC000000 */
} ISP_CAM_DBS_SL_G12_T; //  /* 0x1A004C2C */

typedef union {
    typedef ISP_CAM_DBS_SL_G12_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DBS_SL_G12_T;

typedef struct {
    FIELD  DBS_SL_G3                             : 10;    /*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                :  6;    /* 10..15, 0x0000FC00 */
    FIELD  DBS_SL_G4                             : 10;    /* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                :  6;    /* 26..31, 0xFC000000 */
} ISP_CAM_DBS_SL_G34_T; //  /* 0x1A004C30 */

typedef union {
    typedef ISP_CAM_DBS_SL_G34_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DBS_SL_G34_T;

typedef union {
    enum { COUNT = 12 };
    struct {
        ISP_NVRAM_DBS_SIGMA_T        sigma;
        ISP_NVRAM_DBS_BSTBL_0_T      bstbl_0;
        ISP_NVRAM_DBS_BSTBL_1_T      bstbl_1;
        ISP_NVRAM_DBS_BSTBL_2_T      bstbl_2;
        ISP_NVRAM_DBS_BSTBL_3_T      bstbl_3;
        ISP_NVRAM_DBS_CTL_T          ctl;
        //ISP_NVRAM_DBS_CTL_2_T    ctl_2;
        ISP_NVRAM_DBS_SIGMA_2_T  sigma_2;
        ISP_NVRAM_DBS_YGN_T      ygn;
        ISP_NVRAM_DBS_SL_Y12_T   sl_y12;
        ISP_NVRAM_DBS_SL_Y34_T   sl_y34;
        ISP_NVRAM_DBS_SL_G12_T   sl_g12;
        ISP_NVRAM_DBS_SL_G34_T   sl_g34;
    };
    MUINT32 set[COUNT];
} ISP_NVRAM_DBS_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// OBC
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CAM_OBC_OFFST0 CAM+0500H: OB for B channel
typedef struct {
        FIELD OBC_OFST_B                : 13;
        FIELD rsv_13                    : 19;
} ISP_CAM_OBC_OFFST0_T;

typedef union {
    enum { MASK     = 0x00001FFF };
    typedef ISP_CAM_OBC_OFFST0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_OBC_OFFST0_T;

// CAM_OBC_OFFST1 CAM+0504H: OB for Gr channel
typedef struct {
        FIELD OBC_OFST_GR               : 13;
        FIELD rsv_13                    : 19;
} ISP_CAM_OBC_OFFST1_T;

typedef union {
    enum { MASK     = 0x00001FFF };
    typedef ISP_CAM_OBC_OFFST1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_OBC_OFFST1_T;

// CAM_OBC_OFFST2 CAM+0508H: OB for Gb channel
typedef struct {
        FIELD OBC_OFST_GB               : 13;
        FIELD rsv_13                    : 19;
} ISP_CAM_OBC_OFFST2_T;

typedef union {
    enum { MASK     = 0x00001FFF };
    typedef ISP_CAM_OBC_OFFST2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_OBC_OFFST2_T;

// CAM_OBC_OFFST3 CAM+050CH: OB for R channel
typedef struct {
        FIELD OBC_OFST_R                : 13;
        FIELD rsv_13                    : 19;
} ISP_CAM_OBC_OFFST3_T;

typedef union {
    enum { MASK     = 0x00001FFF };
    typedef ISP_CAM_OBC_OFFST3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_OBC_OFFST3_T;

// CAM_OBC_GAIN0 CAM+0510H: OB gain for B channel
typedef struct {
        FIELD OBC_GAIN_B                : 13;
        FIELD rsv_13                    : 19;
} ISP_CAM_OBC_GAIN0_T;

typedef union {
    enum { MASK     = 0x00001FFF };
    typedef ISP_CAM_OBC_GAIN0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_OBC_GAIN0_T;

// CAM_OBC_GAIN1 CAM+0514H: OB gain for Gr channel
typedef struct {
        FIELD OBC_GAIN_GR               : 13;
        FIELD rsv_13                    : 19;
} ISP_CAM_OBC_GAIN1_T;

typedef union {
    enum { MASK     = 0x00001FFF };
    typedef ISP_CAM_OBC_GAIN1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_OBC_GAIN1_T;

// CAM_OBC_GAIN2 CAM+0518H: OB gain for Gb channel
typedef struct {
        FIELD OBC_GAIN_GB               : 13;
        FIELD rsv_13                    : 19;
} ISP_CAM_OBC_GAIN2_T;

typedef union {
    enum { MASK     = 0x00001FFF };
    typedef ISP_CAM_OBC_GAIN2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_OBC_GAIN2_T;

// CAM_OBC_GAIN3 CAM+051CH: OB gain for R channel
typedef struct {
        FIELD OBC_GAIN_R                : 13;
        FIELD rsv_13                    : 19;
} ISP_CAM_OBC_GAIN3_T;

typedef union {
    enum { MASK     = 0x00001FFF };
    typedef ISP_CAM_OBC_GAIN3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_OBC_GAIN3_T;

//
typedef union {
    enum { COUNT = 8 };
    struct {
        ISP_NVRAM_OBC_OFFST0_T  offst0; // Q.1.12
        ISP_NVRAM_OBC_OFFST1_T  offst1; // Q.1.12
        ISP_NVRAM_OBC_OFFST2_T  offst2; // Q.1.12
        ISP_NVRAM_OBC_OFFST3_T  offst3; // Q.1.12
        ISP_NVRAM_OBC_GAIN0_T   gain0; // Q.0.4.9
        ISP_NVRAM_OBC_GAIN1_T   gain1; // Q.0.4.9
        ISP_NVRAM_OBC_GAIN2_T   gain2; // Q.0.4.9
        ISP_NVRAM_OBC_GAIN3_T   gain3; // Q.0.4.9
    };
    MUINT32 set[COUNT];
} ISP_NVRAM_OBC_T;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// RMG
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef struct {
	FIELD RMG_IHDR_EN				: 1;
	FIELD RMG_IHDR_LE_FIRST			: 1;
	FIELD rsv_2						: 2;
	FIELD RMG_ZHDR_EN				: 1;
	FIELD RMG_ZHDR_RLE 				: 1;
	FIELD RMG_ZHDR_GLE 				: 1;
	FIELD RMG_ZHDR_BLE 				: 1;
	FIELD RMG_HDR_TH				: 12;
	FIELD RMG_OSC_TH				: 12;

} ISP_CAM_RMG_HDR_CFG_T;

typedef union {
    typedef ISP_CAM_RMG_HDR_CFG_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_RMG_HDR_CFG_T;


typedef struct {
	FIELD RMG_HDR_GAIN				: 9;
	FIELD rsv_9						: 7;
	FIELD RMG_HDR_RATIO				: 9;
	FIELD rsv_25					: 3;
	FIELD RMG_LE_INV_CTL			: 4;
} ISP_CAM_RMG_HDR_GAIN_T;

typedef union {
    typedef ISP_CAM_RMG_HDR_GAIN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_RMG_HDR_GAIN_T;

typedef struct{
    FIELD RMG_HDR_THK               : 11;
    FIELD rsv_11                    : 21;
} ISP_CAM_RMG_HDR_CFG2_T;

typedef union {
    typedef ISP_CAM_RMG_HDR_CFG2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_RMG_HDR_CFG2_T;


//
typedef union {
    enum { COUNT = 3 };
    MUINT32 set[COUNT];
    struct {
		ISP_NVRAM_RMG_HDR_CFG_T      hdr_cfg;
		ISP_NVRAM_RMG_HDR_GAIN_T     hdr_gain;
        ISP_NVRAM_RMG_HDR_CFG2_T     hdr_cfg2;
    };
} ISP_NVRAM_RMG_T;



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BNR_BPC
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CAM_BNR_BPC_CON CAM+0800H
typedef struct {
    FIELD  BPC_EN                                :  1;      /*  0.. 0, 0x00000001 */
    FIELD  rsv_1                                 :  3;      /*  1.. 3, 0x0000000E */
    FIELD  BPC_LUT_EN                            :  1;      /*  4.. 4, 0x00000010 */
    FIELD  BPC_TABLE_END_MODE                    :  1;      /*  5.. 5, 0x00000020 */
    FIELD  rsv_6                                 :  2;      /*  6.. 7, 0x000000C0 */
    FIELD  BPC_AVG_MODE                          :  1;      /*  8.. 8, 0x00000100 */
    FIELD  rsv_9                                 :  3;      /*  9..11, 0x00000E00 */
    FIELD  BPC_DTC_MODE                          :  2;      /* 12..13, 0x00003000 */
    FIELD  BPC_CS_MODE                           :  2;      /* 14..15, 0x0000C000 */
    FIELD  BPC_CRC_MODE                          :  2;      /* 16..17, 0x00030000 */
    FIELD  BPC_EXC                               :  1;      /* 18..18, 0x00040000 */
    FIELD  BPC_BLD_MODE                          :  1;      /* 19..19, 0x00080000 */
    FIELD  BNR_LE_INV_CTL                        :  4;      /* 20..23, 0x00F00000 */
    FIELD  BNR_OSC_COUNT                         :  4;      /* 24..27, 0x0F000000 */
    FIELD  BNR_EDGE                              :  4;      /* 28..31, 0xF0000000 */
} ISP_CAM_BNR_BPC_CON_T; // /* 0x1A004420 */

typedef union {
    typedef ISP_CAM_BNR_BPC_CON_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_BPC_CON_T;

typedef struct {
    FIELD  BPC_TH_LWB                            : 12;      /*  0..11, 0x00000FFF */
    FIELD  BPC_TH_Y                              : 12;      /* 12..23, 0x00FFF000 */
    FIELD  BPC_BLD_SLP0                          :  8;      /* 24..31, 0xFF000000 */
} ISP_CAM_BNR_BPC_TH1_T; // /* 0x1A004424 */

typedef union {
    typedef ISP_CAM_BNR_BPC_TH1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_BPC_TH1_T;

typedef struct {
    FIELD  BPC_TH_UPB                            : 12;      /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
    FIELD  BPC_BLD0                              :  7;      /* 16..22, 0x007F0000 */
    FIELD  rsv_23                                :  1;      /* 23..23, 0x00800000 */
    FIELD  BPC_BLD1                              :  7;      /* 24..30, 0x7F000000 */
    FIELD  rsv_31                                :  1;      /* 31..31, 0x80000000 */
} ISP_CAM_BNR_BPC_TH2_T; // /* 0x1A004428 */

typedef union {
    typedef ISP_CAM_BNR_BPC_TH2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_BPC_TH2_T;

typedef struct {
    FIELD  BPC_TH_XA                             : 12;      /*  0..11, 0x00000FFF */
    FIELD  BPC_TH_XB                             : 12;      /* 12..23, 0x00FFF000 */
    FIELD  BPC_TH_SLA                            :  4;      /* 24..27, 0x0F000000 */
    FIELD  BPC_TH_SLB                            :  4;      /* 28..31, 0xF0000000 */
} ISP_CAM_BNR_BPC_TH3_T; // /* 0x1A00442C */

typedef union {
    typedef ISP_CAM_BNR_BPC_TH3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_BPC_TH3_T;

typedef struct {
    FIELD  BPC_DK_TH_XA                          : 12;      /*  0..11, 0x00000FFF */
    FIELD  BPC_DK_TH_XB                          : 12;      /* 12..23, 0x00FFF000 */
    FIELD  BPC_DK_TH_SLA                         :  4;      /* 24..27, 0x0F000000 */
    FIELD  BPC_DK_TH_SLB                         :  4;      /* 28..31, 0xF0000000 */
} ISP_CAM_BNR_BPC_TH4_T; // /* 0x1A004430 */

typedef union {
    typedef ISP_CAM_BNR_BPC_TH4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_BPC_TH4_T;

typedef struct {
    FIELD  BPC_RNG                               :  4;      /*  0.. 3, 0x0000000F */
    FIELD  BPC_CS_RNG                            :  3;      /*  4.. 6, 0x00000070 */
    FIELD  rsv_7                                 :  1;      /*  7.. 7, 0x00000080 */
    FIELD  BPC_CT_LV                             :  4;      /*  8..11, 0x00000F00 */
    FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
    FIELD  BPC_TH_MUL                            :  4;      /* 16..19, 0x000F0000 */
    FIELD  rsv_20                                :  4;      /* 20..23, 0x00F00000 */
    FIELD  BPC_NO_LV                             :  3;      /* 24..26, 0x07000000 */
    FIELD  rsv_27                                :  5;      /* 27..31, 0xF8000000 */
} ISP_CAM_BNR_BPC_DTC_T; // /* 0x1A004434 */

typedef union {
    typedef ISP_CAM_BNR_BPC_DTC_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_BPC_DTC_T;

typedef struct {
    FIELD  BPC_DIR_MAX                           :  8;      /*  0.. 7, 0x000000FF */
    FIELD  BPC_DIR_TH                            :  8;      /*  8..15, 0x0000FF00 */
    FIELD  BPC_RANK_IDXR                         :  3;      /* 16..18, 0x00070000 */
    FIELD  BPC_RANK_IDXG                         :  3;      /* 19..21, 0x00380000 */
    FIELD  rsv_22                                :  2;      /* 22..23, 0x00C00000 */
    FIELD  BPC_DIR_TH2                           :  8;      /* 24..31, 0xFF000000 */
} ISP_CAM_BNR_BPC_COR_T; // /* 0x1A004438 */

typedef union {
    typedef ISP_CAM_BNR_BPC_COR_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_BPC_COR_T;

typedef struct {
    FIELD  BPC_XOFFSET                           : 14;      /*  0..13, 0x00003FFF */
    FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
    FIELD  BPC_YOFFSET                           : 14;      /* 16..29, 0x3FFF0000 */
    FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
} ISP_CAM_BNR_BPC_TBLI1_T; // /* 0x1A00443C */

typedef union {
    typedef ISP_CAM_BNR_BPC_TBLI1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_BPC_TBLI1_T;

typedef struct {
    FIELD  BPC_XSIZE                             : 14;      /*  0..13, 0x00003FFF */
    FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
    FIELD  BPC_YSIZE                             : 14;      /* 16..29, 0x3FFF0000 */
    FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
} ISP_CAM_BNR_BPC_TBLI2_T; // /* 0x1A004440 */

typedef union {
    typedef ISP_CAM_BNR_BPC_TBLI2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_BPC_TBLI2_T;

typedef struct {
    FIELD  BPC_C_TH_LWB                          : 12;      /*  0..11, 0x00000FFF */
    FIELD  BPC_C_TH_Y                            : 12;      /* 12..23, 0x00FFF000 */
    FIELD  rsv_24                                :  8;      /* 24..31, 0xFF000000 */
} ISP_CAM_BNR_BPC_TH1_C_T; // /* 0x1A004444 */

typedef union {
    typedef ISP_CAM_BNR_BPC_TH1_C_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_BPC_TH1_C_T;

typedef struct {
    FIELD  BPC_C_TH_UPB                          : 12;      /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                :  1;      /* 12..12, 0x00001000 */
    FIELD  BPC_RANK_IDXB                         :  3;      /* 13..15, 0x0000E000 */
    FIELD  BPC_BLD_LWB                           : 12;      /* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
} ISP_CAM_BNR_BPC_TH2_C_T; // /* 0x1A004448 */

typedef union {
    typedef ISP_CAM_BNR_BPC_TH2_C_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_BPC_TH2_C_T;

typedef struct {
    FIELD  BPC_C_TH_XA                           : 12;      /*  0..11, 0x00000FFF */
    FIELD  BPC_C_TH_XB                           : 12;      /* 12..23, 0x00FFF000 */
    FIELD  BPC_C_TH_SLA                          :  4;      /* 24..27, 0x0F000000 */
    FIELD  BPC_C_TH_SLB                          :  4;      /* 28..31, 0xF0000000 */
} ISP_CAM_BNR_BPC_TH3_C_T; // /* 0x1A00444C */

typedef union {
    typedef ISP_CAM_BNR_BPC_TH3_C_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_BPC_TH3_C_T;

//
typedef union {
    enum { COUNT = 10 };
    struct {
        ISP_NVRAM_BNR_BPC_CON_T     con;
        ISP_NVRAM_BNR_BPC_TH1_T     th1;
        ISP_NVRAM_BNR_BPC_TH2_T     th2;
        ISP_NVRAM_BNR_BPC_TH3_T     th3;
        ISP_NVRAM_BNR_BPC_TH4_T     th4;
        ISP_NVRAM_BNR_BPC_DTC_T     dtc;
        ISP_NVRAM_BNR_BPC_COR_T     cor;
        //ISP_NVRAM_BNR_BPC_TBLI1_T   tbli1;
        //ISP_NVRAM_BNR_BPC_TBLI1_T   tbli2;
        ISP_NVRAM_BNR_BPC_TH1_C_T   th1_c;
        ISP_NVRAM_BNR_BPC_TH2_C_T   th2_c;
        ISP_NVRAM_BNR_BPC_TH3_C_T   th3_c;
    };
    MUINT32 set[COUNT];
} ISP_NVRAM_BNR_BPC_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BNR_NR1
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CAM_BNR_NR1_CON CAM+0850H
typedef struct {
    FIELD  rsv_0                                 :  4;      /*  0.. 3, 0x0000000F */
    FIELD  NR1_CT_EN                             :  1;      /*  4.. 4, 0x00000010 */
    FIELD  rsv_5                                 : 27;      /*  5..31, 0xFFFFFFE0 */
} ISP_CAM_BNR_NR1_CON_T; // /* 0x1A004450 */

typedef union {
    typedef ISP_CAM_BNR_NR1_CON_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_NR1_CON_T;

typedef struct {
    FIELD  NR1_CT_MD                             :  2;      /*  0.. 1, 0x00000003 */
    FIELD  NR1_CT_MD2                            :  2;      /*  2.. 3, 0x0000000C */
    FIELD  NR1_CT_THRD                           : 10;      /*  4..13, 0x00003FF0 */
    FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
    FIELD  NR1_MBND                              : 10;      /* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                :  2;      /* 26..27, 0x0C000000 */
    FIELD  NR1_CT_SLOPE                          :  2;      /* 28..29, 0x30000000 */
    FIELD  NR1_CT_DIV                            :  2;      /* 30..31, 0xC0000000 */
} ISP_CAM_BNR_NR1_CT_CON_T; // /* 0x1A004454 */

typedef union {
    typedef ISP_CAM_BNR_NR1_CT_CON_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_NR1_CT_CON_T;

typedef struct {
    FIELD  NR1_CT_LWB                            : 12;      /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
    FIELD  NR1_CT_UPB                            : 12;      /* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
} ISP_CAM_BNR_NR1_CT_CON2_T; // /* 0x1A004458 */

typedef union {
    typedef ISP_CAM_BNR_NR1_CT_CON2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_NR1_CT_CON2_T;

typedef struct {
    FIELD  NR1_CT_LSP                            :  5;      /*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                 :  3;      /*  5.. 7, 0x000000E0 */
    FIELD  NR1_CT_USP                            :  5;      /*  8..12, 0x00001F00 */
    FIELD  rsv_13                                : 19;      /* 13..31, 0xFFFFE000 */
} ISP_CAM_BNR_NR1_CT_CON3_T; // /* 0x1A00445C */

typedef union {
    typedef ISP_CAM_BNR_NR1_CT_CON3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_NR1_CT_CON3_T;
//
typedef union {
    enum { COUNT = 4 };
    struct {
        ISP_NVRAM_BNR_NR1_CON_T    con;
        ISP_NVRAM_BNR_NR1_CT_CON_T ct_con;
        ISP_NVRAM_BNR_NR1_CT_CON2_T  ct_con2;
        ISP_NVRAM_BNR_NR1_CT_CON3_T  ct_con3;
    };
    MUINT32 set[COUNT];
} ISP_NVRAM_BNR_NR1_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BNR_PDC
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CAM_BNR_PDC_CON
typedef struct {
    FIELD  PDC_EN                                :  1;      /*  0.. 0, 0x00000001 */
    FIELD  rsv_1                                 :  3;      /*  1.. 3, 0x0000000E */
    FIELD  PDC_CT                                :  1;      /*  4.. 4, 0x00000010 */
    FIELD  rsv_5                                 :  3;      /*  5.. 7, 0x000000E0 */
    FIELD  PDC_MODE                              :  2;      /*  8.. 9, 0x00000300 */
    FIELD  rsv_10                                :  6;      /* 10..15, 0x0000FC00 */
    FIELD  PDC_OUT                               :  1;      /* 16..16, 0x00010000 */
    FIELD  rsv_17                                : 15;      /* 17..31, 0xFFFE0000 */
} ISP_CAM_BNR_PDC_CON_T; // /* 0x1A004460 */

typedef union {
    typedef ISP_CAM_BNR_PDC_CON_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_PDC_CON_T;

typedef struct {
    FIELD  PDC_GCF_L00                           : 12;      /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
    FIELD  PDC_GCF_L10                           : 12;      /* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
} ISP_CAM_BNR_PDC_GAIN_L0_T; // /* 0x1A004464 */

typedef union {
    typedef ISP_CAM_BNR_PDC_GAIN_L0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_PDC_GAIN_L0_T;

typedef struct {
    FIELD  PDC_GCF_L01                           : 12;      /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
    FIELD  PDC_GCF_L20                           : 12;      /* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
} ISP_CAM_BNR_PDC_GAIN_L1_T; // /* 0x1A004468 */

typedef union {
    typedef ISP_CAM_BNR_PDC_GAIN_L1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_PDC_GAIN_L1_T;

typedef struct {
    FIELD  PDC_GCF_L11                           : 12;      /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
    FIELD  PDC_GCF_L02                           : 12;      /* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
} ISP_CAM_BNR_PDC_GAIN_L2_T; // /* 0x1A00446C */

typedef union {
    typedef ISP_CAM_BNR_PDC_GAIN_L2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_PDC_GAIN_L2_T;

typedef struct {
    FIELD  PDC_GCF_L30                           : 12;      /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
    FIELD  PDC_GCF_L21                           : 12;      /* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
} ISP_CAM_BNR_PDC_GAIN_L3_T; // /* 0x1A004470 */

typedef union {
    typedef ISP_CAM_BNR_PDC_GAIN_L3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_PDC_GAIN_L3_T;

typedef struct {
    FIELD  PDC_GCF_L12                           : 12;      /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
    FIELD  PDC_GCF_L03                           : 12;      /* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
} ISP_CAM_BNR_PDC_GAIN_L4_T; // /* 0x1A004474 */

typedef union {
    typedef ISP_CAM_BNR_PDC_GAIN_L4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_PDC_GAIN_L4_T;

typedef struct {
    FIELD  PDC_GCF_R00                           : 12;      /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
    FIELD  PDC_GCF_R10                           : 12;      /* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
} ISP_CAM_BNR_PDC_GAIN_R0_T; // /* 0x1A004478 */

typedef union {
    typedef ISP_CAM_BNR_PDC_GAIN_R0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_PDC_GAIN_R0_T;

typedef struct {
    FIELD  PDC_GCF_R01                           : 12;      /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
    FIELD  PDC_GCF_R20                           : 12;      /* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
} ISP_CAM_BNR_PDC_GAIN_R1_T; // /* 0x1A00447C */

typedef union {
    typedef ISP_CAM_BNR_PDC_GAIN_R1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_PDC_GAIN_R1_T;

typedef struct {
    FIELD  PDC_GCF_R11                           : 12;      /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
    FIELD  PDC_GCF_R02                           : 12;      /* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
} ISP_CAM_BNR_PDC_GAIN_R2_T; // /* 0x1A004480 */

typedef union {
    typedef ISP_CAM_BNR_PDC_GAIN_R2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_PDC_GAIN_R2_T;

typedef struct {
    FIELD  PDC_GCF_R30                           : 12;      /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
    FIELD  PDC_GCF_R21                           : 12;      /* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
} ISP_CAM_BNR_PDC_GAIN_R3_T; // /* 0x1A004484 */

typedef union {
    typedef ISP_CAM_BNR_PDC_GAIN_R3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_PDC_GAIN_R3_T;

typedef struct {
    FIELD  PDC_GCF_R12                           : 12;      /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
    FIELD  PDC_GCF_R03                           : 12;      /* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
} ISP_CAM_BNR_PDC_GAIN_R4_T; // /* 0x1A004488 */

typedef union {
    typedef ISP_CAM_BNR_PDC_GAIN_R4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_PDC_GAIN_R4_T;

typedef struct {
    FIELD  PDC_GTH                               : 12;      /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
    FIELD  PDC_BTH                               : 12;      /* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
} ISP_CAM_BNR_PDC_TH_GB_T; // /* 0x1A00448C */

typedef union {
    typedef ISP_CAM_BNR_PDC_TH_GB_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_PDC_TH_GB_T;

typedef struct {
    FIELD  PDC_ITH                               : 12;      /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
    FIELD  PDC_ATH                               : 12;      /* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
} ISP_CAM_BNR_PDC_TH_IA_T; // /* 0x1A004490 */

typedef union {
    typedef ISP_CAM_BNR_PDC_TH_IA_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_PDC_TH_IA_T;

typedef struct {
    FIELD  PDC_NTH                               : 12;      /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
    FIELD  PDC_DTH                               : 12;      /* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
} ISP_CAM_BNR_PDC_TH_HD_T; // /* 0x1A004494 */

typedef union {
    typedef ISP_CAM_BNR_PDC_TH_HD_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_PDC_TH_HD_T;

typedef struct {
    FIELD  PDC_GSL                               :  4;      /*  0.. 3, 0x0000000F */
    FIELD  PDC_BSL                               :  4;      /*  4.. 7, 0x000000F0 */
    FIELD  PDC_ISL                               :  4;      /*  8..11, 0x00000F00 */
    FIELD  PDC_ASL                               :  4;      /* 12..15, 0x0000F000 */
    FIELD  PDC_GCF_NORM                          :  4;      /* 16..19, 0x000F0000 */
    FIELD  rsv_20                                : 12;      /* 20..31, 0xFFF00000 */
} ISP_CAM_BNR_PDC_SL_T; // /* 0x1A004498 */

typedef union {
    typedef ISP_CAM_BNR_PDC_SL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_PDC_SL_T;

typedef struct {
    FIELD  PDC_XCENTER                           : 14;      /*  0..13, 0x00003FFF */
    FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
    FIELD  PDC_YCENTER                           : 14;      /* 16..29, 0x3FFF0000 */
    FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
} ISP_CAM_BNR_PDC_POS_T; // /* 0x1A00449C */

typedef union {
    typedef ISP_CAM_BNR_PDC_POS_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_PDC_POS_T;


//
typedef union {
    enum { COUNT = 15 };
    struct {
        ISP_NVRAM_BNR_PDC_CON_T      con;
        ISP_NVRAM_BNR_PDC_GAIN_L0_T  gain_l0;
        ISP_NVRAM_BNR_PDC_GAIN_L1_T  gain_l1;
        ISP_NVRAM_BNR_PDC_GAIN_L2_T  gain_l2;
        ISP_NVRAM_BNR_PDC_GAIN_L3_T  gain_l3;
        ISP_NVRAM_BNR_PDC_GAIN_L4_T  gain_l4;
        ISP_NVRAM_BNR_PDC_GAIN_R0_T  gain_r0;
        ISP_NVRAM_BNR_PDC_GAIN_R1_T  gain_r1;
        ISP_NVRAM_BNR_PDC_GAIN_R2_T  gain_r2;
        ISP_NVRAM_BNR_PDC_GAIN_R3_T  gain_r3;
        ISP_NVRAM_BNR_PDC_GAIN_R4_T  gain_r4;
        ISP_NVRAM_BNR_PDC_TH_GB_T    th_gb;
        ISP_NVRAM_BNR_PDC_TH_IA_T    th_ia;
        ISP_NVRAM_BNR_PDC_TH_HD_T    th_hd;
        ISP_NVRAM_BNR_PDC_SL_T       sl;
        //ISP_NVRAM_BNR_PDC_POS_T      pos;
    };
    MUINT32 set[COUNT];
} ISP_NVRAM_BNR_PDC_T;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// RMM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CAM_RMM_OSC CAM+0830H
typedef struct {
    FIELD  RMM_OSC_TH                            : 12;      /*  0..11, 0x00000FFF */
    FIELD  RMM_SEDIR_SL                          :  3;      /* 12..14, 0x00007000 */
    FIELD  rsv_15                                :  1;      /* 15..15, 0x00008000 */
    FIELD  RMM_SEBLD_WD                          :  4;      /* 16..19, 0x000F0000 */
    FIELD  RMM_LEBLD_WD                          :  4;      /* 20..23, 0x00F00000 */
    FIELD  RMM_LE_INV_CTL                        :  4;      /* 24..27, 0x0F000000 */
    FIELD  RMM_EDGE                              :  4;      /* 28..31, 0xF0000000 */
} ISP_CAM_RMM_OSC_T; // /* 0x1A004390 */

typedef union {
    typedef ISP_CAM_RMM_OSC_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_RMM_OSC_T;

typedef struct {
    FIELD  RMM_MO_EDGE                           :  1;      /*  0.. 0, 0x00000001 */
    FIELD  rsv_1                                 :  1;      /*  1.. 1, 0x00000002 */
    FIELD  RMM_MO_EN                             :  1;      /*  2.. 2, 0x00000004 */
    FIELD  rsv_3                                 :  1;      /*  3.. 3, 0x00000008 */
    FIELD  RMM_MOBLD_FT                          :  3;      /*  4.. 6, 0x00000070 */
    FIELD  rsv_7                                 :  1;      /*  7.. 7, 0x00000080 */
    FIELD  RMM_MOTH_RATIO                        :  5;      /*  8..12, 0x00001F00 */
    FIELD  rsv_13                                :  3;      /* 13..15, 0x0000E000 */
    FIELD  RMM_HORI_ADDWT                        :  5;      /* 16..20, 0x001F0000 */
    FIELD  rsv_21                                : 11;      /* 21..31, 0xFFE00000 */
} ISP_CAM_RMM_MC_T; // /* 0x1A004394 */

typedef union {
    typedef ISP_CAM_RMM_MC_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_RMM_MC_T;

typedef struct {
    FIELD  RMM_REVG_R                            : 11;      /*  0..10, 0x000007FF */
    FIELD  rsv_11                                :  5;      /* 11..15, 0x0000F800 */
    FIELD  RMM_REVG_GR                           : 11;      /* 16..26, 0x07FF0000 */
    FIELD  rsv_27                                :  5;      /* 27..31, 0xF8000000 */
} ISP_CAM_RMM_REVG_1_T; // /* 0x1A004398 */

typedef union {
    typedef ISP_CAM_RMM_REVG_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_RMM_REVG_1_T;

typedef struct {
    FIELD  RMM_REVG_B                            : 11;      /*  0..10, 0x000007FF */
    FIELD  rsv_11                                :  5;      /* 11..15, 0x0000F800 */
    FIELD  RMM_REVG_GB                           : 11;      /* 16..26, 0x07FF0000 */
    FIELD  rsv_27                                :  5;      /* 27..31, 0xF8000000 */
} ISP_CAM_RMM_REVG_2_T; // /* 0x1A00439C */

typedef union {
    typedef ISP_CAM_RMM_REVG_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_RMM_REVG_2_T;

typedef struct {
    FIELD  RMM_LEOS_GRAY                         : 12;      /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                : 20;      /* 12..31, 0xFFFFF000 */
} ISP_CAM_RMM_LEOS_T; // /* 0x1A0043A0 */

typedef union {
    typedef ISP_CAM_RMM_LEOS_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_RMM_LEOS_T;

typedef struct {
    FIELD  RMM_MOSE_TH                           : 12;      /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
    FIELD  RMM_MOSE_BLDWD                        :  4;      /* 16..19, 0x000F0000 */
    FIELD  rsv_20                                : 12;      /* 20..31, 0xFFF00000 */
} ISP_CAM_RMM_MC2_T; // /* 0x1A0043A4 */

typedef union {
    typedef ISP_CAM_RMM_MC2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_RMM_MC2_T;

typedef struct {
    FIELD  RMM_DIFF_LB                           : 11;      /*  0..10, 0x000007FF */
    FIELD  rsv_11                                : 21;      /* 11..31, 0xFFFFF800 */
} ISP_CAM_RMM_DIFF_LB_T; // /* 0x1A0043A8 */

typedef union {
    typedef ISP_CAM_RMM_DIFF_LB_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_RMM_DIFF_LB_T;

typedef struct {
    FIELD  RMM_MASE_RATIO                        :  5;      /*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                 :  3;      /*  5.. 7, 0x000000E0 */
    FIELD  RMM_OSBLD_WD                          :  4;      /*  8..11, 0x00000F00 */
    FIELD  RMM_MASE_BLDWD                        :  4;      /* 12..15, 0x0000F000 */
    FIELD  RMM_SENOS_LEFAC                       :  5;      /* 16..20, 0x001F0000 */
    FIELD  rsv_21                                :  3;      /* 21..23, 0x00E00000 */
    FIELD  RMM_SEYOS_LEFAC                       :  5;      /* 24..28, 0x1F000000 */
    FIELD  rsv_29                                :  3;      /* 29..31, 0xE0000000 */
} ISP_CAM_RMM_MA_T; // /* 0x1A0043AC */

typedef union {
    typedef ISP_CAM_RMM_MA_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_RMM_MA_T;

typedef struct {
    FIELD  RMM_PSHOR_SEEN                        :  1;      /*  0.. 0, 0x00000001 */
    FIELD  RMM_PS_BLUR                           :  1;      /*  1.. 1, 0x00000002 */
    FIELD  rsv_2                                 :  2;      /*  2.. 3, 0x0000000C */
    FIELD  RMM_OSC_REPEN                         :  1;      /*  4.. 4, 0x00000010 */
    FIELD  RMM_SOFT_TH_EN                        :  1;      /*  5.. 5, 0x00000020 */
    FIELD  RMM_LE_LOWPA_EN                       :  1;      /*  6.. 6, 0x00000040 */
    FIELD  RMM_SE_LOWPA_EN                       :  1;      /*  7.. 7, 0x00000080 */
    FIELD  RMM_PSSEC_ONLY                        :  1;      /*  8.. 8, 0x00000100 */
    FIELD  rsv_9                                 :  3;      /*  9..11, 0x00000E00 */
    FIELD  RMM_OSCLE_ONLY                        :  1;      /* 12..12, 0x00001000 */
    FIELD  rsv_13                                :  3;      /* 13..15, 0x0000E000 */
    FIELD  RMM_PS_TH                             : 12;      /* 16..27, 0x0FFF0000 */
    FIELD  RMM_MOLE_DIREN                        :  1;      /* 28..28, 0x10000000 */
    FIELD  RMM_MOSE_DIREN                        :  1;      /* 29..29, 0x20000000 */
    FIELD  RMM_MO_2DBLD_EN                       :  1;      /* 30..30, 0x40000000 */
    FIELD  rsv_31                                :  1;      /* 31..31, 0x80000000 */
} ISP_CAM_RMM_TUNE_T; // /* 0x1A0043B0 */

typedef union {
    typedef ISP_CAM_RMM_TUNE_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_RMM_TUNE_T;

// CAM_RMM_IDX virtual (not in HW)
typedef struct {
        FIELD RMM_OSC_IDX               :  8;
        FIELD RMM_PS_IDX                :  8;
        FIELD RMM_MOSE_IDX              :  8;
        FIELD rsv_25                    :  8;
} ISP_CAM_RMM_IDX_T;

typedef union {
    typedef ISP_CAM_RMM_IDX_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_RMM_IDX_T;

//
typedef union {
    enum { COUNT = 6 };
    struct {
        ISP_NVRAM_RMM_OSC_T        osc;
        ISP_NVRAM_RMM_MC_T         mc;
        //ISP_NVRAM_RMM_REVG_1_T     revg_1;
        //ISP_NVRAM_RMM_REVG_2_T     revg_2;
        //ISP_NVRAM_RMM_LEOS_T       leos;
        ISP_NVRAM_RMM_MC2_T        mc2;
        //ISP_NVRAM_RMM_DIFF_LB_T    diff_lb;
        ISP_NVRAM_RMM_MA_T         ma;
        ISP_NVRAM_RMM_TUNE_T       tune;
        ISP_NVRAM_RMM_IDX_T        ridx; //SW parameters
    };
    MUINT32 set[COUNT];
} ISP_NVRAM_RMM_T;

typedef union {
    enum { COUNT = 10 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_RMM_OSC_T        osc;
        ISP_NVRAM_RMM_MC_T         mc;
        ISP_NVRAM_RMM_REVG_1_T     revg_1;
        ISP_NVRAM_RMM_REVG_2_T     revg_2;
        ISP_NVRAM_RMM_LEOS_T       leos;
        ISP_NVRAM_RMM_MC2_T        mc2;
        ISP_NVRAM_RMM_DIFF_LB_T    diff_lb;
        ISP_NVRAM_RMM_MA_T         ma;
        ISP_NVRAM_RMM_TUNE_T       tune;
        ISP_NVRAM_RMM_IDX_T        ridx; //SW parameters
    };
} ISP_NVRAM_RMM_GET_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CPN
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef struct {
    FIELD  CPN_HDR_EN                            :  1;      /*  0.. 0, 0x00000001 */
    FIELD  CPN_AALLE_EN                          :  1;      /*  1.. 1, 0x00000002 */
    FIELD  CPN_PSSE_EN                           :  1;      /*  2.. 2, 0x00000004 */
    FIELD  rsv_3                                 :  1;      /*  3.. 3, 0x00000008 */
    FIELD  CPN_EDGE_INFO                         :  4;      /*  4.. 7, 0x000000F0 */
    FIELD  rsv_8                                 : 24;      /*  8..31, 0xFFFFFF00 */
} ISP_CAM_CPN_HDR_CTL_EN_T; // /* 0x1A004DF0 */

typedef union {
    typedef ISP_CAM_CPN_HDR_CTL_EN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CPN_HDR_CTL_EN_T;

typedef struct {
    FIELD  CPN_IN_IMG_H                          : 16;      /*  0..15, 0x0000FFFF */
    FIELD  CPN_IN_IMG_W                          : 16;      /* 16..31, 0xFFFF0000 */
} ISP_CAM_CPN_IN_IMG_SIZE_T; // /* 0x1A004DF4 */

typedef union {
    typedef ISP_CAM_CPN_IN_IMG_SIZE_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CPN_IN_IMG_SIZE_T;

typedef struct {
    FIELD  CPN_HDR_RATIO                         :  9;      /*  0.. 8, 0x000001FF */
    FIELD  rsv_9                                 :  7;      /*  9..15, 0x0000FE00 */
    FIELD  CPN_HDR_GAIN                          :  9;      /* 16..24, 0x01FF0000 */
    FIELD  rsv_25                                :  7;      /* 25..31, 0xFE000000 */
} ISP_CAM_CPN_ALGO_PARAM1_T; // /* 0x1A004DF8 */

typedef union {
    typedef ISP_CAM_CPN_ALGO_PARAM1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CPN_ALGO_PARAM1_T;

typedef struct {
    FIELD  CPN_HDR_TH_K                          : 12;      /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
    FIELD  CPN_HDR_TH_T                          : 11;      /* 16..26, 0x07FF0000 */
    FIELD  rsv_27                                :  5;      /* 27..31, 0xF8000000 */
} ISP_CAM_CPN_ALGO_PARAM2_T; // /* 0x1A004DFC */

typedef union {
    typedef ISP_CAM_CPN_ALGO_PARAM2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CPN_ALGO_PARAM2_T;

typedef struct {
    FIELD  CPN_GTM_X0                            : 12;      /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
    FIELD  CPN_GTM_X1                            : 12;      /* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
} ISP_CAM_CPN_GTM_X0X1_T; // /* 0x1A004E00 */

typedef union {
    typedef ISP_CAM_CPN_GTM_X0X1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CPN_GTM_X0X1_T;

typedef struct {
    FIELD  CPN_GTM_X2                            : 12;      /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
    FIELD  CPN_GTM_X3                            : 12;      /* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
} ISP_CAM_CPN_GTM_X2X3_T; // /* 0x1A004E04 */

typedef union {
    typedef ISP_CAM_CPN_GTM_X2X3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CPN_GTM_X2X3_T;

typedef struct {
    FIELD  CPN_GTM_X4                            : 12;      /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
    FIELD  CPN_GTM_X5                            : 12;      /* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
} ISP_CAM_CPN_GTM_X4X5_T; // /* 0x1A004E08 */

typedef union {
    typedef ISP_CAM_CPN_GTM_X4X5_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CPN_GTM_X4X5_T;

typedef struct {
    FIELD  CPN_GTM_X6                            : 12;      /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                : 20;      /* 12..31, 0xFFFFF000 */
} ISP_CAM_CPN_GTM_X6_T; // /* 0x1A004E0C */

typedef union {
    typedef ISP_CAM_CPN_GTM_X6_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CPN_GTM_X6_T;

typedef struct {
    FIELD  CPN_GTM_Y0                            : 12;      /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
    FIELD  CPN_GTM_Y1                            : 12;      /* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
} ISP_CAM_CPN_GTM_Y0Y1_T; // /* 0x1A004E10 */

typedef union {
    typedef ISP_CAM_CPN_GTM_Y0Y1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CPN_GTM_Y0Y1_T;

typedef struct {
    FIELD  CPN_GTM_Y2                            : 12;      /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
    FIELD  CPN_GTM_Y3                            : 12;      /* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
} ISP_CAM_CPN_GTM_Y2Y3_T; // /* 0x1A004E14 */

typedef union {
    typedef ISP_CAM_CPN_GTM_Y2Y3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CPN_GTM_Y2Y3_T;

typedef struct {
    FIELD  CPN_GTM_Y4                            : 12;      /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
    FIELD  CPN_GTM_Y5                            : 12;      /* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
} ISP_CAM_CPN_GTM_Y4Y5_T; // /* 0x1A004E18 */

typedef union {
    typedef ISP_CAM_CPN_GTM_Y4Y5_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CPN_GTM_Y4Y5_T;

typedef struct {
    FIELD  CPN_GTM_Y6                            : 12;      /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                : 20;      /* 12..31, 0xFFFFF000 */
} ISP_CAM_CPN_GTM_Y6_T; // /* 0x1A004E1C */

typedef union {
    typedef ISP_CAM_CPN_GTM_Y6_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CPN_GTM_Y6_T;

typedef struct {
    FIELD  CPN_GTM_S0                            : 12;      /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
    FIELD  CPN_GTM_S1                            : 12;      /* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
} ISP_CAM_CPN_GTM_S0S1_T; // /* 0x1A004E20 */

typedef union {
    typedef ISP_CAM_CPN_GTM_S0S1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CPN_GTM_S0S1_T;

typedef struct {
    FIELD  CPN_GTM_S2                            : 12;      /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
    FIELD  CPN_GTM_S3                            : 12;      /* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
} ISP_CAM_CPN_GTM_S2S3_T; // /* 0x1A004E24 */

typedef union {
    typedef ISP_CAM_CPN_GTM_S2S3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CPN_GTM_S2S3_T;

typedef struct {
    FIELD  CPN_GTM_S4                            : 12;      /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
    FIELD  CPN_GTM_S5                            : 12;      /* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
} ISP_CAM_CPN_GTM_S4S5_T; // /* 0x1A004E28 */

typedef union {
    typedef ISP_CAM_CPN_GTM_S4S5_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CPN_GTM_S4S5_T;

typedef struct {
    FIELD  CPN_GTM_S6                            : 12;      /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
    FIELD  CPN_GTM_S7                            : 12;      /* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
} ISP_CAM_CPN_GTM_S6S7_T; // /* 0x1A004E2C */

typedef union {
    typedef ISP_CAM_CPN_GTM_S6S7_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CPN_GTM_S6S7_T;

typedef union {
    enum { COUNT = 15 };
    struct {
        ISP_NVRAM_CPN_HDR_CTL_EN_T  hdr_ctl_en;
        //ISP_NVRAM_CPN_IN_IMG_SIZE_T  in_img_size;
        ISP_NVRAM_CPN_ALGO_PARAM1_T  algo_param1;
        ISP_NVRAM_CPN_ALGO_PARAM2_T  algo_param2;
        ISP_NVRAM_CPN_GTM_X0X1_T  x0x1;
        ISP_NVRAM_CPN_GTM_X2X3_T  x2x3;
        ISP_NVRAM_CPN_GTM_X4X5_T  x4x5;
        ISP_NVRAM_CPN_GTM_X6_T  x6;
        ISP_NVRAM_CPN_GTM_Y0Y1_T  y0y1;
        ISP_NVRAM_CPN_GTM_Y2Y3_T  y2y3;
        ISP_NVRAM_CPN_GTM_Y4Y5_T  y4y5;
        ISP_NVRAM_CPN_GTM_Y6_T  y6;
        ISP_NVRAM_CPN_GTM_S0S1_T  s0s1;
        ISP_NVRAM_CPN_GTM_S2S3_T  s2s3;
        ISP_NVRAM_CPN_GTM_S4S5_T  s4s5;
        ISP_NVRAM_CPN_GTM_S6S7_T  s6s7;
    };
    MUINT32 set[COUNT];
} ISP_NVRAM_CPN_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// DCPN
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct {
    FIELD  DCPN_HDR_EN                           :  1;      /*  0.. 0, 0x00000001 */
    FIELD  rsv_1                                 :  3;      /*  1.. 3, 0x0000000E */
    FIELD  DCPN_EDGE_INFO                        :  4;      /*  4.. 7, 0x000000F0 */
    FIELD  rsv_8                                 : 24;      /*  8..31, 0xFFFFFF00 */
} ISP_CAM_DCPN_HDR_EN_T; // /* 0x1A004DB0 */

typedef union {
    typedef ISP_CAM_DCPN_HDR_EN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DCPN_HDR_EN_T;

typedef struct {
    FIELD  DCPN_IN_IMG_H                         : 16;      /*  0..15, 0x0000FFFF */
    FIELD  DCPN_IN_IMG_W                         : 16;      /* 16..31, 0xFFFF0000 */
} ISP_CAM_DCPN_IN_IMG_SIZE_T; // /* 0x1A004DB4 */

typedef union {
    typedef ISP_CAM_DCPN_IN_IMG_SIZE_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DCPN_IN_IMG_SIZE_T;

typedef struct {
    FIELD  DCPN_HDR_RATIO                        :  9;      /*  0.. 8, 0x000001FF */
    FIELD  rsv_9                                 :  7;      /*  9..15, 0x0000FE00 */
    FIELD  DCPN_HDR_GAIN                         :  9;      /* 16..24, 0x01FF0000 */
    FIELD  rsv_25                                :  7;      /* 25..31, 0xFE000000 */
} ISP_CAM_DCPN_ALGO_PARAM1_T; // /* 0x1A004DB8 */

typedef union {
    typedef ISP_CAM_DCPN_ALGO_PARAM1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DCPN_ALGO_PARAM1_T;

typedef struct {
    FIELD  DCPN_HDR_TH_K                         : 12;      /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
    FIELD  DCPN_HDR_TH_T                         : 11;      /* 16..26, 0x07FF0000 */
    FIELD  rsv_27                                :  5;      /* 27..31, 0xF8000000 */
} ISP_CAM_DCPN_ALGO_PARAM2_T; // /* 0x1A004DBC */

typedef union {
    typedef ISP_CAM_DCPN_ALGO_PARAM2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DCPN_ALGO_PARAM2_T;

typedef struct {
    FIELD  DCPN_GTM_X0                           : 12;      /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                : 20;      /* 12..31, 0xFFFFF000 */
} ISP_CAM_DCPN_GTM_X0_T; // /* 0x1A004DC0 */

typedef union {
    typedef ISP_CAM_DCPN_GTM_X0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DCPN_GTM_X0_T;

typedef struct {
    FIELD  DCPN_GTM_Y0                           : 16;      /*  0..15, 0x0000FFFF */
    FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
} ISP_CAM_DCPN_GTM_Y0_T; // /* 0x1A004DC4 */

typedef union {
    typedef ISP_CAM_DCPN_GTM_Y0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DCPN_GTM_Y0_T;

typedef struct {
    FIELD  DCPN_GTM_S0                           : 20;      /*  0..19, 0x000FFFFF */
    FIELD  rsv_20                                : 12;      /* 20..31, 0xFFF00000 */
} ISP_CAM_DCPN_GTM_S0_T; // /* 0x1A004DC8 */

typedef union {
    typedef ISP_CAM_DCPN_GTM_S0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DCPN_GTM_S0_T;

typedef struct {
    FIELD  DCPN_GTM_S1                           : 20;      /*  0..19, 0x000FFFFF */
    FIELD  rsv_20                                : 12;      /* 20..31, 0xFFF00000 */
} ISP_CAM_DCPN_GTM_S1_T; // /* 0x1A004DCC */

typedef union {
    typedef ISP_CAM_DCPN_GTM_S1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DCPN_GTM_S1_T;

typedef union {
    enum { COUNT = 7 };
    struct {
        ISP_NVRAM_DCPN_HDR_EN_T  hdr_en;
        //ISP_NVRAM_DCPN_IN_IMG_SIZE_T  in_ing_size;
        ISP_NVRAM_DCPN_ALGO_PARAM1_T algo_param1;
        ISP_NVRAM_DCPN_ALGO_PARAM2_T  algo_param2;
        ISP_NVRAM_DCPN_GTM_X0_T  x0;
        ISP_NVRAM_DCPN_GTM_Y0_T  y0;
        ISP_NVRAM_DCPN_GTM_S0_T  s0;
        ISP_NVRAM_DCPN_GTM_S1_T  s1;
    };
    MUINT32 set[COUNT];
} ISP_NVRAM_DCPN_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// LSC
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


// CAM_LSC_CTL1 CAM+0530H
typedef struct {
    FIELD SDBLK_YOFST               : 6;
    FIELD rsv_6                     : 10;
    FIELD SDBLK_XOFST               : 6;
    FIELD LSC_EXTEND_COEF_MODE     : 1;
    FIELD rsv_23                    : 1;
    FIELD SD_COEFRD_MODE            : 1;
    FIELD rsv_25                    : 3;
    FIELD SD_ULTRA_MODE             : 1;
    FIELD LSC_PRC_MODE              : 1;
    FIELD rsv_30                    : 2;
} ISP_CAM_LSC_CTL1_T;

typedef union {
    typedef ISP_CAM_LSC_CTL1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LSC_CTL1_T;

// CAM_LSC_CTL2 CAM+0534H
typedef struct {
    FIELD LSC_SDBLK_WIDTH           : 13;
    FIELD LSC_SDBLK_XNUM            : 5;
    FIELD LSC_OFLN                  : 1;
    FIELD rsv_19                    : 13;
} ISP_CAM_LSC_CTL2_T;

typedef union {
    typedef ISP_CAM_LSC_CTL2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LSC_CTL2_T;

// CAM_LSC_CTL3 CAM+0538H
typedef struct {
    FIELD LSC_SDBLK_HEIGHT          : 13;
    FIELD LSC_SDBLK_YNUM            : 5;
    FIELD LSC_SPARE                 : 14;
} ISP_CAM_LSC_CTL3_T;

typedef union {
    typedef ISP_CAM_LSC_CTL3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LSC_CTL3_T;

// CAM_LSC_LBLOCK CAM+053CH
typedef struct {
    FIELD LSC_SDBLK_lHEIGHT         : 13;
    FIELD rsv_13                    : 3;
    FIELD LSC_SDBLK_lWIDTH          : 13;
    FIELD rsv_29                    : 3;
} ISP_CAM_LSC_LBLOCK_T;

typedef union {
    typedef ISP_CAM_LSC_LBLOCK_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LSC_LBLOCK_T;

// CAM_LSC_RATIO CAM+0540H
typedef struct {
    FIELD LSC_RA03                  : 6;
    FIELD rsv_6                     : 2;
    FIELD LSC_RA02                  : 6;
    FIELD rsv_14                    : 2;
    FIELD LSC_RA01                  : 6;
    FIELD rsv_22                    : 2;
    FIELD LSC_RA00                  : 6;
    FIELD rsv_30                    : 2;
} ISP_CAM_LSC_RATIO_0_T;

typedef union {
    typedef ISP_CAM_LSC_RATIO_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LSC_RATIO_0_T;


// CAM_LSC_TPIPE_OFST CAM+0540H
typedef struct {
    FIELD  LSC_TPIPE_OFST_Y         : 13;
    FIELD  rsv_13                   : 3;
    FIELD  LSC_TPIPE_OFST_X         : 13;
    FIELD  rsv_29                   : 3;
} ISP_CAM_LSC_TPIPE_OFST_T;

typedef union {
    typedef ISP_CAM_LSC_TPIPE_OFST_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LSC_TPIPE_OFST_T;

// CAM_LSC_TPIPE_SIZE CAM+0540H
typedef struct {
    FIELD  LSC_TPIPE_SIZE_Y         : 14;
    FIELD  rsv_14                   : 2;
    FIELD  LSC_TPIPE_SIZE_X         : 14;
    FIELD  rsv_30                   : 2;
} ISP_CAM_LSC_TPIPE_SIZE_T;

typedef union {
    typedef ISP_CAM_LSC_TPIPE_SIZE_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LSC_TPIPE_SIZE_T;


// CAM_LSC_GAIN_TH CAM+054CH
typedef struct {
    FIELD LSC_GAIN_TH2              : 9;
    FIELD rsv_9                     : 1;
    FIELD LSC_GAIN_TH1              : 9;
    FIELD rsv_19                    : 1;
    FIELD LSC_GAIN_TH0              : 9;
    FIELD rsv_29                    : 3;
} ISP_CAM_LSC_GAIN_TH_T;

typedef union {
    typedef ISP_CAM_LSC_GAIN_TH_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LSC_GAIN_TH_T;

// CAM_LSC_RATIO CAM+0540H
typedef struct {
    FIELD LSC_RA13                  : 6;
    FIELD rsv_6                     : 2;
    FIELD LSC_RA12                  : 6;
    FIELD rsv_14                    : 2;
    FIELD LSC_RA11                  : 6;
    FIELD rsv_22                    : 2;
    FIELD LSC_RA10                  : 6;
    FIELD rsv_30                    : 2;
} ISP_CAM_LSC_RATIO_1_T;

typedef union {
    typedef ISP_CAM_LSC_RATIO_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LSC_RATIO_1_T;

typedef union {
    enum { COUNT = 6 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_LSC_CTL1_T    ctl1;
        ISP_NVRAM_LSC_CTL2_T    ctl2;
        ISP_NVRAM_LSC_CTL3_T    ctl3;
        ISP_NVRAM_LSC_LBLOCK_T  lblock;
        ISP_NVRAM_LSC_RATIO_0_T   ratio_0;
        ISP_NVRAM_LSC_RATIO_1_T   ratio_1;
    };
} ISP_NVRAM_LSC_T;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// RPG
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CAM_RPG_SATU_1 CAM+0550H
typedef struct {
        FIELD RPG_SATU_B                : 12;
        FIELD rsv_12                    : 4;
        FIELD RPG_SATU_GB               : 12;
        FIELD rsv_28                    : 4;
} ISP_CAM_RPG_SATU_1_T;

typedef union {
    enum { MASK     = 0x0FFF0FFF };
    enum { DEFAULT  = 0x0FFF0FFF };
    typedef ISP_CAM_RPG_SATU_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_RPG_SATU_1_T;

// CAM_RPG_SATU_2 CAM+0554H
typedef struct {
        FIELD RPG_SATU_GR               : 12;
        FIELD rsv_12                    : 4;
        FIELD RPG_SATU_R                : 12;
        FIELD rsv_28                    : 4;
} ISP_CAM_RPG_SATU_2_T;

typedef union {
    enum { MASK     = 0x0FFF0FFF };
    enum { DEFAULT  = 0x0FFF0FFF };
    typedef ISP_CAM_RPG_SATU_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_RPG_SATU_2_T;

// CAM_RPG_GAIN_1 CAM+0558H
typedef struct {
        FIELD RPG_GAIN_B                : 13;
        FIELD rsv_13                    : 3;
        FIELD RPG_GAIN_GB               : 13;
        FIELD rsv_29                    : 3;
} ISP_CAM_RPG_GAIN_1_T;

typedef union {
    enum { MASK     = 0x1FFF1FFF };
    enum { DEFAULT  = 0x02000200 };
    typedef ISP_CAM_RPG_GAIN_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_RPG_GAIN_1_T;

// CAM_RPG_GAIN_2 CAM+055CH
typedef struct {
        FIELD RPG_GAIN_GR               : 13;
        FIELD rsv_13                    : 3;
        FIELD RPG_GAIN_R                : 13;
        FIELD rsv_29                    : 3;
} ISP_CAM_RPG_GAIN_2_T;

typedef union {
    enum { MASK     = 0x1FFF1FFF };
    enum { DEFAULT  = 0x02000200 };
    typedef ISP_CAM_RPG_GAIN_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_RPG_GAIN_2_T;

// CAM_RPG_OFST_1 CAM+0560H
typedef struct {
        FIELD RPG_OFST_B                : 12;
        FIELD rsv_12                    : 4;
        FIELD RPG_OFST_GB               : 12;
        FIELD rsv_28                    : 4;
} ISP_CAM_RPG_OFST_1_T;

typedef union {
    enum { MASK     = 0x0FFF0FFF };
    enum { DEFAULT  = 0x00000000 };
    typedef ISP_CAM_RPG_OFST_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_RPG_OFST_1_T;

// CAM_RPG_OFST_2 CAM+0564H
typedef struct {
        FIELD RPG_OFST_GR               : 12;
        FIELD rsv_12                    : 4;
        FIELD RPG_OFST_R                : 12;
        FIELD rsv_28                    : 4;
} ISP_CAM_RPG_OFST_2_T;

typedef union {
    enum { MASK     = 0x0FFF0FFF };
    enum { DEFAULT  = 0x00000000 };
    typedef ISP_CAM_RPG_OFST_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_RPG_OFST_2_T;

//
typedef union {
    enum { COUNT = 6 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_RPG_SATU_1_T   satu_1;
        ISP_NVRAM_RPG_SATU_2_T   satu_2;
        ISP_NVRAM_RPG_GAIN_1_T   gain_1;
        ISP_NVRAM_RPG_GAIN_2_T   gain_2;
        ISP_NVRAM_RPG_OFST_1_T   ofst_1;
        ISP_NVRAM_RPG_OFST_2_T   ofst_2;
    };
} ISP_NVRAM_RPG_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SL2
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// DIP_X_SL2_CEN CAM+0F40H
typedef struct {
    FIELD  SL2_CENTR_X                           : 14;		/*  0..13, 0x00003FFF */
    FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
    FIELD  SL2_CENTR_Y                           : 14;		/* 16..29, 0x3FFF0000 */
    FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
} ISP_DIP_X_SL2_CEN_T; //	/* 0x150235C0 */

typedef union {
    typedef ISP_DIP_X_SL2_CEN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SL2_CEN_T;

typedef struct {
    FIELD  SL2_R_0                               : 14;		/*  0..13, 0x00003FFF */
    FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
    FIELD  SL2_R_1                               : 14;		/* 16..29, 0x3FFF0000 */
    FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
} ISP_DIP_X_SL2_RR_CON0_T; //	/* 0x150235C4 */

typedef union {
    typedef ISP_DIP_X_SL2_RR_CON0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SL2_RR_CON0_T;

typedef struct {
    FIELD  SL2_R_2                               : 14;		/*  0..13, 0x00003FFF */
    FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
    FIELD  SL2_GAIN_0                            :  8;		/* 16..23, 0x00FF0000 */
    FIELD  SL2_GAIN_1                            :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_SL2_RR_CON1_T; //	/* 0x150235C8 */

typedef union {
    typedef ISP_DIP_X_SL2_RR_CON1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SL2_RR_CON1_T;

typedef struct {
    FIELD  SL2_GAIN_2                            :  8;		/*  0.. 7, 0x000000FF */
    FIELD  SL2_GAIN_3                            :  8;		/*  8..15, 0x0000FF00 */
    FIELD  SL2_GAIN_4                            :  8;		/* 16..23, 0x00FF0000 */
    FIELD  SL2_SET_ZERO                          :  1;		/* 24..24, 0x01000000 */
    FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
} ISP_DIP_X_SL2_GAIN_T; //	/* 0x150235CC */

typedef union {
    typedef ISP_DIP_X_SL2_GAIN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SL2_GAIN_T;

typedef struct {
    FIELD  SL2_HRZ_COMP                          : 14;		/*  0..13, 0x00003FFF */
    FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
    FIELD  SL2_VRZ_COMP                          : 14;		/* 16..29, 0x3FFF0000 */
    FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
} ISP_DIP_X_SL2_RZ_T; //	/* 0x150235D0 */

typedef union {
    typedef ISP_DIP_X_SL2_RZ_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SL2_RZ_T;

typedef struct {
    FIELD  SL2_X_OFST                            : 28;		/*  0..27, 0x0FFFFFFF */
    FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
} ISP_DIP_X_SL2_XOFF_T; //	/* 0x150235D4 */

typedef union {
    typedef ISP_DIP_X_SL2_XOFF_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SL2_XOFF_T;

typedef struct {
    FIELD  SL2_Y_OFST                            : 28;		/*  0..27, 0x0FFFFFFF */
    FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
} ISP_DIP_X_SL2_YOFF_T; //	/* 0x150235D8 */

typedef union {
    typedef ISP_DIP_X_SL2_YOFF_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SL2_YOFF_T;

typedef struct {
    FIELD  SL2_SLP_1                             : 24;		/*  0..23, 0x00FFFFFF */
    FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_SL2_SLP_CON0_T; //	/* 0x150235DC */

typedef union {
    typedef ISP_DIP_X_SL2_SLP_CON0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SL2_SLP_CON0_T;

typedef struct {
    FIELD  SL2_SLP_2                             : 24;		/*  0..23, 0x00FFFFFF */
    FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_SL2_SLP_CON1_T; //	/* 0x150235E0 */

typedef union {
    typedef ISP_DIP_X_SL2_SLP_CON1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SL2_SLP_CON1_T;

typedef struct {
    FIELD  SL2_SLP_3                             : 24;		/*  0..23, 0x00FFFFFF */
    FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_SL2_SLP_CON2_T; //	/* 0x150235E4 */

typedef union {
    typedef ISP_DIP_X_SL2_SLP_CON2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SL2_SLP_CON2_T;

typedef struct {
    FIELD  SL2_SLP_4                             : 24;		/*  0..23, 0x00FFFFFF */
    FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_SL2_SLP_CON3_T; //	/* 0x150235E8 */

typedef union {
    typedef ISP_DIP_X_SL2_SLP_CON3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SL2_SLP_CON3_T;

typedef struct {
    FIELD  SL2_TPIPE_WD                          : 14;		/*  0..13, 0x00003FFF */
    FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
    FIELD  SL2_TPIPE_HT                          : 14;		/* 16..29, 0x3FFF0000 */
    FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
} ISP_DIP_X_SL2_SIZE_T; //	/* 0x150235EC */

typedef union {
    typedef ISP_DIP_X_SL2_SIZE_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SL2_SIZE_T;

//
typedef union {
    enum { COUNT = 4 };
    struct {
        ISP_NVRAM_SL2_CEN_T      cen;
		ISP_NVRAM_SL2_RR_CON0_T  rr_con0;
		ISP_NVRAM_SL2_RR_CON1_T  rr_con1;
		ISP_NVRAM_SL2_GAIN_T     gain;
    };
    MUINT32 set[COUNT];
} ISP_NVRAM_SL2_T;

typedef union {
    enum { COUNT = 12 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_SL2_CEN_T      cen;
		ISP_NVRAM_SL2_RR_CON0_T  rr_con0;
		ISP_NVRAM_SL2_RR_CON1_T  rr_con1;
		ISP_NVRAM_SL2_GAIN_T     gain;
        ISP_NVRAM_SL2_RZ_T       rz;
        ISP_NVRAM_SL2_XOFF_T     xoff;
        ISP_NVRAM_SL2_YOFF_T     yoff;
        ISP_NVRAM_SL2_SLP_CON0_T con0;
        ISP_NVRAM_SL2_SLP_CON1_T con1;
        ISP_NVRAM_SL2_SLP_CON2_T con2;
        ISP_NVRAM_SL2_SLP_CON3_T con3;
        ISP_NVRAM_SL2_SIZE_T     size;
    };
} ISP_NVRAM_SL2F_GET_T;

typedef union {
    enum { COUNT = 12 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_SL2_CEN_T      cen;
        ISP_NVRAM_SL2_RR_CON0_T  rr_con0;
        ISP_NVRAM_SL2_RR_CON1_T  rr_con1;
        ISP_NVRAM_SL2_GAIN_T     gain;
        ISP_NVRAM_SL2_RZ_T       rz;
        ISP_NVRAM_SL2_XOFF_T     xoff;
        ISP_NVRAM_SL2_YOFF_T     yoff;
        ISP_NVRAM_SL2_SLP_CON0_T con0;
        ISP_NVRAM_SL2_SLP_CON1_T con1;
        ISP_NVRAM_SL2_SLP_CON2_T con2;
        ISP_NVRAM_SL2_SLP_CON3_T con3;
        ISP_NVRAM_SL2_SIZE_T     size;
    };
} ISP_NVRAM_SL2J_GET_T;

typedef union {
    enum { COUNT = 12 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_SL2_CEN_T      cen;
        ISP_NVRAM_SL2_RR_CON0_T  rr_con0;
        ISP_NVRAM_SL2_RR_CON1_T  rr_con1;
        ISP_NVRAM_SL2_GAIN_T     gain;
        ISP_NVRAM_SL2_RZ_T       rz;
        ISP_NVRAM_SL2_XOFF_T     xoff;
        ISP_NVRAM_SL2_YOFF_T     yoff;
        ISP_NVRAM_SL2_SLP_CON0_T con0;
        ISP_NVRAM_SL2_SLP_CON1_T con1;
        ISP_NVRAM_SL2_SLP_CON2_T con2;
        ISP_NVRAM_SL2_SLP_CON3_T con3;
        ISP_NVRAM_SL2_SIZE_T     size;
    };
} ISP_NVRAM_SL2E_GET_T;


typedef union {
    enum { COUNT = 12 };
    MUINT32 set[COUNT];
    struct {
    ISP_NVRAM_SL2_CEN_T      cen;
    ISP_NVRAM_SL2_RR_CON0_T  rr_con0;
    ISP_NVRAM_SL2_RR_CON1_T  rr_con1;
    ISP_NVRAM_SL2_GAIN_T     gain;
    ISP_NVRAM_SL2_RZ_T       rz;
    ISP_NVRAM_SL2_XOFF_T     xoff;
    ISP_NVRAM_SL2_YOFF_T     yoff;
    ISP_NVRAM_SL2_SLP_CON0_T con0;
    ISP_NVRAM_SL2_SLP_CON1_T con1;
    ISP_NVRAM_SL2_SLP_CON2_T con2;
    ISP_NVRAM_SL2_SLP_CON3_T con3;
    ISP_NVRAM_SL2_SIZE_T     size;
    };
} ISP_NVRAM_SL2G_GET_T;

typedef union {
    enum { COUNT = 12 };
    MUINT32 set[COUNT];
    struct {
    ISP_NVRAM_SL2_CEN_T      cen;
    ISP_NVRAM_SL2_RR_CON0_T  rr_con0;
    ISP_NVRAM_SL2_RR_CON1_T  rr_con1;
    ISP_NVRAM_SL2_GAIN_T     gain;
    ISP_NVRAM_SL2_RZ_T       rz;
    ISP_NVRAM_SL2_XOFF_T     xoff;
    ISP_NVRAM_SL2_YOFF_T     yoff;
    ISP_NVRAM_SL2_SLP_CON0_T con0;
    ISP_NVRAM_SL2_SLP_CON1_T con1;
    ISP_NVRAM_SL2_SLP_CON2_T con2;
    ISP_NVRAM_SL2_SLP_CON3_T con3;
    ISP_NVRAM_SL2_SIZE_T     size;
    };
} ISP_NVRAM_SL2_GET_T;

typedef union {
    enum { COUNT = 12 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_SL2_CEN_T      cen;
		ISP_NVRAM_SL2_RR_CON0_T  rr_con0;
		ISP_NVRAM_SL2_RR_CON1_T  rr_con1;
		ISP_NVRAM_SL2_GAIN_T     gain;
        ISP_NVRAM_SL2_RZ_T       rz;
        ISP_NVRAM_SL2_XOFF_T     xoff;
        ISP_NVRAM_SL2_YOFF_T     yoff;
        ISP_NVRAM_SL2_SLP_CON0_T con0;
        ISP_NVRAM_SL2_SLP_CON1_T con1;
        ISP_NVRAM_SL2_SLP_CON2_T con2;
        ISP_NVRAM_SL2_SLP_CON3_T con3;
        ISP_NVRAM_SL2_SIZE_T     size;
    };
} ISP_NVRAM_SL2B_GET_T;

typedef union {
    enum { COUNT = 12 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_SL2_CEN_T      cen;
		ISP_NVRAM_SL2_RR_CON0_T  rr_con0;
		ISP_NVRAM_SL2_RR_CON1_T  rr_con1;
		ISP_NVRAM_SL2_GAIN_T     gain;
        ISP_NVRAM_SL2_RZ_T       rz;
        ISP_NVRAM_SL2_XOFF_T     xoff;
        ISP_NVRAM_SL2_YOFF_T     yoff;
        ISP_NVRAM_SL2_SLP_CON0_T con0;
        ISP_NVRAM_SL2_SLP_CON1_T con1;
        ISP_NVRAM_SL2_SLP_CON2_T con2;
        ISP_NVRAM_SL2_SLP_CON3_T con3;
        ISP_NVRAM_SL2_SIZE_T     size;
    };
} ISP_NVRAM_SL2C_GET_T;

typedef union {
    enum { COUNT = 12 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_SL2_CEN_T      cen;
		ISP_NVRAM_SL2_RR_CON0_T  rr_con0;
		ISP_NVRAM_SL2_RR_CON1_T  rr_con1;
		ISP_NVRAM_SL2_GAIN_T     gain;
        ISP_NVRAM_SL2_RZ_T       rz;
        ISP_NVRAM_SL2_XOFF_T     xoff;
        ISP_NVRAM_SL2_YOFF_T     yoff;
        ISP_NVRAM_SL2_SLP_CON0_T con0;
        ISP_NVRAM_SL2_SLP_CON1_T con1;
        ISP_NVRAM_SL2_SLP_CON2_T con2;
        ISP_NVRAM_SL2_SLP_CON3_T con3;
        ISP_NVRAM_SL2_SIZE_T     size;
    };
} ISP_NVRAM_SL2D_GET_T;

typedef union {
    enum { COUNT = 12 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_SL2_CEN_T      cen;
		ISP_NVRAM_SL2_RR_CON0_T  rr_con0;
		ISP_NVRAM_SL2_RR_CON1_T  rr_con1;
		ISP_NVRAM_SL2_GAIN_T     gain;
        ISP_NVRAM_SL2_RZ_T       rz;
        ISP_NVRAM_SL2_XOFF_T     xoff;
        ISP_NVRAM_SL2_YOFF_T     yoff;
        ISP_NVRAM_SL2_SLP_CON0_T con0;
        ISP_NVRAM_SL2_SLP_CON1_T con1;
        ISP_NVRAM_SL2_SLP_CON2_T con2;
        ISP_NVRAM_SL2_SLP_CON3_T con3;
        ISP_NVRAM_SL2_SIZE_T     size;
    };
} ISP_NVRAM_SL2H_GET_T;

typedef union {
    enum { COUNT = 12 };
    MUINT32 set[COUNT];
    struct {
    ISP_NVRAM_SL2_CEN_T      cen;
    ISP_NVRAM_SL2_RR_CON0_T  rr_con0;
    ISP_NVRAM_SL2_RR_CON1_T  rr_con1;
    ISP_NVRAM_SL2_GAIN_T     gain;
    ISP_NVRAM_SL2_RZ_T       rz;
    ISP_NVRAM_SL2_XOFF_T     xoff;
    ISP_NVRAM_SL2_YOFF_T     yoff;
    ISP_NVRAM_SL2_SLP_CON0_T con0;
    ISP_NVRAM_SL2_SLP_CON1_T con1;
    ISP_NVRAM_SL2_SLP_CON2_T con2;
    ISP_NVRAM_SL2_SLP_CON3_T con3;
    ISP_NVRAM_SL2_SIZE_T     size;
    };
} ISP_NVRAM_SL2I_GET_T;

typedef union {
    enum { COUNT = 12 };
    MUINT32 set[COUNT];
    struct {
    ISP_NVRAM_SL2_CEN_T      cen;
    ISP_NVRAM_SL2_RR_CON0_T  rr_con0;
    ISP_NVRAM_SL2_RR_CON1_T  rr_con1;
    ISP_NVRAM_SL2_GAIN_T     gain;
    ISP_NVRAM_SL2_RZ_T       rz;
    ISP_NVRAM_SL2_XOFF_T     xoff;
    ISP_NVRAM_SL2_YOFF_T     yoff;
    ISP_NVRAM_SL2_SLP_CON0_T con0;
    ISP_NVRAM_SL2_SLP_CON1_T con1;
    ISP_NVRAM_SL2_SLP_CON2_T con2;
    ISP_NVRAM_SL2_SLP_CON3_T con3;
    ISP_NVRAM_SL2_SIZE_T     size;
    };
} ISP_NVRAM_SL2K_GET_T;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// PGN
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// DIP_X_PGN_SATU_1 CAM+0880H
typedef struct {
        FIELD PGN_SATU_B                : 12;
        FIELD rsv_12                    : 4;
        FIELD PGN_SATU_GB               : 12;
        FIELD rsv_28                    : 4;
} ISP_DIP_X_PGN_SATU_1_T;

typedef union {
    enum { MASK     = 0x0FFF0FFF };
    enum { DEFAULT  = 0x0FFF0FFF };
    typedef ISP_DIP_X_PGN_SATU_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_PGN_SATU_1_T;

// DIP_X_PGN_SATU_2 CAM+0884H
typedef struct {
        FIELD PGN_SATU_GR               : 12;
        FIELD rsv_12                    : 4;
        FIELD PGN_SATU_R                : 12;
        FIELD rsv_28                    : 4;
} ISP_DIP_X_PGN_SATU_2_T;

typedef union {
    enum { MASK     = 0x0FFF0FFF };
    enum { DEFAULT  = 0x0FFF0FFF };
    typedef ISP_DIP_X_PGN_SATU_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_PGN_SATU_2_T;

// DIP_X_PGN_GAIN_1 CAM+0888H
typedef struct {
        FIELD PGN_GAIN_B                : 13;
        FIELD rsv_13                    : 3;
        FIELD PGN_GAIN_GB               : 13;
        FIELD rsv_29                    : 3;
} ISP_DIP_X_PGN_GAIN_1_T;

typedef union {
    enum { MASK     = 0x1FFF1FFF };
    enum { DEFAULT  = 0x02000200 };
    typedef ISP_DIP_X_PGN_GAIN_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_PGN_GAIN_1_T;

// DIP_X_PGN_GAIN_2 CAM+088CH
typedef struct {
        FIELD PGN_GAIN_GR               : 13;
        FIELD rsv_13                    : 3;
        FIELD PGN_GAIN_R                : 13;
        FIELD rsv_29                    : 3;
} ISP_DIP_X_PGN_GAIN_2_T;

typedef union {
    enum { MASK     = 0x1FFF1FFF };
    enum { DEFAULT  = 0x02000200 };
    typedef ISP_DIP_X_PGN_GAIN_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_PGN_GAIN_2_T;

// DIP_X_PGN_OFST_1 CAM+0890H
typedef struct {
        FIELD PGN_OFST_B                : 12;
        FIELD rsv_12                    : 4;
        FIELD PGN_OFST_GB               : 12;
        FIELD rsv_28                    : 4;
} ISP_DIP_X_PGN_OFST_1_T;

typedef union {
    enum { MASK     = 0x0FFF0FFF };
    enum { DEFAULT  = 0x00000000 };
    typedef ISP_DIP_X_PGN_OFST_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_PGN_OFST_1_T;

// DIP_X_PGN_OFST_2 CAM+0894H
typedef struct {
        FIELD PGN_OFST_GR               : 12;
        FIELD rsv_12                    : 4;
        FIELD PGN_OFST_R                : 12;
        FIELD rsv_28                    : 4;
} ISP_DIP_X_PGN_OFST_2_T;

typedef union {
    enum { MASK     = 0x0FFF0FFF };
    enum { DEFAULT  = 0x00000000 };
    typedef ISP_DIP_X_PGN_OFST_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_PGN_OFST_2_T;

//
typedef union {
    enum { COUNT = 6 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_PGN_SATU_1_T   satu_1;
        ISP_NVRAM_PGN_SATU_2_T   satu_2;
        ISP_NVRAM_PGN_GAIN_1_T   gain_1;
        ISP_NVRAM_PGN_GAIN_2_T   gain_2;
        ISP_NVRAM_PGN_OFST_1_T   ofst_1;
        ISP_NVRAM_PGN_OFST_2_T   ofst_2;
    };
} ISP_NVRAM_PGN_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//RNR
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct {
    FIELD  RNR_RAT                               :  9;    /*  0.. 8, 0x000001FF */
    FIELD  RNR_EPS                               :  8;    /*  9..16, 0x0001FE00 */
    FIELD  rsv_17                                : 15;    /* 17..31, 0xFFFE0000 */
} ISP_DIP_X_RNR_CON1_T; //  /* 0x150245B0 */

typedef union {
    typedef ISP_DIP_X_RNR_CON1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_RNR_CON1_T;

typedef struct {
    FIELD  RNR_VC0_R                             :  9;    /*  0.. 8, 0x000001FF */
    FIELD  RNR_VC0_G                             :  9;    /*  9..17, 0x0003FE00 */
    FIELD  RNR_VC0_B                             :  9;    /* 18..26, 0x07FC0000 */
    FIELD  rsv_27                                :  5;    /* 27..31, 0xF8000000 */
} ISP_DIP_X_RNR_VC0_T; // /* 0x150245B4 */

typedef union {
    typedef ISP_DIP_X_RNR_VC0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_RNR_VC0_T;

typedef struct {
    FIELD  RNR_VC1_R                             :  9;    /*  0.. 8, 0x000001FF */
    FIELD  RNR_VC1_G                             :  9;    /*  9..17, 0x0003FE00 */
    FIELD  RNR_VC1_B                             :  9;    /* 18..26, 0x07FC0000 */
    FIELD  rsv_27                                :  5;    /* 27..31, 0xF8000000 */
} ISP_DIP_X_RNR_VC1_T; // /* 0x150245B8 */

typedef union {
    typedef ISP_DIP_X_RNR_VC1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_RNR_VC1_T;

typedef struct {
    FIELD  RNR_OFST_B                            : 15;    /*  0..14, 0x00007FFF */
    FIELD  RNR_GAIN_B                            : 13;    /* 15..27, 0x0FFF8000 */
    FIELD  rsv_28                                :  4;    /* 28..31, 0xF0000000 */
} ISP_DIP_X_RNR_GO_B_T; //  /* 0x150245BC */

typedef union {
    typedef ISP_DIP_X_RNR_GO_B_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_RNR_GO_B_T;

typedef struct {
    FIELD  RNR_OFST_G                            : 15;    /*  0..14, 0x00007FFF */
    FIELD  RNR_GAIN_G                            : 13;    /* 15..27, 0x0FFF8000 */
    FIELD  rsv_28                                :  4;    /* 28..31, 0xF0000000 */
} ISP_DIP_X_RNR_GO_G_T; //  /* 0x150245C0 */

typedef union {
    typedef ISP_DIP_X_RNR_GO_G_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_RNR_GO_G_T;

typedef struct {
    FIELD  RNR_OFST_R                            : 15;    /*  0..14, 0x00007FFF */
    FIELD  RNR_GAIN_R                            : 13;    /* 15..27, 0x0FFF8000 */
    FIELD  rsv_28                                :  4;    /* 28..31, 0xF0000000 */
} ISP_DIP_X_RNR_GO_R_T; //  /* 0x150245C4 */

typedef union {
    typedef ISP_DIP_X_RNR_GO_R_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_RNR_GO_R_T;

typedef struct {
    FIELD  RNR_GAIN_ISO                          : 13;    /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                : 19;    /* 13..31, 0xFFFFE000 */
} ISP_DIP_X_RNR_GAIN_ISO_T; //  /* 0x150245C8 */

typedef union {
    typedef ISP_DIP_X_RNR_GAIN_ISO_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_RNR_GAIN_ISO_T;

typedef struct {
    FIELD  RNR_L_R                               :  9;    /*  0.. 8, 0x000001FF */
    FIELD  RNR_L_G                               :  9;    /*  9..17, 0x0003FE00 */
    FIELD  RNR_L_B                               :  9;    /* 18..26, 0x07FC0000 */
    FIELD  rsv_27                                :  5;    /* 27..31, 0xF8000000 */
} ISP_DIP_X_RNR_L_T; // /* 0x150245CC */

typedef union {
    typedef ISP_DIP_X_RNR_L_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_RNR_L_T;

typedef struct {
    FIELD  RNR_C1_R                              :  9;    /*  0.. 8, 0x000001FF */
    FIELD  RNR_C1_G                              :  9;    /*  9..17, 0x0003FE00 */
    FIELD  RNR_C1_B                              :  9;    /* 18..26, 0x07FC0000 */
    FIELD  rsv_27                                :  5;    /* 27..31, 0xF8000000 */
} ISP_DIP_X_RNR_C1_T; //  /* 0x150245D0 */

typedef union {
    typedef ISP_DIP_X_RNR_C1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_RNR_C1_T;

typedef struct {
    FIELD  RNR_C2_R                              :  9;    /*  0.. 8, 0x000001FF */
    FIELD  RNR_C2_G                              :  9;    /*  9..17, 0x0003FE00 */
    FIELD  RNR_C2_B                              :  9;    /* 18..26, 0x07FC0000 */
    FIELD  rsv_27                                :  5;    /* 27..31, 0xF8000000 */
} ISP_DIP_X_RNR_C2_T; //  /* 0x150245D4 */

typedef union {
    typedef ISP_DIP_X_RNR_C2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_RNR_C2_T;

typedef struct {
    FIELD  RNR_RNG1                              :  6;    /*  0.. 5, 0x0000003F */
    FIELD  RNR_RNG2                              :  6;    /*  6..11, 0x00000FC0 */
    FIELD  RNR_RNG3                              :  6;    /* 12..17, 0x0003F000 */
    FIELD  RNR_RNG4                              :  6;    /* 18..23, 0x00FC0000 */
    FIELD  rsv_24                                :  8;    /* 24..31, 0xFF000000 */
} ISP_DIP_X_RNR_RNG_T; // /* 0x150245D8 */

typedef union {
    typedef ISP_DIP_X_RNR_RNG_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_RNR_RNG_T;

typedef struct {
    FIELD  RNR_MED_RAT                           :  7;    /*  0.. 6, 0x0000007F */
    FIELD  RNR_PRF_RAT                           :  7;    /*  7..13, 0x00003F80 */
    FIELD  RNR_RAD                               :  3;    /* 14..16, 0x0001C000 */
    FIELD  RNR_CTHR                              :  6;    /* 17..22, 0x007E0000 */
    FIELD  RNR_CTHL                              :  6;    /* 23..28, 0x1F800000 */
    FIELD  rsv_29                                :  3;    /* 29..31, 0xE0000000 */
} ISP_DIP_X_RNR_CON2_T; //  /* 0x150245DC */

typedef union {
    typedef ISP_DIP_X_RNR_CON2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_RNR_CON2_T;

typedef struct {
    FIELD  RNR_FL_MODE                           :  1;    /*  0.. 0, 0x00000001 */
    FIELD  RNR_GD_RAT                            :  6;    /*  1.. 6, 0x0000007E */
    FIELD  RNR_LLP_RAT                           :  8;    /*  7..14, 0x00007F80 */
    FIELD  RNR_INT_OFST                          :  8;    /* 15..22, 0x007F8000 */
    FIELD  rsv_23                                :  9;    /* 23..31, 0xFF800000 */
} ISP_DIP_X_RNR_CON3_T; //  /* 0x150245E0 */

typedef union {
    typedef ISP_DIP_X_RNR_CON3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_RNR_CON3_T;

typedef struct {
    FIELD  RNR_SL_EN                             :  1;    /*  0.. 0, 0x00000001 */
    FIELD  RNR_SL_Y2                             : 10;    /*  1..10, 0x000007FE */
    FIELD  RNR_SL_Y1                             : 10;    /* 11..20, 0x001FF800 */
    FIELD  rsv_21                                : 11;    /* 21..31, 0xFFE00000 */
} ISP_DIP_X_RNR_SL_T; //  /* 0x150245E4 */

typedef union {
    typedef ISP_DIP_X_RNR_SL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_RNR_SL_T;

typedef struct {
    FIELD  RNR_SSL_C2                            :  8;    /*  0.. 7, 0x000000FF */
    FIELD  RNR_STH_C2                            :  8;    /*  8..15, 0x0000FF00 */
    FIELD  RNR_SSL_C1                            :  8;    /* 16..23, 0x00FF0000 */
    FIELD  RNR_STH_C1                            :  8;    /* 24..31, 0xFF000000 */
} ISP_DIP_X_RNR_SSL_STH_T; // /* 0x150245E8 */

typedef union {
    typedef ISP_DIP_X_RNR_SSL_STH_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_RNR_SSL_STH_T;

typedef struct {
    FIELD  RNR_TILE_EDGE                         :  4;    /*  0.. 3, 0x0000000F */
    FIELD  rsv_4                                 : 28;    /*  4..31, 0xFFFFFFF0 */
} ISP_DIP_X_RNR_TILE_EDGE_T; // /* 0x150245EC */

typedef union {
    typedef ISP_DIP_X_RNR_TILE_EDGE_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_RNR_TILE_EDGE_T;

typedef union {
    enum { COUNT = 8 };
    struct {
        ISP_NVRAM_RNR_CON1_T  con1;
        ISP_NVRAM_RNR_VC0_T  vc0;
        ISP_NVRAM_RNR_VC1_T  vc1;
        //ISP_NVRAM_RNR_GO_B_T  go_b;
        //ISP_NVRAM_RNR_GO_G_T  go_g;
        //ISP_NVRAM_RNR_GO_R_T  go_r;
        //ISP_NVRAM_RNR_GAIN_ISO_T  gain_iso;
        //ISP_NVRAM_RNR_L_T  l;
        //ISP_NVRAM_RNR_C1_T  c1;
        //ISP_NVRAM_RNR_C2_T  c2;
        ISP_NVRAM_RNR_RNG_T  rng;
        ISP_NVRAM_RNR_CON2_T  con2;
        ISP_NVRAM_RNR_CON3_T  con3;
        ISP_NVRAM_RNR_SL_T  sl;
        ISP_NVRAM_RNR_SSL_STH_T  ssl_sth;
        //ISP_NVRAM_RNR_TILE_EDGE_T  tile_edge;
    };
    MUINT32 set[COUNT];
}ISP_NVRAM_RNR_T;

typedef union {
    enum { COUNT = 12 };
    struct {
        ISP_NVRAM_RNR_CON1_T  con1;
        ISP_NVRAM_RNR_VC0_T  vc0;
        ISP_NVRAM_RNR_VC1_T  vc1;
        ISP_NVRAM_RNR_GO_B_T  go_b;
        ISP_NVRAM_RNR_GO_G_T  go_g;
        ISP_NVRAM_RNR_GO_R_T  go_r;
        ISP_NVRAM_RNR_GAIN_ISO_T  gain_iso;
        //ISP_NVRAM_RNR_L_T  l;
        //ISP_NVRAM_RNR_C1_T  c1;
        //ISP_NVRAM_RNR_C2_T  c2;
        ISP_NVRAM_RNR_RNG_T  rng;
        ISP_NVRAM_RNR_CON2_T  con2;
        ISP_NVRAM_RNR_CON3_T  con3;
        ISP_NVRAM_RNR_SL_T  sl;
        ISP_NVRAM_RNR_SSL_STH_T  ssl_sth;
        //ISP_NVRAM_RNR_TILE_EDGE_T  tile_edge;
    };
    MUINT32 set[COUNT];
}ISP_NVRAM_RNR_GET_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//UDM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// DIP_X_UDM_INTP_CRS
typedef struct {
    FIELD  UDM_CDG_SL                            :  4;		/*  0.. 3, 0x0000000F */
    FIELD  UDM_CDG_OFST                          :  8;		/*  4..11, 0x00000FF0 */
    FIELD  UDM_CDG_RAT                           :  5;		/* 12..16, 0x0001F000 */
    FIELD  UDM_CD_KNL                            :  1;		/* 17..17, 0x00020000 */
    FIELD  rsv_18                                : 12;		/* 18..29, 0x3FFC0000 */
    FIELD  UDM_BYP                               :  1;		/* 30..30, 0x40000000 */
    FIELD  UDM_MN_MODE                           :  1;		/* 31..31, 0x80000000 */
} ISP_DIP_X_UDM_INTP_CRS_T; //	/* 0x15023540 */

typedef union {
    typedef ISP_DIP_X_UDM_INTP_CRS_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_UDM_INTP_CRS_T;

typedef struct {
    FIELD  UDM_HL_OFST                           :  8;		/*  0.. 7, 0x000000FF */
    FIELD  UDM_L0_SL                             :  4;		/*  8..11, 0x00000F00 */
    FIELD  UDM_L0_OFST                           :  8;		/* 12..19, 0x000FF000 */
    FIELD  UDM_CD_SLL                            :  5;		/* 20..24, 0x01F00000 */
    FIELD  UDM_CD_SLC                            :  5;		/* 25..29, 0x3E000000 */
    FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
} ISP_DIP_X_UDM_INTP_NAT_T; //	/* 0x15023544 */

typedef union {
    typedef ISP_DIP_X_UDM_INTP_NAT_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_UDM_INTP_NAT_T;

typedef struct {
    FIELD  UDM_DN_OFST                           :  8;		/*  0.. 7, 0x000000FF */
    FIELD  UDM_L2_SL                             :  4;		/*  8..11, 0x00000F00 */
    FIELD  UDM_L2_OFST                           :  8;		/* 12..19, 0x000FF000 */
    FIELD  UDM_L1_SL                             :  4;		/* 20..23, 0x00F00000 */
    FIELD  UDM_L1_OFST                           :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_UDM_INTP_AUG_T; //	/* 0x15023548 */

typedef union {
    typedef ISP_DIP_X_UDM_INTP_AUG_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_UDM_INTP_AUG_T;

typedef struct {
    FIELD  UDM_LM_Y2                             :  9;		/*  0.. 8, 0x000001FF */
    FIELD  UDM_LM_Y1                             :  9;		/*  9..17, 0x0003FE00 */
    FIELD  UDM_LM_Y0                             :  9;		/* 18..26, 0x07FC0000 */
    FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
} ISP_DIP_X_UDM_LUMA_LUT1_T; //	/* 0x1502354C */

typedef union {
    typedef ISP_DIP_X_UDM_LUMA_LUT1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_UDM_LUMA_LUT1_T;

typedef struct {
    FIELD  UDM_LM_Y5                             :  9;		/*  0.. 8, 0x000001FF */
    FIELD  UDM_LM_Y4                             :  9;		/*  9..17, 0x0003FE00 */
    FIELD  UDM_LM_Y3                             :  9;		/* 18..26, 0x07FC0000 */
    FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
} ISP_DIP_X_UDM_LUMA_LUT2_T; //	/* 0x15023550 */

typedef union {
    typedef ISP_DIP_X_UDM_LUMA_LUT2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_UDM_LUMA_LUT2_T;

typedef struct {
    FIELD  UDM_SL_EN                             :  1;		/*  0.. 0, 0x00000001 */
    FIELD  UDM_SL_HR                             :  5;		/*  1.. 5, 0x0000003E */
    FIELD  UDM_SL_Y2                             :  8;		/*  6..13, 0x00003FC0 */
    FIELD  UDM_SL_Y1                             :  8;		/* 14..21, 0x003FC000 */
    FIELD  rsv_22                                : 10;		/* 22..31, 0xFFC00000 */
} ISP_DIP_X_UDM_SL_CTL_T; //	/* 0x15023554 */

typedef union {
    typedef ISP_DIP_X_UDM_SL_CTL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_UDM_SL_CTL_T;

typedef struct {
    FIELD  UDM_CORE_TH1                          :  8;		/*  0.. 7, 0x000000FF */
    FIELD  UDM_HD_GN2                            :  5;		/*  8..12, 0x00001F00 */
    FIELD  UDM_HD_GN1                            :  5;		/* 13..17, 0x0003E000 */
    FIELD  UDM_HT_GN2                            :  5;		/* 18..22, 0x007C0000 */
    FIELD  UDM_HT_GN1                            :  5;		/* 23..27, 0x0F800000 */
    FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
} ISP_DIP_X_UDM_HFTD_CTL_T; //	/* 0x15023558 */

typedef union {
    typedef ISP_DIP_X_UDM_HFTD_CTL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_UDM_HFTD_CTL_T;

typedef struct {
    FIELD  UDM_N2_STR                            :  5;		/*  0.. 4, 0x0000001F */
    FIELD  UDM_N1_STR                            :  5;		/*  5.. 9, 0x000003E0 */
    FIELD  UDM_N0_STR                            :  5;		/* 10..14, 0x00007C00 */
    FIELD  UDM_XTK_SL                            :  4;		/* 15..18, 0x00078000 */
    FIELD  UDM_XTK_OFST                          :  8;		/* 19..26, 0x07F80000 */
    FIELD  UDM_XTK_RAT                           :  5;		/* 27..31, 0xF8000000 */
} ISP_DIP_X_UDM_NR_STR_T; //	/* 0x1502355C */

typedef union {
    typedef ISP_DIP_X_UDM_NR_STR_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_UDM_NR_STR_T;

typedef struct {
    FIELD  UDM_NGR                               :  4;		/*  0.. 3, 0x0000000F */
    FIELD  UDM_NSL                               :  4;		/*  4.. 7, 0x000000F0 */
    FIELD  UDM_N2_OFST                           :  8;		/*  8..15, 0x0000FF00 */
    FIELD  UDM_N1_OFST                           :  8;		/* 16..23, 0x00FF0000 */
    FIELD  UDM_N0_OFST                           :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_UDM_NR_ACT_T; //	/* 0x15023560 */

typedef union {
    typedef ISP_DIP_X_UDM_NR_ACT_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_UDM_NR_ACT_T;

typedef struct {
    FIELD  UDM_CORE_TH2                          :  8;		/*  0.. 7, 0x000000FF */
    FIELD  UDM_HI_RAT                            :  4;		/*  8..11, 0x00000F00 */
    FIELD  UDM_H3_GN                             :  5;		/* 12..16, 0x0001F000 */
    FIELD  UDM_H2_GN                             :  5;		/* 17..21, 0x003E0000 */
    FIELD  UDM_H1_GN                             :  5;		/* 22..26, 0x07C00000 */
    FIELD  UDM_HA_STR                            :  5;		/* 27..31, 0xF8000000 */
} ISP_DIP_X_UDM_HF_STR_T; //	/* 0x15023564 */

typedef union {
    typedef ISP_DIP_X_UDM_HF_STR_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_UDM_HF_STR_T;

typedef struct {
    FIELD  UDM_H2_UPB                            :  8;		/*  0.. 7, 0x000000FF */
    FIELD  UDM_H2_LWB                            :  8;		/*  8..15, 0x0000FF00 */
    FIELD  UDM_H1_UPB                            :  8;		/* 16..23, 0x00FF0000 */
    FIELD  UDM_H1_LWB                            :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_UDM_HF_ACT1_T; //	/* 0x15023568 */

typedef union {
    typedef ISP_DIP_X_UDM_HF_ACT1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_UDM_HF_ACT1_T;

typedef struct {
    FIELD  UDM_HSLL                              :  4;		/*  0.. 3, 0x0000000F */
    FIELD  UDM_HSLR                              :  4;		/*  4.. 7, 0x000000F0 */
    FIELD  UDM_H3_UPB                            :  8;		/*  8..15, 0x0000FF00 */
    FIELD  UDM_H3_LWB                            :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_UDM_HF_ACT2_T; //	/* 0x1502356C */

typedef union {
    typedef ISP_DIP_X_UDM_HF_ACT2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_UDM_HF_ACT2_T;

typedef struct {
    FIELD  UDM_CLIP_TH                           :  8;		/*  0.. 7, 0x000000FF */
    FIELD  UDM_UN_TH                             :  8;		/*  8..15, 0x0000FF00 */
    FIELD  UDM_OV_TH                             :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_UDM_CLIP_T; //	/* 0x15023570 */

typedef union {
    typedef ISP_DIP_X_UDM_CLIP_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_UDM_CLIP_T;

typedef struct {
    FIELD  UDM_SC_RAT                            :  5;		/*  0.. 4, 0x0000001F */
    FIELD  UDM_SL_RAT                            :  5;		/*  5.. 9, 0x000003E0 */
    FIELD  UDM_FL_MODE                           :  1;		/* 10..10, 0x00000400 */
    FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
} ISP_DIP_X_UDM_DSB_T; //	/* 0x15023574 */

typedef union {
    typedef ISP_DIP_X_UDM_DSB_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_UDM_DSB_T;

typedef struct {
    FIELD  UDM_TILE_EDGE                         :  4;		/*  0.. 3, 0x0000000F */
    FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
} ISP_DIP_X_UDM_TILE_EDGE_T; //	/* 0x15023578 */

typedef union {
    typedef ISP_DIP_X_UDM_TILE_EDGE_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_UDM_TILE_EDGE_T;

typedef struct {
    FIELD  UDM_P1_UPB                            :  8;		/*  0.. 7, 0x000000FF */
    FIELD  UDM_P1_LWB                            :  8;		/*  8..15, 0x0000FF00 */
    FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
} ISP_DIP_X_UDM_P1_ACT_T; //	/* 0x1502357C */

typedef union {
    typedef ISP_DIP_X_UDM_P1_ACT_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_UDM_P1_ACT_T;

typedef struct {
    FIELD  UDM_LR_RAT                            :  4;		/*  0.. 3, 0x0000000F */
    FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
} ISP_DIP_X_UDM_LR_RAT_T; //	/* 0x15023580 */

typedef union {
    typedef ISP_DIP_X_UDM_LR_RAT_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_UDM_LR_RAT_T;

typedef struct {
    FIELD  UDM_HD_GN3                            :  6;		/*  0.. 5, 0x0000003F */
    FIELD  UDM_HFRB_GN                           :  6;		/*  6..11, 0x00000FC0 */
    FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
} ISP_DIP_X_UDM_HFTD_CTL2_T; //	/* 0x15023584 */

typedef union {
    typedef ISP_DIP_X_UDM_HFTD_CTL2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_UDM_HFTD_CTL2_T;

typedef struct {
    FIELD  UDM_P2_CLIP                           :  1;		/*  0.. 0, 0x00000001 */
    FIELD  UDM_P1_BLD                            :  5;		/*  1.. 5, 0x0000003E */
    FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
} ISP_DIP_X_UDM_EST_CTL_T; //	/* 0x15023588 */

typedef union {
    typedef ISP_DIP_X_UDM_EST_CTL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_UDM_EST_CTL_T;

typedef struct {
    FIELD  UDM_SPARE_2                           : 32;		/*  0..31, 0xFFFFFFFF */
} ISP_DIP_X_UDM_SPARE_2_T; //	/* 0x1502358C */

typedef union {
    typedef ISP_DIP_X_UDM_SPARE_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_UDM_SPARE_2_T;

typedef struct {
    FIELD  UDM_SPARE_3                           : 32;		/*  0..31, 0xFFFFFFFF */
} ISP_DIP_X_UDM_SPARE_3_T; //	/* 0x15023590 */

typedef union {
    typedef ISP_DIP_X_UDM_SPARE_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_UDM_SPARE_3_T;

typedef struct {
    FIELD  UDM_INT_LTH                           :  4;		/*  0.. 3, 0x0000000F */
    FIELD  UDM_INT_CDTH                          :  4;		/*  4.. 7, 0x000000F0 */
    FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
} ISP_DIP_X_UDM_INT_CTL_T; //	/* 0x15023594 */

typedef union {
    typedef ISP_DIP_X_UDM_INT_CTL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_UDM_INT_CTL_T;

typedef struct {
    FIELD  UDM_HPOS_GN                           :  5;		/*  0.. 4, 0x0000001F */
    FIELD  UDM_HNEG_GN                           :  5;		/*  5.. 9, 0x000003E0 */
    FIELD  rsv_10                                : 22;		/* 10..31, 0xFFFFFC00 */
} ISP_DIP_X_UDM_EE_T; //	/* 0x15023598 */

typedef union {
    typedef ISP_DIP_X_UDM_EE_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_UDM_EE_T;

//
typedef union {
    enum { COUNT = 20 };
    struct {
        ISP_NVRAM_UDM_INTP_CRS_T      intp_crs;
        ISP_NVRAM_UDM_INTP_NAT_T      intp_nat;
        ISP_NVRAM_UDM_INTP_AUG_T      intp_aug;
        ISP_NVRAM_UDM_LUMA_LUT1_T     luma_lut1;
        ISP_NVRAM_UDM_LUMA_LUT2_T     luma_lut2;
        ISP_NVRAM_UDM_SL_CTL_T        sl_ctl;
        ISP_NVRAM_UDM_HFTD_CTL_T      hftd_ctl;
        ISP_NVRAM_UDM_NR_STR_T        nr_str;
        ISP_NVRAM_UDM_NR_ACT_T        nr_act;
        ISP_NVRAM_UDM_HF_STR_T        hf_str;
        ISP_NVRAM_UDM_HF_ACT1_T       hf_act1;
        ISP_NVRAM_UDM_HF_ACT2_T       hf_act2;
        ISP_NVRAM_UDM_CLIP_T          clip;
        ISP_NVRAM_UDM_DSB_T           dsb;
        //ISP_NVRAM_UDM_TILE_EDGE_T     tile_edge;
        ISP_NVRAM_UDM_P1_ACT_T  p1_act;
        ISP_NVRAM_UDM_LR_RAT_T        lr_rat;
        ISP_NVRAM_UDM_HFTD_CTL2_T  hftd_ctl2;
        ISP_NVRAM_UDM_EST_CTL_T  est_ctl;
        //ISP_NVRAM_UDM_SPARE_2_T       spare_2;
        //ISP_NVRAM_UDM_SPARE_3_T       spare_3;
        ISP_NVRAM_UDM_INT_CTL_T  int_ctl;
        ISP_NVRAM_UDM_EE_T  ee;
    };
    MUINT32 set[COUNT];
} ISP_NVRAM_UDM_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// G2G (CCM)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// DIP_X_G2G_CNV_1 CAM+0920H
typedef struct {
        FIELD G2G_CNV_00                : 13;
        FIELD rsv_13                    : 3;
        FIELD G2G_CNV_01                : 13;
        FIELD rsv_29                    : 3;
} ISP_DIP_X_G2G_CNV_1_T;

typedef union {
    enum { MASK     = 0x1FFF1FFF };
    typedef ISP_DIP_X_G2G_CNV_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2G_CNV_1_T;

// DIP_X_G2G_CNV_2 CAM+0924H
typedef struct {
        FIELD G2G_CNV_02                : 13;
        FIELD rsv_13                    : 19;
} ISP_DIP_X_G2G_CNV_2_T;

typedef union {
    enum { MASK     = 0x00001FFF };
    typedef ISP_DIP_X_G2G_CNV_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2G_CNV_2_T;

// DIP_X_G2G_CNV_3 CAM+0928H
typedef struct {
        FIELD G2G_CNV_10                : 13;
        FIELD rsv_13                    : 3;
        FIELD G2G_CNV_11                : 13;
        FIELD rsv_29                    : 3;
} ISP_DIP_X_G2G_CNV_3_T;

typedef union {
    enum { MASK     = 0x1FFF1FFF };
    typedef ISP_DIP_X_G2G_CNV_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2G_CNV_3_T;

// DIP_X_G2G_CNV_4 CAM+092CH
typedef struct {
        FIELD G2G_CNV_12                : 13;
        FIELD rsv_13                    : 19;
} ISP_DIP_X_G2G_CNV_4_T;

typedef union {
    enum { MASK     = 0x00001FFF };
    typedef ISP_DIP_X_G2G_CNV_4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2G_CNV_4_T;

// DIP_X_G2G_CNV_5 CAM+0930H
typedef struct {
        FIELD G2G_CNV_20                : 13;
        FIELD rsv_13                    : 3;
        FIELD G2G_CNV_21                : 13;
        FIELD rsv_29                    : 3;
} ISP_DIP_X_G2G_CNV_5_T;

typedef union {
    enum { MASK     = 0x1FFF1FFF };
    typedef ISP_DIP_X_G2G_CNV_5_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2G_CNV_5_T;

// DIP_X_G2G_CNV_6 CAM+0934H
typedef struct {
        FIELD G2G_CNV_22                : 13;
        FIELD rsv_13                    : 19;
} ISP_DIP_X_G2G_CNV_6_T;

typedef union {
    enum { MASK     = 0x00001FFF };
    typedef ISP_DIP_X_G2G_CNV_6_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2G_CNV_6_T;

// DIP_X_G2G_CTRL CAM+0938H
typedef struct {
        FIELD G2G_ACC                   :  4;
        FIELD G2G_CFC_EN                :  1;
        FIELD G2G_L                     : 12;
        FIELD G2G_H                     : 12;
        FIELD G2G_MOFST_R               :  1;
        FIELD G2G_POFST_R               :  1;
        FIELD rsv_31                    :  1;
} ISP_DIP_X_G2G_CTRL_T;

typedef union {
    enum { MASK     = 0x7FFFFFFF };
    typedef ISP_DIP_X_G2G_CTRL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2G_CTRL_T;

// DIP_X_G2G_CFC CAM+0938H
typedef struct {
        FIELD G2G_LB                    :  5;
        FIELD G2G_HB                    :  5;
        FIELD G2G_LG                    :  5;
        FIELD G2G_HG                    :  5;
        FIELD G2G_LR                    :  5;
        FIELD G2G_HR                    :  5;
        FIELD rsv_30                    :  2;


} ISP_DIP_X_G2G_CFC_T;

typedef union {
    enum { MASK     = 0x3FFFFFFF };
    typedef ISP_DIP_X_G2G_CFC_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2G_CFC_T;


//
typedef union {
    enum { COUNT = 6 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_G2G_CNV_1_T cnv_1;
        ISP_NVRAM_G2G_CNV_2_T cnv_2;
        ISP_NVRAM_G2G_CNV_3_T cnv_3;
        ISP_NVRAM_G2G_CNV_4_T cnv_4;
        ISP_NVRAM_G2G_CNV_5_T cnv_5;
        ISP_NVRAM_G2G_CNV_6_T cnv_6;
    };
} ISP_NVRAM_CCM_T;



typedef union {
    enum { COUNT = 1 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_G2G_CTRL_T ctrl;
        ISP_NVRAM_G2G_CFC_T cfc;

    };
} ISP_NVRAM_CCM_CTL_T;



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// G2G2 (CCM2)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// DIP_X_G2G2_CNV_1
typedef struct {
        FIELD G2G_CNV_00                : 13;
        FIELD rsv_13                    : 3;
        FIELD G2G_CNV_01                : 13;
        FIELD rsv_29                    : 3;
} ISP_DIP_X_G2G2_CNV_1_T;

typedef union {
    enum { MASK     = 0x1FFF1FFF };
    typedef ISP_DIP_X_G2G_CNV_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2G2_CNV_1_T;

// DIP_X_G2G_CNV_2 CAM+0924H
typedef struct {
        FIELD G2G_CNV_02                : 13;
        FIELD rsv_13                    : 19;
} ISP_DIP_X_G2G2_CNV_2_T;

typedef union {
    enum { MASK     = 0x00001FFF };
    typedef ISP_DIP_X_G2G2_CNV_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2G2_CNV_2_T;

// DIP_X_G2G_CNV_3 CAM+0928H
typedef struct {
        FIELD G2G_CNV_10                : 13;
        FIELD rsv_13                    : 3;
        FIELD G2G_CNV_11                : 13;
        FIELD rsv_29                    : 3;
} ISP_DIP_X_G2G2_CNV_3_T;

typedef union {
    enum { MASK     = 0x1FFF1FFF };
    typedef ISP_DIP_X_G2G2_CNV_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2G2_CNV_3_T;

// DIP_X_G2G_CNV_4 CAM+092CH
typedef struct {
        FIELD G2G_CNV_12                : 13;
        FIELD rsv_13                    : 19;
} ISP_DIP_X_G2G2_CNV_4_T;

typedef union {
    enum { MASK     = 0x00001FFF };
    typedef ISP_DIP_X_G2G2_CNV_4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2G2_CNV_4_T;

// DIP_X_G2G_CNV_5 CAM+0930H
typedef struct {
        FIELD G2G_CNV_20                : 13;
        FIELD rsv_13                    : 3;
        FIELD G2G_CNV_21                : 13;
        FIELD rsv_29                    : 3;
} ISP_DIP_X_G2G2_CNV_5_T;

typedef union {
    enum { MASK     = 0x1FFF1FFF };
    typedef ISP_DIP_X_G2G2_CNV_5_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2G2_CNV_5_T;

// DIP_X_G2G_CNV_6 CAM+0934H
typedef struct {
        FIELD G2G_CNV_22                : 13;
        FIELD rsv_13                    : 19;
} ISP_DIP_X_G2G2_CNV_6_T;

typedef union {
    enum { MASK     = 0x00001FFF };
    typedef ISP_DIP_X_G2G2_CNV_6_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2G2_CNV_6_T;

// DIP_X_G2G_CTRL CAM+0938H
typedef struct {
        FIELD G2G_ACC                   :  4;
        FIELD G2G_CFC_EN                :  1;
        FIELD G2G_L                     : 12;
        FIELD G2G_H                     : 12;
        FIELD G2G_MOFST_R               :  1;
        FIELD G2G_POFST_R               :  1;
        FIELD rsv_31                    :  1;
} ISP_DIP_X_G2G2_CTRL_T;

typedef union {
    enum { MASK     = 0x7FFFFFFF };
    typedef ISP_DIP_X_G2G2_CTRL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2G2_CTRL_T;

// DIP_X_G2G_CFC CAM+0938H
typedef struct {
        FIELD G2G_LB                    :  5;
        FIELD G2G_HB                    :  5;
        FIELD G2G_LG                    :  5;
        FIELD G2G_HG                    :  5;
        FIELD G2G_LR                    :  5;
        FIELD G2G_HR                    :  5;
        FIELD rsv_30                    :  2;
} ISP_DIP_X_G2G2_CFC_T;

typedef union {
    enum { MASK     = 0x3FFFFFFF };
    typedef ISP_DIP_X_G2G2_CFC_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2G2_CFC_T;


//
typedef union {
    enum { COUNT = 6 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_G2G2_CNV_1_T cnv_1;
        ISP_NVRAM_G2G2_CNV_2_T cnv_2;
        ISP_NVRAM_G2G2_CNV_3_T cnv_3;
        ISP_NVRAM_G2G2_CNV_4_T cnv_4;
        ISP_NVRAM_G2G2_CNV_5_T cnv_5;
        ISP_NVRAM_G2G2_CNV_6_T cnv_6;
    };
} ISP_NVRAM_CCM2_T;


typedef union {
    enum { COUNT = 1 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_G2G2_CTRL_T ctrl;
        ISP_NVRAM_G2G2_CFC_T cfc;

    };
} ISP_NVRAM_CCM2_CTL_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// LCS
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef struct {
    FIELD  LCS25_LOG                             :  1;		/*  0.. 0, 0x00000001 */
    FIELD  rsv_1                                 :  7;		/*  1.. 7, 0x000000FE */
    FIELD  LCS25_OUT_WD                          :  9;		/*  8..16, 0x0001FF00 */
    FIELD  rsv_17                                :  3;		/* 17..19, 0x000E0000 */
    FIELD  LCS25_OUT_HT                          :  9;		/* 20..28, 0x1FF00000 */
    FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
} ISP_CAM_LCS25_CON_T; //	/* 0x1A004880 */

typedef union {
    typedef ISP_CAM_LCS25_CON_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCS25_CON_T;

typedef struct {
    FIELD  LCS25_START_J                         : 13;		/*  0..12, 0x00001FFF */
    FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
    FIELD  LCS25_START_I                         : 13;		/* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
} ISP_CAM_LCS25_ST_T; //	/* 0x1A004884 */

typedef union {
    typedef ISP_CAM_LCS25_ST_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCS25_ST_T;

typedef struct {
    FIELD  LCS25_IN_WD                           : 13;		/*  0..12, 0x00001FFF */
    FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
    FIELD  LCS25_IN_HT                           : 13;		/* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
} ISP_CAM_LCS25_AWS_T; //	/* 0x1A004888 */

typedef union {
    typedef ISP_CAM_LCS25_AWS_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCS25_AWS_T;

typedef struct {
    FIELD  LCS25_FLR_OFST                        :  8;		/*  0.. 7, 0x000000FF */
    FIELD  LCS25_FLR_GAIN                        : 12;		/*  8..19, 0x000FFF00 */
    FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
} ISP_CAM_LCS25_FLR_T; //	/* 0x1A00488C */

typedef union {
    typedef ISP_CAM_LCS25_FLR_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCS25_FLR_T;

typedef struct {
    FIELD  LCS25_LRZR_X                          : 20;		/*  0..19, 0x000FFFFF */
    FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
} ISP_CAM_LCS25_LRZR_1_T; //	/* 0x1A004890 */

typedef union {
    typedef ISP_CAM_LCS25_LRZR_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCS25_LRZR_1_T;

typedef struct {
    FIELD  LCS25_LRZR_Y                          : 20;		/*  0..19, 0x000FFFFF */
    FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
} ISP_CAM_LCS25_LRZR_2_T; //	/* 0x1A004894 */

typedef union {
    typedef ISP_CAM_LCS25_LRZR_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCS25_LRZR_2_T;

typedef struct {
    FIELD  LCS25_SATU_B                          : 16;		/*  0..15, 0x0000FFFF */
    FIELD  LCS25_SATU_GB                         : 16;		/* 16..31, 0xFFFF0000 */
} ISP_CAM_LCS25_SATU_1_T; //	/* 0x1A004898 */

typedef union {
    typedef ISP_CAM_LCS25_SATU_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCS25_SATU_1_T;

typedef struct {
    FIELD  LCS25_SATU_GR                         : 16;		/*  0..15, 0x0000FFFF */
    FIELD  LCS25_SATU_R                          : 16;		/* 16..31, 0xFFFF0000 */
} ISP_CAM_LCS25_SATU_2_T; //	/* 0x1A00489C */

typedef union {
    typedef ISP_CAM_LCS25_SATU_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCS25_SATU_2_T;

typedef struct {
    FIELD  LCS25_GAIN_B                          : 13;		/*  0..12, 0x00001FFF */
    FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
    FIELD  LCS25_GAIN_GB                         : 13;		/* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
} ISP_CAM_LCS25_GAIN_1_T; //	/* 0x1A0048A0 */

typedef union {
    typedef ISP_CAM_LCS25_GAIN_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCS25_GAIN_1_T;

typedef struct {
    FIELD  LCS25_GAIN_GR                         : 13;		/*  0..12, 0x00001FFF */
    FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
    FIELD  LCS25_GAIN_R                          : 13;		/* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
} ISP_CAM_LCS25_GAIN_2_T; //	/* 0x1A0048A4 */

typedef union {
    typedef ISP_CAM_LCS25_GAIN_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCS25_GAIN_2_T;

typedef struct {
    FIELD  LCS25_OFST_B                          : 16;		/*  0..15, 0x0000FFFF */
    FIELD  LCS25_OFST_GB                         : 16;		/* 16..31, 0xFFFF0000 */
} ISP_CAM_LCS25_OFST_1_T; //	/* 0x1A0048A8 */

typedef union {
    typedef ISP_CAM_LCS25_OFST_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCS25_OFST_1_T;

typedef struct {
    FIELD  LCS25_OFST_GR                         : 16;		/*  0..15, 0x0000FFFF */
    FIELD  LCS25_OFST_R                          : 16;		/* 16..31, 0xFFFF0000 */
} ISP_CAM_LCS25_OFST_2_T; //	/* 0x1A0048AC */

typedef union {
    typedef ISP_CAM_LCS25_OFST_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCS25_OFST_2_T;

typedef struct {
    FIELD  LCS25_G2G_CNV_00                      : 13;		/*  0..12, 0x00001FFF */
    FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
    FIELD  LCS25_G2G_CNV_01                      : 13;		/* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
} ISP_CAM_LCS25_G2G_CNV_1_T; //	/* 0x1A0048B0 */

typedef union {
    typedef ISP_CAM_LCS25_G2G_CNV_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCS25_G2G_CNV_1_T;

typedef struct {
    FIELD  LCS25_G2G_CNV_02                      : 13;		/*  0..12, 0x00001FFF */
    FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
    FIELD  LCS25_G2G_CNV_10                      : 13;		/* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
} ISP_CAM_LCS25_G2G_CNV_2_T; //	/* 0x1A0048B4 */

typedef union {
    typedef ISP_CAM_LCS25_G2G_CNV_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCS25_G2G_CNV_2_T;

typedef struct {
    FIELD  LCS25_G2G_CNV_11                      : 13;		/*  0..12, 0x00001FFF */
    FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
    FIELD  LCS25_G2G_CNV_12                      : 13;		/* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
} ISP_CAM_LCS25_G2G_CNV_3_T; //	/* 0x1A0048B8 */

typedef union {
    typedef ISP_CAM_LCS25_G2G_CNV_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCS25_G2G_CNV_3_T;

typedef struct {
    FIELD  LCS25_G2G_CNV_20                      : 13;		/*  0..12, 0x00001FFF */
    FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
    FIELD  LCS25_G2G_CNV_21                      : 13;		/* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
} ISP_CAM_LCS25_G2G_CNV_4_T; //	/* 0x1A0048BC */

typedef union {
    typedef ISP_CAM_LCS25_G2G_CNV_4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCS25_G2G_CNV_4_T;

typedef struct {
    FIELD  LCS25_G2G_CNV_22                      : 13;		/*  0..12, 0x00001FFF */
    FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
    FIELD  LCS25_G2G_ACC                         :  4;		/* 16..19, 0x000F0000 */
    FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
} ISP_CAM_LCS25_G2G_CNV_5_T; //	/* 0x1A0048C0 */

typedef union {
    typedef ISP_CAM_LCS25_G2G_CNV_5_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCS25_G2G_CNV_5_T;

typedef struct {
    FIELD  LCS25_LPF_EN                          :  1;		/*  0.. 0, 0x00000001 */
    FIELD  rsv_1                                 : 15;		/*  1..15, 0x0000FFFE */
    FIELD  LCS25_LPF_TH                          : 12;		/* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
} ISP_CAM_LCS25_LPF_T; //	/* 0x1A0048C4 */

typedef union {
    typedef ISP_CAM_LCS25_LPF_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCS25_LPF_T;

typedef union {
    enum { COUNT = 12 };
    MUINT32 set[COUNT];
    struct {
        //ISP_NVRAM_LCS25_CON_T        con;
        //ISP_NVRAM_LCS25_ST_T         st;
        //ISP_NVRAM_LCS25_AWS_T        aws;
        ISP_NVRAM_LCS25_FLR_T        flr;
        //ISP_NVRAM_LCS25_LRZR_1_T     lrzr_1;
        //ISP_NVRAM_LCS25_LRZR_2_T     lrzr_2;
        ISP_NVRAM_LCS25_SATU_1_T     satu_1;
        ISP_NVRAM_LCS25_SATU_2_T     satu_2;
        ISP_NVRAM_LCS25_GAIN_1_T     gain_1;
        ISP_NVRAM_LCS25_GAIN_2_T     gain_2;
        ISP_NVRAM_LCS25_OFST_1_T     ofst_1;
        ISP_NVRAM_LCS25_OFST_2_T     ofst_2;
        ISP_NVRAM_LCS25_G2G_CNV_1_T  g2g_cnv_1;
        ISP_NVRAM_LCS25_G2G_CNV_2_T  g2g_cnv_2;
        ISP_NVRAM_LCS25_G2G_CNV_3_T  g2g_cnv_3;
        ISP_NVRAM_LCS25_G2G_CNV_4_T  g2g_cnv_4;
        ISP_NVRAM_LCS25_G2G_CNV_5_T  g2g_cnv_5;
        //ISP_NVRAM_LCS25_LPF_T        lpf;
    };
} ISP_NVRAM_LCS_T;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// LCE
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef struct{
    FIELD  LCE_GLOB                              :  1;    /*  0.. 0, 0x00000001 */
    FIELD  LCE_GLOB_VHALF                        :  7;    /*  1.. 7, 0x000000FE */
    FIELD  LCE_EDGE                              :  4;    /*  8..11, 0x00000F00 */
    FIELD  rsv_12                                : 16;    /* 12..27, 0x0FFFF000 */
    FIELD  RESERVED                              :  4;    /* 28..31, 0xF0000000 */
} ISP_DIP_X_LCE25_CON_T; // /* 0x15022EC0 */

typedef union {
    typedef ISP_DIP_X_LCE25_CON_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE25_CON_T;

typedef struct {
    FIELD  LCE_BCMK_X                            : 15;    /*  0..14, 0x00007FFF */
    FIELD  rsv_15                                :  1;    /* 15..15, 0x00008000 */
    FIELD  LCE_BCMK_Y                            : 15;    /* 16..30, 0x7FFF0000 */
    FIELD  rsv_31                                :  1;    /* 31..31, 0x80000000 */
} ISP_DIP_X_LCE25_ZR_T; //  /* 0x15022EC4 */

typedef union {
    typedef ISP_DIP_X_LCE25_ZR_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE25_ZR_T;

typedef struct {
    FIELD  LCE_SLM_WD                            :  9;    /*  0.. 8, 0x000001FF */
    FIELD  rsv_9                                 :  7;    /*  9..15, 0x0000FE00 */
    FIELD  LCE_SLM_HT                            :  9;    /* 16..24, 0x01FF0000 */
    FIELD  rsv_25                                :  7;    /* 25..31, 0xFE000000 */
} ISP_DIP_X_LCE25_SLM_SIZE_T; //  /* 0x15022EC8 */

typedef union {
    typedef ISP_DIP_X_LCE25_SLM_SIZE_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE25_SLM_SIZE_T;

typedef struct {
        FIELD LCE_OFST_X                : 15;      /*  0..14, 0x00007FFF */
        FIELD rsv_15                    :  1;      /* 15..15, 0x00008000 */
        FIELD LCE_OFST_Y                : 15;      /* 16..30, 0x7FFF0000 */
        FIELD rsv_31                    :  1;      /* 31..31, 0x80000000 */
} ISP_DIP_X_LCE25_OFST_T; //  /* 0x15022ECC */

typedef union {
    typedef ISP_DIP_X_LCE25_OFST_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE25_OFST_T;

typedef struct {
        FIELD LCE_BIAS_X                :  3;      /*  0.. 2, 0x00000007 */
        FIELD rsv_3                     :  5;      /*  3.. 7, 0x000000F8 */
        FIELD LCE_BIAS_Y                :  2;      /*  8.. 9, 0x00000300 */
        FIELD rsv_10                    : 22;      /* 10..31, 0xFFFFFC00 */
} ISP_DIP_X_LCE25_BIAS_T; //  /* 0x15022ED0 */

typedef union {
    typedef ISP_DIP_X_LCE25_BIAS_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE25_BIAS_T;

typedef struct {
        FIELD LCE_IMAGE_WD              : 14;      /*  0..13, 0x00003FFF */
        FIELD rsv_14                    :  2;      /* 14..15, 0x0000C000 */
        FIELD LCE_IMAGE_HT              : 14;      /* 16..29, 0x3FFF0000 */
        FIELD rsv_30                    :  2;      /* 30..31, 0xC0000000 */
} ISP_DIP_X_LCE25_IMAGE_SIZE_T; //  /* 0x15022ED4 */

typedef union {
    typedef ISP_DIP_X_LCE25_IMAGE_SIZE_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE25_IMAGE_SIZE_T;

typedef struct {
    FIELD  LCE_BIL_TH1                           : 12;    /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                :  4;    /* 12..15, 0x0000F000 */
    FIELD  LCE_BIL_TH2                           : 12;    /* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                :  4;    /* 28..31, 0xF0000000 */
} ISP_DIP_X_LCE25_BIL_TH0_T; // /* 0x15022ED8 */

typedef union {
    typedef ISP_DIP_X_LCE25_BIL_TH0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE25_BIL_TH0_T;

typedef struct {
    FIELD  LCE_BIL_TH3                           : 12;    /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                :  4;    /* 12..15, 0x0000F000 */
    FIELD  LCE_BIL_TH4                           : 12;    /* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                :  4;    /* 28..31, 0xF0000000 */
} ISP_DIP_X_LCE25_BIL_TH1_T; // /* 0x15022EDC */

typedef union {
    typedef ISP_DIP_X_LCE25_BIL_TH1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE25_BIL_TH1_T;

typedef struct {
		FIELD  LCE_CEN_END                           : 12;		/*  0..11, 0x00000FFF */
		FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
		FIELD  LCE_CEN_BLD_WT                        :  4;		/* 16..19, 0x000F0000 */
		FIELD  rsv_20                                :  4;		/* 20..23, 0x00F00000 */
		FIELD  LCE_GLOB_TONE                         :  1;		/* 24..24, 0x01000000 */
		FIELD  LCE_GLOB_POS                          :  1;		/* 25..25, 0x02000000 */
		FIELD  LCE_LC_TONE                           :  1;		/* 26..26, 0x04000000 */
		FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
} ISP_DIP_X_LCE25_TM_PARA0_T; //  /* 0x15022EE0 */

typedef union {
    typedef ISP_DIP_X_LCE25_TM_PARA0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE25_TM_PARA0_T;

typedef struct {
    FIELD  LCE_CEN_MAX_SLP                       :  8;    /*  0.. 7, 0x000000FF */
    FIELD  LCE_CEN_MIN_SLP                       :  8;    /*  8..15, 0x0000FF00 */
    FIELD  LCE_TC_P1                             : 12;    /* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                :  4;    /* 28..31, 0xF0000000 */
} ISP_DIP_X_LCE25_TM_PARA1_T; //  /* 0x15022EE4 */

typedef union {
    typedef ISP_DIP_X_LCE25_TM_PARA1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE25_TM_PARA1_T;

typedef struct {
    FIELD  LCE_TC_P50                            : 12;    /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                :  4;    /* 12..15, 0x0000F000 */
    FIELD  LCE_TC_P500                           : 12;    /* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                :  4;    /* 28..31, 0xF0000000 */
} ISP_DIP_X_LCE25_TM_PARA2_T; //  /* 0x15022EE8 */

typedef union {
    typedef ISP_DIP_X_LCE25_TM_PARA2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE25_TM_PARA2_T;

typedef struct {
    FIELD  LCE_TC_P950                           : 12;    /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                :  4;    /* 12..15, 0x0000F000 */
    FIELD  LCE_TC_P999                           : 12;    /* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                :  4;    /* 28..31, 0xF0000000 */
} ISP_DIP_X_LCE25_TM_PARA3_T; //  /* 0x15022EEC */

typedef union {
    typedef ISP_DIP_X_LCE25_TM_PARA3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE25_TM_PARA3_T;

typedef struct {
    FIELD  LCE_TC_O1                             : 12;    /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                :  4;    /* 12..15, 0x0000F000 */
    FIELD  LCE_TC_O50                            : 12;    /* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                :  4;    /* 28..31, 0xF0000000 */
} ISP_DIP_X_LCE25_TM_PARA4_T; //  /* 0x15022EF0 */

typedef union {
    typedef ISP_DIP_X_LCE25_TM_PARA4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE25_TM_PARA4_T;

typedef struct {
    FIELD  LCE_TC_O500                           : 12;    /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                :  4;    /* 12..15, 0x0000F000 */
    FIELD  LCE_TC_O950                           : 12;    /* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                :  4;    /* 28..31, 0xF0000000 */
} ISP_DIP_X_LCE25_TM_PARA5_T; //  /* 0x15022EF4 */

typedef union {
    typedef ISP_DIP_X_LCE25_TM_PARA5_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE25_TM_PARA5_T;

typedef struct {
    FIELD  LCE_TC_O999                           : 12;    /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                : 20;    /* 12..31, 0xFFFFF000 */
} ISP_DIP_X_LCE25_TM_PARA6_T; //  /* 0x15022EF8 */

typedef union {
    typedef ISP_DIP_X_LCE25_TM_PARA6_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE25_TM_PARA6_T;

typedef struct {
			FIELD  LCE_TC_P0                             : 12;		/*  0..11, 0x00000FFF */
			FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
			FIELD  LCE_TC_P250                           : 12;		/* 16..27, 0x0FFF0000 */
			FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
	} ISP_DIP_X_LCE25_TM_PARA7_T; //  /* 0x15022EF4 */

typedef union {
    typedef ISP_DIP_X_LCE25_TM_PARA7_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE25_TM_PARA7_T;

typedef struct {
			FIELD  LCE_TC_O0                             : 12;		/*  0..11, 0x00000FFF */
			FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
			FIELD  LCE_TC_O250                           : 12;		/* 16..27, 0x0FFF0000 */
			FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
} ISP_DIP_X_LCE25_TM_PARA8_T; //  /* 0x15022EF4 */

typedef union {
    typedef ISP_DIP_X_LCE25_TM_PARA8_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE25_TM_PARA8_T;

typedef struct {
			FIELD  LCE_TC_P750                           : 12;		/*  0..11, 0x00000FFF */
			FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
			FIELD  LCE_TC_O750                           : 12;		/* 16..27, 0x0FFF0000 */
} ISP_DIP_X_LCE25_TM_PARA9_T; //  /* 0x15022EF4 */

typedef union {
    typedef ISP_DIP_X_LCE25_TM_PARA9_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE25_TM_PARA9_T;

typedef struct {
		FIELD  LCE_CEN_MAX_SLP_N                     :  8;		/*  0.. 7, 0x000000FF */
		FIELD  LCE_CEN_MIN_SLP_N                     :  8;		/*  8..15, 0x0000FF00 */
		FIELD  rsv_16                                : 15;		/* 16..30, 0x7FFF0000 */
		FIELD  LCE_CEN_SLOPE_DEP_EN                  :  1;		/* 31..31, 0x80000000 */
} ISP_DIP_X_LCE25_TCHL_PARA0_T; //  /* 0x15022EE0 */

typedef union {
    typedef ISP_DIP_X_LCE25_TCHL_PARA0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE25_TCHL_PARA0_T;

typedef struct {
		FIELD  LCE_TCHL_DTH1                         :  8;		/*  0.. 7, 0x000000FF */
		FIELD  rsv_8                                 :  8;		/*  8..15, 0x0000FF00 */
		FIELD  LCE_TCHL_MTH                          :  8;		/* 16..23, 0x00FF0000 */
		FIELD  LCE_TCHL_MGIAN                        :  4;		/* 24..27, 0x0F000000 */
		FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
		FIELD  LCE_TCHL_EN                           :  1;		/* 31..31, 0x80000000 */
} ISP_DIP_X_LCE25_TCHL_PARA1_T; //  /* 0x15022EE4 */

typedef union {
    typedef ISP_DIP_X_LCE25_TCHL_PARA1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE25_TCHL_PARA1_T;

typedef struct {
		FIELD  LCE_TCHL_BW1                          :  4;		/*  0.. 3, 0x0000000F */
		FIELD  LCE_TCHL_BW2                          :  4;		/*  4.. 7, 0x000000F0 */
		FIELD  rsv_8                                 :  8;		/*  8..15, 0x0000FF00 */
		FIELD  LCE_TCHL_DTH2                         : 12;		/* 16..27, 0x0FFF0000 */
		FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
} ISP_DIP_X_LCE25_TCHL_PARA2_T; //  /* 0x15022EE8 */

typedef union {
    typedef ISP_DIP_X_LCE25_TCHL_PARA2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE25_TCHL_PARA2_T;

typedef struct {
		FIELD  LCE_HLR_TC_P1                         : 12;		/*  0..11, 0x00000FFF */
		FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
		FIELD  LCE_HLR_TC_P50                        : 12;		/* 16..27, 0x0FFF0000 */
		FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
		FIELD  LCE_HLR_LMP_EN                        :  1;		/* 31..31, 0x80000000 */
} ISP_DIP_X_LCE25_HLR_PARA0_T; //  /* 0x15022EE0 */

typedef union {
    typedef ISP_DIP_X_LCE25_HLR_PARA0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE25_HLR_PARA0_T;

typedef struct {
		FIELD  LCE_HLR_TC_P500                       : 12;		/*  0..11, 0x00000FFF */
		FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
		FIELD  LCE_HLR_TC_P950                       : 12;		/* 16..27, 0x0FFF0000 */
		FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
} ISP_DIP_X_LCE25_HLR_PARA1_T; //  /* 0x15022EE4 */

typedef union {
    typedef ISP_DIP_X_LCE25_HLR_PARA1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE25_HLR_PARA1_T;

typedef struct {
		FIELD  LCE_HLR_TC_P999                       : 12;		/*  0..11, 0x00000FFF */
		FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
		FIELD  LCE_HLR_TC_O1                         : 12;		/* 16..27, 0x0FFF0000 */
		FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
} ISP_DIP_X_LCE25_HLR_PARA2_T; //  /* 0x15022EE8 */

typedef union {
    typedef ISP_DIP_X_LCE25_HLR_PARA2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE25_HLR_PARA2_T;

typedef struct {
		FIELD  LCE_HLR_TC_O50                        : 12;		/*  0..11, 0x00000FFF */
		FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
		FIELD  LCE_HLR_TC_O500                       : 12;		/* 16..27, 0x0FFF0000 */
		FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
} ISP_DIP_X_LCE25_HLR_PARA3_T; //  /* 0x15022EEC */

typedef union {
    typedef ISP_DIP_X_LCE25_HLR_PARA3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE25_HLR_PARA3_T;

typedef struct {
		FIELD  LCE_HLR_TC_O950                       : 12;		/*  0..11, 0x00000FFF */
		FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
		FIELD  LCE_HLR_TC_O999                       : 12;		/* 16..27, 0x0FFF0000 */
		FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
} ISP_DIP_X_LCE25_HLR_PARA4_T; //  /* 0x15022EF0 */

typedef union {
    typedef ISP_DIP_X_LCE25_HLR_PARA4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE25_HLR_PARA4_T;

typedef struct {
		FIELD  LCE_HLR_TC_P0                         : 12;		/*  0..11, 0x00000FFF */
		FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
		FIELD  LCE_HLR_TC_P250                       : 12;		/* 16..27, 0x0FFF0000 */
		FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
} ISP_DIP_X_LCE25_HLR_PARA5_T; //  /* 0x15022EF4 */

typedef union {
    typedef ISP_DIP_X_LCE25_HLR_PARA5_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE25_HLR_PARA5_T;

typedef struct {
		FIELD  LCE_HLR_TC_O0                         : 12;		/*  0..11, 0x00000FFF */
		FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
		FIELD  LCE_HLR_TC_O250                       : 12;		/* 16..27, 0x0FFF0000 */
		FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
} ISP_DIP_X_LCE25_HLR_PARA6_T; //  /* 0x15022EF8 */

typedef union {
    typedef ISP_DIP_X_LCE25_HLR_PARA6_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE25_HLR_PARA6_T;

typedef struct {
		FIELD  LCE_HLR_TC_P750                       : 12;		/*  0..11, 0x00000FFF */
		FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
		FIELD  LCE_HLR_TC_O750                       : 12;		/* 16..27, 0x0FFF0000 */
		FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
} ISP_DIP_X_LCE25_HLR_PARA7_T; //  /* 0x15022EF4 */

typedef union {
    typedef ISP_DIP_X_LCE25_HLR_PARA7_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE25_HLR_PARA7_T;

typedef union {
    enum { COUNT = 26 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_LCE25_CON_T          con;
        ISP_NVRAM_LCE25_ZR_T           zr;
        ISP_NVRAM_LCE25_SLM_SIZE_T     slm_size;
        //ISP_NVRAM_LCE25_OFST_T         ofst;
        //ISP_NVRAM_LCE25_BIAS_T         bias;
        //ISP_NVRAM_LCE25_IMAGE_SIZE_T   image_size;
        ISP_NVRAM_LCE25_BIL_TH0_T      bil_th0;
        ISP_NVRAM_LCE25_BIL_TH1_T      bil_th1;
        ISP_NVRAM_LCE25_TM_PARA0_T     tm_para0;
        ISP_NVRAM_LCE25_TM_PARA1_T     tm_para1;
        ISP_NVRAM_LCE25_TM_PARA2_T     tm_para2;
        ISP_NVRAM_LCE25_TM_PARA3_T     tm_para3;
        ISP_NVRAM_LCE25_TM_PARA4_T     tm_para4;
        ISP_NVRAM_LCE25_TM_PARA5_T     tm_para5;
        ISP_NVRAM_LCE25_TM_PARA6_T     tm_para6;
        ISP_NVRAM_LCE25_TM_PARA7_T     tm_para7;
        ISP_NVRAM_LCE25_TM_PARA8_T     tm_para8;
        ISP_NVRAM_LCE25_TM_PARA9_T     tm_para9;
        ISP_NVRAM_LCE25_TCHL_PARA0_T     tchl_para0;
        ISP_NVRAM_LCE25_TCHL_PARA1_T     tchl_para1;
        ISP_NVRAM_LCE25_TCHL_PARA2_T     tchl_para2;
        ISP_NVRAM_LCE25_HLR_PARA0_T     hlr_para0;
        ISP_NVRAM_LCE25_HLR_PARA1_T     hlr_para1;
        ISP_NVRAM_LCE25_HLR_PARA2_T     hlr_para2;
        ISP_NVRAM_LCE25_HLR_PARA3_T     hlr_para3;
        ISP_NVRAM_LCE25_HLR_PARA4_T     hlr_para4;
        ISP_NVRAM_LCE25_HLR_PARA5_T     hlr_para5;
        ISP_NVRAM_LCE25_HLR_PARA6_T     hlr_para6;
        ISP_NVRAM_LCE25_HLR_PARA7_T     hlr_para7;
    };
} ISP_NVRAM_LCE_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// GGM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
enum { GGM_LUT_SIZE = 192 };

typedef struct {
    FIELD  GGM_R                                 : 10;		/*  0.. 9, 0x000003FF */
    FIELD  GGM_G                                 : 10;		/* 10..19, 0x000FFC00 */
    FIELD  GGM_B                                 : 10;		/* 20..29, 0x3FF00000 */
    FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
} ISP_DIP_X_GGM_LUT_T; //	/* 0x15024180 ~ 0x1502447F */

typedef union {
    enum { COUNT = GGM_LUT_SIZE };
    MUINT32 set[COUNT];
    struct {
        ISP_DIP_X_GGM_LUT_T lut[GGM_LUT_SIZE];
    };
} ISP_NVRAM_GGM_LUT_T;

typedef struct {
    FIELD GGM_LNR :                     1; /* 0.. 0, 0x00000001 */
    FIELD GGM_END_VAR :                 10; /* 1..10, 0x000007FE */
    FIELD rsv_11 :                      5; /* 11..15, 0x0000F800 */
    FIELD GGM_RMP_VAR :                 14; /* 16..29, 0x3FFF0000 */
    FIELD rsv_30 :                      2; /* 30..31, 0xC0000000 */
} ISP_DIP_X_GGM_CTRL_T; //	/* 0x15024480 */

typedef union {
    typedef ISP_DIP_X_GGM_CTRL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_GGM_CTRL_T;


typedef struct {
    ISP_NVRAM_GGM_LUT_T  lut;
} ISP_NVRAM_GGM_T;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// GGM2
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
enum { GGM2_LUT_SIZE = 192 };

typedef struct {
    FIELD  GGM_R                                 : 10;		/*  0.. 9, 0x000003FF */
    FIELD  GGM_G                                 : 10;		/* 10..19, 0x000FFC00 */
    FIELD  GGM_B                                 : 10;		/* 20..29, 0x3FF00000 */
    FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
} ISP_DIP_X_GGM2_LUT_T; //	/* 0x15024180 ~ 0x1502447F */

typedef union {
    enum { COUNT = GGM2_LUT_SIZE };
    MUINT32 set[COUNT];
    struct {
        ISP_DIP_X_GGM2_LUT_T lut[GGM2_LUT_SIZE];
    };
} ISP_NVRAM_GGM2_LUT_T;

typedef struct {
    FIELD GGM_LNR :                     1; /* 0.. 0, 0x00000001 */
    FIELD GGM_END_VAR :                 10; /* 1..10, 0x000007FE */
    FIELD rsv_11 :                      5; /* 11..15, 0x0000F800 */
    FIELD GGM_RMP_VAR :                 14; /* 16..29, 0x3FFF0000 */
    FIELD rsv_30 :                      2; /* 30..31, 0xC0000000 */
} ISP_DIP_X_GGM2_CTRL_T; //	/* 0x15024480 */

typedef union {
    typedef ISP_DIP_X_GGM2_CTRL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_GGM2_CTRL_T;


typedef struct {
    ISP_NVRAM_GGM2_LUT_T  lut;
} ISP_NVRAM_GGM2_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// G2C
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// DIP_X_G2C_CONV_0A CAM+BA0H
typedef struct {
        FIELD G2C_CNV_00                : 11;
        FIELD rsv_11                    : 5;
        FIELD G2C_CNV_01                : 11;
        FIELD rsv_27                    : 5;
} ISP_DIP_X_G2C_CONV_0A_T;

typedef union {
    enum { MASK     = 0x07FF07FF };
    typedef ISP_DIP_X_G2C_CONV_0A_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2C_CONV_0A_T;

// DIP_X_G2C_CONV_0B CAM+BA4H
typedef struct {
        FIELD G2C_CNV_02                : 11;
        FIELD rsv_11                    : 5;
        FIELD G2C_Y_OFST                : 11;
        FIELD rsv_27                    : 5;
} ISP_DIP_X_G2C_CONV_0B_T;

typedef union {
    enum { MASK     = 0x07FF07FF };
    typedef ISP_DIP_X_G2C_CONV_0B_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2C_CONV_0B_T;

// DIP_X_G2C_CONV_1A CAM+BA8H
typedef struct {
        FIELD G2C_CNV_10                : 11;
        FIELD rsv_11                    : 5;
        FIELD G2C_CNV_11                : 11;
        FIELD rsv_27                    : 5;
} ISP_DIP_X_G2C_CONV_1A_T;

typedef union {
    enum { MASK     = 0x07FF07FF };
    typedef ISP_DIP_X_G2C_CONV_1A_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2C_CONV_1A_T;

// DIP_X_G2C_CONV_1B CAM+BACH
typedef struct {
        FIELD G2C_CNV_12                : 11;
        FIELD rsv_11                    : 5;
        FIELD G2C_U_OFST                : 10;
        FIELD rsv_26                    : 6;
} ISP_DIP_X_G2C_CONV_1B_T;

typedef union {
    enum { MASK     = 0x03FF07FF };
    typedef ISP_DIP_X_G2C_CONV_1B_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2C_CONV_1B_T;

// DIP_X_G2C_CONV_2A CAM+BB0H
typedef struct {
        FIELD G2C_CNV_20                : 11;
        FIELD rsv_11                    : 5;
        FIELD G2C_CNV_21                : 11;
        FIELD rsv_27                    : 5;
} ISP_DIP_X_G2C_CONV_2A_T;

typedef union {
    enum { MASK     = 0x07FF07FF };
    typedef ISP_DIP_X_G2C_CONV_2A_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2C_CONV_2A_T;

// DIP_X_G2C_CONV_2B CAM+BB4H
typedef struct {
        FIELD G2C_CNV_22                : 11;
        FIELD rsv_11                    : 5;
        FIELD G2C_V_OFST                : 10;
        FIELD rsv_26                    : 6;
} ISP_DIP_X_G2C_CONV_2B_T;

typedef union {
    enum { MASK     = 0x03FF07FF };
    typedef ISP_DIP_X_G2C_CONV_2B_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2C_CONV_2B_T;

// DIP_X_G2C_SHADE_CON_1 CAM+BB8H
typedef struct {
        FIELD G2C_SHADE_VAR             : 18;
        FIELD G2C_SHADE_P0              : 11;
        FIELD G2C_SHADE_EN              : 1;
        FIELD rsv_30                    : 2;
} ISP_DIP_X_G2C_SHADE_CON_1_T;

typedef union {
    enum { MASK     = 0x3FFFFFFF };
    typedef ISP_DIP_X_G2C_SHADE_CON_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2C_SHADE_CON_1_T;

// DIP_X_G2C_SHADE_CON_2 CAM+BBCH
typedef struct {
        FIELD G2C_SHADE_P1              : 11;
        FIELD rsv_11                    : 1;
        FIELD G2C_SHADE_P2              : 11;
        FIELD rsv_23                    : 9;
} ISP_DIP_X_G2C_SHADE_CON_2_T;

typedef union {
    enum { MASK     = 0x007FF7FF };
    typedef ISP_DIP_X_G2C_SHADE_CON_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2C_SHADE_CON_2_T;

// DIP_X_G2C_SHADE_CON_3 CAM+BC0H
typedef struct {
        FIELD G2C_SHADE_UB              : 9;
        FIELD rsv_9                     : 23;
} ISP_DIP_X_G2C_SHADE_CON_3_T;

typedef union {
    enum { MASK     = 0x000001FF };
    typedef ISP_DIP_X_G2C_SHADE_CON_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2C_SHADE_CON_3_T;

// DIP_X_G2C_SHADE_TAR CAM+BC4H
typedef struct {
        FIELD G2C_SHADE_XMID            : 14;
        FIELD rsv_14                    : 2;
        FIELD G2C_SHADE_YMID            : 14;
        FIELD rsv_30                    : 2;
} ISP_DIP_X_G2C_SHADE_TAR_T;

typedef union {
    enum { MASK     = 0x3FFF3FFF };
    typedef ISP_DIP_X_G2C_SHADE_TAR_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2C_SHADE_TAR_T;

// DIP_X_G2C_SHADE_SP CAM+BC8H
typedef struct {
        FIELD G2C_SHADE_XSP             : 14;
        FIELD rsv_14                    : 2;
        FIELD G2C_SHADE_YSP             : 14;
        FIELD rsv_30                    : 2;
} ISP_DIP_X_G2C_SHADE_SP_T;

typedef union {
    enum { MASK     = 0x3FFF3FFF };
    typedef ISP_DIP_X_G2C_SHADE_SP_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2C_SHADE_SP_T;

// DIP_X_G2C_CFC_CON_1 CAM+BC8H
typedef struct {
        FIELD  G2C_CFC_EN               :  1;
        FIELD  rsv_1                    :  3;
        FIELD  G2C_L                    : 10;
        FIELD  rsv_14                   :  2;
        FIELD  G2C_H                    : 10;
        FIELD  rsv_26                   :  6;

} ISP_DIP_X_G2C_CFC_CON_1_T;

typedef union {
    enum { MASK     = 0x03FF3FF1 };
    typedef ISP_DIP_X_G2C_CFC_CON_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2C_CFC_CON_1_T;

// DIP_X_G2C_CFC_CON_2 CAM+BC8H
typedef struct {
        FIELD  G2C_HR                   :  5;
        FIELD  G2C_LR                   :  5;
        FIELD  G2C_HG                   :  5;
        FIELD  G2C_LG                   :  5;
        FIELD  G2C_HB                   :  5;
        FIELD  G2C_LB                   :  5;
        FIELD  rsv_30                   :  2;

} ISP_DIP_X_G2C_CFC_CON_2_T;

typedef union {
    enum { MASK     = 0x3FFFFFFF };
    typedef ISP_DIP_X_G2C_CFC_CON_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2C_CFC_CON_2_T;



//
typedef union {
    enum { COUNT = 6 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_G2C_CONV_0A_T     conv_0a;
        ISP_NVRAM_G2C_CONV_0B_T     conv_0b;
        ISP_NVRAM_G2C_CONV_1A_T     conv_1a;
        ISP_NVRAM_G2C_CONV_1B_T     conv_1b;
        ISP_NVRAM_G2C_CONV_2A_T     conv_2a;
        ISP_NVRAM_G2C_CONV_2B_T     conv_2b;
    };
} ISP_NVRAM_G2C_T;

//
typedef union {
    enum { COUNT = 5 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_G2C_SHADE_CON_1_T shade_con_1;
        ISP_NVRAM_G2C_SHADE_CON_2_T shade_con_2;
        ISP_NVRAM_G2C_SHADE_CON_3_T shade_con_3;
        ISP_NVRAM_G2C_SHADE_TAR_T   tar;
        ISP_NVRAM_G2C_SHADE_SP_T    sp;
    };
} ISP_NVRAM_G2C_SHADE_T;

//
typedef union {
    enum { COUNT = 2 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_G2C_CFC_CON_1_T cfc_con_1;
        ISP_NVRAM_G2C_CFC_CON_2_T cfc_con_2;
    };
} ISP_NVRAM_G2C_CFC_T;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ANR_TBL
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

enum { ANR_HW_TBL_NUM = 256 };

//DIP_X_ANR_TBL
typedef struct {
        FIELD ANR_TBL_Y0        : 5;
        FIELD ANR_TBL_U0        : 5;
        FIELD ANR_TBL_V0        : 5;
        FIELD rsv_12            : 1;
        FIELD ANR_TBL_Y1        : 5;
        FIELD ANR_TBL_U1        : 5;
        FIELD ANR_TBL_V1        : 5;
        FIELD rsv_28            : 1;
} ISP_DIP_X_ANR_TBL_T;

typedef union {
    enum { MASK     = 0x0FFF0FFF };
    typedef ISP_DIP_X_ANR_TBL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ANR_TBL_T;


typedef union {
    enum { COUNT = ANR_HW_TBL_NUM };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_ANR_TBL_T   lut[ANR_HW_TBL_NUM];
    };
} ISP_NVRAM_ANR_LUT_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ANR
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef struct {
    FIELD  NBC_ANR_ENC                           :  1;		/*  0.. 0, 0x00000001 */
    FIELD  NBC_ANR_ENY                           :  1;		/*  1.. 1, 0x00000002 */
    FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
    FIELD  NBC_ANR_Y_FLT0_IDX                    :  1;		/*  4.. 4, 0x00000010 */
    FIELD  rsv_5                                 :  1;		/*  5.. 5, 0x00000020 */
    FIELD  NBC_ANR_Y_FLT1_IDX                    :  1;		/*  6.. 6, 0x00000040 */
    FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
    FIELD  NBC_ANR_Y_FLT2_IDX                    :  2;		/*  8.. 9, 0x00000300 */
    FIELD  NBC_ANR_Y_FLT3_IDX                    :  2;		/* 10..11, 0x00000C00 */
    FIELD  NBC_ANR_ACT_LP_EN                     :  1;		/* 12..12, 0x00001000 */
    FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
    FIELD  NBC_ANR_TILE_EDGE                     :  4;		/* 16..19, 0x000F0000 */
    FIELD  NBC_ANR_LTM_LINK                      :  1;		/* 20..20, 0x00100000 */
    FIELD  NBC_ANR_LTM_LUT_BYPASS                :  1;		/* 21..21, 0x00200000 */
    FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
    FIELD  NBC_ANR_SL2_LINK                      :  1;		/* 24..24, 0x01000000 */
    FIELD  rsv_25                                :  3;		/* 25..27, 0x0E000000 */
    FIELD  NBC_ANR_TABLE_EN                      :  1;		/* 28..28, 0x10000000 */
    FIELD  NBC_ANR_TBL_PRC                       :  1;		/* 29..29, 0x20000000 */
    FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
} ISP_DIP_X_NBC_ANR_CON1_T; //	/* 0x15025700 */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_CON1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_CON1_T;

typedef struct {
    FIELD  NBC_ANR_IMPL_MODE                     :  2;		/*  0.. 1, 0x00000003 */
    FIELD  rsv_2                                 :  6;		/*  2.. 7, 0x000000FC */
    FIELD  NBC_ANR_C_SM_EDGE                     :  2;		/*  8.. 9, 0x00000300 */
    FIELD  rsv_10                                :  2;		/* 10..11, 0x00000C00 */
    FIELD  NBC_ANR_FLT_C                         :  1;		/* 12..12, 0x00001000 */
    FIELD  rsv_13                                : 11;		/* 13..23, 0x00FFE000 */
    FIELD  NBC_ANR_C_SM_EDGE_TH                  :  3;		/* 24..26, 0x07000000 */
    FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
} ISP_DIP_X_NBC_ANR_CON2_T; //	/* 0x15025704 */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_CON2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_CON2_T;

typedef struct {
    FIELD  NBC_ANR_CEN_GAIN_LO_TH                :  5;		/*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
    FIELD  NBC_ANR_CEN_GAIN_HI_TH                :  5;		/*  8..12, 0x00001F00 */
    FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
    FIELD  NBC_ANR_Y_SLOPE_V_TH                  :  8;		/* 16..23, 0x00FF0000 */
    FIELD  NBC_ANR_Y_SLOPE_H_TH                  :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_NBC_ANR_YAD1_T; //	/* 0x15025708 */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_YAD1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_YAD1_T;

typedef struct {
    FIELD  NBC_ANR_Y_VERT_ACT_TH                 :  5;		/*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
    FIELD  NBC_ANR_PTY_GAIN_TH                   :  5;		/*  8..12, 0x00001F00 */
    FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
    FIELD  NBC_ANR_Y_VERT_SIGMA                  :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_NBC_ANR_YAD2_T; //	/* 0x1502570C */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_YAD2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_YAD2_T;

typedef struct {
    FIELD  NBC_ANR_Y_CPX1                        :  8;		/*  0.. 7, 0x000000FF */
    FIELD  NBC_ANR_Y_CPX2                        :  8;		/*  8..15, 0x0000FF00 */
    FIELD  NBC_ANR_Y_CPX3                        :  8;		/* 16..23, 0x00FF0000 */
    FIELD  NBC_ANR_Y_CPX4                        :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_NBC_ANR_Y4LUT1_T; //	/* 0x15025710 */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_Y4LUT1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_Y4LUT1_T;

typedef struct {
    FIELD  NBC_ANR_Y_SCALE_CPY0                  :  5;		/*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
    FIELD  NBC_ANR_Y_SCALE_CPY1                  :  5;		/*  8..12, 0x00001F00 */
    FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
    FIELD  NBC_ANR_Y_SCALE_CPY2                  :  5;		/* 16..20, 0x001F0000 */
    FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
    FIELD  NBC_ANR_Y_SCALE_CPY3                  :  5;		/* 24..28, 0x1F000000 */
    FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
} ISP_DIP_X_NBC_ANR_Y4LUT2_T; //	/* 0x15025714 */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_Y4LUT2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_Y4LUT2_T;

typedef struct {
    FIELD  NBC_ANR_Y_SCALE_SP0                   :  5;		/*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
    FIELD  NBC_ANR_Y_SCALE_SP1                   :  5;		/*  8..12, 0x00001F00 */
    FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
    FIELD  NBC_ANR_Y_SCALE_SP2                   :  5;		/* 16..20, 0x001F0000 */
    FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
    FIELD  NBC_ANR_Y_SCALE_SP3                   :  5;		/* 24..28, 0x1F000000 */
    FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
} ISP_DIP_X_NBC_ANR_Y4LUT3_T; //	/* 0x15025718 */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_Y4LUT3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_Y4LUT3_T;

typedef struct {
    FIELD  NBC_ANR_C_CPX1                        :  8;		/*  0.. 7, 0x000000FF */
    FIELD  NBC_ANR_C_CPX2                        :  8;		/*  8..15, 0x0000FF00 */
    FIELD  NBC_ANR_C_CPX3                        :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_NBC_ANR_C4LUT1_T; //	/* 0x1502571C */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_C4LUT1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_C4LUT1_T;

typedef struct {
    FIELD  NBC_ANR_C_SCALE_CPY0                  :  5;		/*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
    FIELD  NBC_ANR_C_SCALE_CPY1                  :  5;		/*  8..12, 0x00001F00 */
    FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
    FIELD  NBC_ANR_C_SCALE_CPY2                  :  5;		/* 16..20, 0x001F0000 */
    FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
    FIELD  NBC_ANR_C_SCALE_CPY3                  :  5;		/* 24..28, 0x1F000000 */
    FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
} ISP_DIP_X_NBC_ANR_C4LUT2_T; //	/* 0x15025720 */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_C4LUT2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_C4LUT2_T;

typedef struct {
    FIELD  NBC_ANR_C_SCALE_SP0                   :  5;		/*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
    FIELD  NBC_ANR_C_SCALE_SP1                   :  5;		/*  8..12, 0x00001F00 */
    FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
    FIELD  NBC_ANR_C_SCALE_SP2                   :  5;		/* 16..20, 0x001F0000 */
    FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
    FIELD  NBC_ANR_C_SCALE_SP3                   :  5;		/* 24..28, 0x1F000000 */
    FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
} ISP_DIP_X_NBC_ANR_C4LUT3_T; //	/* 0x15025724 */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_C4LUT3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_C4LUT3_T;

typedef struct {
    FIELD  NBC_ANR_Y_ACT_CPY0                    :  8;		/*  0.. 7, 0x000000FF */
    FIELD  NBC_ANR_Y_ACT_CPY1                    :  8;		/*  8..15, 0x0000FF00 */
    FIELD  NBC_ANR_Y_ACT_CPY2                    :  8;		/* 16..23, 0x00FF0000 */
    FIELD  NBC_ANR_Y_ACT_CPY3                    :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_NBC_ANR_A4LUT2_T; //	/* 0x15025728 */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_A4LUT2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_A4LUT2_T;

typedef struct {
    FIELD  NBC_ANR_Y_ACT_SP0                     :  6;		/*  0.. 5, 0x0000003F */
    FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
    FIELD  NBC_ANR_Y_ACT_SP1                     :  6;		/*  8..13, 0x00003F00 */
    FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
    FIELD  NBC_ANR_Y_ACT_SP2                     :  6;		/* 16..21, 0x003F0000 */
    FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
    FIELD  NBC_ANR_Y_ACT_SP3                     :  6;		/* 24..29, 0x3F000000 */
    FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
} ISP_DIP_X_NBC_ANR_A4LUT3_T; //	/* 0x1502572C */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_A4LUT3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_A4LUT3_T;

typedef struct {
    FIELD  NBC_ANR_SL2_X1                        :  8;		/*  0.. 7, 0x000000FF */
    FIELD  NBC_ANR_SL2_X2                        :  8;		/*  8..15, 0x0000FF00 */
    FIELD  NBC_ANR_SL2_X3                        :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_NBC_ANR_L4LUT1_T; //	/* 0x15025730 */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_L4LUT1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_L4LUT1_T;

typedef struct {
    FIELD  NBC_ANR_SL2_GAIN0                     :  6;		/*  0.. 5, 0x0000003F */
    FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
    FIELD  NBC_ANR_SL2_GAIN1                     :  6;		/*  8..13, 0x00003F00 */
    FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
    FIELD  NBC_ANR_SL2_GAIN2                     :  6;		/* 16..21, 0x003F0000 */
    FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
    FIELD  NBC_ANR_SL2_GAIN3                     :  6;		/* 24..29, 0x3F000000 */
    FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
} ISP_DIP_X_NBC_ANR_L4LUT2_T; //	/* 0x15025734 */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_L4LUT2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_L4LUT2_T;

typedef struct {
    FIELD  NBC_ANR_SL2_SP0                       :  6;		/*  0.. 5, 0x0000003F */
    FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
    FIELD  NBC_ANR_SL2_SP1                       :  6;		/*  8..13, 0x00003F00 */
    FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
    FIELD  NBC_ANR_SL2_SP2                       :  6;		/* 16..21, 0x003F0000 */
    FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
    FIELD  NBC_ANR_SL2_SP3                       :  6;		/* 24..29, 0x3F000000 */
    FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
} ISP_DIP_X_NBC_ANR_L4LUT3_T; //	/* 0x15025738 */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_L4LUT3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_L4LUT3_T;

typedef struct {
    FIELD  NBC_ANR_Y_L0_V_RNG1                   :  8;		/*  0.. 7, 0x000000FF */
    FIELD  NBC_ANR_Y_L0_V_RNG2                   :  8;		/*  8..15, 0x0000FF00 */
    FIELD  NBC_ANR_Y_L0_V_RNG3                   :  8;		/* 16..23, 0x00FF0000 */
    FIELD  NBC_ANR_Y_L0_V_RNG4                   :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_NBC_ANR_PTY0V_T; //	/* 0x1502573C */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_PTY0V_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_PTY0V_T;

typedef struct {
    FIELD  rsv_0                                 :  8;		/*  0.. 7, 0x000000FF */
    FIELD  NBC_ANR_PTC_GAIN_TH                   :  5;		/*  8..12, 0x00001F00 */
    FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
    FIELD  NBC_ANR_C_L_DIFF_TH                   :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_NBC_ANR_CAD_T; //	/* 0x15025740 */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_CAD_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_CAD_T;

typedef struct {
    FIELD  NBC_ANR_Y_L1_V_RNG1                   :  8;		/*  0.. 7, 0x000000FF */
    FIELD  NBC_ANR_Y_L1_V_RNG2                   :  8;		/*  8..15, 0x0000FF00 */
    FIELD  NBC_ANR_Y_L1_V_RNG3                   :  8;		/* 16..23, 0x00FF0000 */
    FIELD  NBC_ANR_Y_L1_V_RNG4                   :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_NBC_ANR_PTY1V_T; //	/* 0x15025744 */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_PTY1V_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_PTY1V_T;

typedef struct {
    FIELD  NBC_ANR_SL2_C_GAIN                    :  4;		/*  0.. 3, 0x0000000F */
    FIELD  NBC_ANR_SL2_SCALE_GAIN                :  3;		/*  4.. 6, 0x00000070 */
    FIELD  rsv_7                                 : 25;		/*  7..31, 0xFFFFFF80 */
} ISP_DIP_X_NBC_ANR_SL2_T; //	/* 0x15025748 */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_SL2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_SL2_T;

typedef struct {
    FIELD  NBC_ANR_Y_L2_V_RNG1                   :  8;		/*  0.. 7, 0x000000FF */
    FIELD  NBC_ANR_Y_L2_V_RNG2                   :  8;		/*  8..15, 0x0000FF00 */
    FIELD  NBC_ANR_Y_L2_V_RNG3                   :  8;		/* 16..23, 0x00FF0000 */
    FIELD  NBC_ANR_Y_L2_V_RNG4                   :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_NBC_ANR_PTY2V_T; //	/* 0x1502574C */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_PTY2V_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_PTY2V_T;

typedef struct {
    FIELD  NBC_ANR_Y_L3_V_RNG1                   :  8;		/*  0.. 7, 0x000000FF */
    FIELD  NBC_ANR_Y_L3_V_RNG2                   :  8;		/*  8..15, 0x0000FF00 */
    FIELD  NBC_ANR_Y_L3_V_RNG3                   :  8;		/* 16..23, 0x00FF0000 */
    FIELD  NBC_ANR_Y_L3_V_RNG4                   :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_NBC_ANR_PTY3V_T; //	/* 0x15025750 */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_PTY3V_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_PTY3V_T;

typedef struct {
    FIELD  NBC_ANR_Y_L0_H_RNG1                   :  8;		/*  0.. 7, 0x000000FF */
    FIELD  NBC_ANR_Y_L0_H_RNG2                   :  8;		/*  8..15, 0x0000FF00 */
    FIELD  NBC_ANR_Y_L0_H_RNG3                   :  8;		/* 16..23, 0x00FF0000 */
    FIELD  NBC_ANR_Y_L0_H_RNG4                   :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_NBC_ANR_PTY0H_T; //	/* 0x15025754 */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_PTY0H_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_PTY0H_T;

typedef struct {
    FIELD  NBC_ANR_Y_L1_H_RNG1                   :  8;		/*  0.. 7, 0x000000FF */
    FIELD  NBC_ANR_Y_L1_H_RNG2                   :  8;		/*  8..15, 0x0000FF00 */
    FIELD  NBC_ANR_Y_L1_H_RNG3                   :  8;		/* 16..23, 0x00FF0000 */
    FIELD  NBC_ANR_Y_L1_H_RNG4                   :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_NBC_ANR_PTY1H_T; //	/* 0x15025758 */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_PTY1H_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_PTY1H_T;

typedef struct {
    FIELD  NBC_ANR_Y_L2_H_RNG1                   :  8;		/*  0.. 7, 0x000000FF */
    FIELD  NBC_ANR_Y_L2_H_RNG2                   :  8;		/*  8..15, 0x0000FF00 */
    FIELD  NBC_ANR_Y_L2_H_RNG3                   :  8;		/* 16..23, 0x00FF0000 */
    FIELD  NBC_ANR_Y_L2_H_RNG4                   :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_NBC_ANR_PTY2H_T; //	/* 0x1502575C */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_PTY2H_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_PTY2H_T;

typedef struct {
    FIELD  NBC_ANR_TBL_CPX1                      :  8;		/*  0.. 7, 0x000000FF */
    FIELD  NBC_ANR_TBL_CPX2                      :  8;		/*  8..15, 0x0000FF00 */
    FIELD  NBC_ANR_TBL_CPX3                      :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_NBC_ANR_T4LUT1_T; //	/* 0x15025760 */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_T4LUT1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_T4LUT1_T;

typedef struct {
    FIELD  NBC_ANR_TBL_GAIN_CPY0                 :  5;		/*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
    FIELD  NBC_ANR_TBL_GAIN_CPY1                 :  5;		/*  8..12, 0x00001F00 */
    FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
    FIELD  NBC_ANR_TBL_GAIN_CPY2                 :  5;		/* 16..20, 0x001F0000 */
    FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
    FIELD  NBC_ANR_TBL_GAIN_CPY3                 :  5;		/* 24..28, 0x1F000000 */
    FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
} ISP_DIP_X_NBC_ANR_T4LUT2_T; //	/* 0x15025764 */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_T4LUT2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_T4LUT2_T;

typedef struct {
    FIELD  NBC_ANR_TBL_GAIN_SP0                  :  5;		/*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
    FIELD  NBC_ANR_TBL_GAIN_SP1                  :  5;		/*  8..12, 0x00001F00 */
    FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
    FIELD  NBC_ANR_TBL_GAIN_SP2                  :  5;		/* 16..20, 0x001F0000 */
    FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
    FIELD  NBC_ANR_TBL_GAIN_SP3                  :  5;		/* 24..28, 0x1F000000 */
    FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
} ISP_DIP_X_NBC_ANR_T4LUT3_T; //	/* 0x15025768 */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_T4LUT3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_T4LUT3_T;

typedef struct {
    FIELD  NBC_ANR_ACT_SL2_GAIN                  :  4;		/*  0.. 3, 0x0000000F */
    FIELD  rsv_4                                 :  4;		/*  4.. 7, 0x000000F0 */
    FIELD  NBC_ANR_ACT_DIF_HI_TH                 :  5;		/*  8..12, 0x00001F00 */
    FIELD  rsv_13                                :  7;		/* 13..19, 0x000FE000 */
    FIELD  NBC_ANR_ACT_DIF_GAIN                  :  4;		/* 20..23, 0x00F00000 */
    FIELD  NBC_ANR_ACT_DIF_LO_TH                 :  5;		/* 24..28, 0x1F000000 */
    FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
} ISP_DIP_X_NBC_ANR_ACT1_T; //	/* 0x1502576C */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_ACT1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_ACT1_T;

typedef struct {
    FIELD  NBC_ANR_Y_L3_H_RNG1                   :  8;		/*  0.. 7, 0x000000FF */
    FIELD  NBC_ANR_Y_L3_H_RNG2                   :  8;		/*  8..15, 0x0000FF00 */
    FIELD  NBC_ANR_Y_L3_H_RNG3                   :  8;		/* 16..23, 0x00FF0000 */
    FIELD  NBC_ANR_Y_L3_H_RNG4                   :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_NBC_ANR_PTY3H_T; //	/* 0x15025770 */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_PTY3H_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_PTY3H_T;

typedef struct {
    FIELD  NBC_ANR_C_V_RNG1                      :  8;		/*  0.. 7, 0x000000FF */
    FIELD  NBC_ANR_C_V_RNG2                      :  8;		/*  8..15, 0x0000FF00 */
    FIELD  NBC_ANR_C_V_RNG3                      :  8;		/* 16..23, 0x00FF0000 */
    FIELD  NBC_ANR_C_V_RNG4                      :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_NBC_ANR_PTCV_T; //	/* 0x15025774 */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_PTCV_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_PTCV_T;

typedef struct {
    FIELD  NBC_ANR_Y_ACT_CEN_OFT                 :  6;		/*  0.. 5, 0x0000003F */
    FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
    FIELD  NBC_ANR_Y_ACT_CEN_GAIN                :  6;		/*  8..13, 0x00003F00 */
    FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
    FIELD  NBC_ANR_Y_ACT_CEN_TH                  :  6;		/* 16..21, 0x003F0000 */
    FIELD  rsv_22                                : 10;		/* 22..31, 0xFFC00000 */
} ISP_DIP_X_NBC_ANR_ACT4_T; //	/* 0x15025778 */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_ACT4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_ACT4_T;

typedef struct {
    FIELD  NBC_ANR_C_H_RNG1                      :  8;		/*  0.. 7, 0x000000FF */
    FIELD  NBC_ANR_C_H_RNG2                      :  8;		/*  8..15, 0x0000FF00 */
    FIELD  NBC_ANR_C_H_RNG3                      :  8;		/* 16..23, 0x00FF0000 */
    FIELD  NBC_ANR_C_H_RNG4                      :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_NBC_ANR_PTCH_T; //	/* 0x1502577C */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_PTCH_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_PTCH_T;

typedef struct {
    FIELD  NBC_ANR_Y_L0_RNG_RAT_TH               :  4;		/*  0.. 3, 0x0000000F */
    FIELD  NBC_ANR_Y_L1_RNG_RAT_TH               :  4;		/*  4.. 7, 0x000000F0 */
    FIELD  NBC_ANR_Y_L2_RNG_RAT_TH               :  4;		/*  8..11, 0x00000F00 */
    FIELD  NBC_ANR_Y_L3_RNG_RAT_TH               :  4;		/* 12..15, 0x0000F000 */
    FIELD  NBC_ANR_Y_L0_RNG_RAT_SL               :  2;		/* 16..17, 0x00030000 */
    FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
    FIELD  NBC_ANR_Y_L1_RNG_RAT_SL               :  2;		/* 20..21, 0x00300000 */
    FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
    FIELD  NBC_ANR_Y_L2_RNG_RAT_SL               :  2;		/* 24..25, 0x03000000 */
    FIELD  rsv_26                                :  2;		/* 26..27, 0x0C000000 */
    FIELD  NBC_ANR_Y_L3_RNG_RAT_SL               :  2;		/* 28..29, 0x30000000 */
    FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
} ISP_DIP_X_NBC_ANR_YLVL0_T; //	/* 0x15025780 */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_YLVL0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_YLVL0_T;

typedef struct {
    FIELD  NBC_ANR_Y_L0_HF_W                     :  5;		/*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
    FIELD  NBC_ANR_Y_L1_HF_W                     :  5;		/*  8..12, 0x00001F00 */
    FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
    FIELD  NBC_ANR_Y_L2_HF_W                     :  5;		/* 16..20, 0x001F0000 */
    FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
    FIELD  NBC_ANR_Y_L3_HF_W                     :  5;		/* 24..28, 0x1F000000 */
    FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
} ISP_DIP_X_NBC_ANR_YLVL1_T; //	/* 0x15025784 */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_YLVL1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_YLVL1_T;

typedef struct {
    FIELD  NBC_ANR_Y_HF_CORE_TH                  :  8;		/*  0.. 7, 0x000000FF */
    FIELD  NBC_ANR_Y_HF_CORE_SL                  :  2;		/*  8.. 9, 0x00000300 */
    FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
    FIELD  NBC_ANR_Y_HF_CLIP                     :  8;		/* 16..23, 0x00FF0000 */
    FIELD  NBC_ANR_HF_BAL_MODE                   :  2;		/* 24..25, 0x03000000 */
    FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
} ISP_DIP_X_NBC_ANR_HF_COR_T; //	/* 0x15025788 */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_HF_COR_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_HF_COR_T;

typedef struct {
    FIELD  NBC_ANR_Y_HF_ACT_X1                   :  8;		/*  0.. 7, 0x000000FF */
    FIELD  NBC_ANR_Y_HF_ACT_X2                   :  8;		/*  8..15, 0x0000FF00 */
    FIELD  NBC_ANR_Y_HF_ACT_X3                   :  8;		/* 16..23, 0x00FF0000 */
    FIELD  NBC_ANR_Y_HF_ACT_X4                   :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_NBC_ANR_HF_ACT0_T; //	/* 0x1502578C */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_HF_ACT0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_HF_ACT0_T;

typedef struct {
    FIELD  NBC_ANR_Y_HF_ACT_Y0                   :  7;		/*  0.. 6, 0x0000007F */
    FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
    FIELD  NBC_ANR_Y_HF_ACT_Y1                   :  7;		/*  8..14, 0x00007F00 */
    FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
    FIELD  NBC_ANR_Y_HF_ACT_Y2                   :  7;		/* 16..22, 0x007F0000 */
    FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
    FIELD  NBC_ANR_Y_HF_ACT_Y3                   :  7;		/* 24..30, 0x7F000000 */
    FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
} ISP_DIP_X_NBC_ANR_HF_ACT1_T; //	/* 0x15025790 */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_HF_ACT1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_HF_ACT1_T;

typedef struct {
    FIELD  rsv_0                                 :  8;		/*  0.. 7, 0x000000FF */
    FIELD  NBC_ANR_ACT_BLD_BASE_C                :  7;		/*  8..14, 0x00007F00 */
    FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
    FIELD  NBC_ANR_C_DITH_U                      :  5;		/* 16..20, 0x001F0000 */
    FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
    FIELD  NBC_ANR_C_DITH_V                      :  5;		/* 24..28, 0x1F000000 */
    FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
} ISP_DIP_X_NBC_ANR_ACTC_T; //	/* 0x15025794 */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_ACTC_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_ACTC_T;

typedef struct {
    FIELD  NBC_ANR_CEN_GAIN_LO_TH_LPF            :  5;		/*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
    FIELD  NBC_ANR_CEN_GAIN_HI_TH_LPF            :  5;		/*  8..12, 0x00001F00 */
    FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
} ISP_DIP_X_NBC_ANR_YLAD_T; //	/* 0x15025798 */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_YLAD_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_YLAD_T;

typedef struct {
    FIELD  NBC_ANR_Y_HF_ACT_Y4                   :  7;		/*  0.. 6, 0x0000007F */
    FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
    FIELD  NBC_ANR_Y_HF_ACT_SP4                  :  6;		/*  8..13, 0x00003F00 */
    FIELD  rsv_14                                : 18;		/* 14..31, 0xFFFFC000 */
} ISP_DIP_X_NBC_ANR_HF_ACT2_T; //	/* 0x1502579C */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_HF_ACT2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_HF_ACT2_T;

typedef struct {
    FIELD  NBC_ANR_Y_HF_ACT_SP0                  :  6;		/*  0.. 5, 0x0000003F */
    FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
    FIELD  NBC_ANR_Y_HF_ACT_SP1                  :  6;		/*  8..13, 0x00003F00 */
    FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
    FIELD  NBC_ANR_Y_HF_ACT_SP2                  :  6;		/* 16..21, 0x003F0000 */
    FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
    FIELD  NBC_ANR_Y_HF_ACT_SP3                  :  6;		/* 24..29, 0x3F000000 */
    FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
} ISP_DIP_X_NBC_ANR_HF_ACT3_T; //	/* 0x150257A0 */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_HF_ACT3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_HF_ACT3_T;

typedef struct {
    FIELD  NBC_ANR_Y_HF_LUMA_X1                  :  8;		/*  0.. 7, 0x000000FF */
    FIELD  NBC_ANR_Y_HF_LUMA_X2                  :  8;		/*  8..15, 0x0000FF00 */
    FIELD  NBC_ANR_Y_HF_LUMA_Y0                  :  7;		/* 16..22, 0x007F0000 */
    FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
    FIELD  NBC_ANR_Y_HF_LUMA_Y1                  :  7;		/* 24..30, 0x7F000000 */
    FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
} ISP_DIP_X_NBC_ANR_HF_LUMA0_T; //	/* 0x150257A4 */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_HF_LUMA0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_HF_LUMA0_T;

typedef struct {
    FIELD  NBC_ANR_Y_HF_LUMA_Y2                  :  7;		/*  0.. 6, 0x0000007F */
    FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
    FIELD  NBC_ANR_Y_HF_LUMA_SP0                 :  5;		/*  8..12, 0x00001F00 */
    FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
    FIELD  NBC_ANR_Y_HF_LUMA_SP1                 :  5;		/* 16..20, 0x001F0000 */
    FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
    FIELD  NBC_ANR_Y_HF_LUMA_SP2                 :  5;		/* 24..28, 0x1F000000 */
    FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
} ISP_DIP_X_NBC_ANR_HF_LUMA1_T; //	/* 0x150257A8 */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_HF_LUMA1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_HF_LUMA1_T;

typedef struct {
    FIELD  NBC_ANR_LTM_GAIN_S0                   :  8;		/*  0.. 7, 0x000000FF */
    FIELD  NBC_ANR_LTM_GAIN_S1                   :  8;		/*  8..15, 0x0000FF00 */
    FIELD  NBC_ANR_LTM_GAIN_S2                   :  8;		/* 16..23, 0x00FF0000 */
    FIELD  NBC_ANR_LTM_GAIN_S3                   :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_NBC_ANR_LTM_GAIN1_T; //	/* 0x150257AC */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_LTM_GAIN1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_LTM_GAIN1_T;

typedef struct {
    FIELD  NBC_ANR_LTM_GAIN_S4                   :  8;		/*  0.. 7, 0x000000FF */
    FIELD  NBC_ANR_LTM_GAIN_S5                   :  8;		/*  8..15, 0x0000FF00 */
    FIELD  NBC_ANR_LTM_GAIN_S6                   :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_NBC_ANR_LTM_GAIN2_T; //	/* 0x150257B0 */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_LTM_GAIN2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_LTM_GAIN2_T;

typedef struct {
    FIELD  NBC_ANR_LTM_P0_S4                     :  8;		/*  0.. 7, 0x000000FF */
    FIELD  NBC_ANR_LTM_P1_S4                     :  8;		/*  8..15, 0x0000FF00 */
    FIELD  NBC_ANR_LTM_P50_S4                    :  8;		/* 16..23, 0x00FF0000 */
    FIELD  NBC_ANR_LTM_P250_S4                   :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_NBC_ANR_LTM_LUTP1_T; //	/* 0x150257B4 */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_LTM_LUTP1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_LTM_LUTP1_T;

typedef struct {
    FIELD  NBC_ANR_LTM_P500_S4                   :  8;		/*  0.. 7, 0x000000FF */
    FIELD  NBC_ANR_LTM_P750_S4                   :  8;		/*  8..15, 0x0000FF00 */
    FIELD  NBC_ANR_LTM_P950_S4                   :  8;		/* 16..23, 0x00FF0000 */
    FIELD  NBC_ANR_LTM_P999_S4                   :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_NBC_ANR_LTM_LUTP2_T; //	/* 0x150257B8 */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_LTM_LUTP2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_LTM_LUTP2_T;

typedef struct {
    FIELD  NBC_ANR_LTM_O0_S4                     :  8;		/*  0.. 7, 0x000000FF */
    FIELD  NBC_ANR_LTM_O1_S4                     :  8;		/*  8..15, 0x0000FF00 */
    FIELD  NBC_ANR_LTM_O50_S4                    :  8;		/* 16..23, 0x00FF0000 */
    FIELD  NBC_ANR_LTM_O250_S4                   :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_NBC_ANR_LTM_LUTO1_T; //	/* 0x150257BC */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_LTM_LUTO1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_LTM_LUTO1_T;

typedef struct {
    FIELD  NBC_ANR_LTM_O500_S4                   :  8;		/*  0.. 7, 0x000000FF */
    FIELD  NBC_ANR_LTM_O750_S4                   :  8;		/*  8..15, 0x0000FF00 */
    FIELD  NBC_ANR_LTM_O950_S4                   :  8;		/* 16..23, 0x00FF0000 */
    FIELD  NBC_ANR_LTM_O999_S4                   :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_NBC_ANR_LTM_LUTO2_T; //	/* 0x150257C0 */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_LTM_LUTO2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_LTM_LUTO2_T;

typedef struct {
    FIELD  NBC_ANR_LTM_SP0                       : 13;		/*  0..12, 0x00001FFF */
    FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
    FIELD  NBC_ANR_LTM_SP1                       : 13;		/* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
} ISP_DIP_X_NBC_ANR_LTM_LUTS1_T; //	/* 0x150257C4 */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_LTM_LUTS1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_LTM_LUTS1_T;

typedef struct {
    FIELD  NBC_ANR_LTM_SP2                       : 13;		/*  0..12, 0x00001FFF */
    FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
    FIELD  NBC_ANR_LTM_SP3                       : 13;		/* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
} ISP_DIP_X_NBC_ANR_LTM_LUTS2_T; //	/* 0x150257C8 */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_LTM_LUTS2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_LTM_LUTS2_T;

typedef struct {
    FIELD  NBC_ANR_LTM_SP4                       : 13;		/*  0..12, 0x00001FFF */
    FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
    FIELD  NBC_ANR_LTM_SP5                       : 13;		/* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
} ISP_DIP_X_NBC_ANR_LTM_LUTS3_T; //	/* 0x150257CC */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_LTM_LUTS3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_LTM_LUTS3_T;

typedef struct {
    FIELD  rsv_0                                 : 16;		/*  0..15, 0x0000FFFF */
    FIELD  NBC_ANR_LTM_SP6                       : 13;		/* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
} ISP_DIP_X_NBC_ANR_LTM_LUTS4_T; //	/* 0x150257D0 */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_LTM_LUTS4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_LTM_LUTS4_T;

typedef struct {
    FIELD  NBC_ANR_Y_CPX5                        :  8;		/*  0.. 7, 0x000000FF */
    FIELD  NBC_ANR_Y_CPX6                        :  8;		/*  8..15, 0x0000FF00 */
    FIELD  NBC_ANR_Y_CPX7                        :  8;		/* 16..23, 0x00FF0000 */
    FIELD  NBC_ANR_Y_CPX8                        :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_NBC_ANR_Y4LUT4_T; //	/* 0x150257D4 */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_Y4LUT4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_Y4LUT4_T;

typedef struct {
    FIELD  NBC_ANR_Y_SCALE_CPY4                  :  5;		/*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
    FIELD  NBC_ANR_Y_SCALE_CPY5                  :  5;		/*  8..12, 0x00001F00 */
    FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
    FIELD  NBC_ANR_Y_SCALE_CPY6                  :  5;		/* 16..20, 0x001F0000 */
    FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
    FIELD  NBC_ANR_Y_SCALE_CPY7                  :  5;		/* 24..28, 0x1F000000 */
    FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
} ISP_DIP_X_NBC_ANR_Y4LUT5_T; //	/* 0x150257D8 */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_Y4LUT5_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_Y4LUT5_T;

typedef struct {
    FIELD  NBC_ANR_Y_SCALE_SP4                   :  5;		/*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
    FIELD  NBC_ANR_Y_SCALE_SP5                   :  5;		/*  8..12, 0x00001F00 */
    FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
    FIELD  NBC_ANR_Y_SCALE_SP6                   :  5;		/* 16..20, 0x001F0000 */
    FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
    FIELD  NBC_ANR_Y_SCALE_SP7                   :  5;		/* 24..28, 0x1F000000 */
    FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
} ISP_DIP_X_NBC_ANR_Y4LUT6_T; //	/* 0x150257DC */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_Y4LUT6_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_Y4LUT6_T;

typedef struct {
    FIELD  NBC_ANR_Y_SCALE_CPY8                  :  5;		/*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
    FIELD  NBC_ANR_Y_SCALE_SP8                   :  5;		/*  8..12, 0x00001F00 */
    FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
} ISP_DIP_X_NBC_ANR_Y4LUT7_T; //	/* 0x150257E0 */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_Y4LUT7_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_Y4LUT7_T;

typedef struct {
    FIELD  NBC_ANR_Y_ACT_CPX1                    :  8;		/*  0.. 7, 0x000000FF */
    FIELD  NBC_ANR_Y_ACT_CPX2                    :  8;		/*  8..15, 0x0000FF00 */
    FIELD  NBC_ANR_Y_ACT_CPX3                    :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_NBC_ANR_A4LUT1_T; //	/* 0x150257E4 */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_A4LUT1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_A4LUT1_T;

typedef struct {
    FIELD  NBC_ANR_RSV1                          : 32;		/*  0..31, 0xFFFFFFFF */
} ISP_DIP_X_NBC_ANR_RSV1_T; //	/* 0x150257F0 */

typedef union {
    typedef ISP_DIP_X_NBC_ANR_RSV1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC_ANR_RSV1_T;

typedef union {
    enum { COUNT = 48 };
    struct {
        ISP_NVRAM_NBC_ANR_CON1_T  con1;
        ISP_NVRAM_NBC_ANR_CON2_T  con2;
        ISP_NVRAM_NBC_ANR_YAD1_T  yad1;
        ISP_NVRAM_NBC_ANR_YAD2_T  yad2;
        ISP_NVRAM_NBC_ANR_Y4LUT1_T  y4lut1;
        ISP_NVRAM_NBC_ANR_Y4LUT2_T  y4lut2;
        ISP_NVRAM_NBC_ANR_Y4LUT3_T  y4lut3;
        ISP_NVRAM_NBC_ANR_C4LUT1_T  c4lut1;
        ISP_NVRAM_NBC_ANR_C4LUT2_T  c4lut2;
        ISP_NVRAM_NBC_ANR_C4LUT3_T  c4lut3;
        ISP_NVRAM_NBC_ANR_A4LUT2_T  a4lut2;
        ISP_NVRAM_NBC_ANR_A4LUT3_T  a4lut3;
        ISP_NVRAM_NBC_ANR_L4LUT1_T  l4lut1;
        ISP_NVRAM_NBC_ANR_L4LUT2_T  l4lut2;
        ISP_NVRAM_NBC_ANR_L4LUT3_T  l4lut3;
        ISP_NVRAM_NBC_ANR_PTY0V_T  pty0v;
        ISP_NVRAM_NBC_ANR_CAD_T  cad;
        ISP_NVRAM_NBC_ANR_PTY1V_T  pty1v;
        ISP_NVRAM_NBC_ANR_SL2_T  sl2;
        ISP_NVRAM_NBC_ANR_PTY2V_T  pty2v;
        ISP_NVRAM_NBC_ANR_PTY3V_T  pty3v;
        ISP_NVRAM_NBC_ANR_PTY0H_T  pty0h;
        ISP_NVRAM_NBC_ANR_PTY1H_T  pty1h;
        ISP_NVRAM_NBC_ANR_PTY2H_T  pty2h;
        ISP_NVRAM_NBC_ANR_T4LUT1_T  t4lut1;
        ISP_NVRAM_NBC_ANR_T4LUT2_T  t4lut2;
        ISP_NVRAM_NBC_ANR_T4LUT3_T  t4lut3;
        ISP_NVRAM_NBC_ANR_ACT1_T  act1;
        ISP_NVRAM_NBC_ANR_PTY3H_T  pty3h;
        ISP_NVRAM_NBC_ANR_PTCV_T  ptcv;
        ISP_NVRAM_NBC_ANR_ACT4_T  act4;
        ISP_NVRAM_NBC_ANR_PTCH_T  ptch;
        ISP_NVRAM_NBC_ANR_YLVL0_T  ylvl0;
        ISP_NVRAM_NBC_ANR_YLVL1_T  ylvl1;
        ISP_NVRAM_NBC_ANR_HF_COR_T  hf_cor;
        ISP_NVRAM_NBC_ANR_HF_ACT0_T  hf_act0;
        ISP_NVRAM_NBC_ANR_HF_ACT1_T  hf_act1;
        ISP_NVRAM_NBC_ANR_ACTC_T  actc;
        ISP_NVRAM_NBC_ANR_YLAD_T  ylad;
        ISP_NVRAM_NBC_ANR_HF_ACT2_T  hf_act2;
        ISP_NVRAM_NBC_ANR_HF_ACT3_T  hf_act3;
        ISP_NVRAM_NBC_ANR_HF_LUMA0_T  hf_luma0;
        ISP_NVRAM_NBC_ANR_HF_LUMA1_T  hf_luma1;
        ISP_NVRAM_NBC_ANR_Y4LUT4_T  y4lut4;
        ISP_NVRAM_NBC_ANR_Y4LUT5_T  y4lut5;
        ISP_NVRAM_NBC_ANR_Y4LUT6_T  y4lut6;
        ISP_NVRAM_NBC_ANR_Y4LUT7_T  y4lut7;
        ISP_NVRAM_NBC_ANR_A4LUT1_T  a4lut1;
        //ISP_NVRAM_NBC_ANR_RSV1_T  rsv1;
    };
	MUINT32 set[COUNT];
} ISP_NVRAM_ANR_T;

typedef union {
    enum { COUNT = 10 };
    struct {
        ISP_NVRAM_NBC_ANR_LTM_GAIN1_T  ltm_gain1;
        ISP_NVRAM_NBC_ANR_LTM_GAIN2_T  ltm_gain2;
        ISP_NVRAM_NBC_ANR_LTM_LUTP1_T  ltm_lutp1;
        ISP_NVRAM_NBC_ANR_LTM_LUTP2_T  ltm_lutp2;
        ISP_NVRAM_NBC_ANR_LTM_LUTO1_T  ltm_luto1;
        ISP_NVRAM_NBC_ANR_LTM_LUTO2_T  ltm_luto2;
        ISP_NVRAM_NBC_ANR_LTM_LUTS1_T  ltm_luts1;
        ISP_NVRAM_NBC_ANR_LTM_LUTS2_T  ltm_luts2;
        ISP_NVRAM_NBC_ANR_LTM_LUTS3_T  ltm_luts3;
        ISP_NVRAM_NBC_ANR_LTM_LUTS4_T  ltm_luts4;
    };
	MUINT32 set[COUNT];
} ISP_NVRAM_ANR_LTM_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ANR2
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct {
    FIELD  NBC2_ANR2_ENC                         :  1;		/*  0.. 0, 0x00000001 */
    FIELD  NBC2_ANR2_ENY                         :  1;		/*  1.. 1, 0x00000002 */
    FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
    FIELD  NBC2_ANR2_SCALE_MODE                  :  2;		/*  4.. 5, 0x00000030 */
    FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
    FIELD  NBC2_ANR2_VER_C_REF_Y                 :  1;		/*  8.. 8, 0x00000100 */
    FIELD  rsv_9                                 :  3;		/*  9..11, 0x00000E00 */
    FIELD  NBC2_ANR2_MODE                        :  1;		/* 12..12, 0x00001000 */
    FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
    FIELD  NBC2_ANR2_TILE_EDGE                   :  4;		/* 16..19, 0x000F0000 */
    FIELD  rsv_20                                :  4;		/* 20..23, 0x00F00000 */
    FIELD  NBC2_ANR2_SL2_LINK                    :  1;		/* 24..24, 0x01000000 */
    FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
} ISP_DIP_X_NBC2_ANR2_CON1_T; //	/* 0x15026800 */

typedef union {
    typedef ISP_DIP_X_NBC2_ANR2_CON1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC2_ANR2_CON1_T;

typedef struct {
    FIELD  rsv_0                                 :  8;		/*  0.. 7, 0x000000FF */
    FIELD  NBC2_ANR2_C_SM_EDGE                   :  2;		/*  8.. 9, 0x00000300 */
    FIELD  rsv_10                                :  2;		/* 10..11, 0x00000C00 */
    FIELD  NBC2_ANR2_FLT_C                       :  1;		/* 12..12, 0x00001000 */
    FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
} ISP_DIP_X_NBC2_ANR2_CON2_T; //	/* 0x15026804 */

typedef union {
    typedef ISP_DIP_X_NBC2_ANR2_CON2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC2_ANR2_CON2_T;

typedef struct {
    FIELD  rsv_0                                 : 28;		/*  0..27, 0x0FFFFFFF */
    FIELD  NBC2_ANR2_K_TH_C                      :  4;		/* 28..31, 0xF0000000 */
} ISP_DIP_X_NBC2_ANR2_YAD1_T; //	/* 0x15026808 */

typedef union {
    typedef ISP_DIP_X_NBC2_ANR2_YAD1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC2_ANR2_YAD1_T;

typedef struct {
    FIELD  NBC2_ANR2_Y_CPX1                      :  8;		/*  0.. 7, 0x000000FF */
    FIELD  NBC2_ANR2_Y_CPX2                      :  8;		/*  8..15, 0x0000FF00 */
    FIELD  NBC2_ANR2_Y_CPX3                      :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_NBC2_ANR2_Y4LUT1_T; //	/* 0x15026810 */

typedef union {
    typedef ISP_DIP_X_NBC2_ANR2_Y4LUT1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC2_ANR2_Y4LUT1_T;

typedef struct {
    FIELD  NBC2_ANR2_Y_SCALE_CPY0                :  5;		/*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
    FIELD  NBC2_ANR2_Y_SCALE_CPY1                :  5;		/*  8..12, 0x00001F00 */
    FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
    FIELD  NBC2_ANR2_Y_SCALE_CPY2                :  5;		/* 16..20, 0x001F0000 */
    FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
    FIELD  NBC2_ANR2_Y_SCALE_CPY3                :  5;		/* 24..28, 0x1F000000 */
    FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
} ISP_DIP_X_NBC2_ANR2_Y4LUT2_T; //	/* 0x15026814 */

typedef union {
    typedef ISP_DIP_X_NBC2_ANR2_Y4LUT2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC2_ANR2_Y4LUT2_T;

typedef struct {
    FIELD  NBC2_ANR2_Y_SCALE_SP0                 :  5;		/*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
    FIELD  NBC2_ANR2_Y_SCALE_SP1                 :  5;		/*  8..12, 0x00001F00 */
    FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
    FIELD  NBC2_ANR2_Y_SCALE_SP2                 :  5;		/* 16..20, 0x001F0000 */
    FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
    FIELD  NBC2_ANR2_Y_SCALE_SP3                 :  5;		/* 24..28, 0x1F000000 */
    FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
} ISP_DIP_X_NBC2_ANR2_Y4LUT3_T; //	/* 0x15026818 */

typedef union {
    typedef ISP_DIP_X_NBC2_ANR2_Y4LUT3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC2_ANR2_Y4LUT3_T;

typedef struct {
    FIELD  NBC2_ANR2_SL2_X1                      :  8;		/*  0.. 7, 0x000000FF */
    FIELD  NBC2_ANR2_SL2_X2                      :  8;		/*  8..15, 0x0000FF00 */
    FIELD  NBC2_ANR2_SL2_X3                      :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_NBC2_ANR2_L4LUT1_T; //	/* 0x15026830 */

typedef union {
    typedef ISP_DIP_X_NBC2_ANR2_L4LUT1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC2_ANR2_L4LUT1_T;

typedef struct {
    FIELD  NBC2_ANR2_SL2_GAIN0                   :  6;		/*  0.. 5, 0x0000003F */
    FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
    FIELD  NBC2_ANR2_SL2_GAIN1                   :  6;		/*  8..13, 0x00003F00 */
    FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
    FIELD  NBC2_ANR2_SL2_GAIN2                   :  6;		/* 16..21, 0x003F0000 */
    FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
    FIELD  NBC2_ANR2_SL2_GAIN3                   :  6;		/* 24..29, 0x3F000000 */
    FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
} ISP_DIP_X_NBC2_ANR2_L4LUT2_T; //	/* 0x15026834 */

typedef union {
    typedef ISP_DIP_X_NBC2_ANR2_L4LUT2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC2_ANR2_L4LUT2_T;

typedef struct {
    FIELD  NBC2_ANR2_SL2_SP0                     :  6;		/*  0.. 5, 0x0000003F */
    FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
    FIELD  NBC2_ANR2_SL2_SP1                     :  6;		/*  8..13, 0x00003F00 */
    FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
    FIELD  NBC2_ANR2_SL2_SP2                     :  6;		/* 16..21, 0x003F0000 */
    FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
    FIELD  NBC2_ANR2_SL2_SP3                     :  6;		/* 24..29, 0x3F000000 */
    FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
} ISP_DIP_X_NBC2_ANR2_L4LUT3_T; //	/* 0x15026838 */

typedef union {
    typedef ISP_DIP_X_NBC2_ANR2_L4LUT3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC2_ANR2_L4LUT3_T;

typedef struct {
    FIELD  NBC2_ANR2_PTC_VGAIN                   :  4;		/*  0.. 3, 0x0000000F */
    FIELD  NBC2_ANR2_C_GAIN                      :  4;		/*  4.. 7, 0x000000F0 */
    FIELD  NBC2_ANR2_PTC_GAIN_TH                 :  5;		/*  8..12, 0x00001F00 */
    FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
    FIELD  NBC2_ANR2_C_L_DIFF_TH                 :  8;		/* 16..23, 0x00FF0000 */
    FIELD  NBC2_ANR2_C_MODE                      :  1;		/* 24..24, 0x01000000 */
    FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
} ISP_DIP_X_NBC2_ANR2_CAD_T; //	/* 0x15026840 */

typedef union {
    typedef ISP_DIP_X_NBC2_ANR2_CAD_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC2_ANR2_CAD_T;

typedef struct {
    FIELD  NBC2_ANR2_PTC1                        :  8;		/*  0.. 7, 0x000000FF */
    FIELD  NBC2_ANR2_PTC2                        :  8;		/*  8..15, 0x0000FF00 */
    FIELD  NBC2_ANR2_PTC3                        :  8;		/* 16..23, 0x00FF0000 */
    FIELD  NBC2_ANR2_PTC4                        :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_NBC2_ANR2_PTC_T; //	/* 0x15026844 */

typedef union {
    typedef ISP_DIP_X_NBC2_ANR2_PTC_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC2_ANR2_PTC_T;

typedef struct {
    FIELD  NBC2_ANR2_SL2_C_GAIN                  :  4;		/*  0.. 3, 0x0000000F */
    FIELD  rsv_4                                 : 12;		/*  4..15, 0x0000FFF0 */
    FIELD  NBC2_ANR2_LM_WT                       :  4;		/* 16..19, 0x000F0000 */
    FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
} ISP_DIP_X_NBC2_ANR2_SL2_T; //	/* 0x15026848 */

typedef union {
    typedef ISP_DIP_X_NBC2_ANR2_SL2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC2_ANR2_SL2_T;

typedef struct {
    FIELD  NBC2_ANR2_COR_TH                      :  5;		/*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
    FIELD  NBC2_ANR2_COR_SL                      :  3;		/*  8..10, 0x00000700 */
    FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
    FIELD  NBC2_ANR2_MCD_TH                      :  5;		/* 12..16, 0x0001F000 */
    FIELD  rsv_17                                :  3;		/* 17..19, 0x000E0000 */
    FIELD  NBC2_ANR2_MCD_SL                      :  3;		/* 20..22, 0x00700000 */
    FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
    FIELD  NBC2_ANR2_LCL_TH                      :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_NBC2_ANR2_MED1_T; //	/* 0x1502684C */

typedef union {
    typedef ISP_DIP_X_NBC2_ANR2_MED1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC2_ANR2_MED1_T;

typedef struct {
    FIELD  NBC2_ANR2_LCL_SL                      :  3;		/*  0.. 2, 0x00000007 */
    FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
    FIELD  NBC2_ANR2_LCL_LV                      :  5;		/*  4.. 8, 0x000001F0 */
    FIELD  rsv_9                                 :  3;		/*  9..11, 0x00000E00 */
    FIELD  NBC2_ANR2_SCL_TH                      :  6;		/* 12..17, 0x0003F000 */
    FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
    FIELD  NBC2_ANR2_SCL_SL                      :  3;		/* 20..22, 0x00700000 */
    FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
    FIELD  NBC2_ANR2_SCL_LV                      :  5;		/* 24..28, 0x1F000000 */
    FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
} ISP_DIP_X_NBC2_ANR2_MED2_T; //	/* 0x15026850 */

typedef union {
    typedef ISP_DIP_X_NBC2_ANR2_MED2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC2_ANR2_MED2_T;

typedef struct {
    FIELD  NBC2_ANR2_NCL_TH                      :  8;		/*  0.. 7, 0x000000FF */
    FIELD  NBC2_ANR2_NCL_SL                      :  3;		/*  8..10, 0x00000700 */
    FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
    FIELD  NBC2_ANR2_NCL_LV                      :  5;		/* 12..16, 0x0001F000 */
    FIELD  rsv_17                                :  3;		/* 17..19, 0x000E0000 */
    FIELD  NBC2_ANR2_VAR                         :  3;		/* 20..22, 0x00700000 */
    FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
    FIELD  NBC2_ANR2_Y0                          :  6;		/* 24..29, 0x3F000000 */
    FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
} ISP_DIP_X_NBC2_ANR2_MED3_T; //	/* 0x15026854 */

typedef union {
    typedef ISP_DIP_X_NBC2_ANR2_MED3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC2_ANR2_MED3_T;

typedef struct {
    FIELD  NBC2_ANR2_Y1                          :  6;		/*  0.. 5, 0x0000003F */
    FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
    FIELD  NBC2_ANR2_Y2                          :  6;		/*  8..13, 0x00003F00 */
    FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
    FIELD  NBC2_ANR2_Y3                          :  6;		/* 16..21, 0x003F0000 */
    FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
    FIELD  NBC2_ANR2_Y4                          :  6;		/* 24..29, 0x3F000000 */
    FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
} ISP_DIP_X_NBC2_ANR2_MED4_T; //	/* 0x15026858 */

typedef union {
    typedef ISP_DIP_X_NBC2_ANR2_MED4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC2_ANR2_MED4_T;

typedef struct {
    FIELD  NBC2_ANR2_LCL_OFT                     :  8;		/*  0.. 7, 0x000000FF */
    FIELD  NBC2_ANR2_SCL_OFT                     :  6;		/*  8..13, 0x00003F00 */
    FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
    FIELD  NBC2_ANR2_NCL_OFT                     :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_NBC2_ANR2_MED5_T; //	/* 0x1502685C */

typedef union {
    typedef ISP_DIP_X_NBC2_ANR2_MED5_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC2_ANR2_MED5_T;

typedef struct {
    FIELD  rsv_0                                 :  8;		/*  0.. 7, 0x000000FF */
    FIELD  NBC2_ANR2_ACT_BLD_BASE_C              :  7;		/*  8..14, 0x00007F00 */
    FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
    FIELD  NBC2_ANR2_C_DITH_U                    :  5;		/* 16..20, 0x001F0000 */
    FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
    FIELD  NBC2_ANR2_C_DITH_V                    :  5;		/* 24..28, 0x1F000000 */
    FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
} ISP_DIP_X_NBC2_ANR2_ACTC_T; //	/* 0x15026874 */

typedef union {
    typedef ISP_DIP_X_NBC2_ANR2_ACTC_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC2_ANR2_ACTC_T;

typedef struct {
    FIELD  NBC2_ANR2_RSV1                        : 32;		/*  0..31, 0xFFFFFFFF */
} ISP_DIP_X_NBC2_ANR2_RSV1_T; //	/* 0x15026878 */

typedef union {
    typedef ISP_DIP_X_NBC2_ANR2_RSV1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC2_ANR2_RSV1_T;

typedef struct {
    FIELD  NBC2_ANR2_RSV2                        : 32;		/*  0..31, 0xFFFFFFFF */
} ISP_DIP_X_NBC2_ANR2_RSV2_T; //	/* 0x1502687C */

typedef union {
    typedef ISP_DIP_X_NBC2_ANR2_RSV2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC2_ANR2_RSV2_T;

typedef union {
    enum { COUNT = 18 };
    struct {
        ISP_NVRAM_NBC2_ANR2_CON1_T  con1;
        ISP_NVRAM_NBC2_ANR2_CON2_T  con2;
        ISP_NVRAM_NBC2_ANR2_YAD1_T  yad1;
        ISP_NVRAM_NBC2_ANR2_Y4LUT1_T  y4lut1;
        ISP_NVRAM_NBC2_ANR2_Y4LUT2_T  y4lut2;
        ISP_NVRAM_NBC2_ANR2_Y4LUT3_T  y4lut3;
        ISP_NVRAM_NBC2_ANR2_L4LUT1_T  l4lut1;
        ISP_NVRAM_NBC2_ANR2_L4LUT2_T  l4lut2;
        ISP_NVRAM_NBC2_ANR2_L4LUT3_T  l4lut3;
        ISP_NVRAM_NBC2_ANR2_CAD_T  cad;
        ISP_NVRAM_NBC2_ANR2_PTC_T  ptc;
        ISP_NVRAM_NBC2_ANR2_SL2_T  sl2;
        ISP_NVRAM_NBC2_ANR2_MED1_T  med1;
        ISP_NVRAM_NBC2_ANR2_MED2_T  med2;
        ISP_NVRAM_NBC2_ANR2_MED3_T  med3;
        ISP_NVRAM_NBC2_ANR2_MED4_T  med4;
        ISP_NVRAM_NBC2_ANR2_MED5_T  med5;
        ISP_NVRAM_NBC2_ANR2_ACTC_T  actc;
        //ISP_NVRAM_NBC2_ANR2_RSV1_T  rsv1;
        //ISP_NVRAM_NBC2_ANR2_RSV2_T  rsv2;
    };
    MUINT32 set[COUNT];
} ISP_NVRAM_ANR2_T;



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CCR
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef struct {
    FIELD  NBC2_CCR_EN                           :  1;		/*  0.. 0, 0x00000001 */
    FIELD  NBC2_CCR_SL2_LINK                     :  1;		/*  1.. 1, 0x00000002 */
    FIELD  NBC2_CCR_SL2_MODE                     :  2;		/*  2.. 3, 0x0000000C */
    FIELD  rsv_4                                 :  1;    /*  4.. 4, 0x00000010 */
    FIELD  NBC2_CCR_OR_MODE                      :  1;		/*  5.. 5, 0x00000020 */
    FIELD  rsv_6                                 :  2;    /*  6.. 7, 0x000000C0 */
    FIELD  NBC2_CCR_UV_GAIN_MODE                 :  1;		/*  8.. 8, 0x00000100 */
    FIELD  rsv_9                                 :  7;    /*  9..15, 0x0000FE00 */
    FIELD  NBC2_CCR_UV_GAIN2                     :  7;		/* 16..22, 0x007F0000 */
    FIELD  rsv_23                                :  1;    /* 23..23, 0x00800000 */
    FIELD  NBC2_CCR_Y_CPX3                       :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_NBC2_CCR_CON_T; //	/* 0x15026880 */

typedef union {
    typedef ISP_DIP_X_NBC2_CCR_CON_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC2_CCR_CON_T;

typedef struct {
    FIELD  NBC2_CCR_Y_CPX1                       :  8;		/*  0.. 7, 0x000000FF */
    FIELD  NBC2_CCR_Y_CPX2                       :  8;		/*  8..15, 0x0000FF00 */
    FIELD  NBC2_CCR_Y_SP1                        :  7;		/* 16..22, 0x007F0000 */
    FIELD  rsv_23                                :  1;    /* 23..23, 0x00800000 */
    FIELD  NBC2_CCR_Y_CPY1                       :  7;		/* 24..30, 0x7F000000 */
    FIELD  rsv_31                                :  1;    /* 31..31, 0x80000000 */
} ISP_DIP_X_NBC2_CCR_YLUT_T; //	/* 0x15026884 */

typedef union {
    typedef ISP_DIP_X_NBC2_CCR_YLUT_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC2_CCR_YLUT_T;

typedef struct {
    FIELD  NBC2_CCR_UV_X1                        :  8;		/*  0.. 7, 0x000000FF */
    FIELD  NBC2_CCR_UV_X2                        :  8;		/*  8..15, 0x0000FF00 */
    FIELD  NBC2_CCR_UV_X3                        :  8;		/* 16..23, 0x00FF0000 */
    FIELD  NBC2_CCR_UV_GAIN1                     :  7;		/* 24..30, 0x7F000000 */
    FIELD  rsv_31                                :  1;    /* 31..31, 0x80000000 */
} ISP_DIP_X_NBC2_CCR_UVLUT_T; //	/* 0x15026888 */

typedef union {
    typedef ISP_DIP_X_NBC2_CCR_UVLUT_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC2_CCR_UVLUT_T;

typedef struct {
    FIELD  NBC2_CCR_Y_SP0                        :  7;		/*  0.. 6, 0x0000007F */
    FIELD  rsv_7                                 :  1;    /*  7.. 7, 0x00000080 */
    FIELD  NBC2_CCR_Y_SP2                        :  7;		/*  8..14, 0x00007F00 */
    FIELD  rsv_15                                :  1;    /* 15..15, 0x00008000 */
    FIELD  NBC2_CCR_Y_CPY0                       :  7;		/* 16..22, 0x007F0000 */
    FIELD  rsv_23                                :  1;    /* 23..23, 0x00800000 */
    FIELD  NBC2_CCR_Y_CPY2                       :  7;		/* 24..30, 0x7F000000 */
    FIELD  rsv_31                                :  1;    /* 31..31, 0x80000000 */
} ISP_DIP_X_NBC2_CCR_YLUT2_T; //	/* 0x1502688C */

typedef union {
    typedef ISP_DIP_X_NBC2_CCR_YLUT2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC2_CCR_YLUT2_T;

typedef struct {
    FIELD  NBC2_CCR_MODE                         :  1;		/*  0.. 0, 0x00000001 */
    FIELD  rsv_1                                 :  3;    /*  1.. 3, 0x0000000E */
    FIELD  NBC2_CCR_CEN_U                        :  6;		/*  4.. 9, 0x000003F0 */
    FIELD  rsv_10                                :  2;    /* 10..11, 0x00000C00 */
    FIELD  NBC2_CCR_CEN_V                        :  6;		/* 12..17, 0x0003F000 */
    FIELD  rsv_18                                : 14;    /* 18..31, 0xFFFC0000 */
} ISP_DIP_X_NBC2_CCR_SAT_CTRL_T; //	/* 0x15026890 */

typedef union {
    typedef ISP_DIP_X_NBC2_CCR_SAT_CTRL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC2_CCR_SAT_CTRL_T;

typedef struct {
    FIELD  NBC2_CCR_UV_GAIN_SP1                  : 10;		/*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                :  6;    /* 10..15, 0x0000FC00 */
    FIELD  NBC2_CCR_UV_GAIN_SP2                  : 10;		/* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                :  6;    /* 26..31, 0xFC000000 */
} ISP_DIP_X_NBC2_CCR_UVLUT_SP_T; //	/* 0x15026894 */

typedef union {
    typedef ISP_DIP_X_NBC2_CCR_UVLUT_SP_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC2_CCR_UVLUT_SP_T;

typedef struct {
    FIELD  NBC2_CCR_HUE_X1                       :  9;		/*  0.. 8, 0x000001FF */
    FIELD  rsv_9                                 :  7;    /*  9..15, 0x0000FE00 */
    FIELD  NBC2_CCR_HUE_X2                       :  9;		/* 16..24, 0x01FF0000 */
    FIELD  rsv_25                                :  7;    /* 25..31, 0xFE000000 */
} ISP_DIP_X_NBC2_CCR_HUE1_T; //	/* 0x15026898 */

typedef union {
    typedef ISP_DIP_X_NBC2_CCR_HUE1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC2_CCR_HUE1_T;

typedef struct {
    FIELD  NBC2_CCR_HUE_X3                       :  9;		/*  0.. 8, 0x000001FF */
    FIELD  rsv_9                                 :  7;    /*  9..15, 0x0000FE00 */
    FIELD  NBC2_CCR_HUE_X4                       :  9;		/* 16..24, 0x01FF0000 */
    FIELD  rsv_25                                :  7;    /* 25..31, 0xFE000000 */
} ISP_DIP_X_NBC2_CCR_HUE2_T; //	/* 0x1502689C */

typedef union {
    typedef ISP_DIP_X_NBC2_CCR_HUE2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC2_CCR_HUE2_T;

typedef struct {
    FIELD  NBC2_CCR_HUE_SP1                      :  8;		/*  0.. 7, 0x000000FF */
    FIELD  NBC2_CCR_HUE_SP2                      :  8;		/*  8..15, 0x0000FF00 */
    FIELD  NBC2_CCR_HUE_GAIN1                    :  7;		/* 16..22, 0x007F0000 */
    FIELD  rsv_23                                :  1;    /* 23..23, 0x00800000 */
    FIELD  NBC2_CCR_HUE_GAIN2                    :  7;		/* 24..30, 0x7F000000 */
    FIELD  rsv_31                                :  1;    /* 31..31, 0x80000000 */
} ISP_DIP_X_NBC2_CCR_HUE3_T; //	/* 0x150268A0 */

typedef union {
    typedef ISP_DIP_X_NBC2_CCR_HUE3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC2_CCR_HUE3_T;

typedef struct {
    FIELD  NBC2_CCR_SL2_X1                       :  8;		/*  0.. 7, 0x000000FF */
    FIELD  NBC2_CCR_SL2_X2                       :  8;		/*  8..15, 0x0000FF00 */
    FIELD  NBC2_CCR_SL2_X3                       :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                :  8;    /* 24..31, 0xFF000000 */
} ISP_DIP_X_NBC2_CCR_L4LUT1_T; //	/* 0x150268A4 */

typedef union {
    typedef ISP_DIP_X_NBC2_CCR_L4LUT1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC2_CCR_L4LUT1_T;

typedef struct {
    FIELD  NBC2_CCR_SL2_GAIN0                    :  6;		/*  0.. 5, 0x0000003F */
    FIELD  rsv_6                                 :  2;    /*  6.. 7, 0x000000C0 */
    FIELD  NBC2_CCR_SL2_GAIN1                    :  6;		/*  8..13, 0x00003F00 */
    FIELD  rsv_14                                :  2;    /* 14..15, 0x0000C000 */
    FIELD  NBC2_CCR_SL2_GAIN2                    :  6;		/* 16..21, 0x003F0000 */
    FIELD  rsv_22                                :  2;    /* 22..23, 0x00C00000 */
    FIELD  NBC2_CCR_SL2_GAIN3                    :  6;		/* 24..29, 0x3F000000 */
    FIELD  rsv_30                                :  2;    /* 30..31, 0xC0000000 */
} ISP_DIP_X_NBC2_CCR_L4LUT2_T; //	/* 0x150268A8 */

typedef union {
    typedef ISP_DIP_X_NBC2_CCR_L4LUT2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC2_CCR_L4LUT2_T;

typedef struct {
    FIELD  NBC2_CCR_SL2_SP0                      :  6;		/*  0.. 5, 0x0000003F */
    FIELD  rsv_6                                 :  2;    /*  6.. 7, 0x000000C0 */
    FIELD  NBC2_CCR_SL2_SP1                      :  6;		/*  8..13, 0x00003F00 */
    FIELD  rsv_14                                :  2;    /* 14..15, 0x0000C000 */
    FIELD  NBC2_CCR_SL2_SP2                      :  6;		/* 16..21, 0x003F0000 */
    FIELD  rsv_22                                :  2;    /* 22..23, 0x00C00000 */
    FIELD  NBC2_CCR_SL2_SP3                      :  6;		/* 24..29, 0x3F000000 */
    FIELD  rsv_30                                :  2;    /* 30..31, 0xC0000000 */
} ISP_DIP_X_NBC2_CCR_L4LUT3_T; //	/* 0x150268AC */

typedef union {
    typedef ISP_DIP_X_NBC2_CCR_L4LUT3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC2_CCR_L4LUT3_T;

typedef union {
    enum { COUNT = 12 };
    struct {
        ISP_NVRAM_NBC2_CCR_CON_T  con;
        ISP_NVRAM_NBC2_CCR_YLUT_T  ylut;
        ISP_NVRAM_NBC2_CCR_UVLUT_T  uvlut;
        ISP_NVRAM_NBC2_CCR_YLUT2_T  ylut2;
        ISP_NVRAM_NBC2_CCR_SAT_CTRL_T  sat_ctrl;
        ISP_NVRAM_NBC2_CCR_UVLUT_SP_T  uvlut_sp;
        ISP_NVRAM_NBC2_CCR_HUE1_T  hue1;
        ISP_NVRAM_NBC2_CCR_HUE2_T  hue2;
        ISP_NVRAM_NBC2_CCR_HUE3_T  hue3;
        ISP_NVRAM_NBC2_CCR_L4LUT1_T  l4lut1;
        ISP_NVRAM_NBC2_CCR_L4LUT2_T  l4lut2;
        ISP_NVRAM_NBC2_CCR_L4LUT3_T  l4lut3;
    };
    MUINT32 set[COUNT];
} ISP_NVRAM_CCR_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BOK
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// DIP_X_BOK_CON CAM+A94H
typedef struct {
    FIELD  NBC2_BOK_MODE                         :  1;		/*  0.. 0, 0x00000001 */
    FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
    FIELD  NBC2_BOK_AP_MODE                      :  1;		/*  4.. 4, 0x00000010 */
    FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
    FIELD  NBC2_BOK_FGBG_MODE                    :  1;		/*  8.. 8, 0x00000100 */
    FIELD  rsv_9                                 :  3;		/*  9..11, 0x00000E00 */
    FIELD  NBC2_BOK_FGBG_WT                      :  4;		/* 12..15, 0x0000F000 */
    FIELD  NBC2_BOK_PF_EN                        :  1;		/* 16..16, 0x00010000 */
    FIELD  rsv_17                                : 15;		/* 17..31, 0xFFFE0000 */
} ISP_DIP_X_NBC2_BOK_CON_T; //	/* 0x150268B0 */

typedef union {
    typedef ISP_DIP_X_NBC2_BOK_CON_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC2_BOK_CON_T;

typedef struct {
    FIELD  NBC2_BOK_STR_WT                       :  6;		/*  0.. 5, 0x0000003F */
    FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
    FIELD  NBC2_BOK_WT_GAIN                      :  6;		/*  8..13, 0x00003F00 */
    FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
    FIELD  NBC2_BOK_INTENSITY                    :  8;		/* 16..23, 0x00FF0000 */
    FIELD  NBC2_BOK_DOF_M                        :  7;		/* 24..30, 0x7F000000 */
    FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
} ISP_DIP_X_NBC2_BOK_TUN_T; //	/* 0x150268B4 */

typedef union {
    typedef ISP_DIP_X_NBC2_BOK_TUN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC2_BOK_TUN_T;

typedef struct {
    FIELD  NBC2_BOK_XOFF                         :  3;		/*  0.. 2, 0x00000007 */
    FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
    FIELD  NBC2_BOK_YOFF                         :  3;		/*  4.. 6, 0x00000070 */
    FIELD  rsv_7                                 : 25;		/*  7..31, 0xFFFFFF80 */
} ISP_DIP_X_NBC2_BOK_OFF_T; //	/* 0x150268B8 */

typedef union {
    typedef ISP_DIP_X_NBC2_BOK_OFF_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC2_BOK_OFF_T;

typedef struct {
    FIELD  NBC2_BOK_RSV1                         : 32;		/*  0..31, 0xFFFFFFFF */
} ISP_DIP_X_NBC2_BOK_RSV1_T; //	/* 0x150268BC */

typedef union {
    typedef ISP_DIP_X_NBC2_BOK_RSV1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC2_BOK_RSV1_T;

typedef union {
    enum { COUNT = 4 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_NBC2_BOK_CON_T  bok_con;
        ISP_NVRAM_NBC2_BOK_TUN_T  bok_tun;
        //ISP_NVRAM_NBC2_BOK_OFF_T  bok_off;
        //ISP_NVRAM_NBC2_BOK_RSV1_T  bok_rsv1;
    };
} ISP_NVRAM_BOK_T;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// HFG
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct {
    FIELD  DIP_HFC_STD                           :  8;    /*  0.. 7, 0x000000FF */
    FIELD  HFC_LCE_LINK_EN                       :  1;		/*  8.. 8, 0x00000100 */
    FIELD  rsv_9                                 : 23;		/*  9..31, 0xFFFFFE00 */
} ISP_DIP_X_HFG_CON_0_T; // /* 0x15024630 */

typedef union {
    typedef ISP_DIP_X_HFG_CON_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_HFG_CON_0_T;

typedef struct {
    FIELD  HFC_LUMA_CPX1                         :  8;    /*  0.. 7, 0x000000FF */
    FIELD  HFC_LUMA_CPX2                         :  8;    /*  8..15, 0x0000FF00 */
    FIELD  HFC_LUMA_CPX3                         :  8;    /* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                :  8;    /* 24..31, 0xFF000000 */
} ISP_DIP_X_HFG_LUMA_0_T; //  /* 0x15024634 */

typedef union {
    typedef ISP_DIP_X_HFG_LUMA_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_HFG_LUMA_0_T;

typedef struct {
    FIELD  HFC_LUMA_CPY0                         :  6;    /*  0.. 5, 0x0000003F */
    FIELD  rsv_6                                 :  2;    /*  6.. 7, 0x000000C0 */
    FIELD  HFC_LUMA_CPY1                         :  6;    /*  8..13, 0x00003F00 */
    FIELD  rsv_14                                :  2;    /* 14..15, 0x0000C000 */
    FIELD  HFC_LUMA_CPY2                         :  6;    /* 16..21, 0x003F0000 */
    FIELD  rsv_22                                :  2;    /* 22..23, 0x00C00000 */
    FIELD  HFC_LUMA_CPY3                         :  6;    /* 24..29, 0x3F000000 */
    FIELD  rsv_30                                :  2;    /* 30..31, 0xC0000000 */
} ISP_DIP_X_HFG_LUMA_1_T; //  /* 0x15024638 */

typedef union {
    typedef ISP_DIP_X_HFG_LUMA_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_HFG_LUMA_1_T;

typedef struct {
    FIELD  HFC_LUMA_SP0                          :  5;    /*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                 :  3;    /*  5.. 7, 0x000000E0 */
    FIELD  HFC_LUMA_SP1                          :  5;    /*  8..12, 0x00001F00 */
    FIELD  rsv_13                                :  3;    /* 13..15, 0x0000E000 */
    FIELD  HFC_LUMA_SP2                          :  5;    /* 16..20, 0x001F0000 */
    FIELD  rsv_21                                :  3;    /* 21..23, 0x00E00000 */
    FIELD  HFC_LUMA_SP3                          :  5;    /* 24..28, 0x1F000000 */
    FIELD  rsv_29                                :  3;    /* 29..31, 0xE0000000 */
} ISP_DIP_X_HFG_LUMA_2_T; //  /* 0x1502463C */

typedef union {
    typedef ISP_DIP_X_HFG_LUMA_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_HFG_LUMA_2_T;

typedef struct {
    FIELD  HFC_LCE_CPX1                          :  8;    /*  0.. 7, 0x000000FF */
    FIELD  HFC_LCE_CPX2                          :  8;    /*  8..15, 0x0000FF00 */
    FIELD  HFC_LCE_CPX3                          :  8;    /* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                :  8;    /* 24..31, 0xFF000000 */
} ISP_DIP_X_HFG_LCE_0_T; // /* 0x15024640 */

typedef union {
    typedef ISP_DIP_X_HFG_LCE_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_HFG_LCE_0_T;

typedef struct {
    FIELD  HFC_LCE_CPY0                          :  6;    /*  0.. 5, 0x0000003F */
    FIELD  rsv_6                                 :  2;    /*  6.. 7, 0x000000C0 */
    FIELD  HFC_LCE_CPY1                          :  6;    /*  8..13, 0x00003F00 */
    FIELD  rsv_14                                :  2;    /* 14..15, 0x0000C000 */
    FIELD  HFC_LCE_CPY2                          :  6;    /* 16..21, 0x003F0000 */
    FIELD  rsv_22                                :  2;    /* 22..23, 0x00C00000 */
    FIELD  HFC_LCE_CPY3                          :  6;    /* 24..29, 0x3F000000 */
    FIELD  rsv_30                                :  2;    /* 30..31, 0xC0000000 */
} ISP_DIP_X_HFG_LCE_1_T; // /* 0x15024644 */

typedef union {
    typedef ISP_DIP_X_HFG_LCE_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_HFG_LCE_1_T;

typedef struct {
    FIELD  HFC_LCE_SP0                           :  5;    /*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                 :  3;    /*  5.. 7, 0x000000E0 */
    FIELD  HFC_LCE_SP1                           :  5;    /*  8..12, 0x00001F00 */
    FIELD  rsv_13                                :  3;    /* 13..15, 0x0000E000 */
    FIELD  HFC_LCE_SP2                           :  5;    /* 16..20, 0x001F0000 */
    FIELD  rsv_21                                :  3;    /* 21..23, 0x00E00000 */
    FIELD  HFC_LCE_SP3                           :  5;    /* 24..28, 0x1F000000 */
    FIELD  rsv_29                                :  3;    /* 29..31, 0xE0000000 */
} ISP_DIP_X_HFG_LCE_2_T; // /* 0x15024648 */

typedef union {
    typedef ISP_DIP_X_HFG_LCE_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_HFG_LCE_2_T;

typedef struct {
    FIELD  RANSED_GSEED                          : 16;    /*  0..15, 0x0000FFFF */
    FIELD  RANSED_SEED0                          : 16;    /* 16..31, 0xFFFF0000 */
} ISP_DIP_X_HFG_RAN_0_T; // /* 0x1502464C */

typedef union {
    typedef ISP_DIP_X_HFG_RAN_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_HFG_RAN_0_T;

typedef struct {
    FIELD  RANSED_SEED1                          : 16;    /*  0..15, 0x0000FFFF */
    FIELD  RANSED_SEED2                          : 16;    /* 16..31, 0xFFFF0000 */
} ISP_DIP_X_HFG_RAN_1_T; // /* 0x15024650 */

typedef union {
    typedef ISP_DIP_X_HFG_RAN_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_HFG_RAN_1_T;

typedef struct {
    FIELD  RANSED_XOSFS                          : 16;    /*  0..15, 0x0000FFFF */
    FIELD  RANSED_YOSFS                          : 16;    /* 16..31, 0xFFFF0000 */
} ISP_DIP_X_HFG_RAN_2_T; // /* 0x15024654 */

typedef union {
    typedef ISP_DIP_X_HFG_RAN_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_HFG_RAN_2_T;

typedef struct {
    FIELD  RANSED_IMG_WD                         : 16;    /*  0..15, 0x0000FFFF */
    FIELD  rsv_16                                : 16;    /* 16..31, 0xFFFF0000 */
} ISP_DIP_X_HFG_RAN_3_T; // /* 0x15024658 */

typedef union {
    typedef ISP_DIP_X_HFG_RAN_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_HFG_RAN_3_T;

typedef struct {
    FIELD  HFG_T_WIDTH                           : 16;    /*  0..15, 0x0000FFFF */
    FIELD  HFG_T_HEIGHT                          : 16;    /* 16..31, 0xFFFF0000 */
} ISP_DIP_X_HFG_RAN_4_T; // /* 0x1502465C */

typedef union {
    typedef ISP_DIP_X_HFG_RAN_4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_HFG_RAN_4_T;

typedef struct {
    FIELD  HFG_CROP_STR_X                        : 16;    /*  0..15, 0x0000FFFF */
    FIELD  HFG_CROP_END_X                        : 16;    /* 16..31, 0xFFFF0000 */
} ISP_DIP_X_HFG_CROP_X_T; //  /* 0x15024660 */

typedef union {
    typedef ISP_DIP_X_HFG_CROP_X_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_HFG_CROP_X_T;

typedef struct {
    FIELD  HFG_CROP_STR_Y                        : 16;    /*  0..15, 0x0000FFFF */
    FIELD  HFG_CROP_END_Y                        : 16;    /* 16..31, 0xFFFF0000 */
} ISP_DIP_X_HFG_CROP_Y_T; //  /* 0x15024664 */

typedef union {
    typedef ISP_DIP_X_HFG_CROP_Y_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_HFG_CROP_Y_T;

typedef union {
    enum { COUNT = 7 };
    struct {
        ISP_NVRAM_HFG_CON_0_T   con_0;
        ISP_NVRAM_HFG_LUMA_0_T  luma_0;
        ISP_NVRAM_HFG_LUMA_1_T  luma_1;
        ISP_NVRAM_HFG_LUMA_2_T  luma_2;
        ISP_NVRAM_HFG_LCE_0_T   lce_0;
        ISP_NVRAM_HFG_LCE_1_T   lce_1;
        ISP_NVRAM_HFG_LCE_2_T   lce_2;
        //ISP_NVRAM_HFG_RAN_0_T   ran_0;
        //ISP_NVRAM_HFG_RAN_1_T   ran_1;
        //ISP_NVRAM_HFG_RAN_2_T   ran_2;
        //ISP_NVRAM_HFG_RAN_3_T   ran_3;
        //ISP_NVRAM_HFG_RAN_4_T   ran_4;
        //ISP_NVRAM_HFG_CROP_X_T  crop_x;
        //ISP_NVRAM_HFG_CROP_Y_T  crop_y;
    };
    MUINT32 set[COUNT];
} ISP_NVRAM_HFG_T;

typedef union {
    enum { COUNT = 10 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_HFG_CON_0_T   con_0;
        ISP_NVRAM_HFG_LUMA_0_T  luma_0;
        ISP_NVRAM_HFG_LUMA_1_T  luma_1;
        ISP_NVRAM_HFG_LUMA_2_T  luma_2;
        ISP_NVRAM_HFG_LCE_0_T   lce_0;
        ISP_NVRAM_HFG_LCE_1_T   lce_1;
        ISP_NVRAM_HFG_LCE_2_T   lce_2;
        ISP_NVRAM_HFG_RAN_0_T   ran_0;
        ISP_NVRAM_HFG_RAN_1_T   ran_1;
        //ISP_NVRAM_HFG_RAN_2_T   ran_2;
        ISP_NVRAM_HFG_RAN_3_T   ran_3;
        //ISP_NVRAM_HFG_RAN_4_T   ran_4;
        //ISP_NVRAM_HFG_CROP_X_T  crop_x;
        //ISP_NVRAM_HFG_CROP_Y_T  crop_y;
    };
} ISP_NVRAM_HFG_GET_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// NDG
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct {
    FIELD  RANSED_GSEED                          : 16;    /*  0..15, 0x0000FFFF */
    FIELD  RANSED_SEED0                          : 16;    /* 16..31, 0xFFFF0000 */
} ISP_DIP_X_NDG_RAN_0_T; // /* 0x1502464C */

typedef union {
    typedef ISP_DIP_X_NDG_RAN_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NDG_RAN_0_T;

typedef struct {
    FIELD  RANSED_XOSFS                          : 16;    /*  0..15, 0x0000FFFF */
    FIELD  RANSED_YOSFS                          : 16;    /* 16..31, 0xFFFF0000 */
} ISP_DIP_X_NDG_RAN_1_T; // /* 0x15024650 */

typedef union {
    typedef ISP_DIP_X_NDG_RAN_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NDG_RAN_1_T;

typedef struct {
    FIELD  RANSED_IMG_WD                         : 16;    /*  0..15, 0x0000FFFF */
    FIELD  rsv_16                                : 16;    /* 16..31, 0xFFFF0000 */
} ISP_DIP_X_NDG_RAN_2_T; // /* 0x15024654 */

typedef union {
    typedef ISP_DIP_X_NDG_RAN_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NDG_RAN_2_T;

typedef struct {
    FIELD  NDG_T_WIDTH                            : 16;    /*  0..15, 0x0000FFFF */
    FIELD  NDG_T_HEIGHT                           : 16;    /* 16..31, 0xFFFF0000 */
} ISP_DIP_X_NDG_RAN_3_T; // /* 0x15024658 */

typedef union {
    typedef ISP_DIP_X_NDG_RAN_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NDG_RAN_3_T;

typedef struct {
    FIELD  NDG_CROP_STR_X                        : 16;    /*  0..15, 0x0000FFFF */
    FIELD  NDG_CROP_END_X                        : 16;    /* 16..31, 0xFFFF0000 */
} ISP_DIP_X_NDG_CROP_X_T; //  /* 0x15024660 */

typedef union {
    typedef ISP_DIP_X_NDG_CROP_X_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NDG_CROP_X_T;

typedef struct {
    FIELD  NDG_CROP_STR_Y                        : 16;    /*  0..15, 0x0000FFFF */
    FIELD  NDG_CROP_END_Y                        : 16;    /* 16..31, 0xFFFF0000 */
} ISP_DIP_X_NDG_CROP_Y_T; //  /* 0x15024664 */

typedef union {
    typedef ISP_DIP_X_NDG_CROP_Y_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NDG_CROP_Y_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// NDG2
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct {
    FIELD  RANSED_GSEED                          : 16;    /*  0..15, 0x0000FFFF */
    FIELD  RANSED_SEED0                          : 16;    /* 16..31, 0xFFFF0000 */
} ISP_DIP_X_NDG2_RAN_0_T; // /* 0x1502464C */

typedef union {
    typedef ISP_DIP_X_NDG2_RAN_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NDG2_RAN_0_T;

typedef struct {
    FIELD  RANSED_XOSFS                          : 16;    /*  0..15, 0x0000FFFF */
    FIELD  RANSED_YOSFS                          : 16;    /* 16..31, 0xFFFF0000 */
} ISP_DIP_X_NDG2_RAN_1_T; // /* 0x15024650 */

typedef union {
    typedef ISP_DIP_X_NDG2_RAN_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NDG2_RAN_1_T;

typedef struct {
    FIELD  RANSED_IMG_WD                         : 16;    /*  0..15, 0x0000FFFF */
    FIELD  rsv_16                                : 16;    /* 16..31, 0xFFFF0000 */
} ISP_DIP_X_NDG2_RAN_2_T; // /* 0x15024654 */

typedef union {
    typedef ISP_DIP_X_NDG2_RAN_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NDG2_RAN_2_T;

typedef struct {
    FIELD  NDG_T_WIDTH                            : 16;    /*  0..15, 0x0000FFFF */
    FIELD  NDG_T_HEIGHT                           : 16;    /* 16..31, 0xFFFF0000 */
} ISP_DIP_X_NDG2_RAN_3_T; // /* 0x15024658 */

typedef union {
    typedef ISP_DIP_X_NDG2_RAN_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NDG2_RAN_3_T;

typedef struct {
    FIELD  NDG_CROP_STR_X                        : 16;    /*  0..15, 0x0000FFFF */
    FIELD  NDG_CROP_END_X                        : 16;    /* 16..31, 0xFFFF0000 */
} ISP_DIP_X_NDG2_CROP_X_T; //  /* 0x15024660 */

typedef union {
    typedef ISP_DIP_X_NDG2_CROP_X_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NDG2_CROP_X_T;

typedef struct {
    FIELD  NDG_CROP_STR_Y                        : 16;    /*  0..15, 0x0000FFFF */
    FIELD  NDG_CROP_END_Y                        : 16;    /* 16..31, 0xFFFF0000 */
} ISP_DIP_X_NDG2_CROP_Y_T; //  /* 0x15024664 */

typedef union {
    typedef ISP_DIP_X_NDG2_CROP_Y_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NDG2_CROP_Y_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// MFB
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//Notice rsv6 needs mask 2 bits
typedef struct {
    FIELD  BLD_MODE                              :  1;      /*  0.. 0, 0x00000001 */
    FIELD  rsv_1                                 :  3;      /*  1.. 3, 0x0000000E */
    FIELD  BLD_LL_BRZ_EN                         :  1;      /*  4.. 4, 0x00000010 */
    FIELD  BLD_LL_DB_EN                          :  1;      /*  5.. 5, 0x00000020 */
    FIELD  BLD_MBD_WT_EN                         :  1;		/*  6.. 6, 0x00000040 */
    FIELD  BLD_SR_WT_EN                          :  1;		/*  7.. 7, 0x00000080 */
    FIELD  BLD_LL_TH_E                           :  8;      /*  8..15, 0x0000FF00 */
    FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
} ISP_DIP_X_MFB_CON_T; //	/* 0x15025100 */

typedef union {
    typedef ISP_DIP_X_MFB_CON_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MFB_CON_T;

typedef struct {
    FIELD  BLD_LL_FLT_MODE                       :  2;      /*  0.. 1, 0x00000003 */
    FIELD  rsv_2                                 :  2;      /*  2.. 3, 0x0000000C */
    FIELD  BLD_LL_FLT_WT_MODE1                   :  3;      /*  4.. 6, 0x00000070 */
    FIELD  rsv_7                                 :  1;      /*  7.. 7, 0x00000080 */
    FIELD  BLD_LL_FLT_WT_MODE2                   :  2;      /*  8.. 9, 0x00000300 */
    FIELD  rsv_10                                :  6;      /* 10..15, 0x0000FC00 */
    FIELD  BLD_LL_CLIP_TH1                       :  8;      /* 16..23, 0x00FF0000 */
    FIELD  BLD_LL_CLIP_TH2                       :  8;      /* 24..31, 0xFF000000 */
} ISP_DIP_X_MFB_LL_CON1_T; //	/* 0x15025104 */

typedef union {
    typedef ISP_DIP_X_MFB_LL_CON1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MFB_LL_CON1_T;

typedef struct {
    FIELD  BLD_LL_MAX_WT                         :  3;      /*  0.. 2, 0x00000007 */
    FIELD  rsv_3                                 :  5;      /*  3.. 7, 0x000000F8 */
    FIELD  BLD_LL_DT1                            :  8;      /*  8..15, 0x0000FF00 */
    FIELD  BLD_LL_TH1                            :  8;      /* 16..23, 0x00FF0000 */
    FIELD  BLD_LL_TH2                            :  8;      /* 24..31, 0xFF000000 */
} ISP_DIP_X_MFB_LL_CON2_T; //	/* 0x15025108 */

typedef union {
    typedef ISP_DIP_X_MFB_LL_CON2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MFB_LL_CON2_T;

typedef struct {
    FIELD  BLD_LL_OUT_XSIZE                      : 14;      /*  0..13, 0x00003FFF */
    FIELD  rsv_14                                :  1;      /* 14..14, 0x00004000 */
    FIELD  BLD_LL_OUT_XOFST                      :  1;      /* 15..15, 0x00008000 */
    FIELD  BLD_LL_OUT_YSIZE                      : 14;      /* 16..29, 0x3FFF0000 */
    FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
} ISP_DIP_X_MFB_LL_CON3_T; //	/* 0x1502510C */

typedef union {
    typedef ISP_DIP_X_MFB_LL_CON3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MFB_LL_CON3_T;

typedef struct {
    FIELD  BLD_LL_DB_XDIST                       :  4;      /*  0.. 3, 0x0000000F */
    FIELD  BLD_LL_DB_YDIST                       :  4;      /*  4.. 7, 0x000000F0 */
    FIELD  rsv_8                                 : 24;      /*  8..31, 0xFFFFFF00 */
} ISP_DIP_X_MFB_LL_CON4_T; //	/* 0x15025110 */

typedef union {
    typedef ISP_DIP_X_MFB_LL_CON4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MFB_LL_CON4_T;

typedef struct {
    FIELD  BLD_TILE_EDGE                         :  4;      /*  0.. 3, 0x0000000F */
    FIELD  rsv_4                                 : 28;      /*  4..31, 0xFFFFFFF0 */
} ISP_DIP_X_MFB_EDGE_T; //	/* 0x15025114 */

typedef union {
    typedef ISP_DIP_X_MFB_EDGE_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MFB_EDGE_T;

typedef struct {
    FIELD  BLD_LL_GRAD_R1                        :  8;		/*  0.. 7, 0x000000FF */
    FIELD  BLD_LL_GRAD_R2                        :  8;		/*  8..15, 0x0000FF00 */
    FIELD  BLD_LL_TH3                            :  8;		/* 16..23, 0x00FF0000 */
    FIELD  BLD_LL_TH4                            :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_MFB_LL_CON5_T; //	/* 0x15025118 */

typedef union {
    typedef ISP_DIP_X_MFB_LL_CON5_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MFB_LL_CON5_T;

typedef struct {
    FIELD  BLD_LL_TH5                            :  8;		/*  0.. 7, 0x000000FF */
    FIELD  BLD_LL_TH6                            :  8;		/*  8..15, 0x0000FF00 */
    FIELD  BLD_LL_TH7                            :  8;		/* 16..23, 0x00FF0000 */
    FIELD  BLD_LL_TH8                            :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_MFB_LL_CON6_T; //	/* 0x1502511C */

typedef union {
    typedef ISP_DIP_X_MFB_LL_CON6_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MFB_LL_CON6_T;

typedef struct {
    FIELD  BLD_LL_C1                             :  8;		/*  0.. 7, 0x000000FF */
    FIELD  BLD_LL_C2                             :  8;		/*  8..15, 0x0000FF00 */
    FIELD  BLD_LL_C3                             :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_MFB_LL_CON7_T; //	/* 0x15025120 */

typedef union {
    typedef ISP_DIP_X_MFB_LL_CON7_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MFB_LL_CON7_T;

typedef struct {
    FIELD  BLD_LL_SU1                            :  6;		/*  0.. 5, 0x0000003F */
    FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
    FIELD  BLD_LL_SU2                            :  6;		/*  8..13, 0x00003F00 */
    FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
    FIELD  BLD_LL_SU3                            :  6;		/* 16..21, 0x003F0000 */
    FIELD  rsv_22                                : 10;		/* 22..31, 0xFFC00000 */
} ISP_DIP_X_MFB_LL_CON8_T; //	/* 0x15025124 */

typedef union {
    typedef ISP_DIP_X_MFB_LL_CON8_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MFB_LL_CON8_T;

typedef struct {
    FIELD  BLD_LL_SL1                            :  6;		/*  0.. 5, 0x0000003F */
    FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
    FIELD  BLD_LL_SL2                            :  6;		/*  8..13, 0x00003F00 */
    FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
    FIELD  BLD_LL_SL3                            :  6;		/* 16..21, 0x003F0000 */
    FIELD  rsv_22                                : 10;		/* 22..31, 0xFFC00000 */
} ISP_DIP_X_MFB_LL_CON9_T; //	/* 0x15025128 */

typedef union {
    typedef ISP_DIP_X_MFB_LL_CON9_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MFB_LL_CON9_T;

typedef struct {
    FIELD  BLD_CONF_MAP_EN                       :  1;		/*  0.. 0, 0x00000001 */
    FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
    FIELD  BLD_LL_GRAD_EN                        :  1;		/*  4.. 4, 0x00000010 */
    FIELD  rsv_5                                 :  3;      /*  5.. 7, 0x000000E0 */
    FIELD  BLD_LL_GRAD_ENTH                      :  8;		/*  8..15, 0x0000FF00 */
    FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
} ISP_DIP_X_MFB_LL_CON10_T; //	/* 0x1502512C */

typedef union {
    typedef ISP_DIP_X_MFB_LL_CON10_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MFB_LL_CON10_T;

typedef struct {
    FIELD  BLD_MBD_MAX_WT                        :  8;		/*  0.. 7, 0x000000FF */
    FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
} ISP_DIP_X_MFB_MBD_CON0_T; //	/* 0x15025130 */

typedef union {
    typedef ISP_DIP_X_MFB_MBD_CON0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MFB_MBD_CON0_T;

typedef struct {
    FIELD  BLD_MBD_YL1_THL_0                     :  6;		/*  0.. 5, 0x0000003F */
    FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
    FIELD  BLD_MBD_YL1_THL_1                     :  6;		/*  8..13, 0x00003F00 */
    FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
    FIELD  BLD_MBD_YL1_THL_2                     :  6;		/* 16..21, 0x003F0000 */
    FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
    FIELD  BLD_MBD_YL1_THL_3                     :  6;		/* 24..29, 0x3F000000 */
    FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
} ISP_DIP_X_MFB_MBD_CON1_T; //	/* 0x15025134 */

typedef union {
    typedef ISP_DIP_X_MFB_MBD_CON1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MFB_MBD_CON1_T;

typedef struct {
    FIELD  BLD_MBD_YL1_THL_4                     :  6;		/*  0.. 5, 0x0000003F */
    FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
    FIELD  BLD_MBD_YL1_THH_0                     :  6;		/*  8..13, 0x00003F00 */
    FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
    FIELD  BLD_MBD_YL1_THH_1                     :  6;		/* 16..21, 0x003F0000 */
    FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
    FIELD  BLD_MBD_YL1_THH_2                     :  6;		/* 24..29, 0x3F000000 */
    FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
} ISP_DIP_X_MFB_MBD_CON2_T; //	/* 0x15025138 */

typedef union {
    typedef ISP_DIP_X_MFB_MBD_CON2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MFB_MBD_CON2_T;

typedef struct {
    FIELD  BLD_MBD_YL1_THH_3                     :  6;		/*  0.. 5, 0x0000003F */
    FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
    FIELD  BLD_MBD_YL1_THH_4                     :  6;		/*  8..13, 0x00003F00 */
    FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
    FIELD  BLD_MBD_YL2_THL_0                     :  6;		/* 16..21, 0x003F0000 */
    FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
    FIELD  BLD_MBD_YL2_THL_1                     :  6;		/* 24..29, 0x3F000000 */
    FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
} ISP_DIP_X_MFB_MBD_CON3_T; //	/* 0x1502513C */

typedef union {
    typedef ISP_DIP_X_MFB_MBD_CON3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MFB_MBD_CON3_T;

typedef struct {
    FIELD  BLD_MBD_YL2_THL_2                     :  6;		/*  0.. 5, 0x0000003F */
    FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
    FIELD  BLD_MBD_YL2_THL_3                     :  6;		/*  8..13, 0x00003F00 */
    FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
    FIELD  BLD_MBD_YL2_THL_4                     :  6;		/* 16..21, 0x003F0000 */
    FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
    FIELD  BLD_MBD_YL2_THH_0                     :  6;		/* 24..29, 0x3F000000 */
    FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
} ISP_DIP_X_MFB_MBD_CON4_T; //	/* 0x15025140 */

typedef union {
    typedef ISP_DIP_X_MFB_MBD_CON4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MFB_MBD_CON4_T;

typedef struct {
    FIELD  BLD_MBD_YL2_THH_1                     :  6;		/*  0.. 5, 0x0000003F */
    FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
    FIELD  BLD_MBD_YL2_THH_2                     :  6;		/*  8..13, 0x00003F00 */
    FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
    FIELD  BLD_MBD_YL2_THH_3                     :  6;		/* 16..21, 0x003F0000 */
    FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
    FIELD  BLD_MBD_YL2_THH_4                     :  6;		/* 24..29, 0x3F000000 */
    FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
} ISP_DIP_X_MFB_MBD_CON5_T; //	/* 0x15025144 */

typedef union {
    typedef ISP_DIP_X_MFB_MBD_CON5_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MFB_MBD_CON5_T;

typedef struct {
    FIELD  BLD_MBD_YL3_THL_0                     :  6;		/*  0.. 5, 0x0000003F */
    FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
    FIELD  BLD_MBD_YL3_THL_1                     :  6;		/*  8..13, 0x00003F00 */
    FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
    FIELD  BLD_MBD_YL3_THL_2                     :  6;		/* 16..21, 0x003F0000 */
    FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
    FIELD  BLD_MBD_YL3_THL_3                     :  6;		/* 24..29, 0x3F000000 */
    FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
} ISP_DIP_X_MFB_MBD_CON6_T; //	/* 0x15025148 */

typedef union {
    typedef ISP_DIP_X_MFB_MBD_CON6_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MFB_MBD_CON6_T;

typedef struct {
    FIELD  BLD_MBD_YL3_THL_4                     :  6;		/*  0.. 5, 0x0000003F */
    FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
    FIELD  BLD_MBD_YL3_THH_0                     :  6;		/*  8..13, 0x00003F00 */
    FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
    FIELD  BLD_MBD_YL3_THH_1                     :  6;		/* 16..21, 0x003F0000 */
    FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
    FIELD  BLD_MBD_YL3_THH_2                     :  6;		/* 24..29, 0x3F000000 */
    FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
} ISP_DIP_X_MFB_MBD_CON7_T; //	/* 0x1502514C */

typedef union {
    typedef ISP_DIP_X_MFB_MBD_CON7_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MFB_MBD_CON7_T;

typedef struct {
    FIELD  BLD_MBD_YL3_THH_3                     :  6;		/*  0.. 5, 0x0000003F */
    FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
    FIELD  BLD_MBD_YL3_THH_4                     :  6;		/*  8..13, 0x00003F00 */
    FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
    FIELD  BLD_MBD_CL1_THL_0                     :  6;		/* 16..21, 0x003F0000 */
    FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
    FIELD  BLD_MBD_CL1_THL_1                     :  6;		/* 24..29, 0x3F000000 */
    FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
} ISP_DIP_X_MFB_MBD_CON8_T; //	/* 0x15025150 */

typedef union {
    typedef ISP_DIP_X_MFB_MBD_CON8_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MFB_MBD_CON8_T;

typedef struct {
    FIELD  BLD_MBD_CL1_THL_2                     :  6;		/*  0.. 5, 0x0000003F */
    FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
    FIELD  BLD_MBD_CL1_THL_3                     :  6;		/*  8..13, 0x00003F00 */
    FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
    FIELD  BLD_MBD_CL1_THL_4                     :  6;		/* 16..21, 0x003F0000 */
    FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
    FIELD  BLD_MBD_CL1_THH_0                     :  6;		/* 24..29, 0x3F000000 */
    FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
} ISP_DIP_X_MFB_MBD_CON9_T; //	/* 0x15025154 */

typedef union {
    typedef ISP_DIP_X_MFB_MBD_CON9_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MFB_MBD_CON9_T;

typedef struct {
    FIELD  BLD_MBD_CL1_THH_1                     :  6;		/*  0.. 5, 0x0000003F */
    FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
    FIELD  BLD_MBD_CL1_THH_2                     :  6;		/*  8..13, 0x00003F00 */
    FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
    FIELD  BLD_MBD_CL1_THH_3                     :  6;		/* 16..21, 0x003F0000 */
    FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
    FIELD  BLD_MBD_CL1_THH_4                     :  6;		/* 24..29, 0x3F000000 */
    FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
} ISP_DIP_X_MFB_MBD_CON10_T; //	/* 0x15025158 */

typedef union {
    typedef ISP_DIP_X_MFB_MBD_CON10_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MFB_MBD_CON10_T;

typedef union {
    enum { COUNT = 21 };
    struct {
        //ISP_NVRAM_MFB_CON_T  con;
        ISP_NVRAM_MFB_LL_CON1_T  ll_con1;
        ISP_NVRAM_MFB_LL_CON2_T  ll_con2;
        //ISP_NVRAM_MFB_LL_CON3_T  ll_con3;
        //ISP_NVRAM_MFB_LL_CON4_T  ll_con4;
        //ISP_NVRAM_MFB_EDGE_T  edge;
        //ISP_NVRAM_MFB_LL_CON5_T  ll_con5;
        //ISP_NVRAM_MFB_LL_CON6_T  ll_con6;
        //ISP_NVRAM_MFB_LL_CON7_T  ll_con7;
        //ISP_NVRAM_MFB_LL_CON8_T  ll_con8;
        //ISP_NVRAM_MFB_LL_CON9_T  ll_con9;
        //ISP_NVRAM_MFB_LL_CON10_T  ll_con10;
        ISP_NVRAM_MFB_MBD_CON0_T  mbd_con0;
        ISP_NVRAM_MFB_MBD_CON1_T  mbd_con1;
        ISP_NVRAM_MFB_MBD_CON2_T  mbd_con2;
        ISP_NVRAM_MFB_MBD_CON3_T  mbd_con3;
        ISP_NVRAM_MFB_MBD_CON4_T  mbd_con4;
        ISP_NVRAM_MFB_MBD_CON5_T  mbd_con5;
        ISP_NVRAM_MFB_MBD_CON6_T  mbd_con6;
        ISP_NVRAM_MFB_MBD_CON7_T  mbd_con7;
        ISP_NVRAM_MFB_MBD_CON8_T  mbd_con8;
        ISP_NVRAM_MFB_MBD_CON9_T  mbd_con9;
        ISP_NVRAM_MFB_MBD_CON10_T  mbd_con10;
    };
    MUINT32 set[COUNT];
} ISP_NVRAM_MFB_T;





//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// PCA
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// DIP_X_PCA_CON1
typedef struct {
    FIELD  PCA_LUT_360                           :  1;      /*  0.. 0, 0x00000001 */
    FIELD  PCA_Y2YLUT_EN                         :  1;      /*  1.. 1, 0x00000002 */
    FIELD  PCA_CTRL_BDR                          :  1;      /*  2.. 2, 0x00000004 */
    FIELD  PCA_CTRL_OFST                         :  1;      /*  3.. 3, 0x00000008 */
    FIELD  RSV                                   :  1;      /*  4.. 4, 0x00000010 */
    FIELD  PCA_S_TH_EN                           :  1;      /*  5.. 5, 0x00000020 */
    FIELD  PCA_CFC_EN                            :  1;      /*  6.. 6, 0x00000040 */
    FIELD  PCA_CNV_EN                            :  1;      /*  7.. 7, 0x00000080 */
    FIELD  PCA_TILE_EDGE                         :  4;      /*  8..11, 0x00000F00 */
    FIELD  PCA_LR                                :  5;      /* 12..16, 0x0001F000 */
    FIELD  rsv_17                                : 15;      /* 17..31, 0xFFFE0000 */
} ISP_DIP_X_PCA_CON1_T; // /* 0x15023E00 */

typedef union {
    enum { MASK     = 0xFFFFFFFF };
    typedef ISP_DIP_X_PCA_CON1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_PCA_CON1_T;


//// DIP_X_PCA_CON2
typedef struct {
    FIELD  PCA_C_TH                              :  5;      /*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                 :  3;      /*  5.. 7, 0x000000E0 */
    FIELD  PCA_CNV_01                            : 11;      /*  8..18, 0x0007FF00 */
    FIELD  rsv_19                                :  1;      /* 19..19, 0x00080000 */
    FIELD  PCA_CNV_00                            : 11;      /* 20..30, 0x7FF00000 */
    FIELD  rsv_31                                :  1;      /* 31..31, 0x80000000 */
} ISP_DIP_X_PCA_CON2_T; // /* 0x15023E04 */

typedef union {
    enum { MASK     = 0xFFFFFFFF };
    typedef ISP_DIP_X_PCA_CON2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_PCA_CON2_T;


// DIP_X_PCA_CON3
typedef struct {
    FIELD  PCA_HG                                :  5;      /*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                 :  3;      /*  5.. 7, 0x000000E0 */
    FIELD  PCA_CNV_10                            : 11;      /*  8..18, 0x0007FF00 */
    FIELD  rsv_19                                :  1;      /* 19..19, 0x00080000 */
    FIELD  PCA_CNV_02                            : 11;      /* 20..30, 0x7FF00000 */
    FIELD  rsv_31                                :  1;      /* 31..31, 0x80000000 */
} ISP_DIP_X_PCA_CON3_T; // /* 0x15023E08 */

typedef union {
    enum { MASK     = 0xFFFFFFFF };
    typedef ISP_DIP_X_PCA_CON3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_PCA_CON3_T;


// DIP_X_PCA_CON4
typedef struct {
    FIELD  PCA_HR                                :  5;      /*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                 :  3;      /*  5.. 7, 0x000000E0 */
    FIELD  PCA_CNV_12                            : 11;      /*  8..18, 0x0007FF00 */
    FIELD  rsv_19                                :  1;      /* 19..19, 0x00080000 */
    FIELD  PCA_CNV_11                            : 11;      /* 20..30, 0x7FF00000 */
    FIELD  rsv_31                                :  1;      /* 31..31, 0x80000000 */
} ISP_DIP_X_PCA_CON4_T; // /* 0x15023E0C */

typedef union {
    enum { MASK     = 0xFFFFFFFF };
    typedef ISP_DIP_X_PCA_CON4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_PCA_CON4_T;


// DIP_X_PCA_CON5
typedef struct {
    FIELD  PCA_HB                                :  5;      /*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                 :  3;      /*  5.. 7, 0x000000E0 */
    FIELD  PCA_CNV_21                            : 11;      /*  8..18, 0x0007FF00 */
    FIELD  rsv_19                                :  1;      /* 19..19, 0x00080000 */
    FIELD  PCA_CNV_20                            : 11;      /* 20..30, 0x7FF00000 */
    FIELD  rsv_31                                :  1;      /* 31..31, 0x80000000 */
} ISP_DIP_X_PCA_CON5_T; // /* 0x15023E10 */

typedef union {
    enum { MASK     = 0xFFFFFFFF };
    typedef ISP_DIP_X_PCA_CON5_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_PCA_CON5_T;


// DIP_X_PCA_CON6
typedef struct {
    FIELD  PCA_LB                                :  5;      /*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                 :  3;      /*  5.. 7, 0x000000E0 */
    FIELD  PCA_LG                                :  5;      /*  8..12, 0x00001F00 */
    FIELD  rsv_13                                :  3;      /* 13..15, 0x0000E000 */
    FIELD  RSV                                   :  2;      /* 16..17, 0x00030000 */
    FIELD  rsv_18                                :  2;      /* 18..19, 0x000C0000 */
    FIELD  PCA_CNV_22                            : 11;      /* 20..30, 0x7FF00000 */
    FIELD  rsv_31                                :  1;      /* 31..31, 0x80000000 */
} ISP_DIP_X_PCA_CON6_T; // /* 0x15023E14 */

typedef union {
    enum { MASK     = 0xFFFFFFFF };
    typedef ISP_DIP_X_PCA_CON6_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_PCA_CON6_T;


// DIP_X_PCA_CON7
typedef struct {
    FIELD  PCA_S_TH                              :  8;      /*  0.. 7, 0x000000FF */
    FIELD  PCA_CMAX                              :  8;      /*  8..15, 0x0000FF00 */
    FIELD  PCA_H                                 :  8;      /* 16..23, 0x00FF0000 */
    FIELD  PCA_L                                 :  8;      /* 24..31, 0xFF000000 */
} ISP_DIP_X_PCA_CON7_T; // /* 0x15023E18 */

typedef union {
    enum { MASK     = 0xFFFFFFFF };
    typedef ISP_DIP_X_PCA_CON7_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_PCA_CON7_T;


//
typedef union {
    enum { COUNT = 3 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_PCA_CON1_T  con1;
        ISP_NVRAM_PCA_CON2_T  con2;
        //ISP_NVRAM_PCA_CON3_T  con3;
        //ISP_NVRAM_PCA_CON4_T  con4;
        //ISP_NVRAM_PCA_CON5_T  con5;
        //ISP_NVRAM_PCA_CON6_T  con6;
        ISP_NVRAM_PCA_CON7_T  con7;
    };
} ISP_NVRAM_PCA_T;



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// PCA _TBL
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
enum { PCA_BIN_NUM = 180 };

// CAM_PCA_TBL CAM+1800H
typedef struct {
        FIELD PCA_LUMA_GAIN             : 8;
        FIELD PCA_SAT_GAIN              : 8;
        FIELD PCA_HUE_SHIFT             : 8;
        FIELD rsv_24                    : 8;
} ISP_DIP_X_PCA_BIN_T;

typedef union {
    enum { MASK     = 0xFFFFFFFF };
    typedef ISP_DIP_X_PCA_BIN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_PCA_BIN_T;

//
typedef struct {
    /*
    0: origianl PCA,
    2: HDR PCA
    3: flash PCA
    */
    ISP_NVRAM_PCA_BIN_T lut_lo[PCA_BIN_NUM];
    ISP_NVRAM_PCA_BIN_T lut_md[PCA_BIN_NUM];
    ISP_NVRAM_PCA_BIN_T lut_hi[PCA_BIN_NUM];
    ISP_NVRAM_PCA_BIN_T lut_lo2[PCA_BIN_NUM];
    ISP_NVRAM_PCA_BIN_T lut_md2[PCA_BIN_NUM];
    ISP_NVRAM_PCA_BIN_T lut_hi2[PCA_BIN_NUM];
    ISP_NVRAM_PCA_BIN_T lut_lo3[PCA_BIN_NUM];
    ISP_NVRAM_PCA_BIN_T lut_md3[PCA_BIN_NUM];
    ISP_NVRAM_PCA_BIN_T lut_hi3[PCA_BIN_NUM];
} ISP_NVRAM_PCA_LUTS_T;

typedef struct {
        ISP_NVRAM_PCA_BIN_T   lut[PCA_BIN_NUM];

} ISP_NVRAM_PCA_LUT_T;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// EE
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef struct {
    FIELD  SEEE_TILE_EDGE                        :  4;		/*  0.. 3, 0x0000000F */
    FIELD  rsv_4                                 : 24;		/*  4..27, 0x0FFFFFF0 */
    FIELD  RESERVED                              :  4;		/* 28..31, 0xF0000000 */
} ISP_DIP_X_SEEE_CTRL_T; //	/* 0x150266C0 */

typedef union {
    typedef ISP_DIP_X_SEEE_CTRL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_CTRL_T;

typedef struct {
    FIELD  SEEE_OUT_EDGE_SEL                     :  2;		/*  0.. 1, 0x00000003 */
    FIELD  rsv_2                                 :  6;		/*  2.. 7, 0x000000FC */
    FIELD  SEEE_LP_MODE                          :  1;		/*  8.. 8, 0x00000100 */
    FIELD  rsv_9                                 : 23;		/*  9..31, 0xFFFFFE00 */
} ISP_DIP_X_SEEE_TOP_CTRL_T; //	/* 0x150266C4 */

typedef union {
    typedef ISP_DIP_X_SEEE_TOP_CTRL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_TOP_CTRL_T;

typedef struct {
    FIELD  SEEE_H1_DI_BLND_OFST                  :  8;		/*  0.. 7, 0x000000FF */
    FIELD  SEEE_H2_DI_BLND_OFST                  :  8;		/*  8..15, 0x0000FF00 */
    FIELD  SEEE_H3_DI_BLND_OFST                  :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_SEEE_BLND_CTRL_1_T; //	/* 0x150266C8 */

typedef union {
    typedef ISP_DIP_X_SEEE_BLND_CTRL_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_BLND_CTRL_1_T;

typedef struct {
    FIELD  SEEE_H1_DI_BLND_SL                    :  8;		/*  0.. 7, 0x000000FF */
    FIELD  SEEE_H2_DI_BLND_SL                    :  8;		/*  8..15, 0x0000FF00 */
    FIELD  SEEE_H3_DI_BLND_SL                    :  8;		/* 16..23, 0x00FF0000 */
    FIELD  SEEE_HX_ISO_BLND_RAT                  :  5;		/* 24..28, 0x1F000000 */
    FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
} ISP_DIP_X_SEEE_BLND_CTRL_2_T; //	/* 0x150266CC */

typedef union {
    typedef ISP_DIP_X_SEEE_BLND_CTRL_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_BLND_CTRL_2_T;

typedef struct {
    FIELD  SEEE_H1_FLT_CORE_TH                   :  8;		/*  0.. 7, 0x000000FF */
    FIELD  SEEE_H2_FLT_CORE_TH                   :  8;		/*  8..15, 0x0000FF00 */
    FIELD  SEEE_H3_FLT_CORE_TH                   :  8;		/* 16..23, 0x00FF0000 */
    FIELD  SEEE_FLT_CORE_TH                      :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_SEEE_CORE_CTRL_T; //	/* 0x150266D0 */

typedef union {
    typedef ISP_DIP_X_SEEE_CORE_CTRL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_CORE_CTRL_T;

typedef struct {
    FIELD  SEEE_H1_GN                            :  5;		/*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
    FIELD  SEEE_H2_GN                            :  5;		/*  8..12, 0x00001F00 */
    FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
    FIELD  SEEE_H3_GN                            :  5;		/* 16..20, 0x001F0000 */
    FIELD  rsv_21                                : 11;		/* 21..31, 0xFFE00000 */
} ISP_DIP_X_SEEE_GN_CTRL_1_T; //	/* 0x150266D4 */

typedef union {
    typedef ISP_DIP_X_SEEE_GN_CTRL_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_GN_CTRL_1_T;

typedef struct {
    FIELD  SEEE_LUMA_MOD_Y0                      :  9;		/*  0.. 8, 0x000001FF */
    FIELD  rsv_9                                 :  1;		/*  9.. 9, 0x00000200 */
    FIELD  SEEE_LUMA_MOD_Y1                      :  9;		/* 10..18, 0x0007FC00 */
    FIELD  rsv_19                                :  1;		/* 19..19, 0x00080000 */
    FIELD  SEEE_LUMA_MOD_Y2                      :  9;		/* 20..28, 0x1FF00000 */
    FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
} ISP_DIP_X_SEEE_LUMA_CTRL_1_T; //	/* 0x150266D8 */

typedef union {
    typedef ISP_DIP_X_SEEE_LUMA_CTRL_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_LUMA_CTRL_1_T;

typedef struct {
    FIELD  SEEE_LUMA_MOD_Y3                      :  9;		/*  0.. 8, 0x000001FF */
    FIELD  rsv_9                                 :  1;		/*  9.. 9, 0x00000200 */
    FIELD  SEEE_LUMA_MOD_Y4                      :  9;		/* 10..18, 0x0007FC00 */
    FIELD  rsv_19                                :  1;		/* 19..19, 0x00080000 */
    FIELD  SEEE_LUMA_MOD_Y5                      :  9;		/* 20..28, 0x1FF00000 */
    FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
} ISP_DIP_X_SEEE_LUMA_CTRL_2_T; //	/* 0x150266DC */

typedef union {
    typedef ISP_DIP_X_SEEE_LUMA_CTRL_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_LUMA_CTRL_2_T;

typedef struct {
    FIELD  SEEE_SLNK_GN_Y1                       :  8;		/*  0.. 7, 0x000000FF */
    FIELD  SEEE_SLNK_GN_Y2                       :  8;		/*  8..15, 0x0000FF00 */
    FIELD  SEEE_RESP_SLNK_GN_RAT                 :  5;		/* 16..20, 0x001F0000 */
    FIELD  SEEE_GLUT_LINK_EN                     :  1;		/* 21..21, 0x00200000 */
    FIELD  SEEE_LUMA_MOD_Y6                      :  9;		/* 22..30, 0x7FC00000 */
    FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
} ISP_DIP_X_SEEE_LUMA_SLNK_CTRL_T; //	/* 0x150266E0 */

typedef union {
    typedef ISP_DIP_X_SEEE_LUMA_SLNK_CTRL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_LUMA_SLNK_CTRL_T;

typedef struct {
    FIELD  SEEE_GLUT_S1                          :  8;		/*  0.. 7, 0x000000FF */
    FIELD  SEEE_GLUT_X1                          :  8;		/*  8..15, 0x0000FF00 */
    FIELD  SEEE_GLUT_Y1                          : 10;		/* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
} ISP_DIP_X_SEEE_GLUT_CTRL_1_T; //	/* 0x150266E4 */

typedef union {
    typedef ISP_DIP_X_SEEE_GLUT_CTRL_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_GLUT_CTRL_1_T;

typedef struct {
    FIELD  SEEE_GLUT_S2                          :  8;		/*  0.. 7, 0x000000FF */
    FIELD  SEEE_GLUT_X2                          :  8;		/*  8..15, 0x0000FF00 */
    FIELD  SEEE_GLUT_Y2                          : 10;		/* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
} ISP_DIP_X_SEEE_GLUT_CTRL_2_T; //	/* 0x150266E8 */

typedef union {
    typedef ISP_DIP_X_SEEE_GLUT_CTRL_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_GLUT_CTRL_2_T;

typedef struct {
    FIELD  SEEE_GLUT_S3                          :  8;		/*  0.. 7, 0x000000FF */
    FIELD  SEEE_GLUT_X3                          :  8;		/*  8..15, 0x0000FF00 */
    FIELD  SEEE_GLUT_Y3                          : 10;		/* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
} ISP_DIP_X_SEEE_GLUT_CTRL_3_T; //	/* 0x150266EC */

typedef union {
    typedef ISP_DIP_X_SEEE_GLUT_CTRL_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_GLUT_CTRL_3_T;

typedef struct {
    FIELD  SEEE_GLUT_S4                          :  8;		/*  0.. 7, 0x000000FF */
    FIELD  SEEE_GLUT_X4                          :  8;		/*  8..15, 0x0000FF00 */
    FIELD  SEEE_GLUT_Y4                          : 10;		/* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
} ISP_DIP_X_SEEE_GLUT_CTRL_4_T; //	/* 0x150266F0 */

typedef union {
    typedef ISP_DIP_X_SEEE_GLUT_CTRL_4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_GLUT_CTRL_4_T;

typedef struct {
    FIELD  SEEE_GLUT_S5                          :  8;		/*  0.. 7, 0x000000FF */
    FIELD  rsv_8                                 :  8;		/*  8..15, 0x0000FF00 */
    FIELD  SEEE_GLUT_SL_DEC_Y                    : 10;		/* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
} ISP_DIP_X_SEEE_GLUT_CTRL_5_T; //	/* 0x150266F4 */

typedef union {
    typedef ISP_DIP_X_SEEE_GLUT_CTRL_5_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_GLUT_CTRL_5_T;

typedef struct {
    FIELD  SEEE_GLUT_TH_OVR                      :  8;		/*  0.. 7, 0x000000FF */
    FIELD  SEEE_GLUT_TH_UND                      :  8;		/*  8..15, 0x0000FF00 */
    FIELD  SEEE_GLUT_TH_MIN                      :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_SEEE_GLUT_CTRL_6_T; //	/* 0x150266F8 */

typedef union {
    typedef ISP_DIP_X_SEEE_GLUT_CTRL_6_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_GLUT_CTRL_6_T;

typedef struct {
    FIELD  SEEE_RESP_SMO_STR                     :  3;		/*  0.. 2, 0x00000007 */
    FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
    FIELD  SEEE_OVRSH_CLIP_STR                   :  3;		/*  4.. 6, 0x00000070 */
    FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
    FIELD  SEEE_DOT_REDUC_AMNT                   :  8;		/*  8..15, 0x0000FF00 */
    FIELD  SEEE_DOT_TH                           :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_SEEE_ARTIFACT_CTRL_T; //	/* 0x150266FC */

typedef union {
    typedef ISP_DIP_X_SEEE_ARTIFACT_CTRL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_ARTIFACT_CTRL_T;

typedef struct {
    FIELD  SEEE_RESP_CLIP                        :  8;      /*  0.. 7, 0x000000FF */
    FIELD  SEEE_RESP_CLIP_LUMA_SPC_TH            :  8;      /*  8..15, 0x0000FF00 */
    FIELD  SEEE_RESP_CLIP_LUMA_LWB               :  8;      /* 16..23, 0x00FF0000 */
    FIELD  SEEE_RESP_CLIP_LUMA_UPB               :  8;      /* 24..31, 0xFF000000 */
} ISP_DIP_X_SEEE_CLIP_CTRL_T; //	/* 0x15026700 */

typedef union {
    typedef ISP_DIP_X_SEEE_CLIP_CTRL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_CLIP_CTRL_T;

typedef struct {
    FIELD  SEEE_MASTER_GN_NEG                    :  8;		/*  0.. 7, 0x000000FF */
    FIELD  SEEE_MASTER_GN_POS                    :  8;		/*  8..15, 0x0000FF00 */
    FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
} ISP_DIP_X_SEEE_GN_CTRL_2_T; //	/* 0x15026704 */

typedef union {
    typedef ISP_DIP_X_SEEE_GN_CTRL_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_GN_CTRL_2_T;

typedef struct {
    FIELD  SEEE_ST_UB                            :  8;		/*  0.. 7, 0x000000FF */
    FIELD  SEEE_ST_LB                            :  8;		/*  8..15, 0x0000FF00 */
    FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
} ISP_DIP_X_SEEE_ST_CTRL_1_T; //	/* 0x15026708 */

typedef union {
    typedef ISP_DIP_X_SEEE_ST_CTRL_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_ST_CTRL_1_T;

typedef struct {
    FIELD  SEEE_ST_SL_CE                         :  8;		/*  0.. 7, 0x000000FF */
    FIELD  SEEE_ST_OFST_CE                       :  8;		/*  8..15, 0x0000FF00 */
    FIELD  SEEE_ST_SL_RESP                       :  8;		/* 16..23, 0x00FF0000 */
    FIELD  SEEE_ST_OFST_RESP                     :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_SEEE_ST_CTRL_2_T; //	/* 0x1502670C */

typedef union {
    typedef ISP_DIP_X_SEEE_ST_CTRL_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_ST_CTRL_2_T;

typedef struct {
    FIELD  SEEE_LUMA_LMT_DIFF                    :  8;		/*  0.. 7, 0x000000FF */
    FIELD  SEEE_LUMA_CNTST_LV                    :  3;		/*  8..10, 0x00000700 */
    FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
    FIELD  SEEE_LUMA_MINI                        :  3;		/* 12..14, 0x00007000 */
    FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
    FIELD  SEEE_LUMA_MAXI                        :  3;		/* 16..18, 0x00070000 */
    FIELD  rsv_19                                :  1;		/* 19..19, 0x00080000 */
    FIELD  SEEE_CHR_CNTST_LV                     :  3;		/* 20..22, 0x00700000 */
    FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
    FIELD  SEEE_CHR_MINI                         :  2;		/* 24..25, 0x03000000 */
    FIELD  rsv_26                                :  2;		/* 26..27, 0x0C000000 */
    FIELD  SEEE_CHR_MAXI                         :  2;		/* 28..29, 0x30000000 */
    FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
} ISP_DIP_X_SEEE_CT_CTRL_T; //	/* 0x15026710 */

typedef union {
    typedef ISP_DIP_X_SEEE_CT_CTRL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_CT_CTRL_T;

typedef struct {
    FIELD  SEEE_CBOOST_LMT_U                     :  8;		/*  0.. 7, 0x000000FF */
    FIELD  SEEE_CBOOST_LMT_L                     :  8;		/*  8..15, 0x0000FF00 */
    FIELD  SEEE_CBOOST_GAIN                      :  8;		/* 16..23, 0x00FF0000 */
    FIELD  SEEE_CBOOST_EN                        :  1;		/* 24..24, 0x01000000 */
    FIELD  rsv_25                                :  7;      /* 25..31, 0xFE000000 */
} ISP_DIP_X_SEEE_CBOOST_CTRL_1_T; //	/* 0x15026714 */

typedef union {
    typedef ISP_DIP_X_SEEE_CBOOST_CTRL_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_CBOOST_CTRL_1_T;

typedef struct {
    FIELD  SEEE_CBOOST_YCONST                    :  8;		/*  0.. 7, 0x000000FF */
    FIELD  SEEE_CBOOST_YOFFSET                   :  8;		/*  8..15, 0x0000FF00 */
    FIELD  SEEE_CBOOST_YOFFSET_SEL               :  2;		/* 16..17, 0x00030000 */
    FIELD  rsv_18                                : 14;		/* 18..31, 0xFFFC0000 */
} ISP_DIP_X_SEEE_CBOOST_CTRL_2_T; //	/* 0x15026718 */

typedef union {
    typedef ISP_DIP_X_SEEE_CBOOST_CTRL_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_CBOOST_CTRL_2_T;

typedef struct {
    FIELD  SEEE_PBC1_RADIUS_R                    :  6;		/*  0.. 5, 0x0000003F */
    FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
    FIELD  SEEE_PBC1_RSLOPE                      : 10;		/*  8..17, 0x0003FF00 */
    FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
    FIELD  SEEE_PBC1_RSLOPE_1                    : 10;		/* 20..29, 0x3FF00000 */
    FIELD  SEEE_PBC1_EN                          :  1;		/* 30..30, 0x40000000 */
    FIELD  SEEE_PBC_EN                           :  1;		/* 31..31, 0x80000000 */
} ISP_DIP_X_SEEE_PBC1_CTRL_1_T; //	/* 0x1502671C */

typedef union {
    typedef ISP_DIP_X_SEEE_PBC1_CTRL_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_PBC1_CTRL_1_T;

typedef struct {
    FIELD  SEEE_PBC1_TSLOPE                      : 10;		/*  0.. 9, 0x000003FF */
    FIELD  SEEE_PBC1_THETA_R                     :  6;		/* 10..15, 0x0000FC00 */
    FIELD  SEEE_PBC1_THETA_C                     :  8;		/* 16..23, 0x00FF0000 */
    FIELD  SEEE_PBC1_RADIUS_C                    :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_SEEE_PBC1_CTRL_2_T; //	/* 0x15026720 */

typedef union {
    typedef ISP_DIP_X_SEEE_PBC1_CTRL_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_PBC1_CTRL_2_T;

typedef struct {
    FIELD  SEEE_PBC1_LPF_GAIN                    :  6;		/*  0.. 5, 0x0000003F */
    FIELD  SEEE_PBC1_LPF_EN                      :  1;		/*  6.. 6, 0x00000040 */
    FIELD  SEEE_PBC1_EDGE_EN                     :  1;		/*  7.. 7, 0x00000080 */
    FIELD  SEEE_PBC1_EDGE_SLOPE                  :  6;		/*  8..13, 0x00003F00 */
    FIELD  SEEE_PBC1_EDGE_THR                    :  6;		/* 14..19, 0x000FC000 */
    FIELD  SEEE_PBC1_CONF_GAIN                   :  4;		/* 20..23, 0x00F00000 */
    FIELD  SEEE_PBC1_GAIN                        :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_SEEE_PBC1_CTRL_3_T; //	/* 0x15026724 */

typedef union {
    typedef ISP_DIP_X_SEEE_PBC1_CTRL_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_PBC1_CTRL_3_T;

typedef struct {
    FIELD  SEEE_PBC2_RADIUS_R                    :  6;		/*  0.. 5, 0x0000003F */
    FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
    FIELD  SEEE_PBC2_RSLOPE                      : 10;		/*  8..17, 0x0003FF00 */
    FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
    FIELD  SEEE_PBC2_RSLOPE_1                    : 10;		/* 20..29, 0x3FF00000 */
    FIELD  SEEE_PBC2_EN                          :  1;		/* 30..30, 0x40000000 */
    FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
} ISP_DIP_X_SEEE_PBC2_CTRL_1_T; //	/* 0x15026728 */

typedef union {
    typedef ISP_DIP_X_SEEE_PBC2_CTRL_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_PBC2_CTRL_1_T;

typedef struct {
    FIELD  SEEE_PBC2_TSLOPE                      : 10;		/*  0.. 9, 0x000003FF */
    FIELD  SEEE_PBC2_THETA_R                     :  6;		/* 10..15, 0x0000FC00 */
    FIELD  SEEE_PBC2_THETA_C                     :  8;		/* 16..23, 0x00FF0000 */
    FIELD  SEEE_PBC2_RADIUS_C                    :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_SEEE_PBC2_CTRL_2_T; //	/* 0x1502672C */

typedef union {
    typedef ISP_DIP_X_SEEE_PBC2_CTRL_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_PBC2_CTRL_2_T;

typedef struct {
    FIELD  SEEE_PBC2_LPF_GAIN                    :  6;		/*  0.. 5, 0x0000003F */
    FIELD  SEEE_PBC2_LPF_EN                      :  1;		/*  6.. 6, 0x00000040 */
    FIELD  SEEE_PBC2_EDGE_EN                     :  1;		/*  7.. 7, 0x00000080 */
    FIELD  SEEE_PBC2_EDGE_SLOPE                  :  6;		/*  8..13, 0x00003F00 */
    FIELD  SEEE_PBC2_EDGE_THR                    :  6;		/* 14..19, 0x000FC000 */
    FIELD  SEEE_PBC2_CONF_GAIN                   :  4;		/* 20..23, 0x00F00000 */
    FIELD  SEEE_PBC2_GAIN                        :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_SEEE_PBC2_CTRL_3_T; //	/* 0x15026730 */

typedef union {
    typedef ISP_DIP_X_SEEE_PBC2_CTRL_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_PBC2_CTRL_3_T;

typedef struct {
    FIELD  SEEE_PBC3_RADIUS_R                    :  6;		/*  0.. 5, 0x0000003F */
    FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
    FIELD  SEEE_PBC3_RSLOPE                      : 10;		/*  8..17, 0x0003FF00 */
    FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
    FIELD  SEEE_PBC3_RSLOPE_1                    : 10;		/* 20..29, 0x3FF00000 */
    FIELD  SEEE_PBC3_EN                          :  1;		/* 30..30, 0x40000000 */
    FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
} ISP_DIP_X_SEEE_PBC3_CTRL_1_T; //	/* 0x15026734 */

typedef union {
    typedef ISP_DIP_X_SEEE_PBC3_CTRL_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_PBC3_CTRL_1_T;

typedef struct {
    FIELD  SEEE_PBC3_TSLOPE                      : 10;		/*  0.. 9, 0x000003FF */
    FIELD  SEEE_PBC3_THETA_R                     :  6;		/* 10..15, 0x0000FC00 */
    FIELD  SEEE_PBC3_THETA_C                     :  8;		/* 16..23, 0x00FF0000 */
    FIELD  SEEE_PBC3_RADIUS_C                    :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_SEEE_PBC3_CTRL_2_T; //	/* 0x15026738 */

typedef union {
    typedef ISP_DIP_X_SEEE_PBC3_CTRL_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_PBC3_CTRL_2_T;

typedef struct {
    FIELD  SEEE_PBC3_LPF_GAIN                    :  6;		/*  0.. 5, 0x0000003F */
    FIELD  SEEE_PBC3_LPF_EN                      :  1;		/*  6.. 6, 0x00000040 */
    FIELD  SEEE_PBC3_EDGE_EN                     :  1;		/*  7.. 7, 0x00000080 */
    FIELD  SEEE_PBC3_EDGE_SLOPE                  :  6;		/*  8..13, 0x00003F00 */
    FIELD  SEEE_PBC3_EDGE_THR                    :  6;		/* 14..19, 0x000FC000 */
    FIELD  SEEE_PBC3_CONF_GAIN                   :  4;		/* 20..23, 0x00F00000 */
    FIELD  SEEE_PBC3_GAIN                        :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_SEEE_PBC3_CTRL_3_T; //	/* 0x1502673C */

typedef union {
    typedef ISP_DIP_X_SEEE_PBC3_CTRL_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_PBC3_CTRL_3_T;

typedef struct {
    FIELD  SEEE_SE_KNEE_SEL                      :  2;		/*  0.. 1, 0x00000003 */
    FIELD  SEEE_SE_SPECL_HALF_MODE               :  2;		/*  2.. 3, 0x0000000C */
    FIELD  SEEE_SE_SPECL_GAIN                    :  2;		/*  4.. 5, 0x00000030 */
    FIELD  SEEE_SE_SPECL_INV                     :  1;		/*  6.. 6, 0x00000040 */
    FIELD  SEEE_SE_SPECL_ABS                     :  1;		/*  7.. 7, 0x00000080 */
    FIELD  SEEE_SE_CONST_Y_EN                    :  1;		/*  8.. 8, 0x00000100 */
    FIELD  rsv_9                                 :  7;		/*  9..15, 0x0000FE00 */
    FIELD  SEEE_SE_CONST_Y_VAL                   :  8;		/* 16..23, 0x00FF0000 */
    FIELD  SEEE_SE_YOUT_QBIT                     :  4;		/* 24..27, 0x0F000000 */
    FIELD  SEEE_SE_COUT_QBIT                     :  4;		/* 28..31, 0xF0000000 */
} ISP_DIP_X_SEEE_SE_Y_SPECL_CTRL_T; //	/* 0x15026740 */

typedef union {
    typedef ISP_DIP_X_SEEE_SE_Y_SPECL_CTRL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_SE_Y_SPECL_CTRL_T;

typedef struct {
    FIELD  SEEE_SE_HORI_EDGE_SEL                 :  1;		/*  0.. 0, 0x00000001 */
    FIELD  SEEE_SE_HORI_EDGE_GAIN_A              :  4;		/*  1.. 4, 0x0000001E */
    FIELD  SEEE_SE_HORI_EDGE_GAIN_B              :  5;		/*  5.. 9, 0x000003E0 */
    FIELD  SEEE_SE_HORI_EDGE_GAIN_C              :  5;		/* 10..14, 0x00007C00 */
    FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
    FIELD  SEEE_SE_VERT_EDGE_SEL                 :  1;		/* 16..16, 0x00010000 */
    FIELD  SEEE_SE_VERT_EDGE_GAIN_A              :  4;		/* 17..20, 0x001E0000 */
    FIELD  SEEE_SE_VERT_EDGE_GAIN_B              :  5;		/* 21..25, 0x03E00000 */
    FIELD  SEEE_SE_VERT_EDGE_GAIN_C              :  5;		/* 26..30, 0x7C000000 */
    FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
} ISP_DIP_X_SEEE_SE_EDGE_CTRL_1_T; //	/* 0x15026744 */

typedef union {
    typedef ISP_DIP_X_SEEE_SE_EDGE_CTRL_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_SE_EDGE_CTRL_1_T;

typedef struct {
    FIELD  SEEE_SE_CONVT_CORE_TH                 :  7;		/*  0.. 6, 0x0000007F */
    FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
    FIELD  SEEE_SE_CONVT_E_TH                    :  8;		/*  8..15, 0x0000FF00 */
    FIELD  SEEE_SE_CONVT_SLOPE_SEL               :  1;		/* 16..16, 0x00010000 */
    FIELD  SEEE_SE_CONVT_FORCE_EN                :  1;		/* 17..17, 0x00020000 */
    FIELD  SEEE_SE_EDGE_DET_GAIN                 :  5;		/* 18..22, 0x007C0000 */
    FIELD  SEEE_SE_OIL_EN                        :  1;		/* 23..23, 0x00800000 */
    FIELD  SEEE_SE_BOSS_GAIN_OFF                 :  1;		/* 24..24, 0x01000000 */
    FIELD  SEEE_SE_BOSS_IN_SEL                   :  1;		/* 25..25, 0x02000000 */
    FIELD  rsv_26                                :  6;      /* 26..31, 0xFC000000 */
} ISP_DIP_X_SEEE_SE_EDGE_CTRL_2_T; //	/* 0x15026748 */

typedef union {
    typedef ISP_DIP_X_SEEE_SE_EDGE_CTRL_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_SE_EDGE_CTRL_2_T;

typedef struct {
    FIELD  SEEE_SE_CORE_HORI_X0                  :  7;		/*  0.. 6, 0x0000007F */
    FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
    FIELD  SEEE_SE_CORE_HORI_X2                  :  8;		/*  8..15, 0x0000FF00 */
    FIELD  SEEE_SE_CORE_HORI_Y5                  :  6;		/* 16..21, 0x003F0000 */
    FIELD  SEEE_SE_CORE_HORI_SUP                 :  2;		/* 22..23, 0x00C00000 */
    FIELD  SEEE_SE_CORE_HORI_SDN                 :  2;		/* 24..25, 0x03000000 */
    FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
} ISP_DIP_X_SEEE_SE_CORE_CTRL_1_T; //	/* 0x1502674C */

typedef union {
    typedef ISP_DIP_X_SEEE_SE_CORE_CTRL_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_SE_CORE_CTRL_1_T;

typedef struct {
    FIELD  SEEE_SE_CORE_VERT_X0                  :  7;		/*  0.. 6, 0x0000007F */
    FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
    FIELD  SEEE_SE_CORE_VERT_X2                  :  8;		/*  8..15, 0x0000FF00 */
    FIELD  SEEE_SE_CORE_VERT_Y5                  :  6;		/* 16..21, 0x003F0000 */
    FIELD  SEEE_SE_CORE_VERT_SUP                 :  2;		/* 22..23, 0x00C00000 */
    FIELD  SEEE_SE_CORE_VERT_SDN                 :  2;		/* 24..25, 0x03000000 */
    FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
} ISP_DIP_X_SEEE_SE_CORE_CTRL_2_T; //	/* 0x15026750 */

typedef union {
    typedef ISP_DIP_X_SEEE_SE_CORE_CTRL_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_SE_CORE_CTRL_2_T;

typedef union {
    enum { COUNT = 30 };
    struct {
        //ISP_NVRAM_SEEE_CTRL_T  ctrl;
        //ISP_NVRAM_SEEE_TOP_CTRL_T  top_ctrl;
        ISP_NVRAM_SEEE_BLND_CTRL_1_T  blnd_ctrl_1;
        ISP_NVRAM_SEEE_BLND_CTRL_2_T  blnd_ctrl_2;
        ISP_NVRAM_SEEE_CORE_CTRL_T  core_ctrl;
        ISP_NVRAM_SEEE_GN_CTRL_1_T  gn_ctrl_1;
        ISP_NVRAM_SEEE_LUMA_CTRL_1_T  luma_ctrl_1;
        ISP_NVRAM_SEEE_LUMA_CTRL_2_T  luma_ctrl_2;
        ISP_NVRAM_SEEE_LUMA_SLNK_CTRL_T  luma_slnk_ctrl;
        ISP_NVRAM_SEEE_GLUT_CTRL_1_T  glut_ctrl_1;
        ISP_NVRAM_SEEE_GLUT_CTRL_2_T  glut_ctrl_2;
        ISP_NVRAM_SEEE_GLUT_CTRL_3_T  glut_ctrl_3;
        ISP_NVRAM_SEEE_GLUT_CTRL_4_T  glut_ctrl_4;
        ISP_NVRAM_SEEE_GLUT_CTRL_5_T  glut_ctrl_5;
        ISP_NVRAM_SEEE_GLUT_CTRL_6_T  glut_ctrl_6;
        ISP_NVRAM_SEEE_ARTIFACT_CTRL_T  artifact_ctrl;
        ISP_NVRAM_SEEE_CLIP_CTRL_T  clip_ctrl;
        ISP_NVRAM_SEEE_GN_CTRL_2_T  gn_ctrl_2;
        ISP_NVRAM_SEEE_ST_CTRL_1_T  st_ctrl_1;
        ISP_NVRAM_SEEE_ST_CTRL_2_T  st_ctrl_2;
        ISP_NVRAM_SEEE_CT_CTRL_T  ct_ctrl;
        ISP_NVRAM_SEEE_CBOOST_CTRL_1_T  cboost_ctrl_1;
        ISP_NVRAM_SEEE_CBOOST_CTRL_2_T  cboost_ctrl_2;
        ISP_NVRAM_SEEE_PBC1_CTRL_1_T  pbc1_ctrl_1;
        ISP_NVRAM_SEEE_PBC1_CTRL_2_T  pbc1_ctrl_2;
        ISP_NVRAM_SEEE_PBC1_CTRL_3_T  pbc1_ctrl_3;
        ISP_NVRAM_SEEE_PBC2_CTRL_1_T  pbc2_ctrl_1;
        ISP_NVRAM_SEEE_PBC2_CTRL_2_T  pbc2_ctrl_2;
        ISP_NVRAM_SEEE_PBC2_CTRL_3_T  pbc2_ctrl_3;
        ISP_NVRAM_SEEE_PBC3_CTRL_1_T  pbc3_ctrl_1;
        ISP_NVRAM_SEEE_PBC3_CTRL_2_T  pbc3_ctrl_2;
        ISP_NVRAM_SEEE_PBC3_CTRL_3_T  pbc3_ctrl_3;
        //ISP_NVRAM_SEEE_SE_Y_SPECL_CTRL_T  se_y_specl_ctrl;
        //ISP_NVRAM_SEEE_SE_EDGE_CTRL_1_T  se_edge_ctrl_1;
        //ISP_NVRAM_SEEE_SE_EDGE_CTRL_2_T  se_edge_ctrl_2;
        //ISP_NVRAM_SEEE_SE_CORE_CTRL_1_T  se_core_ctrl_1;
        //ISP_NVRAM_SEEE_SE_CORE_CTRL_2_T  se_core_ctrl_2;
    };
    MUINT32 set[COUNT];
} ISP_NVRAM_EE_T;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SE
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//
typedef union{
    enum { COUNT = 6 };
    struct {
        ISP_NVRAM_SEEE_TOP_CTRL_T         top_ctrl;
        ISP_NVRAM_SEEE_SE_Y_SPECL_CTRL_T  se_y_specl_ctrl;
        ISP_NVRAM_SEEE_SE_EDGE_CTRL_1_T  se_edge_ctrl_1;
        ISP_NVRAM_SEEE_SE_EDGE_CTRL_2_T  se_edge_ctrl_2;
        ISP_NVRAM_SEEE_SE_CORE_CTRL_1_T  se_core_ctrl_1;
        ISP_NVRAM_SEEE_SE_CORE_CTRL_2_T  se_core_ctrl_2;
    };
    MUINT32 set[COUNT];
} ISP_NVRAM_SE_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// NR3D
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct {
    FIELD  NR3D_CAM_TNR_EN                       :  1;      /*  0.. 0, 0x00000001 */
    FIELD  NR3D_CAM_TNR_C42_FILT_DIS             :  1;      /*  1.. 1, 0x00000002 */
    FIELD  NR3D_CAM_TNR_DCM_DIS                  :  1;      /*  2.. 2, 0x00000004 */
    FIELD  NR3D_CAM_TNR_RESET                    :  1;      /*  3.. 3, 0x00000008 */
    FIELD  NR3D_CAM_TNR_CROP_H                   :  4;      /*  4.. 7, 0x000000F0 */
    FIELD  NR3D_CAM_TNR_CROP_V                   :  4;      /*  8..11, 0x00000F00 */
    FIELD  NR3D_CAM_TNR_UV_SIGNED                :  1;      /* 12..12, 0x00001000 */
    FIELD  rsv_13                                : 19;      /* 13..31, 0xFFFFE000 */
} ISP_DIP_X_CAM_TNR_ENG_CON_T; //	/* 0x15022A30 */

typedef union {
    typedef ISP_DIP_X_CAM_TNR_ENG_CON_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CAM_TNR_ENG_CON_T;

typedef struct {
    FIELD  NR3D_CAM_TNR_HSIZE                    : 14;		/*  0..13, 0x00003FFF */
    FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
    FIELD  NR3D_CAM_TNR_VSIZE                    : 14;		/* 16..29, 0x3FFF0000 */
    FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
} ISP_DIP_X_CAM_TNR_SIZ_T; //	/* 0x15022A34 */

typedef union {
    typedef ISP_DIP_X_CAM_TNR_SIZ_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CAM_TNR_SIZ_T;

typedef struct {
    FIELD  NR3D_CAM_TNR_TILE_H                   : 14;		/*  0..13, 0x00003FFF */
    FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
    FIELD  NR3D_CAM_TNR_TILE_V                   : 14;		/* 16..29, 0x3FFF0000 */
    FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
} ISP_DIP_X_CAM_TNR_TILE_XY_T; //	/* 0x15022A38 */

typedef union {
    typedef ISP_DIP_X_CAM_TNR_TILE_XY_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CAM_TNR_TILE_XY_T;

typedef struct {
    FIELD  rsv_0                                 :  4;		/*  0.. 3, 0x0000000F */
    FIELD  NR3D_ON_EN                            :  1;		/*  4.. 4, 0x00000010 */
    FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
    FIELD  NR3D_TILE_EDGE                        :  4;		/*  8..11, 0x00000F00 */
    FIELD  rsv_12                                :  8;		/* 12..19, 0x000FF000 */
    FIELD  NR3D_SL2_OFF                          :  1;		/* 20..20, 0x00100000 */
    FIELD  rsv_21                                : 11;		/* 21..31, 0xFFE00000 */
} ISP_DIP_X_NR3D_ON_CON_T; //	/* 0x15022A3C */

typedef union {
    typedef ISP_DIP_X_NR3D_ON_CON_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_ON_CON_T;

typedef struct {
    FIELD  NR3D_ON_OFST_X                        : 14;		/*  0..13, 0x00003FFF */
    FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
    FIELD  NR3D_ON_OFST_Y                        : 14;		/* 16..29, 0x3FFF0000 */
    FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
} ISP_DIP_X_NR3D_ON_OFF_T; //	/* 0x15022A40 */

typedef union {
    typedef ISP_DIP_X_NR3D_ON_OFF_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_ON_OFF_T;

typedef struct {
    FIELD  NR3D_ON_WD                            : 14;		/*  0..13, 0x00003FFF */
    FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
    FIELD  NR3D_ON_HT                            : 14;		/* 16..29, 0x3FFF0000 */
    FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
} ISP_DIP_X_NR3D_ON_SIZ_T; //	/* 0x15022A44 */

typedef union {
    typedef ISP_DIP_X_NR3D_ON_SIZ_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_ON_SIZ_T;

typedef struct {
    FIELD  rsv_0                                 : 30;		/*  0..29, 0x3FFFFFFF */
    FIELD  NR3D_TNR_C_EN                         :  1;		/* 30..30, 0x40000000 */
    FIELD  NR3D_TNR_Y_EN                         :  1;		/* 31..31, 0x80000000 */
} ISP_DIP_X_MDP_TNR_TNR_ENABLE_T; //	/* 0x15022A48 */

typedef union {
    typedef ISP_DIP_X_MDP_TNR_TNR_ENABLE_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MDP_TNR_TNR_ENABLE_T;

typedef struct {
    FIELD  rsv_0                                 :  4;		/*  0.. 3, 0x0000000F */
    FIELD  NR3D_EN_CCR                           :  1;		/*  4.. 4, 0x00000010 */
    FIELD  NR3D_EN_CYCR                          :  1;		/*  5.. 5, 0x00000020 */
    FIELD  NR3D_FLT_STR_MAX                      :  6;		/*  6..11, 0x00000FC0 */
    FIELD  NR3D_BLEND_RATIO_MV                   :  5;		/* 12..16, 0x0001F000 */
    FIELD  NR3D_BLEND_RATIO_TXTR                 :  5;		/* 17..21, 0x003E0000 */
    FIELD  NR3D_BLEND_RATIO_DE                   :  5;		/* 22..26, 0x07C00000 */
    FIELD  NR3D_BLEND_RATIO_BLKY                 :  5;		/* 27..31, 0xF8000000 */
} ISP_DIP_X_MDP_TNR_FLT_CONFIG_T; //	/* 0x15022A4C */

typedef union {
    typedef ISP_DIP_X_MDP_TNR_FLT_CONFIG_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MDP_TNR_FLT_CONFIG_T;

typedef struct {
    FIELD  rsv_0                                 : 20;		/*  0..19, 0x000FFFFF */
    FIELD  NR3D_Q_SP                             :  6;		/* 20..25, 0x03F00000 */
    FIELD  NR3D_Q_NL                             :  6;		/* 26..31, 0xFC000000 */
} ISP_DIP_X_MDP_TNR_FB_INFO1_T; //	/* 0x15022A50 */

typedef union {
    typedef ISP_DIP_X_MDP_TNR_FB_INFO1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MDP_TNR_FB_INFO1_T;

typedef struct {
    FIELD  rsv_0                                 : 10;		/*  0.. 9, 0x000003FF */
    FIELD  NR3D_SMALL_SAD_THR                    :  6;		/* 10..15, 0x0000FC00 */
    FIELD  NR3D_MV_PEN_THR                       :  6;		/* 16..21, 0x003F0000 */
    FIELD  NR3D_MV_PEN_W                         :  4;		/* 22..25, 0x03C00000 */
    FIELD  NR3D_BDI_THR                          :  4;		/* 26..29, 0x3C000000 */
    FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
} ISP_DIP_X_MDP_TNR_THR_1_T; //	/* 0x15022A54 */

typedef union {
    typedef ISP_DIP_X_MDP_TNR_THR_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MDP_TNR_THR_1_T;

typedef struct {
    FIELD  rsv_0                                 :  2;		/*  0.. 1, 0x00000003 */
    FIELD  NR3D_Q_BLKY_Y4                        :  6;		/*  2.. 7, 0x000000FC */
    FIELD  NR3D_Q_BLKY_Y3                        :  6;		/*  8..13, 0x00003F00 */
    FIELD  NR3D_Q_BLKY_Y2                        :  6;		/* 14..19, 0x000FC000 */
    FIELD  NR3D_Q_BLKY_Y1                        :  6;		/* 20..25, 0x03F00000 */
    FIELD  NR3D_Q_BLKY_Y0                        :  6;		/* 26..31, 0xFC000000 */
} ISP_DIP_X_MDP_TNR_CURVE_1_T; //	/* 0x15022A58 */

typedef union {
    typedef ISP_DIP_X_MDP_TNR_CURVE_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MDP_TNR_CURVE_1_T;

typedef struct {
    FIELD  rsv_0                                 :  2;		/*  0.. 1, 0x00000003 */
    FIELD  NR3D_Q_BLKC_Y1                        :  6;		/*  2.. 7, 0x000000FC */
    FIELD  NR3D_Q_BLKC_Y0                        :  6;		/*  8..13, 0x00003F00 */
    FIELD  NR3D_Q_BLKY_Y7                        :  6;		/* 14..19, 0x000FC000 */
    FIELD  NR3D_Q_BLKY_Y6                        :  6;		/* 20..25, 0x03F00000 */
    FIELD  NR3D_Q_BLKY_Y5                        :  6;		/* 26..31, 0xFC000000 */
} ISP_DIP_X_MDP_TNR_CURVE_2_T; //	/* 0x15022A5C */

typedef union {
    typedef ISP_DIP_X_MDP_TNR_CURVE_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MDP_TNR_CURVE_2_T;

typedef struct {
    FIELD  rsv_0                                 :  2;		/*  0.. 1, 0x00000003 */
    FIELD  NR3D_Q_BLKC_Y6                        :  6;		/*  2.. 7, 0x000000FC */
    FIELD  NR3D_Q_BLKC_Y5                        :  6;		/*  8..13, 0x00003F00 */
    FIELD  NR3D_Q_BLKC_Y4                        :  6;		/* 14..19, 0x000FC000 */
    FIELD  NR3D_Q_BLKC_Y3                        :  6;		/* 20..25, 0x03F00000 */
    FIELD  NR3D_Q_BLKC_Y2                        :  6;		/* 26..31, 0xFC000000 */
} ISP_DIP_X_MDP_TNR_CURVE_3_T; //	/* 0x15022A60 */

typedef union {
    typedef ISP_DIP_X_MDP_TNR_CURVE_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MDP_TNR_CURVE_3_T;

typedef struct {
    FIELD  rsv_0                                 :  2;		/*  0.. 1, 0x00000003 */
    FIELD  NR3D_Q_DETXTR_LVL_Y3                  :  6;		/*  2.. 7, 0x000000FC */
    FIELD  NR3D_Q_DETXTR_LVL_Y2                  :  6;		/*  8..13, 0x00003F00 */
    FIELD  NR3D_Q_DETXTR_LVL_Y1                  :  6;		/* 14..19, 0x000FC000 */
    FIELD  NR3D_Q_DETXTR_LVL_Y0                  :  6;		/* 20..25, 0x03F00000 */
    FIELD  NR3D_Q_BLKC_Y7                        :  6;		/* 26..31, 0xFC000000 */
} ISP_DIP_X_MDP_TNR_CURVE_4_T; //	/* 0x15022A64 */

typedef union {
    typedef ISP_DIP_X_MDP_TNR_CURVE_4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MDP_TNR_CURVE_4_T;

typedef struct {
    FIELD  rsv_0                                 :  2;		/*  0.. 1, 0x00000003 */
    FIELD  NR3D_Q_DE1_BASE_Y0                    :  6;		/*  2.. 7, 0x000000FC */
    FIELD  NR3D_Q_DETXTR_LVL_Y7                  :  6;		/*  8..13, 0x00003F00 */
    FIELD  NR3D_Q_DETXTR_LVL_Y6                  :  6;		/* 14..19, 0x000FC000 */
    FIELD  NR3D_Q_DETXTR_LVL_Y5                  :  6;		/* 20..25, 0x03F00000 */
    FIELD  NR3D_Q_DETXTR_LVL_Y4                  :  6;		/* 26..31, 0xFC000000 */
} ISP_DIP_X_MDP_TNR_CURVE_5_T; //	/* 0x15022A68 */

typedef union {
    typedef ISP_DIP_X_MDP_TNR_CURVE_5_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MDP_TNR_CURVE_5_T;

typedef struct {
    FIELD  rsv_0                                 :  2;		/*  0.. 1, 0x00000003 */
    FIELD  NR3D_Q_DE1_BASE_Y5                    :  6;		/*  2.. 7, 0x000000FC */
    FIELD  NR3D_Q_DE1_BASE_Y4                    :  6;		/*  8..13, 0x00003F00 */
    FIELD  NR3D_Q_DE1_BASE_Y3                    :  6;		/* 14..19, 0x000FC000 */
    FIELD  NR3D_Q_DE1_BASE_Y2                    :  6;		/* 20..25, 0x03F00000 */
    FIELD  NR3D_Q_DE1_BASE_Y1                    :  6;		/* 26..31, 0xFC000000 */
} ISP_DIP_X_MDP_TNR_CURVE_6_T; //	/* 0x15022A6C */

typedef union {
    typedef ISP_DIP_X_MDP_TNR_CURVE_6_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MDP_TNR_CURVE_6_T;

typedef struct {
    FIELD  rsv_0                                 :  2;		/*  0.. 1, 0x00000003 */
    FIELD  NR3D_Q_DE2TXTR_BASE_Y2                :  6;		/*  2.. 7, 0x000000FC */
    FIELD  NR3D_Q_DE2TXTR_BASE_Y1                :  6;		/*  8..13, 0x00003F00 */
    FIELD  NR3D_Q_DE2TXTR_BASE_Y0                :  6;		/* 14..19, 0x000FC000 */
    FIELD  NR3D_Q_DE1_BASE_Y7                    :  6;		/* 20..25, 0x03F00000 */
    FIELD  NR3D_Q_DE1_BASE_Y6                    :  6;		/* 26..31, 0xFC000000 */
} ISP_DIP_X_MDP_TNR_CURVE_7_T; //	/* 0x15022A70 */

typedef union {
    typedef ISP_DIP_X_MDP_TNR_CURVE_7_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MDP_TNR_CURVE_7_T;

typedef struct {
    FIELD  rsv_0                                 :  2;		/*  0.. 1, 0x00000003 */
    FIELD  NR3D_Q_DE2TXTR_BASE_Y7                :  6;		/*  2.. 7, 0x000000FC */
    FIELD  NR3D_Q_DE2TXTR_BASE_Y6                :  6;		/*  8..13, 0x00003F00 */
    FIELD  NR3D_Q_DE2TXTR_BASE_Y5                :  6;		/* 14..19, 0x000FC000 */
    FIELD  NR3D_Q_DE2TXTR_BASE_Y4                :  6;		/* 20..25, 0x03F00000 */
    FIELD  NR3D_Q_DE2TXTR_BASE_Y3                :  6;		/* 26..31, 0xFC000000 */
} ISP_DIP_X_MDP_TNR_CURVE_8_T; //	/* 0x15022A74 */

typedef union {
    typedef ISP_DIP_X_MDP_TNR_CURVE_8_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MDP_TNR_CURVE_8_T;

typedef struct {
    FIELD  rsv_0                                 :  2;		/*  0.. 1, 0x00000003 */
    FIELD  NR3D_Q_MV_Y4                          :  6;		/*  2.. 7, 0x000000FC */
    FIELD  NR3D_Q_MV_Y3                          :  6;		/*  8..13, 0x00003F00 */
    FIELD  NR3D_Q_MV_Y2                          :  6;		/* 14..19, 0x000FC000 */
    FIELD  NR3D_Q_MV_Y1                          :  6;		/* 20..25, 0x03F00000 */
    FIELD  NR3D_Q_MV_Y0                          :  6;		/* 26..31, 0xFC000000 */
} ISP_DIP_X_MDP_TNR_CURVE_9_T; //	/* 0x15022A78 */

typedef union {
    typedef ISP_DIP_X_MDP_TNR_CURVE_9_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MDP_TNR_CURVE_9_T;

typedef struct {
    FIELD  rsv_0                                 :  2;		/*  0.. 1, 0x00000003 */
    FIELD  NR3D_Q_WVAR_Y1                        :  6;		/*  2.. 7, 0x000000FC */
    FIELD  NR3D_Q_WVAR_Y0                        :  6;		/*  8..13, 0x00003F00 */
    FIELD  NR3D_Q_MV_Y7                          :  6;		/* 14..19, 0x000FC000 */
    FIELD  NR3D_Q_MV_Y6                          :  6;		/* 20..25, 0x03F00000 */
    FIELD  NR3D_Q_MV_Y5                          :  6;		/* 26..31, 0xFC000000 */
} ISP_DIP_X_MDP_TNR_CURVE_10_T; //	/* 0x15022A7C */

typedef union {
    typedef ISP_DIP_X_MDP_TNR_CURVE_10_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MDP_TNR_CURVE_10_T;

typedef struct {
    FIELD  rsv_0                                 :  2;		/*  0.. 1, 0x00000003 */
    FIELD  NR3D_Q_WVAR_Y6                        :  6;		/*  2.. 7, 0x000000FC */
    FIELD  NR3D_Q_WVAR_Y5                        :  6;		/*  8..13, 0x00003F00 */
    FIELD  NR3D_Q_WVAR_Y4                        :  6;		/* 14..19, 0x000FC000 */
    FIELD  NR3D_Q_WVAR_Y3                        :  6;		/* 20..25, 0x03F00000 */
    FIELD  NR3D_Q_WVAR_Y2                        :  6;		/* 26..31, 0xFC000000 */
} ISP_DIP_X_MDP_TNR_CURVE_11_T; //	/* 0x15022A80 */

typedef union {
    typedef ISP_DIP_X_MDP_TNR_CURVE_11_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MDP_TNR_CURVE_11_T;

typedef struct {
    FIELD  rsv_0                                 :  2;		/*  0.. 1, 0x00000003 */
    FIELD  NR3D_Q_WSM_Y3                         :  6;		/*  2.. 7, 0x000000FC */
    FIELD  NR3D_Q_WSM_Y2                         :  6;		/*  8..13, 0x00003F00 */
    FIELD  NR3D_Q_WSM_Y1                         :  6;		/* 14..19, 0x000FC000 */
    FIELD  NR3D_Q_WSM_Y0                         :  6;		/* 20..25, 0x03F00000 */
    FIELD  NR3D_Q_WVAR_Y7                        :  6;		/* 26..31, 0xFC000000 */
} ISP_DIP_X_MDP_TNR_CURVE_12_T; //	/* 0x15022A84 */

typedef union {
    typedef ISP_DIP_X_MDP_TNR_CURVE_12_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MDP_TNR_CURVE_12_T;

typedef struct {
    FIELD  rsv_0                                 :  8;		/*  0.. 7, 0x000000FF */
    FIELD  NR3D_Q_WSM_Y7                         :  6;		/*  8..13, 0x00003F00 */
    FIELD  NR3D_Q_WSM_Y6                         :  6;		/* 14..19, 0x000FC000 */
    FIELD  NR3D_Q_WSM_Y5                         :  6;		/* 20..25, 0x03F00000 */
    FIELD  NR3D_Q_WSM_Y4                         :  6;		/* 26..31, 0xFC000000 */
} ISP_DIP_X_MDP_TNR_CURVE_13_T; //	/* 0x15022A88 */

typedef union {
    typedef ISP_DIP_X_MDP_TNR_CURVE_13_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MDP_TNR_CURVE_13_T;

typedef struct {
    FIELD  rsv_0                                 :  2;		/*  0.. 1, 0x00000003 */
    FIELD  NR3D_Q_SDL_Y4                         :  6;		/*  2.. 7, 0x000000FC */
    FIELD  NR3D_Q_SDL_Y3                         :  6;		/*  8..13, 0x00003F00 */
    FIELD  NR3D_Q_SDL_Y2                         :  6;		/* 14..19, 0x000FC000 */
    FIELD  NR3D_Q_SDL_Y1                         :  6;		/* 20..25, 0x03F00000 */
    FIELD  NR3D_Q_SDL_Y0                         :  6;		/* 26..31, 0xFC000000 */
} ISP_DIP_X_MDP_TNR_CURVE_14_T; //	/* 0x15022A8C */

typedef union {
    typedef ISP_DIP_X_MDP_TNR_CURVE_14_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MDP_TNR_CURVE_14_T;

typedef struct {
    FIELD  rsv_0                                 :  8;		/*  0.. 7, 0x000000FF */
    FIELD  NR3D_Q_SDL_Y8                         :  6;		/*  8..13, 0x00003F00 */
    FIELD  NR3D_Q_SDL_Y7                         :  6;		/* 14..19, 0x000FC000 */
    FIELD  NR3D_Q_SDL_Y6                         :  6;		/* 20..25, 0x03F00000 */
    FIELD  NR3D_Q_SDL_Y5                         :  6;		/* 26..31, 0xFC000000 */
} ISP_DIP_X_MDP_TNR_CURVE_15_T; //	/* 0x15022A90 */

typedef union {
    typedef ISP_DIP_X_MDP_TNR_CURVE_15_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MDP_TNR_CURVE_15_T;

typedef struct {
    FIELD  NR3D_R2CENC                           :  2;		/*  0.. 1, 0x00000003 */
    FIELD  NR3D_R2C_VAL4                         :  6;		/*  2.. 7, 0x000000FC */
    FIELD  NR3D_R2C_VAL3                         :  6;		/*  8..13, 0x00003F00 */
    FIELD  NR3D_R2C_VAL2                         :  6;		/* 14..19, 0x000FC000 */
    FIELD  NR3D_R2C_VAL1                         :  6;		/* 20..25, 0x03F00000 */
    FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
} ISP_DIP_X_MDP_TNR_R2C_1_T; //	/* 0x15022A94 */

typedef union {
    typedef ISP_DIP_X_MDP_TNR_R2C_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MDP_TNR_R2C_1_T;

typedef struct {
    FIELD  rsv_0                                 :  2;		/*  0.. 1, 0x00000003 */
    FIELD  NR3D_R2C_TXTR_THROFF                  :  6;		/*  2.. 7, 0x000000FC */
    FIELD  NR3D_R2C_TXTR_THR4                    :  6;		/*  8..13, 0x00003F00 */
    FIELD  NR3D_R2C_TXTR_THR3                    :  6;		/* 14..19, 0x000FC000 */
    FIELD  NR3D_R2C_TXTR_THR2                    :  6;		/* 20..25, 0x03F00000 */
    FIELD  NR3D_R2C_TXTR_THR1                    :  6;		/* 26..31, 0xFC000000 */
} ISP_DIP_X_MDP_TNR_R2C_2_T; //	/* 0x15022A98 */

typedef union {
    typedef ISP_DIP_X_MDP_TNR_R2C_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MDP_TNR_R2C_2_T;

typedef struct {
    FIELD  rsv_0                                 :  8;		/*  0.. 7, 0x000000FF */
    FIELD  NR3D_R2CF_CNT4                        :  6;		/*  8..13, 0x00003F00 */
    FIELD  NR3D_R2CF_CNT3                        :  6;		/* 14..19, 0x000FC000 */
    FIELD  NR3D_R2CF_CNT2                        :  6;		/* 20..25, 0x03F00000 */
    FIELD  NR3D_R2CF_CNT1                        :  6;		/* 26..31, 0xFC000000 */
} ISP_DIP_X_MDP_TNR_R2C_3_T; //	/* 0x15022A9C */

typedef union {
    typedef ISP_DIP_X_MDP_TNR_R2C_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MDP_TNR_R2C_3_T;

typedef struct {
    FIELD  rsv_0                                 :  2;		/*  0.. 1, 0x00000003 */
    FIELD  NR3D_FORCE_EN                         :  1;		/*  2.. 2, 0x00000004 */
    FIELD  NR3D_INK_LEVEL_DISP                   : 16;		/*  3..18, 0x0007FFF8 */
    FIELD  NR3D_INK_SEL                          :  5;		/* 19..23, 0x00F80000 */
    FIELD  NR3D_INK_Y_EN                         :  1;		/* 24..24, 0x01000000 */
    FIELD  NR3D_INK_EN                           :  1;		/* 25..25, 0x02000000 */
    FIELD  NR3D_FORCE_FLT_STR                    :  6;		/* 26..31, 0xFC000000 */
} ISP_DIP_X_MDP_TNR_DBG_6_T; //	/* 0x15022AA0 */

typedef union {
    typedef ISP_DIP_X_MDP_TNR_DBG_6_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MDP_TNR_DBG_6_T;

typedef struct {
    FIELD  rsv_0                                 :  2;		/*  0.. 1, 0x00000003 */
    FIELD  NR3D_OSD_TARGV                        : 13;		/*  2..14, 0x00007FFC */
    FIELD  NR3D_OSD_TARGH                        : 13;		/* 15..27, 0x0FFF8000 */
    FIELD  NR3D_OSD_SEL                          :  3;		/* 28..30, 0x70000000 */
    FIELD  NR3D_OSD_EN                           :  1;		/* 31..31, 0x80000000 */
} ISP_DIP_X_MDP_TNR_DBG_15_T; //	/* 0x15022AA4 */

typedef union {
    typedef ISP_DIP_X_MDP_TNR_DBG_15_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MDP_TNR_DBG_15_T;

typedef struct {
    FIELD  rsv_0                                 :  3;		/*  0.. 2, 0x00000007 */
    FIELD  NR3D_OSD_Y_EN                         :  1;		/*  3.. 3, 0x00000008 */
    FIELD  NR3D_OSD_DISP_SCALE                   :  2;		/*  4.. 5, 0x00000030 */
    FIELD  NR3D_OSD_DISPV                        : 13;		/*  6..18, 0x0007FFC0 */
    FIELD  NR3D_OSD_DISPH                        : 13;		/* 19..31, 0xFFF80000 */
} ISP_DIP_X_MDP_TNR_DBG_16_T; //	/* 0x15022AA8 */

typedef union {
    typedef ISP_DIP_X_MDP_TNR_DBG_16_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MDP_TNR_DBG_16_T;

typedef struct {
    FIELD  rsv_0                                 :  4;		/*  0.. 3, 0x0000000F */
    FIELD  NR3D_DEMO_BOT                         : 13;		/*  4..16, 0x0001FFF0 */
    FIELD  NR3D_DEMO_TOP                         : 13;		/* 17..29, 0x3FFE0000 */
    FIELD  NR3D_DEMO_SEL                         :  1;		/* 30..30, 0x40000000 */
    FIELD  NR3D_DEMO_EN                          :  1;		/* 31..31, 0x80000000 */
} ISP_DIP_X_MDP_TNR_DEMO_1_T; //	/* 0x15022AAC */

typedef union {
    typedef ISP_DIP_X_MDP_TNR_DEMO_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MDP_TNR_DEMO_1_T;

typedef struct {
    FIELD  rsv_0                                 :  6;		/*  0.. 5, 0x0000003F */
    FIELD  NR3D_DEMO_RIGHT                       : 13;		/*  6..18, 0x0007FFC0 */
    FIELD  NR3D_DEMO_LEFT                        : 13;		/* 19..31, 0xFFF80000 */
} ISP_DIP_X_MDP_TNR_DEMO_2_T; //	/* 0x15022AB0 */

typedef union {
    typedef ISP_DIP_X_MDP_TNR_DEMO_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MDP_TNR_DEMO_2_T;

typedef struct {
    FIELD  NR3D_TNR_ATPG_OB                      :  1;		/*  0.. 0, 0x00000001 */
    FIELD  NR3D_TNR_ATPG_CT                      :  1;		/*  1.. 1, 0x00000002 */
    FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
} ISP_DIP_X_MDP_TNR_ATPG_T; //	/* 0x15022AB4 */

typedef union {
    typedef ISP_DIP_X_MDP_TNR_ATPG_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MDP_TNR_ATPG_T;

typedef struct {
    FIELD  NR3D_TNR_DUMMY0                       : 32;		/*  0..31, 0xFFFFFFFF */
} ISP_DIP_X_MDP_TNR_DMY_0_T; //	/* 0x15022AB8 */

typedef union {
    typedef ISP_DIP_X_MDP_TNR_DMY_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MDP_TNR_DMY_0_T;

typedef struct {
    FIELD  rsv_0                                 :  4;		/*  0.. 3, 0x0000000F */
    FIELD  NR3D_OSD_INF1                         : 12;		/*  4..15, 0x0000FFF0 */
    FIELD  NR3D_OSD_INF0                         : 12;		/* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
} ISP_DIP_X_MDP_TNR_DBG_17_T; //	/* 0x15022ABC */

typedef union {
    typedef ISP_DIP_X_MDP_TNR_DBG_17_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MDP_TNR_DBG_17_T;

typedef struct {
    FIELD  rsv_0                                 :  8;		/*  0.. 7, 0x000000FF */
    FIELD  NR3D_OSD_INF3                         : 12;		/*  8..19, 0x000FFF00 */
    FIELD  NR3D_OSD_INF2                         : 12;		/* 20..31, 0xFFF00000 */
} ISP_DIP_X_MDP_TNR_DBG_18_T; //	/* 0x15022AC0 */

typedef union {
    typedef ISP_DIP_X_MDP_TNR_DBG_18_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MDP_TNR_DBG_18_T;

typedef struct {
    FIELD  NR3D_TNR_DBG0                         : 32;		/*  0..31, 0xFFFFFFFF */
} ISP_DIP_X_MDP_TNR_DBG_1_T; //	/* 0x15022AC4 */

typedef union {
    typedef ISP_DIP_X_MDP_TNR_DBG_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MDP_TNR_DBG_1_T;

typedef struct {
    FIELD  NR3D_TNR_DBG1                         : 32;		/*  0..31, 0xFFFFFFFF */
} ISP_DIP_X_MDP_TNR_DBG_2_T; //	/* 0x15022AC8 */

typedef union {
    typedef ISP_DIP_X_MDP_TNR_DBG_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MDP_TNR_DBG_2_T;

typedef union {
    enum { COUNT = 21 };
    struct {
        //ISP_NVRAM_CAM_TNR_ENG_CON_T tnr_eng_con;
        //ISP_NVRAM_CAM_TNR_SIZ_T  tnr_siz;
        //ISP_NVRAM_CAM_TNR_TILE_XY_T  tnr_tile_xy;
        ISP_NVRAM_NR3D_ON_CON_T  on_con;
        //ISP_NVRAM_NR3D_ON_OFF_T  on_off;
        //ISP_NVRAM_NR3D_ON_SIZ_T  on_siz;
        ISP_NVRAM_MDP_TNR_TNR_ENABLE_T  tnr_tnr_enable;
        ISP_NVRAM_MDP_TNR_FLT_CONFIG_T  tnr_flt_config;
        ISP_NVRAM_MDP_TNR_FB_INFO1_T  tnr_fb_info1;
        ISP_NVRAM_MDP_TNR_THR_1_T  tnr_thr_1;
        ISP_NVRAM_MDP_TNR_CURVE_1_T  tnr_curve_1;
        ISP_NVRAM_MDP_TNR_CURVE_2_T  tnr_curve_2;
        ISP_NVRAM_MDP_TNR_CURVE_3_T  tnr_curve_3;
        ISP_NVRAM_MDP_TNR_CURVE_4_T  tnr_curve_4;
        ISP_NVRAM_MDP_TNR_CURVE_5_T  tnr_curve_5;
        ISP_NVRAM_MDP_TNR_CURVE_6_T  tnr_curve_6;
        ISP_NVRAM_MDP_TNR_CURVE_7_T  tnr_curve_7;
        ISP_NVRAM_MDP_TNR_CURVE_8_T  tnr_curve_8;
        ISP_NVRAM_MDP_TNR_CURVE_9_T  tnr_curve_9;
        ISP_NVRAM_MDP_TNR_CURVE_10_T  tnr_curve_10;
        ISP_NVRAM_MDP_TNR_CURVE_11_T  tnr_curve_11;
        ISP_NVRAM_MDP_TNR_CURVE_12_T  tnr_curve_12;
        ISP_NVRAM_MDP_TNR_CURVE_13_T  tnr_curve_13;
        ISP_NVRAM_MDP_TNR_CURVE_14_T  tnr_curve_14;
        ISP_NVRAM_MDP_TNR_CURVE_15_T  tnr_curve_15;
        ISP_NVRAM_MDP_TNR_R2C_1_T  tnr_r2c_1;
        ISP_NVRAM_MDP_TNR_R2C_2_T  tnr_r2c_2;
        ISP_NVRAM_MDP_TNR_R2C_3_T  tnr_r2c_3;
        //ISP_NVRAM_MDP_TNR_DBG_6_T  tnr_dbg_6;
        //ISP_NVRAM_MDP_TNR_DBG_15_T  tnr_dbg_15;
        //ISP_NVRAM_MDP_TNR_DBG_16_T  tnr_dbg_16;
        //ISP_NVRAM_MDP_TNR_DEMO_1_T  tnr_demo_1;
        //ISP_NVRAM_MDP_TNR_DEMO_2_T  tnr_demo_2;
        //ISP_NVRAM_MDP_TNR_ATPG_T  tnr_atpg;
        //ISP_NVRAM_MDP_TNR_DMY_0_T  tnr_dmy_0;
        //ISP_NVRAM_MDP_TNR_DBG_17_T  tnr_dbg_17;
        //ISP_NVRAM_MDP_TNR_DBG_18_T  tnr_dbg_18;
        //ISP_NVRAM_MDP_TNR_DBG_1_T  tnr_dbg_1;
        //ISP_NVRAM_MDP_TNR_DBG_2_T  tnr_dbg_2;
    };
    MUINT32 set[COUNT];
} ISP_NVRAM_NR3D_T;

typedef union {
    enum { COUNT = 39 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_CAM_TNR_ENG_CON_T tnr_eng_con;
        ISP_NVRAM_CAM_TNR_SIZ_T  tnr_siz;
        ISP_NVRAM_CAM_TNR_TILE_XY_T  tnr_tile_xy;
        ISP_NVRAM_NR3D_ON_CON_T  on_con;
        ISP_NVRAM_NR3D_ON_OFF_T  on_off;
        ISP_NVRAM_NR3D_ON_SIZ_T  on_siz;
        ISP_NVRAM_MDP_TNR_TNR_ENABLE_T  tnr_tnr_enable;
        ISP_NVRAM_MDP_TNR_FLT_CONFIG_T  tnr_flt_config;
        ISP_NVRAM_MDP_TNR_FB_INFO1_T  tnr_fb_info1;
        ISP_NVRAM_MDP_TNR_THR_1_T  tnr_thr_1;
        ISP_NVRAM_MDP_TNR_CURVE_1_T  tnr_curve_1;
        ISP_NVRAM_MDP_TNR_CURVE_2_T  tnr_curve_2;
        ISP_NVRAM_MDP_TNR_CURVE_3_T  tnr_curve_3;
        ISP_NVRAM_MDP_TNR_CURVE_4_T  tnr_curve_4;
        ISP_NVRAM_MDP_TNR_CURVE_5_T  tnr_curve_5;
        ISP_NVRAM_MDP_TNR_CURVE_6_T  tnr_curve_6;
        ISP_NVRAM_MDP_TNR_CURVE_7_T  tnr_curve_7;
        ISP_NVRAM_MDP_TNR_CURVE_8_T  tnr_curve_8;
        ISP_NVRAM_MDP_TNR_CURVE_9_T  tnr_curve_9;
        ISP_NVRAM_MDP_TNR_CURVE_10_T  tnr_curve_10;
        ISP_NVRAM_MDP_TNR_CURVE_11_T  tnr_curve_11;
        ISP_NVRAM_MDP_TNR_CURVE_12_T  tnr_curve_12;
        ISP_NVRAM_MDP_TNR_CURVE_13_T  tnr_curve_13;
        ISP_NVRAM_MDP_TNR_CURVE_14_T  tnr_curve_14;
        ISP_NVRAM_MDP_TNR_CURVE_15_T  tnr_curve_15;
        ISP_NVRAM_MDP_TNR_R2C_1_T  tnr_r2c_1;
        ISP_NVRAM_MDP_TNR_R2C_2_T  tnr_r2c_2;
        ISP_NVRAM_MDP_TNR_R2C_3_T  tnr_r2c_3;
        ISP_NVRAM_MDP_TNR_DBG_6_T  tnr_dbg_6;
        ISP_NVRAM_MDP_TNR_DBG_15_T  tnr_dbg_15;
        ISP_NVRAM_MDP_TNR_DBG_16_T  tnr_dbg_16;
        ISP_NVRAM_MDP_TNR_DEMO_1_T  tnr_demo_1;
        ISP_NVRAM_MDP_TNR_DEMO_2_T  tnr_demo_2;
        ISP_NVRAM_MDP_TNR_ATPG_T  tnr_atpg;
        ISP_NVRAM_MDP_TNR_DMY_0_T  tnr_dmy_0;
        ISP_NVRAM_MDP_TNR_DBG_17_T  tnr_dbg_17;
        ISP_NVRAM_MDP_TNR_DBG_18_T  tnr_dbg_18;
        ISP_NVRAM_MDP_TNR_DBG_1_T  tnr_dbg_1;
        ISP_NVRAM_MDP_TNR_DBG_2_T  tnr_dbg_2;
    };
} ISP_NVRAM_NR3D_GET_T;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Mixer3
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// DIP_X_MIX3_CTRL_0 CAM+CB0H
typedef struct {
        FIELD  MIX3_WT_SEL             :  1;      /*  0.. 0, 0x00000001 */
        FIELD  rsv_1                   :  3;      /*  1.. 3, 0x0000000E */
        FIELD  MIX3_Y_EN               :  1;      /*  4.. 4, 0x00000010 */
        FIELD  MIX3_Y_DEFAULT          :  1;      /*  5.. 5, 0x00000020 */
        FIELD  MIX3_UV_EN              :  1;      /*  6.. 6, 0x00000040 */
        FIELD  MIX3_UV_DEFAULT         :  1;      /*  7.. 7, 0x00000080 */
        FIELD  MIX3_B0                 :  8;      /*  8..15, 0x0000FF00 */
        FIELD  MIX3_B1                 :  8;      /* 16..23, 0x00FF0000 */
        FIELD  MIX3_DT                 :  8;      /* 24..31, 0xFF000000 */
} ISP_DIP_X_MIX3_CTRL_0_T;

typedef union {
    enum { MASK     = 0xFFFFFF01 };
    typedef ISP_DIP_X_MIX3_CTRL_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MIX3_CTRL_0_T;

// DIP_X_MIX3_CTRL_1 CAM+CB4H
typedef struct {
        FIELD MIX3_M0                   : 8;
        FIELD rsv_8                     : 8;
        FIELD MIX3_M1                   : 8;
        FIELD rsv_24                    : 8;
} ISP_DIP_X_MIX3_CTRL_1_T;

typedef union {
    enum { MASK     = 0x00FF00FF };
    typedef ISP_DIP_X_MIX3_CTRL_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MIX3_CTRL_1_T;

// DIP_X_MIX3_SPARE CAM+CB8H
typedef struct {
        FIELD MIX3_SPARE                : 32;
} ISP_DIP_X_MIX3_SPARE_T;

typedef union {
    enum { MASK     = 0xFFFFFFFF };
    typedef ISP_DIP_X_MIX3_SPARE_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MIX3_SPARE_T;

typedef union {
    enum { COUNT = 2 };
    struct {
        ISP_NVRAM_MIX3_CTRL_0_T  ctrl_0;
        ISP_NVRAM_MIX3_CTRL_1_T  ctrl_1;
        //ISP_NVRAM_MIX3_SPARE_T   spare;
    };
    MUINT32 set[COUNT];
} ISP_NVRAM_MIXER3_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// FLC
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct {
    FIELD  FLC_OFST_B                            : 15;		/*  0..12, 0x00001FFF */
    FIELD  rsv_13                                : 17;		/* 13..31, 0xFFFFE000 */
} ISP_DIP_X_FLC_OFFST0_T; //	/* 0x15022FC0 */

typedef union {
    typedef ISP_DIP_X_FLC_OFFST0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_FLC_OFFST0_T;

typedef struct {
    FIELD  FLC_OFST_G                            : 15;		/*  0..12, 0x00001FFF */
    FIELD  rsv_13                                : 17;		/* 13..31, 0xFFFFE000 */
} ISP_DIP_X_FLC_OFFST1_T; //	/* 0x15022FC4 */

typedef union {
    typedef ISP_DIP_X_FLC_OFFST1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_FLC_OFFST1_T;

typedef struct {
    FIELD  FLC_OFST_R                            : 15;		/*  0..12, 0x00001FFF */
    FIELD  rsv_13                                : 17;		/* 13..31, 0xFFFFE000 */
} ISP_DIP_X_FLC_OFFST2_T; //	/* 0x15022FC8 */

typedef union {
    typedef ISP_DIP_X_FLC_OFFST2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_FLC_OFFST2_T;

typedef struct {
    FIELD  FLC_GAIN_B                            : 13;		/*  0..12, 0x00001FFF */
    FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
} ISP_DIP_X_FLC_GAIN0_T; //	/* 0x15022FD0 */

typedef union {
    typedef ISP_DIP_X_FLC_GAIN0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_FLC_GAIN0_T;

typedef struct {
    FIELD  FLC_GAIN_G                            : 13;		/*  0..12, 0x00001FFF */
    FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
} ISP_DIP_X_FLC_GAIN1_T; //	/* 0x15022FD4 */

typedef union {
    typedef ISP_DIP_X_FLC_GAIN1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_FLC_GAIN1_T;

typedef struct {
    FIELD  FLC_GAIN_R                            : 13;		/*  0..12, 0x00001FFF */
    FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
} ISP_DIP_X_FLC_GAIN2_T; //	/* 0x15022FD8 */

typedef union {
    typedef ISP_DIP_X_FLC_GAIN2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_FLC_GAIN2_T;

typedef union {
    enum { COUNT = 6 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_FLC_OFFST0_T  offst0;
        ISP_NVRAM_FLC_OFFST1_T  offst1;
        ISP_NVRAM_FLC_OFFST2_T  offst2;
        ISP_NVRAM_FLC_GAIN0_T  gain0;
        ISP_NVRAM_FLC_GAIN1_T  gain1;
        ISP_NVRAM_FLC_GAIN2_T  gain2;
    };
} ISP_NVRAM_FLC_T;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// AWB Statistics @ TG1
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CAM_AWB_WIN_ORG CAM+05B0H
typedef struct {
        FIELD AWB_W_HORG                : 13;
        FIELD rsv_13                    : 3;
        FIELD AWB_W_VORG                : 13;
        FIELD rsv_29                    : 3;
} ISP_CAM_AWB_WIN_ORG_T;

typedef union {
    enum { MASK     = 0x1FFF1FFF };
    typedef ISP_CAM_AWB_WIN_ORG_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_WIN_ORG_T;

// CAM_AWB_WIN_SIZE CAM+05B4H
typedef struct {
        FIELD AWB_W_HSIZE               : 13;
        FIELD rsv_13                    : 3;
        FIELD AWB_W_VSIZE               : 13;
        FIELD rsv_29                    : 3;
} ISP_CAM_AWB_WIN_SIZE_T;

typedef union {
    enum { MASK     = 0x1FFF1FFF };
    typedef ISP_CAM_AWB_WIN_SIZE_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_WIN_SIZE_T;

// CAM_AWB_WIN_PIT CAM+05B8H
typedef struct {
        FIELD AWB_W_HPIT                : 13;
        FIELD rsv_13                    : 3;
        FIELD AWB_W_VPIT                : 13;
        FIELD rsv_29                    : 3;
} ISP_CAM_AWB_WIN_PIT_T;

typedef union {
    enum { MASK     = 0x1FFF1FFF };
    typedef ISP_CAM_AWB_WIN_PIT_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_WIN_PIT_T;

// CAM_AWB_WIN_NUM CAM+05BCH
typedef struct {
        FIELD AWB_W_HNUM                : 8;
        FIELD rsv_8                     : 8;
        FIELD AWB_W_VNUM                : 8;
        FIELD rsv_24                    : 8;
} ISP_CAM_AWB_WIN_NUM_T;

typedef union {
    enum { MASK     = 0x00FF00FF };
    typedef ISP_CAM_AWB_WIN_NUM_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_WIN_NUM_T;

// CAM_AWB_GAIN1_0 CAM+05COH
typedef struct {
        FIELD AWB_GAIN1_R               : 13;
        FIELD rsv_13                    : 3;
        FIELD AWB_GAIN1_G               : 13;
        FIELD rsv_29                    : 3;
} ISP_CAM_AWB_GAIN1_0_T;

typedef union {
    enum { MASK     = 0x1FFF1FFF };
    typedef ISP_CAM_AWB_GAIN1_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_GAIN1_0_T;

// CAM_AWB_GAIN1_1 CAM+05C4H
typedef struct {
        FIELD AWB_GAIN1_B               : 13;
        FIELD rsv_13                    : 19;
} ISP_CAM_AWB_GAIN1_1_T;

typedef union {
    enum { MASK     = 0x00001FFF };
    typedef ISP_CAM_AWB_GAIN1_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_GAIN1_1_T;

// CAM_AWB_LMT1_0 CAM+05C8H
typedef struct {
        FIELD AWB_LMT1_R                : 12;
        FIELD rsv_12                    : 4;
        FIELD AWB_LMT1_G                : 12;
        FIELD rsv_28                    : 4;
} ISP_CAM_AWB_LMT1_0_T;

typedef union {
    enum { MASK     = 0x0FFF0FFF };
    typedef ISP_CAM_AWB_LMT1_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_LMT1_0_T;

// CAM_AWB_LMT1_1 CAM+05CCH
typedef struct {
        FIELD AWB_LMT1_B                : 12;
        FIELD rsv_12                    : 20;
} ISP_CAM_AWB_LMT1_1_T;

typedef union {
    enum { MASK     = 0x00000FFF };
    typedef ISP_CAM_AWB_LMT1_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_LMT1_1_T;

// CAM_AWB_LOW_THR CAM+05D0H
typedef struct {
        FIELD AWB_LOW_THR0              : 8;
        FIELD AWB_LOW_THR1              : 8;
        FIELD AWB_LOW_THR2              : 8;
        FIELD rsv_24                    : 8;
} ISP_CAM_AWB_LOW_THR_T;

typedef union {
    enum { MASK     = 0x00FFFFFF };
    typedef ISP_CAM_AWB_LOW_THR_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_LOW_THR_T;

// CAM_AWB_HI_THR CAM+05D4H
typedef struct {
        FIELD AWB_HI_THR0               : 8;
        FIELD AWB_HI_THR1               : 8;
        FIELD AWB_HI_THR2               : 8;
        FIELD rsv_24                    : 8;
} ISP_CAM_AWB_HI_THR_T;

typedef union {
    enum { MASK     = 0x00FFFFFF };
    typedef ISP_CAM_AWB_HI_THR_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_HI_THR_T;

// CAM_AWB_PIXEL_CNT0 CAM+05D8H
typedef struct {
        FIELD AWB_PIXEL_CNT0            : 24;
        FIELD rsv_24                    : 8;
} ISP_CAM_AWB_PIXEL_CNT0_T;

typedef union {
    enum { MASK     = 0x00FFFFFF };
    typedef ISP_CAM_AWB_PIXEL_CNT0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_PIXEL_CNT0_T;

// CAM_AWB_PIXEL_CNT1 CAM+05DCH
typedef struct {
        FIELD AWB_PIXEL_CNT1            : 24;
        FIELD rsv_24                    : 8;
} ISP_CAM_AWB_PIXEL_CNT1_T;

typedef union {
    enum { MASK     = 0x00FFFFFF };
    typedef ISP_CAM_AWB_PIXEL_CNT1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_PIXEL_CNT1_T;

// CAM_AWB_PIXEL_CNT2 CAM+05E0H
typedef struct {
        FIELD AWB_PIXEL_CNT2            : 24;
        FIELD rsv_24                    : 8;
} ISP_CAM_AWB_PIXEL_CNT2_T;

typedef union {
    enum { MASK     = 0x00FFFFFF };
    typedef ISP_CAM_AWB_PIXEL_CNT2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_PIXEL_CNT2_T;

// CAM_AWB_ERR_THR CAM+05E4H
typedef struct {
        FIELD AWB_ERR_THR               : 12;
        FIELD rsv_12                    : 4;
        FIELD AWB_ERR_SFT               : 3;
        FIELD rsv_19                    : 13;
} ISP_CAM_AWB_ERR_THR_T;

typedef union {
    enum { MASK     = 0x00070FFF };
    typedef ISP_CAM_AWB_ERR_THR_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_ERR_THR_T;

// CAM_AWB_ROT CAM+05E8H
typedef struct {
        FIELD AWB_C                     : 10;
        FIELD rsv_10                    : 6;
        FIELD AWB_S                     : 10;
        FIELD rsv_26                    : 6;
} ISP_CAM_AWB_ROT_T;

typedef union {
    enum { MASK     = 0x03FF03FF };
    typedef ISP_CAM_AWB_ROT_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_ROT_T;

// CAM_AWB_L0_X CAM+05ECH
typedef struct {
        FIELD AWB_L0_X_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L0_X_UP               : 14;
        FIELD rsv_30                    : 2;
} ISP_CAM_AWB_L0_X_T;

typedef union {
    enum { MASK     = 0x3FFF3FFF };
    typedef ISP_CAM_AWB_L0_X_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_L0_X_T;

// CAM_AWB_L0_Y CAM+05F0H
typedef struct {
        FIELD AWB_L0_Y_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L0_Y_UP               : 14;
        FIELD rsv_30                    : 2;
} ISP_CAM_AWB_L0_Y_T;

typedef union {
    enum { MASK     = 0x3FFF3FFF };
    typedef ISP_CAM_AWB_L0_Y_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_L0_Y_T;

// CAM_AWB_L1_X CAM+05F4H
typedef struct {
        FIELD AWB_L1_X_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L1_X_UP               : 14;
        FIELD rsv_30                    : 2;
} ISP_CAM_AWB_L1_X_T;

typedef union {
    enum { MASK     = 0x3FFF3FFF };
    typedef ISP_CAM_AWB_L1_X_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_L1_X_T;

// CAM_AWB_L1_Y CAM+05F8H
typedef struct {
        FIELD AWB_L1_Y_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L1_Y_UP               : 14;
        FIELD rsv_30                    : 2;
} ISP_CAM_AWB_L1_Y_T;

typedef union {
    enum { MASK     = 0x3FFF3FFF };
    typedef ISP_CAM_AWB_L1_Y_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_L1_Y_T;

// CAM_AWB_L2_X CAM+05FCH
typedef struct {
        FIELD AWB_L2_X_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L2_X_UP               : 14;
        FIELD rsv_30                    : 2;
} ISP_CAM_AWB_L2_X_T;

typedef union {
    enum { MASK     = 0x3FFF3FFF };
    typedef ISP_CAM_AWB_L2_X_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_L2_X_T;

// CAM_AWB_L2_Y CAM+0600H
typedef struct {
        FIELD AWB_L2_Y_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L2_Y_UP               : 14;
        FIELD rsv_30                    : 2;
} ISP_CAM_AWB_L2_Y_T;

typedef union {
    enum { MASK     = 0x3FFF3FFF };
    typedef ISP_CAM_AWB_L2_Y_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_L2_Y_T;

// CAM_AWB_L3_X CAM+0604H
typedef struct {
        FIELD AWB_L3_X_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L3_X_UP               : 14;
        FIELD rsv_30                    : 2;
} ISP_CAM_AWB_L3_X_T;

typedef union {
    enum { MASK     = 0x3FFF3FFF };
    typedef ISP_CAM_AWB_L3_X_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_L3_X_T;

// CAM_AWB_L3_Y CAM+0608H
typedef struct {
        FIELD AWB_L3_Y_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L3_Y_UP               : 14;
        FIELD rsv_30                    : 2;
} ISP_CAM_AWB_L3_Y_T;

typedef union {
    enum { MASK     = 0x3FFF3FFF };
    typedef ISP_CAM_AWB_L3_Y_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_L3_Y_T;

// CAM_AWB_L4_X CAM+060CH
typedef struct {
        FIELD AWB_L4_X_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L4_X_UP               : 14;
        FIELD rsv_30                    : 2;
} ISP_CAM_AWB_L4_X_T;

typedef union {
    enum { MASK     = 0x3FFF3FFF };
    typedef ISP_CAM_AWB_L4_X_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_L4_X_T;

// CAM_AWB_L4_Y CAM+0610H
typedef struct {
        FIELD AWB_L4_Y_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L4_Y_UP               : 14;
        FIELD rsv_30                    : 2;
} ISP_CAM_AWB_L4_Y_T;

typedef union {
    enum { MASK     = 0x3FFF3FFF };
    typedef ISP_CAM_AWB_L4_Y_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_L4_Y_T;

// CAM_AWB_L5_X CAM+0614H
typedef struct {
        FIELD AWB_L5_X_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L5_X_UP               : 14;
        FIELD rsv_30                    : 2;
} ISP_CAM_AWB_L5_X_T;

typedef union {
    enum { MASK     = 0x3FFF3FFF };
    typedef ISP_CAM_AWB_L5_X_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_L5_X_T;

// CAM_AWB_L5_Y CAM+0618H
typedef struct {
        FIELD AWB_L5_Y_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L5_Y_UP               : 14;
        FIELD rsv_30                    : 2;
} ISP_CAM_AWB_L5_Y_T;

typedef union {
    enum { MASK     = 0x3FFF3FFF };
    typedef ISP_CAM_AWB_L5_Y_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_L5_Y_T;

// CAM_AWB_L6_X CAM+061CH
typedef struct {
        FIELD AWB_L6_X_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L6_X_UP               : 14;
        FIELD rsv_30                    : 2;
} ISP_CAM_AWB_L6_X_T;

typedef union {
    enum { MASK     = 0x3FFF3FFF };
    typedef ISP_CAM_AWB_L6_X_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_L6_X_T;

// CAM_AWB_L6_Y CAM+0620H
typedef struct {
        FIELD AWB_L6_Y_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L6_Y_UP               : 14;
        FIELD rsv_30                    : 2;
} ISP_CAM_AWB_L6_Y_T;

typedef union {
    enum { MASK     = 0x3FFF3FFF };
    typedef ISP_CAM_AWB_L6_Y_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_L6_Y_T;

// CAM_AWB_L7_X CAM+0624H
typedef struct {
        FIELD AWB_L7_X_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L7_X_UP               : 14;
        FIELD rsv_30                    : 2;
} ISP_CAM_AWB_L7_X_T;

typedef union {
    enum { MASK     = 0x3FFF3FFF };
    typedef ISP_CAM_AWB_L7_X_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_L7_X_T;

// CAM_AWB_L7_Y CAM+0628H
typedef struct {
        FIELD AWB_L7_Y_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L7_Y_UP               : 14;
        FIELD rsv_30                    : 2;
} ISP_CAM_AWB_L7_Y_T;

typedef union {
    enum { MASK     = 0x3FFF3FFF };
    typedef ISP_CAM_AWB_L7_Y_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_L7_Y_T;

// CAM_AWB_L8_X CAM+062CH
typedef struct {
        FIELD AWB_L8_X_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L8_X_UP               : 14;
        FIELD rsv_30                    : 2;
} ISP_CAM_AWB_L8_X_T;

typedef union {
    enum { MASK     = 0x3FFF3FFF };
    typedef ISP_CAM_AWB_L8_X_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_L8_X_T;

// CAM_AWB_L8_Y CAM+0630H
typedef struct {
        FIELD AWB_L8_Y_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L8_Y_UP               : 14;
        FIELD rsv_30                    : 2;
} ISP_CAM_AWB_L8_Y_T;

typedef union {
    enum { MASK     = 0x3FFF3FFF };
    typedef ISP_CAM_AWB_L8_Y_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_L8_Y_T;

// CAM_AWB_L9_X CAM+0634H
typedef struct {
        FIELD AWB_L9_X_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L9_X_UP               : 14;
        FIELD rsv_30                    : 2;
} ISP_CAM_AWB_L9_X_T;

typedef union {
    enum { MASK     = 0x3FFF3FFF };
    typedef ISP_CAM_AWB_L9_X_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_L9_X_T;

// CAM_AWB_L9_Y CAM+0638H
typedef struct {
        FIELD AWB_L9_Y_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L9_Y_UP               : 14;
        FIELD rsv_30                    : 2;
} ISP_CAM_AWB_L9_Y_T;

typedef union {
    enum { MASK     = 0x3FFF3FFF };
    typedef ISP_CAM_AWB_L9_Y_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_L9_Y_T;

//
typedef union {
    enum { COUNT = 35 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_AWB_WIN_ORG_T    win_org;
        ISP_NVRAM_AWB_WIN_SIZE_T   win_size;
        ISP_NVRAM_AWB_WIN_PIT_T    win_pit;
        ISP_NVRAM_AWB_WIN_NUM_T    win_num;
        ISP_NVRAM_AWB_GAIN1_0_T    gain1_0;
        ISP_NVRAM_AWB_GAIN1_1_T    gain1_1;
        ISP_NVRAM_AWB_LMT1_0_T     lmt1_0;
        ISP_NVRAM_AWB_LMT1_1_T     lmt1_1;
        ISP_NVRAM_AWB_LOW_THR_T    low_thr;
        ISP_NVRAM_AWB_HI_THR_T     hi_thr;
        ISP_NVRAM_AWB_PIXEL_CNT0_T pixel_cnt0;
        ISP_NVRAM_AWB_PIXEL_CNT1_T pixel_cnt1;
        ISP_NVRAM_AWB_PIXEL_CNT2_T pixel_cnt2;
        ISP_NVRAM_AWB_ERR_THR_T    err_thr;
        ISP_NVRAM_AWB_ROT_T        rot;
        ISP_NVRAM_AWB_L0_X_T       l0_x;
        ISP_NVRAM_AWB_L0_Y_T       l0_y;
        ISP_NVRAM_AWB_L1_X_T       l1_x;
        ISP_NVRAM_AWB_L1_Y_T       l1_y;
        ISP_NVRAM_AWB_L2_X_T       l2_x;
        ISP_NVRAM_AWB_L2_Y_T       l2_y;
        ISP_NVRAM_AWB_L3_X_T       l3_x;
        ISP_NVRAM_AWB_L3_Y_T       l3_y;
        ISP_NVRAM_AWB_L4_X_T       l4_x;
        ISP_NVRAM_AWB_L4_Y_T       l4_y;
        ISP_NVRAM_AWB_L5_X_T       l5_x;
        ISP_NVRAM_AWB_L5_Y_T       l5_y;
        ISP_NVRAM_AWB_L6_X_T       l6_x;
        ISP_NVRAM_AWB_L6_Y_T       l6_y;
        ISP_NVRAM_AWB_L7_X_T       l7_x;
        ISP_NVRAM_AWB_L7_Y_T       l7_y;
        ISP_NVRAM_AWB_L8_X_T       l8_x;
        ISP_NVRAM_AWB_L8_Y_T       l8_y;
        ISP_NVRAM_AWB_L9_X_T       l9_x;
        ISP_NVRAM_AWB_L9_Y_T       l9_y;
    };
} ISP_NVRAM_AWB_STAT_CONFIG_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// AWB Statistics @ TG2
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CAM_AWB_D_WIN_ORG CAM+25B0H
typedef struct {
        FIELD AWB_W_HORG                : 13;
        FIELD rsv_13                    : 3;
        FIELD AWB_W_VORG                : 13;
        FIELD rsv_29                    : 3;
} ISP_CAM_AWB_D_WIN_ORG_T;

typedef union {
    enum { MASK     = 0x1FFF1FFF };
    typedef ISP_CAM_AWB_D_WIN_ORG_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_D_WIN_ORG_T;

// CAM_AWB_D_WIN_SIZE CAM+25B4H
typedef struct {
        FIELD AWB_W_HSIZE               : 13;
        FIELD rsv_13                    : 3;
        FIELD AWB_W_VSIZE               : 13;
        FIELD rsv_29                    : 3;
} ISP_CAM_AWB_D_WIN_SIZE_T;

typedef union {
    enum { MASK     = 0x1FFF1FFF };
    typedef ISP_CAM_AWB_D_WIN_SIZE_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_D_WIN_SIZE_T;

// CAM_AWB_D_WIN_PIT CAM+25B8H
typedef struct {
        FIELD AWB_W_HPIT                : 13;
        FIELD rsv_13                    : 3;
        FIELD AWB_W_VPIT                : 13;
        FIELD rsv_29                    : 3;
} ISP_CAM_AWB_D_WIN_PIT_T;

typedef union {
    enum { MASK     = 0x1FFF1FFF };
    typedef ISP_CAM_AWB_D_WIN_PIT_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_D_WIN_PIT_T;

// CAM_AWB_D_WIN_NUM CAM+25BCH
typedef struct {
        FIELD AWB_W_HNUM                : 8;
        FIELD rsv_8                     : 8;
        FIELD AWB_W_VNUM                : 8;
        FIELD rsv_24                    : 8;
} ISP_CAM_AWB_D_WIN_NUM_T;

typedef union {
    enum { MASK     = 0x00FF00FF };
    typedef ISP_CAM_AWB_D_WIN_NUM_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_D_WIN_NUM_T;

// CAM_AWB_D_GAIN1_0 CAM+25COH
typedef struct {
        FIELD AWB_GAIN1_R               : 13;
        FIELD rsv_13                    : 3;
        FIELD AWB_GAIN1_G               : 13;
        FIELD rsv_29                    : 3;
} ISP_CAM_AWB_D_GAIN1_0_T;

typedef union {
    enum { MASK     = 0x1FFF1FFF };
    typedef ISP_CAM_AWB_D_GAIN1_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_D_GAIN1_0_T;

// CAM_AWB_D_GAIN1_1 CAM+25C4H
typedef struct {
        FIELD AWB_GAIN1_B               : 13;
        FIELD rsv_13                    : 19;
} ISP_CAM_AWB_D_GAIN1_1_T;

typedef union {
    enum { MASK     = 0x00001FFF };
    typedef ISP_CAM_AWB_D_GAIN1_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_D_GAIN1_1_T;

// CAM_AWB_D_LMT1_0 CAM+25C8H
typedef struct {
        FIELD AWB_LMT1_R                : 12;
        FIELD rsv_12                    : 4;
        FIELD AWB_LMT1_G                : 12;
        FIELD rsv_28                    : 4;
} ISP_CAM_AWB_D_LMT1_0_T;

typedef union {
    enum { MASK     = 0x0FFF0FFF };
    typedef ISP_CAM_AWB_D_LMT1_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_D_LMT1_0_T;

// CAM_AWB_D_LMT1_1 CAM+25CCH
typedef struct {
        FIELD AWB_LMT1_B                : 12;
        FIELD rsv_12                    : 20;
} ISP_CAM_AWB_D_LMT1_1_T;

typedef union {
    enum { MASK     = 0x00000FFF };
    typedef ISP_CAM_AWB_D_LMT1_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_D_LMT1_1_T;

// CAM_AWB_D_LOW_THR CAM+25D0H
typedef struct {
        FIELD AWB_LOW_THR0              : 8;
        FIELD AWB_LOW_THR1              : 8;
        FIELD AWB_LOW_THR2              : 8;
        FIELD rsv_24                    : 8;
} ISP_CAM_AWB_D_LOW_THR_T;

typedef union {
    enum { MASK     = 0x00FFFFFF };
    typedef ISP_CAM_AWB_D_LOW_THR_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_D_LOW_THR_T;

// CAM_AWB_D_HI_THR CAM+25D4H
typedef struct {
        FIELD AWB_HI_THR0               : 8;
        FIELD AWB_HI_THR1               : 8;
        FIELD AWB_HI_THR2               : 8;
        FIELD rsv_24                    : 8;
} ISP_CAM_AWB_D_HI_THR_T;

typedef union {
    enum { MASK     = 0x00FFFFFF };
    typedef ISP_CAM_AWB_D_HI_THR_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_D_HI_THR_T;

// CAM_AWB_D_PIXEL_CNT0 CAM+25D8H
typedef struct {
        FIELD AWB_PIXEL_CNT0            : 24;
        FIELD rsv_24                    : 8;
} ISP_CAM_AWB_D_PIXEL_CNT0_T;

typedef union {
    enum { MASK     = 0x00FFFFFF };
    typedef ISP_CAM_AWB_D_PIXEL_CNT0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_D_PIXEL_CNT0_T;

// CAM_AWB_D_PIXEL_CNT1 CAM+25DCH
typedef struct {
        FIELD AWB_PIXEL_CNT1            : 24;
        FIELD rsv_24                    : 8;
} ISP_CAM_AWB_D_PIXEL_CNT1_T;

typedef union {
    enum { MASK     = 0x00FFFFFF };
    typedef ISP_CAM_AWB_D_PIXEL_CNT1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_D_PIXEL_CNT1_T;

// CAM_AWB_D_PIXEL_CNT2 CAM+25E0H
typedef struct {
        FIELD AWB_PIXEL_CNT2            : 24;
        FIELD rsv_24                    : 8;
} ISP_CAM_AWB_D_PIXEL_CNT2_T;

typedef union {
    enum { MASK     = 0x00FFFFFF };
    typedef ISP_CAM_AWB_D_PIXEL_CNT2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_D_PIXEL_CNT2_T;

// CAM_AWB_D_ERR_THR CAM+25E4H
typedef struct {
        FIELD AWB_ERR_THR               : 12;
        FIELD rsv_12                    : 4;
        FIELD AWB_ERR_SFT               : 3;
        FIELD rsv_19                    : 13;
} ISP_CAM_AWB_D_ERR_THR_T;

typedef union {
    enum { MASK     = 0x00070FFF };
    typedef ISP_CAM_AWB_D_ERR_THR_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_D_ERR_THR_T;

// CAM_AWB_D_ROT CAM+25E8H
typedef struct {
        FIELD AWB_C                     : 10;
        FIELD rsv_10                    : 6;
        FIELD AWB_S                     : 10;
        FIELD rsv_26                    : 6;
} ISP_CAM_AWB_D_ROT_T;

typedef union {
    enum { MASK     = 0x03FF03FF };
    typedef ISP_CAM_AWB_D_ROT_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_D_ROT_T;

// CAM_AWB_D_L0_X CAM+25ECH
typedef struct {
        FIELD AWB_L0_X_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L0_X_UP               : 14;
        FIELD rsv_30                    : 2;
} ISP_CAM_AWB_D_L0_X_T;

typedef union {
    enum { MASK     = 0x3FFF3FFF };
    typedef ISP_CAM_AWB_D_L0_X_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_D_L0_X_T;

// CAM_AWB_D_L0_Y CAM+25F0H
typedef struct {
        FIELD AWB_L0_Y_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L0_Y_UP               : 14;
        FIELD rsv_30                    : 2;
} ISP_CAM_AWB_D_L0_Y_T;

typedef union {
    enum { MASK     = 0x3FFF3FFF };
    typedef ISP_CAM_AWB_D_L0_Y_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_D_L0_Y_T;

// CAM_AWB_D_L1_X CAM+25F4H
typedef struct {
        FIELD AWB_L1_X_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L1_X_UP               : 14;
        FIELD rsv_30                    : 2;
} ISP_CAM_AWB_D_L1_X_T;

typedef union {
    enum { MASK     = 0x3FFF3FFF };
    typedef ISP_CAM_AWB_D_L1_X_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_D_L1_X_T;

// CAM_AWB_D_L1_Y CAM+25F8H
typedef struct {
        FIELD AWB_L1_Y_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L1_Y_UP               : 14;
        FIELD rsv_30                    : 2;
} ISP_CAM_AWB_D_L1_Y_T;

typedef union {
    enum { MASK     = 0x3FFF3FFF };
    typedef ISP_CAM_AWB_D_L1_Y_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_D_L1_Y_T;

// CAM_AWB_D_L2_X CAM+25FCH
typedef struct {
        FIELD AWB_L2_X_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L2_X_UP               : 14;
        FIELD rsv_30                    : 2;
} ISP_CAM_AWB_D_L2_X_T;

typedef union {
    enum { MASK     = 0x3FFF3FFF };
    typedef ISP_CAM_AWB_D_L2_X_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_D_L2_X_T;

// CAM_AWB_D_L2_Y CAM+2600H
typedef struct {
        FIELD AWB_L2_Y_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L2_Y_UP               : 14;
        FIELD rsv_30                    : 2;
} ISP_CAM_AWB_D_L2_Y_T;

typedef union {
    enum { MASK     = 0x3FFF3FFF };
    typedef ISP_CAM_AWB_D_L2_Y_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_D_L2_Y_T;

// CAM_AWB_D_L3_X CAM+2604H
typedef struct {
        FIELD AWB_L3_X_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L3_X_UP               : 14;
        FIELD rsv_30                    : 2;
} ISP_CAM_AWB_D_L3_X_T;

typedef union {
    enum { MASK     = 0x3FFF3FFF };
    typedef ISP_CAM_AWB_D_L3_X_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_D_L3_X_T;

// CAM_AWB_D_L3_Y CAM+2608H
typedef struct {
        FIELD AWB_L3_Y_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L3_Y_UP               : 14;
        FIELD rsv_30                    : 2;
} ISP_CAM_AWB_D_L3_Y_T;

typedef union {
    enum { MASK     = 0x3FFF3FFF };
    typedef ISP_CAM_AWB_D_L3_Y_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_D_L3_Y_T;

// CAM_AWB_D_L4_X CAM+260CH
typedef struct {
        FIELD AWB_L4_X_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L4_X_UP               : 14;
        FIELD rsv_30                    : 2;
} ISP_CAM_AWB_D_L4_X_T;

typedef union {
    enum { MASK     = 0x3FFF3FFF };
    typedef ISP_CAM_AWB_D_L4_X_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_D_L4_X_T;

// CAM_AWB_D_L4_Y CAM+2610H
typedef struct {
        FIELD AWB_L4_Y_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L4_Y_UP               : 14;
        FIELD rsv_30                    : 2;
} ISP_CAM_AWB_D_L4_Y_T;

typedef union {
    enum { MASK     = 0x3FFF3FFF };
    typedef ISP_CAM_AWB_D_L4_Y_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_D_L4_Y_T;

// CAM_AWB_D_L5_X CAM+2614H
typedef struct {
        FIELD AWB_L5_X_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L5_X_UP               : 14;
        FIELD rsv_30                    : 2;
} ISP_CAM_AWB_D_L5_X_T;

typedef union {
    enum { MASK     = 0x3FFF3FFF };
    typedef ISP_CAM_AWB_D_L5_X_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_D_L5_X_T;

// CAM_AWB_D_L5_Y CAM+2618H
typedef struct {
        FIELD AWB_L5_Y_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L5_Y_UP               : 14;
        FIELD rsv_30                    : 2;
} ISP_CAM_AWB_D_L5_Y_T;

typedef union {
    enum { MASK     = 0x3FFF3FFF };
    typedef ISP_CAM_AWB_D_L5_Y_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_D_L5_Y_T;

// CAM_AWB_D_L6_X CAM+261CH
typedef struct {
        FIELD AWB_L6_X_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L6_X_UP               : 14;
        FIELD rsv_30                    : 2;
} ISP_CAM_AWB_D_L6_X_T;

typedef union {
    enum { MASK     = 0x3FFF3FFF };
    typedef ISP_CAM_AWB_D_L6_X_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_D_L6_X_T;

// CAM_AWB_D_L6_Y CAM+2620H
typedef struct {
        FIELD AWB_L6_Y_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L6_Y_UP               : 14;
        FIELD rsv_30                    : 2;
} ISP_CAM_AWB_D_L6_Y_T;

typedef union {
    enum { MASK     = 0x3FFF3FFF };
    typedef ISP_CAM_AWB_D_L6_Y_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_D_L6_Y_T;

// CAM_AWB_D_L7_X CAM+2624H
typedef struct {
        FIELD AWB_L7_X_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L7_X_UP               : 14;
        FIELD rsv_30                    : 2;
} ISP_CAM_AWB_D_L7_X_T;

typedef union {
    enum { MASK     = 0x3FFF3FFF };
    typedef ISP_CAM_AWB_D_L7_X_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_D_L7_X_T;

// CAM_AWB_D_L7_Y CAM+2628H
typedef struct {
        FIELD AWB_L7_Y_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L7_Y_UP               : 14;
        FIELD rsv_30                    : 2;
} ISP_CAM_AWB_D_L7_Y_T;

typedef union {
    enum { MASK     = 0x3FFF3FFF };
    typedef ISP_CAM_AWB_D_L7_Y_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_D_L7_Y_T;

// CAM_AWB_D_L8_X CAM+262CH
typedef struct {
        FIELD AWB_L8_X_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L8_X_UP               : 14;
        FIELD rsv_30                    : 2;
} ISP_CAM_AWB_D_L8_X_T;

typedef union {
    enum { MASK     = 0x3FFF3FFF };
    typedef ISP_CAM_AWB_D_L8_X_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_D_L8_X_T;

// CAM_AWB_D_L8_Y CAM+2630H
typedef struct {
        FIELD AWB_L8_Y_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L8_Y_UP               : 14;
        FIELD rsv_30                    : 2;
} ISP_CAM_AWB_D_L8_Y_T;

typedef union {
    enum { MASK     = 0x3FFF3FFF };
    typedef ISP_CAM_AWB_D_L8_Y_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_D_L8_Y_T;

// CAM_AWB_D_L9_X CAM+2634H
typedef struct {
        FIELD AWB_L9_X_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L9_X_UP               : 14;
        FIELD rsv_30                    : 2;
} ISP_CAM_AWB_D_L9_X_T;

typedef union {
    enum { MASK     = 0x3FFF3FFF };
    typedef ISP_CAM_AWB_D_L9_X_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_D_L9_X_T;

// CAM_AWB_D_L9_Y CAM+2638H
typedef struct {
        FIELD AWB_L9_Y_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L9_Y_UP               : 14;
        FIELD rsv_30                    : 2;
} ISP_CAM_AWB_D_L9_Y_T;

typedef union {
    enum { MASK     = 0x3FFF3FFF };
    typedef ISP_CAM_AWB_D_L9_Y_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AWB_D_L9_Y_T;

//
typedef union {
    enum { COUNT = 35 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_AWB_D_WIN_ORG_T    win_org;
        ISP_NVRAM_AWB_D_WIN_SIZE_T   win_size;
        ISP_NVRAM_AWB_D_WIN_PIT_T    win_pit;
        ISP_NVRAM_AWB_D_WIN_NUM_T    win_num;
        ISP_NVRAM_AWB_D_GAIN1_0_T    gain1_0;
        ISP_NVRAM_AWB_D_GAIN1_1_T    gain1_1;
        ISP_NVRAM_AWB_D_LMT1_0_T     lmt1_0;
        ISP_NVRAM_AWB_D_LMT1_1_T     lmt1_1;
        ISP_NVRAM_AWB_D_LOW_THR_T    low_thr;
        ISP_NVRAM_AWB_D_HI_THR_T     hi_thr;
        ISP_NVRAM_AWB_D_PIXEL_CNT0_T pixel_cnt0;
        ISP_NVRAM_AWB_D_PIXEL_CNT1_T pixel_cnt1;
        ISP_NVRAM_AWB_D_PIXEL_CNT2_T pixel_cnt2;
        ISP_NVRAM_AWB_D_ERR_THR_T    err_thr;
        ISP_NVRAM_AWB_D_ROT_T        rot;
        ISP_NVRAM_AWB_D_L0_X_T       l0_x;
        ISP_NVRAM_AWB_D_L0_Y_T       l0_y;
        ISP_NVRAM_AWB_D_L1_X_T       l1_x;
        ISP_NVRAM_AWB_D_L1_Y_T       l1_y;
        ISP_NVRAM_AWB_D_L2_X_T       l2_x;
        ISP_NVRAM_AWB_D_L2_Y_T       l2_y;
        ISP_NVRAM_AWB_D_L3_X_T       l3_x;
        ISP_NVRAM_AWB_D_L3_Y_T       l3_y;
        ISP_NVRAM_AWB_D_L4_X_T       l4_x;
        ISP_NVRAM_AWB_D_L4_Y_T       l4_y;
        ISP_NVRAM_AWB_D_L5_X_T       l5_x;
        ISP_NVRAM_AWB_D_L5_Y_T       l5_y;
        ISP_NVRAM_AWB_D_L6_X_T       l6_x;
        ISP_NVRAM_AWB_D_L6_Y_T       l6_y;
        ISP_NVRAM_AWB_D_L7_X_T       l7_x;
        ISP_NVRAM_AWB_D_L7_Y_T       l7_y;
        ISP_NVRAM_AWB_D_L8_X_T       l8_x;
        ISP_NVRAM_AWB_D_L8_Y_T       l8_y;
        ISP_NVRAM_AWB_D_L9_X_T       l9_x;
        ISP_NVRAM_AWB_D_L9_Y_T       l9_y;
    };
} ISP_NVRAM_AWB_STAT_CONFIG_D_T;



//----------------------------------------------------------------------------------------------------------------


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// AE Statistics
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// CAM_AE_RAWPREGAIN2_0 CAM+0654H
typedef struct {
        MUINT32 RAWPREGAIN2_R             : 12;
        MUINT32 rsv_12                    : 4;
        MUINT32 RAWPREGAIN2_G             : 12;
        MUINT32 rsv_28                    : 4;
} ISP_CAM_AE_RAWPREGAIN2_0_T;

typedef union {
    enum { MASK     = 0x0FFF0FFF };
    enum { DEFAULT  = 0x02000200 };
    typedef ISP_CAM_AE_RAWPREGAIN2_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AE_RAWPREGAIN2_0_T;

// CAM_AE_RAWPREGAIN2_1 CAM+0658H
typedef struct {
        MUINT32 RAWPREGAIN2_B             : 12;
        MUINT32 rsv_12                    : 20;
} ISP_CAM_AE_RAWPREGAIN2_1_T;

typedef union {
    enum { MASK     = 0x00000FFF };
    enum { DEFAULT  = 0x00000200 };
    typedef ISP_CAM_AE_RAWPREGAIN2_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AE_RAWPREGAIN2_1_T;

// CAM_AE_RAWLIMIT2_0 CAM+065CH
typedef struct {
        MUINT32 AE_LIMIT2_R               : 12;
        MUINT32 rsv_12                    : 4;
        MUINT32 AE_LIMIT2_G               : 12;
        MUINT32 rsv_28                    : 4;
} ISP_CAM_AE_RAWLIMIT2_0_T;

typedef union {
    enum { MASK     = 0x0FFF0FFF };
    enum { DEFAULT  = 0x0FFF0FFF };
    typedef ISP_CAM_AE_RAWLIMIT2_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AE_RAWLIMIT2_0_T;

// CAM_AE_RAWLIMIT2_1 CAM+0660H
typedef struct {
        MUINT32 AE_LIMIT2_B               : 12;
        MUINT32 rsv_12                    : 20;
} ISP_CAM_AE_RAWLIMIT2_1_T;

typedef union {
    enum { MASK     = 0x00000FFF };
    enum { DEFAULT  = 0x00000FFF };
    typedef ISP_CAM_AE_RAWLIMIT2_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AE_RAWLIMIT2_1_T;

// CAM_AE_MATRIX_COEF0 CAM+0664H
typedef struct {
        MUINT32 RC_CNV00                  : 11;
        MUINT32 rsv_11                    : 5;
        MUINT32 RC_CNV01                  : 11;
        MUINT32 rsv_27                    : 5;
} ISP_CAM_AE_MATRIX_COEF0_T;

typedef union {
    enum { MASK     = 0x07FF07FF };
    enum { DEFAULT  = 0x00000200 };
    typedef ISP_CAM_AE_MATRIX_COEF0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AE_MATRIX_COEF0_T;

// CAM_AE_MATRIX_COEF1 CAM+0668H
typedef struct {
        MUINT32 RC_CNV02                  : 11;
        MUINT32 rsv_11                    : 5;
        MUINT32 RC_CNV10                  : 11;
        MUINT32 rsv_27                    : 5;
} ISP_CAM_AE_MATRIX_COEF1_T;

typedef union {
    enum { MASK     = 0x07FF07FF };
    enum { DEFAULT  = 0x00000000 };
    typedef ISP_CAM_AE_MATRIX_COEF1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AE_MATRIX_COEF1_T;

// CAM_AE_MATRIX_COEF2 CAM+066CH
typedef struct {
        MUINT32 RC_CNV11                  : 11;
        MUINT32 rsv_11                    : 5;
        MUINT32 RC_CNV12                  : 11;
        MUINT32 rsv_27                    : 5;
} ISP_CAM_AE_MATRIX_COEF2_T;

typedef union {
    enum { MASK     = 0x07FF07FF };
    enum { DEFAULT  = 0x00000200 };
    typedef ISP_CAM_AE_MATRIX_COEF2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AE_MATRIX_COEF2_T;

// CAM_AE_MATRIX_COEF3 CAM+0670H
typedef struct {
        MUINT32 RC_CNV20                  : 11;
        MUINT32 rsv_11                    : 5;
        MUINT32 RC_CNV21                  : 11;
        MUINT32 rsv_27                    : 5;
} ISP_CAM_AE_MATRIX_COEF3_T;

typedef union {
    enum { MASK     = 0x07FF07FF };
    enum { DEFAULT  = 0x00000000 };
    typedef ISP_CAM_AE_MATRIX_COEF3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AE_MATRIX_COEF3_T;

// CAM_AE_MATRIX_COEF4 CAM+0674H
typedef struct {
        MUINT32 RC_CNV22                  : 11;
        MUINT32 rsv_11                    : 5;
        MUINT32 AE_RC_ACC                 : 4;
        MUINT32 rsv_20                    : 12;
} ISP_CAM_AE_MATRIX_COEF4_T;

typedef union {
    enum { MASK     = 0x000F07FF };
    enum { DEFAULT  = 0x00090200 };
    typedef ISP_CAM_AE_MATRIX_COEF4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AE_MATRIX_COEF4_T;

// CAM_AE_YGAMMA_0 CAM+0678H
typedef struct {
        MUINT32 Y_GMR1                    : 8;
        MUINT32 Y_GMR2                    : 8;
        MUINT32 Y_GMR3                    : 8;
        MUINT32 Y_GMR4                    : 8;
} ISP_CAM_AE_YGAMMA_0_T;

typedef union {
    enum { MASK     = 0xFFFFFFFF };
    enum { DEFAULT  = 0x00000000 };
    typedef ISP_CAM_AE_YGAMMA_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AE_YGAMMA_0_T;

// CAM_AE_YGAMMA_1 CAM+067CH
typedef struct {
        MUINT32 Y_GMR5                    : 8;
        MUINT32 rsv_8                     : 24;
} ISP_CAM_AE_YGAMMA_1_T;

typedef union {
    enum { MASK     = 0x000000FF };
    enum { DEFAULT  = 0x00000000 };
    typedef ISP_CAM_AE_YGAMMA_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AE_YGAMMA_1_T;

//
typedef union {
    enum { COUNT = 2 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_AE_RAWPREGAIN2_0_T    rawpregain2_0;
        ISP_NVRAM_AE_RAWPREGAIN2_1_T    rawpregain2_1;
    };
} ISP_NVRAM_AE_RAWPREGAIN2_T;

//
typedef union {
    enum { COUNT = 5 };
    MUINT32 set[COUNT];
    struct {
        ISP_CAM_AE_MATRIX_COEF0_T       matrix_coef0;
        ISP_CAM_AE_MATRIX_COEF1_T       matrix_coef1;
        ISP_CAM_AE_MATRIX_COEF2_T       matrix_coef2;
        ISP_CAM_AE_MATRIX_COEF3_T       matrix_coef3;
        ISP_CAM_AE_MATRIX_COEF4_T       matrix_coef4;
    };
} ISP_NVRAM_AE_MATRIX_COEF_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// AE Statistics @TG2
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CAM_AE_D_HST_CTL CAM+0650H
typedef struct {
        MUINT32 AE_HST0_EN                : 1;
        MUINT32 AE_HST1_EN                : 1;
        MUINT32 AE_HST2_EN                : 1;
        MUINT32 AE_HST3_EN                : 1;
        MUINT32 rsv_4                     : 28;
} ISP_CAM_AE_D_HST_CTL_T;

typedef union {
    enum { MASK     = 0x0000000F };
    enum { DEFAULT  = 0x00000000 };
    typedef ISP_CAM_AE_D_HST_CTL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AE_D_HST_CTL_T;

// CAM_AE_D_RAWPREGAIN2_0 CAM+0654H
typedef struct {
        MUINT32 RAWPREGAIN2_R             : 12;
        MUINT32 rsv_12                    : 4;
        MUINT32 RAWPREGAIN2_G             : 12;
        MUINT32 rsv_28                    : 4;
} ISP_CAM_AE_D_RAWPREGAIN2_0_T;

typedef union {
    enum { MASK     = 0x0FFF0FFF };
    enum { DEFAULT  = 0x02000200 };
    typedef ISP_CAM_AE_D_RAWPREGAIN2_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AE_D_RAWPREGAIN2_0_T;

// CAM_AE_D_RAWPREGAIN2_1 CAM+0658H
typedef struct {
        MUINT32 RAWPREGAIN2_B             : 12;
        MUINT32 rsv_12                    : 20;
} ISP_CAM_AE_D_RAWPREGAIN2_1_T;

typedef union {
    enum { MASK     = 0x00000FFF };
    enum { DEFAULT  = 0x00000200 };
    typedef ISP_CAM_AE_D_RAWPREGAIN2_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AE_D_RAWPREGAIN2_1_T;

// CAM_AE_D_RAWLIMIT2_0 CAM+065CH
typedef struct {
        MUINT32 AE_LIMIT2_R               : 12;
        MUINT32 rsv_12                    : 4;
        MUINT32 AE_LIMIT2_G               : 12;
        MUINT32 rsv_28                    : 4;
} ISP_CAM_AE_D_RAWLIMIT2_0_T;

typedef union {
    enum { MASK     = 0x0FFF0FFF };
    enum { DEFAULT  = 0x0FFF0FFF };
    typedef ISP_CAM_AE_D_RAWLIMIT2_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AE_D_RAWLIMIT2_0_T;

// CAM_AE_D_RAWLIMIT2_1 CAM+0660H
typedef struct {
        MUINT32 AE_LIMIT2_B               : 12;
        MUINT32 rsv_12                    : 20;
} ISP_CAM_AE_D_RAWLIMIT2_1_T;

typedef union {
    enum { MASK     = 0x00000FFF };
    enum { DEFAULT  = 0x00000FFF };
    typedef ISP_CAM_AE_D_RAWLIMIT2_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AE_D_RAWLIMIT2_1_T;

// CAM_AE_D_MATRIX_COEF0 CAM+0664H
typedef struct {
        MUINT32 RC_CNV00                  : 11;
        MUINT32 rsv_11                    : 5;
        MUINT32 RC_CNV01                  : 11;
        MUINT32 rsv_27                    : 5;
} ISP_CAM_AE_D_MATRIX_COEF0_T;

typedef union {
    enum { MASK     = 0x07FF07FF };
    enum { DEFAULT  = 0x00000200 };
    typedef ISP_CAM_AE_D_MATRIX_COEF0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AE_D_MATRIX_COEF0_T;

// CAM_AE_D_MATRIX_COEF1 CAM+0668H
typedef struct {
        MUINT32 RC_CNV02                  : 11;
        MUINT32 rsv_11                    : 5;
        MUINT32 RC_CNV10                  : 11;
        MUINT32 rsv_27                    : 5;
} ISP_CAM_AE_D_MATRIX_COEF1_T;

typedef union {
    enum { MASK     = 0x07FF07FF };
    enum { DEFAULT  = 0x00000000 };
    typedef ISP_CAM_AE_D_MATRIX_COEF1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AE_D_MATRIX_COEF1_T;

// CAM_AE_D_MATRIX_COEF2 CAM+066CH
typedef struct {
        MUINT32 RC_CNV11                  : 11;
        MUINT32 rsv_11                    : 5;
        MUINT32 RC_CNV12                  : 11;
        MUINT32 rsv_27                    : 5;
} ISP_CAM_AE_D_MATRIX_COEF2_T;

typedef union {
    enum { MASK     = 0x07FF07FF };
    enum { DEFAULT  = 0x00000200 };
    typedef ISP_CAM_AE_D_MATRIX_COEF2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AE_D_MATRIX_COEF2_T;

// CAM_AE_D_MATRIX_COEF3 CAM+0670H
typedef struct {
        MUINT32 RC_CNV20                  : 11;
        MUINT32 rsv_11                    : 5;
        MUINT32 RC_CNV21                  : 11;
        MUINT32 rsv_27                    : 5;
} ISP_CAM_AE_D_MATRIX_COEF3_T;

typedef union {
    enum { MASK     = 0x07FF07FF };
    enum { DEFAULT  = 0x00000000 };
    typedef ISP_CAM_AE_D_MATRIX_COEF3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AE_D_MATRIX_COEF3_T;

// CAM_AE_D_MATRIX_COEF4 CAM+0674H
typedef struct {
        MUINT32 RC_CNV22                  : 11;
        MUINT32 rsv_11                    : 5;
        MUINT32 AE_RC_ACC                 : 4;
        MUINT32 rsv_20                    : 12;
} ISP_CAM_AE_D_MATRIX_COEF4_T;

typedef union {
    enum { MASK     = 0x000F07FF };
    enum { DEFAULT  = 0x00090200 };
    typedef ISP_CAM_AE_D_MATRIX_COEF4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AE_D_MATRIX_COEF4_T;

// CAM_AE_D_YGAMMA_0 CAM+0678H
typedef struct {
        MUINT32 Y_GMR1                    : 8;
        MUINT32 Y_GMR2                    : 8;
        MUINT32 Y_GMR3                    : 8;
        MUINT32 Y_GMR4                    : 8;
} ISP_CAM_AE_D_YGAMMA_0_T;

typedef union {
    enum { MASK     = 0xFFFFFFFF };
    enum { DEFAULT  = 0x00000000 };
    typedef ISP_CAM_AE_D_YGAMMA_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AE_D_YGAMMA_0_T;

// CAM_AE_D_YGAMMA_1 CAM+067CH
typedef struct {
        MUINT32 Y_GMR5                    : 8;
        MUINT32 rsv_8                     : 24;
} ISP_CAM_AE_D_YGAMMA_1_T;

typedef union {
    enum { MASK     = 0x000000FF };
    enum { DEFAULT  = 0x00000000 };
    typedef ISP_CAM_AE_D_YGAMMA_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AE_D_YGAMMA_1_T;

// CAM_AE_D_HST_SET CAM+0680H
typedef struct {
        MUINT32 AE_HST0_BIN               : 2;
        MUINT32 rsv_2                     : 2;
        MUINT32 AE_HST1_BIN               : 2;
        MUINT32 rsv_6                     : 2;
        MUINT32 AE_HST2_BIN               : 2;
        MUINT32 rsv_10                    : 2;
        MUINT32 AE_HST3_BIN               : 2;
        MUINT32 rsv_14                    : 2;
        MUINT32 AE_HST0_COLOR             : 3;
        MUINT32 rsv_19                    : 1;
        MUINT32 AE_HST1_COLOR             : 3;
        MUINT32 rsv_23                    : 1;
        MUINT32 AE_HST2_COLOR             : 3;
        MUINT32 rsv_27                    : 1;
        MUINT32 AE_HST3_COLOR             : 3;
        MUINT32 rsv_31                    : 1;
} ISP_CAM_AE_D_HST_SET_T;

typedef union {
    enum { MASK     = 0x77773333 };
    enum { DEFAULT  = 0x00000000 };
    typedef ISP_CAM_AE_D_HST_SET_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AE_D_HST_SET_T;

// CAM_AE_D_HST0_RNG CAM+0684H
typedef struct {
        MUINT32 AE_HST0_X_LOW             : 7;
        MUINT32 rsv_7                     : 1;
        MUINT32 AE_HST0_X_HI              : 7;
        MUINT32 rsv_15                    : 1;
        MUINT32 AE_HST0_Y_LOW             : 7;
        MUINT32 rsv_23                    : 1;
        MUINT32 AE_HST0_Y_HI              : 7;
        MUINT32 rsv_31                    : 1;
} ISP_CAM_AE_D_HST0_RNG_T;

typedef union {
    enum { MASK     = 0x7F7F7F7F };
    enum { DEFAULT  = 0x00000000 };
    typedef ISP_CAM_AE_D_HST0_RNG_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AE_D_HST0_RNG_T;

// CAM_AE_D_HST1_RNG CAM+0688H
typedef struct {
        MUINT32 AE_HST1_X_LOW             : 7;
        MUINT32 rsv_7                     : 1;
        MUINT32 AE_HST1_X_HI              : 7;
        MUINT32 rsv_15                    : 1;
        MUINT32 AE_HST1_Y_LOW             : 7;
        MUINT32 rsv_23                    : 1;
        MUINT32 AE_HST1_Y_HI              : 7;
        MUINT32 rsv_31                    : 1;
} ISP_CAM_AE_D_HST1_RNG_T;

typedef union {
    enum { MASK     = 0x7F7F7F7F };
    enum { DEFAULT  = 0x00000000 };
    typedef ISP_CAM_AE_D_HST1_RNG_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AE_D_HST1_RNG_T;

// CAM_AE_D_HST2_RNG CAM+068CH
typedef struct {
        MUINT32 AE_HST2_X_LOW             : 7;
        MUINT32 rsv_7                     : 1;
        MUINT32 AE_HST2_X_HI              : 7;
        MUINT32 rsv_15                    : 1;
        MUINT32 AE_HST2_Y_LOW             : 7;
        MUINT32 rsv_23                    : 1;
        MUINT32 AE_HST2_Y_HI              : 7;
        MUINT32 rsv_31                    : 1;
} ISP_CAM_AE_D_HST2_RNG_T;

typedef union {
    enum { MASK     = 0x7F7F7F7F };
    enum { DEFAULT  = 0x00000000 };
    typedef ISP_CAM_AE_D_HST2_RNG_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AE_D_HST2_RNG_T;

// CAM_AE_D_HST3_RNG CAM+0690H
typedef struct {
        MUINT32 AE_HST3_X_LOW             : 7;
        MUINT32 rsv_7                     : 1;
        MUINT32 AE_HST3_X_HI              : 7;
        MUINT32 rsv_15                    : 1;
        MUINT32 AE_HST3_Y_LOW             : 7;
        MUINT32 rsv_23                    : 1;
        MUINT32 AE_HST3_Y_HI              : 7;
        MUINT32 rsv_31                    : 1;
} ISP_CAM_AE_D_HST3_RNG_T;

typedef union {
    enum { MASK     = 0x7F7F7F7F };
    enum { DEFAULT  = 0x00000000 };
    typedef ISP_CAM_AE_D_HST3_RNG_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AE_D_HST3_RNG_T;

//
typedef union {
    enum { COUNT = 2 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_AE_D_RAWPREGAIN2_0_T    rawpregain2_0;
        ISP_NVRAM_AE_D_RAWPREGAIN2_1_T    rawpregain2_1;
    };
} ISP_NVRAM_AE_D_RAWPREGAIN2_T;

//
typedef union {
    enum { COUNT = 5 };
    MUINT32 set[COUNT];
    struct {
        ISP_CAM_AE_D_MATRIX_COEF0_T       matrix_coef0;
        ISP_CAM_AE_D_MATRIX_COEF1_T       matrix_coef1;
        ISP_CAM_AE_D_MATRIX_COEF2_T       matrix_coef2;
        ISP_CAM_AE_D_MATRIX_COEF3_T       matrix_coef3;
        ISP_CAM_AE_D_MATRIX_COEF4_T       matrix_coef4;
    };
} ISP_NVRAM_AE_D_MATRIX_COEF_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// HLR
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef struct {
    FIELD  HLR_DTH                               :  8;		/*  0.. 7, 0x000000FF */
    FIELD  HLR_DSL                               : 10;		/*  8..17, 0x0003FF00 */
    FIELD  HLR_MODE                              :  1;		/* 18..18, 0x00040000 */
    FIELD  HLR_SL_EN                             :  1;		/* 19..19, 0x00080000 */
    FIELD  HLR_RAT                               :  6;		/* 20..25, 0x03F00000 */
    FIELD  rsv_26                                :  2;		/* 26..27, 0x0C000000 */
    FIELD  HLR_EDGE                              :  4;		/* 28..31, 0xF0000000 */
} ISP_CAM_HLR_CFG_T; //	/* 0x1A004850 */

typedef union {
    typedef ISP_CAM_HLR_CFG_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_HLR_CFG_T;

typedef struct {
    FIELD  HLR_GAIN_B                            : 13;		/*  0..12, 0x00001FFF */
    FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
    FIELD  HLR_GAIN_GB                           : 13;		/* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
} ISP_CAM_HLR_GAIN_T; //	/* 0x1A004858 */

typedef union {
    typedef ISP_CAM_HLR_GAIN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_HLR_GAIN_T;

typedef struct {
    FIELD  HLR_GAIN_GR                           : 13;		/*  0..12, 0x00001FFF */
    FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
    FIELD  HLR_GAIN_R                            : 13;		/* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
} ISP_CAM_HLR_GAIN_1_T; //	/* 0x1A00485C */

typedef union {
    typedef ISP_CAM_HLR_GAIN_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_HLR_GAIN_1_T;

typedef struct {
    FIELD  HLR_OFST_B                            : 15;		/*  0..14, 0x00007FFF */
    FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
    FIELD  HLR_OFST_GB                           : 15;		/* 16..30, 0x7FFF0000 */
    FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
} ISP_CAM_HLR_OFST_T; //	/* 0x1A004860 */

typedef union {
    typedef ISP_CAM_HLR_OFST_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_HLR_OFST_T;

typedef struct {
    FIELD  HLR_OFST_GR                           : 15;		/*  0..14, 0x00007FFF */
    FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
    FIELD  HLR_OFST_R                            : 15;		/* 16..30, 0x7FFF0000 */
    FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
} ISP_CAM_HLR_OFST_1_T; //	/* 0x1A004864 */

typedef union {
    typedef ISP_CAM_HLR_OFST_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_HLR_OFST_1_T;

typedef struct {
    FIELD  HLR_IVGN_B                            : 11;		/*  0..10, 0x000007FF */
    FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
    FIELD  HLR_IVGN_GB                           : 11;		/* 16..26, 0x07FF0000 */
    FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
} ISP_CAM_HLR_IVGN_T; //	/* 0x1A004868 */

typedef union {
    typedef ISP_CAM_HLR_IVGN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_HLR_IVGN_T;

typedef struct {
    FIELD  HLR_IVGN_GR                           : 11;		/*  0..10, 0x000007FF */
    FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
    FIELD  HLR_IVGN_R                            : 11;		/* 16..26, 0x07FF0000 */
    FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
} ISP_CAM_HLR_IVGN_1_T; //	/* 0x1A00486C */

typedef union {
    typedef ISP_CAM_HLR_IVGN_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_HLR_IVGN_1_T;

typedef struct {
    FIELD  HLR_KC_C0                             : 12;		/*  0..11, 0x00000FFF */
    FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
    FIELD  HLR_KC_C1                             : 12;		/* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
} ISP_CAM_HLR_KC_T; //	/* 0x1A004870 */

typedef union {
    typedef ISP_CAM_HLR_KC_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_HLR_KC_T;

typedef struct {
    FIELD  HLR_STR                               :  7;		/*  0.. 6, 0x0000007F */
    FIELD  rsv_7                                 : 25;		/*  7..31, 0xFFFFFF80 */
} ISP_CAM_HLR_CFG_1_T; //	/* 0x1A004874 */

typedef union {
    typedef ISP_CAM_HLR_CFG_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_HLR_CFG_1_T;

typedef struct {
    FIELD  HLR_SL_Y1                             : 10;		/*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
    FIELD  HLR_SL_Y2                             : 10;		/* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
} ISP_CAM_HLR_SL_PARA_T; //	/* 0x1A004878 */

typedef union {
    typedef ISP_CAM_HLR_SL_PARA_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_HLR_SL_PARA_T;

typedef struct {
    FIELD  HLR_SL_Y3                             : 10;		/*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
    FIELD  HLR_SL_Y4                             : 10;		/* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
} ISP_CAM_HLR_SL_PARA_1_T; //	/* 0x1A00487C */

typedef union {
    typedef ISP_CAM_HLR_SL_PARA_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_HLR_SL_PARA_1_T;

typedef union {
    enum { COUNT = 5 };
    struct {
        ISP_NVRAM_HLR_CFG_T        cfg;
        //ISP_NVRAM_HLR_GAIN_T       gain;
        //ISP_NVRAM_HLR_GAIN_1_T     gain_1;
        //ISP_NVRAM_HLR_OFST_T       ofst;
        //ISP_NVRAM_HLR_OFST_1_T     ofst_1;
        //ISP_NVRAM_HLR_IVGN_T       ivgn;
        //ISP_NVRAM_HLR_IVGN_1_T     ivgn_1;
        ISP_NVRAM_HLR_KC_T         kc;
        ISP_NVRAM_HLR_CFG_1_T      cfg_1;
        ISP_NVRAM_HLR_SL_PARA_T    sl_para;
        ISP_NVRAM_HLR_SL_PARA_1_T  sl_para_1;
    };
    MUINT32 set[COUNT];
} ISP_NVRAM_HLR_T;

typedef union {
    enum { COUNT = 11 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_HLR_CFG_T        cfg;
        ISP_NVRAM_HLR_GAIN_T       gain;
        ISP_NVRAM_HLR_GAIN_1_T     gain_1;
        ISP_NVRAM_HLR_OFST_T       ofst;
        ISP_NVRAM_HLR_OFST_1_T     ofst_1;
        ISP_NVRAM_HLR_IVGN_T       ivgn;
        ISP_NVRAM_HLR_IVGN_1_T     ivgn_1;
        ISP_NVRAM_HLR_KC_T         kc;
        ISP_NVRAM_HLR_CFG_1_T      cfg_1;
        ISP_NVRAM_HLR_SL_PARA_T    sl_para;
        ISP_NVRAM_HLR_SL_PARA_1_T  sl_para_1;
    };
} ISP_NVRAM_HLR_GET_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// color
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef struct {
    FIELD  COLOR_C2P_BYPASS                      :  1;		/*  0.. 0, 0x00000001 */
    FIELD  COLOR_P2C_BYPASS                      :  1;		/*  1.. 1, 0x00000002 */
    FIELD  COLOR_YENG_BYPASS                     :  1;		/*  2.. 2, 0x00000004 */
    FIELD  COLOR_SENG_BYPASS                     :  1;		/*  3.. 3, 0x00000008 */
    FIELD  COLOR_HENG_BYPASS                     :  1;		/*  4.. 4, 0x00000010 */
    FIELD  rsv_5                                 :  2;		/*  5.. 6, 0x00000060 */
    FIELD  COLOR_ALL_BYPASS                      :  1;		/*  7.. 7, 0x00000080 */
    FIELD  COLOR_WIDE_GAMUT_EN                   :  1;		/*  8.. 8, 0x00000100 */
    FIELD  rsv_9                                 :  4;		/*  9..12, 0x00001E00 */
    FIELD  COLOR_SEQ_SEL                         :  1;		/* 13..13, 0x00002000 */
    FIELD  rsv_14                                :  1;		/* 14..14, 0x00004000 */
    FIELD  COLOR_S_G_Y_EN                        :  1;		/* 15..15, 0x00008000 */
    FIELD  COLOR_C_PP_CM_DBG_SEL                 :  4;		/* 16..19, 0x000F0000 */
    FIELD  COLOR_LSP_EN                          :  1;		/* 20..20, 0x00100000 */
    FIELD  COLOR_LSP_SAT_LIMIT                   :  1;		/* 21..21, 0x00200000 */
    FIELD  COLOR_LSP_INK_EN                      :  1;		/* 22..22, 0x00400000 */
    FIELD  COLOR_LSP_SAT_SRC                     :  1;		/* 23..23, 0x00800000 */
    FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_CFG_MAIN_T; //	/* 0x150246F0 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CFG_MAIN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CFG_MAIN_T;

typedef struct {
    FIELD  COLOR_PIC_H_CNT                       : 16;		/*  0..15, 0x0000FFFF */
    FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
} ISP_DIP_X_CAM_COLOR_PXL_CNT_MAIN_T; //	/* 0x150246F4 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_PXL_CNT_MAIN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_PXL_CNT_MAIN_T;

typedef struct {
    FIELD  COLOR_LINE_CNT                        : 16;		/*  0..15, 0x0000FFFF */
    FIELD  COLOR_PIC_V_CNT                       : 16;		/* 16..31, 0xFFFF0000 */
} ISP_DIP_X_CAM_COLOR_LINE_CNT_MAIN_T; //	/* 0x150246F8 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_LINE_CNT_MAIN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_LINE_CNT_MAIN_T;

typedef struct {
    FIELD  COLOR_WIN_X_START                     : 16;		/*  0..15, 0x0000FFFF */
    FIELD  COLOR_WIN_X_END                       : 16;		/* 16..31, 0xFFFF0000 */
} ISP_DIP_X_CAM_COLOR_WIN_X_MAIN_T; //	/* 0x150246FC */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_WIN_X_MAIN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_WIN_X_MAIN_T;

typedef struct {
    FIELD  COLOR_WIN_Y_START                     : 16;		/*  0..15, 0x0000FFFF */
    FIELD  COLOR_WIN_Y_END                       : 16;		/* 16..31, 0xFFFF0000 */
} ISP_DIP_X_CAM_COLOR_WIN_Y_MAIN_T; //	/* 0x15024700 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_WIN_Y_MAIN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_WIN_Y_MAIN_T;

typedef struct {
    FIELD  COLOR_HTOTAL_CNT                      : 16;		/*  0..15, 0x0000FFFF */
    FIELD  COLOR_VTOTAL_CNT                      : 16;		/* 16..31, 0xFFFF0000 */
} ISP_DIP_X_CAM_COLOR_TIMING_DETECTION_0_T; //	/* 0x15024704 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_TIMING_DETECTION_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_TIMING_DETECTION_0_T;

typedef struct {
    FIELD  COLOR_HDE_CNT                         : 16;		/*  0..15, 0x0000FFFF */
    FIELD  COLOR_VDE_CNT                         : 16;		/* 16..31, 0xFFFF0000 */
} ISP_DIP_X_CAM_COLOR_TIMING_DETECTION_1_T; //	/* 0x15024708 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_TIMING_DETECTION_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_TIMING_DETECTION_1_T;

typedef struct {
    FIELD  COLOR_INK_EN                          :  1;		/*  0.. 0, 0x00000001 */
    FIELD  rsv_1                                 :  1;		/*  1.. 1, 0x00000002 */
    FIELD  COLOR_CAP_EN                          :  1;		/*  2.. 2, 0x00000004 */
    FIELD  COLOR_SPLIT_EN                        :  1;		/*  3.. 3, 0x00000008 */
    FIELD  COLOR_SPLIT_SWAP                      :  1;		/*  4.. 4, 0x00000010 */
    FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
    FIELD  COLOR_Y_INK                           :  1;		/*  8.. 8, 0x00000100 */
    FIELD  COLOR_U_INK                           :  1;		/*  9.. 9, 0x00000200 */
    FIELD  COLOR_V_INK                           :  1;		/* 10..10, 0x00000400 */
    FIELD  COLOR_W1_INK_EN                       :  1;		/* 11..11, 0x00000800 */
    FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
    FIELD  COLOR_Y_INK_MODE                      :  3;		/* 16..18, 0x00070000 */
    FIELD  COLOR_U_INK_MODE                      :  3;		/* 19..21, 0x00380000 */
    FIELD  COLOR_V_INK_MODE                      :  3;		/* 22..24, 0x01C00000 */
    FIELD  COLOR_Y_INK_SHIFT                     :  2;		/* 25..26, 0x06000000 */
    FIELD  COLOR_U_INK_SHIFT                     :  2;		/* 27..28, 0x18000000 */
    FIELD  COLOR_V_INK_SHIFT                     :  2;		/* 29..30, 0x60000000 */
    FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
} ISP_DIP_X_CAM_COLOR_DBG_CFG_MAIN_T; //	/* 0x1502470C */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_DBG_CFG_MAIN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_DBG_CFG_MAIN_T;

typedef struct {
    FIELD  COLOR_C_BOOST_GAIN                    :  8;		/*  0.. 7, 0x000000FF */
    FIELD  rsv_8                                 :  5;		/*  8..12, 0x00001F00 */
    FIELD  COLOR_C_NEW_BOOST_EN                  :  1;		/* 13..13, 0x00002000 */
    FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
    FIELD  COLOR_C_NEW_BOOST_LMT_L               :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_C_NEW_BOOST_LMT_U               :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_C_BOOST_MAIN_T; //	/* 0x15024710 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_C_BOOST_MAIN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_C_BOOST_MAIN_T;

typedef struct {
    FIELD  COLOR_COLOR_CBOOST_YOFFSET            :  8;		/*  0.. 7, 0x000000FF */
    FIELD  rsv_8                                 :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_COLOR_CBOOST_YOFFSET_SEL        :  2;		/* 16..17, 0x00030000 */
    FIELD  rsv_18                                :  6;		/* 18..23, 0x00FC0000 */
    FIELD  COLOR_COLOR_CBOOST_YCONST             :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_C_BOOST_MAIN_2_T; //	/* 0x15024714 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_C_BOOST_MAIN_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_C_BOOST_MAIN_2_T;

typedef struct {
    FIELD  COLOR_Y_LEVEL_ADJ                     :  7;		/*  0.. 6, 0x0000007F */
    FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
    FIELD  COLOR_Y_SLOPE_LMT                     :  8;		/*  8..15, 0x0000FF00 */
    FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
} ISP_DIP_X_CAM_COLOR_LUMA_ADJ_T; //	/* 0x15024718 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_LUMA_ADJ_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_LUMA_ADJ_T;

typedef struct {
    FIELD  COLOR_G_CONTRAST                      : 10;		/*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
    FIELD  COLOR_G_BRIGHTNESS                    : 11;		/* 16..26, 0x07FF0000 */
    FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
} ISP_DIP_X_CAM_COLOR_G_PIC_ADJ_MAIN_1_T; //	/* 0x1502471C */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_G_PIC_ADJ_MAIN_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_G_PIC_ADJ_MAIN_1_T;

typedef struct {
    FIELD  COLOR_G_SATURATION                    : 10;		/*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                : 22;		/* 10..31, 0xFFFFFC00 */
} ISP_DIP_X_CAM_COLOR_G_PIC_ADJ_MAIN_2_T; //	/* 0x15024720 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_G_PIC_ADJ_MAIN_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_G_PIC_ADJ_MAIN_2_T;

typedef struct {
    FIELD  COLOR_POS_X                           : 16;		/*  0..15, 0x0000FFFF */
    FIELD  COLOR_POS_Y                           : 16;		/* 16..31, 0xFFFF0000 */
} ISP_DIP_X_CAM_COLOR_POS_MAIN_T; //	/* 0x15024724 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_POS_MAIN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_POS_MAIN_T;

typedef struct {
    FIELD  COLOR_INK_DATA_Y                      : 10;		/*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
    FIELD  COLOR_INK_DATA_U                      : 10;		/* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
} ISP_DIP_X_CAM_COLOR_INK_DATA_MAIN_T; //	/* 0x15024728 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_INK_DATA_MAIN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_INK_DATA_MAIN_T;

typedef struct {
    FIELD  COLOR_INK_DATA_V                      : 10;		/*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                : 22;		/* 10..31, 0xFFFFFC00 */
} ISP_DIP_X_CAM_COLOR_INK_DATA_MAIN_CR_T; //	/* 0x1502472C */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_INK_DATA_MAIN_CR_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_INK_DATA_MAIN_CR_T;

typedef struct {
    FIELD  COLOR_CAP_IN_DATA_Y                   : 10;		/*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
    FIELD  COLOR_CAP_IN_DATA_U                   : 10;		/* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
} ISP_DIP_X_CAM_COLOR_CAP_IN_DATA_MAIN_T; //	/* 0x15024730 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CAP_IN_DATA_MAIN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CAP_IN_DATA_MAIN_T;

typedef struct {
    FIELD  COLOR_CAP_IN_DATA_V                   : 10;		/*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                : 22;		/* 10..31, 0xFFFFFC00 */
} ISP_DIP_X_CAM_COLOR_CAP_IN_DATA_MAIN_CR_T; //	/* 0x15024734 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CAP_IN_DATA_MAIN_CR_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CAP_IN_DATA_MAIN_CR_T;

typedef struct {
    FIELD  COLOR_CAP_OUT_DATA_Y                  : 10;		/*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
    FIELD  COLOR_CAP_OUT_DATA_U                  : 12;		/* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
} ISP_DIP_X_CAM_COLOR_CAP_OUT_DATA_MAIN_T; //	/* 0x15024738 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CAP_OUT_DATA_MAIN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CAP_OUT_DATA_MAIN_T;

typedef struct {
    FIELD  COLOR_CAP_OUT_DATA_V                  : 13;		/*  0..12, 0x00001FFF */
    FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
} ISP_DIP_X_CAM_COLOR_CAP_OUT_DATA_MAIN_CR_T; //	/* 0x1502473C */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CAP_OUT_DATA_MAIN_CR_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CAP_OUT_DATA_MAIN_CR_T;

typedef struct {
    FIELD  COLOR_Y_SLOPE_0                       :  8;		/*  0.. 7, 0x000000FF */
    FIELD  rsv_8                                 :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_Y_SLOPE_1                       :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_Y_SLOPE_1_0_MAIN_T; //	/* 0x15024740 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_Y_SLOPE_1_0_MAIN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_Y_SLOPE_1_0_MAIN_T;

typedef struct {
    FIELD  COLOR_Y_SLOPE_2                       :  8;		/*  0.. 7, 0x000000FF */
    FIELD  rsv_8                                 :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_Y_SLOPE_3                       :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_Y_SLOPE_3_2_MAIN_T; //	/* 0x15024744 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_Y_SLOPE_3_2_MAIN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_Y_SLOPE_3_2_MAIN_T;

typedef struct {
    FIELD  COLOR_Y_SLOPE_4                       :  8;		/*  0.. 7, 0x000000FF */
    FIELD  rsv_8                                 :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_Y_SLOPE_5                       :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_Y_SLOPE_5_4_MAIN_T; //	/* 0x15024748 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_Y_SLOPE_5_4_MAIN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_Y_SLOPE_5_4_MAIN_T;

typedef struct {
    FIELD  COLOR_Y_SLOPE_6                       :  8;		/*  0.. 7, 0x000000FF */
    FIELD  rsv_8                                 :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_Y_SLOPE_7                       :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_Y_SLOPE_7_6_MAIN_T; //	/* 0x1502474C */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_Y_SLOPE_7_6_MAIN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_Y_SLOPE_7_6_MAIN_T;

typedef struct {
    FIELD  COLOR_Y_SLOPE_8                       :  8;		/*  0.. 7, 0x000000FF */
    FIELD  rsv_8                                 :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_Y_SLOPE_9                       :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_Y_SLOPE_9_8_MAIN_T; //	/* 0x15024750 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_Y_SLOPE_9_8_MAIN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_Y_SLOPE_9_8_MAIN_T;

typedef struct {
    FIELD  COLOR_Y_SLOPE_10                      :  8;		/*  0.. 7, 0x000000FF */
    FIELD  rsv_8                                 :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_Y_SLOPE_11                      :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_Y_SLOPE_11_10_MAIN_T; //	/* 0x15024754 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_Y_SLOPE_11_10_MAIN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_Y_SLOPE_11_10_MAIN_T;

typedef struct {
    FIELD  COLOR_Y_SLOPE_12                      :  8;		/*  0.. 7, 0x000000FF */
    FIELD  rsv_8                                 :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_Y_SLOPE_13                      :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_Y_SLOPE_13_12_MAIN_T; //	/* 0x15024758 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_Y_SLOPE_13_12_MAIN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_Y_SLOPE_13_12_MAIN_T;

typedef struct {
    FIELD  COLOR_Y_SLOPE_14                      :  8;		/*  0.. 7, 0x000000FF */
    FIELD  rsv_8                                 :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_Y_SLOPE_15                      :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_Y_SLOPE_15_14_MAIN_T; //	/* 0x1502475C */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_Y_SLOPE_15_14_MAIN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_Y_SLOPE_15_14_MAIN_T;

typedef struct {
    FIELD  COLOR_HUE_TO_HUE_0                    :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_HUE_TO_HUE_1                    :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_HUE_TO_HUE_2                    :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_HUE_TO_HUE_3                    :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_LOCAL_HUE_CD_0_T; //	/* 0x15024760 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_LOCAL_HUE_CD_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_LOCAL_HUE_CD_0_T;

typedef struct {
    FIELD  COLOR_HUE_TO_HUE_4                    :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_HUE_TO_HUE_5                    :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_HUE_TO_HUE_6                    :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_HUE_TO_HUE_7                    :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_LOCAL_HUE_CD_1_T; //	/* 0x15024764 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_LOCAL_HUE_CD_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_LOCAL_HUE_CD_1_T;

typedef struct {
    FIELD  COLOR_HUE_TO_HUE_8                    :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_HUE_TO_HUE_9                    :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_HUE_TO_HUE_10                   :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_HUE_TO_HUE_11                   :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_LOCAL_HUE_CD_2_T; //	/* 0x15024768 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_LOCAL_HUE_CD_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_LOCAL_HUE_CD_2_T;

typedef struct {
    FIELD  COLOR_HUE_TO_HUE_12                   :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_HUE_TO_HUE_13                   :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_HUE_TO_HUE_14                   :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_HUE_TO_HUE_15                   :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_LOCAL_HUE_CD_3_T; //	/* 0x1502476C */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_LOCAL_HUE_CD_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_LOCAL_HUE_CD_3_T;

typedef struct {
    FIELD  COLOR_HUE_TO_HUE_16                   :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_HUE_TO_HUE_17                   :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_HUE_TO_HUE_18                   :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_HUE_TO_HUE_19                   :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_LOCAL_HUE_CD_4_T; //	/* 0x15024770 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_LOCAL_HUE_CD_4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_LOCAL_HUE_CD_4_T;

typedef struct {
    FIELD  COLOR_W1_HUE_LOWER                    :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_W1_HUE_UPPER                    :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_W1_SAT_LOWER                    :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_W1_SAT_UPPER                    :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_TWO_D_WINDOW_1_T; //	/* 0x15024774 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_TWO_D_WINDOW_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_TWO_D_WINDOW_1_T;

typedef struct {
    FIELD  COLOR_W1_HIT_OUTER                    : 24;		/*  0..23, 0x00FFFFFF */
    FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_TWO_D_W1_RESULT_T; //	/* 0x15024778 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_TWO_D_W1_RESULT_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_TWO_D_W1_RESULT_T;

typedef struct {
    FIELD  COLOR_HIST_X_START                    : 16;		/*  0..15, 0x0000FFFF */
    FIELD  COLOR_HIST_X_END                      : 16;		/* 16..31, 0xFFFF0000 */
} ISP_DIP_X_CAM_COLOR_SAT_HIST_X_CFG_MAIN_T; //	/* 0x1502477C */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_SAT_HIST_X_CFG_MAIN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_SAT_HIST_X_CFG_MAIN_T;

typedef struct {
    FIELD  COLOR_HIST_Y_START                    : 16;		/*  0..15, 0x0000FFFF */
    FIELD  COLOR_HIST_Y_END                      : 16;		/* 16..31, 0xFFFF0000 */
} ISP_DIP_X_CAM_COLOR_SAT_HIST_Y_CFG_MAIN_T; //	/* 0x15024780 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_SAT_HIST_Y_CFG_MAIN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_SAT_HIST_Y_CFG_MAIN_T;

typedef struct {
    FIELD  rsv_0                                 : 26;		/*  0..25, 0x03FFFFFF */
    FIELD  COLOR_CAM_COLOR_ATPG_OB               :  1;		/* 26..26, 0x04000000 */
    FIELD  COLOR_CAM_COLOR_ATPG_CT               :  1;		/* 27..27, 0x08000000 */
    FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
} ISP_DIP_X_CAM_COLOR_BWS_2_T; //	/* 0x15024784 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_BWS_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_BWS_2_T;

typedef struct {
    FIELD  COLOR_CRC_R_EN                        :  1;		/*  0.. 0, 0x00000001 */
    FIELD  COLOR_CRC_L_EN                        :  1;		/*  1.. 1, 0x00000002 */
    FIELD  COLOR_C_CRC_STILL_CHECK_TRIG          :  1;		/*  2.. 2, 0x00000004 */
    FIELD  COLOR_CRC_STILL_CHECK_DONE            :  1;		/*  3.. 3, 0x00000008 */
    FIELD  COLOR_CRC_NON_STILL_CNT               :  4;		/*  4.. 7, 0x000000F0 */
    FIELD  COLOR_CRC_STILL_CHECK_MAX             :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_CRC_SRC_SEL                     :  4;		/* 16..19, 0x000F0000 */
    FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
} ISP_DIP_X_CAM_COLOR_CRC_0_T; //	/* 0x15024788 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CRC_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CRC_0_T;

typedef struct {
    FIELD  COLOR_CRC_CLIP_H_START                : 13;		/*  0..12, 0x00001FFF */
    FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
    FIELD  COLOR_CRC_CLIP_H_END                  : 13;		/* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
} ISP_DIP_X_CAM_COLOR_CRC_1_T; //	/* 0x1502478C */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CRC_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CRC_1_T;

typedef struct {
    FIELD  COLOR_CRC_CLIP_V_START                : 12;		/*  0..11, 0x00000FFF */
    FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
    FIELD  COLOR_CRC_CLIP_V_END                  : 12;		/* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
} ISP_DIP_X_CAM_COLOR_CRC_2_T; //	/* 0x15024790 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CRC_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CRC_2_T;

typedef struct {
    FIELD  COLOR_CRC_V_MASK                      : 10;		/*  0.. 9, 0x000003FF */
    FIELD  COLOR_CRC_C_MASK                      : 10;		/* 10..19, 0x000FFC00 */
    FIELD  COLOR_CRC_Y_MASK                      : 10;		/* 20..29, 0x3FF00000 */
    FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
} ISP_DIP_X_CAM_COLOR_CRC_3_T; //	/* 0x15024794 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CRC_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CRC_3_T;

typedef struct {
    FIELD  COLOR_CRC_RESULT                      : 32;		/*  0..31, 0xFFFFFFFF */
} ISP_DIP_X_CAM_COLOR_CRC_4_T; //	/* 0x15024798 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CRC_4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CRC_4_T;

typedef struct {
    FIELD  COLOR_SAT_GAIN1_HUE_0                 :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_SAT_GAIN1_HUE_1                 :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_SAT_GAIN1_HUE_2                 :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_SAT_GAIN1_HUE_3                 :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN1_0_T; //	/* 0x1502479C */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN1_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN1_0_T;

typedef struct {
    FIELD  COLOR_SAT_GAIN1_HUE_4                 :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_SAT_GAIN1_HUE_5                 :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_SAT_GAIN1_HUE_6                 :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_SAT_GAIN1_HUE_7                 :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN1_1_T; //	/* 0x150247A0 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN1_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN1_1_T;

typedef struct {
    FIELD  COLOR_SAT_GAIN1_HUE_8                 :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_SAT_GAIN1_HUE_9                 :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_SAT_GAIN1_HUE_10                :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_SAT_GAIN1_HUE_11                :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN1_2_T; //	/* 0x150247A4 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN1_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN1_2_T;

typedef struct {
    FIELD  COLOR_SAT_GAIN1_HUE_12                :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_SAT_GAIN1_HUE_13                :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_SAT_GAIN1_HUE_14                :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_SAT_GAIN1_HUE_15                :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN1_3_T; //	/* 0x150247A8 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN1_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN1_3_T;

typedef struct {
    FIELD  COLOR_SAT_GAIN1_HUE_16                :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_SAT_GAIN1_HUE_17                :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_SAT_GAIN1_HUE_18                :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_SAT_GAIN1_HUE_19                :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN1_4_T; //	/* 0x150247AC */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN1_4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN1_4_T;

typedef struct {
    FIELD  COLOR_SAT_GAIN2_HUE_0                 :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_SAT_GAIN2_HUE_1                 :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_SAT_GAIN2_HUE_2                 :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_SAT_GAIN2_HUE_3                 :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN2_0_T; //	/* 0x150247B0 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN2_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN2_0_T;

typedef struct {
    FIELD  COLOR_SAT_GAIN2_HUE_4                 :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_SAT_GAIN2_HUE_5                 :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_SAT_GAIN2_HUE_6                 :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_SAT_GAIN2_HUE_7                 :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN2_1_T; //	/* 0x150247B4 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN2_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN2_1_T;

typedef struct {
    FIELD  COLOR_SAT_GAIN2_HUE_8                 :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_SAT_GAIN2_HUE_9                 :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_SAT_GAIN2_HUE_10                :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_SAT_GAIN2_HUE_11                :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN2_2_T; //	/* 0x150247B8 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN2_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN2_2_T;

typedef struct {
    FIELD  COLOR_SAT_GAIN2_HUE_12                :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_SAT_GAIN2_HUE_13                :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_SAT_GAIN2_HUE_14                :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_SAT_GAIN2_HUE_15                :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN2_3_T; //	/* 0x150247BC */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN2_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN2_3_T;

typedef struct {
    FIELD  COLOR_SAT_GAIN2_HUE_16                :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_SAT_GAIN2_HUE_17                :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_SAT_GAIN2_HUE_18                :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_SAT_GAIN2_HUE_19                :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN2_4_T; //	/* 0x150247C0 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN2_4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN2_4_T;

typedef struct {
    FIELD  COLOR_SAT_GAIN3_HUE_0                 :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_SAT_GAIN3_HUE_1                 :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_SAT_GAIN3_HUE_2                 :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_SAT_GAIN3_HUE_3                 :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN3_0_T; //	/* 0x150247C4 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN3_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN3_0_T;

typedef struct {
    FIELD  COLOR_SAT_GAIN3_HUE_4                 :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_SAT_GAIN3_HUE_5                 :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_SAT_GAIN3_HUE_6                 :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_SAT_GAIN3_HUE_7                 :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN3_1_T; //	/* 0x150247C8 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN3_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN3_1_T;

typedef struct {
    FIELD  COLOR_SAT_GAIN3_HUE_8                 :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_SAT_GAIN3_HUE_9                 :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_SAT_GAIN3_HUE_10                :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_SAT_GAIN3_HUE_11                :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN3_2_T; //	/* 0x150247CC */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN3_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN3_2_T;

typedef struct {
    FIELD  COLOR_SAT_GAIN3_HUE_12                :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_SAT_GAIN3_HUE_13                :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_SAT_GAIN3_HUE_14                :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_SAT_GAIN3_HUE_15                :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN3_3_T; //	/* 0x150247D0 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN3_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN3_3_T;

typedef struct {
    FIELD  COLOR_SAT_GAIN3_HUE_16                :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_SAT_GAIN3_HUE_17                :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_SAT_GAIN3_HUE_18                :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_SAT_GAIN3_HUE_19                :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN3_4_T; //	/* 0x150247D4 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN3_4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN3_4_T;

typedef struct {
    FIELD  COLOR_SAT_POINT1_HUE_0                :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_SAT_POINT1_HUE_1                :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_SAT_POINT1_HUE_2                :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_SAT_POINT1_HUE_3                :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_PARTIAL_SAT_POINT1_0_T; //	/* 0x150247D8 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_PARTIAL_SAT_POINT1_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_PARTIAL_SAT_POINT1_0_T;

typedef struct {
    FIELD  COLOR_SAT_POINT1_HUE_4                :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_SAT_POINT1_HUE_5                :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_SAT_POINT1_HUE_6                :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_SAT_POINT1_HUE_7                :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_PARTIAL_SAT_POINT1_1_T; //	/* 0x150247DC */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_PARTIAL_SAT_POINT1_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_PARTIAL_SAT_POINT1_1_T;

typedef struct {
    FIELD  COLOR_SAT_POINT1_HUE_8                :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_SAT_POINT1_HUE_9                :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_SAT_POINT1_HUE_10               :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_SAT_POINT1_HUE_11               :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_PARTIAL_SAT_POINT1_2_T; //	/* 0x150247E0 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_PARTIAL_SAT_POINT1_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_PARTIAL_SAT_POINT1_2_T;

typedef struct {
    FIELD  COLOR_SAT_POINT1_HUE_12               :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_SAT_POINT1_HUE_13               :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_SAT_POINT1_HUE_14               :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_SAT_POINT1_HUE_15               :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_PARTIAL_SAT_POINT1_3_T; //	/* 0x150247E4 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_PARTIAL_SAT_POINT1_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_PARTIAL_SAT_POINT1_3_T;

typedef struct {
    FIELD  COLOR_SAT_POINT1_HUE_16               :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_SAT_POINT1_HUE_17               :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_SAT_POINT1_HUE_18               :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_SAT_POINT1_HUE_19               :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_PARTIAL_SAT_POINT1_4_T; //	/* 0x150247E8 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_PARTIAL_SAT_POINT1_4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_PARTIAL_SAT_POINT1_4_T;

typedef struct {
    FIELD  COLOR_SAT_POINT2_HUE_0                :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_SAT_POINT2_HUE_1                :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_SAT_POINT2_HUE_2                :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_SAT_POINT2_HUE_3                :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_PARTIAL_SAT_POINT2_0_T; //	/* 0x150247EC */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_PARTIAL_SAT_POINT2_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_PARTIAL_SAT_POINT2_0_T;

typedef struct {
    FIELD  COLOR_SAT_POINT2_HUE_4                :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_SAT_POINT2_HUE_5                :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_SAT_POINT2_HUE_6                :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_SAT_POINT2_HUE_7                :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_PARTIAL_SAT_POINT2_1_T; //	/* 0x150247F0 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_PARTIAL_SAT_POINT2_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_PARTIAL_SAT_POINT2_1_T;

typedef struct {
    FIELD  COLOR_SAT_POINT2_HUE_8                :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_SAT_POINT2_HUE_9                :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_SAT_POINT2_HUE_10               :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_SAT_POINT2_HUE_11               :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_PARTIAL_SAT_POINT2_2_T; //	/* 0x150247F4 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_PARTIAL_SAT_POINT2_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_PARTIAL_SAT_POINT2_2_T;

typedef struct {
    FIELD  COLOR_SAT_POINT2_HUE_12               :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_SAT_POINT2_HUE_13               :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_SAT_POINT2_HUE_14               :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_SAT_POINT2_HUE_15               :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_PARTIAL_SAT_POINT2_3_T; //	/* 0x150247F8 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_PARTIAL_SAT_POINT2_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_PARTIAL_SAT_POINT2_3_T;

typedef struct {
    FIELD  COLOR_SAT_POINT2_HUE_16               :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_SAT_POINT2_HUE_17               :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_SAT_POINT2_HUE_18               :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_SAT_POINT2_HUE_19               :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_PARTIAL_SAT_POINT2_4_T; //	/* 0x150247FC */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_PARTIAL_SAT_POINT2_4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_PARTIAL_SAT_POINT2_4_T;

typedef struct {
    FIELD  COLOR_DISP_COLOR_START                :  1;		/*  0.. 0, 0x00000001 */
    FIELD  COLOR_DISP_COLOR_OUT_SEL              :  1;		/*  1.. 1, 0x00000002 */
    FIELD  COLOR_DISP_COLOR_WRAP_MODE            :  1;		/*  2.. 2, 0x00000004 */
    FIELD  COLOR_DISP_COLOR_DBUF_VSYNC           :  1;		/*  3.. 3, 0x00000008 */
    FIELD  COLOR_DIRECT_YUV_BIT_SEL              :  1;		/*  4.. 4, 0x00000010 */
    FIELD  COLOR_STOP_TO_WDMA                    :  1;		/*  5.. 5, 0x00000020 */
    FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
    FIELD  COLOR_DISP_COLOR_SW_RST_ENGINE        :  1;		/*  8.. 8, 0x00000100 */
    FIELD  rsv_9                                 :  7;		/*  9..15, 0x0000FE00 */
    FIELD  COLOR_DISP_COLOR_DBG_SEL              :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_START_T; //	/* 0x15024800 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_START_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_START_T;

typedef struct {
    FIELD  COLOR_DISP_COLOR_INTEN_FR_COMPLETE    :  1;		/*  0.. 0, 0x00000001 */
    FIELD  COLOR_DISP_COLOR_INTEN_FR_DONE        :  1;		/*  1.. 1, 0x00000002 */
    FIELD  COLOR_DISP_COLOR_INTEN_FR_UNDERRUN    :  1;		/*  2.. 2, 0x00000004 */
    FIELD  rsv_3                                 : 29;		/*  3..31, 0xFFFFFFF8 */
} ISP_DIP_X_CAM_COLOR_INTEN_T; //	/* 0x15024804 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_INTEN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_INTEN_T;

typedef struct {
    FIELD  COLOR_DISP_COLOR_INTSTA_FR_COMPLETE   :  1;		/*  0.. 0, 0x00000001 */
    FIELD  COLOR_DISP_COLOR_INTSTA_FR_DONE       :  1;		/*  1.. 1, 0x00000002 */
    FIELD  COLOR_DISP_COLOR_INTSTA_FR_UNDERRUN   :  1;		/*  2.. 2, 0x00000004 */
    FIELD  rsv_3                                 : 29;		/*  3..31, 0xFFFFFFF8 */
} ISP_DIP_X_CAM_COLOR_INTSTA_T; //	/* 0x15024808 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_INTSTA_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_INTSTA_T;

typedef struct {
    FIELD  COLOR_REG_CHA_SEL                     :  1;		/*  0.. 0, 0x00000001 */
    FIELD  COLOR_REG_CHA_ROUND                   :  1;		/*  1.. 1, 0x00000002 */
    FIELD  COLOR_REG_CHA_UV                      :  1;		/*  2.. 2, 0x00000004 */
    FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
    FIELD  COLOR_REG_CHB_SEL                     :  1;		/*  4.. 4, 0x00000010 */
    FIELD  COLOR_REG_CHB_ROUND                   :  1;		/*  5.. 5, 0x00000020 */
    FIELD  COLOR_REG_CHB_UV                      :  1;		/*  6.. 6, 0x00000040 */
    FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
    FIELD  COLOR_REG_CHC_SEL                     :  1;		/*  8.. 8, 0x00000100 */
    FIELD  COLOR_REG_CHC_ROUND                   :  1;		/*  9.. 9, 0x00000200 */
    FIELD  COLOR_REG_CHC_UV                      :  1;		/* 10..10, 0x00000400 */
    FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
} ISP_DIP_X_CAM_COLOR_OUT_SEL_T; //	/* 0x1502480C */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_OUT_SEL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_OUT_SEL_T;

typedef struct {
    FIELD  COLOR_DISP_COLOR_FRAME_DONE_DEL       :  8;		/*  0.. 7, 0x000000FF */
    FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
} ISP_DIP_X_CAM_COLOR_FRAME_DONE_DEL_T; //	/* 0x15024810 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_FRAME_DONE_DEL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_FRAME_DONE_DEL_T;

typedef struct {
    FIELD  COLOR_DISP_COLOR_CRC_CEN              :  1;		/*  0.. 0, 0x00000001 */
    FIELD  COLOR_DISP_COLOR_CRC_START            :  1;		/*  1.. 1, 0x00000002 */
    FIELD  COLOR_DISP_COLOR_CRC_CLR              :  1;		/*  2.. 2, 0x00000004 */
    FIELD  rsv_3                                 : 29;		/*  3..31, 0xFFFFFFF8 */
} ISP_DIP_X_CAM_COLOR_CRC_T; //	/* 0x15024814 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CRC_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CRC_T;

typedef struct {
    FIELD  COLOR_DISP_COLOR_SW_SCRATCH           : 32;		/*  0..31, 0xFFFFFFFF */
} ISP_DIP_X_CAM_COLOR_SW_SCRATCH_T; //	/* 0x15024818 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_SW_SCRATCH_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_SW_SCRATCH_T;

typedef struct {
    FIELD  COLOR_DISP_COLOR_CK_ON                :  1;		/*  0.. 0, 0x00000001 */
    FIELD  rsv_1                                 : 31;		/*  1..31, 0xFFFFFFFE */
} ISP_DIP_X_CAM_COLOR_CK_ON_T; //	/* 0x1502481C */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CK_ON_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CK_ON_T;

typedef struct {
    FIELD  COLOR_DISP_COLOR_INTERNAL_IP_WIDTH    : 14;		/*  0..13, 0x00003FFF */
    FIELD  rsv_14                                : 18;		/* 14..31, 0xFFFFC000 */
} ISP_DIP_X_CAM_COLOR_INTERNAL_IP_WIDTH_T; //	/* 0x15024820 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_INTERNAL_IP_WIDTH_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_INTERNAL_IP_WIDTH_T;

typedef struct {
    FIELD  COLOR_DISP_COLOR_INTERNAL_IP_HEIGHT   : 14;		/*  0..13, 0x00003FFF */
    FIELD  rsv_14                                : 18;		/* 14..31, 0xFFFFC000 */
} ISP_DIP_X_CAM_COLOR_INTERNAL_IP_HEIGHT_T; //	/* 0x15024824 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_INTERNAL_IP_HEIGHT_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_INTERNAL_IP_HEIGHT_T;

typedef struct {
    FIELD  COLOR_DISP_COLOR_CM1_EN               :  1;		/*  0.. 0, 0x00000001 */
    FIELD  COLOR_DISP_COLOR_CM1_CLIP_EN          :  1;		/*  1.. 1, 0x00000002 */
    FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
} ISP_DIP_X_CAM_COLOR_CM1_EN_T; //	/* 0x15024828 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CM1_EN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM1_EN_T;

typedef struct {
    FIELD  COLOR_DISP_COLOR_CM2_EN               :  1;		/*  0.. 0, 0x00000001 */
    FIELD  COLOR_DISP_COLOR_CM2_CLIP_SEL         :  2;		/*  1.. 2, 0x00000006 */
    FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
    FIELD  COLOR_DISP_COLOR_CM2_ROUND_OFF        :  1;		/*  4.. 4, 0x00000010 */
    FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
} ISP_DIP_X_CAM_COLOR_CM2_EN_T; //	/* 0x1502482C */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CM2_EN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM2_EN_T;

typedef struct {
    FIELD  COLOR_BYPASS_SHADOW                   :  1;		/*  0.. 0, 0x00000001 */
    FIELD  COLOR_FORCE_COMMIT                    :  1;		/*  1.. 1, 0x00000002 */
    FIELD  COLOR_READ_WRK_REG                    :  1;		/*  2.. 2, 0x00000004 */
    FIELD  rsv_3                                 : 29;		/*  3..31, 0xFFFFFFF8 */
} ISP_DIP_X_CAM_COLOR_SHADOW_CTRL_T; //	/* 0x15024830 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_SHADOW_CTRL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_SHADOW_CTRL_T;

typedef struct {
    FIELD  COLOR_DISP_COLOR_CRC_OUT_0            : 16;		/*  0..15, 0x0000FFFF */
    FIELD  COLOR_DISP_COLOR_CRC_RDY_0            :  1;		/* 16..16, 0x00010000 */
    FIELD  COLOR_DISP_COLOR_ENGINE_END           :  1;		/* 17..17, 0x00020000 */
    FIELD  rsv_18                                : 14;		/* 18..31, 0xFFFC0000 */
} ISP_DIP_X_CAM_COLOR_R0_CRC_T; //	/* 0x15024834 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_R0_CRC_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_R0_CRC_T;

typedef struct {
    FIELD  COLOR_S_GAIN_Y0_HUE_0                 :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_S_GAIN_Y0_HUE_1                 :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_S_GAIN_Y0_HUE_2                 :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_S_GAIN_Y0_HUE_3                 :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_S_GAIN_BY_Y0_0_T; //	/* 0x15024838 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_S_GAIN_BY_Y0_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_S_GAIN_BY_Y0_0_T;

typedef struct {
    FIELD  COLOR_S_GAIN_Y0_HUE_4                 :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_S_GAIN_Y0_HUE_5                 :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_S_GAIN_Y0_HUE_6                 :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_S_GAIN_Y0_HUE_7                 :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_S_GAIN_BY_Y0_1_T; //	/* 0x1502483C */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_S_GAIN_BY_Y0_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_S_GAIN_BY_Y0_1_T;

typedef struct {
    FIELD  COLOR_S_GAIN_Y0_HUE_8                 :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_S_GAIN_Y0_HUE_9                 :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_S_GAIN_Y0_HUE_10                :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_S_GAIN_Y0_HUE_11                :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_S_GAIN_BY_Y0_2_T; //	/* 0x15024840 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_S_GAIN_BY_Y0_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_S_GAIN_BY_Y0_2_T;

typedef struct {
    FIELD  COLOR_S_GAIN_Y0_HUE_12                :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_S_GAIN_Y0_HUE_13                :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_S_GAIN_Y0_HUE_14                :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_S_GAIN_Y0_HUE_15                :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_S_GAIN_BY_Y0_3_T; //	/* 0x15024844 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_S_GAIN_BY_Y0_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_S_GAIN_BY_Y0_3_T;

typedef struct {
    FIELD  COLOR_S_GAIN_Y0_HUE_16                :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_S_GAIN_Y0_HUE_17                :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_S_GAIN_Y0_HUE_18                :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_S_GAIN_Y0_HUE_19                :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_S_GAIN_BY_Y0_4_T; //	/* 0x15024848 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_S_GAIN_BY_Y0_4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_S_GAIN_BY_Y0_4_T;

typedef struct {
    FIELD  COLOR_S_GAIN_Y64_HUE_0                :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_S_GAIN_Y64_HUE_1                :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_S_GAIN_Y64_HUE_2                :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_S_GAIN_Y64_HUE_3                :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_S_GAIN_BY_Y64_0_T; //	/* 0x1502484C */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_S_GAIN_BY_Y64_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_S_GAIN_BY_Y64_0_T;

typedef struct {
    FIELD  COLOR_S_GAIN_Y64_HUE_4                :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_S_GAIN_Y64_HUE_5                :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_S_GAIN_Y64_HUE_6                :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_S_GAIN_Y64_HUE_7                :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_S_GAIN_BY_Y64_1_T; //	/* 0x15024850 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_S_GAIN_BY_Y64_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_S_GAIN_BY_Y64_1_T;

typedef struct {
    FIELD  COLOR_S_GAIN_Y64_HUE_8                :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_S_GAIN_Y64_HUE_9                :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_S_GAIN_Y64_HUE_10               :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_S_GAIN_Y64_HUE_11               :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_S_GAIN_BY_Y64_2_T; //	/* 0x15024854 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_S_GAIN_BY_Y64_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_S_GAIN_BY_Y64_2_T;

typedef struct {
    FIELD  COLOR_S_GAIN_Y64_HUE_12               :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_S_GAIN_Y64_HUE_13               :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_S_GAIN_Y64_HUE_14               :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_S_GAIN_Y64_HUE_15               :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_S_GAIN_BY_Y64_3_T; //	/* 0x15024858 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_S_GAIN_BY_Y64_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_S_GAIN_BY_Y64_3_T;

typedef struct {
    FIELD  COLOR_S_GAIN_Y64_HUE_16               :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_S_GAIN_Y64_HUE_17               :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_S_GAIN_Y64_HUE_18               :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_S_GAIN_Y64_HUE_19               :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_S_GAIN_BY_Y64_4_T; //	/* 0x1502485C */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_S_GAIN_BY_Y64_4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_S_GAIN_BY_Y64_4_T;

typedef struct {
    FIELD  COLOR_S_GAIN_Y128_HUE_0               :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_S_GAIN_Y128_HUE_1               :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_S_GAIN_Y128_HUE_2               :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_S_GAIN_Y128_HUE_3               :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_S_GAIN_BY_Y128_0_T; //	/* 0x15024860 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_S_GAIN_BY_Y128_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_S_GAIN_BY_Y128_0_T;

typedef struct {
    FIELD  COLOR_S_GAIN_Y128_HUE_4               :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_S_GAIN_Y128_HUE_5               :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_S_GAIN_Y128_HUE_6               :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_S_GAIN_Y128_HUE_7               :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_S_GAIN_BY_Y128_1_T; //	/* 0x15024864 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_S_GAIN_BY_Y128_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_S_GAIN_BY_Y128_1_T;

typedef struct {
    FIELD  COLOR_S_GAIN_Y128_HUE_8               :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_S_GAIN_Y128_HUE_9               :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_S_GAIN_Y128_HUE_10              :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_S_GAIN_Y128_HUE_11              :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_S_GAIN_BY_Y128_2_T; //	/* 0x15024868 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_S_GAIN_BY_Y128_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_S_GAIN_BY_Y128_2_T;

typedef struct {
    FIELD  COLOR_S_GAIN_Y128_HUE_12              :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_S_GAIN_Y128_HUE_13              :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_S_GAIN_Y128_HUE_14              :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_S_GAIN_Y128_HUE_15              :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_S_GAIN_BY_Y128_3_T; //	/* 0x1502486C */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_S_GAIN_BY_Y128_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_S_GAIN_BY_Y128_3_T;

typedef struct {
    FIELD  COLOR_S_GAIN_Y128_HUE_16              :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_S_GAIN_Y128_HUE_17              :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_S_GAIN_Y128_HUE_18              :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_S_GAIN_Y128_HUE_19              :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_S_GAIN_BY_Y128_4_T; //	/* 0x15024870 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_S_GAIN_BY_Y128_4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_S_GAIN_BY_Y128_4_T;

typedef struct {
    FIELD  COLOR_S_GAIN_Y192_HUE_0               :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_S_GAIN_Y192_HUE_1               :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_S_GAIN_Y192_HUE_2               :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_S_GAIN_Y192_HUE_3               :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_S_GAIN_BY_Y192_0_T; //	/* 0x15024874 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_S_GAIN_BY_Y192_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_S_GAIN_BY_Y192_0_T;

typedef struct {
    FIELD  COLOR_S_GAIN_Y192_HUE_4               :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_S_GAIN_Y192_HUE_5               :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_S_GAIN_Y192_HUE_6               :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_S_GAIN_Y192_HUE_7               :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_S_GAIN_BY_Y192_1_T; //	/* 0x15024878 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_S_GAIN_BY_Y192_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_S_GAIN_BY_Y192_1_T;

typedef struct {
    FIELD  COLOR_S_GAIN_Y192_HUE_8               :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_S_GAIN_Y192_HUE_9               :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_S_GAIN_Y192_HUE_10              :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_S_GAIN_Y192_HUE_11              :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_S_GAIN_BY_Y192_2_T; //	/* 0x1502487C */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_S_GAIN_BY_Y192_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_S_GAIN_BY_Y192_2_T;

typedef struct {
    FIELD  COLOR_S_GAIN_Y192_HUE_12              :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_S_GAIN_Y192_HUE_13              :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_S_GAIN_Y192_HUE_14              :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_S_GAIN_Y192_HUE_15              :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_S_GAIN_BY_Y192_3_T; //	/* 0x15024880 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_S_GAIN_BY_Y192_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_S_GAIN_BY_Y192_3_T;

typedef struct {
    FIELD  COLOR_S_GAIN_Y192_HUE_16              :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_S_GAIN_Y192_HUE_17              :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_S_GAIN_Y192_HUE_18              :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_S_GAIN_Y192_HUE_19              :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_S_GAIN_BY_Y192_4_T; //	/* 0x15024884 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_S_GAIN_BY_Y192_4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_S_GAIN_BY_Y192_4_T;

typedef struct {
    FIELD  COLOR_S_GAIN_Y256_HUE_0               :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_S_GAIN_Y256_HUE_1               :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_S_GAIN_Y256_HUE_2               :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_S_GAIN_Y256_HUE_3               :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_S_GAIN_BY_Y256_0_T; //	/* 0x15024888 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_S_GAIN_BY_Y256_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_S_GAIN_BY_Y256_0_T;

typedef struct {
    FIELD  COLOR_S_GAIN_Y256_HUE_4               :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_S_GAIN_Y256_HUE_5               :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_S_GAIN_Y256_HUE_6               :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_S_GAIN_Y256_HUE_7               :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_S_GAIN_BY_Y256_1_T; //	/* 0x1502488C */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_S_GAIN_BY_Y256_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_S_GAIN_BY_Y256_1_T;

typedef struct {
    FIELD  COLOR_S_GAIN_Y256_HUE_8               :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_S_GAIN_Y256_HUE_9               :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_S_GAIN_Y256_HUE_10              :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_S_GAIN_Y256_HUE_11              :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_S_GAIN_BY_Y256_2_T; //	/* 0x15024890 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_S_GAIN_BY_Y256_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_S_GAIN_BY_Y256_2_T;

typedef struct {
    FIELD  COLOR_S_GAIN_Y256_HUE_12              :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_S_GAIN_Y256_HUE_13              :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_S_GAIN_Y256_HUE_14              :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_S_GAIN_Y256_HUE_15              :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_S_GAIN_BY_Y256_3_T; //	/* 0x15024894 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_S_GAIN_BY_Y256_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_S_GAIN_BY_Y256_3_T;

typedef struct {
    FIELD  COLOR_S_GAIN_Y256_HUE_16              :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_S_GAIN_Y256_HUE_17              :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_S_GAIN_Y256_HUE_18              :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_S_GAIN_Y256_HUE_19              :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_S_GAIN_BY_Y256_4_T; //	/* 0x15024898 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_S_GAIN_BY_Y256_4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_S_GAIN_BY_Y256_4_T;

typedef struct {
    FIELD  COLOR_LSP_LIRE_YSLOPE                 :  7;		/*  0.. 6, 0x0000007F */
    FIELD  COLOR_LSP_LIRE_SSLOPE                 :  7;		/*  7..13, 0x00003F80 */
    FIELD  COLOR_LSP_LIRE_YTH                    :  8;		/* 14..21, 0x003FC000 */
    FIELD  COLOR_LSP_LIRE_STH                    :  7;		/* 22..28, 0x1FC00000 */
    FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
} ISP_DIP_X_CAM_COLOR_LSP_1_T; //	/* 0x1502489C */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_LSP_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_LSP_1_T;

typedef struct {
    FIELD  COLOR_LSP_LIRE_GAIN                   :  7;		/*  0.. 6, 0x0000007F */
    FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
    FIELD  COLOR_LSP_LSAT_GAIN                   :  7;		/*  8..14, 0x00007F00 */
    FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
    FIELD  COLOR_LSP_LSAT_STH                    :  7;		/* 16..22, 0x007F0000 */
    FIELD  COLOR_LSP_LSAT_SSLOPE                 :  7;		/* 23..29, 0x3F800000 */
    FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
} ISP_DIP_X_CAM_COLOR_LSP_2_T; //	/* 0x150248A0 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_LSP_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_LSP_2_T;

typedef struct {
    FIELD  COLOR_CM_BYPASS                       :  1;		/*  0.. 0, 0x00000001 */
    FIELD  COLOR_CM_W1_EN                        :  1;		/*  1.. 1, 0x00000002 */
    FIELD  COLOR_CM_W1_WGT_EN                    :  1;		/*  2.. 2, 0x00000004 */
    FIELD  COLOR_CM_W1_INK_EN                    :  1;		/*  3.. 3, 0x00000008 */
    FIELD  COLOR_CM_W2_EN                        :  1;		/*  4.. 4, 0x00000010 */
    FIELD  COLOR_CM_W2_WGT_EN                    :  1;		/*  5.. 5, 0x00000020 */
    FIELD  COLOR_CM_W2_INK_EN                    :  1;		/*  6.. 6, 0x00000040 */
    FIELD  COLOR_CM_W3_EN                        :  1;		/*  7.. 7, 0x00000080 */
    FIELD  COLOR_CM_W3_WGT_EN                    :  1;		/*  8.. 8, 0x00000100 */
    FIELD  COLOR_CM_W3_INK_EN                    :  1;		/*  9.. 9, 0x00000200 */
    FIELD  COLOR_CM_INK_DELTA_MODE               :  1;		/* 10..10, 0x00000400 */
    FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
    FIELD  COLOR_CM_INK_CH                       :  2;		/* 12..13, 0x00003000 */
    FIELD  rsv_14                                : 18;		/* 14..31, 0xFFFFC000 */
} ISP_DIP_X_CAM_COLOR_CM_CONTROL_T; //	/* 0x150248A4 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CM_CONTROL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_CONTROL_T;

typedef struct {
    FIELD  COLOR_CM_H_W1_L                       : 10;		/*  0.. 9, 0x000003FF */
    FIELD  COLOR_CM_H_W1_U                       : 10;		/* 10..19, 0x000FFC00 */
    FIELD  COLOR_CM_H_W1_POINT0                  : 10;		/* 20..29, 0x3FF00000 */
    FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
} ISP_DIP_X_CAM_COLOR_CM_W1_HUE_0_T; //	/* 0x150248A8 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CM_W1_HUE_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W1_HUE_0_T;

typedef struct {
    FIELD  COLOR_CM_H_W1_POINT1                  : 10;		/*  0.. 9, 0x000003FF */
    FIELD  COLOR_CM_H_W1_POINT2                  : 10;		/* 10..19, 0x000FFC00 */
    FIELD  COLOR_CM_H_W1_POINT3                  : 10;		/* 20..29, 0x3FF00000 */
    FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
} ISP_DIP_X_CAM_COLOR_CM_W1_HUE_1_T; //	/* 0x150248AC */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CM_W1_HUE_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W1_HUE_1_T;

typedef struct {
    FIELD  COLOR_CM_H_W1_POINT4                  : 10;		/*  0.. 9, 0x000003FF */
    FIELD  COLOR_CM_H_W1_SLOPE0                  :  8;		/* 10..17, 0x0003FC00 */
    FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
    FIELD  COLOR_CM_H_W1_SLOPE1                  :  8;		/* 20..27, 0x0FF00000 */
    FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
} ISP_DIP_X_CAM_COLOR_CM_W1_HUE_2_T; //	/* 0x150248B0 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CM_W1_HUE_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W1_HUE_2_T;

typedef struct {
    FIELD  COLOR_CM_H_W1_SLOPE2                  :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_CM_H_W1_SLOPE3                  :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_CM_H_W1_SLOPE4                  :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_CM_H_W1_SLOPE5                  :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_CM_W1_HUE_3_T; //	/* 0x150248B4 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CM_W1_HUE_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W1_HUE_3_T;

typedef struct {
    FIELD  COLOR_CM_H_W1_WGT_LSLOPE              : 10;		/*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
    FIELD  COLOR_CM_H_W1_WGT_USLOPE              : 10;		/* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
} ISP_DIP_X_CAM_COLOR_CM_W1_HUE_4_T; //	/* 0x150248B8 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CM_W1_HUE_4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W1_HUE_4_T;

typedef struct {
    FIELD  COLOR_CM_Y_W1_L                       : 10;		/*  0.. 9, 0x000003FF */
    FIELD  COLOR_CM_Y_W1_U                       : 10;		/* 10..19, 0x000FFC00 */
    FIELD  COLOR_CM_Y_W1_POINT0                  : 10;		/* 20..29, 0x3FF00000 */
    FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
} ISP_DIP_X_CAM_COLOR_CM_W1_LUMA_0_T; //	/* 0x150248BC */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CM_W1_LUMA_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W1_LUMA_0_T;

typedef struct {
    FIELD  COLOR_CM_Y_W1_POINT1                  : 10;		/*  0.. 9, 0x000003FF */
    FIELD  COLOR_CM_Y_W1_POINT2                  : 10;		/* 10..19, 0x000FFC00 */
    FIELD  COLOR_CM_Y_W1_POINT3                  : 10;		/* 20..29, 0x3FF00000 */
    FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
} ISP_DIP_X_CAM_COLOR_CM_W1_LUMA_1_T; //	/* 0x150248C0 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CM_W1_LUMA_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W1_LUMA_1_T;

typedef struct {
    FIELD  COLOR_CM_Y_W1_POINT4                  : 10;		/*  0.. 9, 0x000003FF */
    FIELD  COLOR_CM_Y_W1_SLOPE0                  :  8;		/* 10..17, 0x0003FC00 */
    FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
    FIELD  COLOR_CM_Y_W1_SLOPE1                  :  8;		/* 20..27, 0x0FF00000 */
    FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
} ISP_DIP_X_CAM_COLOR_CM_W1_LUMA_2_T; //	/* 0x150248C4 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CM_W1_LUMA_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W1_LUMA_2_T;

typedef struct {
    FIELD  COLOR_CM_Y_W1_SLOPE2                  :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_CM_Y_W1_SLOPE3                  :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_CM_Y_W1_SLOPE4                  :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_CM_Y_W1_SLOPE5                  :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_CM_W1_LUMA_3_T; //	/* 0x150248C8 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CM_W1_LUMA_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W1_LUMA_3_T;

typedef struct {
    FIELD  COLOR_CM_Y_W1_WGT_LSLOPE              : 10;		/*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
    FIELD  COLOR_CM_Y_W1_WGT_USLOPE              : 10;		/* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
} ISP_DIP_X_CAM_COLOR_CM_W1_LUMA_4_T; //	/* 0x150248CC */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CM_W1_LUMA_4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W1_LUMA_4_T;

typedef struct {
    FIELD  COLOR_CM_S_W1_L                       : 10;		/*  0.. 9, 0x000003FF */
    FIELD  COLOR_CM_S_W1_U                       : 10;		/* 10..19, 0x000FFC00 */
    FIELD  COLOR_CM_S_W1_POINT0                  : 10;		/* 20..29, 0x3FF00000 */
    FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
} ISP_DIP_X_CAM_COLOR_CM_W1_SAT_0_T; //	/* 0x150248D0 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CM_W1_SAT_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W1_SAT_0_T;

typedef struct {
    FIELD  COLOR_CM_S_W1_POINT1                  : 10;		/*  0.. 9, 0x000003FF */
    FIELD  COLOR_CM_S_W1_POINT2                  : 10;		/* 10..19, 0x000FFC00 */
    FIELD  COLOR_CM_S_W1_POINT3                  : 10;		/* 20..29, 0x3FF00000 */
    FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
} ISP_DIP_X_CAM_COLOR_CM_W1_SAT_1_T; //	/* 0x150248D4 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CM_W1_SAT_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W1_SAT_1_T;

typedef struct {
    FIELD  COLOR_CM_S_W1_POINT4                  : 10;		/*  0.. 9, 0x000003FF */
    FIELD  COLOR_CM_S_W1_SLOPE0                  :  8;		/* 10..17, 0x0003FC00 */
    FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
    FIELD  COLOR_CM_S_W1_SLOPE1                  :  8;		/* 20..27, 0x0FF00000 */
    FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
} ISP_DIP_X_CAM_COLOR_CM_W1_SAT_2_T; //	/* 0x150248D8 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CM_W1_SAT_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W1_SAT_2_T;

typedef struct {
    FIELD  COLOR_CM_S_W1_SLOPE2                  :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_CM_S_W1_SLOPE3                  :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_CM_S_W1_SLOPE4                  :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_CM_S_W1_SLOPE5                  :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_CM_W1_SAT_3_T; //	/* 0x150248DC */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CM_W1_SAT_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W1_SAT_3_T;

typedef struct {
    FIELD  COLOR_CM_S_W1_WGT_LSLOPE              : 10;		/*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
    FIELD  COLOR_CM_S_W1_WGT_USLOPE              : 10;		/* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
} ISP_DIP_X_CAM_COLOR_CM_W1_SAT_4_T; //	/* 0x150248E0 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CM_W1_SAT_4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W1_SAT_4_T;

typedef struct {
    FIELD  COLOR_CM_H_W2_L                       : 10;		/*  0.. 9, 0x000003FF */
    FIELD  COLOR_CM_H_W2_U                       : 10;		/* 10..19, 0x000FFC00 */
    FIELD  COLOR_CM_H_W2_POINT0                  : 10;		/* 20..29, 0x3FF00000 */
    FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
} ISP_DIP_X_CAM_COLOR_CM_W2_HUE_0_T; //	/* 0x150248E4 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CM_W2_HUE_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W2_HUE_0_T;

typedef struct {
    FIELD  COLOR_CM_H_W2_POINT1                  : 10;		/*  0.. 9, 0x000003FF */
    FIELD  COLOR_CM_H_W2_POINT2                  : 10;		/* 10..19, 0x000FFC00 */
    FIELD  COLOR_CM_H_W2_POINT3                  : 10;		/* 20..29, 0x3FF00000 */
    FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
} ISP_DIP_X_CAM_COLOR_CM_W2_HUE_1_T; //	/* 0x150248E8 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CM_W2_HUE_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W2_HUE_1_T;

typedef struct {
    FIELD  COLOR_CM_H_W2_POINT4                  : 10;		/*  0.. 9, 0x000003FF */
    FIELD  COLOR_CM_H_W2_SLOPE0                  :  8;		/* 10..17, 0x0003FC00 */
    FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
    FIELD  COLOR_CM_H_W2_SLOPE1                  :  8;		/* 20..27, 0x0FF00000 */
    FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
} ISP_DIP_X_CAM_COLOR_CM_W2_HUE_2_T; //	/* 0x150248EC */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CM_W2_HUE_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W2_HUE_2_T;

typedef struct {
    FIELD  COLOR_CM_H_W2_SLOPE2                  :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_CM_H_W2_SLOPE3                  :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_CM_H_W2_SLOPE4                  :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_CM_H_W2_SLOPE5                  :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_CM_W2_HUE_3_T; //	/* 0x150248F0 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CM_W2_HUE_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W2_HUE_3_T;

typedef struct {
    FIELD  COLOR_CM_H_W2_WGT_LSLOPE              : 10;		/*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
    FIELD  COLOR_CM_H_W2_WGT_USLOPE              : 10;		/* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
} ISP_DIP_X_CAM_COLOR_CM_W2_HUE_4_T; //	/* 0x150248F4 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CM_W2_HUE_4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W2_HUE_4_T;

typedef struct {
    FIELD  COLOR_CM_Y_W2_L                       : 10;		/*  0.. 9, 0x000003FF */
    FIELD  COLOR_CM_Y_W2_U                       : 10;		/* 10..19, 0x000FFC00 */
    FIELD  COLOR_CM_Y_W2_POINT0                  : 10;		/* 20..29, 0x3FF00000 */
    FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
} ISP_DIP_X_CAM_COLOR_CM_W2_LUMA_0_T; //	/* 0x150248F8 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CM_W2_LUMA_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W2_LUMA_0_T;

typedef struct {
    FIELD  COLOR_CM_Y_W2_POINT1                  : 10;		/*  0.. 9, 0x000003FF */
    FIELD  COLOR_CM_Y_W2_POINT2                  : 10;		/* 10..19, 0x000FFC00 */
    FIELD  COLOR_CM_Y_W2_POINT3                  : 10;		/* 20..29, 0x3FF00000 */
    FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
} ISP_DIP_X_CAM_COLOR_CM_W2_LUMA_1_T; //	/* 0x150248FC */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CM_W2_LUMA_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W2_LUMA_1_T;

typedef struct {
    FIELD  COLOR_CM_Y_W2_POINT4                  : 10;		/*  0.. 9, 0x000003FF */
    FIELD  COLOR_CM_Y_W2_SLOPE0                  :  8;		/* 10..17, 0x0003FC00 */
    FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
    FIELD  COLOR_CM_Y_W2_SLOPE1                  :  8;		/* 20..27, 0x0FF00000 */
    FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
} ISP_DIP_X_CAM_COLOR_CM_W2_LUMA_2_T; //	/* 0x15024900 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CM_W2_LUMA_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W2_LUMA_2_T;

typedef struct {
    FIELD  COLOR_CM_Y_W2_SLOPE2                  :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_CM_Y_W2_SLOPE3                  :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_CM_Y_W2_SLOPE4                  :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_CM_Y_W2_SLOPE5                  :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_CM_W2_LUMA_3_T; //	/* 0x15024904 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CM_W2_LUMA_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W2_LUMA_3_T;

typedef struct {
    FIELD  COLOR_CM_Y_W2_WGT_LSLOPE              : 10;		/*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
    FIELD  COLOR_CM_Y_W2_WGT_USLOPE              : 10;		/* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
} ISP_DIP_X_CAM_COLOR_CM_W2_LUMA_4_T; //	/* 0x15024908 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CM_W2_LUMA_4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W2_LUMA_4_T;

typedef struct {
    FIELD  COLOR_CM_S_W2_L                       : 10;		/*  0.. 9, 0x000003FF */
    FIELD  COLOR_CM_S_W2_U                       : 10;		/* 10..19, 0x000FFC00 */
    FIELD  COLOR_CM_S_W2_POINT0                  : 10;		/* 20..29, 0x3FF00000 */
    FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
} ISP_DIP_X_CAM_COLOR_CM_W2_SAT_0_T; //	/* 0x1502490C */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CM_W2_SAT_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W2_SAT_0_T;

typedef struct {
    FIELD  COLOR_CM_S_W2_POINT1                  : 10;		/*  0.. 9, 0x000003FF */
    FIELD  COLOR_CM_S_W2_POINT2                  : 10;		/* 10..19, 0x000FFC00 */
    FIELD  COLOR_CM_S_W2_POINT3                  : 10;		/* 20..29, 0x3FF00000 */
    FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
} ISP_DIP_X_CAM_COLOR_CM_W2_SAT_1_T; //	/* 0x15024910 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CM_W2_SAT_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W2_SAT_1_T;

typedef struct {
    FIELD  COLOR_CM_S_W2_POINT4                  : 10;		/*  0.. 9, 0x000003FF */
    FIELD  COLOR_CM_S_W2_SLOPE0                  :  8;		/* 10..17, 0x0003FC00 */
    FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
    FIELD  COLOR_CM_S_W2_SLOPE1                  :  8;		/* 20..27, 0x0FF00000 */
    FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
} ISP_DIP_X_CAM_COLOR_CM_W2_SAT_2_T; //	/* 0x15024914 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CM_W2_SAT_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W2_SAT_2_T;

typedef struct {
    FIELD  COLOR_CM_S_W2_SLOPE2                  :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_CM_S_W2_SLOPE3                  :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_CM_S_W2_SLOPE4                  :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_CM_S_W2_SLOPE5                  :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_CM_W2_SAT_3_T; //	/* 0x15024918 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CM_W2_SAT_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W2_SAT_3_T;

typedef struct {
    FIELD  COLOR_CM_S_W2_WGT_LSLOPE              : 10;		/*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
    FIELD  COLOR_CM_S_W2_WGT_USLOPE              : 10;		/* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
} ISP_DIP_X_CAM_COLOR_CM_W2_SAT_4_T; //	/* 0x1502491C */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CM_W2_SAT_4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W2_SAT_4_T;

typedef struct {
    FIELD  COLOR_CM_H_W3_L                       : 10;		/*  0.. 9, 0x000003FF */
    FIELD  COLOR_CM_H_W3_U                       : 10;		/* 10..19, 0x000FFC00 */
    FIELD  COLOR_CM_H_W3_POINT0                  : 10;		/* 20..29, 0x3FF00000 */
    FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
} ISP_DIP_X_CAM_COLOR_CM_W3_HUE_0_T; //	/* 0x15024920 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CM_W3_HUE_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W3_HUE_0_T;

typedef struct {
    FIELD  COLOR_CM_H_W3_POINT1                  : 10;		/*  0.. 9, 0x000003FF */
    FIELD  COLOR_CM_H_W3_POINT2                  : 10;		/* 10..19, 0x000FFC00 */
    FIELD  COLOR_CM_H_W3_POINT3                  : 10;		/* 20..29, 0x3FF00000 */
    FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
} ISP_DIP_X_CAM_COLOR_CM_W3_HUE_1_T; //	/* 0x15024924 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CM_W3_HUE_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W3_HUE_1_T;

typedef struct {
    FIELD  COLOR_CM_H_W3_POINT4                  : 10;		/*  0.. 9, 0x000003FF */
    FIELD  COLOR_CM_H_W3_SLOPE0                  :  8;		/* 10..17, 0x0003FC00 */
    FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
    FIELD  COLOR_CM_H_W3_SLOPE1                  :  8;		/* 20..27, 0x0FF00000 */
    FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
} ISP_DIP_X_CAM_COLOR_CM_W3_HUE_2_T; //	/* 0x15024928 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CM_W3_HUE_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W3_HUE_2_T;

typedef struct {
    FIELD  COLOR_CM_H_W3_SLOPE2                  :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_CM_H_W3_SLOPE3                  :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_CM_H_W3_SLOPE4                  :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_CM_H_W3_SLOPE5                  :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_CM_W3_HUE_3_T; //	/* 0x1502492C */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CM_W3_HUE_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W3_HUE_3_T;

typedef struct {
    FIELD  COLOR_CM_H_W3_WGT_LSLOPE              : 10;		/*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
    FIELD  COLOR_CM_H_W3_WGT_USLOPE              : 10;		/* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
} ISP_DIP_X_CAM_COLOR_CM_W3_HUE_4_T; //	/* 0x15024930 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CM_W3_HUE_4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W3_HUE_4_T;

typedef struct {
    FIELD  COLOR_CM_Y_W3_L                       : 10;		/*  0.. 9, 0x000003FF */
    FIELD  COLOR_CM_Y_W3_U                       : 10;		/* 10..19, 0x000FFC00 */
    FIELD  COLOR_CM_Y_W3_POINT0                  : 10;		/* 20..29, 0x3FF00000 */
    FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
} ISP_DIP_X_CAM_COLOR_CM_W3_LUMA_0_T; //	/* 0x15024934 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CM_W3_LUMA_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W3_LUMA_0_T;

typedef struct {
    FIELD  COLOR_CM_Y_W3_POINT1                  : 10;		/*  0.. 9, 0x000003FF */
    FIELD  COLOR_CM_Y_W3_POINT2                  : 10;		/* 10..19, 0x000FFC00 */
    FIELD  COLOR_CM_Y_W3_POINT3                  : 10;		/* 20..29, 0x3FF00000 */
    FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
} ISP_DIP_X_CAM_COLOR_CM_W3_LUMA_1_T; //	/* 0x15024938 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CM_W3_LUMA_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W3_LUMA_1_T;

typedef struct {
    FIELD  COLOR_CM_Y_W3_POINT4                  : 10;		/*  0.. 9, 0x000003FF */
    FIELD  COLOR_CM_Y_W3_SLOPE0                  :  8;		/* 10..17, 0x0003FC00 */
    FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
    FIELD  COLOR_CM_Y_W3_SLOPE1                  :  8;		/* 20..27, 0x0FF00000 */
    FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
} ISP_DIP_X_CAM_COLOR_CM_W3_LUMA_2_T; //	/* 0x1502493C */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CM_W3_LUMA_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W3_LUMA_2_T;

typedef struct {
    FIELD  COLOR_CM_Y_W3_SLOPE2                  :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_CM_Y_W3_SLOPE3                  :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_CM_Y_W3_SLOPE4                  :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_CM_Y_W3_SLOPE5                  :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_CM_W3_LUMA_3_T; //	/* 0x15024940 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CM_W3_LUMA_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W3_LUMA_3_T;

typedef struct {
    FIELD  COLOR_CM_Y_W3_WGT_LSLOPE              : 10;		/*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
    FIELD  COLOR_CM_Y_W3_WGT_USLOPE              : 10;		/* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
} ISP_DIP_X_CAM_COLOR_CM_W3_LUMA_4_T; //	/* 0x15024944 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CM_W3_LUMA_4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W3_LUMA_4_T;

typedef struct {
    FIELD  COLOR_CM_S_W3_L                       : 10;		/*  0.. 9, 0x000003FF */
    FIELD  COLOR_CM_S_W3_U                       : 10;		/* 10..19, 0x000FFC00 */
    FIELD  COLOR_CM_S_W3_POINT0                  : 10;		/* 20..29, 0x3FF00000 */
    FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
} ISP_DIP_X_CAM_COLOR_CM_W3_SAT_0_T; //	/* 0x15024948 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CM_W3_SAT_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W3_SAT_0_T;

typedef struct {
    FIELD  COLOR_CM_S_W3_POINT1                  : 10;		/*  0.. 9, 0x000003FF */
    FIELD  COLOR_CM_S_W3_POINT2                  : 10;		/* 10..19, 0x000FFC00 */
    FIELD  COLOR_CM_S_W3_POINT3                  : 10;		/* 20..29, 0x3FF00000 */
    FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
} ISP_DIP_X_CAM_COLOR_CM_W3_SAT_1_T; //	/* 0x1502494C */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CM_W3_SAT_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W3_SAT_1_T;

typedef struct {
    FIELD  COLOR_CM_S_W3_POINT4                  : 10;		/*  0.. 9, 0x000003FF */
    FIELD  COLOR_CM_S_W3_SLOPE0                  :  8;		/* 10..17, 0x0003FC00 */
    FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
    FIELD  COLOR_CM_S_W3_SLOPE1                  :  8;		/* 20..27, 0x0FF00000 */
    FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
} ISP_DIP_X_CAM_COLOR_CM_W3_SAT_2_T; //	/* 0x15024950 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CM_W3_SAT_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W3_SAT_2_T;

typedef struct {
    FIELD  COLOR_CM_S_W3_SLOPE2                  :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_CM_S_W3_SLOPE3                  :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_CM_S_W3_SLOPE4                  :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_CM_S_W3_SLOPE5                  :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_CAM_COLOR_CM_W3_SAT_3_T; //	/* 0x15024954 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CM_W3_SAT_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W3_SAT_3_T;

typedef struct {
    FIELD  COLOR_CM_S_W3_WGT_LSLOPE              : 10;		/*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
    FIELD  COLOR_CM_S_W3_WGT_USLOPE              : 10;		/* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
} ISP_DIP_X_CAM_COLOR_CM_W3_SAT_4_T; //	/* 0x15024958 */

typedef union {
    typedef ISP_DIP_X_CAM_COLOR_CM_W3_SAT_4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W3_SAT_4_T;

typedef union {
    enum { COUNT = 117 };
    struct {
        ISP_NVRAM_COLOR_CFG_MAIN_T cfg_main;
        //ISP_NVRAM_COLOR_PXL_CNT_MAIN_T  pxl_cnt_main;
        //ISP_NVRAM_COLOR_LINE_CNT_MAIN_T  line_cnt_main;
        //ISP_NVRAM_COLOR_WIN_X_MAIN_T  win_x_main;
        //ISP_NVRAM_COLOR_WIN_Y_MAIN_T  win_y_main;
        //ISP_NVRAM_COLOR_TIMING_DETECTION_0_T  timing_detection_0;
        //ISP_NVRAM_COLOR_TIMING_DETECTION_1_T  timing_detection_1;
        //ISP_NVRAM_COLOR_DBG_CFG_MAIN_T  dbg_cfg_main;
        ISP_NVRAM_COLOR_C_BOOST_MAIN_T  c_boost_main;
        ISP_NVRAM_COLOR_C_BOOST_MAIN_2_T  c_boost_main_2;
        ISP_NVRAM_COLOR_LUMA_ADJ_T  luma_adj;
        ISP_NVRAM_COLOR_G_PIC_ADJ_MAIN_1_T  g_pic_adj_main_1;
        ISP_NVRAM_COLOR_G_PIC_ADJ_MAIN_2_T  g_pic_adj_main_2;
        //ISP_NVRAM_COLOR_POS_MAIN_T  pos_main;
        //ISP_NVRAM_COLOR_INK_DATA_MAIN_T  ink_data_main;
        //ISP_NVRAM_COLOR_INK_DATA_MAIN_CR_T  ink_data_main_cr;
        //ISP_NVRAM_COLOR_CAP_IN_DATA_MAIN_T  cap_in_data_main;
        //ISP_NVRAM_COLOR_CAP_IN_DATA_MAIN_CR_T  cap_in_data_main_cr;
        //ISP_NVRAM_COLOR_CAP_OUT_DATA_MAIN_T  cap_out_data_main;
        //ISP_NVRAM_COLOR_CAP_OUT_DATA_MAIN_CR_T  cap_out_data_main_cr;
        ISP_NVRAM_COLOR_Y_SLOPE_1_0_MAIN_T  y_slope_1_0_main;
        ISP_NVRAM_COLOR_Y_SLOPE_3_2_MAIN_T  y_slope_3_2_main;
        ISP_NVRAM_COLOR_Y_SLOPE_5_4_MAIN_T  y_slope_5_4_main;
        ISP_NVRAM_COLOR_Y_SLOPE_7_6_MAIN_T  y_slope_7_6_main;
        ISP_NVRAM_COLOR_Y_SLOPE_9_8_MAIN_T  y_slope_9_8_main;
        ISP_NVRAM_COLOR_Y_SLOPE_11_10_MAIN_T  y_slope_11_10_main;
        ISP_NVRAM_COLOR_Y_SLOPE_13_12_MAIN_T  y_slope_13_12_main;
        ISP_NVRAM_COLOR_Y_SLOPE_15_14_MAIN_T  y_slope_15_14_main;
        ISP_NVRAM_COLOR_LOCAL_HUE_CD_0_T  local_hue_cd_0;
        ISP_NVRAM_COLOR_LOCAL_HUE_CD_1_T  local_hue_cd_1;
        ISP_NVRAM_COLOR_LOCAL_HUE_CD_2_T  local_hue_cd_2;
        ISP_NVRAM_COLOR_LOCAL_HUE_CD_3_T  local_hue_cd_3;
        ISP_NVRAM_COLOR_LOCAL_HUE_CD_4_T  local_hue_cd_4;
        //ISP_NVRAM_COLOR_TWO_D_WINDOW_1_T  two_d_window_1;
        //ISP_NVRAM_COLOR_TWO_D_W1_RESULT_T  two_d_w1_result;
        //ISP_NVRAM_COLOR_SAT_HIST_X_CFG_MAIN_T  sat_hist_x_cfg_main;
        //ISP_NVRAM_COLOR_SAT_HIST_Y_CFG_MAIN_T  sat_hist_y_cfg_main;
        //ISP_NVRAM_COLOR_BWS_2_T  bws_2;
        //ISP_NVRAM_COLOR_CRC_0_T  crc_0;
        //ISP_NVRAM_COLOR_CRC_1_T  crc_1;
        //ISP_NVRAM_COLOR_CRC_2_T  crc_2;
        //ISP_NVRAM_COLOR_CRC_3_T  crc_3;
        //ISP_NVRAM_COLOR_CRC_4_T  crc_4;
        ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN1_0_T  partial_sat_gain1_0;
        ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN1_1_T  partial_sat_gain1_1;
        ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN1_2_T  partial_sat_gain1_2;
        ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN1_3_T  partial_sat_gain1_3;
        ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN1_4_T  partial_sat_gain1_4;
        ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN2_0_T  partial_sat_gain2_0;
        ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN2_1_T  partial_sat_gain2_1;
        ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN2_2_T  partial_sat_gain2_2;
        ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN2_3_T  partial_sat_gain2_3;
        ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN2_4_T  partial_sat_gain2_4;
        ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN3_0_T  partial_sat_gain3_0;
        ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN3_1_T  partial_sat_gain3_1;
        ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN3_2_T  partial_sat_gain3_2;
        ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN3_3_T  partial_sat_gain3_3;
        ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN3_4_T  partial_sat_gain3_4;
        ISP_NVRAM_COLOR_PARTIAL_SAT_POINT1_0_T  partial_sat_point1_0;
        ISP_NVRAM_COLOR_PARTIAL_SAT_POINT1_1_T  partial_sat_point1_1;
        ISP_NVRAM_COLOR_PARTIAL_SAT_POINT1_2_T  partial_sat_point1_2;
        ISP_NVRAM_COLOR_PARTIAL_SAT_POINT1_3_T  partial_sat_point1_3;
        ISP_NVRAM_COLOR_PARTIAL_SAT_POINT1_4_T  partial_sat_point1_4;
        ISP_NVRAM_COLOR_PARTIAL_SAT_POINT2_0_T  partial_sat_point2_0;
        ISP_NVRAM_COLOR_PARTIAL_SAT_POINT2_1_T  partial_sat_point2_1;
        ISP_NVRAM_COLOR_PARTIAL_SAT_POINT2_2_T  partial_sat_point2_2;
        ISP_NVRAM_COLOR_PARTIAL_SAT_POINT2_3_T  partial_sat_point2_3;
        ISP_NVRAM_COLOR_PARTIAL_SAT_POINT2_4_T  partial_sat_point2_4;
        //ISP_NVRAM_COLOR_START_T  start;
        //ISP_NVRAM_COLOR_INTEN_T  inten;
        //ISP_NVRAM_COLOR_INTSTA_T  intsta;
        //ISP_NVRAM_COLOR_OUT_SEL_T  out_sel;
        //ISP_NVRAM_COLOR_FRAME_DONE_DEL_T  frame_done_del;
        //ISP_NVRAM_COLOR_CRC_T  crc;
        //ISP_NVRAM_COLOR_SW_SCRATCH_T  sw_scratch;
        //ISP_NVRAM_COLOR_CK_ON_T  ck_on;
        //ISP_NVRAM_COLOR_INTERNAL_IP_WIDTH_T  internal_ip_width;
        //ISP_NVRAM_COLOR_INTERNAL_IP_HEIGHT_T  internal_ip_height;
        //ISP_NVRAM_COLOR_CM1_EN_T  cm1_en;
        //ISP_NVRAM_COLOR_CM2_EN_T  cm2_en;
        //ISP_NVRAM_COLOR_SHADOW_CTRL_T  shadow_ctrl;
        //ISP_NVRAM_COLOR_R0_CRC_T  r0_crc;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y0_0_T  s_gain_by_y0_0;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y0_1_T  s_gain_by_y0_1;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y0_2_T  s_gain_by_y0_2;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y0_3_T  s_gain_by_y0_3;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y0_4_T  s_gain_by_y0_4;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y64_0_T  s_gain_by_y64_0;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y64_1_T  s_gain_by_y64_1;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y64_2_T  s_gain_by_y64_2;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y64_3_T  s_gain_by_y64_3;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y64_4_T  s_gain_by_y64_4;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y128_0_T  s_gain_by_y128_0;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y128_1_T  s_gain_by_y128_1;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y128_2_T  s_gain_by_y128_2;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y128_3_T  s_gain_by_y128_3;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y128_4_T  s_gain_by_y128_4;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y192_0_T  s_gain_by_y192_0;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y192_1_T  s_gain_by_y192_1;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y192_2_T  s_gain_by_y192_2;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y192_3_T  s_gain_by_y192_3;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y192_4_T  s_gain_by_y192_4;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y256_0_T  s_gain_by_y256_0;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y256_1_T  s_gain_by_y256_1;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y256_2_T  s_gain_by_y256_2;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y256_3_T  s_gain_by_y256_3;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y256_4_T  s_gain_by_y256_4;
        ISP_NVRAM_COLOR_LSP_1_T  lsp_1;
        ISP_NVRAM_COLOR_LSP_2_T  lsp_2;
        ISP_NVRAM_COLOR_CM_CONTROL_T  cm_control;
        ISP_NVRAM_COLOR_CM_W1_HUE_0_T  cm_w1_hue_0;
        ISP_NVRAM_COLOR_CM_W1_HUE_1_T  cm_w1_hue_1;
        ISP_NVRAM_COLOR_CM_W1_HUE_2_T  cm_w1_hue_2;
        ISP_NVRAM_COLOR_CM_W1_HUE_3_T  cm_w1_hue_3;
        ISP_NVRAM_COLOR_CM_W1_HUE_4_T  cm_w1_hue_4;
        ISP_NVRAM_COLOR_CM_W1_LUMA_0_T  cm_w1_luma_0;
        ISP_NVRAM_COLOR_CM_W1_LUMA_1_T  cm_w1_luma_1;
        ISP_NVRAM_COLOR_CM_W1_LUMA_2_T  cm_w1_luma_2;
        ISP_NVRAM_COLOR_CM_W1_LUMA_3_T  cm_w1_luma_3;
        ISP_NVRAM_COLOR_CM_W1_LUMA_4_T  cm_w1_luma_4;
        ISP_NVRAM_COLOR_CM_W1_SAT_0_T  cm_w1_sat_0;
        ISP_NVRAM_COLOR_CM_W1_SAT_1_T  cm_w1_sat_1;
        ISP_NVRAM_COLOR_CM_W1_SAT_2_T  cm_w1_sat_2;
        ISP_NVRAM_COLOR_CM_W1_SAT_3_T  cm_w1_sat_3;
        ISP_NVRAM_COLOR_CM_W1_SAT_4_T  cm_w1_sat_4;
        ISP_NVRAM_COLOR_CM_W2_HUE_0_T  cm_w2_hue_0;
        ISP_NVRAM_COLOR_CM_W2_HUE_1_T  cm_w2_hue_1;
        ISP_NVRAM_COLOR_CM_W2_HUE_2_T  cm_w2_hue_2;
        ISP_NVRAM_COLOR_CM_W2_HUE_3_T  cm_w2_hue_3;
        ISP_NVRAM_COLOR_CM_W2_HUE_4_T  cm_w2_hue_4;
        ISP_NVRAM_COLOR_CM_W2_LUMA_0_T  cm_w2_luma_0;
        ISP_NVRAM_COLOR_CM_W2_LUMA_1_T  cm_w2_luma_1;
        ISP_NVRAM_COLOR_CM_W2_LUMA_2_T  cm_w2_luma_2;
        ISP_NVRAM_COLOR_CM_W2_LUMA_3_T  cm_w2_luma_3;
        ISP_NVRAM_COLOR_CM_W2_LUMA_4_T  cm_w2_luma_4;
        ISP_NVRAM_COLOR_CM_W2_SAT_0_T  cm_w2_sat_0;
        ISP_NVRAM_COLOR_CM_W2_SAT_1_T  cm_w2_sat_1;
        ISP_NVRAM_COLOR_CM_W2_SAT_2_T  cm_w2_sat_2;
        ISP_NVRAM_COLOR_CM_W2_SAT_3_T  cm_w2_sat_3;
        ISP_NVRAM_COLOR_CM_W2_SAT_4_T  cm_w2_sat_4;
        ISP_NVRAM_COLOR_CM_W3_HUE_0_T  cm_w3_hue_0;
        ISP_NVRAM_COLOR_CM_W3_HUE_1_T  cm_w3_hue_1;
        ISP_NVRAM_COLOR_CM_W3_HUE_2_T  cm_w3_hue_2;
        ISP_NVRAM_COLOR_CM_W3_HUE_3_T  cm_w3_hue_3;
        ISP_NVRAM_COLOR_CM_W3_HUE_4_T  cm_w3_hue_4;
        ISP_NVRAM_COLOR_CM_W3_LUMA_0_T  cm_w3_luma_0;
        ISP_NVRAM_COLOR_CM_W3_LUMA_1_T  cm_w3_luma_1;
        ISP_NVRAM_COLOR_CM_W3_LUMA_2_T  cm_w3_luma_2;
        ISP_NVRAM_COLOR_CM_W3_LUMA_3_T  cm_w3_luma_3;
        ISP_NVRAM_COLOR_CM_W3_LUMA_4_T  cm_w3_luma_4;
        ISP_NVRAM_COLOR_CM_W3_SAT_0_T  cm_w3_sat_0;
        ISP_NVRAM_COLOR_CM_W3_SAT_1_T  cm_w3_sat_1;
        ISP_NVRAM_COLOR_CM_W3_SAT_2_T  cm_w3_sat_2;
        ISP_NVRAM_COLOR_CM_W3_SAT_3_T  cm_w3_sat_3;
        ISP_NVRAM_COLOR_CM_W3_SAT_4_T  cm_w3_sat_4;
    };
    MUINT32 set[COUNT];
} ISP_NVRAM_COLOR_T;

typedef union {
    enum { COUNT = 155 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_COLOR_CFG_MAIN_T cfg_main;
        ISP_NVRAM_COLOR_PXL_CNT_MAIN_T  pxl_cnt_main;
        ISP_NVRAM_COLOR_LINE_CNT_MAIN_T  line_cnt_main;
        ISP_NVRAM_COLOR_WIN_X_MAIN_T  win_x_main;
        ISP_NVRAM_COLOR_WIN_Y_MAIN_T  win_y_main;
        ISP_NVRAM_COLOR_TIMING_DETECTION_0_T  timing_detection_0;
        ISP_NVRAM_COLOR_TIMING_DETECTION_1_T  timing_detection_1;
        ISP_NVRAM_COLOR_DBG_CFG_MAIN_T  dbg_cfg_main;
        ISP_NVRAM_COLOR_C_BOOST_MAIN_T  c_boost_main;
        ISP_NVRAM_COLOR_C_BOOST_MAIN_2_T  c_boost_main_2;
        ISP_NVRAM_COLOR_LUMA_ADJ_T  luma_adj;
        ISP_NVRAM_COLOR_G_PIC_ADJ_MAIN_1_T  g_pic_adj_main_1;
        ISP_NVRAM_COLOR_G_PIC_ADJ_MAIN_2_T  g_pic_adj_main_2;
        ISP_NVRAM_COLOR_POS_MAIN_T  pos_main;
        ISP_NVRAM_COLOR_INK_DATA_MAIN_T  ink_data_main;
        ISP_NVRAM_COLOR_INK_DATA_MAIN_CR_T  ink_data_main_cr;
        ISP_NVRAM_COLOR_CAP_IN_DATA_MAIN_T  cap_in_data_main;
        ISP_NVRAM_COLOR_CAP_IN_DATA_MAIN_CR_T  cap_in_data_main_cr;
        ISP_NVRAM_COLOR_CAP_OUT_DATA_MAIN_T  cap_out_data_main;
        ISP_NVRAM_COLOR_CAP_OUT_DATA_MAIN_CR_T  cap_out_data_main_cr;
        ISP_NVRAM_COLOR_Y_SLOPE_1_0_MAIN_T  y_slope_1_0_main;
        ISP_NVRAM_COLOR_Y_SLOPE_3_2_MAIN_T  y_slope_3_2_main;
        ISP_NVRAM_COLOR_Y_SLOPE_5_4_MAIN_T  y_slope_5_4_main;
        ISP_NVRAM_COLOR_Y_SLOPE_7_6_MAIN_T  y_slope_7_6_main;
        ISP_NVRAM_COLOR_Y_SLOPE_9_8_MAIN_T  y_slope_9_8_main;
        ISP_NVRAM_COLOR_Y_SLOPE_11_10_MAIN_T  y_slope_11_10_main;
        ISP_NVRAM_COLOR_Y_SLOPE_13_12_MAIN_T  y_slope_13_12_main;
        ISP_NVRAM_COLOR_Y_SLOPE_15_14_MAIN_T  y_slope_15_14_main;
        ISP_NVRAM_COLOR_LOCAL_HUE_CD_0_T  local_hue_cd_0;
        ISP_NVRAM_COLOR_LOCAL_HUE_CD_1_T  local_hue_cd_1;
        ISP_NVRAM_COLOR_LOCAL_HUE_CD_2_T  local_hue_cd_2;
        ISP_NVRAM_COLOR_LOCAL_HUE_CD_3_T  local_hue_cd_3;
        ISP_NVRAM_COLOR_LOCAL_HUE_CD_4_T  local_hue_cd_4;
        ISP_NVRAM_COLOR_TWO_D_WINDOW_1_T  two_d_window_1;
        ISP_NVRAM_COLOR_TWO_D_W1_RESULT_T  two_d_w1_result;
        ISP_NVRAM_COLOR_SAT_HIST_X_CFG_MAIN_T  sat_hist_x_cfg_main;
        ISP_NVRAM_COLOR_SAT_HIST_Y_CFG_MAIN_T  sat_hist_y_cfg_main;
        ISP_NVRAM_COLOR_BWS_2_T  bws_2;
        ISP_NVRAM_COLOR_CRC_0_T  crc_0;
        ISP_NVRAM_COLOR_CRC_1_T  crc_1;
        ISP_NVRAM_COLOR_CRC_2_T  crc_2;
        ISP_NVRAM_COLOR_CRC_3_T  crc_3;
        ISP_NVRAM_COLOR_CRC_4_T  crc_4;
        ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN1_0_T  partial_sat_gain1_0;
        ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN1_1_T  partial_sat_gain1_1;
        ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN1_2_T  partial_sat_gain1_2;
        ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN1_3_T  partial_sat_gain1_3;
        ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN1_4_T  partial_sat_gain1_4;
        ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN2_0_T  partial_sat_gain2_0;
        ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN2_1_T  partial_sat_gain2_1;
        ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN2_2_T  partial_sat_gain2_2;
        ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN2_3_T  partial_sat_gain2_3;
        ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN2_4_T  partial_sat_gain2_4;
        ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN3_0_T  partial_sat_gain3_0;
        ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN3_1_T  partial_sat_gain3_1;
        ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN3_2_T  partial_sat_gain3_2;
        ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN3_3_T  partial_sat_gain3_3;
        ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN3_4_T  partial_sat_gain3_4;
        ISP_NVRAM_COLOR_PARTIAL_SAT_POINT1_0_T  partial_sat_point1_0;
        ISP_NVRAM_COLOR_PARTIAL_SAT_POINT1_1_T  partial_sat_point1_1;
        ISP_NVRAM_COLOR_PARTIAL_SAT_POINT1_2_T  partial_sat_point1_2;
        ISP_NVRAM_COLOR_PARTIAL_SAT_POINT1_3_T  partial_sat_point1_3;
        ISP_NVRAM_COLOR_PARTIAL_SAT_POINT1_4_T  partial_sat_point1_4;
        ISP_NVRAM_COLOR_PARTIAL_SAT_POINT2_0_T  partial_sat_point2_0;
        ISP_NVRAM_COLOR_PARTIAL_SAT_POINT2_1_T  partial_sat_point2_1;
        ISP_NVRAM_COLOR_PARTIAL_SAT_POINT2_2_T  partial_sat_point2_2;
        ISP_NVRAM_COLOR_PARTIAL_SAT_POINT2_3_T  partial_sat_point2_3;
        ISP_NVRAM_COLOR_PARTIAL_SAT_POINT2_4_T  partial_sat_point2_4;
        ISP_NVRAM_COLOR_START_T  start;
        ISP_NVRAM_COLOR_INTEN_T  inten;
        ISP_NVRAM_COLOR_INTSTA_T  intsta;
        ISP_NVRAM_COLOR_OUT_SEL_T  out_sel;
        ISP_NVRAM_COLOR_FRAME_DONE_DEL_T  frame_done_del;
        ISP_NVRAM_COLOR_CRC_T  crc;
        ISP_NVRAM_COLOR_SW_SCRATCH_T  sw_scratch;
        ISP_NVRAM_COLOR_CK_ON_T  ck_on;
        ISP_NVRAM_COLOR_INTERNAL_IP_WIDTH_T  internal_ip_width;
        ISP_NVRAM_COLOR_INTERNAL_IP_HEIGHT_T  internal_ip_height;
        ISP_NVRAM_COLOR_CM1_EN_T  cm1_en;
        ISP_NVRAM_COLOR_CM2_EN_T  cm2_en;
        ISP_NVRAM_COLOR_SHADOW_CTRL_T  shadow_ctrl;
        ISP_NVRAM_COLOR_R0_CRC_T  r0_crc;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y0_0_T  s_gain_by_y0_0;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y0_1_T  s_gain_by_y0_1;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y0_2_T  s_gain_by_y0_2;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y0_3_T  s_gain_by_y0_3;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y0_4_T  s_gain_by_y0_4;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y64_0_T  s_gain_by_y64_0;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y64_1_T  s_gain_by_y64_1;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y64_2_T  s_gain_by_y64_2;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y64_3_T  s_gain_by_y64_3;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y64_4_T  s_gain_by_y64_4;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y128_0_T  s_gain_by_y128_0;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y128_1_T  s_gain_by_y128_1;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y128_2_T  s_gain_by_y128_2;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y128_3_T  s_gain_by_y128_3;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y128_4_T  s_gain_by_y128_4;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y192_0_T  s_gain_by_y192_0;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y192_1_T  s_gain_by_y192_1;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y192_2_T  s_gain_by_y192_2;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y192_3_T  s_gain_by_y192_3;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y192_4_T  s_gain_by_y192_4;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y256_0_T  s_gain_by_y256_0;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y256_1_T  s_gain_by_y256_1;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y256_2_T  s_gain_by_y256_2;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y256_3_T  s_gain_by_y256_3;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y256_4_T  s_gain_by_y256_4;
        ISP_NVRAM_COLOR_LSP_1_T  lsp_1;
        ISP_NVRAM_COLOR_LSP_2_T  lsp_2;
        ISP_NVRAM_COLOR_CM_CONTROL_T  cm_control;
        ISP_NVRAM_COLOR_CM_W1_HUE_0_T  cm_w1_hue_0;
        ISP_NVRAM_COLOR_CM_W1_HUE_1_T  cm_w1_hue_1;
        ISP_NVRAM_COLOR_CM_W1_HUE_2_T  cm_w1_hue_2;
        ISP_NVRAM_COLOR_CM_W1_HUE_3_T  cm_w1_hue_3;
        ISP_NVRAM_COLOR_CM_W1_HUE_4_T  cm_w1_hue_4;
        ISP_NVRAM_COLOR_CM_W1_LUMA_0_T  cm_w1_luma_0;
        ISP_NVRAM_COLOR_CM_W1_LUMA_1_T  cm_w1_luma_1;
        ISP_NVRAM_COLOR_CM_W1_LUMA_2_T  cm_w1_luma_2;
        ISP_NVRAM_COLOR_CM_W1_LUMA_3_T  cm_w1_luma_3;
        ISP_NVRAM_COLOR_CM_W1_LUMA_4_T  cm_w1_luma_4;
        ISP_NVRAM_COLOR_CM_W1_SAT_0_T  cm_w1_sat_0;
        ISP_NVRAM_COLOR_CM_W1_SAT_1_T  cm_w1_sat_1;
        ISP_NVRAM_COLOR_CM_W1_SAT_2_T  cm_w1_sat_2;
        ISP_NVRAM_COLOR_CM_W1_SAT_3_T  cm_w1_sat_3;
        ISP_NVRAM_COLOR_CM_W1_SAT_4_T  cm_w1_sat_4;
        ISP_NVRAM_COLOR_CM_W2_HUE_0_T  cm_w2_hue_0;
        ISP_NVRAM_COLOR_CM_W2_HUE_1_T  cm_w2_hue_1;
        ISP_NVRAM_COLOR_CM_W2_HUE_2_T  cm_w2_hue_2;
        ISP_NVRAM_COLOR_CM_W2_HUE_3_T  cm_w2_hue_3;
        ISP_NVRAM_COLOR_CM_W2_HUE_4_T  cm_w2_hue_4;
        ISP_NVRAM_COLOR_CM_W2_LUMA_0_T  cm_w2_luma_0;
        ISP_NVRAM_COLOR_CM_W2_LUMA_1_T  cm_w2_luma_1;
        ISP_NVRAM_COLOR_CM_W2_LUMA_2_T  cm_w2_luma_2;
        ISP_NVRAM_COLOR_CM_W2_LUMA_3_T  cm_w2_luma_3;
        ISP_NVRAM_COLOR_CM_W2_LUMA_4_T  cm_w2_luma_4;
        ISP_NVRAM_COLOR_CM_W2_SAT_0_T  cm_w2_sat_0;
        ISP_NVRAM_COLOR_CM_W2_SAT_1_T  cm_w2_sat_1;
        ISP_NVRAM_COLOR_CM_W2_SAT_2_T  cm_w2_sat_2;
        ISP_NVRAM_COLOR_CM_W2_SAT_3_T  cm_w2_sat_3;
        ISP_NVRAM_COLOR_CM_W2_SAT_4_T  cm_w2_sat_4;
        ISP_NVRAM_COLOR_CM_W3_HUE_0_T  cm_w3_hue_0;
        ISP_NVRAM_COLOR_CM_W3_HUE_1_T  cm_w3_hue_1;
        ISP_NVRAM_COLOR_CM_W3_HUE_2_T  cm_w3_hue_2;
        ISP_NVRAM_COLOR_CM_W3_HUE_3_T  cm_w3_hue_3;
        ISP_NVRAM_COLOR_CM_W3_HUE_4_T  cm_w3_hue_4;
        ISP_NVRAM_COLOR_CM_W3_LUMA_0_T  cm_w3_luma_0;
        ISP_NVRAM_COLOR_CM_W3_LUMA_1_T  cm_w3_luma_1;
        ISP_NVRAM_COLOR_CM_W3_LUMA_2_T  cm_w3_luma_2;
        ISP_NVRAM_COLOR_CM_W3_LUMA_3_T  cm_w3_luma_3;
        ISP_NVRAM_COLOR_CM_W3_LUMA_4_T  cm_w3_luma_4;
        ISP_NVRAM_COLOR_CM_W3_SAT_0_T  cm_w3_sat_0;
        ISP_NVRAM_COLOR_CM_W3_SAT_1_T  cm_w3_sat_1;
        ISP_NVRAM_COLOR_CM_W3_SAT_2_T  cm_w3_sat_2;
        ISP_NVRAM_COLOR_CM_W3_SAT_3_T  cm_w3_sat_3;
        ISP_NVRAM_COLOR_CM_W3_SAT_4_T  cm_w3_sat_4;
    };
} ISP_NVRAM_COLOR_GET_T;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ABF
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct {
    FIELD  NBC2_ABF_EN                           :  1;		/*  0.. 0, 0x00000001 */
    FIELD  rsv_1                                 :  7;		/*  1.. 7, 0x000000FE */
    FIELD  NBC2_ABF_BIL_IDX                      :  2;		/*  8.. 9, 0x00000300 */
    FIELD  NBC2_ABF_NSR_IDX                      :  2;		/* 10..11, 0x00000C00 */
    FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
} ISP_DIP_X_NBC2_ABF_CON1_T; //	/* 0x150268C0 */

typedef union {
    typedef ISP_DIP_X_NBC2_ABF_CON1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC2_ABF_CON1_T;

typedef struct {
    FIELD  NBC2_ABF_BF_U_OFST                    :  6;		/*  0.. 5, 0x0000003F */
    FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
} ISP_DIP_X_NBC2_ABF_CON2_T; //	/* 0x150268C4 */

typedef union {
    typedef ISP_DIP_X_NBC2_ABF_CON2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC2_ABF_CON2_T;

typedef struct {
    FIELD  NBC2_ABF_R1                           :  9;		/*  0.. 8, 0x000001FF */
    FIELD  rsv_9                                 :  7;		/*  9..15, 0x0000FE00 */
    FIELD  NBC2_ABF_R2                           :  9;		/* 16..24, 0x01FF0000 */
    FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
} ISP_DIP_X_NBC2_ABF_RCON_T; //	/* 0x150268C8 */

typedef union {
    typedef ISP_DIP_X_NBC2_ABF_RCON_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC2_ABF_RCON_T;

typedef struct {
    FIELD  NBC2_ABF_Y0                           :  8;		/*  0.. 7, 0x000000FF */
    FIELD  NBC2_ABF_Y1                           :  8;		/*  8..15, 0x0000FF00 */
    FIELD  NBC2_ABF_Y2                           :  8;		/* 16..23, 0x00FF0000 */
    FIELD  NBC2_ABF_Y3                           :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_NBC2_ABF_YLUT_T; //	/* 0x150268CC */

typedef union {
    typedef ISP_DIP_X_NBC2_ABF_YLUT_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC2_ABF_YLUT_T;

typedef struct {
    FIELD  NBC2_ABF_CX0                          :  8;		/*  0.. 7, 0x000000FF */
    FIELD  NBC2_ABF_CX1                          :  8;		/*  8..15, 0x0000FF00 */
    FIELD  NBC2_ABF_CX2                          :  8;		/* 16..23, 0x00FF0000 */
    FIELD  NBC2_ABF_CX3                          :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_NBC2_ABF_CXLUT_T; //	/* 0x150268D0 */

typedef union {
    typedef ISP_DIP_X_NBC2_ABF_CXLUT_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC2_ABF_CXLUT_T;

typedef struct {
    FIELD  NBC2_ABF_CY0                          :  8;		/*  0.. 7, 0x000000FF */
    FIELD  NBC2_ABF_CY1                          :  8;		/*  8..15, 0x0000FF00 */
    FIELD  NBC2_ABF_CY2                          :  8;		/* 16..23, 0x00FF0000 */
    FIELD  NBC2_ABF_CY3                          :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_NBC2_ABF_CYLUT_T; //	/* 0x150268D4 */

typedef union {
    typedef ISP_DIP_X_NBC2_ABF_CYLUT_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC2_ABF_CYLUT_T;

typedef struct {
    FIELD  NBC2_ABF_Y_SP0                        : 10;		/*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
    FIELD  NBC2_ABF_Y_SP1                        : 10;		/* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
} ISP_DIP_X_NBC2_ABF_YSP_T; //	/* 0x150268D8 */

typedef union {
    typedef ISP_DIP_X_NBC2_ABF_YSP_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC2_ABF_YSP_T;

typedef struct {
    FIELD  NBC2_ABF_CX_SP0                       : 10;		/*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
    FIELD  NBC2_ABF_CX_SP1                       : 10;		/* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
} ISP_DIP_X_NBC2_ABF_CXSP_T; //	/* 0x150268DC */

typedef union {
    typedef ISP_DIP_X_NBC2_ABF_CXSP_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC2_ABF_CXSP_T;

typedef struct {
    FIELD  NBC2_ABF_CY_SP0                       : 10;		/*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
    FIELD  NBC2_ABF_CY_SP1                       : 10;		/* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
} ISP_DIP_X_NBC2_ABF_CYSP_T; //	/* 0x150268E0 */

typedef union {
    typedef ISP_DIP_X_NBC2_ABF_CYSP_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC2_ABF_CYSP_T;

typedef struct {
    FIELD  NBC2_ABF_STHRE_R                      :  8;		/*  0.. 7, 0x000000FF */
    FIELD  NBC2_ABF_STHRE_G                      :  8;		/*  8..15, 0x0000FF00 */
    FIELD  NBC2_ABF_STHRE_B                      :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
} ISP_DIP_X_NBC2_ABF_CLP_T; //	/* 0x150268E4 */

typedef union {
    typedef ISP_DIP_X_NBC2_ABF_CLP_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC2_ABF_CLP_T;

typedef struct {
    FIELD  NBC2_ABF_RSV                          : 32;		/*  0..31, 0xFFFFFFFF */
} ISP_DIP_X_NBC2_ABF_RSV1_T; //	/* 0x150268EC */

typedef union {
    typedef ISP_DIP_X_NBC2_ABF_RSV1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NBC2_ABF_RSV1_T;

typedef union {
    enum { COUNT = 10 };
    struct {
        ISP_NVRAM_NBC2_ABF_CON1_T  con1;
        ISP_NVRAM_NBC2_ABF_CON2_T  con2;
        ISP_NVRAM_NBC2_ABF_RCON_T  rcon;
        ISP_NVRAM_NBC2_ABF_YLUT_T  ylut;
        ISP_NVRAM_NBC2_ABF_CXLUT_T  cxlut;
        ISP_NVRAM_NBC2_ABF_CYLUT_T  cylut;
        ISP_NVRAM_NBC2_ABF_YSP_T  ysp;
        ISP_NVRAM_NBC2_ABF_CXSP_T  cxsp;
        ISP_NVRAM_NBC2_ABF_CYSP_T  cysp;
        ISP_NVRAM_NBC2_ABF_CLP_T  clp;
        //ISP_NVRAM_NBC2_ABF_RSV1_T  rsv1;
    };
    MUINT32 set[COUNT];
} ISP_NVRAM_ABF_T;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CAC
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct {
    FIELD  CAC_TILE_WIDTH                        : 16;		/*  0..15, 0x0000FFFF */
    FIELD  CAC_TILE_HEIGHT                       : 16;		/* 16..31, 0xFFFF0000 */
} ISP_CAM_CAC_TILE_SIZE_T; //	/* 0x1A004AF0 */

typedef union {
    typedef ISP_CAM_CAC_TILE_SIZE_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CAC_TILE_SIZE_T;

typedef struct {
    FIELD  CAC_OFFSET_X                          : 16;		/*  0..15, 0x0000FFFF */
    FIELD  CAC_OFFSET_Y                          : 16;		/* 16..31, 0xFFFF0000 */
} ISP_CAM_CAC_TILE_OFFSET_T; //	/* 0x1A004AF4 */

typedef union {
    typedef ISP_CAM_CAC_TILE_OFFSET_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CAC_TILE_OFFSET_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// RRZ
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct {
    FIELD  RRZ_HORI_EN                           :  1;		/*  0.. 0, 0x00000001 */
    FIELD  RRZ_VERT_EN                           :  1;		/*  1.. 1, 0x00000002 */
    FIELD  RRZ_OUTPUT_WAIT_EN                    :  1;		/*  2.. 2, 0x00000004 */
    FIELD  rsv_3                                 : 13;		/*  3..15, 0x0000FFF8 */
    FIELD  RRZ_HORI_TBL_SEL                      :  6;		/* 16..21, 0x003F0000 */
    FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
    FIELD  RRZ_VERT_TBL_SEL                      :  6;		/* 24..29, 0x3F000000 */
    FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
} ISP_CAM_RRZ_CTL_T; //	/* 0x1A0046E0 */

typedef union {
    typedef ISP_CAM_RRZ_CTL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_RRZ_CTL_T;

typedef struct {
    FIELD  RRZ_IN_WD                             : 16;		/*  0..15, 0x0000FFFF */
    FIELD  RRZ_IN_HT                             : 16;		/* 16..31, 0xFFFF0000 */
} ISP_CAM_RRZ_IN_IMG_T; //	/* 0x1A0046E4 */

typedef union {
    typedef ISP_CAM_RRZ_IN_IMG_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_RRZ_IN_IMG_T;

typedef struct {
    FIELD  RRZ_OUT_WD                            : 16;		/*  0..15, 0x0000FFFF */
    FIELD  RRZ_OUT_HT                            : 16;		/* 16..31, 0xFFFF0000 */
} ISP_CAM_RRZ_OUT_IMG_T; //	/* 0x1A0046E8 */

typedef union {
    typedef ISP_CAM_RRZ_OUT_IMG_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_RRZ_OUT_IMG_T;

typedef struct {
    FIELD  RRZ_HORI_STEP                         : 18;		/*  0..17, 0x0003FFFF */
    FIELD  rsv_18                                : 14;		/* 18..31, 0xFFFC0000 */
} ISP_CAM_RRZ_HORI_STEP_T; //	/* 0x1A0046EC */

typedef union {
    typedef ISP_CAM_RRZ_HORI_STEP_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_RRZ_HORI_STEP_T;

typedef struct {
    FIELD  RRZ_VERT_STEP                         : 18;		/*  0..17, 0x0003FFFF */
    FIELD  rsv_18                                : 14;		/* 18..31, 0xFFFC0000 */
} ISP_CAM_RRZ_VERT_STEP_T; //	/* 0x1A0046F0 */

typedef union {
    typedef ISP_CAM_RRZ_VERT_STEP_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_RRZ_VERT_STEP_T;

typedef struct {
    FIELD  RRZ_HORI_INT_OFST                     : 16;		/*  0..15, 0x0000FFFF */
    FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
} ISP_CAM_RRZ_HORI_INT_OFST_T; //	/* 0x1A0046F4 */

typedef union {
    typedef ISP_CAM_RRZ_HORI_INT_OFST_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_RRZ_HORI_INT_OFST_T;

typedef struct {
    FIELD  RRZ_HORI_SUB_OFST                     : 15;		/*  0..14, 0x00007FFF */
    FIELD  rsv_15                                : 17;		/* 15..31, 0xFFFF8000 */
} ISP_CAM_RRZ_HORI_SUB_OFST_T; //	/* 0x1A0046F8 */

typedef union {
    typedef ISP_CAM_RRZ_HORI_SUB_OFST_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_RRZ_HORI_SUB_OFST_T;

typedef struct {
    FIELD  RRZ_VERT_INT_OFST                     : 16;		/*  0..15, 0x0000FFFF */
    FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
} ISP_CAM_RRZ_VERT_INT_OFST_T; //	/* 0x1A0046FC */

typedef union {
    typedef ISP_CAM_RRZ_VERT_INT_OFST_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_RRZ_VERT_INT_OFST_T;

typedef struct {
    FIELD  RRZ_VERT_SUB_OFST                     : 15;		/*  0..14, 0x00007FFF */
    FIELD  rsv_15                                : 17;		/* 15..31, 0xFFFF8000 */
} ISP_CAM_RRZ_VERT_SUB_OFST_T; //	/* 0x1A004700 */

typedef union {
    typedef ISP_CAM_RRZ_VERT_SUB_OFST_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_RRZ_VERT_SUB_OFST_T;

typedef struct {
    FIELD  RRZ_TH_MD                             :  8;		/*  0.. 7, 0x000000FF */
    FIELD  RRZ_TH_HI                             :  8;		/*  8..15, 0x0000FF00 */
    FIELD  RRZ_TH_LO                             :  8;		/* 16..23, 0x00FF0000 */
    FIELD  RRZ_TH_MD2                            :  8;		/* 24..31, 0xFF000000 */
} ISP_CAM_RRZ_MODE_TH_T; //	/* 0x1A004704 */

typedef union {
    typedef ISP_CAM_RRZ_MODE_TH_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_RRZ_MODE_TH_T;

typedef struct {
    FIELD  RRZ_PRF_BLD                           :  9;		/*  0.. 8, 0x000001FF */
    FIELD  RRZ_PRF                               :  2;		/*  9..10, 0x00000600 */
    FIELD  RRZ_BLD_SL                            :  5;		/* 11..15, 0x0000F800 */
    FIELD  RRZ_CR_MODE                           :  1;		/* 16..16, 0x00010000 */
    FIELD  rsv_17                                : 15;		/* 17..31, 0xFFFE0000 */
} ISP_CAM_RRZ_MODE_CTL_T; //	/* 0x1A004708 */

typedef union {
    typedef ISP_CAM_RRZ_MODE_CTL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_RRZ_MODE_CTL_T;

typedef union {
    enum { COUNT = 11 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_RRZ_CTL_T            ctl;
        ISP_NVRAM_RRZ_IN_IMG_T         in_img;
        ISP_NVRAM_RRZ_OUT_IMG_T        out_img;
        ISP_NVRAM_RRZ_HORI_STEP_T      hori_step;
        ISP_NVRAM_RRZ_VERT_STEP_T      vert_step;
        ISP_NVRAM_RRZ_HORI_INT_OFST_T  hori_int_ofst;
        ISP_NVRAM_RRZ_HORI_SUB_OFST_T  hori_sub_ofst;
        ISP_NVRAM_RRZ_VERT_INT_OFST_T  veri_int_ofst;
        ISP_NVRAM_RRZ_VERT_SUB_OFST_T  vert_sub_ofst;
        ISP_NVRAM_RRZ_MODE_TH_T        mode_th;
        ISP_NVRAM_RRZ_MODE_CTL_T       mode_ctl;
    };
} ISP_NVRAM_RRZ_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// interpolation
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct
{
    MUINT32 u4RealISO;
	MUINT32 u4UpperISO;
	MUINT32 u4LowerISO;
	MUINT32 i4ZoomRatio_x100;
	MUINT32 u4UpperZoom;
	MUINT32 u4LowerZoom;
    ISP_NVRAM_UDM_T rUpperIso_UpperZoom;
    ISP_NVRAM_UDM_T rLowerIso_UpperZoom;
    ISP_NVRAM_UDM_T rUpperIso_LowerZoom;
    ISP_NVRAM_UDM_T rLowerIso_LowerZoom;
}ISP_NVRAM_UDM_INT_T;
typedef struct
{
	MUINT32 u4RealISO;
	MUINT32 u4UpperISO;
	MUINT32 u4LowerISO;
	MUINT32 i4ZoomRatio_x100;
	MUINT32 u4UpperZoom;
	MUINT32 u4LowerZoom;
	ISP_NVRAM_EE_T rUpperIso_UpperZoom;
	ISP_NVRAM_EE_T rLowerIso_UpperZoom;
	ISP_NVRAM_EE_T rUpperIso_LowerZoom;
	ISP_NVRAM_EE_T rLowerIso_LowerZoom;
}ISP_NVRAM_EE_INT_T;
typedef struct
{
	MUINT32 u4RealISO;
	MUINT32 u4UpperISO;
	MUINT32 u4LowerISO;
	MUINT32 i4ZoomRatio_x100;
	MUINT32 u4UpperZoom;
	MUINT32 u4LowerZoom;
	ISP_NVRAM_ANR2_T rUpperIso_UpperZoom;
	ISP_NVRAM_ANR2_T rLowerIso_UpperZoom;
	ISP_NVRAM_ANR2_T rUpperIso_LowerZoom;
	ISP_NVRAM_ANR2_T rLowerIso_LowerZoom;
}ISP_NVRAM_ANR2_INT_T;
typedef struct
{
	MUINT32 u4RealISO;
	MUINT32 u4UpperISO;
	MUINT32 u4LowerISO;
	MUINT32 i4ZoomRatio_x100;
	MUINT32 u4UpperZoom;
	MUINT32 u4LowerZoom;
	ISP_NVRAM_ANR_T rUpperIso_UpperZoom;
	ISP_NVRAM_ANR_T rLowerIso_UpperZoom;
	ISP_NVRAM_ANR_T rUpperIso_LowerZoom;
	ISP_NVRAM_ANR_T rLowerIso_LowerZoom;
}ISP_NVRAM_ANR_INT_T;
typedef struct
{
	MUINT32 u4RealISO;
	MUINT32 u4UpperISO;
	MUINT32 u4LowerISO;
	MUINT32 i4ZoomRatio_x100;
	MUINT32 u4UpperZoom;
	MUINT32 u4LowerZoom;
	ISP_NVRAM_HFG_T rUpperIso_UpperZoom;
	ISP_NVRAM_HFG_T rLowerIso_UpperZoom;
	ISP_NVRAM_HFG_T rUpperIso_LowerZoom;
	ISP_NVRAM_HFG_T rLowerIso_LowerZoom;
}ISP_NVRAM_HFG_INT_T;

typedef struct
{
	MUINT32 u4RealISO;
	MUINT32 u4UpperISO;
	MUINT32 u4LowerISO;
	MUINT32 i4ZoomRatio_x100;
	MUINT32 u4UpperZoom;
	MUINT32 u4LowerZoom;
	ISP_NVRAM_MIXER3_T rUpperIso_UpperZoom;
	ISP_NVRAM_MIXER3_T rLowerIso_UpperZoom;
	ISP_NVRAM_MIXER3_T rUpperIso_LowerZoom;
	ISP_NVRAM_MIXER3_T rLowerIso_LowerZoom;
}ISP_NVRAM_MIXER3_INT_T;


typedef struct
{
	MUINT32 u4RealISO;
	MUINT32 u4UpperISO;
	MUINT32 u4LowerISO;
	MUINT32 i4ZoomRatio_x100;
	MUINT32 u4UpperZoom;
	MUINT32 u4LowerZoom;
	ISP_NVRAM_MFB_T rUpperIso_UpperZoom;
	ISP_NVRAM_MFB_T rLowerIso_UpperZoom;
	ISP_NVRAM_MFB_T rUpperIso_LowerZoom;
	ISP_NVRAM_MFB_T rLowerIso_LowerZoom;
}ISP_NVRAM_MFB_INT_T;



typedef struct
{
	MUINT32 u4RealISO;
	MUINT32 u4UpperISO;
	MUINT32 u4LowerISO;
	MUINT32 i4ZoomRatio_x100;
	MUINT32 u4UpperZoom;
	MUINT32 u4LowerZoom;
	ISP_NVRAM_NR3D_T rUpperIso_UpperZoom;
	ISP_NVRAM_NR3D_T rLowerIso_UpperZoom;
	ISP_NVRAM_NR3D_T rUpperIso_LowerZoom;
	ISP_NVRAM_NR3D_T rLowerIso_LowerZoom;
}ISP_NVRAM_NR3D_INT_T;

};

#endif  //  _ISPIF_


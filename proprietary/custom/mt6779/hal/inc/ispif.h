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
// ISP Enable (Pass1)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef struct {
        FIELD  CAMCTL_TG_R1_EN                                           :  1;      /*  0.. 0, 0x00000001 */
        FIELD  CAMCTL_SEP_R1_EN                                          :  1;      /*  1.. 1, 0x00000002 */
        FIELD  CAMCTL_BPC_R1_EN                                          :  1;      /*  2.. 2, 0x00000004 */
        FIELD  CAMCTL_OBC_R1_EN                                          :  1;      /*  3.. 3, 0x00000008 */
        FIELD  CAMCTL_FUS_R1_EN                                          :  1;      /*  4.. 4, 0x00000010 */
        FIELD  CAMCTL_DGN_R1_EN                                          :  1;      /*  5.. 5, 0x00000020 */
        FIELD  CAMCTL_LSC_R1_EN                                          :  1;      /*  6.. 6, 0x00000040 */
        FIELD  CAMCTL_WB_R1_EN                                           :  1;      /*  7.. 7, 0x00000080 */
        FIELD  CAMCTL_LTM_R1_EN                                          :  1;      /*  8.. 8, 0x00000100 */
        FIELD  CAMCTL_MRG_R7_EN                                          :  1;      /*  9.. 9, 0x00000200 */
        FIELD  CAMCTL_PAKG_R1_EN                                         :  1;      /* 10..10, 0x00000400 */
        FIELD  CAMCTL_MRG_R5_EN                                          :  1;      /* 11..11, 0x00000800 */
        FIELD  CAMCTL_LCES_R1_EN                                         :  1;      /* 12..12, 0x00001000 */
        FIELD  CAMCTL_QBN_R4_EN                                          :  1;      /* 13..13, 0x00002000 */
        FIELD  CAMCTL_MRG_R4_EN                                          :  1;      /* 14..14, 0x00004000 */
        FIELD  CAMCTL_LTMS_R1_EN                                         :  1;      /* 15..15, 0x00008000 */
        FIELD  CAMCTL_MRG_R1_EN                                          :  1;      /* 16..16, 0x00010000 */
        FIELD  CAMCTL_AA_R1_EN                                           :  1;      /* 17..17, 0x00020000 */
        FIELD  CAMCTL_FLK_R1_EN                                          :  1;      /* 18..18, 0x00040000 */
        FIELD  CAMCTL_CRP_R3_EN                                          :  1;      /* 19..19, 0x00080000 */
        FIELD  CAMCTL_PAK_R1_EN                                          :  1;      /* 20..20, 0x00100000 */
        FIELD  CAMCTL_UNP_R2_EN                                          :  1;      /* 21..21, 0x00200000 */
        FIELD  CAMCTL_MRG_R10_EN                                         :  1;      /* 22..22, 0x00400000 */
        FIELD  CAMCTL_BS_R1_EN                                           :  1;      /* 23..23, 0x00800000 */
        FIELD  CAMCTL_BS_R2_EN                                           :  1;      /* 24..24, 0x01000000 */
        FIELD  CAMCTL_BS_R3_EN                                           :  1;      /* 25..25, 0x02000000 */
        FIELD  CAMCTL_MOBC_R2_EN                                         :  1;      /* 26..26, 0x04000000 */
        FIELD  CAMCTL_MOBC_R3_EN                                         :  1;      /* 27..27, 0x08000000 */
        FIELD  CAMCTL_SLK_R1_EN                                          :  1;      /* 28..28, 0x10000000 */
        FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_CAMCTL_EN_T;

typedef union {
    typedef ISP_CAMCTL_EN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CAMCTL_EN_T;

typedef struct {
        FIELD  CAMCTL_DBN_R1_EN                                          :  1;      /*  0.. 0, 0x00000001 */
        FIELD  CAMCTL_BIN_R1_EN                                          :  1;      /*  1.. 1, 0x00000002 */
        FIELD  CAMCTL_FBND_R1_EN                                         :  1;      /*  2.. 2, 0x00000004 */
        FIELD  CAMCTL_FCIRC_R1_EN                                        :  1;      /*  3.. 3, 0x00000008 */
        FIELD  CAMCTL_BPC_R2_EN                                          :  1;      /*  4.. 4, 0x00000010 */
        FIELD  CAMCTL_OBC_R2_EN                                          :  1;      /*  5.. 5, 0x00000020 */
        FIELD  CAMCTL_ZFUS_R1_EN                                         :  1;      /*  6.. 6, 0x00000040 */
        FIELD  CAMCTL_HLR_R1_EN                                          :  1;      /*  7.. 7, 0x00000080 */
        FIELD  CAMCTL_RRZ_R1_EN                                          :  1;      /*  8.. 8, 0x00000100 */
        FIELD  CAMCTL_UFG_R1_EN                                          :  1;      /*  9.. 9, 0x00000200 */
        FIELD  CAMCTL_QBN_R5_EN                                          :  1;      /* 10..10, 0x00000400 */
        FIELD  CAMCTL_GSE_R1_EN                                          :  1;      /* 11..11, 0x00000800 */
        FIELD  CAMCTL_MRG_R8_EN                                          :  1;      /* 12..12, 0x00001000 */
        FIELD  CAMCTL_HDS_R1_EN                                          :  1;      /* 13..13, 0x00002000 */
        FIELD  CAMCTL_RSS_R1_EN                                          :  1;      /* 14..14, 0x00004000 */
        FIELD  CAMCTL_LMV_R1_EN                                          :  1;      /* 15..15, 0x00008000 */
        FIELD  CAMCTL_CRP_R1_EN                                          :  1;      /* 16..16, 0x00010000 */
        FIELD  CAMCTL_AF_R1_EN                                           :  1;      /* 17..17, 0x00020000 */
        FIELD  CAMCTL_QBN_R1_EN                                          :  1;      /* 18..18, 0x00040000 */
        FIELD  CAMCTL_MRG_R2_EN                                          :  1;      /* 19..19, 0x00080000 */
        FIELD  CAMCTL_QBN_R2_EN                                          :  1;      /* 20..20, 0x00100000 */
        FIELD  CAMCTL_TSFS_R1_EN                                         :  1;      /* 21..21, 0x00200000 */
        FIELD  CAMCTL_LHS_R1_EN                                          :  1;      /* 22..22, 0x00400000 */
        FIELD  CAMCTL_PDE_R1_EN                                          :  1;      /* 23..23, 0x00800000 */
        FIELD  CAMCTL_CRP_R8_EN                                          :  1;      /* 24..24, 0x01000000 */
        FIELD  CAMCTL_PBN_R1_EN                                          :  1;      /* 25..25, 0x02000000 */
        FIELD  CAMCTL_SLK_R2_EN                                          :  1;      /* 26..26, 0x04000000 */
        FIELD  CAMCTL_DM_R1_EN                                           :  1;      /* 27..27, 0x08000000 */
        FIELD  CAMCTL_CCM_R1_EN                                          :  1;      /* 28..28, 0x10000000 */
        FIELD  CAMCTL_GGM_R1_EN                                          :  1;      /* 29..29, 0x20000000 */
        FIELD  CAMCTL_G2C_R1_EN                                          :  1;      /* 30..30, 0x40000000 */
        FIELD  CAMCTL_C42_R1_EN                                          :  1;      /* 31..31, 0x80000000 */
} ISP_CAMCTL_EN2_T;

typedef union {
    typedef ISP_CAMCTL_EN2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CAMCTL_EN2_T;

typedef struct {
        FIELD  CAMCTL_CRSP_R1_EN                                         :  1;      /*  0.. 0, 0x00000001 */
        FIELD  CAMCTL_PLNW_R1_EN                                         :  1;      /*  1.. 1, 0x00000002 */
        FIELD  CAMCTL_PAK_R3_EN                                          :  1;      /*  2.. 2, 0x00000004 */
        FIELD  CAMCTL_PAK_R4_EN                                          :  1;      /*  3.. 3, 0x00000008 */
        FIELD  CAMCTL_MQE_R1_EN                                          :  1;      /*  4.. 4, 0x00000010 */
        FIELD  CAMCTL_MRG_R9_EN                                          :  1;      /*  5.. 5, 0x00000020 */
        FIELD  CAMCTL_MOBC_R1_EN                                         :  1;      /*  6.. 6, 0x00000040 */
        FIELD  CAMCTL_MLSC_R1_EN                                         :  1;      /*  7.. 7, 0x00000080 */
        FIELD  CAMCTL_PAK_R2_EN                                          :  1;      /*  8.. 8, 0x00000100 */
        FIELD  CAMCTL_MAE_R1_EN                                          :  1;      /*  9.. 9, 0x00000200 */
        FIELD  CAMCTL_UFE_R1_EN                                          :  1;      /* 10..10, 0x00000400 */
        FIELD  CAMCTL_MRG_R6_EN                                          :  1;      /* 11..11, 0x00000800 */
        FIELD  CAMCTL_QBN_R3_EN                                          :  1;      /* 12..12, 0x00001000 */
        FIELD  CAMCTL_YNRS_R1_EN                                         :  1;      /* 13..13, 0x00002000 */
        FIELD  CAMCTL_FLC_R1_EN                                          :  1;      /* 14..14, 0x00004000 */
        FIELD  CAMCTL_CRZ_R1_EN                                          :  1;      /* 15..15, 0x00008000 */
        FIELD  CAMCTL_PLNW_R2_EN                                         :  1;      /* 16..16, 0x00010000 */
        FIELD  CAMCTL_MRG_R11_EN                                         :  1;      /* 17..17, 0x00020000 */
        FIELD  CAMCTL_MRG_R12_EN                                         :  1;      /* 18..18, 0x00040000 */
        FIELD  CAMCTL_MRG_R13_EN                                         :  1;      /* 19..19, 0x00080000 */
        FIELD  CAMCTL_MRG_R14_EN                                         :  1;      /* 20..20, 0x00100000 */
        FIELD  CAMCTL_CRP_R4_EN                                          :  1;      /* 21..21, 0x00200000 */
        FIELD  CAMCTL_GGM_R2_EN                                          :  1;      /* 22..22, 0x00400000 */
        FIELD  CAMCTL_G2C_R2_EN                                          :  1;      /* 23..23, 0x00800000 */
        FIELD  CAMCTL_C42_R2_EN                                          :  1;      /* 24..24, 0x01000000 */
        FIELD  CAMCTL_RSS_R2_EN                                          :  1;      /* 25..25, 0x02000000 */
        FIELD  CAMCTL_CRZ_R2_EN                                          :  1;      /* 26..26, 0x04000000 */
        FIELD  CAMCTL_PLNW_R3_EN                                         :  1;      /* 27..27, 0x08000000 */
        FIELD  CAMCTL_PAK_R5_EN                                          :  1;      /* 28..28, 0x10000000 */
        FIELD  CAMCTL_UFD_R2_EN                                          :  1;      /* 29..29, 0x20000000 */
        FIELD  CAMCTL_CRSP_R2_EN                                         :  1;      /* 30..30, 0x40000000 */
        FIELD  rsv_31                                                    :  1;      /* 31..31, 0x80000000 */
} ISP_CAMCTL_EN3_T;

typedef union {
    typedef ISP_CAMCTL_EN3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CAMCTL_EN3_T;

typedef union {
    enum { COUNT = 3 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_CAMCTL_EN_T   en;
        ISP_NVRAM_CAMCTL_EN2_T  en2;
        ISP_NVRAM_CAMCTL_EN3_T  en3;
    };
} ISP_NVRAM_CAMCTL_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ISP Enable (Pass2)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef struct {
		FIELD  DIPCTL_UNP_D1_EN                           :  1;		/*  0.. 0, 0x00000001 */
		FIELD  DIPCTL_UFD_D1_EN                           :  1;		/*  1.. 1, 0x00000002 */
		FIELD  DIPCTL_SMT_D1_EN                           :  1;		/*  2.. 2, 0x00000004 */
		FIELD  DIPCTL_PAKG_D1_EN                          :  1;		/*  3.. 3, 0x00000008 */
		FIELD  DIPCTL_BPC_D1_EN                           :  1;		/*  4.. 4, 0x00000010 */
		FIELD  DIPCTL_OBC_D1_EN                           :  1;		/*  5.. 5, 0x00000020 */
		FIELD  DIPCTL_ZFUS_D1_EN                          :  1;		/*  6.. 6, 0x00000040 */
		FIELD  DIPCTL_DGN_D1_EN                           :  1;		/*  7.. 7, 0x00000080 */
		FIELD  DIPCTL_LSC_D1_EN                           :  1;		/*  8.. 8, 0x00000100 */
		FIELD  DIPCTL_WB_D1_EN                            :  1;		/*  9.. 9, 0x00000200 */
		FIELD  DIPCTL_HLR_D1_EN                           :  1;		/* 10..10, 0x00000400 */
		FIELD  DIPCTL_LTM_D1_EN                           :  1;		/* 11..11, 0x00000800 */
		FIELD  DIPCTL_DM_D1_EN                            :  1;		/* 12..12, 0x00001000 */
		FIELD  DIPCTL_LDNR_D1_EN                          :  1;		/* 13..13, 0x00002000 */
		FIELD  DIPCTL_CRP_D2_EN                           :  1;		/* 14..14, 0x00004000 */
		FIELD  DIPCTL_PAK_D1_EN                           :  1;		/* 15..15, 0x00008000 */
		FIELD  DIPCTL_WIF_D1_EN                           :  1;		/* 16..16, 0x00010000 */
		FIELD  DIPCTL_WIF_D2_EN                           :  1;		/* 17..17, 0x00020000 */
		FIELD  DIPCTL_GDR_D1_EN                           :  1;		/* 18..18, 0x00040000 */
		FIELD  DIPCTL_ALIGN_D1_EN                         :  1;		/* 19..19, 0x00080000 */
		FIELD  DIPCTL_SLK_D1_EN                           :  1;		/* 20..20, 0x00100000 */
		FIELD  DIPCTL_SLK_D6_EN                           :  1;		/* 21..21, 0x00200000 */
		FIELD  DIPCTL_WIF_D3_EN                           :  1;		/* 22..22, 0x00400000 */
		FIELD  rsv_23                                     :  9;		/* 23..31, 0xFF800000 */
} ISP_DIPCTL_RGB_EN1_T;

typedef union {
    typedef ISP_DIPCTL_RGB_EN1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DIPCTL_RGB_EN1_T;

typedef struct {
		FIELD  DIPCTL_FLC_D1_EN                           :  1;		/*  0.. 0, 0x00000001 */
		FIELD  DIPCTL_CCM_D1_EN                           :  1;		/*  1.. 1, 0x00000002 */
		FIELD  DIPCTL_GGM_D1_EN                           :  1;		/*  2.. 2, 0x00000004 */
		FIELD  DIPCTL_WSYNC_D1_EN                         :  1;		/*  3.. 3, 0x00000008 */
		FIELD  DIPCTL_CCM_D2_EN                           :  1;		/*  4.. 4, 0x00000010 */
		FIELD  DIPCTL_GGM_D2_EN                           :  1;		/*  5.. 5, 0x00000020 */
		FIELD  DIPCTL_SMT_D4_EN                           :  1;		/*  6.. 6, 0x00000040 */
		FIELD  DIPCTL_UNP_D4_EN                           :  1;		/*  7.. 7, 0x00000080 */
		FIELD  DIPCTL_PAK_D4_EN                           :  1;		/*  8.. 8, 0x00000100 */
		FIELD  DIPCTL_MCRP_D2_EN                          :  1;		/*  9.. 9, 0x00000200 */
		FIELD  DIPCTL_GGM_D4_EN                           :  1;		/* 10..10, 0x00000400 */
		FIELD  DIPCTL_G2C_D4_EN                           :  1;		/* 11..11, 0x00000800 */
		FIELD  DIPCTL_C42_D4_EN                           :  1;		/* 12..12, 0x00001000 */
		FIELD  DIPCTL_CRP_D4_EN                           :  1;		/* 13..13, 0x00002000 */
		FIELD  rsv_14                                     : 18;		/* 14..31, 0xFFFFC000 */
} ISP_DIPCTL_RGB_EN2_T;

typedef union {
    typedef ISP_DIPCTL_RGB_EN2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DIPCTL_RGB_EN2_T;

typedef struct {
		FIELD  DIPCTL_G2CX_D1_EN                          :  1;		/*  0.. 0, 0x00000001 */
		FIELD  DIPCTL_C42_D1_EN                           :  1;		/*  1.. 1, 0x00000002 */
		FIELD  DIPCTL_MIX_D3_EN                           :  1;		/*  2.. 2, 0x00000004 */
		FIELD  DIPCTL_YNR_D1_EN                           :  1;		/*  3.. 3, 0x00000008 */
		FIELD  DIPCTL_NDG_D1_EN                           :  1;		/*  4.. 4, 0x00000010 */
		FIELD  DIPCTL_MIX_D1_EN                           :  1;		/*  5.. 5, 0x00000020 */
		FIELD  DIPCTL_C24_D3_EN                           :  1;		/*  6.. 6, 0x00000040 */
		FIELD  DIPCTL_C2G_D1_EN                           :  1;		/*  7.. 7, 0x00000080 */
		FIELD  DIPCTL_IGGM_D1_EN                          :  1;		/*  8.. 8, 0x00000100 */
		FIELD  DIPCTL_CCM_D3_EN                           :  1;		/*  9.. 9, 0x00000200 */
		FIELD  DIPCTL_LCE_D1_EN                           :  1;		/* 10..10, 0x00000400 */
		FIELD  DIPCTL_GGM_D3_EN                           :  1;		/* 11..11, 0x00000800 */
		FIELD  DIPCTL_DCE_D1_EN                           :  1;		/* 12..12, 0x00001000 */
		FIELD  DIPCTL_DCES_D1_EN                          :  1;		/* 13..13, 0x00002000 */
		FIELD  DIPCTL_G2C_D1_EN                           :  1;		/* 14..14, 0x00004000 */
		FIELD  DIPCTL_C42_D2_EN                           :  1;		/* 15..15, 0x00008000 */
		FIELD  DIPCTL_EE_D1_EN                            :  1;		/* 16..16, 0x00010000 */
		FIELD  DIPCTL_SMT_D2_EN                           :  1;		/* 17..17, 0x00020000 */
		FIELD  DIPCTL_UNP_D2_EN                           :  1;		/* 18..18, 0x00040000 */
		FIELD  DIPCTL_PAK_D2_EN                           :  1;		/* 19..19, 0x00080000 */
		FIELD  DIPCTL_CNR_D1_EN                           :  1;		/* 20..20, 0x00100000 */
		FIELD  DIPCTL_NDG_D2_EN                           :  1;		/* 21..21, 0x00200000 */
		FIELD  DIPCTL_SMT_D3_EN                           :  1;		/* 22..22, 0x00400000 */
		FIELD  DIPCTL_UNP_D3_EN                           :  1;		/* 23..23, 0x00800000 */
		FIELD  DIPCTL_PAK_D3_EN                           :  1;		/* 24..24, 0x01000000 */
		FIELD  DIPCTL_COLOR_D1_EN                         :  1;		/* 25..25, 0x02000000 */
		FIELD  DIPCTL_MIX_D2_EN                           :  1;		/* 26..26, 0x04000000 */
		FIELD  DIPCTL_MIX_D4_EN                           :  1;		/* 27..27, 0x08000000 */
		FIELD  DIPCTL_CRP_D1_EN                           :  1;		/* 28..28, 0x10000000 */
		FIELD  rsv_29                                     :  3;		/* 29..31, 0xE0000000 */
} ISP_DIPCTL_YUV_EN1_T;

typedef union {
    typedef ISP_DIPCTL_YUV_EN1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DIPCTL_YUV_EN1_T;

typedef struct {
		FIELD  DIPCTL_NR3D_D1_EN                          :  1;		/*  0.. 0, 0x00000001 */
		FIELD  DIPCTL_CRSP_D1_EN                          :  1;		/*  1.. 1, 0x00000002 */
		FIELD  DIPCTL_PLNW_D1_EN                          :  1;		/*  2.. 2, 0x00000004 */
		FIELD  DIPCTL_PAK_D6_EN                           :  1;		/*  3.. 3, 0x00000008 */
		FIELD  DIPCTL_PAK_D7_EN                           :  1;		/*  4.. 4, 0x00000010 */
		FIELD  DIPCTL_PAK_D8_EN                           :  1;		/*  5.. 5, 0x00000020 */
		FIELD  DIPCTL_C24_D2_EN                           :  1;		/*  6.. 6, 0x00000040 */
		FIELD  DIPCTL_MCRP_D1_EN                          :  1;		/*  7.. 7, 0x00000080 */
		FIELD  DIPCTL_CRZ_D1_EN                           :  1;		/*  8.. 8, 0x00000100 */
		FIELD  DIPCTL_PLNW_D2_EN                          :  1;		/*  9.. 9, 0x00000200 */
		FIELD  DIPCTL_SRZ_D1_EN                           :  1;		/* 10..10, 0x00000400 */
		FIELD  DIPCTL_SRZ_D3_EN                           :  1;		/* 11..11, 0x00000800 */
		FIELD  DIPCTL_SRZ_D4_EN                           :  1;		/* 12..12, 0x00001000 */
		FIELD  DIPCTL_UNP_D6_EN                           :  1;		/* 13..13, 0x00002000 */
		FIELD  DIPCTL_UNP_D7_EN                           :  1;		/* 14..14, 0x00004000 */
		FIELD  DIPCTL_UNP_D8_EN                           :  1;		/* 15..15, 0x00008000 */
		FIELD  DIPCTL_PLNR_D2_EN                          :  1;		/* 16..16, 0x00010000 */
		FIELD  DIPCTL_C02_D2_EN                           :  1;		/* 17..17, 0x00020000 */
		FIELD  DIPCTL_C24_D1_EN                           :  1;		/* 18..18, 0x00040000 */
		FIELD  DIPCTL_UNP_D9_EN                           :  1;		/* 19..19, 0x00080000 */
		FIELD  DIPCTL_UNP_D10_EN                          :  1;		/* 20..20, 0x00100000 */
		FIELD  DIPCTL_UNP_D11_EN                          :  1;		/* 21..21, 0x00200000 */
		FIELD  DIPCTL_PLNR_D1_EN                          :  1;		/* 22..22, 0x00400000 */
		FIELD  DIPCTL_C02_D1_EN                           :  1;		/* 23..23, 0x00800000 */
		FIELD  DIPCTL_SLK_D2_EN                           :  1;		/* 24..24, 0x01000000 */
		FIELD  DIPCTL_SLK_D3_EN                           :  1;		/* 25..25, 0x02000000 */
		FIELD  DIPCTL_SLK_D4_EN                           :  1;		/* 26..26, 0x04000000 */
		FIELD  DIPCTL_SLK_D5_EN                           :  1;		/* 27..27, 0x08000000 */
		FIELD  DIPCTL_FM_D1_EN                            :  1;		/* 28..28, 0x10000000 */
		FIELD  DIPCTL_LPCNR_D1_EN                         :  1;		/* 29..29, 0x20000000 */
		FIELD  DIPCTL_BS_D1_EN                            :  1;		/* 30..30, 0x40000000 */
		FIELD  DIPCTL_DFE_D1_EN                           :  1;		/* 31..31, 0x80000000 */
} ISP_DIPCTL_YUV_EN2_T;

typedef union {
    typedef ISP_DIPCTL_YUV_EN2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DIPCTL_YUV_EN2_T;

typedef struct
{

        FIELD DIPCTL_PIX_ID                               :  2;     /* 0.. 1, 0x00000003 */
        FIELD DIPCTL_FG_MODE                              :  1;     /* 2.. 2, 0x00000004 */
        FIELD rsv_3                                       :  1;     /* 3.. 3, 0x00000008 */
        FIELD DIPCTL_SRAM_MODE                            :  2;     /* 4.. 5, 0x00000030 */
        FIELD rsv_6                                       :  2;     /* 6.. 7, 0x000000C0 */
        FIELD DIPCTL_WUV_MODE                             :  1;     /* 8.. 8, 0x00000100 */
        FIELD rsv_9                                       :  1;     /* 9.. 9, 0x00000200 */
        FIELD DIPCTL_YNR_GMAP_LTM_MODE                    :  1;     /* 10..10, 0x00000400 */
        FIELD rsv_11                                      :  3;     /* 11..13, 0x00003800 */
        FIELD DIPCTL_PAKG_D1_FG_OUT                       :  1;     /* 14..14, 0x00004000 */
        FIELD rsv_15                                      :  1;     /* 15..15, 0x00008000 */
        FIELD DIPCTL_HLR_D1_LKMSB                         :  4;     /* 16..19, 0x000F0000 */
        FIELD rsv_20                                      :  4;     /* 20..23, 0x00F00000 */
        FIELD DIPCTL_IMGI_D1_SOF_GATE                     :  1;     /* 24..24, 0x01000000 */
        FIELD rsv_25                                      :  3;     /* 25..27, 0x0E000000 */
        FIELD DIPCTL_LBIT_MODE0                           :  1;     /* 28..28, 0x10000000 */
        FIELD DIPCTL_LBIT_MODE1                           :  1;     /* 29..29, 0x20000000 */
        FIELD rsv_30                                      :  1;     /* 30..30, 0x40000000 */
        FIELD DIPCTL_APB_CLK_GATE_BYPASS                  :  1;     /* 31..31, 0x80000000 */
}ISP_DIPCTL_MISC_SEL_T;

typedef union {
    typedef ISP_DIPCTL_MISC_SEL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DIPCTL_MISC_SEL_T;

typedef union {
    enum { COUNT = 4 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_DIPCTL_RGB_EN1_T   rgb_en1;
        ISP_NVRAM_DIPCTL_RGB_EN2_T   rgb_en2;
        ISP_NVRAM_DIPCTL_YUV_EN1_T   yuv_en1;
        ISP_NVRAM_DIPCTL_YUV_EN2_T   yuv_en2;
        ISP_NVRAM_DIPCTL_MISC_SEL_T  misc_sel;
    };
} ISP_NVRAM_DIPCTL_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// OBC
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef struct {
    FIELD  OBC_EDGE                                                  :  4;      /*  0.. 3, 0x0000000F */
    FIELD  OBC_LE_INV_CTL                                            :  4;      /*  4.. 7, 0x000000F0 */
    FIELD  rsv_8                                                     : 24;      /*  8..31, 0xFFFFFF00 */
} ISP_OBC_CTL_T; // /* 0x1A0104C0 */

typedef union {
    typedef ISP_OBC_CTL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_OBC_CTL_T;

typedef struct {
    FIELD  OBC_DBS_RATIO                                             :  5;      /*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                                     :  3;      /*  5.. 7, 0x000000E0 */
    FIELD  OBC_POSTTUNE_EN                                           :  1;      /*  8.. 8, 0x00000100 */
    FIELD  rsv_9                                                     : 23;      /*  9..31, 0xFFFFFE00 */
} ISP_OBC_DBS_T; // /* 0x1A0104C4 */

typedef union {
    typedef ISP_OBC_DBS_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_OBC_DBS_T;

typedef struct {
    FIELD  OBC_LUMA_MODE                                             :  1;      /*  0.. 0, 0x00000001 */
    FIELD  OBC_GRAY_MODE                                             :  2;      /*  1.. 2, 0x00000006 */
    FIELD  OBC_NORM_BIT                                              :  5;      /*  3.. 7, 0x000000F8 */
    FIELD  rsv_8                                                     : 24;      /*  8..31, 0xFFFFFF00 */
} ISP_OBC_GRAY_BLD_0_T; // /* 0x1A0104C8 */

typedef union {
    typedef ISP_OBC_GRAY_BLD_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_OBC_GRAY_BLD_0_T;

typedef struct {
    FIELD  OBC_BLD_MXRT                                              :  8;      /*  0.. 7, 0x000000FF */
    FIELD  OBC_BLD_LOW                                               : 12;      /*  8..19, 0x000FFF00 */
    FIELD  OBC_BLD_SLP                                               : 12;      /* 20..31, 0xFFF00000 */
} ISP_OBC_GRAY_BLD_1_T; // /* 0x1A0104CC */

typedef union {
    typedef ISP_OBC_GRAY_BLD_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_OBC_GRAY_BLD_1_T;

typedef struct {
    FIELD  OBC_BLD_MXRT_SE                                           :  8;      /*  0.. 7, 0x000000FF */
    FIELD  OBC_BLD_LOW_SE                                            : 12;      /*  8..19, 0x000FFF00 */
    FIELD  OBC_BLD_SLP_SE                                            : 12;      /* 20..31, 0xFFF00000 */
} ISP_OBC_GRAY_BLD_2_T; // /* 0x1A0104D0 */

typedef union {
    typedef ISP_OBC_GRAY_BLD_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_OBC_GRAY_BLD_2_T;

typedef struct {
    FIELD  OBC_BIAS_R0                                               :  8;      /*  0.. 7, 0x000000FF */
    FIELD  OBC_BIAS_R1                                               :  8;      /*  8..15, 0x0000FF00 */
    FIELD  OBC_BIAS_R2                                               :  8;      /* 16..23, 0x00FF0000 */
    FIELD  OBC_BIAS_R3                                               :  8;      /* 24..31, 0xFF000000 */
} ISP_OBC_BIAS_LUT_R0_T; // /* 0x1A0104D4 */

typedef union {
    typedef ISP_OBC_BIAS_LUT_R0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_OBC_BIAS_LUT_R0_T;

typedef struct {
    FIELD  OBC_BIAS_R4                                               :  8;      /*  0.. 7, 0x000000FF */
    FIELD  OBC_BIAS_R5                                               :  8;      /*  8..15, 0x0000FF00 */
    FIELD  OBC_BIAS_R6                                               :  8;      /* 16..23, 0x00FF0000 */
    FIELD  OBC_BIAS_R7                                               :  8;      /* 24..31, 0xFF000000 */
} ISP_OBC_BIAS_LUT_R1_T; // /* 0x1A0104D8 */

typedef union {
    typedef ISP_OBC_BIAS_LUT_R1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_OBC_BIAS_LUT_R1_T;

typedef struct {
    FIELD  OBC_BIAS_R8                                               :  8;      /*  0.. 7, 0x000000FF */
    FIELD  OBC_BIAS_R9                                               :  8;      /*  8..15, 0x0000FF00 */
    FIELD  OBC_BIAS_R10                                              :  8;      /* 16..23, 0x00FF0000 */
    FIELD  OBC_BIAS_R11                                              :  8;      /* 24..31, 0xFF000000 */
} ISP_OBC_BIAS_LUT_R2_T; // /* 0x1A0104DC */

typedef union {
    typedef ISP_OBC_BIAS_LUT_R2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_OBC_BIAS_LUT_R2_T;

typedef struct {
    FIELD  OBC_BIAS_R12                                              :  8;      /*  0.. 7, 0x000000FF */
    FIELD  OBC_BIAS_R13                                              :  8;      /*  8..15, 0x0000FF00 */
    FIELD  OBC_BIAS_R14                                              :  8;      /* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                                    :  8;      /* 24..31, 0xFF000000 */
} ISP_OBC_BIAS_LUT_R3_T; // /* 0x1A0104E0 */

typedef union {
    typedef ISP_OBC_BIAS_LUT_R3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_OBC_BIAS_LUT_R3_T;

typedef struct {
    FIELD  OBC_BIAS_G0                                               :  8;      /*  0.. 7, 0x000000FF */
    FIELD  OBC_BIAS_G1                                               :  8;      /*  8..15, 0x0000FF00 */
    FIELD  OBC_BIAS_G2                                               :  8;      /* 16..23, 0x00FF0000 */
    FIELD  OBC_BIAS_G3                                               :  8;      /* 24..31, 0xFF000000 */
} ISP_OBC_BIAS_LUT_G0_T; // /* 0x1A0104E4 */

typedef union {
    typedef ISP_OBC_BIAS_LUT_G0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_OBC_BIAS_LUT_G0_T;

typedef struct {
    FIELD  OBC_BIAS_G4                                               :  8;      /*  0.. 7, 0x000000FF */
    FIELD  OBC_BIAS_G5                                               :  8;      /*  8..15, 0x0000FF00 */
    FIELD  OBC_BIAS_G6                                               :  8;      /* 16..23, 0x00FF0000 */
    FIELD  OBC_BIAS_G7                                               :  8;      /* 24..31, 0xFF000000 */
} ISP_OBC_BIAS_LUT_G1_T; // /* 0x1A0104E8 */

typedef union {
    typedef ISP_OBC_BIAS_LUT_G1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_OBC_BIAS_LUT_G1_T;

typedef struct {
    FIELD  OBC_BIAS_G8                                               :  8;      /*  0.. 7, 0x000000FF */
    FIELD  OBC_BIAS_G9                                               :  8;      /*  8..15, 0x0000FF00 */
    FIELD  OBC_BIAS_G10                                              :  8;      /* 16..23, 0x00FF0000 */
    FIELD  OBC_BIAS_G11                                              :  8;      /* 24..31, 0xFF000000 */
} ISP_OBC_BIAS_LUT_G2_T; // /* 0x1A0104EC */

typedef union {
    typedef ISP_OBC_BIAS_LUT_G2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_OBC_BIAS_LUT_G2_T;

typedef struct {
    FIELD  OBC_BIAS_G12                                              :  8;      /*  0.. 7, 0x000000FF */
    FIELD  OBC_BIAS_G13                                              :  8;      /*  8..15, 0x0000FF00 */
    FIELD  OBC_BIAS_G14                                              :  8;      /* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                                    :  8;      /* 24..31, 0xFF000000 */
} ISP_OBC_BIAS_LUT_G3_T; // /* 0x1A0104F0 */

typedef union {
    typedef ISP_OBC_BIAS_LUT_G3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_OBC_BIAS_LUT_G3_T;

typedef struct {
    FIELD  OBC_BIAS_B0                                               :  8;      /*  0.. 7, 0x000000FF */
    FIELD  OBC_BIAS_B1                                               :  8;      /*  8..15, 0x0000FF00 */
    FIELD  OBC_BIAS_B2                                               :  8;      /* 16..23, 0x00FF0000 */
    FIELD  OBC_BIAS_B3                                               :  8;      /* 24..31, 0xFF000000 */
} ISP_OBC_BIAS_LUT_B0_T; // /* 0x1A0104F4 */

typedef union {
    typedef ISP_OBC_BIAS_LUT_B0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_OBC_BIAS_LUT_B0_T;

typedef struct {
    FIELD  OBC_BIAS_B4                                               :  8;      /*  0.. 7, 0x000000FF */
    FIELD  OBC_BIAS_B5                                               :  8;      /*  8..15, 0x0000FF00 */
    FIELD  OBC_BIAS_B6                                               :  8;      /* 16..23, 0x00FF0000 */
    FIELD  OBC_BIAS_B7                                               :  8;      /* 24..31, 0xFF000000 */
} ISP_OBC_BIAS_LUT_B1_T; // /* 0x1A0104F8 */

typedef union {
    typedef ISP_OBC_BIAS_LUT_B1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_OBC_BIAS_LUT_B1_T;

typedef struct {
    FIELD  OBC_BIAS_B8                                               :  8;      /*  0.. 7, 0x000000FF */
    FIELD  OBC_BIAS_B9                                               :  8;      /*  8..15, 0x0000FF00 */
    FIELD  OBC_BIAS_B10                                              :  8;      /* 16..23, 0x00FF0000 */
    FIELD  OBC_BIAS_B11                                              :  8;      /* 24..31, 0xFF000000 */
} ISP_OBC_BIAS_LUT_B2_T; // /* 0x1A0104FC */

typedef union {
    typedef ISP_OBC_BIAS_LUT_B2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_OBC_BIAS_LUT_B2_T;

typedef struct {
    FIELD  OBC_BIAS_B12                                              :  8;      /*  0.. 7, 0x000000FF */
    FIELD  OBC_BIAS_B13                                              :  8;      /*  8..15, 0x0000FF00 */
    FIELD  OBC_BIAS_B14                                              :  8;      /* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                                    :  8;      /* 24..31, 0xFF000000 */
} ISP_OBC_BIAS_LUT_B3_T; // /* 0x1A010500 */

typedef union {
    typedef ISP_OBC_BIAS_LUT_B3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_OBC_BIAS_LUT_B3_T;

typedef struct {
    FIELD  OBC_PGN_R                                                 : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  OBC_PGN_B                                                 : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_OBC_WBG_RB_T; // /* 0x1A010504 */

typedef union {
    typedef ISP_OBC_WBG_RB_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_OBC_WBG_RB_T;

typedef struct {
    FIELD  OBC_PGN_G                                                 : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    : 19;      /* 13..31, 0xFFFFE000 */
} ISP_OBC_WBG_G_T; // /* 0x1A010508 */

typedef union {
    typedef ISP_OBC_WBG_G_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_OBC_WBG_G_T;

typedef struct {
    FIELD  OBC_IVGN_R                                                : 10;      /*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                                    :  6;      /* 10..15, 0x0000FC00 */
    FIELD  OBC_IVGN_B                                                : 10;      /* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                                    :  6;      /* 26..31, 0xFC000000 */
} ISP_OBC_WBIG_RB_T; // /* 0x1A01050C */

typedef union {
    typedef ISP_OBC_WBIG_RB_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_OBC_WBIG_RB_T;

typedef struct {
    FIELD  OBC_IVGN_G                                                : 10;      /*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                                    : 22;      /* 10..31, 0xFFFFFC00 */
} ISP_OBC_WBIG_G_T; // /* 0x1A010510 */

typedef union {
    typedef ISP_OBC_WBIG_G_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_OBC_WBIG_G_T;

typedef struct {
    FIELD  OBC_GAIN_R                                                : 12;      /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                                    :  4;      /* 12..15, 0x0000F000 */
    FIELD  OBC_GAIN_B                                                : 12;      /* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                                    :  4;      /* 28..31, 0xF0000000 */
} ISP_OBC_OBG_RB_T; // /* 0x1A010514 */

typedef union {
    typedef ISP_OBC_OBG_RB_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_OBC_OBG_RB_T;

typedef struct {
    FIELD  OBC_GAIN_GR                                               : 12;      /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                                    :  4;      /* 12..15, 0x0000F000 */
    FIELD  OBC_GAIN_GB                                               : 12;      /* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                                    :  4;      /* 28..31, 0xF0000000 */
} ISP_OBC_OBG_G_T; // /* 0x1A010518 */

typedef union {
    typedef ISP_OBC_OBG_G_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_OBC_OBG_G_T;

typedef struct {
    FIELD  OBC_OFST_R                                                : 22;      /*  0..21, 0x003FFFFF */
    FIELD  rsv_22                                                    : 10;      /* 22..31, 0xFFC00000 */
} ISP_OBC_OFFSET_R_T; // /* 0x1A01051C */

typedef union {
    typedef ISP_OBC_OFFSET_R_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_OBC_OFFSET_R_T;

typedef struct {
    FIELD  OBC_OFST_GR                                               : 22;      /*  0..21, 0x003FFFFF */
    FIELD  rsv_22                                                    : 10;      /* 22..31, 0xFFC00000 */
} ISP_OBC_OFFSET_GR_T; // /* 0x1A010520 */

typedef union {
    typedef ISP_OBC_OFFSET_GR_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_OBC_OFFSET_GR_T;

typedef struct {
    FIELD  OBC_OFST_GB                                               : 22;      /*  0..21, 0x003FFFFF */
    FIELD  rsv_22                                                    : 10;      /* 22..31, 0xFFC00000 */
} ISP_OBC_OFFSET_GB_T; // /* 0x1A010524 */

typedef union {
    typedef ISP_OBC_OFFSET_GB_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_OBC_OFFSET_GB_T;

typedef struct {
    FIELD  OBC_OFST_B                                                : 22;      /*  0..21, 0x003FFFFF */
    FIELD  rsv_22                                                    : 10;      /* 22..31, 0xFFC00000 */
} ISP_OBC_OFFSET_B_T; // /* 0x1A010528 */

typedef union {
    typedef ISP_OBC_OFFSET_B_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_OBC_OFFSET_B_T;

typedef struct {
    FIELD  OBC_HDR_GNP                                               :  3;      /*  0.. 2, 0x00000007 */
    FIELD  rsv_3                                                     :  1;      /*  3.. 3, 0x00000008 */
    FIELD  OBC_ZHDR_EN                                               :  1;      /*  4.. 4, 0x00000010 */
    FIELD  OBC_GLE_FIRST                                             :  1;      /*  5.. 5, 0x00000020 */
    FIELD  OBC_RLE_FIRST                                             :  1;      /*  6.. 6, 0x00000040 */
    FIELD  OBC_BLE_FIRST                                             :  1;      /*  7.. 7, 0x00000080 */
    FIELD  rsv_8                                                     : 24;      /*  8..31, 0xFFFFFF00 */
} ISP_OBC_HDR_T; // /* 0x1A01052C */

typedef union {
    typedef ISP_OBC_HDR_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_OBC_HDR_T;

typedef union {
    enum { COUNT = 19 };
    MUINT32 set[COUNT];
    struct {
        //ISP_NVRAM_OBC_CTL_T  			        ctl;
        ISP_NVRAM_OBC_DBS_T  			        dbs;
        ISP_NVRAM_OBC_GRAY_BLD_0_T  			gray_bld_0;
        ISP_NVRAM_OBC_GRAY_BLD_1_T  			gray_bld_1;
        //ISP_NVRAM_OBC_GRAY_BLD_2_T  			gray_bld_2;
        ISP_NVRAM_OBC_BIAS_LUT_R0_T  			bias_lut_r0;
        ISP_NVRAM_OBC_BIAS_LUT_R1_T  			bias_lut_r1;
        ISP_NVRAM_OBC_BIAS_LUT_R2_T  			bias_lut_r2;
        ISP_NVRAM_OBC_BIAS_LUT_R3_T  			bias_lut_r3;
        ISP_NVRAM_OBC_BIAS_LUT_G0_T  			bias_lut_g0;
        ISP_NVRAM_OBC_BIAS_LUT_G1_T  			bias_lut_g1;
        ISP_NVRAM_OBC_BIAS_LUT_G2_T  			bias_lut_g2;
        ISP_NVRAM_OBC_BIAS_LUT_G3_T  			bias_lut_g3;
        ISP_NVRAM_OBC_BIAS_LUT_B0_T  			bias_lut_b0;
        ISP_NVRAM_OBC_BIAS_LUT_B1_T  			bias_lut_b1;
        ISP_NVRAM_OBC_BIAS_LUT_B2_T  			bias_lut_b2;
        ISP_NVRAM_OBC_BIAS_LUT_B3_T  			bias_lut_b3;
        //ISP_NVRAM_OBC_WBG_RB_T  		    	wbg_rb;
        //ISP_NVRAM_OBC_WBG_G_T  			    wbg_g;
        //ISP_NVRAM_OBC_WBIG_RB_T  		    	wbig_rb;
        //ISP_NVRAM_OBC_WBIG_G_T  		    	wbig_g;
        //ISP_NVRAM_OBC_OBG_RB_T  		    	obg_rb;
        //ISP_NVRAM_OBC_OBG_G_T  		        obg_g;
        ISP_NVRAM_OBC_OFFSET_R_T  		    	offset_r;
        ISP_NVRAM_OBC_OFFSET_GR_T  		        offset_gr;
        ISP_NVRAM_OBC_OFFSET_GB_T  		    	offset_gb;
        ISP_NVRAM_OBC_OFFSET_B_T  		    	offset_b;
        //ISP_NVRAM_OBC_HDR_T  			        hdr;
    };
} ISP_NVRAM_OBC_T;




//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BPC_BPC
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct {
    FIELD  BPC_BPC_EN                                                :  1;      /*  0.. 0, 0x00000001 */
    FIELD  rsv_1                                                     :  3;      /*  1.. 3, 0x0000000E */
    FIELD  BPC_BPC_LUT_EN                                            :  1;      /*  4.. 4, 0x00000010 */
    FIELD  BPC_BPC_TABLE_END_MODE                                    :  1;      /*  5.. 5, 0x00000020 */
    FIELD  rsv_6                                                     :  2;      /*  6.. 7, 0x000000C0 */
    FIELD  BPC_BPC_AVG_MODE                                          :  1;      /*  8.. 8, 0x00000100 */
    FIELD  rsv_9                                                     :  3;      /*  9..11, 0x00000E00 */
    FIELD  BPC_BPC_DTC_MODE                                          :  2;      /* 12..13, 0x00003000 */
    FIELD  BPC_BPC_CS_MODE                                           :  2;      /* 14..15, 0x0000C000 */
    FIELD  BPC_BPC_CRC_MODE                                          :  2;      /* 16..17, 0x00030000 */
    FIELD  BPC_BPC_EXC                                               :  1;      /* 18..18, 0x00040000 */
    FIELD  BPC_BPC_BLD_MODE                                          :  1;      /* 19..19, 0x00080000 */
    FIELD  BPC_LE_INV_CTL                                            :  4;      /* 20..23, 0x00F00000 */
    FIELD  BPC_BPC_AUX_MAP_EN                                        :  1;      /* 24..24, 0x01000000 */
    FIELD  BPC_BPC_AUX_MAP_OR_EN                                     :  1;      /* 25..25, 0x02000000 */
    FIELD  rsv_26                                                    :  2;      /* 26..27, 0x0C000000 */
    FIELD  BPC_BNR_EDGE                                              :  4;      /* 28..31, 0xF0000000 */
} ISP_BPC_BPC_CON_T; // /* 0x1A0103C0 */

typedef union {
    typedef ISP_BPC_BPC_CON_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BPC_BPC_CON_T;

typedef struct {
    FIELD  BPC_BPC_BLD0                                              :  7;      /*  0.. 6, 0x0000007F */
    FIELD  rsv_7                                                     :  1;      /*  7.. 7, 0x00000080 */
    FIELD  BPC_BPC_BLD1                                              :  7;      /*  8..14, 0x00007F00 */
    FIELD  rsv_15                                                    :  1;      /* 15..15, 0x00008000 */
    FIELD  BPC_BPC_BLD_SP                                            :  8;      /* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                                    :  8;      /* 24..31, 0xFF000000 */
} ISP_BPC_BPC_BLD_T; // /* 0x1A0103C4 */

typedef union {
    typedef ISP_BPC_BPC_BLD_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BPC_BPC_BLD_T;

typedef struct {
    FIELD  BPC_BPC_TH_LWB                                            : 15;      /*  0..14, 0x00007FFF */
    FIELD  rsv_15                                                    :  1;      /* 15..15, 0x00008000 */
    FIELD  BPC_BPC_TH_Y                                              : 15;      /* 16..30, 0x7FFF0000 */
    FIELD  rsv_31                                                    :  1;      /* 31..31, 0x80000000 */
} ISP_BPC_BPC_TH1_T; // /* 0x1A0103C8 */

typedef union {
    typedef ISP_BPC_BPC_TH1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BPC_BPC_TH1_T;

typedef struct {
    FIELD  BPC_BPC_TH_XB                                             : 15;      /*  0..14, 0x00007FFF */
    FIELD  rsv_15                                                    :  1;      /* 15..15, 0x00008000 */
    FIELD  BPC_BPC_TH_UPB                                            : 15;      /* 16..30, 0x7FFF0000 */
    FIELD  rsv_31                                                    :  1;      /* 31..31, 0x80000000 */
} ISP_BPC_BPC_TH2_T; // /* 0x1A0103CC */

typedef union {
    typedef ISP_BPC_BPC_TH2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BPC_BPC_TH2_T;

typedef struct {
    FIELD  BPC_BPC_DK_TH_XB                                          : 15;      /*  0..14, 0x00007FFF */
    FIELD  rsv_15                                                    :  1;      /* 15..15, 0x00008000 */
    FIELD  BPC_BPC_TH_XA                                             : 15;      /* 16..30, 0x7FFF0000 */
    FIELD  rsv_31                                                    :  1;      /* 31..31, 0x80000000 */
} ISP_BPC_BPC_TH3_T; // /* 0x1A0103D0 */

typedef union {
    typedef ISP_BPC_BPC_TH3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BPC_BPC_TH3_T;

typedef struct {
    FIELD  BPC_BPC_DK_TH_XA                                          : 15;      /*  0..14, 0x00007FFF */
    FIELD  rsv_15                                                    : 17;      /* 15..31, 0xFFFF8000 */
} ISP_BPC_BPC_TH4_T; // /* 0x1A0103D4 */

typedef union {
    typedef ISP_BPC_BPC_TH4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BPC_BPC_TH4_T;

typedef struct {
    FIELD  BPC_BPC_TH_SLA                                            :  4;      /*  0.. 3, 0x0000000F */
    FIELD  BPC_BPC_TH_SLB                                            :  4;      /*  4.. 7, 0x000000F0 */
    FIELD  rsv_8                                                     : 24;      /*  8..31, 0xFFFFFF00 */
} ISP_BPC_BPC_TH5_T; // /* 0x1A0103D8 */

typedef union {
    typedef ISP_BPC_BPC_TH5_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BPC_BPC_TH5_T;

typedef struct {
    FIELD  BPC_BPC_DK_TH_SLA                                         :  4;      /*  0.. 3, 0x0000000F */
    FIELD  BPC_BPC_DK_TH_SLB                                         :  4;      /*  4.. 7, 0x000000F0 */
    FIELD  rsv_8                                                     : 24;      /*  8..31, 0xFFFFFF00 */
} ISP_BPC_BPC_TH6_T; // /* 0x1A0103DC */

typedef union {
    typedef ISP_BPC_BPC_TH6_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BPC_BPC_TH6_T;

typedef struct {
    FIELD  BPC_BPC_RNG                                               :  4;      /*  0.. 3, 0x0000000F */
    FIELD  BPC_BPC_CS_RNG                                            :  3;      /*  4.. 6, 0x00000070 */
    FIELD  rsv_7                                                     :  1;      /*  7.. 7, 0x00000080 */
    FIELD  BPC_BPC_CT_LV                                             :  4;      /*  8..11, 0x00000F00 */
    FIELD  rsv_12                                                    :  4;      /* 12..15, 0x0000F000 */
    FIELD  BPC_BPC_TH_MUL                                            :  4;      /* 16..19, 0x000F0000 */
    FIELD  rsv_20                                                    :  4;      /* 20..23, 0x00F00000 */
    FIELD  BPC_BPC_NO_LV                                             :  3;      /* 24..26, 0x07000000 */
    FIELD  rsv_27                                                    :  5;      /* 27..31, 0xF8000000 */
} ISP_BPC_BPC_DTC_T; // /* 0x1A0103E0 */

typedef union {
    typedef ISP_BPC_BPC_DTC_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BPC_BPC_DTC_T;

typedef struct {
    FIELD  BPC_BPC_DIR_MAX                                           :  8;      /*  0.. 7, 0x000000FF */
    FIELD  BPC_BPC_DIR_TH                                            :  8;      /*  8..15, 0x0000FF00 */
    FIELD  rsv_16                                                    :  8;      /* 16..23, 0x00FF0000 */
    FIELD  BPC_BPC_DIR_TH2                                           :  8;      /* 24..31, 0xFF000000 */
} ISP_BPC_BPC_COR_T; // /* 0x1A0103E4 */

typedef union {
    typedef ISP_BPC_BPC_COR_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BPC_BPC_COR_T;

typedef struct {
    FIELD  BPC_BPC_RANK_IDXB                                         :  3;      /*  0.. 2, 0x00000007 */
    FIELD  rsv_3                                                     :  1;      /*  3.. 3, 0x00000008 */
    FIELD  BPC_BPC_RANK_IDXR                                         :  3;      /*  4.. 6, 0x00000070 */
    FIELD  rsv_7                                                     :  1;      /*  7.. 7, 0x00000080 */
    FIELD  BPC_BPC_RANK_IDXG                                         :  3;      /*  8..10, 0x00000700 */
    FIELD  rsv_11                                                    :  1;      /* 11..11, 0x00000800 */
    FIELD  BPC_BPC_BLD_LWB                                           : 12;      /* 12..23, 0x00FFF000 */
    FIELD  rsv_24                                                    :  8;      /* 24..31, 0xFF000000 */
} ISP_BPC_BPC_RANK_T; // /* 0x1A0103E8 */

typedef union {
    typedef ISP_BPC_BPC_RANK_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BPC_BPC_RANK_T;

typedef struct {
    FIELD  BPC_XOFFSET                                               : 14;      /*  0..13, 0x00003FFF */
    FIELD  rsv_14                                                    :  2;      /* 14..15, 0x0000C000 */
    FIELD  BPC_YOFFSET                                               : 14;      /* 16..29, 0x3FFF0000 */
    FIELD  rsv_30                                                    :  2;      /* 30..31, 0xC0000000 */
} ISP_BPC_BPC_TBLI1_T; // /* 0x1A0103EC */

typedef union {
    typedef ISP_BPC_BPC_TBLI1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BPC_BPC_TBLI1_T;

typedef struct {
    FIELD  BPC_XSIZE                                                 : 14;      /*  0..13, 0x00003FFF */
    FIELD  rsv_14                                                    :  2;      /* 14..15, 0x0000C000 */
    FIELD  BPC_YSIZE                                                 : 14;      /* 16..29, 0x3FFF0000 */
    FIELD  rsv_30                                                    :  2;      /* 30..31, 0xC0000000 */
} ISP_BPC_BPC_TBLI2_T; // /* 0x1A0103F0 */

typedef union {
    typedef ISP_BPC_BPC_TBLI2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BPC_BPC_TBLI2_T;

typedef struct {
    FIELD  BPC_BPC_C_TH_LWB                                          : 15;      /*  0..14, 0x00007FFF */
    FIELD  rsv_15                                                    :  1;      /* 15..15, 0x00008000 */
    FIELD  BPC_BPC_C_TH_Y                                            : 15;      /* 16..30, 0x7FFF0000 */
    FIELD  rsv_31                                                    :  1;      /* 31..31, 0x80000000 */
} ISP_BPC_BPC_TH1_C_T; // /* 0x1A0103F4 */

typedef union {
    typedef ISP_BPC_BPC_TH1_C_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BPC_BPC_TH1_C_T;

typedef struct {
    FIELD  BPC_BPC_C_TH_XA                                           : 15;      /*  0..14, 0x00007FFF */
    FIELD  rsv_15                                                    :  1;      /* 15..15, 0x00008000 */
    FIELD  BPC_BPC_C_TH_XB                                           : 15;      /* 16..30, 0x7FFF0000 */
    FIELD  rsv_31                                                    :  1;      /* 31..31, 0x80000000 */
} ISP_BPC_BPC_TH2_C_T; // /* 0x1A0103F8 */

typedef union {
    typedef ISP_BPC_BPC_TH2_C_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BPC_BPC_TH2_C_T;

typedef struct {
    FIELD  BPC_BPC_C_TH_SLA                                          :  4;      /*  0.. 3, 0x0000000F */
    FIELD  BPC_BPC_C_TH_SLB                                          :  4;      /*  4.. 7, 0x000000F0 */
    FIELD  BPC_BPC_C_TH_UPB                                          : 15;      /*  8..22, 0x007FFF00 */
    FIELD  rsv_23                                                    :  9;      /* 23..31, 0xFF800000 */
} ISP_BPC_BPC_TH3_C_T; // /* 0x1A0103FC */

typedef union {
    typedef ISP_BPC_BPC_TH3_C_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BPC_BPC_TH3_C_T;

typedef struct {
    FIELD  BPC_BPC_LL_LWB                                            : 12;      /*  0..11, 0x00000FFF */
    FIELD  BPC_BPC_LL_LSP                                            :  5;      /* 12..16, 0x0001F000 */
    FIELD  rsv_17                                                    : 15;      /* 17..31, 0xFFFE0000 */
} ISP_BPC_BPC_LL_T; // /* 0x1A010400 */

typedef union {
    typedef ISP_BPC_BPC_LL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BPC_BPC_LL_T;

typedef struct {
    FIELD  BPC_ZHDR_EN                                               :  1;      /*  0.. 0, 0x00000001 */
    FIELD  rsv_1                                                     :  3;      /*  1.. 3, 0x0000000E */
    FIELD  BPC_ZHDR_GLE_FIRST                                        :  1;      /*  4.. 4, 0x00000010 */
    FIELD  rsv_5                                                     :  3;      /*  5.. 7, 0x000000E0 */
    FIELD  BPC_ZHDR_RLE_FIRST                                        :  1;      /*  8.. 8, 0x00000100 */
    FIELD  rsv_9                                                     :  3;      /*  9..11, 0x00000E00 */
    FIELD  BPC_ZHDR_BLE_FIRST                                        :  1;      /* 12..12, 0x00001000 */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  BPC_ZHDR_OSC_COUNT                                        :  4;      /* 16..19, 0x000F0000 */
    FIELD  BPC_ZHDR_OSC_TH                                           : 12;      /* 20..31, 0xFFF00000 */
} ISP_BPC_ZHDR_CON_T; // /* 0x1A01049C */

typedef union {
    typedef ISP_BPC_ZHDR_CON_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BPC_ZHDR_CON_T;

typedef struct {
    FIELD  BPC_ZHDR_RATIO                                            :  8;      /*  0.. 7, 0x000000FF */
    FIELD  rsv_8                                                     :  4;      /*  8..11, 0x00000F00 */
    FIELD  BPC_ZHDR_GAIN                                             :  9;      /* 12..20, 0x001FF000 */
    FIELD  rsv_21                                                    : 11;      /* 21..31, 0xFFE00000 */
} ISP_BPC_ZHDR_RMG_T; // /* 0x1A0104A0 */

typedef union {
    typedef ISP_BPC_ZHDR_RMG_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BPC_ZHDR_RMG_T;

typedef struct {
    FIELD  BPC_ZHDR_CLAMP_EN                                         :  1;      /*  0.. 0, 0x00000001 */
    FIELD  rsv_1                                                     :  7;      /*  1.. 7, 0x000000FE */
    FIELD  BPC_ZHDR_OUT_SEL_EN                                       :  1;      /*  8.. 8, 0x00000100 */
    FIELD  rsv_9                                                     :  7;      /*  9..15, 0x0000FE00 */
    FIELD  BPC_BPC_OC_MEAN_EN                                        :  1;      /* 16..16, 0x00010000 */
    FIELD  rsv_17                                                    :  3;      /* 17..19, 0x000E0000 */
    FIELD  BPC_PDC_GAIN_MODE_EN                                      :  1;      /* 20..20, 0x00100000 */
    FIELD  rsv_21                                                    : 11;      /* 21..31, 0xFFE00000 */
} ISP_BPC_PSEUDO_T; // /* 0x1A0104A4 */

typedef union {
    typedef ISP_BPC_PSEUDO_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BPC_PSEUDO_T;

typedef union {
    enum { COUNT = 17 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_BPC_BPC_CON_T  			con;
        ISP_NVRAM_BPC_BPC_BLD_T  			bld;
        ISP_NVRAM_BPC_BPC_TH1_T  			th1;
        ISP_NVRAM_BPC_BPC_TH2_T  			th2;
        ISP_NVRAM_BPC_BPC_TH3_T  			th3;
        ISP_NVRAM_BPC_BPC_TH4_T  			th4;
        ISP_NVRAM_BPC_BPC_TH5_T  			th5;
        ISP_NVRAM_BPC_BPC_TH6_T  			th6;
        ISP_NVRAM_BPC_BPC_DTC_T  			dtc;
        ISP_NVRAM_BPC_BPC_COR_T  			cor;
        ISP_NVRAM_BPC_BPC_RANK_T  			rank;
        //ISP_NVRAM_BPC_BPC_TBLI1_T  			tbli1;
        //ISP_NVRAM_BPC_BPC_TBLI2_T  			tbli2;
        ISP_NVRAM_BPC_BPC_TH1_C_T  			th1_c;
        ISP_NVRAM_BPC_BPC_TH2_C_T  			th2_c;
        ISP_NVRAM_BPC_BPC_TH3_C_T  			th3_c;
        ISP_NVRAM_BPC_BPC_LL_T  			ll;
        ISP_NVRAM_BPC_ZHDR_CON_T  			zhdr_con;
        //ISP_NVRAM_BPC_ZHDR_RMG_T  			zhdr_rmg;
        ISP_NVRAM_BPC_PSEUDO_T  			pseudo;
    };
} ISP_NVRAM_BPC_BPC_T;



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BPC_CT
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef struct {
    FIELD  rsv_0                                                     :  4;      /*  0.. 3, 0x0000000F */
    FIELD  BPC_CT_EN                                                 :  1;      /*  4.. 4, 0x00000010 */
    FIELD  rsv_5                                                     : 27;      /*  5..31, 0xFFFFFFE0 */
} ISP_BPC_CT_CON1_T; // /* 0x1A01040C */

typedef union {
    typedef ISP_BPC_CT_CON1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BPC_CT_CON1_T;

typedef struct {
    FIELD  BPC_CT_MD                                                 :  2;      /*  0.. 1, 0x00000003 */
    FIELD  BPC_CT_MD2                                                :  2;      /*  2.. 3, 0x0000000C */
    FIELD  BPC_CT_THRD                                               : 10;      /*  4..13, 0x00003FF0 */
    FIELD  rsv_14                                                    :  2;      /* 14..15, 0x0000C000 */
    FIELD  BPC_CT_MBND                                               : 10;      /* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                                    :  2;      /* 26..27, 0x0C000000 */
    FIELD  BPC_CT_SLOPE                                              :  2;      /* 28..29, 0x30000000 */
    FIELD  BPC_CT_DIV                                                :  2;      /* 30..31, 0xC0000000 */
} ISP_BPC_CT_CON2_T; // /* 0x1A010410 */

typedef union {
    typedef ISP_BPC_CT_CON2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BPC_CT_CON2_T;

typedef struct {
    FIELD  BPC_CT_UPB                                                : 12;      /*  0..11, 0x00000FFF */
    FIELD  BPC_CT_USP                                                :  5;      /* 12..16, 0x0001F000 */
    FIELD  rsv_17                                                    : 15;      /* 17..31, 0xFFFE0000 */
} ISP_BPC_CT_BLD1_T; // /* 0x1A010414 */

typedef union {
    typedef ISP_BPC_CT_BLD1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BPC_CT_BLD1_T;

typedef struct {
    FIELD  BPC_CT_LWB                                                : 12;      /*  0..11, 0x00000FFF */
    FIELD  BPC_CT_LSP                                                :  5;      /* 12..16, 0x0001F000 */
    FIELD  rsv_17                                                    : 15;      /* 17..31, 0xFFFE0000 */
} ISP_BPC_CT_BLD2_T; // /* 0x1A010418 */

typedef union {
    typedef ISP_BPC_CT_BLD2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BPC_CT_BLD2_T;

typedef struct {
    FIELD  BPC_RCCC_CT_EN                                            :  1;      /*  0.. 0, 0x00000001 */
    FIELD  rsv_1                                                     :  3;      /*  1.. 3, 0x0000000E */
    FIELD  BPC_RCCC_CT_MODE                                          :  1;      /*  4.. 4, 0x00000010 */
    FIELD  rsv_5                                                     :  3;      /*  5.. 7, 0x000000E0 */
    FIELD  BPC_RCCC_CT_COEF                                          :  9;      /*  8..16, 0x0001FF00 */
    FIELD  rsv_17                                                    : 15;      /* 17..31, 0xFFFE0000 */
} ISP_BPC_RCCC_CT_CON_T; // /* 0x1A010498 */

typedef union {
    typedef ISP_BPC_RCCC_CT_CON_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BPC_RCCC_CT_CON_T;

typedef union {
    enum { COUNT = 4 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_BPC_CT_CON1_T  			con1;
        ISP_NVRAM_BPC_CT_CON2_T  			con2;
        ISP_NVRAM_BPC_CT_BLD1_T  			bld1;
        ISP_NVRAM_BPC_CT_BLD2_T  			bld2;
        //ISP_NVRAM_BPC_RCCC_CT_CON_T  	    rccc_ct_con;
    };
} ISP_NVRAM_BPC_CT_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BPC_PDC
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef struct {
    FIELD  BPC_PDC_EN                                                :  1;      /*  0.. 0, 0x00000001 */
    FIELD  rsv_1                                                     :  3;      /*  1.. 3, 0x0000000E */
    FIELD  BPC_PDC_CT                                                :  1;      /*  4.. 4, 0x00000010 */
    FIELD  rsv_5                                                     :  3;      /*  5.. 7, 0x000000E0 */
    FIELD  BPC_PDC_MODE                                              :  2;      /*  8.. 9, 0x00000300 */
    FIELD  rsv_10                                                    :  6;      /* 10..15, 0x0000FC00 */
    FIELD  BPC_PDC_OUT                                               :  1;      /* 16..16, 0x00010000 */
    FIELD  rsv_17                                                    :  3;      /* 17..19, 0x000E0000 */
    FIELD  BPC_PDC_ORI_EN                                            :  1;      /* 20..20, 0x00100000 */
    FIELD  rsv_21                                                    :  3;      /* 21..23, 0x00E00000 */
    FIELD  BPC_PDC_INTP_CC_EN                                        :  1;      /* 24..24, 0x01000000 */
    FIELD  rsv_25                                                    :  7;      /* 25..31, 0xFE000000 */
} ISP_BPC_PDC_CON_T; // /* 0x1A01041C */

typedef union {
    typedef ISP_BPC_PDC_CON_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BPC_PDC_CON_T;

typedef struct {
    FIELD  BPC_PDC_GCF_L00                                           : 12;      /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                                    :  4;      /* 12..15, 0x0000F000 */
    FIELD  BPC_PDC_GCF_L10                                           : 12;      /* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                                    :  4;      /* 28..31, 0xF0000000 */
} ISP_BPC_PDC_GAIN_L0_T; // /* 0x1A010420 */

typedef union {
    typedef ISP_BPC_PDC_GAIN_L0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BPC_PDC_GAIN_L0_T;

typedef struct {
    FIELD  BPC_PDC_GCF_L01                                           : 12;      /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                                    :  4;      /* 12..15, 0x0000F000 */
    FIELD  BPC_PDC_GCF_L20                                           : 12;      /* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                                    :  4;      /* 28..31, 0xF0000000 */
} ISP_BPC_PDC_GAIN_L1_T; // /* 0x1A010424 */

typedef union {
    typedef ISP_BPC_PDC_GAIN_L1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BPC_PDC_GAIN_L1_T;

typedef struct {
    FIELD  BPC_PDC_GCF_L11                                           : 12;      /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                                    :  4;      /* 12..15, 0x0000F000 */
    FIELD  BPC_PDC_GCF_L02                                           : 12;      /* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                                    :  4;      /* 28..31, 0xF0000000 */
} ISP_BPC_PDC_GAIN_L2_T; // /* 0x1A010428 */

typedef union {
    typedef ISP_BPC_PDC_GAIN_L2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BPC_PDC_GAIN_L2_T;

typedef struct {
    FIELD  BPC_PDC_GCF_L30                                           : 12;      /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                                    :  4;      /* 12..15, 0x0000F000 */
    FIELD  BPC_PDC_GCF_L21                                           : 12;      /* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                                    :  4;      /* 28..31, 0xF0000000 */
} ISP_BPC_PDC_GAIN_L3_T; // /* 0x1A01042C */

typedef union {
    typedef ISP_BPC_PDC_GAIN_L3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BPC_PDC_GAIN_L3_T;

typedef struct {
    FIELD  BPC_PDC_GCF_L12                                           : 12;      /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                                    :  4;      /* 12..15, 0x0000F000 */
    FIELD  BPC_PDC_GCF_L03                                           : 12;      /* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                                    :  4;      /* 28..31, 0xF0000000 */
} ISP_BPC_PDC_GAIN_L4_T; // /* 0x1A010430 */

typedef union {
    typedef ISP_BPC_PDC_GAIN_L4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BPC_PDC_GAIN_L4_T;

typedef struct {
    FIELD  BPC_PDC_GCF_R00                                           : 12;      /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                                    :  4;      /* 12..15, 0x0000F000 */
    FIELD  BPC_PDC_GCF_R10                                           : 12;      /* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                                    :  4;      /* 28..31, 0xF0000000 */
} ISP_BPC_PDC_GAIN_R0_T; // /* 0x1A010434 */

typedef union {
    typedef ISP_BPC_PDC_GAIN_R0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BPC_PDC_GAIN_R0_T;

typedef struct {
    FIELD  BPC_PDC_GCF_R01                                           : 12;      /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                                    :  4;      /* 12..15, 0x0000F000 */
    FIELD  BPC_PDC_GCF_R20                                           : 12;      /* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                                    :  4;      /* 28..31, 0xF0000000 */
} ISP_BPC_PDC_GAIN_R1_T; // /* 0x1A010438 */

typedef union {
    typedef ISP_BPC_PDC_GAIN_R1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BPC_PDC_GAIN_R1_T;

typedef struct {
    FIELD  BPC_PDC_GCF_R11                                           : 12;      /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                                    :  4;      /* 12..15, 0x0000F000 */
    FIELD  BPC_PDC_GCF_R02                                           : 12;      /* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                                    :  4;      /* 28..31, 0xF0000000 */
} ISP_BPC_PDC_GAIN_R2_T; // /* 0x1A01043C */

typedef union {
    typedef ISP_BPC_PDC_GAIN_R2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BPC_PDC_GAIN_R2_T;

typedef struct {
    FIELD  BPC_PDC_GCF_R30                                           : 12;      /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                                    :  4;      /* 12..15, 0x0000F000 */
    FIELD  BPC_PDC_GCF_R21                                           : 12;      /* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                                    :  4;      /* 28..31, 0xF0000000 */
} ISP_BPC_PDC_GAIN_R3_T; // /* 0x1A010440 */

typedef union {
    typedef ISP_BPC_PDC_GAIN_R3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BPC_PDC_GAIN_R3_T;

typedef struct {
    FIELD  BPC_PDC_GCF_R12                                           : 12;      /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                                    :  4;      /* 12..15, 0x0000F000 */
    FIELD  BPC_PDC_GCF_R03                                           : 12;      /* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                                    :  4;      /* 28..31, 0xF0000000 */
} ISP_BPC_PDC_GAIN_R4_T; // /* 0x1A010444 */

typedef union {
    typedef ISP_BPC_PDC_GAIN_R4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BPC_PDC_GAIN_R4_T;

typedef struct {
    FIELD  BPC_PDC_GTH                                               : 12;      /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                                    :  4;      /* 12..15, 0x0000F000 */
    FIELD  BPC_PDC_BTH                                               : 12;      /* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                                    :  4;      /* 28..31, 0xF0000000 */
} ISP_BPC_PDC_TH_GB_T; // /* 0x1A010448 */

typedef union {
    typedef ISP_BPC_PDC_TH_GB_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BPC_PDC_TH_GB_T;

typedef struct {
    FIELD  BPC_PDC_ITH                                               : 12;      /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                                    :  4;      /* 12..15, 0x0000F000 */
    FIELD  BPC_PDC_ATH                                               : 12;      /* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                                    :  4;      /* 28..31, 0xF0000000 */
} ISP_BPC_PDC_TH_IA_T; // /* 0x1A01044C */

typedef union {
    typedef ISP_BPC_PDC_TH_IA_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BPC_PDC_TH_IA_T;

typedef struct {
    FIELD  BPC_PDC_NTH                                               : 12;      /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                                    :  4;      /* 12..15, 0x0000F000 */
    FIELD  BPC_PDC_DTH                                               : 12;      /* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                                    :  4;      /* 28..31, 0xF0000000 */
} ISP_BPC_PDC_TH_HD_T; // /* 0x1A010450 */

typedef union {
    typedef ISP_BPC_PDC_TH_HD_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BPC_PDC_TH_HD_T;

typedef struct {
    FIELD  BPC_PDC_GSL                                               :  4;      /*  0.. 3, 0x0000000F */
    FIELD  BPC_PDC_BSL                                               :  4;      /*  4.. 7, 0x000000F0 */
    FIELD  BPC_PDC_ISL                                               :  4;      /*  8..11, 0x00000F00 */
    FIELD  BPC_PDC_ASL                                               :  4;      /* 12..15, 0x0000F000 */
    FIELD  BPC_PDC_GCF_NORM                                          :  4;      /* 16..19, 0x000F0000 */
    FIELD  rsv_20                                                    : 12;      /* 20..31, 0xFFF00000 */
} ISP_BPC_PDC_SL_T; // /* 0x1A010454 */

typedef union {
    typedef ISP_BPC_PDC_SL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BPC_PDC_SL_T;

typedef struct {
    FIELD  BPC_PDC_XCENTER                                           : 14;      /*  0..13, 0x00003FFF */
    FIELD  rsv_14                                                    :  2;      /* 14..15, 0x0000C000 */
    FIELD  BPC_PDC_YCENTER                                           : 14;      /* 16..29, 0x3FFF0000 */
    FIELD  rsv_30                                                    :  2;      /* 30..31, 0xC0000000 */
} ISP_BPC_PDC_POS_T; // /* 0x1A010458 */

typedef union {
    typedef ISP_BPC_PDC_POS_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BPC_PDC_POS_T;

typedef struct {
    FIELD  BPC_PDC_DIFF_TH_GR_Y1                                     : 12;      /*  0..11, 0x00000FFF */
    FIELD  BPC_PDC_DIFF_TH_GR_SLP1                                   :  7;      /* 12..18, 0x0007F000 */
    FIELD  rsv_19                                                    :  1;      /* 19..19, 0x00080000 */
    FIELD  BPC_PDC_BLD_WT_SLP                                        :  5;      /* 20..24, 0x01F00000 */
    FIELD  rsv_25                                                    :  7;      /* 25..31, 0xFE000000 */
} ISP_BPC_PDC_DIFF_TH_GR1_T; // /* 0x1A01045C */

typedef union {
    typedef ISP_BPC_PDC_DIFF_TH_GR1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BPC_PDC_DIFF_TH_GR1_T;

typedef struct {
    FIELD  BPC_PDC_DIFF_TH_GR_X2                                     : 12;      /*  0..11, 0x00000FFF */
    FIELD  BPC_PDC_DIFF_TH_GR_Y2                                     : 12;      /* 12..23, 0x00FFF000 */
    FIELD  BPC_PDC_DIFF_TH_GR_SLP2                                   :  7;      /* 24..30, 0x7F000000 */
    FIELD  rsv_31                                                    :  1;      /* 31..31, 0x80000000 */
} ISP_BPC_PDC_DIFF_TH_GR2_T; // /* 0x1A010460 */

typedef union {
    typedef ISP_BPC_PDC_DIFF_TH_GR2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BPC_PDC_DIFF_TH_GR2_T;

typedef struct {
    FIELD  BPC_PDC_DIFF_TH_GR_X3                                     : 12;      /*  0..11, 0x00000FFF */
    FIELD  BPC_PDC_DIFF_TH_GR_Y3                                     : 12;      /* 12..23, 0x00FFF000 */
    FIELD  BPC_PDC_DIFF_TH_GR_SLP3                                   :  7;      /* 24..30, 0x7F000000 */
    FIELD  rsv_31                                                    :  1;      /* 31..31, 0x80000000 */
} ISP_BPC_PDC_DIFF_TH_GR3_T; // /* 0x1A010464 */

typedef union {
    typedef ISP_BPC_PDC_DIFF_TH_GR3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BPC_PDC_DIFF_TH_GR3_T;

typedef struct {
    FIELD  BPC_PDC_DIFF_TH_GB_Y1                                     : 12;      /*  0..11, 0x00000FFF */
    FIELD  BPC_PDC_DIFF_TH_GB_SLP1                                   :  7;      /* 12..18, 0x0007F000 */
    FIELD  rsv_19                                                    : 13;      /* 19..31, 0xFFF80000 */
} ISP_BPC_PDC_DIFF_TH_GB1_T; // /* 0x1A010468 */

typedef union {
    typedef ISP_BPC_PDC_DIFF_TH_GB1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BPC_PDC_DIFF_TH_GB1_T;

typedef struct {
    FIELD  BPC_PDC_DIFF_TH_GB_X2                                     : 12;      /*  0..11, 0x00000FFF */
    FIELD  BPC_PDC_DIFF_TH_GB_Y2                                     : 12;      /* 12..23, 0x00FFF000 */
    FIELD  BPC_PDC_DIFF_TH_GB_SLP2                                   :  7;      /* 24..30, 0x7F000000 */
    FIELD  rsv_31                                                    :  1;      /* 31..31, 0x80000000 */
} ISP_BPC_PDC_DIFF_TH_GB2_T; // /* 0x1A01046C */

typedef union {
    typedef ISP_BPC_PDC_DIFF_TH_GB2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BPC_PDC_DIFF_TH_GB2_T;

typedef struct {
    FIELD  BPC_PDC_DIFF_TH_GB_X3                                     : 12;      /*  0..11, 0x00000FFF */
    FIELD  BPC_PDC_DIFF_TH_GB_Y3                                     : 12;      /* 12..23, 0x00FFF000 */
    FIELD  BPC_PDC_DIFF_TH_GB_SLP3                                   :  7;      /* 24..30, 0x7F000000 */
    FIELD  rsv_31                                                    :  1;      /* 31..31, 0x80000000 */
} ISP_BPC_PDC_DIFF_TH_GB3_T; // /* 0x1A010470 */

typedef union {
    typedef ISP_BPC_PDC_DIFF_TH_GB3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BPC_PDC_DIFF_TH_GB3_T;

typedef struct {
    FIELD  BPC_PDC_DIFF_TH_R_Y1                                      : 12;      /*  0..11, 0x00000FFF */
    FIELD  BPC_PDC_DIFF_TH_R_SLP1                                    :  7;      /* 12..18, 0x0007F000 */
    FIELD  rsv_19                                                    : 13;      /* 19..31, 0xFFF80000 */
} ISP_BPC_PDC_DIFF_TH_R1_T; // /* 0x1A010474 */

typedef union {
    typedef ISP_BPC_PDC_DIFF_TH_R1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BPC_PDC_DIFF_TH_R1_T;

typedef struct {
    FIELD  BPC_PDC_DIFF_TH_R_X2                                      : 12;      /*  0..11, 0x00000FFF */
    FIELD  BPC_PDC_DIFF_TH_R_Y2                                      : 12;      /* 12..23, 0x00FFF000 */
    FIELD  BPC_PDC_DIFF_TH_R_SLP2                                    :  7;      /* 24..30, 0x7F000000 */
    FIELD  rsv_31                                                    :  1;      /* 31..31, 0x80000000 */
} ISP_BPC_PDC_DIFF_TH_R2_T; // /* 0x1A010478 */

typedef union {
    typedef ISP_BPC_PDC_DIFF_TH_R2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BPC_PDC_DIFF_TH_R2_T;

typedef struct {
    FIELD  BPC_PDC_DIFF_TH_R_X3                                      : 12;      /*  0..11, 0x00000FFF */
    FIELD  BPC_PDC_DIFF_TH_R_Y3                                      : 12;      /* 12..23, 0x00FFF000 */
    FIELD  BPC_PDC_DIFF_TH_R_SLP3                                    :  7;      /* 24..30, 0x7F000000 */
    FIELD  rsv_31                                                    :  1;      /* 31..31, 0x80000000 */
} ISP_BPC_PDC_DIFF_TH_R3_T; // /* 0x1A01047C */

typedef union {
    typedef ISP_BPC_PDC_DIFF_TH_R3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BPC_PDC_DIFF_TH_R3_T;

typedef struct {
    FIELD  BPC_PDC_DIFF_TH_B_Y1                                      : 12;      /*  0..11, 0x00000FFF */
    FIELD  BPC_PDC_DIFF_TH_B_SLP1                                    :  7;      /* 12..18, 0x0007F000 */
    FIELD  rsv_19                                                    : 13;      /* 19..31, 0xFFF80000 */
} ISP_BPC_PDC_DIFF_TH_B1_T; // /* 0x1A010480 */

typedef union {
    typedef ISP_BPC_PDC_DIFF_TH_B1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BPC_PDC_DIFF_TH_B1_T;

typedef struct {
    FIELD  BPC_PDC_DIFF_TH_B_X2                                      : 12;      /*  0..11, 0x00000FFF */
    FIELD  BPC_PDC_DIFF_TH_B_Y2                                      : 12;      /* 12..23, 0x00FFF000 */
    FIELD  BPC_PDC_DIFF_TH_B_SLP2                                    :  7;      /* 24..30, 0x7F000000 */
    FIELD  rsv_31                                                    :  1;      /* 31..31, 0x80000000 */
} ISP_BPC_PDC_DIFF_TH_B2_T; // /* 0x1A010484 */

typedef union {
    typedef ISP_BPC_PDC_DIFF_TH_B2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BPC_PDC_DIFF_TH_B2_T;

typedef struct {
    FIELD  BPC_PDC_DIFF_TH_B_X3                                      : 12;      /*  0..11, 0x00000FFF */
    FIELD  BPC_PDC_DIFF_TH_B_Y3                                      : 12;      /* 12..23, 0x00FFF000 */
    FIELD  BPC_PDC_DIFF_TH_B_SLP3                                    :  7;      /* 24..30, 0x7F000000 */
    FIELD  rsv_31                                                    :  1;      /* 31..31, 0x80000000 */
} ISP_BPC_PDC_DIFF_TH_B3_T; // /* 0x1A010488 */

typedef union {
    typedef ISP_BPC_PDC_DIFF_TH_B3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BPC_PDC_DIFF_TH_B3_T;

typedef struct {
    FIELD  BPC_PDC_GAIN_GB                                           : 12;      /*  0..11, 0x00000FFF */
    FIELD  BPC_PDC_GAIN_GR                                           : 12;      /* 12..23, 0x00FFF000 */
    FIELD  BPC_PDC_GAIN_MODE                                         :  2;      /* 24..25, 0x03000000 */
    FIELD  rsv_26                                                    :  6;      /* 26..31, 0xFC000000 */
} ISP_BPC_PDC_GAIN1_T; // /* 0x1A01048C */

typedef union {
    typedef ISP_BPC_PDC_GAIN1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BPC_PDC_GAIN1_T;

typedef struct {
    FIELD  BPC_PDC_GAIN_B                                            : 12;      /*  0..11, 0x00000FFF */
    FIELD  BPC_PDC_GAIN_R                                            : 12;      /* 12..23, 0x00FFF000 */
    FIELD  rsv_24                                                    :  8;      /* 24..31, 0xFF000000 */
} ISP_BPC_PDC_GAIN2_T; // /* 0x1A010490 */

typedef union {
    typedef ISP_BPC_PDC_GAIN2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BPC_PDC_GAIN2_T;

typedef struct {
    FIELD  BPC_ZHDR_OSC_PD_MODE                                      :  1;      /*  0.. 0, 0x00000001 */
    FIELD  rsv_1                                                     :  3;      /*  1.. 3, 0x0000000E */
    FIELD  BPC_PDC_INTP_PD_MODE                                      :  1;      /*  4.. 4, 0x00000010 */
    FIELD  rsv_5                                                     :  3;      /*  5.. 7, 0x000000E0 */
    FIELD  BPC_BPC_RANK_PD_MODE                                      :  1;      /*  8.. 8, 0x00000100 */
    FIELD  rsv_9                                                     :  3;      /*  9..11, 0x00000E00 */
    FIELD  BPC_BPC_LBL_PD_MODE                                       :  1;      /* 12..12, 0x00001000 */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  BPC_BPC_AVG_PD_MODE                                       :  1;      /* 16..16, 0x00010000 */
    FIELD  rsv_17                                                    : 15;      /* 17..31, 0xFFFE0000 */
} ISP_BPC_PD_MODE_T; // /* 0x1A010494 */

typedef union {
    typedef ISP_BPC_PD_MODE_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BPC_PD_MODE_T;

typedef union {
    enum { COUNT = 22 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_BPC_PDC_CON_T  			    con;
        ISP_NVRAM_BPC_PDC_GAIN_L0_T  			gain_l0;
        //ISP_NVRAM_BPC_PDC_GAIN_L1_T  			gain_l1;
        //ISP_NVRAM_BPC_PDC_GAIN_L2_T  			gain_l2;
        //ISP_NVRAM_BPC_PDC_GAIN_L3_T  			gain_l3;
        //ISP_NVRAM_BPC_PDC_GAIN_L4_T  			gain_l4;
        ISP_NVRAM_BPC_PDC_GAIN_R0_T  			gain_r0;
        //ISP_NVRAM_BPC_PDC_GAIN_R1_T  			gain_r1;
        //ISP_NVRAM_BPC_PDC_GAIN_R2_T  			gain_r2;
        //ISP_NVRAM_BPC_PDC_GAIN_R3_T  			gain_r3;
        //ISP_NVRAM_BPC_PDC_GAIN_R4_T  			gain_r4;
        ISP_NVRAM_BPC_PDC_TH_GB_T  		    	th_gb;
        ISP_NVRAM_BPC_PDC_TH_IA_T  			    th_ia;
        ISP_NVRAM_BPC_PDC_TH_HD_T  			    th_hd;
        ISP_NVRAM_BPC_PDC_SL_T  			    sl;
        //ISP_NVRAM_BPC_PDC_POS_T  			    pos;
        ISP_NVRAM_BPC_PDC_DIFF_TH_GR1_T  		diff_th_gr1;
        ISP_NVRAM_BPC_PDC_DIFF_TH_GR2_T  		diff_th_gr2;
        ISP_NVRAM_BPC_PDC_DIFF_TH_GR3_T  		diff_th_gr3;
        ISP_NVRAM_BPC_PDC_DIFF_TH_GB1_T  		diff_th_gb1;
        ISP_NVRAM_BPC_PDC_DIFF_TH_GB2_T  		diff_th_gb2;
        ISP_NVRAM_BPC_PDC_DIFF_TH_GB3_T  		diff_th_gb3;
        ISP_NVRAM_BPC_PDC_DIFF_TH_R1_T  		diff_th_r1;
        ISP_NVRAM_BPC_PDC_DIFF_TH_R2_T  		diff_th_r2;
        ISP_NVRAM_BPC_PDC_DIFF_TH_R3_T  		diff_th_r3;
        ISP_NVRAM_BPC_PDC_DIFF_TH_B1_T  		diff_th_b1;
        ISP_NVRAM_BPC_PDC_DIFF_TH_B2_T  		diff_th_b2;
        ISP_NVRAM_BPC_PDC_DIFF_TH_B3_T  		diff_th_b3;
        ISP_NVRAM_BPC_PDC_GAIN1_T  			    gain1;
        ISP_NVRAM_BPC_PDC_GAIN2_T  			    gain2;
        ISP_NVRAM_BPC_PD_MODE_T  			    pd_mode;
    };
} ISP_NVRAM_BPC_PDC_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// DGN
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct {
    FIELD  DGN_GAIN_B                                                : 21;      /*  0..20, 0x001FFFFF */
    FIELD  rsv_21                                                    : 11;      /* 21..31, 0xFFE00000 */
} ISP_DGN_GN0_T; // /* 0x1A010580 */

typedef union {
    typedef ISP_DGN_GN0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DGN_GN0_T;

typedef struct {
    FIELD  DGN_GAIN_GB                                               : 21;      /*  0..20, 0x001FFFFF */
    FIELD  rsv_21                                                    : 11;      /* 21..31, 0xFFE00000 */
} ISP_DGN_GN1_T; // /* 0x1A010584 */

typedef union {
    typedef ISP_DGN_GN1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DGN_GN1_T;

typedef struct {
    FIELD  DGN_GAIN_GR                                               : 21;      /*  0..20, 0x001FFFFF */
    FIELD  rsv_21                                                    : 11;      /* 21..31, 0xFFE00000 */
} ISP_DGN_GN2_T; // /* 0x1A010588 */

typedef union {
    typedef ISP_DGN_GN2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DGN_GN2_T;

typedef struct {
    FIELD  DGN_GAIN_R                                                : 21;      /*  0..20, 0x001FFFFF */
    FIELD  rsv_21                                                    : 11;      /* 21..31, 0xFFE00000 */
} ISP_DGN_GN3_T; // /* 0x1A01058C */

typedef union {
    typedef ISP_DGN_GN3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DGN_GN3_T;

typedef struct {
    FIELD  DGN_LKMODE                                                :  1;      /*  0.. 0, 0x00000001 */
    FIELD  rsv_1                                                     : 31;      /*  1..31, 0xFFFFFFFE */
} ISP_DGN_LINK_T; // /* 0x1A010590 */

typedef union {
    typedef ISP_DGN_LINK_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DGN_LINK_T;

typedef union {
    enum { COUNT = 5 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_DGN_GN0_T  			gn0;
        ISP_NVRAM_DGN_GN1_T  			gn1;
        ISP_NVRAM_DGN_GN2_T  			gn2;
        ISP_NVRAM_DGN_GN3_T  			gn3;
        //ISP_NVRAM_DGN_LINK_T  			link;
    };
} ISP_NVRAM_DGN_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// WB
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct {
    FIELD  WB_GAIN_B                                                 : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  WB_GAIN_GB                                                : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_WB_GAIN_1_T; // /* 0x1A010600 */

typedef union {
    typedef ISP_WB_GAIN_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_WB_GAIN_1_T;

typedef struct {
    FIELD  WB_GAIN_GR                                                : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  WB_GAIN_R                                                 : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_WB_GAIN_2_T; // /* 0x1A010604 */

typedef union {
    typedef ISP_WB_GAIN_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_WB_GAIN_2_T;

typedef struct {
    FIELD  WB_CLP_VAL                                                : 24;      /*  0..23, 0x00FFFFFF */
    FIELD  rsv_24                                                    :  8;      /* 24..31, 0xFF000000 */
} ISP_WB_HLC_T; // /* 0x1A010608 */

typedef union {
    typedef ISP_WB_HLC_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_WB_HLC_T;

typedef union {
    enum { COUNT = 3 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_WB_GAIN_1_T  			gain_1;
        ISP_NVRAM_WB_GAIN_2_T  			gain_2;
        ISP_NVRAM_WB_HLC_T  			hlc;
    };
} ISP_NVRAM_WB_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// HLR
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct {
    FIELD  HLR_SAT_0                                                 : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  HLR_SAT_1                                                 : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_HLR_EST_Y0_T; // /* 0x1A011080 */

typedef union {
    typedef ISP_HLR_EST_Y0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_HLR_EST_Y0_T;

typedef struct {
    FIELD  HLR_SAT_2                                                 : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  HLR_SAT_3                                                 : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_HLR_EST_Y1_T; // /* 0x1A011084 */

typedef union {
    typedef ISP_HLR_EST_Y1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_HLR_EST_Y1_T;

typedef struct {
    FIELD  HLR_SAT_4                                                 : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  HLR_SAT_5                                                 : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_HLR_EST_Y2_T; // /* 0x1A011088 */

typedef union {
    typedef ISP_HLR_EST_Y2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_HLR_EST_Y2_T;

typedef struct {
    FIELD  HLR_SAT_6                                                 : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  HLR_SAT_7                                                 : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_HLR_EST_Y3_T; // /* 0x1A01108C */

typedef union {
    typedef ISP_HLR_EST_Y3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_HLR_EST_Y3_T;

typedef struct {
    FIELD  HLR_NODE_0                                                : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  HLR_NODE_1                                                : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_HLR_EST_X0_T; // /* 0x1A011090 */

typedef union {
    typedef ISP_HLR_EST_X0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_HLR_EST_X0_T;

typedef struct {
    FIELD  HLR_NODE_2                                                : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  HLR_NODE_3                                                : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_HLR_EST_X1_T; // /* 0x1A011094 */

typedef union {
    typedef ISP_HLR_EST_X1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_HLR_EST_X1_T;

typedef struct {
    FIELD  HLR_NODE_4                                                : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  HLR_NODE_5                                                : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_HLR_EST_X2_T; // /* 0x1A011098 */

typedef union {
    typedef ISP_HLR_EST_X2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_HLR_EST_X2_T;

typedef struct {
    FIELD  HLR_NODE_6                                                : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  HLR_NODE_7                                                : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_HLR_EST_X3_T; // /* 0x1A01109C */

typedef union {
    typedef ISP_HLR_EST_X3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_HLR_EST_X3_T;

typedef struct {
    FIELD  HLR_SLP_0                                                 : 10;      /*  0.. 9, 0x000003FF */
    FIELD  HLR_SLP_1                                                 : 10;      /* 10..19, 0x000FFC00 */
    FIELD  HLR_SLP_2                                                 : 10;      /* 20..29, 0x3FF00000 */
    FIELD  rsv_30                                                    :  2;      /* 30..31, 0xC0000000 */
} ISP_HLR_EST_S0_T; // /* 0x1A0110A0 */

typedef union {
    typedef ISP_HLR_EST_S0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_HLR_EST_S0_T;

typedef struct {
    FIELD  HLR_SLP_3                                                 : 10;      /*  0.. 9, 0x000003FF */
    FIELD  HLR_SLP_4                                                 : 10;      /* 10..19, 0x000FFC00 */
    FIELD  HLR_SLP_5                                                 : 10;      /* 20..29, 0x3FF00000 */
    FIELD  rsv_30                                                    :  2;      /* 30..31, 0xC0000000 */
} ISP_HLR_EST_S1_T; // /* 0x1A0110A4 */

typedef union {
    typedef ISP_HLR_EST_S1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_HLR_EST_S1_T;

typedef struct {
    FIELD  HLR_SLP_6                                                 : 10;      /*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                                    :  2;      /* 10..11, 0x00000C00 */
    FIELD  HLR_RED_PRT_STR                                           :  5;      /* 12..16, 0x0001F000 */
    FIELD  rsv_17                                                    :  3;      /* 17..19, 0x000E0000 */
    FIELD  HLR_BLUE_PRT_STR                                          :  5;      /* 20..24, 0x01F00000 */
    FIELD  rsv_25                                                    :  3;      /* 25..27, 0x0E000000 */
    FIELD  HLR_MAX_RAT                                               :  3;      /* 28..30, 0x70000000 */
    FIELD  rsv_31                                                    :  1;      /* 31..31, 0x80000000 */
} ISP_HLR_EST_S2_T; // /* 0x1A0110A8 */

typedef union {
    typedef ISP_HLR_EST_S2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_HLR_EST_S2_T;

typedef struct {
    FIELD  HLR_BLD_FG                                                :  6;      /*  0.. 5, 0x0000003F */
    FIELD  HLR_BLD_HIGH                                              : 12;      /*  6..17, 0x0003FFC0 */
    FIELD  HLR_BLD_SLP                                               : 10;      /* 18..27, 0x0FFC0000 */
    FIELD  rsv_28                                                    :  4;      /* 28..31, 0xF0000000 */
} ISP_HLR_LMG_T; // /* 0x1A0110AC */

typedef union {
    typedef ISP_HLR_LMG_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_HLR_LMG_T;

typedef struct {
    FIELD  HLR_PRT_TH                                                : 12;      /*  0..11, 0x00000FFF */
    FIELD  HLR_PRT_SLP                                               : 10;      /* 12..21, 0x003FF000 */
    FIELD  rsv_22                                                    :  2;      /* 22..23, 0x00C00000 */
    FIELD  HLR_PRT_EN                                                :  1;      /* 24..24, 0x01000000 */
    FIELD  rsv_25                                                    :  7;      /* 25..31, 0xFE000000 */
} ISP_HLR_PRT_T; // /* 0x1A0110B0 */

typedef union {
    typedef ISP_HLR_PRT_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_HLR_PRT_T;

typedef struct {
    FIELD  HLR_CLP_VAL                                               : 19;      /*  0..18, 0x0007FFFF */
    FIELD  rsv_19                                                    : 13;      /* 19..31, 0xFFF80000 */
} ISP_HLR_CLP_T; // /* 0x1A0110B4 */

typedef union {
    typedef ISP_HLR_CLP_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_HLR_CLP_T;

typedef struct {
    FIELD  HLR_OFF_CLP_VAL                                           : 19;      /*  0..18, 0x0007FFFF */
    FIELD  HLR_EFCT_ON                                               :  1;      /* 19..19, 0x00080000 */
    FIELD  rsv_20                                                    : 12;      /* 20..31, 0xFFF00000 */
} ISP_HLR_EFCT_T; // /* 0x1A0110B8 */

typedef union {
    typedef ISP_HLR_EFCT_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_HLR_EFCT_T;

typedef struct {
    FIELD  HLR_TILE_EDGE                                             :  4;      /*  0.. 3, 0x0000000F */
    FIELD  rsv_4                                                     : 28;      /*  4..31, 0xFFFFFFF0 */
} ISP_HLR_CTL_T; // /* 0x1A0110BC */

typedef union {
    typedef ISP_HLR_CTL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_HLR_CTL_T;

typedef struct {
    FIELD  HLR_TDR_WD                                                : 16;      /*  0..15, 0x0000FFFF */
    FIELD  HLR_TDR_HT                                                : 16;      /* 16..31, 0xFFFF0000 */
} ISP_HLR_CTL2_T; // /* 0x1A0110C0 */

typedef union {
    typedef ISP_HLR_CTL2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_HLR_CTL2_T;

typedef union {
    FIELD   CAMCTL_HLR_R1_LKMSB                                      :  4;      /*  0.. 3, 0x0000000F */
    FIELD   rsv_4                                                    : 28;      /*  4..31, 0xFFFFFFF0 */
} ISP_HLR_LKMSB_T; // /*0x1A010050*/

typedef union {
    typedef ISP_HLR_LKMSB_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_HLR_LKMSB_T;


typedef union {
    enum { COUNT = 17 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_HLR_EST_Y0_T 			est_y0;
        ISP_NVRAM_HLR_EST_Y1_T  		est_y1;
        ISP_NVRAM_HLR_EST_Y2_T  		est_y2;
        ISP_NVRAM_HLR_EST_Y3_T  		est_y3;
        ISP_NVRAM_HLR_EST_X0_T  		est_x0;
        ISP_NVRAM_HLR_EST_X1_T  		est_x1;
        ISP_NVRAM_HLR_EST_X2_T  		est_x2;
        ISP_NVRAM_HLR_EST_X3_T  		est_x3;
        ISP_NVRAM_HLR_EST_S0_T  		est_s0;
        ISP_NVRAM_HLR_EST_S1_T  		est_s1;
        ISP_NVRAM_HLR_EST_S2_T  		est_s2;
        ISP_NVRAM_HLR_LMG_T  			lmg;
        ISP_NVRAM_HLR_PRT_T  			prt;
        ISP_NVRAM_HLR_CLP_T  			clp;
        ISP_NVRAM_HLR_EFCT_T  			efct;
        ISP_NVRAM_HLR_CTL_T  			ctl;
        ISP_NVRAM_HLR_CTL2_T  			ctl2;
        ISP_NVRAM_HLR_LKMSB_T           lkmsb;
    };
} ISP_NVRAM_HLR_T;



#if 1
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// FUS
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef struct {
    FIELD  FUS_HDR_RATIO_0                                           : 10;      /*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                                    :  2;      /* 10..11, 0x00000C00 */
    FIELD  FUS_HDR_RATIO_1                                           : 10;      /* 12..21, 0x003FF000 */
    FIELD  rsv_22                                                    :  2;      /* 22..23, 0x00C00000 */
    FIELD  FUS_MOSE_RATIO_0                                          :  4;      /* 24..27, 0x0F000000 */
    FIELD  FUS_MOSE_RATIO_1                                          :  4;      /* 28..31, 0xF0000000 */
} ISP_FUS_RATIO1_T; // /* 0x1A013A80 */

typedef union {
    typedef ISP_FUS_RATIO1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_FUS_RATIO1_T;

typedef struct {
    FIELD  FUS_HDR_RATIO_2                                           : 10;      /*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                                    :  2;      /* 10..11, 0x00000C00 */
    FIELD  FUS_MOSE_RATIO_2                                          :  4;      /* 12..15, 0x0000F000 */
    FIELD  FUS_ALIGN_LP_TH_0                                         :  3;      /* 16..18, 0x00070000 */
    FIELD  rsv_19                                                    :  1;      /* 19..19, 0x00080000 */
    FIELD  FUS_ALIGN_LP_TH_1                                         :  3;      /* 20..22, 0x00700000 */
    FIELD  rsv_23                                                    :  1;      /* 23..23, 0x00800000 */
    FIELD  FUS_ALIGN_LP_TH_2                                         :  3;      /* 24..26, 0x07000000 */
    FIELD  rsv_27                                                    :  1;      /* 27..27, 0x08000000 */
    FIELD  FUS_ALIGN_LP_TH_3                                         :  3;      /* 28..30, 0x70000000 */
    FIELD  rsv_31                                                    :  1;      /* 31..31, 0x80000000 */
} ISP_FUS_RATIO2_T; // /* 0x1A013A84 */

typedef union {
    typedef ISP_FUS_RATIO2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_FUS_RATIO2_T;

typedef struct {
    FIELD  FUS_HDR_ALIGN_1                                           : 16;      /*  0..15, 0x0000FFFF */
    FIELD  FUS_HDR_ALIGN_2                                           : 16;      /* 16..31, 0xFFFF0000 */
} ISP_FUS_ALIGN_T; // /* 0x1A013A88 */

typedef union {
    typedef ISP_FUS_ALIGN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_FUS_ALIGN_T;

typedef struct {
    FIELD  FUS_OSC_TH                                                : 12;      /*  0..11, 0x00000FFF */
    FIELD  FUS_OSC_COUNT                                             :  5;      /* 12..16, 0x0001F000 */
    FIELD  FUS_MO_EN                                                 :  1;      /* 17..17, 0x00020000 */
    FIELD  FUS_INPUT_MODE                                            :  2;      /* 18..19, 0x000C0000 */
    FIELD  FUS_MO_TH                                                 :  4;      /* 20..23, 0x00F00000 */
    FIELD  FUS_OSC_BLDWD                                             :  4;      /* 24..27, 0x0F000000 */
    FIELD  FUS_MO_BLDWD                                              :  4;      /* 28..31, 0xF0000000 */
} ISP_FUS_CFG_T; // /* 0x1A013A8C */

typedef union {
    typedef ISP_FUS_CFG_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_FUS_CFG_T;

typedef struct {
    FIELD  FUS_PCCM_CNV_00_0                                         : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  FUS_PCCM_CNV_00_1                                         : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_FUS_CNV_01_00_T; // /* 0x1A013A90 */

typedef union {
    typedef ISP_FUS_CNV_01_00_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_FUS_CNV_01_00_T;

typedef struct {
    FIELD  FUS_PCCM_CNV_01_0                                         : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  FUS_PCCM_CNV_01_1                                         : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_FUS_CNV_01_01_T; // /* 0x1A013A94 */

typedef union {
    typedef ISP_FUS_CNV_01_01_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_FUS_CNV_01_01_T;

typedef struct {
    FIELD  FUS_PCCM_CNV_02_0                                         : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  FUS_PCCM_CNV_02_1                                         : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_FUS_CNV_01_02_T; // /* 0x1A013A98 */

typedef union {
    typedef ISP_FUS_CNV_01_02_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_FUS_CNV_01_02_T;

typedef struct {
    FIELD  FUS_PCCM_CNV_10_0                                         : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  FUS_PCCM_CNV_10_1                                         : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_FUS_CNV_01_10_T; // /* 0x1A013A9C */

typedef union {
    typedef ISP_FUS_CNV_01_10_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_FUS_CNV_01_10_T;

typedef struct {
    FIELD  FUS_PCCM_CNV_11_0                                         : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  FUS_PCCM_CNV_11_1                                         : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_FUS_CNV_01_11_T; // /* 0x1A013AA0 */

typedef union {
    typedef ISP_FUS_CNV_01_11_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_FUS_CNV_01_11_T;

typedef struct {
    FIELD  FUS_PCCM_CNV_12_0                                         : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  FUS_PCCM_CNV_12_1                                         : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_FUS_CNV_01_12_T; // /* 0x1A013AA4 */

typedef union {
    typedef ISP_FUS_CNV_01_12_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_FUS_CNV_01_12_T;

typedef struct {
    FIELD  FUS_PCCM_CNV_20_0                                         : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  FUS_PCCM_CNV_20_1                                         : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_FUS_CNV_01_20_T; // /* 0x1A013AA8 */

typedef union {
    typedef ISP_FUS_CNV_01_20_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_FUS_CNV_01_20_T;

typedef struct {
    FIELD  FUS_PCCM_CNV_21_0                                         : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  FUS_PCCM_CNV_21_1                                         : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_FUS_CNV_01_21_T; // /* 0x1A013AAC */

typedef union {
    typedef ISP_FUS_CNV_01_21_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_FUS_CNV_01_21_T;

typedef struct {
    FIELD  FUS_PCCM_CNV_22_0                                         : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  FUS_PCCM_CNV_22_1                                         : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_FUS_CNV_01_22_T; // /* 0x1A013AB0 */

typedef union {
    typedef ISP_FUS_CNV_01_22_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_FUS_CNV_01_22_T;

typedef struct {
    FIELD  FUS_PCCM_CNV_00_2                                         : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  FUS_PCCM_CNV_00_3                                         : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_FUS_CNV_23_00_T; // /* 0x1A013AB4 */

typedef union {
    typedef ISP_FUS_CNV_23_00_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_FUS_CNV_23_00_T;

typedef struct {
    FIELD  FUS_PCCM_CNV_01_2                                         : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  FUS_PCCM_CNV_01_3                                         : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_FUS_CNV_23_01_T; // /* 0x1A013AB8 */

typedef union {
    typedef ISP_FUS_CNV_23_01_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_FUS_CNV_23_01_T;

typedef struct {
    FIELD  FUS_PCCM_CNV_02_2                                         : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  FUS_PCCM_CNV_02_3                                         : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_FUS_CNV_23_02_T; // /* 0x1A013ABC */

typedef union {
    typedef ISP_FUS_CNV_23_02_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_FUS_CNV_23_02_T;

typedef struct {
    FIELD  FUS_PCCM_CNV_10_2                                         : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  FUS_PCCM_CNV_10_3                                         : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_FUS_CNV_23_10_T; // /* 0x1A013AC0 */

typedef union {
    typedef ISP_FUS_CNV_23_10_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_FUS_CNV_23_10_T;

typedef struct {
    FIELD  FUS_PCCM_CNV_11_2                                         : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  FUS_PCCM_CNV_11_3                                         : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_FUS_CNV_23_11_T; // /* 0x1A013AC4 */

typedef union {
    typedef ISP_FUS_CNV_23_11_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_FUS_CNV_23_11_T;

typedef struct {
    FIELD  FUS_PCCM_CNV_12_2                                         : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  FUS_PCCM_CNV_12_3                                         : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_FUS_CNV_23_12_T; // /* 0x1A013AC8 */

typedef union {
    typedef ISP_FUS_CNV_23_12_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_FUS_CNV_23_12_T;

typedef struct {
    FIELD  FUS_PCCM_CNV_20_2                                         : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  FUS_PCCM_CNV_20_3                                         : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_FUS_CNV_23_20_T; // /* 0x1A013ACC */

typedef union {
    typedef ISP_FUS_CNV_23_20_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_FUS_CNV_23_20_T;

typedef struct {
    FIELD  FUS_PCCM_CNV_21_2                                         : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  FUS_PCCM_CNV_21_3                                         : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_FUS_CNV_23_21_T; // /* 0x1A013AD0 */

typedef union {
    typedef ISP_FUS_CNV_23_21_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_FUS_CNV_23_21_T;

typedef struct {
    FIELD  FUS_PCCM_CNV_22_2                                         : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  FUS_PCCM_CNV_22_3                                         : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_FUS_CNV_23_22_T; // /* 0x1A013AD4 */

typedef union {
    typedef ISP_FUS_CNV_23_22_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_FUS_CNV_23_22_T;

typedef struct {
    FIELD  FUS_HSIZE                                                 : 13;      /*  0..12, 0x00001FFF */
    FIELD  FUS_VSIZE                                                 : 13;      /* 13..25, 0x03FFE000 */
    FIELD  rsv_26                                                    :  6;      /* 26..31, 0xFC000000 */
} ISP_FUS_SIZE_T; // /* 0x1A013AD8 */

typedef union {
    typedef ISP_FUS_SIZE_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_FUS_SIZE_T;

typedef struct {
    FIELD  FUS_FUS_ATPG_OB                                           :  1;      /*  0.. 0, 0x00000001 */
    FIELD  FUS_FUS_ATPG_CT                                           :  1;      /*  1.. 1, 0x00000002 */
    FIELD  rsv_2                                                     : 30;      /*  2..31, 0xFFFFFFFC */
} ISP_FUS_ATPG_T; // /* 0x1A013ADC */

typedef union {
    typedef ISP_FUS_ATPG_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_FUS_ATPG_T;

typedef struct {
    FIELD  FUS_INPUT_SEL_0                                           :  2;      /*  0.. 1, 0x00000003 */
    FIELD  FUS_INPUT_SEL_1                                           :  2;      /*  2.. 3, 0x0000000C */
    FIELD  FUS_INPUT_SEL_2                                           :  2;      /*  4.. 5, 0x00000030 */
    FIELD  FUS_INPUT_SEL_3                                           :  2;      /*  6.. 7, 0x000000C0 */
    FIELD  rsv_8                                                     : 24;      /*  8..31, 0xFFFFFF00 */
} ISP_FUS_CFG2_T; // /* 0x1A013AE0 */

typedef union {
    typedef ISP_FUS_CFG2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_FUS_CFG2_T;

typedef struct {
    FIELD  FUS_TILE_EDGE                                             :  4;      /*  0.. 3, 0x0000000F */
    FIELD  FUS_CROP_H                                                :  4;      /*  4.. 7, 0x000000F0 */
    FIELD  FUS_CROP_V                                                :  4;      /*  8..11, 0x00000F00 */
    FIELD  rsv_12                                                    : 20;      /* 12..31, 0xFFFFF000 */
} ISP_FUS_TILE_T; // /* 0x1A013AE4 */

typedef union {
    typedef ISP_FUS_TILE_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_FUS_TILE_T;

typedef union {
    enum { COUNT = 26 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_FUS_RATIO1_T  			ratio1;
        ISP_NVRAM_FUS_RATIO2_T  			ratio2;
        ISP_NVRAM_FUS_ALIGN_T  		    	align;
        ISP_NVRAM_FUS_CFG_T  		    	cfg;
        ISP_NVRAM_FUS_CNV_01_00_T  			cnv_01_00;
        ISP_NVRAM_FUS_CNV_01_01_T  			cnv_01_01;
        ISP_NVRAM_FUS_CNV_01_02_T  			cnv_01_02;
        ISP_NVRAM_FUS_CNV_01_10_T  			cnv_01_10;
        ISP_NVRAM_FUS_CNV_01_11_T  			cnv_01_11;
        ISP_NVRAM_FUS_CNV_01_12_T  			cnv_01_12;
        ISP_NVRAM_FUS_CNV_01_20_T  			cnv_01_20;
        ISP_NVRAM_FUS_CNV_01_21_T  			cnv_01_21;
        ISP_NVRAM_FUS_CNV_01_22_T  			cnv_01_22;
        ISP_NVRAM_FUS_CNV_23_00_T  			cnv_23_00;
        ISP_NVRAM_FUS_CNV_23_01_T  			cnv_23_01;
        ISP_NVRAM_FUS_CNV_23_02_T  			cnv_23_02;
        ISP_NVRAM_FUS_CNV_23_10_T  			cnv_23_10;
        ISP_NVRAM_FUS_CNV_23_11_T  			cnv_23_11;
        ISP_NVRAM_FUS_CNV_23_12_T  			cnv_23_12;
        ISP_NVRAM_FUS_CNV_23_20_T  			cnv_23_20;
        ISP_NVRAM_FUS_CNV_23_21_T  			cnv_23_21;
        ISP_NVRAM_FUS_CNV_23_22_T  			cnv_23_22;
        ISP_NVRAM_FUS_SIZE_T  		    	size;
        ISP_NVRAM_FUS_ATPG_T  		    	atpg;
        ISP_NVRAM_FUS_CFG2_T  			    cfg2;
        ISP_NVRAM_FUS_TILE_T  			    tile;
    };
} ISP_NVRAM_FUS_T;



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ZFUS
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef struct {
    FIELD  ZFUS_OSC_TH                                               : 12;      /*  0..11, 0x00000FFF */
    FIELD  ZFUS_SEDIR_SL                                             :  3;      /* 12..14, 0x00007000 */
    FIELD  rsv_15                                                    :  1;      /* 15..15, 0x00008000 */
    FIELD  ZFUS_SEBLD_WD                                             :  4;      /* 16..19, 0x000F0000 */
    FIELD  ZFUS_LEBLD_WD                                             :  4;      /* 20..23, 0x00F00000 */
    FIELD  ZFUS_LE_INV_CTL                                           :  4;      /* 24..27, 0x0F000000 */
    FIELD  ZFUS_EDGE                                                 :  4;      /* 28..31, 0xF0000000 */
} ISP_ZFUS_OSC_T; // /* 0x1A011000 */

typedef union {
    typedef ISP_ZFUS_OSC_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ZFUS_OSC_T;

typedef struct {
    FIELD  ZFUS_MO_EN                                                :  1;      /*  0.. 0, 0x00000001 */
    FIELD  rsv_1                                                     :  3;      /*  1.. 3, 0x0000000E */
    FIELD  ZFUS_MOBLD_FT                                             :  3;      /*  4.. 6, 0x00000070 */
    FIELD  rsv_7                                                     :  1;      /*  7.. 7, 0x00000080 */
    FIELD  ZFUS_MOTH_RATIO                                           :  5;      /*  8..12, 0x00001F00 */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  ZFUS_MOSE_TH                                              : 12;      /* 16..27, 0x0FFF0000 */
    FIELD  ZFUS_MOSE_BLDWD                                           :  4;      /* 28..31, 0xF0000000 */
} ISP_ZFUS_MC_T; // /* 0x1A011004 */

typedef union {
    typedef ISP_ZFUS_MC_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ZFUS_MC_T;

typedef struct {
    FIELD  ZFUS_MASE_BLDWD                                           :  4;      /*  0.. 3, 0x0000000F */
    FIELD  ZFUS_OSBLD_WD                                             :  4;      /*  4.. 7, 0x000000F0 */
    FIELD  ZFUS_SEYOS_LEFAC                                          :  5;      /*  8..12, 0x00001F00 */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  ZFUS_SENOS_LEFAC                                          :  5;      /* 16..20, 0x001F0000 */
    FIELD  ZFUS_DIFF_LB                                              : 11;      /* 21..31, 0xFFE00000 */
} ISP_ZFUS_MA_T; // /* 0x1A011008 */

typedef union {
    typedef ISP_ZFUS_MA_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ZFUS_MA_T;

typedef struct {
    FIELD  ZFUS_PS_BLUR                                              :  1;      /*  0.. 0, 0x00000001 */
    FIELD  ZFUS_OSC_REPEN                                            :  1;      /*  1.. 1, 0x00000002 */
    FIELD  ZFUS_SOFT_TH_EN                                           :  1;      /*  2.. 2, 0x00000004 */
    FIELD  ZFUS_LE_LOWPA_EN                                          :  1;      /*  3.. 3, 0x00000008 */
    FIELD  ZFUS_SE_LOWPA_EN                                          :  1;      /*  4.. 4, 0x00000010 */
    FIELD  ZFUS_PSSEC_ONLY                                           :  1;      /*  5.. 5, 0x00000020 */
    FIELD  ZFUS_OSCLE_ONLY                                           :  1;      /*  6.. 6, 0x00000040 */
    FIELD  ZFUS_MOLE_DIREN                                           :  1;      /*  7.. 7, 0x00000080 */
    FIELD  ZFUS_MOSE_DIREN                                           :  1;      /*  8.. 8, 0x00000100 */
    FIELD  ZFUS_MO_2DBLD_EN                                          :  1;      /*  9.. 9, 0x00000200 */
    FIELD  rsv_10                                                    :  2;      /* 10..11, 0x00000C00 */
    FIELD  ZFUS_PS_TH                                                : 12;      /* 12..23, 0x00FFF000 */
    FIELD  rsv_24                                                    :  8;      /* 24..31, 0xFF000000 */
} ISP_ZFUS_TUNE_T; // /* 0x1A01100C */

typedef union {
    typedef ISP_ZFUS_TUNE_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ZFUS_TUNE_T;

typedef struct {
    FIELD  ZFUS_ZHDR_GLE_FIRST                                       :  1;      /*  0.. 0, 0x00000001 */
    FIELD  ZFUS_ZHDR_RLE_FIRST                                       :  1;      /*  1.. 1, 0x00000002 */
    FIELD  ZFUS_ZHDR_BLE_FIRST                                       :  1;      /*  2.. 2, 0x00000004 */
    FIELD  rsv_3                                                     :  1;      /*  3.. 3, 0x00000008 */
    FIELD  ZFUS_HDR_GAIN                                             :  9;      /*  4..12, 0x00001FF0 */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  ZFUS_HDR_RATIO                                            :  9;      /* 16..24, 0x01FF0000 */
    FIELD  rsv_25                                                    :  7;      /* 25..31, 0xFE000000 */
} ISP_ZFUS_HDRCFG_T; // /* 0x1A011010 */

typedef union {
    typedef ISP_ZFUS_HDRCFG_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ZFUS_HDRCFG_T;

typedef union {
    enum { COUNT = 5 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_ZFUS_OSC_T  			osc;
        ISP_NVRAM_ZFUS_MC_T  			mc;
        ISP_NVRAM_ZFUS_MA_T  			ma;
        ISP_NVRAM_ZFUS_TUNE_T  			tune;
        ISP_NVRAM_ZFUS_HDRCFG_T  		hdrcfg;
    };
} ISP_NVRAM_ZFUS_T;
#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SLK
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef struct {
    FIELD  SLK_CENTR_X                                               : 14;      /*  0..13, 0x00003FFF */
    FIELD  rsv_14                                                    :  2;      /* 14..15, 0x0000C000 */
    FIELD  SLK_CENTR_Y                                               : 14;      /* 16..29, 0x3FFF0000 */
    FIELD  rsv_30                                                    :  2;      /* 30..31, 0xC0000000 */
} ISP_SLK_CEN_T; // /* 0x1A0117C0 */

typedef union {
    typedef ISP_SLK_CEN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SLK_CEN_T;

typedef struct {
    FIELD  SLK_R_0                                                   : 14;      /*  0..13, 0x00003FFF */
    FIELD  rsv_14                                                    :  2;      /* 14..15, 0x0000C000 */
    FIELD  SLK_R_1                                                   : 14;      /* 16..29, 0x3FFF0000 */
    FIELD  rsv_30                                                    :  2;      /* 30..31, 0xC0000000 */
} ISP_SLK_RR_CON0_T; // /* 0x1A0117C4 */

typedef union {
    typedef ISP_SLK_RR_CON0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SLK_RR_CON0_T;

typedef struct {
    FIELD  SLK_R_2                                                   : 14;      /*  0..13, 0x00003FFF */
    FIELD  rsv_14                                                    :  2;      /* 14..15, 0x0000C000 */
    FIELD  SLK_GAIN_0                                                :  8;      /* 16..23, 0x00FF0000 */
    FIELD  SLK_GAIN_1                                                :  8;      /* 24..31, 0xFF000000 */
} ISP_SLK_RR_CON1_T; // /* 0x1A0117C8 */

typedef union {
    typedef ISP_SLK_RR_CON1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SLK_RR_CON1_T;

typedef struct {
    FIELD  SLK_GAIN_2                                                :  8;      /*  0.. 7, 0x000000FF */
    FIELD  SLK_GAIN_3                                                :  8;      /*  8..15, 0x0000FF00 */
    FIELD  SLK_GAIN_4                                                :  8;      /* 16..23, 0x00FF0000 */
    FIELD  SLK_SET_ZERO                                              :  1;      /* 24..24, 0x01000000 */
    FIELD  rsv_25                                                    :  7;      /* 25..31, 0xFE000000 */
} ISP_SLK_GAIN_T; // /* 0x1A0117CC */

typedef union {
    typedef ISP_SLK_GAIN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SLK_GAIN_T;

typedef struct {
    FIELD  SLK_HRZ_COMP                                              : 14;      /*  0..13, 0x00003FFF */
    FIELD  rsv_14                                                    :  2;      /* 14..15, 0x0000C000 */
    FIELD  SLK_VRZ_COMP                                              : 14;      /* 16..29, 0x3FFF0000 */
    FIELD  rsv_30                                                    :  2;      /* 30..31, 0xC0000000 */
} ISP_SLK_RZ_T; // /* 0x1A0117D0 */

typedef union {
    typedef ISP_SLK_RZ_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SLK_RZ_T;

typedef struct {
    FIELD  SLK_X_OFST                                                : 28;      /*  0..27, 0x0FFFFFFF */
    FIELD  rsv_28                                                    :  4;      /* 28..31, 0xF0000000 */
} ISP_SLK_XOFF_T; // /* 0x1A0117D4 */

typedef union {
    typedef ISP_SLK_XOFF_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SLK_XOFF_T;

typedef struct {
    FIELD  SLK_Y_OFST                                                : 28;      /*  0..27, 0x0FFFFFFF */
    FIELD  rsv_28                                                    :  4;      /* 28..31, 0xF0000000 */
} ISP_SLK_YOFF_T; // /* 0x1A0117D8 */

typedef union {
    typedef ISP_SLK_YOFF_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SLK_YOFF_T;

typedef struct {
    FIELD  SLK_SLP_1                                                 : 24;      /*  0..23, 0x00FFFFFF */
    FIELD  rsv_24                                                    :  8;      /* 24..31, 0xFF000000 */
} ISP_SLK_SLP_CON0_T; // /* 0x1A0117DC */

typedef union {
    typedef ISP_SLK_SLP_CON0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SLK_SLP_CON0_T;

typedef struct {
    FIELD  SLK_SLP_2                                                 : 24;      /*  0..23, 0x00FFFFFF */
    FIELD  rsv_24                                                    :  8;      /* 24..31, 0xFF000000 */
} ISP_SLK_SLP_CON1_T; // /* 0x1A0117E0 */

typedef union {
    typedef ISP_SLK_SLP_CON1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SLK_SLP_CON1_T;

typedef struct {
    FIELD  SLK_SLP_3                                                 : 24;      /*  0..23, 0x00FFFFFF */
    FIELD  rsv_24                                                    :  8;      /* 24..31, 0xFF000000 */
} ISP_SLK_SLP_CON2_T; // /* 0x1A0117E4 */

typedef union {
    typedef ISP_SLK_SLP_CON2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SLK_SLP_CON2_T;

typedef struct {
    FIELD  SLK_SLP_4                                                 : 24;      /*  0..23, 0x00FFFFFF */
    FIELD  rsv_24                                                    :  8;      /* 24..31, 0xFF000000 */
} ISP_SLK_SLP_CON3_T; // /* 0x1A0117E8 */

typedef union {
    typedef ISP_SLK_SLP_CON3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SLK_SLP_CON3_T;

typedef struct {
    FIELD  SLK_TPIPE_WD                                              : 14;      /*  0..13, 0x00003FFF */
    FIELD  rsv_14                                                    :  2;      /* 14..15, 0x0000C000 */
    FIELD  SLK_TPIPE_HT                                              : 14;      /* 16..29, 0x3FFF0000 */
    FIELD  rsv_30                                                    :  2;      /* 30..31, 0xC0000000 */
} ISP_SLK_SIZE_T; // /* 0x1A0117EC */

typedef union {
    typedef ISP_SLK_SIZE_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SLK_SIZE_T;

typedef union {
    enum { COUNT = 2 };
    MUINT32 set[COUNT];
    struct {
        //ISP_NVRAM_SLK_CEN_T  			cen;
        //ISP_NVRAM_SLK_RR_CON0_T  		rr_con0;
        ISP_NVRAM_SLK_RR_CON1_T  		rr_con1;
        ISP_NVRAM_SLK_GAIN_T  			gain;
        //ISP_NVRAM_SLK_RZ_T  			rz;
        //ISP_NVRAM_SLK_XOFF_T  		xoff;
        //ISP_NVRAM_SLK_YOFF_T  		yoff;
        //ISP_NVRAM_SLK_SLP_CON0_T  	slp_con0;
        //ISP_NVRAM_SLK_SLP_CON1_T  	slp_con1;
        //ISP_NVRAM_SLK_SLP_CON2_T  	slp_con2;
        //ISP_NVRAM_SLK_SLP_CON3_T  	slp_con3;
        //ISP_NVRAM_SLK_SIZE_T  		size;
    };
} ISP_NVRAM_SLK_T;



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//DM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef struct {
    FIELD  DM_CDG_SL                                                 :  4;      /*  0.. 3, 0x0000000F */
    FIELD  DM_CDG_OFST                                               :  8;      /*  4..11, 0x00000FF0 */
    FIELD  DM_CDG_RAT                                                :  5;      /* 12..16, 0x0001F000 */
    FIELD  DM_CD_KNL                                                 :  1;      /* 17..17, 0x00020000 */
    FIELD  rsv_18                                                    : 12;      /* 18..29, 0x3FFC0000 */
    FIELD  DM_BYP                                                    :  1;      /* 30..30, 0x40000000 */
    FIELD  DM_MN_MODE                                                :  1;      /* 31..31, 0x80000000 */
} ISP_DM_INTP_CRS_T; // /* 0x1A011800 */

typedef union {
    typedef ISP_DM_INTP_CRS_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DM_INTP_CRS_T;

typedef struct {
    FIELD  DM_HL_OFST                                                :  8;      /*  0.. 7, 0x000000FF */
    FIELD  DM_L0_SL                                                  :  4;      /*  8..11, 0x00000F00 */
    FIELD  DM_L0_OFST                                                :  8;      /* 12..19, 0x000FF000 */
    FIELD  DM_CD_SLL                                                 :  5;      /* 20..24, 0x01F00000 */
    FIELD  DM_CD_SLC                                                 :  5;      /* 25..29, 0x3E000000 */
    FIELD  rsv_30                                                    :  2;      /* 30..31, 0xC0000000 */
} ISP_DM_INTP_NAT_T; // /* 0x1A011804 */

typedef union {
    typedef ISP_DM_INTP_NAT_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DM_INTP_NAT_T;

typedef struct {
    FIELD  DM_DN_OFST                                                :  8;      /*  0.. 7, 0x000000FF */
    FIELD  DM_L2_SL                                                  :  4;      /*  8..11, 0x00000F00 */
    FIELD  DM_L2_OFST                                                :  8;      /* 12..19, 0x000FF000 */
    FIELD  DM_L1_SL                                                  :  4;      /* 20..23, 0x00F00000 */
    FIELD  DM_L1_OFST                                                :  8;      /* 24..31, 0xFF000000 */
} ISP_DM_INTP_AUG_T; // /* 0x1A011808 */

typedef union {
    typedef ISP_DM_INTP_AUG_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DM_INTP_AUG_T;

typedef struct {
    FIELD  DM_LM_Y2                                                  :  9;      /*  0.. 8, 0x000001FF */
    FIELD  DM_LM_Y1                                                  :  9;      /*  9..17, 0x0003FE00 */
    FIELD  DM_LM_Y0                                                  :  9;      /* 18..26, 0x07FC0000 */
    FIELD  rsv_27                                                    :  5;      /* 27..31, 0xF8000000 */
} ISP_DM_LUMA_LUT1_T; // /* 0x1A01180C */

typedef union {
    typedef ISP_DM_LUMA_LUT1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DM_LUMA_LUT1_T;

typedef struct {
    FIELD  DM_LM_Y5                                                  :  9;      /*  0.. 8, 0x000001FF */
    FIELD  DM_LM_Y4                                                  :  9;      /*  9..17, 0x0003FE00 */
    FIELD  DM_LM_Y3                                                  :  9;      /* 18..26, 0x07FC0000 */
    FIELD  rsv_27                                                    :  5;      /* 27..31, 0xF8000000 */
} ISP_DM_LUMA_LUT2_T; // /* 0x1A011810 */

typedef union {
    typedef ISP_DM_LUMA_LUT2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DM_LUMA_LUT2_T;

typedef struct {
    FIELD  DM_SL_EN                                                  :  1;      /*  0.. 0, 0x00000001 */
    FIELD  DM_SL_HR                                                  :  5;      /*  1.. 5, 0x0000003E */
    FIELD  DM_SL_Y2                                                  :  8;      /*  6..13, 0x00003FC0 */
    FIELD  DM_SL_Y1                                                  :  8;      /* 14..21, 0x003FC000 */
    FIELD  rsv_22                                                    : 10;      /* 22..31, 0xFFC00000 */
} ISP_DM_SL_CTL_T; // /* 0x1A011814 */

typedef union {
    typedef ISP_DM_SL_CTL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DM_SL_CTL_T;

typedef struct {
    FIELD  DM_CORE_TH1                                               :  8;      /*  0.. 7, 0x000000FF */
    FIELD  DM_HD_GN2                                                 :  5;      /*  8..12, 0x00001F00 */
    FIELD  DM_HD_GN1                                                 :  5;      /* 13..17, 0x0003E000 */
    FIELD  DM_HT_GN2                                                 :  5;      /* 18..22, 0x007C0000 */
    FIELD  DM_HT_GN1                                                 :  5;      /* 23..27, 0x0F800000 */
    FIELD  rsv_28                                                    :  4;      /* 28..31, 0xF0000000 */
} ISP_DM_HFTD_CTL_T; // /* 0x1A011818 */

typedef union {
    typedef ISP_DM_HFTD_CTL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DM_HFTD_CTL_T;

typedef struct {
    FIELD  DM_N2_STR                                                 :  5;      /*  0.. 4, 0x0000001F */
    FIELD  DM_N1_STR                                                 :  5;      /*  5.. 9, 0x000003E0 */
    FIELD  DM_N0_STR                                                 :  5;      /* 10..14, 0x00007C00 */
    FIELD  DM_XTK_SL                                                 :  4;      /* 15..18, 0x00078000 */
    FIELD  DM_XTK_OFST                                               :  8;      /* 19..26, 0x07F80000 */
    FIELD  DM_XTK_RAT                                                :  5;      /* 27..31, 0xF8000000 */
} ISP_DM_NR_STR_T; // /* 0x1A01181C */

typedef union {
    typedef ISP_DM_NR_STR_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DM_NR_STR_T;

typedef struct {
    FIELD  DM_NGR                                                    :  4;      /*  0.. 3, 0x0000000F */
    FIELD  DM_NSL                                                    :  4;      /*  4.. 7, 0x000000F0 */
    FIELD  DM_N2_OFST                                                :  8;      /*  8..15, 0x0000FF00 */
    FIELD  DM_N1_OFST                                                :  8;      /* 16..23, 0x00FF0000 */
    FIELD  DM_N0_OFST                                                :  8;      /* 24..31, 0xFF000000 */
} ISP_DM_NR_ACT_T; // /* 0x1A011820 */

typedef union {
    typedef ISP_DM_NR_ACT_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DM_NR_ACT_T;

typedef struct {
    FIELD  DM_CORE_TH2                                               :  8;      /*  0.. 7, 0x000000FF */
    FIELD  DM_HI_RAT                                                 :  4;      /*  8..11, 0x00000F00 */
    FIELD  DM_H3_GN                                                  :  5;      /* 12..16, 0x0001F000 */
    FIELD  DM_H2_GN                                                  :  5;      /* 17..21, 0x003E0000 */
    FIELD  DM_H1_GN                                                  :  5;      /* 22..26, 0x07C00000 */
    FIELD  DM_HA_STR                                                 :  5;      /* 27..31, 0xF8000000 */
} ISP_DM_HF_STR_T; // /* 0x1A011824 */

typedef union {
    typedef ISP_DM_HF_STR_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DM_HF_STR_T;

typedef struct {
    FIELD  DM_H2_UPB                                                 :  8;      /*  0.. 7, 0x000000FF */
    FIELD  DM_H2_LWB                                                 :  8;      /*  8..15, 0x0000FF00 */
    FIELD  DM_H1_UPB                                                 :  8;      /* 16..23, 0x00FF0000 */
    FIELD  DM_H1_LWB                                                 :  8;      /* 24..31, 0xFF000000 */
} ISP_DM_HF_ACT1_T; // /* 0x1A011828 */

typedef union {
    typedef ISP_DM_HF_ACT1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DM_HF_ACT1_T;

typedef struct {
    FIELD  DM_HSLL                                                   :  4;      /*  0.. 3, 0x0000000F */
    FIELD  DM_HSLR                                                   :  4;      /*  4.. 7, 0x000000F0 */
    FIELD  DM_H3_UPB                                                 :  8;      /*  8..15, 0x0000FF00 */
    FIELD  DM_H3_LWB                                                 :  8;      /* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                                    :  8;      /* 24..31, 0xFF000000 */
} ISP_DM_HF_ACT2_T; // /* 0x1A01182C */

typedef union {
    typedef ISP_DM_HF_ACT2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DM_HF_ACT2_T;

typedef struct {
    FIELD  DM_CLIP_TH                                                :  8;      /*  0.. 7, 0x000000FF */
    FIELD  DM_UN_TH                                                  :  8;      /*  8..15, 0x0000FF00 */
    FIELD  DM_OV_TH                                                  :  8;      /* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                                    :  8;      /* 24..31, 0xFF000000 */
} ISP_DM_CLIP_T; // /* 0x1A011830 */

typedef union {
    typedef ISP_DM_CLIP_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DM_CLIP_T;

typedef struct {
    FIELD  DM_SC_RAT                                                 :  5;      /*  0.. 4, 0x0000001F */
    FIELD  DM_SL_RAT                                                 :  5;      /*  5.. 9, 0x000003E0 */
    FIELD  DM_FL_MODE                                                :  1;      /* 10..10, 0x00000400 */
    FIELD  rsv_11                                                    : 21;      /* 11..31, 0xFFFFF800 */
} ISP_DM_DSB_T; // /* 0x1A011834 */

typedef union {
    typedef ISP_DM_DSB_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DM_DSB_T;

typedef struct {
    FIELD  DM_TILE_EDGE                                              :  4;      /*  0.. 3, 0x0000000F */
    FIELD  rsv_4                                                     : 28;      /*  4..31, 0xFFFFFFF0 */
} ISP_DM_TILE_EDGE_T; // /* 0x1A011838 */

typedef union {
    typedef ISP_DM_TILE_EDGE_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DM_TILE_EDGE_T;

typedef struct {
    FIELD  DM_P1_UPB                                                 :  8;      /*  0.. 7, 0x000000FF */
    FIELD  DM_P1_LWB                                                 :  8;      /*  8..15, 0x0000FF00 */
    FIELD  rsv_16                                                    : 16;      /* 16..31, 0xFFFF0000 */
} ISP_DM_P1_ACT_T; // /* 0x1A01183C */

typedef union {
    typedef ISP_DM_P1_ACT_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DM_P1_ACT_T;

typedef struct {
    FIELD  DM_LR_RAT                                                 :  4;      /*  0.. 3, 0x0000000F */
    FIELD  rsv_4                                                     : 28;      /*  4..31, 0xFFFFFFF0 */
} ISP_DM_LR_RAT_T; // /* 0x1A011840 */

typedef union {
    typedef ISP_DM_LR_RAT_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DM_LR_RAT_T;

typedef struct {
    FIELD  DM_HD_GN3                                                 :  6;      /*  0.. 5, 0x0000003F */
    FIELD  DM_HFRB_GN                                                :  6;      /*  6..11, 0x00000FC0 */
    FIELD  rsv_12                                                    : 20;      /* 12..31, 0xFFFFF000 */
} ISP_DM_HFTD_CTL2_T; // /* 0x1A011844 */

typedef union {
    typedef ISP_DM_HFTD_CTL2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DM_HFTD_CTL2_T;

typedef struct {
    FIELD  DM_P2_CLIP                                                :  1;      /*  0.. 0, 0x00000001 */
    FIELD  DM_P1_BLD                                                 :  5;      /*  1.. 5, 0x0000003E */
    FIELD  rsv_6                                                     : 26;      /*  6..31, 0xFFFFFFC0 */
} ISP_DM_EST_CTL_T; // /* 0x1A011848 */

typedef union {
    typedef ISP_DM_EST_CTL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DM_EST_CTL_T;

typedef struct {
    FIELD  DM_SPARE_2                                                : 32;      /*  0..31, 0xFFFFFFFF */
} ISP_DM_SPARE_2_T; // /* 0x1A01184C */

typedef union {
    typedef ISP_DM_SPARE_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DM_SPARE_2_T;

typedef struct {
    FIELD  DM_SPARE_3                                                : 32;      /*  0..31, 0xFFFFFFFF */
} ISP_DM_SPARE_3_T; // /* 0x1A011850 */

typedef union {
    typedef ISP_DM_SPARE_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DM_SPARE_3_T;

typedef struct {
    FIELD  DM_INT_LTH                                                :  4;      /*  0.. 3, 0x0000000F */
    FIELD  DM_INT_CDTH                                               :  4;      /*  4.. 7, 0x000000F0 */
    FIELD  rsv_8                                                     : 24;      /*  8..31, 0xFFFFFF00 */
} ISP_DM_INT_CTL_T; // /* 0x1A011854 */

typedef union {
    typedef ISP_DM_INT_CTL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DM_INT_CTL_T;

typedef struct {
    FIELD  DM_HPOS_GN                                                :  5;      /*  0.. 4, 0x0000001F */
    FIELD  DM_HNEG_GN                                                :  5;      /*  5.. 9, 0x000003E0 */
    FIELD  rsv_10                                                    : 22;      /* 10..31, 0xFFFFFC00 */
} ISP_DM_EE_T; // /* 0x1A011858 */

typedef union {
    typedef ISP_DM_EE_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DM_EE_T;

typedef struct {
    FIELD  DM_LMT_UPB                                                :  8;      /*  0.. 7, 0x000000FF */
    FIELD  DM_LMT_LWB                                                :  8;      /*  8..15, 0x0000FF00 */
    FIELD  DM_LMT_EN                                                 :  1;      /* 16..16, 0x00010000 */
    FIELD  rsv_17                                                    : 15;      /* 17..31, 0xFFFE0000 */
} ISP_DM_LMT_T; // /* 0x1A011860 */

typedef union {
    typedef ISP_DM_LMT_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DM_LMT_T;

typedef struct {
    FIELD  DM_RCDM_SL                                                : 13;      /*  0..12, 0x00001FFF */
    FIELD  DM_RCCC_EN                                                :  1;      /* 13..13, 0x00002000 */
    FIELD  rsv_14                                                    : 18;      /* 14..31, 0xFFFFC000 */
} ISP_DM_RCCC_T; // /* 0x1A011864 */

typedef union {
    typedef ISP_DM_RCCC_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DM_RCCC_T;

typedef union {
    enum { COUNT = 21 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_DM_INTP_CRS_T  		intp_crs;
        ISP_NVRAM_DM_INTP_NAT_T  		intp_nat;
        ISP_NVRAM_DM_INTP_AUG_T  		intp_aug;
        ISP_NVRAM_DM_LUMA_LUT1_T  		luma_lut1;
        ISP_NVRAM_DM_LUMA_LUT2_T  		luma_lut2;
        ISP_NVRAM_DM_SL_CTL_T  			sl_ctl;
        ISP_NVRAM_DM_HFTD_CTL_T  		hftd_ctl;
        ISP_NVRAM_DM_NR_STR_T  			nr_str;
        ISP_NVRAM_DM_NR_ACT_T  			nr_act;
        ISP_NVRAM_DM_HF_STR_T  			hf_str;
        ISP_NVRAM_DM_HF_ACT1_T  		hf_act1;
        ISP_NVRAM_DM_HF_ACT2_T  		hf_act2;
        ISP_NVRAM_DM_CLIP_T  			clip;
        //ISP_NVRAM_DM_DSB_T  			dsb;
        //ISP_NVRAM_DM_TILE_EDGE_T  		tile_edge;
        ISP_NVRAM_DM_P1_ACT_T  			p1_act;
        ISP_NVRAM_DM_LR_RAT_T  			lr_rat;
        ISP_NVRAM_DM_HFTD_CTL2_T  		hftd_ctl2;
        ISP_NVRAM_DM_EST_CTL_T  		est_ctl;
        //ISP_NVRAM_DM_SPARE_2_T  		spare_2;
        //ISP_NVRAM_DM_SPARE_3_T  		spare_3;
        ISP_NVRAM_DM_INT_CTL_T  		int_ctl;
        ISP_NVRAM_DM_EE_T  			    ee;
        ISP_NVRAM_DM_LMT_T  			lmt;
        ISP_NVRAM_DM_RCCC_T  			rccc;
    };
} ISP_NVRAM_DM_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CCM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct {
    FIELD  CCM_CNV_00                                                : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  CCM_CNV_01                                                : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_CCM_CNV_1_T; // /* 0x1A011880 */

typedef union {
    typedef ISP_CCM_CNV_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CCM_CNV_1_T;

typedef struct {
    FIELD  CCM_CNV_02                                                : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    : 19;      /* 13..31, 0xFFFFE000 */
} ISP_CCM_CNV_2_T; // /* 0x1A011884 */

typedef union {
    typedef ISP_CCM_CNV_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CCM_CNV_2_T;

typedef struct {
    FIELD  CCM_CNV_10                                                : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  CCM_CNV_11                                                : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_CCM_CNV_3_T; // /* 0x1A011888 */

typedef union {
    typedef ISP_CCM_CNV_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CCM_CNV_3_T;

typedef struct {
    FIELD  CCM_CNV_12                                                : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    : 19;      /* 13..31, 0xFFFFE000 */
} ISP_CCM_CNV_4_T; // /* 0x1A01188C */

typedef union {
    typedef ISP_CCM_CNV_4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CCM_CNV_4_T;

typedef struct {
    FIELD  CCM_CNV_20                                                : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  CCM_CNV_21                                                : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_CCM_CNV_5_T; // /* 0x1A011890 */

typedef union {
    typedef ISP_CCM_CNV_5_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CCM_CNV_5_T;

typedef struct {
    FIELD  CCM_CNV_22                                                : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    : 19;      /* 13..31, 0xFFFFE000 */
} ISP_CCM_CNV_6_T; // /* 0x1A011894 */

typedef union {
    typedef ISP_CCM_CNV_6_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CCM_CNV_6_T;

typedef union {
    enum { COUNT = 6 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_CCM_CNV_1_T  			cnv_1;
        ISP_NVRAM_CCM_CNV_2_T  			cnv_2;
        ISP_NVRAM_CCM_CNV_3_T  			cnv_3;
        ISP_NVRAM_CCM_CNV_4_T  			cnv_4;
        ISP_NVRAM_CCM_CNV_5_T  			cnv_5;
        ISP_NVRAM_CCM_CNV_6_T  			cnv_6;
    };
} ISP_NVRAM_CCM_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//GGM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

enum { GGM_LUT_SIZE = 192 };

typedef struct {
    FIELD  GGM_R                                 : 10;		/*  0.. 9, 0x000003FF */
    FIELD  GGM_G                                 : 10;		/* 10..19, 0x000FFC00 */
    FIELD  GGM_B                                 : 10;		/* 20..29, 0x3FF00000 */
    FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
} ISP_GGM_LUT_T;

typedef union {
    typedef ISP_GGM_LUT_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_GGM_LUT_T;


typedef union {
    enum { COUNT = GGM_LUT_SIZE };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_GGM_LUT_T   lut[GGM_LUT_SIZE];
    };
} ISP_NVRAM_GGM_T;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//IGGM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

enum { IGGM_LUT_SIZE = 192 };

typedef struct {
		FIELD  IGGM_R                                     : 12;		/*  0..11, 0x00000FFF */
		FIELD  rsv_12                                     :  4;		/* 12..15, 0x0000F000 */
		FIELD  IGGM_G                                     : 12;		/* 16..27, 0x0FFF0000 */
		FIELD  rsv_28                                     :  4;		/* 28..31, 0xF0000000 */
} ISP_IGGM_LUT_RG_T;

typedef struct {
		FIELD  IGGM_B                                     : 12;		/*  0..11, 0x00000FFF */
		FIELD  rsv_12                                     : 20;		/* 12..31, 0xFFFFF000 */
} ISP_IGGM_LUT_B_T;

typedef union {
    enum { COUNT = IGGM_LUT_SIZE };
    MUINT32 set[COUNT];
    struct {
        ISP_IGGM_LUT_RG_T  lut[IGGM_LUT_SIZE];
    };
} ISP_NVRAM_IGGM_LUT_RG_T;
//
typedef union {
    enum { COUNT = IGGM_LUT_SIZE };
    MUINT32 set[COUNT];
    struct {
        ISP_IGGM_LUT_B_T   lut[IGGM_LUT_SIZE];
    };
} ISP_NVRAM_IGGM_LUT_B_T;
//
typedef struct {
    ISP_NVRAM_IGGM_LUT_RG_T  lut_rg;
    ISP_NVRAM_IGGM_LUT_B_T   lut_b;
} ISP_NVRAM_IGGM_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//YNRS Table
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
enum { YNRS_TBL_NUM = 256 };
typedef struct {
        FIELD  YNRS_TBL_Y0                                               :  5;      /*  0.. 4, 0x0000001F */
        FIELD  YNRS_TBL_U0                                               :  5;      /*  5.. 9, 0x000003E0 */
        FIELD  YNRS_TBL_V0                                               :  5;      /* 10..14, 0x00007C00 */
        FIELD  rsv_15                                                    :  1;      /* 15..15, 0x00008000 */
        FIELD  YNRS_TBL_Y1                                               :  5;      /* 16..20, 0x001F0000 */
        FIELD  YNRS_TBL_U1                                               :  5;      /* 21..25, 0x03E00000 */
        FIELD  YNRS_TBL_V1                                               :  5;      /* 26..30, 0x7C000000 */
        FIELD  rsv_31                                                    :  1;      /* 31..31, 0x80000000 */
} ISP_YNRS_TBL_T;

typedef union {
    typedef ISP_YNRS_TBL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNRS_TBL_T;

typedef union {
    enum { COUNT = YNRS_TBL_NUM };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_YNRS_TBL_T   lut[YNRS_TBL_NUM];
    };
} ISP_NVRAM_YNRS_LUT_T;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//YNRS
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct {
    FIELD  YNRS_ENC                                                  :  1;      /*  0.. 0, 0x00000001 */
    FIELD  YNRS_ENY                                                  :  1;      /*  1.. 1, 0x00000002 */
    FIELD  rsv_2                                                     :  2;      /*  2.. 3, 0x0000000C */
    FIELD  YNRS_Y_FLT0_IDX                                           :  2;      /*  4.. 5, 0x00000030 */
    FIELD  rsv_6                                                     : 10;      /*  6..15, 0x0000FFC0 */
    FIELD  YNRS_TILE_EDGE                                            :  4;      /* 16..19, 0x000F0000 */
    FIELD  rsv_20                                                    :  4;      /* 20..23, 0x00F00000 */
    FIELD  YNRS_SL2_LINK                                             :  1;      /* 24..24, 0x01000000 */
    FIELD  rsv_25                                                    :  1;      /* 25..25, 0x02000000 */
    FIELD  YNRS_LBIT_MODE                                            :  1;      /* 26..26, 0x04000000 */
    FIELD  rsv_27                                                    :  1;      /* 27..27, 0x08000000 */
    FIELD  YNRS_TABLE_EN                                             :  1;      /* 28..28, 0x10000000 */
    FIELD  YNRS_TBL_PRC                                              :  1;      /* 29..29, 0x20000000 */
    FIELD  rsv_30                                                    :  2;      /* 30..31, 0xC0000000 */
} ISP_YNRS_CON1_T; // /* 0x1A013400 */

typedef union {
    typedef ISP_YNRS_CON1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNRS_CON1_T;

typedef struct {
    FIELD  YNRS_IMPL_MODE                                            :  2;      /*  0.. 1, 0x00000003 */
    FIELD  rsv_2                                                     :  6;      /*  2.. 7, 0x000000FC */
    FIELD  YNRS_C_SM_EDGE                                            :  2;      /*  8.. 9, 0x00000300 */
    FIELD  rsv_10                                                    : 14;      /* 10..23, 0x00FFFC00 */
    FIELD  YNRS_C_SM_EDGE_TH                                         :  3;      /* 24..26, 0x07000000 */
    FIELD  rsv_27                                                    :  5;      /* 27..31, 0xF8000000 */
} ISP_YNRS_CON2_T; // /* 0x1A013404 */

typedef union {
    typedef ISP_YNRS_CON2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNRS_CON2_T;

typedef struct {
    FIELD  rsv_0                                                     :  8;      /*  0.. 7, 0x000000FF */
    FIELD  YNRS_PTY_GAIN_TH                                          :  5;      /*  8..12, 0x00001F00 */
    FIELD  rsv_13                                                    : 19;      /* 13..31, 0xFFFFE000 */
} ISP_YNRS_YAD2_T; // /* 0x1A013408 */

typedef union {
    typedef ISP_YNRS_YAD2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNRS_YAD2_T;

typedef struct {
    FIELD  YNRS_Y_CPX1                                               :  8;      /*  0.. 7, 0x000000FF */
    FIELD  YNRS_Y_CPX2                                               :  8;      /*  8..15, 0x0000FF00 */
    FIELD  YNRS_Y_CPX3                                               :  8;      /* 16..23, 0x00FF0000 */
    FIELD  YNRS_Y_CPX4                                               :  8;      /* 24..31, 0xFF000000 */
} ISP_YNRS_Y4LUT1_T; // /* 0x1A01340C */

typedef union {
    typedef ISP_YNRS_Y4LUT1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNRS_Y4LUT1_T;

typedef struct {
    FIELD  YNRS_Y_SCALE_CPY0                                         :  5;      /*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                                     :  3;      /*  5.. 7, 0x000000E0 */
    FIELD  YNRS_Y_SCALE_CPY1                                         :  5;      /*  8..12, 0x00001F00 */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  YNRS_Y_SCALE_CPY2                                         :  5;      /* 16..20, 0x001F0000 */
    FIELD  rsv_21                                                    :  3;      /* 21..23, 0x00E00000 */
    FIELD  YNRS_Y_SCALE_CPY3                                         :  5;      /* 24..28, 0x1F000000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_YNRS_Y4LUT2_T; // /* 0x1A013410 */

typedef union {
    typedef ISP_YNRS_Y4LUT2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNRS_Y4LUT2_T;

typedef struct {
    FIELD  YNRS_Y_SCALE_SP0                                          :  5;      /*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                                     :  3;      /*  5.. 7, 0x000000E0 */
    FIELD  YNRS_Y_SCALE_SP1                                          :  5;      /*  8..12, 0x00001F00 */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  YNRS_Y_SCALE_SP2                                          :  5;      /* 16..20, 0x001F0000 */
    FIELD  rsv_21                                                    :  3;      /* 21..23, 0x00E00000 */
    FIELD  YNRS_Y_SCALE_SP3                                          :  5;      /* 24..28, 0x1F000000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_YNRS_Y4LUT3_T; // /* 0x1A013414 */

typedef union {
    typedef ISP_YNRS_Y4LUT3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNRS_Y4LUT3_T;

typedef struct {
    FIELD  YNRS_C_CPX1                                               :  8;      /*  0.. 7, 0x000000FF */
    FIELD  YNRS_C_CPX2                                               :  8;      /*  8..15, 0x0000FF00 */
    FIELD  YNRS_C_CPX3                                               :  8;      /* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                                    :  8;      /* 24..31, 0xFF000000 */
} ISP_YNRS_C4LUT1_T; // /* 0x1A013418 */

typedef union {
    typedef ISP_YNRS_C4LUT1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNRS_C4LUT1_T;

typedef struct {
    FIELD  YNRS_C_SCALE_CPY0                                         :  5;      /*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                                     :  3;      /*  5.. 7, 0x000000E0 */
    FIELD  YNRS_C_SCALE_CPY1                                         :  5;      /*  8..12, 0x00001F00 */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  YNRS_C_SCALE_CPY2                                         :  5;      /* 16..20, 0x001F0000 */
    FIELD  rsv_21                                                    :  3;      /* 21..23, 0x00E00000 */
    FIELD  YNRS_C_SCALE_CPY3                                         :  5;      /* 24..28, 0x1F000000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_YNRS_C4LUT2_T; // /* 0x1A01341C */

typedef union {
    typedef ISP_YNRS_C4LUT2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNRS_C4LUT2_T;

typedef struct {
    FIELD  YNRS_C_SCALE_SP0                                          :  5;      /*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                                     :  3;      /*  5.. 7, 0x000000E0 */
    FIELD  YNRS_C_SCALE_SP1                                          :  5;      /*  8..12, 0x00001F00 */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  YNRS_C_SCALE_SP2                                          :  5;      /* 16..20, 0x001F0000 */
    FIELD  rsv_21                                                    :  3;      /* 21..23, 0x00E00000 */
    FIELD  YNRS_C_SCALE_SP3                                          :  5;      /* 24..28, 0x1F000000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_YNRS_C4LUT3_T; // /* 0x1A013420 */

typedef union {
    typedef ISP_YNRS_C4LUT3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNRS_C4LUT3_T;

typedef struct {
    FIELD  YNRS_Y_ACT_CPY0                                           :  8;      /*  0.. 7, 0x000000FF */
    FIELD  YNRS_Y_ACT_CPY1                                           :  8;      /*  8..15, 0x0000FF00 */
    FIELD  YNRS_Y_ACT_CPY2                                           :  8;      /* 16..23, 0x00FF0000 */
    FIELD  YNRS_Y_ACT_CPY3                                           :  8;      /* 24..31, 0xFF000000 */
} ISP_YNRS_A4LUT2_T; // /* 0x1A013424 */

typedef union {
    typedef ISP_YNRS_A4LUT2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNRS_A4LUT2_T;

typedef struct {
    FIELD  YNRS_Y_ACT_SP0                                            :  6;      /*  0.. 5, 0x0000003F */
    FIELD  rsv_6                                                     :  2;      /*  6.. 7, 0x000000C0 */
    FIELD  YNRS_Y_ACT_SP1                                            :  6;      /*  8..13, 0x00003F00 */
    FIELD  rsv_14                                                    :  2;      /* 14..15, 0x0000C000 */
    FIELD  YNRS_Y_ACT_SP2                                            :  6;      /* 16..21, 0x003F0000 */
    FIELD  rsv_22                                                    :  2;      /* 22..23, 0x00C00000 */
    FIELD  YNRS_Y_ACT_SP3                                            :  6;      /* 24..29, 0x3F000000 */
    FIELD  rsv_30                                                    :  2;      /* 30..31, 0xC0000000 */
} ISP_YNRS_A4LUT3_T; // /* 0x1A013428 */

typedef union {
    typedef ISP_YNRS_A4LUT3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNRS_A4LUT3_T;

typedef struct {
    FIELD  YNRS_SL2_X1                                               :  8;      /*  0.. 7, 0x000000FF */
    FIELD  YNRS_SL2_X2                                               :  8;      /*  8..15, 0x0000FF00 */
    FIELD  YNRS_SL2_X3                                               :  8;      /* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                                    :  8;      /* 24..31, 0xFF000000 */
} ISP_YNRS_L4LUT1_T; // /* 0x1A01342C */

typedef union {
    typedef ISP_YNRS_L4LUT1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNRS_L4LUT1_T;

typedef struct {
    FIELD  YNRS_SL2_GAIN0                                            :  6;      /*  0.. 5, 0x0000003F */
    FIELD  rsv_6                                                     :  2;      /*  6.. 7, 0x000000C0 */
    FIELD  YNRS_SL2_GAIN1                                            :  6;      /*  8..13, 0x00003F00 */
    FIELD  rsv_14                                                    :  2;      /* 14..15, 0x0000C000 */
    FIELD  YNRS_SL2_GAIN2                                            :  6;      /* 16..21, 0x003F0000 */
    FIELD  rsv_22                                                    :  2;      /* 22..23, 0x00C00000 */
    FIELD  YNRS_SL2_GAIN3                                            :  6;      /* 24..29, 0x3F000000 */
    FIELD  rsv_30                                                    :  2;      /* 30..31, 0xC0000000 */
} ISP_YNRS_L4LUT2_T; // /* 0x1A013430 */

typedef union {
    typedef ISP_YNRS_L4LUT2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNRS_L4LUT2_T;

typedef struct {
    FIELD  YNRS_SL2_SP0                                              :  6;      /*  0.. 5, 0x0000003F */
    FIELD  rsv_6                                                     :  2;      /*  6.. 7, 0x000000C0 */
    FIELD  YNRS_SL2_SP1                                              :  6;      /*  8..13, 0x00003F00 */
    FIELD  rsv_14                                                    :  2;      /* 14..15, 0x0000C000 */
    FIELD  YNRS_SL2_SP2                                              :  6;      /* 16..21, 0x003F0000 */
    FIELD  rsv_22                                                    :  2;      /* 22..23, 0x00C00000 */
    FIELD  YNRS_SL2_SP3                                              :  6;      /* 24..29, 0x3F000000 */
    FIELD  rsv_30                                                    :  2;      /* 30..31, 0xC0000000 */
} ISP_YNRS_L4LUT3_T; // /* 0x1A013434 */

typedef union {
    typedef ISP_YNRS_L4LUT3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNRS_L4LUT3_T;

typedef struct {
    FIELD  YNRS_Y_L0_V_RNG1                                          :  8;      /*  0.. 7, 0x000000FF */
    FIELD  YNRS_Y_L0_V_RNG2                                          :  8;      /*  8..15, 0x0000FF00 */
    FIELD  YNRS_Y_L0_V_RNG3                                          :  8;      /* 16..23, 0x00FF0000 */
    FIELD  YNRS_Y_L0_V_RNG4                                          :  8;      /* 24..31, 0xFF000000 */
} ISP_YNRS_PTY0V_T; // /* 0x1A013438 */

typedef union {
    typedef ISP_YNRS_PTY0V_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNRS_PTY0V_T;

typedef struct {
    FIELD  rsv_0                                                     :  8;      /*  0.. 7, 0x000000FF */
    FIELD  YNRS_PTC_GAIN_TH                                          :  5;      /*  8..12, 0x00001F00 */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  YNRS_C_L_DIFF_TH                                          :  8;      /* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                                    :  8;      /* 24..31, 0xFF000000 */
} ISP_YNRS_CAD_T; // /* 0x1A01343C */

typedef union {
    typedef ISP_YNRS_CAD_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNRS_CAD_T;

typedef struct {
    FIELD  YNRS_SL2_C_GAIN                                           :  4;      /*  0.. 3, 0x0000000F */
    FIELD  YNRS_SL2_SCALE_GAIN                                       :  3;      /*  4.. 6, 0x00000070 */
    FIELD  rsv_7                                                     : 25;      /*  7..31, 0xFFFFFF80 */
} ISP_YNRS_SL2_T; // /* 0x1A013440 */

typedef union {
    typedef ISP_YNRS_SL2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNRS_SL2_T;

typedef struct {
    FIELD  YNRS_Y_L0_H_RNG1                                          :  8;      /*  0.. 7, 0x000000FF */
    FIELD  YNRS_Y_L0_H_RNG2                                          :  8;      /*  8..15, 0x0000FF00 */
    FIELD  YNRS_Y_L0_H_RNG3                                          :  8;      /* 16..23, 0x00FF0000 */
    FIELD  YNRS_Y_L0_H_RNG4                                          :  8;      /* 24..31, 0xFF000000 */
} ISP_YNRS_PTY0H_T; // /* 0x1A013444 */

typedef union {
    typedef ISP_YNRS_PTY0H_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNRS_PTY0H_T;

typedef struct {
    FIELD  YNRS_TBL_CPX1                                             :  8;      /*  0.. 7, 0x000000FF */
    FIELD  YNRS_TBL_CPX2                                             :  8;      /*  8..15, 0x0000FF00 */
    FIELD  YNRS_TBL_CPX3                                             :  8;      /* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                                    :  8;      /* 24..31, 0xFF000000 */
} ISP_YNRS_T4LUT1_T; // /* 0x1A013448 */

typedef union {
    typedef ISP_YNRS_T4LUT1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNRS_T4LUT1_T;

typedef struct {
    FIELD  YNRS_TBL_GAIN_CPY0                                        :  5;      /*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                                     :  3;      /*  5.. 7, 0x000000E0 */
    FIELD  YNRS_TBL_GAIN_CPY1                                        :  5;      /*  8..12, 0x00001F00 */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  YNRS_TBL_GAIN_CPY2                                        :  5;      /* 16..20, 0x001F0000 */
    FIELD  rsv_21                                                    :  3;      /* 21..23, 0x00E00000 */
    FIELD  YNRS_TBL_GAIN_CPY3                                        :  5;      /* 24..28, 0x1F000000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_YNRS_T4LUT2_T; // /* 0x1A01344C */

typedef union {
    typedef ISP_YNRS_T4LUT2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNRS_T4LUT2_T;

typedef struct {
    FIELD  YNRS_TBL_GAIN_SP0                                         :  5;      /*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                                     :  3;      /*  5.. 7, 0x000000E0 */
    FIELD  YNRS_TBL_GAIN_SP1                                         :  5;      /*  8..12, 0x00001F00 */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  YNRS_TBL_GAIN_SP2                                         :  5;      /* 16..20, 0x001F0000 */
    FIELD  rsv_21                                                    :  3;      /* 21..23, 0x00E00000 */
    FIELD  YNRS_TBL_GAIN_SP3                                         :  5;      /* 24..28, 0x1F000000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_YNRS_T4LUT3_T; // /* 0x1A013450 */

typedef union {
    typedef ISP_YNRS_T4LUT3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNRS_T4LUT3_T;

typedef struct {
    FIELD  YNRS_ACT_SL2_GAIN                                         :  4;      /*  0.. 3, 0x0000000F */
    FIELD  rsv_4                                                     :  4;      /*  4.. 7, 0x000000F0 */
    FIELD  YNRS_ACT_DIF_HI_TH                                        :  5;      /*  8..12, 0x00001F00 */
    FIELD  rsv_13                                                    :  7;      /* 13..19, 0x000FE000 */
    FIELD  YNRS_ACT_DIF_GAIN                                         :  4;      /* 20..23, 0x00F00000 */
    FIELD  YNRS_ACT_DIF_LO_TH                                        :  5;      /* 24..28, 0x1F000000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_YNRS_ACT1_T; // /* 0x1A013454 */

typedef union {
    typedef ISP_YNRS_ACT1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNRS_ACT1_T;

typedef struct {
    FIELD  YNRS_C_V_RNG1                                             :  8;      /*  0.. 7, 0x000000FF */
    FIELD  YNRS_C_V_RNG2                                             :  8;      /*  8..15, 0x0000FF00 */
    FIELD  YNRS_C_V_RNG3                                             :  8;      /* 16..23, 0x00FF0000 */
    FIELD  YNRS_C_V_RNG4                                             :  8;      /* 24..31, 0xFF000000 */
} ISP_YNRS_PTCV_T; // /* 0x1A013458 */

typedef union {
    typedef ISP_YNRS_PTCV_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNRS_PTCV_T;

typedef struct {
    FIELD  YNRS_Y_ACT_CEN_OFT                                        :  6;      /*  0.. 5, 0x0000003F */
    FIELD  rsv_6                                                     :  2;      /*  6.. 7, 0x000000C0 */
    FIELD  YNRS_Y_ACT_CEN_GAIN                                       :  6;      /*  8..13, 0x00003F00 */
    FIELD  rsv_14                                                    :  2;      /* 14..15, 0x0000C000 */
    FIELD  YNRS_Y_ACT_CEN_TH                                         :  6;      /* 16..21, 0x003F0000 */
    FIELD  rsv_22                                                    : 10;      /* 22..31, 0xFFC00000 */
} ISP_YNRS_ACT4_T; // /* 0x1A01345C */

typedef union {
    typedef ISP_YNRS_ACT4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNRS_ACT4_T;

typedef struct {
    FIELD  YNRS_C_H_RNG1                                             :  8;      /*  0.. 7, 0x000000FF */
    FIELD  YNRS_C_H_RNG2                                             :  8;      /*  8..15, 0x0000FF00 */
    FIELD  YNRS_C_H_RNG3                                             :  8;      /* 16..23, 0x00FF0000 */
    FIELD  YNRS_C_H_RNG4                                             :  8;      /* 24..31, 0xFF000000 */
} ISP_YNRS_PTCH_T; // /* 0x1A013460 */

typedef union {
    typedef ISP_YNRS_PTCH_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNRS_PTCH_T;

typedef struct {
    FIELD  YNRS_Y_HF_CORE_TH                                         :  8;      /*  0.. 7, 0x000000FF */
    FIELD  YNRS_Y_HF_CORE_SL                                         :  2;      /*  8.. 9, 0x00000300 */
    FIELD  rsv_10                                                    :  6;      /* 10..15, 0x0000FC00 */
    FIELD  YNRS_Y_HF_CLIP                                            :  8;      /* 16..23, 0x00FF0000 */
    FIELD  YNRS_Y_HF_BAL_MODE                                        :  2;      /* 24..25, 0x03000000 */
    FIELD  YNRS_Y_HF_BAL_STR                                         :  6;      /* 26..31, 0xFC000000 */
} ISP_YNRS_HF_COR_T; // /* 0x1A013464 */

typedef union {
    typedef ISP_YNRS_HF_COR_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNRS_HF_COR_T;

typedef struct {
    FIELD  YNRS_Y_HF_ACT_X1                                          :  8;      /*  0.. 7, 0x000000FF */
    FIELD  YNRS_Y_HF_ACT_X2                                          :  8;      /*  8..15, 0x0000FF00 */
    FIELD  YNRS_Y_HF_ACT_X3                                          :  8;      /* 16..23, 0x00FF0000 */
    FIELD  YNRS_Y_HF_ACT_X4                                          :  8;      /* 24..31, 0xFF000000 */
} ISP_YNRS_HF_ACT0_T; // /* 0x1A013468 */

typedef union {
    typedef ISP_YNRS_HF_ACT0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNRS_HF_ACT0_T;

typedef struct {
    FIELD  YNRS_Y_HF_ACT_Y0                                          :  7;      /*  0.. 6, 0x0000007F */
    FIELD  rsv_7                                                     :  1;      /*  7.. 7, 0x00000080 */
    FIELD  YNRS_Y_HF_ACT_Y1                                          :  7;      /*  8..14, 0x00007F00 */
    FIELD  rsv_15                                                    :  1;      /* 15..15, 0x00008000 */
    FIELD  YNRS_Y_HF_ACT_Y2                                          :  7;      /* 16..22, 0x007F0000 */
    FIELD  rsv_23                                                    :  1;      /* 23..23, 0x00800000 */
    FIELD  YNRS_Y_HF_ACT_Y3                                          :  7;      /* 24..30, 0x7F000000 */
    FIELD  rsv_31                                                    :  1;      /* 31..31, 0x80000000 */
} ISP_YNRS_HF_ACT1_T; // /* 0x1A01346C */

typedef union {
    typedef ISP_YNRS_HF_ACT1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNRS_HF_ACT1_T;

typedef struct {
    FIELD  YNRS_RSV                                                  :  8;      /*  0.. 7, 0x000000FF */
    FIELD  YNRS_ACT_BLD_BASE_C                                       :  7;      /*  8..14, 0x00007F00 */
    FIELD  rsv_15                                                    : 17;      /* 15..31, 0xFFFF8000 */
} ISP_YNRS_ACTC_T; // /* 0x1A013470 */

typedef union {
    typedef ISP_YNRS_ACTC_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNRS_ACTC_T;

typedef struct {
    FIELD  YNRS_CEN_GAIN_LO_TH_LPF                                   :  5;      /*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                                     :  3;      /*  5.. 7, 0x000000E0 */
    FIELD  YNRS_CEN_GAIN_HI_TH_LPF                                   :  5;      /*  8..12, 0x00001F00 */
    FIELD  rsv_13                                                    : 19;      /* 13..31, 0xFFFFE000 */
} ISP_YNRS_YLAD_T; // /* 0x1A013474 */

typedef union {
    typedef ISP_YNRS_YLAD_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNRS_YLAD_T;

typedef struct {
    FIELD  YNRS_Y_HF_ACT_Y4                                          :  7;      /*  0.. 6, 0x0000007F */
    FIELD  rsv_7                                                     :  1;      /*  7.. 7, 0x00000080 */
    FIELD  YNRS_Y_HF_ACT_SP4                                         :  6;      /*  8..13, 0x00003F00 */
    FIELD  rsv_14                                                    : 18;      /* 14..31, 0xFFFFC000 */
} ISP_YNRS_HF_ACT2_T; // /* 0x1A013478 */

typedef union {
    typedef ISP_YNRS_HF_ACT2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNRS_HF_ACT2_T;

typedef struct {
    FIELD  YNRS_Y_HF_ACT_SP0                                         :  6;      /*  0.. 5, 0x0000003F */
    FIELD  rsv_6                                                     :  2;      /*  6.. 7, 0x000000C0 */
    FIELD  YNRS_Y_HF_ACT_SP1                                         :  6;      /*  8..13, 0x00003F00 */
    FIELD  rsv_14                                                    :  2;      /* 14..15, 0x0000C000 */
    FIELD  YNRS_Y_HF_ACT_SP2                                         :  6;      /* 16..21, 0x003F0000 */
    FIELD  rsv_22                                                    :  2;      /* 22..23, 0x00C00000 */
    FIELD  YNRS_Y_HF_ACT_SP3                                         :  6;      /* 24..29, 0x3F000000 */
    FIELD  rsv_30                                                    :  2;      /* 30..31, 0xC0000000 */
} ISP_YNRS_HF_ACT3_T; // /* 0x1A01347C */

typedef union {
    typedef ISP_YNRS_HF_ACT3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNRS_HF_ACT3_T;

typedef struct {
    FIELD  YNRS_Y_HF_LUMA_X1                                         :  8;      /*  0.. 7, 0x000000FF */
    FIELD  YNRS_Y_HF_LUMA_X2                                         :  8;      /*  8..15, 0x0000FF00 */
    FIELD  YNRS_Y_HF_LUMA_Y0                                         :  7;      /* 16..22, 0x007F0000 */
    FIELD  rsv_23                                                    :  1;      /* 23..23, 0x00800000 */
    FIELD  YNRS_Y_HF_LUMA_Y1                                         :  7;      /* 24..30, 0x7F000000 */
    FIELD  rsv_31                                                    :  1;      /* 31..31, 0x80000000 */
} ISP_YNRS_HF_LUMA0_T; // /* 0x1A013480 */

typedef union {
    typedef ISP_YNRS_HF_LUMA0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNRS_HF_LUMA0_T;

typedef struct {
    FIELD  YNRS_Y_HF_LUMA_Y2                                         :  7;      /*  0.. 6, 0x0000007F */
    FIELD  rsv_7                                                     :  1;      /*  7.. 7, 0x00000080 */
    FIELD  YNRS_Y_HF_LUMA_SP0                                        :  6;      /*  8..13, 0x00003F00 */
    FIELD  rsv_14                                                    :  2;      /* 14..15, 0x0000C000 */
    FIELD  YNRS_Y_HF_LUMA_SP1                                        :  6;      /* 16..21, 0x003F0000 */
    FIELD  rsv_22                                                    :  2;      /* 22..23, 0x00C00000 */
    FIELD  YNRS_Y_HF_LUMA_SP2                                        :  6;      /* 24..29, 0x3F000000 */
    FIELD  rsv_30                                                    :  2;      /* 30..31, 0xC0000000 */
} ISP_YNRS_HF_LUMA1_T; // /* 0x1A013484 */

typedef union {
    typedef ISP_YNRS_HF_LUMA1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNRS_HF_LUMA1_T;

typedef struct {
    FIELD  YNRS_Y_CPX5                                               :  8;      /*  0.. 7, 0x000000FF */
    FIELD  YNRS_Y_CPX6                                               :  8;      /*  8..15, 0x0000FF00 */
    FIELD  YNRS_Y_CPX7                                               :  8;      /* 16..23, 0x00FF0000 */
    FIELD  YNRS_Y_CPX8                                               :  8;      /* 24..31, 0xFF000000 */
} ISP_YNRS_Y4LUT4_T; // /* 0x1A013488 */

typedef union {
    typedef ISP_YNRS_Y4LUT4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNRS_Y4LUT4_T;

typedef struct {
    FIELD  YNRS_Y_SCALE_CPY4                                         :  5;      /*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                                     :  3;      /*  5.. 7, 0x000000E0 */
    FIELD  YNRS_Y_SCALE_CPY5                                         :  5;      /*  8..12, 0x00001F00 */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  YNRS_Y_SCALE_CPY6                                         :  5;      /* 16..20, 0x001F0000 */
    FIELD  rsv_21                                                    :  3;      /* 21..23, 0x00E00000 */
    FIELD  YNRS_Y_SCALE_CPY7                                         :  5;      /* 24..28, 0x1F000000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_YNRS_Y4LUT5_T; // /* 0x1A01348C */

typedef union {
    typedef ISP_YNRS_Y4LUT5_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNRS_Y4LUT5_T;

typedef struct {
    FIELD  YNRS_Y_SCALE_SP4                                          :  5;      /*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                                     :  3;      /*  5.. 7, 0x000000E0 */
    FIELD  YNRS_Y_SCALE_SP5                                          :  5;      /*  8..12, 0x00001F00 */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  YNRS_Y_SCALE_SP6                                          :  5;      /* 16..20, 0x001F0000 */
    FIELD  rsv_21                                                    :  3;      /* 21..23, 0x00E00000 */
    FIELD  YNRS_Y_SCALE_SP7                                          :  5;      /* 24..28, 0x1F000000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_YNRS_Y4LUT6_T; // /* 0x1A013490 */

typedef union {
    typedef ISP_YNRS_Y4LUT6_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNRS_Y4LUT6_T;

typedef struct {
    FIELD  YNRS_Y_SCALE_CPY8                                         :  5;      /*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                                     :  3;      /*  5.. 7, 0x000000E0 */
    FIELD  YNRS_Y_SCALE_SP8                                          :  5;      /*  8..12, 0x00001F00 */
    FIELD  rsv_13                                                    : 19;      /* 13..31, 0xFFFFE000 */
} ISP_YNRS_Y4LUT7_T; // /* 0x1A013494 */

typedef union {
    typedef ISP_YNRS_Y4LUT7_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNRS_Y4LUT7_T;

typedef struct {
    FIELD  YNRS_Y_ACT_CPX1                                           :  8;      /*  0.. 7, 0x000000FF */
    FIELD  YNRS_Y_ACT_CPX2                                           :  8;      /*  8..15, 0x0000FF00 */
    FIELD  YNRS_Y_ACT_CPX3                                           :  8;      /* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                                    :  8;      /* 24..31, 0xFF000000 */
} ISP_YNRS_A4LUT1_T; // /* 0x1A013498 */

typedef union {
    typedef ISP_YNRS_A4LUT1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNRS_A4LUT1_T;

typedef struct {
    FIELD  YNRS_SRAM_PINGPONG                                        :  1;      /*  0.. 0, 0x00000001 */
    FIELD  rsv_1                                                     : 31;      /*  1..31, 0xFFFFFFFE */
} ISP_YNRS_SRAM_PINGPONG_T; // /* 0x1A01349C */

typedef union {
    typedef ISP_YNRS_SRAM_PINGPONG_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNRS_SRAM_PINGPONG_T;

typedef union {
    enum { COUNT = 39 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_YNRS_CON1_T  		    	con1;
        ISP_NVRAM_YNRS_CON2_T  		    	con2;
        ISP_NVRAM_YNRS_YAD2_T  		    	yad2;
        ISP_NVRAM_YNRS_Y4LUT1_T  			y4lut1;
        ISP_NVRAM_YNRS_Y4LUT2_T  			y4lut2;
        ISP_NVRAM_YNRS_Y4LUT3_T  			y4lut3;
        ISP_NVRAM_YNRS_C4LUT1_T  			c4lut1;
        ISP_NVRAM_YNRS_C4LUT2_T  			c4lut2;
        ISP_NVRAM_YNRS_C4LUT3_T  			c4lut3;
        ISP_NVRAM_YNRS_A4LUT2_T  			a4lut2;
        ISP_NVRAM_YNRS_A4LUT3_T  			a4lut3;
        ISP_NVRAM_YNRS_L4LUT1_T  			l4lut1;
        ISP_NVRAM_YNRS_L4LUT2_T  			l4lut2;
        ISP_NVRAM_YNRS_L4LUT3_T  			l4lut3;
        ISP_NVRAM_YNRS_PTY0V_T  			pty0v;
        ISP_NVRAM_YNRS_CAD_T  			    cad;
        ISP_NVRAM_YNRS_SL2_T  			    sl2;
        ISP_NVRAM_YNRS_PTY0H_T  			pty0h;
        ISP_NVRAM_YNRS_T4LUT1_T  			t4lut1;
        ISP_NVRAM_YNRS_T4LUT2_T  			t4lut2;
        ISP_NVRAM_YNRS_T4LUT3_T  			t4lut3;
        ISP_NVRAM_YNRS_ACT1_T  			    act1;
        ISP_NVRAM_YNRS_PTCV_T  			    ptcv;
        ISP_NVRAM_YNRS_ACT4_T  			    act4;
        ISP_NVRAM_YNRS_PTCH_T  			    ptch;
        ISP_NVRAM_YNRS_HF_COR_T  			hf_cor;
        ISP_NVRAM_YNRS_HF_ACT0_T  			hf_act0;
        ISP_NVRAM_YNRS_HF_ACT1_T  			hf_act1;
        ISP_NVRAM_YNRS_ACTC_T  			    actc;
        ISP_NVRAM_YNRS_YLAD_T  			    ylad;
        ISP_NVRAM_YNRS_HF_ACT2_T  			hf_act2;
        ISP_NVRAM_YNRS_HF_ACT3_T  			hf_act3;
        ISP_NVRAM_YNRS_HF_LUMA0_T  			hf_luma0;
        ISP_NVRAM_YNRS_HF_LUMA1_T  			hf_luma1;
        ISP_NVRAM_YNRS_Y4LUT4_T  			y4lut4;
        ISP_NVRAM_YNRS_Y4LUT5_T  			y4lut5;
        ISP_NVRAM_YNRS_Y4LUT6_T  			y4lut6;
        ISP_NVRAM_YNRS_Y4LUT7_T  			y4lut7;
        ISP_NVRAM_YNRS_A4LUT1_T  			a4lut1;
        //ISP_NVRAM_YNRS_SRAM_PINGPONG_T  	sram_pingpong;
    };
} ISP_NVRAM_YNRS_T;



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//LDNR
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef struct {
    FIELD  LDNR_RAT                                   :  9;		/*  0.. 8, 0x000001FF */
    FIELD  rsv_9                                      : 23;		/*  9..31, 0xFFFFFE00 */
} ISP_LDNR_CON1_T; //	/* 0x150236C0 */

typedef union {
    typedef ISP_LDNR_CON1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LDNR_CON1_T;

typedef struct {
    FIELD  LDNR_RAW_FMT                               :  2;		/*  0.. 1, 0x00000003 */
    FIELD  LDNR_LR_RAT                                :  4;		/*  2.. 5, 0x0000003C */
    FIELD  LDNR_ACT_G_EN                              :  1;		/*  6.. 6, 0x00000040 */
    FIELD  rsv_7                                      : 25;		/*  7..31, 0xFFFFFF80 */
} ISP_LDNR_EE_CON1_T; //	/* 0x150236C4 */

typedef union {
    typedef ISP_LDNR_EE_CON1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LDNR_EE_CON1_T;

typedef struct {
    FIELD  LDNR_LM_Y0                                 :  9;		/*  0.. 8, 0x000001FF */
    FIELD  LDNR_LM_Y1                                 :  9;		/*  9..17, 0x0003FE00 */
    FIELD  LDNR_LM_Y2                                 :  9;		/* 18..26, 0x07FC0000 */
    FIELD  rsv_27                                     :  5;		/* 27..31, 0xF8000000 */
} ISP_LDNR_EE_LM_Y012_T; //	/* 0x150236C8 */

typedef union {
    typedef ISP_LDNR_EE_LM_Y012_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LDNR_EE_LM_Y012_T;

typedef struct {
    FIELD  LDNR_LM_Y3                                 :  9;		/*  0.. 8, 0x000001FF */
    FIELD  LDNR_LM_Y4                                 :  9;		/*  9..17, 0x0003FE00 */
    FIELD  LDNR_LM_Y5                                 :  9;		/* 18..26, 0x07FC0000 */
    FIELD  rsv_27                                     :  5;		/* 27..31, 0xF8000000 */
} ISP_LDNR_EE_LM_Y345_T; //	/* 0x150236CC */

typedef union {
    typedef ISP_LDNR_EE_LM_Y345_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LDNR_EE_LM_Y345_T;

typedef struct {
    FIELD  LDNR_SL_L_Y1                               :  8;		/*  0.. 7, 0x000000FF */
    FIELD  LDNR_SL_L_Y2                               :  8;		/*  8..15, 0x0000FF00 */
    FIELD  LDNR_SL_HR                                 :  5;		/* 16..20, 0x001F0000 */
    FIELD  rsv_21                                     : 11;		/* 21..31, 0xFFE00000 */
} ISP_LDNR_EE_SL_T; //	/* 0x150236D0 */

typedef union {
    typedef ISP_LDNR_EE_SL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LDNR_EE_SL_T;

typedef struct {
    FIELD  LDNR_H1_LWB                                :  8;		/*  0.. 7, 0x000000FF */
    FIELD  LDNR_H2_LWB                                :  8;		/*  8..15, 0x0000FF00 */
    FIELD  LDNR_H3_LWB                                :  8;		/* 16..23, 0x00FF0000 */
    FIELD  LDNR_HSLL                                  :  4;		/* 24..27, 0x0F000000 */
    FIELD  rsv_28                                     :  4;		/* 28..31, 0xF0000000 */
} ISP_LDNR_EE_LWB_T; //	/* 0x150236D4 */

typedef union {
    typedef ISP_LDNR_EE_LWB_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LDNR_EE_LWB_T;

typedef struct {
    FIELD  LDNR_H1_UPB                                :  8;		/*  0.. 7, 0x000000FF */
    FIELD  LDNR_H2_UPB                                :  8;		/*  8..15, 0x0000FF00 */
    FIELD  LDNR_H3_UPB                                :  8;		/* 16..23, 0x00FF0000 */
    FIELD  LDNR_HSLR                                  :  4;		/* 24..27, 0x0F000000 */
    FIELD  rsv_28                                     :  4;		/* 28..31, 0xF0000000 */
} ISP_LDNR_EE_UPB_T; //	/* 0x150236D8 */

typedef union {
    typedef ISP_LDNR_EE_UPB_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LDNR_EE_UPB_T;

typedef struct {
    FIELD  LDNR_RNG1                                  :  6;		/*  0.. 5, 0x0000003F */
    FIELD  LDNR_RNG2                                  :  6;		/*  6..11, 0x00000FC0 */
    FIELD  LDNR_RNG3                                  :  6;		/* 12..17, 0x0003F000 */
    FIELD  LDNR_RNG4                                  :  6;		/* 18..23, 0x00FC0000 */
    FIELD  rsv_24                                     :  8;		/* 24..31, 0xFF000000 */
} ISP_LDNR_RNG_T; //	/* 0x150236DC */

typedef union {
    typedef ISP_LDNR_RNG_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LDNR_RNG_T;

typedef struct {
    FIELD  LDNR_MED_RAT                               :  7;		/*  0.. 6, 0x0000007F */
    FIELD  LDNR_PRF_RAT                               :  7;		/*  7..13, 0x00003F80 */
    FIELD  LDNR_RAD                                   :  2;		/* 14..15, 0x0000C000 */
    FIELD  rsv_16                                     :  1;		/* 16..16, 0x00010000 */
    FIELD  LDNR_CTHR                                  :  6;		/* 17..22, 0x007E0000 */
    FIELD  LDNR_CTHL                                  :  6;		/* 23..28, 0x1F800000 */
    FIELD  rsv_29                                     :  3;		/* 29..31, 0xE0000000 */
} ISP_LDNR_CON2_T; //	/* 0x150236E0 */

typedef union {
    typedef ISP_LDNR_CON2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LDNR_CON2_T;

typedef struct {
    FIELD  LDNR_SL_EN                                 :  1;		/*  0.. 0, 0x00000001 */
    FIELD  LDNR_SL_Y2                                 : 10;		/*  1..10, 0x000007FE */
    FIELD  LDNR_SL_Y1                                 : 10;		/* 11..20, 0x001FF800 */
    FIELD  rsv_21                                     : 11;		/* 21..31, 0xFFE00000 */
} ISP_LDNR_SL_T; //	/* 0x150236E8 */

typedef union {
    typedef ISP_LDNR_SL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LDNR_SL_T;

typedef struct {
    FIELD  LDNR_SSL_C2                                :  8;		/*  0.. 7, 0x000000FF */
    FIELD  LDNR_STH_C2                                :  8;		/*  8..15, 0x0000FF00 */
    FIELD  LDNR_SSL_C1                                :  8;		/* 16..23, 0x00FF0000 */
    FIELD  LDNR_STH_C1                                :  8;		/* 24..31, 0xFF000000 */
} ISP_LDNR_SSL_STH_T; //	/* 0x150236EC */

typedef union {
    typedef ISP_LDNR_SSL_STH_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LDNR_SSL_STH_T;

typedef struct {
    FIELD  LDNR_TILE_EDGE                             :  4;		/*  0.. 3, 0x0000000F */
    FIELD  rsv_4                                      : 28;		/*  4..31, 0xFFFFFFF0 */
} ISP_LDNR_TILE_EDGE_T; //	/* 0x150236F0 */

typedef union {
    typedef ISP_LDNR_TILE_EDGE_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LDNR_TILE_EDGE_T;

typedef struct {
    FIELD  LDNR_HA_STR                                :  5;		/*  0.. 4, 0x0000001F */
    FIELD  LDNR_H1_GN                                 :  5;		/*  5.. 9, 0x000003E0 */
    FIELD  LDNR_H2_GN                                 :  5;		/* 10..14, 0x00007C00 */
    FIELD  LDNR_H3_GN                                 :  5;		/* 15..19, 0x000F8000 */
    FIELD  rsv_20                                     : 12;		/* 20..31, 0xFFF00000 */
} ISP_LDNR_EE_CON2_T; //	/* 0x150236F4 */

typedef union {
    typedef ISP_LDNR_EE_CON2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LDNR_EE_CON2_T;

typedef struct {
    FIELD  LDNR_CORE_TH                               :  8;		/*  0.. 7, 0x000000FF */
    FIELD  LDNR_CLIP_TH                               :  8;		/*  8..15, 0x0000FF00 */
    FIELD  LDNR_OV_TH                                 :  8;		/* 16..23, 0x00FF0000 */
    FIELD  LDNR_UN_TH                                 :  8;		/* 24..31, 0xFF000000 */
} ISP_LDNR_EE_TH_T; //	/* 0x150236F8 */

typedef union {
    typedef ISP_LDNR_EE_TH_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LDNR_EE_TH_T;

typedef struct {
    FIELD  LDNR_HPOS_GN                               :  5;		/*  0.. 4, 0x0000001F */
    FIELD  LDNR_HNEG_GN                               :  5;		/*  5.. 9, 0x000003E0 */
    FIELD  rsv_10                                     : 22;		/* 10..31, 0xFFFFFC00 */
} ISP_LDNR_EE_HGN_T; //	/* 0x150236FC */

typedef union {
    typedef ISP_LDNR_EE_HGN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LDNR_EE_HGN_T;

typedef struct {
    FIELD  LDNR_CNV_00                                : 11;		/*  0..10, 0x000007FF */
    FIELD  LDNR_CNV_01                                : 11;		/* 11..21, 0x003FF800 */
    FIELD  rsv_22                                     : 10;		/* 22..31, 0xFFC00000 */
} ISP_LDNR_CNV_00_01_T; //	/* 0x15023708 */

typedef union {
    typedef ISP_LDNR_CNV_00_01_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LDNR_CNV_00_01_T;

typedef struct {
    FIELD  LDNR_CNV_02                                : 11;		/*  0..10, 0x000007FF */
    FIELD  rsv_11                                     : 21;		/* 11..31, 0xFFFFF800 */
} ISP_LDNR_CNV_02_T; //	/* 0x1502370C */

typedef union {
    typedef ISP_LDNR_CNV_02_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LDNR_CNV_02_T;

typedef struct {
    FIELD  LDNR_CNV_10                                : 11;		/*  0..10, 0x000007FF */
    FIELD  LDNR_CNV_11                                : 11;		/* 11..21, 0x003FF800 */
    FIELD  rsv_22                                     : 10;		/* 22..31, 0xFFC00000 */
} ISP_LDNR_CNV_10_11_T; //	/* 0x15023710 */

typedef union {
    typedef ISP_LDNR_CNV_10_11_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LDNR_CNV_10_11_T;

typedef struct {
    FIELD  LDNR_CNV_12                                : 11;		/*  0..10, 0x000007FF */
    FIELD  rsv_11                                     : 21;		/* 11..31, 0xFFFFF800 */
} ISP_LDNR_CNV_12_T; //	/* 0x15023714 */

typedef union {
    typedef ISP_LDNR_CNV_12_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LDNR_CNV_12_T;

typedef struct {
    FIELD  LDNR_CNV_20                                : 11;		/*  0..10, 0x000007FF */
    FIELD  LDNR_CNV_21                                : 11;		/* 11..21, 0x003FF800 */
    FIELD  rsv_22                                     : 10;		/* 22..31, 0xFFC00000 */
} ISP_LDNR_CNV_20_21_T; //	/* 0x15023718 */

typedef union {
    typedef ISP_LDNR_CNV_20_21_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LDNR_CNV_20_21_T;

typedef struct {
    FIELD  LDNR_CNV_22                                : 11;		/*  0..10, 0x000007FF */
    FIELD  rsv_11                                     : 21;		/* 11..31, 0xFFFFF800 */
} ISP_LDNR_CNV_22_T; //	/* 0x1502371C */

typedef union {
    typedef ISP_LDNR_CNV_22_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LDNR_CNV_22_T;

typedef struct {
    FIELD  LDNR_ICNV_00                               : 11;		/*  0..10, 0x000007FF */
    FIELD  LDNR_ICNV_01                               : 11;		/* 11..21, 0x003FF800 */
    FIELD  rsv_22                                     : 10;		/* 22..31, 0xFFC00000 */
} ISP_LDNR_ICNV_00_01_T; //	/* 0x15023720 */

typedef union {
    typedef ISP_LDNR_ICNV_00_01_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LDNR_ICNV_00_01_T;

typedef struct {
    FIELD  LDNR_ICNV_02                               : 11;		/*  0..10, 0x000007FF */
    FIELD  rsv_11                                     : 21;		/* 11..31, 0xFFFFF800 */
} ISP_LDNR_ICNV_02_T; //	/* 0x15023724 */

typedef union {
    typedef ISP_LDNR_ICNV_02_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LDNR_ICNV_02_T;

typedef struct {
    FIELD  LDNR_ICNV_10                               : 11;		/*  0..10, 0x000007FF */
    FIELD  LDNR_ICNV_11                               : 11;		/* 11..21, 0x003FF800 */
    FIELD  rsv_22                                     : 10;		/* 22..31, 0xFFC00000 */
} ISP_LDNR_ICNV_10_11_T; //	/* 0x15023728 */

typedef union {
    typedef ISP_LDNR_ICNV_10_11_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LDNR_ICNV_10_11_T;

typedef struct {
    FIELD  LDNR_ICNV_12                               : 11;		/*  0..10, 0x000007FF */
    FIELD  rsv_11                                     : 21;		/* 11..31, 0xFFFFF800 */
} ISP_LDNR_ICNV_12_T; //	/* 0x1502372C */

typedef union {
    typedef ISP_LDNR_ICNV_12_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LDNR_ICNV_12_T;

typedef struct {
    FIELD  LDNR_ICNV_20                               : 11;		/*  0..10, 0x000007FF */
    FIELD  LDNR_ICNV_21                               : 11;		/* 11..21, 0x003FF800 */
    FIELD  rsv_22                                     : 10;		/* 22..31, 0xFFC00000 */
} ISP_LDNR_ICNV_20_21_T; //	/* 0x15023730 */

typedef union {
    typedef ISP_LDNR_ICNV_20_21_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LDNR_ICNV_20_21_T;

typedef struct {
    FIELD  LDNR_ICNV_22                               : 11;		/*  0..10, 0x000007FF */
    FIELD  rsv_11                                     : 21;		/* 11..31, 0xFFFFF800 */
} ISP_LDNR_ICNV_22_T; //	/* 0x15023734 */

typedef union {
    typedef ISP_LDNR_ICNV_22_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LDNR_ICNV_22_T;

typedef struct {
    FIELD  LDNR_COEF_R_C1                             :  8;		/*  0.. 7, 0x000000FF */
    FIELD  LDNR_COEF_G_C1                             :  9;		/*  8..16, 0x0001FF00 */
    FIELD  LDNR_COEF_B_C1                             :  8;		/* 17..24, 0x01FE0000 */
    FIELD  rsv_25                                     :  7;		/* 25..31, 0xFE000000 */
} ISP_LDNR_COEF_C1_T; //	/* 0x15023738 */

typedef union {
    typedef ISP_LDNR_COEF_C1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LDNR_COEF_C1_T;

typedef struct {
    FIELD  LDNR_COEF_R_C2                             :  8;		/*  0.. 7, 0x000000FF */
    FIELD  LDNR_COEF_G_C2                             :  9;		/*  8..16, 0x0001FF00 */
    FIELD  LDNR_COEF_B_C2                             :  8;		/* 17..24, 0x01FE0000 */
    FIELD  rsv_25                                     :  7;		/* 25..31, 0xFE000000 */
} ISP_LDNR_COEF_C2_T; //	/* 0x1502373C */

typedef union {
    typedef ISP_LDNR_COEF_C2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LDNR_COEF_C2_T;

typedef struct {
    FIELD  LDNR_NM_OFST_B                             : 11;		/*  0..10, 0x000007FF */
    FIELD  LDNR_NM_SL_B                               : 10;		/* 11..20, 0x001FF800 */
    FIELD  rsv_21                                     : 11;		/* 21..31, 0xFFE00000 */
} ISP_LDNR_NM_B_T; //	/* 0x15023740 */

typedef union {
    typedef ISP_LDNR_NM_B_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LDNR_NM_B_T;

typedef struct {
    FIELD  LDNR_NM_OFST_G                             : 11;		/*  0..10, 0x000007FF */
    FIELD  LDNR_NM_SL_G                               : 10;		/* 11..20, 0x001FF800 */
    FIELD  rsv_21                                     : 11;		/* 21..31, 0xFFE00000 */
} ISP_LDNR_NM_G_T; //	/* 0x15023744 */

typedef union {
    typedef ISP_LDNR_NM_G_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LDNR_NM_G_T;

typedef struct {
    FIELD  LDNR_NM_OFST_R                             : 11;		/*  0..10, 0x000007FF */
    FIELD  LDNR_NM_SL_R                               : 10;		/* 11..20, 0x001FF800 */
    FIELD  rsv_21                                     : 11;		/* 21..31, 0xFFE00000 */
} ISP_LDNR_NM_R_T; //	/* 0x1502374C */

typedef union {
    typedef ISP_LDNR_NM_R_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LDNR_NM_R_T;

typedef union {
    enum { COUNT = 31 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_LDNR_CON1_T  			con1;
        ISP_NVRAM_LDNR_EE_CON1_T  		ee_con1;
        ISP_NVRAM_LDNR_EE_LM_Y012_T  	ee_lm_y012;
        ISP_NVRAM_LDNR_EE_LM_Y345_T  	ee_lm_y345;
        ISP_NVRAM_LDNR_EE_SL_T  		ee_sl;
        ISP_NVRAM_LDNR_EE_LWB_T  		ee_lwb;
        ISP_NVRAM_LDNR_EE_UPB_T  		ee_upb;
        ISP_NVRAM_LDNR_RNG_T  			rng;
        ISP_NVRAM_LDNR_CON2_T  			con2;
        ISP_NVRAM_LDNR_SL_T  			sl;
        ISP_NVRAM_LDNR_SSL_STH_T  		ssl_sth;
        //ISP_NVRAM_LDNR_TILE_EDGE_T  	tile_edge;
        ISP_NVRAM_LDNR_EE_CON2_T  		ee_con2;
        ISP_NVRAM_LDNR_EE_TH_T  		ee_th;
        ISP_NVRAM_LDNR_EE_HGN_T  		ee_hgn;
        ISP_NVRAM_LDNR_CNV_00_01_T  	cnv_00_01;
        ISP_NVRAM_LDNR_CNV_02_T  		cnv_02;
        ISP_NVRAM_LDNR_CNV_10_11_T  	cnv_10_11;
        ISP_NVRAM_LDNR_CNV_12_T  		cnv_12;
        ISP_NVRAM_LDNR_CNV_20_21_T  	cnv_20_21;
        ISP_NVRAM_LDNR_CNV_22_T  		cnv_22;
        ISP_NVRAM_LDNR_ICNV_00_01_T  	icnv_00_01;
        ISP_NVRAM_LDNR_ICNV_02_T  		icnv_02;
        ISP_NVRAM_LDNR_ICNV_10_11_T  	icnv_10_11;
        ISP_NVRAM_LDNR_ICNV_12_T  		icnv_12;
        ISP_NVRAM_LDNR_ICNV_20_21_T  	icnv_20_21;
        ISP_NVRAM_LDNR_ICNV_22_T  		icnv_22;
        ISP_NVRAM_LDNR_COEF_C1_T  		coef_c1;
        ISP_NVRAM_LDNR_COEF_C2_T  		coef_c2;
        ISP_NVRAM_LDNR_NM_B_T  			nm_b;
        ISP_NVRAM_LDNR_NM_G_T  			nm_g;
        ISP_NVRAM_LDNR_NM_R_T  			nm_r;
    };
} ISP_NVRAM_LDNR_T;
#if 0
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// G2C
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef struct {
    FIELD  G2C_CNV_00                                : 11;		/*  0..10, 0x000007FF */
    FIELD  rsv_11                                     :  5;		/* 11..15, 0x0000F800 */
    FIELD  G2C_CNV_01                                : 11;		/* 16..26, 0x07FF0000 */
    FIELD  rsv_27                                     :  5;		/* 27..31, 0xF8000000 */
} ISP_G2C_CONV_0A_T; //	/* 0x15025200 */

typedef union {
    typedef ISP_G2C_CONV_0A_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2C_CONV_0A_T;

typedef struct {
    FIELD  G2C_CNV_02                                : 11;		/*  0..10, 0x000007FF */
    FIELD  rsv_11                                     :  5;		/* 11..15, 0x0000F800 */
    FIELD  G2C_Y_OFST                                : 11;		/* 16..26, 0x07FF0000 */
    FIELD  rsv_27                                     :  5;		/* 27..31, 0xF8000000 */
} ISP_G2C_CONV_0B_T; //	/* 0x15025204 */

typedef union {
    typedef ISP_G2C_CONV_0B_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2C_CONV_0B_T;

typedef struct {
    FIELD  G2C_CNV_10                                : 11;		/*  0..10, 0x000007FF */
    FIELD  rsv_11                                     :  5;		/* 11..15, 0x0000F800 */
    FIELD  G2C_CNV_11                                : 11;		/* 16..26, 0x07FF0000 */
    FIELD  rsv_27                                     :  5;		/* 27..31, 0xF8000000 */
} ISP_G2C_CONV_1A_T; //	/* 0x15025208 */

typedef union {
    typedef ISP_G2C_CONV_1A_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2C_CONV_1A_T;

typedef struct {
    FIELD  G2C_CNV_12                                : 11;		/*  0..10, 0x000007FF */
    FIELD  rsv_11                                     :  5;		/* 11..15, 0x0000F800 */
    FIELD  G2C_U_OFST                                : 10;		/* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                     :  6;		/* 26..31, 0xFC000000 */
} ISP_G2C_CONV_1B_T; //	/* 0x1502520C */

typedef union {
    typedef ISP_G2C_CONV_1B_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2C_CONV_1B_T;

typedef struct {
    FIELD  G2C_CNV_20                                : 11;		/*  0..10, 0x000007FF */
    FIELD  rsv_11                                     :  5;		/* 11..15, 0x0000F800 */
    FIELD  G2C_CNV_21                                : 11;		/* 16..26, 0x07FF0000 */
    FIELD  rsv_27                                     :  5;		/* 27..31, 0xF8000000 */
} ISP_G2C_CONV_2A_T; //	/* 0x15025210 */

typedef union {
    typedef ISP_G2C_CONV_2A_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2C_CONV_2A_T;

typedef struct {
    FIELD  G2C_CNV_22                                : 11;		/*  0..10, 0x000007FF */
    FIELD  rsv_11                                     :  5;		/* 11..15, 0x0000F800 */
    FIELD  G2C_V_OFST                                : 10;		/* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                     :  6;		/* 26..31, 0xFC000000 */
} ISP_G2C_CONV_2B_T; //	/* 0x15025214 */

typedef union {
    typedef ISP_G2C_CONV_2B_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2C_CONV_2B_T;

typedef union {
    enum { COUNT = 6 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_G2C_CONV_0A_T  			conv_0a;
        ISP_NVRAM_G2C_CONV_0B_T  			conv_0b;
        ISP_NVRAM_G2C_CONV_1A_T  			conv_1a;
        ISP_NVRAM_G2C_CONV_1B_T  			conv_1b;
        ISP_NVRAM_G2C_CONV_2A_T  			conv_2a;
        ISP_NVRAM_G2C_CONV_2B_T  			conv_2b;
    };
} ISP_NVRAM_G2C_T;
#endif
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// G2CX
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef struct {
    FIELD  G2CX_CNV_00                                : 11;		/*  0..10, 0x000007FF */
    FIELD  rsv_11                                     :  5;		/* 11..15, 0x0000F800 */
    FIELD  G2CX_CNV_01                                : 11;		/* 16..26, 0x07FF0000 */
    FIELD  rsv_27                                     :  5;		/* 27..31, 0xF8000000 */
} ISP_G2CX_CONV_0A_T; //	/* 0x15025200 */

typedef union {
    typedef ISP_G2CX_CONV_0A_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2CX_CONV_0A_T;

typedef struct {
    FIELD  G2CX_CNV_02                                : 11;		/*  0..10, 0x000007FF */
    FIELD  rsv_11                                     :  5;		/* 11..15, 0x0000F800 */
    FIELD  G2CX_Y_OFST                                : 11;		/* 16..26, 0x07FF0000 */
    FIELD  rsv_27                                     :  5;		/* 27..31, 0xF8000000 */
} ISP_G2CX_CONV_0B_T; //	/* 0x15025204 */

typedef union {
    typedef ISP_G2CX_CONV_0B_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2CX_CONV_0B_T;

typedef struct {
    FIELD  G2CX_CNV_10                                : 11;		/*  0..10, 0x000007FF */
    FIELD  rsv_11                                     :  5;		/* 11..15, 0x0000F800 */
    FIELD  G2CX_CNV_11                                : 11;		/* 16..26, 0x07FF0000 */
    FIELD  rsv_27                                     :  5;		/* 27..31, 0xF8000000 */
} ISP_G2CX_CONV_1A_T; //	/* 0x15025208 */

typedef union {
    typedef ISP_G2CX_CONV_1A_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2CX_CONV_1A_T;

typedef struct {
    FIELD  G2CX_CNV_12                                : 11;		/*  0..10, 0x000007FF */
    FIELD  rsv_11                                     :  5;		/* 11..15, 0x0000F800 */
    FIELD  G2CX_U_OFST                                : 10;		/* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                     :  6;		/* 26..31, 0xFC000000 */
} ISP_G2CX_CONV_1B_T; //	/* 0x1502520C */

typedef union {
    typedef ISP_G2CX_CONV_1B_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2CX_CONV_1B_T;

typedef struct {
    FIELD  G2CX_CNV_20                                : 11;		/*  0..10, 0x000007FF */
    FIELD  rsv_11                                     :  5;		/* 11..15, 0x0000F800 */
    FIELD  G2CX_CNV_21                                : 11;		/* 16..26, 0x07FF0000 */
    FIELD  rsv_27                                     :  5;		/* 27..31, 0xF8000000 */
} ISP_G2CX_CONV_2A_T; //	/* 0x15025210 */

typedef union {
    typedef ISP_G2CX_CONV_2A_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2CX_CONV_2A_T;

typedef struct {
    FIELD  G2CX_CNV_22                                : 11;		/*  0..10, 0x000007FF */
    FIELD  rsv_11                                     :  5;		/* 11..15, 0x0000F800 */
    FIELD  G2CX_V_OFST                                : 10;		/* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                     :  6;		/* 26..31, 0xFC000000 */
} ISP_G2CX_CONV_2B_T; //	/* 0x15025214 */

typedef union {
    typedef ISP_G2CX_CONV_2B_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2CX_CONV_2B_T;

typedef struct {
    FIELD  G2CX_SHADE_VAR                             : 18;		/*  0..17, 0x0003FFFF */
    FIELD  G2CX_SHADE_P0                              : 11;		/* 18..28, 0x1FFC0000 */
    FIELD  G2CX_SHADE_EN                              :  1;		/* 29..29, 0x20000000 */
    FIELD  rsv_30                                     :  2;		/* 30..31, 0xC0000000 */
} ISP_G2CX_SHADE_CON_1_T; //	/* 0x15025218 */
#if 0
typedef union {
    typedef ISP_G2CX_SHADE_CON_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2CX_SHADE_CON_1_T;

typedef struct {
    FIELD  G2CX_SHADE_P1                              : 11;		/*  0..10, 0x000007FF */
    FIELD  rsv_11                                     :  1;		/* 11..11, 0x00000800 */
    FIELD  G2CX_SHADE_P2                              : 11;		/* 12..22, 0x007FF000 */
    FIELD  rsv_23                                     :  9;		/* 23..31, 0xFF800000 */
} ISP_G2CX_SHADE_CON_2_T; //	/* 0x1502521C */

typedef union {
    typedef ISP_G2CX_SHADE_CON_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2CX_SHADE_CON_2_T;

typedef struct {
    FIELD  G2CX_SHADE_UB                              :  9;		/*  0.. 8, 0x000001FF */
    FIELD  rsv_9                                      : 23;		/*  9..31, 0xFFFFFE00 */
} ISP_G2CX_SHADE_CON_3_T; //	/* 0x15025220 */

typedef union {
    typedef ISP_G2CX_SHADE_CON_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2CX_SHADE_CON_3_T;

typedef struct {
    FIELD  G2CX_SHADE_XMID                            : 14;		/*  0..13, 0x00003FFF */
    FIELD  rsv_14                                     :  2;		/* 14..15, 0x0000C000 */
    FIELD  G2CX_SHADE_YMID                            : 14;		/* 16..29, 0x3FFF0000 */
    FIELD  rsv_30                                     :  2;		/* 30..31, 0xC0000000 */
} ISP_G2CX_SHADE_TAR_T; //	/* 0x15025224 */

typedef union {
    typedef ISP_G2CX_SHADE_TAR_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2CX_SHADE_TAR_T;

typedef struct {
    FIELD  G2CX_SHADE_XSP                             : 14;		/*  0..13, 0x00003FFF */
    FIELD  rsv_14                                     :  2;		/* 14..15, 0x0000C000 */
    FIELD  G2CX_SHADE_YSP                             : 14;		/* 16..29, 0x3FFF0000 */
    FIELD  rsv_30                                     :  2;		/* 30..31, 0xC0000000 */
} ISP_G2CX_SHADE_SP_T; //	/* 0x15025228 */

typedef union {
    typedef ISP_G2CX_SHADE_SP_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2CX_SHADE_SP_T;

typedef struct {
    FIELD  G2CX_CFC_EN                                :  1;		/*  0.. 0, 0x00000001 */
    FIELD  rsv_1                                      :  3;		/*  1.. 3, 0x0000000E */
    FIELD  G2CX_L                                     : 10;		/*  4..13, 0x00003FF0 */
    FIELD  rsv_14                                     :  2;		/* 14..15, 0x0000C000 */
    FIELD  G2CX_H                                     : 10;		/* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                     :  6;		/* 26..31, 0xFC000000 */
} ISP_G2CX_CFC_CON_1_T; //	/* 0x1502522C */

typedef union {
    typedef ISP_G2CX_CFC_CON_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2CX_CFC_CON_1_T;

typedef struct {
    FIELD  G2CX_HR                                    :  5;		/*  0.. 4, 0x0000001F */
    FIELD  G2CX_LR                                    :  5;		/*  5.. 9, 0x000003E0 */
    FIELD  G2CX_HG                                    :  5;		/* 10..14, 0x00007C00 */
    FIELD  G2CX_LG                                    :  5;		/* 15..19, 0x000F8000 */
    FIELD  G2CX_HB                                    :  5;		/* 20..24, 0x01F00000 */
    FIELD  G2CX_LB                                    :  5;		/* 25..29, 0x3E000000 */
    FIELD  rsv_30                                     :  2;		/* 30..31, 0xC0000000 */
} ISP_G2CX_CFC_CON_2_T; //	/* 0x15025230 */

typedef union {
    typedef ISP_G2CX_CFC_CON_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2CX_CFC_CON_2_T;
#endif

typedef union {
    enum { COUNT = 6 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_G2CX_CONV_0A_T  			conv_0a;
        ISP_NVRAM_G2CX_CONV_0B_T  			conv_0b;
        ISP_NVRAM_G2CX_CONV_1A_T  			conv_1a;
        ISP_NVRAM_G2CX_CONV_1B_T  			conv_1b;
        ISP_NVRAM_G2CX_CONV_2A_T  			conv_2a;
        ISP_NVRAM_G2CX_CONV_2B_T  			conv_2b;
        //ISP_NVRAM_G2CX_SHADE_CON_1_T  		shade_con_1;
        //ISP_NVRAM_G2CX_SHADE_CON_2_T  		shade_con_2;
        //ISP_NVRAM_G2CX_SHADE_CON_3_T  		shade_con_3;
        //ISP_NVRAM_G2CX_SHADE_TAR_T  		shade_tar;
        //ISP_NVRAM_G2CX_SHADE_SP_T  			shade_sp;
        //ISP_NVRAM_G2CX_CFC_CON_1_T  		cfc_con_1;
        //ISP_NVRAM_G2CX_CFC_CON_2_T  		cfc_con_2;
    };
} ISP_NVRAM_G2CX_T;
#if 0
typedef union {
    enum { COUNT = 5 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_G2CX_SHADE_CON_1_T  		shade_con_1;
        ISP_NVRAM_G2CX_SHADE_CON_2_T  		shade_con_2;
        ISP_NVRAM_G2CX_SHADE_CON_3_T  		shade_con_3;
        ISP_NVRAM_G2CX_SHADE_TAR_T  		shade_tar;
        ISP_NVRAM_G2CX_SHADE_SP_T  			shade_sp;
    };
} ISP_NVRAM_G2CX_SHADE_T;
#endif

#if 0
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// NDG
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef struct {
    FIELD  NDG_NOISE_GSEED                            : 16;		/*  0..15, 0x0000FFFF */
    FIELD  NDG_NOISE_SEED0                            : 16;		/* 16..31, 0xFFFF0000 */
} ISP_NDG_RAN_0_T; //	/* 0x15025900 */

typedef union {
    typedef ISP_NDG_RAN_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NDG_RAN_0_T;

typedef struct {
    FIELD  NDG_X_OFST                                 : 16;		/*  0..15, 0x0000FFFF */
    FIELD  NDG_Y_OFST                                 : 16;		/* 16..31, 0xFFFF0000 */
} ISP_NDG_RAN_1_T; //	/* 0x15025904 */

typedef union {
    typedef ISP_NDG_RAN_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NDG_RAN_1_T;

typedef struct {
    FIELD  NDG_IMAGE_WD                               : 16;		/*  0..15, 0x0000FFFF */
    FIELD  rsv_16                                     : 16;		/* 16..31, 0xFFFF0000 */
} ISP_NDG_RAN_2_T; //	/* 0x15025908 */

typedef union {
    typedef ISP_NDG_RAN_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NDG_RAN_2_T;

typedef struct {
    FIELD  NDG_TILE_WD                                : 16;		/*  0..15, 0x0000FFFF */
    FIELD  NDG_TILE_HT                                : 16;		/* 16..31, 0xFFFF0000 */
} ISP_NDG_RAN_3_T; //	/* 0x1502590C */

typedef union {
    typedef ISP_NDG_RAN_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NDG_RAN_3_T;

typedef struct {
    FIELD  NDG_CROP_XSTART                            : 16;		/*  0..15, 0x0000FFFF */
    FIELD  NDG_CROP_XEND                              : 16;		/* 16..31, 0xFFFF0000 */
} ISP_NDG_CROP_X_T; //	/* 0x15025910 */

typedef union {
    typedef ISP_NDG_CROP_X_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NDG_CROP_X_T;

typedef struct {
    FIELD  NDG_CROP_YSTART                            : 16;		/*  0..15, 0x0000FFFF */
    FIELD  NDG_CROP_YEND                              : 16;		/* 16..31, 0xFFFF0000 */
} ISP_NDG_CROP_Y_T; //	/* 0x15025914 */

typedef union {
    typedef ISP_NDG_CROP_Y_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NDG_CROP_Y_T;

typedef union {
    enum { COUNT = 6 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_NDG_RAN_0_T  			ran_0;
        ISP_NVRAM_NDG_RAN_1_T  			ran_1;
        ISP_NVRAM_NDG_RAN_2_T  			ran_2;
        ISP_NVRAM_NDG_RAN_3_T  			ran_3;
        ISP_NVRAM_NDG_CROP_X_T  		crop_x;
        ISP_NVRAM_NDG_CROP_Y_T  		crop_y;
    };
} ISP_NVRAM_NDG_T;
#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//YNR Table
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
enum { YNR_TBL_NUM = 256 };
typedef struct{
		FIELD  YNR_TBL_Y0                                 :  5;		/*  0.. 4, 0x0000001F */
		FIELD  YNR_TBL_U0                                 :  5;		/*  5.. 9, 0x000003E0 */
		FIELD  YNR_TBL_V0                                 :  5;		/* 10..14, 0x00007C00 */
		FIELD  rsv_15                                     :  1;		/* 15..15, 0x00008000 */
		FIELD  YNR_TBL_Y1                                 :  5;		/* 16..20, 0x001F0000 */
		FIELD  YNR_TBL_U1                                 :  5;		/* 21..25, 0x03E00000 */
		FIELD  YNR_TBL_V1                                 :  5;		/* 26..30, 0x7C000000 */
		FIELD  rsv_31                                     :  1;		/* 31..31, 0x80000000 */
} ISP_YNR_TBL_T;

typedef union {
    typedef ISP_YNR_TBL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_TBL_T;

typedef union {
    enum { COUNT = YNR_TBL_NUM };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_YNR_TBL_T   lut[YNR_TBL_NUM];
    };
} ISP_NVRAM_YNR_LUT_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// YNR
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct {
    FIELD  YNR_ENC                                    :  1;		/*  0.. 0, 0x00000001 */
    FIELD  YNR_ENY                                    :  1;		/*  1.. 1, 0x00000002 */
    FIELD  rsv_2                                      :  2;		/*  2.. 3, 0x0000000C */
    FIELD  YNR_Y_FLT0_IDX                             :  1;		/*  4.. 4, 0x00000010 */
    FIELD  rsv_5                                      :  1;		/*  5.. 5, 0x00000020 */
    FIELD  YNR_Y_FLT1_IDX                             :  1;		/*  6.. 6, 0x00000040 */
    FIELD  rsv_7                                      :  1;		/*  7.. 7, 0x00000080 */
    FIELD  YNR_Y_FLT2_IDX                             :  2;		/*  8.. 9, 0x00000300 */
    FIELD  YNR_Y_FLT3_IDX                             :  3;		/* 10..12, 0x00001C00 */
    FIELD  rsv_13                                     :  3;		/* 13..15, 0x0000E000 */
    FIELD  YNR_TILE_EDGE                              :  4;		/* 16..19, 0x000F0000 */
    FIELD  YNR_LCE_LINK                               :  1;		/* 20..20, 0x00100000 */
    FIELD  YNR_LCE_LUT_BYPASS                         :  1;		/* 21..21, 0x00200000 */
    FIELD  rsv_22                                     :  2;		/* 22..23, 0x00C00000 */
    FIELD  YNR_SL2_LINK                               :  1;		/* 24..24, 0x01000000 */
    FIELD  YNR_VIDEO_MODE                             :  1;		/* 25..25, 0x02000000 */
    FIELD  YNR_LBIT_MODE                              :  1;		/* 26..26, 0x04000000 */
    FIELD  YNR_LP_MODE                                :  1;		/* 27..27, 0x08000000 */
    FIELD  YNR_TABLE_EN                               :  1;		/* 28..28, 0x10000000 */
    FIELD  YNR_TBL_PRC                                :  1;		/* 29..29, 0x20000000 */
    FIELD  rsv_30                                     :  2;		/* 30..31, 0xC0000000 */
} ISP_YNR_CON1_T; //	/* 0x15025700 */

typedef union {
    typedef ISP_YNR_CON1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_CON1_T;

typedef struct {
    FIELD  YNR_IMPL_MODE                              :  2;		/*  0.. 1, 0x00000003 */
    FIELD  rsv_2                                      :  6;		/*  2.. 7, 0x000000FC */
    FIELD  YNR_C_SM_EDGE                              :  2;		/*  8.. 9, 0x00000300 */
    FIELD  rsv_10                                     :  2;		/* 10..11, 0x00000C00 */
    FIELD  YNR_FLT_C                                  :  1;		/* 12..12, 0x00001000 */
    FIELD  rsv_13                                     : 11;		/* 13..23, 0x00FFE000 */
    FIELD  YNR_C_SM_EDGE_TH                           :  3;		/* 24..26, 0x07000000 */
    FIELD  rsv_27                                     :  5;		/* 27..31, 0xF8000000 */
} ISP_YNR_CON2_T; //	/* 0x15025704 */

typedef union {
    typedef ISP_YNR_CON2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_CON2_T;

typedef struct {
    FIELD  YNR_CEN_GAIN_LO_TH                         :  5;		/*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                      :  3;		/*  5.. 7, 0x000000E0 */
    FIELD  YNR_CEN_GAIN_HI_TH                         :  5;		/*  8..12, 0x00001F00 */
    FIELD  rsv_13                                     :  3;		/* 13..15, 0x0000E000 */
    FIELD  YNR_Y_SLOPE_V_TH                           :  8;		/* 16..23, 0x00FF0000 */
    FIELD  YNR_Y_SLOPE_H_TH                           :  8;		/* 24..31, 0xFF000000 */
} ISP_YNR_YAD1_T; //	/* 0x15025708 */

typedef union {
    typedef ISP_YNR_YAD1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_YAD1_T;

typedef struct {
    FIELD  YNR_Y_VERT_ACT_TH                          :  5;		/*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                      :  3;		/*  5.. 7, 0x000000E0 */
    FIELD  YNR_PTY_GAIN_TH                            :  5;		/*  8..12, 0x00001F00 */
    FIELD  rsv_13                                     :  3;		/* 13..15, 0x0000E000 */
    FIELD  YNR_Y_VERT_SIGMA                           :  8;		/* 16..23, 0x00FF0000 */
    FIELD  YNR_GAIN_PRC                               :  3;		/* 24..26, 0x07000000 */
    FIELD  rsv_27                                     :  5;		/* 27..31, 0xF8000000 */
} ISP_YNR_YAD2_T; //	/* 0x1502570C */

typedef union {
    typedef ISP_YNR_YAD2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_YAD2_T;

typedef struct {
    FIELD  YNR_Y_CPX1                                 :  8;		/*  0.. 7, 0x000000FF */
    FIELD  YNR_Y_CPX2                                 :  8;		/*  8..15, 0x0000FF00 */
    FIELD  YNR_Y_CPX3                                 :  8;		/* 16..23, 0x00FF0000 */
    FIELD  YNR_Y_CPX4                                 :  8;		/* 24..31, 0xFF000000 */
} ISP_YNR_Y4LUT1_T; //	/* 0x15025710 */

typedef union {
    typedef ISP_YNR_Y4LUT1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_Y4LUT1_T;

typedef struct {
    FIELD  YNR_Y_SCALE_CPY0                           :  5;		/*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                      :  3;		/*  5.. 7, 0x000000E0 */
    FIELD  YNR_Y_SCALE_CPY1                           :  5;		/*  8..12, 0x00001F00 */
    FIELD  rsv_13                                     :  3;		/* 13..15, 0x0000E000 */
    FIELD  YNR_Y_SCALE_CPY2                           :  5;		/* 16..20, 0x001F0000 */
    FIELD  rsv_21                                     :  3;		/* 21..23, 0x00E00000 */
    FIELD  YNR_Y_SCALE_CPY3                           :  5;		/* 24..28, 0x1F000000 */
    FIELD  rsv_29                                     :  3;		/* 29..31, 0xE0000000 */
} ISP_YNR_Y4LUT2_T; //	/* 0x15025714 */

typedef union {
    typedef ISP_YNR_Y4LUT2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_Y4LUT2_T;

typedef struct {
    FIELD  YNR_Y_SCALE_SP0                            :  5;		/*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                      :  3;		/*  5.. 7, 0x000000E0 */
    FIELD  YNR_Y_SCALE_SP1                            :  5;		/*  8..12, 0x00001F00 */
    FIELD  rsv_13                                     :  3;		/* 13..15, 0x0000E000 */
    FIELD  YNR_Y_SCALE_SP2                            :  5;		/* 16..20, 0x001F0000 */
    FIELD  rsv_21                                     :  3;		/* 21..23, 0x00E00000 */
    FIELD  YNR_Y_SCALE_SP3                            :  5;		/* 24..28, 0x1F000000 */
    FIELD  rsv_29                                     :  3;		/* 29..31, 0xE0000000 */
} ISP_YNR_Y4LUT3_T; //	/* 0x15025718 */

typedef union {
    typedef ISP_YNR_Y4LUT3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_Y4LUT3_T;

typedef struct {
    FIELD  YNR_C_CPX1                                 :  8;		/*  0.. 7, 0x000000FF */
    FIELD  YNR_C_CPX2                                 :  8;		/*  8..15, 0x0000FF00 */
    FIELD  YNR_C_CPX3                                 :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                     :  8;		/* 24..31, 0xFF000000 */
} ISP_YNR_C4LUT1_T; //	/* 0x1502571C */

typedef union {
    typedef ISP_YNR_C4LUT1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_C4LUT1_T;

typedef struct {
    FIELD  YNR_C_SCALE_CPY0                           :  5;		/*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                      :  3;		/*  5.. 7, 0x000000E0 */
    FIELD  YNR_C_SCALE_CPY1                           :  5;		/*  8..12, 0x00001F00 */
    FIELD  rsv_13                                     :  3;		/* 13..15, 0x0000E000 */
    FIELD  YNR_C_SCALE_CPY2                           :  5;		/* 16..20, 0x001F0000 */
    FIELD  rsv_21                                     :  3;		/* 21..23, 0x00E00000 */
    FIELD  YNR_C_SCALE_CPY3                           :  5;		/* 24..28, 0x1F000000 */
    FIELD  rsv_29                                     :  3;		/* 29..31, 0xE0000000 */
} ISP_YNR_C4LUT2_T; //	/* 0x15025720 */

typedef union {
    typedef ISP_YNR_C4LUT2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_C4LUT2_T;

typedef struct {
    FIELD  YNR_C_SCALE_SP0                            :  5;		/*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                      :  3;		/*  5.. 7, 0x000000E0 */
    FIELD  YNR_C_SCALE_SP1                            :  5;		/*  8..12, 0x00001F00 */
    FIELD  rsv_13                                     :  3;		/* 13..15, 0x0000E000 */
    FIELD  YNR_C_SCALE_SP2                            :  5;		/* 16..20, 0x001F0000 */
    FIELD  rsv_21                                     :  3;		/* 21..23, 0x00E00000 */
    FIELD  YNR_C_SCALE_SP3                            :  5;		/* 24..28, 0x1F000000 */
    FIELD  rsv_29                                     :  3;		/* 29..31, 0xE0000000 */
} ISP_YNR_C4LUT3_T; //	/* 0x15025724 */

typedef union {
    typedef ISP_YNR_C4LUT3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_C4LUT3_T;

typedef struct {
    FIELD  YNR_Y_ACT_CPY0                             :  8;		/*  0.. 7, 0x000000FF */
    FIELD  YNR_Y_ACT_CPY1                             :  8;		/*  8..15, 0x0000FF00 */
    FIELD  YNR_Y_ACT_CPY2                             :  8;		/* 16..23, 0x00FF0000 */
    FIELD  YNR_Y_ACT_CPY3                             :  8;		/* 24..31, 0xFF000000 */
} ISP_YNR_A4LUT2_T; //	/* 0x15025728 */

typedef union {
    typedef ISP_YNR_A4LUT2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_A4LUT2_T;

typedef struct {
    FIELD  YNR_Y_ACT_SP0                              :  6;		/*  0.. 5, 0x0000003F */
    FIELD  rsv_6                                      :  2;		/*  6.. 7, 0x000000C0 */
    FIELD  YNR_Y_ACT_SP1                              :  6;		/*  8..13, 0x00003F00 */
    FIELD  rsv_14                                     :  2;		/* 14..15, 0x0000C000 */
    FIELD  YNR_Y_ACT_SP2                              :  6;		/* 16..21, 0x003F0000 */
    FIELD  rsv_22                                     :  2;		/* 22..23, 0x00C00000 */
    FIELD  YNR_Y_ACT_SP3                              :  6;		/* 24..29, 0x3F000000 */
    FIELD  rsv_30                                     :  2;		/* 30..31, 0xC0000000 */
} ISP_YNR_A4LUT3_T; //	/* 0x1502572C */

typedef union {
    typedef ISP_YNR_A4LUT3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_A4LUT3_T;

typedef struct {
    FIELD  YNR_SL2_X1                                 :  8;		/*  0.. 7, 0x000000FF */
    FIELD  YNR_SL2_X2                                 :  8;		/*  8..15, 0x0000FF00 */
    FIELD  YNR_SL2_X3                                 :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                     :  8;		/* 24..31, 0xFF000000 */
} ISP_YNR_L4LUT1_T; //	/* 0x15025730 */

typedef union {
    typedef ISP_YNR_L4LUT1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_L4LUT1_T;

typedef struct {
    FIELD  YNR_SL2_GAIN0                              :  6;		/*  0.. 5, 0x0000003F */
    FIELD  rsv_6                                      :  2;		/*  6.. 7, 0x000000C0 */
    FIELD  YNR_SL2_GAIN1                              :  6;		/*  8..13, 0x00003F00 */
    FIELD  rsv_14                                     :  2;		/* 14..15, 0x0000C000 */
    FIELD  YNR_SL2_GAIN2                              :  6;		/* 16..21, 0x003F0000 */
    FIELD  rsv_22                                     :  2;		/* 22..23, 0x00C00000 */
    FIELD  YNR_SL2_GAIN3                              :  6;		/* 24..29, 0x3F000000 */
    FIELD  rsv_30                                     :  2;		/* 30..31, 0xC0000000 */
} ISP_YNR_L4LUT2_T; //	/* 0x15025734 */

typedef union {
    typedef ISP_YNR_L4LUT2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_L4LUT2_T;

typedef struct {
    FIELD  YNR_SL2_SP0                                :  6;		/*  0.. 5, 0x0000003F */
    FIELD  rsv_6                                      :  2;		/*  6.. 7, 0x000000C0 */
    FIELD  YNR_SL2_SP1                                :  6;		/*  8..13, 0x00003F00 */
    FIELD  rsv_14                                     :  2;		/* 14..15, 0x0000C000 */
    FIELD  YNR_SL2_SP2                                :  6;		/* 16..21, 0x003F0000 */
    FIELD  rsv_22                                     :  2;		/* 22..23, 0x00C00000 */
    FIELD  YNR_SL2_SP3                                :  6;		/* 24..29, 0x3F000000 */
    FIELD  rsv_30                                     :  2;		/* 30..31, 0xC0000000 */
} ISP_YNR_L4LUT3_T; //	/* 0x15025738 */

typedef union {
    typedef ISP_YNR_L4LUT3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_L4LUT3_T;

typedef struct {
    FIELD  YNR_Y_L0_V_RNG1                            :  8;		/*  0.. 7, 0x000000FF */
    FIELD  YNR_Y_L0_V_RNG2                            :  8;		/*  8..15, 0x0000FF00 */
    FIELD  YNR_Y_L0_V_RNG3                            :  8;		/* 16..23, 0x00FF0000 */
    FIELD  YNR_Y_L0_V_RNG4                            :  8;		/* 24..31, 0xFF000000 */
} ISP_YNR_PTY0V_T; //	/* 0x1502573C */

typedef union {
    typedef ISP_YNR_PTY0V_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_PTY0V_T;

typedef struct {
    FIELD  rsv_0                                      :  8;		/*  0.. 7, 0x000000FF */
    FIELD  YNR_PTC_GAIN_TH                            :  5;		/*  8..12, 0x00001F00 */
    FIELD  rsv_13                                     :  3;		/* 13..15, 0x0000E000 */
    FIELD  YNR_C_L_DIFF_TH                            :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                     :  8;		/* 24..31, 0xFF000000 */
} ISP_YNR_CAD_T; //	/* 0x15025740 */

typedef union {
    typedef ISP_YNR_CAD_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_CAD_T;

typedef struct {
    FIELD  YNR_Y_L1_V_RNG1                            :  8;		/*  0.. 7, 0x000000FF */
    FIELD  YNR_Y_L1_V_RNG2                            :  8;		/*  8..15, 0x0000FF00 */
    FIELD  YNR_Y_L1_V_RNG3                            :  8;		/* 16..23, 0x00FF0000 */
    FIELD  YNR_Y_L1_V_RNG4                            :  8;		/* 24..31, 0xFF000000 */
} ISP_YNR_PTY1V_T; //	/* 0x15025744 */

typedef union {
    typedef ISP_YNR_PTY1V_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_PTY1V_T;

typedef struct {
    FIELD  YNR_SL2_C_GAIN                             :  4;		/*  0.. 3, 0x0000000F */
    FIELD  YNR_SL2_SCALE_GAIN                         :  3;		/*  4.. 6, 0x00000070 */
    FIELD  rsv_7                                      : 25;		/*  7..31, 0xFFFFFF80 */
} ISP_YNR_SL2_T; //	/* 0x15025748 */

typedef union {
    typedef ISP_YNR_SL2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_SL2_T;

typedef struct {
    FIELD  YNR_Y_L2_V_RNG1                            :  8;		/*  0.. 7, 0x000000FF */
    FIELD  YNR_Y_L2_V_RNG2                            :  8;		/*  8..15, 0x0000FF00 */
    FIELD  YNR_Y_L2_V_RNG3                            :  8;		/* 16..23, 0x00FF0000 */
    FIELD  YNR_Y_L2_V_RNG4                            :  8;		/* 24..31, 0xFF000000 */
} ISP_YNR_PTY2V_T; //	/* 0x1502574C */

typedef union {
    typedef ISP_YNR_PTY2V_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_PTY2V_T;

typedef struct {
    FIELD  YNR_Y_L3_V_RNG1                            :  8;		/*  0.. 7, 0x000000FF */
    FIELD  YNR_Y_L3_V_RNG2                            :  8;		/*  8..15, 0x0000FF00 */
    FIELD  YNR_Y_L3_V_RNG3                            :  8;		/* 16..23, 0x00FF0000 */
    FIELD  YNR_Y_L3_V_RNG4                            :  8;		/* 24..31, 0xFF000000 */
} ISP_YNR_PTY3V_T; //	/* 0x15025750 */

typedef union {
    typedef ISP_YNR_PTY3V_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_PTY3V_T;

typedef struct {
    FIELD  YNR_Y_L0_H_RNG1                            :  8;		/*  0.. 7, 0x000000FF */
    FIELD  YNR_Y_L0_H_RNG2                            :  8;		/*  8..15, 0x0000FF00 */
    FIELD  YNR_Y_L0_H_RNG3                            :  8;		/* 16..23, 0x00FF0000 */
    FIELD  YNR_Y_L0_H_RNG4                            :  8;		/* 24..31, 0xFF000000 */
} ISP_YNR_PTY0H_T; //	/* 0x15025754 */

typedef union {
    typedef ISP_YNR_PTY0H_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_PTY0H_T;

typedef struct {
    FIELD  YNR_Y_L1_H_RNG1                            :  8;		/*  0.. 7, 0x000000FF */
    FIELD  YNR_Y_L1_H_RNG2                            :  8;		/*  8..15, 0x0000FF00 */
    FIELD  YNR_Y_L1_H_RNG3                            :  8;		/* 16..23, 0x00FF0000 */
    FIELD  YNR_Y_L1_H_RNG4                            :  8;		/* 24..31, 0xFF000000 */
} ISP_YNR_PTY1H_T; //	/* 0x15025758 */

typedef union {
    typedef ISP_YNR_PTY1H_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_PTY1H_T;

typedef struct {
    FIELD  YNR_Y_L2_H_RNG1                            :  8;		/*  0.. 7, 0x000000FF */
    FIELD  YNR_Y_L2_H_RNG2                            :  8;		/*  8..15, 0x0000FF00 */
    FIELD  YNR_Y_L2_H_RNG3                            :  8;		/* 16..23, 0x00FF0000 */
    FIELD  YNR_Y_L2_H_RNG4                            :  8;		/* 24..31, 0xFF000000 */
} ISP_YNR_PTY2H_T; //	/* 0x1502575C */

typedef union {
    typedef ISP_YNR_PTY2H_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_PTY2H_T;

typedef struct {
    FIELD  YNR_TBL_CPX1                               :  8;		/*  0.. 7, 0x000000FF */
    FIELD  YNR_TBL_CPX2                               :  8;		/*  8..15, 0x0000FF00 */
    FIELD  YNR_TBL_CPX3                               :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                     :  8;		/* 24..31, 0xFF000000 */
} ISP_YNR_T4LUT1_T; //	/* 0x15025760 */

typedef union {
    typedef ISP_YNR_T4LUT1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_T4LUT1_T;

typedef struct {
    FIELD  YNR_TBL_GAIN_CPY0                          :  5;		/*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                      :  3;		/*  5.. 7, 0x000000E0 */
    FIELD  YNR_TBL_GAIN_CPY1                          :  5;		/*  8..12, 0x00001F00 */
    FIELD  rsv_13                                     :  3;		/* 13..15, 0x0000E000 */
    FIELD  YNR_TBL_GAIN_CPY2                          :  5;		/* 16..20, 0x001F0000 */
    FIELD  rsv_21                                     :  3;		/* 21..23, 0x00E00000 */
    FIELD  YNR_TBL_GAIN_CPY3                          :  5;		/* 24..28, 0x1F000000 */
    FIELD  rsv_29                                     :  3;		/* 29..31, 0xE0000000 */
} ISP_YNR_T4LUT2_T; //	/* 0x15025764 */

typedef union {
    typedef ISP_YNR_T4LUT2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_T4LUT2_T;

typedef struct {
    FIELD  YNR_TBL_GAIN_SP0                           :  5;		/*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                      :  3;		/*  5.. 7, 0x000000E0 */
    FIELD  YNR_TBL_GAIN_SP1                           :  5;		/*  8..12, 0x00001F00 */
    FIELD  rsv_13                                     :  3;		/* 13..15, 0x0000E000 */
    FIELD  YNR_TBL_GAIN_SP2                           :  5;		/* 16..20, 0x001F0000 */
    FIELD  rsv_21                                     :  3;		/* 21..23, 0x00E00000 */
    FIELD  YNR_TBL_GAIN_SP3                           :  5;		/* 24..28, 0x1F000000 */
    FIELD  rsv_29                                     :  3;		/* 29..31, 0xE0000000 */
} ISP_YNR_T4LUT3_T; //	/* 0x15025768 */

typedef union {
    typedef ISP_YNR_T4LUT3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_T4LUT3_T;

typedef struct {
    FIELD  YNR_ACT_SL2_GAIN                           :  4;		/*  0.. 3, 0x0000000F */
    FIELD  rsv_4                                      :  4;		/*  4.. 7, 0x000000F0 */
    FIELD  YNR_ACT_DIF_HI_TH                          :  5;		/*  8..12, 0x00001F00 */
    FIELD  rsv_13                                     :  7;		/* 13..19, 0x000FE000 */
    FIELD  YNR_ACT_DIF_GAIN                           :  4;		/* 20..23, 0x00F00000 */
    FIELD  YNR_ACT_DIF_LO_TH                          :  5;		/* 24..28, 0x1F000000 */
    FIELD  rsv_29                                     :  3;		/* 29..31, 0xE0000000 */
} ISP_YNR_ACT1_T; //	/* 0x1502576C */

typedef union {
    typedef ISP_YNR_ACT1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_ACT1_T;

typedef struct {
    FIELD  YNR_Y_L3_H_RNG1                            :  8;		/*  0.. 7, 0x000000FF */
    FIELD  YNR_Y_L3_H_RNG2                            :  8;		/*  8..15, 0x0000FF00 */
    FIELD  YNR_Y_L3_H_RNG3                            :  8;		/* 16..23, 0x00FF0000 */
    FIELD  YNR_Y_L3_H_RNG4                            :  8;		/* 24..31, 0xFF000000 */
} ISP_YNR_PTY3H_T; //	/* 0x15025770 */

typedef union {
    typedef ISP_YNR_PTY3H_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_PTY3H_T;

typedef struct {
    FIELD  YNR_C_V_RNG1                               :  8;		/*  0.. 7, 0x000000FF */
    FIELD  YNR_C_V_RNG2                               :  8;		/*  8..15, 0x0000FF00 */
    FIELD  YNR_C_V_RNG3                               :  8;		/* 16..23, 0x00FF0000 */
    FIELD  YNR_C_V_RNG4                               :  8;		/* 24..31, 0xFF000000 */
} ISP_YNR_PTCV_T; //	/* 0x15025774 */

typedef union {
    typedef ISP_YNR_PTCV_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_PTCV_T;

typedef struct {
    FIELD  YNR_Y_ACT_CEN_OFT                          :  6;		/*  0.. 5, 0x0000003F */
    FIELD  rsv_6                                      :  2;		/*  6.. 7, 0x000000C0 */
    FIELD  YNR_Y_ACT_CEN_GAIN                         :  6;		/*  8..13, 0x00003F00 */
    FIELD  rsv_14                                     :  2;		/* 14..15, 0x0000C000 */
    FIELD  YNR_Y_ACT_CEN_TH                           :  6;		/* 16..21, 0x003F0000 */
    FIELD  rsv_22                                     : 10;		/* 22..31, 0xFFC00000 */
} ISP_YNR_ACT4_T; //	/* 0x15025778 */

typedef union {
    typedef ISP_YNR_ACT4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_ACT4_T;

typedef struct {
    FIELD  YNR_C_H_RNG1                               :  8;		/*  0.. 7, 0x000000FF */
    FIELD  YNR_C_H_RNG2                               :  8;		/*  8..15, 0x0000FF00 */
    FIELD  YNR_C_H_RNG3                               :  8;		/* 16..23, 0x00FF0000 */
    FIELD  YNR_C_H_RNG4                               :  8;		/* 24..31, 0xFF000000 */
} ISP_YNR_PTCH_T; //	/* 0x1502577C */

typedef union {
    typedef ISP_YNR_PTCH_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_PTCH_T;

typedef struct {
    FIELD  YNR_Y_L0_RNG_RAT_TH                        :  4;		/*  0.. 3, 0x0000000F */
    FIELD  YNR_Y_L1_RNG_RAT_TH                        :  4;		/*  4.. 7, 0x000000F0 */
    FIELD  YNR_Y_L2_RNG_RAT_TH                        :  4;		/*  8..11, 0x00000F00 */
    FIELD  YNR_Y_L3_RNG_RAT_TH                        :  4;		/* 12..15, 0x0000F000 */
    FIELD  YNR_Y_L0_RNG_RAT_SL                        :  2;		/* 16..17, 0x00030000 */
    FIELD  rsv_18                                     :  2;		/* 18..19, 0x000C0000 */
    FIELD  YNR_Y_L1_RNG_RAT_SL                        :  2;		/* 20..21, 0x00300000 */
    FIELD  rsv_22                                     :  2;		/* 22..23, 0x00C00000 */
    FIELD  YNR_Y_L2_RNG_RAT_SL                        :  2;		/* 24..25, 0x03000000 */
    FIELD  rsv_26                                     :  2;		/* 26..27, 0x0C000000 */
    FIELD  YNR_Y_L3_RNG_RAT_SL                        :  2;		/* 28..29, 0x30000000 */
    FIELD  rsv_30                                     :  2;		/* 30..31, 0xC0000000 */
} ISP_YNR_YLVL0_T; //	/* 0x15025780 */

typedef union {
    typedef ISP_YNR_YLVL0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_YLVL0_T;

typedef struct {
    FIELD  YNR_Y_L0_HF_W                              :  6;		/*  0.. 5, 0x0000003F */
    FIELD  rsv_6                                      :  2;		/*  6.. 7, 0x000000C0 */
    FIELD  YNR_Y_L1_HF_W                              :  6;		/*  8..13, 0x00003F00 */
    FIELD  rsv_14                                     :  2;		/* 14..15, 0x0000C000 */
    FIELD  YNR_Y_L2_HF_W                              :  6;		/* 16..21, 0x003F0000 */
    FIELD  rsv_22                                     :  2;		/* 22..23, 0x00C00000 */
    FIELD  YNR_Y_L3_HF_W                              :  6;		/* 24..29, 0x3F000000 */
    FIELD  rsv_30                                     :  2;		/* 30..31, 0xC0000000 */
} ISP_YNR_YLVL1_T; //	/* 0x15025784 */

typedef union {
    typedef ISP_YNR_YLVL1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_YLVL1_T;

typedef struct {
    FIELD  YNR_Y_HF_CORE_TH                           :  8;		/*  0.. 7, 0x000000FF */
    FIELD  YNR_Y_HF_CORE_SL                           :  2;		/*  8.. 9, 0x00000300 */
    FIELD  rsv_10                                     :  6;		/* 10..15, 0x0000FC00 */
    FIELD  YNR_Y_HF_CLIP                              :  8;		/* 16..23, 0x00FF0000 */
    FIELD  YNR_Y_HF_BAL_MODE                          :  2;		/* 24..25, 0x03000000 */
    FIELD  YNR_Y_HF_BAL_STR                           :  6;		/* 26..31, 0xFC000000 */
} ISP_YNR_HF_COR_T; //	/* 0x15025788 */

typedef union {
    typedef ISP_YNR_HF_COR_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_HF_COR_T;

typedef struct {
    FIELD  YNR_Y_HF_ACT_X1                            :  8;		/*  0.. 7, 0x000000FF */
    FIELD  YNR_Y_HF_ACT_X2                            :  8;		/*  8..15, 0x0000FF00 */
    FIELD  YNR_Y_HF_ACT_X3                            :  8;		/* 16..23, 0x00FF0000 */
    FIELD  YNR_Y_HF_ACT_X4                            :  8;		/* 24..31, 0xFF000000 */
} ISP_YNR_HF_ACT0_T; //	/* 0x1502578C */

typedef union {
    typedef ISP_YNR_HF_ACT0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_HF_ACT0_T;

typedef struct {
    FIELD  YNR_Y_HF_ACT_Y0                            :  7;		/*  0.. 6, 0x0000007F */
    FIELD  rsv_7                                      :  1;		/*  7.. 7, 0x00000080 */
    FIELD  YNR_Y_HF_ACT_Y1                            :  7;		/*  8..14, 0x00007F00 */
    FIELD  rsv_15                                     :  1;		/* 15..15, 0x00008000 */
    FIELD  YNR_Y_HF_ACT_Y2                            :  7;		/* 16..22, 0x007F0000 */
    FIELD  rsv_23                                     :  1;		/* 23..23, 0x00800000 */
    FIELD  YNR_Y_HF_ACT_Y3                            :  7;		/* 24..30, 0x7F000000 */
    FIELD  rsv_31                                     :  1;		/* 31..31, 0x80000000 */
} ISP_YNR_HF_ACT1_T; //	/* 0x15025790 */

typedef union {
    typedef ISP_YNR_HF_ACT1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_HF_ACT1_T;

typedef struct {
    FIELD  YNR_RSV                                    :  8;		/*  0.. 7, 0x000000FF */
    FIELD  YNR_ACT_BLD_BASE_C                         :  7;		/*  8..14, 0x00007F00 */
    FIELD  rsv_15                                     :  1;		/* 15..15, 0x00008000 */
    FIELD  YNR_C_DITH_U                               :  5;		/* 16..20, 0x001F0000 */
    FIELD  rsv_21                                     :  3;		/* 21..23, 0x00E00000 */
    FIELD  YNR_C_DITH_V                               :  5;		/* 24..28, 0x1F000000 */
    FIELD  rsv_29                                     :  3;		/* 29..31, 0xE0000000 */
} ISP_YNR_ACTC_T; //	/* 0x15025794 */

typedef union {
    typedef ISP_YNR_ACTC_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_ACTC_T;

typedef struct {
    FIELD  YNR_CEN_GAIN_LO_TH_LPF                     :  5;		/*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                      :  3;		/*  5.. 7, 0x000000E0 */
    FIELD  YNR_CEN_GAIN_HI_TH_LPF                     :  5;		/*  8..12, 0x00001F00 */
    FIELD  rsv_13                                     : 19;		/* 13..31, 0xFFFFE000 */
} ISP_YNR_YLAD_T; //	/* 0x15025798 */

typedef union {
    typedef ISP_YNR_YLAD_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_YLAD_T;

typedef struct {
    FIELD  YNR_Y_HF_ACT_Y4                            :  7;		/*  0.. 6, 0x0000007F */
    FIELD  rsv_7                                      :  1;		/*  7.. 7, 0x00000080 */
    FIELD  YNR_Y_HF_ACT_SP4                           :  6;		/*  8..13, 0x00003F00 */
    FIELD  rsv_14                                     : 18;		/* 14..31, 0xFFFFC000 */
} ISP_YNR_HF_ACT2_T; //	/* 0x1502579C */

typedef union {
    typedef ISP_YNR_HF_ACT2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_HF_ACT2_T;

typedef struct {
    FIELD  YNR_Y_HF_ACT_SP0                           :  6;		/*  0.. 5, 0x0000003F */
    FIELD  rsv_6                                      :  2;		/*  6.. 7, 0x000000C0 */
    FIELD  YNR_Y_HF_ACT_SP1                           :  6;		/*  8..13, 0x00003F00 */
    FIELD  rsv_14                                     :  2;		/* 14..15, 0x0000C000 */
    FIELD  YNR_Y_HF_ACT_SP2                           :  6;		/* 16..21, 0x003F0000 */
    FIELD  rsv_22                                     :  2;		/* 22..23, 0x00C00000 */
    FIELD  YNR_Y_HF_ACT_SP3                           :  6;		/* 24..29, 0x3F000000 */
    FIELD  rsv_30                                     :  2;		/* 30..31, 0xC0000000 */
} ISP_YNR_HF_ACT3_T; //	/* 0x150257A0 */

typedef union {
    typedef ISP_YNR_HF_ACT3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_HF_ACT3_T;

typedef struct {
    FIELD  YNR_Y_HF_LUMA_X1                           :  8;		/*  0.. 7, 0x000000FF */
    FIELD  YNR_Y_HF_LUMA_X2                           :  8;		/*  8..15, 0x0000FF00 */
    FIELD  YNR_Y_HF_LUMA_Y0                           :  7;		/* 16..22, 0x007F0000 */
    FIELD  rsv_23                                     :  1;		/* 23..23, 0x00800000 */
    FIELD  YNR_Y_HF_LUMA_Y1                           :  7;		/* 24..30, 0x7F000000 */
    FIELD  rsv_31                                     :  1;		/* 31..31, 0x80000000 */
} ISP_YNR_HF_LUMA0_T; //	/* 0x150257A4 */

typedef union {
    typedef ISP_YNR_HF_LUMA0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_HF_LUMA0_T;

typedef struct {
    FIELD  YNR_Y_HF_LUMA_Y2                           :  7;		/*  0.. 6, 0x0000007F */
    FIELD  rsv_7                                      :  1;		/*  7.. 7, 0x00000080 */
    FIELD  YNR_Y_HF_LUMA_SP0                          :  6;		/*  8..13, 0x00003F00 */
    FIELD  rsv_14                                     :  2;		/* 14..15, 0x0000C000 */
    FIELD  YNR_Y_HF_LUMA_SP1                          :  6;		/* 16..21, 0x003F0000 */
    FIELD  rsv_22                                     :  2;		/* 22..23, 0x00C00000 */
    FIELD  YNR_Y_HF_LUMA_SP2                          :  6;		/* 24..29, 0x3F000000 */
    FIELD  rsv_30                                     :  2;		/* 30..31, 0xC0000000 */
} ISP_YNR_HF_LUMA1_T; //	/* 0x150257A8 */

typedef union {
    typedef ISP_YNR_HF_LUMA1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_HF_LUMA1_T;

typedef struct {
    FIELD  YNR_LCE_GAIN_S0                            :  8;		/*  0.. 7, 0x000000FF */
    FIELD  YNR_LCE_GAIN_S1                            :  8;		/*  8..15, 0x0000FF00 */
    FIELD  YNR_LCE_GAIN_S2                            :  8;		/* 16..23, 0x00FF0000 */
    FIELD  YNR_LCE_GAIN_S3                            :  8;		/* 24..31, 0xFF000000 */
} ISP_YNR_LCE_GAIN1_T; //	/* 0x150257AC */

typedef union {
    typedef ISP_YNR_LCE_GAIN1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_LCE_GAIN1_T;

typedef struct {
    FIELD  YNR_LCE_GAIN_S4                            :  8;		/*  0.. 7, 0x000000FF */
    FIELD  YNR_LCE_GAIN_S5                            :  8;		/*  8..15, 0x0000FF00 */
    FIELD  YNR_LCE_GAIN_S6                            :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                     :  8;		/* 24..31, 0xFF000000 */
} ISP_YNR_LCE_GAIN2_T; //	/* 0x150257B0 */

typedef union {
    typedef ISP_YNR_LCE_GAIN2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_LCE_GAIN2_T;

typedef struct {
    FIELD  YNR_LCE_P0_S4                              :  8;		/*  0.. 7, 0x000000FF */
    FIELD  YNR_LCE_P1_S4                              :  8;		/*  8..15, 0x0000FF00 */
    FIELD  YNR_LCE_P50_S4                             :  8;		/* 16..23, 0x00FF0000 */
    FIELD  YNR_LCE_P250_S4                            :  8;		/* 24..31, 0xFF000000 */
} ISP_YNR_LCE_LUTP1_T; //	/* 0x150257B4 */

typedef union {
    typedef ISP_YNR_LCE_LUTP1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_LCE_LUTP1_T;

typedef struct {
    FIELD  YNR_LCE_P500_S4                            :  8;		/*  0.. 7, 0x000000FF */
    FIELD  YNR_LCE_P750_S4                            :  8;		/*  8..15, 0x0000FF00 */
    FIELD  YNR_LCE_P950_S4                            :  8;		/* 16..23, 0x00FF0000 */
    FIELD  YNR_LCE_P999_S4                            :  8;		/* 24..31, 0xFF000000 */
} ISP_YNR_LCE_LUTP2_T; //	/* 0x150257B8 */

typedef union {
    typedef ISP_YNR_LCE_LUTP2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_LCE_LUTP2_T;

typedef struct {
    FIELD  YNR_LCE_O0_S4                              :  8;		/*  0.. 7, 0x000000FF */
    FIELD  YNR_LCE_O1_S4                              :  8;		/*  8..15, 0x0000FF00 */
    FIELD  YNR_LCE_O50_S4                             :  8;		/* 16..23, 0x00FF0000 */
    FIELD  YNR_LCE_O250_S4                            :  8;		/* 24..31, 0xFF000000 */
} ISP_YNR_LCE_LUTO1_T; //	/* 0x150257BC */

typedef union {
    typedef ISP_YNR_LCE_LUTO1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_LCE_LUTO1_T;

typedef struct {
    FIELD  YNR_LCE_O500_S4                            :  8;		/*  0.. 7, 0x000000FF */
    FIELD  YNR_LCE_O750_S4                            :  8;		/*  8..15, 0x0000FF00 */
    FIELD  YNR_LCE_O950_S4                            :  8;		/* 16..23, 0x00FF0000 */
    FIELD  YNR_LCE_O999_S4                            :  8;		/* 24..31, 0xFF000000 */
} ISP_YNR_LCE_LUTO2_T; //	/* 0x150257C0 */

typedef union {
    typedef ISP_YNR_LCE_LUTO2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_LCE_LUTO2_T;

typedef struct {
    FIELD  YNR_LCE_SP0                                : 13;		/*  0..12, 0x00001FFF */
    FIELD  rsv_13                                     :  3;		/* 13..15, 0x0000E000 */
    FIELD  YNR_LCE_SP1                                : 13;		/* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                     :  3;		/* 29..31, 0xE0000000 */
} ISP_YNR_LCE_LUTS1_T; //	/* 0x150257C4 */

typedef union {
    typedef ISP_YNR_LCE_LUTS1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_LCE_LUTS1_T;

typedef struct {
    FIELD  YNR_LCE_SP2                                : 13;		/*  0..12, 0x00001FFF */
    FIELD  rsv_13                                     :  3;		/* 13..15, 0x0000E000 */
    FIELD  YNR_LCE_SP3                                : 13;		/* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                     :  3;		/* 29..31, 0xE0000000 */
} ISP_YNR_LCE_LUTS2_T; //	/* 0x150257C8 */

typedef union {
    typedef ISP_YNR_LCE_LUTS2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_LCE_LUTS2_T;

typedef struct {
    FIELD  YNR_LCE_SP4                                : 13;		/*  0..12, 0x00001FFF */
    FIELD  rsv_13                                     :  3;		/* 13..15, 0x0000E000 */
    FIELD  YNR_LCE_SP5                                : 13;		/* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                     :  3;		/* 29..31, 0xE0000000 */
} ISP_YNR_LCE_LUTS3_T; //	/* 0x150257CC */

typedef union {
    typedef ISP_YNR_LCE_LUTS3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_LCE_LUTS3_T;

typedef struct {
    FIELD  YNR_LCE_SP6                                : 13;		/*  0..12, 0x00001FFF */
    FIELD  rsv_13                                     : 19;		/* 13..31, 0xFFFFE000 */
} ISP_YNR_LCE_LUTS4_T; //	/* 0x150257D0 */

typedef union {
    typedef ISP_YNR_LCE_LUTS4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_LCE_LUTS4_T;

typedef struct {
    FIELD  YNR_Y_CPX5                                 :  8;		/*  0.. 7, 0x000000FF */
    FIELD  YNR_Y_CPX6                                 :  8;		/*  8..15, 0x0000FF00 */
    FIELD  YNR_Y_CPX7                                 :  8;		/* 16..23, 0x00FF0000 */
    FIELD  YNR_Y_CPX8                                 :  8;		/* 24..31, 0xFF000000 */
} ISP_YNR_Y4LUT4_T; //	/* 0x150257D4 */

typedef union {
    typedef ISP_YNR_Y4LUT4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_Y4LUT4_T;

typedef struct {
    FIELD  YNR_Y_SCALE_CPY4                           :  5;		/*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                      :  3;		/*  5.. 7, 0x000000E0 */
    FIELD  YNR_Y_SCALE_CPY5                           :  5;		/*  8..12, 0x00001F00 */
    FIELD  rsv_13                                     :  3;		/* 13..15, 0x0000E000 */
    FIELD  YNR_Y_SCALE_CPY6                           :  5;		/* 16..20, 0x001F0000 */
    FIELD  rsv_21                                     :  3;		/* 21..23, 0x00E00000 */
    FIELD  YNR_Y_SCALE_CPY7                           :  5;		/* 24..28, 0x1F000000 */
    FIELD  rsv_29                                     :  3;		/* 29..31, 0xE0000000 */
} ISP_YNR_Y4LUT5_T; //	/* 0x150257D8 */

typedef union {
    typedef ISP_YNR_Y4LUT5_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_Y4LUT5_T;

typedef struct {
    FIELD  YNR_Y_SCALE_SP4                            :  5;		/*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                      :  3;		/*  5.. 7, 0x000000E0 */
    FIELD  YNR_Y_SCALE_SP5                            :  5;		/*  8..12, 0x00001F00 */
    FIELD  rsv_13                                     :  3;		/* 13..15, 0x0000E000 */
    FIELD  YNR_Y_SCALE_SP6                            :  5;		/* 16..20, 0x001F0000 */
    FIELD  rsv_21                                     :  3;		/* 21..23, 0x00E00000 */
    FIELD  YNR_Y_SCALE_SP7                            :  5;		/* 24..28, 0x1F000000 */
    FIELD  rsv_29                                     :  3;		/* 29..31, 0xE0000000 */
} ISP_YNR_Y4LUT6_T; //	/* 0x150257DC */

typedef union {
    typedef ISP_YNR_Y4LUT6_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_Y4LUT6_T;

typedef struct {
    FIELD  YNR_Y_SCALE_CPY8                           :  5;		/*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                      :  3;		/*  5.. 7, 0x000000E0 */
    FIELD  YNR_Y_SCALE_SP8                            :  5;		/*  8..12, 0x00001F00 */
    FIELD  rsv_13                                     : 19;		/* 13..31, 0xFFFFE000 */
} ISP_YNR_Y4LUT7_T; //	/* 0x150257E0 */

typedef union {
    typedef ISP_YNR_Y4LUT7_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_Y4LUT7_T;

typedef struct {
    FIELD  YNR_Y_ACT_CPX1                             :  8;		/*  0.. 7, 0x000000FF */
    FIELD  YNR_Y_ACT_CPX2                             :  8;		/*  8..15, 0x0000FF00 */
    FIELD  YNR_Y_ACT_CPX3                             :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                     :  8;		/* 24..31, 0xFF000000 */
} ISP_YNR_A4LUT1_T; //	/* 0x150257E4 */

typedef union {
    typedef ISP_YNR_A4LUT1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_A4LUT1_T;

typedef struct {
    FIELD  YNR_SKIN_LINK                              :  1;		/*  0.. 0, 0x00000001 */
    FIELD  rsv_1                                      :  7;		/*  1.. 7, 0x000000FE */
    FIELD  YNR_SKIN_GAIN_HI                           :  8;		/*  8..15, 0x0000FF00 */
    FIELD  YNR_SKIN_GAIN_LO                           :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                     :  8;		/* 24..31, 0xFF000000 */
} ISP_YNR_SKIN_CON_T; //	/* 0x150257E8 */

typedef union {
    typedef ISP_YNR_SKIN_CON_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_SKIN_CON_T;

typedef struct {
    FIELD  YNR_SKIN1_Y_THSL                           :  8;		/*  0.. 7, 0x000000FF */
    FIELD  YNR_SKIN1_Y_THH                            :  8;		/*  8..15, 0x0000FF00 */
    FIELD  YNR_SKIN1_Y_THL                            :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                     :  8;		/* 24..31, 0xFF000000 */
} ISP_YNR_SKIN1_Y_T; //	/* 0x150257EC */

typedef union {
    typedef ISP_YNR_SKIN1_Y_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_SKIN1_Y_T;

typedef struct {
    FIELD  YNR_SKIN1_U_THSL                           :  8;		/*  0.. 7, 0x000000FF */
    FIELD  YNR_SKIN1_U_THH                            :  8;		/*  8..15, 0x0000FF00 */
    FIELD  YNR_SKIN1_U_THL                            :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                     :  8;		/* 24..31, 0xFF000000 */
} ISP_YNR_SKIN1_U_T; //	/* 0x150257F0 */

typedef union {
    typedef ISP_YNR_SKIN1_U_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_SKIN1_U_T;

typedef struct {
    FIELD  YNR_SKIN1_V_THSL                           :  8;		/*  0.. 7, 0x000000FF */
    FIELD  YNR_SKIN1_V_THH                            :  8;		/*  8..15, 0x0000FF00 */
    FIELD  YNR_SKIN1_V_THL                            :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                     :  8;		/* 24..31, 0xFF000000 */
} ISP_YNR_SKIN1_V_T; //	/* 0x150257F4 */

typedef union {
    typedef ISP_YNR_SKIN1_V_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_SKIN1_V_T;

typedef struct {
    FIELD  YNR_SKIN2_Y_THSL                           :  8;		/*  0.. 7, 0x000000FF */
    FIELD  YNR_SKIN2_Y_THH                            :  8;		/*  8..15, 0x0000FF00 */
    FIELD  YNR_SKIN2_Y_THL                            :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                     :  8;		/* 24..31, 0xFF000000 */
} ISP_YNR_SKIN2_Y_T; //	/* 0x150257F8 */

typedef union {
    typedef ISP_YNR_SKIN2_Y_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_SKIN2_Y_T;

typedef struct {
    FIELD  YNR_SKIN2_U_THSL                           :  8;		/*  0.. 7, 0x000000FF */
    FIELD  YNR_SKIN2_U_THH                            :  8;		/*  8..15, 0x0000FF00 */
    FIELD  YNR_SKIN2_U_THL                            :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                     :  8;		/* 24..31, 0xFF000000 */
} ISP_YNR_SKIN2_U_T; //	/* 0x150257FC */

typedef union {
    typedef ISP_YNR_SKIN2_U_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_SKIN2_U_T;

typedef struct {
    FIELD  YNR_SKIN2_V_THSL                           :  8;		/*  0.. 7, 0x000000FF */
    FIELD  YNR_SKIN2_V_THH                            :  8;		/*  8..15, 0x0000FF00 */
    FIELD  YNR_SKIN2_V_THL                            :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                     :  8;		/* 24..31, 0xFF000000 */
} ISP_YNR_SKIN2_V_T; //	/* 0x15025800 */

typedef union {
    typedef ISP_YNR_SKIN2_V_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_SKIN2_V_T;

typedef struct {
    FIELD  YNR_SKIN3_Y_THSL                           :  8;		/*  0.. 7, 0x000000FF */
    FIELD  YNR_SKIN3_Y_THH                            :  8;		/*  8..15, 0x0000FF00 */
    FIELD  YNR_SKIN3_Y_THL                            :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                     :  8;		/* 24..31, 0xFF000000 */
} ISP_YNR_SKIN3_Y_T; //	/* 0x15025804 */

typedef union {
    typedef ISP_YNR_SKIN3_Y_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_SKIN3_Y_T;

typedef struct {
    FIELD  YNR_SKIN3_U_THSL                           :  8;		/*  0.. 7, 0x000000FF */
    FIELD  YNR_SKIN3_U_THH                            :  8;		/*  8..15, 0x0000FF00 */
    FIELD  YNR_SKIN3_U_THL                            :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                     :  8;		/* 24..31, 0xFF000000 */
} ISP_YNR_SKIN3_U_T; //	/* 0x15025808 */

typedef union {
    typedef ISP_YNR_SKIN3_U_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_SKIN3_U_T;

typedef struct {
    FIELD  YNR_SKIN3_V_THSL                           :  8;		/*  0.. 7, 0x000000FF */
    FIELD  YNR_SKIN3_V_THH                            :  8;		/*  8..15, 0x0000FF00 */
    FIELD  YNR_SKIN3_V_THL                            :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                     :  8;		/* 24..31, 0xFF000000 */
} ISP_YNR_SKIN3_V_T; //	/* 0x1502580C */

typedef union {
    typedef ISP_YNR_SKIN3_V_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_SKIN3_V_T;

typedef struct {
    FIELD  YNR_RSV1                                   : 32;		/*  0..31, 0xFFFFFFFF */
} ISP_YNR_RSV1_T; //	/* 0x15025810 */

typedef union {
    typedef ISP_YNR_RSV1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_RSV1_T;

typedef struct {
    FIELD  YNR_DGLC_EN                                :  1;		/*  0.. 0, 0x00000001 */
    FIELD  rsv_1                                      : 31;		/*  1..31, 0xFFFFFFFE */
} ISP_YNR_DGLC_CTRL_T; //	/* 0x15025830 */

typedef union {
    typedef ISP_YNR_DGLC_CTRL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_DGLC_CTRL_T;

typedef struct {
    FIELD  YNR_DGLC_Y_TH                              : 10;		/*  0.. 9, 0x000003FF */
    FIELD  YNR_DGLC_U_TH                              : 10;		/* 10..19, 0x000FFC00 */
    FIELD  YNR_DGLC_V_TH                              : 10;		/* 20..29, 0x3FF00000 */
    FIELD  rsv_30                                     :  2;		/* 30..31, 0xC0000000 */
} ISP_YNR_DGLC_TH_T; //	/* 0x15025834 */

typedef union {
    typedef ISP_YNR_DGLC_TH_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_DGLC_TH_T;

typedef struct {
    FIELD  YNR_DGLC_Y_GAIN1                           : 14;		/*  0..13, 0x00003FFF */
    FIELD  YNR_DGLC_Y_GAIN2                           : 14;		/* 14..27, 0x0FFFC000 */
    FIELD  rsv_28                                     :  4;		/* 28..31, 0xF0000000 */
} ISP_YNR_DGLC_GAIN_Y_T; //	/* 0x15025838 */

typedef union {
    typedef ISP_YNR_DGLC_GAIN_Y_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_DGLC_GAIN_Y_T;

typedef struct {
    FIELD  YNR_DGLC_U_GAIN1                           : 14;		/*  0..13, 0x00003FFF */
    FIELD  YNR_DGLC_U_GAIN2                           : 14;		/* 14..27, 0x0FFFC000 */
    FIELD  rsv_28                                     :  4;		/* 28..31, 0xF0000000 */
} ISP_YNR_DGLC_GAIN_U_T; //	/* 0x1502583C */

typedef union {
    typedef ISP_YNR_DGLC_GAIN_U_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_DGLC_GAIN_U_T;

typedef struct {
    FIELD  YNR_DGLC_V_GAIN1                           : 14;		/*  0..13, 0x00003FFF */
    FIELD  YNR_DGLC_V_GAIN2                           : 14;		/* 14..27, 0x0FFFC000 */
    FIELD  rsv_28                                     :  4;		/* 28..31, 0xF0000000 */
} ISP_YNR_DGLC_GAIN_V_T; //	/* 0x15025840 */

typedef union {
    typedef ISP_YNR_DGLC_GAIN_V_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_DGLC_GAIN_V_T;

typedef struct {
    FIELD  YNR_DGLC_Y_OFFSET1                         : 12;		/*  0..11, 0x00000FFF */
    FIELD  YNR_DGLC_Y_OFFSET2                         : 12;		/* 12..23, 0x00FFF000 */
    FIELD  rsv_24                                     :  8;		/* 24..31, 0xFF000000 */
} ISP_YNR_DGLC_OFST_Y_T; //	/* 0x15025844 */

typedef union {
    typedef ISP_YNR_DGLC_OFST_Y_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_DGLC_OFST_Y_T;

typedef struct {
    FIELD  YNR_DGLC_U_OFFSET1                         : 12;		/*  0..11, 0x00000FFF */
    FIELD  YNR_DGLC_U_OFFSET2                         : 12;		/* 12..23, 0x00FFF000 */
    FIELD  rsv_24                                     :  8;		/* 24..31, 0xFF000000 */
} ISP_YNR_DGLC_OFST_U_T; //	/* 0x15025848 */

typedef union {
    typedef ISP_YNR_DGLC_OFST_U_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_DGLC_OFST_U_T;

typedef struct {
    FIELD  YNR_DGLC_V_OFFSET1                         : 12;		/*  0..11, 0x00000FFF */
    FIELD  YNR_DGLC_V_OFFSET2                         : 12;		/* 12..23, 0x00FFF000 */
    FIELD  rsv_24                                     :  8;		/* 24..31, 0xFF000000 */
} ISP_YNR_DGLC_OFST_V_T; //	/* 0x1502584C */

typedef union {
    typedef ISP_YNR_DGLC_OFST_V_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_YNR_DGLC_OFST_V_T;

typedef union {
    enum { COUNT = 49 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_YNR_CON1_T  			con1;
        ISP_NVRAM_YNR_CON2_T  			con2;
        ISP_NVRAM_YNR_YAD1_T  			yad1;
        ISP_NVRAM_YNR_YAD2_T  			yad2;
        ISP_NVRAM_YNR_Y4LUT1_T  		y4lut1;
        ISP_NVRAM_YNR_Y4LUT2_T  		y4lut2;
        ISP_NVRAM_YNR_Y4LUT3_T  		y4lut3;
        ISP_NVRAM_YNR_C4LUT1_T  		c4lut1;
        ISP_NVRAM_YNR_C4LUT2_T  		c4lut2;
        ISP_NVRAM_YNR_C4LUT3_T  		c4lut3;
        ISP_NVRAM_YNR_A4LUT2_T  		a4lut2;
        ISP_NVRAM_YNR_A4LUT3_T  		a4lut3;
        ISP_NVRAM_YNR_L4LUT1_T  		l4lut1;
        ISP_NVRAM_YNR_L4LUT2_T  		l4lut2;
        ISP_NVRAM_YNR_L4LUT3_T  		l4lut3;
        ISP_NVRAM_YNR_PTY0V_T  			pty0v;
        ISP_NVRAM_YNR_CAD_T  			cad;
        ISP_NVRAM_YNR_PTY1V_T  			pty1v;
        ISP_NVRAM_YNR_SL2_T  			sl2;
        ISP_NVRAM_YNR_PTY2V_T  			pty2v;
        ISP_NVRAM_YNR_PTY3V_T  			pty3v;
        ISP_NVRAM_YNR_PTY0H_T  			pty0h;
        ISP_NVRAM_YNR_PTY1H_T  			pty1h;
        ISP_NVRAM_YNR_PTY2H_T  			pty2h;
        ISP_NVRAM_YNR_T4LUT1_T  		t4lut1;
        ISP_NVRAM_YNR_T4LUT2_T  		t4lut2;
        ISP_NVRAM_YNR_T4LUT3_T  		t4lut3;
        ISP_NVRAM_YNR_ACT1_T  			act1;
        ISP_NVRAM_YNR_PTY3H_T  			pty3h;
        ISP_NVRAM_YNR_PTCV_T  			ptcv;
        ISP_NVRAM_YNR_ACT4_T  			act4;
        ISP_NVRAM_YNR_PTCH_T  			ptch;
        ISP_NVRAM_YNR_YLVL0_T  			ylvl0;
        ISP_NVRAM_YNR_YLVL1_T  			ylvl1;
        ISP_NVRAM_YNR_HF_COR_T  		hf_cor;
        ISP_NVRAM_YNR_HF_ACT0_T  		hf_act0;
        ISP_NVRAM_YNR_HF_ACT1_T  		hf_act1;
        ISP_NVRAM_YNR_ACTC_T  			actc;
        ISP_NVRAM_YNR_YLAD_T  			ylad;
        ISP_NVRAM_YNR_HF_ACT2_T  		hf_act2;
        ISP_NVRAM_YNR_HF_ACT3_T  		hf_act3;
        ISP_NVRAM_YNR_HF_LUMA0_T  		hf_luma0;
        ISP_NVRAM_YNR_HF_LUMA1_T  		hf_luma1;
        //ISP_NVRAM_YNR_LCE_GAIN1_T  		lce_gain1;
        //ISP_NVRAM_YNR_LCE_GAIN2_T  		lce_gain2;
        //ISP_NVRAM_YNR_LCE_LUTP1_T  		lce_lutp1;
        //ISP_NVRAM_YNR_LCE_LUTP2_T  		lce_lutp2;
        //ISP_NVRAM_YNR_LCE_LUTO1_T  		lce_luto1;
        //ISP_NVRAM_YNR_LCE_LUTO2_T  		lce_luto2;
        //ISP_NVRAM_YNR_LCE_LUTS1_T  		lce_luts1;
        //ISP_NVRAM_YNR_LCE_LUTS2_T  		lce_luts2;
        //ISP_NVRAM_YNR_LCE_LUTS3_T  		lce_luts3;
        //ISP_NVRAM_YNR_LCE_LUTS4_T  		lce_luts4;
        ISP_NVRAM_YNR_Y4LUT4_T  		y4lut4;
        ISP_NVRAM_YNR_Y4LUT5_T  		y4lut5;
        ISP_NVRAM_YNR_Y4LUT6_T  		y4lut6;
        ISP_NVRAM_YNR_Y4LUT7_T  		y4lut7;
        ISP_NVRAM_YNR_A4LUT1_T  		a4lut1;
        ISP_NVRAM_YNR_SKIN_CON_T  		skin_con;
        //ISP_NVRAM_YNR_SKIN1_Y_T  		skin1_y;
        //ISP_NVRAM_YNR_SKIN1_U_T  		skin1_u;
        //ISP_NVRAM_YNR_SKIN1_V_T  		skin1_v;
        //ISP_NVRAM_YNR_SKIN2_Y_T  		skin2_y;
        //ISP_NVRAM_YNR_SKIN2_U_T  		skin2_u;
        //ISP_NVRAM_YNR_SKIN2_V_T  		skin2_v;
        //ISP_NVRAM_YNR_SKIN3_Y_T  		skin3_y;
        //ISP_NVRAM_YNR_SKIN3_U_T  		skin3_u;
        //ISP_NVRAM_YNR_SKIN3_V_T  		skin3_v;
        //ISP_NVRAM_YNR_RSV1_T  		rsv1;
        //ISP_NVRAM_YNR_DGLC_CTRL_T  	dglc_ctrl;
        //ISP_NVRAM_YNR_DGLC_TH_T  		dglc_th;
        //ISP_NVRAM_YNR_DGLC_GAIN_Y_T  	dglc_gain_y;
        //ISP_NVRAM_YNR_DGLC_GAIN_U_T  	dglc_gain_u;
        //ISP_NVRAM_YNR_DGLC_GAIN_V_T  	dglc_gain_v;
        //ISP_NVRAM_YNR_DGLC_OFST_Y_T  	dglc_ofst_y;
        //ISP_NVRAM_YNR_DGLC_OFST_U_T  	dglc_ofst_u;
        //ISP_NVRAM_YNR_DGLC_OFST_V_T  	dglc_ofst_v;
    };
} ISP_NVRAM_YNR_T;


typedef union {
    enum { COUNT = 10 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_YNR_SKIN_CON_T  		skin_con;
        ISP_NVRAM_YNR_SKIN1_Y_T  		skin1_y;
        ISP_NVRAM_YNR_SKIN1_U_T  		skin1_u;
        ISP_NVRAM_YNR_SKIN1_V_T  		skin1_v;
        ISP_NVRAM_YNR_SKIN2_Y_T  		skin2_y;
        ISP_NVRAM_YNR_SKIN2_U_T  		skin2_u;
        ISP_NVRAM_YNR_SKIN2_V_T  		skin2_v;
        ISP_NVRAM_YNR_SKIN3_Y_T  		skin3_y;
        ISP_NVRAM_YNR_SKIN3_U_T  		skin3_u;
        ISP_NVRAM_YNR_SKIN3_V_T  		skin3_v;
    };
} ISP_NVRAM_YNR_SKIN_T;

typedef union {
    enum { COUNT = 8 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_YNR_LCE_LUTP1_T         lce_lutp1;
        ISP_NVRAM_YNR_LCE_LUTP2_T         lce_lutp2;
        ISP_NVRAM_YNR_LCE_LUTO1_T         lce_luto1;
        ISP_NVRAM_YNR_LCE_LUTO2_T         lce_luto2;
        ISP_NVRAM_YNR_LCE_LUTS1_T         lce_luts1;
        ISP_NVRAM_YNR_LCE_LUTS2_T         lce_luts2;
        ISP_NVRAM_YNR_LCE_LUTS3_T         lce_luts3;
        ISP_NVRAM_YNR_LCE_LUTS4_T         lce_luts4;
    };
} ISP_NVRAM_YNR_LCE_OUT_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// EE
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#if 0
typedef struct {
    FIELD  EE_TILE_EDGE                               :  4;		/*  0.. 3, 0x0000000F */
    FIELD  rsv_4                                      : 28;		/*  4..31, 0xFFFFFFF0 */
} ISP_EE_CTRL_T; //	/* 0x15027200 */

typedef union {
    typedef ISP_EE_CTRL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_EE_CTRL_T;

typedef struct {
    FIELD  EE_OUT_EDGE_SEL                            :  2;		/*  0.. 1, 0x00000003 */
    FIELD  rsv_2                                      :  6;		/*  2.. 7, 0x000000FC */
    FIELD  EE_VIDEO_MODE                              :  1;		/*  8.. 8, 0x00000100 */
    FIELD  rsv_9                                      : 23;		/*  9..31, 0xFFFFFE00 */
} ISP_EE_TOP_CTRL_T; //	/* 0x15027204 */

typedef union {
    typedef ISP_EE_TOP_CTRL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_EE_TOP_CTRL_T;
#endif
typedef struct {
    FIELD  EE_H1_DI_BLND_OFST                         :  8;		/*  0.. 7, 0x000000FF */
    FIELD  EE_H2_DI_BLND_OFST                         :  8;		/*  8..15, 0x0000FF00 */
    FIELD  EE_H3_DI_BLND_OFST                         :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                     :  8;		/* 24..31, 0xFF000000 */
} ISP_EE_BLND_CTRL_1_T; //	/* 0x15027208 */

typedef union {
    typedef ISP_EE_BLND_CTRL_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_EE_BLND_CTRL_1_T;

typedef struct {
    FIELD  EE_H1_DI_BLND_SL                           :  8;		/*  0.. 7, 0x000000FF */
    FIELD  EE_H2_DI_BLND_SL                           :  8;		/*  8..15, 0x0000FF00 */
    FIELD  EE_H3_DI_BLND_SL                           :  8;		/* 16..23, 0x00FF0000 */
    FIELD  EE_HX_ISO_BLND_RAT                         :  5;		/* 24..28, 0x1F000000 */
    FIELD  rsv_29                                     :  3;		/* 29..31, 0xE0000000 */
} ISP_EE_BLND_CTRL_2_T; //	/* 0x1502720C */

typedef union {
    typedef ISP_EE_BLND_CTRL_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_EE_BLND_CTRL_2_T;

typedef struct {
    FIELD  EE_H1_FLT_CORE_TH                          :  8;		/*  0.. 7, 0x000000FF */
    FIELD  EE_H2_FLT_CORE_TH                          :  8;		/*  8..15, 0x0000FF00 */
    FIELD  EE_H3_FLT_CORE_TH                          :  8;		/* 16..23, 0x00FF0000 */
    FIELD  EE_FLT_CORE_TH                             :  8;		/* 24..31, 0xFF000000 */
} ISP_EE_CORE_CTRL_T; //	/* 0x15027210 */

typedef union {
    typedef ISP_EE_CORE_CTRL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_EE_CORE_CTRL_T;

typedef struct {
    FIELD  EE_H1_GN                                   :  5;		/*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                      :  3;		/*  5.. 7, 0x000000E0 */
    FIELD  EE_H2_GN                                   :  5;		/*  8..12, 0x00001F00 */
    FIELD  rsv_13                                     :  3;		/* 13..15, 0x0000E000 */
    FIELD  EE_H3_GN                                   :  5;		/* 16..20, 0x001F0000 */
    FIELD  EE_LUMA_MOD_Y6                             :  9;		/* 21..29, 0x3FE00000 */
    FIELD  rsv_30                                     :  2;		/* 30..31, 0xC0000000 */
} ISP_EE_GN_CTRL_1_T; //	/* 0x15027214 */

typedef union {
    typedef ISP_EE_GN_CTRL_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_EE_GN_CTRL_1_T;

typedef struct {
    FIELD  EE_LUMA_MOD_Y0                             :  9;		/*  0.. 8, 0x000001FF */
    FIELD  rsv_9                                      :  1;		/*  9.. 9, 0x00000200 */
    FIELD  EE_LUMA_MOD_Y1                             :  9;		/* 10..18, 0x0007FC00 */
    FIELD  rsv_19                                     :  1;		/* 19..19, 0x00080000 */
    FIELD  EE_LUMA_MOD_Y2                             :  9;		/* 20..28, 0x1FF00000 */
    FIELD  EE_LUMA_MOD_EN                             :  1;		/* 29..29, 0x20000000 */
    FIELD  rsv_30                                     :  2;		/* 30..31, 0xC0000000 */
} ISP_EE_LUMA_CTRL_1_T; //	/* 0x15027218 */

typedef union {
    typedef ISP_EE_LUMA_CTRL_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_EE_LUMA_CTRL_1_T;

typedef struct {
    FIELD  EE_LUMA_MOD_Y3                             :  9;		/*  0.. 8, 0x000001FF */
    FIELD  rsv_9                                      :  1;		/*  9.. 9, 0x00000200 */
    FIELD  EE_LUMA_MOD_Y4                             :  9;		/* 10..18, 0x0007FC00 */
    FIELD  rsv_19                                     :  1;		/* 19..19, 0x00080000 */
    FIELD  EE_LUMA_MOD_Y5                             :  9;		/* 20..28, 0x1FF00000 */
    FIELD  rsv_29                                     :  3;		/* 29..31, 0xE0000000 */
} ISP_EE_LUMA_CTRL_2_T; //	/* 0x1502721C */

typedef union {
    typedef ISP_EE_LUMA_CTRL_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_EE_LUMA_CTRL_2_T;

typedef struct {
    FIELD  EE_SLNK_GN_Y0                              :  9;		/*  0.. 8, 0x000001FF */
    FIELD  EE_SLNK_GN_Y1                              :  9;		/*  9..17, 0x0003FE00 */
    FIELD  EE_SLNK_GN_Y2                              :  9;		/* 18..26, 0x07FC0000 */
    FIELD  EE_GLUT_LINK_EN                            :  1;		/* 27..27, 0x08000000 */
    FIELD  rsv_28                                     :  4;		/* 28..31, 0xF0000000 */
} ISP_EE_LUMA_SLNK_CTRL_T; //	/* 0x15027220 */

typedef union {
    typedef ISP_EE_LUMA_SLNK_CTRL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_EE_LUMA_SLNK_CTRL_T;

typedef struct {
    FIELD  EE_GLUT_S1                                 :  8;		/*  0.. 7, 0x000000FF */
    FIELD  EE_GLUT_X1                                 :  8;		/*  8..15, 0x0000FF00 */
    FIELD  EE_GLUT_Y1                                 : 10;		/* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                     :  6;		/* 26..31, 0xFC000000 */
} ISP_EE_GLUT_CTRL_1_T; //	/* 0x15027224 */

typedef union {
    typedef ISP_EE_GLUT_CTRL_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_EE_GLUT_CTRL_1_T;

typedef struct {
    FIELD  EE_GLUT_S2                                 :  8;		/*  0.. 7, 0x000000FF */
    FIELD  EE_GLUT_X2                                 :  8;		/*  8..15, 0x0000FF00 */
    FIELD  EE_GLUT_Y2                                 : 10;		/* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                     :  6;		/* 26..31, 0xFC000000 */
} ISP_EE_GLUT_CTRL_2_T; //	/* 0x15027228 */

typedef union {
    typedef ISP_EE_GLUT_CTRL_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_EE_GLUT_CTRL_2_T;

typedef struct {
    FIELD  EE_GLUT_S3                                 :  8;		/*  0.. 7, 0x000000FF */
    FIELD  EE_GLUT_X3                                 :  8;		/*  8..15, 0x0000FF00 */
    FIELD  EE_GLUT_Y3                                 : 10;		/* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                     :  6;		/* 26..31, 0xFC000000 */
} ISP_EE_GLUT_CTRL_3_T; //	/* 0x1502722C */

typedef union {
    typedef ISP_EE_GLUT_CTRL_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_EE_GLUT_CTRL_3_T;

typedef struct {
    FIELD  EE_GLUT_S4                                 :  8;		/*  0.. 7, 0x000000FF */
    FIELD  EE_GLUT_X4                                 :  8;		/*  8..15, 0x0000FF00 */
    FIELD  EE_GLUT_Y4                                 : 10;		/* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                     :  6;		/* 26..31, 0xFC000000 */
} ISP_EE_GLUT_CTRL_4_T; //	/* 0x15027230 */

typedef union {
    typedef ISP_EE_GLUT_CTRL_4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_EE_GLUT_CTRL_4_T;

typedef struct {
    FIELD  EE_GLUT_S5                                 :  8;		/*  0.. 7, 0x000000FF */
    FIELD  rsv_8                                      :  8;		/*  8..15, 0x0000FF00 */
    FIELD  EE_GLUT_SL_DEC_Y                           : 10;		/* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                     :  6;		/* 26..31, 0xFC000000 */
} ISP_EE_GLUT_CTRL_5_T; //	/* 0x15027234 */

typedef union {
    typedef ISP_EE_GLUT_CTRL_5_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_EE_GLUT_CTRL_5_T;

typedef struct {
    FIELD  EE_GLUT_TH_OVR                             :  8;		/*  0.. 7, 0x000000FF */
    FIELD  EE_GLUT_TH_UND                             :  8;		/*  8..15, 0x0000FF00 */
    FIELD  EE_GLUT_TH_MIN                             :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                     :  8;		/* 24..31, 0xFF000000 */
} ISP_EE_GLUT_CTRL_6_T; //	/* 0x15027238 */

typedef union {
    typedef ISP_EE_GLUT_CTRL_6_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_EE_GLUT_CTRL_6_T;

typedef struct {
    FIELD  EE_RESP_SMO_STR                            :  3;		/*  0.. 2, 0x00000007 */
    FIELD  rsv_3                                      :  1;		/*  3.. 3, 0x00000008 */
    FIELD  EE_OVRSH_CLIP_STR                          :  3;		/*  4.. 6, 0x00000070 */
    FIELD  rsv_7                                      :  1;		/*  7.. 7, 0x00000080 */
    FIELD  EE_DOT_REDUC_AMNT                          :  8;		/*  8..15, 0x0000FF00 */
    FIELD  EE_DOT_TH                                  :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                     :  8;		/* 24..31, 0xFF000000 */
} ISP_EE_ARTIFACT_CTRL_T; //	/* 0x1502723C */

typedef union {
    typedef ISP_EE_ARTIFACT_CTRL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_EE_ARTIFACT_CTRL_T;

typedef struct {
    FIELD  EE_RESP_CLIP                               :  8;		/*  0.. 7, 0x000000FF */
    FIELD  EE_RESP_CLIP_LUMA_SPC_TH                   :  8;		/*  8..15, 0x0000FF00 */
    FIELD  EE_RESP_CLIP_LUMA_LWB                      :  8;		/* 16..23, 0x00FF0000 */
    FIELD  EE_RESP_CLIP_LUMA_UPB                      :  8;		/* 24..31, 0xFF000000 */
} ISP_EE_CLIP_CTRL_T; //	/* 0x15027240 */

typedef union {
    typedef ISP_EE_CLIP_CTRL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_EE_CLIP_CTRL_T;

typedef struct {
    FIELD  EE_MASTER_GN_NEG                           :  8;		/*  0.. 7, 0x000000FF */
    FIELD  EE_MASTER_GN_POS                           :  8;		/*  8..15, 0x0000FF00 */
    FIELD  rsv_16                                     : 16;		/* 16..31, 0xFFFF0000 */
} ISP_EE_GN_CTRL_2_T; //	/* 0x15027244 */

typedef union {
    typedef ISP_EE_GN_CTRL_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_EE_GN_CTRL_2_T;

typedef struct {
    FIELD  EE_ST_UB                                   :  8;		/*  0.. 7, 0x000000FF */
    FIELD  EE_ST_LB                                   :  8;		/*  8..15, 0x0000FF00 */
    FIELD  rsv_16                                     : 16;		/* 16..31, 0xFFFF0000 */
} ISP_EE_ST_CTRL_1_T; //	/* 0x15027248 */

typedef union {
    typedef ISP_EE_ST_CTRL_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_EE_ST_CTRL_1_T;

typedef struct {
    FIELD  EE_ST_SL_CE                                :  8;		/*  0.. 7, 0x000000FF */
    FIELD  EE_ST_OFST_CE                              :  8;		/*  8..15, 0x0000FF00 */
    FIELD  EE_ST_SL_RESP                              :  8;		/* 16..23, 0x00FF0000 */
    FIELD  EE_ST_OFST_RESP                            :  8;		/* 24..31, 0xFF000000 */
} ISP_EE_ST_CTRL_2_T; //	/* 0x1502724C */

typedef union {
    typedef ISP_EE_ST_CTRL_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_EE_ST_CTRL_2_T;

typedef struct {
    FIELD  EE_LUMA_LMT_DIFF                           :  8;		/*  0.. 7, 0x000000FF */
    FIELD  EE_LUMA_CNTST_LV                           :  3;		/*  8..10, 0x00000700 */
    FIELD  rsv_11                                     :  1;		/* 11..11, 0x00000800 */
    FIELD  EE_LUMA_MINI                               :  3;		/* 12..14, 0x00007000 */
    FIELD  rsv_15                                     :  1;		/* 15..15, 0x00008000 */
    FIELD  EE_LUMA_MAXI                               :  3;		/* 16..18, 0x00070000 */
    FIELD  rsv_19                                     :  1;		/* 19..19, 0x00080000 */
    FIELD  EE_CHR_CNTST_LV                            :  3;		/* 20..22, 0x00700000 */
    FIELD  rsv_23                                     :  1;		/* 23..23, 0x00800000 */
    FIELD  EE_CHR_MINI                                :  2;		/* 24..25, 0x03000000 */
    FIELD  rsv_26                                     :  2;		/* 26..27, 0x0C000000 */
    FIELD  EE_CHR_MAXI                                :  2;		/* 28..29, 0x30000000 */
    FIELD  rsv_30                                     :  2;		/* 30..31, 0xC0000000 */
} ISP_EE_CE_CTRL_T; //	/* 0x15027250 */

typedef union {
    typedef ISP_EE_CE_CTRL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_EE_CE_CTRL_T;

typedef struct {
    FIELD  EE_CE_SLMOD_EN                             :  1;		/*  0.. 0, 0x00000001 */
    FIELD  EE_CE_SLMOD_Y1                             :  4;		/*  1.. 4, 0x0000001E */
    FIELD  EE_CE_SLMOD_Y2                             :  4;		/*  5.. 8, 0x000001E0 */
    FIELD  rsv_9                                      : 23;		/*  9..31, 0xFFFFFE00 */
} ISP_EE_CE_SL_CTRL_T; //	/* 0x15027254 */

typedef union {
    typedef ISP_EE_CE_SL_CTRL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_EE_CE_SL_CTRL_T;

typedef struct {
    FIELD  EE_CBOOST_LMT_U                            :  8;		/*  0.. 7, 0x000000FF */
    FIELD  EE_CBOOST_LMT_L                            :  8;		/*  8..15, 0x0000FF00 */
    FIELD  EE_CBOOST_GAIN                             :  8;		/* 16..23, 0x00FF0000 */
    FIELD  EE_CBOOST_EN                               :  1;		/* 24..24, 0x01000000 */
    FIELD  rsv_25                                     :  7;		/* 25..31, 0xFE000000 */
} ISP_EE_CBOOST_CTRL_1_T; //	/* 0x15027258 */

typedef union {
    typedef ISP_EE_CBOOST_CTRL_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_EE_CBOOST_CTRL_1_T;

typedef struct {
    FIELD  EE_CBOOST_YCONST                           :  8;		/*  0.. 7, 0x000000FF */
    FIELD  EE_CBOOST_YOFFSET                          :  8;		/*  8..15, 0x0000FF00 */
    FIELD  EE_CBOOST_YOFFSET_SEL                      :  2;		/* 16..17, 0x00030000 */
    FIELD  rsv_18                                     : 14;		/* 18..31, 0xFFFC0000 */
} ISP_EE_CBOOST_CTRL_2_T; //	/* 0x1502725C */

typedef union {
    typedef ISP_EE_CBOOST_CTRL_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_EE_CBOOST_CTRL_2_T;

typedef struct {
    FIELD  EE_PBC1_LUMA_THL                           :  8;		/*  0.. 7, 0x000000FF */
    FIELD  EE_PBC1_EDS_EN                             :  1;		/*  8.. 8, 0x00000100 */
    FIELD  rsv_9                                      :  3;		/*  9..11, 0x00000E00 */
    FIELD  EE_PBC1_EN                                 :  1;		/* 12..12, 0x00001000 */
    FIELD  rsv_13                                     :  3;		/* 13..15, 0x0000E000 */
    FIELD  EE_PBC_EDS_SL                              :  6;		/* 16..21, 0x003F0000 */
    FIELD  EE_PBC_EDS_TH                              :  8;		/* 22..29, 0x3FC00000 */
    FIELD  rsv_30                                     :  1;		/* 30..30, 0x40000000 */
    FIELD  EE_PBC_EN                                  :  1;		/* 31..31, 0x80000000 */
} ISP_EE_PBC1_CTRL_0_T; //	/* 0x15027260 */

typedef union {
    typedef ISP_EE_PBC1_CTRL_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_EE_PBC1_CTRL_0_T;

typedef struct {
    FIELD  EE_PBC1_HUE_THH                            :  8;		/*  0.. 7, 0x000000FF */
    FIELD  EE_PBC1_HUE_THL                            :  8;		/*  8..15, 0x0000FF00 */
    FIELD  EE_PBC1_LUMA_SL                            :  8;		/* 16..23, 0x00FF0000 */
    FIELD  EE_PBC1_LUMA_THH                           :  8;		/* 24..31, 0xFF000000 */
} ISP_EE_PBC1_CTRL_1_T; //	/* 0x15027264 */

typedef union {
    typedef ISP_EE_PBC1_CTRL_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_EE_PBC1_CTRL_1_T;

typedef struct {
    FIELD  EE_PBC1_SAT_SL                             :  8;		/*  0.. 7, 0x000000FF */
    FIELD  EE_PBC1_SAT_THH                            :  8;		/*  8..15, 0x0000FF00 */
    FIELD  EE_PBC1_SAT_THL                            :  8;		/* 16..23, 0x00FF0000 */
    FIELD  EE_PBC1_HUE_SL                             :  8;		/* 24..31, 0xFF000000 */
} ISP_EE_PBC1_CTRL_2_T; //	/* 0x15027268 */

typedef union {
    typedef ISP_EE_PBC1_CTRL_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_EE_PBC1_CTRL_2_T;

typedef struct {
    FIELD  EE_PBC1_LPF_GAIN                           :  6;		/*  0.. 5, 0x0000003F */
    FIELD  EE_PBC1_LPF_EN                             :  1;		/*  6.. 6, 0x00000040 */
    FIELD  EE_PBC1_EDGE_EN                            :  1;		/*  7.. 7, 0x00000080 */
    FIELD  EE_PBC1_EDGE_SLOPE                         :  6;		/*  8..13, 0x00003F00 */
    FIELD  EE_PBC1_EDGE_THR                           :  6;		/* 14..19, 0x000FC000 */
    FIELD  EE_PBC1_CONF_GAIN                          :  4;		/* 20..23, 0x00F00000 */
    FIELD  EE_PBC1_GAIN                               :  8;		/* 24..31, 0xFF000000 */
} ISP_EE_PBC1_CTRL_3_T; //	/* 0x1502726C */

typedef union {
    typedef ISP_EE_PBC1_CTRL_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_EE_PBC1_CTRL_3_T;

typedef struct {
    FIELD  EE_PBC2_LUMA_THL                           :  8;		/*  0.. 7, 0x000000FF */
    FIELD  EE_PBC2_EDS_EN                             :  1;		/*  8.. 8, 0x00000100 */
    FIELD  rsv_9                                      :  3;		/*  9..11, 0x00000E00 */
    FIELD  EE_PBC2_EN                                 :  1;		/* 12..12, 0x00001000 */
    FIELD  rsv_13                                     : 19;		/* 13..31, 0xFFFFE000 */
} ISP_EE_PBC2_CTRL_0_T; //	/* 0x15027270 */

typedef union {
    typedef ISP_EE_PBC2_CTRL_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_EE_PBC2_CTRL_0_T;

typedef struct {
    FIELD  EE_PBC2_HUE_THH                            :  8;		/*  0.. 7, 0x000000FF */
    FIELD  EE_PBC2_HUE_THL                            :  8;		/*  8..15, 0x0000FF00 */
    FIELD  EE_PBC2_LUMA_SL                            :  8;		/* 16..23, 0x00FF0000 */
    FIELD  EE_PBC2_LUMA_THH                           :  8;		/* 24..31, 0xFF000000 */
} ISP_EE_PBC2_CTRL_1_T; //	/* 0x15027274 */

typedef union {
    typedef ISP_EE_PBC2_CTRL_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_EE_PBC2_CTRL_1_T;

typedef struct {
    FIELD  EE_PBC2_SAT_SL                             :  8;		/*  0.. 7, 0x000000FF */
    FIELD  EE_PBC2_SAT_THH                            :  8;		/*  8..15, 0x0000FF00 */
    FIELD  EE_PBC2_SAT_THL                            :  8;		/* 16..23, 0x00FF0000 */
    FIELD  EE_PBC2_HUE_SL                             :  8;		/* 24..31, 0xFF000000 */
} ISP_EE_PBC2_CTRL_2_T; //	/* 0x15027278 */

typedef union {
    typedef ISP_EE_PBC2_CTRL_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_EE_PBC2_CTRL_2_T;

typedef struct {
    FIELD  EE_PBC2_LPF_GAIN                           :  6;		/*  0.. 5, 0x0000003F */
    FIELD  EE_PBC2_LPF_EN                             :  1;		/*  6.. 6, 0x00000040 */
    FIELD  EE_PBC2_EDGE_EN                            :  1;		/*  7.. 7, 0x00000080 */
    FIELD  EE_PBC2_EDGE_SLOPE                         :  6;		/*  8..13, 0x00003F00 */
    FIELD  EE_PBC2_EDGE_THR                           :  6;		/* 14..19, 0x000FC000 */
    FIELD  EE_PBC2_CONF_GAIN                          :  4;		/* 20..23, 0x00F00000 */
    FIELD  EE_PBC2_GAIN                               :  8;		/* 24..31, 0xFF000000 */
} ISP_EE_PBC2_CTRL_3_T; //	/* 0x1502727C */

typedef union {
    typedef ISP_EE_PBC2_CTRL_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_EE_PBC2_CTRL_3_T;

typedef struct {
    FIELD  EE_PBC3_LUMA_THL                           :  8;		/*  0.. 7, 0x000000FF */
    FIELD  EE_PBC3_EDS_EN                             :  1;		/*  8.. 8, 0x00000100 */
    FIELD  rsv_9                                      :  3;		/*  9..11, 0x00000E00 */
    FIELD  EE_PBC3_EN                                 :  1;		/* 12..12, 0x00001000 */
    FIELD  rsv_13                                     : 19;		/* 13..31, 0xFFFFE000 */
} ISP_EE_PBC3_CTRL_0_T; //	/* 0x15027280 */

typedef union {
    typedef ISP_EE_PBC3_CTRL_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_EE_PBC3_CTRL_0_T;

typedef struct {
    FIELD  EE_PBC3_HUE_THH                            :  8;		/*  0.. 7, 0x000000FF */
    FIELD  EE_PBC3_HUE_THL                            :  8;		/*  8..15, 0x0000FF00 */
    FIELD  EE_PBC3_LUMA_SL                            :  8;		/* 16..23, 0x00FF0000 */
    FIELD  EE_PBC3_LUMA_THH                           :  8;		/* 24..31, 0xFF000000 */
} ISP_EE_PBC3_CTRL_1_T; //	/* 0x15027284 */

typedef union {
    typedef ISP_EE_PBC3_CTRL_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_EE_PBC3_CTRL_1_T;

typedef struct {
    FIELD  EE_PBC3_SAT_SL                             :  8;		/*  0.. 7, 0x000000FF */
    FIELD  EE_PBC3_SAT_THH                            :  8;		/*  8..15, 0x0000FF00 */
    FIELD  EE_PBC3_SAT_THL                            :  8;		/* 16..23, 0x00FF0000 */
    FIELD  EE_PBC3_HUE_SL                             :  8;		/* 24..31, 0xFF000000 */
} ISP_EE_PBC3_CTRL_2_T; //	/* 0x15027288 */

typedef union {
    typedef ISP_EE_PBC3_CTRL_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_EE_PBC3_CTRL_2_T;

typedef struct {
    FIELD  EE_PBC3_LPF_GAIN                           :  6;		/*  0.. 5, 0x0000003F */
    FIELD  EE_PBC3_LPF_EN                             :  1;		/*  6.. 6, 0x00000040 */
    FIELD  EE_PBC3_EDGE_EN                            :  1;		/*  7.. 7, 0x00000080 */
    FIELD  EE_PBC3_EDGE_SLOPE                         :  6;		/*  8..13, 0x00003F00 */
    FIELD  EE_PBC3_EDGE_THR                           :  6;		/* 14..19, 0x000FC000 */
    FIELD  EE_PBC3_CONF_GAIN                          :  4;		/* 20..23, 0x00F00000 */
    FIELD  EE_PBC3_GAIN                               :  8;		/* 24..31, 0xFF000000 */
} ISP_EE_PBC3_CTRL_3_T; //	/* 0x1502728C */

typedef union {
    typedef ISP_EE_PBC3_CTRL_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_EE_PBC3_CTRL_3_T;
#if 0
typedef struct {
    FIELD  EE_SE_KNEE_SEL                             :  2;		/*  0.. 1, 0x00000003 */
    FIELD  EE_SE_SPECL_HALF_MODE                      :  2;		/*  2.. 3, 0x0000000C */
    FIELD  EE_SE_SPECL_GAIN                           :  2;		/*  4.. 5, 0x00000030 */
    FIELD  EE_SE_SPECL_INV                            :  1;		/*  6.. 6, 0x00000040 */
    FIELD  EE_SE_SPECL_ABS                            :  1;		/*  7.. 7, 0x00000080 */
    FIELD  EE_SE_CONST_Y_EN                           :  1;		/*  8.. 8, 0x00000100 */
    FIELD  rsv_9                                      :  5;		/*  9..13, 0x00003E00 */
    FIELD  EE_SE_CONST_Y_VAL                          : 10;		/* 14..23, 0x00FFC000 */
    FIELD  EE_SE_YOUT_QBIT                            :  4;		/* 24..27, 0x0F000000 */
    FIELD  EE_SE_COUT_QBIT                            :  4;		/* 28..31, 0xF0000000 */
} ISP_EE_SE_Y_SPECL_CTRL_T; //	/* 0x15027290 */

typedef union {
    typedef ISP_EE_SE_Y_SPECL_CTRL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_EE_SE_Y_SPECL_CTRL_T;

typedef struct {
    FIELD  EE_SE_HORI_EDGE_SEL                        :  1;		/*  0.. 0, 0x00000001 */
    FIELD  EE_SE_HORI_EDGE_GAIN_A                     :  4;		/*  1.. 4, 0x0000001E */
    FIELD  EE_SE_HORI_EDGE_GAIN_B                     :  5;		/*  5.. 9, 0x000003E0 */
    FIELD  EE_SE_HORI_EDGE_GAIN_C                     :  5;		/* 10..14, 0x00007C00 */
    FIELD  rsv_15                                     :  1;		/* 15..15, 0x00008000 */
    FIELD  EE_SE_VERT_EDGE_SEL                        :  1;		/* 16..16, 0x00010000 */
    FIELD  EE_SE_VERT_EDGE_GAIN_A                     :  4;		/* 17..20, 0x001E0000 */
    FIELD  EE_SE_VERT_EDGE_GAIN_B                     :  5;		/* 21..25, 0x03E00000 */
    FIELD  EE_SE_VERT_EDGE_GAIN_C                     :  5;		/* 26..30, 0x7C000000 */
    FIELD  rsv_31                                     :  1;		/* 31..31, 0x80000000 */
} ISP_EE_SE_EDGE_CTRL_1_T; //	/* 0x15027294 */

typedef union {
    typedef ISP_EE_SE_EDGE_CTRL_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_EE_SE_EDGE_CTRL_1_T;

typedef struct {
    FIELD  EE_SE_CONVT_CORE_TH                        :  7;		/*  0.. 6, 0x0000007F */
    FIELD  rsv_7                                      :  1;		/*  7.. 7, 0x00000080 */
    FIELD  EE_SE_CONVT_E_TH                           :  8;		/*  8..15, 0x0000FF00 */
    FIELD  EE_SE_CONVT_SLOPE_SEL                      :  1;		/* 16..16, 0x00010000 */
    FIELD  EE_SE_CONVT_FORCE_EN                       :  1;		/* 17..17, 0x00020000 */
    FIELD  EE_SE_EDGE_DET_GAIN                        :  5;		/* 18..22, 0x007C0000 */
    FIELD  EE_SE_OIL_EN                               :  1;		/* 23..23, 0x00800000 */
    FIELD  EE_SE_BOSS_GAIN_OFF                        :  1;		/* 24..24, 0x01000000 */
    FIELD  EE_SE_BOSS_IN_SEL                          :  1;		/* 25..25, 0x02000000 */
    FIELD  rsv_26                                     :  6;		/* 26..31, 0xFC000000 */
} ISP_EE_SE_EDGE_CTRL_2_T; //	/* 0x15027298 */

typedef union {
    typedef ISP_EE_SE_EDGE_CTRL_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_EE_SE_EDGE_CTRL_2_T;

typedef struct {
    FIELD  EE_SE_CORE_HORI_X0                         :  7;		/*  0.. 6, 0x0000007F */
    FIELD  rsv_7                                      :  1;		/*  7.. 7, 0x00000080 */
    FIELD  EE_SE_CORE_HORI_X2                         :  8;		/*  8..15, 0x0000FF00 */
    FIELD  EE_SE_CORE_HORI_Y5                         :  6;		/* 16..21, 0x003F0000 */
    FIELD  EE_SE_CORE_HORI_SUP                        :  2;		/* 22..23, 0x00C00000 */
    FIELD  EE_SE_CORE_HORI_SDN                        :  2;		/* 24..25, 0x03000000 */
    FIELD  rsv_26                                     :  6;		/* 26..31, 0xFC000000 */
} ISP_EE_SE_CORE_CTRL_1_T; //	/* 0x1502729C */

typedef union {
    typedef ISP_EE_SE_CORE_CTRL_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_EE_SE_CORE_CTRL_1_T;

typedef struct {
    FIELD  EE_SE_CORE_VERT_X0                         :  7;		/*  0.. 6, 0x0000007F */
    FIELD  rsv_7                                      :  1;		/*  7.. 7, 0x00000080 */
    FIELD  EE_SE_CORE_VERT_X2                         :  8;		/*  8..15, 0x0000FF00 */
    FIELD  EE_SE_CORE_VERT_Y5                         :  6;		/* 16..21, 0x003F0000 */
    FIELD  EE_SE_CORE_VERT_SUP                        :  2;		/* 22..23, 0x00C00000 */
    FIELD  EE_SE_CORE_VERT_SDN                        :  2;		/* 24..25, 0x03000000 */
    FIELD  rsv_26                                     :  6;		/* 26..31, 0xFC000000 */
} ISP_EE_SE_CORE_CTRL_2_T; //	/* 0x150272A0 */

typedef union {
    typedef ISP_EE_SE_CORE_CTRL_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_EE_SE_CORE_CTRL_2_T;
#endif
typedef union {
    enum { COUNT = 34 };
    MUINT32 set[COUNT];
    struct {
        //ISP_NVRAM_EE_CTRL_T  			    ctrl;
        //ISP_NVRAM_EE_TOP_CTRL_T  			top_ctrl;
        ISP_NVRAM_EE_BLND_CTRL_1_T  		blnd_ctrl_1;
        ISP_NVRAM_EE_BLND_CTRL_2_T  		blnd_ctrl_2;
        ISP_NVRAM_EE_CORE_CTRL_T  			core_ctrl;
        ISP_NVRAM_EE_GN_CTRL_1_T  			gn_ctrl_1;
        ISP_NVRAM_EE_LUMA_CTRL_1_T  		luma_ctrl_1;
        ISP_NVRAM_EE_LUMA_CTRL_2_T  		luma_ctrl_2;
        ISP_NVRAM_EE_LUMA_SLNK_CTRL_T  		luma_slnk_ctrl;
        ISP_NVRAM_EE_GLUT_CTRL_1_T  		glut_ctrl_1;
        ISP_NVRAM_EE_GLUT_CTRL_2_T  		glut_ctrl_2;
        ISP_NVRAM_EE_GLUT_CTRL_3_T  		glut_ctrl_3;
        ISP_NVRAM_EE_GLUT_CTRL_4_T  		glut_ctrl_4;
        ISP_NVRAM_EE_GLUT_CTRL_5_T  		glut_ctrl_5;
        ISP_NVRAM_EE_GLUT_CTRL_6_T  		glut_ctrl_6;
        ISP_NVRAM_EE_ARTIFACT_CTRL_T  		artifact_ctrl;
        ISP_NVRAM_EE_CLIP_CTRL_T  			clip_ctrl;
        ISP_NVRAM_EE_GN_CTRL_2_T  			gn_ctrl_2;
        ISP_NVRAM_EE_ST_CTRL_1_T  			st_ctrl_1;
        ISP_NVRAM_EE_ST_CTRL_2_T  			st_ctrl_2;
        ISP_NVRAM_EE_CE_CTRL_T  			ce_ctrl;
        ISP_NVRAM_EE_CE_SL_CTRL_T  			ce_sl_ctrl;
        ISP_NVRAM_EE_CBOOST_CTRL_1_T  		cboost_ctrl_1;
        ISP_NVRAM_EE_CBOOST_CTRL_2_T  		cboost_ctrl_2;
        ISP_NVRAM_EE_PBC1_CTRL_0_T  		pbc1_ctrl_0;
        ISP_NVRAM_EE_PBC1_CTRL_1_T  		pbc1_ctrl_1;
        ISP_NVRAM_EE_PBC1_CTRL_2_T  		pbc1_ctrl_2;
        ISP_NVRAM_EE_PBC1_CTRL_3_T  		pbc1_ctrl_3;
        ISP_NVRAM_EE_PBC2_CTRL_0_T  		pbc2_ctrl_0;
        ISP_NVRAM_EE_PBC2_CTRL_1_T  		pbc2_ctrl_1;
        ISP_NVRAM_EE_PBC2_CTRL_2_T  		pbc2_ctrl_2;
        ISP_NVRAM_EE_PBC2_CTRL_3_T  		pbc2_ctrl_3;
        ISP_NVRAM_EE_PBC3_CTRL_0_T  		pbc3_ctrl_0;
        ISP_NVRAM_EE_PBC3_CTRL_1_T  		pbc3_ctrl_1;
        ISP_NVRAM_EE_PBC3_CTRL_2_T  		pbc3_ctrl_2;
        ISP_NVRAM_EE_PBC3_CTRL_3_T  		pbc3_ctrl_3;
        //ISP_NVRAM_EE_SE_Y_SPECL_CTRL_T  	se_y_specl_ctrl;
        //ISP_NVRAM_EE_SE_EDGE_CTRL_1_T  		se_edge_ctrl_1;
        //ISP_NVRAM_EE_SE_EDGE_CTRL_2_T  		se_edge_ctrl_2;
        //ISP_NVRAM_EE_SE_CORE_CTRL_1_T  		se_core_ctrl_1;
        //ISP_NVRAM_EE_SE_CORE_CTRL_2_T  		se_core_ctrl_2;
    };
} ISP_NVRAM_EE_T;
#if 0
typedef union {
    enum { COUNT = 6 };
    MUINT32 set[COUNT];
    struct {
        //ISP_NVRAM_EE_CTRL_T               ctrl;
        ISP_NVRAM_EE_TOP_CTRL_T             top_ctrl;
        ISP_NVRAM_EE_SE_Y_SPECL_CTRL_T  	se_y_specl_ctrl;
        ISP_NVRAM_EE_SE_EDGE_CTRL_1_T  		se_edge_ctrl_1;
        ISP_NVRAM_EE_SE_EDGE_CTRL_2_T  		se_edge_ctrl_2;
        ISP_NVRAM_EE_SE_CORE_CTRL_1_T  		se_core_ctrl_1;
        ISP_NVRAM_EE_SE_CORE_CTRL_2_T  		se_core_ctrl_2;
    };
} ISP_NVRAM_SE_T;
#endif
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CNR_CNR
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct {
    FIELD  CNR_CNR_ENC                                :  1;		/*  0.. 0, 0x00000001 */
    FIELD  CNR_BPC_EN                                 :  1;		/*  1.. 1, 0x00000002 */
    FIELD  CNR_VIDEO_MODE                             :  1;		/*  2.. 2, 0x00000004 */
    FIELD  CNR_LBIT_MODE                              :  1;		/*  3.. 3, 0x00000008 */
    FIELD  CNR_CNR_SCALE_MODE                         :  2;		/*  4.. 5, 0x00000030 */
    FIELD  rsv_6                                      :  2;		/*  6.. 7, 0x000000C0 */
    FIELD  CNR_CNR_VER_C_REF_Y                        :  1;		/*  8.. 8, 0x00000100 */
    FIELD  rsv_9                                      :  3;		/*  9..11, 0x00000E00 */
    FIELD  CNR_MODE                                   :  1;		/* 12..12, 0x00001000 */
    FIELD  rsv_13                                     :  3;		/* 13..15, 0x0000E000 */
    FIELD  CNR_TILE_EDGE                              :  4;		/* 16..19, 0x000F0000 */
    FIELD  rsv_20                                     :  4;		/* 20..23, 0x00F00000 */
    FIELD  CNR_CNR_SL2_LINK                           :  1;		/* 24..24, 0x01000000 */
    FIELD  rsv_25                                     :  7;		/* 25..31, 0xFE000000 */
} ISP_CNR_CNR_CON1_T; //	/* 0x150273C0 */

typedef union {
    typedef ISP_CNR_CNR_CON1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CNR_CNR_CON1_T;

typedef struct {
    FIELD  rsv_0                                      :  8;		/*  0.. 7, 0x000000FF */
    FIELD  CNR_CNR_C_SM_EDGE                          :  2;		/*  8.. 9, 0x00000300 */
    FIELD  rsv_10                                     :  2;		/* 10..11, 0x00000C00 */
    FIELD  CNR_CNR_FLT_C                              :  1;		/* 12..12, 0x00001000 */
    FIELD  rsv_13                                     : 19;		/* 13..31, 0xFFFFE000 */
} ISP_CNR_CNR_CON2_T; //	/* 0x150273C4 */

typedef union {
    typedef ISP_CNR_CNR_CON2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CNR_CNR_CON2_T;

typedef struct {
    FIELD  rsv_0                                      : 28;		/*  0..27, 0x0FFFFFFF */
    FIELD  CNR_CNR_K_TH_C                             :  4;		/* 28..31, 0xF0000000 */
} ISP_CNR_CNR_YAD1_T; //	/* 0x150273C8 */

typedef union {
    typedef ISP_CNR_CNR_YAD1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CNR_CNR_YAD1_T;

typedef struct {
    FIELD  CNR_CNR_Y_CPX1                             :  8;		/*  0.. 7, 0x000000FF */
    FIELD  CNR_CNR_Y_CPX2                             :  8;		/*  8..15, 0x0000FF00 */
    FIELD  CNR_CNR_Y_CPX3                             :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                     :  8;		/* 24..31, 0xFF000000 */
} ISP_CNR_CNR_Y4LUT1_T; //	/* 0x150273CC */

typedef union {
    typedef ISP_CNR_CNR_Y4LUT1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CNR_CNR_Y4LUT1_T;

typedef struct {
    FIELD  CNR_CNR_Y_SCALE_CPY0                       :  5;		/*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                      :  3;		/*  5.. 7, 0x000000E0 */
    FIELD  CNR_CNR_Y_SCALE_CPY1                       :  5;		/*  8..12, 0x00001F00 */
    FIELD  rsv_13                                     :  3;		/* 13..15, 0x0000E000 */
    FIELD  CNR_CNR_Y_SCALE_CPY2                       :  5;		/* 16..20, 0x001F0000 */
    FIELD  rsv_21                                     :  3;		/* 21..23, 0x00E00000 */
    FIELD  CNR_CNR_Y_SCALE_CPY3                       :  5;		/* 24..28, 0x1F000000 */
    FIELD  rsv_29                                     :  3;		/* 29..31, 0xE0000000 */
} ISP_CNR_CNR_Y4LUT2_T; //	/* 0x150273D0 */

typedef union {
    typedef ISP_CNR_CNR_Y4LUT2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CNR_CNR_Y4LUT2_T;

typedef struct {
    FIELD  CNR_CNR_Y_SCALE_SP0                        :  5;		/*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                      :  3;		/*  5.. 7, 0x000000E0 */
    FIELD  CNR_CNR_Y_SCALE_SP1                        :  5;		/*  8..12, 0x00001F00 */
    FIELD  rsv_13                                     :  3;		/* 13..15, 0x0000E000 */
    FIELD  CNR_CNR_Y_SCALE_SP2                        :  5;		/* 16..20, 0x001F0000 */
    FIELD  rsv_21                                     :  3;		/* 21..23, 0x00E00000 */
    FIELD  CNR_CNR_Y_SCALE_SP3                        :  5;		/* 24..28, 0x1F000000 */
    FIELD  rsv_29                                     :  3;		/* 29..31, 0xE0000000 */
} ISP_CNR_CNR_Y4LUT3_T; //	/* 0x150273D4 */

typedef union {
    typedef ISP_CNR_CNR_Y4LUT3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CNR_CNR_Y4LUT3_T;

typedef struct {
    FIELD  CNR_CNR_SL2_X1                             :  8;		/*  0.. 7, 0x000000FF */
    FIELD  CNR_CNR_SL2_X2                             :  8;		/*  8..15, 0x0000FF00 */
    FIELD  CNR_CNR_SL2_X3                             :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                     :  8;		/* 24..31, 0xFF000000 */
} ISP_CNR_CNR_L4LUT1_T; //	/* 0x150273D8 */

typedef union {
    typedef ISP_CNR_CNR_L4LUT1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CNR_CNR_L4LUT1_T;

typedef struct {
    FIELD  CNR_CNR_SL2_GAIN0                          :  6;		/*  0.. 5, 0x0000003F */
    FIELD  rsv_6                                      :  2;		/*  6.. 7, 0x000000C0 */
    FIELD  CNR_CNR_SL2_GAIN1                          :  6;		/*  8..13, 0x00003F00 */
    FIELD  rsv_14                                     :  2;		/* 14..15, 0x0000C000 */
    FIELD  CNR_CNR_SL2_GAIN2                          :  6;		/* 16..21, 0x003F0000 */
    FIELD  rsv_22                                     :  2;		/* 22..23, 0x00C00000 */
    FIELD  CNR_CNR_SL2_GAIN3                          :  6;		/* 24..29, 0x3F000000 */
    FIELD  rsv_30                                     :  2;		/* 30..31, 0xC0000000 */
} ISP_CNR_CNR_L4LUT2_T; //	/* 0x150273DC */

typedef union {
    typedef ISP_CNR_CNR_L4LUT2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CNR_CNR_L4LUT2_T;

typedef struct {
    FIELD  CNR_CNR_SL2_SP0                            :  6;		/*  0.. 5, 0x0000003F */
    FIELD  rsv_6                                      :  2;		/*  6.. 7, 0x000000C0 */
    FIELD  CNR_CNR_SL2_SP1                            :  6;		/*  8..13, 0x00003F00 */
    FIELD  rsv_14                                     :  2;		/* 14..15, 0x0000C000 */
    FIELD  CNR_CNR_SL2_SP2                            :  6;		/* 16..21, 0x003F0000 */
    FIELD  rsv_22                                     :  2;		/* 22..23, 0x00C00000 */
    FIELD  CNR_CNR_SL2_SP3                            :  6;		/* 24..29, 0x3F000000 */
    FIELD  rsv_30                                     :  2;		/* 30..31, 0xC0000000 */
} ISP_CNR_CNR_L4LUT3_T; //	/* 0x150273E0 */

typedef union {
    typedef ISP_CNR_CNR_L4LUT3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CNR_CNR_L4LUT3_T;

typedef struct {
    FIELD  rsv_0                                      :  4;		/*  0.. 3, 0x0000000F */
    FIELD  CNR_CNR_C_GAIN                             :  4;		/*  4.. 7, 0x000000F0 */
    FIELD  CNR_CNR_PTC_GAIN_TH                        :  5;		/*  8..12, 0x00001F00 */
    FIELD  rsv_13                                     :  3;		/* 13..15, 0x0000E000 */
    FIELD  CNR_CNR_C_L_DIFF_TH                        :  8;		/* 16..23, 0x00FF0000 */
    FIELD  CNR_CNR_C_MODE                             :  2;		/* 24..25, 0x03000000 */
    FIELD  rsv_26                                     :  6;		/* 26..31, 0xFC000000 */
} ISP_CNR_CNR_CAD_T; //	/* 0x150273E4 */

typedef union {
    typedef ISP_CNR_CNR_CAD_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CNR_CNR_CAD_T;

typedef struct {
    FIELD  CNR_CNR_CB_V_RNG1                          :  8;		/*  0.. 7, 0x000000FF */
    FIELD  CNR_CNR_CB_V_RNG2                          :  8;		/*  8..15, 0x0000FF00 */
    FIELD  CNR_CNR_CB_V_RNG3                          :  8;		/* 16..23, 0x00FF0000 */
    FIELD  CNR_CNR_CB_V_RNG4                          :  8;		/* 24..31, 0xFF000000 */
} ISP_CNR_CNR_CB_VRNG_T; //	/* 0x150273E8 */

typedef union {
    typedef ISP_CNR_CNR_CB_VRNG_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CNR_CNR_CB_VRNG_T;

typedef struct {
    FIELD  CNR_CNR_CB_H_RNG1                          :  8;		/*  0.. 7, 0x000000FF */
    FIELD  CNR_CNR_CB_H_RNG2                          :  8;		/*  8..15, 0x0000FF00 */
    FIELD  CNR_CNR_CB_H_RNG3                          :  8;		/* 16..23, 0x00FF0000 */
    FIELD  CNR_CNR_CB_H_RNG4                          :  8;		/* 24..31, 0xFF000000 */
} ISP_CNR_CNR_CB_HRNG_T; //	/* 0x150273EC */

typedef union {
    typedef ISP_CNR_CNR_CB_HRNG_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CNR_CNR_CB_HRNG_T;

typedef struct {
    FIELD  CNR_CNR_CR_V_RNG1                          :  8;		/*  0.. 7, 0x000000FF */
    FIELD  CNR_CNR_CR_V_RNG2                          :  8;		/*  8..15, 0x0000FF00 */
    FIELD  CNR_CNR_CR_V_RNG3                          :  8;		/* 16..23, 0x00FF0000 */
    FIELD  CNR_CNR_CR_V_RNG4                          :  8;		/* 24..31, 0xFF000000 */
} ISP_CNR_CNR_CR_VRNG_T; //	/* 0x150273F0 */

typedef union {
    typedef ISP_CNR_CNR_CR_VRNG_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CNR_CNR_CR_VRNG_T;

typedef struct {
    FIELD  CNR_CNR_CR_H_RNG1                          :  8;		/*  0.. 7, 0x000000FF */
    FIELD  CNR_CNR_CR_H_RNG2                          :  8;		/*  8..15, 0x0000FF00 */
    FIELD  CNR_CNR_CR_H_RNG3                          :  8;		/* 16..23, 0x00FF0000 */
    FIELD  CNR_CNR_CR_H_RNG4                          :  8;		/* 24..31, 0xFF000000 */
} ISP_CNR_CNR_CR_HRNG_T; //	/* 0x150273F4 */

typedef union {
    typedef ISP_CNR_CNR_CR_HRNG_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CNR_CNR_CR_HRNG_T;

typedef struct {
    FIELD  CNR_CNR_SL2_C_GAIN                         :  4;		/*  0.. 3, 0x0000000F */
    FIELD  rsv_4                                      : 12;		/*  4..15, 0x0000FFF0 */
    FIELD  CNR_BPC_LM_WT                              :  4;		/* 16..19, 0x000F0000 */
    FIELD  rsv_20                                     : 12;		/* 20..31, 0xFFF00000 */
} ISP_CNR_CNR_SL2_T; //	/* 0x150273F8 */

typedef union {
    typedef ISP_CNR_CNR_SL2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CNR_CNR_SL2_T;

typedef struct {
    FIELD  CNR_BPC_COR_TH                             :  5;		/*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                      :  3;		/*  5.. 7, 0x000000E0 */
    FIELD  CNR_BPC_COR_SL                             :  3;		/*  8..10, 0x00000700 */
    FIELD  rsv_11                                     :  1;		/* 11..11, 0x00000800 */
    FIELD  CNR_BPC_MCD_TH                             :  5;		/* 12..16, 0x0001F000 */
    FIELD  rsv_17                                     :  3;		/* 17..19, 0x000E0000 */
    FIELD  CNR_BPC_MCD_SL                             :  3;		/* 20..22, 0x00700000 */
    FIELD  rsv_23                                     :  1;		/* 23..23, 0x00800000 */
    FIELD  CNR_BPC_LCL_TH                             :  8;		/* 24..31, 0xFF000000 */
} ISP_CNR_CNR_MED1_T; //	/* 0x150273FC */

typedef union {
    typedef ISP_CNR_CNR_MED1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CNR_CNR_MED1_T;

typedef struct {
    FIELD  CNR_BPC_LCL_SL                             :  3;		/*  0.. 2, 0x00000007 */
    FIELD  rsv_3                                      :  1;		/*  3.. 3, 0x00000008 */
    FIELD  CNR_BPC_LCL_LV                             :  5;		/*  4.. 8, 0x000001F0 */
    FIELD  rsv_9                                      :  3;		/*  9..11, 0x00000E00 */
    FIELD  CNR_BPC_SCL_TH                             :  6;		/* 12..17, 0x0003F000 */
    FIELD  rsv_18                                     :  2;		/* 18..19, 0x000C0000 */
    FIELD  CNR_BPC_SCL_SL                             :  3;		/* 20..22, 0x00700000 */
    FIELD  rsv_23                                     :  1;		/* 23..23, 0x00800000 */
    FIELD  CNR_BPC_SCL_LV                             :  5;		/* 24..28, 0x1F000000 */
    FIELD  rsv_29                                     :  3;		/* 29..31, 0xE0000000 */
} ISP_CNR_CNR_MED2_T; //	/* 0x15027400 */

typedef union {
    typedef ISP_CNR_CNR_MED2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CNR_CNR_MED2_T;

typedef struct {
    FIELD  CNR_BPC_NCL_TH                             :  8;		/*  0.. 7, 0x000000FF */
    FIELD  CNR_BPC_NCL_SL                             :  3;		/*  8..10, 0x00000700 */
    FIELD  rsv_11                                     :  1;		/* 11..11, 0x00000800 */
    FIELD  CNR_BPC_NCL_LV                             :  5;		/* 12..16, 0x0001F000 */
    FIELD  rsv_17                                     :  3;		/* 17..19, 0x000E0000 */
    FIELD  CNR_BPC_VAR                                :  3;		/* 20..22, 0x00700000 */
    FIELD  rsv_23                                     :  1;		/* 23..23, 0x00800000 */
    FIELD  CNR_BPC_Y0                                 :  6;		/* 24..29, 0x3F000000 */
    FIELD  rsv_30                                     :  2;		/* 30..31, 0xC0000000 */
} ISP_CNR_CNR_MED3_T; //	/* 0x15027404 */

typedef union {
    typedef ISP_CNR_CNR_MED3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CNR_CNR_MED3_T;

typedef struct {
    FIELD  CNR_BPC_Y1                                 :  6;		/*  0.. 5, 0x0000003F */
    FIELD  rsv_6                                      :  2;		/*  6.. 7, 0x000000C0 */
    FIELD  CNR_BPC_Y2                                 :  6;		/*  8..13, 0x00003F00 */
    FIELD  rsv_14                                     :  2;		/* 14..15, 0x0000C000 */
    FIELD  CNR_BPC_Y3                                 :  6;		/* 16..21, 0x003F0000 */
    FIELD  rsv_22                                     :  2;		/* 22..23, 0x00C00000 */
    FIELD  CNR_BPC_Y4                                 :  6;		/* 24..29, 0x3F000000 */
    FIELD  rsv_30                                     :  2;		/* 30..31, 0xC0000000 */
} ISP_CNR_CNR_MED4_T; //	/* 0x15027408 */

typedef union {
    typedef ISP_CNR_CNR_MED4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CNR_CNR_MED4_T;

typedef struct {
    FIELD  CNR_BPC_LCL_OFT                            :  8;		/*  0.. 7, 0x000000FF */
    FIELD  CNR_BPC_SCL_OFT                            :  6;		/*  8..13, 0x00003F00 */
    FIELD  rsv_14                                     :  2;		/* 14..15, 0x0000C000 */
    FIELD  CNR_BPC_NCL_OFT                            :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                     :  8;		/* 24..31, 0xFF000000 */
} ISP_CNR_CNR_MED5_T; //	/* 0x1502740C */

typedef union {
    typedef ISP_CNR_CNR_MED5_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CNR_CNR_MED5_T;

typedef struct {
    FIELD  CNR_CNR_RSV                                :  8;		/*  0.. 7, 0x000000FF */
    FIELD  CNR_CNR_ACT_BLD_BASE_C                     :  7;		/*  8..14, 0x00007F00 */
    FIELD  rsv_15                                     :  1;		/* 15..15, 0x00008000 */
    FIELD  CNR_CNR_C_DITH_U                           :  5;		/* 16..20, 0x001F0000 */
    FIELD  rsv_21                                     :  3;		/* 21..23, 0x00E00000 */
    FIELD  CNR_CNR_C_DITH_V                           :  5;		/* 24..28, 0x1F000000 */
    FIELD  rsv_29                                     :  3;		/* 29..31, 0xE0000000 */
} ISP_CNR_CNR_ACTC_T; //	/* 0x15027410 */

typedef union {
    typedef ISP_CNR_CNR_ACTC_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CNR_CNR_ACTC_T;

typedef struct {
    FIELD  CNR_CNR_RSV1                               : 32;		/*  0..31, 0xFFFFFFFF */
} ISP_CNR_CNR_RSV1_T; //	/* 0x15027414 */

typedef union {
    typedef ISP_CNR_CNR_RSV1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CNR_CNR_RSV1_T;

typedef struct {
    FIELD  CNR_CNR_RSV2                               : 32;		/*  0..31, 0xFFFFFFFF */
} ISP_CNR_CNR_RSV2_T; //	/* 0x15027418 */

typedef union {
    typedef ISP_CNR_CNR_RSV2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CNR_CNR_RSV2_T;

typedef struct {
    FIELD  CNR_BPC_DK_Y1                              :  6;		/*  0.. 5, 0x0000003F */
    FIELD  rsv_6                                      :  2;		/*  6.. 7, 0x000000C0 */
    FIELD  CNR_BPC_DK_Y2                              :  6;		/*  8..13, 0x00003F00 */
    FIELD  rsv_14                                     :  2;		/* 14..15, 0x0000C000 */
    FIELD  CNR_BPC_DK_Y3                              :  6;		/* 16..21, 0x003F0000 */
    FIELD  rsv_22                                     :  2;		/* 22..23, 0x00C00000 */
    FIELD  CNR_BPC_DK_Y4                              :  6;		/* 24..29, 0x3F000000 */
    FIELD  rsv_30                                     :  2;		/* 30..31, 0xC0000000 */
} ISP_CNR_CNR_MED6_T; //	/* 0x1502741C */

typedef union {
    typedef ISP_CNR_CNR_MED6_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CNR_CNR_MED6_T;

typedef struct {
    FIELD  CNR_BPC_LCL_LV_CEN                         :  5;		/*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                      :  3;		/*  5.. 7, 0x000000E0 */
    FIELD  CNR_BPC_SCL_LV_CEN                         :  5;		/*  8..12, 0x00001F00 */
    FIELD  rsv_13                                     :  3;		/* 13..15, 0x0000E000 */
    FIELD  CNR_BPC_NCL_LV_CEN                         :  5;		/* 16..20, 0x001F0000 */
    FIELD  rsv_21                                     :  3;		/* 21..23, 0x00E00000 */
    FIELD  CNR_BPC_DK_Y0                              :  6;		/* 24..29, 0x3F000000 */
    FIELD  rsv_30                                     :  2;		/* 30..31, 0xC0000000 */
} ISP_CNR_CNR_MED7_T; //	/* 0x15027420 */

typedef union {
    typedef ISP_CNR_CNR_MED7_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CNR_CNR_MED7_T;

typedef struct {
    FIELD  CNR_BPC_OUT_TH                             :  5;		/*  0.. 4, 0x0000001F */
    FIELD  CNR_BPC_BND_SL                             :  3;		/*  5.. 7, 0x000000E0 */
    FIELD  CNR_BPC_BND_TH_LO                          :  8;		/*  8..15, 0x0000FF00 */
    FIELD  CNR_BPC_BND_TH_HI                          :  8;		/* 16..23, 0x00FF0000 */
    FIELD  CNR_BPC_BND_TH_EDGE                        :  8;		/* 24..31, 0xFF000000 */
} ISP_CNR_CNR_MED8_T; //	/* 0x15027424 */

typedef union {
    typedef ISP_CNR_CNR_MED8_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CNR_CNR_MED8_T;

typedef struct {
    FIELD  CNR_BPC_EDG_TH                             :  5;		/*  0.. 4, 0x0000001F */
    FIELD  CNR_BPC_EDG_SL                             :  3;		/*  5.. 7, 0x000000E0 */
    FIELD  CNR_BPC_LINE_X0                            :  8;		/*  8..15, 0x0000FF00 */
    FIELD  CNR_BPC_LINE_TH0                           :  8;		/* 16..23, 0x00FF0000 */
    FIELD  CNR_BPC_LINE_TH1                           :  8;		/* 24..31, 0xFF000000 */
} ISP_CNR_CNR_MED9_T; //	/* 0x15027428 */

typedef union {
    typedef ISP_CNR_CNR_MED9_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CNR_CNR_MED9_T;

typedef struct {
    FIELD  CNR_BPC_LINE_SL0                           :  7;		/*  0.. 6, 0x0000007F */
    FIELD  rsv_7                                      :  1;		/*  7.. 7, 0x00000080 */
    FIELD  CNR_BPC_LINE_TH_SL                         :  3;		/*  8..10, 0x00000700 */
    FIELD  rsv_11                                     : 21;		/* 11..31, 0xFFFFF800 */
} ISP_CNR_CNR_MED10_T; //	/* 0x1502742C */

typedef union {
    typedef ISP_CNR_CNR_MED10_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CNR_CNR_MED10_T;

typedef struct {
    FIELD  CNR_BPC_SL2_LINK                           :  1;		/*  0.. 0, 0x00000001 */
    FIELD  rsv_1                                      :  7;		/*  1.. 7, 0x000000FE */
    FIELD  CNR_BPC_SL2_X1                             :  8;		/*  8..15, 0x0000FF00 */
    FIELD  CNR_BPC_SL2_X2                             :  8;		/* 16..23, 0x00FF0000 */
    FIELD  CNR_BPC_SL2_X3                             :  8;		/* 24..31, 0xFF000000 */
} ISP_CNR_CNR_MED11_T; //	/* 0x15027430 */

typedef union {
    typedef ISP_CNR_CNR_MED11_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CNR_CNR_MED11_T;

typedef struct {
    FIELD  CNR_BPC_SL2_GAIN0                          :  5;		/*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                      :  3;		/*  5.. 7, 0x000000E0 */
    FIELD  CNR_BPC_SL2_GAIN1                          :  5;		/*  8..12, 0x00001F00 */
    FIELD  rsv_13                                     :  3;		/* 13..15, 0x0000E000 */
    FIELD  CNR_BPC_SL2_GAIN2                          :  5;		/* 16..20, 0x001F0000 */
    FIELD  rsv_21                                     :  3;		/* 21..23, 0x00E00000 */
    FIELD  CNR_BPC_SL2_GAIN3                          :  5;		/* 24..28, 0x1F000000 */
    FIELD  rsv_29                                     :  3;		/* 29..31, 0xE0000000 */
} ISP_CNR_CNR_MED12_T; //	/* 0x15027434 */

typedef union {
    typedef ISP_CNR_CNR_MED12_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CNR_CNR_MED12_T;

typedef struct {
    FIELD  CNR_BPC_SL2_SP0                            :  6;		/*  0.. 5, 0x0000003F */
    FIELD  rsv_6                                      :  2;		/*  6.. 7, 0x000000C0 */
    FIELD  CNR_BPC_SL2_SP1                            :  6;		/*  8..13, 0x00003F00 */
    FIELD  rsv_14                                     :  2;		/* 14..15, 0x0000C000 */
    FIELD  CNR_BPC_SL2_SP2                            :  6;		/* 16..21, 0x003F0000 */
    FIELD  rsv_22                                     :  2;		/* 22..23, 0x00C00000 */
    FIELD  CNR_BPC_SL2_SP3                            :  6;		/* 24..29, 0x3F000000 */
    FIELD  rsv_30                                     :  2;		/* 30..31, 0xC0000000 */
} ISP_CNR_CNR_MED13_T; //	/* 0x15027438 */

typedef union {
    typedef ISP_CNR_CNR_MED13_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CNR_CNR_MED13_T;

typedef union {
    enum { COUNT = 29 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_CNR_CNR_CON1_T  			con1;
        ISP_NVRAM_CNR_CNR_CON2_T  			con2;
        ISP_NVRAM_CNR_CNR_YAD1_T  			yad1;
        ISP_NVRAM_CNR_CNR_Y4LUT1_T  		y4lut1;
        ISP_NVRAM_CNR_CNR_Y4LUT2_T  		y4lut2;
        ISP_NVRAM_CNR_CNR_Y4LUT3_T  		y4lut3;
        ISP_NVRAM_CNR_CNR_L4LUT1_T  		l4lut1;
        ISP_NVRAM_CNR_CNR_L4LUT2_T  		l4lut2;
        ISP_NVRAM_CNR_CNR_L4LUT3_T  		l4lut3;
        ISP_NVRAM_CNR_CNR_CAD_T  			cad;
        ISP_NVRAM_CNR_CNR_CB_VRNG_T  		cb_vrng;
        ISP_NVRAM_CNR_CNR_CB_HRNG_T  		cb_hrng;
        ISP_NVRAM_CNR_CNR_CR_VRNG_T  		cr_vrng;
        ISP_NVRAM_CNR_CNR_CR_HRNG_T  		cr_hrng;
        ISP_NVRAM_CNR_CNR_SL2_T  			sl2;
        ISP_NVRAM_CNR_CNR_MED1_T  			med1;
        ISP_NVRAM_CNR_CNR_MED2_T  			med2;
        ISP_NVRAM_CNR_CNR_MED3_T  			med3;
        ISP_NVRAM_CNR_CNR_MED4_T  			med4;
        ISP_NVRAM_CNR_CNR_MED5_T  			med5;
        ISP_NVRAM_CNR_CNR_ACTC_T  			actc;
        //ISP_NVRAM_CNR_CNR_RSV1_T  			rsv1;
        //ISP_NVRAM_CNR_CNR_RSV2_T  			rsv2;
        ISP_NVRAM_CNR_CNR_MED6_T  			med6;
        ISP_NVRAM_CNR_CNR_MED7_T  			med7;
        ISP_NVRAM_CNR_CNR_MED8_T  			med8;
        ISP_NVRAM_CNR_CNR_MED9_T  			med9;
        ISP_NVRAM_CNR_CNR_MED10_T  			med10;
        ISP_NVRAM_CNR_CNR_MED11_T  			med11;
        ISP_NVRAM_CNR_CNR_MED12_T  			med12;
        ISP_NVRAM_CNR_CNR_MED13_T  			med13;
    };
} ISP_NVRAM_CNR_CNR_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CNR_CCR
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef struct {
    FIELD  CNR_CCR_EN                                 :  1;		/*  0.. 0, 0x00000001 */
    FIELD  CNR_CCR_SL2_LINK                           :  1;		/*  1.. 1, 0x00000002 */
    FIELD  CNR_CCR_SL2_MODE                           :  2;		/*  2.. 3, 0x0000000C */
    FIELD  rsv_4                                      :  1;		/*  4.. 4, 0x00000010 */
    FIELD  CNR_CCR_OR_MODE                            :  1;		/*  5.. 5, 0x00000020 */
    FIELD  rsv_6                                      :  2;		/*  6.. 7, 0x000000C0 */
    FIELD  CNR_CCR_UV_GAIN_MODE                       :  1;		/*  8.. 8, 0x00000100 */
    FIELD  rsv_9                                      :  7;		/*  9..15, 0x0000FE00 */
    FIELD  CNR_CCR_UV_GAIN2                           :  7;		/* 16..22, 0x007F0000 */
    FIELD  rsv_23                                     :  1;		/* 23..23, 0x00800000 */
    FIELD  CNR_CCR_Y_CPX3                             :  8;		/* 24..31, 0xFF000000 */
} ISP_CNR_CCR_CON_T; //	/* 0x15027440 */

typedef union {
    typedef ISP_CNR_CCR_CON_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CNR_CCR_CON_T;

typedef struct {
    FIELD  CNR_CCR_Y_CPX1                             :  8;		/*  0.. 7, 0x000000FF */
    FIELD  CNR_CCR_Y_CPX2                             :  8;		/*  8..15, 0x0000FF00 */
    FIELD  CNR_CCR_Y_SP1                              :  7;		/* 16..22, 0x007F0000 */
    FIELD  rsv_23                                     :  1;		/* 23..23, 0x00800000 */
    FIELD  CNR_CCR_Y_CPY1                             :  7;		/* 24..30, 0x7F000000 */
    FIELD  rsv_31                                     :  1;		/* 31..31, 0x80000000 */
} ISP_CNR_CCR_YLUT_T; //	/* 0x15027444 */

typedef union {
    typedef ISP_CNR_CCR_YLUT_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CNR_CCR_YLUT_T;

typedef struct {
    FIELD  CNR_CCR_UV_X1                              :  8;		/*  0.. 7, 0x000000FF */
    FIELD  CNR_CCR_UV_X2                              :  8;		/*  8..15, 0x0000FF00 */
    FIELD  CNR_CCR_UV_X3                              :  8;		/* 16..23, 0x00FF0000 */
    FIELD  CNR_CCR_UV_GAIN1                           :  7;		/* 24..30, 0x7F000000 */
    FIELD  rsv_31                                     :  1;		/* 31..31, 0x80000000 */
} ISP_CNR_CCR_UVLUT_T; //	/* 0x15027448 */

typedef union {
    typedef ISP_CNR_CCR_UVLUT_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CNR_CCR_UVLUT_T;

typedef struct {
    FIELD  CNR_CCR_Y_SP0                              :  7;		/*  0.. 6, 0x0000007F */
    FIELD  rsv_7                                      :  1;		/*  7.. 7, 0x00000080 */
    FIELD  CNR_CCR_Y_SP2                              :  7;		/*  8..14, 0x00007F00 */
    FIELD  rsv_15                                     :  1;		/* 15..15, 0x00008000 */
    FIELD  CNR_CCR_Y_CPY0                             :  7;		/* 16..22, 0x007F0000 */
    FIELD  rsv_23                                     :  1;		/* 23..23, 0x00800000 */
    FIELD  CNR_CCR_Y_CPY2                             :  7;		/* 24..30, 0x7F000000 */
    FIELD  rsv_31                                     :  1;		/* 31..31, 0x80000000 */
} ISP_CNR_CCR_YLUT2_T; //	/* 0x1502744C */

typedef union {
    typedef ISP_CNR_CCR_YLUT2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CNR_CCR_YLUT2_T;

typedef struct {
    FIELD  CNR_CCR_MODE                               :  1;		/*  0.. 0, 0x00000001 */
    FIELD  rsv_1                                      :  3;		/*  1.. 3, 0x0000000E */
    FIELD  CNR_CCR_CEN_U                              :  6;		/*  4.. 9, 0x000003F0 */
    FIELD  rsv_10                                     :  2;		/* 10..11, 0x00000C00 */
    FIELD  CNR_CCR_CEN_V                              :  6;		/* 12..17, 0x0003F000 */
    FIELD  rsv_18                                     : 14;		/* 18..31, 0xFFFC0000 */
} ISP_CNR_CCR_SAT_CTRL_T; //	/* 0x15027450 */

typedef union {
    typedef ISP_CNR_CCR_SAT_CTRL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CNR_CCR_SAT_CTRL_T;

typedef struct {
    FIELD  CNR_CCR_UV_GAIN_SP1                        : 10;		/*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                     :  6;		/* 10..15, 0x0000FC00 */
    FIELD  CNR_CCR_UV_GAIN_SP2                        : 10;		/* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                     :  6;		/* 26..31, 0xFC000000 */
} ISP_CNR_CCR_UVLUT_SP_T; //	/* 0x15027454 */

typedef union {
    typedef ISP_CNR_CCR_UVLUT_SP_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CNR_CCR_UVLUT_SP_T;

typedef struct {
    FIELD  CNR_CCR_HUE_X1                             :  9;		/*  0.. 8, 0x000001FF */
    FIELD  rsv_9                                      :  7;		/*  9..15, 0x0000FE00 */
    FIELD  CNR_CCR_HUE_X2                             :  9;		/* 16..24, 0x01FF0000 */
    FIELD  rsv_25                                     :  7;		/* 25..31, 0xFE000000 */
} ISP_CNR_CCR_HUE1_T; //	/* 0x15027458 */

typedef union {
    typedef ISP_CNR_CCR_HUE1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CNR_CCR_HUE1_T;

typedef struct {
    FIELD  CNR_CCR_HUE_X3                             :  9;		/*  0.. 8, 0x000001FF */
    FIELD  rsv_9                                      :  7;		/*  9..15, 0x0000FE00 */
    FIELD  CNR_CCR_HUE_X4                             :  9;		/* 16..24, 0x01FF0000 */
    FIELD  rsv_25                                     :  7;		/* 25..31, 0xFE000000 */
} ISP_CNR_CCR_HUE2_T; //	/* 0x1502745C */

typedef union {
    typedef ISP_CNR_CCR_HUE2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CNR_CCR_HUE2_T;

typedef struct {
    FIELD  CNR_CCR_HUE_SP1                            :  8;		/*  0.. 7, 0x000000FF */
    FIELD  CNR_CCR_HUE_SP2                            :  8;		/*  8..15, 0x0000FF00 */
    FIELD  CNR_CCR_HUE_GAIN1                          :  7;		/* 16..22, 0x007F0000 */
    FIELD  rsv_23                                     :  1;		/* 23..23, 0x00800000 */
    FIELD  CNR_CCR_HUE_GAIN2                          :  7;		/* 24..30, 0x7F000000 */
    FIELD  rsv_31                                     :  1;		/* 31..31, 0x80000000 */
} ISP_CNR_CCR_HUE3_T; //	/* 0x15027460 */

typedef union {
    typedef ISP_CNR_CCR_HUE3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CNR_CCR_HUE3_T;

typedef struct {
    FIELD  CNR_CCR_SL2_X1                             :  8;		/*  0.. 7, 0x000000FF */
    FIELD  CNR_CCR_SL2_X2                             :  8;		/*  8..15, 0x0000FF00 */
    FIELD  CNR_CCR_SL2_X3                             :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                     :  8;		/* 24..31, 0xFF000000 */
} ISP_CNR_CCR_L4LUT1_T; //	/* 0x15027464 */

typedef union {
    typedef ISP_CNR_CCR_L4LUT1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CNR_CCR_L4LUT1_T;

typedef struct {
    FIELD  CNR_CCR_SL2_GAIN0                          :  6;		/*  0.. 5, 0x0000003F */
    FIELD  rsv_6                                      :  2;		/*  6.. 7, 0x000000C0 */
    FIELD  CNR_CCR_SL2_GAIN1                          :  6;		/*  8..13, 0x00003F00 */
    FIELD  rsv_14                                     :  2;		/* 14..15, 0x0000C000 */
    FIELD  CNR_CCR_SL2_GAIN2                          :  6;		/* 16..21, 0x003F0000 */
    FIELD  rsv_22                                     :  2;		/* 22..23, 0x00C00000 */
    FIELD  CNR_CCR_SL2_GAIN3                          :  6;		/* 24..29, 0x3F000000 */
    FIELD  rsv_30                                     :  2;		/* 30..31, 0xC0000000 */
} ISP_CNR_CCR_L4LUT2_T; //	/* 0x15027468 */

typedef union {
    typedef ISP_CNR_CCR_L4LUT2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CNR_CCR_L4LUT2_T;

typedef struct {
    FIELD  CNR_CCR_SL2_SP0                            :  6;		/*  0.. 5, 0x0000003F */
    FIELD  rsv_6                                      :  2;		/*  6.. 7, 0x000000C0 */
    FIELD  CNR_CCR_SL2_SP1                            :  6;		/*  8..13, 0x00003F00 */
    FIELD  rsv_14                                     :  2;		/* 14..15, 0x0000C000 */
    FIELD  CNR_CCR_SL2_SP2                            :  6;		/* 16..21, 0x003F0000 */
    FIELD  rsv_22                                     :  2;		/* 22..23, 0x00C00000 */
    FIELD  CNR_CCR_SL2_SP3                            :  6;		/* 24..29, 0x3F000000 */
    FIELD  rsv_30                                     :  2;		/* 30..31, 0xC0000000 */
} ISP_CNR_CCR_L4LUT3_T; //	/* 0x1502746C */

typedef union {
    typedef ISP_CNR_CCR_L4LUT3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CNR_CCR_L4LUT3_T;

typedef union {
    enum { COUNT = 12 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_CNR_CCR_CON_T  			con;
        ISP_NVRAM_CNR_CCR_YLUT_T  			ylut;
        ISP_NVRAM_CNR_CCR_UVLUT_T  			uvlut;
        ISP_NVRAM_CNR_CCR_YLUT2_T  			ylut2;
        ISP_NVRAM_CNR_CCR_SAT_CTRL_T  		sat_ctrl;
        ISP_NVRAM_CNR_CCR_UVLUT_SP_T  		uvlut_sp;
        ISP_NVRAM_CNR_CCR_HUE1_T  			hue1;
        ISP_NVRAM_CNR_CCR_HUE2_T  			hue2;
        ISP_NVRAM_CNR_CCR_HUE3_T  			hue3;
        ISP_NVRAM_CNR_CCR_L4LUT1_T  		l4lut1;
        ISP_NVRAM_CNR_CCR_L4LUT2_T  		l4lut2;
        ISP_NVRAM_CNR_CCR_L4LUT3_T  		l4lut3;
    };
} ISP_NVRAM_CNR_CCR_T;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CNR_ABF
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef struct {
    FIELD  CNR_ABF_EN                                 :  1;		/*  0.. 0, 0x00000001 */
    FIELD  rsv_1                                      :  7;		/*  1.. 7, 0x000000FE */
    FIELD  CNR_ABF_BIL_IDX                            :  2;		/*  8.. 9, 0x00000300 */
    FIELD  CNR_ABF_NSR_IDX                            :  2;		/* 10..11, 0x00000C00 */
    FIELD  rsv_12                                     : 20;		/* 12..31, 0xFFFFF000 */
} ISP_CNR_ABF_CON1_T; //	/* 0x15027480 */

typedef union {
    typedef ISP_CNR_ABF_CON1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CNR_ABF_CON1_T;

typedef struct {
    FIELD  CNR_ABF_BF_U_OFST                          :  6;		/*  0.. 5, 0x0000003F */
    FIELD  rsv_6                                      : 26;		/*  6..31, 0xFFFFFFC0 */
} ISP_CNR_ABF_CON2_T; //	/* 0x15027484 */

typedef union {
    typedef ISP_CNR_ABF_CON2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CNR_ABF_CON2_T;

typedef struct {
    FIELD  CNR_ABF_R1                                 :  9;		/*  0.. 8, 0x000001FF */
    FIELD  rsv_9                                      :  7;		/*  9..15, 0x0000FE00 */
    FIELD  CNR_ABF_R2                                 :  9;		/* 16..24, 0x01FF0000 */
    FIELD  rsv_25                                     :  7;		/* 25..31, 0xFE000000 */
} ISP_CNR_ABF_RCON_T; //	/* 0x15027488 */

typedef union {
    typedef ISP_CNR_ABF_RCON_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CNR_ABF_RCON_T;

typedef struct {
    FIELD  CNR_ABF_Y0                                 :  8;		/*  0.. 7, 0x000000FF */
    FIELD  CNR_ABF_Y1                                 :  8;		/*  8..15, 0x0000FF00 */
    FIELD  CNR_ABF_Y2                                 :  8;		/* 16..23, 0x00FF0000 */
    FIELD  CNR_ABF_Y3                                 :  8;		/* 24..31, 0xFF000000 */
} ISP_CNR_ABF_YLUT_T; //	/* 0x1502748C */

typedef union {
    typedef ISP_CNR_ABF_YLUT_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CNR_ABF_YLUT_T;

typedef struct {
    FIELD  CNR_ABF_CX0                                :  8;		/*  0.. 7, 0x000000FF */
    FIELD  CNR_ABF_CX1                                :  8;		/*  8..15, 0x0000FF00 */
    FIELD  CNR_ABF_CX2                                :  8;		/* 16..23, 0x00FF0000 */
    FIELD  CNR_ABF_CX3                                :  8;		/* 24..31, 0xFF000000 */
} ISP_CNR_ABF_CXLUT_T; //	/* 0x15027490 */

typedef union {
    typedef ISP_CNR_ABF_CXLUT_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CNR_ABF_CXLUT_T;

typedef struct {
    FIELD  CNR_ABF_CY0                                :  8;		/*  0.. 7, 0x000000FF */
    FIELD  CNR_ABF_CY1                                :  8;		/*  8..15, 0x0000FF00 */
    FIELD  CNR_ABF_CY2                                :  8;		/* 16..23, 0x00FF0000 */
    FIELD  CNR_ABF_CY3                                :  8;		/* 24..31, 0xFF000000 */
} ISP_CNR_ABF_CYLUT_T; //	/* 0x15027494 */

typedef union {
    typedef ISP_CNR_ABF_CYLUT_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CNR_ABF_CYLUT_T;

typedef struct {
    FIELD  CNR_ABF_Y_SP0                              : 10;		/*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                     :  6;		/* 10..15, 0x0000FC00 */
    FIELD  CNR_ABF_Y_SP1                              : 10;		/* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                     :  6;		/* 26..31, 0xFC000000 */
} ISP_CNR_ABF_YSP_T; //	/* 0x15027498 */

typedef union {
    typedef ISP_CNR_ABF_YSP_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CNR_ABF_YSP_T;

typedef struct {
    FIELD  CNR_ABF_CX_SP0                             : 10;		/*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                     :  6;		/* 10..15, 0x0000FC00 */
    FIELD  CNR_ABF_CX_SP1                             : 10;		/* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                     :  6;		/* 26..31, 0xFC000000 */
} ISP_CNR_ABF_CXSP_T; //	/* 0x1502749C */

typedef union {
    typedef ISP_CNR_ABF_CXSP_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CNR_ABF_CXSP_T;

typedef struct {
    FIELD  CNR_ABF_CY_SP0                             : 10;		/*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                     :  6;		/* 10..15, 0x0000FC00 */
    FIELD  CNR_ABF_CY_SP1                             : 10;		/* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                     :  6;		/* 26..31, 0xFC000000 */
} ISP_CNR_ABF_CYSP_T; //	/* 0x150274A0 */

typedef union {
    typedef ISP_CNR_ABF_CYSP_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CNR_ABF_CYSP_T;

typedef struct {
    FIELD  CNR_ABF_STHRE_R                            :  8;		/*  0.. 7, 0x000000FF */
    FIELD  CNR_ABF_STHRE_G                            :  8;		/*  8..15, 0x0000FF00 */
    FIELD  CNR_ABF_STHRE_B                            :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                     :  8;		/* 24..31, 0xFF000000 */
} ISP_CNR_ABF_CLP_T; //	/* 0x150274A4 */

typedef union {
    typedef ISP_CNR_ABF_CLP_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CNR_ABF_CLP_T;

typedef struct {
    FIELD  CNR_ABF_RSV                                : 32;		/*  0..31, 0xFFFFFFFF */
} ISP_CNR_ABF_RSV1_T; //	/* 0x150274A8 */

typedef union {
    typedef ISP_CNR_ABF_RSV1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CNR_ABF_RSV1_T;

typedef union {
    enum { COUNT = 10 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_CNR_ABF_CON1_T  			con1;
        ISP_NVRAM_CNR_ABF_CON2_T  			con2;
        ISP_NVRAM_CNR_ABF_RCON_T  			rcon;
        ISP_NVRAM_CNR_ABF_YLUT_T  			ylut;
        ISP_NVRAM_CNR_ABF_CXLUT_T  			cxlut;
        ISP_NVRAM_CNR_ABF_CYLUT_T  			cylut;
        ISP_NVRAM_CNR_ABF_YSP_T  			ysp;
        ISP_NVRAM_CNR_ABF_CXSP_T  			cxsp;
        ISP_NVRAM_CNR_ABF_CYSP_T  			cysp;
        ISP_NVRAM_CNR_ABF_CLP_T  			clp;
        //ISP_NVRAM_CNR_ABF_RSV1_T  			rsv1;
    };
} ISP_NVRAM_CNR_ABF_T;

#if 0
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CNR_BOK
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef struct {
    FIELD  CNR_BOK_BLUR_MUL                           :  6;		/*  0.. 5, 0x0000003F */
    FIELD  CNR_BOK_BLUR_SHIFT                         :  4;		/*  6.. 9, 0x000003C0 */
    FIELD  CNR_BOK_BLUR_OFFSET                        :  5;		/* 10..14, 0x00007C00 */
    FIELD  rsv_15                                     :  1;		/* 15..15, 0x00008000 */
    FIELD  CNR_BOK_BLUR_MAX                           :  4;		/* 16..19, 0x000F0000 */
    FIELD  CNR_BOK_BLEND_START                        :  5;		/* 20..24, 0x01F00000 */
    FIELD  rsv_25                                     :  1;		/* 25..25, 0x02000000 */
    FIELD  CNR_BOK_BLEND_SLOPE                        :  5;		/* 26..30, 0x7C000000 */
    FIELD  rsv_31                                     :  1;		/* 31..31, 0x80000000 */
} ISP_CNR_BOK_TUN_T; //	/* 0x150274AC */

typedef union {
    typedef ISP_CNR_BOK_TUN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CNR_BOK_TUN_T;

typedef union {
    enum { COUNT = 1 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_CNR_BOK_TUN_T  			tun;
    };
} ISP_NVRAM_CNR_BOK_T;
#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// COLOR
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct {
    FIELD  COLOR_c2p_bypass                           :  1;		/*  0.. 0, 0x00000001 */
    FIELD  COLOR_p2c_bypass                           :  1;		/*  1.. 1, 0x00000002 */
    FIELD  COLOR_yeng_bypass                          :  1;		/*  2.. 2, 0x00000004 */
    FIELD  COLOR_seng_bypass                          :  1;		/*  3.. 3, 0x00000008 */
    FIELD  COLOR_heng_bypass                          :  1;		/*  4.. 4, 0x00000010 */
    FIELD  rsv_5                                      :  2;		/*  5.. 6, 0x00000060 */
    FIELD  COLOR_all_bypass                           :  1;		/*  7.. 7, 0x00000080 */
    FIELD  COLOR_wide_gamut_en                        :  1;		/*  8.. 8, 0x00000100 */
    FIELD  rsv_9                                      :  4;		/*  9..12, 0x00001E00 */
    FIELD  COLOR_seq_sel                              :  1;		/* 13..13, 0x00002000 */
    FIELD  rsv_14                                     :  1;		/* 14..14, 0x00004000 */
    FIELD  COLOR_s_g_y_en                             :  1;		/* 15..15, 0x00008000 */
    FIELD  COLOR_c_pp_cm_dbg_sel                      :  4;		/* 16..19, 0x000F0000 */
    FIELD  COLOR_lsp_en                               :  1;		/* 20..20, 0x00100000 */
    FIELD  COLOR_lsp_sat_limit                        :  1;		/* 21..21, 0x00200000 */
    FIELD  COLOR_lsp_ink_en                           :  1;		/* 22..22, 0x00400000 */
    FIELD  COLOR_lsp_sat_src                          :  1;		/* 23..23, 0x00800000 */
    FIELD  rsv_24                                     :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_CFG_MAIN_T; //	/* 0x15027640 */

typedef union {
    typedef ISP_COLOR_CFG_MAIN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CFG_MAIN_T;

typedef struct {
    FIELD  COLOR_pic_h_cnt                            : 16;		/*  0..15, 0x0000FFFF */
    FIELD  rsv_16                                     : 16;		/* 16..31, 0xFFFF0000 */
} ISP_COLOR_PXL_CNT_MAIN_T; //	/* 0x15027644 */

typedef union {
    typedef ISP_COLOR_PXL_CNT_MAIN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_PXL_CNT_MAIN_T;

typedef struct {
    FIELD  COLOR_line_cnt                             : 16;		/*  0..15, 0x0000FFFF */
    FIELD  COLOR_pic_v_cnt                            : 16;		/* 16..31, 0xFFFF0000 */
} ISP_COLOR_LINE_CNT_MAIN_T; //	/* 0x15027648 */

typedef union {
    typedef ISP_COLOR_LINE_CNT_MAIN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_LINE_CNT_MAIN_T;

typedef struct {
    FIELD  COLOR_win_x_start                          : 16;		/*  0..15, 0x0000FFFF */
    FIELD  COLOR_win_x_end                            : 16;		/* 16..31, 0xFFFF0000 */
} ISP_COLOR_WIN_X_MAIN_T; //	/* 0x1502764C */

typedef union {
    typedef ISP_COLOR_WIN_X_MAIN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_WIN_X_MAIN_T;

typedef struct {
    FIELD  COLOR_win_y_start                          : 16;		/*  0..15, 0x0000FFFF */
    FIELD  COLOR_win_y_end                            : 16;		/* 16..31, 0xFFFF0000 */
} ISP_COLOR_WIN_Y_MAIN_T; //	/* 0x15027650 */

typedef union {
    typedef ISP_COLOR_WIN_Y_MAIN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_WIN_Y_MAIN_T;

typedef struct {
    FIELD  COLOR_htotal_cnt                           : 16;		/*  0..15, 0x0000FFFF */
    FIELD  COLOR_vtotal_cnt                           : 16;		/* 16..31, 0xFFFF0000 */
} ISP_COLOR_TIMING_DETECTION_0_T; //	/* 0x15027654 */

typedef union {
    typedef ISP_COLOR_TIMING_DETECTION_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_TIMING_DETECTION_0_T;

typedef struct {
    FIELD  COLOR_hde_cnt                              : 16;		/*  0..15, 0x0000FFFF */
    FIELD  COLOR_vde_cnt                              : 16;		/* 16..31, 0xFFFF0000 */
} ISP_COLOR_TIMING_DETECTION_1_T; //	/* 0x15027658 */

typedef union {
    typedef ISP_COLOR_TIMING_DETECTION_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_TIMING_DETECTION_1_T;

typedef struct {
    FIELD  COLOR_ink_en                               :  1;		/*  0.. 0, 0x00000001 */
    FIELD  rsv_1                                      :  1;		/*  1.. 1, 0x00000002 */
    FIELD  COLOR_cap_en                               :  1;		/*  2.. 2, 0x00000004 */
    FIELD  COLOR_split_en                             :  1;		/*  3.. 3, 0x00000008 */
    FIELD  COLOR_split_swap                           :  1;		/*  4.. 4, 0x00000010 */
    FIELD  rsv_5                                      :  3;		/*  5.. 7, 0x000000E0 */
    FIELD  COLOR_y_ink                                :  1;		/*  8.. 8, 0x00000100 */
    FIELD  COLOR_u_ink                                :  1;		/*  9.. 9, 0x00000200 */
    FIELD  COLOR_v_ink                                :  1;		/* 10..10, 0x00000400 */
    FIELD  COLOR_w1_ink_en                            :  1;		/* 11..11, 0x00000800 */
    FIELD  rsv_12                                     :  4;		/* 12..15, 0x0000F000 */
    FIELD  COLOR_y_ink_mode                           :  3;		/* 16..18, 0x00070000 */
    FIELD  COLOR_u_ink_mode                           :  3;		/* 19..21, 0x00380000 */
    FIELD  COLOR_v_ink_mode                           :  3;		/* 22..24, 0x01C00000 */
    FIELD  COLOR_y_ink_shift                          :  2;		/* 25..26, 0x06000000 */
    FIELD  COLOR_u_ink_shift                          :  2;		/* 27..28, 0x18000000 */
    FIELD  COLOR_v_ink_shift                          :  2;		/* 29..30, 0x60000000 */
    FIELD  rsv_31                                     :  1;		/* 31..31, 0x80000000 */
} ISP_COLOR_DBG_CFG_MAIN_T; //	/* 0x1502765C */

typedef union {
    typedef ISP_COLOR_DBG_CFG_MAIN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_DBG_CFG_MAIN_T;

typedef struct {
    FIELD  COLOR_c_boost_gain                         :  8;		/*  0.. 7, 0x000000FF */
    FIELD  rsv_8                                      :  5;		/*  8..12, 0x00001F00 */
    FIELD  COLOR_c_new_boost_en                       :  1;		/* 13..13, 0x00002000 */
    FIELD  rsv_14                                     :  2;		/* 14..15, 0x0000C000 */
    FIELD  COLOR_c_new_boost_lmt_l                    :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_c_new_boost_lmt_u                    :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_C_BOOST_MAIN_T; //	/* 0x15027660 */

typedef union {
    typedef ISP_COLOR_C_BOOST_MAIN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_C_BOOST_MAIN_T;

typedef struct {
    FIELD  COLOR_color_cboost_yoffset                 :  8;		/*  0.. 7, 0x000000FF */
    FIELD  rsv_8                                      :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_color_cboost_yoffset_sel             :  2;		/* 16..17, 0x00030000 */
    FIELD  rsv_18                                     :  6;		/* 18..23, 0x00FC0000 */
    FIELD  COLOR_color_cboost_yconst                  :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_C_BOOST_MAIN_2_T; //	/* 0x15027664 */

typedef union {
    typedef ISP_COLOR_C_BOOST_MAIN_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_C_BOOST_MAIN_2_T;

typedef struct {
    FIELD  COLOR_y_level_adj                          :  7;		/*  0.. 6, 0x0000007F */
    FIELD  rsv_7                                      :  1;		/*  7.. 7, 0x00000080 */
    FIELD  COLOR_y_slope_lmt                          :  8;		/*  8..15, 0x0000FF00 */
    FIELD  rsv_16                                     : 16;		/* 16..31, 0xFFFF0000 */
} ISP_COLOR_LUMA_ADJ_T; //	/* 0x15027668 */

typedef union {
    typedef ISP_COLOR_LUMA_ADJ_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_LUMA_ADJ_T;

typedef struct {
    FIELD  COLOR_g_contrast                           : 10;		/*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                     :  6;		/* 10..15, 0x0000FC00 */
    FIELD  COLOR_g_brightness                         : 11;		/* 16..26, 0x07FF0000 */
    FIELD  rsv_27                                     :  5;		/* 27..31, 0xF8000000 */
} ISP_COLOR_G_PIC_ADJ_MAIN_1_T; //	/* 0x1502766C */

typedef union {
    typedef ISP_COLOR_G_PIC_ADJ_MAIN_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_G_PIC_ADJ_MAIN_1_T;

typedef struct {
    FIELD  COLOR_g_saturation                         : 10;		/*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                     : 22;		/* 10..31, 0xFFFFFC00 */
} ISP_COLOR_G_PIC_ADJ_MAIN_2_T; //	/* 0x15027670 */

typedef union {
    typedef ISP_COLOR_G_PIC_ADJ_MAIN_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_G_PIC_ADJ_MAIN_2_T;

typedef struct {
    FIELD  COLOR_pos_x                                : 16;		/*  0..15, 0x0000FFFF */
    FIELD  COLOR_pos_y                                : 16;		/* 16..31, 0xFFFF0000 */
} ISP_COLOR_POS_MAIN_T; //	/* 0x15027674 */

typedef union {
    typedef ISP_COLOR_POS_MAIN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_POS_MAIN_T;

typedef struct {
    FIELD  COLOR_ink_data_y                           : 10;		/*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                     :  6;		/* 10..15, 0x0000FC00 */
    FIELD  COLOR_ink_data_u                           : 10;		/* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                     :  6;		/* 26..31, 0xFC000000 */
} ISP_COLOR_INK_DATA_MAIN_T; //	/* 0x15027678 */

typedef union {
    typedef ISP_COLOR_INK_DATA_MAIN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_INK_DATA_MAIN_T;

typedef struct {
    FIELD  COLOR_ink_data_v                           : 10;		/*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                     : 22;		/* 10..31, 0xFFFFFC00 */
} ISP_COLOR_INK_DATA_MAIN_CR_T; //	/* 0x1502767C */

typedef union {
    typedef ISP_COLOR_INK_DATA_MAIN_CR_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_INK_DATA_MAIN_CR_T;

typedef struct {
    FIELD  COLOR_cap_in_data_y                        : 10;		/*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                     :  6;		/* 10..15, 0x0000FC00 */
    FIELD  COLOR_cap_in_data_u                        : 10;		/* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                     :  6;		/* 26..31, 0xFC000000 */
} ISP_COLOR_CAP_IN_DATA_MAIN_T; //	/* 0x15027680 */

typedef union {
    typedef ISP_COLOR_CAP_IN_DATA_MAIN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CAP_IN_DATA_MAIN_T;

typedef struct {
    FIELD  COLOR_cap_in_data_v                        : 10;		/*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                     : 22;		/* 10..31, 0xFFFFFC00 */
} ISP_COLOR_CAP_IN_DATA_MAIN_CR_T; //	/* 0x15027684 */

typedef union {
    typedef ISP_COLOR_CAP_IN_DATA_MAIN_CR_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CAP_IN_DATA_MAIN_CR_T;

typedef struct {
    FIELD  COLOR_cap_out_data_y                       : 10;		/*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                     :  6;		/* 10..15, 0x0000FC00 */
    FIELD  COLOR_cap_out_data_u                       : 12;		/* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                     :  4;		/* 28..31, 0xF0000000 */
} ISP_COLOR_CAP_OUT_DATA_MAIN_T; //	/* 0x15027688 */

typedef union {
    typedef ISP_COLOR_CAP_OUT_DATA_MAIN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CAP_OUT_DATA_MAIN_T;

typedef struct {
    FIELD  COLOR_cap_out_data_v                       : 13;		/*  0..12, 0x00001FFF */
    FIELD  rsv_13                                     : 19;		/* 13..31, 0xFFFFE000 */
} ISP_COLOR_CAP_OUT_DATA_MAIN_CR_T; //	/* 0x1502768C */

typedef union {
    typedef ISP_COLOR_CAP_OUT_DATA_MAIN_CR_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CAP_OUT_DATA_MAIN_CR_T;

typedef struct {
    FIELD  COLOR_y_slope_0                            :  8;		/*  0.. 7, 0x000000FF */
    FIELD  rsv_8                                      :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_y_slope_1                            :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                     :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_Y_SLOPE_1_0_MAIN_T; //	/* 0x15027690 */

typedef union {
    typedef ISP_COLOR_Y_SLOPE_1_0_MAIN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_Y_SLOPE_1_0_MAIN_T;

typedef struct {
    FIELD  COLOR_y_slope_2                            :  8;		/*  0.. 7, 0x000000FF */
    FIELD  rsv_8                                      :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_y_slope_3                            :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                     :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_Y_SLOPE_3_2_MAIN_T; //	/* 0x15027694 */

typedef union {
    typedef ISP_COLOR_Y_SLOPE_3_2_MAIN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_Y_SLOPE_3_2_MAIN_T;

typedef struct {
    FIELD  COLOR_y_slope_4                            :  8;		/*  0.. 7, 0x000000FF */
    FIELD  rsv_8                                      :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_y_slope_5                            :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                     :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_Y_SLOPE_5_4_MAIN_T; //	/* 0x15027698 */

typedef union {
    typedef ISP_COLOR_Y_SLOPE_5_4_MAIN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_Y_SLOPE_5_4_MAIN_T;

typedef struct {
    FIELD  COLOR_y_slope_6                            :  8;		/*  0.. 7, 0x000000FF */
    FIELD  rsv_8                                      :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_y_slope_7                            :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                     :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_Y_SLOPE_7_6_MAIN_T; //	/* 0x1502769C */

typedef union {
    typedef ISP_COLOR_Y_SLOPE_7_6_MAIN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_Y_SLOPE_7_6_MAIN_T;

typedef struct {
    FIELD  COLOR_y_slope_8                            :  8;		/*  0.. 7, 0x000000FF */
    FIELD  rsv_8                                      :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_y_slope_9                            :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                     :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_Y_SLOPE_9_8_MAIN_T; //	/* 0x150276A0 */

typedef union {
    typedef ISP_COLOR_Y_SLOPE_9_8_MAIN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_Y_SLOPE_9_8_MAIN_T;

typedef struct {
    FIELD  COLOR_y_slope_10                           :  8;		/*  0.. 7, 0x000000FF */
    FIELD  rsv_8                                      :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_y_slope_11                           :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                     :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_Y_SLOPE_11_10_MAIN_T; //	/* 0x150276A4 */

typedef union {
    typedef ISP_COLOR_Y_SLOPE_11_10_MAIN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_Y_SLOPE_11_10_MAIN_T;

typedef struct {
    FIELD  COLOR_y_slope_12                           :  8;		/*  0.. 7, 0x000000FF */
    FIELD  rsv_8                                      :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_y_slope_13                           :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                     :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_Y_SLOPE_13_12_MAIN_T; //	/* 0x150276A8 */

typedef union {
    typedef ISP_COLOR_Y_SLOPE_13_12_MAIN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_Y_SLOPE_13_12_MAIN_T;

typedef struct {
    FIELD  COLOR_y_slope_14                           :  8;		/*  0.. 7, 0x000000FF */
    FIELD  rsv_8                                      :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_y_slope_15                           :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                     :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_Y_SLOPE_15_14_MAIN_T; //	/* 0x150276AC */

typedef union {
    typedef ISP_COLOR_Y_SLOPE_15_14_MAIN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_Y_SLOPE_15_14_MAIN_T;

typedef struct {
    FIELD  COLOR_hue_to_hue_0                         :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_hue_to_hue_1                         :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_hue_to_hue_2                         :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_hue_to_hue_3                         :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_LOCAL_HUE_CD_0_T; //	/* 0x150276B0 */

typedef union {
    typedef ISP_COLOR_LOCAL_HUE_CD_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_LOCAL_HUE_CD_0_T;

typedef struct {
    FIELD  COLOR_hue_to_hue_4                         :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_hue_to_hue_5                         :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_hue_to_hue_6                         :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_hue_to_hue_7                         :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_LOCAL_HUE_CD_1_T; //	/* 0x150276B4 */

typedef union {
    typedef ISP_COLOR_LOCAL_HUE_CD_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_LOCAL_HUE_CD_1_T;

typedef struct {
    FIELD  COLOR_hue_to_hue_8                         :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_hue_to_hue_9                         :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_hue_to_hue_10                        :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_hue_to_hue_11                        :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_LOCAL_HUE_CD_2_T; //	/* 0x150276B8 */

typedef union {
    typedef ISP_COLOR_LOCAL_HUE_CD_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_LOCAL_HUE_CD_2_T;

typedef struct {
    FIELD  COLOR_hue_to_hue_12                        :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_hue_to_hue_13                        :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_hue_to_hue_14                        :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_hue_to_hue_15                        :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_LOCAL_HUE_CD_3_T; //	/* 0x150276BC */

typedef union {
    typedef ISP_COLOR_LOCAL_HUE_CD_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_LOCAL_HUE_CD_3_T;

typedef struct {
    FIELD  COLOR_hue_to_hue_16                        :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_hue_to_hue_17                        :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_hue_to_hue_18                        :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_hue_to_hue_19                        :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_LOCAL_HUE_CD_4_T; //	/* 0x150276C0 */

typedef union {
    typedef ISP_COLOR_LOCAL_HUE_CD_4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_LOCAL_HUE_CD_4_T;

typedef struct {
    FIELD  COLOR_w1_hue_lower                         :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_w1_hue_upper                         :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_w1_sat_lower                         :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_w1_sat_upper                         :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_TWO_D_WINDOW_1_T; //	/* 0x150276C4 */

typedef union {
    typedef ISP_COLOR_TWO_D_WINDOW_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_TWO_D_WINDOW_1_T;

typedef struct {
    FIELD  COLOR_w1_hit_outer                         : 24;		/*  0..23, 0x00FFFFFF */
    FIELD  rsv_24                                     :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_TWO_D_W1_RESULT_T; //	/* 0x150276C8 */

typedef union {
    typedef ISP_COLOR_TWO_D_W1_RESULT_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_TWO_D_W1_RESULT_T;

typedef struct {
    FIELD  COLOR_hist_x_start                         : 16;		/*  0..15, 0x0000FFFF */
    FIELD  COLOR_hist_x_end                           : 16;		/* 16..31, 0xFFFF0000 */
} ISP_COLOR_SAT_HIST_X_CFG_MAIN_T; //	/* 0x150276CC */

typedef union {
    typedef ISP_COLOR_SAT_HIST_X_CFG_MAIN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_SAT_HIST_X_CFG_MAIN_T;

typedef struct {
    FIELD  COLOR_hist_y_start                         : 16;		/*  0..15, 0x0000FFFF */
    FIELD  COLOR_hist_y_end                           : 16;		/* 16..31, 0xFFFF0000 */
} ISP_COLOR_SAT_HIST_Y_CFG_MAIN_T; //	/* 0x150276D0 */

typedef union {
    typedef ISP_COLOR_SAT_HIST_Y_CFG_MAIN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_SAT_HIST_Y_CFG_MAIN_T;

typedef struct {
    FIELD  rsv_0                                      : 26;		/*  0..25, 0x03FFFFFF */
    FIELD  COLOR_color_atpg_ob                        :  1;		/* 26..26, 0x04000000 */
    FIELD  COLOR_color_atpg_ct                        :  1;		/* 27..27, 0x08000000 */
    FIELD  rsv_28                                     :  4;		/* 28..31, 0xF0000000 */
} ISP_COLOR_BWS_2_T; //	/* 0x150276D4 */

typedef union {
    typedef ISP_COLOR_BWS_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_BWS_2_T;

typedef struct {
    FIELD  COLOR_crc_r_en                             :  1;		/*  0.. 0, 0x00000001 */
    FIELD  COLOR_crc_l_en                             :  1;		/*  1.. 1, 0x00000002 */
    FIELD  COLOR_c_crc_still_check_trig               :  1;		/*  2.. 2, 0x00000004 */
    FIELD  COLOR_crc_still_check_done                 :  1;		/*  3.. 3, 0x00000008 */
    FIELD  COLOR_crc_non_still_cnt                    :  4;		/*  4.. 7, 0x000000F0 */
    FIELD  COLOR_crc_still_check_max                  :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_crc_src_sel                          :  4;		/* 16..19, 0x000F0000 */
    FIELD  rsv_20                                     : 12;		/* 20..31, 0xFFF00000 */
} ISP_COLOR_CRC_0_T; //	/* 0x150276D8 */

typedef union {
    typedef ISP_COLOR_CRC_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CRC_0_T;

typedef struct {
    FIELD  COLOR_crc_clip_h_start                     : 13;		/*  0..12, 0x00001FFF */
    FIELD  rsv_13                                     :  3;		/* 13..15, 0x0000E000 */
    FIELD  COLOR_crc_clip_h_end                       : 13;		/* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                     :  3;		/* 29..31, 0xE0000000 */
} ISP_COLOR_CRC_1_T; //	/* 0x150276DC */

typedef union {
    typedef ISP_COLOR_CRC_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CRC_1_T;

typedef struct {
    FIELD  COLOR_crc_clip_v_start                     : 12;		/*  0..11, 0x00000FFF */
    FIELD  rsv_12                                     :  4;		/* 12..15, 0x0000F000 */
    FIELD  COLOR_crc_clip_v_end                       : 12;		/* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                     :  4;		/* 28..31, 0xF0000000 */
} ISP_COLOR_CRC_2_T; //	/* 0x150276E0 */

typedef union {
    typedef ISP_COLOR_CRC_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CRC_2_T;

typedef struct {
    FIELD  COLOR_crc_v_mask                           : 10;		/*  0.. 9, 0x000003FF */
    FIELD  COLOR_crc_c_mask                           : 10;		/* 10..19, 0x000FFC00 */
    FIELD  COLOR_crc_y_mask                           : 10;		/* 20..29, 0x3FF00000 */
    FIELD  rsv_30                                     :  2;		/* 30..31, 0xC0000000 */
} ISP_COLOR_CRC_3_T; //	/* 0x150276E4 */

typedef union {
    typedef ISP_COLOR_CRC_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CRC_3_T;

typedef struct {
    FIELD  COLOR_crc_result                           : 32;		/*  0..31, 0xFFFFFFFF */
} ISP_COLOR_CRC_4_T; //	/* 0x150276E8 */

typedef union {
    typedef ISP_COLOR_CRC_4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CRC_4_T;

typedef struct {
    FIELD  COLOR_sat_gain1_hue_0                      :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_sat_gain1_hue_1                      :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_sat_gain1_hue_2                      :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_sat_gain1_hue_3                      :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_PARTIAL_SAT_GAIN1_0_T; //	/* 0x150276EC */

typedef union {
    typedef ISP_COLOR_PARTIAL_SAT_GAIN1_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN1_0_T;

typedef struct {
    FIELD  COLOR_sat_gain1_hue_4                      :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_sat_gain1_hue_5                      :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_sat_gain1_hue_6                      :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_sat_gain1_hue_7                      :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_PARTIAL_SAT_GAIN1_1_T; //	/* 0x150276F0 */

typedef union {
    typedef ISP_COLOR_PARTIAL_SAT_GAIN1_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN1_1_T;

typedef struct {
    FIELD  COLOR_sat_gain1_hue_8                      :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_sat_gain1_hue_9                      :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_sat_gain1_hue_10                     :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_sat_gain1_hue_11                     :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_PARTIAL_SAT_GAIN1_2_T; //	/* 0x150276F4 */

typedef union {
    typedef ISP_COLOR_PARTIAL_SAT_GAIN1_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN1_2_T;

typedef struct {
    FIELD  COLOR_sat_gain1_hue_12                     :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_sat_gain1_hue_13                     :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_sat_gain1_hue_14                     :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_sat_gain1_hue_15                     :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_PARTIAL_SAT_GAIN1_3_T; //	/* 0x150276F8 */

typedef union {
    typedef ISP_COLOR_PARTIAL_SAT_GAIN1_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN1_3_T;

typedef struct {
    FIELD  COLOR_sat_gain1_hue_16                     :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_sat_gain1_hue_17                     :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_sat_gain1_hue_18                     :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_sat_gain1_hue_19                     :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_PARTIAL_SAT_GAIN1_4_T; //	/* 0x150276FC */

typedef union {
    typedef ISP_COLOR_PARTIAL_SAT_GAIN1_4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN1_4_T;

typedef struct {
    FIELD  COLOR_sat_gain2_hue_0                      :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_sat_gain2_hue_1                      :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_sat_gain2_hue_2                      :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_sat_gain2_hue_3                      :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_PARTIAL_SAT_GAIN2_0_T; //	/* 0x15027700 */

typedef union {
    typedef ISP_COLOR_PARTIAL_SAT_GAIN2_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN2_0_T;

typedef struct {
    FIELD  COLOR_sat_gain2_hue_4                      :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_sat_gain2_hue_5                      :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_sat_gain2_hue_6                      :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_sat_gain2_hue_7                      :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_PARTIAL_SAT_GAIN2_1_T; //	/* 0x15027704 */

typedef union {
    typedef ISP_COLOR_PARTIAL_SAT_GAIN2_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN2_1_T;

typedef struct {
    FIELD  COLOR_sat_gain2_hue_8                      :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_sat_gain2_hue_9                      :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_sat_gain2_hue_10                     :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_sat_gain2_hue_11                     :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_PARTIAL_SAT_GAIN2_2_T; //	/* 0x15027708 */

typedef union {
    typedef ISP_COLOR_PARTIAL_SAT_GAIN2_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN2_2_T;

typedef struct {
    FIELD  COLOR_sat_gain2_hue_12                     :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_sat_gain2_hue_13                     :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_sat_gain2_hue_14                     :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_sat_gain2_hue_15                     :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_PARTIAL_SAT_GAIN2_3_T; //	/* 0x1502770C */

typedef union {
    typedef ISP_COLOR_PARTIAL_SAT_GAIN2_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN2_3_T;

typedef struct {
    FIELD  COLOR_sat_gain2_hue_16                     :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_sat_gain2_hue_17                     :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_sat_gain2_hue_18                     :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_sat_gain2_hue_19                     :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_PARTIAL_SAT_GAIN2_4_T; //	/* 0x15027710 */

typedef union {
    typedef ISP_COLOR_PARTIAL_SAT_GAIN2_4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN2_4_T;

typedef struct {
    FIELD  COLOR_sat_gain3_hue_0                      :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_sat_gain3_hue_1                      :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_sat_gain3_hue_2                      :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_sat_gain3_hue_3                      :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_PARTIAL_SAT_GAIN3_0_T; //	/* 0x15027714 */

typedef union {
    typedef ISP_COLOR_PARTIAL_SAT_GAIN3_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN3_0_T;

typedef struct {
    FIELD  COLOR_sat_gain3_hue_4                      :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_sat_gain3_hue_5                      :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_sat_gain3_hue_6                      :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_sat_gain3_hue_7                      :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_PARTIAL_SAT_GAIN3_1_T; //	/* 0x15027718 */

typedef union {
    typedef ISP_COLOR_PARTIAL_SAT_GAIN3_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN3_1_T;

typedef struct {
    FIELD  COLOR_sat_gain3_hue_8                      :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_sat_gain3_hue_9                      :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_sat_gain3_hue_10                     :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_sat_gain3_hue_11                     :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_PARTIAL_SAT_GAIN3_2_T; //	/* 0x1502771C */

typedef union {
    typedef ISP_COLOR_PARTIAL_SAT_GAIN3_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN3_2_T;

typedef struct {
    FIELD  COLOR_sat_gain3_hue_12                     :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_sat_gain3_hue_13                     :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_sat_gain3_hue_14                     :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_sat_gain3_hue_15                     :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_PARTIAL_SAT_GAIN3_3_T; //	/* 0x15027720 */

typedef union {
    typedef ISP_COLOR_PARTIAL_SAT_GAIN3_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN3_3_T;

typedef struct {
    FIELD  COLOR_sat_gain3_hue_16                     :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_sat_gain3_hue_17                     :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_sat_gain3_hue_18                     :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_sat_gain3_hue_19                     :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_PARTIAL_SAT_GAIN3_4_T; //	/* 0x15027724 */

typedef union {
    typedef ISP_COLOR_PARTIAL_SAT_GAIN3_4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN3_4_T;

typedef struct {
    FIELD  COLOR_sat_point1_hue_0                     :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_sat_point1_hue_1                     :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_sat_point1_hue_2                     :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_sat_point1_hue_3                     :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_PARTIAL_SAT_POINT1_0_T; //	/* 0x15027728 */

typedef union {
    typedef ISP_COLOR_PARTIAL_SAT_POINT1_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_PARTIAL_SAT_POINT1_0_T;

typedef struct {
    FIELD  COLOR_sat_point1_hue_4                     :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_sat_point1_hue_5                     :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_sat_point1_hue_6                     :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_sat_point1_hue_7                     :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_PARTIAL_SAT_POINT1_1_T; //	/* 0x1502772C */

typedef union {
    typedef ISP_COLOR_PARTIAL_SAT_POINT1_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_PARTIAL_SAT_POINT1_1_T;

typedef struct {
    FIELD  COLOR_sat_point1_hue_8                     :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_sat_point1_hue_9                     :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_sat_point1_hue_10                    :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_sat_point1_hue_11                    :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_PARTIAL_SAT_POINT1_2_T; //	/* 0x15027730 */

typedef union {
    typedef ISP_COLOR_PARTIAL_SAT_POINT1_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_PARTIAL_SAT_POINT1_2_T;

typedef struct {
    FIELD  COLOR_sat_point1_hue_12                    :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_sat_point1_hue_13                    :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_sat_point1_hue_14                    :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_sat_point1_hue_15                    :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_PARTIAL_SAT_POINT1_3_T; //	/* 0x15027734 */

typedef union {
    typedef ISP_COLOR_PARTIAL_SAT_POINT1_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_PARTIAL_SAT_POINT1_3_T;

typedef struct {
    FIELD  COLOR_sat_point1_hue_16                    :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_sat_point1_hue_17                    :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_sat_point1_hue_18                    :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_sat_point1_hue_19                    :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_PARTIAL_SAT_POINT1_4_T; //	/* 0x15027738 */

typedef union {
    typedef ISP_COLOR_PARTIAL_SAT_POINT1_4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_PARTIAL_SAT_POINT1_4_T;

typedef struct {
    FIELD  COLOR_sat_point2_hue_0                     :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_sat_point2_hue_1                     :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_sat_point2_hue_2                     :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_sat_point2_hue_3                     :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_PARTIAL_SAT_POINT2_0_T; //	/* 0x1502773C */

typedef union {
    typedef ISP_COLOR_PARTIAL_SAT_POINT2_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_PARTIAL_SAT_POINT2_0_T;

typedef struct {
    FIELD  COLOR_sat_point2_hue_4                     :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_sat_point2_hue_5                     :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_sat_point2_hue_6                     :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_sat_point2_hue_7                     :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_PARTIAL_SAT_POINT2_1_T; //	/* 0x15027740 */

typedef union {
    typedef ISP_COLOR_PARTIAL_SAT_POINT2_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_PARTIAL_SAT_POINT2_1_T;

typedef struct {
    FIELD  COLOR_sat_point2_hue_8                     :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_sat_point2_hue_9                     :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_sat_point2_hue_10                    :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_sat_point2_hue_11                    :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_PARTIAL_SAT_POINT2_2_T; //	/* 0x15027744 */

typedef union {
    typedef ISP_COLOR_PARTIAL_SAT_POINT2_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_PARTIAL_SAT_POINT2_2_T;

typedef struct {
    FIELD  COLOR_sat_point2_hue_12                    :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_sat_point2_hue_13                    :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_sat_point2_hue_14                    :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_sat_point2_hue_15                    :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_PARTIAL_SAT_POINT2_3_T; //	/* 0x15027748 */

typedef union {
    typedef ISP_COLOR_PARTIAL_SAT_POINT2_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_PARTIAL_SAT_POINT2_3_T;

typedef struct {
    FIELD  COLOR_sat_point2_hue_16                    :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_sat_point2_hue_17                    :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_sat_point2_hue_18                    :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_sat_point2_hue_19                    :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_PARTIAL_SAT_POINT2_4_T; //	/* 0x1502774C */

typedef union {
    typedef ISP_COLOR_PARTIAL_SAT_POINT2_4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_PARTIAL_SAT_POINT2_4_T;

typedef struct {
    FIELD  COLOR_disp_color_start                     :  1;		/*  0.. 0, 0x00000001 */
    FIELD  COLOR_disp_color_out_sel                   :  1;		/*  1.. 1, 0x00000002 */
    FIELD  COLOR_disp_color_wrap_mode                 :  1;		/*  2.. 2, 0x00000004 */
    FIELD  COLOR_disp_color_dbuf_vsync                :  1;		/*  3.. 3, 0x00000008 */
    FIELD  COLOR_direct_yuv_bit_sel                   :  1;		/*  4.. 4, 0x00000010 */
    FIELD  COLOR_stop_to_wdma                         :  1;		/*  5.. 5, 0x00000020 */
    FIELD  rsv_6                                      :  2;		/*  6.. 7, 0x000000C0 */
    FIELD  COLOR_disp_color_sw_rst_engine             :  1;		/*  8.. 8, 0x00000100 */
    FIELD  rsv_9                                      :  7;		/*  9..15, 0x0000FE00 */
    FIELD  COLOR_disp_color_dbg_sel                   :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                     :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_START_T; //	/* 0x15027750 */

typedef union {
    typedef ISP_COLOR_START_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_START_T;

typedef struct {
    FIELD  COLOR_disp_color_inten_fr_complete         :  1;		/*  0.. 0, 0x00000001 */
    FIELD  COLOR_disp_color_inten_fr_done             :  1;		/*  1.. 1, 0x00000002 */
    FIELD  COLOR_disp_color_inten_fr_underrun         :  1;		/*  2.. 2, 0x00000004 */
    FIELD  rsv_3                                      : 29;		/*  3..31, 0xFFFFFFF8 */
} ISP_COLOR_INTEN_T; //	/* 0x15027754 */

typedef union {
    typedef ISP_COLOR_INTEN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_INTEN_T;

typedef struct {
    FIELD  COLOR_disp_color_intsta_fr_complete        :  1;		/*  0.. 0, 0x00000001 */
    FIELD  COLOR_disp_color_intsta_fr_done            :  1;		/*  1.. 1, 0x00000002 */
    FIELD  COLOR_disp_color_intsta_fr_underrun        :  1;		/*  2.. 2, 0x00000004 */
    FIELD  rsv_3                                      : 29;		/*  3..31, 0xFFFFFFF8 */
} ISP_COLOR_INTSTA_T; //	/* 0x15027758 */

typedef union {
    typedef ISP_COLOR_INTSTA_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_INTSTA_T;

typedef struct {
    FIELD  COLOR_reg_cha_sel                          :  1;		/*  0.. 0, 0x00000001 */
    FIELD  COLOR_reg_cha_round                        :  1;		/*  1.. 1, 0x00000002 */
    FIELD  COLOR_reg_cha_uv                           :  1;		/*  2.. 2, 0x00000004 */
    FIELD  rsv_3                                      :  1;		/*  3.. 3, 0x00000008 */
    FIELD  COLOR_reg_chb_sel                          :  1;		/*  4.. 4, 0x00000010 */
    FIELD  COLOR_reg_chb_round                        :  1;		/*  5.. 5, 0x00000020 */
    FIELD  COLOR_reg_chb_uv                           :  1;		/*  6.. 6, 0x00000040 */
    FIELD  rsv_7                                      :  1;		/*  7.. 7, 0x00000080 */
    FIELD  COLOR_reg_chc_sel                          :  1;		/*  8.. 8, 0x00000100 */
    FIELD  COLOR_reg_chc_round                        :  1;		/*  9.. 9, 0x00000200 */
    FIELD  COLOR_reg_chc_uv                           :  1;		/* 10..10, 0x00000400 */
    FIELD  rsv_11                                     : 21;		/* 11..31, 0xFFFFF800 */
} ISP_COLOR_OUT_SEL_T; //	/* 0x1502775C */

typedef union {
    typedef ISP_COLOR_OUT_SEL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_OUT_SEL_T;

typedef struct {
    FIELD  COLOR_disp_color_frame_done_del            :  8;		/*  0.. 7, 0x000000FF */
    FIELD  rsv_8                                      : 24;		/*  8..31, 0xFFFFFF00 */
} ISP_COLOR_FRAME_DONE_DEL_T; //	/* 0x15027760 */

typedef union {
    typedef ISP_COLOR_FRAME_DONE_DEL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_FRAME_DONE_DEL_T;

typedef struct {
    FIELD  COLOR_disp_color_crc_cen                   :  1;		/*  0.. 0, 0x00000001 */
    FIELD  COLOR_disp_color_crc_start                 :  1;		/*  1.. 1, 0x00000002 */
    FIELD  COLOR_disp_color_crc_clr                   :  1;		/*  2.. 2, 0x00000004 */
    FIELD  rsv_3                                      : 29;		/*  3..31, 0xFFFFFFF8 */
} ISP_COLOR_CRC_T; //	/* 0x15027764 */

typedef union {
    typedef ISP_COLOR_CRC_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CRC_T;

typedef struct {
    FIELD  COLOR_disp_color_sw_scratch                : 32;		/*  0..31, 0xFFFFFFFF */
} ISP_COLOR_SW_SCRATCH_T; //	/* 0x15027768 */

typedef union {
    typedef ISP_COLOR_SW_SCRATCH_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_SW_SCRATCH_T;

typedef struct {
    FIELD  COLOR_disp_color_ck_on                     :  1;		/*  0.. 0, 0x00000001 */
    FIELD  rsv_1                                      : 31;		/*  1..31, 0xFFFFFFFE */
} ISP_COLOR_CK_ON_T; //	/* 0x1502776C */

typedef union {
    typedef ISP_COLOR_CK_ON_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CK_ON_T;

typedef struct {
    FIELD  COLOR_disp_color_internal_ip_width         : 14;		/*  0..13, 0x00003FFF */
    FIELD  rsv_14                                     : 18;		/* 14..31, 0xFFFFC000 */
} ISP_COLOR_INTERNAL_IP_WIDTH_T; //	/* 0x15027770 */

typedef union {
    typedef ISP_COLOR_INTERNAL_IP_WIDTH_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_INTERNAL_IP_WIDTH_T;

typedef struct {
    FIELD  COLOR_disp_color_internal_ip_height        : 14;		/*  0..13, 0x00003FFF */
    FIELD  rsv_14                                     : 18;		/* 14..31, 0xFFFFC000 */
} ISP_COLOR_INTERNAL_IP_HEIGHT_T; //	/* 0x15027774 */

typedef union {
    typedef ISP_COLOR_INTERNAL_IP_HEIGHT_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_INTERNAL_IP_HEIGHT_T;

typedef struct {
    FIELD  COLOR_disp_color_cm1_en                    :  1;		/*  0.. 0, 0x00000001 */
    FIELD  COLOR_disp_color_cm1_clip_en               :  1;		/*  1.. 1, 0x00000002 */
    FIELD  rsv_2                                      : 30;		/*  2..31, 0xFFFFFFFC */
} ISP_COLOR_CM1_EN_T; //	/* 0x15027778 */

typedef union {
    typedef ISP_COLOR_CM1_EN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM1_EN_T;

typedef struct {
    FIELD  COLOR_disp_color_cm2_en                    :  1;		/*  0.. 0, 0x00000001 */
    FIELD  COLOR_disp_color_cm2_clip_sel              :  2;		/*  1.. 2, 0x00000006 */
    FIELD  rsv_3                                      :  1;		/*  3.. 3, 0x00000008 */
    FIELD  COLOR_disp_color_cm2_round_off             :  1;		/*  4.. 4, 0x00000010 */
    FIELD  rsv_5                                      : 27;		/*  5..31, 0xFFFFFFE0 */
} ISP_COLOR_CM2_EN_T; //	/* 0x1502777C */

typedef union {
    typedef ISP_COLOR_CM2_EN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM2_EN_T;

typedef struct {
    FIELD  COLOR_bypass_shadow                        :  1;		/*  0.. 0, 0x00000001 */
    FIELD  COLOR_force_commit                         :  1;		/*  1.. 1, 0x00000002 */
    FIELD  COLOR_read_wrk_reg                         :  1;		/*  2.. 2, 0x00000004 */
    FIELD  rsv_3                                      : 29;		/*  3..31, 0xFFFFFFF8 */
} ISP_COLOR_SHADOW_CTRL_T; //	/* 0x15027780 */

typedef union {
    typedef ISP_COLOR_SHADOW_CTRL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_SHADOW_CTRL_T;

typedef struct {
    FIELD  COLOR_disp_color_crc_out_0                 : 16;		/*  0..15, 0x0000FFFF */
    FIELD  COLOR_disp_color_crc_rdy_0                 :  1;		/* 16..16, 0x00010000 */
    FIELD  COLOR_disp_color_engine_end                :  1;		/* 17..17, 0x00020000 */
    FIELD  rsv_18                                     : 14;		/* 18..31, 0xFFFC0000 */
} ISP_COLOR_R0_CRC_T; //	/* 0x15027784 */

typedef union {
    typedef ISP_COLOR_R0_CRC_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_R0_CRC_T;

typedef struct {
    FIELD  COLOR_s_gain_y0_hue_0                      :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_s_gain_y0_hue_1                      :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_s_gain_y0_hue_2                      :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_s_gain_y0_hue_3                      :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_S_GAIN_BY_Y0_0_T; //	/* 0x15027788 */

typedef union {
    typedef ISP_COLOR_S_GAIN_BY_Y0_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_S_GAIN_BY_Y0_0_T;

typedef struct {
    FIELD  COLOR_s_gain_y0_hue_4                      :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_s_gain_y0_hue_5                      :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_s_gain_y0_hue_6                      :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_s_gain_y0_hue_7                      :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_S_GAIN_BY_Y0_1_T; //	/* 0x1502778C */

typedef union {
    typedef ISP_COLOR_S_GAIN_BY_Y0_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_S_GAIN_BY_Y0_1_T;

typedef struct {
    FIELD  COLOR_s_gain_y0_hue_8                      :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_s_gain_y0_hue_9                      :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_s_gain_y0_hue_10                     :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_s_gain_y0_hue_11                     :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_S_GAIN_BY_Y0_2_T; //	/* 0x15027790 */

typedef union {
    typedef ISP_COLOR_S_GAIN_BY_Y0_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_S_GAIN_BY_Y0_2_T;

typedef struct {
    FIELD  COLOR_s_gain_y0_hue_12                     :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_s_gain_y0_hue_13                     :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_s_gain_y0_hue_14                     :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_s_gain_y0_hue_15                     :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_S_GAIN_BY_Y0_3_T; //	/* 0x15027794 */

typedef union {
    typedef ISP_COLOR_S_GAIN_BY_Y0_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_S_GAIN_BY_Y0_3_T;

typedef struct {
    FIELD  COLOR_s_gain_y0_hue_16                     :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_s_gain_y0_hue_17                     :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_s_gain_y0_hue_18                     :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_s_gain_y0_hue_19                     :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_S_GAIN_BY_Y0_4_T; //	/* 0x15027798 */

typedef union {
    typedef ISP_COLOR_S_GAIN_BY_Y0_4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_S_GAIN_BY_Y0_4_T;

typedef struct {
    FIELD  COLOR_s_gain_y64_hue_0                     :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_s_gain_y64_hue_1                     :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_s_gain_y64_hue_2                     :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_s_gain_y64_hue_3                     :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_S_GAIN_BY_Y64_0_T; //	/* 0x1502779C */

typedef union {
    typedef ISP_COLOR_S_GAIN_BY_Y64_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_S_GAIN_BY_Y64_0_T;

typedef struct {
    FIELD  COLOR_s_gain_y64_hue_4                     :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_s_gain_y64_hue_5                     :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_s_gain_y64_hue_6                     :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_s_gain_y64_hue_7                     :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_S_GAIN_BY_Y64_1_T; //	/* 0x150277A0 */

typedef union {
    typedef ISP_COLOR_S_GAIN_BY_Y64_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_S_GAIN_BY_Y64_1_T;

typedef struct {
    FIELD  COLOR_s_gain_y64_hue_8                     :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_s_gain_y64_hue_9                     :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_s_gain_y64_hue_10                    :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_s_gain_y64_hue_11                    :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_S_GAIN_BY_Y64_2_T; //	/* 0x150277A4 */

typedef union {
    typedef ISP_COLOR_S_GAIN_BY_Y64_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_S_GAIN_BY_Y64_2_T;

typedef struct {
    FIELD  COLOR_s_gain_y64_hue_12                    :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_s_gain_y64_hue_13                    :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_s_gain_y64_hue_14                    :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_s_gain_y64_hue_15                    :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_S_GAIN_BY_Y64_3_T; //	/* 0x150277A8 */

typedef union {
    typedef ISP_COLOR_S_GAIN_BY_Y64_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_S_GAIN_BY_Y64_3_T;

typedef struct {
    FIELD  COLOR_s_gain_y64_hue_16                    :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_s_gain_y64_hue_17                    :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_s_gain_y64_hue_18                    :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_s_gain_y64_hue_19                    :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_S_GAIN_BY_Y64_4_T; //	/* 0x150277AC */

typedef union {
    typedef ISP_COLOR_S_GAIN_BY_Y64_4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_S_GAIN_BY_Y64_4_T;

typedef struct {
    FIELD  COLOR_s_gain_y128_hue_0                    :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_s_gain_y128_hue_1                    :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_s_gain_y128_hue_2                    :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_s_gain_y128_hue_3                    :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_S_GAIN_BY_Y128_0_T; //	/* 0x150277B0 */

typedef union {
    typedef ISP_COLOR_S_GAIN_BY_Y128_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_S_GAIN_BY_Y128_0_T;

typedef struct {
    FIELD  COLOR_s_gain_y128_hue_4                    :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_s_gain_y128_hue_5                    :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_s_gain_y128_hue_6                    :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_s_gain_y128_hue_7                    :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_S_GAIN_BY_Y128_1_T; //	/* 0x150277B4 */

typedef union {
    typedef ISP_COLOR_S_GAIN_BY_Y128_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_S_GAIN_BY_Y128_1_T;

typedef struct {
    FIELD  COLOR_s_gain_y128_hue_8                    :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_s_gain_y128_hue_9                    :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_s_gain_y128_hue_10                   :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_s_gain_y128_hue_11                   :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_S_GAIN_BY_Y128_2_T; //	/* 0x150277B8 */

typedef union {
    typedef ISP_COLOR_S_GAIN_BY_Y128_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_S_GAIN_BY_Y128_2_T;

typedef struct {
    FIELD  COLOR_s_gain_y128_hue_12                   :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_s_gain_y128_hue_13                   :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_s_gain_y128_hue_14                   :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_s_gain_y128_hue_15                   :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_S_GAIN_BY_Y128_3_T; //	/* 0x150277BC */

typedef union {
    typedef ISP_COLOR_S_GAIN_BY_Y128_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_S_GAIN_BY_Y128_3_T;

typedef struct {
    FIELD  COLOR_s_gain_y128_hue_16                   :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_s_gain_y128_hue_17                   :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_s_gain_y128_hue_18                   :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_s_gain_y128_hue_19                   :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_S_GAIN_BY_Y128_4_T; //	/* 0x150277C0 */

typedef union {
    typedef ISP_COLOR_S_GAIN_BY_Y128_4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_S_GAIN_BY_Y128_4_T;

typedef struct {
    FIELD  COLOR_s_gain_y192_hue_0                    :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_s_gain_y192_hue_1                    :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_s_gain_y192_hue_2                    :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_s_gain_y192_hue_3                    :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_S_GAIN_BY_Y192_0_T; //	/* 0x150277C4 */

typedef union {
    typedef ISP_COLOR_S_GAIN_BY_Y192_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_S_GAIN_BY_Y192_0_T;

typedef struct {
    FIELD  COLOR_s_gain_y192_hue_4                    :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_s_gain_y192_hue_5                    :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_s_gain_y192_hue_6                    :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_s_gain_y192_hue_7                    :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_S_GAIN_BY_Y192_1_T; //	/* 0x150277C8 */

typedef union {
    typedef ISP_COLOR_S_GAIN_BY_Y192_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_S_GAIN_BY_Y192_1_T;

typedef struct {
    FIELD  COLOR_s_gain_y192_hue_8                    :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_s_gain_y192_hue_9                    :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_s_gain_y192_hue_10                   :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_s_gain_y192_hue_11                   :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_S_GAIN_BY_Y192_2_T; //	/* 0x150277CC */

typedef union {
    typedef ISP_COLOR_S_GAIN_BY_Y192_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_S_GAIN_BY_Y192_2_T;

typedef struct {
    FIELD  COLOR_s_gain_y192_hue_12                   :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_s_gain_y192_hue_13                   :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_s_gain_y192_hue_14                   :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_s_gain_y192_hue_15                   :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_S_GAIN_BY_Y192_3_T; //	/* 0x150277D0 */

typedef union {
    typedef ISP_COLOR_S_GAIN_BY_Y192_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_S_GAIN_BY_Y192_3_T;

typedef struct {
    FIELD  COLOR_s_gain_y192_hue_16                   :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_s_gain_y192_hue_17                   :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_s_gain_y192_hue_18                   :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_s_gain_y192_hue_19                   :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_S_GAIN_BY_Y192_4_T; //	/* 0x150277D4 */

typedef union {
    typedef ISP_COLOR_S_GAIN_BY_Y192_4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_S_GAIN_BY_Y192_4_T;

typedef struct {
    FIELD  COLOR_s_gain_y256_hue_0                    :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_s_gain_y256_hue_1                    :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_s_gain_y256_hue_2                    :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_s_gain_y256_hue_3                    :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_S_GAIN_BY_Y256_0_T; //	/* 0x150277D8 */

typedef union {
    typedef ISP_COLOR_S_GAIN_BY_Y256_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_S_GAIN_BY_Y256_0_T;

typedef struct {
    FIELD  COLOR_s_gain_y256_hue_4                    :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_s_gain_y256_hue_5                    :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_s_gain_y256_hue_6                    :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_s_gain_y256_hue_7                    :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_S_GAIN_BY_Y256_1_T; //	/* 0x150277DC */

typedef union {
    typedef ISP_COLOR_S_GAIN_BY_Y256_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_S_GAIN_BY_Y256_1_T;

typedef struct {
    FIELD  COLOR_s_gain_y256_hue_8                    :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_s_gain_y256_hue_9                    :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_s_gain_y256_hue_10                   :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_s_gain_y256_hue_11                   :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_S_GAIN_BY_Y256_2_T; //	/* 0x150277E0 */

typedef union {
    typedef ISP_COLOR_S_GAIN_BY_Y256_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_S_GAIN_BY_Y256_2_T;

typedef struct {
    FIELD  COLOR_s_gain_y256_hue_12                   :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_s_gain_y256_hue_13                   :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_s_gain_y256_hue_14                   :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_s_gain_y256_hue_15                   :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_S_GAIN_BY_Y256_3_T; //	/* 0x150277E4 */

typedef union {
    typedef ISP_COLOR_S_GAIN_BY_Y256_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_S_GAIN_BY_Y256_3_T;

typedef struct {
    FIELD  COLOR_s_gain_y256_hue_16                   :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_s_gain_y256_hue_17                   :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_s_gain_y256_hue_18                   :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_s_gain_y256_hue_19                   :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_S_GAIN_BY_Y256_4_T; //	/* 0x150277E8 */

typedef union {
    typedef ISP_COLOR_S_GAIN_BY_Y256_4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_S_GAIN_BY_Y256_4_T;

typedef struct {
    FIELD  COLOR_lsp_lire_yslope                      :  7;		/*  0.. 6, 0x0000007F */
    FIELD  COLOR_lsp_lire_sslope                      :  7;		/*  7..13, 0x00003F80 */
    FIELD  COLOR_lsp_lire_yth                         :  8;		/* 14..21, 0x003FC000 */
    FIELD  COLOR_lsp_lire_sth                         :  7;		/* 22..28, 0x1FC00000 */
    FIELD  rsv_29                                     :  3;		/* 29..31, 0xE0000000 */
} ISP_COLOR_LSP_1_T; //	/* 0x150277EC */

typedef union {
    typedef ISP_COLOR_LSP_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_LSP_1_T;

typedef struct {
    FIELD  COLOR_lsp_lire_gain                        :  7;		/*  0.. 6, 0x0000007F */
    FIELD  rsv_7                                      :  1;		/*  7.. 7, 0x00000080 */
    FIELD  COLOR_lsp_lsat_gain                        :  7;		/*  8..14, 0x00007F00 */
    FIELD  rsv_15                                     :  1;		/* 15..15, 0x00008000 */
    FIELD  COLOR_lsp_lsat_sth                         :  7;		/* 16..22, 0x007F0000 */
    FIELD  COLOR_lsp_lsat_sslope                      :  7;		/* 23..29, 0x3F800000 */
    FIELD  rsv_30                                     :  2;		/* 30..31, 0xC0000000 */
} ISP_COLOR_LSP_2_T; //	/* 0x150277F0 */

typedef union {
    typedef ISP_COLOR_LSP_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_LSP_2_T;

typedef struct {
    FIELD  COLOR_cm_bypass                            :  1;		/*  0.. 0, 0x00000001 */
    FIELD  COLOR_cm_w1_en                             :  1;		/*  1.. 1, 0x00000002 */
    FIELD  COLOR_cm_w1_wgt_en                         :  1;		/*  2.. 2, 0x00000004 */
    FIELD  COLOR_cm_w1_ink_en                         :  1;		/*  3.. 3, 0x00000008 */
    FIELD  COLOR_cm_w2_en                             :  1;		/*  4.. 4, 0x00000010 */
    FIELD  COLOR_cm_w2_wgt_en                         :  1;		/*  5.. 5, 0x00000020 */
    FIELD  COLOR_cm_w2_ink_en                         :  1;		/*  6.. 6, 0x00000040 */
    FIELD  COLOR_cm_w3_en                             :  1;		/*  7.. 7, 0x00000080 */
    FIELD  COLOR_cm_w3_wgt_en                         :  1;		/*  8.. 8, 0x00000100 */
    FIELD  COLOR_cm_w3_ink_en                         :  1;		/*  9.. 9, 0x00000200 */
    FIELD  COLOR_cm_ink_delta_mode                    :  1;		/* 10..10, 0x00000400 */
    FIELD  rsv_11                                     :  1;		/* 11..11, 0x00000800 */
    FIELD  COLOR_cm_ink_ch                            :  2;		/* 12..13, 0x00003000 */
    FIELD  rsv_14                                     : 18;		/* 14..31, 0xFFFFC000 */
} ISP_COLOR_CM_CONTROL_T; //	/* 0x150277F4 */

typedef union {
    typedef ISP_COLOR_CM_CONTROL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_CONTROL_T;

typedef struct {
    FIELD  COLOR_cm_h_w1_l                            : 10;		/*  0.. 9, 0x000003FF */
    FIELD  COLOR_cm_h_w1_u                            : 10;		/* 10..19, 0x000FFC00 */
    FIELD  COLOR_cm_h_w1_point0                       : 10;		/* 20..29, 0x3FF00000 */
    FIELD  rsv_30                                     :  2;		/* 30..31, 0xC0000000 */
} ISP_COLOR_CM_W1_HUE_0_T; //	/* 0x150277F8 */

typedef union {
    typedef ISP_COLOR_CM_W1_HUE_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W1_HUE_0_T;

typedef struct {
    FIELD  COLOR_cm_h_w1_point1                       : 10;		/*  0.. 9, 0x000003FF */
    FIELD  COLOR_cm_h_w1_point2                       : 10;		/* 10..19, 0x000FFC00 */
    FIELD  COLOR_cm_h_w1_point3                       : 10;		/* 20..29, 0x3FF00000 */
    FIELD  rsv_30                                     :  2;		/* 30..31, 0xC0000000 */
} ISP_COLOR_CM_W1_HUE_1_T; //	/* 0x150277FC */

typedef union {
    typedef ISP_COLOR_CM_W1_HUE_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W1_HUE_1_T;

typedef struct {
    FIELD  COLOR_cm_h_w1_point4                       : 10;		/*  0.. 9, 0x000003FF */
    FIELD  COLOR_cm_h_w1_slope0                       :  8;		/* 10..17, 0x0003FC00 */
    FIELD  rsv_18                                     :  2;		/* 18..19, 0x000C0000 */
    FIELD  COLOR_cm_h_w1_slope1                       :  8;		/* 20..27, 0x0FF00000 */
    FIELD  rsv_28                                     :  4;		/* 28..31, 0xF0000000 */
} ISP_COLOR_CM_W1_HUE_2_T; //	/* 0x15027800 */

typedef union {
    typedef ISP_COLOR_CM_W1_HUE_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W1_HUE_2_T;

typedef struct {
    FIELD  COLOR_cm_h_w1_slope2                       :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_cm_h_w1_slope3                       :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_cm_h_w1_slope4                       :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_cm_h_w1_slope5                       :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_CM_W1_HUE_3_T; //	/* 0x15027804 */

typedef union {
    typedef ISP_COLOR_CM_W1_HUE_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W1_HUE_3_T;

typedef struct {
    FIELD  COLOR_cm_h_w1_wgt_lslope                   : 10;		/*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                     :  6;		/* 10..15, 0x0000FC00 */
    FIELD  COLOR_cm_h_w1_wgt_uslope                   : 10;		/* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                     :  6;		/* 26..31, 0xFC000000 */
} ISP_COLOR_CM_W1_HUE_4_T; //	/* 0x15027808 */

typedef union {
    typedef ISP_COLOR_CM_W1_HUE_4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W1_HUE_4_T;

typedef struct {
    FIELD  COLOR_cm_y_w1_l                            : 10;		/*  0.. 9, 0x000003FF */
    FIELD  COLOR_cm_y_w1_u                            : 10;		/* 10..19, 0x000FFC00 */
    FIELD  COLOR_cm_y_w1_point0                       : 10;		/* 20..29, 0x3FF00000 */
    FIELD  rsv_30                                     :  2;		/* 30..31, 0xC0000000 */
} ISP_COLOR_CM_W1_LUMA_0_T; //	/* 0x1502780C */

typedef union {
    typedef ISP_COLOR_CM_W1_LUMA_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W1_LUMA_0_T;

typedef struct {
    FIELD  COLOR_cm_y_w1_point1                       : 10;		/*  0.. 9, 0x000003FF */
    FIELD  COLOR_cm_y_w1_point2                       : 10;		/* 10..19, 0x000FFC00 */
    FIELD  COLOR_cm_y_w1_point3                       : 10;		/* 20..29, 0x3FF00000 */
    FIELD  rsv_30                                     :  2;		/* 30..31, 0xC0000000 */
} ISP_COLOR_CM_W1_LUMA_1_T; //	/* 0x15027810 */

typedef union {
    typedef ISP_COLOR_CM_W1_LUMA_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W1_LUMA_1_T;

typedef struct {
    FIELD  COLOR_cm_y_w1_point4                       : 10;		/*  0.. 9, 0x000003FF */
    FIELD  COLOR_cm_y_w1_slope0                       :  8;		/* 10..17, 0x0003FC00 */
    FIELD  rsv_18                                     :  2;		/* 18..19, 0x000C0000 */
    FIELD  COLOR_cm_y_w1_slope1                       :  8;		/* 20..27, 0x0FF00000 */
    FIELD  rsv_28                                     :  4;		/* 28..31, 0xF0000000 */
} ISP_COLOR_CM_W1_LUMA_2_T; //	/* 0x15027814 */

typedef union {
    typedef ISP_COLOR_CM_W1_LUMA_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W1_LUMA_2_T;

typedef struct {
    FIELD  COLOR_cm_y_w1_slope2                       :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_cm_y_w1_slope3                       :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_cm_y_w1_slope4                       :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_cm_y_w1_slope5                       :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_CM_W1_LUMA_3_T; //	/* 0x15027818 */

typedef union {
    typedef ISP_COLOR_CM_W1_LUMA_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W1_LUMA_3_T;

typedef struct {
    FIELD  COLOR_cm_y_w1_wgt_lslope                   : 10;		/*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                     :  6;		/* 10..15, 0x0000FC00 */
    FIELD  COLOR_cm_y_w1_wgt_uslope                   : 10;		/* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                     :  6;		/* 26..31, 0xFC000000 */
} ISP_COLOR_CM_W1_LUMA_4_T; //	/* 0x1502781C */

typedef union {
    typedef ISP_COLOR_CM_W1_LUMA_4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W1_LUMA_4_T;

typedef struct {
    FIELD  COLOR_cm_s_w1_l                            : 10;		/*  0.. 9, 0x000003FF */
    FIELD  COLOR_cm_s_w1_u                            : 10;		/* 10..19, 0x000FFC00 */
    FIELD  COLOR_cm_s_w1_point0                       : 10;		/* 20..29, 0x3FF00000 */
    FIELD  rsv_30                                     :  2;		/* 30..31, 0xC0000000 */
} ISP_COLOR_CM_W1_SAT_0_T; //	/* 0x15027820 */

typedef union {
    typedef ISP_COLOR_CM_W1_SAT_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W1_SAT_0_T;

typedef struct {
    FIELD  COLOR_cm_s_w1_point1                       : 10;		/*  0.. 9, 0x000003FF */
    FIELD  COLOR_cm_s_w1_point2                       : 10;		/* 10..19, 0x000FFC00 */
    FIELD  COLOR_cm_s_w1_point3                       : 10;		/* 20..29, 0x3FF00000 */
    FIELD  rsv_30                                     :  2;		/* 30..31, 0xC0000000 */
} ISP_COLOR_CM_W1_SAT_1_T; //	/* 0x15027824 */

typedef union {
    typedef ISP_COLOR_CM_W1_SAT_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W1_SAT_1_T;

typedef struct {
    FIELD  COLOR_cm_s_w1_point4                       : 10;		/*  0.. 9, 0x000003FF */
    FIELD  COLOR_cm_s_w1_slope0                       :  8;		/* 10..17, 0x0003FC00 */
    FIELD  rsv_18                                     :  2;		/* 18..19, 0x000C0000 */
    FIELD  COLOR_cm_s_w1_slope1                       :  8;		/* 20..27, 0x0FF00000 */
    FIELD  rsv_28                                     :  4;		/* 28..31, 0xF0000000 */
} ISP_COLOR_CM_W1_SAT_2_T; //	/* 0x15027828 */

typedef union {
    typedef ISP_COLOR_CM_W1_SAT_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W1_SAT_2_T;

typedef struct {
    FIELD  COLOR_cm_s_w1_slope2                       :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_cm_s_w1_slope3                       :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_cm_s_w1_slope4                       :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_cm_s_w1_slope5                       :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_CM_W1_SAT_3_T; //	/* 0x1502782C */

typedef union {
    typedef ISP_COLOR_CM_W1_SAT_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W1_SAT_3_T;

typedef struct {
    FIELD  COLOR_cm_s_w1_wgt_lslope                   : 10;		/*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                     :  6;		/* 10..15, 0x0000FC00 */
    FIELD  COLOR_cm_s_w1_wgt_uslope                   : 10;		/* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                     :  6;		/* 26..31, 0xFC000000 */
} ISP_COLOR_CM_W1_SAT_4_T; //	/* 0x15027830 */

typedef union {
    typedef ISP_COLOR_CM_W1_SAT_4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W1_SAT_4_T;

typedef struct {
    FIELD  COLOR_cm_h_w2_l                            : 10;		/*  0.. 9, 0x000003FF */
    FIELD  COLOR_cm_h_w2_u                            : 10;		/* 10..19, 0x000FFC00 */
    FIELD  COLOR_cm_h_w2_point0                       : 10;		/* 20..29, 0x3FF00000 */
    FIELD  rsv_30                                     :  2;		/* 30..31, 0xC0000000 */
} ISP_COLOR_CM_W2_HUE_0_T; //	/* 0x15027834 */

typedef union {
    typedef ISP_COLOR_CM_W2_HUE_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W2_HUE_0_T;

typedef struct {
    FIELD  COLOR_cm_h_w2_point1                       : 10;		/*  0.. 9, 0x000003FF */
    FIELD  COLOR_cm_h_w2_point2                       : 10;		/* 10..19, 0x000FFC00 */
    FIELD  COLOR_cm_h_w2_point3                       : 10;		/* 20..29, 0x3FF00000 */
    FIELD  rsv_30                                     :  2;		/* 30..31, 0xC0000000 */
} ISP_COLOR_CM_W2_HUE_1_T; //	/* 0x15027838 */

typedef union {
    typedef ISP_COLOR_CM_W2_HUE_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W2_HUE_1_T;

typedef struct {
    FIELD  COLOR_cm_h_w2_point4                       : 10;		/*  0.. 9, 0x000003FF */
    FIELD  COLOR_cm_h_w2_slope0                       :  8;		/* 10..17, 0x0003FC00 */
    FIELD  rsv_18                                     :  2;		/* 18..19, 0x000C0000 */
    FIELD  COLOR_cm_h_w2_slope1                       :  8;		/* 20..27, 0x0FF00000 */
    FIELD  rsv_28                                     :  4;		/* 28..31, 0xF0000000 */
} ISP_COLOR_CM_W2_HUE_2_T; //	/* 0x1502783C */

typedef union {
    typedef ISP_COLOR_CM_W2_HUE_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W2_HUE_2_T;

typedef struct {
    FIELD  COLOR_cm_h_w2_slope2                       :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_cm_h_w2_slope3                       :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_cm_h_w2_slope4                       :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_cm_h_w2_slope5                       :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_CM_W2_HUE_3_T; //	/* 0x15027840 */

typedef union {
    typedef ISP_COLOR_CM_W2_HUE_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W2_HUE_3_T;

typedef struct {
    FIELD  COLOR_cm_h_w2_wgt_lslope                   : 10;		/*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                     :  6;		/* 10..15, 0x0000FC00 */
    FIELD  COLOR_cm_h_w2_wgt_uslope                   : 10;		/* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                     :  6;		/* 26..31, 0xFC000000 */
} ISP_COLOR_CM_W2_HUE_4_T; //	/* 0x15027844 */

typedef union {
    typedef ISP_COLOR_CM_W2_HUE_4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W2_HUE_4_T;

typedef struct {
    FIELD  COLOR_cm_y_w2_l                            : 10;		/*  0.. 9, 0x000003FF */
    FIELD  COLOR_cm_y_w2_u                            : 10;		/* 10..19, 0x000FFC00 */
    FIELD  COLOR_cm_y_w2_point0                       : 10;		/* 20..29, 0x3FF00000 */
    FIELD  rsv_30                                     :  2;		/* 30..31, 0xC0000000 */
} ISP_COLOR_CM_W2_LUMA_0_T; //	/* 0x15027848 */

typedef union {
    typedef ISP_COLOR_CM_W2_LUMA_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W2_LUMA_0_T;

typedef struct {
    FIELD  COLOR_cm_y_w2_point1                       : 10;		/*  0.. 9, 0x000003FF */
    FIELD  COLOR_cm_y_w2_point2                       : 10;		/* 10..19, 0x000FFC00 */
    FIELD  COLOR_cm_y_w2_point3                       : 10;		/* 20..29, 0x3FF00000 */
    FIELD  rsv_30                                     :  2;		/* 30..31, 0xC0000000 */
} ISP_COLOR_CM_W2_LUMA_1_T; //	/* 0x1502784C */

typedef union {
    typedef ISP_COLOR_CM_W2_LUMA_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W2_LUMA_1_T;

typedef struct {
    FIELD  COLOR_cm_y_w2_point4                       : 10;		/*  0.. 9, 0x000003FF */
    FIELD  COLOR_cm_y_w2_slope0                       :  8;		/* 10..17, 0x0003FC00 */
    FIELD  rsv_18                                     :  2;		/* 18..19, 0x000C0000 */
    FIELD  COLOR_cm_y_w2_slope1                       :  8;		/* 20..27, 0x0FF00000 */
    FIELD  rsv_28                                     :  4;		/* 28..31, 0xF0000000 */
} ISP_COLOR_CM_W2_LUMA_2_T; //	/* 0x15027850 */

typedef union {
    typedef ISP_COLOR_CM_W2_LUMA_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W2_LUMA_2_T;

typedef struct {
    FIELD  COLOR_cm_y_w2_slope2                       :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_cm_y_w2_slope3                       :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_cm_y_w2_slope4                       :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_cm_y_w2_slope5                       :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_CM_W2_LUMA_3_T; //	/* 0x15027854 */

typedef union {
    typedef ISP_COLOR_CM_W2_LUMA_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W2_LUMA_3_T;

typedef struct {
    FIELD  COLOR_cm_y_w2_wgt_lslope                   : 10;		/*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                     :  6;		/* 10..15, 0x0000FC00 */
    FIELD  COLOR_cm_y_w2_wgt_uslope                   : 10;		/* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                     :  6;		/* 26..31, 0xFC000000 */
} ISP_COLOR_CM_W2_LUMA_4_T; //	/* 0x15027858 */

typedef union {
    typedef ISP_COLOR_CM_W2_LUMA_4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W2_LUMA_4_T;

typedef struct {
    FIELD  COLOR_cm_s_w2_l                            : 10;		/*  0.. 9, 0x000003FF */
    FIELD  COLOR_cm_s_w2_u                            : 10;		/* 10..19, 0x000FFC00 */
    FIELD  COLOR_cm_s_w2_point0                       : 10;		/* 20..29, 0x3FF00000 */
    FIELD  rsv_30                                     :  2;		/* 30..31, 0xC0000000 */
} ISP_COLOR_CM_W2_SAT_0_T; //	/* 0x1502785C */

typedef union {
    typedef ISP_COLOR_CM_W2_SAT_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W2_SAT_0_T;

typedef struct {
    FIELD  COLOR_cm_s_w2_point1                       : 10;		/*  0.. 9, 0x000003FF */
    FIELD  COLOR_cm_s_w2_point2                       : 10;		/* 10..19, 0x000FFC00 */
    FIELD  COLOR_cm_s_w2_point3                       : 10;		/* 20..29, 0x3FF00000 */
    FIELD  rsv_30                                     :  2;		/* 30..31, 0xC0000000 */
} ISP_COLOR_CM_W2_SAT_1_T; //	/* 0x15027860 */

typedef union {
    typedef ISP_COLOR_CM_W2_SAT_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W2_SAT_1_T;

typedef struct {
    FIELD  COLOR_cm_s_w2_point4                       : 10;		/*  0.. 9, 0x000003FF */
    FIELD  COLOR_cm_s_w2_slope0                       :  8;		/* 10..17, 0x0003FC00 */
    FIELD  rsv_18                                     :  2;		/* 18..19, 0x000C0000 */
    FIELD  COLOR_cm_s_w2_slope1                       :  8;		/* 20..27, 0x0FF00000 */
    FIELD  rsv_28                                     :  4;		/* 28..31, 0xF0000000 */
} ISP_COLOR_CM_W2_SAT_2_T; //	/* 0x15027864 */

typedef union {
    typedef ISP_COLOR_CM_W2_SAT_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W2_SAT_2_T;

typedef struct {
    FIELD  COLOR_cm_s_w2_slope2                       :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_cm_s_w2_slope3                       :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_cm_s_w2_slope4                       :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_cm_s_w2_slope5                       :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_CM_W2_SAT_3_T; //	/* 0x15027868 */

typedef union {
    typedef ISP_COLOR_CM_W2_SAT_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W2_SAT_3_T;

typedef struct {
    FIELD  COLOR_cm_s_w2_wgt_lslope                   : 10;		/*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                     :  6;		/* 10..15, 0x0000FC00 */
    FIELD  COLOR_cm_s_w2_wgt_uslope                   : 10;		/* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                     :  6;		/* 26..31, 0xFC000000 */
} ISP_COLOR_CM_W2_SAT_4_T; //	/* 0x1502786C */

typedef union {
    typedef ISP_COLOR_CM_W2_SAT_4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W2_SAT_4_T;

typedef struct {
    FIELD  COLOR_cm_h_w3_l                            : 10;		/*  0.. 9, 0x000003FF */
    FIELD  COLOR_cm_h_w3_u                            : 10;		/* 10..19, 0x000FFC00 */
    FIELD  COLOR_cm_h_w3_point0                       : 10;		/* 20..29, 0x3FF00000 */
    FIELD  rsv_30                                     :  2;		/* 30..31, 0xC0000000 */
} ISP_COLOR_CM_W3_HUE_0_T; //	/* 0x15027870 */

typedef union {
    typedef ISP_COLOR_CM_W3_HUE_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W3_HUE_0_T;

typedef struct {
    FIELD  COLOR_cm_h_w3_point1                       : 10;		/*  0.. 9, 0x000003FF */
    FIELD  COLOR_cm_h_w3_point2                       : 10;		/* 10..19, 0x000FFC00 */
    FIELD  COLOR_cm_h_w3_point3                       : 10;		/* 20..29, 0x3FF00000 */
    FIELD  rsv_30                                     :  2;		/* 30..31, 0xC0000000 */
} ISP_COLOR_CM_W3_HUE_1_T; //	/* 0x15027874 */

typedef union {
    typedef ISP_COLOR_CM_W3_HUE_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W3_HUE_1_T;

typedef struct {
    FIELD  COLOR_cm_h_w3_point4                       : 10;		/*  0.. 9, 0x000003FF */
    FIELD  COLOR_cm_h_w3_slope0                       :  8;		/* 10..17, 0x0003FC00 */
    FIELD  rsv_18                                     :  2;		/* 18..19, 0x000C0000 */
    FIELD  COLOR_cm_h_w3_slope1                       :  8;		/* 20..27, 0x0FF00000 */
    FIELD  rsv_28                                     :  4;		/* 28..31, 0xF0000000 */
} ISP_COLOR_CM_W3_HUE_2_T; //	/* 0x15027878 */

typedef union {
    typedef ISP_COLOR_CM_W3_HUE_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W3_HUE_2_T;

typedef struct {
    FIELD  COLOR_cm_h_w3_slope2                       :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_cm_h_w3_slope3                       :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_cm_h_w3_slope4                       :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_cm_h_w3_slope5                       :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_CM_W3_HUE_3_T; //	/* 0x1502787C */

typedef union {
    typedef ISP_COLOR_CM_W3_HUE_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W3_HUE_3_T;

typedef struct {
    FIELD  COLOR_cm_h_w3_wgt_lslope                   : 10;		/*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                     :  6;		/* 10..15, 0x0000FC00 */
    FIELD  COLOR_cm_h_w3_wgt_uslope                   : 10;		/* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                     :  6;		/* 26..31, 0xFC000000 */
} ISP_COLOR_CM_W3_HUE_4_T; //	/* 0x15027880 */

typedef union {
    typedef ISP_COLOR_CM_W3_HUE_4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W3_HUE_4_T;

typedef struct {
    FIELD  COLOR_cm_y_w3_l                            : 10;		/*  0.. 9, 0x000003FF */
    FIELD  COLOR_cm_y_w3_u                            : 10;		/* 10..19, 0x000FFC00 */
    FIELD  COLOR_cm_y_w3_point0                       : 10;		/* 20..29, 0x3FF00000 */
    FIELD  rsv_30                                     :  2;		/* 30..31, 0xC0000000 */
} ISP_COLOR_CM_W3_LUMA_0_T; //	/* 0x15027884 */

typedef union {
    typedef ISP_COLOR_CM_W3_LUMA_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W3_LUMA_0_T;

typedef struct {
    FIELD  COLOR_cm_y_w3_point1                       : 10;		/*  0.. 9, 0x000003FF */
    FIELD  COLOR_cm_y_w3_point2                       : 10;		/* 10..19, 0x000FFC00 */
    FIELD  COLOR_cm_y_w3_point3                       : 10;		/* 20..29, 0x3FF00000 */
    FIELD  rsv_30                                     :  2;		/* 30..31, 0xC0000000 */
} ISP_COLOR_CM_W3_LUMA_1_T; //	/* 0x15027888 */

typedef union {
    typedef ISP_COLOR_CM_W3_LUMA_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W3_LUMA_1_T;

typedef struct {
    FIELD  COLOR_cm_y_w3_point4                       : 10;		/*  0.. 9, 0x000003FF */
    FIELD  COLOR_cm_y_w3_slope0                       :  8;		/* 10..17, 0x0003FC00 */
    FIELD  rsv_18                                     :  2;		/* 18..19, 0x000C0000 */
    FIELD  COLOR_cm_y_w3_slope1                       :  8;		/* 20..27, 0x0FF00000 */
    FIELD  rsv_28                                     :  4;		/* 28..31, 0xF0000000 */
} ISP_COLOR_CM_W3_LUMA_2_T; //	/* 0x1502788C */

typedef union {
    typedef ISP_COLOR_CM_W3_LUMA_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W3_LUMA_2_T;

typedef struct {
    FIELD  COLOR_cm_y_w3_slope2                       :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_cm_y_w3_slope3                       :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_cm_y_w3_slope4                       :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_cm_y_w3_slope5                       :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_CM_W3_LUMA_3_T; //	/* 0x15027890 */

typedef union {
    typedef ISP_COLOR_CM_W3_LUMA_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W3_LUMA_3_T;

typedef struct {
    FIELD  COLOR_cm_y_w3_wgt_lslope                   : 10;		/*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                     :  6;		/* 10..15, 0x0000FC00 */
    FIELD  COLOR_cm_y_w3_wgt_uslope                   : 10;		/* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                     :  6;		/* 26..31, 0xFC000000 */
} ISP_COLOR_CM_W3_LUMA_4_T; //	/* 0x15027894 */

typedef union {
    typedef ISP_COLOR_CM_W3_LUMA_4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W3_LUMA_4_T;

typedef struct {
    FIELD  COLOR_cm_s_w3_l                            : 10;		/*  0.. 9, 0x000003FF */
    FIELD  COLOR_cm_s_w3_u                            : 10;		/* 10..19, 0x000FFC00 */
    FIELD  COLOR_cm_s_w3_point0                       : 10;		/* 20..29, 0x3FF00000 */
    FIELD  rsv_30                                     :  2;		/* 30..31, 0xC0000000 */
} ISP_COLOR_CM_W3_SAT_0_T; //	/* 0x15027898 */

typedef union {
    typedef ISP_COLOR_CM_W3_SAT_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W3_SAT_0_T;

typedef struct {
    FIELD  COLOR_cm_s_w3_point1                       : 10;		/*  0.. 9, 0x000003FF */
    FIELD  COLOR_cm_s_w3_point2                       : 10;		/* 10..19, 0x000FFC00 */
    FIELD  COLOR_cm_s_w3_point3                       : 10;		/* 20..29, 0x3FF00000 */
    FIELD  rsv_30                                     :  2;		/* 30..31, 0xC0000000 */
} ISP_COLOR_CM_W3_SAT_1_T; //	/* 0x1502789C */

typedef union {
    typedef ISP_COLOR_CM_W3_SAT_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W3_SAT_1_T;

typedef struct {
    FIELD  COLOR_cm_s_w3_point4                       : 10;		/*  0.. 9, 0x000003FF */
    FIELD  COLOR_cm_s_w3_slope0                       :  8;		/* 10..17, 0x0003FC00 */
    FIELD  rsv_18                                     :  2;		/* 18..19, 0x000C0000 */
    FIELD  COLOR_cm_s_w3_slope1                       :  8;		/* 20..27, 0x0FF00000 */
    FIELD  rsv_28                                     :  4;		/* 28..31, 0xF0000000 */
} ISP_COLOR_CM_W3_SAT_2_T; //	/* 0x150278A0 */

typedef union {
    typedef ISP_COLOR_CM_W3_SAT_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W3_SAT_2_T;

typedef struct {
    FIELD  COLOR_cm_s_w3_slope2                       :  8;		/*  0.. 7, 0x000000FF */
    FIELD  COLOR_cm_s_w3_slope3                       :  8;		/*  8..15, 0x0000FF00 */
    FIELD  COLOR_cm_s_w3_slope4                       :  8;		/* 16..23, 0x00FF0000 */
    FIELD  COLOR_cm_s_w3_slope5                       :  8;		/* 24..31, 0xFF000000 */
} ISP_COLOR_CM_W3_SAT_3_T; //	/* 0x150278A4 */

typedef union {
    typedef ISP_COLOR_CM_W3_SAT_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W3_SAT_3_T;

typedef struct {
    FIELD  COLOR_cm_s_w3_wgt_lslope                   : 10;		/*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                     :  6;		/* 10..15, 0x0000FC00 */
    FIELD  COLOR_cm_s_w3_wgt_uslope                   : 10;		/* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                     :  6;		/* 26..31, 0xFC000000 */
} ISP_COLOR_CM_W3_SAT_4_T; //	/* 0x150278A8 */

typedef union {
    typedef ISP_COLOR_CM_W3_SAT_4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_CM_W3_SAT_4_T;

typedef struct {
    FIELD  COLOR_tile_edge                            :  4;		/*  0.. 3, 0x0000000F */
    FIELD  rsv_4                                      :  4;		/*  4.. 7, 0x000000F0 */
    FIELD  COLOR_crop_h                               :  4;		/*  8..11, 0x00000F00 */
    FIELD  rsv_12                                     :  4;		/* 12..15, 0x0000F000 */
    FIELD  COLOR_crop_v                               :  4;		/* 16..19, 0x000F0000 */
    FIELD  rsv_20                                     : 12;		/* 20..31, 0xFFF00000 */
} ISP_COLOR_TILE_T; //	/* 0x150278AC */

typedef union {
    typedef ISP_COLOR_TILE_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_COLOR_TILE_T;

typedef union {
    enum { COUNT = 117 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_COLOR_CFG_MAIN_T  			        cfg_main;
        //ISP_NVRAM_COLOR_PXL_CNT_MAIN_T  			    pxl_cnt_main;
        //ISP_NVRAM_COLOR_LINE_CNT_MAIN_T  			    line_cnt_main;
        //ISP_NVRAM_COLOR_WIN_X_MAIN_T  			    win_x_main;
        //ISP_NVRAM_COLOR_WIN_Y_MAIN_T  			    win_y_main;
        //ISP_NVRAM_COLOR_TIMING_DETECTION_0_T  		timing_detection_0;
        //ISP_NVRAM_COLOR_TIMING_DETECTION_1_T  		timing_detection_1;
        //ISP_NVRAM_COLOR_DBG_CFG_MAIN_T  			    dbg_cfg_main;
        ISP_NVRAM_COLOR_C_BOOST_MAIN_T  			    c_boost_main;
        ISP_NVRAM_COLOR_C_BOOST_MAIN_2_T  			    c_boost_main_2;
        ISP_NVRAM_COLOR_LUMA_ADJ_T  			        luma_adj;
        ISP_NVRAM_COLOR_G_PIC_ADJ_MAIN_1_T  			g_pic_adj_main_1;
        ISP_NVRAM_COLOR_G_PIC_ADJ_MAIN_2_T  			g_pic_adj_main_2;
        //ISP_NVRAM_COLOR_POS_MAIN_T  			        pos_main;
        //ISP_NVRAM_COLOR_INK_DATA_MAIN_T  			    ink_data_main;
        //ISP_NVRAM_COLOR_INK_DATA_MAIN_CR_T  			ink_data_main_cr;
        //ISP_NVRAM_COLOR_CAP_IN_DATA_MAIN_T  			cap_in_data_main;
        //ISP_NVRAM_COLOR_CAP_IN_DATA_MAIN_CR_T  		cap_in_data_main_cr;
        //ISP_NVRAM_COLOR_CAP_OUT_DATA_MAIN_T  			cap_out_data_main;
        //ISP_NVRAM_COLOR_CAP_OUT_DATA_MAIN_CR_T  		cap_out_data_main_cr;
        ISP_NVRAM_COLOR_Y_SLOPE_1_0_MAIN_T  			y_slope_1_0_main;
        ISP_NVRAM_COLOR_Y_SLOPE_3_2_MAIN_T  			y_slope_3_2_main;
        ISP_NVRAM_COLOR_Y_SLOPE_5_4_MAIN_T  			y_slope_5_4_main;
        ISP_NVRAM_COLOR_Y_SLOPE_7_6_MAIN_T  			y_slope_7_6_main;
        ISP_NVRAM_COLOR_Y_SLOPE_9_8_MAIN_T  			y_slope_9_8_main;
        ISP_NVRAM_COLOR_Y_SLOPE_11_10_MAIN_T  			y_slope_11_10_main;
        ISP_NVRAM_COLOR_Y_SLOPE_13_12_MAIN_T  			y_slope_13_12_main;
        ISP_NVRAM_COLOR_Y_SLOPE_15_14_MAIN_T  			y_slope_15_14_main;
        ISP_NVRAM_COLOR_LOCAL_HUE_CD_0_T  			    local_hue_cd_0;
        ISP_NVRAM_COLOR_LOCAL_HUE_CD_1_T  			    local_hue_cd_1;
        ISP_NVRAM_COLOR_LOCAL_HUE_CD_2_T  			    local_hue_cd_2;
        ISP_NVRAM_COLOR_LOCAL_HUE_CD_3_T  			    local_hue_cd_3;
        ISP_NVRAM_COLOR_LOCAL_HUE_CD_4_T  			    local_hue_cd_4;
        //ISP_NVRAM_COLOR_TWO_D_WINDOW_1_T  		    two_d_window_1;
        //ISP_NVRAM_COLOR_TWO_D_W1_RESULT_T  		    two_d_w1_result;
        //ISP_NVRAM_COLOR_SAT_HIST_X_CFG_MAIN_T  		sat_hist_x_cfg_main;
        //ISP_NVRAM_COLOR_SAT_HIST_Y_CFG_MAIN_T  		sat_hist_y_cfg_main;
        //ISP_NVRAM_COLOR_BWS_2_T  			            bws_2;
        //ISP_NVRAM_COLOR_CRC_0_T  		            	crc_0;
        //ISP_NVRAM_COLOR_CRC_1_T             			crc_1;
        //ISP_NVRAM_COLOR_CRC_2_T             			crc_2;
        //ISP_NVRAM_COLOR_CRC_3_T             			crc_3;
        //ISP_NVRAM_COLOR_CRC_4_T  	            		crc_4;
        ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN1_0_T  			partial_sat_gain1_0;
        ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN1_1_T  			partial_sat_gain1_1;
        ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN1_2_T  			partial_sat_gain1_2;
        ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN1_3_T  			partial_sat_gain1_3;
        ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN1_4_T  			partial_sat_gain1_4;
        ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN2_0_T  			partial_sat_gain2_0;
        ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN2_1_T  			partial_sat_gain2_1;
        ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN2_2_T  			partial_sat_gain2_2;
        ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN2_3_T  			partial_sat_gain2_3;
        ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN2_4_T  			partial_sat_gain2_4;
        ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN3_0_T  			partial_sat_gain3_0;
        ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN3_1_T  			partial_sat_gain3_1;
        ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN3_2_T  			partial_sat_gain3_2;
        ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN3_3_T  			partial_sat_gain3_3;
        ISP_NVRAM_COLOR_PARTIAL_SAT_GAIN3_4_T  			partial_sat_gain3_4;
        ISP_NVRAM_COLOR_PARTIAL_SAT_POINT1_0_T  		partial_sat_point1_0;
        ISP_NVRAM_COLOR_PARTIAL_SAT_POINT1_1_T  		partial_sat_point1_1;
        ISP_NVRAM_COLOR_PARTIAL_SAT_POINT1_2_T  		partial_sat_point1_2;
        ISP_NVRAM_COLOR_PARTIAL_SAT_POINT1_3_T  		partial_sat_point1_3;
        ISP_NVRAM_COLOR_PARTIAL_SAT_POINT1_4_T  		partial_sat_point1_4;
        ISP_NVRAM_COLOR_PARTIAL_SAT_POINT2_0_T  		partial_sat_point2_0;
        ISP_NVRAM_COLOR_PARTIAL_SAT_POINT2_1_T  		partial_sat_point2_1;
        ISP_NVRAM_COLOR_PARTIAL_SAT_POINT2_2_T  		partial_sat_point2_2;
        ISP_NVRAM_COLOR_PARTIAL_SAT_POINT2_3_T  		partial_sat_point2_3;
        ISP_NVRAM_COLOR_PARTIAL_SAT_POINT2_4_T  		partial_sat_point2_4;
        //ISP_NVRAM_COLOR_START_T  			            start;
        //ISP_NVRAM_COLOR_INTEN_T  						inten;
        //ISP_NVRAM_COLOR_INTSTA_T  					intsta;
        //ISP_NVRAM_COLOR_OUT_SEL_T  					out_sel;
        //ISP_NVRAM_COLOR_FRAME_DONE_DEL_T  			frame_done_del;
        //ISP_NVRAM_COLOR_CRC_T  						crc;
        //ISP_NVRAM_COLOR_SW_SCRATCH_T  				sw_scratch;
        //ISP_NVRAM_COLOR_CK_ON_T  						ck_on;
        //ISP_NVRAM_COLOR_INTERNAL_IP_WIDTH_T  			internal_ip_width;
        //ISP_NVRAM_COLOR_INTERNAL_IP_HEIGHT_T  		internal_ip_height;
        //ISP_NVRAM_COLOR_CM1_EN_T  					cm1_en;
        //ISP_NVRAM_COLOR_CM2_EN_T  					cm2_en;
        //ISP_NVRAM_COLOR_SHADOW_CTRL_T  				shadow_ctrl;
        //ISP_NVRAM_COLOR_R0_CRC_T  					r0_crc;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y0_0_T  				s_gain_by_y0_0;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y0_1_T  				s_gain_by_y0_1;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y0_2_T  				s_gain_by_y0_2;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y0_3_T  				s_gain_by_y0_3;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y0_4_T  				s_gain_by_y0_4;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y64_0_T  				s_gain_by_y64_0;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y64_1_T  				s_gain_by_y64_1;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y64_2_T  				s_gain_by_y64_2;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y64_3_T  				s_gain_by_y64_3;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y64_4_T  				s_gain_by_y64_4;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y128_0_T  			s_gain_by_y128_0;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y128_1_T  			s_gain_by_y128_1;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y128_2_T  			s_gain_by_y128_2;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y128_3_T  			s_gain_by_y128_3;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y128_4_T  			s_gain_by_y128_4;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y192_0_T  			s_gain_by_y192_0;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y192_1_T  			s_gain_by_y192_1;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y192_2_T  			s_gain_by_y192_2;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y192_3_T  			s_gain_by_y192_3;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y192_4_T  			s_gain_by_y192_4;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y256_0_T  			s_gain_by_y256_0;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y256_1_T  			s_gain_by_y256_1;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y256_2_T  			s_gain_by_y256_2;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y256_3_T  			s_gain_by_y256_3;
        ISP_NVRAM_COLOR_S_GAIN_BY_Y256_4_T  			s_gain_by_y256_4;
        ISP_NVRAM_COLOR_LSP_1_T  						lsp_1;
        ISP_NVRAM_COLOR_LSP_2_T  						lsp_2;
        ISP_NVRAM_COLOR_CM_CONTROL_T  					cm_control;
        ISP_NVRAM_COLOR_CM_W1_HUE_0_T  					cm_w1_hue_0;
        ISP_NVRAM_COLOR_CM_W1_HUE_1_T  					cm_w1_hue_1;
        ISP_NVRAM_COLOR_CM_W1_HUE_2_T  					cm_w1_hue_2;
        ISP_NVRAM_COLOR_CM_W1_HUE_3_T  					cm_w1_hue_3;
        ISP_NVRAM_COLOR_CM_W1_HUE_4_T  					cm_w1_hue_4;
        ISP_NVRAM_COLOR_CM_W1_LUMA_0_T  				cm_w1_luma_0;
        ISP_NVRAM_COLOR_CM_W1_LUMA_1_T  				cm_w1_luma_1;
        ISP_NVRAM_COLOR_CM_W1_LUMA_2_T  				cm_w1_luma_2;
        ISP_NVRAM_COLOR_CM_W1_LUMA_3_T  				cm_w1_luma_3;
        ISP_NVRAM_COLOR_CM_W1_LUMA_4_T  				cm_w1_luma_4;
        ISP_NVRAM_COLOR_CM_W1_SAT_0_T  					cm_w1_sat_0;
        ISP_NVRAM_COLOR_CM_W1_SAT_1_T  					cm_w1_sat_1;
        ISP_NVRAM_COLOR_CM_W1_SAT_2_T  					cm_w1_sat_2;
        ISP_NVRAM_COLOR_CM_W1_SAT_3_T  					cm_w1_sat_3;
        ISP_NVRAM_COLOR_CM_W1_SAT_4_T  					cm_w1_sat_4;
        ISP_NVRAM_COLOR_CM_W2_HUE_0_T  					cm_w2_hue_0;
        ISP_NVRAM_COLOR_CM_W2_HUE_1_T  					cm_w2_hue_1;
        ISP_NVRAM_COLOR_CM_W2_HUE_2_T  					cm_w2_hue_2;
        ISP_NVRAM_COLOR_CM_W2_HUE_3_T  					cm_w2_hue_3;
        ISP_NVRAM_COLOR_CM_W2_HUE_4_T  					cm_w2_hue_4;
        ISP_NVRAM_COLOR_CM_W2_LUMA_0_T  				cm_w2_luma_0;
        ISP_NVRAM_COLOR_CM_W2_LUMA_1_T  				cm_w2_luma_1;
        ISP_NVRAM_COLOR_CM_W2_LUMA_2_T  				cm_w2_luma_2;
        ISP_NVRAM_COLOR_CM_W2_LUMA_3_T  				cm_w2_luma_3;
        ISP_NVRAM_COLOR_CM_W2_LUMA_4_T  				cm_w2_luma_4;
        ISP_NVRAM_COLOR_CM_W2_SAT_0_T  					cm_w2_sat_0;
        ISP_NVRAM_COLOR_CM_W2_SAT_1_T  					cm_w2_sat_1;
        ISP_NVRAM_COLOR_CM_W2_SAT_2_T  					cm_w2_sat_2;
        ISP_NVRAM_COLOR_CM_W2_SAT_3_T  					cm_w2_sat_3;
        ISP_NVRAM_COLOR_CM_W2_SAT_4_T  					cm_w2_sat_4;
        ISP_NVRAM_COLOR_CM_W3_HUE_0_T  					cm_w3_hue_0;
        ISP_NVRAM_COLOR_CM_W3_HUE_1_T  					cm_w3_hue_1;
        ISP_NVRAM_COLOR_CM_W3_HUE_2_T  					cm_w3_hue_2;
        ISP_NVRAM_COLOR_CM_W3_HUE_3_T  					cm_w3_hue_3;
        ISP_NVRAM_COLOR_CM_W3_HUE_4_T  					cm_w3_hue_4;
        ISP_NVRAM_COLOR_CM_W3_LUMA_0_T  				cm_w3_luma_0;
        ISP_NVRAM_COLOR_CM_W3_LUMA_1_T  				cm_w3_luma_1;
        ISP_NVRAM_COLOR_CM_W3_LUMA_2_T  				cm_w3_luma_2;
        ISP_NVRAM_COLOR_CM_W3_LUMA_3_T  				cm_w3_luma_3;
        ISP_NVRAM_COLOR_CM_W3_LUMA_4_T  				cm_w3_luma_4;
        ISP_NVRAM_COLOR_CM_W3_SAT_0_T  					cm_w3_sat_0;
        ISP_NVRAM_COLOR_CM_W3_SAT_1_T  					cm_w3_sat_1;
        ISP_NVRAM_COLOR_CM_W3_SAT_2_T  					cm_w3_sat_2;
        ISP_NVRAM_COLOR_CM_W3_SAT_3_T  					cm_w3_sat_3;
        ISP_NVRAM_COLOR_CM_W3_SAT_4_T  					cm_w3_sat_4;
        //ISP_NVRAM_COLOR_TILE_T  						tile;
    };
} ISP_NVRAM_COLOR_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// NR3D
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct {
    FIELD  NR3D_yclnr_en                              :  1;		/*  0.. 0, 0x00000001 */
    FIELD  NR3D_nr3d_lbit_mode                        :  1;		/*  1.. 1, 0x00000002 */
    FIELD  NR3D_nr3d_dcm_dis                          :  1;		/*  2.. 2, 0x00000004 */
    FIELD  NR3D_nr3d_reset                            :  1;		/*  3.. 3, 0x00000008 */
    FIELD  NR3D_r2c_lbit_mode                         :  1;		/*  4.. 4, 0x00000010 */
    FIELD  NR3D_in_rnd_mode                           :  2;		/*  5.. 6, 0x00000060 */
    FIELD  rsv_7                                      :  5;		/*  7..11, 0x00000F80 */
    FIELD  NR3D_nr3d_uv_signed                        :  1;		/* 12..12, 0x00001000 */
    FIELD  NR3D_nr3d_skip_mode                        :  1;		/* 13..13, 0x00002000 */
    FIELD  NR3D_eot_seq_mode                          :  1;		/* 14..14, 0x00004000 */
    FIELD  NR3D_stat_sof_clr                          :  1;		/* 15..15, 0x00008000 */
    FIELD  NR3D_usr_tile_loss_en                      :  1;		/* 16..16, 0x00010000 */
    FIELD  rsv_17                                     : 15;		/* 17..31, 0xFFFE0000 */
} ISP_NR3D_ENG_CON_T; //	/* 0x15028000 */

typedef union {
    typedef ISP_NR3D_ENG_CON_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_ENG_CON_T;

typedef struct {
    FIELD  NR3D_nr3d_wd                               : 13;		/*  0..12, 0x00001FFF */
    FIELD  rsv_13                                     :  3;		/* 13..15, 0x0000E000 */
    FIELD  NR3D_nr3d_ht                               : 13;		/* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                     :  3;		/* 29..31, 0xE0000000 */
} ISP_NR3D_SIZ_T; //	/* 0x15028004 */

typedef union {
    typedef ISP_NR3D_SIZ_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_SIZ_T;

typedef struct {
    FIELD  NR3D_nr3d_tile_x                           : 13;		/*  0..12, 0x00001FFF */
    FIELD  rsv_13                                     :  3;		/* 13..15, 0x0000E000 */
    FIELD  NR3D_nr3d_tile_y                           : 13;		/* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                     :  3;		/* 29..31, 0xE0000000 */
} ISP_NR3D_TILE_XY_T; //	/* 0x15028008 */

typedef union {
    typedef ISP_NR3D_TILE_XY_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_TILE_XY_T;

typedef struct {
    FIELD  rsv_0                                      :  4;		/*  0.. 3, 0x0000000F */
    FIELD  NR3D_on_en                                 :  1;		/*  4.. 4, 0x00000010 */
    FIELD  rsv_5                                      :  3;		/*  5.. 7, 0x000000E0 */
    FIELD  NR3D_nr3d_tile_edge                        :  4;		/*  8..11, 0x00000F00 */
    FIELD  rsv_12                                     :  8;		/* 12..19, 0x000FF000 */
    FIELD  NR3D_sl2_off                               :  1;		/* 20..20, 0x00100000 */
    FIELD  rsv_21                                     : 11;		/* 21..31, 0xFFE00000 */
} ISP_NR3D_ON_CON_T; //	/* 0x1502800C */

typedef union {
    typedef ISP_NR3D_ON_CON_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_ON_CON_T;

typedef struct {
    FIELD  NR3D_nr3d_on_ofst_x                        : 13;		/*  0..12, 0x00001FFF */
    FIELD  rsv_13                                     :  3;		/* 13..15, 0x0000E000 */
    FIELD  NR3D_nr3d_on_ofst_y                        : 13;		/* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                     :  3;		/* 29..31, 0xE0000000 */
} ISP_NR3D_ON_OFF_T; //	/* 0x15028010 */

typedef union {
    typedef ISP_NR3D_ON_OFF_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_ON_OFF_T;

typedef struct {
    FIELD  NR3D_nr3d_on_wd                            : 13;		/*  0..12, 0x00001FFF */
    FIELD  rsv_13                                     :  3;		/* 13..15, 0x0000E000 */
    FIELD  NR3D_nr3d_on_ht                            : 13;		/* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                     :  3;		/* 29..31, 0xE0000000 */
} ISP_NR3D_ON_SIZ_T; //	/* 0x15028014 */

typedef union {
    typedef ISP_NR3D_ON_SIZ_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_ON_SIZ_T;

typedef struct {
    FIELD  rsv_0                                      : 30;		/*  0..29, 0x3FFFFFFF */
    FIELD  NR3D_tnr_c_en                              :  1;		/* 30..30, 0x40000000 */
    FIELD  NR3D_tnr_y_en                              :  1;		/* 31..31, 0x80000000 */
} ISP_NR3D_TNR_ENABLE_T; //	/* 0x15028018 */

typedef union {
    typedef ISP_NR3D_TNR_ENABLE_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_TNR_ENABLE_T;

typedef struct {
    FIELD  rsv_0                                      :  4;		/*  0.. 3, 0x0000000F */
    FIELD  NR3D_en_ccr                                :  1;		/*  4.. 4, 0x00000010 */
    FIELD  NR3D_en_cycr                               :  1;		/*  5.. 5, 0x00000020 */
    FIELD  NR3D_flt_str_max                           :  6;		/*  6..11, 0x00000FC0 */
    FIELD  NR3D_blend_ratio_mv                        :  5;		/* 12..16, 0x0001F000 */
    FIELD  NR3D_blend_ratio_txtr                      :  5;		/* 17..21, 0x003E0000 */
    FIELD  NR3D_blend_ratio_de                        :  5;		/* 22..26, 0x07C00000 */
    FIELD  NR3D_blend_ratio_blky                      :  5;		/* 27..31, 0xF8000000 */
} ISP_NR3D_FLT_CONFIG_T; //	/* 0x1502801C */

typedef union {
    typedef ISP_NR3D_FLT_CONFIG_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_FLT_CONFIG_T;

typedef struct {
    FIELD  rsv_0                                      : 20;		/*  0..19, 0x000FFFFF */
    FIELD  NR3D_q_sp                                  :  6;		/* 20..25, 0x03F00000 */
    FIELD  NR3D_q_nl                                  :  6;		/* 26..31, 0xFC000000 */
} ISP_NR3D_FB_INFO1_T; //	/* 0x15028020 */

typedef union {
    typedef ISP_NR3D_FB_INFO1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_FB_INFO1_T;

typedef struct {
    FIELD  rsv_0                                      : 10;		/*  0.. 9, 0x000003FF */
    FIELD  NR3D_small_sad_thr                         :  6;		/* 10..15, 0x0000FC00 */
    FIELD  NR3D_mv_pen_thr                            :  6;		/* 16..21, 0x003F0000 */
    FIELD  NR3D_mv_pen_w                              :  4;		/* 22..25, 0x03C00000 */
    FIELD  NR3D_bdi_thr                               :  4;		/* 26..29, 0x3C000000 */
    FIELD  rsv_30                                     :  2;		/* 30..31, 0xC0000000 */
} ISP_NR3D_THR_1_T; //	/* 0x15028024 */

typedef union {
    typedef ISP_NR3D_THR_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_THR_1_T;

typedef struct {
    FIELD  rsv_0                                      :  2;		/*  0.. 1, 0x00000003 */
    FIELD  NR3D_q_blky_y4                             :  6;		/*  2.. 7, 0x000000FC */
    FIELD  NR3D_q_blky_y3                             :  6;		/*  8..13, 0x00003F00 */
    FIELD  NR3D_q_blky_y2                             :  6;		/* 14..19, 0x000FC000 */
    FIELD  NR3D_q_blky_y1                             :  6;		/* 20..25, 0x03F00000 */
    FIELD  NR3D_q_blky_y0                             :  6;		/* 26..31, 0xFC000000 */
} ISP_NR3D_CURVE_1_T; //	/* 0x15028028 */

typedef union {
    typedef ISP_NR3D_CURVE_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_CURVE_1_T;

typedef struct {
    FIELD  rsv_0                                      :  2;		/*  0.. 1, 0x00000003 */
    FIELD  NR3D_q_blkc_y1                             :  6;		/*  2.. 7, 0x000000FC */
    FIELD  NR3D_q_blkc_y0                             :  6;		/*  8..13, 0x00003F00 */
    FIELD  NR3D_q_blky_y7                             :  6;		/* 14..19, 0x000FC000 */
    FIELD  NR3D_q_blky_y6                             :  6;		/* 20..25, 0x03F00000 */
    FIELD  NR3D_q_blky_y5                             :  6;		/* 26..31, 0xFC000000 */
} ISP_NR3D_CURVE_2_T; //	/* 0x1502802C */

typedef union {
    typedef ISP_NR3D_CURVE_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_CURVE_2_T;

typedef struct {
    FIELD  rsv_0                                      :  2;		/*  0.. 1, 0x00000003 */
    FIELD  NR3D_q_blkc_y6                             :  6;		/*  2.. 7, 0x000000FC */
    FIELD  NR3D_q_blkc_y5                             :  6;		/*  8..13, 0x00003F00 */
    FIELD  NR3D_q_blkc_y4                             :  6;		/* 14..19, 0x000FC000 */
    FIELD  NR3D_q_blkc_y3                             :  6;		/* 20..25, 0x03F00000 */
    FIELD  NR3D_q_blkc_y2                             :  6;		/* 26..31, 0xFC000000 */
} ISP_NR3D_CURVE_3_T; //	/* 0x15028030 */

typedef union {
    typedef ISP_NR3D_CURVE_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_CURVE_3_T;

typedef struct {
    FIELD  rsv_0                                      :  2;		/*  0.. 1, 0x00000003 */
    FIELD  NR3D_q_detxtr_lvl_y3                       :  6;		/*  2.. 7, 0x000000FC */
    FIELD  NR3D_q_detxtr_lvl_y2                       :  6;		/*  8..13, 0x00003F00 */
    FIELD  NR3D_q_detxtr_lvl_y1                       :  6;		/* 14..19, 0x000FC000 */
    FIELD  NR3D_q_detxtr_lvl_y0                       :  6;		/* 20..25, 0x03F00000 */
    FIELD  NR3D_q_blkc_y7                             :  6;		/* 26..31, 0xFC000000 */
} ISP_NR3D_CURVE_4_T; //	/* 0x15028034 */

typedef union {
    typedef ISP_NR3D_CURVE_4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_CURVE_4_T;

typedef struct {
    FIELD  rsv_0                                      :  2;		/*  0.. 1, 0x00000003 */
    FIELD  NR3D_q_de1_base_y0                         :  6;		/*  2.. 7, 0x000000FC */
    FIELD  NR3D_q_detxtr_lvl_y7                       :  6;		/*  8..13, 0x00003F00 */
    FIELD  NR3D_q_detxtr_lvl_y6                       :  6;		/* 14..19, 0x000FC000 */
    FIELD  NR3D_q_detxtr_lvl_y5                       :  6;		/* 20..25, 0x03F00000 */
    FIELD  NR3D_q_detxtr_lvl_y4                       :  6;		/* 26..31, 0xFC000000 */
} ISP_NR3D_CURVE_5_T; //	/* 0x15028038 */

typedef union {
    typedef ISP_NR3D_CURVE_5_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_CURVE_5_T;

typedef struct {
    FIELD  rsv_0                                      :  2;		/*  0.. 1, 0x00000003 */
    FIELD  NR3D_q_de1_base_y5                         :  6;		/*  2.. 7, 0x000000FC */
    FIELD  NR3D_q_de1_base_y4                         :  6;		/*  8..13, 0x00003F00 */
    FIELD  NR3D_q_de1_base_y3                         :  6;		/* 14..19, 0x000FC000 */
    FIELD  NR3D_q_de1_base_y2                         :  6;		/* 20..25, 0x03F00000 */
    FIELD  NR3D_q_de1_base_y1                         :  6;		/* 26..31, 0xFC000000 */
} ISP_NR3D_CURVE_6_T; //	/* 0x1502803C */

typedef union {
    typedef ISP_NR3D_CURVE_6_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_CURVE_6_T;

typedef struct {
    FIELD  rsv_0                                      :  2;		/*  0.. 1, 0x00000003 */
    FIELD  NR3D_q_de2txtr_base_y2                     :  6;		/*  2.. 7, 0x000000FC */
    FIELD  NR3D_q_de2txtr_base_y1                     :  6;		/*  8..13, 0x00003F00 */
    FIELD  NR3D_q_de2txtr_base_y0                     :  6;		/* 14..19, 0x000FC000 */
    FIELD  NR3D_q_de1_base_y7                         :  6;		/* 20..25, 0x03F00000 */
    FIELD  NR3D_q_de1_base_y6                         :  6;		/* 26..31, 0xFC000000 */
} ISP_NR3D_CURVE_7_T; //	/* 0x15028040 */

typedef union {
    typedef ISP_NR3D_CURVE_7_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_CURVE_7_T;

typedef struct {
    FIELD  rsv_0                                      :  2;		/*  0.. 1, 0x00000003 */
    FIELD  NR3D_q_de2txtr_base_y7                     :  6;		/*  2.. 7, 0x000000FC */
    FIELD  NR3D_q_de2txtr_base_y6                     :  6;		/*  8..13, 0x00003F00 */
    FIELD  NR3D_q_de2txtr_base_y5                     :  6;		/* 14..19, 0x000FC000 */
    FIELD  NR3D_q_de2txtr_base_y4                     :  6;		/* 20..25, 0x03F00000 */
    FIELD  NR3D_q_de2txtr_base_y3                     :  6;		/* 26..31, 0xFC000000 */
} ISP_NR3D_CURVE_8_T; //	/* 0x15028044 */

typedef union {
    typedef ISP_NR3D_CURVE_8_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_CURVE_8_T;

typedef struct {
    FIELD  rsv_0                                      :  2;		/*  0.. 1, 0x00000003 */
    FIELD  NR3D_q_mv_y4                               :  6;		/*  2.. 7, 0x000000FC */
    FIELD  NR3D_q_mv_y3                               :  6;		/*  8..13, 0x00003F00 */
    FIELD  NR3D_q_mv_y2                               :  6;		/* 14..19, 0x000FC000 */
    FIELD  NR3D_q_mv_y1                               :  6;		/* 20..25, 0x03F00000 */
    FIELD  NR3D_q_mv_y0                               :  6;		/* 26..31, 0xFC000000 */
} ISP_NR3D_CURVE_9_T; //	/* 0x15028048 */

typedef union {
    typedef ISP_NR3D_CURVE_9_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_CURVE_9_T;

typedef struct {
    FIELD  rsv_0                                      :  2;		/*  0.. 1, 0x00000003 */
    FIELD  NR3D_q_wvar_y1                             :  6;		/*  2.. 7, 0x000000FC */
    FIELD  NR3D_q_wvar_y0                             :  6;		/*  8..13, 0x00003F00 */
    FIELD  NR3D_q_mv_y7                               :  6;		/* 14..19, 0x000FC000 */
    FIELD  NR3D_q_mv_y6                               :  6;		/* 20..25, 0x03F00000 */
    FIELD  NR3D_q_mv_y5                               :  6;		/* 26..31, 0xFC000000 */
} ISP_NR3D_CURVE_10_T; //	/* 0x1502804C */

typedef union {
    typedef ISP_NR3D_CURVE_10_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_CURVE_10_T;

typedef struct {
    FIELD  rsv_0                                      :  2;		/*  0.. 1, 0x00000003 */
    FIELD  NR3D_q_wvar_y6                             :  6;		/*  2.. 7, 0x000000FC */
    FIELD  NR3D_q_wvar_y5                             :  6;		/*  8..13, 0x00003F00 */
    FIELD  NR3D_q_wvar_y4                             :  6;		/* 14..19, 0x000FC000 */
    FIELD  NR3D_q_wvar_y3                             :  6;		/* 20..25, 0x03F00000 */
    FIELD  NR3D_q_wvar_y2                             :  6;		/* 26..31, 0xFC000000 */
} ISP_NR3D_CURVE_11_T; //	/* 0x15028050 */

typedef union {
    typedef ISP_NR3D_CURVE_11_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_CURVE_11_T;

typedef struct {
    FIELD  rsv_0                                      :  2;		/*  0.. 1, 0x00000003 */
    FIELD  NR3D_q_wsm_y3                              :  6;		/*  2.. 7, 0x000000FC */
    FIELD  NR3D_q_wsm_y2                              :  6;		/*  8..13, 0x00003F00 */
    FIELD  NR3D_q_wsm_y1                              :  6;		/* 14..19, 0x000FC000 */
    FIELD  NR3D_q_wsm_y0                              :  6;		/* 20..25, 0x03F00000 */
    FIELD  NR3D_q_wvar_y7                             :  6;		/* 26..31, 0xFC000000 */
} ISP_NR3D_CURVE_12_T; //	/* 0x15028054 */

typedef union {
    typedef ISP_NR3D_CURVE_12_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_CURVE_12_T;

typedef struct {
    FIELD  rsv_0                                      :  8;		/*  0.. 7, 0x000000FF */
    FIELD  NR3D_q_wsm_y7                              :  6;		/*  8..13, 0x00003F00 */
    FIELD  NR3D_q_wsm_y6                              :  6;		/* 14..19, 0x000FC000 */
    FIELD  NR3D_q_wsm_y5                              :  6;		/* 20..25, 0x03F00000 */
    FIELD  NR3D_q_wsm_y4                              :  6;		/* 26..31, 0xFC000000 */
} ISP_NR3D_CURVE_13_T; //	/* 0x15028058 */

typedef union {
    typedef ISP_NR3D_CURVE_13_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_CURVE_13_T;

typedef struct {
    FIELD  rsv_0                                      :  2;		/*  0.. 1, 0x00000003 */
    FIELD  NR3D_q_sdl_y4                              :  6;		/*  2.. 7, 0x000000FC */
    FIELD  NR3D_q_sdl_y3                              :  6;		/*  8..13, 0x00003F00 */
    FIELD  NR3D_q_sdl_y2                              :  6;		/* 14..19, 0x000FC000 */
    FIELD  NR3D_q_sdl_y1                              :  6;		/* 20..25, 0x03F00000 */
    FIELD  NR3D_q_sdl_y0                              :  6;		/* 26..31, 0xFC000000 */
} ISP_NR3D_CURVE_14_T; //	/* 0x1502805C */

typedef union {
    typedef ISP_NR3D_CURVE_14_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_CURVE_14_T;

typedef struct {
    FIELD  rsv_0                                      :  8;		/*  0.. 7, 0x000000FF */
    FIELD  NR3D_q_sdl_y8                              :  6;		/*  8..13, 0x00003F00 */
    FIELD  NR3D_q_sdl_y7                              :  6;		/* 14..19, 0x000FC000 */
    FIELD  NR3D_q_sdl_y6                              :  6;		/* 20..25, 0x03F00000 */
    FIELD  NR3D_q_sdl_y5                              :  6;		/* 26..31, 0xFC000000 */
} ISP_NR3D_CURVE_15_T; //	/* 0x15028060 */

typedef union {
    typedef ISP_NR3D_CURVE_15_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_CURVE_15_T;

typedef struct {
    FIELD  NR3D_r2cenc                                :  2;		/*  0.. 1, 0x00000003 */
    FIELD  NR3D_r2c_val4                              :  6;		/*  2.. 7, 0x000000FC */
    FIELD  NR3D_r2c_val3                              :  6;		/*  8..13, 0x00003F00 */
    FIELD  NR3D_r2c_val2                              :  6;		/* 14..19, 0x000FC000 */
    FIELD  NR3D_r2c_val1                              :  6;		/* 20..25, 0x03F00000 */
    FIELD  rsv_26                                     :  6;		/* 26..31, 0xFC000000 */
} ISP_NR3D_R2C_1_T; //	/* 0x15028064 */

typedef union {
    typedef ISP_NR3D_R2C_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_R2C_1_T;

typedef struct {
    FIELD  rsv_0                                      :  2;		/*  0.. 1, 0x00000003 */
    FIELD  NR3D_r2c_txtr_throff                       :  6;		/*  2.. 7, 0x000000FC */
    FIELD  NR3D_r2c_txtr_thr4                         :  6;		/*  8..13, 0x00003F00 */
    FIELD  NR3D_r2c_txtr_thr3                         :  6;		/* 14..19, 0x000FC000 */
    FIELD  NR3D_r2c_txtr_thr2                         :  6;		/* 20..25, 0x03F00000 */
    FIELD  NR3D_r2c_txtr_thr1                         :  6;		/* 26..31, 0xFC000000 */
} ISP_NR3D_R2C_2_T; //	/* 0x15028068 */

typedef union {
    typedef ISP_NR3D_R2C_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_R2C_2_T;

typedef struct {
    FIELD  rsv_0                                      :  8;		/*  0.. 7, 0x000000FF */
    FIELD  NR3D_r2cf_cnt4                             :  6;		/*  8..13, 0x00003F00 */
    FIELD  NR3D_r2cf_cnt3                             :  6;		/* 14..19, 0x000FC000 */
    FIELD  NR3D_r2cf_cnt2                             :  6;		/* 20..25, 0x03F00000 */
    FIELD  NR3D_r2cf_cnt1                             :  6;		/* 26..31, 0xFC000000 */
} ISP_NR3D_R2C_3_T; //	/* 0x1502806C */

typedef union {
    typedef ISP_NR3D_R2C_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_R2C_3_T;

typedef struct {
    FIELD  NR3D_one_frame_mode                        :  1;		/*  0.. 0, 0x00000001 */
    FIELD  rsv_1                                      :  1;		/*  1.. 1, 0x00000002 */
    FIELD  NR3D_force_en                              :  1;		/*  2.. 2, 0x00000004 */
    FIELD  NR3D_ink_level_disp                        : 16;		/*  3..18, 0x0007FFF8 */
    FIELD  NR3D_ink_sel                               :  5;		/* 19..23, 0x00F80000 */
    FIELD  NR3D_ink_y_en                              :  1;		/* 24..24, 0x01000000 */
    FIELD  NR3D_ink_en                                :  1;		/* 25..25, 0x02000000 */
    FIELD  NR3D_force_flt_str                         :  6;		/* 26..31, 0xFC000000 */
} ISP_NR3D_DBG_6_T; //	/* 0x15028070 */

typedef union {
    typedef ISP_NR3D_DBG_6_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_DBG_6_T;

typedef struct {
    FIELD  rsv_0                                      :  2;		/*  0.. 1, 0x00000003 */
    FIELD  NR3D_osd_targv                             : 13;		/*  2..14, 0x00007FFC */
    FIELD  NR3D_osd_targh                             : 13;		/* 15..27, 0x0FFF8000 */
    FIELD  NR3D_osd_sel                               :  3;		/* 28..30, 0x70000000 */
    FIELD  NR3D_osd_en                                :  1;		/* 31..31, 0x80000000 */
} ISP_NR3D_DBG_15_T; //	/* 0x15028074 */

typedef union {
    typedef ISP_NR3D_DBG_15_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_DBG_15_T;

typedef struct {
    FIELD  rsv_0                                      :  3;		/*  0.. 2, 0x00000007 */
    FIELD  NR3D_osd_y_en                              :  1;		/*  3.. 3, 0x00000008 */
    FIELD  NR3D_osd_disp_scale                        :  2;		/*  4.. 5, 0x00000030 */
    FIELD  NR3D_osd_dispv                             : 13;		/*  6..18, 0x0007FFC0 */
    FIELD  NR3D_osd_disph                             : 13;		/* 19..31, 0xFFF80000 */
} ISP_NR3D_DBG_16_T; //	/* 0x15028078 */

typedef union {
    typedef ISP_NR3D_DBG_16_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_DBG_16_T;

typedef struct {
    FIELD  rsv_0                                      :  4;		/*  0.. 3, 0x0000000F */
    FIELD  NR3D_demo_bot                              : 13;		/*  4..16, 0x0001FFF0 */
    FIELD  NR3D_demo_top                              : 13;		/* 17..29, 0x3FFE0000 */
    FIELD  NR3D_demo_sel                              :  1;		/* 30..30, 0x40000000 */
    FIELD  NR3D_demo_en                               :  1;		/* 31..31, 0x80000000 */
} ISP_NR3D_DEMO_1_T; //	/* 0x1502807C */

typedef union {
    typedef ISP_NR3D_DEMO_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_DEMO_1_T;

typedef struct {
    FIELD  rsv_0                                      :  6;		/*  0.. 5, 0x0000003F */
    FIELD  NR3D_demo_right                            : 13;		/*  6..18, 0x0007FFC0 */
    FIELD  NR3D_demo_left                             : 13;		/* 19..31, 0xFFF80000 */
} ISP_NR3D_DEMO_2_T; //	/* 0x15028080 */

typedef union {
    typedef ISP_NR3D_DEMO_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_DEMO_2_T;

typedef struct {
    FIELD  NR3D_nr3d_atpg_ob                          :  1;		/*  0.. 0, 0x00000001 */
    FIELD  NR3D_nr3d_atpg_ct                          :  1;		/*  1.. 1, 0x00000002 */
    FIELD  rsv_2                                      : 30;		/*  2..31, 0xFFFFFFFC */
} ISP_NR3D_ATPG_T; //	/* 0x15028084 */

typedef union {
    typedef ISP_NR3D_ATPG_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_ATPG_T;

typedef struct {
    FIELD  NR3D_tnr_dummy0                            : 32;		/*  0..31, 0xFFFFFFFF */
} ISP_NR3D_DMY_0_T; //	/* 0x15028088 */

typedef union {
    typedef ISP_NR3D_DMY_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_DMY_0_T;

typedef struct {
    FIELD  NR3D_osd_info_y                            : 16;		/*  0..15, 0x0000FFFF */
    FIELD  NR3D_osd_info_c                            : 16;		/* 16..31, 0xFFFF0000 */
} ISP_NR3D_DBG_17_T; //	/* 0x1502808C */

typedef union {
    typedef ISP_NR3D_DBG_17_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_DBG_17_T;

typedef struct {
    FIELD  NR3D_int_if1_en                            :  1;		/*  0.. 0, 0x00000001 */
    FIELD  NR3D_int_if2_en                            :  1;		/*  1.. 1, 0x00000002 */
    FIELD  NR3D_int_if3_en                            :  1;		/*  2.. 2, 0x00000004 */
    FIELD  NR3D_int_of_en                             :  1;		/*  3.. 3, 0x00000008 */
    FIELD  NR3D_int_rdma_en                           :  1;		/*  4.. 4, 0x00000010 */
    FIELD  rsv_5                                      : 27;		/*  5..31, 0xFFFFFFE0 */
} ISP_NR3D_INTERR_T; //	/* 0x150280BC */

typedef union {
    typedef ISP_NR3D_INTERR_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_INTERR_T;

typedef struct {
    FIELD  NR3D_q_nl_sum                              : 32;		/*  0..31, 0xFFFFFFFF */
} ISP_NR3D_FB_INFO2_T; //	/* 0x150280C4 */

typedef union {
    typedef ISP_NR3D_FB_INFO2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_FB_INFO2_T;

typedef struct {
    FIELD  NR3D_q_sp_sum                              : 32;		/*  0..31, 0xFFFFFFFF */
} ISP_NR3D_FB_INFO3_T; //	/* 0x150280C8 */

typedef union {
    typedef ISP_NR3D_FB_INFO3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_FB_INFO3_T;

typedef struct {
    FIELD  NR3D_q_still_cnt                           : 32;		/*  0..31, 0xFFFFFFFF */
} ISP_NR3D_FB_INFO4_T; //	/* 0x150280CC */

typedef union {
    typedef ISP_NR3D_FB_INFO4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_FB_INFO4_T;

typedef struct {
    FIELD  rsv_0                                      : 29;		/*  0..28, 0x1FFFFFFF */
    FIELD  NR3D_read_working                          :  1;		/* 29..29, 0x20000000 */
    FIELD  NR3D_reg_protect                           :  1;		/* 30..30, 0x40000000 */
    FIELD  NR3D_dbuf_en                               :  1;		/* 31..31, 0x80000000 */
} ISP_NR3D_DBFISH_T; //	/* 0x150280D0 */

typedef union {
    typedef ISP_NR3D_DBFISH_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_DBFISH_T;

typedef struct {
    FIELD  NR3D_ink_color_y7                          :  4;		/*  0.. 3, 0x0000000F */
    FIELD  NR3D_ink_color_y6                          :  4;		/*  4.. 7, 0x000000F0 */
    FIELD  NR3D_ink_color_y5                          :  4;		/*  8..11, 0x00000F00 */
    FIELD  NR3D_ink_color_y4                          :  4;		/* 12..15, 0x0000F000 */
    FIELD  NR3D_ink_color_y3                          :  4;		/* 16..19, 0x000F0000 */
    FIELD  NR3D_ink_color_y2                          :  4;		/* 20..23, 0x00F00000 */
    FIELD  NR3D_ink_color_y1                          :  4;		/* 24..27, 0x0F000000 */
    FIELD  NR3D_ink_color_y0                          :  4;		/* 28..31, 0xF0000000 */
} ISP_NR3D_DBG_9_T; //	/* 0x150280D4 */

typedef union {
    typedef ISP_NR3D_DBG_9_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_DBG_9_T;

typedef struct {
    FIELD  NR3D_ink_color_y15                         :  4;		/*  0.. 3, 0x0000000F */
    FIELD  NR3D_ink_color_y14                         :  4;		/*  4.. 7, 0x000000F0 */
    FIELD  NR3D_ink_color_y13                         :  4;		/*  8..11, 0x00000F00 */
    FIELD  NR3D_ink_color_y12                         :  4;		/* 12..15, 0x0000F000 */
    FIELD  NR3D_ink_color_y11                         :  4;		/* 16..19, 0x000F0000 */
    FIELD  NR3D_ink_color_y10                         :  4;		/* 20..23, 0x00F00000 */
    FIELD  NR3D_ink_color_y9                          :  4;		/* 24..27, 0x0F000000 */
    FIELD  NR3D_ink_color_y8                          :  4;		/* 28..31, 0xF0000000 */
} ISP_NR3D_DBG_10_T; //	/* 0x150280D8 */

typedef union {
    typedef ISP_NR3D_DBG_10_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_DBG_10_T;

typedef struct {
    FIELD  NR3D_ink_color_c7                          :  4;		/*  0.. 3, 0x0000000F */
    FIELD  NR3D_ink_color_c6                          :  4;		/*  4.. 7, 0x000000F0 */
    FIELD  NR3D_ink_color_c5                          :  4;		/*  8..11, 0x00000F00 */
    FIELD  NR3D_ink_color_c4                          :  4;		/* 12..15, 0x0000F000 */
    FIELD  NR3D_ink_color_c3                          :  4;		/* 16..19, 0x000F0000 */
    FIELD  NR3D_ink_color_c2                          :  4;		/* 20..23, 0x00F00000 */
    FIELD  NR3D_ink_color_c1                          :  4;		/* 24..27, 0x0F000000 */
    FIELD  NR3D_ink_color_c0                          :  4;		/* 28..31, 0xF0000000 */
} ISP_NR3D_DBG_11_T; //	/* 0x150280DC */

typedef union {
    typedef ISP_NR3D_DBG_11_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_DBG_11_T;

typedef struct {
    FIELD  NR3D_ink_color_c15                         :  4;		/*  0.. 3, 0x0000000F */
    FIELD  NR3D_ink_color_c14                         :  4;		/*  4.. 7, 0x000000F0 */
    FIELD  NR3D_ink_color_c13                         :  4;		/*  8..11, 0x00000F00 */
    FIELD  NR3D_ink_color_c12                         :  4;		/* 12..15, 0x0000F000 */
    FIELD  NR3D_ink_color_c11                         :  4;		/* 16..19, 0x000F0000 */
    FIELD  NR3D_ink_color_c10                         :  4;		/* 20..23, 0x00F00000 */
    FIELD  NR3D_ink_color_c9                          :  4;		/* 24..27, 0x0F000000 */
    FIELD  NR3D_ink_color_c8                          :  4;		/* 28..31, 0xF0000000 */
} ISP_NR3D_DBG_12_T; //	/* 0x150280E0 */

typedef union {
    typedef ISP_NR3D_DBG_12_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_DBG_12_T;

typedef struct {
    FIELD  NR3D_osd_color_fg1_c                       :  4;		/*  0.. 3, 0x0000000F */
    FIELD  NR3D_osd_color_fg1_y                       :  4;		/*  4.. 7, 0x000000F0 */
    FIELD  rsv_8                                      :  4;		/*  8..11, 0x00000F00 */
    FIELD  NR3D_osd_color_fg0_c                       :  4;		/* 12..15, 0x0000F000 */
    FIELD  NR3D_osd_color_fg0_y                       :  4;		/* 16..19, 0x000F0000 */
    FIELD  rsv_20                                     :  4;		/* 20..23, 0x00F00000 */
    FIELD  NR3D_osd_color_bg_c                        :  4;		/* 24..27, 0x0F000000 */
    FIELD  NR3D_osd_color_bg_y                        :  4;		/* 28..31, 0xF0000000 */
} ISP_NR3D_DBG_7_T; //	/* 0x150280EC */

typedef union {
    typedef ISP_NR3D_DBG_7_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_DBG_7_T;

typedef struct {
    FIELD  NR3D_tnr_dummy1                            : 32;		/*  0..31, 0xFFFFFFFF */
} ISP_NR3D_DMY_1_T; //	/* 0x15028104 */

typedef union {
    typedef ISP_NR3D_DMY_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_DMY_1_T;

typedef struct {
    FIELD  NR3D_tnr_dummy2                            : 32;		/*  0..31, 0xFFFFFFFF */
} ISP_NR3D_DMY_2_T; //	/* 0x15028108 */

typedef union {
    typedef ISP_NR3D_DMY_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_DMY_2_T;

typedef struct {
    FIELD  rsv_0                                      :  6;		/*  0.. 5, 0x0000003F */
    FIELD  NR3D_hist_bot                              : 13;		/*  6..18, 0x0007FFC0 */
    FIELD  NR3D_hist_top                              : 13;		/* 19..31, 0xFFF80000 */
} ISP_NR3D_SAVE_INFO1_T; //	/* 0x15028120 */

typedef union {
    typedef ISP_NR3D_SAVE_INFO1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_SAVE_INFO1_T;

typedef struct {
    FIELD  rsv_0                                      :  6;		/*  0.. 5, 0x0000003F */
    FIELD  NR3D_hist_right                            : 13;		/*  6..18, 0x0007FFC0 */
    FIELD  NR3D_hist_left                             : 13;		/* 19..31, 0xFFF80000 */
} ISP_NR3D_SAVE_INFO2_T; //	/* 0x15028124 */

typedef union {
    typedef ISP_NR3D_SAVE_INFO2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_SAVE_INFO2_T;

typedef struct {
    FIELD  rsv_0                                      :  2;		/*  0.. 1, 0x00000003 */
    FIELD  NR3D_q_snr_tpre_y4                         :  6;		/*  2.. 7, 0x000000FC */
    FIELD  NR3D_q_snr_tpre_y3                         :  6;		/*  8..13, 0x00003F00 */
    FIELD  NR3D_q_snr_tpre_y2                         :  6;		/* 14..19, 0x000FC000 */
    FIELD  NR3D_q_snr_tpre_y1                         :  6;		/* 20..25, 0x03F00000 */
    FIELD  NR3D_q_snr_tpre_y0                         :  6;		/* 26..31, 0xFC000000 */
} ISP_NR3D_SNR_CURVE_1_T; //	/* 0x150281CC */

typedef union {
    typedef ISP_NR3D_SNR_CURVE_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_SNR_CURVE_1_T;

typedef struct {
    FIELD  rsv_0                                      :  2;		/*  0.. 1, 0x00000003 */
    FIELD  NR3D_q_snr_txtr_y0                         :  6;		/*  2.. 7, 0x000000FC */
    FIELD  NR3D_q_snr_tpre_y8                         :  6;		/*  8..13, 0x00003F00 */
    FIELD  NR3D_q_snr_tpre_y7                         :  6;		/* 14..19, 0x000FC000 */
    FIELD  NR3D_q_snr_tpre_y6                         :  6;		/* 20..25, 0x03F00000 */
    FIELD  NR3D_q_snr_tpre_y5                         :  6;		/* 26..31, 0xFC000000 */
} ISP_NR3D_SNR_CURVE_2_T; //	/* 0x150281D0 */

typedef union {
    typedef ISP_NR3D_SNR_CURVE_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_SNR_CURVE_2_T;

typedef struct {
    FIELD  rsv_0                                      :  2;		/*  0.. 1, 0x00000003 */
    FIELD  NR3D_q_snr_txtr_y5                         :  6;		/*  2.. 7, 0x000000FC */
    FIELD  NR3D_q_snr_txtr_y4                         :  6;		/*  8..13, 0x00003F00 */
    FIELD  NR3D_q_snr_txtr_y3                         :  6;		/* 14..19, 0x000FC000 */
    FIELD  NR3D_q_snr_txtr_y2                         :  6;		/* 20..25, 0x03F00000 */
    FIELD  NR3D_q_snr_txtr_y1                         :  6;		/* 26..31, 0xFC000000 */
} ISP_NR3D_SNR_CURVE_3_T; //	/* 0x150281D4 */

typedef union {
    typedef ISP_NR3D_SNR_CURVE_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_SNR_CURVE_3_T;

typedef struct {
    FIELD  rsv_0                                      :  2;		/*  0.. 1, 0x00000003 */
    FIELD  NR3D_q_snr_luma_y2                         :  6;		/*  2.. 7, 0x000000FC */
    FIELD  NR3D_q_snr_luma_y1                         :  6;		/*  8..13, 0x00003F00 */
    FIELD  NR3D_q_snr_luma_y0                         :  6;		/* 14..19, 0x000FC000 */
    FIELD  NR3D_q_snr_txtr_y7                         :  6;		/* 20..25, 0x03F00000 */
    FIELD  NR3D_q_snr_txtr_y6                         :  6;		/* 26..31, 0xFC000000 */
} ISP_NR3D_SNR_CURVE_4_T; //	/* 0x150281D8 */

typedef union {
    typedef ISP_NR3D_SNR_CURVE_4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_SNR_CURVE_4_T;

typedef struct {
    FIELD  rsv_0                                      :  2;		/*  0.. 1, 0x00000003 */
    FIELD  NR3D_q_snr_luma_y7                         :  6;		/*  2.. 7, 0x000000FC */
    FIELD  NR3D_q_snr_luma_y6                         :  6;		/*  8..13, 0x00003F00 */
    FIELD  NR3D_q_snr_luma_y5                         :  6;		/* 14..19, 0x000FC000 */
    FIELD  NR3D_q_snr_luma_y4                         :  6;		/* 20..25, 0x03F00000 */
    FIELD  NR3D_q_snr_luma_y3                         :  6;		/* 26..31, 0xFC000000 */
} ISP_NR3D_SNR_CURVE_5_T; //	/* 0x150281DC */

typedef union {
    typedef ISP_NR3D_SNR_CURVE_5_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_SNR_CURVE_5_T;

typedef struct {
    FIELD  rsv_0                                      :  2;		/*  0.. 1, 0x00000003 */
    FIELD  NR3D_q_snr_tpst_y3                         :  6;		/*  2.. 7, 0x000000FC */
    FIELD  NR3D_q_snr_tpst_y2                         :  6;		/*  8..13, 0x00003F00 */
    FIELD  NR3D_q_snr_tpst_y1                         :  6;		/* 14..19, 0x000FC000 */
    FIELD  NR3D_q_snr_tpst_y0                         :  6;		/* 20..25, 0x03F00000 */
    FIELD  NR3D_q_snr_luma_y8                         :  6;		/* 26..31, 0xFC000000 */
} ISP_NR3D_SNR_CURVE_6_T; //	/* 0x150281E0 */

typedef union {
    typedef ISP_NR3D_SNR_CURVE_6_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_SNR_CURVE_6_T;

typedef struct {
    FIELD  rsv_0                                      :  2;		/*  0.. 1, 0x00000003 */
    FIELD  NR3D_q_snr_tpst_y8                         :  6;		/*  2.. 7, 0x000000FC */
    FIELD  NR3D_q_snr_tpst_y7                         :  6;		/*  8..13, 0x00003F00 */
    FIELD  NR3D_q_snr_tpst_y6                         :  6;		/* 14..19, 0x000FC000 */
    FIELD  NR3D_q_snr_tpst_y5                         :  6;		/* 20..25, 0x03F00000 */
    FIELD  NR3D_q_snr_tpst_y4                         :  6;		/* 26..31, 0xFC000000 */
} ISP_NR3D_SNR_CURVE_7_T; //	/* 0x150281E4 */

typedef union {
    typedef ISP_NR3D_SNR_CURVE_7_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_SNR_CURVE_7_T;

typedef struct {
    FIELD  rsv_0                                      : 17;		/*  0..16, 0x0001FFFF */
    FIELD  NR3D_snr_blend_maxmin                      :  1;		/* 17..17, 0x00020000 */
    FIELD  NR3D_snr_luma_en                           :  1;		/* 18..18, 0x00040000 */
    FIELD  rsv_19                                     :  1;		/* 19..19, 0x00080000 */
    FIELD  NR3D_snr_txtr_en                           :  1;		/* 20..20, 0x00100000 */
    FIELD  rsv_21                                     :  7;		/* 21..27, 0x0FE00000 */
    FIELD  NR3D_snr_tpre_en                           :  1;		/* 28..28, 0x10000000 */
    FIELD  NR3D_snr_post2d_en                         :  1;		/* 29..29, 0x20000000 */
    FIELD  NR3D_snr_pre2d_en                          :  1;		/* 30..30, 0x40000000 */
    FIELD  NR3D_snr_en                                :  1;		/* 31..31, 0x80000000 */
} ISP_NR3D_SNR_CONTROL_1_T; //	/* 0x150281FC */

typedef union {
    typedef ISP_NR3D_SNR_CONTROL_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_SNR_CONTROL_1_T;

typedef struct {
    FIELD  rsv_0                                      : 10;		/*  0.. 9, 0x000003FF */
    FIELD  NR3D_tolerance_clip                        :  6;		/* 10..15, 0x0000FC00 */
    FIELD  NR3D_centvar_tolerance                     :  8;		/* 16..23, 0x00FF0000 */
    FIELD  NR3D_blkvar_tolerance                      :  8;		/* 24..31, 0xFF000000 */
} ISP_NR3D_SNR_THR_2_T; //	/* 0x1502820C */

typedef union {
    typedef ISP_NR3D_SNR_THR_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_SNR_THR_2_T;

typedef struct {
    FIELD  rsv_0                                      :  4;		/*  0.. 3, 0x0000000F */
    FIELD  NR3D_cent_cand_wei                         :  6;		/*  4.. 9, 0x000003F0 */
    FIELD  NR3D_sad_invwei                            :  6;		/* 10..15, 0x0000FC00 */
    FIELD  rsv_16                                     : 16;		/* 16..31, 0xFFFF0000 */
} ISP_NR3D_SNR_THR_3_T; //	/* 0x15028210 */

typedef union {
    typedef ISP_NR3D_SNR_THR_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_SNR_THR_3_T;

typedef struct {
    FIELD  rsv_0                                      :  2;		/*  0.. 1, 0x00000003 */
    FIELD  NR3D_hfrr_post2d_mf_ratio                  :  6;		/*  2.. 7, 0x000000FC */
    FIELD  NR3D_hfrr_post2d_hf_ratio                  :  6;		/*  8..13, 0x00003F00 */
    FIELD  NR3D_add_curr_ratio                        :  6;		/* 14..19, 0x000FC000 */
    FIELD  NR3D_hfrr_mf_ratio                         :  6;		/* 20..25, 0x03F00000 */
    FIELD  NR3D_hfrr_hf_ratio                         :  6;		/* 26..31, 0xFC000000 */
} ISP_NR3D_SNR_THR_4_T; //	/* 0x15028214 */

typedef union {
    typedef ISP_NR3D_SNR_THR_4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_SNR_THR_4_T;

typedef struct {
    FIELD  NR3D_in1_pxl_cnt                           : 13;		/*  0..12, 0x00001FFF */
    FIELD  rsv_13                                     :  3;		/* 13..15, 0x0000E000 */
    FIELD  NR3D_in1_line_cnt                          : 13;		/* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                     :  3;		/* 29..31, 0xE0000000 */
} ISP_NR3D_IN1_CNT_T; //	/* 0x15028218 */

typedef union {
    typedef ISP_NR3D_IN1_CNT_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_IN1_CNT_T;

typedef struct {
    FIELD  NR3D_in2_pxl_cnt                           : 13;		/*  0..12, 0x00001FFF */
    FIELD  rsv_13                                     :  3;		/* 13..15, 0x0000E000 */
    FIELD  NR3D_in2_line_cnt                          : 13;		/* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                     :  3;		/* 29..31, 0xE0000000 */
} ISP_NR3D_IN2_CNT_T; //	/* 0x1502821C */

typedef union {
    typedef ISP_NR3D_IN2_CNT_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_IN2_CNT_T;

typedef struct {
    FIELD  NR3D_in3_pxl_cnt                           : 13;		/*  0..12, 0x00001FFF */
    FIELD  rsv_13                                     :  3;		/* 13..15, 0x0000E000 */
    FIELD  NR3D_in3_line_cnt                          : 13;		/* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                     :  3;		/* 29..31, 0xE0000000 */
} ISP_NR3D_IN3_CNT_T; //	/* 0x15028220 */

typedef union {
    typedef ISP_NR3D_IN3_CNT_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_IN3_CNT_T;

typedef struct {
    FIELD  NR3D_out_pxl_cnt                           : 13;		/*  0..12, 0x00001FFF */
    FIELD  rsv_13                                     :  3;		/* 13..15, 0x0000E000 */
    FIELD  NR3D_out_line_cnt                          : 13;		/* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                     :  3;		/* 29..31, 0xE0000000 */
} ISP_NR3D_OUT_CNT_T; //	/* 0x15028224 */

typedef union {
    typedef ISP_NR3D_OUT_CNT_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_OUT_CNT_T;

typedef struct {
    FIELD  NR3D_if1_end                               :  1;		/*  0.. 0, 0x00000001 */
    FIELD  NR3D_if2_end                               :  1;		/*  1.. 1, 0x00000002 */
    FIELD  NR3D_if3_end                               :  1;		/*  2.. 2, 0x00000004 */
    FIELD  NR3D_of_end                                :  1;		/*  3.. 3, 0x00000008 */
    FIELD  NR3D_rdma_unf                              :  1;		/*  4.. 4, 0x00000010 */
    FIELD  rsv_5                                      : 27;		/*  5..31, 0xFFFFFFE0 */
} ISP_NR3D_STATUS_T; //	/* 0x15028228 */

typedef union {
    typedef ISP_NR3D_STATUS_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_STATUS_T;

typedef struct {
    FIELD  NR3D_usr_tile_loss_h                       : 13;		/*  0..12, 0x00001FFF */
    FIELD  rsv_13                                     :  3;		/* 13..15, 0x0000E000 */
    FIELD  NR3D_usr_tile_loss_v                       : 13;		/* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                     :  3;		/* 29..31, 0xE0000000 */
} ISP_NR3D_TILE_LOSS_T; //	/* 0x1502822C */

typedef union {
    typedef ISP_NR3D_TILE_LOSS_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_TILE_LOSS_T;

typedef struct {
    FIELD  rsv_0                                      :  2;		/*  0.. 1, 0x00000003 */
    FIELD  NR3D_if_hd_f_dead_zone                     :  6;		/*  2.. 7, 0x000000FC */
    FIELD  NR3D_if_hd_h_m_dead_zone                   :  8;		/*  8..15, 0x0000FF00 */
    FIELD  NR3D_if_hd_h_dead_zone                     :  8;		/* 16..23, 0x00FF0000 */
    FIELD  NR3D_if_hd_h_mi_normaliz                   :  4;		/* 24..27, 0x0F000000 */
    FIELD  NR3D_if_hd_h_hi_normaliz                   :  4;		/* 28..31, 0xF0000000 */
} ISP_NR3D_MCVP_STAT1_T; //	/* 0x15028230 */

typedef union {
    typedef ISP_NR3D_MCVP_STAT1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_MCVP_STAT1_T;

typedef struct {
    FIELD  rsv_0                                      :  8;		/*  0.. 7, 0x000000FF */
    FIELD  NR3D_if_hd_field_hi_f_sum                  :  8;		/*  8..15, 0x0000FF00 */
    FIELD  NR3D_if_hd_hor_mid_f_sum                   :  8;		/* 16..23, 0x00FF0000 */
    FIELD  NR3D_if_hd_hor_hi_f_sum                    :  8;		/* 24..31, 0xFF000000 */
} ISP_NR3D_MCVP_STAT2_T; //	/* 0x15028234 */

typedef union {
    typedef ISP_NR3D_MCVP_STAT2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_MCVP_STAT2_T;

typedef struct {
    FIELD  rsv_0                                      :  9;		/*  0.. 8, 0x000001FF */
    FIELD  NR3D_if_hd_v_hi_normaliz                   :  4;		/*  9..12, 0x00001E00 */
    FIELD  NR3D_if_hd_field_hi_f_dbl                  :  1;		/* 13..13, 0x00002000 */
    FIELD  rsv_14                                     : 18;		/* 14..31, 0xFFFFC000 */
} ISP_NR3D_MCVP_STAT3_T; //	/* 0x15028238 */

typedef union {
    typedef ISP_NR3D_MCVP_STAT3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_MCVP_STAT3_T;

typedef struct {
    FIELD  NR3D_c_fd_quan_value4                      :  8;		/*  0.. 7, 0x000000FF */
    FIELD  NR3D_c_fd_quan_value3                      :  8;		/*  8..15, 0x0000FF00 */
    FIELD  NR3D_c_fd_quan_value2                      :  8;		/* 16..23, 0x00FF0000 */
    FIELD  NR3D_c_fd_quan_value1                      :  8;		/* 24..31, 0xFF000000 */
} ISP_NR3D_MCVP_STAT4_T; //	/* 0x1502823C */

typedef union {
    typedef ISP_NR3D_MCVP_STAT4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_MCVP_STAT4_T;

typedef struct {
    FIELD  NR3D_c_fd_quan_th2                         :  8;		/*  0.. 7, 0x000000FF */
    FIELD  NR3D_c_fd_quan_th1                         :  8;		/*  8..15, 0x0000FF00 */
    FIELD  NR3D_c_fd_quan_value6                      :  8;		/* 16..23, 0x00FF0000 */
    FIELD  NR3D_c_fd_quan_value5                      :  8;		/* 24..31, 0xFF000000 */
} ISP_NR3D_MCVP_STAT5_T; //	/* 0x15028240 */

typedef union {
    typedef ISP_NR3D_MCVP_STAT5_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_MCVP_STAT5_T;

typedef struct {
    FIELD  NR3D_c_fd_mov_pxl_th1                      :  4;		/*  0.. 3, 0x0000000F */
    FIELD  NR3D_c_fd_mov_pxl_th2                      :  4;		/*  4.. 7, 0x000000F0 */
    FIELD  NR3D_c_fd_quan_th5                         :  8;		/*  8..15, 0x0000FF00 */
    FIELD  NR3D_c_fd_quan_th4                         :  8;		/* 16..23, 0x00FF0000 */
    FIELD  NR3D_c_fd_quan_th3                         :  8;		/* 24..31, 0xFF000000 */
} ISP_NR3D_MCVP_STAT6_T; //	/* 0x15028244 */

typedef union {
    typedef ISP_NR3D_MCVP_STAT6_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_MCVP_STAT6_T;

typedef struct {
    FIELD  rsv_0                                      : 15;		/*  0..14, 0x00007FFF */
    FIELD  NR3D_c_fd_prv_frm_small                    :  1;		/* 15..15, 0x00008000 */
    FIELD  NR3D_c_fd_mov_max_thr                      : 16;		/* 16..31, 0xFFFF0000 */
} ISP_NR3D_MCVP_STAT7_T; //	/* 0x15028248 */

typedef union {
    typedef ISP_NR3D_MCVP_STAT7_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_MCVP_STAT7_T;

typedef struct {
    FIELD  NR3D_sta_mov_frm_sum                       : 32;		/*  0..31, 0xFFFFFFFF */
} ISP_NR3D_MCVP_STAT8_T; //	/* 0x1502824C */

typedef union {
    typedef ISP_NR3D_MCVP_STAT8_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_MCVP_STAT8_T;

typedef struct {
    FIELD  NR3D_sta_mov_frm_sum_fix                   : 32;		/*  0..31, 0xFFFFFFFF */
} ISP_NR3D_MCVP_STAT9_T; //	/* 0x15028250 */

typedef union {
    typedef ISP_NR3D_MCVP_STAT9_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_MCVP_STAT9_T;

typedef union {
    enum { COUNT = 34 };
    MUINT32 set[COUNT];
    struct {
        //ISP_NVRAM_NR3D_ENG_CON_T  		eng_con;
        //ISP_NVRAM_NR3D_SIZ_T  			siz;
        //ISP_NVRAM_NR3D_TILE_XY_T  		tile_xy;
        ISP_NVRAM_NR3D_ON_CON_T  			on_con;
        //ISP_NVRAM_NR3D_ON_OFF_T  			on_off;
        //ISP_NVRAM_NR3D_ON_SIZ_T  			on_siz;
        ISP_NVRAM_NR3D_TNR_ENABLE_T  		tnr_enable;
        ISP_NVRAM_NR3D_FLT_CONFIG_T  		flt_config;
        ISP_NVRAM_NR3D_FB_INFO1_T  			fb_info1;
        ISP_NVRAM_NR3D_THR_1_T  			thr_1;
        ISP_NVRAM_NR3D_CURVE_1_T  			curve_1;
        ISP_NVRAM_NR3D_CURVE_2_T  			curve_2;
        ISP_NVRAM_NR3D_CURVE_3_T  			curve_3;
        ISP_NVRAM_NR3D_CURVE_4_T  			curve_4;
        ISP_NVRAM_NR3D_CURVE_5_T  			curve_5;
        ISP_NVRAM_NR3D_CURVE_6_T  			curve_6;
        ISP_NVRAM_NR3D_CURVE_7_T  			curve_7;
        ISP_NVRAM_NR3D_CURVE_8_T  			curve_8;
        ISP_NVRAM_NR3D_CURVE_9_T  			curve_9;
        ISP_NVRAM_NR3D_CURVE_10_T  			curve_10;
        ISP_NVRAM_NR3D_CURVE_11_T  			curve_11;
        ISP_NVRAM_NR3D_CURVE_12_T  			curve_12;
        ISP_NVRAM_NR3D_CURVE_13_T  			curve_13;
        ISP_NVRAM_NR3D_CURVE_14_T  			curve_14;
        ISP_NVRAM_NR3D_CURVE_15_T  			curve_15;
        ISP_NVRAM_NR3D_R2C_1_T  			r2c_1;
        ISP_NVRAM_NR3D_R2C_2_T  			r2c_2;
        //ISP_NVRAM_NR3D_R2C_3_T  			r2c_3;
        //ISP_NVRAM_NR3D_DBG_6_T  			dbg_6;
        //ISP_NVRAM_NR3D_DBG_15_T  			dbg_15;
        //ISP_NVRAM_NR3D_DBG_16_T  			dbg_16;
        //ISP_NVRAM_NR3D_DEMO_1_T  			demo_1;
        //ISP_NVRAM_NR3D_DEMO_2_T  			demo_2;
        //ISP_NVRAM_NR3D_ATPG_T  			atpg;
        //ISP_NVRAM_NR3D_DMY_0_T  			dmy_0;
        //ISP_NVRAM_NR3D_DBG_17_T  			dbg_17;
        //ISP_NVRAM_NR3D_INTERR_T  			interr;
        //ISP_NVRAM_NR3D_FB_INFO2_T  		fb_info2;
        //ISP_NVRAM_NR3D_FB_INFO3_T  		fb_info3;
        //ISP_NVRAM_NR3D_FB_INFO4_T  		fb_info4;
        //ISP_NVRAM_NR3D_DBFISH_T  			dbfish;
        //ISP_NVRAM_NR3D_DBG_9_T  			dbg_9;
        //ISP_NVRAM_NR3D_DBG_10_T  			dbg_10;
        //ISP_NVRAM_NR3D_DBG_11_T  			dbg_11;
        //ISP_NVRAM_NR3D_DBG_12_T  			dbg_12;
        //ISP_NVRAM_NR3D_DBG_7_T  			dbg_7;
        //ISP_NVRAM_NR3D_DMY_1_T  			dmy_1;
        //ISP_NVRAM_NR3D_DMY_2_T  			dmy_2;
        //ISP_NVRAM_NR3D_SAVE_INFO1_T  		save_info1;
        //ISP_NVRAM_NR3D_SAVE_INFO2_T  		save_info2;
        ISP_NVRAM_NR3D_SNR_CURVE_1_T  		snr_curve_1;
        ISP_NVRAM_NR3D_SNR_CURVE_2_T  		snr_curve_2;
        ISP_NVRAM_NR3D_SNR_CURVE_3_T  		snr_curve_3;
        ISP_NVRAM_NR3D_SNR_CURVE_4_T  		snr_curve_4;
        ISP_NVRAM_NR3D_SNR_CURVE_5_T  		snr_curve_5;
        ISP_NVRAM_NR3D_SNR_CURVE_6_T  		snr_curve_6;
        ISP_NVRAM_NR3D_SNR_CURVE_7_T  		snr_curve_7;
        ISP_NVRAM_NR3D_SNR_CONTROL_1_T  	snr_control_1;
        ISP_NVRAM_NR3D_SNR_THR_2_T  		snr_thr_2;
        ISP_NVRAM_NR3D_SNR_THR_3_T  		snr_thr_3;
        ISP_NVRAM_NR3D_SNR_THR_4_T  		snr_thr_4;
        //ISP_NVRAM_NR3D_IN1_CNT_T  		in1_cnt;
        //ISP_NVRAM_NR3D_IN2_CNT_T  		in2_cnt;
        //ISP_NVRAM_NR3D_IN3_CNT_T  		in3_cnt;
        //ISP_NVRAM_NR3D_OUT_CNT_T  		out_cnt;
        //ISP_NVRAM_NR3D_STATUS_T  			status;
        //ISP_NVRAM_NR3D_TILE_LOSS_T  		tile_loss;
        //ISP_NVRAM_NR3D_MCVP_STAT1_T  		mcvp_stat1;
        //ISP_NVRAM_NR3D_MCVP_STAT2_T  		mcvp_stat2;
        //ISP_NVRAM_NR3D_MCVP_STAT3_T  		mcvp_stat3;
        //ISP_NVRAM_NR3D_MCVP_STAT4_T  		mcvp_stat4;
        //ISP_NVRAM_NR3D_MCVP_STAT5_T  		mcvp_stat5;
        //ISP_NVRAM_NR3D_MCVP_STAT6_T  		mcvp_stat6;
        //ISP_NVRAM_NR3D_MCVP_STAT7_T  		mcvp_stat7;
        //ISP_NVRAM_NR3D_MCVP_STAT8_T  		mcvp_stat8;
        //ISP_NVRAM_NR3D_MCVP_STAT9_T  		mcvp_stat9;
    };
} ISP_NVRAM_NR3D_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// LPCNR
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct {
    FIELD  LPCNR_PASS_MODE                            :  1;		/*  0.. 0, 0x00000001 */
    FIELD  LPCNR_LBIT_MODE                            :  1;		/*  1.. 1, 0x00000002 */
    FIELD  LPCNR_lpcnr_reset                          :  1;		/*  2.. 2, 0x00000004 */
    FIELD  LPCNR_lpcnr_dcm_dis                        :  1;		/*  3.. 3, 0x00000008 */
    FIELD  LPCNR_eot_seq_mode                         :  1;		/*  4.. 4, 0x00000010 */
    FIELD  LPCNR_stat_sof_clr                         :  1;		/*  5.. 5, 0x00000020 */
    FIELD  LPCNR_lpcnr_core_en                        :  1;		/*  6.. 6, 0x00000040 */
    FIELD  LPCNR_lpcnr_path_sel                       :  2;		/*  7.. 8, 0x00000180 */
    FIELD  rsv_9                                      : 23;		/*  9..31, 0xFFFFFE00 */
} ISP_LPCNR_TOP_T; //	/* 0x15028840 */

typedef union {
    typedef ISP_LPCNR_TOP_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LPCNR_TOP_T;

typedef struct {
    FIELD  LPCNR_lpcnr_atpg_ct                        :  1;		/*  0.. 0, 0x00000001 */
    FIELD  LPCNR_lpcnr_atpg_ob                        :  1;		/*  1.. 1, 0x00000002 */
    FIELD  rsv_2                                      : 30;		/*  2..31, 0xFFFFFFFC */
} ISP_LPCNR_ATPG_T; //	/* 0x15028844 */

typedef union {
    typedef ISP_LPCNR_ATPG_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LPCNR_ATPG_T;

typedef struct {
    FIELD  LPCNR_IN_HSIZE                             : 13;		/*  0..12, 0x00001FFF */
    FIELD  rsv_13                                     :  3;		/* 13..15, 0x0000E000 */
    FIELD  LPCNR_IN_VSIZE                             : 13;		/* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                     :  3;		/* 29..31, 0xE0000000 */
} ISP_LPCNR_SIZE_T; //	/* 0x15028848 */

typedef union {
    typedef ISP_LPCNR_SIZE_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LPCNR_SIZE_T;

typedef struct {
    FIELD  LPCNR_NR_W00                               :  8;		/*  0.. 7, 0x000000FF */
    FIELD  LPCNR_NR_W01                               :  8;		/*  8..15, 0x0000FF00 */
    FIELD  LPCNR_NR_W02                               :  8;		/* 16..23, 0x00FF0000 */
    FIELD  LPCNR_NR_W03                               :  8;		/* 24..31, 0xFF000000 */
} ISP_LPCNR_NR_W1_T; //	/* 0x1502884C */

typedef union {
    typedef ISP_LPCNR_NR_W1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LPCNR_NR_W1_T;

typedef struct {
    FIELD  LPCNR_NR_W04                               :  8;		/*  0.. 7, 0x000000FF */
    FIELD  LPCNR_NR_W05                               :  8;		/*  8..15, 0x0000FF00 */
    FIELD  LPCNR_NR_W06                               :  8;		/* 16..23, 0x00FF0000 */
    FIELD  LPCNR_NR_W07                               :  8;		/* 24..31, 0xFF000000 */
} ISP_LPCNR_NR_W2_T; //	/* 0x15028850 */

typedef union {
    typedef ISP_LPCNR_NR_W2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LPCNR_NR_W2_T;

typedef struct {
    FIELD  LPCNR_NR_W08                               :  8;		/*  0.. 7, 0x000000FF */
    FIELD  LPCNR_NR_W09                               :  8;		/*  8..15, 0x0000FF00 */
    FIELD  LPCNR_NR_W10                               :  8;		/* 16..23, 0x00FF0000 */
    FIELD  LPCNR_NR_W11                               :  8;		/* 24..31, 0xFF000000 */
} ISP_LPCNR_NR_W3_T; //	/* 0x15028854 */

typedef union {
    typedef ISP_LPCNR_NR_W3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LPCNR_NR_W3_T;

typedef struct {
    FIELD  LPCNR_NR_W12                               :  8;		/*  0.. 7, 0x000000FF */
    FIELD  LPCNR_NR_W13                               :  8;		/*  8..15, 0x0000FF00 */
    FIELD  LPCNR_NR_W14                               :  8;		/* 16..23, 0x00FF0000 */
    FIELD  LPCNR_NR_W15                               :  8;		/* 24..31, 0xFF000000 */
} ISP_LPCNR_NR_W4_T; //	/* 0x15028858 */

typedef union {
    typedef ISP_LPCNR_NR_W4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LPCNR_NR_W4_T;

typedef struct {
    FIELD  LPCNR_NR_W16                               :  8;		/*  0.. 7, 0x000000FF */
    FIELD  LPCNR_NR_W17                               :  8;		/*  8..15, 0x0000FF00 */
    FIELD  LPCNR_NR_W18                               :  8;		/* 16..23, 0x00FF0000 */
    FIELD  LPCNR_NR_W19                               :  8;		/* 24..31, 0xFF000000 */
} ISP_LPCNR_NR_W5_T; //	/* 0x1502885C */

typedef union {
    typedef ISP_LPCNR_NR_W5_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LPCNR_NR_W5_T;

typedef struct {
    FIELD  LPCNR_NR_W20                               :  8;		/*  0.. 7, 0x000000FF */
    FIELD  LPCNR_NR_W21                               :  8;		/*  8..15, 0x0000FF00 */
    FIELD  LPCNR_NR_W22                               :  8;		/* 16..23, 0x00FF0000 */
    FIELD  LPCNR_NR_W23                               :  8;		/* 24..31, 0xFF000000 */
} ISP_LPCNR_NR_W6_T; //	/* 0x15028860 */

typedef union {
    typedef ISP_LPCNR_NR_W6_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LPCNR_NR_W6_T;

typedef struct {
    FIELD  LPCNR_NR_W24                               :  8;		/*  0.. 7, 0x000000FF */
    FIELD  LPCNR_NR_W25                               :  8;		/*  8..15, 0x0000FF00 */
    FIELD  LPCNR_NR_W26                               :  8;		/* 16..23, 0x00FF0000 */
    FIELD  LPCNR_NR_W27                               :  8;		/* 24..31, 0xFF000000 */
} ISP_LPCNR_NR_W7_T; //	/* 0x15028864 */

typedef union {
    typedef ISP_LPCNR_NR_W7_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LPCNR_NR_W7_T;

typedef struct {
    FIELD  LPCNR_NR_W28                               :  8;		/*  0.. 7, 0x000000FF */
    FIELD  LPCNR_NR_W29                               :  8;		/*  8..15, 0x0000FF00 */
    FIELD  LPCNR_NR_W30                               :  8;		/* 16..23, 0x00FF0000 */
    FIELD  LPCNR_NR_W31                               :  8;		/* 24..31, 0xFF000000 */
} ISP_LPCNR_NR_W8_T; //	/* 0x15028868 */

typedef union {
    typedef ISP_LPCNR_NR_W8_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LPCNR_NR_W8_T;

typedef struct {
    FIELD  LPCNR_NR_W32                               :  8;		/*  0.. 7, 0x000000FF */
    FIELD  LPCNR_NR_W33                               :  8;		/*  8..15, 0x0000FF00 */
    FIELD  LPCNR_NR_W34                               :  8;		/* 16..23, 0x00FF0000 */
    FIELD  LPCNR_NR_W35                               :  8;		/* 24..31, 0xFF000000 */
} ISP_LPCNR_NR_W9_T; //	/* 0x1502886C */

typedef union {
    typedef ISP_LPCNR_NR_W9_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LPCNR_NR_W9_T;

typedef struct {
    FIELD  LPCNR_NR_W36                               :  8;		/*  0.. 7, 0x000000FF */
    FIELD  LPCNR_NR_W37                               :  8;		/*  8..15, 0x0000FF00 */
    FIELD  LPCNR_NR_W38                               :  8;		/* 16..23, 0x00FF0000 */
    FIELD  LPCNR_NR_W39                               :  8;		/* 24..31, 0xFF000000 */
} ISP_LPCNR_NR_W10_T; //	/* 0x15028870 */

typedef union {
    typedef ISP_LPCNR_NR_W10_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LPCNR_NR_W10_T;

typedef struct {
    FIELD  LPCNR_NR_W40                               :  8;		/*  0.. 7, 0x000000FF */
    FIELD  LPCNR_NR_W41                               :  8;		/*  8..15, 0x0000FF00 */
    FIELD  LPCNR_NR_W42                               :  8;		/* 16..23, 0x00FF0000 */
    FIELD  LPCNR_NR_W43                               :  8;		/* 24..31, 0xFF000000 */
} ISP_LPCNR_NR_W11_T; //	/* 0x15028874 */

typedef union {
    typedef ISP_LPCNR_NR_W11_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LPCNR_NR_W11_T;

typedef struct {
    FIELD  LPCNR_NR_W44                               :  8;		/*  0.. 7, 0x000000FF */
    FIELD  LPCNR_NR_W45                               :  8;		/*  8..15, 0x0000FF00 */
    FIELD  LPCNR_NR_W46                               :  8;		/* 16..23, 0x00FF0000 */
    FIELD  LPCNR_NR_W47                               :  8;		/* 24..31, 0xFF000000 */
} ISP_LPCNR_NR_W12_T; //	/* 0x15028878 */

typedef union {
    typedef ISP_LPCNR_NR_W12_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LPCNR_NR_W12_T;

typedef struct {
    FIELD  LPCNR_NR_W48                               :  8;		/*  0.. 7, 0x000000FF */
    FIELD  LPCNR_NR_W49                               :  8;		/*  8..15, 0x0000FF00 */
    FIELD  LPCNR_NR_W50                               :  8;		/* 16..23, 0x00FF0000 */
    FIELD  LPCNR_NR_W51                               :  8;		/* 24..31, 0xFF000000 */
} ISP_LPCNR_NR_W13_T; //	/* 0x1502887C */

typedef union {
    typedef ISP_LPCNR_NR_W13_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LPCNR_NR_W13_T;

typedef struct {
    FIELD  LPCNR_NR_W52                               :  8;		/*  0.. 7, 0x000000FF */
    FIELD  LPCNR_NR_W53                               :  8;		/*  8..15, 0x0000FF00 */
    FIELD  rsv_16                                     : 16;		/* 16..31, 0xFFFF0000 */
} ISP_LPCNR_NR_W14_T; //	/* 0x15028880 */

typedef union {
    typedef ISP_LPCNR_NR_W14_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LPCNR_NR_W14_T;

typedef struct {
    FIELD  LPCNR_NR_SD                                : 16;		/*  0..15, 0x0000FFFF */
    FIELD  LPCNR_SM_TH                                :  5;		/* 16..20, 0x001F0000 */
    FIELD  rsv_21                                     : 11;		/* 21..31, 0xFFE00000 */
} ISP_LPCNR_SM_T; //	/* 0x15028884 */

typedef union {
    typedef ISP_LPCNR_SM_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LPCNR_SM_T;

typedef struct {
    FIELD  LPCNR_BLD_TH                               :  8;		/*  0.. 7, 0x000000FF */
    FIELD  LPCNR_BLD_SP                               :  5;		/*  8..12, 0x00001F00 */
    FIELD  rsv_13                                     :  3;		/* 13..15, 0x0000E000 */
    FIELD  LPCNR_BLD_W0                               :  7;		/* 16..22, 0x007F0000 */
    FIELD  rsv_23                                     :  1;		/* 23..23, 0x00800000 */
    FIELD  LPCNR_BLD_W1                               :  7;		/* 24..30, 0x7F000000 */
    FIELD  rsv_31                                     :  1;		/* 31..31, 0x80000000 */
} ISP_LPCNR_BLD_T; //	/* 0x15028888 */

typedef union {
    typedef ISP_LPCNR_BLD_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LPCNR_BLD_T;

typedef struct {
    FIELD  LPCNR_NTRL_Y_TH0                           :  8;		/*  0.. 7, 0x000000FF */
    FIELD  LPCNR_NTRL_Y_TH1                           :  8;		/*  8..15, 0x0000FF00 */
    FIELD  LPCNR_NTRL_Y_SP0                           :  4;		/* 16..19, 0x000F0000 */
    FIELD  LPCNR_NTRL_Y_SP1                           :  4;		/* 20..23, 0x00F00000 */
    FIELD  rsv_24                                     :  8;		/* 24..31, 0xFF000000 */
} ISP_LPCNR_NTRL_Y_T; //	/* 0x1502888C */

typedef union {
    typedef ISP_LPCNR_NTRL_Y_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LPCNR_NTRL_Y_T;

typedef struct {
    FIELD  LPCNR_NTRL_C_TH                            :  8;		/*  0.. 7, 0x000000FF */
    FIELD  LPCNR_NTRL_C_SP                            :  4;		/*  8..11, 0x00000F00 */
    FIELD  rsv_12                                     : 20;		/* 12..31, 0xFFFFF000 */
} ISP_LPCNR_NTRL_C_T; //	/* 0x15028890 */

typedef union {
    typedef ISP_LPCNR_NTRL_C_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LPCNR_NTRL_C_T;

typedef struct {
    FIELD  LPCNR_lpcnr_dummy                          : 32;		/*  0..31, 0xFFFFFFFF */
} ISP_LPCNR_DUMMY_T; //	/* 0x15028894 */

typedef union {
    typedef ISP_LPCNR_DUMMY_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LPCNR_DUMMY_T;

typedef struct {
    FIELD  LPCNR_in1_pxl_cnt                          : 13;		/*  0..12, 0x00001FFF */
    FIELD  rsv_13                                     :  3;		/* 13..15, 0x0000E000 */
    FIELD  LPCNR_in1_line_cnt                         : 13;		/* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                     :  3;		/* 29..31, 0xE0000000 */
} ISP_LPCNR_IN1_CNT_T; //	/* 0x15028898 */

typedef union {
    typedef ISP_LPCNR_IN1_CNT_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LPCNR_IN1_CNT_T;

typedef struct {
    FIELD  LPCNR_in2_pxl_cnt                          : 13;		/*  0..12, 0x00001FFF */
    FIELD  rsv_13                                     :  3;		/* 13..15, 0x0000E000 */
    FIELD  LPCNR_in2_line_cnt                         : 13;		/* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                     :  3;		/* 29..31, 0xE0000000 */
} ISP_LPCNR_IN2_CNT_T; //	/* 0x1502889C */

typedef union {
    typedef ISP_LPCNR_IN2_CNT_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LPCNR_IN2_CNT_T;

typedef struct {
    FIELD  LPCNR_in3_pxl_cnt                          : 13;		/*  0..12, 0x00001FFF */
    FIELD  rsv_13                                     :  3;		/* 13..15, 0x0000E000 */
    FIELD  LPCNR_in3_line_cnt                         : 13;		/* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                     :  3;		/* 29..31, 0xE0000000 */
} ISP_LPCNR_IN3_CNT_T; //	/* 0x150288A0 */

typedef union {
    typedef ISP_LPCNR_IN3_CNT_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LPCNR_IN3_CNT_T;

typedef struct {
    FIELD  LPCNR_out_pxl_cnt                          : 13;		/*  0..12, 0x00001FFF */
    FIELD  rsv_13                                     :  3;		/* 13..15, 0x0000E000 */
    FIELD  LPCNR_out_line_cnt                         : 13;		/* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                     :  3;		/* 29..31, 0xE0000000 */
} ISP_LPCNR_OUT_CNT_T; //	/* 0x150288A4 */

typedef union {
    typedef ISP_LPCNR_OUT_CNT_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LPCNR_OUT_CNT_T;

typedef struct {
    FIELD  LPCNR_if1_end                              :  1;		/*  0.. 0, 0x00000001 */
    FIELD  LPCNR_if2_end                              :  1;		/*  1.. 1, 0x00000002 */
    FIELD  LPCNR_if3_end                              :  1;		/*  2.. 2, 0x00000004 */
    FIELD  LPCNR_of_end                               :  1;		/*  3.. 3, 0x00000008 */
    FIELD  rsv_4                                      : 28;		/*  4..31, 0xFFFFFFF0 */
} ISP_LPCNR_STATUS_T; //	/* 0x150288A8 */

typedef union {
    typedef ISP_LPCNR_STATUS_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LPCNR_STATUS_T;

typedef union {
    enum { COUNT = 18};
    MUINT32 set[COUNT];
    struct {
        //ISP_NVRAM_LPCNR_TOP_T  	    	top;
        //ISP_NVRAM_LPCNR_ATPG_T  			atpg;
        //ISP_NVRAM_LPCNR_SIZE_T  			size;
        ISP_NVRAM_LPCNR_NR_W1_T  			nr_w1;
        ISP_NVRAM_LPCNR_NR_W2_T  			nr_w2;
        ISP_NVRAM_LPCNR_NR_W3_T  			nr_w3;
        ISP_NVRAM_LPCNR_NR_W4_T  			nr_w4;
        ISP_NVRAM_LPCNR_NR_W5_T  			nr_w5;
        ISP_NVRAM_LPCNR_NR_W6_T  			nr_w6;
        ISP_NVRAM_LPCNR_NR_W7_T  			nr_w7;
        ISP_NVRAM_LPCNR_NR_W8_T  			nr_w8;
        ISP_NVRAM_LPCNR_NR_W9_T  			nr_w9;
        ISP_NVRAM_LPCNR_NR_W10_T  			nr_w10;
        ISP_NVRAM_LPCNR_NR_W11_T  			nr_w11;
        ISP_NVRAM_LPCNR_NR_W12_T  			nr_w12;
        ISP_NVRAM_LPCNR_NR_W13_T  			nr_w13;
        ISP_NVRAM_LPCNR_NR_W14_T  			nr_w14;
        ISP_NVRAM_LPCNR_SM_T  		    	sm;
        ISP_NVRAM_LPCNR_BLD_T  		    	bld;
        ISP_NVRAM_LPCNR_NTRL_Y_T  			ntrl_y;
        ISP_NVRAM_LPCNR_NTRL_C_T  			ntrl_c;
        //ISP_NVRAM_LPCNR_DUMMY_T  			dummy;
        //ISP_NVRAM_LPCNR_IN1_CNT_T  		in1_cnt;
        //ISP_NVRAM_LPCNR_IN2_CNT_T  		in2_cnt;
        //ISP_NVRAM_LPCNR_IN3_CNT_T  		in3_cnt;
        //ISP_NVRAM_LPCNR_OUT_CNT_T  		out_cnt;
        //ISP_NVRAM_LPCNR_STATUS_T  		status;
    };
} ISP_NVRAM_LPCNR_T;

#if 0
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// LCES
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct {
    FIELD  LCES_START_I                                              : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  LCES_START_J                                              : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_LCES_START_T; // /* 0x1A010800 */

typedef union {
    typedef ISP_LCES_START_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCES_START_T;

typedef struct {
    FIELD  LCES_IN_HT                                                : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  LCES_IN_WD                                                : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_LCES_IN_SIZE_T; // /* 0x1A010804 */

typedef union {
    typedef ISP_LCES_IN_SIZE_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCES_IN_SIZE_T;

typedef struct {
    FIELD  LCES_OUT_HT                                               :  9;      /*  0.. 8, 0x000001FF */
    FIELD  rsv_9                                                     :  7;      /*  9..15, 0x0000FE00 */
    FIELD  LCES_OUT_WD                                               :  9;      /* 16..24, 0x01FF0000 */
    FIELD  rsv_25                                                    :  7;      /* 25..31, 0xFE000000 */
} ISP_LCES_OUT_SIZE_T; // /* 0x1A010808 */

typedef union {
    typedef ISP_LCES_OUT_SIZE_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCES_OUT_SIZE_T;

typedef struct {
    FIELD  LCES_LOG                                                  :  1;      /*  0.. 0, 0x00000001 */
    FIELD  rsv_1                                                     : 31;      /*  1..31, 0xFFFFFFFE */
} ISP_LCES_LOG_T; // /* 0x1A01080C */

typedef union {
    typedef ISP_LCES_LOG_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCES_LOG_T;

typedef struct {
    FIELD  LCES_LRZR_X                                               : 20;      /*  0..19, 0x000FFFFF */
    FIELD  rsv_20                                                    : 12;      /* 20..31, 0xFFF00000 */
} ISP_LCES_LRZRX_T; // /* 0x1A010810 */

typedef union {
    typedef ISP_LCES_LRZRX_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCES_LRZRX_T;

typedef struct {
    FIELD  LCES_LRZR_Y                                               : 20;      /*  0..19, 0x000FFFFF */
    FIELD  rsv_20                                                    : 12;      /* 20..31, 0xFFF00000 */
} ISP_LCES_LRZRY_T; // /* 0x1A010814 */

typedef union {
    typedef ISP_LCES_LRZRY_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCES_LRZRY_T;

typedef struct {
    FIELD  LCES_FLC_OFST_G                                           : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  LCES_FLC_OFST_R                                           : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_LCES_FLC0_T; // /* 0x1A010820 */

typedef union {
    typedef ISP_LCES_FLC0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCES_FLC0_T;

typedef struct {
    FIELD  LCES_FLC_GAIN_R                                           : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  LCES_FLC_OFST_B                                           : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_LCES_FLC1_T; // /* 0x1A010824 */

typedef union {
    typedef ISP_LCES_FLC1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCES_FLC1_T;

typedef struct {
    FIELD  LCES_FLC_GAIN_B                                           : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  LCES_FLC_GAIN_G                                           : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_LCES_FLC2_T; // /* 0x1A010828 */

typedef union {
    typedef ISP_LCES_FLC2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCES_FLC2_T;

typedef struct {
    FIELD  LCES_G2G_CNV_01                                           : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  LCES_G2G_CNV_00                                           : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_LCES_G2G0_T; // /* 0x1A01082C */

typedef union {
    typedef ISP_LCES_G2G0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCES_G2G0_T;

typedef struct {
    FIELD  LCES_G2G_CNV_10                                           : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  LCES_G2G_CNV_02                                           : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_LCES_G2G1_T; // /* 0x1A010830 */

typedef union {
    typedef ISP_LCES_G2G1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCES_G2G1_T;

typedef struct {
    FIELD  LCES_G2G_CNV_12                                           : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  LCES_G2G_CNV_11                                           : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_LCES_G2G2_T; // /* 0x1A010834 */

typedef union {
    typedef ISP_LCES_G2G2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCES_G2G2_T;

typedef struct {
    FIELD  LCES_G2G_CNV_21                                           : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  LCES_G2G_CNV_20                                           : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_LCES_G2G3_T; // /* 0x1A010838 */

typedef union {
    typedef ISP_LCES_G2G3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCES_G2G3_T;

typedef struct {
    FIELD  LCES_G2G_ACC                                              :  4;      /*  0.. 3, 0x0000000F */
    FIELD  rsv_4                                                     : 12;      /*  4..15, 0x0000FFF0 */
    FIELD  LCES_G2G_CNV_22                                           : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_LCES_G2G4_T; // /* 0x1A01083C */

typedef union {
    typedef ISP_LCES_G2G4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCES_G2G4_T;

typedef struct {
    FIELD  LCES_LPF_TH                                               : 12;      /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                                    :  4;      /* 12..15, 0x0000F000 */
    FIELD  LCES_LPF_EN                                               :  1;      /* 16..16, 0x00010000 */
    FIELD  rsv_17                                                    : 15;      /* 17..31, 0xFFFE0000 */
} ISP_LCES_LPF_T; // /* 0x1A010840 */

typedef union {
    typedef ISP_LCES_LPF_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCES_LPF_T;

typedef struct {
    FIELD  LCES_CCUB                                                 : 12;      /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                                    :  2;      /* 12..13, 0x00003000 */
    FIELD  LCES_CCLB                                                 : 12;      /* 14..25, 0x03FFC000 */
    FIELD  rsv_26                                                    :  2;      /* 26..27, 0x0C000000 */
    FIELD  LCES_ACT_HISTO                                            :  2;      /* 28..29, 0x30000000 */
    FIELD  rsv_30                                                    :  1;      /* 30..30, 0x40000000 */
    FIELD  LCES_HIST_OUT_EN                                          :  1;      /* 31..31, 0x80000000 */
} ISP_LCES_AH_T; // /* 0x1A010844 */

typedef union {
    typedef ISP_LCES_AH_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCES_AH_T;

typedef union {
    enum { COUNT = 16 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_LCES_START_T  		start;
        ISP_NVRAM_LCES_IN_SIZE_T  		in_size;
        ISP_NVRAM_LCES_OUT_SIZE_T  		out_size;
        ISP_NVRAM_LCES_LOG_T  			log;
        ISP_NVRAM_LCES_LRZRX_T  		lrzrx;
        ISP_NVRAM_LCES_LRZRY_T  		lrzry;
        ISP_NVRAM_LCES_FLC0_T  			flc0;
        ISP_NVRAM_LCES_FLC1_T  			flc1;
        ISP_NVRAM_LCES_FLC2_T  			flc2;
        ISP_NVRAM_LCES_G2G0_T  			g2g0;
        ISP_NVRAM_LCES_G2G1_T  			g2g1;
        ISP_NVRAM_LCES_G2G2_T  			g2g2;
        ISP_NVRAM_LCES_G2G3_T  			g2g3;
        ISP_NVRAM_LCES_G2G4_T  			g2g4;
        ISP_NVRAM_LCES_LPF_T  			lpf;
        ISP_NVRAM_LCES_AH_T  			ah;
    };
} ISP_NVRAM_LCES_T;
#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// LCE
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct {
    FIELD  LCE_GLOB                                   :  1;		/*  0.. 0, 0x00000001 */
    FIELD  LCE_GLOB_VHALF                             :  7;		/*  1.. 7, 0x000000FE */
    FIELD  LCE_EDGE                                   :  4;		/*  8..11, 0x00000F00 */
    FIELD  rsv_12                                     : 16;		/* 12..27, 0x0FFFF000 */
    FIELD  LCE_RESERVED                               :  4;		/* 28..31, 0xF0000000 */
} ISP_LCE_CON_T; //	/* 0x15026A00 */

typedef union {
    typedef ISP_LCE_CON_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE_CON_T;

typedef struct {
    FIELD  LCE_BCMK_Y                                 : 15;		/*  0..14, 0x00007FFF */
    FIELD  rsv_15                                     :  1;		/* 15..15, 0x00008000 */
    FIELD  LCE_BCMK_X                                 : 15;		/* 16..30, 0x7FFF0000 */
    FIELD  rsv_31                                     :  1;		/* 31..31, 0x80000000 */
} ISP_LCE_ZR_T; //	/* 0x15026A04 */

typedef union {
    typedef ISP_LCE_ZR_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE_ZR_T;

typedef struct {
    FIELD  LCE_SLM_HT                                 :  9;		/*  0.. 8, 0x000001FF */
    FIELD  rsv_9                                      :  7;		/*  9..15, 0x0000FE00 */
    FIELD  LCE_SLM_WD                                 :  9;		/* 16..24, 0x01FF0000 */
    FIELD  rsv_25                                     :  7;		/* 25..31, 0xFE000000 */
} ISP_LCE_SLM_SIZE_T; //	/* 0x15026A08 */

typedef union {
    typedef ISP_LCE_SLM_SIZE_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE_SLM_SIZE_T;

typedef struct {
    FIELD  LCE_OFST_Y                                 : 15;		/*  0..14, 0x00007FFF */
    FIELD  rsv_15                                     :  1;		/* 15..15, 0x00008000 */
    FIELD  LCE_OFST_X                                 : 15;		/* 16..30, 0x7FFF0000 */
    FIELD  rsv_31                                     :  1;		/* 31..31, 0x80000000 */
} ISP_LCE_OFST_T; //	/* 0x15026A0C */

typedef union {
    typedef ISP_LCE_OFST_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE_OFST_T;

typedef struct {
    FIELD  LCE_BIAS_Y                                 :  9;		/*  0.. 8, 0x000001FF */
    FIELD  rsv_9                                      :  6;		/*  9..14, 0x00007E00 */
    FIELD  LCE_BIAS_X                                 :  9;		/* 15..23, 0x00FF8000 */
    FIELD  rsv_24                                     :  8;		/* 24..31, 0xFF000000 */
} ISP_LCE_BIAS_T; //	/* 0x15026A10 */

typedef union {
    typedef ISP_LCE_BIAS_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE_BIAS_T;

typedef struct {
    FIELD  LCE_IMAGE_HT                               : 14;		/*  0..13, 0x00003FFF */
    FIELD  rsv_14                                     :  2;		/* 14..15, 0x0000C000 */
    FIELD  LCE_IMAGE_WD                               : 14;		/* 16..29, 0x3FFF0000 */
    FIELD  rsv_30                                     :  2;		/* 30..31, 0xC0000000 */
} ISP_LCE_IMAGE_SIZE_T; //	/* 0x15026A14 */

typedef union {
    typedef ISP_LCE_IMAGE_SIZE_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE_IMAGE_SIZE_T;

typedef struct {
    FIELD  rsv_0                                      : 24;		/*  0..23, 0x00FFFFFF */
    FIELD  LCE_GLOB_POS                               :  1;		/* 24..24, 0x01000000 */
    FIELD  LCE_GLOB_TONE                              :  1;		/* 25..25, 0x02000000 */
    FIELD  LCE_LC_TONE                                :  1;		/* 26..26, 0x04000000 */
    FIELD  rsv_27                                     :  5;		/* 27..31, 0xF8000000 */
} ISP_LCE_GLOBAL_T; //	/* 0x15026A18 */

typedef union {
    typedef ISP_LCE_GLOBAL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE_GLOBAL_T;

typedef struct {
    FIELD  LCE_CEN_END                                : 12;		/*  0..11, 0x00000FFF */
    FIELD  LCE_CEN_BLD_WT                             :  4;		/* 12..15, 0x0000F000 */
    FIELD  LCE_CORING_TH                              : 12;		/* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                     :  3;		/* 28..30, 0x70000000 */
    FIELD  LCE_CEN_SLOPE_DEP_EN                       :  1;		/* 31..31, 0x80000000 */
} ISP_LCE_CEN_PARA0_T; //	/* 0x15026A1C */

typedef union {
    typedef ISP_LCE_CEN_PARA0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE_CEN_PARA0_T;

typedef struct {
    FIELD  LCE_CEN_MAX_SLOPE                          :  8;		/*  0.. 7, 0x000000FF */
    FIELD  rsv_8                                      :  8;		/*  8..15, 0x0000FF00 */
    FIELD  LCE_CEN_MIN_SLOPE                          :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                     :  8;		/* 24..31, 0xFF000000 */
} ISP_LCE_CEN_PARA1_T; //	/* 0x15026A20 */

typedef union {
    typedef ISP_LCE_CEN_PARA1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE_CEN_PARA1_T;

typedef struct {
    FIELD  LCE_CEN_MAX_SLOPE_N                        :  8;		/*  0.. 7, 0x000000FF */
    FIELD  rsv_8                                      :  8;		/*  8..15, 0x0000FF00 */
    FIELD  LCE_CEN_MIN_SLOPE_N                        :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                     :  8;		/* 24..31, 0xFF000000 */
} ISP_LCE_CEN_PARA2_T; //	/* 0x15026A24 */

typedef union {
    typedef ISP_LCE_CEN_PARA2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE_CEN_PARA2_T;

typedef struct {
    FIELD  LCE_BIL_TH2                                : 12;		/*  0..11, 0x00000FFF */
    FIELD  rsv_12                                     :  4;		/* 12..15, 0x0000F000 */
    FIELD  LCE_BIL_TH1                                : 12;		/* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                     :  4;		/* 28..31, 0xF0000000 */
} ISP_LCE_BIL_TH0_T; //	/* 0x15026A28 */

typedef union {
    typedef ISP_LCE_BIL_TH0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE_BIL_TH0_T;

typedef struct {
    FIELD  LCE_BIL_TH4                                : 12;		/*  0..11, 0x00000FFF */
    FIELD  rsv_12                                     :  4;		/* 12..15, 0x0000F000 */
    FIELD  LCE_BIL_TH3                                : 12;		/* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                     :  4;		/* 28..31, 0xF0000000 */
} ISP_LCE_BIL_TH1_T; //	/* 0x15026A2C */

typedef union {
    typedef ISP_LCE_BIL_TH1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE_BIL_TH1_T;

typedef struct {
    FIELD  LCE_TC_P1                                  : 12;		/*  0..11, 0x00000FFF */
    FIELD  rsv_12                                     :  4;		/* 12..15, 0x0000F000 */
    FIELD  LCE_TC_P0                                  : 12;		/* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                     :  4;		/* 28..31, 0xF0000000 */
} ISP_LCE_TM_PARA0_T; //	/* 0x15026A30 */

typedef union {
    typedef ISP_LCE_TM_PARA0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE_TM_PARA0_T;

typedef struct {
    FIELD  LCE_TC_P250                                : 12;		/*  0..11, 0x00000FFF */
    FIELD  rsv_12                                     :  4;		/* 12..15, 0x0000F000 */
    FIELD  LCE_TC_P50                                 : 12;		/* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                     :  4;		/* 28..31, 0xF0000000 */
} ISP_LCE_TM_PARA1_T; //	/* 0x15026A34 */

typedef union {
    typedef ISP_LCE_TM_PARA1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE_TM_PARA1_T;

typedef struct {
    FIELD  LCE_TC_P750                                : 12;		/*  0..11, 0x00000FFF */
    FIELD  rsv_12                                     :  4;		/* 12..15, 0x0000F000 */
    FIELD  LCE_TC_P500                                : 12;		/* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                     :  4;		/* 28..31, 0xF0000000 */
} ISP_LCE_TM_PARA2_T; //	/* 0x15026A38 */

typedef union {
    typedef ISP_LCE_TM_PARA2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE_TM_PARA2_T;

typedef struct {
    FIELD  LCE_TC_P999                                : 12;		/*  0..11, 0x00000FFF */
    FIELD  rsv_12                                     :  4;		/* 12..15, 0x0000F000 */
    FIELD  LCE_TC_P950                                : 12;		/* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                     :  4;		/* 28..31, 0xF0000000 */
} ISP_LCE_TM_PARA3_T; //	/* 0x15026A3C */

typedef union {
    typedef ISP_LCE_TM_PARA3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE_TM_PARA3_T;

typedef struct {
    FIELD  LCE_TC_O1                                  : 12;		/*  0..11, 0x00000FFF */
    FIELD  rsv_12                                     :  4;		/* 12..15, 0x0000F000 */
    FIELD  LCE_TC_O0                                  : 12;		/* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                     :  4;		/* 28..31, 0xF0000000 */
} ISP_LCE_TM_PARA4_T; //	/* 0x15026A40 */

typedef union {
    typedef ISP_LCE_TM_PARA4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE_TM_PARA4_T;

typedef struct {
    FIELD  LCE_TC_O250                                : 12;		/*  0..11, 0x00000FFF */
    FIELD  rsv_12                                     :  4;		/* 12..15, 0x0000F000 */
    FIELD  LCE_TC_O50                                 : 12;		/* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                     :  4;		/* 28..31, 0xF0000000 */
} ISP_LCE_TM_PARA5_T; //	/* 0x15026A44 */

typedef union {
    typedef ISP_LCE_TM_PARA5_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE_TM_PARA5_T;

typedef struct {
    FIELD  LCE_TC_O750                                : 12;		/*  0..11, 0x00000FFF */
    FIELD  rsv_12                                     :  4;		/* 12..15, 0x0000F000 */
    FIELD  LCE_TC_O500                                : 12;		/* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                     :  4;		/* 28..31, 0xF0000000 */
} ISP_LCE_TM_PARA6_T; //	/* 0x15026A48 */

typedef union {
    typedef ISP_LCE_TM_PARA6_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE_TM_PARA6_T;

typedef struct {
    FIELD  LCE_TC_O999                                : 12;		/*  0..11, 0x00000FFF */
    FIELD  rsv_12                                     :  4;		/* 12..15, 0x0000F000 */
    FIELD  LCE_TC_O950                                : 12;		/* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                     :  4;		/* 28..31, 0xF0000000 */
} ISP_LCE_TM_PARA7_T; //	/* 0x15026A4C */

typedef union {
    typedef ISP_LCE_TM_PARA7_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE_TM_PARA7_T;

typedef struct {
    FIELD  LCE_HLR_TC_P1                              : 12;		/*  0..11, 0x00000FFF */
    FIELD  rsv_12                                     :  4;		/* 12..15, 0x0000F000 */
    FIELD  LCE_HLR_TC_P0                              : 12;		/* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                     :  4;		/* 28..31, 0xF0000000 */
} ISP_LCE_HLR_PARA0_T; //	/* 0x15026A50 */

typedef union {
    typedef ISP_LCE_HLR_PARA0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE_HLR_PARA0_T;

typedef struct {
    FIELD  LCE_HLR_TC_P250                            : 12;		/*  0..11, 0x00000FFF */
    FIELD  rsv_12                                     :  4;		/* 12..15, 0x0000F000 */
    FIELD  LCE_HLR_TC_P50                             : 12;		/* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                     :  4;		/* 28..31, 0xF0000000 */
} ISP_LCE_HLR_PARA1_T; //	/* 0x15026A54 */

typedef union {
    typedef ISP_LCE_HLR_PARA1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE_HLR_PARA1_T;

typedef struct {
    FIELD  LCE_HLR_TC_P750                            : 12;		/*  0..11, 0x00000FFF */
    FIELD  rsv_12                                     :  4;		/* 12..15, 0x0000F000 */
    FIELD  LCE_HLR_TC_P500                            : 12;		/* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                     :  4;		/* 28..31, 0xF0000000 */
} ISP_LCE_HLR_PARA2_T; //	/* 0x15026A58 */

typedef union {
    typedef ISP_LCE_HLR_PARA2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE_HLR_PARA2_T;

typedef struct {
    FIELD  LCE_HLR_TC_P999                            : 12;		/*  0..11, 0x00000FFF */
    FIELD  rsv_12                                     :  4;		/* 12..15, 0x0000F000 */
    FIELD  LCE_HLR_TC_P950                            : 12;		/* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                     :  4;		/* 28..31, 0xF0000000 */
} ISP_LCE_HLR_PARA3_T; //	/* 0x15026A5C */

typedef union {
    typedef ISP_LCE_HLR_PARA3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE_HLR_PARA3_T;

typedef struct {
    FIELD  LCE_HLR_TC_O1                              : 12;		/*  0..11, 0x00000FFF */
    FIELD  rsv_12                                     :  4;		/* 12..15, 0x0000F000 */
    FIELD  LCE_HLR_TC_O0                              : 12;		/* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                     :  4;		/* 28..31, 0xF0000000 */
} ISP_LCE_HLR_PARA4_T; //	/* 0x15026A60 */

typedef union {
    typedef ISP_LCE_HLR_PARA4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE_HLR_PARA4_T;

typedef struct {
    FIELD  LCE_HLR_TC_O250                            : 12;		/*  0..11, 0x00000FFF */
    FIELD  rsv_12                                     :  4;		/* 12..15, 0x0000F000 */
    FIELD  LCE_HLR_TC_O50                             : 12;		/* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                     :  4;		/* 28..31, 0xF0000000 */
} ISP_LCE_HLR_PARA5_T; //	/* 0x15026A64 */

typedef union {
    typedef ISP_LCE_HLR_PARA5_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE_HLR_PARA5_T;

typedef struct {
    FIELD  LCE_HLR_TC_O750                            : 12;		/*  0..11, 0x00000FFF */
    FIELD  rsv_12                                     :  4;		/* 12..15, 0x0000F000 */
    FIELD  LCE_HLR_TC_O500                            : 12;		/* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                     :  4;		/* 28..31, 0xF0000000 */
} ISP_LCE_HLR_PARA6_T; //	/* 0x15026A68 */

typedef union {
    typedef ISP_LCE_HLR_PARA6_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE_HLR_PARA6_T;

typedef struct {
    FIELD  LCE_HLR_TC_O999                            : 12;		/*  0..11, 0x00000FFF */
    FIELD  rsv_12                                     :  4;		/* 12..15, 0x0000F000 */
    FIELD  LCE_HLR_TC_O950                            : 12;		/* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                     :  4;		/* 28..31, 0xF0000000 */
} ISP_LCE_HLR_PARA7_T; //	/* 0x15026A6C */

typedef union {
    typedef ISP_LCE_HLR_PARA7_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE_HLR_PARA7_T;

typedef struct {
    FIELD  LCE_TCHL_DTH1                              :  8;		/*  0.. 7, 0x000000FF */
    FIELD  rsv_8                                      :  8;		/*  8..15, 0x0000FF00 */
    FIELD  LCE_TCHL_MTH                               :  8;		/* 16..23, 0x00FF0000 */
    FIELD  LCE_TCHL_MGAIN                             :  4;		/* 24..27, 0x0F000000 */
    FIELD  rsv_28                                     :  3;		/* 28..30, 0x70000000 */
    FIELD  LCE_TCHL_EN                                :  1;		/* 31..31, 0x80000000 */
} ISP_LCE_TCHL_PARA0_T; //	/* 0x15026A70 */

typedef union {
    typedef ISP_LCE_TCHL_PARA0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE_TCHL_PARA0_T;

typedef struct {
    FIELD  LCE_TCHL_BW2                               :  4;		/*  0.. 3, 0x0000000F */
    FIELD  rsv_4                                      :  4;		/*  4.. 7, 0x000000F0 */
    FIELD  LCE_TCHL_BW1                               :  4;		/*  8..11, 0x00000F00 */
    FIELD  rsv_12                                     :  4;		/* 12..15, 0x0000F000 */
    FIELD  LCE_TCHL_DTH2                              : 12;		/* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                     :  4;		/* 28..31, 0xF0000000 */
} ISP_LCE_TCHL_PARA1_T; //	/* 0x15026A74 */

typedef union {
    typedef ISP_LCE_TCHL_PARA1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE_TCHL_PARA1_T;

typedef struct {
    FIELD  rsv_0                                      : 24;		/*  0..23, 0x00FFFFFF */
    FIELD  LCE_CLR_BLDWD                              :  4;		/* 24..27, 0x0F000000 */
    FIELD  rsv_28                                     :  3;		/* 28..30, 0x70000000 */
    FIELD  LCE_HLR_LMP_EN                             :  1;		/* 31..31, 0x80000000 */
} ISP_LCE_DUM_T; //	/* 0x15026A78 */

typedef union {
    typedef ISP_LCE_DUM_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE_DUM_T;

typedef union {
    enum { COUNT = 21 };
    MUINT32 set[COUNT];
    struct {
        //ISP_NVRAM_LCE_CON_T  			con;
        //ISP_NVRAM_LCE_ZR_T  			zr;
        //ISP_NVRAM_LCE_SLM_SIZE_T      slm_size;
        //ISP_NVRAM_LCE_OFST_T  		ofst;
        //ISP_NVRAM_LCE_BIAS_T  		bias;
        //ISP_NVRAM_LCE_IMAGE_SIZE_T  	image_size;
        //ISP_NVRAM_LCE_GLOBAL_T  		global;
        ISP_NVRAM_LCE_CEN_PARA0_T  		cen_para0;
        ISP_NVRAM_LCE_CEN_PARA1_T  		cen_para1;
        ISP_NVRAM_LCE_CEN_PARA2_T  		cen_para2;
        //ISP_NVRAM_LCE_BIL_TH0_T  		bil_th0;
        //ISP_NVRAM_LCE_BIL_TH1_T  		bil_th1;
        ISP_NVRAM_LCE_TM_PARA0_T  		tm_para0;
        ISP_NVRAM_LCE_TM_PARA1_T  		tm_para1;
        ISP_NVRAM_LCE_TM_PARA2_T  		tm_para2;
        ISP_NVRAM_LCE_TM_PARA3_T  		tm_para3;
        ISP_NVRAM_LCE_TM_PARA4_T  		tm_para4;
        ISP_NVRAM_LCE_TM_PARA5_T  		tm_para5;
        ISP_NVRAM_LCE_TM_PARA6_T  		tm_para6;
        ISP_NVRAM_LCE_TM_PARA7_T  		tm_para7;
        ISP_NVRAM_LCE_HLR_PARA0_T  		hlr_para0;
        ISP_NVRAM_LCE_HLR_PARA1_T  		hlr_para1;
        ISP_NVRAM_LCE_HLR_PARA2_T  		hlr_para2;
        ISP_NVRAM_LCE_HLR_PARA3_T  		hlr_para3;
        ISP_NVRAM_LCE_HLR_PARA4_T  		hlr_para4;
        ISP_NVRAM_LCE_HLR_PARA5_T  		hlr_para5;
        ISP_NVRAM_LCE_HLR_PARA6_T  		hlr_para6;
        ISP_NVRAM_LCE_HLR_PARA7_T  		hlr_para7;
        ISP_NVRAM_LCE_TCHL_PARA0_T  	tchl_para0;
        ISP_NVRAM_LCE_TCHL_PARA1_T  	tchl_para1;
        //ISP_NVRAM_LCE_DUM_T  			dum;
    };
} ISP_NVRAM_LCE_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// LTMS
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct {
    FIELD  LTMS_RESIZE_MODE                                          :  2;      /*  0.. 1, 0x00000003 */
    FIELD  rsv_2                                                     : 30;      /*  2..31, 0xFFFFFFFC */
} ISP_LTMS_CTRL_T; // /* 0x1A0108C0 */

typedef union {
    typedef ISP_LTMS_CTRL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTMS_CTRL_T;

typedef struct {
    FIELD  LTMS_BLK_X_NUM                                            :  5;      /*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                                     :  3;      /*  5.. 7, 0x000000E0 */
    FIELD  LTMS_BLK_Y_NUM                                            :  5;      /*  8..12, 0x00001F00 */
    FIELD  rsv_13                                                    : 19;      /* 13..31, 0xFFFFE000 */
} ISP_LTMS_BLK_NUM_T; // /* 0x1A0108C4 */

typedef union {
    typedef ISP_LTMS_BLK_NUM_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTMS_BLK_NUM_T;

typedef struct {
    FIELD  LTMS_BLK_WIDTH                                            : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  LTMS_BLK_HEIGHT                                           : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_LTMS_BLK_SZ_T; // /* 0x1A0108C8 */

typedef union {
    typedef ISP_LTMS_BLK_SZ_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTMS_BLK_SZ_T;

typedef struct {
    FIELD  LTMS_BLK_AREA                                             : 20;      /*  0..19, 0x000FFFFF */
    FIELD  rsv_20                                                    : 12;      /* 20..31, 0xFFF00000 */
} ISP_LTMS_BLK_AREA_T; // /* 0x1A0108CC */

typedef union {
    typedef ISP_LTMS_BLK_AREA_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTMS_BLK_AREA_T;

typedef struct {
    FIELD  LTMS_DETAIL_HALF_WNDW                                     :  4;      /*  0.. 3, 0x0000000F */
    FIELD  rsv_4                                                     :  4;      /*  4.. 7, 0x000000F0 */
    FIELD  LTMS_DETAIL_TH                                            :  9;      /*  8..16, 0x0001FF00 */
    FIELD  rsv_17                                                    :  3;      /* 17..19, 0x000E0000 */
    FIELD  LTMS_DETAIL_SLP                                           :  9;      /* 20..28, 0x1FF00000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_LTMS_DETAIL_T; // /* 0x1A0108D0 */

typedef union {
    typedef ISP_LTMS_DETAIL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTMS_DETAIL_T;

typedef struct {
    FIELD  LTMS_HIST_BLKPXLNUM_TH                                    : 24;      /*  0..23, 0x00FFFFFF */
    FIELD  rsv_24                                                    :  8;      /* 24..31, 0xFF000000 */
} ISP_LTMS_HIST_T; // /* 0x1A0108D4 */

typedef union {
    typedef ISP_LTMS_HIST_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTMS_HIST_T;

typedef struct {
    FIELD  LTMS_FLATLINE_TH                                          : 10;      /*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                                    :  6;      /* 10..15, 0x0000FC00 */
    FIELD  LTMS_FLATLINE_SLP                                         : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_LTMS_FLTLINE_T; // /* 0x1A0108D8 */

typedef union {
    typedef ISP_LTMS_FLTLINE_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTMS_FLTLINE_T;

typedef struct {
    FIELD  LTMS_FLATBLK_TH                                           : 10;      /*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                                    :  6;      /* 10..15, 0x0000FC00 */
    FIELD  LTMS_FLATBLK_SLP                                          : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_LTMS_FLTBLK_T; // /* 0x1A0108DC */

typedef union {
    typedef ISP_LTMS_FLTBLK_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTMS_FLTBLK_T;

typedef struct {
    FIELD  LTMS_VPLMAX_TH                                            : 10;      /*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                                    : 22;      /* 10..31, 0xFFFFFC00 */
} ISP_LTMS_CLIP_T; // /* 0x1A0108E0 */

typedef union {
    typedef ISP_LTMS_CLIP_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTMS_CLIP_T;

typedef struct {
    FIELD  LTMS_MAXVALUE_ALPHA_BASE                                  : 10;      /*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                                    :  6;      /* 10..15, 0x0000FC00 */
    FIELD  LTMS_MAXVALUE_ALPHA_BASE_SHIFT_BIT                        :  5;      /* 16..20, 0x001F0000 */
    FIELD  rsv_21                                                    : 11;      /* 21..31, 0xFFE00000 */
} ISP_LTMS_MAX_DIV_T; // /* 0x1A0108E4 */

typedef union {
    typedef ISP_LTMS_MAX_DIV_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTMS_MAX_DIV_T;

typedef struct {
    FIELD  LTMS_LTMS_ENGINE_EN                                       :  1;      /*  0.. 0, 0x00000001 */
    FIELD  rsv_1                                                     :  3;      /*  1.. 3, 0x0000000E */
    FIELD  LTMS_LTMS_CG_DISABLE                                      :  1;      /*  4.. 4, 0x00000010 */
    FIELD  LTMS_KEEP_LPXL                                            :  1;      /*  5.. 5, 0x00000020 */
    FIELD  rsv_6                                                     : 22;      /*  6..27, 0x0FFFFFC0 */
    FIELD  LTMS_CHKSUM_EN                                            :  1;      /* 28..28, 0x10000000 */
    FIELD  LTMS_CHKSUM_SEL                                           :  2;      /* 29..30, 0x60000000 */
    FIELD  rsv_31                                                    :  1;      /* 31..31, 0x80000000 */
} ISP_LTMS_CFG_T; // /* 0x1A010910 */

typedef union {
    typedef ISP_LTMS_CFG_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTMS_CFG_T;

typedef struct {
    FIELD  LTMS_LTMS_RESET                                           :  1;      /*  0.. 0, 0x00000001 */
    FIELD  rsv_1                                                     : 31;      /*  1..31, 0xFFFFFFFE */
} ISP_LTMS_RESET_T; // /* 0x1A010914 */

typedef union {
    typedef ISP_LTMS_RESET_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTMS_RESET_T;

typedef struct {
    FIELD  LTMS_IF_END_INT_EN                                        :  1;      /*  0.. 0, 0x00000001 */
    FIELD  LTMS_OF_END_INT_EN                                        :  1;      /*  1.. 1, 0x00000002 */
    FIELD  rsv_2                                                     : 30;      /*  2..31, 0xFFFFFFFC */
} ISP_LTMS_INTEN_T; // /* 0x1A010918 */

typedef union {
    typedef ISP_LTMS_INTEN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTMS_INTEN_T;

typedef struct {
    FIELD  LTMS_IF_END_INT                                           :  1;      /*  0.. 0, 0x00000001 */
    FIELD  LTMS_OF_END_INT                                           :  1;      /*  1.. 1, 0x00000002 */
    FIELD  rsv_2                                                     : 30;      /*  2..31, 0xFFFFFFFC */
} ISP_LTMS_INTSTA_T; // /* 0x1A01091C */

typedef union {
    typedef ISP_LTMS_INTSTA_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTMS_INTSTA_T;

typedef struct {
    FIELD  LTMS_IF_UNFINISH                                          :  1;      /*  0.. 0, 0x00000001 */
    FIELD  LTMS_OF_UNFINISH                                          :  1;      /*  1.. 1, 0x00000002 */
    FIELD  rsv_2                                                     :  2;      /*  2.. 3, 0x0000000C */
    FIELD  LTMS_HANDSHAKE                                            : 24;      /*  4..27, 0x0FFFFFF0 */
    FIELD  rsv_28                                                    :  4;      /* 28..31, 0xF0000000 */
} ISP_LTMS_STATUS_T; // /* 0x1A010920 */

typedef union {
    typedef ISP_LTMS_STATUS_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTMS_STATUS_T;

typedef struct {
    FIELD  LTMS_INP_PIX_CNT                                          : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  LTMS_INP_LINE_CNT                                         : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_LTMS_INPUT_COUNT_T; // /* 0x1A010924 */

typedef union {
    typedef ISP_LTMS_INPUT_COUNT_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTMS_INPUT_COUNT_T;

typedef struct {
    FIELD  LTMS_OUTP_PIX_CNT                                         : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  LTMS_OUTP_LINE_CNT                                        : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_LTMS_OUTPUT_COUNT_T; // /* 0x1A010928 */

typedef union {
    typedef ISP_LTMS_OUTPUT_COUNT_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTMS_OUTPUT_COUNT_T;

typedef struct {
    FIELD  LTMS_CHKSUM                                               : 32;      /*  0..31, 0xFFFFFFFF */
} ISP_LTMS_CHKSUM_T; // /* 0x1A01092C */

typedef union {
    typedef ISP_LTMS_CHKSUM_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTMS_CHKSUM_T;

typedef struct {
    FIELD  LTMS_IN_HSIZE                                             : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  LTMS_IN_VSIZE                                             : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_LTMS_IN_SIZE_T; // /* 0x1A010930 */

typedef union {
    typedef ISP_LTMS_IN_SIZE_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTMS_IN_SIZE_T;

typedef struct {
    FIELD  LTMS_OUT_HSIZE                                            : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  LTMS_OUT_VSIZE                                            : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_LTMS_OUT_SIZE_T; // /* 0x1A010934 */

typedef union {
    typedef ISP_LTMS_OUT_SIZE_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTMS_OUT_SIZE_T;

typedef struct {
    FIELD  LTMS_ACT_WIN_X_START                                      : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  LTMS_ACT_WIN_X_END                                        : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_LTMS_ACT_WINDOW_X_T; // /* 0x1A010938 */

typedef union {
    typedef ISP_LTMS_ACT_WINDOW_X_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTMS_ACT_WINDOW_X_T;

typedef struct {
    FIELD  LTMS_ACT_WIN_Y_START                                      : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  LTMS_ACT_WIN_Y_END                                        : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_LTMS_ACT_WINDOW_Y_T; // /* 0x1A01093C */

typedef union {
    typedef ISP_LTMS_ACT_WINDOW_Y_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTMS_ACT_WINDOW_Y_T;

typedef struct {
    FIELD  LTMS_OUT_DATA_NUM                                         : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    : 19;      /* 13..31, 0xFFFFE000 */
} ISP_LTMS_OUT_DATA_NUM_T; // /* 0x1A010940 */

typedef union {
    typedef ISP_LTMS_OUT_DATA_NUM_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTMS_OUT_DATA_NUM_T;

typedef struct {
    FIELD  LTMS_DUMMY_REG                                            : 32;      /*  0..31, 0xFFFFFFFF */
} ISP_LTMS_DUMMY_REG_T; // /* 0x1A010944 */

typedef union {
    typedef ISP_LTMS_DUMMY_REG_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTMS_DUMMY_REG_T;

typedef struct {
    FIELD  rsv_0                                                     : 16;      /*  0..15, 0x0000FFFF */
    FIELD  LTMS_SRAM_RREQ_EN                                         :  1;      /* 16..16, 0x00010000 */
    FIELD  rsv_17                                                    :  3;      /* 17..19, 0x000E0000 */
    FIELD  LTMS_SRAM_WREQ_EN                                         :  1;      /* 20..20, 0x00100000 */
    FIELD  rsv_21                                                    : 11;      /* 21..31, 0xFFE00000 */
} ISP_LTMS_SRAM_CFG_T; // /* 0x1A010948 */

typedef union {
    typedef ISP_LTMS_SRAM_CFG_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTMS_SRAM_CFG_T;

typedef struct {
    FIELD  LTMS_ltms_atpg_ob                                         :  1;      /*  0.. 0, 0x00000001 */
    FIELD  LTMS_ltms_atpg_ct                                         :  1;      /*  1.. 1, 0x00000002 */
    FIELD  rsv_2                                                     : 30;      /*  2..31, 0xFFFFFFFC */
} ISP_LTMS_ATPG_T; // /* 0x1A010960 */

typedef union {
    typedef ISP_LTMS_ATPG_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTMS_ATPG_T;

typedef struct {
    FIELD  LTMS_BYPASS_SHADOW                                        :  1;      /*  0.. 0, 0x00000001 */
    FIELD  LTMS_FORCE_COMMIT                                         :  1;      /*  1.. 1, 0x00000002 */
    FIELD  rsv_2                                                     : 30;      /*  2..31, 0xFFFFFFFC */
} ISP_LTMS_SHADOW_CTRL_T; // /* 0x1A010964 */

typedef union {
    typedef ISP_LTMS_SHADOW_CTRL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTMS_SHADOW_CTRL_T;

typedef struct {
    FIELD  LTMS_HIST_R_BLKPXLNUM_TH                                  : 24;      /*  0..23, 0x00FFFFFF */
    FIELD  rsv_24                                                    :  8;      /* 24..31, 0xFF000000 */
} ISP_LTMS_HIST_R_T; // /* 0x1A010968 */

typedef union {
    typedef ISP_LTMS_HIST_R_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTMS_HIST_R_T;

typedef struct {
    FIELD  LTMS_HIST_B_BLKPXLNUM_TH                                  : 24;      /*  0..23, 0x00FFFFFF */
    FIELD  rsv_24                                                    :  8;      /* 24..31, 0xFF000000 */
} ISP_LTMS_HIST_B_T; // /* 0x1A01096C */

typedef union {
    typedef ISP_LTMS_HIST_B_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTMS_HIST_B_T;

typedef struct {
    FIELD  LTMS_HIST_C_BLKPXLNUM_TH                                  : 24;      /*  0..23, 0x00FFFFFF */
    FIELD  rsv_24                                                    :  8;      /* 24..31, 0xFF000000 */
} ISP_LTMS_HIST_C_T; // /* 0x1A010970 */

typedef union {
    typedef ISP_LTMS_HIST_C_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTMS_HIST_C_T;

typedef struct {
    FIELD  LTMS_FLATLINE_R_TH                                        : 10;      /*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                                    :  6;      /* 10..15, 0x0000FC00 */
    FIELD  LTMS_FLATLINE_R_SLP                                       : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_LTMS_FLATLINE_R_T; // /* 0x1A010974 */

typedef union {
    typedef ISP_LTMS_FLATLINE_R_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTMS_FLATLINE_R_T;

typedef struct {
    FIELD  LTMS_FLATBLK_B_TH                                         : 10;      /*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                                    :  6;      /* 10..15, 0x0000FC00 */
    FIELD  LTMS_FLATBLK_B_SLP                                        : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_LTMS_FLATBLK_B_T; // /* 0x1A010978 */

typedef union {
    typedef ISP_LTMS_FLATBLK_B_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTMS_FLATBLK_B_T;

typedef struct {
    FIELD  LTMS_BLK_R_AREA                                           : 20;      /*  0..19, 0x000FFFFF */
    FIELD  rsv_20                                                    : 12;      /* 20..31, 0xFFF00000 */
} ISP_LTMS_BLK_R_AREA_T; // /* 0x1A01097C */

typedef union {
    typedef ISP_LTMS_BLK_R_AREA_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTMS_BLK_R_AREA_T;

typedef struct {
    FIELD  LTMS_BLK_B_AREA                                           : 20;      /*  0..19, 0x000FFFFF */
    FIELD  rsv_20                                                    : 12;      /* 20..31, 0xFFF00000 */
} ISP_LTMS_BLK_B_AREA_T; // /* 0x1A010980 */

typedef union {
    typedef ISP_LTMS_BLK_B_AREA_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTMS_BLK_B_AREA_T;

typedef struct {
    FIELD  LTMS_BLK_C_AREA                                           : 20;      /*  0..19, 0x000FFFFF */
    FIELD  rsv_20                                                    : 12;      /* 20..31, 0xFFF00000 */
} ISP_LTMS_BLK_C_AREA_T; // /* 0x1A010984 */

typedef union {
    typedef ISP_LTMS_BLK_C_AREA_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTMS_BLK_C_AREA_T;

typedef union {
    enum { COUNT = 35 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_LTMS_CTRL_T  			ctrl;
        ISP_NVRAM_LTMS_BLK_NUM_T  		blk_num;
        ISP_NVRAM_LTMS_BLK_SZ_T  		blk_sz;
        ISP_NVRAM_LTMS_BLK_AREA_T  		blk_area;
        ISP_NVRAM_LTMS_DETAIL_T  		detail;
        ISP_NVRAM_LTMS_HIST_T  			hist;
        ISP_NVRAM_LTMS_FLTLINE_T  		fltline;
        ISP_NVRAM_LTMS_FLTBLK_T  		fltblk;
        ISP_NVRAM_LTMS_CLIP_T  			clip;
        ISP_NVRAM_LTMS_MAX_DIV_T  		max_div;
        ISP_NVRAM_LTMS_CFG_T  			cfg;
        ISP_NVRAM_LTMS_RESET_T  		reset;
        ISP_NVRAM_LTMS_INTEN_T  		inten;
        ISP_NVRAM_LTMS_INTSTA_T  		intsta;
        ISP_NVRAM_LTMS_STATUS_T  		status;
        ISP_NVRAM_LTMS_INPUT_COUNT_T  	input_count;
        ISP_NVRAM_LTMS_OUTPUT_COUNT_T  	output_count;
        ISP_NVRAM_LTMS_CHKSUM_T  		chksum;
        ISP_NVRAM_LTMS_IN_SIZE_T  		in_size;
        ISP_NVRAM_LTMS_OUT_SIZE_T  		out_size;
        ISP_NVRAM_LTMS_ACT_WINDOW_X_T  	act_window_x;
        ISP_NVRAM_LTMS_ACT_WINDOW_Y_T  	act_window_y;
        ISP_NVRAM_LTMS_OUT_DATA_NUM_T  	out_data_num;
        ISP_NVRAM_LTMS_DUMMY_REG_T  	dummy_reg;
        ISP_NVRAM_LTMS_SRAM_CFG_T  		sram_cfg;
        ISP_NVRAM_LTMS_ATPG_T  			atpg;
        ISP_NVRAM_LTMS_SHADOW_CTRL_T  	shadow_ctrl;
        ISP_NVRAM_LTMS_HIST_R_T  		hist_r;
        ISP_NVRAM_LTMS_HIST_B_T  		hist_b;
        ISP_NVRAM_LTMS_HIST_C_T  		hist_c;
        ISP_NVRAM_LTMS_FLATLINE_R_T  	flatline_r;
        ISP_NVRAM_LTMS_FLATBLK_B_T  	flatblk_b;
        ISP_NVRAM_LTMS_BLK_R_AREA_T  	blk_r_area;
        ISP_NVRAM_LTMS_BLK_B_AREA_T  	blk_b_area;
        ISP_NVRAM_LTMS_BLK_C_AREA_T  	blk_c_area;
    };
} ISP_NVRAM_LTMS_T;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// LTM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef struct {
    FIELD  LTM_LUM_MAP_MODE                                          :  2;      /*  0.. 1, 0x00000003 */
    FIELD  rsv_2                                                     : 30;      /*  2..31, 0xFFFFFFFC */
} ISP_LTM_CTRL_T; // /* 0x1A010640 */

typedef union {
    typedef ISP_LTM_CTRL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTM_CTRL_T;

typedef struct {
    FIELD  LTM_BLK_X_NUM                                             :  5;      /*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                                     :  3;      /*  5.. 7, 0x000000E0 */
    FIELD  LTM_BLK_Y_NUM                                             :  5;      /*  8..12, 0x00001F00 */
    FIELD  rsv_13                                                    : 19;      /* 13..31, 0xFFFFE000 */
} ISP_LTM_BLK_NUM_T; // /* 0x1A010644 */

typedef union {
    typedef ISP_LTM_BLK_NUM_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTM_BLK_NUM_T;

typedef struct {
    FIELD  LTM_BLK_WIDTH                                             : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  LTM_BLK_HEIGHT                                            : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_LTM_BLK_SZ_T; // /* 0x1A010648 */

typedef union {
    typedef ISP_LTM_BLK_SZ_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTM_BLK_SZ_T;

typedef struct {
    FIELD  LTM_X_ALPHA_BASE                                          :  9;      /*  0.. 8, 0x000001FF */
    FIELD  rsv_9                                                     :  7;      /*  9..15, 0x0000FE00 */
    FIELD  LTM_X_ALPHA_BASE_SHIFT_BIT                                :  4;      /* 16..19, 0x000F0000 */
    FIELD  rsv_20                                                    : 12;      /* 20..31, 0xFFF00000 */
} ISP_LTM_BLK_DIVX_T; // /* 0x1A01064C */

typedef union {
    typedef ISP_LTM_BLK_DIVX_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTM_BLK_DIVX_T;

typedef struct {
    FIELD  LTM_Y_ALPHA_BASE                                          :  9;      /*  0.. 8, 0x000001FF */
    FIELD  rsv_9                                                     :  7;      /*  9..15, 0x0000FE00 */
    FIELD  LTM_Y_ALPHA_BASE_SHIFT_BIT                                :  4;      /* 16..19, 0x000F0000 */
    FIELD  rsv_20                                                    : 12;      /* 20..31, 0xFFF00000 */
} ISP_LTM_BLK_DIVY_T; // /* 0x1A010650 */

typedef union {
    typedef ISP_LTM_BLK_DIVY_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTM_BLK_DIVY_T;

typedef struct {
    FIELD  LTM_CLIP_TH_ALPHA_BASE                                    : 10;      /*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                                    :  6;      /* 10..15, 0x0000FC00 */
    FIELD  LTM_CLIP_TH_ALPHA_BASE_SHIFT_BIT                          :  5;      /* 16..20, 0x001F0000 */
    FIELD  rsv_21                                                    : 11;      /* 21..31, 0xFFE00000 */
} ISP_LTM_MAX_DIV_T; // /* 0x1A010654 */

typedef union {
    typedef ISP_LTM_MAX_DIV_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTM_MAX_DIV_T;

typedef struct {
    FIELD  LTM_GAIN_TH                                               :  6;      /*  0.. 5, 0x0000003F */
    FIELD  rsv_6                                                     : 26;      /*  6..31, 0xFFFFFFC0 */
} ISP_LTM_CLIP_T; // /* 0x1A01065C */

typedef union {
    typedef ISP_LTM_CLIP_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTM_CLIP_T;

typedef struct {
    FIELD  LTM_TILE_BLK_X_NUM_START                                  :  5;      /*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                                     :  3;      /*  5.. 7, 0x000000E0 */
    FIELD  LTM_TILE_BLK_X_NUM_END                                    :  5;      /*  8..12, 0x00001F00 */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  LTM_TILE_BLK_Y_NUM_START                                  :  5;      /* 16..20, 0x001F0000 */
    FIELD  rsv_21                                                    :  3;      /* 21..23, 0x00E00000 */
    FIELD  LTM_TILE_BLK_Y_NUM_END                                    :  5;      /* 24..28, 0x1F000000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_LTM_TILE_NUM_T; // /* 0x1A010660 */

typedef union {
    typedef ISP_LTM_TILE_NUM_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTM_TILE_NUM_T;

typedef struct {
    FIELD  LTM_TILE_BLK_X_CNT_START                                  : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  LTM_TILE_BLK_X_CNT_END                                    : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_LTM_TILE_CNTX_T; // /* 0x1A010664 */

typedef union {
    typedef ISP_LTM_TILE_CNTX_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTM_TILE_CNTX_T;

typedef struct {
    FIELD  LTM_TILE_BLK_Y_CNT_START                                  : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  LTM_TILE_BLK_Y_CNT_END                                    : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_LTM_TILE_CNTY_T; // /* 0x1A010668 */

typedef union {
    typedef ISP_LTM_TILE_CNTY_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTM_TILE_CNTY_T;

typedef struct {
    FIELD  LTM_LTM_ENGINE_EN                                         :  1;      /*  0.. 0, 0x00000001 */
    FIELD  rsv_1                                                     :  3;      /*  1.. 3, 0x0000000E */
    FIELD  LTM_LTM_CG_DISABLE                                        :  1;      /*  4.. 4, 0x00000010 */
    FIELD  rsv_5                                                     : 23;      /*  5..27, 0x0FFFFFE0 */
    FIELD  LTM_CHKSUM_EN                                             :  1;      /* 28..28, 0x10000000 */
    FIELD  LTM_CHKSUM_SEL                                            :  2;      /* 29..30, 0x60000000 */
    FIELD  rsv_31                                                    :  1;      /* 31..31, 0x80000000 */
} ISP_LTM_CFG_T; // /* 0x1A010674 */

typedef union {
    typedef ISP_LTM_CFG_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTM_CFG_T;

typedef struct {
    FIELD  LTM_LTM_RESET                                             :  1;      /*  0.. 0, 0x00000001 */
    FIELD  rsv_1                                                     : 31;      /*  1..31, 0xFFFFFFFE */
} ISP_LTM_RESET_T; // /* 0x1A010690 */

typedef union {
    typedef ISP_LTM_RESET_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTM_RESET_T;

typedef struct {
    FIELD  LTM_IF_END_INT_EN                                         :  1;      /*  0.. 0, 0x00000001 */
    FIELD  LTM_OF_END_INT_EN                                         :  1;      /*  1.. 1, 0x00000002 */
    FIELD  rsv_2                                                     : 30;      /*  2..31, 0xFFFFFFFC */
} ISP_LTM_INTEN_T; // /* 0x1A010694 */

typedef union {
    typedef ISP_LTM_INTEN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTM_INTEN_T;

typedef struct {
    FIELD  LTM_IF_END_INT                                            :  1;      /*  0.. 0, 0x00000001 */
    FIELD  LTM_OF_END_INT                                            :  1;      /*  1.. 1, 0x00000002 */
    FIELD  rsv_2                                                     : 30;      /*  2..31, 0xFFFFFFFC */
} ISP_LTM_INTSTA_T; // /* 0x1A010698 */

typedef union {
    typedef ISP_LTM_INTSTA_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTM_INTSTA_T;

typedef struct {
    FIELD  LTM_IF_UNFINISH                                           :  1;      /*  0.. 0, 0x00000001 */
    FIELD  LTM_OF_UNFINISH                                           :  1;      /*  1.. 1, 0x00000002 */
    FIELD  rsv_2                                                     :  2;      /*  2.. 3, 0x0000000C */
    FIELD  LTM_HANDSHAKE                                             : 24;      /*  4..27, 0x0FFFFFF0 */
    FIELD  rsv_28                                                    :  4;      /* 28..31, 0xF0000000 */
} ISP_LTM_STATUS_T; // /* 0x1A01069C */

typedef union {
    typedef ISP_LTM_STATUS_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTM_STATUS_T;

typedef struct {
    FIELD  LTM_INP_PIX_CNT                                           : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  LTM_INP_LINE_CNT                                          : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_LTM_INPUT_COUNT_T; // /* 0x1A0106A0 */

typedef union {
    typedef ISP_LTM_INPUT_COUNT_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTM_INPUT_COUNT_T;

typedef struct {
    FIELD  LTM_OUTP_PIX_CNT                                          : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  LTM_OUTP_LINE_CNT                                         : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_LTM_OUTPUT_COUNT_T; // /* 0x1A0106A4 */

typedef union {
    typedef ISP_LTM_OUTPUT_COUNT_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTM_OUTPUT_COUNT_T;

typedef struct {
    FIELD  LTM_CHKSUM                                                : 32;      /*  0..31, 0xFFFFFFFF */
} ISP_LTM_CHKSUM_T; // /* 0x1A0106A8 */

typedef union {
    typedef ISP_LTM_CHKSUM_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTM_CHKSUM_T;

typedef struct {
    FIELD  LTM_TILE_HSIZE                                            : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  LTM_TILE_VSIZE                                            : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_LTM_TILE_SIZE_T; // /* 0x1A0106AC */

typedef union {
    typedef ISP_LTM_TILE_SIZE_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTM_TILE_SIZE_T;

typedef struct {
    FIELD  LTM_TILE_EDGE                                             :  4;      /*  0.. 3, 0x0000000F */
    FIELD  rsv_4                                                     : 28;      /*  4..31, 0xFFFFFFF0 */
} ISP_LTM_TILE_EDGE_T; // /* 0x1A0106B0 */

typedef union {
    typedef ISP_LTM_TILE_EDGE_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTM_TILE_EDGE_T;

typedef struct {
    FIELD  LTM_CROP_H                                                :  4;      /*  0.. 3, 0x0000000F */
    FIELD  LTM_CROP_V                                                :  4;      /*  4.. 7, 0x000000F0 */
    FIELD  rsv_8                                                     : 24;      /*  8..31, 0xFFFFFF00 */
} ISP_LTM_TILE_CROP_T; // /* 0x1A0106B4 */

typedef union {
    typedef ISP_LTM_TILE_CROP_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTM_TILE_CROP_T;

typedef struct {
    FIELD  LTM_DUMMY_REG                                             : 32;      /*  0..31, 0xFFFFFFFF */
} ISP_LTM_DUMMY_REG_T; // /* 0x1A0106B8 */

typedef union {
    typedef ISP_LTM_DUMMY_REG_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTM_DUMMY_REG_T;

typedef struct {
    FIELD  rsv_0                                                     :  4;      /*  0.. 3, 0x0000000F */
    FIELD  LTM_FORCE_SRAM_EN                                         :  1;      /*  4.. 4, 0x00000010 */
    FIELD  LTM_FORCE_SRAM_APB                                        :  1;      /*  5.. 5, 0x00000020 */
    FIELD  LTM_FORCE_SRAM_INT                                        :  1;      /*  6.. 6, 0x00000040 */
    FIELD  rsv_7                                                     : 25;      /*  7..31, 0xFFFFFF80 */
} ISP_LTM_SRAM_CFG_T; // /* 0x1A0106BC */

typedef union {
    typedef ISP_LTM_SRAM_CFG_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTM_SRAM_CFG_T;

typedef struct {
    FIELD  LTM_RB_SRAM_IDX                                           :  1;      /*  0.. 0, 0x00000001 */
    FIELD  LTM_RB_SRAM_IDX_OUT                                       :  1;      /*  1.. 1, 0x00000002 */
    FIELD  rsv_2                                                     : 30;      /*  2..31, 0xFFFFFFFC */
} ISP_LTM_SRAM_STATUS_T; // /* 0x1A0106C0 */

typedef union {
    typedef ISP_LTM_SRAM_STATUS_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTM_SRAM_STATUS_T;

typedef struct {
    FIELD  LTM_ltm_atpg_ob                                           :  1;      /*  0.. 0, 0x00000001 */
    FIELD  LTM_ltm_atpg_ct                                           :  1;      /*  1.. 1, 0x00000002 */
    FIELD  rsv_2                                                     : 30;      /*  2..31, 0xFFFFFFFC */
} ISP_LTM_ATPG_T; // /* 0x1A0106D4 */

typedef union {
    typedef ISP_LTM_ATPG_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTM_ATPG_T;

typedef struct {
    FIELD  LTM_BYPASS_SHADOW                                         :  1;      /*  0.. 0, 0x00000001 */
    FIELD  LTM_FORCE_COMMIT                                          :  1;      /*  1.. 1, 0x00000002 */
    FIELD  rsv_2                                                     : 30;      /*  2..31, 0xFFFFFFFC */
} ISP_LTM_SHADOW_CTRL_T; // /* 0x1A0106D8 */

typedef union {
    typedef ISP_LTM_SHADOW_CTRL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTM_SHADOW_CTRL_T;

typedef struct {
    FIELD  LTM_SELRGB_GRAD0                                          : 10;      /*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                                    :  6;      /* 10..15, 0x0000FC00 */
    FIELD  LTM_SELRGB_GRAD1                                          : 10;      /* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                                    :  6;      /* 26..31, 0xFC000000 */
} ISP_LTM_SELRGB_GRAD0_T; // /* 0x1A0106DC */

typedef union {
    typedef ISP_LTM_SELRGB_GRAD0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTM_SELRGB_GRAD0_T;

typedef struct {
    FIELD  LTM_SELRGB_GRAD2                                          : 10;      /*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                                    :  6;      /* 10..15, 0x0000FC00 */
    FIELD  LTM_SELRGB_GRAD3                                          : 10;      /* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                                    :  6;      /* 26..31, 0xFC000000 */
} ISP_LTM_SELRGB_GRAD1_T; // /* 0x1A0106E0 */

typedef union {
    typedef ISP_LTM_SELRGB_GRAD1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTM_SELRGB_GRAD1_T;

typedef struct {
    FIELD  LTM_SELRGB_GRAD4                                          : 10;      /*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                                    :  6;      /* 10..15, 0x0000FC00 */
    FIELD  LTM_SELRGB_GRAD5                                          : 10;      /* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                                    :  6;      /* 26..31, 0xFC000000 */
} ISP_LTM_SELRGB_GRAD2_T; // /* 0x1A0106E4 */

typedef union {
    typedef ISP_LTM_SELRGB_GRAD2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTM_SELRGB_GRAD2_T;

typedef struct {
    FIELD  LTM_SELRGB_GRAD6                                          : 10;      /*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                                    :  6;      /* 10..15, 0x0000FC00 */
    FIELD  LTM_SELRGB_GRAD7                                          : 10;      /* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                                    :  6;      /* 26..31, 0xFC000000 */
} ISP_LTM_SELRGB_GRAD3_T; // /* 0x1A0106E8 */

typedef union {
    typedef ISP_LTM_SELRGB_GRAD3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTM_SELRGB_GRAD3_T;

typedef struct {
    FIELD  LTM_SELRGB_TH0                                            : 12;      /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                                    :  4;      /* 12..15, 0x0000F000 */
    FIELD  LTM_SELRGB_TH1                                            : 12;      /* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                                    :  4;      /* 28..31, 0xF0000000 */
} ISP_LTM_SELRGB_TH0_T; // /* 0x1A0106EC */

typedef union {
    typedef ISP_LTM_SELRGB_TH0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTM_SELRGB_TH0_T;

typedef struct {
    FIELD  LTM_SELRGB_TH2                                            : 12;      /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                                    :  4;      /* 12..15, 0x0000F000 */
    FIELD  LTM_SELRGB_TH3                                            : 12;      /* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                                    :  4;      /* 28..31, 0xF0000000 */
} ISP_LTM_SELRGB_TH1_T; // /* 0x1A0106F0 */

typedef union {
    typedef ISP_LTM_SELRGB_TH1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTM_SELRGB_TH1_T;

typedef struct {
    FIELD  LTM_SELRGB_TH4                                            : 12;      /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                                    :  4;      /* 12..15, 0x0000F000 */
    FIELD  LTM_SELRGB_TH5                                            : 12;      /* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                                    :  4;      /* 28..31, 0xF0000000 */
} ISP_LTM_SELRGB_TH2_T; // /* 0x1A0106F4 */

typedef union {
    typedef ISP_LTM_SELRGB_TH2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTM_SELRGB_TH2_T;

typedef struct {
    FIELD  LTM_SELRGB_TH6                                            : 12;      /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                                    :  4;      /* 12..15, 0x0000F000 */
    FIELD  LTM_SELRGB_TH7                                            : 12;      /* 16..27, 0x0FFF0000 */
    FIELD  rsv_28                                                    :  4;      /* 28..31, 0xF0000000 */
} ISP_LTM_SELRGB_TH3_T; // /* 0x1A0106F8 */

typedef union {
    typedef ISP_LTM_SELRGB_TH3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTM_SELRGB_TH3_T;

typedef struct {
    FIELD  LTM_SELRGB_SLP0                                           : 18;      /*  0..17, 0x0003FFFF */
    FIELD  rsv_18                                                    : 14;      /* 18..31, 0xFFFC0000 */
} ISP_LTM_SELRGB_SLP0_T; // /* 0x1A0106FC */

typedef union {
    typedef ISP_LTM_SELRGB_SLP0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTM_SELRGB_SLP0_T;

typedef struct {
    FIELD  LTM_SELRGB_SLP1                                           : 18;      /*  0..17, 0x0003FFFF */
    FIELD  rsv_18                                                    : 14;      /* 18..31, 0xFFFC0000 */
} ISP_LTM_SELRGB_SLP1_T; // /* 0x1A010700 */

typedef union {
    typedef ISP_LTM_SELRGB_SLP1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTM_SELRGB_SLP1_T;

typedef struct {
    FIELD  LTM_SELRGB_SLP2                                           : 18;      /*  0..17, 0x0003FFFF */
    FIELD  rsv_18                                                    : 14;      /* 18..31, 0xFFFC0000 */
} ISP_LTM_SELRGB_SLP2_T; // /* 0x1A010704 */

typedef union {
    typedef ISP_LTM_SELRGB_SLP2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTM_SELRGB_SLP2_T;

typedef struct {
    FIELD  LTM_SELRGB_SLP3                                           : 18;      /*  0..17, 0x0003FFFF */
    FIELD  rsv_18                                                    : 14;      /* 18..31, 0xFFFC0000 */
} ISP_LTM_SELRGB_SLP3_T; // /* 0x1A010708 */

typedef union {
    typedef ISP_LTM_SELRGB_SLP3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTM_SELRGB_SLP3_T;

typedef struct {
    FIELD  LTM_SELRGB_SLP4                                           : 18;      /*  0..17, 0x0003FFFF */
    FIELD  rsv_18                                                    : 14;      /* 18..31, 0xFFFC0000 */
} ISP_LTM_SELRGB_SLP4_T; // /* 0x1A01070C */

typedef union {
    typedef ISP_LTM_SELRGB_SLP4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTM_SELRGB_SLP4_T;

typedef struct {
    FIELD  LTM_SELRGB_SLP5                                           : 18;      /*  0..17, 0x0003FFFF */
    FIELD  rsv_18                                                    : 14;      /* 18..31, 0xFFFC0000 */
} ISP_LTM_SELRGB_SLP5_T; // /* 0x1A010710 */

typedef union {
    typedef ISP_LTM_SELRGB_SLP5_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTM_SELRGB_SLP5_T;

typedef struct {
    FIELD  LTM_SELRGB_SLP6                                           : 18;      /*  0..17, 0x0003FFFF */
    FIELD  rsv_18                                                    : 14;      /* 18..31, 0xFFFC0000 */
} ISP_LTM_SELRGB_SLP6_T; // /* 0x1A010714 */

typedef union {
    typedef ISP_LTM_SELRGB_SLP6_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTM_SELRGB_SLP6_T;

typedef struct {
    FIELD  LTM_OUT_STR                                               :  5;      /*  0.. 4, 0x0000001F */
    FIELD  rsv_5                                                     : 27;      /*  5..31, 0xFFFFFFE0 */
} ISP_LTM_OUT_STR_T; // /* 0x1A010718 */

typedef union {
    typedef ISP_LTM_OUT_STR_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTM_OUT_STR_T;

typedef struct {
    FIELD  LTM_SRAM_PINGPONG                                         :  1;      /*  0.. 0, 0x00000001 */
    FIELD  rsv_1                                                     : 31;      /*  1..31, 0xFFFFFFFE */
} ISP_LTM_SRAM_PINGPONG_T; // /* 0x1A01071C */

typedef union {
    typedef ISP_LTM_SRAM_PINGPONG_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTM_SRAM_PINGPONG_T;

typedef union {
    enum { COUNT = 43 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_LTM_CTRL_T  			ctrl;
        ISP_NVRAM_LTM_BLK_NUM_T  			blk_num;
        ISP_NVRAM_LTM_BLK_SZ_T  			blk_sz;
        ISP_NVRAM_LTM_BLK_DIVX_T  			blk_divx;
        ISP_NVRAM_LTM_BLK_DIVY_T  			blk_divy;
        ISP_NVRAM_LTM_MAX_DIV_T  			max_div;
        ISP_NVRAM_LTM_CLIP_T  			clip;
        ISP_NVRAM_LTM_TILE_NUM_T  			tile_num;
        ISP_NVRAM_LTM_TILE_CNTX_T  			tile_cntx;
        ISP_NVRAM_LTM_TILE_CNTY_T  			tile_cnty;
        ISP_NVRAM_LTM_CFG_T  			cfg;
        ISP_NVRAM_LTM_RESET_T  			reset;
        ISP_NVRAM_LTM_INTEN_T  			inten;
        ISP_NVRAM_LTM_INTSTA_T  			intsta;
        ISP_NVRAM_LTM_STATUS_T  			status;
        ISP_NVRAM_LTM_INPUT_COUNT_T  			input_count;
        ISP_NVRAM_LTM_OUTPUT_COUNT_T  			output_count;
        ISP_NVRAM_LTM_CHKSUM_T  			chksum;
        ISP_NVRAM_LTM_TILE_SIZE_T  			tile_size;
        ISP_NVRAM_LTM_TILE_EDGE_T  			tile_edge;
        ISP_NVRAM_LTM_TILE_CROP_T  			tile_crop;
        ISP_NVRAM_LTM_DUMMY_REG_T  			dummy_reg;
        ISP_NVRAM_LTM_SRAM_CFG_T  			sram_cfg;
        ISP_NVRAM_LTM_SRAM_STATUS_T  			sram_status;
        ISP_NVRAM_LTM_ATPG_T  			atpg;
        ISP_NVRAM_LTM_SHADOW_CTRL_T  			shadow_ctrl;
        ISP_NVRAM_LTM_SELRGB_GRAD0_T  			selrgb_grad0;
        ISP_NVRAM_LTM_SELRGB_GRAD1_T  			selrgb_grad1;
        ISP_NVRAM_LTM_SELRGB_GRAD2_T  			selrgb_grad2;
        ISP_NVRAM_LTM_SELRGB_GRAD3_T  			selrgb_grad3;
        ISP_NVRAM_LTM_SELRGB_TH0_T  			selrgb_th0;
        ISP_NVRAM_LTM_SELRGB_TH1_T  			selrgb_th1;
        ISP_NVRAM_LTM_SELRGB_TH2_T  			selrgb_th2;
        ISP_NVRAM_LTM_SELRGB_TH3_T  			selrgb_th3;
        ISP_NVRAM_LTM_SELRGB_SLP0_T  			selrgb_slp0;
        ISP_NVRAM_LTM_SELRGB_SLP1_T  			selrgb_slp1;
        ISP_NVRAM_LTM_SELRGB_SLP2_T  			selrgb_slp2;
        ISP_NVRAM_LTM_SELRGB_SLP3_T  			selrgb_slp3;
        ISP_NVRAM_LTM_SELRGB_SLP4_T  			selrgb_slp4;
        ISP_NVRAM_LTM_SELRGB_SLP5_T  			selrgb_slp5;
        ISP_NVRAM_LTM_SELRGB_SLP6_T  			selrgb_slp6;
        ISP_NVRAM_LTM_OUT_STR_T  			out_str;
        ISP_NVRAM_LTM_SRAM_PINGPONG_T  			sram_pingpong;
    };
} ISP_NVRAM_LTM_T;


enum { LTM_CURVE_SIZE = 1782 };

typedef struct {
      	FIELD  LTMTC_TONECURVE_LUT_L                      : 14;		/*  0..13, 0x00003FFF */
      	FIELD  rsv_14                                     :  2;		/* 14..15, 0x0000C000 */
      	FIELD  LTMTC_TONECURVE_LUT_H                      : 14;		/* 16..29, 0x3FFF0000 */
      	FIELD  rsv_30                                     :  2;		/* 30..31, 0xC0000000 */
} ISP_LTM_LUT_T;

typedef union {
    typedef ISP_LTM_LUT_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LTM_LUT_T;

typedef union {
    enum { COUNT = LTM_CURVE_SIZE };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_LTM_LUT_T   lut[LTM_CURVE_SIZE];
    };
} ISP_NVRAM_LTM_CURVE_T;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// FLC
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct {
    FIELD  FLC_OFST_B                                                : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  FLC_OFST_R                                                : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_FLC_OFST_RB_T; // /* 0x1A012DC0 */

typedef union {
    typedef ISP_FLC_OFST_RB_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_FLC_OFST_RB_T;

typedef struct {
    FIELD  FLC_OFST_G                                                : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    : 19;      /* 13..31, 0xFFFFE000 */
} ISP_FLC_OFST_G_T; // /* 0x1A012DC4 */

typedef union {
    typedef ISP_FLC_OFST_G_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_FLC_OFST_G_T;

typedef struct {
    FIELD  FLC_GAIN_B                                                : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    :  3;      /* 13..15, 0x0000E000 */
    FIELD  FLC_GAIN_R                                                : 13;      /* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                                    :  3;      /* 29..31, 0xE0000000 */
} ISP_FLC_GN_RB_T; // /* 0x1A012DC8 */

typedef union {
    typedef ISP_FLC_GN_RB_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_FLC_GN_RB_T;

typedef struct {
    FIELD  FLC_GAIN_G                                                : 13;      /*  0..12, 0x00001FFF */
    FIELD  rsv_13                                                    : 19;      /* 13..31, 0xFFFFE000 */
} ISP_FLC_GN_G_T; // /* 0x1A012DCC */

typedef union {
    typedef ISP_FLC_GN_G_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_FLC_GN_G_T;

typedef union {
    enum { COUNT = 4 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_FLC_OFST_RB_T  	    ofst_rb;
        ISP_NVRAM_FLC_OFST_G_T  		ofst_g;
        ISP_NVRAM_FLC_GN_RB_T  			gn_rb;
        ISP_NVRAM_FLC_GN_G_T  			gn_g;
    };
} ISP_NVRAM_FLC_T;

#if 0
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// DCES
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef struct {
    FIELD  DCES_CLEAR                                 :  2;		/*  0.. 1, 0x00000003 */
    FIELD  rsv_2                                      :  2;		/*  2.. 3, 0x0000000C */
    FIELD  DCES_DC_MODE                               :  3;		/*  4.. 6, 0x00000070 */
    FIELD  rsv_7                                      :  1;		/*  7.. 7, 0x00000080 */
    FIELD  DCES_DC_TH                                 : 10;		/*  8..17, 0x0003FF00 */
    FIELD  rsv_18                                     :  2;		/* 18..19, 0x000C0000 */
    FIELD  DCES_DC_SLOPE                              :  6;		/* 20..25, 0x03F00000 */
    FIELD  rsv_26                                     :  6;		/* 26..31, 0xFC000000 */
} ISP_DCES_DC_CTRL_T; //	/* 0x15027140 */

typedef union {
    typedef ISP_DCES_DC_CTRL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DCES_DC_CTRL_T;

typedef struct {
    FIELD  DCES_CROP_XSTART                           : 14;		/*  0..13, 0x00003FFF */
    FIELD  rsv_14                                     :  2;		/* 14..15, 0x0000C000 */
    FIELD  DCES_CROP_XEND                             : 14;		/* 16..29, 0x3FFF0000 */
    FIELD  rsv_30                                     :  2;		/* 30..31, 0xC0000000 */
} ISP_DCES_CROP_X_T; //	/* 0x15027144 */

typedef union {
    typedef ISP_DCES_CROP_X_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DCES_CROP_X_T;

typedef struct {
    FIELD  DCES_CROP_YSTART                           : 14;		/*  0..13, 0x00003FFF */
    FIELD  rsv_14                                     :  2;		/* 14..15, 0x0000C000 */
    FIELD  DCES_CROP_YEND                             : 14;		/* 16..29, 0x3FFF0000 */
    FIELD  rsv_30                                     :  2;		/* 30..31, 0xC0000000 */
} ISP_DCES_CROP_Y_T; //	/* 0x15027148 */

typedef union {
    typedef ISP_DCES_CROP_Y_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DCES_CROP_Y_T;

typedef struct {
    FIELD  DCES_CNV_G                                 : 11;		/*  0..10, 0x000007FF */
    FIELD  rsv_11                                     :  5;		/* 11..15, 0x0000F800 */
    FIELD  DCES_CNV_R                                 : 11;		/* 16..26, 0x07FF0000 */
    FIELD  rsv_27                                     :  5;		/* 27..31, 0xF8000000 */
} ISP_DCES_V_Y_CH_0_T; //	/* 0x1502714C */

typedef union {
    typedef ISP_DCES_V_Y_CH_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DCES_V_Y_CH_0_T;

typedef struct {
    FIELD  DCES_Y_OFST                                : 11;		/*  0..10, 0x000007FF */
    FIELD  DCES_CNV_B                                 : 11;		/* 11..21, 0x003FF800 */
    FIELD  rsv_22                                     :  4;		/* 22..25, 0x03C00000 */
    FIELD  DCES_VY_BLD_WT                             :  6;		/* 26..31, 0xFC000000 */
} ISP_DCES_V_Y_CH_1_T; //	/* 0x15027150 */

typedef union {
    typedef ISP_DCES_V_Y_CH_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DCES_V_Y_CH_1_T;

typedef struct {
    FIELD  DCES_hsize                                 : 13;		/*  0..12, 0x00001FFF */
    FIELD  DCES_vsize                                 : 13;		/* 13..25, 0x03FFE000 */
    FIELD  rsv_26                                     :  6;		/* 26..31, 0xFC000000 */
} ISP_DCES_SIZE_T; //	/* 0x15027154 */

typedef union {
    typedef ISP_DCES_SIZE_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DCES_SIZE_T;

typedef struct {
    FIELD  DCES_dces_atpg_ob                          :  1;		/*  0.. 0, 0x00000001 */
    FIELD  DCES_dces_atpg_ct                          :  1;		/*  1.. 1, 0x00000002 */
    FIELD  rsv_2                                      : 30;		/*  2..31, 0xFFFFFFFC */
} ISP_DCES_ATPG_T; //	/* 0x15027158 */

typedef union {
    typedef ISP_DCES_ATPG_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DCES_ATPG_T;

typedef union {
    enum { COUNT = 7 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_DCES_DC_CTRL_T  			dc_ctrl;
        ISP_NVRAM_DCES_CROP_X_T  			crop_x;
        ISP_NVRAM_DCES_CROP_Y_T  			crop_y;
        ISP_NVRAM_DCES_V_Y_CH_0_T  			v_y_ch_0;
        ISP_NVRAM_DCES_V_Y_CH_1_T  			v_y_ch_1;
        ISP_NVRAM_DCES_SIZE_T  			    size;
        ISP_NVRAM_DCES_ATPG_T  			    atpg;
    };
} ISP_NVRAM_DCES_T;
#endif


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// DCE
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct {
    FIELD  DCE_TC_P1                                  : 11;		/*  0..10, 0x000007FF */
    FIELD  rsv_11                                     :  5;		/* 11..15, 0x0000F800 */
    FIELD  DCE_TC_P2                                  : 11;		/* 16..26, 0x07FF0000 */
    FIELD  rsv_27                                     :  5;		/* 27..31, 0xF8000000 */
} ISP_DCE_TC_G1_T; //	/* 0x15027000 */

typedef union {
    typedef ISP_DCE_TC_G1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DCE_TC_G1_T;

typedef struct {
    FIELD  DCE_TC_P3                                  : 11;		/*  0..10, 0x000007FF */
    FIELD  rsv_11                                     :  5;		/* 11..15, 0x0000F800 */
    FIELD  DCE_TC_P4                                  : 11;		/* 16..26, 0x07FF0000 */
    FIELD  rsv_27                                     :  5;		/* 27..31, 0xF8000000 */
} ISP_DCE_TC_G2_T; //	/* 0x15027004 */

typedef union {
    typedef ISP_DCE_TC_G2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DCE_TC_G2_T;

typedef struct {
    FIELD  DCE_TC_P5                                  : 11;		/*  0..10, 0x000007FF */
    FIELD  rsv_11                                     :  5;		/* 11..15, 0x0000F800 */
    FIELD  DCE_TC_P6                                  : 11;		/* 16..26, 0x07FF0000 */
    FIELD  rsv_27                                     :  5;		/* 27..31, 0xF8000000 */
} ISP_DCE_TC_G3_T; //	/* 0x15027008 */

typedef union {
    typedef ISP_DCE_TC_G3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DCE_TC_G3_T;

typedef struct {
    FIELD  DCE_TC_P7                                  : 11;		/*  0..10, 0x000007FF */
    FIELD  rsv_11                                     :  5;		/* 11..15, 0x0000F800 */
    FIELD  DCE_TC_P8                                  : 11;		/* 16..26, 0x07FF0000 */
    FIELD  rsv_27                                     :  5;		/* 27..31, 0xF8000000 */
} ISP_DCE_TC_G4_T; //	/* 0x1502700C */

typedef union {
    typedef ISP_DCE_TC_G4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DCE_TC_G4_T;

typedef struct {
    FIELD  DCE_TC_P9                                  : 11;		/*  0..10, 0x000007FF */
    FIELD  rsv_11                                     :  5;		/* 11..15, 0x0000F800 */
    FIELD  DCE_TC_P10                                 : 11;		/* 16..26, 0x07FF0000 */
    FIELD  rsv_27                                     :  5;		/* 27..31, 0xF8000000 */
} ISP_DCE_TC_G5_T; //	/* 0x15027010 */

typedef union {
    typedef ISP_DCE_TC_G5_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DCE_TC_G5_T;

typedef struct {
    FIELD  DCE_TC_P11                                 : 11;		/*  0..10, 0x000007FF */
    FIELD  rsv_11                                     :  5;		/* 11..15, 0x0000F800 */
    FIELD  DCE_TC_P12                                 : 11;		/* 16..26, 0x07FF0000 */
    FIELD  rsv_27                                     :  5;		/* 27..31, 0xF8000000 */
} ISP_DCE_TC_G6_T; //	/* 0x15027014 */

typedef union {
    typedef ISP_DCE_TC_G6_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DCE_TC_G6_T;

typedef struct {
    FIELD  DCE_TC_P13                                 : 11;		/*  0..10, 0x000007FF */
    FIELD  rsv_11                                     :  5;		/* 11..15, 0x0000F800 */
    FIELD  DCE_TC_P14                                 : 11;		/* 16..26, 0x07FF0000 */
    FIELD  rsv_27                                     :  5;		/* 27..31, 0xF8000000 */
} ISP_DCE_TC_G7_T; //	/* 0x15027018 */

typedef union {
    typedef ISP_DCE_TC_G7_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DCE_TC_G7_T;

typedef struct {
    FIELD  DCE_TC_P15                                 : 11;		/*  0..10, 0x000007FF */
    FIELD  rsv_11                                     :  5;		/* 11..15, 0x0000F800 */
    FIELD  DCE_TC_P16                                 : 11;		/* 16..26, 0x07FF0000 */
    FIELD  rsv_27                                     :  5;		/* 27..31, 0xF8000000 */
} ISP_DCE_TC_G8_T; //	/* 0x1502701C */

typedef union {
    typedef ISP_DCE_TC_G8_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DCE_TC_G8_T;

typedef struct {
    FIELD  DCE_TC_P17                                 : 11;		/*  0..10, 0x000007FF */
    FIELD  rsv_11                                     :  5;		/* 11..15, 0x0000F800 */
    FIELD  DCE_TC_P18                                 : 11;		/* 16..26, 0x07FF0000 */
    FIELD  rsv_27                                     :  5;		/* 27..31, 0xF8000000 */
} ISP_DCE_TC_G9_T; //	/* 0x15027020 */

typedef union {
    typedef ISP_DCE_TC_G9_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DCE_TC_G9_T;

typedef struct {
    FIELD  DCE_TC_P19                                 : 11;		/*  0..10, 0x000007FF */
    FIELD  rsv_11                                     :  5;		/* 11..15, 0x0000F800 */
    FIELD  DCE_TC_P20                                 : 11;		/* 16..26, 0x07FF0000 */
    FIELD  rsv_27                                     :  5;		/* 27..31, 0xF8000000 */
} ISP_DCE_TC_G10_T; //	/* 0x15027024 */

typedef union {
    typedef ISP_DCE_TC_G10_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DCE_TC_G10_T;

typedef struct {
    FIELD  DCE_TC_P21                                 : 11;		/*  0..10, 0x000007FF */
    FIELD  rsv_11                                     :  5;		/* 11..15, 0x0000F800 */
    FIELD  DCE_TC_P22                                 : 11;		/* 16..26, 0x07FF0000 */
    FIELD  rsv_27                                     :  5;		/* 27..31, 0xF8000000 */
} ISP_DCE_TC_G11_T; //	/* 0x15027028 */

typedef union {
    typedef ISP_DCE_TC_G11_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DCE_TC_G11_T;

typedef struct {
    FIELD  DCE_TC_P23                                 : 11;		/*  0..10, 0x000007FF */
    FIELD  rsv_11                                     :  5;		/* 11..15, 0x0000F800 */
    FIELD  DCE_TC_P24                                 : 11;		/* 16..26, 0x07FF0000 */
    FIELD  rsv_27                                     :  5;		/* 27..31, 0xF8000000 */
} ISP_DCE_TC_G12_T; //	/* 0x1502702C */

typedef union {
    typedef ISP_DCE_TC_G12_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DCE_TC_G12_T;

typedef struct {
    FIELD  DCE_TC_P25                                 : 11;		/*  0..10, 0x000007FF */
    FIELD  rsv_11                                     :  5;		/* 11..15, 0x0000F800 */
    FIELD  DCE_TC_P26                                 : 11;		/* 16..26, 0x07FF0000 */
    FIELD  rsv_27                                     :  5;		/* 27..31, 0xF8000000 */
} ISP_DCE_TC_G13_T; //	/* 0x15027030 */

typedef union {
    typedef ISP_DCE_TC_G13_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DCE_TC_G13_T;

typedef struct {
    FIELD  DCE_TC_P27                                 : 11;		/*  0..10, 0x000007FF */
    FIELD  rsv_11                                     :  5;		/* 11..15, 0x0000F800 */
    FIELD  DCE_TC_P28                                 : 11;		/* 16..26, 0x07FF0000 */
    FIELD  rsv_27                                     :  5;		/* 27..31, 0xF8000000 */
} ISP_DCE_TC_G14_T; //	/* 0x15027034 */

typedef union {
    typedef ISP_DCE_TC_G14_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DCE_TC_G14_T;

typedef struct {
    FIELD  DCE_TC_P29                                 : 11;		/*  0..10, 0x000007FF */
    FIELD  rsv_11                                     :  5;		/* 11..15, 0x0000F800 */
    FIELD  DCE_TC_P30                                 : 11;		/* 16..26, 0x07FF0000 */
    FIELD  rsv_27                                     :  5;		/* 27..31, 0xF8000000 */
} ISP_DCE_TC_G15_T; //	/* 0x15027038 */

typedef union {
    typedef ISP_DCE_TC_G15_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DCE_TC_G15_T;

typedef struct {
    FIELD  DCE_TC_P31                                 : 11;		/*  0..10, 0x000007FF */
    FIELD  rsv_11                                     :  5;		/* 11..15, 0x0000F800 */
    FIELD  DCE_TC_P32                                 : 11;		/* 16..26, 0x07FF0000 */
    FIELD  rsv_27                                     :  5;		/* 27..31, 0xF8000000 */
} ISP_DCE_TC_G16_T; //	/* 0x1502703C */

typedef union {
    typedef ISP_DCE_TC_G16_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DCE_TC_G16_T;

typedef struct {
    FIELD  DCE_TC_P33                                 : 11;		/*  0..10, 0x000007FF */
    FIELD  rsv_11                                     :  5;		/* 11..15, 0x0000F800 */
    FIELD  DCE_TC_P34                                 : 11;		/* 16..26, 0x07FF0000 */
    FIELD  rsv_27                                     :  5;		/* 27..31, 0xF8000000 */
} ISP_DCE_TC_G17_T; //	/* 0x15027040 */

typedef union {
    typedef ISP_DCE_TC_G17_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DCE_TC_G17_T;

typedef struct {
    FIELD  DCE_TC_P35                                 : 11;		/*  0..10, 0x000007FF */
    FIELD  rsv_11                                     :  5;		/* 11..15, 0x0000F800 */
    FIELD  DCE_TC_P36                                 : 11;		/* 16..26, 0x07FF0000 */
    FIELD  rsv_27                                     :  5;		/* 27..31, 0xF8000000 */
} ISP_DCE_TC_G18_T; //	/* 0x15027044 */

typedef union {
    typedef ISP_DCE_TC_G18_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DCE_TC_G18_T;

typedef struct {
    FIELD  DCE_TC_P37                                 : 11;		/*  0..10, 0x000007FF */
    FIELD  rsv_11                                     :  5;		/* 11..15, 0x0000F800 */
    FIELD  DCE_TC_P38                                 : 11;		/* 16..26, 0x07FF0000 */
    FIELD  rsv_27                                     :  5;		/* 27..31, 0xF8000000 */
} ISP_DCE_TC_G19_T; //	/* 0x15027048 */

typedef union {
    typedef ISP_DCE_TC_G19_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DCE_TC_G19_T;

typedef struct {
    FIELD  DCE_TC_P39                                 : 11;		/*  0..10, 0x000007FF */
    FIELD  rsv_11                                     :  5;		/* 11..15, 0x0000F800 */
    FIELD  DCE_TC_P40                                 : 11;		/* 16..26, 0x07FF0000 */
    FIELD  rsv_27                                     :  5;		/* 27..31, 0xF8000000 */
} ISP_DCE_TC_G20_T; //	/* 0x1502704C */

typedef union {
    typedef ISP_DCE_TC_G20_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DCE_TC_G20_T;

typedef struct {
    FIELD  DCE_TC_P41                                 : 11;		/*  0..10, 0x000007FF */
    FIELD  rsv_11                                     :  5;		/* 11..15, 0x0000F800 */
    FIELD  DCE_TC_P42                                 : 11;		/* 16..26, 0x07FF0000 */
    FIELD  rsv_27                                     :  5;		/* 27..31, 0xF8000000 */
} ISP_DCE_TC_G21_T; //	/* 0x15027050 */

typedef union {
    typedef ISP_DCE_TC_G21_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DCE_TC_G21_T;

typedef struct {
    FIELD  DCE_TC_P43                                 : 11;		/*  0..10, 0x000007FF */
    FIELD  rsv_11                                     :  5;		/* 11..15, 0x0000F800 */
    FIELD  DCE_TC_P44                                 : 11;		/* 16..26, 0x07FF0000 */
    FIELD  rsv_27                                     :  5;		/* 27..31, 0xF8000000 */
} ISP_DCE_TC_G22_T; //	/* 0x15027054 */

typedef union {
    typedef ISP_DCE_TC_G22_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DCE_TC_G22_T;

typedef struct {
    FIELD  DCE_TC_P45                                 : 11;		/*  0..10, 0x000007FF */
    FIELD  rsv_11                                     :  5;		/* 11..15, 0x0000F800 */
    FIELD  DCE_TC_P46                                 : 11;		/* 16..26, 0x07FF0000 */
    FIELD  rsv_27                                     :  5;		/* 27..31, 0xF8000000 */
} ISP_DCE_TC_G23_T; //	/* 0x15027058 */

typedef union {
    typedef ISP_DCE_TC_G23_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DCE_TC_G23_T;

typedef struct {
    FIELD  DCE_TC_P47                                 : 11;		/*  0..10, 0x000007FF */
    FIELD  rsv_11                                     :  5;		/* 11..15, 0x0000F800 */
    FIELD  DCE_TC_P48                                 : 11;		/* 16..26, 0x07FF0000 */
    FIELD  rsv_27                                     :  5;		/* 27..31, 0xF8000000 */
} ISP_DCE_TC_G24_T; //	/* 0x1502705C */

typedef union {
    typedef ISP_DCE_TC_G24_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DCE_TC_G24_T;

typedef struct {
    FIELD  DCE_TC_P49                                 : 11;		/*  0..10, 0x000007FF */
    FIELD  rsv_11                                     :  5;		/* 11..15, 0x0000F800 */
    FIELD  DCE_TC_P50                                 : 11;		/* 16..26, 0x07FF0000 */
    FIELD  rsv_27                                     :  5;		/* 27..31, 0xF8000000 */
} ISP_DCE_TC_G25_T; //	/* 0x15027060 */

typedef union {
    typedef ISP_DCE_TC_G25_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DCE_TC_G25_T;

typedef struct {
    FIELD  DCE_TC_P51                                 : 11;		/*  0..10, 0x000007FF */
    FIELD  rsv_11                                     :  5;		/* 11..15, 0x0000F800 */
    FIELD  DCE_TC_P52                                 : 11;		/* 16..26, 0x07FF0000 */
    FIELD  rsv_27                                     :  5;		/* 27..31, 0xF8000000 */
} ISP_DCE_TC_G26_T; //	/* 0x15027064 */

typedef union {
    typedef ISP_DCE_TC_G26_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DCE_TC_G26_T;

typedef struct {
    FIELD  DCE_TC_P53                                 : 11;		/*  0..10, 0x000007FF */
    FIELD  rsv_11                                     :  5;		/* 11..15, 0x0000F800 */
    FIELD  DCE_TC_P54                                 : 11;		/* 16..26, 0x07FF0000 */
    FIELD  rsv_27                                     :  5;		/* 27..31, 0xF8000000 */
} ISP_DCE_TC_G27_T; //	/* 0x15027068 */

typedef union {
    typedef ISP_DCE_TC_G27_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DCE_TC_G27_T;

typedef struct {
    FIELD  DCE_TC_P55                                 : 11;		/*  0..10, 0x000007FF */
    FIELD  rsv_11                                     :  5;		/* 11..15, 0x0000F800 */
    FIELD  DCE_TC_P56                                 : 11;		/* 16..26, 0x07FF0000 */
    FIELD  rsv_27                                     :  5;		/* 27..31, 0xF8000000 */
} ISP_DCE_TC_G28_T; //	/* 0x1502706C */

typedef union {
    typedef ISP_DCE_TC_G28_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DCE_TC_G28_T;

typedef struct {
    FIELD  DCE_TC_P57                                 : 11;		/*  0..10, 0x000007FF */
    FIELD  rsv_11                                     :  5;		/* 11..15, 0x0000F800 */
    FIELD  DCE_TC_P58                                 : 11;		/* 16..26, 0x07FF0000 */
    FIELD  rsv_27                                     :  5;		/* 27..31, 0xF8000000 */
} ISP_DCE_TC_G29_T; //	/* 0x15027070 */

typedef union {
    typedef ISP_DCE_TC_G29_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DCE_TC_G29_T;

typedef struct {
    FIELD  DCE_TC_P59                                 : 11;		/*  0..10, 0x000007FF */
    FIELD  rsv_11                                     :  5;		/* 11..15, 0x0000F800 */
    FIELD  DCE_TC_P60                                 : 11;		/* 16..26, 0x07FF0000 */
    FIELD  rsv_27                                     :  5;		/* 27..31, 0xF8000000 */
} ISP_DCE_TC_G30_T; //	/* 0x15027074 */

typedef union {
    typedef ISP_DCE_TC_G30_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DCE_TC_G30_T;

typedef struct {
    FIELD  DCE_TC_P61                                 : 11;		/*  0..10, 0x000007FF */
    FIELD  rsv_11                                     :  5;		/* 11..15, 0x0000F800 */
    FIELD  DCE_TC_P62                                 : 11;		/* 16..26, 0x07FF0000 */
    FIELD  rsv_27                                     :  5;		/* 27..31, 0xF8000000 */
} ISP_DCE_TC_G31_T; //	/* 0x15027078 */

typedef union {
    typedef ISP_DCE_TC_G31_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DCE_TC_G31_T;

typedef struct {
    FIELD  DCE_TC_P63                                 : 11;		/*  0..10, 0x000007FF */
    FIELD  rsv_11                                     :  5;		/* 11..15, 0x0000F800 */
    FIELD  DCE_TC_P64                                 : 11;		/* 16..26, 0x07FF0000 */
    FIELD  rsv_27                                     :  5;		/* 27..31, 0xF8000000 */
} ISP_DCE_TC_G32_T; //	/* 0x1502707C */

typedef union {
    typedef ISP_DCE_TC_G32_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DCE_TC_G32_T;

typedef struct {
    FIELD  DCE_CNV_G                                  : 11;		/*  0..10, 0x000007FF */
    FIELD  rsv_11                                     :  5;		/* 11..15, 0x0000F800 */
    FIELD  DCE_CNV_R                                  : 11;		/* 16..26, 0x07FF0000 */
    FIELD  rsv_27                                     :  5;		/* 27..31, 0xF8000000 */
} ISP_DCE_V_Y_CH_0_T; //	/* 0x15027080 */

typedef union {
    typedef ISP_DCE_V_Y_CH_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DCE_V_Y_CH_0_T;

typedef struct {
    FIELD  DCE_Y_OFST                                 : 11;		/*  0..10, 0x000007FF */
    FIELD  DCE_CNV_B                                  : 11;		/* 11..21, 0x003FF800 */
    FIELD  rsv_22                                     :  4;		/* 22..25, 0x03C00000 */
    FIELD  DCE_VY_BLD_WT                              :  6;		/* 26..31, 0xFC000000 */
} ISP_DCE_V_Y_CH_1_T; //	/* 0x15027084 */

typedef union {
    typedef ISP_DCE_V_Y_CH_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DCE_V_Y_CH_1_T;

typedef struct {
    FIELD  DCE_relay_mode                             :  1;		/*  0.. 0, 0x00000001 */
    FIELD  DCE_dce_engine_en                          :  1;		/*  1.. 1, 0x00000002 */
    FIELD  DCE_dce_reset                              :  1;		/*  2.. 2, 0x00000004 */
    FIELD  rsv_3                                      : 29;		/*  3..31, 0xFFFFFFF8 */
} ISP_DCE_DCE_CONF_T; //	/* 0x15027088 */

typedef union {
    typedef ISP_DCE_DCE_CONF_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DCE_DCE_CONF_T;

typedef struct {
    FIELD  DCE_vsize                                  : 13;		/*  0..12, 0x00001FFF */
    FIELD  DCE_hsize                                  : 13;		/* 13..25, 0x03FFE000 */
    FIELD  rsv_26                                     :  6;		/* 26..31, 0xFC000000 */
} ISP_DCE_SIZE_T; //	/* 0x1502708C */

typedef union {
    typedef ISP_DCE_SIZE_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DCE_SIZE_T;

typedef struct {
    FIELD  DCE_dce_atpg_ct                            :  1;		/*  0.. 0, 0x00000001 */
    FIELD  DCE_dce_atpg_ob                            :  1;		/*  1.. 1, 0x00000002 */
    FIELD  rsv_2                                      : 30;		/*  2..31, 0xFFFFFFFC */
} ISP_DCE_ATPG_T; //	/* 0x15027090 */

typedef union {
    typedef ISP_DCE_ATPG_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DCE_ATPG_T;

typedef union {
    enum { COUNT = 32 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_DCE_TC_G1_T  			tc_g1;
        ISP_NVRAM_DCE_TC_G2_T  			tc_g2;
        ISP_NVRAM_DCE_TC_G3_T  			tc_g3;
        ISP_NVRAM_DCE_TC_G4_T  			tc_g4;
        ISP_NVRAM_DCE_TC_G5_T  			tc_g5;
        ISP_NVRAM_DCE_TC_G6_T  			tc_g6;
        ISP_NVRAM_DCE_TC_G7_T  			tc_g7;
        ISP_NVRAM_DCE_TC_G8_T  			tc_g8;
        ISP_NVRAM_DCE_TC_G9_T  			tc_g9;
        ISP_NVRAM_DCE_TC_G10_T  		tc_g10;
        ISP_NVRAM_DCE_TC_G11_T  		tc_g11;
        ISP_NVRAM_DCE_TC_G12_T  		tc_g12;
        ISP_NVRAM_DCE_TC_G13_T  		tc_g13;
        ISP_NVRAM_DCE_TC_G14_T  		tc_g14;
        ISP_NVRAM_DCE_TC_G15_T  		tc_g15;
        ISP_NVRAM_DCE_TC_G16_T  		tc_g16;
        ISP_NVRAM_DCE_TC_G17_T  		tc_g17;
        ISP_NVRAM_DCE_TC_G18_T  		tc_g18;
        ISP_NVRAM_DCE_TC_G19_T  		tc_g19;
        ISP_NVRAM_DCE_TC_G20_T  		tc_g20;
        ISP_NVRAM_DCE_TC_G21_T  		tc_g21;
        ISP_NVRAM_DCE_TC_G22_T  		tc_g22;
        ISP_NVRAM_DCE_TC_G23_T  		tc_g23;
        ISP_NVRAM_DCE_TC_G24_T  		tc_g24;
        ISP_NVRAM_DCE_TC_G25_T  		tc_g25;
        ISP_NVRAM_DCE_TC_G26_T  		tc_g26;
        ISP_NVRAM_DCE_TC_G27_T  		tc_g27;
        ISP_NVRAM_DCE_TC_G28_T  		tc_g28;
        ISP_NVRAM_DCE_TC_G29_T  		tc_g29;
        ISP_NVRAM_DCE_TC_G30_T  		tc_g30;
        ISP_NVRAM_DCE_TC_G31_T  		tc_g31;
        ISP_NVRAM_DCE_TC_G32_T  		tc_g32;
        //ISP_NVRAM_DCE_V_Y_CH_0_T  		v_y_ch_0;
        //ISP_NVRAM_DCE_V_Y_CH_1_T  		v_y_ch_1;
        //ISP_NVRAM_DCE_DCE_CONF_T  		dce_conf;
        //ISP_NVRAM_DCE_SIZE_T  			size;
        //ISP_NVRAM_DCE_ATPG_T  			atpg;
    };
} ISP_NVRAM_DCE_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// MIX
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct {
    FIELD  MIX_WT_SEL                                 :  1;		/*  0.. 0, 0x00000001 */
    FIELD  MIX_SLOPE_SEL                              :  1;		/*  1.. 1, 0x00000002 */
    FIELD  rsv_2                                      :  2;		/*  2.. 3, 0x0000000C */
    FIELD  MIX_Y_EN                                   :  1;		/*  4.. 4, 0x00000010 */
    FIELD  MIX_Y_DEFAULT                              :  1;		/*  5.. 5, 0x00000020 */
    FIELD  MIX_UV_EN                                  :  1;		/*  6.. 6, 0x00000040 */
    FIELD  MIX_UV_DEFAULT                             :  1;		/*  7.. 7, 0x00000080 */
    FIELD  MIX_B0                                     :  8;		/*  8..15, 0x0000FF00 */
    FIELD  MIX_B1                                     :  8;		/* 16..23, 0x00FF0000 */
    FIELD  MIX_DT                                     :  8;		/* 24..31, 0xFF000000 */
} ISP_MIX_CTRL0_T; //	/* 0x15025280 */

typedef union {
    typedef ISP_MIX_CTRL0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MIX_CTRL0_T;

typedef struct {
    FIELD  MIX_M0                                     :  8;		/*  0.. 7, 0x000000FF */
    FIELD  rsv_8                                      :  8;		/*  8..15, 0x0000FF00 */
    FIELD  MIX_M1                                     :  8;		/* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                     :  8;		/* 24..31, 0xFF000000 */
} ISP_MIX_CTRL1_T; //	/* 0x15025284 */

typedef union {
    typedef ISP_MIX_CTRL1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MIX_CTRL1_T;

typedef union {
    enum { COUNT = 2 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_MIX_CTRL0_T  			ctrl0;
        ISP_NVRAM_MIX_CTRL1_T  			ctrl1;
    };
} ISP_NVRAM_MIX_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// MFB
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct {
    FIELD  MFB_BLD_MODE          : 2;
    FIELD  MFB_BLD_OUT_VALID     : 1;
    FIELD  rsv_3                 : 3;
    FIELD  MFB_BLD_MBD_WT_EN     : 1;
    FIELD  MFB_BLD_SR_WT_EN      : 1;
    FIELD  rsv_8                 : 8;
    FIELD  MFB_BLD_BYPASS        : 1;
    FIELD  rsv_17                : 15;
} ISP_MFB_CON_T; //	/* 0x15010200 */

typedef union {
    typedef ISP_MFB_CON_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MFB_CON_T;

typedef struct {
    FIELD  MFB_BLD_LL_FLT_MODE       : 2;
    FIELD  rsv_2                     : 2;
    FIELD  MFB_BLD_LL_FLT_WT_MODE1   : 3;
    FIELD  rsv_7                     : 1;
    FIELD  MFB_BLD_LL_FLT_WT_MODE2   : 2;
    FIELD  rsv_10                    : 6;
    FIELD  MFB_BLD_LL_CLIP_TH1       : 8;
    FIELD  MFB_BLD_LL_CLIP_TH2       : 8;
} ISP_MFB_LL_CON1_T; //	/* 0x15010204 */

typedef union {
    typedef ISP_MFB_LL_CON1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MFB_LL_CON1_T;

typedef struct {
    FIELD  rsv_0                     : 8;
    FIELD  MFB_BLD_LL_DT1            : 8;
    FIELD  MFB_BLD_LL_TH1            : 8;
    FIELD  MFB_BLD_LL_TH2            : 8;
} ISP_MFB_LL_CON2_T; //	/* 0x15010208 */

typedef union {
    typedef ISP_MFB_LL_CON2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MFB_LL_CON2_T;

typedef struct {
    FIELD  rsv_0                 : 32;
} ISP_MFB_LL_CON3_T; //	/* 0x1501020C */

typedef union {
    typedef ISP_MFB_LL_CON3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MFB_LL_CON3_T;

typedef struct {
    FIELD  rsv_0                 : 32;
} ISP_MFB_LL_CON4_T; //	/* 0x15010210 */

typedef union {
    typedef ISP_MFB_LL_CON4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MFB_LL_CON4_T;

typedef struct {
    FIELD  MFB_BLD_TILE_EDGE     : 4;
    FIELD  rsv_4                 : 28;
} ISP_MFB_EDGE_T; //	/* 0x15010214 */

typedef union {
    typedef ISP_MFB_EDGE_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MFB_EDGE_T;

typedef struct {
    FIELD  MFB_BLD_LL_GRAD_R1        : 8;
    FIELD  MFB_BLD_LL_GRAD_R2        : 8;
    FIELD  MFB_BLD_LL_TH3            : 8;
    FIELD  MFB_BLD_LL_TH4            : 8;
} ISP_MFB_LL_CON5_T; //	/* 0x15010218 */

typedef union {
    typedef ISP_MFB_LL_CON5_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MFB_LL_CON5_T;

typedef struct {
    FIELD  MFB_BLD_LL_TH5            : 8;
    FIELD  MFB_BLD_LL_TH6            : 8;
    FIELD  MFB_BLD_LL_TH7            : 8;
    FIELD  MFB_BLD_LL_TH8            : 8;
} ISP_MFB_LL_CON6_T; //	/* 0x1501021C */

typedef union {
    typedef ISP_MFB_LL_CON6_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MFB_LL_CON6_T;

typedef struct {
    FIELD  MFB_BLD_LL_C1             : 8;
    FIELD  MFB_BLD_LL_C2             : 8;
    FIELD  MFB_BLD_LL_C3             : 8;
    FIELD  rsv_24                    : 8;
} ISP_MFB_LL_CON7_T; //	/* 0x15010220 */

typedef union {
    typedef ISP_MFB_LL_CON7_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MFB_LL_CON7_T;

typedef struct {
    FIELD  MFB_BLD_LL_SU1        : 6;
    FIELD  rsv_6                 : 2;
    FIELD  MFB_BLD_LL_SU2        : 6;
    FIELD  rsv_14                : 2;
    FIELD  MFB_BLD_LL_SU3        : 6;
    FIELD  rsv_22                : 10;
} ISP_MFB_LL_CON8_T; //	/* 0x15010224 */

typedef union {
    typedef ISP_MFB_LL_CON8_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MFB_LL_CON8_T;

typedef struct {
    FIELD  MFB_BLD_LL_SL1        : 6;
    FIELD  rsv_6                 : 2;
    FIELD  MFB_BLD_LL_SL2        : 6;
    FIELD  rsv_14                : 2;
    FIELD  MFB_BLD_LL_SL3        : 6;
    FIELD  rsv_22                : 10;
} ISP_MFB_LL_CON9_T; //	/* 0x15010228 */

typedef union {
    typedef ISP_MFB_LL_CON9_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MFB_LL_CON9_T;

typedef struct {
    FIELD  MFB_BLD_CONF_MAP_EN       : 1;
    FIELD  rsv_1                     : 3;
    FIELD  MFB_BLD_LL_GRAD_EN        : 1;
    FIELD  rsv_5                     : 3;
    FIELD  MFB_BLD_LL_GRAD_ENTH      : 8;
    FIELD  MFB_BLD_LL_MAX_WT_FMT2    : 8;
    FIELD  rsv_24                    : 8;
} ISP_MFB_LL_CON10_T; //	/* 0x1501022C */

typedef union {
    typedef ISP_MFB_LL_CON10_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MFB_LL_CON10_T;

typedef struct {
    FIELD  MFB_BLD_MBD_MAX_WT    : 8;
    FIELD  rsv_8                 : 24;
} ISP_MFB_MBD_CON0_T; //	/* 0x15010230 */

typedef union {
    typedef ISP_MFB_MBD_CON0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MFB_MBD_CON0_T;

typedef struct {
    FIELD  MFB_BLD_MBD_YL1_THL_0     : 6;
    FIELD  rsv_6                     : 2;
    FIELD  MFB_BLD_MBD_YL1_THL_1     : 6;
    FIELD  rsv_14                    : 2;
    FIELD  MFB_BLD_MBD_YL1_THL_2     : 6;
    FIELD  rsv_22                    : 2;
    FIELD  MFB_BLD_MBD_YL1_THL_3     : 6;
    FIELD  rsv_30                    : 2;
} ISP_MFB_MBD_CON1_T; //	/* 0x15010234 */

typedef union {
    typedef ISP_MFB_MBD_CON1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MFB_MBD_CON1_T;

typedef struct {
    FIELD  MFB_BLD_MBD_YL1_THL_4     : 6;
    FIELD  rsv_6                     : 2;
    FIELD  MFB_BLD_MBD_YL1_THH_0     : 6;
    FIELD  rsv_14                    : 2;
    FIELD  MFB_BLD_MBD_YL1_THH_1     : 6;
    FIELD  rsv_22                    : 2;
    FIELD  MFB_BLD_MBD_YL1_THH_2     : 6;
    FIELD  rsv_30                    : 2;
} ISP_MFB_MBD_CON2_T; //	/* 0x15010238 */

typedef union {
    typedef ISP_MFB_MBD_CON2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MFB_MBD_CON2_T;

typedef struct {
    FIELD  MFB_BLD_MBD_YL1_THH_3     : 6;
    FIELD  rsv_6                     : 2;
    FIELD  MFB_BLD_MBD_YL1_THH_4     : 6;
    FIELD  rsv_14                    : 2;
    FIELD  MFB_BLD_MBD_YL2_THL_0     : 6;
    FIELD  rsv_22                    : 2;
    FIELD  MFB_BLD_MBD_YL2_THL_1     : 6;
    FIELD  rsv_30                    : 2;
} ISP_MFB_MBD_CON3_T; //	/* 0x1501023C */

typedef union {
    typedef ISP_MFB_MBD_CON3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MFB_MBD_CON3_T;

typedef struct {
    FIELD  MFB_BLD_MBD_YL2_THL_2     : 6;
    FIELD  rsv_6                     : 2;
    FIELD  MFB_BLD_MBD_YL2_THL_3     : 6;
    FIELD  rsv_14                    : 2;
    FIELD  MFB_BLD_MBD_YL2_THL_4     : 6;
    FIELD  rsv_22                    : 2;
    FIELD  MFB_BLD_MBD_YL2_THH_0     : 6;
    FIELD  rsv_30                    : 2;
} ISP_MFB_MBD_CON4_T; //	/* 0x15010240 */

typedef union {
    typedef ISP_MFB_MBD_CON4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MFB_MBD_CON4_T;

typedef struct {
    FIELD  MFB_BLD_MBD_YL2_THH_1     : 6;
    FIELD  rsv_6                     : 2;
    FIELD  MFB_BLD_MBD_YL2_THH_2     : 6;
    FIELD  rsv_14                    : 2;
    FIELD  MFB_BLD_MBD_YL2_THH_3     : 6;
    FIELD  rsv_22                    : 2;
    FIELD  MFB_BLD_MBD_YL2_THH_4     : 6;
    FIELD  rsv_30                    : 2;
} ISP_MFB_MBD_CON5_T; //	/* 0x15010244 */

typedef union {
    typedef ISP_MFB_MBD_CON5_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MFB_MBD_CON5_T;

typedef struct {
    FIELD  MFB_BLD_MBD_YL3_THL_0     : 6;
    FIELD  rsv_6                     : 2;
    FIELD  MFB_BLD_MBD_YL3_THL_1     : 6;
    FIELD  rsv_14                    : 2;
    FIELD  MFB_BLD_MBD_YL3_THL_2     : 6;
    FIELD  rsv_22                    : 2;
    FIELD  MFB_BLD_MBD_YL3_THL_3     : 6;
    FIELD  rsv_30                    : 2;
} ISP_MFB_MBD_CON6_T; //	/* 0x15010248 */

typedef union {
    typedef ISP_MFB_MBD_CON6_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MFB_MBD_CON6_T;

typedef struct {
    FIELD  MFB_BLD_MBD_YL3_THL_4     : 6;
    FIELD  rsv_6                     : 2;
    FIELD  MFB_BLD_MBD_YL3_THH_0     : 6;
    FIELD  rsv_14                    : 2;
    FIELD  MFB_BLD_MBD_YL3_THH_1     : 6;
    FIELD  rsv_22                    : 2;
    FIELD  MFB_BLD_MBD_YL3_THH_2     : 6;
    FIELD  rsv_30                    : 2;
} ISP_MFB_MBD_CON7_T; //	/* 0x1501024C */

typedef union {
    typedef ISP_MFB_MBD_CON7_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MFB_MBD_CON7_T;

typedef struct {
    FIELD  MFB_BLD_MBD_YL3_THH_3     : 6;
    FIELD  rsv_6                     : 2;
    FIELD  MFB_BLD_MBD_YL3_THH_4     : 6;
    FIELD  rsv_14                    : 2;
    FIELD  MFB_BLD_MBD_CL1_THL_0     : 6;
    FIELD  rsv_22                    : 2;
    FIELD  MFB_BLD_MBD_CL1_THL_1     : 6;
    FIELD  rsv_30                    : 2;
} ISP_MFB_MBD_CON8_T; //	/* 0x15010250 */

typedef union {
    typedef ISP_MFB_MBD_CON8_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MFB_MBD_CON8_T;

typedef struct {
    FIELD  MFB_BLD_MBD_CL1_THL_2     : 6;
    FIELD  rsv_6                     : 2;
    FIELD  MFB_BLD_MBD_CL1_THL_3     : 6;
    FIELD  rsv_14                    : 2;
    FIELD  MFB_BLD_MBD_CL1_THL_4     : 6;
    FIELD  rsv_22                    : 2;
    FIELD  MFB_BLD_MBD_CL1_THH_0     : 6;
    FIELD  rsv_30                    : 2;
} ISP_MFB_MBD_CON9_T; //	/* 0x15010254 */

typedef union {
    typedef ISP_MFB_MBD_CON9_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MFB_MBD_CON9_T;

typedef struct {
    FIELD  MFB_BLD_MBD_CL1_THH_1     : 6;
    FIELD  rsv_6                     : 2;
    FIELD  MFB_BLD_MBD_CL1_THH_2     : 6;
    FIELD  rsv_14                    : 2;
    FIELD  MFB_BLD_MBD_CL1_THH_3     : 6;
    FIELD  rsv_22                    : 2;
    FIELD  MFB_BLD_MBD_CL1_THH_4     : 6;
    FIELD  rsv_30                    : 2;
} ISP_MFB_MBD_CON10_T; //	/* 0x15010258 */

typedef union {
    typedef ISP_MFB_MBD_CON10_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MFB_MBD_CON10_T;


typedef union {
    enum { COUNT = 10 };
    MUINT32 set[COUNT];
    struct {
        //ISP_NVRAM_MFB_CON_T  			    con;
        //ISP_NVRAM_MFB_LL_CON1_T  			ll_con1;
        //ISP_NVRAM_MFB_LL_CON2_T  			ll_con2;
        //ISP_NVRAM_MFB_LL_CON3_T  			ll_con3;
        //ISP_NVRAM_MFB_LL_CON4_T  			ll_con4;
        //ISP_NVRAM_MFB_EDGE_T  		    edge;
        //ISP_NVRAM_MFB_LL_CON5_T  			ll_con5;
        //ISP_NVRAM_MFB_LL_CON6_T  			ll_con6;
        //ISP_NVRAM_MFB_LL_CON7_T  			ll_con7;
        //ISP_NVRAM_MFB_LL_CON8_T  			ll_con8;
        //ISP_NVRAM_MFB_LL_CON9_T  			ll_con9;
        //ISP_NVRAM_MFB_LL_CON10_T  		ll_con10;
        //ISP_NVRAM_MFB_MBD_CON0_T  		mbd_con0;
        ISP_NVRAM_MFB_MBD_CON1_T  			mbd_con1;
        ISP_NVRAM_MFB_MBD_CON2_T  			mbd_con2;
        ISP_NVRAM_MFB_MBD_CON3_T  			mbd_con3;
        ISP_NVRAM_MFB_MBD_CON4_T  			mbd_con4;
        ISP_NVRAM_MFB_MBD_CON5_T  			mbd_con5;
        ISP_NVRAM_MFB_MBD_CON6_T  			mbd_con6;
        ISP_NVRAM_MFB_MBD_CON7_T  			mbd_con7;
        ISP_NVRAM_MFB_MBD_CON8_T  			mbd_con8;
        ISP_NVRAM_MFB_MBD_CON9_T  			mbd_con9;
        ISP_NVRAM_MFB_MBD_CON10_T  			mbd_con10;
    };
} ISP_NVRAM_MFB_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// LSC
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct {
    FIELD  LSC_SDBLK_YOFST                            :  6;		/*  0.. 5, 0x0000003F */
    FIELD  rsv_6                                      : 10;		/*  6..15, 0x0000FFC0 */
    FIELD  LSC_SDBLK_XOFST                            :  6;		/* 16..21, 0x003F0000 */
    FIELD  LSC_EXTEND_COEF_MODE                       :  1;		/* 22..22, 0x00400000 */
    FIELD  rsv_23                                     :  1;		/* 23..23, 0x00800000 */
    FIELD  LSC_SD_COEFRD_MODE                         :  1;		/* 24..24, 0x01000000 */
    FIELD  rsv_25                                     :  3;		/* 25..27, 0x0E000000 */
    FIELD  LSC_SD_ULTRA_MODE                          :  1;		/* 28..28, 0x10000000 */
    FIELD  LSC_PRC_MODE                               :  1;		/* 29..29, 0x20000000 */
    FIELD  rsv_30                                     :  2;		/* 30..31, 0xC0000000 */
} ISP_LSC_CTL1_T; //	/* 0x15023340 */

typedef union {
    typedef ISP_LSC_CTL1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LSC_CTL1_T;

typedef struct {
    FIELD  LSC_SDBLK_WIDTH                            : 13;		/*  0..12, 0x00001FFF */
    FIELD  LSC_SDBLK_XNUM                             :  5;		/* 13..17, 0x0003E000 */
    FIELD  LSC_OFLN                                   :  1;		/* 18..18, 0x00040000 */
    FIELD  rsv_19                                     : 13;		/* 19..31, 0xFFF80000 */
} ISP_LSC_CTL2_T; //	/* 0x15023344 */

typedef union {
    typedef ISP_LSC_CTL2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LSC_CTL2_T;

typedef struct {
    FIELD  LSC_SDBLK_HEIGHT                           : 13;		/*  0..12, 0x00001FFF */
    FIELD  LSC_SDBLK_YNUM                             :  5;		/* 13..17, 0x0003E000 */
    FIELD  LSC_SPARE                                  : 14;		/* 18..31, 0xFFFC0000 */
} ISP_LSC_CTL3_T; //	/* 0x15023348 */

typedef union {
    typedef ISP_LSC_CTL3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LSC_CTL3_T;

typedef struct {
    FIELD  LSC_SDBLK_lHEIGHT                          : 13;		/*  0..12, 0x00001FFF */
    FIELD  rsv_13                                     :  3;		/* 13..15, 0x0000E000 */
    FIELD  LSC_SDBLK_lWIDTH                           : 13;		/* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                     :  3;		/* 29..31, 0xE0000000 */
} ISP_LSC_LBLOCK_T; //	/* 0x1502334C */

typedef union {
    typedef ISP_LSC_LBLOCK_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LSC_LBLOCK_T;

typedef struct {
    FIELD  LSC_RA03                                   :  6;		/*  0.. 5, 0x0000003F */
    FIELD  rsv_6                                      :  2;		/*  6.. 7, 0x000000C0 */
    FIELD  LSC_RA02                                   :  6;		/*  8..13, 0x00003F00 */
    FIELD  rsv_14                                     :  2;		/* 14..15, 0x0000C000 */
    FIELD  LSC_RA01                                   :  6;		/* 16..21, 0x003F0000 */
    FIELD  rsv_22                                     :  2;		/* 22..23, 0x00C00000 */
    FIELD  LSC_RA00                                   :  6;		/* 24..29, 0x3F000000 */
    FIELD  rsv_30                                     :  2;		/* 30..31, 0xC0000000 */
} ISP_LSC_RATIO_0_T; //	/* 0x15023350 */

typedef union {
    typedef ISP_LSC_RATIO_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LSC_RATIO_0_T;

typedef struct {
    FIELD  LSC_TPIPE_OFST_Y                           : 13;		/*  0..12, 0x00001FFF */
    FIELD  rsv_13                                     :  3;		/* 13..15, 0x0000E000 */
    FIELD  LSC_TPIPE_OFST_X                           : 13;		/* 16..28, 0x1FFF0000 */
    FIELD  rsv_29                                     :  3;		/* 29..31, 0xE0000000 */
} ISP_LSC_TPIPE_OFST_T; //	/* 0x15023354 */

typedef union {
    typedef ISP_LSC_TPIPE_OFST_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LSC_TPIPE_OFST_T;

typedef struct {
    FIELD  LSC_TPIPE_SIZE_Y                           : 14;		/*  0..13, 0x00003FFF */
    FIELD  rsv_14                                     :  2;		/* 14..15, 0x0000C000 */
    FIELD  LSC_TPIPE_SIZE_X                           : 14;		/* 16..29, 0x3FFF0000 */
    FIELD  rsv_30                                     :  2;		/* 30..31, 0xC0000000 */
} ISP_LSC_TPIPE_SIZE_T; //	/* 0x15023358 */

typedef union {
    typedef ISP_LSC_TPIPE_SIZE_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LSC_TPIPE_SIZE_T;

typedef struct {
    FIELD  LSC_GAIN_TH2                               :  9;		/*  0.. 8, 0x000001FF */
    FIELD  rsv_9                                      :  1;		/*  9.. 9, 0x00000200 */
    FIELD  LSC_GAIN_TH1                               :  9;		/* 10..18, 0x0007FC00 */
    FIELD  rsv_19                                     :  1;		/* 19..19, 0x00080000 */
    FIELD  LSC_GAIN_TH0                               :  9;		/* 20..28, 0x1FF00000 */
    FIELD  rsv_29                                     :  3;		/* 29..31, 0xE0000000 */
} ISP_LSC_GAIN_TH_T; //	/* 0x1502335C */

typedef union {
    typedef ISP_LSC_GAIN_TH_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LSC_GAIN_TH_T;

typedef struct {
    FIELD  LSC_RA13                                   :  6;		/*  0.. 5, 0x0000003F */
    FIELD  rsv_6                                      :  2;		/*  6.. 7, 0x000000C0 */
    FIELD  LSC_RA12                                   :  6;		/*  8..13, 0x00003F00 */
    FIELD  rsv_14                                     :  2;		/* 14..15, 0x0000C000 */
    FIELD  LSC_RA11                                   :  6;		/* 16..21, 0x003F0000 */
    FIELD  rsv_22                                     :  2;		/* 22..23, 0x00C00000 */
    FIELD  LSC_RA10                                   :  6;		/* 24..29, 0x3F000000 */
    FIELD  rsv_30                                     :  2;		/* 30..31, 0xC0000000 */
} ISP_LSC_RATIO_1_T; //	/* 0x15023360 */

typedef union {
    typedef ISP_LSC_RATIO_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LSC_RATIO_1_T;

typedef struct {
    FIELD  LSC_UPB_GB                                 : 14;		/*  0..13, 0x00003FFF */
    FIELD  rsv_14                                     :  2;		/* 14..15, 0x0000C000 */
    FIELD  LSC_UPB_B                                  : 14;		/* 16..29, 0x3FFF0000 */
    FIELD  rsv_30                                     :  2;		/* 30..31, 0xC0000000 */
} ISP_LSC_UPB_B_GB_T; //	/* 0x15023364 */

typedef union {
    typedef ISP_LSC_UPB_B_GB_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LSC_UPB_B_GB_T;

typedef struct {
    FIELD  LSC_UPB_R                                  : 14;		/*  0..13, 0x00003FFF */
    FIELD  rsv_14                                     :  2;		/* 14..15, 0x0000C000 */
    FIELD  LSC_UPB_GR                                 : 14;		/* 16..29, 0x3FFF0000 */
    FIELD  rsv_30                                     :  2;		/* 30..31, 0xC0000000 */
} ISP_LSC_UPB_GR_R_T; //	/* 0x15023368 */

typedef union {
    typedef ISP_LSC_UPB_GR_R_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LSC_UPB_GR_R_T;

typedef union {
    enum { COUNT = 11 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_LSC_CTL1_T  			ctl1;
        ISP_NVRAM_LSC_CTL2_T  			ctl2;
        ISP_NVRAM_LSC_CTL3_T  			ctl3;
        ISP_NVRAM_LSC_LBLOCK_T  		lblock;
        ISP_NVRAM_LSC_RATIO_0_T  		ratio_0;
        ISP_NVRAM_LSC_TPIPE_OFST_T  	tpipe_ofst;
        ISP_NVRAM_LSC_TPIPE_SIZE_T  	tpipe_size;
        ISP_NVRAM_LSC_GAIN_TH_T  		gain_th;
        ISP_NVRAM_LSC_RATIO_1_T  		ratio_1;
        ISP_NVRAM_LSC_UPB_B_GB_T  		upb_b_gb;
        ISP_NVRAM_LSC_UPB_GR_R_T  		upb_gr_r;
    };
} ISP_NVRAM_LSC_T;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// interpolation
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#if 0
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

#endif

//Chooo
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

};
typedef struct ISP_INDEX_STRUCT
{
    MUINT16  OBC;
    MUINT16  BPC;
    MUINT16  CT;
    MUINT16  PDC;
    MUINT16  DM;
    MUINT16  YNR;
    MUINT16  CNR;
    MUINT16  ABF;
    MUINT16  CCR;
    MUINT16  BOK;
    MUINT16  EE;
    MUINT16  NR3D;
    MUINT16  MFB;
    MUINT16  MIXER3;
    MUINT16  SL;
    MUINT16  CCM;
    MUINT16  GGM;
    MUINT16  COLOR;

    //UINT16 PCA;
    //UINT16 ANR_TBL;
} ISP_INDEX_T;

typedef struct ISP_DEBUG_STRUCT //Disable
{
    MBOOL  bOBC_R1;
    MBOOL  bOBC_D1;
    MBOOL  bMOBC_R2;
    MBOOL  bBPC_BPC_R1;
    MBOOL  bBPC_CT_R1;
    MBOOL  bBPC_PDC_R1;
    MBOOL  bBPC_BPC_D1;
    MBOOL  bBPC_CT_D1;
    MBOOL  bBPC_PDC_D1;
    MBOOL  bDGN_R1;
    MBOOL  bDGN_D1;
    MBOOL  bLSC_R1;
    MBOOL  bLSC_D1;
    MBOOL  bWB_R1;
    MBOOL  bWB_D1;
    MBOOL  bHLR_R1;
    MBOOL  bHLR_D1;
    MBOOL  bLTM_R1;
    MBOOL  bLTM_D1;
    MBOOL  bSLK_R1;
    MBOOL  bSLK_R2;
    MBOOL  bSLK_D1;
    MBOOL  bSLK_D2;
    MBOOL  bSLK_D3;
    MBOOL  bSLK_D4;
    MBOOL  bSLK_D5;
    MBOOL  bSLK_D6;
    MBOOL  bDM_R1;
    MBOOL  bDM_D1;
    MBOOL  bFLC_R1;
    MBOOL  bFLC_D1;
    MBOOL  bCCM_R1;
    MBOOL  bCCM_D1;
    MBOOL  bCCM_D2;
    MBOOL  bCCM_D3;
    MBOOL  bGGM_R1;
    MBOOL  bGGM_R2;
    MBOOL  bGGM_D1;
    MBOOL  bGGM_D2;
    MBOOL  bGGM_D3;
    MBOOL  bIGGM_D1;
    MBOOL  bG2C_R1;
    MBOOL  bG2C_R2;
    MBOOL  bG2C_D1;
    MBOOL  bG2CX_D1;
    MBOOL  bC2G_D1;
    MBOOL  bYNRS_R1;
    MBOOL  bYNR_D1;
    MBOOL  bYNR_D1_LINK;
    MBOOL  bYNR_D1_MAP;
    MBOOL  bLDNR_D1;
    MBOOL  bNDG_D1;
    MBOOL  bNDG_D2;
    MBOOL  bLCES_R1;
    MBOOL  bLCE_D1;
    MBOOL  bDCES_D1;
    MBOOL  bDCE_D1;
    MBOOL  bEE_D1;
    MBOOL  bCNR_CNR_D1;
    MBOOL  bCNR_CCR_D1;
    MBOOL  bCNR_ABF_D1;
    MBOOL  bCNR_BOK_D1;
    MBOOL  bCOLOR_D1;
    MBOOL  bMIX_D3;
    MBOOL  bMFB_D1;
    MBOOL  bNR3D_D1;
    MBOOL  bLPCNR_D1;
} ISP_DEBUG_CONTROL_T;

#endif  //  _ISPIF_


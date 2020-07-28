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

typedef MUINT32 FIELD;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ISP Enable (Pass1@TG1)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CAM_CTL_EN_P1 CAM+0004H: ISP Enable (Pass1@TG1)
typedef struct {
        FIELD  TG1_EN                                :  1;      /*  0.. 0, 0x00000001 */
        FIELD  DMX_EN                                :  1;      /*  1.. 1, 0x00000002 */
        FIELD  rsv_2                                 :  1;      /*  2.. 2, 0x00000004 */
        FIELD  rsv_3                                 :  1;      /*  3.. 3, 0x00000008 */
        FIELD  OB_EN                                 :  1;      /*  4.. 4, 0x00000010 */
        FIELD  BNR_EN                                :  1;      /*  5.. 5, 0x00000020 */
        FIELD  LSC_EN                                :  1;      /*  6.. 6, 0x00000040 */
        FIELD  RPG_EN                                :  1;      /*  7.. 7, 0x00000080 */
        FIELD  RRZ_EN                                :  1;      /*  8.. 8, 0x00000100 */
        FIELD  RMX_EN                                :  1;      /*  9.. 9, 0x00000200 */
        FIELD  PAKG_EN                               :  1;      /* 10..10, 0x00000400 */
        FIELD  BMX_EN                                :  1;      /* 11..11, 0x00000800 */
        FIELD  PAK_EN                                :  1;      /* 12..12, 0x00001000 */
        FIELD  rsv_13                                :  1;      /* 13..13, 0x00002000 */
        FIELD  rsv_14                                :  1;      /* 14..14, 0x00004000 */
        FIELD  SGG1_EN                               :  1;      /* 15..15, 0x00008000 */
        FIELD  AF_EN                                 :  1;      /* 16..16, 0x00010000 */
        FIELD  FLK_EN                                :  1;      /* 17..17, 0x00020000 */
        FIELD  HBIN2_EN                              :  1;      /* 18..18, 0x00040000 */
        FIELD  AA_EN                                 :  1;      /* 19..19, 0x00080000 */
        FIELD  SGG2_EN                               :  1;      /* 20..20, 0x00100000 */
        FIELD  EIS_EN                                :  1;      /* 21..21, 0x00200000 */
        FIELD  RMG_EN                                :  1;      /* 22..22, 0x00400000 */
        FIELD  CPG_EN                                :  1;      /* 23..23, 0x00800000 */
        FIELD  DBS_EN                                :  1;      /* 24..24, 0x01000000 */
        FIELD  RLB_EN                                :  1;      /* 25..25, 0x02000000 */
        FIELD  HBIN1_EN                              :  1;      /* 26..26, 0x04000000 */
        FIELD  SGG3_EN                               :  1;      /* 27..27, 0x08000000 */
        FIELD  SGG5_EN                               :  1;      /* 28..28, 0x10000000 */
        FIELD  rsv_29                                :  3;      /* 29..31, 0xE0000000 */

} ISP_CAM_CTL_EN_P1_T;

typedef union {
    enum { MASK     = 0x007FFFFF };
    typedef ISP_CAM_CTL_EN_P1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_CTL_EN_P1_T;

//
typedef union {
    enum { COUNT = 1 };
    MUINT32 set[COUNT];
    struct {
        ISP_CTL_EN_P1_T  en_p1;
    };
} ISP_NVRAM_CTL_EN_P1_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ISP Enable (Pass1@TG2)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CAM_CTL_EN_P1_D CAM+0010H: ISP Enable (Pass1@TG2)
typedef struct {
        FIELD  rsv_0                                 :  1;      /*  0.. 0, 0x00000001 */
        FIELD  DMX_D_EN                              :  1;      /*  1.. 1, 0x00000002 */
        FIELD  rsv_2                                 :  1;      /*  2.. 2, 0x00000004 */
        FIELD  rsv_3                                 :  1;      /*  3.. 3, 0x00000008 */
        FIELD  OB_D_EN                               :  1;      /*  4.. 4, 0x00000010 */
        FIELD  BNR_D_EN                              :  1;      /*  5.. 5, 0x00000020 */
        FIELD  LSC_D_EN                              :  1;      /*  6.. 6, 0x00000040 */
        FIELD  RPG_D_EN                              :  1;      /*  7.. 7, 0x00000080 */
        FIELD  RRZ_D_EN                              :  1;      /*  8.. 8, 0x00000100 */
        FIELD  RMX_D_EN                              :  1;      /*  9.. 9, 0x00000200 */
        FIELD  rsv_10                                :  1;      /* 10..10, 0x00000400 */
        FIELD  BMX_D_EN                              :  1;      /* 11..11, 0x00000800 */
        FIELD  rsv_12                                :  1;      /* 12..12, 0x00001000 */
        FIELD  rsv_13                                :  1;      /* 13..13, 0x00002000 */
        FIELD  rsv_14                                :  1;      /* 14..14, 0x00004000 */
        FIELD  rsv_15                                :  1;      /* 15..15, 0x00008000 */
        FIELD  rsv_16                                :  1;      /* 16..16, 0x00010000 */
        FIELD  rsv_17                                :  2;      /* 17..18, 0x00060000 */
        FIELD  rsv_19                                :  1;      /* 19..19, 0x00080000 */
        FIELD  rsv_20                                :  2;      /* 20..21, 0x00300000 */
        FIELD  RMG_D_EN                              :  1;      /* 22..22, 0x00400000 */
        FIELD  rsv_23                                :  1;      /* 23..23, 0x00800000 */
        FIELD  DBS_D_EN                              :  1;      /* 24..24, 0x01000000 */
        FIELD  rsv_25                                :  7;      /* 25..31, 0xFE000000 */
} ISP_CAM_CTL_EN_P1_D_T;

typedef union {
    enum { MASK     = 0x0049DFFF };
    typedef ISP_CAM_CTL_EN_P1_D_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_CTL_EN_P1_D_T;

//
typedef union {
    enum { COUNT = 1 };
    MUINT32 set[COUNT];
    struct {
        ISP_CTL_EN_P1_D_T  en_p1_d;
    };
} ISP_NVRAM_CTL_EN_P1_D_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ISP Enable (Pass2)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CAM_CTL_EN_P2 CAM+0018H: ISP Enable (Pass2)
typedef struct {
        FIELD  rsv_0                                 :  1;      /*  0.. 0, 0x00000001 */
        FIELD  PGN_EN                                :  1;      /*  1.. 1, 0x00000002 */
        FIELD  SL2_EN                                :  1;      /*  2.. 2, 0x00000004 */
        FIELD  CFA_EN                                :  1;      /*  3.. 3, 0x00000008 */
        FIELD  CCL_EN                                :  1;      /*  4.. 4, 0x00000010 */
        FIELD  G2G_EN                                :  1;      /*  5.. 5, 0x00000020 */
        FIELD  GGM_EN                                :  1;      /*  6.. 6, 0x00000040 */
        FIELD  rsv_7                                 :  1;      /*  7.. 7, 0x00000080 */
        FIELD  C24_EN                                :  1;      /*  8.. 8, 0x00000100 */
        FIELD  G2C_EN                                :  1;      /*  9.. 9, 0x00000200 */
        FIELD  C42_EN                                :  1;      /* 10..10, 0x00000400 */
        FIELD  NBC_EN                                :  1;      /* 11..11, 0x00000800 */
        FIELD  PCA_EN                                :  1;      /* 12..12, 0x00001000 */
        FIELD  SEEE_EN                               :  1;      /* 13..13, 0x00002000 */
        FIELD  rsv_14                                :  1;      /* 14..14, 0x00004000 */
        FIELD  rsv_15                                :  1;      /* 15..15, 0x00008000 */
        FIELD  rsv_16                                :  1;      /* 16..16, 0x00010000 */
        FIELD  SL2C_EN                               :  1;      /* 17..17, 0x00020000 */
        FIELD  rsv_18                                :  1;      /* 18..18, 0x00040000 */
        FIELD  rsv_19                                :  1;      /* 19..19, 0x00080000 */
        FIELD  CRZ_EN                                :  1;      /* 20..20, 0x00100000 */
        FIELD  rsv_21                                :  1;      /* 21..21, 0x00200000 */
        FIELD  rsv_22                                :  1;      /* 22..22, 0x00400000 */
        FIELD  rsv_23                                :  1;      /* 23..23, 0x00800000 */
        FIELD  rsv_24                                :  2;      /* 24..25, 0x03000000 */
        FIELD  rsv_25                                :  1;      /* 26..26, 0x04000000 */
        FIELD  C24B_EN                               :  1;      /* 27..27, 0x08000000 */
        FIELD  MDPCROP_EN                            :  1;      /* 28..28, 0x10000000 */
        FIELD  rsv_29                                :  1;      /* 29..29, 0x20000000 */
        FIELD  UNP_EN                                :  1;      /* 30..30, 0x40000000 */
        FIELD  NSL2A_EN                              :  1;      /* 31..31, 0x80000000 */
} ISP_CAM_CTL_EN_P2_T;

typedef union {
    enum { MASK     = 0xB883C01F };
    typedef ISP_CAM_CTL_EN_P2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_CTL_EN_P2_T;

//
typedef union {
    enum { COUNT = 1 };
    MUINT32 set[COUNT];
    struct {
        ISP_CTL_EN_P2_T  en_p2;
    };
} ISP_NVRAM_CTL_EN_P2_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// OBC @ TG1
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
    MUINT32 set[COUNT];
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
} ISP_NVRAM_OBC_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// OBC @ TG2
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CAM_OBC_D_OFFST0 CAM+2500H: OB for B channel
typedef struct {
        FIELD OBC_D_OFST_B              : 13;
        FIELD rsv_13                    : 19;
} ISP_CAM_OBC_D_OFFST0_T;

typedef union {
    enum { MASK     = 0x00001FFF };
    typedef ISP_CAM_OBC_D_OFFST0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_OBC_D_OFFST0_T;

// CAM_OBC_D_OFFST1 CAM+2504H: OB for Gr channel
typedef struct {
        FIELD OBC_D_OFST_GR             : 13;
        FIELD rsv_13                    : 19;
} ISP_CAM_OBC_D_OFFST1_T;

typedef union {
    enum { MASK     = 0x00001FFF };
    typedef ISP_CAM_OBC_D_OFFST1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_OBC_D_OFFST1_T;

// CAM_OBC_D_OFFST2 CAM+2508H: OB for Gb channel
typedef struct {
        FIELD OBC_D_OFST_GB             : 13;
        FIELD rsv_13                    : 19;
} ISP_CAM_OBC_D_OFFST2_T;

typedef union {
    enum { MASK     = 0x00001FFF };
    typedef ISP_CAM_OBC_D_OFFST2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_OBC_D_OFFST2_T;

// CAM_OBC_D_OFFST3 CAM+250CH: OB for R channel
typedef struct {
        FIELD OBC_D_OFST_R              : 13;
        FIELD rsv_13                    : 19;
} ISP_CAM_OBC_D_OFFST3_T;

typedef union {
    enum { MASK     = 0x00001FFF };
    typedef ISP_CAM_OBC_D_OFFST3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_OBC_D_OFFST3_T;

// CAM_OBC_D_GAIN0 CAM+2510H: OB gain for B channel
typedef struct {
        FIELD OBC_D_GAIN_B              : 13;
        FIELD rsv_13                    : 19;
} ISP_CAM_OBC_D_GAIN0_T;

typedef union {
    enum { MASK     = 0x00001FFF };
    typedef ISP_CAM_OBC_D_GAIN0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_OBC_D_GAIN0_T;

// CAM_OBC_D_GAIN1 CAM+2514H: OB gain for Gr channel
typedef struct {
        FIELD OBC_D_GAIN_GR             : 13;
        FIELD rsv_13                    : 19;
} ISP_CAM_OBC_D_GAIN1_T;

typedef union {
    enum { MASK     = 0x00001FFF };
    typedef ISP_CAM_OBC_D_GAIN1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_OBC_D_GAIN1_T;

// CAM_OBC_D_GAIN2 CAM+2518H: OB gain for Gb channel
typedef struct {
        FIELD OBC_D_GAIN_GB             : 13;
        FIELD rsv_13                    : 19;
} ISP_CAM_OBC_D_GAIN2_T;

typedef union {
    enum { MASK     = 0x00001FFF };
    typedef ISP_CAM_OBC_D_GAIN2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_OBC_D_GAIN2_T;

// CAM_OBC_D_GAIN3 CAM+251CH: OB gain for R channel
typedef struct {
        FIELD OBC_D_GAIN_R              : 13;
        FIELD rsv_13                    : 19;
} ISP_CAM_OBC_D_GAIN3_T;

typedef union {
    enum { MASK     = 0x00001FFF };
    typedef ISP_CAM_OBC_D_GAIN3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_OBC_D_GAIN3_T;

//
typedef union {
    enum { COUNT = 8 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_OBC_D_OFFST0_T  offst0; // Q.1.12
        ISP_NVRAM_OBC_D_OFFST1_T  offst1; // Q.1.12
        ISP_NVRAM_OBC_D_OFFST2_T  offst2; // Q.1.12
        ISP_NVRAM_OBC_D_OFFST3_T  offst3; // Q.1.12
        ISP_NVRAM_OBC_D_GAIN0_T   gain0; // Q.0.4.9
        ISP_NVRAM_OBC_D_GAIN1_T   gain1; // Q.0.4.9
        ISP_NVRAM_OBC_D_GAIN2_T   gain2; // Q.0.4.9
        ISP_NVRAM_OBC_D_GAIN3_T   gain3; // Q.0.4.9
    };
} ISP_NVRAM_OBC_D_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BNR_BPC
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CAM_BNR_BPC_CON CAM+0800H
typedef struct {
        FIELD BPC_EN                    : 1;
        FIELD rsv_1                     : 3;
        FIELD BPC_LUT_EN                : 1;
        FIELD BPC_TABLE_END_MODE        : 1;
        FIELD rsv_6                     : 2;
        FIELD BPC_AVG_MODE              : 1;
        FIELD rsv_9                     : 3;
        FIELD BPC_DTC_MODE              : 2;
        FIELD BPC_CS_MODE               : 2;
        FIELD BPC_CRC_MODE              : 2;
        FIELD BPC_EXC                   : 1;
        FIELD BPC_BLD_MODE              : 1;
        FIELD BNR_LE_INV_CTL            : 4;
        FIELD BNR_OSC_COUNT             : 4;
        FIELD BNR_EDGE                  : 4;
} ISP_CAM_BNR_BPC_CON_T;

typedef union {
    typedef ISP_CAM_BNR_BPC_CON_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_BPC_CON_T;

// CAM_BNR_BPC_TH1 CAM+0804H
typedef struct {
        FIELD BPC_TH_LWB                : 12;
        FIELD BPC_TH_Y                  : 12;
        FIELD BPC_BLD_SLP0              : 8;
} ISP_CAM_BNR_BPC_TH1_T;

typedef union {
    typedef ISP_CAM_BNR_BPC_TH1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_BPC_TH1_T;

// CAM_BNR_BPC_TH2 CAM+0808H
typedef struct {
        FIELD BPC_TH_UPB                : 12;
        FIELD rsv_12                    :  4;
        FIELD BPC_BLD0                  :  7;
        FIELD rsv_23                    :  1;
        FIELD BPC_BLD1                  :  7;
        FIELD rsv_31                    :  1;
} ISP_CAM_BNR_BPC_TH2_T;

typedef union {
    typedef ISP_CAM_BNR_BPC_TH2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_BPC_TH2_T;

// CAM_BNR_BPC_TH3 CAM+080CH
typedef struct {
        FIELD BPC_TH_XA                 : 12;
        FIELD BPC_TH_XB                 : 12;
        FIELD BPC_TH_SLA                : 4;
        FIELD BPC_TH_SLB                : 4;
} ISP_CAM_BNR_BPC_TH3_T;

typedef union {
    typedef ISP_CAM_BNR_BPC_TH3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_BPC_TH3_T;

// CAM_BNR_BPC_TH4 CAM+0810H
typedef struct {
        FIELD BPC_DK_TH_XA              : 12;
        FIELD BPC_DK_TH_XB              : 12;
        FIELD BPC_DK_TH_SLA             : 4;
        FIELD BPC_DK_TH_SLB             : 4;
} ISP_CAM_BNR_BPC_TH4_T;

typedef union {
    typedef ISP_CAM_BNR_BPC_TH4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_BPC_TH4_T;

// CAM_BNR_BPC_DTC CAM+0814H
typedef struct {
        FIELD BPC_RNG                   : 4;
        FIELD BPC_CS_RNG                : 3;
        FIELD rsv_7                     : 1;
        FIELD BPC_CT_LV                 : 4;
        FIELD rsv_12                    : 4;
        FIELD BPC_TH_MUL                : 4;
        FIELD rsv_20                    : 4;
        FIELD BPC_NO_LV                 : 3;
        FIELD rsv_27                    : 5;
} ISP_CAM_BNR_BPC_DTC_T;

typedef union {
    typedef ISP_CAM_BNR_BPC_DTC_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_BPC_DTC_T;

// CAM_BNR_BPC_COR CAM+0818H
typedef struct {
        FIELD BPC_DIR_MAX               : 8;
        FIELD BPC_DIR_TH                : 8;
        FIELD BPC_RANK_IDXR             : 3;
        FIELD BPC_RANK_IDXG             : 3;
        FIELD rsv_22                    : 2;
        FIELD BPC_DIR_TH2               : 8;
} ISP_CAM_BNR_BPC_COR_T;

typedef union {
    typedef ISP_CAM_BNR_BPC_COR_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_BPC_COR_T;

// CAM_BNR_BPC_TBLI1 CAM+081CH
typedef struct {
        FIELD BPC_XOFFSET               : 14;
        FIELD rsv_14                    : 2;
        FIELD BPC_YOFFSET               : 14;
        FIELD rsv_30                    : 2;
} ISP_CAM_BNR_BPC_TBLI1_T;

typedef union {
    typedef ISP_CAM_BNR_BPC_TBLI1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_BPC_TBLI1_T;

// CAM_BNR_BPC_TBLI2 CAM+0820H
typedef struct {
        FIELD BPC_XSIZE                 : 14;
        FIELD rsv_14                    : 2;
        FIELD BPC_YSIZE                 : 14;
        FIELD rsv_30                    : 2;
} ISP_CAM_BNR_BPC_TBLI2_T;

typedef union {
    typedef ISP_CAM_BNR_BPC_TBLI2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_BPC_TBLI2_T;

// CAM_BNR_BPC_TH1_C CAM+0824H
typedef struct {
        FIELD BPC_C_TH_LWB              : 12;
        FIELD BPC_C_TH_Y                : 12;
        FIELD rsv_24                    : 8;
} ISP_CAM_BNR_BPC_TH1_C_T;

typedef union {
    typedef ISP_CAM_BNR_BPC_TH1_C_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_BPC_TH1_C_T;

// CAM_BNR_BPC_TH2_C CAM+0828H
typedef struct {
        FIELD BPC_C_TH_UPB              : 12;
        FIELD rsv_12                    :  1;
        FIELD BPC_RANK_IDXB             :  3;
        FIELD BPC_BLD_LWB               : 12;
        FIELD rsv_28                    :  4;

} ISP_CAM_BNR_BPC_TH2_C_T;

typedef union {
    typedef ISP_CAM_BNR_BPC_TH2_C_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_BPC_TH2_C_T;

// CAM_BNR_BPC_TH3_C CAM+082CH
typedef struct {
        FIELD BPC_C_TH_XA               : 12;
        FIELD BPC_C_TH_XB               : 12;
        FIELD BPC_C_TH_SLA              : 4;
        FIELD BPC_C_TH_SLB              : 4;
} ISP_CAM_BNR_BPC_TH3_C_T;

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
        //ISP_NVRAM_BNR_BPC_TBLI2_T   tbli2;
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
        MUINT32 rsv_0                     : 4;
        MUINT32 NR1_CT_EN                 : 1;
        MUINT32 rsv_5                     : 27;
} ISP_CAM_BNR_NR1_CON_T;

typedef union {
    enum { MASK     = 0x00000010 };
    typedef ISP_CAM_BNR_NR1_CON_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_NR1_CON_T;

// CAM_BNR_NR1_CT_CON CAM+0854H
typedef struct {
        MUINT32 NR1_CT_MD                 : 2;
        MUINT32 NR1_CT_MD2                : 2;
        MUINT32 NR1_CT_THRD               : 10;
        MUINT32 rsv_14                    : 2;
        MUINT32 NR1_MBND                  : 10;
        MUINT32 rsv_26                    : 2;
        MUINT32 NR1_CT_SLOPE              : 2;
        MUINT32 NR1_CT_DIV                : 2;
} ISP_CAM_BNR_NR1_CT_CON_T;

typedef union {
    enum { MASK     = 0xF3FF3FFF };
    typedef ISP_CAM_BNR_NR1_CT_CON_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_NR1_CT_CON_T;

//
typedef union {
    enum { COUNT = 2 };
    struct {
        ISP_NVRAM_BNR_NR1_CON_T    con;
        ISP_NVRAM_BNR_NR1_CT_CON_T ct_con;
    };
    MUINT32 set[COUNT];
} ISP_NVRAM_BNR_NR1_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BNR_PDC
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CAM_BNR_PDC_CON
typedef struct {
        FIELD PDC_EN                    :  1;
        FIELD rsv_1                     :  3;
        FIELD PDC_CT                    :  1;
        FIELD rsv_5                     :  3;
        FIELD PDC_MODE                  :  2;
        FIELD rsv_10                    :  6;
        FIELD PDC_OUT                   :  1;
        FIELD rsv_17                    : 15;
} ISP_CAM_BNR_PDC_CON_T;

typedef union {
    enum { MASK     = 0x00010311 };
    typedef ISP_CAM_BNR_PDC_CON_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_PDC_CON_T;

//CAM_BNR_PDC_GAIN_L0
typedef struct {
        FIELD PDC_GCF_L00               : 12;
        FIELD rsv_12                    :  4;
        FIELD PDC_GCF_L10               : 12;
        FIELD rsv_28                    :  4;
} ISP_CAM_BNR_PDC_GAIN_L0_T;

typedef union {
    enum { MASK     = 0x0FFF0FFF };
    typedef ISP_CAM_BNR_PDC_GAIN_L0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_PDC_GAIN_L0_T;

//CAM_BNR_PDC_GAIN_L1
typedef struct {
        FIELD PDC_GCF_L01               : 12;
        FIELD rsv_12                    :  4;
        FIELD PDC_GCF_L20               : 12;
        FIELD rsv_28                    :  4;
} ISP_CAM_BNR_PDC_GAIN_L1_T;

typedef union {
    enum { MASK     = 0x0FFF0FFF };
    typedef ISP_CAM_BNR_PDC_GAIN_L1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_PDC_GAIN_L1_T;


//CAM_BNR_PDC_GAIN_L2
typedef struct {
        FIELD PDC_GCF_L11               : 12;
        FIELD rsv_12                    :  4;
        FIELD PDC_GCF_L02               : 12;
        FIELD rsv_28                    :  4;
} ISP_CAM_BNR_PDC_GAIN_L2_T;

typedef union {
    enum { MASK     = 0x0FFF0FFF};
    typedef ISP_CAM_BNR_PDC_GAIN_L2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_PDC_GAIN_L2_T;


//CAM_BNR_PDC_GAIN_L3
typedef struct {
        FIELD PDC_GCF_L30               : 12;
        FIELD rsv_12                    :  4;
        FIELD PDC_GCF_L21               : 12;
        FIELD rsv_28                    :  4;
} ISP_CAM_BNR_PDC_GAIN_L3_T;

typedef union {
    enum { MASK     = 0x0FFF0FFF };
    typedef ISP_CAM_BNR_PDC_GAIN_L3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_PDC_GAIN_L3_T;


//CAM_BNR_PDC_GAIN_L4
typedef struct {
        FIELD PDC_GCF_L12               : 12;
        FIELD rsv_12                    :  4;
        FIELD PDC_GCF_L03               : 12;
        FIELD rsv_28                    :  4;
} ISP_CAM_BNR_PDC_GAIN_L4_T;

typedef union {
    enum { MASK     = 0x0FFF0FFF };
    typedef ISP_CAM_BNR_PDC_GAIN_L4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_PDC_GAIN_L4_T;


//CAM_BNR_PDC_GAIN_R0
typedef struct {
        FIELD PDC_GCF_R00               : 12;
        FIELD rsv_12                    :  4;
        FIELD PDC_GCF_R10               : 12;
        FIELD rsv_28                    :  4;
} ISP_CAM_BNR_PDC_GAIN_R0_T;

typedef union {
    enum { MASK     = 0x0FFF0FFF };
    typedef ISP_CAM_BNR_PDC_GAIN_R0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_PDC_GAIN_R0_T;


//CAM_BNR_PDC_GAIN_R1
typedef struct {
        FIELD PDC_GCF_R01               : 12;
        FIELD rsv_12                    :  4;
        FIELD PDC_GCF_R20               : 12;
        FIELD rsv_28                    :  4;
} ISP_CAM_BNR_PDC_GAIN_R1_T;

typedef union {
    enum { MASK     = 0x0FFF0FFF };
    typedef ISP_CAM_BNR_PDC_GAIN_R1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_PDC_GAIN_R1_T;


//CAM_BNR_PDC_GAIN_R2
typedef struct {
        FIELD PDC_GCF_R11               : 12;
        FIELD rsv_12                    :  4;
        FIELD PDC_GCF_R02               : 12;
        FIELD rsv_28                    :  4;
} ISP_CAM_BNR_PDC_GAIN_R2_T;

typedef union {
    enum { MASK     = 0x0FFF0FFF };
    typedef ISP_CAM_BNR_PDC_GAIN_R2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_PDC_GAIN_R2_T;


//CAM_BNR_PDC_GAIN_R3
typedef struct {
        FIELD PDC_GCF_R30               : 12;
        FIELD rsv_12                    :  4;
        FIELD PDC_GCF_R21               : 12;
        FIELD rsv_28                    :  4;
} ISP_CAM_BNR_PDC_GAIN_R3_T;

typedef union {
    enum { MASK     = 0x0FFF0FFF };
    typedef ISP_CAM_BNR_PDC_GAIN_R3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_PDC_GAIN_R3_T;

//CAM_BNR_PDC_GAIN_R4
typedef struct {
        FIELD PDC_GCF_R12               : 12;
        FIELD rsv_12                    :  4;
        FIELD PDC_GCF_R03               : 12;
        FIELD rsv_28                    :  4;
} ISP_CAM_BNR_PDC_GAIN_R4_T;

typedef union {
    enum { MASK     = 0x0FFF0FFF };
    typedef ISP_CAM_BNR_PDC_GAIN_R4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_PDC_GAIN_R4_T;

//CAM_BNR_PDC_TH_GB;
typedef struct {
        FIELD PDC_GTH                   : 12;
        FIELD rsv_12                    :  4;
        FIELD PDC_BTH                   : 12;
        FIELD rsv_28                    :  4;
} ISP_CAM_BNR_PDC_TH_GB_T;

typedef union {
    enum { MASK     = 0x0FFF0FFF };
    typedef ISP_CAM_BNR_PDC_TH_GB_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_PDC_TH_GB_T;

//CAM_BNR_PDC_TH_IA;
typedef struct {
        FIELD PDC_ITH                   : 12;
        FIELD rsv_12                    :  4;
        FIELD PDC_ATH                   : 12;
        FIELD rsv_28                    :  4;
} ISP_CAM_BNR_PDC_TH_IA_T;

typedef union {
    enum { MASK     = 0x0FFF0FFF };
    typedef ISP_CAM_BNR_PDC_TH_IA_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_PDC_TH_IA_T;

//CAM_BNR_PDC_TH_HD;
typedef struct {
        FIELD PDC_NTH                   : 12;
        FIELD rsv_12                    :  4;
        FIELD PDC_DTH                   : 12;
        FIELD rsv_28                    :  4;
} ISP_CAM_BNR_PDC_TH_HD_T;

typedef union {
    enum { MASK     = 0x0FFF0FFF };
    typedef ISP_CAM_BNR_PDC_TH_HD_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_PDC_TH_HD_T;

//CAM_BNR_PDC_SL;
typedef struct {
        FIELD PDC_GSL                   :  4;
        FIELD PDC_BSL                   :  4;
        FIELD PDC_ISL                   :  4;
        FIELD PDC_ASL                   :  4;
        FIELD PDC_GCF_NORM              :  4;
        FIELD rsv_20                    : 12;
} ISP_CAM_BNR_PDC_SL_T;

typedef union {
    enum { MASK     = 0x000FFFFF };
    typedef ISP_CAM_BNR_PDC_SL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_PDC_SL_T;

//CAM_BNR_PDC_POS;
typedef struct {
        FIELD PDC_XCENTER               : 14;
        FIELD rsv_14                    :  2;
        FIELD PDC_YCENTER               : 14;
        FIELD rsv_30                    :  2;
} ISP_CAM_BNR_PDC_POS_T;

typedef union {
    enum { MASK     = 0x3FFF3FFF };
    typedef ISP_CAM_BNR_PDC_POS_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_PDC_POS_T;


//
typedef union {
    enum { COUNT = 16 };
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
        ISP_NVRAM_BNR_PDC_POS_T      pos;
    };
    MUINT32 set[COUNT];
} ISP_NVRAM_BNR_PDC_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BNR_D_BPC
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CAM_BNR_D_BPC_CON CAM+0800H
typedef struct {
        FIELD BPC_EN                    : 1;
        FIELD rsv_1                     : 3;
        FIELD BPC_LUT_EN                : 1;
        FIELD BPC_TABLE_END_MODE        : 1;
        FIELD rsv_6                     : 2;
        FIELD BPC_AVG_MODE              : 1;
        FIELD rsv_9                     : 3;
        FIELD BPC_DTC_MODE              : 2;
        FIELD BPC_CS_MODE               : 2;
        FIELD BPC_CRC_MODE              : 2;
        FIELD BPC_EXC                   : 1;
        FIELD BPC_BLD_MODE              : 1;
        FIELD BNR_LE_INV_CTL            : 4;
        FIELD BNR_OSC_COUNT             : 4;
        FIELD BNR_EDGE                  : 4;
} ISP_CAM_BNR_D_BPC_CON_T;

typedef union {
    typedef ISP_CAM_BNR_D_BPC_CON_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_D_BPC_CON_T;

// CAM_BNR_D_BPC_TH1 CAM+0804H
typedef struct {
        FIELD BPC_TH_LWB                : 12;
        FIELD BPC_TH_Y                  : 12;
        FIELD BPC_BLD_SLP0              : 8;
} ISP_CAM_BNR_D_BPC_TH1_T;

typedef union {
    typedef ISP_CAM_BNR_D_BPC_TH1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_D_BPC_TH1_T;

// CAM_BNR_D_BPC_TH2 CAM+0808H
typedef struct {
        FIELD BPC_TH_UPB                : 12;
        FIELD rsv_12                    :  4;
        FIELD BPC_BLD0                  :  7;
        FIELD rsv_23                    :  1;
        FIELD BPC_BLD1                  :  7;
        FIELD rsv_31                    :  1;
} ISP_CAM_BNR_D_BPC_TH2_T;

typedef union {
    typedef ISP_CAM_BNR_D_BPC_TH2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_D_BPC_TH2_T;

// CAM_BNR_D_BPC_TH3 CAM+080CH
typedef struct {
        FIELD BPC_TH_XA                 : 12;
        FIELD BPC_TH_XB                 : 12;
        FIELD BPC_TH_SLA                : 4;
        FIELD BPC_TH_SLB                : 4;
} ISP_CAM_BNR_D_BPC_TH3_T;

typedef union {
    typedef ISP_CAM_BNR_D_BPC_TH3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_D_BPC_TH3_T;

// CAM_BNR_D_BPC_TH4 CAM+0810H
typedef struct {
        FIELD BPC_DK_TH_XA              : 12;
        FIELD BPC_DK_TH_XB              : 12;
        FIELD BPC_DK_TH_SLA             : 4;
        FIELD BPC_DK_TH_SLB             : 4;
} ISP_CAM_BNR_D_BPC_TH4_T;

typedef union {
    typedef ISP_CAM_BNR_D_BPC_TH4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_D_BPC_TH4_T;

// CAM_BNR_D_BPC_DTC CAM+0814H
typedef struct {
        FIELD BPC_RNG                   : 4;
        FIELD BPC_CS_RNG                : 3;
        FIELD rsv_7                     : 1;
        FIELD BPC_CT_LV                 : 4;
        FIELD rsv_12                    : 4;
        FIELD BPC_TH_MUL                : 4;
        FIELD rsv_20                    : 4;
        FIELD BPC_NO_LV                 : 3;
        FIELD rsv_27                    : 5;
} ISP_CAM_BNR_D_BPC_DTC_T;

typedef union {
    typedef ISP_CAM_BNR_D_BPC_DTC_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_D_BPC_DTC_T;

// CAM_BNR_D_BPC_COR CAM+0818H
typedef struct {
        FIELD BPC_DIR_MAX               : 8;
        FIELD BPC_DIR_TH                : 8;
        FIELD BPC_RANK_IDXR             : 3;
        FIELD BPC_RANK_IDXG             : 3;
        FIELD rsv_22                    : 2;
        FIELD BPC_DIR_TH2               : 8;
} ISP_CAM_BNR_D_BPC_COR_T;

typedef union {
    typedef ISP_CAM_BNR_D_BPC_COR_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_D_BPC_COR_T;

// CAM_BNR_D_BPC_TBLI1 CAM+081CH
typedef struct {
        FIELD BPC_XOFFSET               : 14;
        FIELD rsv_14                    : 2;
        FIELD BPC_YOFFSET               : 14;
        FIELD rsv_30                    : 2;
} ISP_CAM_BNR_D_BPC_TBLI1_T;

typedef union {
    typedef ISP_CAM_BNR_D_BPC_TBLI1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_D_BPC_TBLI1_T;

// CAM_BNR_D_BPC_TBLI2 CAM+0820H
typedef struct {
        FIELD BPC_XSIZE                 : 14;
        FIELD rsv_14                    : 2;
        FIELD BPC_YSIZE                 : 14;
        FIELD rsv_30                    : 2;
} ISP_CAM_BNR_D_BPC_TBLI2_T;

typedef union {
    typedef ISP_CAM_BNR_D_BPC_TBLI2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_D_BPC_TBLI2_T;

// CAM_BNR_D_BPC_TH1_C CAM+0824H
typedef struct {
        FIELD BPC_C_TH_LWB              : 12;
        FIELD BPC_C_TH_Y                : 12;
        FIELD rsv_24                    : 8;
} ISP_CAM_BNR_D_BPC_TH1_C_T;

typedef union {
    typedef ISP_CAM_BNR_D_BPC_TH1_C_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_D_BPC_TH1_C_T;

// CAM_BNR_D_BPC_TH2_C CAM+0828H
typedef struct {
        FIELD BPC_C_TH_UPB              : 12;
        FIELD rsv_12                    :  1;
        FIELD BPC_RANK_IDXB             :  3;
        FIELD BPC_BLD_LWB               : 12;
        FIELD rsv_28                    :  4;

} ISP_CAM_BNR_D_BPC_TH2_C_T;

typedef union {
    typedef ISP_CAM_BNR_D_BPC_TH2_C_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_D_BPC_TH2_C_T;

// CAM_BNR_D_BPC_TH3_C CAM+082CH
typedef struct {
        FIELD BPC_C_TH_XA               : 12;
        FIELD BPC_C_TH_XB               : 12;
        FIELD BPC_C_TH_SLA              : 4;
        FIELD BPC_C_TH_SLB              : 4;
} ISP_CAM_BNR_D_BPC_TH3_C_T;

typedef union {
    typedef ISP_CAM_BNR_D_BPC_TH3_C_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_D_BPC_TH3_C_T;

//
typedef union {
    enum { COUNT = 10 };
    struct {
        ISP_NVRAM_BNR_D_BPC_CON_T     con;
        ISP_NVRAM_BNR_D_BPC_TH1_T     th1;
        ISP_NVRAM_BNR_D_BPC_TH2_T     th2;
        ISP_NVRAM_BNR_D_BPC_TH3_T     th3;
        ISP_NVRAM_BNR_D_BPC_TH4_T     th4;
        ISP_NVRAM_BNR_D_BPC_DTC_T     dtc;
        ISP_NVRAM_BNR_D_BPC_COR_T     cor;
        //ISP_NVRAM_BNR_D_BPC_TBLI1_T   tbli1;
        //ISP_NVRAM_BNR_D_BPC_TBLI2_T   tbli2;
        ISP_NVRAM_BNR_D_BPC_TH1_C_T   th1_c;
        ISP_NVRAM_BNR_D_BPC_TH2_C_T   th2_c;
        ISP_NVRAM_BNR_D_BPC_TH3_C_T   th3_c;
    };
    MUINT32 set[COUNT];
} ISP_NVRAM_BNR_D_BPC_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BNR_D_NR1
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CAM_BNR_D_NR1_CON CAM+0850H
typedef struct {
        MUINT32 rsv_0                     : 4;
        MUINT32 NR1_CT_EN                 : 1;
        MUINT32 rsv_5                     : 27;
} ISP_CAM_BNR_D_NR1_CON_T;

typedef union {
    enum { MASK     = 0x00000010 };
    typedef ISP_CAM_BNR_D_NR1_CON_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_D_NR1_CON_T;

// CAM_BNR_D_NR1_CT_CON CAM+0854H
typedef struct {
        MUINT32 NR1_CT_MD                 : 2;
        MUINT32 NR1_CT_MD2                : 2;
        MUINT32 NR1_CT_THRD               : 10;
        MUINT32 rsv_14                    : 2;
        MUINT32 NR1_MBND                  : 10;
        MUINT32 rsv_26                    : 2;
        MUINT32 NR1_CT_SLOPE              : 2;
        MUINT32 NR1_CT_DIV                : 2;
} ISP_CAM_BNR_D_NR1_CT_CON_T;

typedef union {
    enum { MASK     = 0xF3FF3FFF };
    typedef ISP_CAM_BNR_D_NR1_CT_CON_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_D_NR1_CT_CON_T;

//
typedef union {
    enum { COUNT = 2 };
    struct {
        ISP_NVRAM_BNR_D_NR1_CON_T    con;
        ISP_NVRAM_BNR_D_NR1_CT_CON_T ct_con;
    };
    MUINT32 set[COUNT];
} ISP_NVRAM_BNR_D_NR1_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BNR_D_PDC
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CAM_BNR_D_PDC_CON
typedef struct {
        FIELD PDC_EN                    :  1;
        FIELD rsv_1                     :  3;
        FIELD PDC_CT                    :  1;
        FIELD rsv_5                     :  3;
        FIELD PDC_MODE                  :  2;
        FIELD rsv_10                    :  6;
        FIELD PDC_OUT                   :  1;
        FIELD rsv_17                    : 15;
} ISP_CAM_BNR_D_PDC_CON_T;

typedef union {
    enum { MASK     = 0x00010311 };
    typedef ISP_CAM_BNR_D_PDC_CON_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_D_PDC_CON_T;

//CAM_BNR_D_PDC_GAIN_L0
typedef struct {
        FIELD PDC_GCF_L00               : 12;
        FIELD rsv_12                    :  4;
        FIELD PDC_GCF_L10               : 12;
        FIELD rsv_28                    :  4;
} ISP_CAM_BNR_D_PDC_GAIN_L0_T;

typedef union {
    enum { MASK     = 0x0FFF0FFF };
    typedef ISP_CAM_BNR_D_PDC_GAIN_L0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_D_PDC_GAIN_L0_T;

//CAM_BNR_D_PDC_GAIN_L1
typedef struct {
        FIELD PDC_GCF_L01               : 12;
        FIELD rsv_12                    :  4;
        FIELD PDC_GCF_L20               : 12;
        FIELD rsv_28                    :  4;
} ISP_CAM_BNR_D_PDC_GAIN_L1_T;

typedef union {
    enum { MASK     = 0x0FFF0FFF };
    typedef ISP_CAM_BNR_D_PDC_GAIN_L1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_D_PDC_GAIN_L1_T;


//CAM_BNR_D_PDC_GAIN_L2
typedef struct {
        FIELD PDC_GCF_L11               : 12;
        FIELD rsv_12                    :  4;
        FIELD PDC_GCF_L02               : 12;
        FIELD rsv_28                    :  4;
} ISP_CAM_BNR_D_PDC_GAIN_L2_T;

typedef union {
    enum { MASK     = 0x0FFF0FFF};
    typedef ISP_CAM_BNR_D_PDC_GAIN_L2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_D_PDC_GAIN_L2_T;


//CAM_BNR_D_PDC_GAIN_L3
typedef struct {
        FIELD PDC_GCF_L30               : 12;
        FIELD rsv_12                    :  4;
        FIELD PDC_GCF_L21               : 12;
        FIELD rsv_28                    :  4;
} ISP_CAM_BNR_D_PDC_GAIN_L3_T;

typedef union {
    enum { MASK     = 0x0FFF0FFF };
    typedef ISP_CAM_BNR_D_PDC_GAIN_L3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_D_PDC_GAIN_L3_T;


//CAM_BNR_D_PDC_GAIN_L4
typedef struct {
        FIELD PDC_GCF_L12               : 12;
        FIELD rsv_12                    :  4;
        FIELD PDC_GCF_L03               : 12;
        FIELD rsv_28                    :  4;
} ISP_CAM_BNR_D_PDC_GAIN_L4_T;

typedef union {
    enum { MASK     = 0x0FFF0FFF };
    typedef ISP_CAM_BNR_D_PDC_GAIN_L4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_D_PDC_GAIN_L4_T;


//CAM_BNR_D_PDC_GAIN_R0
typedef struct {
        FIELD PDC_GCF_R00               : 12;
        FIELD rsv_12                    :  4;
        FIELD PDC_GCF_R10               : 12;
        FIELD rsv_28                    :  4;
} ISP_CAM_BNR_D_PDC_GAIN_R0_T;

typedef union {
    enum { MASK     = 0x0FFF0FFF };
    typedef ISP_CAM_BNR_D_PDC_GAIN_R0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_D_PDC_GAIN_R0_T;


//CAM_BNR_D_PDC_GAIN_R1
typedef struct {
        FIELD PDC_GCF_R01               : 12;
        FIELD rsv_12                    :  4;
        FIELD PDC_GCF_R20               : 12;
        FIELD rsv_28                    :  4;
} ISP_CAM_BNR_D_PDC_GAIN_R1_T;

typedef union {
    enum { MASK     = 0x0FFF0FFF };
    typedef ISP_CAM_BNR_D_PDC_GAIN_R1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_D_PDC_GAIN_R1_T;


//CAM_BNR_D_PDC_GAIN_R2
typedef struct {
        FIELD PDC_GCF_R11               : 12;
        FIELD rsv_12                    :  4;
        FIELD PDC_GCF_R02               : 12;
        FIELD rsv_28                    :  4;
} ISP_CAM_BNR_D_PDC_GAIN_R2_T;

typedef union {
    enum { MASK     = 0x0FFF0FFF };
    typedef ISP_CAM_BNR_D_PDC_GAIN_R2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_D_PDC_GAIN_R2_T;


//CAM_BNR_D_PDC_GAIN_R3
typedef struct {
        FIELD PDC_GCF_R30               : 12;
        FIELD rsv_12                    :  4;
        FIELD PDC_GCF_R21               : 12;
        FIELD rsv_28                    :  4;
} ISP_CAM_BNR_D_PDC_GAIN_R3_T;

typedef union {
    enum { MASK     = 0x0FFF0FFF };
    typedef ISP_CAM_BNR_D_PDC_GAIN_R3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_D_PDC_GAIN_R3_T;

//CAM_BNR_D_PDC_GAIN_R4
typedef struct {
        FIELD PDC_GCF_R12               : 12;
        FIELD rsv_12                    :  4;
        FIELD PDC_GCF_R03               : 12;
        FIELD rsv_28                    :  4;
} ISP_CAM_BNR_D_PDC_GAIN_R4_T;

typedef union {
    enum { MASK     = 0x0FFF0FFF };
    typedef ISP_CAM_BNR_D_PDC_GAIN_R4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_D_PDC_GAIN_R4_T;

//CAM_BNR_D_PDC_TH_GB;
typedef struct {
        FIELD PDC_GTH                   : 12;
        FIELD rsv_12                    :  4;
        FIELD PDC_BTH                   : 12;
        FIELD rsv_28                    :  4;
} ISP_CAM_BNR_D_PDC_TH_GB_T;

typedef union {
    enum { MASK     = 0x0FFF0FFF };
    typedef ISP_CAM_BNR_D_PDC_TH_GB_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_D_PDC_TH_GB_T;

//CAM_BNR_D_PDC_TH_IA;
typedef struct {
        FIELD PDC_ITH                   : 12;
        FIELD rsv_12                    :  4;
        FIELD PDC_ATH                   : 12;
        FIELD rsv_28                    :  4;
} ISP_CAM_BNR_D_PDC_TH_IA_T;

typedef union {
    enum { MASK     = 0x0FFF0FFF };
    typedef ISP_CAM_BNR_D_PDC_TH_IA_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_D_PDC_TH_IA_T;

//CAM_BNR_D_PDC_TH_HD;
typedef struct {
        FIELD PDC_NTH                   : 12;
        FIELD rsv_12                    :  4;
        FIELD PDC_DTH                   : 12;
        FIELD rsv_28                    :  4;
} ISP_CAM_BNR_D_PDC_TH_HD_T;

typedef union {
    enum { MASK     = 0x0FFF0FFF };
    typedef ISP_CAM_BNR_D_PDC_TH_HD_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_D_PDC_TH_HD_T;

//CAM_BNR_D_PDC_SL;
typedef struct {
        FIELD PDC_GSL                   :  4;
        FIELD PDC_BSL                   :  4;
        FIELD PDC_ISL                   :  4;
        FIELD PDC_ASL                   :  4;
        FIELD PDC_GCF_NORM              :  4;
        FIELD rsv_20                    : 12;
} ISP_CAM_BNR_D_PDC_SL_T;

typedef union {
    enum { MASK     = 0x000FFFFF };
    typedef ISP_CAM_BNR_D_PDC_SL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_D_PDC_SL_T;

//CAM_BNR_D_PDC_POS;
typedef struct {
        FIELD PDC_XCENTER               : 14;
        FIELD rsv_14                    :  2;
        FIELD PDC_YCENTER               : 14;
        FIELD rsv_30                    :  2;
} ISP_CAM_BNR_D_PDC_POS_T;

typedef union {
    enum { MASK     = 0x3FFF3FFF };
    typedef ISP_CAM_BNR_D_PDC_POS_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_BNR_D_PDC_POS_T;


//
typedef union {
    enum { COUNT = 16 };
    struct {
        ISP_NVRAM_BNR_D_PDC_CON_T      con;
        ISP_NVRAM_BNR_D_PDC_GAIN_L0_T  gain_l0;
        ISP_NVRAM_BNR_D_PDC_GAIN_L1_T  gain_l1;
        ISP_NVRAM_BNR_D_PDC_GAIN_L2_T  gain_l2;
        ISP_NVRAM_BNR_D_PDC_GAIN_L3_T  gain_l3;
        ISP_NVRAM_BNR_D_PDC_GAIN_L4_T  gain_l4;
        ISP_NVRAM_BNR_D_PDC_GAIN_R0_T  gain_r0;
        ISP_NVRAM_BNR_D_PDC_GAIN_R1_T  gain_r1;
        ISP_NVRAM_BNR_D_PDC_GAIN_R2_T  gain_r2;
        ISP_NVRAM_BNR_D_PDC_GAIN_R3_T  gain_r3;
        ISP_NVRAM_BNR_D_PDC_GAIN_R4_T  gain_r4;
        ISP_NVRAM_BNR_D_PDC_TH_GB_T    th_gb;
        ISP_NVRAM_BNR_D_PDC_TH_IA_T    th_ia;
        ISP_NVRAM_BNR_D_PDC_TH_HD_T    th_hd;
        ISP_NVRAM_BNR_D_PDC_SL_T       sl;
        ISP_NVRAM_BNR_D_PDC_POS_T      pos;
    };
    MUINT32 set[COUNT];
} ISP_NVRAM_BNR_D_PDC_T;


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
// DBS_D
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef struct {
    FIELD  DBS_OFST                              : 12;    /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                :  4;    /* 12..15, 0x0000F000 */
    FIELD  DBS_SL                                :  8;    /* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                :  8;    /* 24..31, 0xFF000000 */
} ISP_CAM_DBS_D_SIGMA_T; // /* 0x1A004C00 */

typedef union {
    typedef ISP_CAM_DBS_D_SIGMA_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DBS_D_SIGMA_T;

typedef struct {
    FIELD  DBS_BIAS_Y0                           :  8;    /*  0.. 7, 0x000000FF */
    FIELD  DBS_BIAS_Y1                           :  8;    /*  8..15, 0x0000FF00 */
    FIELD  DBS_BIAS_Y2                           :  8;    /* 16..23, 0x00FF0000 */
    FIELD  DBS_BIAS_Y3                           :  8;    /* 24..31, 0xFF000000 */
} ISP_CAM_DBS_D_BSTBL_0_T; // /* 0x1A004C04 */

typedef union {
    typedef ISP_CAM_DBS_D_BSTBL_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DBS_D_BSTBL_0_T;

typedef struct {
    FIELD  DBS_BIAS_Y4                           :  8;    /*  0.. 7, 0x000000FF */
    FIELD  DBS_BIAS_Y5                           :  8;    /*  8..15, 0x0000FF00 */
    FIELD  DBS_BIAS_Y6                           :  8;    /* 16..23, 0x00FF0000 */
    FIELD  DBS_BIAS_Y7                           :  8;    /* 24..31, 0xFF000000 */
} ISP_CAM_DBS_D_BSTBL_1_T; // /* 0x1A004C08 */

typedef union {
    typedef ISP_CAM_DBS_D_BSTBL_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DBS_D_BSTBL_1_T;

typedef struct {
    FIELD  DBS_BIAS_Y8                           :  8;    /*  0.. 7, 0x000000FF */
    FIELD  DBS_BIAS_Y9                           :  8;    /*  8..15, 0x0000FF00 */
    FIELD  DBS_BIAS_Y10                          :  8;    /* 16..23, 0x00FF0000 */
    FIELD  DBS_BIAS_Y11                          :  8;    /* 24..31, 0xFF000000 */
} ISP_CAM_DBS_D_BSTBL_2_T; // /* 0x1A004C0C */

typedef union {
    typedef ISP_CAM_DBS_D_BSTBL_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DBS_D_BSTBL_2_T;

typedef struct {
    FIELD  DBS_BIAS_Y12                          :  8;    /*  0.. 7, 0x000000FF */
    FIELD  DBS_BIAS_Y13                          :  8;    /*  8..15, 0x0000FF00 */
    FIELD  DBS_BIAS_Y14                          :  8;    /* 16..23, 0x00FF0000 */
    FIELD  rsv_24                                :  8;    /* 24..31, 0xFF000000 */
} ISP_CAM_DBS_D_BSTBL_3_T; // /* 0x1A004C10 */

typedef union {
    typedef ISP_CAM_DBS_D_BSTBL_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DBS_D_BSTBL_3_T;

typedef struct {
    FIELD  DBS_HDR_GNP                           :  3;    /*  0.. 2, 0x00000007 */
    FIELD  rsv_3                                 :  1;    /*  3.. 3, 0x00000008 */
    FIELD  DBS_SL_EN                             :  1;    /*  4.. 4, 0x00000010 */
    FIELD  rsv_5                                 :  3;    /*  5.. 7, 0x000000E0 */
    FIELD  DBS_LE_INV_CTL                        :  4;    /*  8..11, 0x00000F00 */
    FIELD  DBS_EDGE                              :  4;    /* 12..15, 0x0000F000 */
    FIELD  DBS_HDR_GAIN                          :  8;    /* 16..23, 0x00FF0000 */
    FIELD  DBS_HDR_GAIN2                         :  8;    /* 24..31, 0xFF000000 */
} ISP_CAM_DBS_D_CTL_T; // /* 0x1A004C14 */

typedef union {
    typedef ISP_CAM_DBS_D_CTL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DBS_D_CTL_T;

typedef struct {
    FIELD  DBS_HDR_OSCTH                         : 12;    /*  0..11, 0x00000FFF */
    FIELD  rsv_12                                : 20;    /* 12..31, 0xFFFFF000 */
} ISP_CAM_DBS_D_CTL_2_T; // /* 0x1A004C18 */

typedef union {
    typedef ISP_CAM_DBS_D_CTL_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DBS_D_CTL_2_T;

typedef struct {
    FIELD  DBS_MUL_B                             :  8;    /*  0.. 7, 0x000000FF */
    FIELD  DBS_MUL_GB                            :  8;    /*  8..15, 0x0000FF00 */
    FIELD  DBS_MUL_GR                            :  8;    /* 16..23, 0x00FF0000 */
    FIELD  DBS_MUL_R                             :  8;    /* 24..31, 0xFF000000 */
} ISP_CAM_DBS_D_SIGMA_2_T; // /* 0x1A004C1C */

typedef union {
    typedef ISP_CAM_DBS_D_SIGMA_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DBS_D_SIGMA_2_T;

typedef struct {
    FIELD  DBS_YGN_B                             :  6;    /*  0.. 5, 0x0000003F */
    FIELD  rsv_6                                 :  2;    /*  6.. 7, 0x000000C0 */
    FIELD  DBS_YGN_GB                            :  6;    /*  8..13, 0x00003F00 */
    FIELD  rsv_14                                :  2;    /* 14..15, 0x0000C000 */
    FIELD  DBS_YGN_GR                            :  6;    /* 16..21, 0x003F0000 */
    FIELD  rsv_22                                :  2;    /* 22..23, 0x00C00000 */
    FIELD  DBS_YGN_R                             :  6;    /* 24..29, 0x3F000000 */
    FIELD  rsv_30                                :  2;    /* 30..31, 0xC0000000 */
} ISP_CAM_DBS_D_YGN_T; // /* 0x1A004C20 */

typedef union {
    typedef ISP_CAM_DBS_D_YGN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DBS_D_YGN_T;

typedef struct {
    FIELD  DBS_SL_Y1                             : 10;    /*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                :  6;    /* 10..15, 0x0000FC00 */
    FIELD  DBS_SL_Y2                             : 10;    /* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                :  6;    /* 26..31, 0xFC000000 */
} ISP_CAM_DBS_D_SL_Y12_T; //  /* 0x1A004C24 */

typedef union {
    typedef ISP_CAM_DBS_D_SL_Y12_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DBS_D_SL_Y12_T;

typedef struct {
    FIELD  DBS_SL_Y3                             : 10;    /*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                :  6;    /* 10..15, 0x0000FC00 */
    FIELD  DBS_SL_Y4                             : 10;    /* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                :  6;    /* 26..31, 0xFC000000 */
} ISP_CAM_DBS_D_SL_Y34_T; //  /* 0x1A004C28 */

typedef union {
    typedef ISP_CAM_DBS_D_SL_Y34_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DBS_D_SL_Y34_T;

typedef struct {
    FIELD  DBS_SL_G1                             : 10;    /*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                :  6;    /* 10..15, 0x0000FC00 */
    FIELD  DBS_SL_G2                             : 10;    /* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                :  6;    /* 26..31, 0xFC000000 */
} ISP_CAM_DBS_D_SL_G12_T; //  /* 0x1A004C2C */

typedef union {
    typedef ISP_CAM_DBS_D_SL_G12_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DBS_D_SL_G12_T;

typedef struct {
    FIELD  DBS_SL_G3                             : 10;    /*  0.. 9, 0x000003FF */
    FIELD  rsv_10                                :  6;    /* 10..15, 0x0000FC00 */
    FIELD  DBS_SL_G4                             : 10;    /* 16..25, 0x03FF0000 */
    FIELD  rsv_26                                :  6;    /* 26..31, 0xFC000000 */
} ISP_CAM_DBS_D_SL_G34_T; //  /* 0x1A004C30 */

typedef union {
    typedef ISP_CAM_DBS_D_SL_G34_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DBS_D_SL_G34_T;

typedef union {
    enum { COUNT = 12 };
    struct {
        ISP_NVRAM_DBS_D_SIGMA_T        sigma;
        ISP_NVRAM_DBS_D_BSTBL_0_T      bstbl_0;
        ISP_NVRAM_DBS_D_BSTBL_1_T      bstbl_1;
        ISP_NVRAM_DBS_D_BSTBL_2_T      bstbl_2;
        ISP_NVRAM_DBS_D_BSTBL_3_T      bstbl_3;
        ISP_NVRAM_DBS_D_CTL_T          ctl;
        //ISP_NVRAM_DBS_D_CTL_2_T    ctl_2;
        ISP_NVRAM_DBS_D_SIGMA_2_T  sigma_2;
        ISP_NVRAM_DBS_D_YGN_T      ygn;
        ISP_NVRAM_DBS_D_SL_Y12_T   sl_y12;
        ISP_NVRAM_DBS_D_SL_Y34_T   sl_y34;
        ISP_NVRAM_DBS_D_SL_G12_T   sl_g12;
        ISP_NVRAM_DBS_D_SL_G34_T   sl_g34;
    };
    MUINT32 set[COUNT];
} ISP_NVRAM_DBS_D_T;

#if 0
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// NR1 @ TG1
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CAM_NR1_CON CAM+0850H
typedef struct {
        MUINT32 rsv_0                     : 4;
        MUINT32 NR1_CT_EN                 : 1;
        MUINT32 rsv_5                     : 27;
} ISP_CAM_NR1_CON_T;

typedef union {
    enum { MASK     = 0x00000010 };
    typedef ISP_CAM_NR1_CON_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR1_CON_T;

// CAM_NR1_CT_CON CAM+0854H
typedef struct {
        MUINT32 NR1_CT_MD                 : 2;
        MUINT32 NR1_CT_MD2                : 2;
        MUINT32 NR1_CT_THRD               : 10;
        MUINT32 rsv_14                    : 2;
        MUINT32 NR1_MBND                  : 10;
        MUINT32 rsv_26                    : 2;
        MUINT32 NR1_CT_SLOPE              : 2;
        MUINT32 NR1_CT_DIV                : 2;
} ISP_CAM_NR1_CT_CON_T;

typedef union {
    enum { MASK     = 0xF3FF3FFF };
    typedef ISP_CAM_NR1_CT_CON_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR1_CT_CON_T;

//
typedef union {
    enum { COUNT = 2 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_NR1_CON_T    con;
        ISP_NVRAM_NR1_CT_CON_T ct_con;
    };
} ISP_NVRAM_NR1_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// NR1 @ TG2
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CAM_NR1_D_CON CAM+2850H
typedef struct {
        MUINT32 rsv_0                     : 4;
        MUINT32 NR1_CT_EN                 : 1;
        MUINT32 rsv_5                     : 27;
} ISP_CAM_NR1_D_CON_T;

typedef union {
    enum { MASK     = 0x00000010 };
    typedef ISP_CAM_NR1_D_CON_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR1_D_CON_T;

// CAM_NR1_D_CT_CON CAM+2854H
typedef struct {
        MUINT32 NR1_CT_MD                 : 2;
        MUINT32 NR1_CT_MD2                : 2;
        MUINT32 NR1_CT_THRD               : 10;
        MUINT32 rsv_14                    : 2;
        MUINT32 NR1_MBND                  : 10;
        MUINT32 rsv_26                    : 2;
        MUINT32 NR1_CT_SLOPE              : 2;
        MUINT32 NR1_CT_DIV                : 2;
} ISP_CAM_NR1_D_CT_CON_T;

typedef union {
    enum { MASK     = 0xF3FF3FFF };
    typedef ISP_CAM_NR1_D_CT_CON_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR1_D_CT_CON_T;

//
typedef union {
    enum { COUNT = 2 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_NR1_D_CON_T    con;
        ISP_NVRAM_NR1_D_CT_CON_T ct_con;
    };
} ISP_NVRAM_NR1_D_T;
#endif
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// RPG @ TG1
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
// RPG @ TG2
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CAM_RPG_D_SATU_1 CAM+2550H
typedef struct {
        FIELD RPG_SATU_B                : 12;
        FIELD rsv_12                    : 4;
        FIELD RPG_SATU_GB               : 12;
        FIELD rsv_28                    : 4;
} ISP_CAM_RPG_D_SATU_1_T;

typedef union {
    enum { MASK     = 0x0FFF0FFF };
    enum { DEFAULT  = 0x0FFF0FFF };
    typedef ISP_CAM_RPG_D_SATU_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_RPG_D_SATU_1_T;

// CAM_RPG_D_SATU_2 CAM+2554H
typedef struct {
        FIELD RPG_SATU_GR               : 12;
        FIELD rsv_12                    : 4;
        FIELD RPG_SATU_R                : 12;
        FIELD rsv_28                    : 4;
} ISP_CAM_RPG_D_SATU_2_T;

typedef union {
    enum { MASK     = 0x0FFF0FFF };
    enum { DEFAULT  = 0x0FFF0FFF };
    typedef ISP_CAM_RPG_D_SATU_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_RPG_D_SATU_2_T;

// CAM_RPG_D_GAIN_1 CAM+2558H
typedef struct {
        FIELD RPG_GAIN_B                : 13;
        FIELD rsv_13                    : 3;
        FIELD RPG_GAIN_GB               : 13;
        FIELD rsv_29                    : 3;
} ISP_CAM_RPG_D_GAIN_1_T;

typedef union {
    enum { MASK     = 0x1FFF1FFF };
    enum { DEFAULT  = 0x02000200 };
    typedef ISP_CAM_RPG_D_GAIN_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_RPG_D_GAIN_1_T;

// CAM_RPG_D_GAIN_2 CAM+255CH
typedef struct {
        FIELD RPG_GAIN_GR               : 13;
        FIELD rsv_13                    : 3;
        FIELD RPG_GAIN_R                : 13;
        FIELD rsv_29                    : 3;
} ISP_CAM_RPG_D_GAIN_2_T;

typedef union {
    enum { MASK     = 0x1FFF1FFF };
    enum { DEFAULT  = 0x02000200 };
    typedef ISP_CAM_RPG_D_GAIN_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_RPG_D_GAIN_2_T;

// CAM_RPG_D_OFST_1 CAM+2560H
typedef struct {
        FIELD RPG_OFST_B                : 12;
        FIELD rsv_12                    : 4;
        FIELD RPG_OFST_GB               : 12;
        FIELD rsv_28                    : 4;
} ISP_CAM_RPG_D_OFST_1_T;

typedef union {
    enum { MASK     = 0x0FFF0FFF };
    enum { DEFAULT  = 0x00000000 };
    typedef ISP_CAM_RPG_D_OFST_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_RPG_D_OFST_1_T;

// CAM_RPG_D_OFST_2 CAM+2564H
typedef struct {
        FIELD RPG_OFST_GR               : 12;
        FIELD rsv_12                    : 4;
        FIELD RPG_OFST_R                : 12;
        FIELD rsv_28                    : 4;
} ISP_CAM_RPG_D_OFST_2_T;

typedef union {
    enum { MASK     = 0x0FFF0FFF };
    enum { DEFAULT  = 0x00000000 };
    typedef ISP_CAM_RPG_D_OFST_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_RPG_D_OFST_2_T;

//
typedef union {
    enum { COUNT = 6 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_RPG_D_SATU_1_T   satu_1;
        ISP_NVRAM_RPG_D_SATU_2_T   satu_2;
        ISP_NVRAM_RPG_D_GAIN_1_T   gain_1;
        ISP_NVRAM_RPG_D_GAIN_2_T   gain_2;
        ISP_NVRAM_RPG_D_OFST_1_T   ofst_1;
        ISP_NVRAM_RPG_D_OFST_2_T   ofst_2;
    };
} ISP_NVRAM_RPG_D_T;



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// PGN
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CAM_PGN_SATU_1 CAM+0880H
typedef struct {
        FIELD PGN_SATU_B                : 12;
        FIELD rsv_12                    : 4;
        FIELD PGN_SATU_GB               : 12;
        FIELD rsv_28                    : 4;
} ISP_CAM_PGN_SATU_1_T;

typedef union {
    enum { MASK     = 0x0FFF0FFF };
    enum { DEFAULT  = 0x0FFF0FFF };
    typedef ISP_CAM_PGN_SATU_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_PGN_SATU_1_T;

// CAM_PGN_SATU_2 CAM+0884H
typedef struct {
        FIELD PGN_SATU_GR               : 12;
        FIELD rsv_12                    : 4;
        FIELD PGN_SATU_R                : 12;
        FIELD rsv_28                    : 4;
} ISP_CAM_PGN_SATU_2_T;

typedef union {
    enum { MASK     = 0x0FFF0FFF };
    enum { DEFAULT  = 0x0FFF0FFF };
    typedef ISP_CAM_PGN_SATU_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_PGN_SATU_2_T;

// CAM_PGN_GAIN_1 CAM+0888H
typedef struct {
        FIELD PGN_GAIN_B                : 13;
        FIELD rsv_13                    : 3;
        FIELD PGN_GAIN_GB               : 13;
        FIELD rsv_29                    : 3;
} ISP_CAM_PGN_GAIN_1_T;

typedef union {
    enum { MASK     = 0x1FFF1FFF };
    enum { DEFAULT  = 0x02000200 };
    typedef ISP_CAM_PGN_GAIN_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_PGN_GAIN_1_T;

// CAM_PGN_GAIN_2 CAM+088CH
typedef struct {
        FIELD PGN_GAIN_GR               : 13;
        FIELD rsv_13                    : 3;
        FIELD PGN_GAIN_R                : 13;
        FIELD rsv_29                    : 3;
} ISP_CAM_PGN_GAIN_2_T;

typedef union {
    enum { MASK     = 0x1FFF1FFF };
    enum { DEFAULT  = 0x02000200 };
    typedef ISP_CAM_PGN_GAIN_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_PGN_GAIN_2_T;

// CAM_PGN_OFST_1 CAM+0890H
typedef struct {
        FIELD PGN_OFST_B                : 12;
        FIELD rsv_12                    : 4;
        FIELD PGN_OFST_GB               : 12;
        FIELD rsv_28                    : 4;
} ISP_CAM_PGN_OFST_1_T;

typedef union {
    enum { MASK     = 0x0FFF0FFF };
    enum { DEFAULT  = 0x00000000 };
    typedef ISP_CAM_PGN_OFST_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_PGN_OFST_1_T;

// CAM_PGN_OFST_2 CAM+0894H
typedef struct {
        FIELD PGN_OFST_GR               : 12;
        FIELD rsv_12                    : 4;
        FIELD PGN_OFST_R                : 12;
        FIELD rsv_28                    : 4;
} ISP_CAM_PGN_OFST_2_T;

typedef union {
    enum { MASK     = 0x0FFF0FFF };
    enum { DEFAULT  = 0x00000000 };
    typedef ISP_CAM_PGN_OFST_2_T reg_t;
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
// CFA
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CAM_DM_O_BYP CAM+08A0H
typedef struct {
        FIELD DM_BYP                    : 1;
        FIELD DM_DEBUG_MODE             : 2;
        FIELD DM_HF_LSC_MAX_BYP         : 1;
        FIELD DM_FG_MODE                : 1;
        FIELD DM_SPARE1                 : 1;
        FIELD rsv_6                     : 26;
} ISP_CAM_DM_O_BYP_T;

typedef union {
    enum { MASK     = 0x0000003F };
    typedef ISP_CAM_DM_O_BYP_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DM_O_BYP_T;

// CAM_DM_O_ED_FLAT CAM+08A4H
typedef struct {
        FIELD DM_FLAT_DET_MODE          : 1;
        FIELD DM_STEP_DET_MODE          : 1;
        FIELD rsv_2                     : 6;
        FIELD DM_FLAT_TH                : 8;
        FIELD rsv_16                    : 16;
} ISP_CAM_DM_O_ED_FLAT_T;

typedef union {
    enum { MASK     = 0x0000FF03 };
    typedef ISP_CAM_DM_O_ED_FLAT_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DM_O_ED_FLAT_T;

// CAM_DM_O_ED_NYQ CAM+08A8H
typedef struct {
        FIELD DM_NYQ_TH_1               : 8;
        FIELD DM_NYQ_TH_2               : 8;
        FIELD DM_NYQ_TH_3               : 8;
        FIELD DM_HF_NYQ_GAIN            : 2;
        FIELD rsv_26                    : 6;
} ISP_CAM_DM_O_ED_NYQ_T;

typedef union {
    enum { MASK     = 0x03FFFFFF };
    typedef ISP_CAM_DM_O_ED_NYQ_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DM_O_ED_NYQ_T;

// CAM_DM_O_ED_STEP CAM+08ACH
typedef struct {
        FIELD DM_STEP_TH_1              : 8;
        FIELD DM_STEP_TH_2              : 8;
        FIELD DM_STEP_TH_3              : 8;
        FIELD rsv_24                    : 7;
        FIELD DM_RB_MODE                : 1;
} ISP_CAM_DM_O_ED_STEP_T;

typedef union {
    enum { MASK     = 0x80FFFFFF };
    typedef ISP_CAM_DM_O_ED_STEP_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DM_O_ED_STEP_T;

// CAM_DM_O_RGB_HF CAM+08B0H
typedef struct {
        FIELD DM_HF_CORE_GAIN           : 5;
        FIELD DM_ROUGH_RB_F             : 5;
        FIELD DM_ROUGH_RB_D             : 5;
        FIELD DM_ROUGH_G_F              : 5;
        FIELD DM_ROUGH_G_D              : 5;
        FIELD DM_RB_MODE_F              : 2;
        FIELD DM_RB_MODE_D              : 2;
        FIELD DM_RB_MODE_HV             : 2;
        FIELD DM_SSG_MODE               : 1;
} ISP_CAM_DM_O_RGB_HF_T;

typedef union {
    enum { MASK     = 0xFFFFFFFF };
    typedef ISP_CAM_DM_O_RGB_HF_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DM_O_RGB_HF_T;

// CAM_DM_O_DOT CAM+08B4H
typedef struct {
        FIELD DM_DOT_B_TH               : 8;
        FIELD DM_DOT_W_TH               : 8;
        FIELD rsv_16                    : 16;
} ISP_CAM_DM_O_DOT_T;

typedef union {
    enum { MASK     = 0x0000FFFF };
    typedef ISP_CAM_DM_O_DOT_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DM_O_DOT_T;

// CAM_DM_O_F1_ACT CAM+08B8H
typedef struct {
        FIELD DM_F1_TH_1                : 9;
        FIELD DM_F1_TH_2                : 9;
        FIELD DM_F1_SLOPE_1             : 2;
        FIELD DM_F1_SLOPE_2             : 2;
        FIELD rsv_22                    : 10;
} ISP_CAM_DM_O_F1_ACT_T;

typedef union {
    enum { MASK     = 0x003FFFFF };
    typedef ISP_CAM_DM_O_F1_ACT_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DM_O_F1_ACT_T;

// CAM_DM_O_F2_ACT CAM+08BCH
typedef struct {
        FIELD DM_F2_TH_1                : 9;
        FIELD DM_F2_TH_2                : 9;
        FIELD DM_F2_SLOPE_1             : 2;
        FIELD DM_F2_SLOPE_2             : 2;
        FIELD rsv_22                    : 10;
} ISP_CAM_DM_O_F2_ACT_T;

typedef union {
    enum { MASK     = 0x003FFFFF };
    typedef ISP_CAM_DM_O_F2_ACT_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DM_O_F2_ACT_T;

// CAM_DM_O_F3_ACT CAM+08C0H
typedef struct {
        FIELD DM_F3_TH_1                : 9;
        FIELD DM_F3_TH_2                : 9;
        FIELD DM_F3_SLOPE_1             : 2;
        FIELD DM_F3_SLOPE_2             : 2;
        FIELD rsv_22                    : 10;
} ISP_CAM_DM_O_F3_ACT_T;

typedef union {
    enum { MASK     = 0x003FFFFF };
    typedef ISP_CAM_DM_O_F3_ACT_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DM_O_F3_ACT_T;

// CAM_DM_O_F4_ACT CAM+08C4H
typedef struct {
        FIELD DM_F4_TH_1                : 9;
        FIELD DM_F4_TH_2                : 9;
        FIELD DM_F4_SLOPE_1             : 2;
        FIELD DM_F4_SLOPE_2             : 2;
        FIELD rsv_22                    : 10;
} ISP_CAM_DM_O_F4_ACT_T;

typedef union {
    enum { MASK     = 0x003FFFFF };
    typedef ISP_CAM_DM_O_F4_ACT_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DM_O_F4_ACT_T;

// CAM_DM_O_F1_L CAM+08C8H
typedef struct {
        FIELD DM_F1_LLUT_Y0             : 5;
        FIELD DM_F1_LLUT_Y1             : 5;
        FIELD DM_F1_LLUT_Y2             : 5;
        FIELD DM_F1_LLUT_Y3             : 5;
        FIELD DM_F1_LLUT_Y4             : 5;
        FIELD rsv_25                    : 7;
} ISP_CAM_DM_O_F1_L_T;

typedef union {
    enum { MASK     = 0x01FFFFFF };
    typedef ISP_CAM_DM_O_F1_L_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DM_O_F1_L_T;

// CAM_DM_O_F2_L CAM+08CCH
typedef struct {
        FIELD DM_F2_LLUT_Y0             : 5;
        FIELD DM_F2_LLUT_Y1             : 5;
        FIELD DM_F2_LLUT_Y2             : 5;
        FIELD DM_F2_LLUT_Y3             : 5;
        FIELD DM_F2_LLUT_Y4             : 5;
        FIELD rsv_25                    : 7;
} ISP_CAM_DM_O_F2_L_T;

typedef union {
    enum { MASK     = 0x01FFFFFF };
    typedef ISP_CAM_DM_O_F2_L_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DM_O_F2_L_T;

// CAM_DM_O_F3_L CAM+08D0H
typedef struct {
        FIELD DM_F3_LLUT_Y0             : 5;
        FIELD DM_F3_LLUT_Y1             : 5;
        FIELD DM_F3_LLUT_Y2             : 5;
        FIELD DM_F3_LLUT_Y3             : 5;
        FIELD DM_F3_LLUT_Y4             : 5;
        FIELD rsv_25                    : 7;
} ISP_CAM_DM_O_F3_L_T;

typedef union {
    enum { MASK     = 0x01FFFFFF };
    typedef ISP_CAM_DM_O_F3_L_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DM_O_F3_L_T;

// CAM_DM_O_F4_L CAM+08D4H
typedef struct {
        FIELD DM_F4_LLUT_Y0             : 5;
        FIELD DM_F4_LLUT_Y1             : 5;
        FIELD DM_F4_LLUT_Y2             : 5;
        FIELD DM_F4_LLUT_Y3             : 5;
        FIELD DM_F4_LLUT_Y4             : 5;
        FIELD rsv_25                    : 7;
} ISP_CAM_DM_O_F4_L_T;

typedef union {
    enum { MASK     = 0x01FFFFFF };
    typedef ISP_CAM_DM_O_F4_L_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DM_O_F4_L_T;

// CAM_DM_O_HF_RB CAM+08D8H
typedef struct {
        FIELD DM_RB_DIFF_TH             : 10;
        FIELD DM_HF_CLIP                : 9;
        FIELD rsv_19                    : 13;
} ISP_CAM_DM_O_HF_RB_T;

typedef union {
    enum { MASK     = 0x0007FFFF };
    typedef ISP_CAM_DM_O_HF_RB_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DM_O_HF_RB_T;

// CAM_DM_O_HF_GAIN CAM+08DCH
typedef struct {
        FIELD DM_HF_GLOBL_GAIN          : 4;
        FIELD rsv_4                     : 28;
} ISP_CAM_DM_O_HF_GAIN_T;

typedef union {
    enum { MASK     = 0x0000000F };
    typedef ISP_CAM_DM_O_HF_GAIN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DM_O_HF_GAIN_T;

// CAM_DM_O_HF_COMP CAM+08E0H
typedef struct {
        FIELD DM_HF_LSC_GAIN0           : 4;
        FIELD DM_HF_LSC_GAIN1           : 4;
        FIELD DM_HF_LSC_GAIN2           : 4;
        FIELD DM_HF_LSC_GAIN3           : 4;
        FIELD DM_HF_UND_TH              : 8;
        FIELD DM_HF_UND_ACT_TH          : 8;
} ISP_CAM_DM_O_HF_COMP_T;

typedef union {
    enum { MASK     = 0xFFFFFFFF };
    typedef ISP_CAM_DM_O_HF_COMP_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DM_O_HF_COMP_T;

// CAM_DM_O_HF_CORIN_TH CAM+08E4H
typedef struct {
        FIELD DM_HF_CORIN_TH            : 8;
        FIELD rsv_8                     : 24;
} ISP_CAM_DM_O_HF_CORIN_TH_T;

typedef union {
    enum { MASK     = 0x000000FF };
    typedef ISP_CAM_DM_O_HF_CORIN_TH_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DM_O_HF_CORIN_TH_T;

// CAM_DM_O_ACT_LUT CAM+08E8H
typedef struct {
        FIELD DM_ACT_LUT_Y0             : 5;
        FIELD DM_ACT_LUT_Y1             : 5;
        FIELD DM_ACT_LUT_Y2             : 5;
        FIELD DM_ACT_LUT_Y3             : 5;
        FIELD DM_ACT_LUT_Y4             : 5;
        FIELD rsv_25                    : 7;
} ISP_CAM_DM_O_ACT_LUT_T;

typedef union {
    enum { MASK     = 0x01FFFFFF };
    typedef ISP_CAM_DM_O_ACT_LUT_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DM_O_ACT_LUT_T;

// CAM_DM_O_SPARE CAM+08F0H
typedef struct {
        FIELD DM_O_SPARE                : 32;
} ISP_CAM_DM_O_SPARE_T;

typedef union {
    enum { MASK     = 0xFFFFFFFF };
    typedef ISP_CAM_DM_O_SPARE_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DM_O_SPARE_T;

// CAM_DM_O_BB CAM+08F4H
typedef struct {
        FIELD DM_BB_TH_1                : 9;
        FIELD rsv_9                     : 3;
        FIELD DM_BB_TH_2                : 9;
        FIELD rsv_21                    : 11;
} ISP_CAM_DM_O_BB_T;

typedef union {
    enum { MASK     = 0x001FF1FF };
    typedef ISP_CAM_DM_O_BB_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_DM_O_BB_T;

//
typedef union {
    enum { COUNT = 20 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_DM_O_BYP_T          byp;
        ISP_NVRAM_DM_O_ED_FLAT_T      ed_flat;
        ISP_NVRAM_DM_O_ED_NYQ_T       ed_nyq;
        ISP_NVRAM_DM_O_ED_STEP_T      ed_step;
        ISP_NVRAM_DM_O_RGB_HF_T       rgb_hf;
        ISP_NVRAM_DM_O_DOT_T          dot;
        ISP_NVRAM_DM_O_F1_ACT_T       f1_act;
        ISP_NVRAM_DM_O_F2_ACT_T       f2_act;
        ISP_NVRAM_DM_O_F3_ACT_T       f3_act;
        ISP_NVRAM_DM_O_F4_ACT_T       f4_act;
        ISP_NVRAM_DM_O_F1_L_T         f1_l;
        ISP_NVRAM_DM_O_F2_L_T         f2_l;
        ISP_NVRAM_DM_O_F3_L_T         f3_l;
        ISP_NVRAM_DM_O_F4_L_T         f4_l;
        ISP_NVRAM_DM_O_HF_RB_T        hf_rb;
        ISP_NVRAM_DM_O_HF_GAIN_T      hf_gain;
        ISP_NVRAM_DM_O_HF_COMP_T      hf_comp;
        ISP_NVRAM_DM_O_HF_CORIN_TH_T  hf_coring_th;
        ISP_NVRAM_DM_O_ACT_LUT_T      act_lut;
        //ISP_NVRAM_DM_O_SPARE_T        spare;
        ISP_NVRAM_DM_O_BB_T           bb;
    };
} ISP_NVRAM_CFA_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// G2G (CCM) (NOT the same as 89)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CAM_G2G_CNV_1 CAM+0920H
typedef struct {
        FIELD G2G_CNV_00                : 13;
        FIELD rsv_13                    : 3;
        FIELD G2G_CNV_01                : 13;
        FIELD rsv_29                    : 3;
} ISP_CAM_G2G_CNV_1_T;

typedef union {
    enum { MASK     = 0x1FFF1FFF };
    typedef ISP_CAM_G2G_CNV_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2G_CNV_1_T;

// CAM_G2G_CNV_2 CAM+0924H
typedef struct {
        FIELD G2G_CNV_02                : 13;
        FIELD rsv_13                    : 19;
} ISP_CAM_G2G_CNV_2_T;

typedef union {
    enum { MASK     = 0x00001FFF };
    typedef ISP_CAM_G2G_CNV_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2G_CNV_2_T;

// CAM_G2G_CNV_3 CAM+0928H
typedef struct {
        FIELD G2G_CNV_10                : 13;
        FIELD rsv_13                    : 3;
        FIELD G2G_CNV_11                : 13;
        FIELD rsv_29                    : 3;
} ISP_CAM_G2G_CNV_3_T;

typedef union {
    enum { MASK     = 0x1FFF1FFF };
    typedef ISP_CAM_G2G_CNV_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2G_CNV_3_T;

// CAM_G2G_CNV_4 CAM+092CH
typedef struct {
        FIELD G2G_CNV_12                : 13;
        FIELD rsv_13                    : 19;
} ISP_CAM_G2G_CNV_4_T;

typedef union {
    enum { MASK     = 0x00001FFF };
    typedef ISP_CAM_G2G_CNV_4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2G_CNV_4_T;

// CAM_G2G_CNV_5 CAM+0930H
typedef struct {
        FIELD G2G_CNV_20                : 13;
        FIELD rsv_13                    : 3;
        FIELD G2G_CNV_21                : 13;
        FIELD rsv_29                    : 3;
} ISP_CAM_G2G_CNV_5_T;

typedef union {
    enum { MASK     = 0x1FFF1FFF };
    typedef ISP_CAM_G2G_CNV_5_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2G_CNV_5_T;

// CAM_G2G_CNV_6 CAM+0934H
typedef struct {
        FIELD G2G_CNV_22                : 13;
        FIELD rsv_13                    : 19;
} ISP_CAM_G2G_CNV_6_T;

typedef union {
    enum { MASK     = 0x00001FFF };
    typedef ISP_CAM_G2G_CNV_6_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2G_CNV_6_T;

// CAM_G2G_CTRL CAM+0938H
typedef struct {
        FIELD G2G_ACC                   : 4;
        FIELD G2G_MOFST_R               : 1;
        FIELD G2G_POFST_R               : 1;
        FIELD rsv_6                     : 26;
} ISP_CAM_G2G_CTRL_T;

typedef union {
    enum { MASK     = 0x0000002F };
    typedef ISP_CAM_G2G_CTRL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2G_CTRL_T;

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

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// GGM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
enum { GGM_LUT_SIZE = 144 };

// CAM_GGM_CTRL CAM+1480H
typedef struct {
        FIELD GGM_EN                    : 1;
        FIELD rsv_1                     : 31;
} ISP_CAM_GGM_CTRL_T;

typedef union {
    enum { MASK     = 0x00000001 };
    typedef ISP_CAM_GGM_CTRL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_GGM_CTRL_T;

// CAM_GGM_LUT_RB CAM+1000H
typedef struct {
        FIELD GGM_R                     : 16;
        FIELD GGM_B                     : 16;
} ISP_CAM_GGM_LUT_RB_T;

// CAM_GGM_LUT_G CAM+1240H
typedef struct {
        FIELD GGM_G                     : 16;
        FIELD rsv_16                    : 16;
} ISP_CAM_GGM_LUT_G_T;

//
typedef union {
    enum { COUNT = 1 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_GGM_CTRL_T ctrl;
    };
} ISP_NVRAM_GGM_CON_T;

//
typedef union {
    enum { COUNT = GGM_LUT_SIZE };
    MUINT32 set[COUNT];
    struct {
        ISP_CAM_GGM_LUT_RB_T  lut[GGM_LUT_SIZE];
    };
} ISP_NVRAM_GGM_LUT_RB_T;

//
typedef union {
    enum { COUNT = GGM_LUT_SIZE };
    MUINT32 set[COUNT];
    struct {
        ISP_CAM_GGM_LUT_G_T   lut[GGM_LUT_SIZE];
    };
} ISP_NVRAM_GGM_LUT_G_T;

//
typedef struct {
    ISP_NVRAM_GGM_LUT_RB_T  lut_rb;
    ISP_NVRAM_GGM_LUT_G_T   lut_g;
} ISP_NVRAM_GGM_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// G2C (NOT the same as 89)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CAM_G2C_CONV_0A CAM+BA0H
typedef struct {
        FIELD G2C_CNV_00                : 11;
        FIELD rsv_11                    : 5;
        FIELD G2C_CNV_01                : 11;
        FIELD rsv_27                    : 5;
} ISP_CAM_G2C_CONV_0A_T;

typedef union {
    enum { MASK     = 0x07FF07FF };
    typedef ISP_CAM_G2C_CONV_0A_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2C_CONV_0A_T;

// CAM_G2C_CONV_0B CAM+BA4H
typedef struct {
        FIELD G2C_CNV_02                : 11;
        FIELD rsv_11                    : 5;
        FIELD G2C_Y_OFST                : 11;
        FIELD rsv_27                    : 5;
} ISP_CAM_G2C_CONV_0B_T;

typedef union {
    enum { MASK     = 0x07FF07FF };
    typedef ISP_CAM_G2C_CONV_0B_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2C_CONV_0B_T;

// CAM_G2C_CONV_1A CAM+BA8H
typedef struct {
        FIELD G2C_CNV_10                : 11;
        FIELD rsv_11                    : 5;
        FIELD G2C_CNV_11                : 11;
        FIELD rsv_27                    : 5;
} ISP_CAM_G2C_CONV_1A_T;

typedef union {
    enum { MASK     = 0x07FF07FF };
    typedef ISP_CAM_G2C_CONV_1A_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2C_CONV_1A_T;

// CAM_G2C_CONV_1B CAM+BACH
typedef struct {
        FIELD G2C_CNV_12                : 11;
        FIELD rsv_11                    : 5;
        FIELD G2C_U_OFST                : 10;
        FIELD rsv_26                    : 6;
} ISP_CAM_G2C_CONV_1B_T;

typedef union {
    enum { MASK     = 0x03FF07FF };
    typedef ISP_CAM_G2C_CONV_1B_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2C_CONV_1B_T;

// CAM_G2C_CONV_2A CAM+BB0H
typedef struct {
        FIELD G2C_CNV_20                : 11;
        FIELD rsv_11                    : 5;
        FIELD G2C_CNV_21                : 11;
        FIELD rsv_27                    : 5;
} ISP_CAM_G2C_CONV_2A_T;

typedef union {
    enum { MASK     = 0x07FF07FF };
    typedef ISP_CAM_G2C_CONV_2A_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2C_CONV_2A_T;

// CAM_G2C_CONV_2B CAM+BB4H
typedef struct {
        FIELD G2C_CNV_22                : 11;
        FIELD rsv_11                    : 5;
        FIELD G2C_V_OFST                : 10;
        FIELD rsv_26                    : 6;
} ISP_CAM_G2C_CONV_2B_T;

typedef union {
    enum { MASK     = 0x03FF07FF };
    typedef ISP_CAM_G2C_CONV_2B_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2C_CONV_2B_T;

// CAM_G2C_SHADE_CON_1 CAM+BB8H
typedef struct {
        FIELD G2C_SHADE_VAR             : 16;
        FIELD G2C_SHADE_P0              : 11;
        FIELD rsv_27                    : 1;
        FIELD G2C_SHADE_EN              : 1;
        FIELD rsv_29                    : 3;
} ISP_CAM_G2C_SHADE_CON_1_T;

typedef union {
    enum { MASK     = 0x17FFFFFF };
    typedef ISP_CAM_G2C_SHADE_CON_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2C_SHADE_CON_1_T;

// CAM_G2C_SHADE_CON_2 CAM+BBCH
typedef struct {
        FIELD G2C_SHADE_P1              : 11;
        FIELD rsv_11                    : 1;
        FIELD G2C_SHADE_P2              : 11;
        FIELD rsv_23                    : 9;
} ISP_CAM_G2C_SHADE_CON_2_T;

typedef union {
    enum { MASK     = 0x007FF7FF };
    typedef ISP_CAM_G2C_SHADE_CON_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2C_SHADE_CON_2_T;

// CAM_G2C_SHADE_CON_3 CAM+BC0H
typedef struct {
        FIELD G2C_SHADE_UB              : 9;
        FIELD rsv_9                     : 23;
} ISP_CAM_G2C_SHADE_CON_3_T;

typedef union {
    enum { MASK     = 0x000001FF };
    typedef ISP_CAM_G2C_SHADE_CON_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2C_SHADE_CON_3_T;

// CAM_G2C_SHADE_TAR CAM+BC4H
typedef struct {
        FIELD G2C_SHADE_XMID            : 13;
        FIELD rsv_13                    : 3;
        FIELD G2C_SHADE_YMID            : 13;
        FIELD rsv_29                    : 3;
} ISP_CAM_G2C_SHADE_TAR_T;

typedef union {
    enum { MASK     = 0x1FFF1FFF };
    typedef ISP_CAM_G2C_SHADE_TAR_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2C_SHADE_TAR_T;

// CAM_G2C_SHADE_SP CAM+BC8H
typedef struct {
        FIELD G2C_SHADE_XSP             : 13;
        FIELD rsv_13                    : 3;
        FIELD G2C_SHADE_YSP             : 13;
        FIELD rsv_29                    : 3;
} ISP_CAM_G2C_SHADE_SP_T;

typedef union {
    enum { MASK     = 0x1FFF1FFF };
    typedef ISP_CAM_G2C_SHADE_SP_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_G2C_SHADE_SP_T;

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
        ISP_NVRAM_G2C_SHADE_CON_1_T con_1;
        ISP_NVRAM_G2C_SHADE_CON_2_T con_2;
        ISP_NVRAM_G2C_SHADE_CON_3_T con_3;
        ISP_NVRAM_G2C_SHADE_TAR_T   tar;
        ISP_NVRAM_G2C_SHADE_SP_T    sp;
    };
} ISP_NVRAM_G2C_SHADE_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ANR_TBL
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
enum { ANR_HW_TBL_NUM = 256 };
//CAM_ANR_TBL
typedef struct {
        FIELD ANR_TBL_Y0        : 4;
        FIELD ANR_TBL_U0        : 4;
        FIELD ANR_TBL_V0        : 4;
        FIELD rsv_12            : 4;
        FIELD ANR_TBL_Y1        : 4;
        FIELD ANR_TBL_U1        : 4;
        FIELD ANR_TBL_V1        : 4;
        FIELD rsv_28            : 4;
} ISP_CAM_ANR_TBL_T;
typedef union {
    enum { MASK     = 0x0FFF0FFF };
    typedef ISP_CAM_ANR_TBL_T reg_t;
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
// CAM_ANR_CON1 CAM+A20H
typedef struct {
        FIELD ANR_ENC                  : 1;        //615revision
        FIELD ANR_ENY                  : 1;        //615revision
        FIELD rsv_2                    : 2;        //615revision
        FIELD ANR_SCALE_MODE           : 2;        //615revision
        FIELD rsv_6                    : 10;       //615revision
        FIELD ANR_TILE_EDGE            : 4;        //615revision
        FIELD ANR_MEDIAN_EN            : 1;        //615revision
        FIELD rsv_21                   : 3;        //615revision
        FIELD ANR_LCE_LINK             : 1;        //615revision
        FIELD rsv_25                   : 3;        //615revision
        FIELD ANR_TABLE_EN             : 1;        //615revision
        FIELD ANR_TBL_PRC              : 1;        //615revision
        FIELD rsv_30                   : 2;        //615revision
} ISP_CAM_ANR_CON1_T;
typedef union {
    enum { MASK     = 0x311F0033 };
    typedef ISP_CAM_ANR_CON1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ANR_CON1_T;
// CAM_ANR_CON2 CAM+A24H
typedef struct {
		FIELD ANR_IMPL_MODE            : 2;		//615revision
		FIELD rsv_2                    : 2;		//615revision
		FIELD ANR_C_MED_EN             : 2;		//615revision
		FIELD rsv_6                    : 2;		//615revision
		FIELD ANR_C_SM_EDGE            : 2;		//615revision
		FIELD rsv_10                   : 2;		//615revision
		FIELD ANR_FLT_C                : 1;		//615revision
		FIELD rsv_13                   : 3;		//615revision
		FIELD ANR_LR                   : 1;		//615revision
		FIELD rsv_17                   : 3;		//615revision
		FIELD ANR_ALPHA                : 3;		//615revision
		FIELD rsv_23                   : 9;        //615revision
} ISP_CAM_ANR_CON2_T;
typedef union {
    enum { MASK     = 0x00711333 };
    typedef ISP_CAM_ANR_CON2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ANR_CON2_T;
// CAM_ANR_YAD1 CAM+A2CH
typedef struct {
        FIELD ANR_CEN_GAIN_LO_TH       : 5;
		FIELD rsv_5                    : 3;
		FIELD ANR_CEN_GAIN_HI_TH       : 5;
		FIELD rsv_13                   : 3;
		FIELD ANR_K_LO_TH              : 4;
		FIELD rsv_20                   : 4;
		FIELD ANR_K_HI_TH              : 4;
		FIELD ANR_K_TH_C               : 4;
} ISP_CAM_ANR_YAD1_T;
typedef union {
    enum { MASK     = 0xFF0F1F1F };
    typedef ISP_CAM_ANR_YAD1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ANR_YAD1_T;
// CAM_ANR_YAD2 CAM+A30H
typedef struct {
        FIELD ANR_PTY_VGAIN            : 4;
        FIELD rsv_4                    : 4;
        FIELD ANR_PTY_GAIN_TH          : 5;
        FIELD rsv_13                   : 19;
} ISP_CAM_ANR_YAD2_T;
typedef union {
    enum { MASK     = 0x00001F0F };
    typedef ISP_CAM_ANR_YAD2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ANR_YAD2_T;
// CAM_ANR_Y4LUT1 CAM+A34H
typedef struct {
		FIELD ANR_Y_CPX1               : 8;
		FIELD ANR_Y_CPX2               : 8;
		FIELD ANR_Y_CPX3               : 8;
		FIELD rsv_24                   : 8;
} ISP_CAM_ANR_Y4LUT1_T;
typedef union {
    enum { MASK     = 0x00FFFFFF };
    typedef ISP_CAM_ANR_Y4LUT1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ANR_Y4LUT1_T;
// CAM_ANR_Y4LUT2 CAM+A38H
typedef struct {
        FIELD ANR_Y_SCALE_CPY0         : 5;
        FIELD rsv_5                    : 3;
        FIELD ANR_Y_SCALE_CPY1         : 5;
        FIELD rsv_13                   : 3;
        FIELD ANR_Y_SCALE_CPY2         : 5;
        FIELD rsv_21                   : 3;
        FIELD ANR_Y_SCALE_CPY3         : 5;
        FIELD rsv_29                   : 3;
} ISP_CAM_ANR_Y4LUT2_T;
typedef union {
    enum { MASK     = 0x1F1F1F1F };
    typedef ISP_CAM_ANR_Y4LUT2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ANR_Y4LUT2_T;
// CAM_ANR_Y4LUT3 CAM+A3CH
typedef struct {
        FIELD ANR_Y_SCALE_SP0          : 5;
        FIELD rsv_5                    : 3;
        FIELD ANR_Y_SCALE_SP1          : 5;
        FIELD rsv_13                   : 3;
        FIELD ANR_Y_SCALE_SP2          : 5;
        FIELD rsv_21                   : 3;
        FIELD ANR_Y_SCALE_SP3          : 5;
        FIELD rsv_29                   : 3;
} ISP_CAM_ANR_Y4LUT3_T;
typedef union {
    enum { MASK     = 0x1F1F1F1F };
    typedef ISP_CAM_ANR_Y4LUT3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ANR_Y4LUT3_T;
// CAM_ANR_C4LUT1
typedef struct {
		FIELD ANR_C_CPX1    			 : 8;
		FIELD ANR_C_CPX2	    		 : 8;
		FIELD ANR_C_CPX3				 : 8;
		FIELD rsv_24     		    	 : 8;
} ISP_CAM_ANR_C4LUT1_T;
typedef union {
    enum { MASK     = 0x00FFFFFF };
    typedef ISP_CAM_ANR_C4LUT1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ANR_C4LUT1_T;
// CAM_ANR_C4LUT2
typedef struct {
		FIELD ANR_C_SCALE_CPY0			 : 5;
		FIELD rsv_5					     : 3;
		FIELD ANR_C_SCALE_CPY1			 : 5;
		FIELD rsv_13				     : 3;
		FIELD ANR_C_SCALE_CPY2			 : 5;
		FIELD rsv_21			         : 3;
		FIELD ANR_C_SCALE_CPY3			 : 5;
		FIELD rsv_29				     : 3;
} ISP_CAM_ANR_C4LUT2_T;
typedef union {
    enum { MASK     = 0x1F1F1F1F };
    typedef ISP_CAM_ANR_C4LUT2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ANR_C4LUT2_T;
// CAM_ANR_C4LUT3
typedef struct {
		FIELD ANR_C_SCALE_SP0 			 : 5;
		FIELD rsv_5				         : 3;
		FIELD ANR_C_SCALE_SP1 			 : 5;
		FIELD rsv_13				     : 3;
		FIELD ANR_C_SCALE_SP2 			 : 5;
		FIELD rsv_21				     : 3;
		FIELD ANR_C_SCALE_SP3 		     : 5;
		FIELD rsv_29			    	 : 3;
} ISP_CAM_ANR_C4LUT3_T;
typedef union {
    enum { MASK     = 0x1F1F1F1F };
    typedef ISP_CAM_ANR_C4LUT3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ANR_C4LUT3_T;
// CAM_ANR_A4LUT2
typedef struct {
		FIELD ANR_Y_ACT_CPY0            : 8;
		FIELD ANR_Y_ACT_CPY1            : 8;
		FIELD ANR_Y_ACT_CPY2            : 8;
		FIELD ANR_Y_ACT_CPY3            : 8;
} ISP_CAM_ANR_A4LUT2_T;
typedef union {
    enum { MASK     = 0xFFFFFFFF };
    typedef ISP_CAM_ANR_A4LUT2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ANR_A4LUT2_T;
// CAM_ANR_A4LUT3
typedef struct {
		FIELD ANR_Y_ACT_SP0             : 6;
		FIELD rsv_6                     : 2;
		FIELD ANR_Y_ACT_SP1             : 6;
		FIELD rsv_14                    : 2;
		FIELD ANR_Y_ACT_SP2             : 6;
		FIELD rsv_22                    : 2;
		FIELD ANR_Y_ACT_SP3             : 6;
		FIELD rsv_30                    : 2;
} ISP_CAM_ANR_A4LUT3_T;
typedef union {
    enum { MASK     = 0x3F3F3F3F };
    typedef ISP_CAM_ANR_A4LUT3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ANR_A4LUT3_T;
// CAM_ANR_L4LUT1
typedef struct {
		FIELD ANR_LCE_X1				 : 8;
		FIELD ANR_LCE_X2				 : 8;
		FIELD ANR_LCE_X3				 : 8;
		FIELD rsv_24     				 : 8;
} ISP_CAM_ANR_L4LUT1_T;
typedef union {
    enum { MASK     = 0x00FFFFFF };
    typedef ISP_CAM_ANR_L4LUT1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ANR_L4LUT1_T;
// CAM_ANR_L4LUT2
typedef struct {
		FIELD ANR_LCE_GAIN0			 : 6;
		FIELD rsv_6				     : 2;
		FIELD ANR_LCE_GAIN1			 : 6;
		FIELD rsv_14				 : 2;
		FIELD ANR_LCE_GAIN2			 : 6;
		FIELD rsv_22				 : 2;
		FIELD ANR_LCE_GAIN3			 : 6;
		FIELD rsv_30				 : 2;
} ISP_CAM_ANR_L4LUT2_T;
typedef union {
    enum { MASK     = 0x3F3F3F3F };
    typedef ISP_CAM_ANR_L4LUT2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ANR_L4LUT2_T;
// CAM_ANR_L4LUT3
typedef struct {
		FIELD ANR_LCE_SP0 				 : 6;
		FIELD rsv_6						 : 2;
		FIELD ANR_LCE_SP1 				 : 6;
		FIELD rsv_14					 : 2;
		FIELD ANR_LCE_SP2 				 : 6;
		FIELD rsv_22					 : 2;
		FIELD ANR_LCE_SP3 				 : 6;
		FIELD rsv_30					 : 2;
} ISP_CAM_ANR_L4LUT3_T;
typedef union {
    enum { MASK     = 0x3F3F3F3F };
    typedef ISP_CAM_ANR_L4LUT3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ANR_L4LUT3_T;
// CAM_ANR_PTY CAM+A40H
typedef struct {
        FIELD ANR_PTY1                  : 8;
        FIELD ANR_PTY2                  : 8;
        FIELD ANR_PTY3                  : 8;
        FIELD ANR_PTY4                  : 8;
} ISP_CAM_ANR_PTY_T;
typedef union {
    enum { MASK     = 0xFFFFFFFF };
    typedef ISP_CAM_ANR_PTY_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ANR_PTY_T;
// CAM_ANR_CAD CAM+A44H
typedef struct {
        FIELD ANR_PTC_VGAIN            : 4;
        FIELD rsv_4                    : 4;
        FIELD ANR_PTC_GAIN_TH          : 5;
        FIELD rsv_13                   : 3;
        FIELD ANR_C_L_DIFF_TH          : 8;
        FIELD rsv_24                   : 8;
} ISP_CAM_ANR_CAD_T;
typedef union {
    enum { MASK     = 0x00FF1F0F };
    typedef ISP_CAM_ANR_CAD_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ANR_CAD_T;
// CAM_ANR_PTC CAM+A48H
typedef struct {
        FIELD ANR_PTC1                  : 8;
        FIELD ANR_PTC2                  : 8;
        FIELD ANR_PTC3                  : 8;
        FIELD ANR_PTC4                  : 8;
} ISP_CAM_ANR_PTC_T;
typedef union {
    enum { MASK     = 0xFFFFFFFF };
    typedef ISP_CAM_ANR_PTC_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ANR_PTC_T;
// CAM_ANR_LCE CAM+A4CH
typedef struct {
        FIELD  ANR_LCE_C_GAIN           :  4;
        FIELD  ANR_LCE_SCALE_GAIN       :  3;
        FIELD  rsv_7                    :  9;
        FIELD  ANR_LM_WT                :  4;
        FIELD  rsv_20                   : 12;
} ISP_CAM_ANR_LCE_T;
typedef union {
    enum { MASK     = 0x000F007F };
    typedef ISP_CAM_ANR_LCE_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ANR_LCE_T;
// CAM_ANR_MED1
typedef struct {
		FIELD ANR_COR_TH				 : 5;
		FIELD rsv_5			    	     : 3;
		FIELD ANR_COR_SL   			     : 3;
		FIELD rsv_11				     : 1;
		FIELD ANR_MCD_TH				 : 5;
		FIELD rsv_17				     : 3;
		FIELD ANR_MCD_SL				 : 3;
		FIELD rsv_23				     : 1;
		FIELD ANR_LCL_TH				 : 8;
} ISP_CAM_ANR_MED1_T;
typedef union {
    enum { MASK     = 0xFF71F71F };
    typedef ISP_CAM_ANR_MED1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ANR_MED1_T;
// CAM_ANR_MED2
typedef struct {
		FIELD ANR_LCL_SL				 : 3;
		FIELD rsv_3						 : 1;
		FIELD ANR_LCL_LV				 : 5;
		FIELD rsv_9						 : 3;
		FIELD ANR_SCL_TH				 : 6;
		FIELD rsv_18					 : 2;
		FIELD ANR_SCL_SL				 : 3;
		FIELD rsv_23					 : 1;
		FIELD ANR_SCL_LV				 : 5;
		FIELD rsv_29					 : 3;
} ISP_CAM_ANR_MED2_T;
typedef union {
    enum { MASK     = 0x1F73F1F7 };
    typedef ISP_CAM_ANR_MED2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ANR_MED2_T;
// CAM_ANR_MED3
typedef struct {
		FIELD ANR_NCL_TH				 : 8;
		FIELD ANR_NCL_SL				 : 3;
		FIELD rsv_11					 : 1;
		FIELD ANR_NCL_LV				 : 5;
		FIELD rsv_17					 : 3;
		FIELD ANR_VAR 					 : 3;
		FIELD rsv_23			    	 : 1;
		FIELD ANR_Y0					 : 6;
		FIELD rsv_30					 : 2;
} ISP_CAM_ANR_MED3_T;
typedef union {
    enum { MASK     = 0x3F71F7FF };
    typedef ISP_CAM_ANR_MED3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ANR_MED3_T;
// CAM_ANR_MED4
typedef struct {
		FIELD ANR_Y1					 : 6;
		FIELD rsv_6						 : 2;
		FIELD ANR_Y2					 : 6;
		FIELD rsv_14					 : 2;
		FIELD ANR_Y3					 : 6;
		FIELD rsv_22					 : 2;
		FIELD ANR_Y4					 : 6;
		FIELD rsv_30					 : 2;
} ISP_CAM_ANR_MED4_T;
typedef union {
    enum { MASK     = 0x3F3F3F3F };
    typedef ISP_CAM_ANR_MED4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ANR_MED4_T;
// CAM_ANR_HP1 CAM+A54H
typedef struct {
        FIELD ANR_HP_A                 : 8;
        FIELD ANR_HP_B                 : 6;
        FIELD rsv_14                   : 2;
        FIELD ANR_HP_C                 : 5;
        FIELD rsv_21                   : 3;
        FIELD ANR_HP_D                 : 4;
        FIELD ANR_HP_E                 : 4;
} ISP_CAM_ANR_HP1_T;
typedef union {
    enum { MASK     = 0xFF1F3FFF };
    typedef ISP_CAM_ANR_HP1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ANR_HP1_T;
// CAM_ANR_HP2 CAM+A58H
typedef struct {
        FIELD ANR_HP_S1                : 4;
        FIELD ANR_HP_S2                : 4;
        FIELD ANR_HP_X1                : 7;
        FIELD rsv_15                   : 1;
        FIELD ANR_HP_F                 : 3;
        FIELD rsv_19                   : 13;
} ISP_CAM_ANR_HP2_T;
typedef union {
    enum { MASK     = 0x00077FFF };
    typedef ISP_CAM_ANR_HP2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ANR_HP2_T;
// CAM_ANR_HP3 CAM+A5CH
typedef struct {
        FIELD ANR_HP_Y_GAIN_CLIP       : 7;
        FIELD rsv_7                    : 1;
        FIELD ANR_HP_Y_SP              : 5;
        FIELD rsv_13                   : 3;
        FIELD ANR_HP_Y_LO              : 8;
        FIELD ANR_HP_CLIP              : 8;
} ISP_CAM_ANR_HP3_T;
typedef union {
    enum { MASK     = 0xFFFF1F7F };
    typedef ISP_CAM_ANR_HP3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ANR_HP3_T;
// CAM_ANR_ACT1 CAM+A64H
typedef struct {
		FIELD ANR_ACT_LCE_GAIN			: 4;
		FIELD rsv_4	                    : 4;
		FIELD ANR_ACT_SCALE_OFT		    : 5;
		FIELD rsv_13					: 3;
		FIELD ANR_ACT_SCALE_GAIN		: 3;
		FIELD rsv_19					: 1;
		FIELD ANR_ACT_DIF_GAIN			: 3;
		FIELD rsv_23					: 1;
		FIELD ANR_ACT_DIF_LO_TH		    : 5;
		FIELD rsv_29					: 3;
} ISP_CAM_ANR_ACT1_T;
typedef union {
    enum { MASK     = 0x1F771F0F };
    typedef ISP_CAM_ANR_ACT1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ANR_ACT1_T;
// CAM_ANR_ACT2 CAM+A64H
typedef struct {
		FIELD ANR_ACT_SIZE_GAIN         : 3;
		FIELD rsv_3						: 5;
		FIELD ANR_ACT_SIZE_LO_TH		: 5;
		FIELD rsv_13					: 3;
		FIELD ANR_COR_TH1 				: 5;
		FIELD rsv_21					: 3;
		FIELD ANR_COR_SL1 				: 3;
		FIELD rsv_27					: 5;
} ISP_CAM_ANR_ACT2_T;
typedef union {
    enum { MASK     = 0x071F1F07 };
    typedef ISP_CAM_ANR_ACT2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ANR_ACT2_T;
// CAM_ANR_ACT3 CAM+A64H
typedef struct {
		FIELD ANR_COR_ACT_TH			: 8;
		FIELD ANR_COR_ACT_SL1 			: 4;
		FIELD ANR_COR_ACT_SL2 		    : 4;
		FIELD rsv_16					: 16;
} ISP_CAM_ANR_ACT3_T;
typedef union {
    enum { MASK     = 0x0000FFFF };
    typedef ISP_CAM_ANR_ACT3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ANR_ACT3_T;
// CAM_ANR_ACTYH CAM+A60H
typedef struct {
        FIELD ANR_ACT_TH_Y_H            : 8;
        FIELD ANR_ACT_BLD_BASE_Y_H      : 7;
        FIELD rsv_15                    : 1;
        FIELD ANR_ACT_SLANT_Y_H         : 5;
        FIELD rsv_21                    : 3;
        FIELD ANR_ACT_BLD_TH_Y_H        : 7;
        FIELD rsv_31                    : 1;
} ISP_CAM_ANR_ACTYH_T;
typedef union {
    enum { MASK     = 0x7F1F7FFF };
    typedef ISP_CAM_ANR_ACTYH_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ANR_ACTYH_T;
// CAM_ANR_ACTC CAM+A64H
typedef struct {
        FIELD ANR_ACT_TH_C             : 8;
        FIELD ANR_ACT_BLD_BASE_C       : 7;
        FIELD rsv_15                   : 1;
        FIELD ANR_ACT_SLANT_C          : 5;
        FIELD rsv_21                   : 3;
        FIELD ANR_ACT_BLD_TH_C         : 7;
        FIELD rsv_31                   : 1;
} ISP_CAM_ANR_ACTC_T;
typedef union {
    enum { MASK     = 0x7F1F7FFF };
    typedef ISP_CAM_ANR_ACTC_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ANR_ACTC_T;
// CAM_ANR_ACTYL CAM+A60H
typedef struct {
        FIELD ANR_ACT_TH_Y_L            : 8;
        FIELD ANR_ACT_BLD_BASE_Y_L      : 7;
        FIELD rsv_15                    : 1;
        FIELD ANR_ACT_SLANT_Y_L         : 5;
        FIELD rsv_21                    : 3;
        FIELD ANR_ACT_BLD_TH_Y_L        : 7;
        FIELD rsv_31                    : 1;
} ISP_CAM_ANR_ACTYL_T;
typedef union {
    enum { MASK     = 0x7F1F7FFF };
    typedef ISP_CAM_ANR_ACTYL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ANR_ACTYL_T;
// CAM_ANR_YLAD CAM+A60H
typedef struct {
        FIELD  ANR_CEN_GAIN_LO_TH_LPF   :  5;
        FIELD  rsv_5                    :  3;
        FIELD  ANR_CEN_GAIN_HI_TH_LPF   :  5;
        FIELD  rsv_13                   :  3;
        FIELD  ANR_K_LMT                :  1;
        FIELD  rsv_17                   :  3;
        FIELD  ANR_K_LPF                :  2;
        FIELD  rsv_22                   : 10;
} ISP_CAM_ANR_YLAD_T;
typedef union {
    enum { MASK     = 0x00311F1F };
    typedef ISP_CAM_ANR_YLAD_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ANR_YLAD_T;
// CAM_ANR_PTYL CAM+A60H
typedef struct {
        FIELD  ANR_PTY1_LPF             :  8;
        FIELD  ANR_PTY2_LPF             :  8;
        FIELD  ANR_PTY3_LPF             :  8;
        FIELD  ANR_PTY4_LPF             :  8;
} ISP_CAM_ANR_PTYL_T;
typedef union {
    enum { MASK     = 0xFFFFFFFF };
    typedef ISP_CAM_ANR_PTYL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ANR_PTYL_T;
// CAM_ANR_LCOEF CAM+A60H
typedef struct {
        FIELD  ANR_COEF_A               :  4;
        FIELD  ANR_COEF_B               :  4;
        FIELD  ANR_COEF_C               :  4;
        FIELD  ANR_COEF_D               :  4;
        FIELD  rsv_16                   : 16;
} ISP_CAM_ANR_LCOEF_T;
typedef union {
    enum { MASK     = 0x0000FFFF };
    typedef ISP_CAM_ANR_LCOEF_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ANR_LCOEF_T;
// CAM_ANR_RSV1
typedef struct {
		FIELD ANR_RSV1					: 32;
} ISP_CAM_ANR_RSV1_T;
typedef union {
    enum { MASK     = 0xFFFFFFFF };
    typedef ISP_CAM_ANR_RSV1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_ANR_RSV1_T;
//
typedef union {
    enum { COUNT = 35 };
    MUINT32 set[COUNT];
    struct {
		//ISP_NVRAM_ANR_TBL_T    tbl;     //0724forbuild
        ISP_NVRAM_ANR_CON1_T   con1;
        ISP_NVRAM_ANR_CON2_T   con2;
        ISP_NVRAM_ANR_YAD1_T   yad1;
        ISP_NVRAM_ANR_YAD2_T   yad2;
        ISP_NVRAM_ANR_Y4LUT1_T lut1;
        ISP_NVRAM_ANR_Y4LUT2_T lut2;
        ISP_NVRAM_ANR_Y4LUT3_T lut3;
		ISP_NVRAM_ANR_C4LUT1_T clut1;
		ISP_NVRAM_ANR_C4LUT2_T clut2;
		ISP_NVRAM_ANR_C4LUT3_T clut3;
		ISP_NVRAM_ANR_A4LUT2_T alut2;
		ISP_NVRAM_ANR_A4LUT3_T alut3;
		ISP_NVRAM_ANR_L4LUT1_T llut1;
		ISP_NVRAM_ANR_L4LUT2_T llut2;
		ISP_NVRAM_ANR_L4LUT3_T llut3;
        ISP_NVRAM_ANR_PTY_T    pty;
        ISP_NVRAM_ANR_CAD_T    cad;
        ISP_NVRAM_ANR_PTC_T    ptc;
        ISP_NVRAM_ANR_LCE_T    lce;
		ISP_NVRAM_ANR_MED1_T   med1;
		ISP_NVRAM_ANR_MED2_T   med2;
		ISP_NVRAM_ANR_MED3_T   med3;
		ISP_NVRAM_ANR_MED4_T   med4;
        ISP_NVRAM_ANR_HP1_T    hp1;
        ISP_NVRAM_ANR_HP2_T    hp2;
        ISP_NVRAM_ANR_HP3_T    hp3;
		ISP_NVRAM_ANR_ACT1_T   act1;
		ISP_NVRAM_ANR_ACT2_T   act2;
		ISP_NVRAM_ANR_ACT3_T   act3;
        ISP_NVRAM_ANR_ACTYH_T  actyh;
        ISP_NVRAM_ANR_ACTC_T   actc;
        ISP_NVRAM_ANR_ACTYL_T  actyl;
        ISP_NVRAM_ANR_YLAD_T   ylad;
        ISP_NVRAM_ANR_PTYL_T   ptyl;
        ISP_NVRAM_ANR_LCOEF_T  lcoef;
    };
} ISP_NVRAM_ANR_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CCR
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CAM_CCR_CON CAM+A80H
typedef struct {
        FIELD CCR_EN                    : 1;
        FIELD rsv_1                     : 4;
        FIELD CCR_OR_MODE               : 1;
        FIELD rsv_6                     : 2;
        FIELD CCR_UV_GAIN_MODE          : 1;
        FIELD rsv_9                     : 7;
        FIELD CCR_UV_GAIN2              : 7;
        FIELD rsv_23                    : 1;
        FIELD CCR_Y_CPX3                : 8;
} ISP_CAM_CCR_CON_T;
typedef union {
    enum { MASK     = 0xFF7F0121 };
    typedef ISP_CAM_CCR_CON_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CCR_CON_T;
// CAM_CCR_YLUT CAM+A84H
typedef struct {
        FIELD CCR_Y_CPX1                : 8;
        FIELD CCR_Y_CPX2                : 8;
        FIELD CCR_Y_SP1                 : 7;
        FIELD rsv_23                    : 1;
        FIELD CCR_Y_CPY1                : 7;
        FIELD rsv_31                    : 1;
} ISP_CAM_CCR_YLUT_T;
typedef union {
    enum { MASK     = 0x7F7FFFFF };
    typedef ISP_CAM_CCR_YLUT_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CCR_YLUT_T;
// CAM_CCR_UVLUT CAM+A88H
typedef struct {
        FIELD CCR_UV_X1                 : 8;
        FIELD CCR_UV_X2                 : 8;
        FIELD CCR_UV_X3                 : 8;
        FIELD CCR_UV_GAIN1              : 7;
        FIELD rsv_31                    : 1;
} ISP_CAM_CCR_UVLUT_T;
typedef union {
    enum { MASK     = 0x7FFFFFFF };
    typedef ISP_CAM_CCR_UVLUT_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CCR_UVLUT_T;
// CAM_CCR_YLUT2 CAM+A8CH
typedef struct {
        FIELD CCR_Y_SP0                 : 7;
        FIELD rsv_7                     : 1;
        FIELD CCR_Y_SP2                 : 7;
        FIELD rsv_15                    : 1;
        FIELD CCR_Y_CPY0                : 7;
        FIELD rsv_23                    : 1;
        FIELD CCR_Y_CPY2                : 7;
        FIELD rsv_31                    : 1;
} ISP_CAM_CCR_YLUT2_T;
typedef union {
    enum { MASK     = 0x7F7F7F7F };
    typedef ISP_CAM_CCR_YLUT2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CCR_YLUT2_T;
// CAM_CCR_SAT_CTRL CAM+A90H
typedef struct {
        FIELD CCR_MODE                  : 1;
        FIELD rsv_1                     : 3;
        FIELD CCR_CEN_U                 : 6;
        FIELD rsv_10                    : 2;
        FIELD CCR_CEN_V                 : 6;
        FIELD rsv_18                    : 14;
} ISP_CAM_CCR_SAT_CTRL_T;
typedef union {
    enum { MASK     = 0x0003F3F1 };
    typedef ISP_CAM_CCR_SAT_CTRL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CCR_SAT_CTRL_T;
// CAM_CCR_UVLUT_SP CAM+A94H
typedef struct {
        FIELD CCR_UV_GAIN_SP1           : 10;
        FIELD rsv_10                    : 6;
        FIELD CCR_UV_GAIN_SP2           : 10;
        FIELD rsv_26                    : 6;
} ISP_CAM_CCR_UVLUT_SP_T;
typedef union {
    enum { MASK     = 0x03FF03FF };
    typedef ISP_CAM_CCR_UVLUT_SP_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CCR_UVLUT_SP_T;
// CAM_CCR_HUE1 CAM+A94H
typedef struct {
		FIELD CCR_HUE_X1				: 9;
		FIELD rsv_9						: 7;
		FIELD CCR_HUE_X2				: 9;
		FIELD rsv_25					: 7;
} ISP_CAM_CCR_HUE1_T;
typedef union {
    enum { MASK     = 0x01FF01FF };
    typedef ISP_CAM_CCR_HUE1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CCR_HUE1_T;
// CAM_CCR_HUE2 CAM+A94H
typedef struct {
		FIELD CCR_HUE_X3				: 9;
		FIELD rsv_9						: 7;
		FIELD CCR_HUE_X4				: 9;
		FIELD rsv_25					: 7;
} ISP_CAM_CCR_HUE2_T;
typedef union {
    enum { MASK     = 0x01FF01FF };
    typedef ISP_CAM_CCR_HUE2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CCR_HUE2_T;
// CAM_CCR_HUE3 CAM+A94H
typedef struct {
		FIELD CCR_HUE_SP1				: 8;
		FIELD CCR_HUE_SP2				: 8;
		FIELD CCR_HUE_GAIN1				: 7;
		FIELD rsv_23					: 1;
		FIELD CCR_HUE_GAIN2				: 7;
		FIELD rsv_31					: 1;
} ISP_CAM_CCR_HUE3_T;
typedef union {
    enum { MASK     = 0x7F7FFFFF };
    typedef ISP_CAM_CCR_HUE3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CCR_HUE3_T;
// CAM_CCR_RSV1 CAM+A94H
typedef struct {
		FIELD CCR_RSV1 					: 32;
} ISP_CAM_CCR_RSV1_T;
typedef union {
    enum { MASK     = 0xFFFFFFFF };
    typedef ISP_CAM_CCR_RSV1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_CCR_RSV1_T;
//
typedef union {
    enum { COUNT = 9 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_CCR_CON_T      con;
        ISP_NVRAM_CCR_YLUT_T     ylut;
        ISP_NVRAM_CCR_UVLUT_T    uvlut;
        ISP_NVRAM_CCR_YLUT2_T    ylut2;
        ISP_NVRAM_CCR_SAT_CTRL_T sat_ctrl;
		ISP_NVRAM_CCR_UVLUT_SP_T uvlut_sp;
        ISP_NVRAM_CCR_HUE1_T     hue1;
        ISP_NVRAM_CCR_HUE2_T     hue2;
        ISP_NVRAM_CCR_HUE3_T     hue3;
    };
} ISP_NVRAM_CCR_T;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// PCA (Not the same as 89)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CAM_PCA_CON1 CAM+1E00H
typedef struct {
        FIELD PCA_LUT_360               : 1;
        FIELD rsv_1                     : 3;
        FIELD RSV                       : 1;
        FIELD rsv_5                     : 27;
} ISP_CAM_PCA_CON1_T;

typedef union {
    enum { MASK     = 0x00000001 };
    typedef ISP_CAM_PCA_CON1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_PCA_CON1_T;

// CAM_PCA_CON2 CAM+1E04H
typedef struct {
        FIELD PCA_C_TH                  : 5;
        FIELD rsv_5                     : 3;
        FIELD RSV                       : 2;
        FIELD rsv_10                    : 2;
        FIELD PCA_S_TH_EN               : 1;
        FIELD rsv_13                    : 3;
        FIELD PCA_S_TH                  : 8;
        FIELD PCA_Y2YLUT_EN             : 1;
        FIELD rsv_25                    : 7;
} ISP_CAM_PCA_CON2_T;

typedef union {
    enum { MASK     = 0x01FF101F };
    typedef ISP_CAM_PCA_CON2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_PCA_CON2_T;

//
typedef union{
    enum { COUNT = 2 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_PCA_CON1_T  con1;
        ISP_NVRAM_PCA_CON2_T  con2;
    };
} ISP_NVRAM_PCA_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// PCA LUT
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
enum { PCA_BIN_NUM = 180 };

// CAM_PCA_TBL CAM+1800H
typedef struct {
        FIELD PCA_LUMA_GAIN             : 8;
        FIELD PCA_SAT_GAIN              : 8;
        FIELD PCA_HUE_SHIFT             : 8;
        FIELD rsv_24                    : 8;
} ISP_CAM_PCA_BIN_T;

typedef union {
    enum { MASK     = 0xFFFFFFFF };
    typedef ISP_CAM_PCA_BIN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_PCA_BIN_T;

//
typedef struct {
    ISP_NVRAM_PCA_BIN_T lut_lo[PCA_BIN_NUM];
    ISP_NVRAM_PCA_BIN_T lut_md[PCA_BIN_NUM];
    ISP_NVRAM_PCA_BIN_T lut_hi[PCA_BIN_NUM];
    ISP_NVRAM_PCA_BIN_T lut_lo2[PCA_BIN_NUM];
    ISP_NVRAM_PCA_BIN_T lut_md2[PCA_BIN_NUM];
    ISP_NVRAM_PCA_BIN_T lut_hi2[PCA_BIN_NUM];
    //for flash PCA
    ISP_NVRAM_PCA_BIN_T lut_lo3[PCA_BIN_NUM];
    ISP_NVRAM_PCA_BIN_T lut_md3[PCA_BIN_NUM];
    ISP_NVRAM_PCA_BIN_T lut_hi3[PCA_BIN_NUM];
} ISP_NVRAM_PCA_LUTS_T;

typedef struct {
        ISP_NVRAM_PCA_BIN_T   lut[PCA_BIN_NUM];

} ISP_NVRAM_PCA_LUT_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// EE (the same as 89)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CAM_SEEE_SRK_CTRL CAM+AA0H
typedef struct {
        FIELD rsv_0                     : 28;
        FIELD RESERVED                  : 4;
} ISP_CAM_SEEE_SRK_CTRL_T;

typedef union {
    enum { MASK     = 0x00000000 };
    typedef ISP_CAM_SEEE_SRK_CTRL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_SRK_CTRL_T;

// CAM_SEEE_CLIP_CTRL CAM+AA4H
typedef struct {
        FIELD SEEE_OVRSH_CLIP_EN        : 1;
        FIELD SEEE_OVRSH_CLIP_STR       : 3;
        FIELD SEEE_DOT_REDUC_EN         : 1;
        FIELD SEEE_DOT_WF               : 3;
        FIELD SEEE_DOT_TH               : 8;
        FIELD SEEE_RESP_CLIP            : 8;
        FIELD rsv_24                    : 8;
} ISP_CAM_SEEE_CLIP_CTRL_T;

typedef union {
    enum { MASK     = 0x00FFFFFF };
    typedef ISP_CAM_SEEE_CLIP_CTRL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_CLIP_CTRL_T;

// CAM_SEEE_FLT_CTRL_1 CAM+AA8H
typedef struct {
        FIELD SEEE_FLT_TH               : 8;
        FIELD SEEE_FLT_AMP              : 3;
        FIELD rsv_11                    : 21;
} ISP_CAM_SEEE_FLT_CTRL_1_T;

typedef union {
    enum { MASK     = 0x000007FF };
    typedef ISP_CAM_SEEE_FLT_CTRL_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_FLT_CTRL_1_T;

// CAM_SEEE_FLT_CTRL_2 CAM+AACH
typedef struct {
        FIELD SEEE_FLT_G1               : 8;
        FIELD SEEE_FLT_G2               : 6;
        FIELD SEEE_FLT_G3               : 5;
        FIELD SEEE_FLT_G4               : 4;
        FIELD SEEE_FLT_G5               : 4;
        FIELD SEEE_FLT_G6               : 3;
        FIELD rsv_30                    : 2;
} ISP_CAM_SEEE_FLT_CTRL_2_T;

typedef union {
    enum { MASK     = 0x3FFFFFFF };
    typedef ISP_CAM_SEEE_FLT_CTRL_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_FLT_CTRL_2_T;

// CAM_SEEE_GLUT_CTRL_01 CAM+AB0H
typedef struct {
        FIELD SEEE_GLUT_X1              : 8;
        FIELD SEEE_GLUT_S1              : 8;
        FIELD SEEE_GLUT_Y1              : 10;
        FIELD rsv_26                    : 6;
} ISP_CAM_SEEE_GLUT_CTRL_01_T;

typedef union {
    enum { MASK     = 0x03FFFFFF };
    typedef ISP_CAM_SEEE_GLUT_CTRL_01_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_GLUT_CTRL_01_T;

// CAM_SEEE_GLUT_CTRL_02 CAM+AB4H
typedef struct {
        FIELD SEEE_GLUT_X2              : 8;
        FIELD SEEE_GLUT_S2              : 8;
        FIELD SEEE_GLUT_Y2              : 10;
        FIELD rsv_26                    : 6;
} ISP_CAM_SEEE_GLUT_CTRL_02_T;

typedef union {
    enum { MASK     = 0x03FFFFFF };
    typedef ISP_CAM_SEEE_GLUT_CTRL_02_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_GLUT_CTRL_02_T;

// CAM_SEEE_GLUT_CTRL_03 CAM+AB8H
typedef struct {
        FIELD SEEE_GLUT_X3              : 8;
        FIELD SEEE_GLUT_S3              : 8;
        FIELD SEEE_GLUT_Y3              : 10;
        FIELD rsv_26                    : 6;
} ISP_CAM_SEEE_GLUT_CTRL_03_T;

typedef union {
    enum { MASK     = 0x03FFFFFF };
    typedef ISP_CAM_SEEE_GLUT_CTRL_03_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_GLUT_CTRL_03_T;

// CAM_SEEE_GLUT_CTRL_04 CAM+ABCH
typedef struct {
        FIELD SEEE_GLUT_X4              : 8;
        FIELD SEEE_GLUT_S4              : 8;
        FIELD SEEE_GLUT_Y4              : 10;
        FIELD rsv_26                    : 6;
} ISP_CAM_SEEE_GLUT_CTRL_04_T;

typedef union {
    enum { MASK     = 0x03FFFFFF };
    typedef ISP_CAM_SEEE_GLUT_CTRL_04_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_GLUT_CTRL_04_T;

// CAM_SEEE_GLUT_CTRL_05 CAM+AC0H
typedef struct {
        FIELD rsv_0                     : 8;
        FIELD SEEE_GLUT_S5              : 8;
        FIELD rsv_16                    : 16;
} ISP_CAM_SEEE_GLUT_CTRL_05_T;

typedef union {
    enum { MASK     = 0x0000FF00 };
    typedef ISP_CAM_SEEE_GLUT_CTRL_05_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_GLUT_CTRL_05_T;

// CAM_SEEE_GLUT_CTRL_06 CAM+AC4H
typedef struct {
        FIELD SEEE_GLUT_TH_OVR          : 8;
        FIELD SEEE_GLUT_TH_UND          : 8;
        FIELD SEEE_GLUT_TH_MIN          : 8;
        FIELD rsv_24                    : 8;
} ISP_CAM_SEEE_GLUT_CTRL_06_T;

typedef union {
    enum { MASK     = 0x00FFFFFF };
    typedef ISP_CAM_SEEE_GLUT_CTRL_06_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_GLUT_CTRL_06_T;

// CAM_SEEE_EDTR_CTRL CAM+AC8H
typedef struct {
        FIELD SEEE_EDTR_DIAG_AMP        : 3;
        FIELD SEEE_EDTR_AMP             : 6;
        FIELD SEEE_EDTR_LV              : 3;
        FIELD SEEE_EDTR_FLT_MODE        : 2;
        FIELD SEEE_EDTR_FLT_2_EN        : 1;
        FIELD rsv_15                    : 17;
} ISP_CAM_SEEE_EDTR_CTRL_T;

typedef union {
    enum { MASK     = 0x00007FFF };
    typedef ISP_CAM_SEEE_EDTR_CTRL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_EDTR_CTRL_T;

// CAM_SEEE_GLUT_CTRL_07 CAM+AECH
typedef struct {
        FIELD SEEE_GLUT_X1_1            : 8;
        FIELD SEEE_GLUT_S1_1            : 8;
        FIELD SEEE_GLUT_S2_1            : 8;
        FIELD SEEE_GLUT_LLINK_EN        : 1;
        FIELD rsv_25                    : 7;
} ISP_CAM_SEEE_GLUT_CTRL_07_T;

typedef union {
    enum { MASK     = 0x01FFFFFF };
    typedef ISP_CAM_SEEE_GLUT_CTRL_07_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_GLUT_CTRL_07_T;

// CAM_SEEE_GLUT_CTRL_08 CAM+AF0H
typedef struct {
        FIELD SEEE_GLUT_X1_2            : 8;
        FIELD SEEE_GLUT_S1_2            : 8;
        FIELD SEEE_GLUT_S2_2            : 8;
        FIELD rsv_24                    : 8;
} ISP_CAM_SEEE_GLUT_CTRL_08_T;

typedef union {
    enum { MASK     = 0x00FFFFFF };
    typedef ISP_CAM_SEEE_GLUT_CTRL_08_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_GLUT_CTRL_08_T;

// CAM_SEEE_GLUT_CTRL_09 CAM+AF4H
typedef struct {
        FIELD SEEE_GLUT_X1_3            : 8;
        FIELD SEEE_GLUT_S1_3            : 8;
        FIELD SEEE_GLUT_S2_3            : 8;
        FIELD rsv_24                    : 8;
} ISP_CAM_SEEE_GLUT_CTRL_09_T;

typedef union {
    enum { MASK     = 0x00FFFFFF };
    typedef ISP_CAM_SEEE_GLUT_CTRL_09_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_GLUT_CTRL_09_T;

// CAM_SEEE_GLUT_CTRL_10 CAM+AF8H
typedef struct {
        FIELD SEEE_GLUT_TH_OVR_1        : 8;
        FIELD SEEE_GLUT_TH_UND_1        : 8;
        FIELD SEEE_GLUT_TH_OVR_2        : 8;
        FIELD SEEE_GLUT_TH_UND_2        : 8;
} ISP_CAM_SEEE_GLUT_CTRL_10_T;

typedef union {
    enum { MASK     = 0xFFFFFFFF };
    typedef ISP_CAM_SEEE_GLUT_CTRL_10_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_GLUT_CTRL_10_T;

// CAM_SEEE_GLUT_CTRL_11 CAM+AFCH
typedef struct {
        FIELD SEEE_GLUT_TH_OVR_3        : 8;
        FIELD SEEE_GLUT_TH_UND_3        : 8;
        FIELD rsv_16                    : 16;
} ISP_CAM_SEEE_GLUT_CTRL_11_T;

typedef union {
    enum { MASK     = 0x0000FFFF };
    typedef ISP_CAM_SEEE_GLUT_CTRL_11_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_GLUT_CTRL_11_T;

//
typedef union{
    enum { COUNT = 16 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_SEEE_SRK_CTRL_T      srk_ctrl;
        ISP_NVRAM_SEEE_CLIP_CTRL_T     clip_ctrl;
        ISP_NVRAM_SEEE_FLT_CTRL_1_T    flt_ctrl_1;
        ISP_NVRAM_SEEE_FLT_CTRL_2_T    flt_ctrl_2;
        ISP_NVRAM_SEEE_GLUT_CTRL_01_T  glut_ctrl_01;
        ISP_NVRAM_SEEE_GLUT_CTRL_02_T  glut_ctrl_02;
        ISP_NVRAM_SEEE_GLUT_CTRL_03_T  glut_ctrl_03;
        ISP_NVRAM_SEEE_GLUT_CTRL_04_T  glut_ctrl_04;
        ISP_NVRAM_SEEE_GLUT_CTRL_05_T  glut_ctrl_05;
        ISP_NVRAM_SEEE_GLUT_CTRL_06_T  glut_ctrl_06;
        ISP_NVRAM_SEEE_EDTR_CTRL_T     edtr_ctrl;
        ISP_NVRAM_SEEE_GLUT_CTRL_07_T  glut_ctrl_07;
        ISP_NVRAM_SEEE_GLUT_CTRL_08_T  glut_ctrl_08;
        ISP_NVRAM_SEEE_GLUT_CTRL_09_T  glut_ctrl_09;
        ISP_NVRAM_SEEE_GLUT_CTRL_10_T  glut_ctrl_10;
        ISP_NVRAM_SEEE_GLUT_CTRL_11_T  glut_ctrl_11;
    };
} ISP_NVRAM_EE_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SE (same as 89)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CAM_SEEE_OUT_EDGE_CTRL CAM+ACCH
typedef struct {
        FIELD SEEE_OUT_EDGE_SEL         : 2;
        FIELD rsv_2                     : 30;
} ISP_CAM_SEEE_OUT_EDGE_CTRL_T;

typedef union {
    enum { MASK     = 0x00000003 };
    typedef ISP_CAM_SEEE_OUT_EDGE_CTRL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_OUT_EDGE_CTRL_T;

// CAM_SEEE_SE_Y_CTRL CAM+AD0H
typedef struct {
        FIELD SEEE_SE_CONST_Y_EN        : 1;
        FIELD SEEE_SE_CONST_Y_VAL       : 8;
        FIELD rsv_9                     : 7;
        FIELD SEEE_SE_YOUT_QBIT         : 3;
        FIELD rsv_19                    : 1;
        FIELD SEEE_SE_COUT_QBIT         : 3;
        FIELD rsv_23                    : 9;
} ISP_CAM_SEEE_SE_Y_CTRL_T;

typedef union {
    enum { MASK     = 0x007701FF };
    typedef ISP_CAM_SEEE_SE_Y_CTRL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_SE_Y_CTRL_T;

// CAM_SEEE_SE_EDGE_CTRL_1 CAM+AD4H
typedef struct {
        FIELD SEEE_SE_HORI_EDGE_SEL     : 1;
        FIELD SEEE_SE_HORI_EDGE_GAIN_A  : 4;
        FIELD SEEE_SE_HORI_EDGE_GAIN_B  : 5;
        FIELD SEEE_SE_HORI_EDGE_GAIN_C  : 5;
        FIELD rsv_15                    : 1;
        FIELD SEEE_SE_VERT_EDGE_SEL     : 1;
        FIELD SEEE_SE_VERT_EDGE_GAIN_A  : 4;
        FIELD SEEE_SE_VERT_EDGE_GAIN_B  : 5;
        FIELD SEEE_SE_VERT_EDGE_GAIN_C  : 5;
        FIELD rsv_31                    : 1;
} ISP_CAM_SEEE_SE_EDGE_CTRL_1_T;

typedef union {
    enum { MASK     = 0x7FFF7FFF };
    typedef ISP_CAM_SEEE_SE_EDGE_CTRL_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_SE_EDGE_CTRL_1_T;

// CAM_SEEE_SE_EDGE_CTRL_2 CAM+AD8H
typedef struct {
        FIELD SEEE_SE_EDGE_DET_GAIN     : 5;
        FIELD SEEE_SE_BOSS_IN_SEL       : 1;
        FIELD SEEE_SE_BOSS_GAIN_OFF     : 1;
        FIELD rsv_7                     : 25;
} ISP_CAM_SEEE_SE_EDGE_CTRL_2_T;

typedef union {
    enum { MASK     = 0x0000007F };
    typedef ISP_CAM_SEEE_SE_EDGE_CTRL_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_SE_EDGE_CTRL_2_T;

// CAM_SEEE_SE_EDGE_CTRL_3 CAM+ADCH
typedef struct {
        FIELD SEEE_SE_CONVT_FORCE_EN    : 1;
        FIELD SEEE_SE_CONVT_CORE_TH     : 7;
        FIELD SEEE_SE_CONVT_E_TH        : 8;
        FIELD SEEE_SE_CONVT_SLOPE_SEL   : 1;
        FIELD SEEE_SE_OIL_EN            : 1;
        FIELD rsv_18                    : 14;
} ISP_CAM_SEEE_SE_EDGE_CTRL_3_T;

typedef union {
    enum { MASK     = 0x0003FFFF };
    typedef ISP_CAM_SEEE_SE_EDGE_CTRL_3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_SE_EDGE_CTRL_3_T;

// CAM_SEEE_SE_SPECL_CTRL CAM+AE0H
typedef struct {
        FIELD SEEE_SE_SPECL_HALF_MODE   : 2;
        FIELD SEEE_SE_SPECL_ABS         : 1;
        FIELD SEEE_SE_SPECL_INV         : 1;
        FIELD SEEE_SE_SPECL_GAIN        : 2;
        FIELD SEEE_SE_KNEE_SEL          : 2;
        FIELD rsv_8                     : 24;
} ISP_CAM_SEEE_SE_SPECL_CTRL_T;

typedef union {
    enum { MASK     = 0x000000FF };
    typedef ISP_CAM_SEEE_SE_SPECL_CTRL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_SE_SPECL_CTRL_T;

// CAM_SEEE_SE_CORE_CTRL_1 CAM+AE4H
typedef struct {
        FIELD SEEE_SE_CORE_HORI_X0      : 7;
        FIELD SEEE_SE_CORE_HORI_SUP     : 2;
        FIELD SEEE_SE_CORE_HORI_X2      : 8;
        FIELD SEEE_SE_CORE_HORI_SDN     : 2;
        FIELD SEEE_SE_CORE_HORI_Y5      : 6;
        FIELD rsv_25                    : 7;
} ISP_CAM_SEEE_SE_CORE_CTRL_1_T;

typedef union {
    enum { MASK     = 0x01FFFFFF };
    typedef ISP_CAM_SEEE_SE_CORE_CTRL_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_SE_CORE_CTRL_1_T;

// CAM_SEEE_SE_CORE_CTRL_2 CAM+AE8H
typedef struct {
        FIELD SEEE_SE_CORE_VERT_X0      : 7;
        FIELD SEEE_SE_CORE_VERT_SUP     : 2;
        FIELD SEEE_SE_CORE_VERT_X2      : 8;
        FIELD SEEE_SE_CORE_VERT_SDN     : 2;
        FIELD SEEE_SE_CORE_VERT_Y5      : 6;
        FIELD rsv_25                    : 7;
} ISP_CAM_SEEE_SE_CORE_CTRL_2_T;

typedef union {
    enum { MASK     = 0x01FFFFFF };
    typedef ISP_CAM_SEEE_SE_CORE_CTRL_2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SEEE_SE_CORE_CTRL_2_T;

//
typedef union{
    enum { COUNT = 8 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_SEEE_OUT_EDGE_CTRL_T   out_edge_ctrl;
        ISP_NVRAM_SEEE_SE_Y_CTRL_T       y_ctrl;
        ISP_NVRAM_SEEE_SE_EDGE_CTRL_1_T  edge_ctrl_1;
        ISP_NVRAM_SEEE_SE_EDGE_CTRL_2_T  edge_ctrl_2;
        ISP_NVRAM_SEEE_SE_EDGE_CTRL_3_T  edge_ctrl_3;
        ISP_NVRAM_SEEE_SE_SPECL_CTRL_T   special_ctrl;
        ISP_NVRAM_SEEE_SE_CORE_CTRL_1_T  core_ctrl_1;
        ISP_NVRAM_SEEE_SE_CORE_CTRL_2_T  core_ctrl_2;
    };
} ISP_NVRAM_SE_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// NR3D (Not the same as 89)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CAM_NR3D_BLEND CAM+CC0H
typedef struct {
        FIELD NR3D_GAIN                 : 5;
        FIELD rsv_5                     : 3;
        FIELD NR3D_RND_Y                : 5;
        FIELD rsv_13                    : 3;
        FIELD NR3D_RND_U                : 5;
        FIELD rsv_21                    : 3;
        FIELD NR3D_RND_V                : 5;
        FIELD rsv_29                    : 3;
} ISP_CAM_NR3D_BLEND_T;

typedef union {
    enum { MASK     = 0x1F1F1F1F };
    typedef ISP_CAM_NR3D_BLEND_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_BLEND_T;

// CAM_NR3D_FBCNT_OFF CAM+CC4H
#if 0
typedef struct {
        FIELD rsv_0                     : 1;
        FIELD NR3D_FB_CNT_XOFF          : 13;
        FIELD rsv_14                    : 2;
        FIELD NR3D_FB_CNT_YOFF          : 14;
        FIELD rsv_30                    : 2;
} ISP_CAM_NR3D_FBCNT_OFF_T;

typedef union {
    enum { MASK     = 0x3FFF3FFE };
    typedef ISP_CAM_NR3D_FBCNT_OFF_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_FBCNT_OFF_T;
#endif

// CAM_NR3D_FBCNT_SIZ CAM+CC8H
#if 0
typedef struct {
        FIELD rsv_0                     : 1;
        FIELD NR3D_FB_CNT_XSIZ          : 13;
        FIELD rsv_14                    : 2;
        FIELD NR3D_FB_CNT_YSIZ          : 14;
        FIELD rsv_30                    : 2;
} ISP_CAM_NR3D_FBCNT_SIZ_T;

typedef union {
    enum { MASK     = 0x3FFF3FFE };
    typedef ISP_CAM_NR3D_FBCNT_SIZ_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_FBCNT_SIZ_T;
#endif

// CAM_NR3D_FB_COUNT CAM+CCCH
#if 0
typedef struct {
        FIELD NR3D_FB_CNT               : 28;
        FIELD rsv_28                    : 4;
} ISP_CAM_NR3D_FB_COUNT_T;

typedef union {
    enum { MASK     = 0x0FFFFFFF };
    typedef ISP_CAM_NR3D_FB_COUNT_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_FB_COUNT_T;
#endif

// CAM_NR3D_LMT_CPX CAM+CD0H
typedef struct {
        FIELD NR3D_LMT_CPX1             : 8;
        FIELD NR3D_LMT_CPX2             : 8;
        FIELD NR3D_LMT_CPX3             : 8;
        FIELD NR3D_LMT_CPX4             : 8;
} ISP_CAM_NR3D_LMT_CPX_T;

typedef union {
    enum { MASK     = 0xFFFFFFFF };
    typedef ISP_CAM_NR3D_LMT_CPX_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_LMT_CPX_T;

// CAM_NR3D_LMT_Y_CON1 CAM+CD4H
typedef struct {
        FIELD NR3D_LMT_Y0               : 5;
        FIELD rsv_5                     : 3;
        FIELD NR3D_LMT_Y0_TH            : 5;
        FIELD rsv_13                    : 3;
        FIELD NR3D_LMT_Y1               : 5;
        FIELD rsv_21                    : 3;
        FIELD NR3D_LMT_Y1_TH            : 5;
        FIELD rsv_29                    : 3;
} ISP_CAM_NR3D_LMT_Y_CON1_T;

typedef union {
    enum { MASK     = 0x1F1F1F1F };
    typedef ISP_CAM_NR3D_LMT_Y_CON1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_LMT_Y_CON1_T;

// CAM_NR3D_LMT_Y_CON2 CAM+CD8H
typedef struct {
        FIELD NR3D_LMT_Y2               : 5;
        FIELD rsv_5                     : 3;
        FIELD NR3D_LMT_Y2_TH            : 5;
        FIELD rsv_13                    : 3;
        FIELD NR3D_LMT_Y3               : 5;
        FIELD rsv_21                    : 3;
        FIELD NR3D_LMT_Y3_TH            : 5;
        FIELD rsv_29                    : 3;
} ISP_CAM_NR3D_LMT_Y_CON2_T;

typedef union {
    enum { MASK     = 0x1F1F1F1F };
    typedef ISP_CAM_NR3D_LMT_Y_CON2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_LMT_Y_CON2_T;

// CAM_NR3D_LMT_Y_CON3 CAM+CDCH
typedef struct {
        FIELD NR3D_LMT_Y4               : 5;
        FIELD rsv_5                     : 3;
        FIELD NR3D_LMT_Y4_TH            : 5;
        FIELD rsv_13                    : 19;
} ISP_CAM_NR3D_LMT_Y_CON3_T;

typedef union {
    enum { MASK     = 0x00001F1F };
    typedef ISP_CAM_NR3D_LMT_Y_CON3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_LMT_Y_CON3_T;

// CAM_NR3D_LMT_U_CON1 CAM+CE0H
typedef struct {
        FIELD NR3D_LMT_U0               : 5;
        FIELD rsv_5                     : 3;
        FIELD NR3D_LMT_U0_TH            : 5;
        FIELD rsv_13                    : 3;
        FIELD NR3D_LMT_U1               : 5;
        FIELD rsv_21                    : 3;
        FIELD NR3D_LMT_U1_TH            : 5;
        FIELD rsv_29                    : 3;
} ISP_CAM_NR3D_LMT_U_CON1_T;

typedef union {
    enum { MASK     = 0x1F1F1F1F };
    typedef ISP_CAM_NR3D_LMT_U_CON1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_LMT_U_CON1_T;

// CAM_NR3D_LMT_U_CON2 CAM+CE4H
typedef struct {
        FIELD NR3D_LMT_U2               : 5;
        FIELD rsv_5                     : 3;
        FIELD NR3D_LMT_U2_TH            : 5;
        FIELD rsv_13                    : 3;
        FIELD NR3D_LMT_U3               : 5;
        FIELD rsv_21                    : 3;
        FIELD NR3D_LMT_U3_TH            : 5;
        FIELD rsv_29                    : 3;
} ISP_CAM_NR3D_LMT_U_CON2_T;

typedef union {
    enum { MASK     = 0x1F1F1F1F };
    typedef ISP_CAM_NR3D_LMT_U_CON2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_LMT_U_CON2_T;

// CAM_NR3D_LMT_U_CON3 CAM+CE8H
typedef struct {
        FIELD NR3D_LMT_U4               : 5;
        FIELD rsv_5                     : 3;
        FIELD NR3D_LMT_U4_TH            : 5;
        FIELD rsv_13                    : 19;
} ISP_CAM_NR3D_LMT_U_CON3_T;

typedef union {
    enum { MASK     = 0x00001F1F };
    typedef ISP_CAM_NR3D_LMT_U_CON3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_LMT_U_CON3_T;

// CAM_NR3D_LMT_V_CON1 CAM+CECH
typedef struct {
        FIELD NR3D_LMT_V0               : 5;
        FIELD rsv_5                     : 3;
        FIELD NR3D_LMT_V0_TH            : 5;
        FIELD rsv_13                    : 3;
        FIELD NR3D_LMT_V1               : 5;
        FIELD rsv_21                    : 3;
        FIELD NR3D_LMT_V1_TH            : 5;
        FIELD rsv_29                    : 3;
} ISP_CAM_NR3D_LMT_V_CON1_T;

typedef union {
    enum { MASK     = 0x1F1F1F1F };
    typedef ISP_CAM_NR3D_LMT_V_CON1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_LMT_V_CON1_T;

// CAM_NR3D_LMT_V_CON2 CAM+CF0H
typedef struct {
        FIELD NR3D_LMT_V2               : 5;
        FIELD rsv_5                     : 3;
        FIELD NR3D_LMT_V2_TH            : 5;
        FIELD rsv_13                    : 3;
        FIELD NR3D_LMT_V3               : 5;
        FIELD rsv_21                    : 3;
        FIELD NR3D_LMT_V3_TH            : 5;
        FIELD rsv_29                    : 3;
} ISP_CAM_NR3D_LMT_V_CON2_T;

typedef union {
    enum { MASK     = 0x1F1F1F1F };
    typedef ISP_CAM_NR3D_LMT_V_CON2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_LMT_V_CON2_T;

// CAM_NR3D_LMT_V_CON3 CAM+CF4H
typedef struct {
        FIELD NR3D_LMT_V4               : 5;
        FIELD rsv_5                     : 3;
        FIELD NR3D_LMT_V4_TH            : 5;
        FIELD rsv_13                    : 19;
} ISP_CAM_NR3D_LMT_V_CON3_T;

typedef union {
    enum { MASK     = 0x00001F1F };
    typedef ISP_CAM_NR3D_LMT_V_CON3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_LMT_V_CON3_T;

// CAM_NR3D_CTRL CAM+CF8H
#if 0
typedef struct {
        FIELD NR3D_LMT_OUT_CNT_TH       : 2;
        FIELD rsv_2                     : 2;
        FIELD NR3D_ON_EN                : 1;
        FIELD rsv_5                     : 27;
} ISP_CAM_NR3D_CTRL_T;

typedef union {
    enum { MASK     = 0x00000013 };
    typedef ISP_CAM_NR3D_CTRL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_CTRL_T;
#endif

// CAM_NR3D_ON_OFF CAM+CFCH
#if 0
typedef struct {
        FIELD NR3D_ON_OFST_X            : 14;
        FIELD rsv_14                    : 2;
        FIELD NR3D_ON_OFST_Y            : 14;
        FIELD rsv_30                    : 2;
} ISP_CAM_NR3D_ON_OFF_T;

typedef union {
    enum { MASK     = 0x3FFF3FFF };
    typedef ISP_CAM_NR3D_ON_OFF_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_ON_OFF_T;
#endif

// CAM_NR3D_ON_SIZ CAM+D00H
#if 0
typedef struct {
        FIELD NR3D_ON_WD                : 14;
        FIELD rsv_14                    : 2;
        FIELD NR3D_ON_HT                : 14;
        FIELD rsv_30                    : 2;
} ISP_CAM_NR3D_ON_SIZ_T;

typedef union {
    enum { MASK     = 0x3FFF3FFF };
    typedef ISP_CAM_NR3D_ON_SIZ_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NR3D_ON_SIZ_T;
#endif

//
typedef union{
    enum { COUNT = 11 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_NR3D_BLEND_T      blend;
        //ISP_NVRAM_NR3D_FBCNT_OFF_T  fbcnt_off;
        //ISP_NVRAM_NR3D_FBCNT_SIZ_T  fbcnt_siz;
        //ISP_NVRAM_NR3D_FB_COUNT_T   fb_count;
        ISP_NVRAM_NR3D_LMT_CPX_T    lmt_cpx;
        ISP_NVRAM_NR3D_LMT_Y_CON1_T lmt_y_con1;
        ISP_NVRAM_NR3D_LMT_Y_CON2_T lmt_y_con2;
        ISP_NVRAM_NR3D_LMT_Y_CON3_T lmt_y_con3;
        ISP_NVRAM_NR3D_LMT_U_CON1_T lmt_u_con1;
        ISP_NVRAM_NR3D_LMT_U_CON2_T lmt_u_con2;
        ISP_NVRAM_NR3D_LMT_U_CON3_T lmt_u_con3;
        ISP_NVRAM_NR3D_LMT_V_CON1_T lmt_v_con1;
        ISP_NVRAM_NR3D_LMT_V_CON2_T lmt_v_con2;
        ISP_NVRAM_NR3D_LMT_V_CON3_T lmt_v_con3;
        //ISP_NVRAM_NR3D_CTRL_T       ctrl;
        //ISP_NVRAM_NR3D_ON_OFF_T     on_off;
        //ISP_NVRAM_NR3D_ON_SIZ_T     on_siz;
    };
} ISP_NVRAM_NR3D_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// MFB
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CAM_MFB_LL_CON2 CAM+968H
typedef struct {
        FIELD BLD_LL_MAX_WT             : 3;
        FIELD rsv_3                     : 5;
        FIELD BLD_LL_DT1                : 8;
        FIELD BLD_LL_TH1                : 8;
        FIELD BLD_LL_TH2                : 8;
} ISP_CAM_MFB_LL_CON2_T;

typedef union {
    enum { MASK     = 0xFFFFFF07 };
    typedef ISP_CAM_MFB_LL_CON2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MFB_LL_CON2_T;

// CAM_MFB_LL_CON3 CAM+96CH
typedef struct {
        FIELD BLD_LL_BRZ_EN             : 1;
        FIELD rsv_1                     : 3;
        FIELD BLD_LL_DB_EN              : 1;
        FIELD rsv_5                     : 3;
        FIELD BLD_LL_DB_XDIST           : 4;
        FIELD BLD_LL_DB_YDIST           : 4;
        FIELD BLD_LL_TH_E               : 8;
        FIELD rsv_24                    : 8;
} ISP_CAM_MFB_LL_CON3_T;

typedef union {
    enum { MASK     = 0x00FFFF11 };
    typedef ISP_CAM_MFB_LL_CON3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MFB_LL_CON3_T;

// CAM_MFB_LL_CON4 CAM+970H
typedef struct {
        FIELD BLD_LL_FLT_MODE           : 2;
        FIELD rsv_2                     : 2;
        FIELD BLD_LL_FLT_WT_MODE1       : 3;
        FIELD rsv_7                     : 1;
        FIELD BLD_LL_FLT_WT_MODE2       : 2;
        FIELD rsv_10                    : 6;
        FIELD BLD_LL_CLIP_TH1           : 8;
        FIELD BLD_LL_CLIP_TH2           : 8;
} ISP_CAM_MFB_LL_CON4_T;

typedef union {
    enum { MASK     = 0xFFFF0373 };
    typedef ISP_CAM_MFB_LL_CON4_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MFB_LL_CON4_T;

// CAM_MFB_LL_CON5 CAM+974H
typedef struct {
        FIELD BLD_LL_MX_M0              : 8;
        FIELD BLD_LL_MX_M1              : 8;
        FIELD BLD_LL_MX_DT              : 8;
        FIELD rsv_24                    : 8;
} ISP_CAM_MFB_LL_CON5_T;

typedef union {
    enum { MASK     = 0x00FFFFFF };
    typedef ISP_CAM_MFB_LL_CON5_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MFB_LL_CON5_T;

// CAM_MFB_LL_CON6 CAM+978H
typedef struct {
        FIELD BLD_LL_MX_B0              : 8;
        FIELD BLD_LL_MX_B1              : 8;
        FIELD rsv_16                    : 16;
} ISP_CAM_MFB_LL_CON6_T;

typedef union {
    enum { MASK     = 0x0000FFFF };
    typedef ISP_CAM_MFB_LL_CON6_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MFB_LL_CON6_T;

//
typedef union{
    enum { COUNT = 5 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_MFB_LL_CON2_T  ll_con2;
        ISP_NVRAM_MFB_LL_CON3_T  ll_con3;
        ISP_NVRAM_MFB_LL_CON4_T  ll_con4;
        ISP_NVRAM_MFB_LL_CON5_T  ll_con5;
        ISP_NVRAM_MFB_LL_CON6_T  ll_con6;
    };
} ISP_NVRAM_MFB_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Mixer3
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CAM_MIX3_CTRL_0 CAM+CB0H
typedef struct {
        FIELD MIX3_WT_SEL               : 1;
        FIELD rsv_1                     : 7;
        FIELD MIX3_B0                   : 8;
        FIELD MIX3_B1                   : 8;
        FIELD MIX3_DT                   : 8;
} ISP_CAM_MIX3_CTRL_0_T;

typedef union {
    enum { MASK     = 0xFFFFFF01 };
    typedef ISP_CAM_MIX3_CTRL_0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MIX3_CTRL_0_T;

// CAM_MIX3_CTRL_1 CAM+CB4H
typedef struct {
        FIELD MIX3_M0                   : 8;
        FIELD rsv_8                     : 8;
        FIELD MIX3_M1                   : 8;
        FIELD rsv_24                    : 8;
} ISP_CAM_MIX3_CTRL_1_T;

typedef union {
    enum { MASK     = 0x00FF00FF };
    typedef ISP_CAM_MIX3_CTRL_1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MIX3_CTRL_1_T;

// CAM_MIX3_SPARE CAM+CB8H
typedef struct {
        FIELD MIX3_SPARE                : 32;
} ISP_CAM_MIX3_SPARE_T;

typedef union {
    enum { MASK     = 0x00000000 };
    typedef ISP_CAM_MIX3_SPARE_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_MIX3_SPARE_T;

typedef union {
    enum { COUNT = 3 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_MIX3_CTRL_0_T  ctrl_0;
        ISP_NVRAM_MIX3_CTRL_1_T  ctrl_1;
        ISP_NVRAM_MIX3_SPARE_T   spare;
    };
} ISP_NVRAM_MIXER3_T;
/*
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// LCE
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CAM_LCE_QUA CAM+9C8H
typedef struct {
        FIELD LCE_PA                    : 7;
        FIELD LCE_PB                    : 9;
        FIELD LCE_BA                    : 8;
        FIELD rsv_24                    : 8;
} ISP_CAM_LCE_QUA_T;

typedef union {
    enum { MASK     = 0xFFFFFFFF};
    typedef ISP_CAM_LCE_QUA_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LCE_QUA_T;

typedef union {
    enum { COUNT = 1 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_LCE_QUA_T  qua;
    };
} ISP_NVRAM_LCE_T;
*/
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
// SL2
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CAM_SL2_CEN CAM+0F40H
typedef struct {
    FIELD SL2_CENTR_X               : 13;
    FIELD rsv_13                    : 3;
    FIELD SL2_CENTR_Y               : 13;
    FIELD rsv_29                    : 3;
} ISP_CAM_SL2_CEN_T;

typedef union {
    enum { MASK     = 0x1FFF1FFF };
    enum { DEFAULT  = 0x00000000 };
    typedef ISP_CAM_SL2_CEN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SL2_CEN_T;

// CAM_SL2_MAX0_RR CAM+0F44H
typedef struct {
    FIELD SL2_RR_0                  : 26;
    FIELD rsv_26                    : 6;
} ISP_CAM_SL2_MAX0_RR_T;

typedef union {
    enum { MASK     = 0x03FFFFFF };
    enum { DEFAULT  = 0x00000000 };
    typedef ISP_CAM_SL2_MAX0_RR_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SL2_MAX0_RR_T;

// CAM_SL2_MAX1_RR CAM+0F48H
typedef struct {
    FIELD SL2_RR_1                  : 26;
    FIELD rsv_26                    : 6;
} ISP_CAM_SL2_MAX1_RR_T;

typedef union {
    enum { MASK     = 0x03FFFFFF };
    enum { DEFAULT  = 0x00000000 };
    typedef ISP_CAM_SL2_MAX1_RR_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SL2_MAX1_RR_T;

// CAM_SL2_MAX2_RR CAM+0F4CH
typedef struct {
    FIELD SL2_RR_2                  : 26;
    FIELD rsv_26                    : 6;
} ISP_CAM_SL2_MAX2_RR_T;

typedef union {
    enum { MASK     = 0x03FFFFFF };
    enum { DEFAULT  = 0x00000000 };
    typedef ISP_CAM_SL2_MAX2_RR_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_SL2_MAX2_RR_T;

//
typedef union {
    enum { COUNT = 4 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_SL2_CEN_T      cen;
        ISP_NVRAM_SL2_MAX0_RR_T  max0_rr;
        ISP_NVRAM_SL2_MAX1_RR_T  max1_rr;
        ISP_NVRAM_SL2_MAX2_RR_T  max2_rr;
    };
} ISP_NVRAM_SL2_T;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// NSL2A
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CAM_NSL2A_CEN CAM+0F40H
typedef struct {
  FIELD SL2_CENTR_X       : 14;
  FIELD rsv_14            : 2;
  FIELD SL2_CENTR_Y       : 14;
  FIELD rsv_30            : 2;
} ISP_CAM_NSL2A_CEN_T;

typedef union {
    typedef ISP_CAM_NSL2A_CEN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NSL2A_CEN_T;

//CAM_NSL2A_RR_CON0
typedef struct {
  FIELD SL2_R_0         : 14;
  FIELD rsv_14          : 2;
  FIELD SL2_R_1         : 14;
  FIELD rsv_30          : 2;
} ISP_CAM_NSL2A_RR_CON0_T;

typedef union {
    typedef ISP_CAM_NSL2A_RR_CON0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NSL2A_RR_CON0_T;

//CAM_NSL2A_RR_CON1
typedef struct {
  FIELD SL2_R_2           : 14;
  FIELD rsv_14            : 2;
  FIELD SL2_GAIN_0        : 8;
  FIELD SL2_GAIN_1        : 8;
} ISP_CAM_NSL2A_RR_CON1_T;

typedef union {
    typedef ISP_CAM_NSL2A_RR_CON1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NSL2A_RR_CON1_T;

// CAM_NSL2A_GAIN CAM+0F40H
typedef struct {
  FIELD SL2_GAIN_2          : 8;
  FIELD SL2_GAIN_3          : 8;
  FIELD SL2_GAIN_4          : 8;
  FIELD SL2_SET_ZERO        : 1;
  FIELD rsv_25              : 7;
} ISP_CAM_NSL2A_GAIN_T;

typedef union {
    typedef ISP_CAM_NSL2A_GAIN_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NSL2A_GAIN_T;

// CAM_NSL2A_RZ CAM+0F40H
typedef struct {
  FIELD SL2_HRZ_COMP       : 14;
  FIELD rsv_14             : 2;
  FIELD SL2_VRZ_COMP       : 14;
  FIELD rsv_30             : 2;
} ISP_CAM_NSL2A_RZ_T;

typedef union {
    typedef ISP_CAM_NSL2A_RZ_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NSL2A_RZ_T;

// CAM_NSL2A_XOFF CAM+0F40H
typedef struct {
  FIELD SL2_X_OFST        : 28;
  FIELD rsv_28            : 4;
} ISP_CAM_NSL2A_XOFF_T;

typedef union {
    typedef ISP_CAM_NSL2A_XOFF_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NSL2A_XOFF_T;

// CAM_NSL2A_YOFF CAM+0F40H
typedef struct {
  FIELD SL2_Y_OFST        : 28;
  FIELD rsv_28            : 4;
} ISP_CAM_NSL2A_YOFF_T;

typedef union {
    typedef ISP_CAM_NSL2A_YOFF_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NSL2A_YOFF_T;

//CAM_NSL2A_SLP_CON0
typedef struct {
    FIELD  SL2_SLP_1                             : 16;
    FIELD  SL2_SLP_2                             : 16;
} ISP_CAM_NSL2A_SLP_CON0_T;

typedef union {
    typedef ISP_CAM_NSL2A_SLP_CON0_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NSL2A_SLP_CON0_T;

//CAM_NSL2A_SLP_CON1
typedef struct {
    FIELD  SL2_SLP_3                             : 16;
    FIELD  SL2_SLP_4                             : 16;
} ISP_CAM_NSL2A_SLP_CON1_T;

typedef union {
    typedef ISP_CAM_NSL2A_SLP_CON1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NSL2A_SLP_CON1_T;


// CAM_NSL2A_SIZE CAM+0F40H
typedef struct {
	FIELD SL2_TPIPE_WD 				: 14;
	FIELD rsv_14					: 2;
	FIELD SL2_TPIPE_HT 				: 14;
	FIELD rsv_30					: 2;
} ISP_CAM_NSL2A_SIZE_T;

typedef union {
    typedef ISP_CAM_NSL2A_SIZE_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_NSL2A_SIZE_T;

//
typedef union {
    enum { COUNT = 4 };
    struct {
        ISP_NVRAM_NSL2A_CEN_T      cen;
		ISP_NVRAM_NSL2A_RR_CON0_T  rr_con0;
		ISP_NVRAM_NSL2A_RR_CON1_T  rr_con1;
		ISP_NVRAM_NSL2A_GAIN_T     gain;
    };
    MUINT32 set[COUNT];
} ISP_NVRAM_NSL2A_T;

typedef union {
    enum { COUNT = 10 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_NSL2A_CEN_T      cen;
		ISP_NVRAM_NSL2A_RR_CON0_T  rr_con0;
		ISP_NVRAM_NSL2A_RR_CON1_T  rr_con1;
		ISP_NVRAM_NSL2A_GAIN_T     gain;
        ISP_NVRAM_NSL2A_RZ_T       rz;
        ISP_NVRAM_NSL2A_XOFF_T     xoff;
        ISP_NVRAM_NSL2A_YOFF_T     yoff;
        ISP_NVRAM_NSL2A_SLP_CON0_T con0;
        ISP_NVRAM_NSL2A_SLP_CON1_T con1;
        ISP_NVRAM_NSL2A_SIZE_T     size;
    };
} ISP_NVRAM_NSL2A_GET_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// LSC
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


// CAM_LSC_CTL1 CAM+0530H
typedef struct {
    FIELD SDBLK_YOFST               : 6;
    FIELD rsv_6                     : 10;
    FIELD SDBLK_XOFST               : 6;
    FIELD rsv_22                    : 2;
    FIELD SD_COEFRD_MODE            : 1;
    FIELD rsv_25                    : 3;
    FIELD SD_ULTRA_MODE             : 1;
    FIELD LSC_PRC_MODE              : 1;
    FIELD LSC_SPARE2                : 1;
    FIELD rsv_31                    : 1;
} ISP_CAM_LSC_CTL1_T;

typedef union {
    enum { MASK     = 0x713F003F };
    enum { DEFAULT  = 0x00000000 };
    typedef ISP_CAM_LSC_CTL1_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LSC_CTL1_T;

// CAM_LSC_CTL2 CAM+0534H
typedef struct {
    FIELD LSC_SDBLK_WIDTH           : 12;
    FIELD LSC_SDBLK_XNUM            : 5;
    FIELD LSC_OFLN                  : 1;
    FIELD rsv_18                    : 14;
} ISP_CAM_LSC_CTL2_T;

typedef union {
    enum { MASK     = 0x0003FFFF };
    enum { DEFAULT  = 0x00006000 };
    typedef ISP_CAM_LSC_CTL2_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LSC_CTL2_T;

// CAM_LSC_CTL3 CAM+0538H
typedef struct {
    FIELD LSC_SDBLK_HEIGHT          : 12;
    FIELD LSC_SDBLK_YNUM            : 5;
    FIELD LSC_SPARE                 : 15;
} ISP_CAM_LSC_CTL3_T;

typedef union {
    enum { MASK     = 0xFFFFFFFF };
    enum { DEFAULT  = 0x00000000 };
    typedef ISP_CAM_LSC_CTL3_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LSC_CTL3_T;

// CAM_LSC_LBLOCK CAM+053CH
typedef struct {
    FIELD LSC_SDBLK_lHEIGHT         : 12;
    FIELD rsv_12                    : 4;
    FIELD LSC_SDBLK_lWIDTH          : 12;
    FIELD rsv_28                    : 4;
} ISP_CAM_LSC_LBLOCK_T;

typedef union {
    enum { MASK     = 0x0FFF0FFF };
    enum { DEFAULT  = 0x00000000 };
    typedef ISP_CAM_LSC_LBLOCK_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LSC_LBLOCK_T;

// CAM_LSC_RATIO CAM+0540H
typedef struct {
    FIELD LSC_RA3                   : 6;
    FIELD rsv_6                     : 2;
    FIELD LSC_RA2                   : 6;
    FIELD rsv_14                    : 2;
    FIELD LSC_RA1                   : 6;
    FIELD rsv_22                    : 2;
    FIELD LSC_RA0                   : 6;
    FIELD rsv_30                    : 2;
} ISP_CAM_LSC_RATIO_T;

typedef union {
    enum { MASK     = 0x3F3F3F3F };
    enum { DEFAULT  = 0x20202020 };
    typedef ISP_CAM_LSC_RATIO_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LSC_RATIO_T;

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
    enum { MASK     = 0x1FF7FDFF };
    enum { DEFAULT  = 0x00000000 };
    typedef ISP_CAM_LSC_GAIN_TH_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_LSC_GAIN_TH_T;

#if 0
typedef union {
    enum { COUNT = 10 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_LSCI_EN_T     lsci_en;
        ISP_NVRAM_LSCI_BA_T     baseaddr;
        ISP_NVRAM_LSCI_XSIZE_T  xsize;
        ISP_NVRAM_LSC_EN_T      lsc_en;
        ISP_NVRAM_LSC_CTL1_T    ctl1;
        ISP_NVRAM_LSC_CTL2_T    ctl2;
        ISP_NVRAM_LSC_CTL3_T    ctl3;
        ISP_NVRAM_LSC_LBLOCK_T  lblock;
        ISP_NVRAM_LSC_RATIO_T   ratio;
        ISP_NVRAM_LSC_GAIN_TH_T gain_th;
    };
} ISP_NVRAM_LSC_T;
#else
//
typedef union {
    enum { COUNT = 5 };
    MUINT32 set[COUNT];
    struct {
        ISP_NVRAM_LSC_CTL1_T    ctl1;
        ISP_NVRAM_LSC_CTL2_T    ctl2;
        ISP_NVRAM_LSC_CTL3_T    ctl3;
        ISP_NVRAM_LSC_LBLOCK_T  lblock;
        ISP_NVRAM_LSC_RATIO_T   ratio;
    };
} ISP_NVRAM_LSC_T;
#endif


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// AE Statistics
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CAM_AE_HST_CTL CAM+0650H
typedef struct {
        MUINT32 AE_HST0_EN                : 1;
        MUINT32 AE_HST1_EN                : 1;
        MUINT32 AE_HST2_EN                : 1;
        MUINT32 AE_HST3_EN                : 1;
        MUINT32 rsv_4                     : 28;
} ISP_CAM_AE_HST_CTL_T;

typedef union {
    enum { MASK     = 0x0000000F };
    enum { DEFAULT  = 0x00000000 };
    typedef ISP_CAM_AE_HST_CTL_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AE_HST_CTL_T;

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

// CAM_AE_HST_SET CAM+0680H
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
} ISP_CAM_AE_HST_SET_T;

typedef union {
    enum { MASK     = 0x77773333 };
    enum { DEFAULT  = 0x00000000 };
    typedef ISP_CAM_AE_HST_SET_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AE_HST_SET_T;

// CAM_AE_HST0_RNG CAM+0684H
typedef struct {
        MUINT32 AE_HST0_X_LOW             : 7;
        MUINT32 rsv_7                     : 1;
        MUINT32 AE_HST0_X_HI              : 7;
        MUINT32 rsv_15                    : 1;
        MUINT32 AE_HST0_Y_LOW             : 7;
        MUINT32 rsv_23                    : 1;
        MUINT32 AE_HST0_Y_HI              : 7;
        MUINT32 rsv_31                    : 1;
} ISP_CAM_AE_HST0_RNG_T;

typedef union {
    enum { MASK     = 0x7F7F7F7F };
    enum { DEFAULT  = 0x00000000 };
    typedef ISP_CAM_AE_HST0_RNG_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AE_HST0_RNG_T;

// CAM_AE_HST1_RNG CAM+0688H
typedef struct {
        MUINT32 AE_HST1_X_LOW             : 7;
        MUINT32 rsv_7                     : 1;
        MUINT32 AE_HST1_X_HI              : 7;
        MUINT32 rsv_15                    : 1;
        MUINT32 AE_HST1_Y_LOW             : 7;
        MUINT32 rsv_23                    : 1;
        MUINT32 AE_HST1_Y_HI              : 7;
        MUINT32 rsv_31                    : 1;
} ISP_CAM_AE_HST1_RNG_T;

typedef union {
    enum { MASK     = 0x7F7F7F7F };
    enum { DEFAULT  = 0x00000000 };
    typedef ISP_CAM_AE_HST1_RNG_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AE_HST1_RNG_T;

// CAM_AE_HST2_RNG CAM+068CH
typedef struct {
        MUINT32 AE_HST2_X_LOW             : 7;
        MUINT32 rsv_7                     : 1;
        MUINT32 AE_HST2_X_HI              : 7;
        MUINT32 rsv_15                    : 1;
        MUINT32 AE_HST2_Y_LOW             : 7;
        MUINT32 rsv_23                    : 1;
        MUINT32 AE_HST2_Y_HI              : 7;
        MUINT32 rsv_31                    : 1;
} ISP_CAM_AE_HST2_RNG_T;

typedef union {
    enum { MASK     = 0x7F7F7F7F };
    enum { DEFAULT  = 0x00000000 };
    typedef ISP_CAM_AE_HST2_RNG_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AE_HST2_RNG_T;

// CAM_AE_HST3_RNG CAM+0690H
typedef struct {
        MUINT32 AE_HST3_X_LOW             : 7;
        MUINT32 rsv_7                     : 1;
        MUINT32 AE_HST3_X_HI              : 7;
        MUINT32 rsv_15                    : 1;
        MUINT32 AE_HST3_Y_LOW             : 7;
        MUINT32 rsv_23                    : 1;
        MUINT32 AE_HST3_Y_HI              : 7;
        MUINT32 rsv_31                    : 1;
} ISP_CAM_AE_HST3_RNG_T;

typedef union {
    enum { MASK     = 0x7F7F7F7F };
    enum { DEFAULT  = 0x00000000 };
    typedef ISP_CAM_AE_HST3_RNG_T reg_t;
    reg_t   bits;
    MUINT32 val;
} ISP_NVRAM_AE_HST3_RNG_T;

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


};

#endif  //  _ISPIF_


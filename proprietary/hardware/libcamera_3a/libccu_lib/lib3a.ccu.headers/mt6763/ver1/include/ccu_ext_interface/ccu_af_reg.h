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
#ifndef _CCU_AF_REG_H_
#define _CCU_AF_REG_H_

#include <mtkcam/def/BuiltinTypes.h>    // For type definitions.
#include <drv/isp_reg.h>    // For ISP register structures.

typedef volatile struct _cam_reg_af_t_ /* 0x1A004000..0x1A004FEB */
{
    CAM_REG_CTL_EN                                  CAM_CTL_EN;                                      /* 0004, 0x1A004004, CAM_A_CTL_EN */
    CAM_REG_CTL_DMA_EN                              CAM_CTL_DMA_EN;                                  /* 0008, 0x1A004008, CAM_A_CTL_DMA_EN */

    CAM_REG_AFO_BASE_ADDR                           CAM_AFO_BASE_ADDR;                               /* 02B0, 0x1A0042B0, CAM_A_AFO_BASE_ADDR */
    CAM_REG_AFO_OFST_ADDR                           CAM_AFO_OFST_ADDR;                               /* 02B8, 0x1A0042B8, CAM_A_AFO_OFST_ADDR */
    // CAM_REG_AFO_DRS                                 CAM_AFO_DRS;                                     /* 02BC, 0x1A0042BC, CAM_A_AFO_DRS */
    CAM_REG_AFO_XSIZE                               CAM_AFO_XSIZE;                                   /* 02C0, 0x1A0042C0, CAM_A_AFO_XSIZE */
    CAM_REG_AFO_YSIZE                               CAM_AFO_YSIZE;                                   /* 02C4, 0x1A0042C4, CAM_A_AFO_YSIZE */
    CAM_REG_AFO_STRIDE                              CAM_AFO_STRIDE;                                  /* 02C8, 0x1A0042C8, CAM_A_AFO_STRIDE */
    // CAM_REG_AFO_CON                                 CAM_AFO_CON;                                     /* 02CC, 0x1A0042CC, CAM_A_AFO_CON */
    // CAM_REG_AFO_CON2                                CAM_AFO_CON2;                                    /* 02D0, 0x1A0042D0, CAM_A_AFO_CON2 */
    // CAM_REG_AFO_CON3                                CAM_AFO_CON3;                                    /* 02D4, 0x1A0042D4, CAM_A_AFO_CON3 */
    // CAM_REG_AFO_CON4                                CAM_AFO_CON4;                                    /* 02DC, 0x1A0042DC, CAM_A_AFO_CON4 */
    // CAM_REG_AFO_ERR_STAT                            CAM_AFO_ERR_STAT;                                /* 0450, 0x1A004450, CAM_A_AFO_ERR_STAT */
    CAM_REG_SGG5_PGN                                CAM_SGG5_PGN;                                    /* 0760, 0x1A004760, CAM_A_SGG5_PGN */
    CAM_REG_SGG5_GMRC_1                             CAM_SGG5_GMRC_1;                                 /* 0764, 0x1A004764, CAM_A_SGG5_GMRC_1 */
    CAM_REG_SGG5_GMRC_2                             CAM_SGG5_GMRC_2;                                 /* 0768, 0x1A004768, CAM_A_SGG5_GMRC_2 */
    CAM_REG_AF_CON                                  CAM_AF_CON;                                      /* 0800, 0x1A004800, CAM_A_AF_CON */
    CAM_REG_AF_TH_0                                 CAM_AF_TH_0;                                     /* 0804, 0x1A004804, CAM_A_AF_TH_0 */
    CAM_REG_AF_TH_1                                 CAM_AF_TH_1;                                     /* 0808, 0x1A004808, CAM_A_AF_TH_1 */
    CAM_REG_AF_FLT_1                                CAM_AF_FLT_1;                                    /* 080C, 0x1A00480C, CAM_A_AF_FLT_1 */
    CAM_REG_AF_FLT_2                                CAM_AF_FLT_2;                                    /* 0810, 0x1A004810, CAM_A_AF_FLT_2 */
    CAM_REG_AF_FLT_3                                CAM_AF_FLT_3;                                    /* 0814, 0x1A004814, CAM_A_AF_FLT_3 */
    CAM_REG_AF_FLT_4                                CAM_AF_FLT_4;                                    /* 0818, 0x1A004818, CAM_A_AF_FLT_4 */
    CAM_REG_AF_FLT_5                                CAM_AF_FLT_5;                                    /* 081C, 0x1A00481C, CAM_A_AF_FLT_5 */
    CAM_REG_AF_FLT_6                                CAM_AF_FLT_6;                                    /* 0820, 0x1A004820, CAM_A_AF_FLT_6 */
    CAM_REG_AF_FLT_7                                CAM_AF_FLT_7;                                    /* 0824, 0x1A004824, CAM_A_AF_FLT_7 */
    CAM_REG_AF_FLT_8                                CAM_AF_FLT_8;                                    /* 0828, 0x1A004828, CAM_A_AF_FLT_8 */
    CAM_REG_AF_SIZE                                 CAM_AF_SIZE;                                     /* 0830, 0x1A004830, CAM_A_AF_SIZE */
    CAM_REG_AF_VLD                                  CAM_AF_VLD;                                      /* 0834, 0x1A004834, CAM_A_AF_VLD */
    CAM_REG_AF_BLK_0                                CAM_AF_BLK_0;                                    /* 0838, 0x1A004838, CAM_A_AF_BLK_0 */
    CAM_REG_AF_BLK_1                                CAM_AF_BLK_1;                                    /* 083C, 0x1A00483C, CAM_A_AF_BLK_1 */
    CAM_REG_AF_TH_2                                 CAM_AF_TH_2;                                     /* 0840, 0x1A004840, CAM_A_AF_TH_2 */

    CAM_REG_RCP_CROP_CON1                           CAM_RCP_CROP_CON1;                               /* 08F0, 0x1A0048F0, CAM_A_RCP_CROP_CON1 */

    CAM_REG_SGG1_PGN                                CAM_SGG1_PGN;                                    /* 0900, 0x1A004900, CAM_A_SGG1_PGN */
    CAM_REG_SGG1_GMRC_1                             CAM_SGG1_GMRC_1;                                 /* 0904, 0x1A004904, CAM_A_SGG1_GMRC_1 */
    CAM_REG_SGG1_GMRC_2                             CAM_SGG1_GMRC_2;                                 /* 0908, 0x1A004908, CAM_A_SGG1_GMRC_2 */

    //CAM_REG_AFO_FH_BASE_ADDR                        CAM_AFO_FH_BASE_ADDR;                            /* 0E10, 0x1A004E10, CAM_A_AFO_FH_BASE_ADDR */
    //CAM_REG_AFO_FH_SPARE_2                          CAM_AFO_FH_SPARE_2;                              /* 0EF0, 0x1A004EF0, CAM_A_AFO_FH_SPARE_2 */
    //CAM_REG_AFO_FH_SPARE_3                          CAM_AFO_FH_SPARE_3;                              /* 0EF4, 0x1A004EF4, CAM_A_AFO_FH_SPARE_3 */
    //CAM_REG_AFO_FH_SPARE_4                          CAM_AFO_FH_SPARE_4;                              /* 0EF8, 0x1A004EF8, CAM_A_AFO_FH_SPARE_4 */
    //CAM_REG_AFO_FH_SPARE_5                          CAM_AFO_FH_SPARE_5;                              /* 0EFC, 0x1A004EFC, CAM_A_AFO_FH_SPARE_5 */
    //CAM_REG_AFO_FH_SPARE_6                          CAM_AFO_FH_SPARE_6;                              /* 0F00, 0x1A004F00, CAM_A_AFO_FH_SPARE_6 */
    //CAM_REG_AFO_FH_SPARE_7                          CAM_AFO_FH_SPARE_7;                              /* 0F04, 0x1A004F04, CAM_A_AFO_FH_SPARE_7 */
    //CAM_REG_AFO_FH_SPARE_8                          CAM_AFO_FH_SPARE_8;                              /* 0F08, 0x1A004F08, CAM_A_AFO_FH_SPARE_8 */
    //CAM_REG_AFO_FH_SPARE_9                          CAM_AFO_FH_SPARE_9;                              /* 0F0C, 0x1A004F0C, CAM_A_AFO_FH_SPARE_9 */
    //CAM_REG_AFO_FH_SPARE_10                         CAM_AFO_FH_SPARE_10;                             /* 0F10, 0x1A004F10, CAM_A_AFO_FH_SPARE_10 */
    //CAM_REG_AFO_FH_SPARE_11                         CAM_AFO_FH_SPARE_11;                             /* 0F14, 0x1A004F14, CAM_A_AFO_FH_SPARE_11 */
    //CAM_REG_AFO_FH_SPARE_12                         CAM_AFO_FH_SPARE_12;                             /* 0F18, 0x1A004F18, CAM_A_AFO_FH_SPARE_12 */
    //CAM_REG_AFO_FH_SPARE_13                         CAM_AFO_FH_SPARE_13;                             /* 0F1C, 0x1A004F1C, CAM_A_AFO_FH_SPARE_13 */
    //CAM_REG_AFO_FH_SPARE_14                         CAM_AFO_FH_SPARE_14;                             /* 0F20, 0x1A004F20, CAM_A_AFO_FH_SPARE_14 */
    //CAM_REG_AFO_FH_SPARE_15                         CAM_AFO_FH_SPARE_15;                             /* 0F24, 0x1A004F24, CAM_A_AFO_FH_SPARE_15 */
    //CAM_REG_AFO_FH_SPARE_16                         CAM_AFO_FH_SPARE_16;                             /* 0F28, 0x1A004F28, CAM_A_AFO_FH_SPARE_16 */
}cam_reg_af_t;

#define REG_AF_NUM (sizeof(cam_reg_af_t) >> 2)

typedef volatile union _CAM_REG_AF_UNION_
{
    UINT32 val[REG_AF_NUM];
    cam_reg_af_t reg;
}CAM_REG_AF_DATA;

typedef volatile struct _CAM_REG_AF_FMT_
{
    UINT32 configNum;
    UINT32 binWidth;
    UINT32 addr[REG_AF_NUM];
    CAM_REG_AF_DATA data;
    CAM_REG_AF_DATA data_a;
    CAM_REG_AF_DATA data_b;
}CAM_REG_AF_FMT;

typedef struct _CAM_REG_AF_DATA
{
    UINT32 val[REG_AF_NUM];
}CAM_REG_AF_DATA_T;

typedef struct CAM_REG_AF
{
    UINT32 Config_num;
    UINT32 AFORegAddr;
    UINT32 addr[REG_AF_NUM];
    UINT32 data[REG_AF_NUM];
    UINT32 data_a[REG_AF_NUM];
    UINT32 data_b[REG_AF_NUM];
    CAM_REG_AF_FMT ori_data;
}CAM_REG_AF_T;

#define INIT_REG_AF_INFO_ADDR(pCAM_REG_AF, REG_NAME)\
    pCAM_REG_AF->addr[(MUINT32)(offsetof(cam_reg_af_t, REG_NAME) >> 2)] = ((MUINT32)offsetof(cam_reg_t, REG_NAME));

#define REG_AF_INFO_VALUE(pREGAF, REG_NAME) \
    (pREGAF->REG_NAME.Raw)


extern "C" {
    int cal_dual_af(int, CAM_REG_AF_FMT*);    int cal_dual_af(int, CAM_REG_AF_FMT*);
}

#endif

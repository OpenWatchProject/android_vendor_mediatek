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

#define NEW_ISP 0

#define DUAL_AF_HW_DECLARE(a, b, c, d, e, f, g, h,...) g h;

#define DUAL_AF_HW_COPY(a, b, c, d, e, f, g, h,...) (a)->h.Raw = (b)->h.Raw;
#define DUAL_AF_SW_COPY(a, b, c, d, e, f, g, h, i, j,...) \
   {\
       int j##_temp = 0;\
       read_int_register_by_name(&j##_temp, j);\
       (a)->h.Bits.i = j##_temp;\
   }\

/* input copy from c model */
#define DUAL_AF_SW(CMD, a, b, c, d, e, f) \
   CMD(a, b, c, d, e, f, CAM_REG_AF_VLD, CAM_AF_VLD, AF_VLD_XSTART, TWIN_AF_OFFSET)/* 0644, 0x1A004644, CAM_A_AF_VLD */\
   CMD(a, b, c, d, e, f, CAM_REG_AF_BLK_1, CAM_AF_BLK_1, AF_BLK_XNUM, TWIN_AF_BLOCK_XNUM)/* 064C, 0x1A00464C, CAM_A_AF_BLK_1 */\

#define DUAL_AF_HW_UNI(CMD, a, b, c, d, e, f) \
   CMD(a, b, c, d, e, f, CAM_UNI_REG_TOP_CTL, CAM_UNI_TOP_CTL)/* 0000, 0x1A003000 */\

/* partial update */
#define DUAL_AF_HW(CMD, a, b, c, d, e, f) \
   CMD(a, b, c, d, e, f, CAM_REG_AFO_BASE_ADDR, CAM_AFO_BASE_ADDR)/* 0004, 0x1A004004, CAM_A_CTL_EN */\
   CMD(a, b, c, d, e, f, CAM_REG_CTL_EN, CAM_CTL_EN)/* 0004, 0x1A004004, CAM_A_CTL_EN */\
    CMD(a, b, c, d, e, f, CAM_REG_CTL_EN2, CAM_CTL_EN2)/* 0004, 0x1A004004, CAM_A_CTL_EN */\
   CMD(a, b, c, d, e, f, CAM_REG_CTL_DMA_EN, CAM_CTL_DMA_EN)/* 0008, 0x1A004008, CAM_A_CTL_DMA_EN */\
   CMD(a, b, c, d, e, f, CAM_REG_CTL_SEL, CAM_CTL_SEL)/* 0010, 0x1A004010, CAM_A_CTL_SEL */\
   CMD(a, b, c, d, e, f, CAM_REG_AF_SIZE, CAM_AF_SIZE)/* 0640, 0x1A004640, CAM_A_AF_SIZE */\
   CMD(a, b, c, d, e, f, CAM_REG_AF_VLD, CAM_AF_VLD)/* 0644, 0x1A004644, CAM_A_AF_VLD */\
   CMD(a, b, c, d, e, f, CAM_REG_AF_BLK_1, CAM_AF_BLK_1)/* 064C, 0x1A00464C, CAM_A_AF_BLK_1 */\
   CMD(a, b, c, d, e, f, CAM_REG_AFO_OFST_ADDR, CAM_AFO_OFST_ADDR)/* 10B8, 0x1A0050B8, CAM_A_AFO_OFST_ADDR */\
   CMD(a, b, c, d, e, f, CAM_REG_AFO_XSIZE, CAM_AFO_XSIZE)/* 10C0, 0x1A0050C0, CAM_A_AFO_XSIZE */\
   CMD(a, b, c, d, e, f, CAM_REG_AFO_YSIZE, CAM_AFO_YSIZE)/* 10C4, 0x1A0050C4, CAM_A_AFO_YSIZE */\
   CMD(a, b, c, d, e, f, CAM_REG_AFO_STRIDE, CAM_AFO_STRIDE)/* 10C8, 0x1A0050C8, CAM_A_AFO_STRIDE */\

/* direct copy */
#define DUAL_AF_HW_R_RW(CMD, a, b, c, d, e, f) \
   /* AF */\
   CMD(a, b, c, d, e, f, CAM_REG_AF_CON, CAM_AF_CON)/* 0610, 0x1A004610, CAM_A_AF_CON */\
    CMD(a, b, c, d, e, f, CAM_REG_AF_CON2, CAM_AF_CON2)/* 0610, 0x1A004610, CAM_A_AF_CON */\
   CMD(a, b, c, d, e, f, CAM_REG_AF_BLK_0, CAM_AF_BLK_0)/* 0648, 0x1A004648, CAM_A_AF_BLK_0 */\
   /* AFO */\

   //CMD(a, b, c, d, e, f, CAM_REG_AFO_DRS, CAM_AFO_DRS)/* 10BC, 0x1A0050BC, CAM_A_AFO_DRS */\
   //CMD(a, b, c, d, e, f, CAM_REG_AFO_CON, CAM_AFO_CON)/* 10CC, 0x1A0050CC, CAM_A_AFO_CON */\
   //CMD(a, b, c, d, e, f, CAM_REG_AFO_CON2, CAM_AFO_CON2)/* 10D0, 0x1A0050D0, CAM_A_AFO_CON2 */\
   //CMD(a, b, c, d, e, f, CAM_REG_AFO_CON3, CAM_AFO_CON3)/* 10D4, 0x1A0050D4, CAM_A_AFO_CON3 */\
   //CMD(a, b, c, d, e, f, CAM_REG_AFO_CON4, CAM_AFO_CON4)/* 10DC, 0x1A0050DC, CAM_A_AFO_CON4 */\

/* direct copy */
#define DUAL_AF_HW_R_W(CMD, a, b, c, d, e, f) \
   CMD(a, b, c, d, e, f, CAM_REG_AF_TH_0, CAM_AF_TH_0)/* 0614, 0x1A004614, CAM_A_AF_TH_0 */\
   CMD(a, b, c, d, e, f, CAM_REG_AF_TH_1, CAM_AF_TH_1)/* 0618, 0x1A004618, CAM_A_AF_TH_1 */\
   CMD(a, b, c, d, e, f, CAM_REG_AF_TH_2, CAM_AF_TH_2)/* 0650, 0x1A004650, CAM_A_AF_TH_2 */\
   CMD(a, b, c, d, e, f, CAM_REG_AF_FLT_1, CAM_AF_FLT_1)/* 061C, 0x1A00461C, CAM_A_AF_FLT_1 */\
   CMD(a, b, c, d, e, f, CAM_REG_AF_FLT_2, CAM_AF_FLT_2)/* 0620, 0x1A004620, CAM_A_AF_FLT_2 */\
   CMD(a, b, c, d, e, f, CAM_REG_AF_FLT_3, CAM_AF_FLT_3)/* 0624, 0x1A004624, CAM_A_AF_FLT_3 */\
   CMD(a, b, c, d, e, f, CAM_REG_AF_FLT_4, CAM_AF_FLT_4)/* 0628, 0x1A004628, CAM_A_AF_FLT_4 */\
   CMD(a, b, c, d, e, f, CAM_REG_AF_FLT_5, CAM_AF_FLT_5)/* 062C, 0x1A00462C, CAM_A_AF_FLT_5 */\
   CMD(a, b, c, d, e, f, CAM_REG_AF_FLT_6, CAM_AF_FLT_6)/* 0630, 0x1A004630, CAM_A_AF_FLT_6 */\
   CMD(a, b, c, d, e, f, CAM_REG_AF_FLT_7, CAM_AF_FLT_7)/* 0634, 0x1A004634, CAM_A_AF_FLT_7 */\
   CMD(a, b, c, d, e, f, CAM_REG_AF_FLT_8, CAM_AF_FLT_8)/* 0638, 0x1A004638, CAM_A_AF_FLT_8 */\
    CMD(a, b, c, d, e, f, CAM_REG_AF_FLT_9, CAM_AF_FLT_9)/* 0638, 0x1A004638, CAM_A_AF_FLT_8 */\
    CMD(a, b, c, d, e, f, CAM_REG_AF_FLT_10, CAM_AF_FLT_10)/* 0638, 0x1A004638, CAM_A_AF_FLT_8 */\
    CMD(a, b, c, d, e, f, CAM_REG_AF_FLT_11, CAM_AF_FLT_11)/* 0638, 0x1A004638, CAM_A_AF_FLT_8 */\
    CMD(a, b, c, d, e, f, CAM_REG_AF_FLT_12, CAM_AF_FLT_12)/* 0638, 0x1A004638, CAM_A_AF_FLT_8 */\
    CMD(a, b, c, d, e, f, CAM_REG_AF_LUT_H0_0, CAM_AF_LUT_H0_0)\
    CMD(a, b, c, d, e, f, CAM_REG_AF_LUT_H0_1, CAM_AF_LUT_H0_1)\
    CMD(a, b, c, d, e, f, CAM_REG_AF_LUT_H0_2, CAM_AF_LUT_H0_2)\
    CMD(a, b, c, d, e, f, CAM_REG_AF_LUT_H0_3, CAM_AF_LUT_H0_3)\
    CMD(a, b, c, d, e, f, CAM_REG_AF_LUT_H0_4, CAM_AF_LUT_H0_4)\
    CMD(a, b, c, d, e, f, CAM_REG_AF_LUT_H1_0, CAM_AF_LUT_H1_0)\
    CMD(a, b, c, d, e, f, CAM_REG_AF_LUT_H1_1, CAM_AF_LUT_H1_1)\
    CMD(a, b, c, d, e, f, CAM_REG_AF_LUT_H1_2, CAM_AF_LUT_H1_2)\
    CMD(a, b, c, d, e, f, CAM_REG_AF_LUT_H1_3, CAM_AF_LUT_H1_3)\
    CMD(a, b, c, d, e, f, CAM_REG_AF_LUT_H1_4, CAM_AF_LUT_H1_4)\
    CMD(a, b, c, d, e, f, CAM_REG_AF_LUT_V_0, CAM_AF_LUT_V_0)\
    CMD(a, b, c, d, e, f, CAM_REG_AF_LUT_V_1, CAM_AF_LUT_V_1)\
    CMD(a, b, c, d, e, f, CAM_REG_AF_LUT_V_2, CAM_AF_LUT_V_2)\
    CMD(a, b, c, d, e, f, CAM_REG_AF_LUT_V_3, CAM_AF_LUT_V_3)\
    CMD(a, b, c, d, e, f, CAM_REG_AF_LUT_V_4, CAM_AF_LUT_V_4)\
   CMD(a, b, c, d, e, f, CAM_REG_SGG1_PGN, CAM_SGG1_PGN)/* 0810, 0x1A004810, CAM_A_SGG1_PGN */\
   CMD(a, b, c, d, e, f, CAM_REG_SGG1_GMRC_1, CAM_SGG1_GMRC_1)/* 0814, 0x1A004814, CAM_A_SGG1_GMRC_1 */\
   CMD(a, b, c, d, e, f, CAM_REG_SGG1_GMRC_2, CAM_SGG1_GMRC_2)/* 0818, 0x1A004818, CAM_A_SGG1_GMRC_2 */\
   CMD(a, b, c, d, e, f, CAM_REG_SGG5_PGN, CAM_SGG5_PGN)/* 0CB0, 0x1A004CB0, CAM_A_SGG5_PGN */\
   CMD(a, b, c, d, e, f, CAM_REG_SGG5_GMRC_1, CAM_SGG5_GMRC_1)/* 0CB4, 0x1A004CB4, CAM_A_SGG5_GMRC_1 */\
   CMD(a, b, c, d, e, f, CAM_REG_SGG5_GMRC_2, CAM_SGG5_GMRC_2)/* 0CB8, 0x1A004CB8, CAM_A_SGG5_GMRC_2 */\

/* HW all */
#define DUAL_AF_HW_ALL(CMD, a, b, c, d, e, f) \
   DUAL_AF_HW(CMD, a, b, c, d, e, f) \
   DUAL_AF_HW_R_RW(CMD, a, b, c, d, e, f) \
   DUAL_AF_HW_R_W(CMD, a, b, c, d, e, f) \

typedef volatile struct _cam_reg_uni_t_ /* 0x1A004000..0x1A004FEB */
{
    DUAL_AF_HW_UNI(DUAL_AF_HW_DECLARE,,,,,,);
}cam_reg_uni_t;

typedef volatile struct _cam_reg_af_t_
{                             
   DUAL_AF_HW(DUAL_AF_HW_DECLARE,,,,,,); 
   DUAL_AF_HW_R_RW(DUAL_AF_HW_DECLARE,,,,,,);
   DUAL_AF_HW_R_W(DUAL_AF_HW_DECLARE,,,,,,);
 }cam_reg_af_t;

#define REG_AF_NUM (sizeof(cam_reg_af_t) >> 2)
#define REG_UNI_NUM (sizeof(cam_reg_uni_t) >> 2)

typedef union _CAM_REG_AF_UNION_
{
    UINT32 val[REG_AF_NUM];
    cam_reg_af_t reg;
}CAM_REG_AF_DATA;

typedef volatile union _CAM_REG_UNI_UNION_
{
    UINT32 val[REG_UNI_NUM];
    cam_reg_uni_t reg;
}CAM_REG_UNI_DATA;

typedef struct _CAM_REG_AF_FMT_
{
    UINT32 configNum;
    UINT32 binWidth;
    UINT32 dualSel;
    UINT32 AFORegAddr;
    UINT32 addrUni[REG_UNI_NUM];
    UINT32 addr[REG_AF_NUM];
    CAM_REG_UNI_DATA uni;
    CAM_REG_AF_DATA data;
    CAM_REG_AF_DATA data_a;
    CAM_REG_AF_DATA data_b;
    CAM_REG_AF_DATA data_c;
}CAM_REG_AF_FMT;

typedef struct _CAM_REG_AF_DATA
{
    UINT32 val[REG_AF_NUM];
}CAM_REG_AF_DATA_T;


#define INIT_REG_AF_INFO_ADDR(pCAM_REG_AF, REG_NAME)\
    pCAM_REG_AF->addr[(MUINT32)(offsetof(cam_reg_af_t, REG_NAME) >> 2)] = ((MUINT32)offsetof(cam_reg_t, REG_NAME));

#define REG_AF_INFO_VALUE(pREGAF, REG_NAME) \
    (pREGAF->REG_NAME.Raw)


extern "C" {
    int cal_dual_af(int, CAM_REG_AF_FMT*);    int cal_dual_af(int, CAM_REG_AF_FMT*);
}

#endif

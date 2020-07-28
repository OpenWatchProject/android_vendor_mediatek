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

#include "af_cxu_flow.h"
#ifndef __PLATFORM__
#include <drv/isp_reg.h>
#else
#include <isp_ccu_reg.h>
#include <ccu_log.h>
#endif

#if 1
// get config value
#define CONFIG_VALUE(FLD) (ptrAfConfigReg->FLD)

// set config value to reg
#define SET_REG_FLD(FLD) (reg.Bits.FLD)=(ptrAfConfigReg->FLD)

#ifdef __PLATFORM__ // CCU
#define AF_REG(FLD) AF_REG_R1A_##FLD
#define AFO_REG(FLD) AFO_REG_R1A_##FLD
#define CAMCTL_REG(FLD) CAMCTL_REG_R1A_##FLD
#define CRP_REG(FLD) CRP_REG_R1A_##FLD
#define AF(FLD) AF_R1A_##FLD
#define AFO(FLD) AFO_R1A_##FLD
#define CAMCTL(FLD) CAMCTL_R1A_##FLD
#define CRP(FLD) CRP_R1A_##FLD
#define MediaTekK 1
#define CAM_REG_START cam_a_reg_t
#else //CPU
#define AF_REG(FLD) REG_AF_R1_##FLD
#define AFO_REG(FLD) REG_AFO_R1_##FLD
#define AF(FLD) AF_R1_##FLD
#define AFO(FLD) AFO_R1_##FLD
#define MediaTekK 0
#define CAM_REG_START cam_reg_t
#endif
// set reg to regtable
#define MAPPING_REGTABLE(REG) \
    { \
        pAFRegInfo[ERegInfo_##REG].val = (reg.Raw); \
        pAFRegInfo[ERegInfo_##REG].addr = ((MUINT32)offsetof(CAM_REG_START, REG)); \
    }


// get value from regtable(reg)
#define REGTABLE_VALUE(REG) (pAFRegInfo[ERegInfo_##REG].val)

#define CHECK_BLK_NUM_X(InHWBlkNumX) ((InHWBlkNumX < MIN_AF_HW_WIN_X) ? MIN_AF_HW_WIN_X : ((InHWBlkNumX > MAX_AF_HW_WIN_X) ? MAX_AF_HW_WIN_X : InHWBlkNumX))
#define CHECK_BLK_NUM_Y(InHWBlkNumY) ((InHWBlkNumY < MIN_AF_HW_WIN_Y) ? MIN_AF_HW_WIN_Y : ((InHWBlkNumY > MAX_AF_HW_WIN_Y) ? MAX_AF_HW_WIN_Y : InHWBlkNumY))

#else
// get config value
#define CONFIG_VALUE(FLD)      (ptrAfConfigReg->FLD)
// set config value to reg
#define SET_REG_FLD(REG, FLD) (REG.Bits.FLD)=(ptrAfConfigReg->FLD)

#ifndef __PLATFORM__
#define CAM_REG(FLD) CAM_REG_##FLD
#define CAM(FLD) CAM_##FLD
#else
#define CAM_REG(FLD) CAM_A_REG_##FLD
#define CAM(FLD) CAM_A_##FLD
#define cam_reg_t cam_a_reg_t
#endif

// set reg to regtable
#define MAPPING_REGTABLE(REG, VALUE) \
    { \
        pAFRegInfo[ERegInfo_##REG].addr = ((MUINT32)offsetof(cam_reg_t, REG)); \
        pAFRegInfo[ERegInfo_##REG].val  = VALUE; \
    }
// get value from regtable(reg)
#define REGTABLE_VALUE(REG) (pAFRegInfo[ERegInfo_##REG].val)
#define CHECK_BLK_NUM_X(InHWBlkNumX) ((InHWBlkNumX < MIN_AF_HW_WIN_X) ? MIN_AF_HW_WIN_X : ((InHWBlkNumX > MAX_AF_HW_WIN_X) ? MAX_AF_HW_WIN_X : InHWBlkNumX))
#define CHECK_BLK_NUM_Y(InHWBlkNumY) ((InHWBlkNumY < MIN_AF_HW_WIN_Y) ? MIN_AF_HW_WIN_Y : ((InHWBlkNumY > MAX_AF_HW_WIN_Y) ? MAX_AF_HW_WIN_Y : InHWBlkNumY))
#endif

MINT32 CxUFlow_AFConfig(AF_CONFIG_T* a_sAFConfig, AFRESULT_ISPREG_T* p_sAFResultConfig)
{
    MINT32 hwErr = 0xFFFFFFFF;
    MINT32 BIN_SZ_W = a_sAFConfig->sBIN_SZ.i4W;
    MINT32 BIN_SZ_H = a_sAFConfig->sBIN_SZ.i4H;
    MINT32 TG_SZ_W  = a_sAFConfig->sTG_SZ.i4W;
    MINT32 TG_SZ_H  = a_sAFConfig->sTG_SZ.i4H;
    AF_CONFIG_REG_T *ptrAfConfigReg = &a_sAFConfig->sConfigReg;
    AFRegInfo_T     *pAFRegInfo     = &p_sAFResultConfig->rAFRegInfo[0];

    if ((BIN_SZ_W == 0) || (BIN_SZ_H == 0))
    {
        // setCamScenarioMode() and ISP_AF_CONFIG_T::config() are called at the same time.
        // sBIN_SZ will be zero.
        return hwErr;
    }

    // 0. Checking HW constrain and resetROI if necessary
    hwErr = CxUFlow_checkHwConstraint(a_sAFConfig);
    if(hwErr)
    {
        //ccu_log(CCU_MUST_LOGTAG, 0, "HW CONSTRAIN HAPPEND\n",0,0,0,0);
        CxUFlow_resetROI(a_sAFConfig);
    }

    // 1. [ConfigValue to RegValue] + [RegValue to RegTable]
    /**************************     REG_AFO_R1_AFO_XSIZE ~ YSIZE    ********************************/
#ifdef __PLATFORM__
    {
        AFO_REG(AFO_BASE_ADDR) reg;
        reg.Raw = 0;
        MAPPING_REGTABLE(AFO(AFO_BASE_ADDR));
    }
    {
        AFO_REG(AFO_OFST_ADDR) reg;
        reg.Raw = 0;
        MAPPING_REGTABLE(AFO(AFO_OFST_ADDR));
    }
    {
        CAMCTL_REG(CAMCTL_EN2) reg;
        reg.Raw = 0;
        reg.Bits.CAMCTL_AF_R1_EN = 1;
        MAPPING_REGTABLE(CAMCTL(CAMCTL_EN2));
    }
    {
        CAMCTL_REG(CAMCTL_DMA_EN) reg;
        reg.Raw = 0;
        reg.Bits.CAMCTL_AFO_R1_EN = 1;
        MAPPING_REGTABLE(CAMCTL(CAMCTL_DMA_EN));
    }
    {
        CAMCTL_REG(CAMCTL_SEL) reg;
        reg.Raw = 0;
        MAPPING_REGTABLE(CAMCTL(CAMCTL_SEL));
    }
    {
        CRP_REG(CRP_X_POS) reg;
        reg.Raw = 0;
        MAPPING_REGTABLE(CRP(CRP_X_POS));
    }
    {
        AFO_REG(AFO_STRIDE) reg;
        reg.Raw = 0;
        reg.Bits.AFO_STRIDE = (CONFIG_VALUE(AF_BLK_XNUM)+1) * (a_sAFConfig->u4AfoBlkSzByte);
        MAPPING_REGTABLE(AFO(AFO_STRIDE));
    }
#endif
    {
        AFO_REG(AFO_XSIZE) reg;
        reg.Raw = 0;
        reg.Bits.AFO_XSIZE = CONFIG_VALUE(AF_BLK_XNUM) * a_sAFConfig->u4AfoBlkSzByte - MediaTekK;
        MAPPING_REGTABLE(AFO(AFO_XSIZE));
    }
    {
        AFO_REG(AFO_YSIZE) reg;
        reg.Raw = 0;
        reg.Bits.AFO_YSIZE = CONFIG_VALUE(AF_BLK_YNUM) - MediaTekK;
        MAPPING_REGTABLE(AFO(AFO_YSIZE));
    }
    /**************************     REG_AF_R1_AF_CON     ********************************/
    {
        AF_REG(AF_CON) reg;
        reg.Raw = 0x200000; // default value
        SET_REG_FLD(AF_BLF_EN);
        SET_REG_FLD(AF_BLF_D_LVL);
        SET_REG_FLD(AF_BLF_R_LVL);
        SET_REG_FLD(AF_BLF_VFIR_MUX);
        SET_REG_FLD(AF_H_GONLY);
        SET_REG_FLD(AF_V_GONLY);
        SET_REG_FLD(AF_V_AVG_LVL);
        SET_REG_FLD(AF_VFLT_MODE);
        MAPPING_REGTABLE(AF(AF_CON));
    }
    /**************************     REG_AF_R1_AF_CON2     ********************************/
    {
        AF_REG(AF_CON2) reg;
        reg.Raw = 0x0; //default value
        SET_REG_FLD(AF_DS_EN);
        SET_REG_FLD(AF_H_FV0_ABS);
        SET_REG_FLD(AF_H_FV1_ABS);
        SET_REG_FLD(AF_H_FV2_ABS);
        SET_REG_FLD(AF_V_FV0_ABS);
        SET_REG_FLD(AF_PL_H_FV_ABS);
        SET_REG_FLD(AF_PL_V_FV_ABS);
        SET_REG_FLD(AF_H_FV0_EN);
        SET_REG_FLD(AF_H_FV1_EN);
        SET_REG_FLD(AF_H_FV2_EN);
        SET_REG_FLD(AF_V_FV0_EN);
        SET_REG_FLD(AF_PL_FV_EN);
        SET_REG_FLD(AF_8BIT_LOWPOWER_EN);
        SET_REG_FLD(AF_PL_BITSEL);
        MAPPING_REGTABLE(AF(AF_CON2));
    }
    /**************************     REG_AF_R1_AF_SIZE     ********************************/
    {
        AF_REG(AF_SIZE) reg;
        reg.Raw = 0;
        SET_REG_FLD(AF_IMAGE_WD);
        MAPPING_REGTABLE(AF(AF_SIZE));
    }
    /**************************     REG_AF_R1_AF_VLD     ********************************/
    {
        AF_REG(AF_VLD) reg;
        reg.Raw = 0;
        SET_REG_FLD(AF_VLD_XSTART);
        SET_REG_FLD(AF_VLD_YSTART);
        MAPPING_REGTABLE(AF(AF_VLD));
    }
    /**************************     REG_AF_R1_AF_BLK_0 ~ PROT    ******************************/
    {
        AF_REG(AF_BLK_0) reg; // block size
        reg.Raw = 0;
        SET_REG_FLD(AF_BLK_XSIZE);
        SET_REG_FLD(AF_BLK_YSIZE);
        MAPPING_REGTABLE(AF(AF_BLK_0));
    }
    {
        AF_REG(AF_BLK_1) reg; // block number
        reg.Raw = 0;
        SET_REG_FLD(AF_BLK_XNUM);
        SET_REG_FLD(AF_BLK_YNUM);
        MAPPING_REGTABLE(AF(AF_BLK_1));
    }
    {
        AF_REG(AF_BLK_PROT) reg;
        reg.Raw = 0;
        SET_REG_FLD(AF_PROT_BLK_XSIZE);
        SET_REG_FLD(AF_PROT_BLK_YSIZE);
        MAPPING_REGTABLE(AF(AF_BLK_PROT));
    }
    /**************************     REG_AF_R1_AF_HFLT0_1 ~ 3    ******************************/
    {
        AF_REG(AF_HFLT0_1) reg;
        reg.Raw = 0;
        SET_REG_FLD(AF_HFLT0_P1);
        SET_REG_FLD(AF_HFLT0_P2);
        SET_REG_FLD(AF_HFLT0_P3);
        SET_REG_FLD(AF_HFLT0_P4);
        MAPPING_REGTABLE(AF(AF_HFLT0_1));
    }
    {
        AF_REG(AF_HFLT0_2) reg;
        reg.Raw = 0;
        SET_REG_FLD(AF_HFLT0_P5);
        SET_REG_FLD(AF_HFLT0_P6);
        SET_REG_FLD(AF_HFLT0_P7);
        SET_REG_FLD(AF_HFLT0_P8);
        MAPPING_REGTABLE(AF(AF_HFLT0_2));
    }
    {
        AF_REG(AF_HFLT0_3) reg;
        reg.Raw = 0;
        SET_REG_FLD(AF_HFLT0_P9);
        SET_REG_FLD(AF_HFLT0_P10);
        SET_REG_FLD(AF_HFLT0_P11);
        SET_REG_FLD(AF_HFLT0_P12);
        MAPPING_REGTABLE(AF(AF_HFLT0_3));
    }
    /**************************     REG_AF_R1_AF_HFLT1_1 ~ 3    ******************************/
    {
        AF_REG(AF_HFLT1_1) reg;
        reg.Raw = 0;
        SET_REG_FLD(AF_HFLT1_P1);
        SET_REG_FLD(AF_HFLT1_P2);
        SET_REG_FLD(AF_HFLT1_P3);
        SET_REG_FLD(AF_HFLT1_P4);
        MAPPING_REGTABLE(AF(AF_HFLT1_1));
    }
    {
        AF_REG(AF_HFLT1_2) reg;
        reg.Raw = 0;
        SET_REG_FLD(AF_HFLT1_P5);
        SET_REG_FLD(AF_HFLT1_P6);
        SET_REG_FLD(AF_HFLT1_P7);
        SET_REG_FLD(AF_HFLT1_P8);
        MAPPING_REGTABLE(AF(AF_HFLT1_2));
    }
    {
        AF_REG(AF_HFLT1_3) reg;
        reg.Raw = 0;
        SET_REG_FLD(AF_HFLT1_P9);
        SET_REG_FLD(AF_HFLT1_P10);
        SET_REG_FLD(AF_HFLT1_P11);
        SET_REG_FLD(AF_HFLT1_P12);
        MAPPING_REGTABLE(AF(AF_HFLT1_3));
    }
    /**************************     REG_AF_R1_AF_HFLT2_1 ~ 3    ******************************/
    {
        AF_REG(AF_HFLT2_1) reg;
        reg.Raw = 0;
        SET_REG_FLD(AF_HFLT2_P1);
        SET_REG_FLD(AF_HFLT2_P2);
        SET_REG_FLD(AF_HFLT2_P3);
        SET_REG_FLD(AF_HFLT2_P4);
        MAPPING_REGTABLE(AF(AF_HFLT2_1));
    }
    {
        AF_REG(AF_HFLT2_2) reg;
        reg.Raw = 0;
        SET_REG_FLD(AF_HFLT2_P5);
        SET_REG_FLD(AF_HFLT2_P6);
        SET_REG_FLD(AF_HFLT2_P7);
        SET_REG_FLD(AF_HFLT2_P8);
        MAPPING_REGTABLE(AF(AF_HFLT2_2));
    }
    {
        AF_REG(AF_HFLT2_3) reg;
        reg.Raw =0;
        SET_REG_FLD(AF_HFLT2_P9);
        SET_REG_FLD(AF_HFLT2_P10);
        SET_REG_FLD(AF_HFLT2_P11);
        SET_REG_FLD(AF_HFLT2_P12);
        MAPPING_REGTABLE(AF(AF_HFLT2_3));
    }
    /**************************     REG_AF_R1_AF_VFLT_1 ~ 3   ******************************/
    {
        AF_REG(AF_VFLT_1) reg;
        reg.Raw =0;
        SET_REG_FLD(AF_VFLT_X1);
        SET_REG_FLD(AF_VFLT_X2);
        SET_REG_FLD(AF_VFLT_X3);
        SET_REG_FLD(AF_VFLT_X4);
        MAPPING_REGTABLE(AF(AF_VFLT_1));
    }
    {
        AF_REG(AF_VFLT_2) reg;
        reg.Raw =0;
        SET_REG_FLD(AF_VFLT_X5);
        SET_REG_FLD(AF_VFLT_X6);
        SET_REG_FLD(AF_VFLT_X7);
        SET_REG_FLD(AF_VFLT_X8);
        MAPPING_REGTABLE(AF(AF_VFLT_2));
    }
    {
        AF_REG(AF_VFLT_3) reg;
        reg.Raw =0;
        SET_REG_FLD(AF_VFLT_X9);
        SET_REG_FLD(AF_VFLT_X10);
        SET_REG_FLD(AF_VFLT_X11);
        SET_REG_FLD(AF_VFLT_X12);
        MAPPING_REGTABLE(AF(AF_VFLT_3));
    }
    /**************************     REG_AF_R1_AF_PL_HFLT_1 ~ 3   ******************************/
    {
        AF_REG(AF_PL_HFLT_1) reg;
        reg.Raw =0;
        SET_REG_FLD(AF_PL_HFLT_P1);
        SET_REG_FLD(AF_PL_HFLT_P2);
        SET_REG_FLD(AF_PL_HFLT_P3);
        SET_REG_FLD(AF_PL_HFLT_P4);
        MAPPING_REGTABLE(AF(AF_PL_HFLT_1));
    }
    {
        AF_REG(AF_PL_HFLT_2) reg;
        reg.Raw =0;
        SET_REG_FLD(AF_PL_HFLT_P5);
        SET_REG_FLD(AF_PL_HFLT_P6);
        SET_REG_FLD(AF_PL_HFLT_P7);
        SET_REG_FLD(AF_PL_HFLT_P8);
        MAPPING_REGTABLE(AF(AF_PL_HFLT_2));
    }
    {
        AF_REG(AF_PL_HFLT_3) reg;
        reg.Raw =0;
        SET_REG_FLD(AF_PL_HFLT_P9);
        SET_REG_FLD(AF_PL_HFLT_P10);
        SET_REG_FLD(AF_PL_HFLT_P11);
        SET_REG_FLD(AF_PL_HFLT_P12);
        MAPPING_REGTABLE(AF(AF_PL_HFLT_3));
    }
    /**************************     REG_AF_R1_AF_PL_VFLT_1 ~ 3   ******************************/
    {
        AF_REG(AF_PL_VFLT_1) reg;
        reg.Raw =0;
        SET_REG_FLD(AF_PL_VFLT_X1);
        SET_REG_FLD(AF_PL_VFLT_X2);
        SET_REG_FLD(AF_PL_VFLT_X3);
        SET_REG_FLD(AF_PL_VFLT_X4);
        MAPPING_REGTABLE(AF(AF_PL_VFLT_1));
    }
    {
        AF_REG(AF_PL_VFLT_2) reg;
        reg.Raw =0;
        SET_REG_FLD(AF_PL_VFLT_X5);
        SET_REG_FLD(AF_PL_VFLT_X6);
        SET_REG_FLD(AF_PL_VFLT_X7);
        SET_REG_FLD(AF_PL_VFLT_X8);
        MAPPING_REGTABLE(AF(AF_PL_VFLT_2));
    }
    {
        AF_REG(AF_PL_VFLT_3) reg;
        reg.Raw =0;
        SET_REG_FLD(AF_PL_VFLT_X9);
        SET_REG_FLD(AF_PL_VFLT_X10);
        SET_REG_FLD(AF_PL_VFLT_X11);
        SET_REG_FLD(AF_PL_VFLT_X12);
        MAPPING_REGTABLE(AF(AF_PL_VFLT_3));
    }
    /**************************     REG_AF_R1_AF_TH_0 ~ 4   ******************************/
    {
        AF_REG(AF_TH_0) reg;
        reg.Raw =0;
        SET_REG_FLD(AF_H_TH_0);
        SET_REG_FLD(AF_H_TH_1);
        MAPPING_REGTABLE(AF(AF_TH_0));
    }
    {
        AF_REG(AF_TH_1) reg;
        reg.Raw =0;
        SET_REG_FLD(AF_H_TH_2);
        SET_REG_FLD(AF_V_TH);
        MAPPING_REGTABLE(AF(AF_TH_1));
    }
    {
        AF_REG(AF_TH_2) reg;
        reg.Raw =0;
        SET_REG_FLD(AF_PL_H_TH);
        MAPPING_REGTABLE(AF(AF_TH_2));
    }
    {
        AF_REG(AF_TH_3) reg;
        reg.Raw =0;
        SET_REG_FLD(AF_G_SAT_TH);
        SET_REG_FLD(AF_B_SAT_TH);
        MAPPING_REGTABLE(AF(AF_TH_3));
    }
    {
        AF_REG(AF_TH_4) reg;
        reg.Raw =0;
        SET_REG_FLD(AF_R_SAT_TH);
        MAPPING_REGTABLE(AF(AF_TH_4) );
    }
    /**************************     REG_AF_R1_AF_LUT_H0_0 ~ 4 ******************************/
    {
        AF_REG(AF_LUT_H0_0) reg;
        reg.Raw =0;
        SET_REG_FLD(AF_H_TH_0_LUT_MODE);
        SET_REG_FLD(AF_H_TH_0_GAIN);
        MAPPING_REGTABLE(AF(AF_LUT_H0_0));
    }
    {
        AF_REG(AF_LUT_H0_1) reg;
        reg.Raw =0;
        SET_REG_FLD(AF_H_TH_0_D1);
        SET_REG_FLD(AF_H_TH_0_D2);
        SET_REG_FLD(AF_H_TH_0_D3);
        SET_REG_FLD(AF_H_TH_0_D4);
        MAPPING_REGTABLE(AF(AF_LUT_H0_1));
    }
    {
        AF_REG(AF_LUT_H0_2) reg;
        reg.Raw =0;
        SET_REG_FLD(AF_H_TH_0_D5);
        SET_REG_FLD(AF_H_TH_0_D6);
        SET_REG_FLD(AF_H_TH_0_D7);
        SET_REG_FLD(AF_H_TH_0_D8);
        MAPPING_REGTABLE(AF(AF_LUT_H0_2));
    }
    {
        AF_REG(AF_LUT_H0_3) reg;
        reg.Raw =0;
        SET_REG_FLD(AF_H_TH_0_D9);
        SET_REG_FLD(AF_H_TH_0_D10);
        SET_REG_FLD(AF_H_TH_0_D11);
        SET_REG_FLD(AF_H_TH_0_D12);
        MAPPING_REGTABLE(AF(AF_LUT_H0_3));
    }
    {
        AF_REG(AF_LUT_H0_4) reg;
        reg.Raw =0;
        SET_REG_FLD(AF_H_TH_0_D13);
        SET_REG_FLD(AF_H_TH_0_D14);
        SET_REG_FLD(AF_H_TH_0_D15);
        SET_REG_FLD(AF_H_TH_0_D16);
        MAPPING_REGTABLE(AF(AF_LUT_H0_4));
    }

    /**************************     REG_AF_R1_AF_LUT_H1_0 ~ 4 ******************************/
    {
        AF_REG(AF_LUT_H1_0) reg;
        reg.Raw =0;
        SET_REG_FLD(AF_H_TH_1_LUT_MODE);
        SET_REG_FLD(AF_H_TH_1_GAIN);
        MAPPING_REGTABLE(AF(AF_LUT_H1_0));
    }
    {
        AF_REG(AF_LUT_H1_1) reg;
        reg.Raw =0;
        SET_REG_FLD(AF_H_TH_1_D1);
        SET_REG_FLD(AF_H_TH_1_D2);
        SET_REG_FLD(AF_H_TH_1_D3);
        SET_REG_FLD(AF_H_TH_1_D4);
        MAPPING_REGTABLE(AF(AF_LUT_H1_1));
    }
    {
        AF_REG(AF_LUT_H1_2) reg;
        reg.Raw =0;
        SET_REG_FLD(AF_H_TH_1_D5);
        SET_REG_FLD(AF_H_TH_1_D6);
        SET_REG_FLD(AF_H_TH_1_D7);
        SET_REG_FLD(AF_H_TH_1_D8);
        MAPPING_REGTABLE(AF(AF_LUT_H1_2));
    }
    {
        AF_REG(AF_LUT_H1_3) reg;
        reg.Raw =0;
        SET_REG_FLD(AF_H_TH_1_D9);
        SET_REG_FLD(AF_H_TH_1_D10);
        SET_REG_FLD(AF_H_TH_1_D11);
        SET_REG_FLD(AF_H_TH_1_D12);
        MAPPING_REGTABLE(AF(AF_LUT_H1_3));
    }
    {
        AF_REG(AF_LUT_H1_4) reg;
        reg.Raw =0;
        SET_REG_FLD(AF_H_TH_1_D13);
        SET_REG_FLD(AF_H_TH_1_D14);
        SET_REG_FLD(AF_H_TH_1_D15);
        SET_REG_FLD(AF_H_TH_1_D16);
        MAPPING_REGTABLE(AF(AF_LUT_H1_4));
    }
    /**************************     REG_AF_R1_AF_LUT_H2_0~ 4  ******************************/
    {
        AF_REG(AF_LUT_H2_0) reg;
        reg.Raw =0;
        SET_REG_FLD(AF_H_TH_2_LUT_MODE);
        SET_REG_FLD(AF_H_TH_2_GAIN);
        MAPPING_REGTABLE(AF(AF_LUT_H2_0));
    }
    {
        AF_REG(AF_LUT_H2_1) reg;
        reg.Raw =0;
        SET_REG_FLD(AF_H_TH_2_D1);
        SET_REG_FLD(AF_H_TH_2_D2);
        SET_REG_FLD(AF_H_TH_2_D3);
        SET_REG_FLD(AF_H_TH_2_D4);
        MAPPING_REGTABLE(AF(AF_LUT_H2_1));
    }
    {
        AF_REG(AF_LUT_H2_2) reg;
        reg.Raw =0;
        SET_REG_FLD(AF_H_TH_2_D5);
        SET_REG_FLD(AF_H_TH_2_D6);
        SET_REG_FLD(AF_H_TH_2_D7);
        SET_REG_FLD(AF_H_TH_2_D8);
        MAPPING_REGTABLE(AF(AF_LUT_H2_2));
    }
    {
        AF_REG(AF_LUT_H2_3) reg;
        reg.Raw =0;
        SET_REG_FLD(AF_H_TH_2_D9);
        SET_REG_FLD(AF_H_TH_2_D10);
        SET_REG_FLD(AF_H_TH_2_D11);
        SET_REG_FLD(AF_H_TH_2_D12);
        MAPPING_REGTABLE(AF(AF_LUT_H2_3));
    }
    {
        AF_REG(AF_LUT_H2_4) reg;
        reg.Raw =0;
        SET_REG_FLD(AF_H_TH_2_D13);
        SET_REG_FLD(AF_H_TH_2_D14);
        SET_REG_FLD(AF_H_TH_2_D15);
        SET_REG_FLD(AF_H_TH_2_D16);
        MAPPING_REGTABLE(AF(AF_LUT_H2_4));
    }
    /**************************     REG_AF_R1_AF_LUT_V_0~ 4  ******************************/
    {
        AF_REG(AF_LUT_V_0) reg;
        reg.Raw =0;
        SET_REG_FLD(AF_V_TH_LUT_MODE);
        SET_REG_FLD(AF_V_TH_GAIN);
        MAPPING_REGTABLE(AF(AF_LUT_V_0));
    }
    {
        AF_REG(AF_LUT_V_1) reg;
        reg.Raw =0;
        SET_REG_FLD(AF_V_TH_D1);
        SET_REG_FLD(AF_V_TH_D2);
        SET_REG_FLD(AF_V_TH_D3);
        SET_REG_FLD(AF_V_TH_D4);
        MAPPING_REGTABLE(AF(AF_LUT_V_1));
    }
    {
        AF_REG(AF_LUT_V_2) reg;
        reg.Raw =0;
        SET_REG_FLD(AF_V_TH_D5);
        SET_REG_FLD(AF_V_TH_D6);
        SET_REG_FLD(AF_V_TH_D7);
        SET_REG_FLD(AF_V_TH_D8);
        MAPPING_REGTABLE(AF(AF_LUT_V_2));
    }
    {
        AF_REG(AF_LUT_V_3) reg;
        reg.Raw =0;
        SET_REG_FLD(AF_V_TH_D9);
        SET_REG_FLD(AF_V_TH_D10);
        SET_REG_FLD(AF_V_TH_D11);
        SET_REG_FLD(AF_V_TH_D12);
        MAPPING_REGTABLE(AF(AF_LUT_V_3));
    }
    {
        AF_REG(AF_LUT_V_4) reg;
        reg.Raw =0;
        SET_REG_FLD(AF_V_TH_D13);
        SET_REG_FLD(AF_V_TH_D14);
        SET_REG_FLD(AF_V_TH_D15);
        SET_REG_FLD(AF_V_TH_D16);
        MAPPING_REGTABLE(AF(AF_LUT_V_4));
    }
    /**************************     REG_AF_R1_AF_SGG1_0~ 5  ******************************/
    {
        AF_REG(AF_SGG1_0) reg;
        reg.Raw =0;
        SET_REG_FLD(AF_SGG1_GAIN);
        SET_REG_FLD(AF_SGG1_GMR_1);
        MAPPING_REGTABLE(AF(AF_SGG1_0));
    }
    {
        AF_REG(AF_SGG1_1) reg;
        reg.Raw =0;
        SET_REG_FLD(AF_SGG1_GMR_2);
        SET_REG_FLD(AF_SGG1_GMR_3);
        MAPPING_REGTABLE(AF(AF_SGG1_1));
    }
    {
        AF_REG(AF_SGG1_2) reg;
        reg.Raw =0;
        SET_REG_FLD(AF_SGG1_GMR_4);
        SET_REG_FLD(AF_SGG1_GMR_5);
        MAPPING_REGTABLE(AF(AF_SGG1_2));
    }
    {
        AF_REG(AF_SGG1_3) reg;
        reg.Raw =0;
        SET_REG_FLD(AF_SGG1_GMR_6);
        SET_REG_FLD(AF_SGG1_GMR_7);
        MAPPING_REGTABLE(AF(AF_SGG1_3));
    }
    {
        AF_REG(AF_SGG1_4) reg;
        reg.Raw =0;
        SET_REG_FLD(AF_SGG1_GMR_8);
        SET_REG_FLD(AF_SGG1_GMR_9);
        MAPPING_REGTABLE(AF(AF_SGG1_4));
    }
    {
        AF_REG(AF_SGG1_5) reg;
        reg.Raw =0;
        SET_REG_FLD(AF_SGG1_GMR_10);
        SET_REG_FLD(AF_SGG1_GMR_11);
        MAPPING_REGTABLE(AF(AF_SGG1_5));
    }
    /**************************     REG_AF_R1_AF_SGG5_0~ 5  ******************************/
    {
        AF_REG(AF_SGG5_0) reg;
        reg.Raw =0;
        SET_REG_FLD(AF_SGG5_GAIN);
        SET_REG_FLD(AF_SGG5_GMR_1);
        MAPPING_REGTABLE(AF(AF_SGG5_0));
    }
    {
        AF_REG(AF_SGG5_1) reg;
        reg.Raw =0;
        SET_REG_FLD(AF_SGG5_GMR_2);
        SET_REG_FLD(AF_SGG5_GMR_3);
        MAPPING_REGTABLE(AF(AF_SGG5_1) );
    }
    {
        AF_REG(AF_SGG5_2) reg;
        reg.Raw =0;
        SET_REG_FLD(AF_SGG5_GMR_4);
        SET_REG_FLD(AF_SGG5_GMR_5);
        MAPPING_REGTABLE(AF(AF_SGG5_2));
    }
    {
        AF_REG(AF_SGG5_3) reg;
        reg.Raw =0;
        SET_REG_FLD(AF_SGG5_GMR_6);
        SET_REG_FLD(AF_SGG5_GMR_7);
        MAPPING_REGTABLE(AF(AF_SGG5_3));
    }
    {
        AF_REG(AF_SGG5_4) reg;
        reg.Raw =0;
        SET_REG_FLD(AF_SGG5_GMR_8);
        SET_REG_FLD(AF_SGG5_GMR_9);
        MAPPING_REGTABLE(AF(AF_SGG5_4));
    }
    {
        AF_REG(AF_SGG5_5) reg;
        reg.Raw =0;
        SET_REG_FLD(AF_SGG5_GMR_10);
        SET_REG_FLD(AF_SGG5_GMR_11);
        MAPPING_REGTABLE(AF(AF_SGG5_5));
    }

    // 3. Update OutAFInfo & p_sAFResultConfig
    p_sAFResultConfig->enableAFHw = 1;
    p_sAFResultConfig->configNum  = a_sAFConfig->u4ConfigNum;

#if 0
    // 4. Log
    CAM_LOGD("HW-%s Config(%d)", __FUNCTION__, a_sAFConfig->u4ConfigNum);
    CAM_LOGD("HW : AFO_XSIZE(0x%x) AFO_YSIZE(0x%x) AF_SIZE(0x%x)",
             REGTABLE_VALUE(AFO_R1_AFO_XSIZE), REGTABLE_VALUE(AFO_R1_AFO_YSIZE), REGTABLE_VALUE(AF_R1_AF_SIZE));
    CAM_LOGD("HW : AF_CON(0x%x) AF_VLD(0x%x) AF_BLK_0(SIZE)(0x%x) AF_BLK_1(NUM)(0x%x)",
             REGTABLE_VALUE(AF_R1_AF_CON), REGTABLE_VALUE(AF_R1_AF_VLD), REGTABLE_VALUE(AF_R1_AF_BLK_0), REGTABLE_VALUE(AF_R1_AF_BLK_1));
    CAM_LOGD("HW : AF_TH_0(0x%x) AF_TH_1(0x%x) AF_TH_2(0x%x) AF_TH_3(0x%x) AF_TH_4(0x%x)",
             REGTABLE_VALUE(AF_R1_AF_TH_0), REGTABLE_VALUE(AF_R1_AF_TH_1), REGTABLE_VALUE(AF_R1_AF_TH_2), REGTABLE_VALUE(AF_R1_AF_TH_3), REGTABLE_VALUE(AF_R1_AF_TH_4));
    CAM_LOGD("HW : AF_SGG1_0(0x%x) AF_SGG1_1(0x%x) AF_SGG1_2(0x%x) AF_SGG1_3(0x%x) AF_SGG1_4(0x%x) AF_SGG1_5(0x%x)",
             REGTABLE_VALUE(AF_R1_AF_SGG1_0), REGTABLE_VALUE(AF_R1_AF_SGG1_1), REGTABLE_VALUE(AF_R1_AF_SGG1_2), REGTABLE_VALUE(AF_R1_AF_SGG1_3), REGTABLE_VALUE(AF_R1_AF_SGG1_4), REGTABLE_VALUE(AF_R1_AF_SGG1_5));
    CAM_LOGD("HW : AF_SGG5_0(0x%x) AF_SGG5_1(0x%x) AF_SGG5_2(0x%x) AF_SGG5_3(0x%x) AF_SGG5_4(0x%x) AF_SGG5_5(0x%x)",
             REGTABLE_VALUE(AF_R1_AF_SGG5_0), REGTABLE_VALUE(AF_R1_AF_SGG5_1), REGTABLE_VALUE(AF_R1_AF_SGG5_2), REGTABLE_VALUE(AF_R1_AF_SGG5_3), REGTABLE_VALUE(AF_R1_AF_SGG5_4), REGTABLE_VALUE(AF_R1_AF_SGG5_5));
    CAM_LOGD("HW : AF_HFLT0_1(0x%x) AF_HFLT0_2(0x%x) AF_HFLT0_3(0x%x)",
             REGTABLE_VALUE(AF_R1_AF_HFLT0_1), REGTABLE_VALUE(AF_R1_AF_HFLT0_1), REGTABLE_VALUE(AF_R1_AF_HFLT0_1));
    CAM_LOGD("HW : AF_HFLT1_1(0x%x) AF_HFLT1_2(0x%x) AF_HFLT1_3(0x%x)",
             REGTABLE_VALUE(AF_R1_AF_HFLT1_1), REGTABLE_VALUE(AF_R1_AF_HFLT1_1), REGTABLE_VALUE(AF_R1_AF_HFLT1_1));
    CAM_LOGD("HW : AF_HFLT2_1(0x%x) AF_HFLT2_2(0x%x) AF_HFLT2_3(0x%x)",
             REGTABLE_VALUE(AF_R1_AF_HFLT2_1), REGTABLE_VALUE(AF_R1_AF_HFLT2_1), REGTABLE_VALUE(AF_R1_AF_HFLT2_1));
#endif
    return hwErr;
}

MINT32 CxUFlow_checkHwConstraint(AF_CONFIG_T *a_sAFConfig)
{
    MINT32 hwErr=0;
    MINT32 BIN_SZ_W = a_sAFConfig->sBIN_SZ.i4W;
    MINT32 BIN_SZ_H = a_sAFConfig->sBIN_SZ.i4H;
    MINT32  TG_SZ_W  = a_sAFConfig->sTG_SZ.i4W;
    MINT32  TG_SZ_H  = a_sAFConfig->sTG_SZ.i4H;
    AF_CONFIG_REG_T *ptrAfConfigReg = &a_sAFConfig->sConfigReg;
    // constraint 1
    if((CONFIG_VALUE(AF_VLD_XSTART)&(0x01))!=0)
    {
        hwErr |= 1<<EHWCONSTRAINT_1;
        //CAM_LOGW("%s HWCONSTRAIN(%x) : AF_VLD_XSTART(%d)", __FUNCTION__, hwErr, CONFIG_VALUE(AF_VLD_XSTART));
    }
#if 0
    // constraint 2
    if(CONFIG_VALUE(AF_IMG_WIDTH)>4096)
    {
        hwErr |= 1<<EHWCONSTRAINT_2;
    }
#endif
    // constraint 3
    if(CONFIG_VALUE(AF_V_AVG_LVL)<2 || CONFIG_VALUE(AF_V_AVG_LVL)>3)
    {
        hwErr |= 1<<EHWCONSTRAINT_3;
    }
    // constraint 4
    if(CONFIG_VALUE(AF_V_AVG_LVL)==3)
    {
        if(CONFIG_VALUE(AF_V_GONLY)==1)
        {
            if((CONFIG_VALUE(AF_BLK_XSIZE)&31)!=0) // bit 0~4
            {
                hwErr |= 1<<EHWCONSTRAINT_4;
            }
        }
        else
        {
            if((CONFIG_VALUE(AF_BLK_XSIZE)&15)!=0) // bit 0~3
            {
                hwErr |= 1<<EHWCONSTRAINT_4;
            }
        }
    }
    else if(CONFIG_VALUE(AF_V_AVG_LVL)==2)
    {
        if(CONFIG_VALUE(AF_V_GONLY)==1)
        {
            if((CONFIG_VALUE(AF_BLK_XSIZE)&15)!=0) // bit 0~3
            {
                hwErr |= 1<<EHWCONSTRAINT_4;
            }
        }
        else
        {
            if((CONFIG_VALUE(AF_BLK_XSIZE)&7)!=0) // bit 0~2
            {
                hwErr |= 1<<EHWCONSTRAINT_4;
            }
        }
    }
    else
    {
        if((CONFIG_VALUE(AF_BLK_XSIZE)&7)!=0) // bit 0~2
        {
            hwErr |= 1<<EHWCONSTRAINT_4;
        }
    }
    if(CONFIG_VALUE(AF_BLK_XSIZE)<1 || CONFIG_VALUE(AF_BLK_XSIZE)>128 || CONFIG_VALUE(AF_BLK_YSIZE)<1 || CONFIG_VALUE(AF_BLK_YSIZE)>128)
    {
        hwErr |= 1<<EHWCONSTRAINT_4;
    }
    // constraint 5
    if(CONFIG_VALUE(AF_BLK_XNUM)<1 || CONFIG_VALUE(AF_BLK_XNUM)>128 || CONFIG_VALUE(AF_BLK_YNUM)<1 || CONFIG_VALUE(AF_BLK_YNUM)>128)
    {
        hwErr |= 1<<EHWCONSTRAINT_5;
    }
    // constraint 6
    if(CONFIG_VALUE(AF_VLD_XSTART) + CONFIG_VALUE(AF_BLK_XSIZE) * CONFIG_VALUE(AF_BLK_XNUM) > BIN_SZ_W)
    {
        hwErr |= 1<<EHWCONSTRAINT_6;
    }
    if(CONFIG_VALUE(AF_VLD_YSTART) + CONFIG_VALUE(AF_BLK_YSIZE) * CONFIG_VALUE(AF_BLK_YNUM) > BIN_SZ_H)
    {
        hwErr |= 1<<EHWCONSTRAINT_6;
    }
    // constraint 10
    if(CONFIG_VALUE(AF_H_GONLY)==1)
    {
        if(CONFIG_VALUE(AF_DS_EN)!=0)
        {
            hwErr |= 1<<EHWCONSTRAINT_10;
        }
    }
    // constrain 11
    if(CONFIG_VALUE(AF_IMAGE_WD) < 48)
    {
        if(CONFIG_VALUE(AF_H_GONLY)!=0)
        {
            hwErr |= 1<<EHWCONSTRAINT_11;
        }
    }
    else if(CONFIG_VALUE(AF_IMAGE_WD) < 56)
    {
        if(CONFIG_VALUE(AF_DS_EN)!=0)
        {
            hwErr |= 1<<EHWCONSTRAINT_11;
        }
    }
    // constraint 12
    if(CONFIG_VALUE(AF_BLK_XSIZE)<CONFIG_VALUE(AF_PROT_BLK_XSIZE) || CONFIG_VALUE(AF_BLK_YSIZE)<CONFIG_VALUE(AF_PROT_BLK_YSIZE))
    {
        hwErr |= 1<<EHWCONSTRAINT_12;
    }
    // constraint 13
    if(CONFIG_VALUE(AF_SGG1_GMR_7)==0 || CONFIG_VALUE(AF_SGG5_GMR_7)==0 || CONFIG_VALUE(AF_SGG1_GMR_11)==0 || CONFIG_VALUE(AF_SGG5_GMR_11)==0)
    {
        hwErr |= 1<<EHWCONSTRAINT_13;
    }
#if 0
    // debug log
    if(hwErr!=0)
    {
        CAM_LOGE("%s HWCONSTRAINT(%x)", __FUNCTION__, hwErr);
        if(hwErr&(1<<EHWCONSTRAINT_1))
        {
            CAM_LOGE("%s (1) AF_VLD_XSTART = %d", __FUNCTION__, CONFIG_VALUE(AF_VLD_XSTART));
        }
    #if 0
        if(hwErr&(1<<EHWCONSTRAINT_2))
        {
            CAM_LOGE("%s (2) AF_IMG_WD = %d", __FUNCTION__, CONFIG_VALUE(AF_IMAGE_WD));
        }
    #endif
        if(hwErr&(1<<EHWCONSTRAINT_3))
        {
            CAM_LOGE("%s (3) AF_V_AVG_LVL = %d", __FUNCTION__, CONFIG_VALUE(AF_V_AVG_LVL));
        }
        if(hwErr&(1<<EHWCONSTRAINT_4))
        {
            CAM_LOGE("%s (4) AF_V_AVG_LVL = %d, AF_V_GONLY = %d, AF_BLK_XSIZE = %d, AF_BLK_YSIZE = %d",
                     __FUNCTION__,
                     CONFIG_VALUE(AF_V_AVG_LVL),
                     CONFIG_VALUE(AF_V_GONLY),
                     CONFIG_VALUE(AF_BLK_XSIZE),
                     CONFIG_VALUE(AF_BLK_YSIZE));
        }
        if(hwErr&(1<<EHWCONSTRAINT_5))
        {
            CAM_LOGE("%s (5) AF_BLK_XNUM = %d, AF_BLK_YNUM = %d",
                     __FUNCTION__,
                     CONFIG_VALUE(AF_BLK_XNUM),
                     CONFIG_VALUE(AF_BLK_YNUM));
        }
        if(hwErr&(1<<EHWCONSTRAINT_6))
        {
            CAM_LOGE("%s (6) %d > %d, %d > %d",
                     __FUNCTION__,
                     CONFIG_VALUE(AF_VLD_XSTART)+CONFIG_VALUE(AF_BLK_XSIZE)*CONFIG_VALUE(AF_BLK_XNUM),
                     BIN_SZ_W,
                     CONFIG_VALUE(AF_VLD_YSTART)+CONFIG_VALUE(AF_BLK_YSIZE)*CONFIG_VALUE(AF_BLK_YNUM),
                     BIN_SZ_H);
        }
        if(hwErr&(1<<EHWCONSTRAINT_10))
        {
            CAM_LOGE("%s (10) AF_H_GONLY = %d, AF_DS_EN = %d",
                     __FUNCTION__,
                     CONFIG_VALUE(AF_H_GONLY),
                     CONFIG_VALUE(AF_DS_EN));
        }
        if(hwErr&(1<<EHWCONSTRAINT_11))
        {
            CAM_LOGE("%s (11) AF_IMG_WD = %d, AF_H_GONLY = %d, AF_DS_EN = %d",
                     __FUNCTION__,
                     CONFIG_VALUE(AF_IMAGE_WD),
                     CONFIG_VALUE(AF_H_GONLY),
                     CONFIG_VALUE(AF_DS_EN));
        }
        if(hwErr&(1<<EHWCONSTRAINT_12))
        {
            CAM_LOGE("%s (12) AF_BLK_XSIZE = %d, AF_BLK_YSIZE = %d, AF_PROT_BLK_XSIZE = %d, AF_PROT_BLK_YSIZE = %d",
                     __FUNCTION__,
                     CONFIG_VALUE(AF_BLK_XSIZE),
                     CONFIG_VALUE(AF_BLK_YSIZE),
                     CONFIG_VALUE(AF_PROT_BLK_XSIZE),
                     CONFIG_VALUE(AF_PROT_BLK_YSIZE));
        }
        if(hwErr&(1<<EHWCONSTRAINT_13))
        {
            CAM_LOGE("%s (13) AF_SGG1_GMR_7 = %d, AF_SGG5_GMR_7 = %d, AF_SGG1_GMR_11 = %d, AF_SGG5_GMR_11 = %d",
                     __FUNCTION__,
                     CONFIG_VALUE(AF_SGG1_GMR_7),
                     CONFIG_VALUE(AF_SGG5_GMR_7),
                     CONFIG_VALUE(AF_SGG1_GMR_11),
                     CONFIG_VALUE(AF_SGG5_GMR_11));
        }
    }
#endif
    return hwErr;
}



MVOID CxUFlow_resetROI(AF_CONFIG_T *a_sAFConfig)
{
    // reshape the roi ==> BLKSIZE, BLKNUM, VLD_START
    MINT32  BIN_SZ_W = a_sAFConfig->sBIN_SZ.i4W;
    MINT32  BIN_SZ_H = a_sAFConfig->sBIN_SZ.i4H;
    MINT32  TG_SZ_W  = a_sAFConfig->sTG_SZ.i4W;
    MINT32  TG_SZ_H  = a_sAFConfig->sTG_SZ.i4H;
    AF_CONFIG_REG_T *ptrAfConfigReg = &a_sAFConfig->sConfigReg;

    /*************************     Configure ROI setting     *******************************/
    // Convert ROI coordinate from TG coordinate to BIN block coordinate. (AREA_T -> af_param.h)
    AREA_T Roi2HWCoord;
    Roi2HWCoord.i4X =  a_sAFConfig->sRoi.i4X * BIN_SZ_W / TG_SZ_W;
    Roi2HWCoord.i4Y =  a_sAFConfig->sRoi.i4Y * BIN_SZ_H / TG_SZ_H;
    Roi2HWCoord.i4W =  a_sAFConfig->sRoi.i4W * BIN_SZ_W / TG_SZ_W;
    Roi2HWCoord.i4H =  a_sAFConfig->sRoi.i4H * BIN_SZ_H / TG_SZ_H;
    Roi2HWCoord.i4Info = 0;

    //min constraint
    MUINT32 minHSz=8;
    MUINT32 minVSz=1;
    MUINT32 af_v_gonly   = CONFIG_VALUE(AF_V_GONLY);
    MUINT32 af_v_avg_lvl = CONFIG_VALUE(AF_V_AVG_LVL);
    if(     (af_v_avg_lvl == 3) && (af_v_gonly == 1)) minHSz = 32;
    else if((af_v_avg_lvl == 3) && (af_v_gonly == 0)) minHSz = 16;
    else if((af_v_avg_lvl == 2) && (af_v_gonly == 1)) minHSz = 16;
    else                                              minHSz =  8;

    // ROI boundary check :
    if ((Roi2HWCoord.i4X < 0) ||
            (Roi2HWCoord.i4X > BIN_SZ_W) ||
            (Roi2HWCoord.i4W < (MINT32)minHSz) ||
            (BIN_SZ_W < (Roi2HWCoord.i4X + Roi2HWCoord.i4W))) /*X*/
    {
        MINT32 x = BIN_SZ_W * 30 / 100;
        MINT32 w = BIN_SZ_W * 40 / 100;
        Roi2HWCoord.i4X = x;
        Roi2HWCoord.i4W = w;
    }
    if ((Roi2HWCoord.i4Y < 0) ||
            (Roi2HWCoord.i4Y > BIN_SZ_H) ||
            (Roi2HWCoord.i4H < (MINT32)minVSz) ||
            (BIN_SZ_H < (Roi2HWCoord.i4Y + Roi2HWCoord.i4H))) /*Y*/
    {
        MINT32 y = BIN_SZ_H * 30 / 100;
        MINT32 h = BIN_SZ_H * 40 / 100;
        Roi2HWCoord.i4Y = y;
        Roi2HWCoord.i4H = h;
    }
    AREA_T Roi2HWCoordTmp = Roi2HWCoord;

    /**************************     CAM_REG_AF_BLK_0 (BLKNUM)    ******************************/
    //-------------
    // AF block sz width
    //-------------
    CONFIG_VALUE(AF_BLK_XNUM) = CHECK_BLK_NUM_X(CONFIG_VALUE(AF_BLK_XNUM));
    MUINT32 win_h_size = Roi2HWCoord.i4W / CONFIG_VALUE(AF_BLK_XNUM);
    if( win_h_size > 40)
    {
        win_h_size = 40; //constraint for twin driver
        MINT32 nh = Roi2HWCoord.i4W / win_h_size;
        //Because block size is changed, check blcok number again.
        CONFIG_VALUE(AF_BLK_XNUM) = CHECK_BLK_NUM_X(nh);
    }
    else if (win_h_size < minHSz)
    {
        win_h_size = minHSz;
        MINT32 nh = Roi2HWCoord.i4W / win_h_size;
        //Because block size is changed, check blcok number again.
        CONFIG_VALUE(AF_BLK_XNUM) = CHECK_BLK_NUM_X(nh);
    }
    if (af_v_gonly == 1)
        win_h_size = win_h_size / 4 * 4; // 2 bits zero
    else
        win_h_size = win_h_size / 2 * 2; // 1 bits zero

    //-------------
    // AF block sz height
    //-------------
    CONFIG_VALUE(AF_BLK_YNUM) = CHECK_BLK_NUM_Y(CONFIG_VALUE(AF_BLK_YNUM));
    MUINT32 win_v_size = Roi2HWCoord.i4H / CONFIG_VALUE(AF_BLK_YNUM);
    if (win_v_size > 128)
    {
        win_v_size = 128;
        MINT32 nv = Roi2HWCoord.i4H / win_v_size;

        //Because block size is changed, check blcok number again.
        CONFIG_VALUE(AF_BLK_YNUM) = CHECK_BLK_NUM_Y(nv);
    }
    else if (win_v_size < minVSz)
    {
        win_v_size = minVSz;
        MINT32 nv = Roi2HWCoord.i4H / win_v_size;

        //Because block size is changed, check blcok number again.
        CONFIG_VALUE(AF_BLK_YNUM) = CHECK_BLK_NUM_Y(nv);
    }

    //-------------
    // Set AF block size.
    //-------------
    CONFIG_VALUE(AF_BLK_XSIZE) = win_h_size;
    CONFIG_VALUE(AF_BLK_YSIZE) = win_v_size;

    /**************************     CAM_REG_AF_VLD     ********************************/
    //-------------
    // Final HW ROI.
    //-------------
    Roi2HWCoord.i4W = CONFIG_VALUE(AF_BLK_XNUM) * CONFIG_VALUE(AF_BLK_XSIZE);
    Roi2HWCoord.i4H = CONFIG_VALUE(AF_BLK_YNUM) * CONFIG_VALUE(AF_BLK_YSIZE);
    Roi2HWCoord.i4X = Roi2HWCoordTmp.i4X + (Roi2HWCoordTmp.i4W / 2) - (Roi2HWCoord.i4W / 2);
    Roi2HWCoord.i4Y = Roi2HWCoordTmp.i4Y + (Roi2HWCoordTmp.i4H / 2) - (Roi2HWCoord.i4H / 2);

    //-------------
    // HW ROI Size checking.
    //-------------
    if (BIN_SZ_W < (MINT32)(Roi2HWCoord.i4X + Roi2HWCoord.i4W) )
    {
        MINT32 x = BIN_SZ_W - CONFIG_VALUE(AF_BLK_XNUM) * CONFIG_VALUE(AF_BLK_XSIZE);
        Roi2HWCoord.i4X = x;
    }

    if( BIN_SZ_H < (MINT32)(Roi2HWCoord.i4Y + Roi2HWCoord.i4H) )
    {
        MINT32 y = BIN_SZ_H - CONFIG_VALUE(AF_BLK_YNUM) * CONFIG_VALUE(AF_BLK_YSIZE);
        Roi2HWCoord.i4Y = y;
    }

    //constraint : The window start point must be multiples of 2
    Roi2HWCoord.i4X = (Roi2HWCoord.i4X / 2) * 2;
    Roi2HWCoord.i4Y = (Roi2HWCoord.i4Y / 2) * 2;
    CONFIG_VALUE(AF_VLD_XSTART) = Roi2HWCoord.i4X;
    CONFIG_VALUE(AF_VLD_YSTART) = Roi2HWCoord.i4Y;

    /**************************     CAM_REG_AF_BLK_1     ******************************/
    //window num
    //reg_af_blk_1.Bits.AF_BLK_XNUM = *(a_sOutAFInfo->hwBlkNumX);
    //reg_af_blk_1.Bits.AF_BLK_YNUM = *(a_sOutAFInfo->hwBlkNumY);

    a_sAFConfig->sRoi.i4X =  Roi2HWCoord.i4X * TG_SZ_W / BIN_SZ_W;
    a_sAFConfig->sRoi.i4Y =  Roi2HWCoord.i4Y * TG_SZ_H / BIN_SZ_H;
    a_sAFConfig->sRoi.i4W =  Roi2HWCoord.i4W * TG_SZ_W / BIN_SZ_W;
    a_sAFConfig->sRoi.i4H =  Roi2HWCoord.i4H * TG_SZ_H / BIN_SZ_H;
}

MINT32 CxUFlow_doAlgoCommand(ext_ESensorDev_T Dev, AlgoCommand_T in, AlgoCommand_T* out)
{
    MINT32 err = 0;

    // 1. To update requestNum
    out->requestNum = in.requestNum;

    // 2. To update LibMode
    if(out->afLibMode != in.afLibMode)
    {
        out->afLibMode = in.afLibMode;
        if(out->afLibMode>=LIB3A_AF_MODE_MIN && out->afLibMode<=LIB3A_AF_MODE_MAX)
        {
            afc_setAFMode(Dev, out->afLibMode);
        }
    }

    // 3. To update MFPos
    if(out->mfPos != in.mfPos)
    {
        out->mfPos = in.mfPos;
        if(in.afLibMode == LIB3A_AF_MODE_MF && out->mfPos>=0)
        {
            afc_setMFPos(Dev, out->mfPos);
            afc_trigger(Dev);
        }
    }

    // 4. targetAssistMove
    if(out->bTargetAssistMove != in.bTargetAssistMove)
    {
        out->bTargetAssistMove = in.bTargetAssistMove;
        if(in.bTargetAssistMove ==1)
        {
            afc_targetAssistMove(Dev);
        }
    }

    // 5. lockAlgo or unlockAlgo
    if(out->eLockAlgo != in.eLockAlgo)
    {
        out->eLockAlgo = in.eLockAlgo;
        if(out->eLockAlgo == AfCommand_Start)
        {
            afc_cancel(Dev);
            afc_lock(Dev);
        }
        else if(out->eLockAlgo == AfCommand_Cancel)
        {
            afc_unlock(Dev);
        }
    }

    // 6. TriggerAlgo or CancelAlgo
    if((out->bCancel != in.bCancel) && (out->bTrigger != in.bTrigger))
    {
        // Cancel + Trigger at the same frame ==> Cancel followed by Trigger
        out->bCancel = in.bCancel;
        if(out->bCancel)
        {
            afc_cancel(Dev);
        }
        out->bTrigger = in.bTrigger;
        if(out->bTrigger)
        {
            afc_trigger(Dev);
        }
    }
    else if(out->bTrigger != in.bTrigger)
    {
        out->bTrigger = in.bTrigger;
        if(out->bTrigger)
        {
            afc_trigger(Dev);
        }
    }
    else if(out->bCancel != in.bCancel)
    {
        out->bCancel = in.bCancel;
        if(out->bCancel)
        {
            afc_cancel(Dev);
        }
    }
    return err;
}


/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2018. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#ifndef __PMIC_WRAP_PLATFORM_H__
#define __PMIC_WRAP_PLATFORM_H__

#define MTK_PLATFORM_MT6358      1

/* #define PMIC_WRAP_DEBUG */
/***********  platform info, PMIC info ********************/
#define PMIC_WRAP_BASE (0xA0010000)
#ifndef TOPCKGEN_BASE
#define TOPCKGEN_BASE           (0xA0210000)
#endif
#define CKSYS_BASE              TOPCKGEN_BASE
#define INFRACFG_AO_REG_BASE    (0xA0000000)

#define PMIC_WRAP_REG_RANGE     (300)
#define PMIC_WRAP_WACS2_RDATA_OFFSET    0xA4
#define PMIC_WRAP_WACS2_VLDCLR_OFFSET   0xA8
#define PMIC_WRAP_INIT_DONE2_OFFSET     0x9C


/************* macro for spi clock config ******************************/
#define CLK_CFG_4_SET                       (TOPCKGEN_BASE+0x144)
#define CLK_CFG_4_CLR                       (TOPCKGEN_BASE+0x148)

#define CLK_SPI_CK_26M                       0xf3000000
#define MODULE_CLK_SEL                      (INFRACFG_AO_REG_BASE+0x098)
#define MODULE_SW_CG_0_SET                  (INFRACFG_AO_REG_BASE+0x080)
#define MODULE_SW_CG_0_CLR                  (INFRACFG_AO_REG_BASE+0x084)
#define INFRA_GLOBALCON_RST2_SET            (INFRACFG_AO_REG_BASE+0x140)
#define INFRA_GLOBALCON_RST2_CLR            (INFRACFG_AO_REG_BASE+0x144)

/****************P2P Reg ********************************/
#if (PMIC_WRAP_SCP)
#include <reg_PMIC_WRAP_P2P_mac.h>
#endif
/*****************************************************************/
#include <reg_PMIC_WRAP_mac.h>
/*******************macro for  regsister@PMIC *******************************/
#define PMIC_REG_BASE (0x0000)
#define DEW_READ_TEST        ((UINT32)(PMIC_REG_BASE+0x040e))
#define DEW_WRITE_TEST       ((UINT32)(PMIC_REG_BASE+0x0410))
#endif /*__PMIC_WRAP_PLATFORM_H__*/

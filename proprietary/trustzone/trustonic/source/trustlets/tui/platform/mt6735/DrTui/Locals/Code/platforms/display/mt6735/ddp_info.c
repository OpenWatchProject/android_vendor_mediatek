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
 * MediaTek Inc. (C) 2019. All rights reserved.
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

#define LOG_TAG "INFO"
#include "ddp_log.h"
#include "ddp_hal.h"
#include "DpDataType.h"
#include "lcm_drv.h"

char* ddp_get_module_name(DISP_MODULE_ENUM module)
{
    switch(module)
    {
        case DISP_MODULE_UFOE   :    return "ufoe ";
        case DISP_MODULE_AAL    :    return "aal ";
        case DISP_MODULE_COLOR0 :    return "color0 ";
        case DISP_MODULE_COLOR1 :    return "color1 ";
        case DISP_MODULE_RDMA0  :    return "rdma0 ";
        case DISP_MODULE_RDMA1  :    return "rdma1 ";
        case DISP_MODULE_RDMA2  :    return "rdma2 ";
        case DISP_MODULE_WDMA0  :    return "wdma0 ";
        case DISP_MODULE_WDMA1  :    return "wdma1 ";
        case DISP_MODULE_OVL0   :    return "ovl0 ";
        case DISP_MODULE_OVL1   :    return "ovl1 ";
        case DISP_MODULE_GAMMA  :    return "gamma ";
        case DISP_MODULE_PWM0   :    return "pwm0 ";
        case DISP_MODULE_PWM1   :    return "pwm1 ";
        case DISP_MODULE_OD     :    return "od ";
        case DISP_MODULE_MERGE  :    return "merge ";
        case DISP_MODULE_SPLIT0 :    return "split0 ";
        case DISP_MODULE_SPLIT1 :    return "split1 ";
        case DISP_MODULE_DSI0   :    return "dsi0 ";
        case DISP_MODULE_DSI1   :    return "dsi1 ";
        case DISP_MODULE_DSIDUAL:    return "dsidual ";
        case DISP_MODULE_DPI    :    return "dpi ";
        case DISP_MODULE_SMI    :    return "smi ";
        case DISP_MODULE_CONFIG :    return "config ";
        case DISP_MODULE_CMDQ   :    return "cmdq ";
        case DISP_MODULE_MUTEX  :    return "mutex ";
        case DISP_MODULE_CCORR  :    return "ccorr";
        case DISP_MODULE_DITHER :    return "dither";
		case DISP_MODULE_SMI_LARB0  :return "smi_larb0";
        case DISP_MODULE_SMI_COMMON :return "smi_common";
        default:
             DDPMSG("invalid module id=%d", module);
             return "unknown";
    }
}
char* ddp_get_reg_module_name(DISP_REG_ENUM module)
{
    switch(module)
    {
        case DISP_REG_OVL0      : return "ovl0";
        case DISP_REG_OVL1      : return "ovl1";
        case DISP_REG_RDMA0     : return "rdma0";
        case DISP_REG_RDMA1     : return "rdma1";
        case DISP_REG_WDMA0     : return "wdma0";
        case DISP_REG_COLOR     : return "color";
        case DISP_REG_CCORR     : return "ccorr";
        case DISP_REG_AAL       : return "aal";
        case DISP_REG_GAMMA     : return "gamma";
        case DISP_REG_DITHER    : return "dither";
        case DISP_REG_UFOE      : return "ufoe";
        case DISP_REG_PWM       : return "pwm";
        case DISP_REG_WDMA1     : return "wdma1";
        case DISP_REG_MUTEX     : return "mutex";
        case DISP_REG_DSI0      : return "dsi0";
        case DISP_REG_DPI0      : return "dpi0";
        case DISP_REG_CONFIG    : return "config";
        case DISP_REG_SMI_LARB0 : return "smi_larb0";
        case DISP_REG_SMI_COMMON: return "smi_common";
        case DISP_REG_CONFIG2   : return "config_clk";
        case DISP_REG_CONFIG3   : return "config_pll";
		case DISP_REG_IO_DRIVING1: return "dpi_io_drive1";
		case DISP_REG_IO_DRIVING2: return "dpi_io_drive2";
		case DISP_REG_IO_DRIVING3: return "dpi_io_drive3";
		case DISP_REG_TIMING1: return "timing1";
		case DISP_REG_TIMING2: return "timing2";
		case DISP_REG_TIMING3: return "timing3";
		case DISP_REG_MIPI      : return "mipi";
		case DISP_REG_OD        : return "od";
        default:
             DDPMSG("invalid module id=%d", module);
             return "unknown";
    }
}

int ddp_get_module_max_irq_bit(DISP_MODULE_ENUM module)
{
    switch(module)
    {
        case DISP_MODULE_UFOE   :    return 0;
        case DISP_MODULE_AAL    :    return 1;
        case DISP_MODULE_COLOR0 :    return 2;
        case DISP_MODULE_COLOR1 :    return 2;
        case DISP_MODULE_RDMA0  :    return 5;
        case DISP_MODULE_RDMA1  :    return 5;
        case DISP_MODULE_RDMA2  :    return 5;
        case DISP_MODULE_WDMA0  :    return 1;
        case DISP_MODULE_WDMA1  :    return 1;
        case DISP_MODULE_OVL0   :    return 3;
        case DISP_MODULE_OVL1   :    return 3;
        case DISP_MODULE_GAMMA  :    return 0;
        case DISP_MODULE_PWM0   :    return 0;
        case DISP_MODULE_PWM1   :    return 0;
        case DISP_MODULE_OD     :    return 0;
        case DISP_MODULE_MERGE  :    return 0;
        case DISP_MODULE_SPLIT0 :    return 0;
        case DISP_MODULE_SPLIT1 :    return 0;
        case DISP_MODULE_DSI0   :    return 6;
        case DISP_MODULE_DSI1   :    return 6;
        case DISP_MODULE_DSIDUAL:    return 6;
        case DISP_MODULE_DPI    :    return 2;
        case DISP_MODULE_SMI    :    return 0;
        case DISP_MODULE_CONFIG :    return 0;
        case DISP_MODULE_CMDQ   :    return 0;
        case DISP_MODULE_MUTEX  :    return 14;
        case DISP_MODULE_CCORR  :    return 0;
        case DISP_MODULE_DITHER :    return 0;
        default:
             DDPMSG("invalid module id=%d", module);
    }
    return 0;
}

unsigned int ddp_module_to_idx(int module)
{
    unsigned int id=0;
    switch(module)
    {
        case DISP_MODULE_UFOE:
        case DISP_MODULE_AAL:
        case DISP_MODULE_COLOR0:
        case DISP_MODULE_RDMA0:
        case DISP_MODULE_WDMA0:
        case DISP_MODULE_OVL0:
        case DISP_MODULE_GAMMA:
        case DISP_MODULE_PWM0:
        case DISP_MODULE_OD:
        case DISP_MODULE_SPLIT0:
        case DISP_MODULE_DSI0:
        case DISP_MODULE_DPI:
        case DISP_MODULE_DITHER:
        case DISP_MODULE_CCORR:
          id = 0;
          break;

        case DISP_MODULE_COLOR1:
        case DISP_MODULE_RDMA1:
        case DISP_MODULE_WDMA1:
        case DISP_MODULE_OVL1:
        case DISP_MODULE_PWM1:
        case DISP_MODULE_SPLIT1:
        case DISP_MODULE_DSI1:
          id = 1;
          break;
        case DISP_MODULE_RDMA2:
	    case DISP_MODULE_DSIDUAL:
          id = 2;
          break;
        default:
          DDPERR("ddp_module_to_idx, module=0x%x \n", module);
    }

    return id;
}

#if 0

// dsi
extern DDP_MODULE_DRIVER ddp_driver_dsi0;
extern DDP_MODULE_DRIVER ddp_driver_dsi1;
extern DDP_MODULE_DRIVER ddp_driver_dsidual;
// dpi
extern DDP_MODULE_DRIVER ddp_driver_dpi;

// ovl
extern DDP_MODULE_DRIVER ddp_driver_ovl;
// rdma
extern DDP_MODULE_DRIVER ddp_driver_rdma;
// wdma
 extern DDP_MODULE_DRIVER ddp_driver_wdma;
// color
 extern DDP_MODULE_DRIVER ddp_driver_color;
// aal
 extern DDP_MODULE_DRIVER ddp_driver_aal;
// od
extern  DDP_MODULE_DRIVER ddp_driver_od;
// ufoe
extern DDP_MODULE_DRIVER ddp_driver_ufoe;
// gamma
extern DDP_MODULE_DRIVER ddp_driver_gamma;

// split
extern DDP_MODULE_DRIVER ddp_driver_split;

// pwm
extern DDP_MODULE_DRIVER ddp_driver_pwm;

DDP_MODULE_DRIVER  * ddp_modules_driver[DISP_MODULE_NUM] =
{

    &ddp_driver_ufoe, //DISP_MODULE_UFOE   ,
    NULL,//DISP_MODULE_AAL    ,
    NULL,//DISP_MODULE_COLOR0 ,
    NULL,//DISP_MODULE_COLOR1 ,
    &ddp_driver_rdma,//DISP_MODULE_RDMA0  ,
    &ddp_driver_rdma,//DISP_MODULE_RDMA1  ,  // 5
    &ddp_driver_rdma,//DISP_MODULE_RDMA2  ,
    &ddp_driver_wdma,//DISP_MODULE_WDMA0  ,
    &ddp_driver_wdma,//DISP_MODULE_WDMA1  ,
    &ddp_driver_ovl,//DISP_MODULE_OVL0   ,
    &ddp_driver_ovl,//DISP_MODULE_OVL1   ,  // 10
    NULL,//DISP_MODULE_GAMMA  ,
    NULL,//DISP_MODULE_PWM0   ,
    NULL,//DISP_MODULE_PWM1   ,
    NULL,//DISP_MODULE_OD     ,
    0,//DISP_MODULE_MERGE  ,  // 15
    &ddp_driver_split,//DISP_MODULE_SPLIT0 ,
    &ddp_driver_split,//DISP_MODULE_SPLIT1 ,
    &ddp_driver_dsi0,//DISP_MODULE_DSI0,
    &ddp_driver_dsi1,//DISP_MODULE_DSI1,
    &ddp_driver_dsidual,//DISP_MODULE_DSIDUAL, // 20
    NULL,//DISP_MODULE_DPI,
    0,//DISP_MODULE_SMI,
    0,//DISP_MODULE_CONFIG,
    0,//DISP_MODULE_CMDQ,
    0,//DISP_MODULE_MUTEX,	// 25
    0,//DISP_MODULE_UNKNOWN,


};

#endif

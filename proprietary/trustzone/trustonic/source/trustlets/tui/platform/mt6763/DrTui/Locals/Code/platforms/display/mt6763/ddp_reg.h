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

#ifndef _DDP_REG_H_
#define _DDP_REG_H_
//#include <mach/sync_write.h>

//#include "display_recorder.h"
//#include "cmdq_record.h"
//#include "cmdq_core.h"

#include "cmdq_sec_record.h"
#include "ddp_hal.h"
#include "ddp_path.h"
#include "ddp_dsi.h"
#include "ddp_drv.h"


/* //////////////////////////////////// macro /////////////////////////////////////////// */

#define ENABLE_CLK_MGR

/* field definition */
/* ------------------------------------------------------------- */
/* MIPITX */
struct MIPITX_DSI_IMPENDANCE_0_REG {
	unsigned RG_DSI0_D2P_RT_CODE:5;
	unsigned rsv_5:3;
	unsigned RG_DSI0_D2N_RT_CODE:5;
	unsigned rev_13:3;
	unsigned RG_DSI0_D0P_RT_CODE:5;
	unsigned rev_21:3;
	unsigned RG_DSI0_D0N_RT_CODE:5;
	unsigned rev_29:3;
};


struct MIPITX_DSI_IMPENDANCE_1_REG {
	unsigned RG_DSI0_CKP_RT_CODE:5;
	unsigned rsv_5:3;
	unsigned RG_DSI0_CKN_RT_CODE:5;
	unsigned rsv_13:3;
	unsigned RG_DSI0_D1P_RT_CODE:5;
	unsigned rsv_21:3;
	unsigned RG_DSI0_D1N_RT_CODE:5;
	unsigned rsv_29:3;
};


struct MIPITX_DSI_IMPENDANCE_2_REG {
	unsigned RG_DSI0_D3P_RT_CODE:5;
	unsigned rsv_5:3;
	unsigned RG_DSI0_D3N_RT_CODE:5;
	unsigned rsv_13:3;
	unsigned RG_DSI0_D2_CKMODE_EN:1;
	unsigned RG_DSI0_D0_CKMODE_EN:1;
	unsigned RG_DSI0_CK_CKMODE_EN:1;
	unsigned RG_DSI0_D1_CKMODE_EN:1;
	unsigned RG_DSI0_D3_CKMODE_EN:1;
	unsigned rsv_21:3;
	unsigned RG_DSI0_D2_PN_SWAP_EN:1;
	unsigned RG_DSI0_D0_PN_SWAP_EN:1;
	unsigned RG_DSI0_CK_PN_SWAP_EN:1;
	unsigned RG_DSI0_D1_PN_SWAP_EN:1;
	unsigned RG_DSI0_D3_PN_SWAP_EN:1;
	unsigned rsv_29:3;
};

struct MIPITX_DSI_LANE_CON_REG {
	unsigned RG_DSI0_CPHY_T1DRV_EN:1;
	unsigned RG_DSI0_ANA_CK_SEL:1;
	unsigned RG_DSI0_PHY_CK_SEL:1;
	unsigned RG_DSI0_CPHY_EN:1;
	unsigned RG_DSI0_PHYCK_INV_EN:1;
	unsigned RG_DSI0_PWR04_EN:1;
	unsigned RG_DSI0_BG_LPF_EN:1;
	unsigned RG_DSI0_BG_CORE_EN:1;
	unsigned RG_DSI0_PAD_TIEL_SEL:1;
	unsigned rsv_9:11;
	unsigned RG_DSI0_PAD_D2P_T0A_TIEL_EN:1;
	unsigned RG_DSI0_PAD_D2N_T0B_TIEL_EN:1;
	unsigned RG_DSI0_PAD_D0P_T0C_TIEL_EN:1;
	unsigned RG_DSI0_PAD_D0N_T1A_TIEL_EN:1;
	unsigned RG_DSI0_PAD_CKP_T1B_TIEL_EN:1;
	unsigned RG_DSI0_PAD_CKN_T1C_TIEL_EN:1;
	unsigned RG_DSI0_PAD_D1P_T2A_TIEL_EN:1;
	unsigned RG_DSI0_PAD_D1N_T2B_TIEL_EN:1;
	unsigned RG_DSI0_PAD_D3P_T2C_TIEL_EN:1;
	unsigned RG_DSI0_PAD_D3N_XXX_TIEL_EN:1;
	unsigned rsv_30:2;
};

struct MIPITX_DSI_VOLTAGE_SEL_REG {
	unsigned RG_DSI0_PRD_REF_SEL:6;
	unsigned RG_DSI0_HSTX_LDO_REF_SEL:4;
	unsigned RG_DSI0_V2I_REF_SEL:4;
	unsigned rsv_14:2;
	unsigned RG_DSI0_LPRX_VTH_SEL:3;
	unsigned rsv_19:1;
	unsigned RG_DSI0_LPRX_VTL_SEL:3;
	unsigned rsv_23:1;
	unsigned RG_DSI0_LPCD_VTH_SEL:3;
	unsigned rsv_27:1;
	unsigned RG_DSI0_LPCD_VTL_SEL:3;
	unsigned rsv_31:1;
};


struct MIPITX_DSI_PRESERVED_REG {
	unsigned RG_DSI0_PRESERVE0:16;
	unsigned RG_DSI0_PRESERVE1:16;
};


struct MIPITX_DSI_TEST_CON_REG {
	unsigned RG_DSI0_D2_SER_BISTTOG:1;
	unsigned RG_DSI0_D0_SER_BISTTOG:1;
	unsigned RG_DSI0_CK_SER_BISTTOG:1;
	unsigned RG_DSI0_D1_SER_BISTTOG:1;
	unsigned RG_DSI0_D3_SER_BISTTOG:1;
	unsigned rsv_5:3;
	unsigned RG_DSI0_D2_SER_DIN_SEL:1;
	unsigned RG_DSI0_D0_SER_DIN_SEL:1;
	unsigned RG_DSI0_CK_SER_DIN_SEL:1;
	unsigned RG_DSI0_D1_SER_DIN_SEL:1;
	unsigned RG_DSI0_D3_SER_DIN_SEL:1;
	unsigned rsv_13:3;
	unsigned RG_DSI0_SER_DATA:8;
	unsigned RG_DSI0_MON_BYTECK_EN:1;
	unsigned RG_DSI0_MON_DIGCK_EN:1;
	unsigned RG_DSI0_TEST_DIV:2;
	unsigned RG_DSI0_TEST_SEL:4;
};


struct MIPITX_DSI_PLL_PWR_REG {
	unsigned AD_DSI0_PLL_SDM_PWR_ON:1;
	unsigned AD_DSI0_PLL_SDM_ISO_EN:1;
	unsigned rsv_2:6;
	unsigned DA_DSI0_PLL_SDM_PWR_ACK:1;
	unsigned rsv_9:23;
};


struct MIPITX_DSI_PLL_CON0_REG {
	unsigned RG_DSI0_PLL_SDM_PCW;
};


struct MIPITX_DSI_PLL_CON1_REG {
	unsigned RG_DSI0_PLL_SDM_PCW_CHG:1;
	unsigned rsv_1:3;
	unsigned RG_DSI0_PLL_EN:1;
	unsigned RG_DSI0_PLL_EN_V18:1;
	unsigned rsv_6:2;
	unsigned RG_DSI0_PLL_POSDIV:3;
	unsigned RG_DSI0_PLL_PREDIV:2;
	unsigned RG_DSI0_PLL_SDM_FRA_EN:1;
	unsigned RG_DSI0_PLL_SDM_HREN:1;
	unsigned RG_DSI0_PLL_LVROD_EN:1;
	unsigned RG_DSI0_PLL_BP:1;
	unsigned RG_DSI0_PLL_BR:1;
	unsigned RG_DSI0_PLL_BLP:1;
	unsigned rsv_19:1;
	unsigned RG_DSI0_PLL_RST_DLY:2;
	unsigned rsv_22:10;
};


struct MIPITX_DSI_PLL_CON2_REG {
	unsigned RG_DSI0_PLL_SDM_SSC_PH_INIT:1;
	unsigned RG_DSI0_PLL_SDM_SSC_EN:1;
	unsigned rsv_2:14;
	unsigned RG_DSI0_PLL_SDM_SSC_PRD:16;
};


struct MIPITX_DSI_PLL_CON3_REG {
	unsigned RG_DSI0_PLL_SDM_SSC_DELTA1:16;
	unsigned RG_DSI0_PLL_SDM_SSC_DELTA:16;
};


struct MIPITX_DSI_PLL_CON4_REG {
	unsigned RG_DSI0_PLL_MONCK_EN:1;
	unsigned rsv_1:1;
	unsigned RG_DSI0_PLL_MONVC_EN:2;
	unsigned RG_DSI0_PLL_MONREF_EN:1;
	unsigned RG_DSI0_PLL_BW:3;
	unsigned RG_DSI0_PLL_FS:2;
	unsigned RG_DSI0_PLL_IBIAS:2;
	unsigned RG_DSI0_PLL_ICHP:2;
	unsigned rsv_14:2;
	unsigned RG_DSI0_PLL_RESERVED:16;
};

struct MIPITX_DSI_PHY_SEL0_REG {
	unsigned MIPI_TX_CPHY_EN:1;
	unsigned MIPI_TX_SW_CTRL_EN:1;
	unsigned rsv_2:2;
	unsigned MIPI_TX_PHY2_SEL:4;
	unsigned MIPI_TX_CPHY0BC_SEL:4;
	unsigned MIPI_TX_PHY0_SEL:4;
	unsigned MIPI_TX_PHY1AB_SEL:4;
	unsigned MIPI_TX_PHYC_SEL:4;
	unsigned MIPI_TX_CPHY1CA_SEL:4;
	unsigned MIPI_TX_PHY1_SEL:4;
};


struct MIPITX_DSI_PHY_SEL1_REG {
	unsigned MIPI_TX_PHY2BC_SEL:4;
	unsigned MIPI_TX_PHY3_SEL:4;
	unsigned MIPI_TX_CPHYXXX_SEL:4;
	unsigned MIPI_TX_LPRX0AB_SEL:4;
	unsigned MIPI_TX_LPRX0BC_SEL:4;
	unsigned MIPI_TX_LPRX0CA_SEL:4;
	unsigned MIPI_TX_CPHY0_HS_SEL:2;
	unsigned MIPI_TX_CPHY1_HS_SEL:2;
	unsigned MIPI_TX_CPHY2_HS_SEL:2;
	unsigned rsv_30:2;
};


struct MIPITX_DSI_PHY_SEL2_REG {
	unsigned MIPI_TX_PHY2_HSDATA_SEL:4;
	unsigned MIPI_TX_CPHY0BC_HSDATA_SEL:4;
	unsigned MIPI_TX_PHY0_HSDATA_SEL:4;
	unsigned MIPI_TX_PHY1AB_HSDATA_SEL:4;
	unsigned MIPI_TX_PHYC_HSDATA_SEL:4;
	unsigned MIPI_TX_CPHY1CA_HSDATA_SEL:4;
	unsigned MIPI_TX_PHY1_HSDATA_SEL:4;
	unsigned MIPI_TX_PHY2BC_HSDATA_SEL:4;
};


struct MIPITX_DSI_PHY_SEL3_REG {
	unsigned MIPI_TX_PHY3_HSDATA_SEL:4;
	unsigned rsv_4:28;
};

struct MIPITX_DSI_SW_CTRL_CON4_REG {
	unsigned rsv_0:8;
	unsigned MIPI_TX_SW_ANA_CK_EN:1;
	unsigned rsv_9:23;
};


struct MIPITX_DSI_CD_CON_REG {
	unsigned MIPI_TX_CD_CON:10;
	unsigned rsv_10:22;
};

struct MIPITX_DSI_DBG_CON_REG {
	unsigned rsv_0:4;
	unsigned MIPI_TX_DBG_OUT_EN:1;
	unsigned MIPI_TX_GPIO_MODE_EN:1;
	unsigned MIPI_TX_APB_ASYNC_CNT_EN:1;
	unsigned rsv_7: 25;
};

struct MIPI_DSI_DBG_OUT {
	unsigned rsv_0:4;
	unsigned MIPI_TX_DBG_OUT_EN:1;
	unsigned MIPI_TX_GPIO_MODE_EN:1;
	unsigned MIPI_TX_APB_ASYNC:1;
	unsigned rsv_7:26;
};

struct MIPITX_DSI_APB_ASYNC_STA_REG {
	unsigned MIPI_TX_APB_ASYNC_ERR:1;
	unsigned MIPI_TX_APB_ASYNC_ERR_ADDR:10;
	unsigned rsv_11:21;
};

struct MIPITX_DSI_BIST_CON_REG {
	unsigned DPHY_BIST_MODE:1;
	unsigned DPHY_BIST_ENABLE:1;
	unsigned DPHY_BIST_FIX_PATTERN:1;
	unsigned DPHY_BIST_SPECIFIED_PATTERN:1;
	unsigned DPHY_BIST_HS_FREE:1;
	unsigned rsv_5:2;
	unsigned DPHY_BIST_DESKEW_MODE:1;
	unsigned DPHY_BIST_LANE_NUM:4;
	unsigned rsv_12:4;
	unsigned DPHY_BIST_TIMIING:8;
	unsigned DPHY_BIST_CK_HS_PATTERN:8;
};


struct MIPITX_DSI_BIST_PAT_REG {
	unsigned DPHY_BIST_PATTERN;
};


struct MIPITX_CPHY_BIST_CON0_REG {
	unsigned CPHY_BIST_MODE:1;
	unsigned CPHY_BIST_PROGSEQ_EN:1;
	unsigned CPHY_BIST_ESCAPE_EN:1;
	unsigned CPHY_BIST_START:1;
	unsigned CPHY_BIST_PACKET_TEST_MODE:1;
	unsigned CPHY_BIST_PACKET_TEST_MODE_OUT_EN:1;
	unsigned CPHY_BIST_SETTLE_SKIP_EN:1;
	unsigned rsv7:1;
	unsigned CPHY_BIST_PREBEGIN_CNT:8;
	unsigned CPHY_BIST_POST_CNT:8;
	unsigned rsv_24:8;
};


struct MIPITX_CPHY_BIST_CON1_REG {
	unsigned CPHY_BIST_PROGSEQ_SYMBOL_LSB;
};


struct MIPITX_CPHY_BIST_CON2_REG {
	unsigned CPHY_BIST_PROGSEQ_SYMBOL_MSB:10;
	unsigned rsv_10:2;
	unsigned CPHY_BIST_PRB_SEED:18;
	unsigned rsv_30:2;
};


struct MIPITX_CPHY_BIST_CON3_REG {
	unsigned CPHY_BIST_PRB_SEL:3;
	unsigned CPHY_BIST_PRB_FIX:1;
	unsigned CPHY_BIST_INIT_WIRE_STATE:3;
	unsigned CPHY_BIST_WIRE_STATE_FIX:1;
	unsigned rsv_8:24;
};

/* field definition */
/* ------------------------------------------------------------- */
/* DSI */

struct DSI_START_REG {
	unsigned DSI_START:1;
	unsigned rsv_1:1;
	unsigned SLEEPOUT_START:1;
	unsigned rsv_3:1;
	unsigned SKEWCAL_START:1;
	unsigned rsv_5:11;
	unsigned VM_CMD_START:1;
	unsigned rsv_17:15;
};


struct DSI_STATUS_REG {
	unsigned rsv_0:4;
	unsigned ESC_ENTRY_ERR:1;
	unsigned ESC_SYNC_ERR:1;
	unsigned CTRL_ERR:1;
	unsigned CONTENT_ERR:1;
	unsigned rsv_8:24;
};


struct DSI_INT_ENABLE_REG {
	unsigned RD_RDY:1;
	unsigned CMD_DONE:1;
	unsigned TE_RDY:1;
	unsigned VM_DONE:1;
	unsigned FRAME_DONE_INT_EN:1;
	unsigned VM_CMD_DONE:1;
	unsigned SLEEPOUT_DONE:1;
	unsigned TE_TIMEOUT_INT_EN:1;
	unsigned VM_VBP_STR_INT_EN:1;
	unsigned VM_VACT_STR_INT_EN:1;
	unsigned VM_VFP_STR_INT_EN:1;
	unsigned SKEWCAL_DONE_INT_EN:1;
	unsigned BUFFER_UNDERRUN_INT_EN:1;
	unsigned BTA_TIMEOUT_INT_EN:1;
	unsigned INP_UNFINISH_INT_EN:1;
	unsigned SLEEPIN_ULPS_INT_EN:1;
	unsigned LPRX_RD_RDY_EVENT_EN:1;
	unsigned CMD_DONE_EVENT_EN:1;
	unsigned TE_RDY_EVENT_EN:1;
	unsigned VM_DONE_EVENT_EN:1;
	unsigned FRAME_DONE_EVENT_EN:1;
	unsigned VM_CMD_DONE_EVENT_EN:1;
	unsigned SLEEPOUT_DONE_EVENT_EN:1;
	unsigned TE_TIMEOUT_EVENT_EN:1;
	unsigned VM_VBP_STR_EVENT_EN:1;
	unsigned VM_VACT_STR_EVENT_EN:1;
	unsigned VM_VFP_STR_EVENT_EN:1;
	unsigned SKEWCAL_DONE_EVENT_EN:1;
	unsigned BUFFER_UNDERRUN_EVENT_EN:1;
	unsigned BTA_TIMEOUT_EVENT_EN:1;
	unsigned INP_UNFINISH_EVENT_EN:1;
	unsigned SLEEPIN_ULPS_EVENT_EN:1;
};


struct DSI_INT_STATUS_REG {
	unsigned RD_RDY:1;
	unsigned CMD_DONE:1;
	unsigned TE_RDY:1;
	unsigned VM_DONE:1;
	unsigned FRAME_DONE_INT_EN:1;
	unsigned VM_CMD_DONE:1;
	unsigned SLEEPOUT_DONE:1;
	unsigned TE_TIMEOUT_INT_EN:1;
	unsigned VM_VBP_STR_INT_EN:1;
	unsigned VM_VACT_STR_INT_EN:1;
	unsigned VM_VFP_STR_INT_EN:1;
	unsigned SKEWCAL_DONE_INT_EN:1;
	unsigned BUFFER_UNDERRUN_INT_EN:1;
	unsigned BTA_TIMEOUT_INT_EN:1;
	unsigned INP_UNFINISH_INT_EN:1;
	unsigned SLEEPIN_DONE:1;
	unsigned rsv_16:15;
	unsigned BUSY:1;
};


struct DSI_COM_CTRL_REG {
	unsigned DSI_RESET:1;
	unsigned rsv_1:1;
	unsigned DPHY_RESET:1;
	unsigned rsv_3:1;
	unsigned DSI_DUAL_EN:1;
	unsigned rsv_5:27;
};


enum DSI_MODE_CTRL {
	DSI_CMD_MODE = 0,
	DSI_SYNC_PULSE_VDO_MODE = 1,
	DSI_SYNC_EVENT_VDO_MODE = 2,
	DSI_BURST_VDO_MODE = 3
};


struct DSI_MODE_CTRL_REG {
	unsigned MODE:2;
	unsigned rsv_2:6;
	unsigned INTERLACE_MODE:2;
	unsigned rsv_10:6;
	unsigned FRM_MODE:1;
	unsigned MIX_MODE:1;
	unsigned V2C_SWITCH_ON:1;
	unsigned C2V_SWITCH_ON:1;
	unsigned SLEEP_MODE:1;
	unsigned rsv_21:11;
};


enum DSI_LANE_NUM {
	ONE_LANE = 1,
	TWO_LANE = 2,
	THREE_LANE = 3,
	FOUR_LANE = 4
};


struct DSI_TXRX_CTRL_REG {
	unsigned VC_NUM:2;
	unsigned LANE_NUM:4;
	unsigned DIS_EOT:1;
	unsigned BLLP_EN:1;
	unsigned TE_FREERUN:1;
	unsigned EXT_TE_EN:1;
	unsigned EXT_TE_EDGE:1;
	unsigned TE_AUTO_SYNC:1;
	unsigned MAX_RTN_SIZE:4;
	unsigned HSTX_CKLP_EN:1;
	unsigned TYPE1_BTA_SEL:1;
	unsigned TE_WITH_CMD_EN:1;
	unsigned TE_TIMEOUT_CHK_EN:1;
	unsigned EXT_TE_TIME_VM:4;
	unsigned RGB_PKT_CNT:4;
	unsigned LP_ONLY_VBLK:1;
	unsigned BTA_TIMEOUT_CHK_EN:1;
	unsigned rsv_30:2;
};


enum DSI_PS_TYPE {
	PACKED_PS_16BIT_RGB565 = 0,
	LOOSELY_PS_18BIT_RGB666 = 1,
	PACKED_PS_24BIT_RGB888 = 2,
	PACKED_PS_18BIT_RGB666 = 3
};


struct DSI_PSCTRL_REG {
	unsigned DSI_PS_WC:15;
	unsigned rsv_15:1;
	unsigned DSI_PS_SEL:3;
	unsigned rsv_19:5;
	unsigned RGB_SWAP:1;
	unsigned BYTE_SWAP:1;
	unsigned CUSTOM_HEADER:6;
};


struct DSI_VSA_NL_REG {
	unsigned VSA_NL:12;
	unsigned rsv_12:20;
};


struct DSI_VBP_NL_REG {
	unsigned VBP_NL:12;
	unsigned rsv_12:20;
};


struct DSI_VFP_NL_REG {
	unsigned VFP_NL:12;
	unsigned rsv_12:20;
};


struct DSI_VACT_NL_REG {
	unsigned VACT_NL:15;
	unsigned rsv_15:17;
};

struct DSI_LFR_CON_REG {
	unsigned LFR_MODE:2;
	unsigned LFR_TYPE:2;
	unsigned LFR_EN:1;
	unsigned LFR_UPDATE:1;
	unsigned LFR_VSE_DIS:1;
	unsigned rsv_7:1;
	unsigned LFR_SKIP_NUM:6;
	unsigned rsv_14:18;
};


struct DSI_LFR_STA_REG {
	unsigned LFR_SKIP_CNT:6;
	unsigned rsv_6:2;
	unsigned LFR_SKIP_STA:1;
	unsigned rsv_9:23;
};


struct DSI_SIZE_CON_REG {
	unsigned DSI_WIDTH:15;
	unsigned rsv_15:1;
	unsigned DSI_HEIGHT:15;
	unsigned rsv_31:1;
};

struct DSI_VFP_EARLY_STOP_REG {
	unsigned VFP_EARLY_STOP_EN:1;
	unsigned VFP_EARLY_STOP_SKIP_VSA_EN:1;
	unsigned rsv_2:2;
	unsigned VFP_UNLIMITED_MODE:1;
	unsigned rsv_5:3;
	unsigned VFP_EARLY_STOP1;
	unsigned rsv_9:7;
	unsigned VFP_EARLY_STOP_MIN_NL:12;
	unsigned rsv_28:4;
};
struct DSI_HSA_WC_REG {
	unsigned HSA_WC:12;
	unsigned rsv_12:20;
};


struct DSI_HBP_WC_REG {
	unsigned HBP_WC:12;
	unsigned rsv_12:20;
};


struct DSI_HFP_WC_REG {
	unsigned HFP_WC:12;
	unsigned rsv_12:20;
};

struct DSI_BLLP_WC_REG {
	unsigned BLLP_WC:12;
	unsigned rsv_12:20;
};

struct DSI_CMDQ_CTRL_REG {
	unsigned CMDQ_SIZE:8;
	unsigned rsv_8:24;
};

struct DSI_HSTX_CKLP_REG {
	unsigned rsv_0:2;
	unsigned HSTX_CKLP_WC:14;
	unsigned HSTX_CKLP_WC_AUTO:1;
	unsigned rsv_17:15;
};

struct DSI_HSTX_CKLP_WC_AUTO_RESULT_REG {
	unsigned HSTX_CKLP_WC_AUTO_RESULT:16;
	unsigned rsv_16:16;
};

struct DSI_RX_DATA_REG {
	unsigned char byte0;
	unsigned char byte1;
	unsigned char byte2;
	unsigned char byte3;
};


struct DSI_RACK_REG {
	unsigned DSI_RACK:1;
	unsigned DSI_RACK_BYPASS:1;
	unsigned rsv2:30;
};


struct DSI_TRIG_STA_REG {
	unsigned TRIG0:1;	/* remote rst */
	unsigned TRIG1:1;	/* TE */
	unsigned TRIG2:1;	/* ack */
	unsigned TRIG3:1;	/* rsv */
	unsigned RX_ULPS:1;
	unsigned DIRECTION:1;
	unsigned RX_LPDT:1;
	unsigned rsv7:1;
	unsigned RX_POINTER:4;
	unsigned rsv12:20;
};


struct DSI_MEM_CONTI_REG {
	unsigned RWMEM_CONTI:16;
	unsigned rsv16:16;
};


struct DSI_FRM_BC_REG {
	unsigned FRM_BC:21;
	unsigned rsv21:11;
};

struct DSI_3D_CON_REG {
	unsigned _3D_MODE:2;
	unsigned _3D_FMT:2;
	unsigned _3D_VSYNC:1;
	unsigned _3D_LR:1;
	unsigned rsv6:2;
	unsigned _3D_EN:1;
	unsigned rsv9:23;
};

struct DSI_TIME_CON0_REG {
	unsigned UPLS_WAKEUP_PRD:16;
	unsigned SKEWCALL_PRD:16;
};

struct DSI_TIME_CON1_REG {
	unsigned TE_TIMEOUT_PRD:16;
	unsigned PREFETCH_TIME:15;
	unsigned PREFETCH_EN:1;
};

struct DSI_TIME_CON2_REG {
	unsigned BTA_TIMEOUT_PRD:16;
	unsigned rsv_16:16;
};

struct DSI_PHY_LCPAT_REG {
	unsigned LC_HSTX_CK_PAT:8;
	unsigned rsv8:24;
};

struct DSI_PHY_LCCON_REG {
	unsigned LC_HS_TX_EN:1;
	unsigned LC_ULPM_EN:1;
	unsigned LC_WAKEUP_EN:1;
	unsigned TRAIL_FIX:1;
	unsigned rsv4:4;
	unsigned EARLY_DRDY:5;
	unsigned rsv13:3;
	unsigned EARLY_HS_POE:5;
	unsigned rsv21:11;
};


struct DSI_PHY_LD0CON_REG {
	unsigned L0_RM_TRIG_EN:1;
	unsigned L0_ULPM_EN:1;
	unsigned L0_WAKEUP_EN:1;
	unsigned Lx_ULPM_AS_L0:1;
	unsigned L0_RX_FILTER_EN:1;
	unsigned rsv5:27;
};


struct DSI_PHY_SYNCON_REG {
	unsigned HS_SYNC_CODE:8;
	unsigned HS_SYNC_CODE2:8;
	unsigned HS_SKEWCAL_PAT:8;
	unsigned HS_DB_SYNC_EN:1;
	unsigned rsv25:7;
};


struct DSI_PHY_TIMCON0_REG {
	unsigned char LPX;
	unsigned char HS_PRPR;
	unsigned char HS_ZERO;
	unsigned char HS_TRAIL;
};


struct DSI_PHY_TIMCON1_REG {
	unsigned char TA_GO;
	unsigned char TA_SURE;
	unsigned char TA_GET;
	unsigned char DA_HS_EXIT;
};


struct DSI_PHY_TIMCON2_REG {
	unsigned char CONT_DET;
	unsigned char DA_HS_SYNC;
	unsigned char CLK_ZERO;
	unsigned char CLK_TRAIL;
};


struct DSI_PHY_TIMCON3_REG {
	unsigned char CLK_HS_PRPR;
	unsigned char CLK_HS_POST;
	unsigned char CLK_HS_EXIT;
	unsigned rsv24:8;
};


struct DSI_VM_CMD_CON_REG {
	unsigned VM_CMD_EN:1;
	unsigned LONG_PKT:1;
	unsigned TIME_SEL:1;
	unsigned TS_VSA_EN:1;
	unsigned TS_VBP_EN:1;
	unsigned TS_VFP_EN:1;
	unsigned rsv6:2;
	unsigned CM_DATA_ID:8;
	unsigned CM_DATA_0:8;
	unsigned CM_DATA_1:8;
};


struct DSI_PHY_TIMCON_REG {
	struct DSI_PHY_TIMCON0_REG CTRL0;
	struct DSI_PHY_TIMCON1_REG CTRL1;
	struct DSI_PHY_TIMCON2_REG CTRL2;
	struct DSI_PHY_TIMCON3_REG CTRL3;
};


struct DSI_CKSM_OUT_REG {
	unsigned PKT_CHECK_SUM:16;
	unsigned ACC_CHECK_SUM:16;
};


struct DSI_STATE_DBG0_REG {
	unsigned DPHY_CTL_STATE_C:9;
	unsigned rsv9:7;
	unsigned DPHY_HS_TX_STATE_C:5;
	unsigned rsv21:11;
};


struct DSI_STATE_DBG1_REG {
	unsigned CTL_STATE_C:15;
	unsigned rsv15:1;
	unsigned HS_TX_STATE_0:5;
	unsigned rsv21:3;
	unsigned ESC_STATE_0:8;
};


struct DSI_STATE_DBG2_REG {
	unsigned RX_ESC_STATE:10;
	unsigned rsv10:6;
	unsigned TA_T2R_STATE:5;
	unsigned rsv21:3;
	unsigned TA_R2T_STATE:5;
	unsigned rsv29:3;
};


struct DSI_STATE_DBG3_REG {
	unsigned CTL_STATE_1:5;
	unsigned rsv5:3;
	unsigned HS_TX_STATE_1:5;
	unsigned rsv13:3;
	unsigned CTL_STATE_2:5;
	unsigned rsv21:3;
	unsigned HS_TX_STATE_2:5;
	unsigned rsv29:3;
};


struct DSI_STATE_DBG4_REG {
	unsigned CTL_STATE_3:5;
	unsigned rsv5:3;
	unsigned HS_TX_STATE_3:5;
	unsigned rsv13:19;
};


struct DSI_STATE_DBG5_REG {
	unsigned TIMER_COUNTER:16;
	unsigned TIMER_BUSY:1;
	unsigned rsv17:11;
	unsigned WAKEUP_STATE:4;
};


struct DSI_STATE_DBG6_REG {
	unsigned CMTRL_STATE:15;
	unsigned rsv15:1;
	unsigned CMDQ_STATE:7;
	unsigned rsv23:9;
};


struct DSI_STATE_DBG7_REG {
	unsigned VMCTL_STATE:11;
	unsigned rsv11:1;
	unsigned VFP_PERIOD:1;
	unsigned VACT_PERIOD:1;
	unsigned VBP_PERIOD:1;
	unsigned VSA_PERIOD:1;
	unsigned EVEN_FIELD:1;
	unsigned rsv17:15;
};


struct DSI_STATE_DBG8_REG {
	unsigned WORD_COUNTER:14;
	unsigned rsv14:2;
	unsigned PREFETCH_COUNTER:15;
	unsigned DSI_PREFETCH_MUTEX:1;
};


struct DSI_STATE_DBG9_REG {
	unsigned LINE_COUNTER:22;
	unsigned rsv22:10;
};


struct DSI_DEBUG_SEL_REG {
	unsigned DEBUG_OUT_SEL:5;
	unsigned rsv5:3;
	unsigned CHKSUM_REC_EN:1;
	unsigned C2V_START_CON:1;
	unsigned rsv10:2;
	unsigned DYNAMIC_CG_CON:20;
};

struct DSI_STATE_DBG10_REG {
	unsigned LIMIT_W:15;
	unsigned rsv15:1;
	unsigned LIMIT_H:15;
	unsigned rsv31:1;
};


struct DSI_BIST_CON_REG {
	unsigned BIST_MODE:1;
	unsigned BIST_ENABLE:1;
	unsigned BIST_FIX_PATTERN:1;
	unsigned BIST_SPC_PATTERN:1;
	unsigned BIST_HS_FREE:1;
	unsigned rsv_05:1;
	unsigned SELF_PAT_MODE:1;
	unsigned rsv_07:9;
	unsigned BIST_TIMING:8;
	unsigned rsv24:8;
};


struct DSI_SHADOW_DBG_REG {
	unsigned FORCE_COMMIT:1;
	unsigned BYPASS_SHADOW:1;
	unsigned READ_WORKING:1;
	unsigned rsv_3:29;
};

struct DSI_SHADOW_STA_REG {
	unsigned VACT_UPDATE_ERR:1;
	unsigned VFP_UPDATE_ERR:1;
	unsigned rsv_2:30;
};

struct DSI_MMCLK_STALL_DBG1_REG {
	unsigned CHECKER_EN:1;
	unsigned CHECKER_MODE:1;
	unsigned rsv_2:6;
	unsigned CHECKER_THREASHOLD:7;
	unsigned rsv_15:17;
};

struct DSI_MMCLK_STALL_DBG2_REG {
	unsigned ASYNC_FIFO_RD_PTR_BINARY_MM_DOMAIN:6;
	unsigned rsv_6:2;
	unsigned ASYNC_FIFO_WR_PTR_BINARY_MM_DOMAIN:6;
	unsigned rsv_14:2;
	unsigned TIMES_OF_MMCLK_STALL_END:2;
	unsigned rsv_18:2;
	unsigned TIMES_OF_MMCLK_STALL_START:2;
	unsigned rsv_22:2;
	unsigned TIMES_OF_MMCLK_STALL_BUFFER_UNDER_RUN:2;
	unsigned rsv_26:6;
};

struct DSI_MMCLK_STALL_DBG3_REG {
	unsigned ASYNC_FIFO_RD_PTR_BINARY_DSI_DOMAIN_AT_STALL_START:6;
	unsigned rsv_6:2;
	unsigned ASYNC_FIFO_WR_PTR_BINARY_DSI_DOMAIN_AT_STALL_START:6;
	unsigned rsv_14:2;
	unsigned ASYNC_FIFO_RD_PTR_BINARY_DSI_DOMAIN_AT_STALL_END:6;
	unsigned rsv_22:2;
	unsigned ASYNC_FIFO_WR_PTR_BINARY_DSI_DOMAIN_AT_STALL_END:6;
	unsigned rsv_30:2;
};

struct DSI_MMCLK_STALL_DBG4_REG {
	unsigned ASYNC_FIFO_RD_PTR_BINARY_DSI_DOMAIN:6;
	unsigned rsv_6:2;
	unsigned ASYNC_FIFO_WR_PTR_BINARY_DSI_DOMAIN:6;
	unsigned rsv_14:2;
	unsigned COUNTER_AT_MMCLK_BUFFER_UNDER_RUN:7;
	unsigned rsv_23:1;
	unsigned COUNTER_AT_MMCLK_STALL_END:7;
	unsigned rsv_31:1;
};

struct DSI_INPUT_SETTING_REG {
	unsigned INP_REDUNDANT_PROCESSING_EN:1;
	unsigned rsv_1:31;
};

struct DSI_INPUT_DBG_REG {
	unsigned INP_PIXEL_COUNT:13;
	unsigned rsv_13:3;
	unsigned INP_LINE_COUNT:13;
	unsigned rsv_29:1;
	unsigned INP_END_SYNC_TO_DSI:1;
	unsigned INP_REDUNDANT_REGION:1;
};

struct DSI_REGS {
	struct DSI_START_REG DSI_START;	/* 0000 */
	struct DSI_STATUS_REG DSI_STA;	/* 0004 */
	struct DSI_INT_ENABLE_REG DSI_INTEN;	/* 0008 */
	struct DSI_INT_STATUS_REG DSI_INTSTA;	/* 000C */
	struct DSI_COM_CTRL_REG DSI_COM_CTRL;	/* 0010 */
	struct DSI_MODE_CTRL_REG DSI_MODE_CTRL;	/* 0014 */
	struct DSI_TXRX_CTRL_REG DSI_TXRX_CTRL;	/* 0018 */
	struct DSI_PSCTRL_REG DSI_PSCTRL;	/* 001C */
	struct DSI_VSA_NL_REG DSI_VSA_NL;	/* 0020 */
	struct DSI_VBP_NL_REG DSI_VBP_NL;	/* 0024 */
	struct DSI_VFP_NL_REG DSI_VFP_NL;	/* 0028 */
	struct DSI_VACT_NL_REG DSI_VACT_NL;	/* 002C */
	struct DSI_LFR_CON_REG DSI_LFR_CON;	/* 0030 */
	struct DSI_LFR_STA_REG DSI_LFR_STA;	/* 0034 */
	struct DSI_SIZE_CON_REG DSI_SIZE_CON;	/* 0038 */
	struct DSI_VFP_EARLY_STOP_REG DSI_VFP_EARLY_STOP;  /* 003C */
	UINT32 rsv_40[4];	/* 0040..004C */
	struct DSI_HSA_WC_REG DSI_HSA_WC;	/* 0050 */
	struct DSI_HBP_WC_REG DSI_HBP_WC;	/* 0054 */
	struct DSI_HFP_WC_REG DSI_HFP_WC;	/* 0058 */
	struct DSI_BLLP_WC_REG DSI_BLLP_WC;	/* 005C */
	struct DSI_CMDQ_CTRL_REG DSI_CMDQ_SIZE;	/* 0060 */
	struct DSI_HSTX_CKLP_REG DSI_HSTX_CKL_WC;	/* 0064 */
	struct DSI_HSTX_CKLP_WC_AUTO_RESULT_REG DSI_HSTX_CKL_WC_AUTO_RESULT;	/* 0068 */
	UINT32 rsv_006C[2];	/* 006c..0070 */
	struct DSI_RX_DATA_REG DSI_RX_DATA0;	/* 0074 */
	struct DSI_RX_DATA_REG DSI_RX_DATA1;	/* 0078 */
	struct DSI_RX_DATA_REG DSI_RX_DATA2;	/* 007c */
	struct DSI_RX_DATA_REG DSI_RX_DATA3;	/* 0080 */
	struct DSI_RACK_REG DSI_RACK;	/* 0084 */
	struct DSI_TRIG_STA_REG DSI_TRIG_STA;	/* 0088 */
	UINT32 rsv_008C;	/* 008C */
	struct DSI_MEM_CONTI_REG DSI_MEM_CONTI;	/* 0090 */
	struct DSI_FRM_BC_REG DSI_FRM_BC;	/* 0094 */
	struct DSI_3D_CON_REG DSI_3D_CON;	/* 0098 */
	UINT32 rsv_009C;	/* 009c */
	struct DSI_TIME_CON0_REG DSI_TIME_CON0;	/* 00A0 */
	struct DSI_TIME_CON1_REG DSI_TIME_CON1;	/* 00A4 */
	struct DSI_TIME_CON2_REG DSI_TIME_CON2;	/* 00A8 */

	UINT32 rsv_00AC[18];	/* 0AC..0F0 */
	UINT32 DSI_RESERVED; /* 00F0 */
	UINT32 rsv_00F4[3];      /* 0F4..0FC*/
	UINT32 DSI_PHY_PCPAT;	/* 00100 */

	struct DSI_PHY_LCCON_REG DSI_PHY_LCCON;	/* 0104 */
	struct DSI_PHY_LD0CON_REG DSI_PHY_LD0CON;	/* 0108 */
	struct DSI_PHY_SYNCON_REG DSI_PHY_SYNCON;	/* 010C */
	struct DSI_PHY_TIMCON0_REG DSI_PHY_TIMECON0;	/* 0110 */
	struct DSI_PHY_TIMCON1_REG DSI_PHY_TIMECON1;	/* 0114 */
	struct DSI_PHY_TIMCON2_REG DSI_PHY_TIMECON2;	/* 0118 */
	struct DSI_PHY_TIMCON3_REG DSI_PHY_TIMECON3;	/* 011C */
	UINT32 rsv_0120[4];	/* 0120..012c */
	struct DSI_VM_CMD_CON_REG DSI_VM_CMD_CON;	/* 0130 */
	UINT32 DSI_VM_CMD_DATA0;	/* 0134 */
	UINT32 DSI_VM_CMD_DATA4;	/* 0138 */
	UINT32 DSI_VM_CMD_DATA8;	/* 013C */
	UINT32 DSI_VM_CMD_DATAC;	/* 0140 */
	struct DSI_CKSM_OUT_REG DSI_CKSM_OUT;	/* 0144 */
	struct DSI_STATE_DBG0_REG DSI_STATE_DBG0;	/* 0148 */
	struct DSI_STATE_DBG1_REG DSI_STATE_DBG1;	/* 014C */
	struct DSI_STATE_DBG2_REG DSI_STATE_DBG2;	/* 0150 */
	struct DSI_STATE_DBG3_REG DSI_STATE_DBG3;	/* 0154 */
	struct DSI_STATE_DBG4_REG DSI_STATE_DBG4;	/* 0158 */
	struct DSI_STATE_DBG5_REG DSI_STATE_DBG5;	/* 015C */
	struct DSI_STATE_DBG6_REG DSI_STATE_DBG6;	/* 0160 */
	struct DSI_STATE_DBG7_REG DSI_STATE_DBG7;	/* 0164 */
	struct DSI_STATE_DBG8_REG DSI_STATE_DBG8;	/* 0168 */
	struct DSI_STATE_DBG9_REG DSI_STATE_DBG9;	/* 016C */
	struct DSI_DEBUG_SEL_REG DSI_DEBUG_SEL;	/* 0170 */
	struct DSI_STATE_DBG10_REG DSI_STATE_DBG10;	/* 0174 */
	UINT32 DSI_BIST_PATTERN;	/* 0178 */
	struct DSI_BIST_CON_REG DSI_BIST_CON;	/* 017C */
	UINT32 DSI_VM_CMD_DATA10;	/* 0180 */
	UINT32 DSI_VM_CMD_DATA14;	/* 0184 */
	UINT32 DSI_VM_CMD_DATA18;	/* 0188 */
	UINT32 DSI_VM_CMD_DATA1C;	/* 018C */
	struct DSI_SHADOW_DBG_REG DSI_SHADOW_DBG;   /* 0198 */
	struct DSI_SHADOW_STA_REG DSI_SHADOW_STA;   /* 019C */
	UINT32 DSI_VM_CMD_DATA20;	/* 01A0 */
	UINT32 DSI_VM_CMD_DATA24;	/* 01A4 */
	UINT32 DSI_VM_CMD_DATA28;	/* 01A8 */
	UINT32 DSI_VM_CMD_DATA2C;	/* 01AC */
	UINT32 DSI_VM_CMD_DATA30;	/* 01B0 */
	UINT32 DSI_VM_CMD_DATA34;	/* 01B4 */
	UINT32 DSI_VM_CMD_DATA38;	/* 01B8 */
	UINT32 DSI_VM_CMD_DATA3C;	/* 01BC */
	struct DSI_MMCLK_STALL_DBG1_REG DSI_MMCLK_STALL_DBG1; /* 01C0 */
	struct DSI_MMCLK_STALL_DBG2_REG DSI_MMCLK_STALL_DBG2; /* 01C4 */
	struct DSI_MMCLK_STALL_DBG3_REG DSI_MMCLK_STALL_DBG3; /* 01C8 */
	struct DSI_MMCLK_STALL_DBG4_REG DSI_MMCLK_STALL_DBG4; /* 01CC */
	struct DSI_INPUT_SETTING_REG DSI_INPUT_SETTING; /* 01D0 */
	struct DSI_INPUT_DBG_REG DSI_INPUT_DBG;         /* 01D4 */
};

/* 0~1 TYPE ,2 BTA,3 HS, 4 CL,5 TE,6~7 RESV, 8~15 DATA_ID,16~23 DATA_0,24~31 DATA_1 */
struct DSI_CMDQ {
	unsigned char byte0;
	unsigned char byte1;
	unsigned char byte2;
	unsigned char byte3;
};

struct DSI_CMDQ_REGS {
	struct DSI_CMDQ data[128];
};

struct DSI_VM_CMDQ {
	unsigned char byte0;
	unsigned char byte1;
	unsigned char byte2;
	unsigned char byte3;
};

struct DSI_VM_CMDQ_REGS {
	struct DSI_VM_CMDQ data[4];
};

struct DSI_PHY_REGS {
	struct MIPITX_DSI_IMPENDANCE_0_REG MIPITX_DSI_IMPENDANCE_0;	/* 0000 */
	struct MIPITX_DSI_IMPENDANCE_1_REG MIPITX_DSI_IMPENDANCE_1;	/* 0004 */
	struct MIPITX_DSI_IMPENDANCE_2_REG MIPITX_DSI_IMPENDANCE_2;	/* 0008 */
	struct MIPITX_DSI_LANE_CON_REG MIPITX_DSI_LANE_CON;		/* 000C */
	struct MIPITX_DSI_VOLTAGE_SEL_REG MIPITX_DSI_VOLTAGE_SEL;	/* 0010 */
	struct MIPITX_DSI_PRESERVED_REG MIPITX_DSI_PRESERVED;		/* 0014 */
	struct MIPITX_DSI_TEST_CON_REG MIPITX_DSI_TEST_CON;		/* 0018 */
	UINT32 rsv_1C[3];	/* 001C..0024 */
	struct MIPITX_DSI_PLL_PWR_REG MIPITX_DSI_PLL_PWR;		/* 0028 */
	struct MIPITX_DSI_PLL_CON0_REG MIPITX_DSI_PLL_CON0;		/* 002C */
	struct MIPITX_DSI_PLL_CON1_REG MIPITX_DSI_PLL_CON1;		/* 0030 */
	struct MIPITX_DSI_PLL_CON2_REG MIPITX_DSI_PLL_CON2;		/* 0034 */
	struct MIPITX_DSI_PLL_CON3_REG MIPITX_DSI_PLL_CON3;		/* 0038 */
	struct MIPITX_DSI_PLL_CON4_REG MIPITX_DSI_PLL_CON4;		/* 003C */
	struct MIPITX_DSI_PHY_SEL0_REG MIPITX_DSI_PHY_SEL0;		/* 0040 */
	struct MIPITX_DSI_PHY_SEL1_REG MIPITX_DSI_PHY_SEL1;		/* 0044 */
	struct MIPITX_DSI_PHY_SEL2_REG MIPITX_DSI_PHY_SEL2;		/* 0048 */
	struct MIPITX_DSI_PHY_SEL3_REG MIPITX_DSI_PHY_SEL3;		/* 004C */
	struct MIPITX_DSI_SW_CTRL_CON4_REG MIPITX_DSI_SW_CTRL_CON4;	/* 0060 */
	UINT32 rsv_64;	/* 0064 */
	struct MIPITX_DSI_CD_CON_REG MIPITX_DSI_CD_CON;		/* 0068 */
	UINT32 rsv_6C;	/* 006C */
	struct MIPITX_DSI_DBG_CON_REG MIPITX_DSI_DBG_CON;		/* 0070 */
	UINT32 MIPITX_DSI_DBG_OUT;				/* 0074 */
	struct MIPITX_DSI_APB_ASYNC_STA_REG MIPITX_DSI_APB_ASYNC_STA;	/* 0078 */
	UINT32 rsv_7C;	/* 007C */
	struct MIPITX_DSI_BIST_CON_REG MIPITX_DSI_BIST_CON;		/* 0080 */
	UINT32 MIPITX_DSI_BIST_PAT;				/* 0084 */
	UINT32 rsv_88[2];	/* 0088..008C */
	struct MIPITX_CPHY_BIST_CON0_REG MIPITX_CPHY_BIST_CON0;	/* 0090 */
	UINT32 MIPITX_CPHY_BIST_CON1;				/* 0094 */
	struct MIPITX_CPHY_BIST_CON2_REG MIPITX_CPHY_BIST_CON2;	/* 0098 */
	struct MIPITX_CPHY_BIST_CON3_REG MIPITX_CPHY_BIST_CON3;	/* 009c */
};

#if 0
/* mipi and dsi's extern variable */
extern volatile unsigned long dispsys_reg[DISP_REG_NUM];
extern unsigned long ddp_reg_pa_base[DISP_REG_NUM];
extern volatile unsigned long mipi_tx_reg;
extern volatile unsigned long dsi_reg_va;
#endif
/* DTS will assign reigister address dynamically, so can not define to 0x1000 */
/* #define DISP_INDEX_OFFSET 0x1000 */

#define DISP_RDMA_INDEX_OFFSET  (dispsys_reg[DISP_REG_RDMA1] - dispsys_reg[DISP_REG_RDMA0])

#define DDP_REG_BASE_MMSYS_CONFIG  dispsys_reg[DISP_REG_CONFIG]
#define DDP_REG_BASE_DISP_OVL0     dispsys_reg[DISP_REG_OVL0]
#define DDP_REG_BASE_DISP_OVL0_2L  dispsys_reg[DISP_REG_OVL0_2L]
#define DDP_REG_BASE_DISP_OVL1_2L  dispsys_reg[DISP_REG_OVL1_2L]
#define DDP_REG_BASE_DISP_RDMA0    dispsys_reg[DISP_REG_RDMA0]
#define DDP_REG_BASE_DISP_RDMA1    dispsys_reg[DISP_REG_RDMA1]
#define DDP_REG_BASE_DISP_WDMA0    dispsys_reg[DISP_REG_WDMA0]
#define DDP_REG_BASE_DISP_COLOR0   dispsys_reg[DISP_REG_COLOR0]
#define DDP_REG_BASE_DISP_CCORR0   dispsys_reg[DISP_REG_CCORR0]
#define DDP_REG_BASE_DISP_AAL0     dispsys_reg[DISP_REG_AAL0]
#define DDP_REG_BASE_DISP_GAMMA0   dispsys_reg[DISP_REG_GAMMA0]
#define DDP_REG_BASE_DISP_DITHER0  dispsys_reg[DISP_REG_DITHER0]
#define DDP_REG_BASE_DISP_UFOE     dispsys_reg[DISP_REG_UFOE]
#define DDP_REG_BASE_DISP_SPLIT0   dispsys_reg[DISP_REG_SPLIT0]
#define DDP_REG_BASE_DSI0          dispsys_reg[DISP_REG_DSI0]
#define DDP_REG_BASE_DPI           dispsys_reg[DISP_REG_DPI0]
#define DDP_REG_BASE_DISP_PWM0     dispsys_reg[DISP_REG_PWM0]
#define DDP_REG_BASE_MM_MUTEX      dispsys_reg[DISP_REG_MUTEX]
#define DDP_REG_BASE_DISP_RSZ0     dispsys_reg[DISP_REG_RSZ0]
#define DDP_REG_BASE_SMI_LARB0     dispsys_reg[DISP_REG_SMI_LARB0]
#define DDP_REG_BASE_SMI_LARB1     dispsys_reg[DISP_REG_SMI_LARB1]
#define DDP_REG_BASE_SMI_COMMON    dispsys_reg[DISP_REG_SMI_COMMON]

#define DDP_REG_BASE_MIPITX0       dispsys_reg[DISP_REG_MIPI0]
#define DDP_REG_BASE_MIPITX1       dispsys_reg[DISP_REG_MIPI1]

//#define DISPSYS_REG_ADDR_MIN            (DISPSYS_CONFIG_BASE)
//#define DISPSYS_REG_ADDR_MAX            (DDP_REG_BASE_DISP_OVL1_2L)

#define DISPSYS_CONFIG_BASE			DDP_REG_BASE_MMSYS_CONFIG
#define DISPSYS_OVL0_BASE		        DDP_REG_BASE_DISP_OVL0
//#define DISPSYS_OVL1_BASE		        DDP_REG_BASE_DISP_OVL1
#define DISPSYS_OVL0_2L_BASE		    DDP_REG_BASE_DISP_OVL0_2L
#define DISPSYS_OVL1_2L_BASE		    DDP_REG_BASE_DISP_OVL1_2L
#define DISPSYS_RDMA0_BASE		        DDP_REG_BASE_DISP_RDMA0
#define DISPSYS_RDMA1_BASE		        DDP_REG_BASE_DISP_RDMA1
//#define DISPSYS_RDMA2_BASE		        DDP_REG_BASE_DISP_RDMA2
#define DISPSYS_WDMA0_BASE		        DDP_REG_BASE_DISP_WDMA0
//#define DISPSYS_WDMA1_BASE		        DDP_REG_BASE_DISP_WDMA1
#define DISPSYS_COLOR0_BASE		        DDP_REG_BASE_DISP_COLOR0
//#define DISPSYS_COLOR1_BASE		        DDP_REG_BASE_DISP_COLOR1
#define DISPSYS_CCORR0_BASE		        DDP_REG_BASE_DISP_CCORR0
//#define DISPSYS_CCORR1_BASE		        DDP_REG_BASE_DISP_CCORR1
#define DISPSYS_AAL0_BASE		        DDP_REG_BASE_DISP_AAL0
//#define DISPSYS_AAL1_BASE		        DDP_REG_BASE_DISP_AAL1
#define DISPSYS_GAMMA0_BASE		        DDP_REG_BASE_DISP_GAMMA0
//#define DISPSYS_GAMMA1_BASE		        DDP_REG_BASE_DISP_GAMMA1
//#define DISPSYS_OD_BASE                 DDP_REG_BASE_DISP_OD
#define DISPSYS_DITHER0_BASE		    DDP_REG_BASE_DISP_DITHER0
//#define DISPSYS_DITHER1_BASE		    DDP_REG_BASE_DISP_DITHER1
#define DISPSYS_UFOE_BASE		        DDP_REG_BASE_DISP_UFOE
//#define DISPSYS_DSC_BASE		        DDP_REG_BASE_DISP_DSC
//#define DISPSYS_DSC_2ND_BASE			DDP_REG_BASE_DISP_DSC_2ND
#define DISPSYS_SPLIT0_BASE		        DDP_REG_BASE_DISP_SPLIT0
#define DISPSYS_DSI0_BASE		        DDP_REG_BASE_DSI0
//#define DISPSYS_DSI1_BASE		        DDP_REG_BASE_DSI1
#define DISPSYS_DPI_BASE				DDP_REG_BASE_DPI
#define DISPSYS_PWM0_BASE		        DDP_REG_BASE_DISP_PWM0
//#define DISPSYS_PWM1_BASE		        DDP_REG_BASE_DISP_PWM1
#define DISPSYS_MUTEX_BASE			DDP_REG_BASE_MM_MUTEX
#define DISPSYS_RSZ0_BASE				DDP_REG_BASE_DISP_RSZ0
//#define DISPSYS_RSZ1_BASE				DDP_REG_BASE_DISP_RSZ1
#define DISPSYS_SMI_LARB0_BASE		    DDP_REG_BASE_SMI_LARB0
#define DISPSYS_SMI_LARB1_BASE		    DDP_REG_BASE_SMI_LARB1
#define DISPSYS_SMI_COMMON_BASE		    DDP_REG_BASE_SMI_COMMON

/* --------------------------------------------------------------------------- */
/* Register Field Access */
/* --------------------------------------------------------------------------- */

#define REG_FLD(width, shift) \
	((unsigned int)((((width) & 0xFF) << 16) | ((shift) & 0xFF)))

#define REG_FLD_MSB_LSB(msb, lsb) REG_FLD((msb) - (lsb) + 1, (lsb))

#define REG_FLD_WIDTH(field) \
	((unsigned int)(((field) >> 16) & 0xFF))

#define REG_FLD_SHIFT(field) \
	((unsigned int)((field) & 0xFF))

#define REG_FLD_MASK(field) \
	((unsigned int)((1ULL << REG_FLD_WIDTH(field)) - 1) << REG_FLD_SHIFT(field))

#define REG_FLD_VAL(field, val) \
	(((val) << REG_FLD_SHIFT(field)) & REG_FLD_MASK(field))

#define REG_FLD_VAL_GET(field, regval) \
	(((regval) & REG_FLD_MASK(field)) >> REG_FLD_SHIFT(field))

#define mt_reg_sync_writel(val, reg32)	(*(volatile unsigned int*)(reg32) = (val))
#define DISP_REG_GET(reg32) 	(*(volatile unsigned int*)(reg32))
#define DISP_REG_GET_FIELD(field, reg32) ((*(volatile unsigned int*)(reg32)\
		& REG_FLD_MASK(field)) >> REG_FLD_SHIFT(field))
#define INREG32(reg)	DISP_REG_GET(reg)


/* polling register until masked bit is 1 */
#define DDP_REG_POLLING(reg32, mask) \
	do { \
		while (!((DISP_REG_GET(reg32))&mask))\
			; \
	} while (0)

/* Polling register until masked bit is 0 */
#define DDP_REG_POLLING_NEG(reg32, mask) \
	do { \
		while ((DISP_REG_GET(reg32))&mask)\
			; \
	} while (0)

#define DISP_CPU_REG_SET(reg32, val) \
	do {\
		if (0) \
		mt_reg_sync_writel(val, (volatile unsigned long*)(reg32));\
	} while (0)

/* after apply device tree va/pa is not mapped by a fixed offset */
static inline unsigned long disp_addr_convert(unsigned long va)
{
	unsigned int i = 0;

	for (i = 0; i < DISP_REG_NUM; i++) {
		if (dispsys_reg[i] == (va & (~0xfffl)))
			return ddp_reg_pa_base[i] + (va & 0xfffl);
	}
	DDPAEE("DDP/can not find reg addr for va=0x%lx!\n", va);
	return 0;
}

#define DISP_REG_MASK_EXT(handle, reg_va, reg_pa, val, mask) 	\
	do { \
	   if(handle==NULL) \
	   { \
			 mt_reg_sync_writel((unsigned int)(DISP_REG_GET(reg_va)&~(mask))|(val),(volatile void*)(reg_va) );\
	   } \
	   else \
	   { \
			 cmdqRecWrite(handle, reg_pa, val, mask); \
	   }  \
	} while (0)


#define DISP_REG_MASK(handle, reg32, val, mask)	\
	do { \
		 if (handle == NULL) { \
			mt_reg_sync_writel((unsigned int)(INREG32(reg32)&~(mask))|(val), (reg32));\
		 } else { \
			cmdqRecWrite(handle, disp_addr_convert((unsigned long)(reg32)), val, mask); \
		 }	\
	} while (0)

#define DISP_REG_SET_PA(handle,reg32, val) \
	do { \
	  if(handle==0) \
	  { \
		 mt_reg_sync_writel(val, (volatile unsigned long*)(reg32));\
	  } \
	  else \
	  { \
		 cmdqRecWrite(handle, reg32, val, ~0); \
	  }  \
	} while (0)


#define DISP_REG_SET(handle, reg32, val) \
	do { \
		if (handle == NULL) { \
			mt_reg_sync_writel(val, (volatile unsigned long*)(reg32));\
		} else { \
			cmdqRecWrite(handle, disp_addr_convert((unsigned long)(reg32)), val, ~0); \
		}  \
	} while (0)


#define DISP_REG_SET_FIELD(handle, field, reg32, val)  \
	do {  \
		if (handle == NULL) { \
			unsigned int regval; \
			regval = (*(volatile unsigned int*)(reg32)); \
			regval  = regval & ~REG_FLD_MASK(field) | REG_FLD_VAL((field), (val)); \
			mt_reg_sync_writel(regval, (reg32));  \
		} else { \
			cmdqRecWrite(handle, disp_addr_convert(reg32), val<<REG_FLD_SHIFT(field), REG_FLD_MASK(field));\
		} \
	} while (0)

#define DISP_REG_CMDQ_POLLING(handle, reg32, val, mask) \
	do { \
		if (handle == NULL) { \
			while (!((DISP_REG_GET(reg32))&val))\
				; \
		} else { \
			cmdqRecPoll(handle, disp_addr_convert((phys_addr_t)(reg32)), val, mask); \
		}  \
	} while (0)

#define DISP_REG_BACKUP(handle, hSlot, idx, reg32) \
	do { \
		if (handle != NULL) { \
			if (hSlot) \
				cmdqRecBackupRegisterToSlot(handle, hSlot, idx,\
							    disp_addr_convert((unsigned long)(reg32)));\
		}  \
	} while (0)

/* field definition */
/* ------------------------------------------------------------- */
/* AAL */
#define DISP_AAL_EN                             (DISPSYS_AAL0_BASE + 0x000)
#define DISP_AAL_RESET                          (DISPSYS_AAL0_BASE + 0x004)
#define DISP_AAL_INTEN                          (DISPSYS_AAL0_BASE + 0x008)
#define DISP_AAL_INTSTA                         (DISPSYS_AAL0_BASE + 0x00c)
#define DISP_AAL_STATUS                         (DISPSYS_AAL0_BASE + 0x010)
#define DISP_AAL_CFG                            (DISPSYS_AAL0_BASE + 0x020)
#define DISP_AAL_IN_CNT                         (DISPSYS_AAL0_BASE + 0x024)
#define DISP_AAL_OUT_CNT                        (DISPSYS_AAL0_BASE + 0x028)
#define DISP_AAL_CHKSUM                         (DISPSYS_AAL0_BASE + 0x02c)
#define DISP_AAL_SIZE                           (DISPSYS_AAL0_BASE + 0x030)
#define DISP_AAL_SHADOW_CTL                     (DISPSYS_AAL0_BASE + 0x0B0)
#define DISP_AAL_DUMMY_REG                      (DISPSYS_AAL0_BASE + 0x0c0)
#define DISP_AAL_MAX_HIST_CONFIG_00             (DISPSYS_AAL0_BASE + 0x204)
#define DISP_AAL_CABC_00                        (DISPSYS_AAL0_BASE + 0x20c)
#define DISP_AAL_CABC_02                        (DISPSYS_AAL0_BASE + 0x214)
#define DISP_AAL_CABC_04                        (DISPSYS_AAL0_BASE + 0x21c)
#define DISP_AAL_STATUS_00                      (DISPSYS_AAL0_BASE + 0x224)
/* 00 ~ 32: max histogram */
#define DISP_AAL_STATUS_32                      (DISPSYS_AAL0_BASE + 0x2a4)
/* bit 8: dre_gain_force_en */
#define DISP_AAL_DRE_GAIN_FILTER_00             (DISPSYS_AAL0_BASE + 0x354)
#define DISP_AAL_DRE_FLT_FORCE(idx)             (DISPSYS_AAL0_BASE + 0x358 + (idx) * 4)
#define DISP_AAL_DRE_CRV_CAL_00                 (DISPSYS_AAL0_BASE + 0x344)
#define DISP_AAL_DRE_MAPPING_00                 (DISPSYS_AAL0_BASE + 0x3b0)
#define DISP_AAL_CABC_GAINLMT_TBL(idx)          (DISPSYS_AAL0_BASE + 0x40c + (idx) * 4)
#define DISP_AAL_DRE_FLT_FORCE_11             (DISPSYS_AAL0_BASE + 0x44C)
#define DISP_AAL_DRE_FLT_FORCE_12               (DISPSYS_AAL0_BASE + 0x450)

#define DISP_PWM_EN_OFF                         (0x00)
#define DISP_PWM_RST_RX                         (0x04)
#define DISP_PWM_COMMIT_OFF                     (0x0C)
#define DISP_PWM_CON_0_OFF                      (0x18)
#define DISP_PWM_CON_1_OFF                      (0x1C)
#define DISP_PWM_DEBUG                          (0x80)

/* field definition */
/* ------------------------------------------------------------- */
/* COLOR */
#define CFG_MAIN_FLD_M_REG_RESET                 REG_FLD(1, 31)
#define CFG_MAIN_FLD_M_DISP_RESET                REG_FLD(1, 30)
#define CFG_MAIN_FLD_COLOR_DBUF_EN               REG_FLD(1, 29)
#define CFG_MAIN_FLD_C_PP_CM_DBG_SEL             REG_FLD(4, 16)
#define CFG_MAIN_FLD_SEQ_SEL                     REG_FLD(1, 13)
#define CFG_MAIN_FLD_ALLBP                       REG_FLD(1, 7)
#define CFG_MAIN_FLD_HEBP                        REG_FLD(1, 4)
#define CFG_MAIN_FLD_SEBP                        REG_FLD(1, 3)
#define CFG_MAIN_FLD_YEBP                        REG_FLD(1, 2)
#define CFG_MAIN_FLD_P2CBP                       REG_FLD(1, 1)
#define CFG_MAIN_FLD_C2PBP                       REG_FLD(1, 0)
#define START_FLD_DISP_COLOR_START               REG_FLD(1, 0)

#define DISP_COLOR_CFG_MAIN             (DISPSYS_COLOR0_BASE+0x400)
#define DISP_COLOR_PXL_CNT_MAIN         (DISPSYS_COLOR0_BASE+0x404)
#define DISP_COLOR_LINE_CNT_MAIN        (DISPSYS_COLOR0_BASE+0x408)
#define DISP_COLOR_WIN_X_MAIN           (DISPSYS_COLOR0_BASE+0x40C)
#define DISP_COLOR_WIN_Y_MAIN           (DISPSYS_COLOR0_BASE+0x410)
#define DISP_COLOR_TIMING_DETECTION_0   (DISPSYS_COLOR0_BASE+0x418)
#define DISP_COLOR_TIMING_DETECTION_1   (DISPSYS_COLOR0_BASE+0x41c)
#define DISP_COLOR_DBG_CFG_MAIN         (DISPSYS_COLOR0_BASE+0x420)
#define DISP_COLOR_C_BOOST_MAIN         (DISPSYS_COLOR0_BASE+0x428)
#define DISP_COLOR_C_BOOST_MAIN_2       (DISPSYS_COLOR0_BASE+0x42C)
#define DISP_COLOR_LUMA_ADJ             (DISPSYS_COLOR0_BASE+0x430)
#define DISP_COLOR_G_PIC_ADJ_MAIN_1     (DISPSYS_COLOR0_BASE+0x434)
#define DISP_COLOR_G_PIC_ADJ_MAIN_2     (DISPSYS_COLOR0_BASE+0x438)
#define DISP_COLOR_POS_MAIN             (DISPSYS_COLOR0_BASE+0x484)
#define DISP_COLOR_INK_DATA_MAIN        (DISPSYS_COLOR0_BASE+0x488)
#define DISP_COLOR_INK_DATA_MAIN_CR     (DISPSYS_COLOR0_BASE+0x48c)
#define DISP_COLOR_CAP_IN_DATA_MAIN     (DISPSYS_COLOR0_BASE+0x490)
#define DISP_COLOR_CAP_IN_DATA_MAIN_CR  (DISPSYS_COLOR0_BASE+0x494)
#define DISP_COLOR_CAP_OUT_DATA_MAIN    (DISPSYS_COLOR0_BASE+0x498)
#define DISP_COLOR_CAP_OUT_DATA_MAIN_CR (DISPSYS_COLOR0_BASE+0x49c)
#define DISP_COLOR_Y_SLOPE_1_0_MAIN     (DISPSYS_COLOR0_BASE+0x4A0)
#define DISP_COLOR_LOCAL_HUE_CD_0       (DISPSYS_COLOR0_BASE+0x620)
#define DISP_COLOR_TWO_D_WINDOW_1       (DISPSYS_COLOR0_BASE+0x740)
#define DISP_COLOR_TWO_D_W1_RESULT      (DISPSYS_COLOR0_BASE+0x74C)
#define DISP_COLOR_SAT_HIST_X_CFG_MAIN  (DISPSYS_COLOR0_BASE+0x768)
#define DISP_COLOR_SAT_HIST_Y_CFG_MAIN  (DISPSYS_COLOR0_BASE+0x76c)
#define DISP_COLOR_BWS_2                (DISPSYS_COLOR0_BASE+0x79c)
#define DISP_COLOR_CRC_0                (DISPSYS_COLOR0_BASE+0x7e0)
#define DISP_COLOR_PART_SAT_GAIN1_0     (DISPSYS_COLOR0_BASE+0x7FC)
#define DISP_COLOR_PART_SAT_GAIN1_1     (DISPSYS_COLOR0_BASE+0x800)
#define DISP_COLOR_PART_SAT_GAIN1_2     (DISPSYS_COLOR0_BASE+0x804)
#define DISP_COLOR_PART_SAT_GAIN1_3     (DISPSYS_COLOR0_BASE+0x808)
#define DISP_COLOR_PART_SAT_GAIN1_4     (DISPSYS_COLOR0_BASE+0x80C)
#define DISP_COLOR_PART_SAT_GAIN2_0     (DISPSYS_COLOR0_BASE+0x810)
#define DISP_COLOR_PART_SAT_GAIN2_1     (DISPSYS_COLOR0_BASE+0x814)
#define DISP_COLOR_PART_SAT_GAIN2_2     (DISPSYS_COLOR0_BASE+0x818)
#define DISP_COLOR_PART_SAT_GAIN2_3	    (DISPSYS_COLOR0_BASE+0x81C)
#define DISP_COLOR_PART_SAT_GAIN2_4     (DISPSYS_COLOR0_BASE+0x820)
#define DISP_COLOR_PART_SAT_GAIN3_0     (DISPSYS_COLOR0_BASE+0x824)
#define DISP_COLOR_PART_SAT_GAIN3_1     (DISPSYS_COLOR0_BASE+0x828)
#define DISP_COLOR_PART_SAT_GAIN3_2     (DISPSYS_COLOR0_BASE+0x82C)
#define DISP_COLOR_PART_SAT_GAIN3_3     (DISPSYS_COLOR0_BASE+0x830)
#define DISP_COLOR_PART_SAT_GAIN3_4     (DISPSYS_COLOR0_BASE+0x834)
#define DISP_COLOR_PART_SAT_POINT1_0    (DISPSYS_COLOR0_BASE+0x838)
#define DISP_COLOR_PART_SAT_POINT1_1    (DISPSYS_COLOR0_BASE+0x83C)
#define DISP_COLOR_PART_SAT_POINT1_2    (DISPSYS_COLOR0_BASE+0x840)
#define DISP_COLOR_PART_SAT_POINT1_3    (DISPSYS_COLOR0_BASE+0x844)
#define DISP_COLOR_PART_SAT_POINT1_4    (DISPSYS_COLOR0_BASE+0x848)
#define DISP_COLOR_PART_SAT_POINT2_0    (DISPSYS_COLOR0_BASE+0x84C)
#define DISP_COLOR_PART_SAT_POINT2_1    (DISPSYS_COLOR0_BASE+0x850)
#define DISP_COLOR_PART_SAT_POINT2_2    (DISPSYS_COLOR0_BASE+0x854)
#define DISP_COLOR_PART_SAT_POINT2_3    (DISPSYS_COLOR0_BASE+0x858)
#define DISP_COLOR_PART_SAT_POINT2_4    (DISPSYS_COLOR0_BASE+0x85C)

#define DISP_COLOR_CM_CONTROL           (DISPSYS_COLOR0_BASE+0x860)
#define DISP_COLOR_CM_W1_HUE_0          (DISPSYS_COLOR0_BASE+0x864)
#define DISP_COLOR_CM_W1_HUE_1          (DISPSYS_COLOR0_BASE+0x868)
#define DISP_COLOR_CM_W1_HUE_2          (DISPSYS_COLOR0_BASE+0x86C)
#define DISP_COLOR_CM_W1_HUE_3          (DISPSYS_COLOR0_BASE+0x870)
#define DISP_COLOR_CM_W1_HUE_4          (DISPSYS_COLOR0_BASE+0x874)
#define DISP_COLOR_CM_W1_LUMA_0         (DISPSYS_COLOR0_BASE+0x878)
#define DISP_COLOR_CM_W1_LUMA_1         (DISPSYS_COLOR0_BASE+0x87C)
#define DISP_COLOR_CM_W1_LUMA_2         (DISPSYS_COLOR0_BASE+0x880)
#define DISP_COLOR_CM_W1_LUMA_3         (DISPSYS_COLOR0_BASE+0x884)
#define DISP_COLOR_CM_W1_LUMA_4         (DISPSYS_COLOR0_BASE+0x888)
#define DISP_COLOR_CM_W1_SAT_0          (DISPSYS_COLOR0_BASE+0x88C)
#define DISP_COLOR_CM_W1_SAT_1          (DISPSYS_COLOR0_BASE+0x890)
#define DISP_COLOR_CM_W1_SAT_2          (DISPSYS_COLOR0_BASE+0x894)
#define DISP_COLOR_CM_W1_SAT_3          (DISPSYS_COLOR0_BASE+0x898)
#define DISP_COLOR_CM_W1_SAT_4          (DISPSYS_COLOR0_BASE+0x89C)
#define DISP_COLOR_CM_W2_HUE_0          (DISPSYS_COLOR0_BASE+0x8A0)
#define DISP_COLOR_CM_W2_HUE_1          (DISPSYS_COLOR0_BASE+0x8A4)
#define DISP_COLOR_CM_W2_HUE_2          (DISPSYS_COLOR0_BASE+0x8A8)
#define DISP_COLOR_CM_W2_HUE_3          (DISPSYS_COLOR0_BASE+0x8AC)
#define DISP_COLOR_CM_W2_HUE_4          (DISPSYS_COLOR0_BASE+0x8B0)
#define DISP_COLOR_CM_W2_LUMA_0         (DISPSYS_COLOR0_BASE+0x8B4)
#define DISP_COLOR_CM_W2_LUMA_1         (DISPSYS_COLOR0_BASE+0x8B8)
#define DISP_COLOR_CM_W2_LUMA_2         (DISPSYS_COLOR0_BASE+0x8BC)
#define DISP_COLOR_CM_W2_LUMA_3         (DISPSYS_COLOR0_BASE+0x8C0)
#define DISP_COLOR_CM_W2_LUMA_4         (DISPSYS_COLOR0_BASE+0x8C4)
#define DISP_COLOR_CM_W2_SAT_0          (DISPSYS_COLOR0_BASE+0x8C8)
#define DISP_COLOR_CM_W2_SAT_1          (DISPSYS_COLOR0_BASE+0x8CC)
#define DISP_COLOR_CM_W2_SAT_2          (DISPSYS_COLOR0_BASE+0x8D0)
#define DISP_COLOR_CM_W2_SAT_3          (DISPSYS_COLOR0_BASE+0x8D4)
#define DISP_COLOR_CM_W2_SAT_4          (DISPSYS_COLOR0_BASE+0x8D8)
#define DISP_COLOR_CM_W3_HUE_0          (DISPSYS_COLOR0_BASE+0x8DC)
#define DISP_COLOR_CM_W3_HUE_1          (DISPSYS_COLOR0_BASE+0x8E0)
#define DISP_COLOR_CM_W3_HUE_2          (DISPSYS_COLOR0_BASE+0x8E4)
#define DISP_COLOR_CM_W3_HUE_3          (DISPSYS_COLOR0_BASE+0x8E8)
#define DISP_COLOR_CM_W3_HUE_4          (DISPSYS_COLOR0_BASE+0x8EC)
#define DISP_COLOR_CM_W3_LUMA_0         (DISPSYS_COLOR0_BASE+0x8F0)
#define DISP_COLOR_CM_W3_LUMA_1         (DISPSYS_COLOR0_BASE+0x8F4)
#define DISP_COLOR_CM_W3_LUMA_2         (DISPSYS_COLOR0_BASE+0x8F8)
#define DISP_COLOR_CM_W3_LUMA_3         (DISPSYS_COLOR0_BASE+0x8FC)
#define DISP_COLOR_CM_W3_LUMA_4         (DISPSYS_COLOR0_BASE+0x900)
#define DISP_COLOR_CM_W3_SAT_0          (DISPSYS_COLOR0_BASE+0x904)
#define DISP_COLOR_CM_W3_SAT_1          (DISPSYS_COLOR0_BASE+0x908)
#define DISP_COLOR_CM_W3_SAT_2          (DISPSYS_COLOR0_BASE+0x90C)
#define DISP_COLOR_CM_W3_SAT_3          (DISPSYS_COLOR0_BASE+0x910)
#define DISP_COLOR_CM_W3_SAT_4          (DISPSYS_COLOR0_BASE+0x914)

#define DISP_COLOR_START                (DISPSYS_COLOR0_BASE+0xC00)
#define DISP_COLOR_INTEN                (DISPSYS_COLOR0_BASE+0xC04)
#define DISP_COLOR_OUT_SEL              (DISPSYS_COLOR0_BASE+0xC08)
#define DISP_COLOR_FRAME_DONE_DEL       (DISPSYS_COLOR0_BASE+0xC10)
#define DISP_COLOR_CRC                  (DISPSYS_COLOR0_BASE+0xC14)
#define DISP_COLOR_SW_SCRATCH           (DISPSYS_COLOR0_BASE+0xC18)
#define DISP_COLOR_CK_ON                (DISPSYS_COLOR0_BASE+0xC28)
#define DISP_COLOR_INTERNAL_IP_WIDTH    (DISPSYS_COLOR0_BASE+0xC50)
#define DISP_COLOR_INTERNAL_IP_HEIGHT   (DISPSYS_COLOR0_BASE+0xC54)
#define DISP_COLOR_CM1_EN               (DISPSYS_COLOR0_BASE+0xC60)
#define DISP_COLOR_CM2_EN               (DISPSYS_COLOR0_BASE+0xCA0)
#define DISP_COLOR_SHADOW_CTRL          (DISPSYS_COLOR0_BASE+0xCB0)
#define DISP_COLOR_R0_CRC               (DISPSYS_COLOR0_BASE+0xCF0)
#define DISP_COLOR_S_GAIN_BY_Y0_0       (DISPSYS_COLOR0_BASE+0xCF4)
#define DISP_COLOR_S_GAIN_BY_Y0_1       (DISPSYS_COLOR0_BASE+0xCF8)
#define DISP_COLOR_S_GAIN_BY_Y0_2       (DISPSYS_COLOR0_BASE+0xCFC)
#define DISP_COLOR_S_GAIN_BY_Y0_3       (DISPSYS_COLOR0_BASE+0xD00)
#define DISP_COLOR_S_GAIN_BY_Y0_4       (DISPSYS_COLOR0_BASE+0xD04)
#define DISP_COLOR_S_GAIN_BY_Y64_0      (DISPSYS_COLOR0_BASE+0xD08)
#define DISP_COLOR_S_GAIN_BY_Y64_1      (DISPSYS_COLOR0_BASE+0xD0C)
#define DISP_COLOR_S_GAIN_BY_Y64_2      (DISPSYS_COLOR0_BASE+0xD10)
#define DISP_COLOR_S_GAIN_BY_Y64_3      (DISPSYS_COLOR0_BASE+0xD14)
#define DISP_COLOR_S_GAIN_BY_Y64_4      (DISPSYS_COLOR0_BASE+0xD18)
#define DISP_COLOR_S_GAIN_BY_Y128_0     (DISPSYS_COLOR0_BASE+0xD1C)
#define DISP_COLOR_S_GAIN_BY_Y128_1     (DISPSYS_COLOR0_BASE+0xD20)
#define DISP_COLOR_S_GAIN_BY_Y128_2     (DISPSYS_COLOR0_BASE+0xD24)
#define DISP_COLOR_S_GAIN_BY_Y128_3     (DISPSYS_COLOR0_BASE+0xD28)
#define DISP_COLOR_S_GAIN_BY_Y128_4     (DISPSYS_COLOR0_BASE+0xD2C)
#define DISP_COLOR_S_GAIN_BY_Y192_0     (DISPSYS_COLOR0_BASE+0xD30)
#define DISP_COLOR_S_GAIN_BY_Y192_1     (DISPSYS_COLOR0_BASE+0xD34)
#define DISP_COLOR_S_GAIN_BY_Y192_2     (DISPSYS_COLOR0_BASE+0xD38)
#define DISP_COLOR_S_GAIN_BY_Y192_3     (DISPSYS_COLOR0_BASE+0xD3C)
#define DISP_COLOR_S_GAIN_BY_Y192_4     (DISPSYS_COLOR0_BASE+0xD40)
#define DISP_COLOR_S_GAIN_BY_Y256_0     (DISPSYS_COLOR0_BASE+0xD44)
#define DISP_COLOR_S_GAIN_BY_Y256_1     (DISPSYS_COLOR0_BASE+0xD48)
#define DISP_COLOR_S_GAIN_BY_Y256_2     (DISPSYS_COLOR0_BASE+0xD4C)
#define DISP_COLOR_S_GAIN_BY_Y256_3     (DISPSYS_COLOR0_BASE+0xD50)
#define DISP_COLOR_S_GAIN_BY_Y256_4     (DISPSYS_COLOR0_BASE+0xD54)
#define DISP_COLOR_LSP_1                (DISPSYS_COLOR0_BASE+0xD58)
#define DISP_COLOR_LSP_2                (DISPSYS_COLOR0_BASE+0xD5C)

/* field definition */
/* ------------------------------------------------------------- */
/* Config */
#define DISP_REG_CONFIG_MMSYS_INTEN				(DISPSYS_CONFIG_BASE + 0x0)
#define DISP_REG_CONFIG_MMSYS_INTSTA				(DISPSYS_CONFIG_BASE + 0x4)
#define DISP_REG_CONFIG_ISP_APB_TX_CON				(DISPSYS_CONFIG_BASE + 0x8)
#define DISP_REG_CONFIG_MJC_APB_TX_CON				(DISPSYS_CONFIG_BASE + 0xc)
#define DISP_REG_CONFIG_CAM_APB_TX_CON				(DISPSYS_CONFIG_BASE + 0x010)
#define DISP_REG_CONFIG_IPU_APB_TX_CON				(DISPSYS_CONFIG_BASE + 0x014)
#define DISP_REG_CONFIG_VDEC_APB_TX_CON				(DISPSYS_CONFIG_BASE + 0x018)
#define DISP_REG_CONFIG_VENC_APB_TX_CON				(DISPSYS_CONFIG_BASE + 0x01C)
#define DISP_REG_CONFIG_MMSYS_MOUT_RST				(DISPSYS_CONFIG_BASE + 0x048)
#define DISP_REG_CONFIG_MMSYS_MISC				(DISPSYS_CONFIG_BASE + 0x0F0)
#define DISP_REG_CONFIG_MMSYS_SMI_LARB_SEL			(DISPSYS_CONFIG_BASE + 0x0F4)
#define DISP_REG_CONFIG_MMSYS_SODI_REQ_MASK			(DISPSYS_CONFIG_BASE + 0x0F8)
#define DISP_REG_CONFIG_MMSYS_CG_CON0				(DISPSYS_CONFIG_BASE + 0x100)
#define DISP_REG_CONFIG_MMSYS_CG_SET0				(DISPSYS_CONFIG_BASE + 0x104)
#define DISP_REG_CONFIG_MMSYS_CG_CLR0				(DISPSYS_CONFIG_BASE + 0x108)
#define DISP_REG_CONFIG_MMSYS_CG_CON1				(DISPSYS_CONFIG_BASE + 0x110)
#define DISP_REG_CONFIG_MMSYS_CG_SET1				(DISPSYS_CONFIG_BASE + 0x114)
#define DISP_REG_CONFIG_MMSYS_CG_CLR1				(DISPSYS_CONFIG_BASE + 0x118)
#define DISP_REG_CONFIG_MMSYS_HW_DCM_DIS0			(DISPSYS_CONFIG_BASE + 0x120)
#define DISP_REG_CONFIG_MMSYS_HW_DCM_DIS_SET0			(DISPSYS_CONFIG_BASE + 0x124)
#define DISP_REG_CONFIG_MMSYS_HW_DCM_DIS_CLR0			(DISPSYS_CONFIG_BASE + 0x128)
#define DISP_REG_CONFIG_MMSYS_HW_DCM_DIS1			(DISPSYS_CONFIG_BASE + 0x130)
#define DISP_REG_CONFIG_MMSYS_HW_DCM_DIS_SET1			(DISPSYS_CONFIG_BASE + 0x134)
#define DISP_REG_CONFIG_MMSYS_HW_DCM_DIS_CLR1			(DISPSYS_CONFIG_BASE + 0x138)
#define DISP_REG_CONFIG_MMSYS_SW0_RST_B			(DISPSYS_CONFIG_BASE + 0x140)
#define DISP_REG_CONFIG_MMSYS_SW1_RST_B			(DISPSYS_CONFIG_BASE + 0x144)
#define DISP_REG_CONFIG_MMSYS_LCM_RST_B			(DISPSYS_CONFIG_BASE + 0x150)

#define DISP_REG_CONFIG_SMI_N21MUX_CFG_WR		(DISPSYS_CONFIG_BASE + 0x168)
#define DISP_REG_CONFIG_SMI_N21MUX_CFG_RD		(DISPSYS_CONFIG_BASE + 0x16c)
#define DISP_REG_CONFIG_ELA2GMC_BASE_ADDR		(DISPSYS_CONFIG_BASE + 0x170)
#define DISP_REG_CONFIG_ELA2GMC_BASE_ADDR_END		(DISPSYS_CONFIG_BASE + 0x174)
#define DISP_REG_CONFIG_ELA2GMC_FINAL_ADDR		(DISPSYS_CONFIG_BASE + 0x178)
#define DISP_REG_CONFIG_ELA2GMC_STATUS			(DISPSYS_CONFIG_BASE + 0x17c)
#define DISP_REG_CONFIG_LARB6_AXI_ASIF_CFG_WD		(DISPSYS_CONFIG_BASE + 0x180)
#define DISP_REG_CONFIG_LARB6_AXI_ASIF_CFG_RD		(DISPSYS_CONFIG_BASE + 0x184)
#define DISP_REG_CONFIG_PROC_TRACK_EMI_BUSY_CON		(DISPSYS_CONFIG_BASE + 0x190)

#define DISP_REG_CONFIG_DISP_FAKE_ENG_EN		(DISPSYS_CONFIG_BASE + 0x200)
#define DISP_REG_CONFIG_DISP_FAKE_ENG_RST			(DISPSYS_CONFIG_BASE + 0x204)
#define DISP_REG_CONFIG_DISP_FAKE_ENG_CON0			(DISPSYS_CONFIG_BASE + 0x208)
#define DISP_REG_CONFIG_DISP_FAKE_ENG_CON1			(DISPSYS_CONFIG_BASE + 0x20c)
#define DISP_REG_CONFIG_DISP_FAKE_ENG_RD_ADDR			(DISPSYS_CONFIG_BASE + 0x210)
#define DISP_REG_CONFIG_DISP_FAKE_ENG_WR_ADDR			(DISPSYS_CONFIG_BASE + 0x214)
#define DISP_REG_CONFIG_DISP_FAKE_ENG_STATE			(DISPSYS_CONFIG_BASE + 0x218)
#define DISP_REG_CONFIG_DISP_FAKE2_ENG_EN			(DISPSYS_CONFIG_BASE + 0x220)
#define DISP_REG_CONFIG_DISP_FAKE2_ENG_RST			(DISPSYS_CONFIG_BASE + 0x224)
#define DISP_REG_CONFIG_DISP_FAKE2_ENG_CON0			(DISPSYS_CONFIG_BASE + 0x228)
#define DISP_REG_CONFIG_DISP_FAKE2_ENG_CON1			(DISPSYS_CONFIG_BASE + 0x22c)
#define DISP_REG_CONFIG_DISP_FAKE2_ENG_RD_ADDR			(DISPSYS_CONFIG_BASE + 0x230)
#define DISP_REG_CONFIG_DISP_FAKE2_ENG_WR_ADDR			(DISPSYS_CONFIG_BASE + 0x234)
#define DISP_REG_CONFIG_DISP_FAKE2_ENG_STATE			(DISPSYS_CONFIG_BASE + 0x238)

#define DISP_REG_CONFIG_MMSYS_MBIST_CON				(DISPSYS_CONFIG_BASE + 0x800)
#define DISP_REG_CONFIG_MMSYS_MBIST_DONE			(DISPSYS_CONFIG_BASE + 0x804)
#define DISP_REG_CONFIG_MMSYS_MBIST_HOLDB			(DISPSYS_CONFIG_BASE + 0x808)
#define DISP_REG_CONFIG_MMSYS_MBIST_MODE			(DISPSYS_CONFIG_BASE + 0x80c)
#define DISP_REG_CONFIG_MMSYS_MBIST_FAIL0			(DISPSYS_CONFIG_BASE + 0x810)
#define DISP_REG_CONFIG_MMSYS_MBIST_FAIL1			(DISPSYS_CONFIG_BASE + 0x814)
#define DISP_REG_CONFIG_MMSYS_MBIST_FAIL2			(DISPSYS_CONFIG_BASE + 0x818)
#define DISP_REG_CONFIG_MMSYS_MBIST_FAIL3			(DISPSYS_CONFIG_BASE + 0x81c)
#define DISP_REG_CONFIG_MMSYS_MBIST_DEBUG		(DISPSYS_CONFIG_BASE + 0x820)
#define DISP_REG_CONFIG_MMSYS_MBIST_DIAG_SCANOUT	(DISPSYS_CONFIG_BASE + 0x824)
#define DISP_REG_CONFIG_MMSYS_MBIST_PRE_FUSE		(DISPSYS_CONFIG_BASE + 0x828)

#define DISP_REG_CONFIG_MMSYS_MBIST_BSEL0		(DISPSYS_CONFIG_BASE + 0x82c)
#define DISP_REG_CONFIG_MMSYS_MBIST_BSEL1		(DISPSYS_CONFIG_BASE + 0x830)
#define DISP_REG_CONFIG_MMSYS_MBIST_BSEL2		(DISPSYS_CONFIG_BASE + 0x834)
#define DISP_REG_CONFIG_MMSYS_MBIST_BSEL3		(DISPSYS_CONFIG_BASE + 0x838)
#define DISP_REG_CONFIG_MMSYS_MBIST_HDEN		(DISPSYS_CONFIG_BASE + 0x83C)

#define DISP_REG_CONFIG_MDP_RDMA0_MEM_DELSEL		(DISPSYS_CONFIG_BASE + 0x840)
#define DISP_REG_CONFIG_MDP_RDMA1_MEM_DELSEL		(DISPSYS_CONFIG_BASE + 0x844)
#define DISP_REG_CONFIG_MDP_RSZ_MEM_DELSEL		(DISPSYS_CONFIG_BASE + 0x848)

#define DISP_REG_CONFIG_MDP_TDSHP_MEM_DELSEL		(DISPSYS_CONFIG_BASE + 0x84c)
/*#define DISP_REG_CONFIG_MDP_WDMA_MEM_DELSEL		(DISPSYS_CONFIG_BASE + 0x850)*/
#define DISP_REG_CONFIG_MDP_WROT0_MEM_DELSEL		(DISPSYS_CONFIG_BASE + 0x854)
#define DISP_REG_CONFIG_MDP_WDMA_MEM_DELSEL		(DISPSYS_CONFIG_BASE + 0x858)

#define DISP_REG_CONFIG_DISP_OVL_MEM_DELSEL		(DISPSYS_CONFIG_BASE + 0x85c)
#define DISP_REG_CONFIG_DISP_OVL_2L_MEM_DELSEL		(DISPSYS_CONFIG_BASE + 0x860)
#define DISP_REG_CONFIG_DISP_RDMA_MEM_DELSEL		(DISPSYS_CONFIG_BASE + 0x864)
#define DISP_REG_CONFIG_DISP_WDMA0_MEM_DELSEL		(DISPSYS_CONFIG_BASE + 0x868)
#define DISP_REG_CONFIG_DISP_WDMA1_MEM_DELSEL		(DISPSYS_CONFIG_BASE + 0x86c)
#define DISP_REG_CONFIG_DISP_GAMMA_MEM_DELSEL		(DISPSYS_CONFIG_BASE + 0x870)
#define DISP_REG_CONFIG_DSI_MEM_DELSEL			(DISPSYS_CONFIG_BASE + 0x874)
#define DISP_REG_CONFIG_DISP_SPLIT_MEM_DELSEL		(DISPSYS_CONFIG_BASE + 0x878)
#define DISP_REG_CONFIG_DISP_DSC_MEM_DELSEL		(DISPSYS_CONFIG_BASE + 0x87C)
#define DISP_REG_CONFIG_DISP_OD_MEM_DELSEL0		(DISPSYS_CONFIG_BASE + 0x880)
#define DISP_REG_CONFIG_DISP_OD_MEM_DELSEL1		(DISPSYS_CONFIG_BASE + 0x884)
#define DISP_REG_CONFIG_DISP_OD_MEM_DELSEL2		(DISPSYS_CONFIG_BASE + 0x888)

#define DISP_REG_CONFIG_MMSYS_DEBUG_OUT_SEL		(DISPSYS_CONFIG_BASE + 0x88C)
#define DISP_REG_CONFIG_MMSYS_MBIST_RP_RST_B		(DISPSYS_CONFIG_BASE + 0x890)
#define DISP_REG_CONFIG_MMSYS_MBIST_RP_FAIL0		(DISPSYS_CONFIG_BASE + 0x894)
#define DISP_REG_CONFIG_MMSYS_MBIST_RP_FAIL1		(DISPSYS_CONFIG_BASE + 0x898)
#define DISP_REG_CONFIG_MMSYS_MBIST_RP_OK0		(DISPSYS_CONFIG_BASE + 0x89c)
#define DISP_REG_CONFIG_MMSYS_MBIST_RP_OK1		(DISPSYS_CONFIG_BASE + 0x8A0)

#define DISP_REG_CONFIG_MMSYS_DUMMY0			(DISPSYS_CONFIG_BASE + 0x8A4)
#define DISP_REG_CONFIG_MMSYS_DUMMY1			(DISPSYS_CONFIG_BASE + 0x8A8)
#define DISP_REG_CONFIG_MMSYS_DUMMY2			(DISPSYS_CONFIG_BASE + 0x8AC)
#define DISP_REG_CONFIG_MMSYS_DUMMY3			(DISPSYS_CONFIG_BASE + 0x8B0)

#define DISP_REG_CONFIG_DISP_DL_VALID_0			(DISPSYS_CONFIG_BASE + 0x8B4)
#define DISP_REG_CONFIG_DISP_DL_VALID_1			(DISPSYS_CONFIG_BASE + 0x8B8)
#define DISP_REG_CONFIG_DISP_DL_VALID_2			(DISPSYS_CONFIG_BASE + 0x8BC)

#define DISP_REG_CONFIG_DISP_DL_READY_0			(DISPSYS_CONFIG_BASE + 0x8C0)
#define DISP_REG_CONFIG_DISP_DL_READY_1			(DISPSYS_CONFIG_BASE + 0x8C4)
#define DISP_REG_CONFIG_DISP_DL_READY_2			(DISPSYS_CONFIG_BASE + 0x8C8)

#define DISP_REG_CONFIG_MDP_DL_VALID_0			(DISPSYS_CONFIG_BASE + 0x8CC)
#define DISP_REG_CONFIG_MDP_DL_VALID_1			(DISPSYS_CONFIG_BASE + 0x8D0)
#define DISP_REG_CONFIG_MDP_DL_READY_0			(DISPSYS_CONFIG_BASE + 0x8D4)
#define DISP_REG_CONFIG_MDP_DL_READY_1			(DISPSYS_CONFIG_BASE + 0x8D8)
#define DISP_REG_CONFIG_SMI_LARB0_GREQ			(DISPSYS_CONFIG_BASE + 0x8DC)

#define DISP_REG_CONFIG_DISP_MOUT_MASK			(DISPSYS_CONFIG_BASE + 0x8E0)
#define DISP_REG_CONFIG_DISP_MOUT_MASK1			(DISPSYS_CONFIG_BASE + 0x8E4)
#define DISP_REG_CONFIG_MDP_MOUT_MASK				(DISPSYS_CONFIG_BASE + 0x8E8)
#define DISP_REG_CONFIG_POWER_READ				(DISPSYS_CONFIG_BASE + 0x8EC)
#define DISP_REG_CONFIG_HRT_WEIGHT_READ				(DISPSYS_CONFIG_BASE + 0x8F0)

#define DISP_REG_CONFIG_DISP_OVL0_MOUT_EN		(DISPSYS_CONFIG_BASE + 0xF00)
#define DISP_REG_CONFIG_DISP_OVL0_2L_MOUT_EN	(DISPSYS_CONFIG_BASE + 0xF04)
#define DISP_REG_CONFIG_DISP_OVL1_2L_MOUT_EN	(DISPSYS_CONFIG_BASE + 0xF08)
#define DISP_REG_CONFIG_DISP_DITHER_MOUT_EN		(DISPSYS_CONFIG_BASE + 0xF0C)
#define DISP_REG_CONFIG_DISP_RSZ_MOUT_EN		(DISPSYS_CONFIG_BASE + 0xF10)
#define DISP_REG_CONFIG_DISP_COLOR_OUT_SEL_IN	(DISPSYS_CONFIG_BASE + 0xF20)
#define DISP_REG_CONFIG_DISP_PATH0_SEL_IN		(DISPSYS_CONFIG_BASE + 0xF24)
#define DISP_REG_CONFIG_DISP_WDMA0_PRE_SEL_IN	(DISPSYS_CONFIG_BASE + 0xF28)
#define DISP_REG_CONFIG_DISP_DSI0_SEL_IN		(DISPSYS_CONFIG_BASE + 0xF2C)
#define DISP_REG_CONFIG_DISP_DPI0_SEL_IN		(DISPSYS_CONFIG_BASE + 0xF30)

#define DISP_REG_CONFIG_DISP_OVLTOWDMA_SEL_IN	(DISPSYS_CONFIG_BASE + 0xF40)
#define DISP_REG_CONFIG_DISP_RSZ_SEL_IN			(DISPSYS_CONFIG_BASE + 0xF48)
#define DISP_REG_CONFIG_DISP_RDMA0_SOUT_SEL_IN	(DISPSYS_CONFIG_BASE + 0xF50)
#define DISP_REG_CONFIG_DISP_RDMA1_SOUT_SEL_IN	(DISPSYS_CONFIG_BASE + 0xF54)
#define DISP_REG_CONFIG_DISP_WDMA0_SEL_IN		(DISPSYS_CONFIG_BASE + 0xFCC)


/* #define DISP_REG_CONFIG_C08					(DISPSYS_CONFIG_BASE + 0xc08) */
/* #define DISP_REG_CLK_CFG_0_MM_CLK				(DISPSYS_CONFIG_BASE + 0x40) */
/* #define DISP_REG_CLK_CFG_0_CLR				(DISPSYS_CONFIG_BASE + 0x48) */
/* #define DISP_REG_CLK_CFG_1_CLR				(DISPSYS_CONFIG_BASE + 0x58) */
/* #define DISP_REG_CLK_CFG_6_DPI				(DISPSYS_CONFIG_BASE + 0xA0) */
/* #define DISP_REG_CLK_CFG_6_CLR				(DISPSYS_CONFIG_BASE + 0xA8) */
/* #define DISP_REG_VENCPLL_CON0				0xf0209260  // bit0 1 */

#define MMSYS_INTEN_FLD_MMSYS_INTEN				REG_FLD(8, 0)
#define MMSYS_INSTA_FLD_MMSYS_INSTA				REG_FLD(1, 0)


#define MMSYS_MOUT_RST_FLD_MMSYS_MOUT_RST				REG_FLD(16, 0)

#define MMSYS_MISC_FLD_SMI_LARB0_TEST_MODE					    REG_FLD(1, 0)

#define MMSYS_SMI_LARB_SEL_FLD_DISP_OVL0_2L_LARB_SEL		REG_FLD(1, 0)
#define MMSYS_SMI_LARB_SEL_FLD_DISP_WDMA0_LARB_SEL		REG_FLD(1, 1)
#define MMSYS_SODI_REQ_MASK_FLD_MMSYS_SODI_REQ_MASK		REG_FLD(5, 0)
#define MMSYS_SODI_REQ_MASK_FLD_DVFS_HALT_MASK			REG_FLD(5, 16)
#define MMSYS_CG_CON0_FLD_CG0                                   REG_FLD(32, 0)
#define MMSYS_CG_SET0_FLD_CG0                                   REG_FLD(32, 0)
#define MMSYS_CG_CLR0_FLD_CG0                                   REG_FLD(32, 0)
#define MMSYS_CG_CON1_FLD_CG1                                   REG_FLD(32, 0)
#define MMSYS_CG_SET1_FLD_CG1                                   REG_FLD(32, 0)
#define MMSYS_CG_CLR1_FLD_CG1                                   REG_FLD(32, 0)
#define MMSYS_CG_CON2_FLD_CG2                                   REG_FLD(32, 0)
#define MMSYS_CG_SET2_FLD_CG2                                   REG_FLD(32, 0)
#define MMSYS_CG_CLR2_FLD_CG2                                   REG_FLD(32, 0)
#define MMSYS_HW_DCM_DIS0_FLD_DCM_DIS0                          REG_FLD(32, 0)
#define MMSYS_HW_DCM_DIS_SET0_FLD_DCM_DIS0                      REG_FLD(32, 0)
#define MMSYS_HW_DCM_DIS_CLR0_FLD_DCM_DIS0                      REG_FLD(32, 0)
#define MMSYS_HW_DCM_DIS1_FLD_DCM_DIS1                          REG_FLD(32, 0)
#define MMSYS_HW_DCM_DIS_SET1_FLD_DCM_DIS1                      REG_FLD(32, 0)
#define MMSYS_HW_DCM_DIS_CLR1_FLD_DCM_DIS1                      REG_FLD(32, 0)
#define MMSYS_HW_DCM_DIS2_FLD_DCM_DIS2                          REG_FLD(32, 0)
#define MMSYS_HW_DCM_DIS_SET2_FLD_DCM_DIS2                      REG_FLD(32, 0)
#define MMSYS_HW_DCM_DIS_CLR2_FLD_DCM_DIS2                      REG_FLD(32, 0)
#define MMSYS_HW_DCM_DIS3_FLD_DCM_DIS3                          REG_FLD(32, 0)
#define MMSYS_HW_DCM_DIS_SET3_FLD_DCM_DIS3                      REG_FLD(32, 0)
#define MMSYS_HW_DCM_DIS_CLR3_FLD_DCM_DIS3                      REG_FLD(32, 0)
#define MMSYS_SW0_RST_B_FLD_SW0_RST_B                           REG_FLD(32, 0)
#define MMSYS_SW1_RST_B_FLD_SW1_RST_B                           REG_FLD(32, 0)
#define MMSYS_LCM_RST_B_FLD_LCM_RST_B                           REG_FLD(1, 0)
#define MMSYS_LCM_RST_B_FLD_LCM1_RST_B                          REG_FLD(1, 1)
#define MMSYS_MBIST_CON_FLD_MMSYS_MBIST_BACKGROUND			REG_FLD(3, 16)
#define MMSYS_MBIST_CON_FLD_MMSYS_MBIST_RSTB			REG_FLD(1, 15)
#define MMSYS_MBIST_CON_FLD_MMSYS_MBIST_SCANOUT_SEL			REG_FLD(4, 8)
#define MMSYS_MBIST_CON_FLD_MMSYS_MBIST_DEBUG				REG_FLD(1, 7)
#define MMSYS_MBIST_CON_FLD_MMSYS_MBIST_FAILOUT_SEL			REG_FLD(6, 0)
#define MMSYS_MBIST_DONE_FLD_MMSYS_MBIST_DONE				REG_FLD(32, 0)

#define MMSYS_MBIST_HOLDB_FLD_MMSYS_MBIST_HOLDB			REG_FLD(32, 0)
#define MMSYS_MBIST_MODE_FLD_MMSYS_MBIST_MODE				REG_FLD(32, 0)
#define MMSYS_MBIST_FAIL0_FLD_MMSYS_MBIST_FAIL0				REG_FLD(32, 0)
#define MMSYS_MBIST_FAIL1_FLD_MMSYS_MBIST_FAIL1				REG_FLD(32, 0)
#define MMSYS_MBIST_FAIL2_FLD_MMSYS_MBIST_FAIL2				REG_FLD(32, 0)
#define MMSYS_MBIST_FAIL2_FLD_MMSYS_MBIST_FAIL3				REG_FLD(32, 0)
#define MMSYS_MBIST_DEBUG_FLD_MMSYS_MBIST_DEBUG				REG_FLD(8, 0)
#define MMSYS_MBIST_DIAG_SCANOUT_FLD_MMSYS_MBIST_DIAG_SCANOUT		REG_FLD(6, 0)

#define MMSYS_MBIST_BSEL0_FLD_MDP_TDSHP_MBIST_BSEL			REG_FLD(16, 0)
#define MMSYS_MBIST_BSEL0_FLD_MDP_RDMA0_MBIST_BSEL		REG_FLD(5, 16)
#define MMSYS_MBIST_BSEL1_FLD_MDP_RDMA1_MBIST_BSEL		REG_FLD(5, 0)
#define MMSYS_MBIST_BSEL1_FLD_DISP_RSZ0_MBIST_BSEL		REG_FLD(7, 8)
#define MMSYS_MBIST_BSEL1_FLD_MDP_WROT0_MBIST_BSEL		REG_FLD(8, 16)
#define MMSYS_MBIST_BSEL1_FLD_MDP_WROT1_MBIST_BSEL		REG_FLD(8, 24)

#define MMSYS_MBIST_BSEL3_FLD_MDP_RSZ2_MBIST_BSEL		REG_FLD(12, 0)
#define MMSYS_MBIST_BSEL3_FLD_DISP_UFOE_MBIST_BSEL		REG_FLD(6, 16)
#define MMSYS_MBIST_BSEL3_FLD_DISP_RSZ1_MBIST_BSEL		REG_FLD(7, 24)

#define MMSYS_MEM_DELSEL0_FLD_MDP_RSZ_MEM_DELSEL                REG_FLD(16, 0)
#define MMSYS_MEM_DELSEL0_FLD_MDP_RDMA_MEM_DELSEL               REG_FLD(14, 16)
#define MMSYS_MEM_DELSEL1_FLD_MDP_WROT_MEM_DELSEL				REG_FLD(22, 0)
#define MMSYS_MEM_DELSEL1_FLD_MDP_WDMA_MEM_DELSEL               REG_FLD(6, 22)
#define MMSYS_MEM_DELSEL1_FLD_MDP_TDSHP_MEM_DELSEL              REG_FLD(4, 28)
#define MMSYS_MBIST_HDEN_FLD_MMSYS_MBIST_HDEN			REG_FLD(1, 0)
#define DISP_OVL_MEM_DELSEL_FLD_DISP_OVL_MEM_DELSEL		REG_FLD(6, 0)
#define DISP_OVL_2L_MEM_DELSEL_FLD_DISP_OVL_2L_MEM_DELSEL	REG_FLD(6, 0)
#define DISP_RDMA_MEM_DELSEL_FLD_DISP_RDMA_MEM_DELSEL		REG_FLD(6, 0)
#define DISP_WDMA0_MEM_DELSEL_FLD_DISP_WDMA0_MEM_DELSEL		REG_FLD(12, 0)
#define DISP_WDMA1_MEM_DELSEL_FLD_DISP_WDMA1_MEM_DELSEL		REG_FLD(12, 0)
#define DISP_GAMMA_MEM_DELSEL_FLD_DISP_GAMMA_MEM_DELSEL		REG_FLD(6, 0)
#define DSI_MEM_DELSEL_FLD_DSI_MEM_DELSEL			REG_FLD(10, 0)
#define DISP_UFOE_MEM_DELSEL_FLD_DISP_UFOE_MEM_DELSEL		REG_FLD(18, 0)
#define DISP_DSC_MEM_DELSEL_FLD_DISP_DSC_MEM_DELSEL		REG_FLD(24, 0)
#define DISP_OD_MEM_DELSEL0_FLD_DISP_OD_MEM_DELSEL0		REG_FLD(30, 0)
#define DISP_OD_MEM_DELSEL1_FLD_DISP_OD_MEM_DELSEL1		REG_FLD(30, 0)
#define DISP_OD_MEM_DELSEL2_FLD_DISP_OD_MEM_DELSEL2		REG_FLD(30, 0)
#define DISP_RSZ_MEM_DELSEL_FLD_DISP_RSZ_MEM_DELSEL		REG_FLD(6, 0)
#define MMSYS_MBIST_RP_RST_B_FLD_MMSYS_MBIST_RP_RST_B		REG_FLD(1, 0)
#define MMSYS_DUMMY0_FLD_MMSYS_DUMMY0                           REG_FLD(32, 0)
#define MMSYS_DUMMY1_FLD_MMSYS_DUMMY1                           REG_FLD(32, 0)
#define MMSYS_DUMMY2_FLD_MMSYS_DUMMY2                           REG_FLD(32, 0)
#define MMSYS_DUMMY3_FLD_MMSYS_DUMMY3                           REG_FLD(32, 0)
#define DISP_DL_VALID_0_FLD_DISP_DL_VALID_0                     REG_FLD(32, 0)
#define DISP_DL_VALID_1_FLD_DISP_DL_VALID_1                     REG_FLD(32, 0)
#define DISP_DL_VALID_2_FLD_DISP_DL_VALID_2                     REG_FLD(32, 0)
#define DISP_DL_VALID_3_FLD_DISP_DL_VALID_3                     REG_FLD(32, 0)
#define DISP_DL_READY_0_FLD_DISP_DL_READY_0                     REG_FLD(32, 0)
#define DISP_DL_READY_1_FLD_DISP_DL_READY_1                     REG_FLD(32, 0)
#define DISP_DL_READY_2_FLD_DISP_DL_READY_2                     REG_FLD(32, 0)
#define DISP_DL_READY_3_FLD_DISP_DL_READY_3                     REG_FLD(32, 0)
#define SMI_LARB0_GREQ_FLD_SMI_LARB0_GREQ                       REG_FLD(8, 0)
#define SMI_LARB0_GREQ_FLD_SMI_LARB1_GREQ                       REG_FLD(10, 16)

#define DISP_MOUT_MASK0_FLD_DISP_MOUT_MASK0			REG_FLD(32, 0)
#define DISP_MOUT_MASK1_FLD_DISP_MOUT_MASK1			REG_FLD(32, 0)

#define MMSYS_POWER_READ_FLD_DISP_RDMA0_SMI_REQ			REG_FLD(1, 0)
#define MMSYS_POWER_READ_FLD_DISP_RDMA1_SMI_REQ			REG_FLD(1, 1)
#define MMSYS_POWER_READ_FLD_DISP_RDMA2_SMI_REQ			REG_FLD(1, 2)
#define MMSYS_POWER_READ_FLD_DISP_OD_R_SMI_REQ			REG_FLD(2, 4)
#define MMSYS_POWER_READ_FLD_DISP_RDMA0_HALT_FOR_DVFS		REG_FLD(1, 8)
#define MMSYS_POWER_READ_FLD_DISP_RDMA1_HALT_FOR_DVFS		REG_FLD(1, 9)
#define MMSYS_POWER_READ_FLD_DISP_RDMA2_HALT_FOR_DVFS		REG_FLD(1, 10)
#define MMSYS_POWER_READ_FLD_DISP_RDMA0_WROT_SHARE_EN		REG_FLD(1, 12)
#define MMSYS_POWER_READ_FLD_DISP_RDMA1_WROT_SHARE_EN		REG_FLD(1, 13)

#define MMSYS_DEBUG_OUT_SEL_FLD_MMSYS_DEBUG_OUT_SEL             REG_FLD(5, 0)
#define MMSYS_DEBUG_OUT_SEL_FLD_SUBSYS_DEBUG_OUT_SEL		REG_FLD(3, 8)

#define DISP_OVL0_MOUT_EN_FLD_DISP_OVL0_MOUT_EN			REG_FLD(8, 0)
#define DISP_OVL0_PQ_MOUT_EN_FLD_DISP_OVL0_PQ_MOUT_EN		REG_FLD(8, 0)
#define DISP_OVL1_MOUT_EN_FLD_DISP_OVL1_MOUT_EN			REG_FLD(8, 0)
#define DISP_OVL1_PQ_MOUT_EN_FLD_DISP_OVL1_PQ_MOUT_EN		REG_FLD(8, 0)
#define DISP_DITHER0_MOUT_EN_FLD_DISP_DITHER0_MOUT_EN		REG_FLD(8, 0)
#define DISP_DITHER1_MOUT_EN_FLD_DISP_DITHER1_MOUT_EN		REG_FLD(8, 0)
#define DISP_UFOE_MOUT_EN_FLD_DISP_UFOE_MOUT_EN			REG_FLD(8, 0)
#define DISP_DSC_MOUT_EN_FLD_DISP_DSC_MOUT_EN			REG_FLD(8, 0)
#define DISP_DSC_2ND_MOUT_EN_FLD_DISP_DSC_2ND_MOUT_EN		REG_FLD(8, 0)
#define DISP_COLOR0_SEL_IN_FLD_DISP_COLOR0_SEL_IN		REG_FLD(4, 0)
#define DISP_COLOR1_SEL_IN_FLD_DISP_COLOR1_SEL_IN		REG_FLD(4, 0)
#define DISP_WDMA0_SEL_IN_FLD_DISP_WDMA0_SEL_IN			REG_FLD(4, 0)
#define DISP_WDMA1_SEL_IN_FLD_DISP_WDMA1_SEL_IN			REG_FLD(4, 0)
#define DISP_UFOE_SEL_IN_FLD_DISP_UFOE_SEL_IN			REG_FLD(4, 0)
#define DISP_DSC_SEL_IN_FLD_DISP_DSC_SEL_IN			REG_FLD(4, 0)
#define DSI0_SEL_IN_FLD_DSI0_SEL_IN				REG_FLD(4, 0)
#define DSI1_SEL_IN_FLD_DSI1_SEL_IN				REG_FLD(4, 0)
#define DPI0_SEL_IN_FLD_DPI0_SEL_IN				REG_FLD(4, 0)
#define DISP_PATH0_SEL_IN_FLD_DISP_PATH0_SEL_IN			REG_FLD(4, 0)
#define DISP_PATH1_SEL_IN_FLD_DISP_PATH1_SEL_IN			REG_FLD(4, 0)
#define DISP_OVL0_SEL_IN_FLD_DISP_OVL0_SEL_IN			REG_FLD(4, 0)
#define DISP_OVL1_2L_SEL_IN_FLD_DISP_OVL1_2L_SEL_IN		REG_FLD(4, 0)
#define DISP_OVL0_2L_SEL_IN_FLD_DISP_OVL0_2L_SEL_IN		REG_FLD(4, 0)
#define DISP_RSZ0_SEL_IN_FLD_DISP_RSZ0_SEL_IN			REG_FLD(4, 0)
#define DISP_RSZ1_SEL_IN_FLD_DISP_RSZ1_SEL_IN			REG_FLD(4, 0)
#define DISP_AAL0_SEL_IN_FLD_DISP_AAL0_SEL_IN			REG_FLD(4, 0)
#define DISP_AAL1_SEL_IN_FLD_DISP_AAL1_SEL_IN			REG_FLD(4, 0)
#define DISP_PATH0_SOUT_SEL_IN_FLD_DISP_PATH0_SOUT_SEL_IN	REG_FLD(4, 0)
#define DISP_PATH1_SOUT_SEL_IN_FLD_DISP_PATH1_SOUT_SEL_IN	REG_FLD(4, 0)
#define DISP_RDMA0_SOUT_SEL_IN_FLD_DISP_RDMA0_SOUT_SEL_IN	REG_FLD(4, 0)
#define DISP_RDMA1_SOUT_SEL_IN_FLD_DISP_RDMA1_SOUT_SEL_IN	REG_FLD(4, 0)
#define DISP_RDMA2_SOUT_SEL_IN_FLD_DISP_RDMA2_SOUT_SEL_IN	REG_FLD(4, 0)

#define DISP_OVL0_SOUT_SEL_IN_FLD_DISP_OVL0_SOUT_SEL_IN		REG_FLD(4, 0)
#define DISP_RSZ0_SOUT_SEL_IN_FLD_DISP_RSZ0_SOUT_SEL_IN		REG_FLD(4, 0)
#define DISP_RSZ1_SOUT_SEL_IN_FLD_DISP_RSZ1_SOUT_SEL_IN		REG_FLD(4, 0)
#define DISP_CCORR0_SOUT_SEL_IN_FLD_DISP_CCORR0_SOUT_SEL_IN	REG_FLD(4, 0)
#define DISP_CCORR1_SOUT_SEL_IN_FLD_DISP_CCORR1_SOUT_SEL_IN	REG_FLD(4, 0)
#define	OVL1_INT_SOUT_SEL_IN_FLD_OVL1_INT_SOUT_SEL_IN		REG_FLD(4, 0)
#define	OVL0_2L_INT_SOUT_SEL_IN_FLD_OVL0_2L_INT_SOUT_SEL_IN	REG_FLD(4, 0)
#define	OVL1_2L_INT_SOUT_SEL_IN_FLD_OVL1_2L_INT_SOUT_SEL_IN	REG_FLD(4, 0)



#define MDP_DL_VALID_0_FLD_MDP_DL_VALID_0                       REG_FLD(32, 0)
#define MDP_DL_READY_0_FLD_MDP_DL_READY_0                       REG_FLD(32, 0)

/* SMI_LARB0 */
#define DISP_REG_SMI_LARB0_NON_SEC_CON (DISPSYS_SMI_LARB0_BASE+0x380)
#define DISP_REG_SMI_LARB0_SEC_CON (DISPSYS_SMI_LARB0_BASE+0xf80)
/* SMI_LARB1 */
#define DISP_REG_SMI_LARB1_NON_SEC_CON (DISPSYS_SMI_LARB1_BASE+0x380)
#define DISP_REG_SMI_LARB1_SEC_CON (DISPSYS_SMI_LARB1_BASE+0xf80)

/* field definition */
/* ------------------------------------------------------------- */
/* DPI */
#define DISP_REG_DPI_EN								(DISPSYS_DPI_BASE + 0x000)
#define DISP_REG_DPI_RST							(DISPSYS_DPI_BASE + 0x004)
#define DISP_REG_DPI_INTEN							(DISPSYS_DPI_BASE + 0x008)
#define DISP_REG_DPI_INSTA							(DISPSYS_DPI_BASE + 0x00C)
#define DISP_REG_DPI_CON							(DISPSYS_DPI_BASE + 0x010)
#define DISP_REG_DPI_OUTPUT_SETTING					(DISPSYS_DPI_BASE + 0x014)
#define DISP_REG_DPI_SIZE							(DISPSYS_DPI_BASE + 0x018)
#define DISP_REG_DPI_DDR_SETTING					(DISPSYS_DPI_BASE + 0x01c)
#define DISP_REG_DPI_TGEN_HWIDTH					(DISPSYS_DPI_BASE + 0x020)
#define DISP_REG_DPI_TGEN_HPORCH					(DISPSYS_DPI_BASE + 0x024)
#define DISP_REG_DPI_TGEN_VWIDTH					(DISPSYS_DPI_BASE + 0x028)
#define DISP_REG_DPI_TGEN_VPORCH					(DISPSYS_DPI_BASE + 0x02C)
#define DISP_REG_DPI_BG_HCNTL						(DISPSYS_DPI_BASE + 0x030)
#define DISP_REG_DPI_BG_VCNTL						(DISPSYS_DPI_BASE + 0x034)
#define DISP_REG_DPI_BG_COLOR						(DISPSYS_DPI_BASE + 0x038)
#define DISP_REG_DPI_FIFO_CTL						(DISPSYS_DPI_BASE + 0x03C)
#define DISP_REG_DPI_STATUS							(DISPSYS_DPI_BASE + 0x040)
#define DISP_REG_DPI_TMODE							(DISPSYS_DPI_BASE + 0x044)
#define DISP_REG_DPI_CHKSUM							(DISPSYS_DPI_BASE + 0x048)
#define DISP_REG_DPI_DUMMY							(DISPSYS_DPI_BASE + 0x050)
#define DISP_REG_DPI_TGEN_VWIDTH_LEVEN				(DISPSYS_DPI_BASE + 0x068)
#define DISP_REG_DPI_TGEN_VPORCH_LEVEN				(DISPSYS_DPI_BASE + 0x06c)
#define DISP_REG_DPI_TGEN_VWIDTH_RODD				(DISPSYS_DPI_BASE + 0x070)
#define DISP_REG_DPI_TGEN_VPORCH_RODD				(DISPSYS_DPI_BASE + 0x074)
#define DISP_REG_DPI_TGEN_VWIDTH_REVEN				(DISPSYS_DPI_BASE + 0x078)
#define DISP_REG_DPI_TGEN_VPORCH_REVEN				(DISPSYS_DPI_BASE + 0x07c)
#define DISP_REG_DPI_ESAV_VTIM_L					(DISPSYS_DPI_BASE + 0x080)
/* #define DISP_REG_DPI_ESAV_VTIM_R					(DISPSYS_DPI_BASE + 0x084) */
#define DISP_REG_DPI_ESAV_VTIM_R					(DISPSYS_DPI_BASE + 0x088)
#define DISP_REG_DPI_CLPF_SETTING					(DISPSYS_DPI_BASE + 0x08c)
#define DISP_REG_DPI_Y_LIMIT						(DISPSYS_DPI_BASE + 0x090)
#define DISP_REG_DPI_C_LIMIT						(DISPSYS_DPI_BASE + 0x094)
#define DISP_REG_DPI_YUV422_SETTING					(DISPSYS_DPI_BASE + 0x098)
#define DISP_REG_DPI_EMBSYNC_SETTING				(DISPSYS_DPI_BASE + 0x09c)
/* #define DISP_REG_DPI_ESAV_CODE_SET0					(DISPSYS_DPI_BASE + 0x0a0) */
/* #define DISP_REG_DPI_ESAV_CODE_SET1					(DISPSYS_DPI_BASE + 0x0a4) */
#define DISP_REG_DPI_ESAV_CODE_SET0					(DISPSYS_DPI_BASE + 0x0a8)
#define DISP_REG_DPI_ESAV_CODE_SET1					(DISPSYS_DPI_BASE + 0x0ac)
#define DISP_REG_DPI_BLANK_CODE_SET					(DISPSYS_DPI_BASE + 0x0b0)
#define DISP_REG_DPI_MATRIX_SET						(DISPSYS_DPI_BASE + 0x0b4)

#define EN_FLD_EN							REG_FLD(1, 0)
#define RST_FLD_RST						REG_FLD(1, 0)
#define INTEN_FLD_INT_UNDERFLOW_EN				REG_FLD(1, 2)
#define INTEN_FLD_INT_VDE_EN					REG_FLD(1, 1)
#define INTEN_FLD_INT_VSYNC_EN					REG_FLD(1, 0)
#define INSTA_FLD_INTSTA_UNDERFLOW_EN				REG_FLD(1, 2)
#define INSTA_FLD_INTSTA_VDE_EN					REG_FLD(1, 1)
#define INSTA_FLD_INTSTA_VSYNC_EN				REG_FLD(1, 0)
#define CON_FLD_IN_RB_SWAP					REG_FLD(1, 1)
#define CON_FLD_BG_ENABLE						REG_FLD(1, 0)
#define OUTPUT_SETTING_FLD_EDGE_SEL					        REG_FLD(1, 17)
#define OUTPUT_SETTING_FLD_OEN_OFF				REG_FLD(1, 16)
#define OUTPUT_SETTING_FLD_CK_POL				REG_FLD(1, 15)
#define OUTPUT_SETTING_FLD_VSYNC_POL				REG_FLD(1, 14)
#define OUTPUT_SETTING_FLD_HSYNC_POL				REG_FLD(1, 13)
#define OUTPUT_SETTING_FLD_DE_POL				REG_FLD(1, 12)
#define OUTPUT_SETTING_FLD_VS_MASK				REG_FLD(1, 10)
#define OUTPUT_SETTING_FLD_HS_MASK				REG_FLD(1, 9)
#define OUTPUT_SETTING_FLD_DE_MASK				REG_FLD(1, 8)
#define OUTPUT_SETTING_FLD_R_MASK				REG_FLD(1, 6)
#define OUTPUT_SETTING_FLD_G_MASK				REG_FLD(1, 5)
#define OUTPUT_SETTING_FLD_B_MASK				REG_FLD(1, 4)
#define OUTPUT_SETTING_FLD_BIT_SWAP			REG_FLD(1, 3)
#define OUTPUT_SETTING_FLD_CH_SWAP					REG_FLD(3, 0)
#define DPI_SIZE_FLD_HSIZE							REG_FLD(11, 16)
#define DPI_SIZE_FLD_VSIZE							REG_FLD(11, 0)
#define TGEN_HWIDTH_FLD_HPW							REG_FLD(12, 0)
#define TGEN_HPORCH_FLD_HFP							REG_FLD(12, 16)
#define TGEN_HPORCH_FLD_HBP							REG_FLD(12, 0)
#define TGEN_VWIDTH_FLD_VPW							REG_FLD(8, 0)
#define TGEN_VPORCH_FLD_VFP							REG_FLD(8, 16)
#define TGEN_VPORCH_FLD_VBP							REG_FLD(8, 0)
#define BG_HCNTL_FLD_BG_LEFT						REG_FLD(11, 16)
#define BG_HCNTL_FLD_BG_RIGHT						REG_FLD(11, 0)
#define BG_VCNTL_FLD_BG_TOP							REG_FLD(11, 16)
#define BG_VCNTL_FLD_BG_BOT							REG_FLD(11, 0)
#define BG_COLOR_FLD_BG_R							REG_FLD(8, 16)
#define BG_COLOR_FLD_BG_G							REG_FLD(8, 8)
#define BG_COLOR_FLD_BG_B							REG_FLD(8, 0)
#define FIFO_CTL_FLD_FIFO_RST_SEL					REG_FLD(1, 8)
#define FIFO_CTL_FLD_FIFO_VALID_SET					REG_FLD(5, 0)
#define STATUS_FLD_OUTEN						REG_FLD(1, 17)
#define STATUS_FLD_DPI_BUSY							REG_FLD(1, 16)
#define STATUS_FLD_V_COUNTER						REG_FLD(13, 0)
#define TMODE_FLD_DPI_OEN_ON						REG_FLD(1, 0)
#define CHKSUM_FLD_DPI_CHKSUM_EN				REG_FLD(1, 31)
#define CHKSUM_FLD_DPI_CHKSUM_READY				    REG_FLD(1, 30)
#define CHKSUM_FLD_DPI_CHKSUM						REG_FLD(24, 0)
#define PATTERN_FLD_PAT_R_MAN							REG_FLD(8, 24)
#define PATTERN_FLD_PAT_G_MAN						    REG_FLD(8, 16)
#define PATTERN_FLD_PAT_B_MAN							REG_FLD(8, 8)
#define PATTERN_FLD_PAT_SEL							REG_FLD(3, 4)
#define PATTERN_FLD_PAT_EN						REG_FLD(1, 0)
/* field definition */
/* ------------------------------------------------------------- */
/* CCORR */
#define DISP_REG_CCORR_EN                                    (DISPSYS_CCORR0_BASE + 0x000)
#define DISP_REG_CCORR_RESET                                 (DISPSYS_CCORR0_BASE + 0x004)
#define DISP_REG_CCORR_INTEN                                 (DISPSYS_CCORR0_BASE + 0x008)
#define DISP_REG_CCORR_INTSTA                                (DISPSYS_CCORR0_BASE + 0x00c)
#define DISP_REG_CCORR_STATUS                                (DISPSYS_CCORR0_BASE + 0x010)
#define DISP_REG_CCORR_CFG                                   (DISPSYS_CCORR0_BASE + 0x020)
#define DISP_REG_CCORR_IN_CNT                                (DISPSYS_CCORR0_BASE + 0x024)
#define DISP_REG_CCORR_OUT_CNT                               (DISPSYS_CCORR0_BASE + 0x028)
#define DISP_REG_CCORR_CHKSUM                                (DISPSYS_CCORR0_BASE + 0x02c)
#define DISP_REG_CCORR_SIZE                                  (DISPSYS_CCORR0_BASE + 0x030)
#define DISP_REG_CCORR_COEF_0                                (DISPSYS_CCORR0_BASE + 0x080)
#define DISP_REG_CCORR_SHADOW                                (DISPSYS_CCORR0_BASE + 0x0a0)
#define DISP_REG_CCORR_DUMMY_REG                             (DISPSYS_CCORR0_BASE + 0x0c0)

#define CCORR_SIZE_FLD_HSIZE                                  REG_FLD(13, 16)
#define CCORR_SIZE_FLD_VSIZE                                  REG_FLD(13, 0)
#define CCORR_CFG_FLD_CHKSUM_SEL                              REG_FLD(3, 29)
#define CCORR_CFG_FLD_CHKSUM_EN                               REG_FLD(1, 28)
#define CCORR_CFG_FLD_CCORR_GAMMA_OFF                         REG_FLD(1, 2)
#define CCORR_CFG_FLD_CCORR_ENGINE_EN                         REG_FLD(1, 1)
/* field definition */
/* ------------------------------------------------------------- */
/* GAMMA */
#define DISP_REG_GAMMA_EN								(DISPSYS_GAMMA0_BASE + 0x000)
#define DISP_REG_GAMMA_RESET							(DISPSYS_GAMMA0_BASE + 0x004)
#define DISP_REG_GAMMA_INTEN							(DISPSYS_GAMMA0_BASE + 0x008)
#define DISP_REG_GAMMA_INTSTA							(DISPSYS_GAMMA0_BASE + 0x00c)
#define DISP_REG_GAMMA_STATUS						    (DISPSYS_GAMMA0_BASE + 0x010)
#define DISP_REG_GAMMA_CFG							    (DISPSYS_GAMMA0_BASE + 0x020)
#define DISP_REG_GAMMA_INPUT_COUNT					    (DISPSYS_GAMMA0_BASE + 0x024)
#define DISP_REG_GAMMA_OUTPUT_COUNT					    (DISPSYS_GAMMA0_BASE + 0x028)
#define DISP_REG_GAMMA_CHKSUM						    (DISPSYS_GAMMA0_BASE + 0x02c)
#define DISP_REG_GAMMA_SIZE							    (DISPSYS_GAMMA0_BASE + 0x030)
#define DISP_REG_GAMMA_DEBUG						    (DISPSYS_GAMMA0_BASE + 0x034)
#define DISP_REG_GAMMA_DUMMY_REG					    (DISPSYS_GAMMA0_BASE + 0x0c0)
#define DISP_REG_GAMMA_LUT							    (DISPSYS_GAMMA0_BASE + 0x700)

#define EN_FLD_GAMMA_EN                         REG_FLD(1, 0)
#define RESET_FLD_GAMMA_RESET                   REG_FLD(1, 0)
#define INTEN_FLD_OF_END_INT_EN                 REG_FLD(1, 1)
#define INTEN_FLD_IF_END_INT_EN                 REG_FLD(1, 0)
#define INTSTA_FLD_OF_END_INT                   REG_FLD(1, 1)
#define INTSTA_FLD_IF_END_INT                   REG_FLD(1, 0)
#define STATUS_FLD_IN_VALID				REG_FLD(1, 7)
#define STATUS_FLD_IN_READY				REG_FLD(1, 6)
#define STATUS_FLD_OUT_VALID	                REG_FLD(1, 5)
#define STATUS_FLD_OUT_READY	                REG_FLD(1, 4)
#define STATUS_FLD_OF_UNFINISH	                REG_FLD(1, 1)
#define STATUS_FLD_IF_UNFINISH	                REG_FLD(1, 0)
#define CFG_FLD_CHKSUM_SEL				REG_FLD(2, 29)
#define CFG_FLD_CHKSUM_EN				REG_FLD(1, 28)
#define CFG_FLD_CCORR_GAMMA_OFF                 REG_FLD(1, 5)
#define CFG_FLD_CCORR_EN			            REG_FLD(1, 4)
#define CFG_FLD_DITHER_EN			            REG_FLD(1, 2)
#define CFG_FLD_GAMMA_LUT_EN			REG_FLD(1, 1)
#define CFG_FLD_RELAY_MODE				REG_FLD(1, 0)
#define INPUT_COUNT_FLD_INP_LINE_CNT			REG_FLD(13, 16)
#define INPUT_COUNT_FLD_INP_PIX_CNT			REG_FLD(13, 0)
#define OUTPUT_COUNT_FLD_OUTP_LINE_CNT			REG_FLD(13, 16)
#define OUTPUT_COUNT_FLD_OUTP_PIX_CNT			REG_FLD(13, 0)
#define CHKSUM_FLD_CHKSUM						REG_FLD(30, 0)
#define SIZE_FLD_HSIZE							REG_FLD(13, 16)
#define SIZE_FLD_VSIZE							REG_FLD(13, 0)
#define CCORR_0_FLD_CCORR_C00					REG_FLD(12, 16)
#define CCORR_0_FLD_CCORR_C01					REG_FLD(12, 0)
#define CCORR_1_FLD_CCORR_C02					REG_FLD(12, 16)
#define CCORR_1_FLD_CCORR_C10					REG_FLD(12, 0)
#define CCORR_2_FLD_CCORR_C11					REG_FLD(12, 16)
#define CCORR_2_FLD_CCORR_C12					REG_FLD(12, 0)
#define CCORR_3_FLD_CCORR_C20					REG_FLD(12, 16)
#define CCORR_3_FLD_CCORR_C21					REG_FLD(12, 0)
#define CCORR_4_FLD_CCORR_C22					REG_FLD(12, 16)
#define DUMMY_REG_FLD_DUMMY_REG					REG_FLD(32, 0)
#define DITHER_0_FLD_CRC_CLR					REG_FLD(1, 24)
#define DITHER_0_FLD_CRC_START					REG_FLD(1, 20)
#define DITHER_0_FLD_CRC_CEN					REG_FLD(1, 16)
#define DITHER_0_FLD_FRAME_DONE_DEL				REG_FLD(8, 8)
#define DITHER_0_FLD_OUT_SEL					REG_FLD(1, 4)
#define DITHER_5_FLD_W_DEMO						REG_FLD(16, 0)
#define DITHER_6_FLD_WRAP_MODE					REG_FLD(1, 16)
#define DITHER_6_FLD_LEFT_EN					REG_FLD(2, 14)
#define DITHER_6_FLD_FPHASE_R					REG_FLD(1, 13)
#define DITHER_6_FLD_FPHASE_EN					REG_FLD(1, 12)
#define DITHER_6_FLD_FPHASE						REG_FLD(6, 4)
#define DITHER_6_FLD_ROUND_EN					REG_FLD(1, 3)
#define DITHER_6_FLD_RDITHER_EN					REG_FLD(1, 2)
#define DITHER_6_FLD_LFSR_EN					REG_FLD(1, 1)
#define DITHER_6_FLD_EDITHER_EN					REG_FLD(1, 0)
#define DITHER_7_FLD_DRMOD_B					REG_FLD(2, 8)
#define DITHER_7_FLD_DRMOD_G					REG_FLD(2, 4)
#define DITHER_7_FLD_DRMOD_R					REG_FLD(2, 0)
#define GAMMA_DITHER_8_FLD_INK_DATA_R			REG_FLD(12, 16)
#define DITHER_8_FLD_INK						REG_FLD(1, 0)
#define GAMMA_DITHER_9_FLD_INK_DATA_B			REG_FLD(12, 16)
#define GAMMA_DITHER_9_FLD_INK_DATA_G			REG_FLD(12, 0)
#define DITHER_10_FLD_FPHASE_BIT				REG_FLD(3, 8)
#define DITHER_10_FLD_FPHASE_SEL				REG_FLD(2, 4)
#define DITHER_10_FLD_FPHASE_CTRL				REG_FLD(2, 0)
#define DITHER_11_FLD_SUB_B						REG_FLD(2, 12)
#define DITHER_11_FLD_SUB_G						REG_FLD(2, 8)
#define DITHER_11_FLD_SUB_R						REG_FLD(2, 4)
#define DITHER_11_FLD_SUBPIX_EN					REG_FLD(1, 0)
#define DITHER_12_FLD_H_ACTIVE					REG_FLD(16, 16)
#define DITHER_12_FLD_TABLE_EN					REG_FLD(2, 4)
#define DITHER_12_FLD_LSB_OFF					REG_FLD(1, 0)
#define DITHER_13_FLD_RSHIFT_B					REG_FLD(3, 8)
#define DITHER_13_FLD_RSHIFT_G					REG_FLD(3, 4)
#define DITHER_13_FLD_RSHIFT_R					REG_FLD(3, 0)
#define DITHER_14_FLD_DEBUG_MODE				REG_FLD(2, 8)
#define DITHER_14_FLD_DIFF_SHIFT				REG_FLD(3, 4)
#define DITHER_14_FLD_TESTPIN_EN				REG_FLD(1, 0)
#define DITHER_15_FLD_LSB_ERR_SHIFT_R			REG_FLD(3, 28)
#define DITHER_15_FLD_LSB_OVFLW_BIT_R			REG_FLD(3, 24)
#define DITHER_15_FLD_LSB_ADD_LSHIFT_R			REG_FLD(3, 20)
#define DITHER_15_FLD_LSB_INPUT_RSHIFT_R		REG_FLD(3, 16)
#define DITHER_15_FLD_LSB_NEW_BIT_MODE			REG_FLD(1, 0)
#define DITHER_16_FLD_LSB_ERR_SHIFT_B			REG_FLD(3, 28)
#define DITHER_16_FLD_OVFLW_BIT_B				REG_FLD(3, 24)
#define DITHER_16_FLD_ADD_LSHIFT_B				REG_FLD(3, 20)
#define DITHER_16_FLD_INPUT_RSHIFT_B			REG_FLD(3, 16)
#define DITHER_16_FLD_LSB_ERR_SHIFT_G			REG_FLD(3, 12)
#define DITHER_16_FLD_OVFLW_BIT_G				REG_FLD(3, 8)
#define DITHER_16_FLD_ADD_LSHIFT_G				REG_FLD(3, 4)
#define DITHER_16_FLD_INPUT_RSHIFT_G				REG_FLD(3, 0)
#define DITHER_17_FLD_CRC_RDY					REG_FLD(1, 16)
#define DITHER_17_FLD_CRC_OUT					REG_FLD(16, 0)
#define LUT_FLD_GAMMA_LUT_R					REG_FLD(10, 20)
#define LUT_FLD_GAMMA_LUT_G					REG_FLD(10, 10)
#define LUT_FLD_GAMMA_LUT_B					REG_FLD(10, 0)

/* ------------------------------------------------------------- */
/* Dither */
#define DISP_REG_DITHER_EN                                        (DISPSYS_DITHER0_BASE + 0x000)
#define DISP_REG_DITHER_RESET                                     (DISPSYS_DITHER0_BASE + 0x004)
#define DISP_REG_DITHER_INTEN                                     (DISPSYS_DITHER0_BASE + 0x008)
#define DISP_REG_DITHER_INTSTA                                    (DISPSYS_DITHER0_BASE + 0x00c)
#define DISP_REG_DITHER_STATUS                                    (DISPSYS_DITHER0_BASE + 0x010)
#define DISP_REG_DITHER_CFG                                       (DISPSYS_DITHER0_BASE + 0x020)
#define DISP_REG_DITHER_IN_CNT                                    (DISPSYS_DITHER0_BASE + 0x024)
#define DISP_REG_DITHER_OUT_CNT                                   (DISPSYS_DITHER0_BASE + 0x028)
#define DISP_REG_DITHER_CHKSUM                                    (DISPSYS_DITHER0_BASE + 0x02c)
#define DISP_REG_DITHER_SIZE                                      (DISPSYS_DITHER0_BASE + 0x030)
#define DISP_REG_DITHER_DUMMY_REG                                 (DISPSYS_DITHER0_BASE + 0x0c0)
#define DISP_REG_DITHER_0                                         (DISPSYS_DITHER0_BASE + 0x100)
#define DISP_REG_DITHER_5                                         (DISPSYS_DITHER0_BASE + 0x114)
#define DISP_REG_DITHER_6                                         (DISPSYS_DITHER0_BASE + 0x118)
#define DISP_REG_DITHER_7                                         (DISPSYS_DITHER0_BASE + 0x11c)
#define DISP_REG_DITHER_8                                         (DISPSYS_DITHER0_BASE + 0x120)
#define DISP_REG_DITHER_9                                         (DISPSYS_DITHER0_BASE + 0x124)
#define DISP_REG_DITHER_10                                        (DISPSYS_DITHER0_BASE + 0x128)
#define DISP_REG_DITHER_11                                        (DISPSYS_DITHER0_BASE + 0x12c)
#define DISP_REG_DITHER_12                                        (DISPSYS_DITHER0_BASE + 0x130)
#define DISP_REG_DITHER_13                                        (DISPSYS_DITHER0_BASE + 0x134)
#define DISP_REG_DITHER_14                                        (DISPSYS_DITHER0_BASE + 0x138)
#define DISP_REG_DITHER_15                                        (DISPSYS_DITHER0_BASE + 0x13c)
#define DISP_REG_DITHER_16                                        (DISPSYS_DITHER0_BASE + 0x140)
#define DISP_REG_DITHER_17                                        (DISPSYS_DITHER0_BASE + 0x144)

#define DITHER_CFG_FLD_CHKSUM_SEL				REG_FLD(2, 29)
#define DITHER_CFG_FLD_CHKSUM_EN				REG_FLD(1, 28)
#define DITHER_CFG_FLD_DITHER_ENGINE_EN				REG_FLD(1, 1)
#define DITHER_CFG_FLD_RELAY_MODE				REG_FLD(1, 0)
#define DITHER_SIZE_FLD_HSIZE					REG_FLD(13, 16)
#define DITHER_SIZE_FLD_VSIZE					REG_FLD(13, 0)

/* ------------------------------------------------------------- */
/* MUTEX */
#define DISP_OVL_SEPARATE_MUTEX_ID (DISP_MUTEX_DDP_LAST+1)	/* other disp will not see mutex 4 */
#define DISP_REG_CONFIG_MUTEX_INTEN					(DISPSYS_MUTEX_BASE + 0x000)
#define DISP_REG_CONFIG_MUTEX_INTSTA				(DISPSYS_MUTEX_BASE + 0x004)
#define DISP_REG_CONFIG_MUTEX_CFG					(DISPSYS_MUTEX_BASE + 0x008)
	#define CFG_FLD_HW_CG								REG_FLD(1, 0)

#define DISP_REG_CONFIG_MUTEX_REG_UPD_TIMEOUT		(DISPSYS_MUTEX_BASE + 0x00C)
	#define UPD_FLD_UPD_TIMEOUT							REG_FLD(8, 0)

#define DISP_REG_CONFIG_MUTEX_REG_COMMIT0			(DISPSYS_MUTEX_BASE + 0x010)
#define DISP_REG_CONFIG_MUTEX_INTEN_1				(DISPSYS_MUTEX_BASE + 0x018)
#define DISP_REG_CONFIG_MUTEX_INTSTA_1				(DISPSYS_MUTEX_BASE + 0x01C)

/* mutex0 */
#define DISP_REG_CONFIG_MUTEX0_EN					(DISPSYS_MUTEX_BASE + 0x020)
	#define EN_FLD_MUTEX0_EN							REG_FLD(1, 0)

#define DISP_REG_CONFIG_MUTEX0_GET					(DISPSYS_MUTEX_BASE + 0x024)
	#define GET_FLD_MUTEX0_GET							REG_FLD(1, 0)
	#define GET_FLD_INT_MUTEX0_EN						REG_FLD(1, 1)

#define DISP_REG_CONFIG_MUTEX0_RST					(DISPSYS_MUTEX_BASE + 0x028)
	#define RST_FLD_MUTEX0_RST							REG_FLD(1, 0)

#define DISP_REG_CONFIG_MUTEX0_SOF					(DISPSYS_MUTEX_BASE + 0x02C)
	#define SOF_FLD_MUTEX0_SOF							REG_FLD(3, 0)
	#define SOF_FLD_MUTEX0_SOF_TIMING					REG_FLD(2, 3)
	#define SOF_FLD_MUTEX0_SOF_WAIT						REG_FLD(1, 5)
	#define SOF_FLD_MUTEX0_EOF							REG_FLD(3, 6)
	#define SOF_FLD_MUTEX0_FOF_TIMING					REG_FLD(2, 9)
	#define SOF_FLD_MUTEX0_EOF_WAIT						REG_FLD(1, 11)

#define SOF_VAL_MUTEX0_SOF_SINGLE_MODE					(0)
#define SOF_VAL_MUTEX0_SOF_FROM_DSI0					(1)
#define SOF_VAL_MUTEX0_SOF_FROM_DSI1					(0) /*Bianco no DSI1*/
#define SOF_VAL_MUTEX0_SOF_FROM_DPI						(2)
#define SOF_VAL_MUTEX0_SOF_RESERVED						(5)
#define SOF_VAL_MUTEX0_EOF_SINGLE_MODE					(0)
#define SOF_VAL_MUTEX0_EOF_FROM_DSI0					(1)
#define SOF_VAL_MUTEX0_EOF_FROM_DSI1					(0) /*Bianco no DSI1*/
#define SOF_VAL_MUTEX0_EOF_FROM_DPI						(2)
#define SOF_VAL_MUTEX0_EOF_RESERVED						(5)

#define DISP_REG_CONFIG_MUTEX0_MOD0					(DISPSYS_MUTEX_BASE + 0x030)

#define DISP_REG_CONFIG_MUTEX1_EN					(DISPSYS_MUTEX_BASE + 0x040)
#define DISP_REG_CONFIG_MUTEX1_GET					(DISPSYS_MUTEX_BASE + 0x044)
#define DISP_REG_CONFIG_MUTEX1_RST					(DISPSYS_MUTEX_BASE + 0x048)
#define DISP_REG_CONFIG_MUTEX1_SOF					(DISPSYS_MUTEX_BASE + 0x04C)
#define DISP_REG_CONFIG_MUTEX1_MOD0					(DISPSYS_MUTEX_BASE + 0x050)

#define DISP_REG_CONFIG_MUTEX2_EN					(DISPSYS_MUTEX_BASE + 0x060)
#define DISP_REG_CONFIG_MUTEX2_GET					(DISPSYS_MUTEX_BASE + 0x064)
#define DISP_REG_CONFIG_MUTEX2_RST					(DISPSYS_MUTEX_BASE + 0x068)
#define DISP_REG_CONFIG_MUTEX2_SOF					(DISPSYS_MUTEX_BASE + 0x06C)
#define DISP_REG_CONFIG_MUTEX2_MOD0					(DISPSYS_MUTEX_BASE + 0x070)

#define DISP_REG_CONFIG_MUTEX3_EN					(DISPSYS_MUTEX_BASE + 0x080)
#define DISP_REG_CONFIG_MUTEX3_GET					(DISPSYS_MUTEX_BASE + 0x084)
#define DISP_REG_CONFIG_MUTEX3_RST					(DISPSYS_MUTEX_BASE + 0x088)
#define DISP_REG_CONFIG_MUTEX3_SOF					(DISPSYS_MUTEX_BASE + 0x08C)
#define DISP_REG_CONFIG_MUTEX3_MOD0					(DISPSYS_MUTEX_BASE + 0x090)

#define DISP_REG_CONFIG_MUTEX4_EN					(DISPSYS_MUTEX_BASE + 0x0A0)
#define DISP_REG_CONFIG_MUTEX4_GET					(DISPSYS_MUTEX_BASE + 0x0A4)
#define DISP_REG_CONFIG_MUTEX4_RST					(DISPSYS_MUTEX_BASE + 0x0A8)
#define DISP_REG_CONFIG_MUTEX4_SOF					(DISPSYS_MUTEX_BASE + 0x0AC)
#define DISP_REG_CONFIG_MUTEX4_MOD0					(DISPSYS_MUTEX_BASE + 0x0B0)

#define DISP_REG_CONFIG_MUTEX5_EN					(DISPSYS_MUTEX_BASE + 0x0C0)
#define DISP_REG_CONFIG_MUTEX5_GET					(DISPSYS_MUTEX_BASE + 0x0C4)
#define DISP_REG_CONFIG_MUTEX5_RST					(DISPSYS_MUTEX_BASE + 0x0C8)
#define DISP_REG_CONFIG_MUTEX5_SOF					(DISPSYS_MUTEX_BASE + 0x0CC)
#define DISP_REG_CONFIG_MUTEX5_MOD0					(DISPSYS_MUTEX_BASE + 0x0D0)

#define DISP_REG_CONFIG_MUTEX6_EN					(DISPSYS_MUTEX_BASE + 0x0E0)
#define DISP_REG_CONFIG_MUTEX6_GET					(DISPSYS_MUTEX_BASE + 0x0E4)
#define DISP_REG_CONFIG_MUTEX6_RST					(DISPSYS_MUTEX_BASE + 0x0E8)
#define DISP_REG_CONFIG_MUTEX6_SOF					(DISPSYS_MUTEX_BASE + 0x0EC)
#define DISP_REG_CONFIG_MUTEX6_MOD0					(DISPSYS_MUTEX_BASE + 0x0F0)

#define DISP_REG_CONFIG_MUTEX7_EN					(DISPSYS_MUTEX_BASE + 0x100)
#define DISP_REG_CONFIG_MUTEX7_GET					(DISPSYS_MUTEX_BASE + 0x104)
#define DISP_REG_CONFIG_MUTEX7_RST					(DISPSYS_MUTEX_BASE + 0x108)
#define DISP_REG_CONFIG_MUTEX7_SOF					(DISPSYS_MUTEX_BASE + 0x10C)
#define DISP_REG_CONFIG_MUTEX7_MOD0					(DISPSYS_MUTEX_BASE + 0x110)

#define DISP_REG_CONFIG_DEBUG_OUT_SEL				(DISPSYS_MUTEX_BASE + 0x30C)
	#define DEBUG_OUT_SEL_FLD_DEBUG_OUT_SEL					REG_FLD(2, 0)

#define DISP_REG_CONFIG_MUTEX_EN(n)					(DISP_REG_CONFIG_MUTEX0_EN + (0x20 * (n)))
#define DISP_REG_CONFIG_MUTEX_GET(n)					(DISP_REG_CONFIG_MUTEX0_GET + (0x20 * (n)))
#define DISP_REG_CONFIG_MUTEX_RST(n)					(DISP_REG_CONFIG_MUTEX0_RST + (0x20 * (n)))
#define DISP_REG_CONFIG_MUTEX_MOD0(n)				(DISP_REG_CONFIG_MUTEX0_MOD0 + (0x20 * (n)))
#define DISP_REG_CONFIG_MUTEX_SOF(n)					(DISP_REG_CONFIG_MUTEX0_SOF + (0x20 * (n)))

/* ------------------------------------------------------------- */


/* ------------------------------------------------------------- */
/* RDMA */
#define DISP_REG_RDMA_INT_ENABLE				(0x000)
	#define INT_ENABLE_FLD_SOF_ABNORMAL_INT           		REG_FLD(1, 7)
	#define INT_ENABLE_FLD_FIFO_EMPTY_INT_EN               	REG_FLD(1, 6)
    #define INT_ENABLE_FLD_TARGET_LINE_INT_EN		        REG_FLD(1, 5)
    #define INT_ENABLE_FLD_FIFO_UNDERFLOW_INT_EN		    REG_FLD(1, 4)
    #define INT_ENABLE_FLD_EOF_ABNORMAL_INT_EN		        REG_FLD(1, 3)
    #define INT_ENABLE_FLD_FRAME_END_INT_EN				    REG_FLD(1, 2)
    #define INT_ENABLE_FLD_FRAME_START_INT_EN			    REG_FLD(1, 1)
    #define INT_ENABLE_FLD_REG_UPDATE_INT_EN			    REG_FLD(1, 0)

#define DISP_REG_RDMA_INT_STATUS				(0x004)
	#define INT_STATUS_FLD_SOF_ABNORMAL_INT_FLAG       		REG_FLD(1, 7)
    #define INT_STATUS_FLD_FIFO_EMPTY_INT_FLAG		        REG_FLD(1, 6)
    #define INT_STATUS_FLD_TARGET_LINE_INT_FLAG		        REG_FLD(1, 5)
    #define INT_STATUS_FLD_FIFO_UNDERFLOW_INT_FLAG		    REG_FLD(1, 4)
    #define INT_STATUS_FLD_EOF_ABNORMAL_INT_FLAG		    REG_FLD(1, 3)
    #define INT_STATUS_FLD_FRAME_END_INT_FLAG			    REG_FLD(1, 2)
    #define INT_STATUS_FLD_FRAME_START_INT_FLAG		        REG_FLD(1, 1)
    #define INT_STATUS_FLD_REG_UPDATE_INT_FLAG		        REG_FLD(1, 0)

#define DISP_REG_RDMA_GLOBAL_CON				(0x010)
    #define GLOBAL_CON_FLD_SMI_BUSY					REG_FLD(1, 12)
    #define GLOBAL_CON_FLD_RESET_STATE				REG_FLD(3, 8)
    #define GLOBAL_CON_FLD_SOFT_RESET				REG_FLD(1, 4)
    #define GLOBAL_CON_FLD_MODE_SEL					REG_FLD(1, 1)
    #define GLOBAL_CON_FLD_ENGINE_EN				REG_FLD(1, 0)

#define DISP_REG_RDMA_SIZE_CON_0                (0x014)
    #define SIZE_CON_0_FLD_MATRIX_INT_MTX_SEL			    REG_FLD(4, 20)
    #define SIZE_CON_0_FLD_MATRIX_WIDE_GAMUT_EN		        REG_FLD(1, 18)
    #define SIZE_CON_0_FLD_MATRIX_ENABLE    				REG_FLD(1, 17)
    #define SIZE_CON_0_FLD_MATRIX_EXT_MTX_EN    		    REG_FLD(1, 16)
    #define SIZE_CON_0_FLD_OUTPUT_FRAME_WIDTH		        REG_FLD(13, 0)

#define DISP_REG_RDMA_SIZE_CON_1                (0x018)
    #define SIZE_CON_1_FLD_OUTPUT_FRAME_HEIGHT		        REG_FLD(20, 0)

#define DISP_REG_RDMA_TARGET_LINE				(0x01C)
    #define TARGET_LINE_FLD_TARGET_LINE			        	REG_FLD(20, 0)
#define MEM_SRC_PITCH_FLD_MEM_MODE_SRC_PITCH		        REG_FLD(16, 0)

#define DISP_REG_RDMA_MEM_CON                   (0x024)
    #define MEM_CON_FLD_MEM_MODE_HORI_BLOCK_NUM		        REG_FLD(8, 24)
	#define FLD_RG_DITHER_INIT_VPOS                         REG_FLD(2, 21)
	#define FLD_RG_DITHER_INIT_HPOS                  	    REG_FLD(2, 19)
	#define FLD_RG_DITHER_CEN                             	REG_FLD(3, 16)
    #define MEM_CON_FLD_MEM_MODE_INPUT_COSITE		        REG_FLD(1, 13)
    #define MEM_CON_FLD_MEM_MODE_INPUT_UPSAMPLE		        REG_FLD(1, 12)
    #define MEM_CON_FLD_MEM_MODE_INPUT_SWAP				    REG_FLD(1, 8)
    #define MEM_CON_FLD_MEM_MODE_INPUT_FORMAT		        REG_FLD(4, 4)
	#define FLD_MEM_MODE_INTERLACE_EN                     	REG_FLD(1, 2)
    #define MEM_CON_FLD_MEM_MODE_TILE_INTERLACE		        REG_FLD(1, 1)
    #define MEM_CON_FLD_MEM_MODE_TILE_EN                    REG_FLD(1, 0)

#define DISP_REG_RDMA_MEM_SRC_PITCH				(0x02C)
	#define FLD_MEM_MODE_SRC_PITCH   	           	REG_FLD(16, 0)

#define DISP_REG_RDMA_MEM_GMC_SETTING_0		(0x030)
	#define MEM_GMC_SETTING_0_FLD_PRE_ULTRA_THRESHOLD_LOW				REG_FLD_MSB_LSB(11, 0)
	#define MEM_GMC_SETTING_0_FLD_PRE_ULTRA_THRESHOLD_HIGH				REG_FLD_MSB_LSB(27, 16)
	#define MEM_GMC_SETTING_0_FLD_RG_VALID_THRESHOLD_FORCE_PREULTRA		REG_FLD_MSB_LSB(30, 30)
	#define MEM_GMC_SETTING_0_FLD_RG_FORCE_PREULTRA	                	REG_FLD_MSB_LSB(31, 31)

#define DISP_REG_RDMA_MEM_GMC_SETTING_1		(0x034)
	#define MEM_GMC_SETTING_1_FLD_ULTRA_THRESHOLD_LOW					REG_FLD_MSB_LSB(11, 0)
	#define MEM_GMC_SETTING_1_FLD_ULTRA_THRESHOLD_HIGH					REG_FLD_MSB_LSB(27, 16)
	#define MEM_GMC_SETTING_1_FLD_RG_VALID_THRESHOLD_BLOCK_ULTRA		REG_FLD_MSB_LSB(30, 30)
	#define MEM_GMC_SETTING_1_FLD_RG_BLOCK_ULTRA	                	REG_FLD_MSB_LSB(31, 31)

#define DISP_REG_RDMA_MEM_SLOW_CON		(0x038)
    #define MEM_SLOW_CON_FLD_MEM_MODE_SLOW_COUNT		            	REG_FLD(16, 16)
    #define MEM_SLOW_CON_FLD_MEM_MODE_SLOW_EN	                		REG_FLD(1, 0)

#define DISP_REG_RDMA_MEM_GMC_SETTING_2		(0x03c)
	#define MEM_GMC_SETTING_2_FLD_ISSUE_REQ_THRESHOLD		REG_FLD_MSB_LSB(11, 0)

#define DISP_REG_RDMA_FIFO_CON                  (0x040)
	#define FIFO_CON_FLD_OUTPUT_VALID_FIFO_THRESHOLD		REG_FLD_MSB_LSB(11, 0)
	#define FIFO_CON_FLD_FIFO_PSEUDO_SIZE					REG_FLD_MSB_LSB(27, 16)
	#define FIFO_CON_FLD_FIFO_UNDERFLOW_EN					REG_FLD_MSB_LSB(31, 31)

#define DISP_REG_RDMA_FIFO_LOG                  (0x044)
    #define FIFO_LOG_FLD_RDMA_FIFO_LOG					REG_FLD(12, 0)

#define DISP_REG_RDMA_C00                       (0x054)
    #define C00_FLD_DISP_RDMA_C00					REG_FLD(13, 0)
#define DISP_REG_RDMA_C01                       (0x058)
    #define C01_FLD_DISP_RDMA_C01                   REG_FLD(13, 0)
#define DISP_REG_RDMA_C02                       (0x05C)
    #define C02_FLD_DISP_RDMA_C02					REG_FLD(13, 0)
#define DISP_REG_RDMA_C10                       (0x060)
    #define C10_FLD_DISP_RDMA_C10					REG_FLD(13, 0)
#define DISP_REG_RDMA_C11                       (0x064)
    #define C11_FLD_DISP_RDMA_C11					REG_FLD(13, 0)
#define DISP_REG_RDMA_C12                       (0x068)
    #define C12_FLD_DISP_RDMA_C12					REG_FLD(13, 0)
#define DISP_REG_RDMA_C20                       (0x06C)
    #define C20_FLD_DISP_RDMA_C20					REG_FLD(13, 0)
#define DISP_REG_RDMA_C21                       (0x070)
    #define C21_FLD_DISP_RDMA_C21					REG_FLD(13, 0)
#define DISP_REG_RDMA_C22                       (0x074)
    #define C22_FLD_DISP_RDMA_C22					REG_FLD(13, 0)

#define DISP_REG_RDMA_PRE_ADD_0					(0x078)
    #define PRE_ADD_0_FLD_DISP_RDMA_PRE_ADD_0				    REG_FLD(9, 0)
#define DISP_REG_RDMA_PRE_ADD_1                 (0x07C)
    #define PRE_ADD_1_FLD_DISP_RDMA_PRE_ADD_1				    REG_FLD(9, 0)
#define DISP_REG_RDMA_PRE_ADD_2                 (0x080)
    #define PRE_ADD_2_FLD_DISP_RDMA_PRE_ADD_2				    REG_FLD(9, 0)
#define DISP_REG_RDMA_POST_ADD_0				(0x084)
    #define POST_ADD_0_FLD_DISP_RDMA_POST_ADD_0				    REG_FLD(9, 0)
#define DISP_REG_RDMA_POST_ADD_1                (0x088)
    #define POST_ADD_1_FLD_DISP_RDMA_POST_ADD_1			        REG_FLD(9, 0)
#define DISP_REG_RDMA_POST_ADD_2                (0x08C)
    #define POST_ADD_2_FLD_DISP_RDMA_POST_ADD_2			        REG_FLD(9, 0)

#define DISP_REG_RDMA_DUMMY                     (0x090)
    #define DUMMY_FLD_DISP_RDMA_DUMMY				REG_FLD(32, 0)

#define DISP_REG_RDMA_DEBUG_OUT_SEL             (0x094)
    #define DEBUG_OUT_SEL_FLD_DISP_RDMA_DEBUG_OUT_SEL	        REG_FLD(4, 0)

#define DISP_REG_RDMA_MEM_START_ADDR			(0xf00)
    #define MEM_START_ADDR_FLD_MEM_MODE_START_ADDR		        REG_FLD(32, 0)

#define DISP_REG_RDMA_BG_CON_0                  (0x0a0)
    #define RDMA_BG_CON_0_LEFT					REG_FLD(13, 0)
    #define RDMA_BG_CON_0_RIGHT					REG_FLD(13, 16)

#define DISP_REG_RDMA_BG_CON_1                  (0x0a4)
    #define RDMA_BG_CON_1_TOP					REG_FLD(13, 0)
    #define RDMA_BG_CON_1_BOTTOM				REG_FLD(13, 16)

#define DISP_REG_RDMA_THRESHOLD_FOR_SODI        (0x0a8)
	#define RDMA_THRESHOLD_FOR_SODI_FLD_LOW				REG_FLD_MSB_LSB(11, 0)
	#define RDMA_THRESHOLD_FOR_SODI_FLD_HIGH			REG_FLD_MSB_LSB(27, 16)

#define DISP_REG_RDMA_THRESHOLD_FOR_DVFS		(0x0ac)
	#define RDMA_THRESHOLD_FOR_DVFS_FLD_LOW				REG_FLD_MSB_LSB(11, 0)
	#define RDMA_THRESHOLD_FOR_DVFS_FLD_HIGH			REG_FLD_MSB_LSB(27, 16)

#define DISP_REG_RDMA_SRAM_SEL					(0x0b0)
	#define FLD_DISP_RDMA_SRAM_SEL                 		REG_FLD(1, 0)

#define DISP_REG_RDMA_STALL_CG_CON				(0x0b4)
	#define CG_FLD_RDMA_POST_ENG_CG               		REG_FLD(1, 0)
	#define CG_FLD_RDMA_ASYNC_READ_CG           		REG_FLD(1, 1)
	#define CG_FLD_RDMA_ASYNC_WRITE_CG             		REG_FLD(1, 2)
	#define CG_FLD_RDMA_LINE_BUF_CG               		REG_FLD(1, 3)
	#define CG_FLD_RDMA_GMC_ENG_CG        	        	REG_FLD(1, 4)
	#define CG_FLD_RDMA_PRE_ENG_CG               		REG_FLD(1, 5)
	#define CG_FLD_RDMA_FRAME_CTL_CG               		REG_FLD(1, 6)
	#define CG_FLD_RDMA_FRAME_CTL_SHARED_BUF_CG    		REG_FLD(1, 7)
	#define CG_FLD_RDMA_FENG_CK_EN_CG              		REG_FLD(1, 8)
	#define CG_FLD_RDMA_RELAY_CG                   		REG_FLD(1, 9)
	#define CG_FLD_RDMA_REG_CG                   		REG_FLD(1, 10)
	#define CG_FLD_RDMA_MEM_CG                   		REG_FLD(1, 11)
	#define CG_FLD_RDMA_OUT_RELAY_CG               		REG_FLD(1, 12)
	#define CG_FLD_RDMA_UV_UPSAMPLE_CG               	REG_FLD(1, 13)
	#define CG_FLD_RDMA_MATRIX_CG               		REG_FLD(1, 14)
	#define CG_FLD_RDMA_HG_FRDMA_SMI_BCLK_DCM_DIS_CK	REG_FLD(1, 16)
	#define CG_FLD_RDMA_HG_FRDMA_BCLK_DCM_DIS_CK	    REG_FLD(1, 17)
	#define CG_FLD_RDMA_HG_FRDMA_OUT_DCM_DIS_CK	        REG_FLD(1, 18)

#define DISP_REG_RDMA_SHADOW_UPDATE				(0x0bc)
	#define FLD_RDMA_FORCE_COMMIT               		REG_FLD(1, 0)
	#define FLD_RDMA_BYPASS_SHADOW                 		REG_FLD(1, 1)
	#define FLD_RDMA_READ_WORK_REG               		REG_FLD(1, 2)

#define DISP_REG_RDMA_DRAM_CON					(0x0c0)
	#define FLD_RDMA_FORCE_GCLAST_0               		REG_FLD(1, 0)
	#define FLD_RDMA_BANK_BOUNDARY_SEL             		REG_FLD(2, 16)

#define DISP_REG_RDMA_DVFS_SETTING_PRE			(0x0d0)
	#define RG_DVFS_PRE_ULTRA_THRESHOLD_LOW				REG_FLD_MSB_LSB(11, 0)
	#define RG_DVFS_PRE_ULTRA_THRESHOLD_HIGH			REG_FLD_MSB_LSB(27, 16)

#define DISP_REG_RDMA_DVFS_SETTING_ULTRA		(0x0d4)
	#define RG_DVFS_ULTRA_THRESHOLD_LOW					REG_FLD_MSB_LSB(11, 0)
	#define RG_DVFS_ULTRA_THRESHOLD_HIGH				REG_FLD_MSB_LSB(27, 16)

#define DISP_REG_RDMA_LEAVE_DRS_SETTING			(0x0d8)
	#define RG_IS_DRS_STATUS_THRESHOLD_LOW				REG_FLD_MSB_LSB(11, 0)
	#define RG_IS_DRS_STATUS_THRESHOLD_HIGH				REG_FLD_MSB_LSB(27, 16)

#define DISP_REG_RDMA_ENTER_DRS_SETTING			(0x0dc)
	#define RG_NOT_DRS_STATUS_THRESHOLD_LOW	    		REG_FLD_MSB_LSB(11, 0)
	#define RG_NOT_DRS_STATUS_THRESHOLD_HIGH			REG_FLD_MSB_LSB(27, 16)

#define DISP_REG_RDMA_CROP_CON0                 (0x0e0)
	#define FLD_DISP_RDMA_CROP_LEFT 	        		REG_FLD_MSB_LSB(12, 0)
	#define FLD_DISP_RDMA_CROP_RIGHT			        REG_FLD_MSB_LSB(28, 16)

#define DISP_REG_RDMA_CROP_CON1                 (0x0e4)
	#define FLD_DISP_RDMA_CROP_TOP  	        		REG_FLD_MSB_LSB(12, 0)
	#define FLD_DISP_RDMA_CROP_BOTTOM			        REG_FLD_MSB_LSB(28, 16)

#if 1
#define DISP_REG_RDMA_IN_P_CNT                  (DISPSYS_RDMA0_BASE+0x0f0)
#define DISP_REG_RDMA_IN_LINE_CNT               (DISPSYS_RDMA0_BASE+0x0f4)
#define DISP_REG_RDMA_OUT_P_CNT                 (DISPSYS_RDMA0_BASE+0x0f8)
#define DISP_REG_RDMA_OUT_LINE_CNT              (DISPSYS_RDMA0_BASE+0x0fc)
#define DISP_REG_RDMA_DBG_OUT					(DISPSYS_RDMA0_BASE+0x100)
#define DISP_REG_RDMA_DBG_OUT1					(DISPSYS_RDMA0_BASE+0x10c)
#endif 

/* ------------------------------------------------------------- */
/* WDMA */
#define DISP_REG_WDMA_INTEN							(DISPSYS_WDMA0_BASE+0x000)
    #define INTEN_FLD_FIFO_FULL                      		REG_FLD(1, 2)
    #define INTEN_FLD_FRAME_UNDERRUN                        REG_FLD(1, 1)
    #define INTEN_FLD_FRAME_COMPLETE                        REG_FLD(1, 0)

#define DISP_REG_WDMA_INTSTA							(DISPSYS_WDMA0_BASE+0x004)
    #define INTSTA_FLD_FIFO_FULL                      		REG_FLD(1, 2)
    #define INTSTA_FLD_FRAME_UNDERRUN                       REG_FLD(1, 1)
    #define INTSTA_FLD_FRAME_COMPLETE                       REG_FLD(1, 0)

#define DISP_REG_WDMA_EN							(DISPSYS_WDMA0_BASE+0x008)
	#define WDMA_EN_FLD_ENABLE						REG_FLD_MSB_LSB(0, 0)
	#define WDMA_EN_FLD_SOF_RESET_DISABLE			REG_FLD_MSB_LSB(4, 4)
	#define WDMA_EN_FLD_INTERNAL_GLOBAL_CG_DISABLE	REG_FLD_MSB_LSB(30, 30)
	#define WDMA_EN_FLD_INTERNAL_CG_DISABLE			REG_FLD_MSB_LSB(31, 31)

#define DISP_REG_WDMA_RST							(DISPSYS_WDMA0_BASE+0x00C)
	#define RST_FLD_SW_RESET		                REG_FLD(1, 0)

#define DISP_REG_WDMA_SMI_CON						(DISPSYS_WDMA0_BASE+0x010)
    #define SMI_CON_FLD_SMI_OBUF_FULL_REQ					REG_FLD(1, 28)
    #define SMI_CON_FLD_SMI_V_REPEAT_NUM					REG_FLD(4, 24)
    #define SMI_CON_FLD_SMI_U_REPEAT_NUM					REG_FLD(4, 20)
    #define SMI_CON_FLD_SMI_Y_REPEAT_NUM					REG_FLD(4, 16)
    #define SMI_CON_FLD_SLOW_COUNT							REG_FLD(8, 8)
    #define SMI_CON_FLD_SLOW_LEVEL							REG_FLD(3, 5)
    #define SMI_CON_FLD_SLOW_ENABLE							REG_FLD(1, 4)
    #define SMI_CON_FLD_THRESHOLD							REG_FLD(4, 0)

#define DISP_REG_WDMA_CFG							(DISPSYS_WDMA0_BASE+0x014)
    #define CFG_FLD_DEBUG_SEL								REG_FLD(4, 28)
    #define CFG_FLD_INT_MTX_SEL								REG_FLD(4, 24)
    #define CFG_FLD_BKGD_ENABLE								REG_FLD(1, 20)
    #define CFG_FLD_UNI_CONFIG								REG_FLD(1, 17)
    #define CFG_FLD_SWAP									REG_FLD(1, 16)
    #define CFG_FLD_DNSP_SEL								REG_FLD(1, 15)
    #define CFG_FLD_EXT_MTX_EN								REG_FLD(1, 13)
    #define CFG_FLD_VERTICAL_AVG							REG_FLD(1, 12)
    #define CFG_FLD_CT_EN									REG_FLD(1, 11)
    #define CFG_FLD_UV_SWAP									REG_FLD(1, 10)
    #define CFG_FLD_RGB_SWAP								REG_FLD(1, 9)
    #define CFG_FLD_BYTE_SWAP								REG_FLD(1, 8)
    #define CFG_FLD_OUT_FORMAT								REG_FLD(4, 4)

#define DISP_REG_WDMA_SRC_SIZE						(DISPSYS_WDMA0_BASE+0x018)
    #define SRC_SIZE_FLD_HEIGHT								REG_FLD(14, 16)
    #define SRC_SIZE_FLD_WIDTH								REG_FLD(14, 0)

#define DISP_REG_WDMA_CLIP_SIZE						(DISPSYS_WDMA0_BASE+0x01C)
    #define CLIP_SIZE_FLD_HEIGHT							REG_FLD(14, 16)
    #define CLIP_SIZE_FLD_WIDTH								REG_FLD(14, 0)

#define DISP_REG_WDMA_CLIP_COORD					(DISPSYS_WDMA0_BASE+0x020)
    #define CLIP_COORD_FLD_Y_COORD							REG_FLD(14, 16)
    #define CLIP_COORD_FLD_X_COORD							REG_FLD(14, 0)

#define DISP_REG_WDMA_DST_W_IN_BYTE					(DISPSYS_WDMA0_BASE+0x028)
    #define DST_W_IN_BYTE_FLD_DST_W_IN_BYTE					REG_FLD(16, 0)

#define DISP_REG_WDMA_ALPHA							(DISPSYS_WDMA0_BASE+0x02C)
    #define ALPHA_FLD_A_SEL									REG_FLD(1, 31)
    #define ALPHA_FLD_A_VALUE								REG_FLD(8, 0)

#define DISP_REG_WDMA_BKGD							(DISPSYS_WDMA0_BASE+0x030)
#define DISP_REG_WDMA_BUF_CON1						(DISPSYS_WDMA0_BASE+0x038)
    #define BUF_CON1_FLD_ULTRA_ENABLE						REG_FLD(1, 31)
    #define BUF_CON1_FLD_PRE_ULTRA_ENABLE	    			REG_FLD(1, 30)
    #define BUF_CON1_FLD_GCLAST_ENABLE	    			    REG_FLD(1, 29)
    #define BUF_CON1_FLD_FRAME_END_ULTRA					REG_FLD(1, 28)
    #define BUF_CON1_FLD_ALPHA_MASK_ENABLE	   		    	REG_FLD(1, 27)
    #define BUF_CON1_FLD_FIFO_PSEUDO_SIZE					REG_FLD(10, 0)

#if 0
#define DISP_REG_WDMA_SHADOW_CTL					(DISPSYS_WDMA0_BASE+0x034)
	#define WDMA_SHADOW_FLD_BYPASS_SHADOW		REG_FLD_MSB_LSB(1, 1)
	#define WDMA_SHADOW_FLD_FORCE_COMMIT		REG_FLD_MSB_LSB(0, 0)
	#define WDMA_SHADOW_FLD_READ_SHADOW			REG_FLD_MSB_LSB(2, 2)

#define DISP_REG_WDMA_BUF_CON2						(DISPSYS_WDMA0_BASE+0x03C)
    #define BUF_CON2_FLD_ULTRA_TH_HIGH_OFS					REG_FLD(8, 24)
    #define BUF_CON2_FLD_PRE_ULTRA_TH_HIGH_OFS				REG_FLD(8, 16)
    #define BUF_CON2_FLD_ULTRA_TH_LOW_OFS					REG_FLD(8, 8)
    #define BUF_CON2_FLD_PRE_ULTRA_TH_LOW					REG_FLD(8, 0)
#define DISP_REG_WDMA_C00							(DISPSYS_WDMA0_BASE+0x040)
    #define C00_FLD_C01										REG_FLD(13, 16)
    #define C00_FLD_C00										REG_FLD(13, 0)

#define DISP_REG_WDMA_C02							(DISPSYS_WDMA0_BASE+0x044)
    #define C02_FLD_C02										REG_FLD(13, 0)

#define DISP_REG_WDMA_C10							(DISPSYS_WDMA0_BASE+0x048)
    #define C10_FLD_C11										REG_FLD(13, 16)
    #define C10_FLD_C10										REG_FLD(13, 0)

#define DISP_REG_WDMA_C12							(DISPSYS_WDMA0_BASE+0x04C)
    #define C12_FLD_C12										REG_FLD(13, 0)

#define DISP_REG_WDMA_C20							(DISPSYS_WDMA0_BASE+0x050)
    #define C20_FLD_C21										REG_FLD(13, 16)
    #define C20_FLD_C20										REG_FLD(13, 0)

#define DISP_REG_WDMA_C22							(DISPSYS_WDMA0_BASE+0x054)
    #define C22_FLD_C22										REG_FLD(13, 0)

#define DISP_REG_WDMA_PRE_ADD0						(DISPSYS_WDMA0_BASE+0x058)
    #define PRE_ADD0_FLD_PRE_ADD_1							REG_FLD(9, 16)
    #define PRE_ADD0_FLD_PRE_ADD_0							REG_FLD(9, 0)
#define DISP_REG_WDMA_PRE_ADD2						(DISPSYS_WDMA0_BASE+0x05C)
    #define PRE_ADD2_FLD_PRE_ADD_2							REG_FLD(9, 0)
#define DISP_REG_WDMA_POST_ADD0						(DISPSYS_WDMA0_BASE+0x060)
    #define POST_ADD0_FLD_POST_ADD_1						REG_FLD(9, 16)
    #define POST_ADD0_FLD_POST_ADD_0						REG_FLD(9, 0)
#define DISP_REG_WDMA_POST_ADD2						(DISPSYS_WDMA0_BASE+0x064)
    #define POST_ADD2_FLD_POST_ADD_2						REG_FLD(9, 0)
#endif

#define DISP_REG_WDMA_DST_UV_PITCH					(DISPSYS_WDMA0_BASE+0x078)
    #define DST_UV_PITCH_FLD_UV_DST_W_IN_BYTE			REG_FLD(16, 0)

#define DISP_REG_WDMA_DST_ADDR_OFFSET0				(DISPSYS_WDMA0_BASE+0x080)
    #define DST_ADDR_OFFSET0_FLD_WDMA_DESTINATION_ADDRESS_OFFSET0	REG_FLD(28, 0)

#define DISP_REG_WDMA_DST_ADDR_OFFSET1				(DISPSYS_WDMA0_BASE+0x084)
    #define DST_ADDR_OFFSET1_FLD_WDMA_DESTINATION_ADDRESS_OFFSET1	REG_FLD(28, 0)

#define DISP_REG_WDMA_DST_ADDR_OFFSET2				(DISPSYS_WDMA0_BASE+0x088)
    #define DST_ADDR_OFFSET2_FLD_WDMA_DESTINATION_ADDRESS_OFFSET2	REG_FLD(28, 0)

#if 0
#define DISP_REG_WDMA_PROC_TRACK_CON_0				(DISPSYS_WDMA0_BASE+0x090)
#define DISP_REG_WDMA_PROC_TRACK_CON_1				(DISPSYS_WDMA0_BASE+0x094)
#define DISP_REG_WDMA_PROC_TRACK_CON_2				(DISPSYS_WDMA0_BASE+0x098)
#endif 

#define DISP_REG_WDMA_FLOW_CTRL_DBG					(DISPSYS_WDMA0_BASE+0x0A0)
    #define FLOW_CTRL_DBG_FLD_WDMA_STA_FLOW_CTRL						REG_FLD(10, 0)
    #define FLOW_CTRL_DBG_FLD_WDMA_FIFO_FULL    						REG_FLD(1, 12)
    #define FLOW_CTRL_DBG_FLD_WDMA_GREQ            						REG_FLD(1, 13)
    #define FLOW_CTRL_DBG_FLD_WDMA_IN_READY        						REG_FLD(1, 14)
    #define FLOW_CTRL_DBG_FLD_WDMA_IN_VALID       						REG_FLD(1, 15)
    #define FLOW_CTRL_DBG_FLD_BUF_RENG_DATA_BUSY  						REG_FLD(1, 20)
    #define FLOW_CTRL_DBG_FLD_SRAM_BUSY            						REG_FLD(1, 21)
    #define FLOW_CTRL_DBG_FLD_BUF_WEN_BUSY        						REG_FLD(1, 22)
    #define FLOW_CTRL_DBG_FLD_CT_BUSY            						REG_FLD(1, 25)
    #define FLOW_CTRL_DBG_FLD_CLIN_BUSY            						REG_FLD(1, 26)
    #define FLOW_CTRL_DBG_FLD_SMI_DATA_BUSY        						REG_FLD(1, 28)
    #define FLOW_CTRL_DBG_FLD_SMI_CMD_BUSY        						REG_FLD(1, 29)

#define DISP_REG_WDMA_EXEC_DBG						(DISPSYS_WDMA0_BASE+0x0A4)
    #define EXEC_DBG_FLD_WDMA_CLEAR_STS_CNT        						REG_FLD(1, 31)
    #define EXEC_DBG_FLD_WDMA_FRAME_COMPLETE_CNT  						REG_FLD(6, 16)
    #define EXEC_DBG_FLD_WDMA_FRAME_RUN_CNT       						REG_FLD(6, 16)

#define DISP_REG_WDMA_CT_DBG						(DISPSYS_WDMA0_BASE+0x0A8)
    #define CT_DBG_FLD_WDMA_INPUT_CNT_X         						REG_FLD(14, 0)
    #define CT_DBG_FLD_WDMA_INPUT_CNT_Y         						REG_FLD(14, 16)

#define DISP_REG_WDMA_SMI_TRAFFIC_DBG               (DISPSYS_WDMA0_BASE+0x0AC)
    #define TRAFFIC_DBG_FLD_WDMA_SMI_TRAFFIC      						REG_FLD(7, 0)
#if 0
#define DISP_REG_WDMA_PROC_TRACK_DBG_0              (DISPSYS_WDMA0_BASE+0x0B0)
#define DISP_REG_WDMA_PROC_TRACK_DBG_1              (DISPSYS_WDMA0_BASE+0x0B4)
#endif 

#define DISP_REG_WDMA_DEBUG							(DISPSYS_WDMA0_BASE+0x0B8)
    #define DEBUG_FLD_WDMA_STA_DEBUG				REG_FLD(32, 0)

#define DISP_REG_WDMA_DUMMY							(DISPSYS_WDMA0_BASE+0x100)
    #define DUMMY_FLD_WDMA_DUMMY					REG_FLD(32, 0)

#define DISP_REG_WDMA_BUF_CON3						(DISPSYS_WDMA0_BASE+0x104)
	#define BUF_CON3_FLD_ISSUE_REQ_TH_U				REG_FLD(9, 16)
	#define BUF_CON3_FLD_ISSUE_REQ_TH_Y				REG_FLD(9, 0)

#define DISP_REG_WDMA_BUF_CON4						(DISPSYS_WDMA0_BASE+0x108)
	#define BUF_CON4_FLD_ISSUE_REQ_TH_V				REG_FLD(9, 0)

#define DISP_REG_WDMA_BUF_CON5							(DISPSYS_WDMA0_BASE+0x200)
    #define BUF_CON5_FLD_PRE_ULTRA_LOW_Y							REG_FLD(10, 0)
    #define BUF_CON5_FLD_ULTRA_LOW_Y								REG_FLD(10, 16)

#define DISP_REG_WDMA_BUF_CON6							(DISPSYS_WDMA0_BASE+0x204)
    #define BUF_CON6_FLD_PRE_ULTRA_HIGH_Y							REG_FLD(10, 0)
    #define BUF_CON6_FLD_ULTRA_HIGH_Y								REG_FLD(10, 16)

#define DISP_REG_WDMA_BUF_CON7							(DISPSYS_WDMA0_BASE+0x208)
    #define BUF_CON7_FLD_PRE_ULTRA_LOW_U							REG_FLD(10, 0)
    #define BUF_CON7_FLD_ULTRA_LOW_U								REG_FLD(10, 16)

#define DISP_REG_WDMA_BUF_CON8							(DISPSYS_WDMA0_BASE+0x20C)
    #define BUF_CON8_FLD_PRE_ULTRA_HIGH_U							REG_FLD(10, 0)
    #define BUF_CON8_FLD_ULTRA_HIGH_U								REG_FLD(10, 16)

#define DISP_REG_WDMA_BUF_CON9							(DISPSYS_WDMA0_BASE+0x210)
    #define BUF_CON9_FLD_PRE_ULTRA_LOW_V							REG_FLD(10, 0)
    #define BUF_CON9_FLD_ULTRA_LOW_V								REG_FLD(10, 16)

#define DISP_REG_WDMA_BUF_CON10							(DISPSYS_WDMA0_BASE+0x214)
    #define BUF_CON10_FLD_PRE_ULTRA_HIGH_V							REG_FLD(10, 0)
    #define BUF_CON10_FLD_ULTRA_HIGH_V								REG_FLD(10, 16)

#define DISP_REG_WDMA_BUF_CON11							(DISPSYS_WDMA0_BASE+0x218)
    #define BUF_CON11_FLD_PRE_ULTRA_LOW_Y_DVFS  					REG_FLD(10, 0)
    #define BUF_CON11_FLD_ULTRA_LOW_Y_DVFS		    				REG_FLD(10, 16)

#define DISP_REG_WDMA_BUF_CON12							(DISPSYS_WDMA0_BASE+0x21C)
    #define BUF_CON12_FLD_PRE_ULTRA_HIGH_Y_DVFS  					REG_FLD(10, 0)
    #define BUF_CON12_FLD_ULTRA_HIGH_Y_DVFS		    				REG_FLD(10, 16)

#define DISP_REG_WDMA_BUF_CON13							(DISPSYS_WDMA0_BASE+0x220)
    #define BUF_CON13_FLD_PRE_ULTRA_LOW_U_DVFS  					REG_FLD(10, 0)
    #define BUF_CON13_FLD_ULTRA_LOW_U_DVFS		    				REG_FLD(10, 16)

#define DISP_REG_WDMA_BUF_CON14							(DISPSYS_WDMA0_BASE+0x224)
    #define BUF_CON14_FLD_PRE_ULTRA_HIGH_U_DVFS  					REG_FLD(10, 0)
    #define BUF_CON14_FLD_ULTRA_HIGH_U_DVFS		    				REG_FLD(10, 16)

#define DISP_REG_WDMA_BUF_CON15							(DISPSYS_WDMA0_BASE+0x228)
    #define BUF_CON15_FLD_PRE_ULTRA_LOW_V_DVFS  					REG_FLD(10, 0)
    #define BUF_CON15_FLD_ULTRA_LOW_V_DVFS		    				REG_FLD(10, 16)

#define DISP_REG_WDMA_BUF_CON16							(DISPSYS_WDMA0_BASE+0x22C)
    #define BUF_CON16_FLD_PRE_ULTRA_HIGH_V_DVFS  					REG_FLD(10, 0)
    #define BUF_CON16_FLD_ULTRA_HIGH_V_DVFS		    				REG_FLD(10, 16)

    #define BUF_CON_FLD_PRE_ULTRA_HIGH							REG_FLD(10, 0)
    #define BUF_CON_FLD_ULTRA_HIGH								REG_FLD(10, 16)
    #define BUF_CON_FLD_PRE_ULTRA_LOW							REG_FLD(10, 0)
    #define BUF_CON_FLD_ULTRA_LOW								REG_FLD(10, 16)

#define DISP_REG_WDMA_BUF_CON17							(DISPSYS_WDMA0_BASE+0x230)
	#define BUF_CON17_FLD_WDMA_DVFS_EN						REG_FLD(1, 0)
	#define BUF_CON17_FLD_DVFS_TH_Y							REG_FLD(10, 16)

#define DISP_REG_WDMA_BUF_CON18							(DISPSYS_WDMA0_BASE+0x234)
	#define BUF_CON18_FLD_DVFS_TH_U							REG_FLD(10, 0)
	#define BUF_CON18_FLD_DVFS_TH_V							REG_FLD(10, 16)

#define DISP_REG_WDMA_DRS_CON0							(DISPSYS_WDMA0_BASE+0x250)
	#define WDMA_DRS_EN										REG_FLD(1, 0)
	#define BUF_DRS_FLD_ENTER_DRS_TH_Y						REG_FLD(10, 16)

#define DISP_REG_WDMA_DRS_CON1							(DISPSYS_WDMA0_BASE+0x254)
	#define BUF_DRS_FLD_ENTER_DRS_TH_U							REG_FLD(10, 0)
	#define BUF_DRS_FLD_ENTER_DRS_TH_V							REG_FLD(10, 16)

#define DISP_REG_WDMA_DRS_CON2							(DISPSYS_WDMA0_BASE+0x258)
	#define BUF_DRS_FLD_LEAVE_DRS_TH_Y							REG_FLD(10, 16)

#define DISP_REG_WDMA_DRS_CON3							(DISPSYS_WDMA0_BASE+0x25C)
	#define BUF_DRS_FLD_LEAVE_DRS_TH_U							REG_FLD(10, 0)
	#define BUF_DRS_FLD_LEAVE_DRS_TH_V							REG_FLD(10, 16)

#if 0
#define DISP_REG_WDMA_DITHER_0						(DISPSYS_WDMA0_BASE+0xE00)
   #define DITHER_0_FLD_CRC_CLR						REG_FLD(1, 24)
   #define DITHER_0_FLD_CRC_START					REG_FLD(1, 20)
   #define DITHER_0_FLD_CRC_CEN						REG_FLD(1, 16)
   #define DITHER_0_FLD_FRAME_DONE_DEL				REG_FLD(8, 8)
   #define DITHER_0_FLD_OUT_SEL						REG_FLD(1, 4)
   #define DITHER_0_FLD_START						REG_FLD(1, 0)
#define DISP_REG_WDMA_DITHER_5						(DISPSYS_WDMA0_BASE+0xE14)
   #define DITHER_5_FLD_W_DEMO						REG_FLD(16, 0)
#define DISP_REG_WDMA_DITHER_6						(DISPSYS_WDMA0_BASE+0xE18)
   #define DITHER_6_FLD_WRAP_MODE					REG_FLD(1, 16)
   #define DITHER_6_FLD_LEFT_EN						REG_FLD(2, 14)
   #define DITHER_6_FLD_FPHASE_R						REG_FLD(1, 13)
   #define DITHER_6_FLD_FPHASE_EN					REG_FLD(1, 12)
   #define DITHER_6_FLD_FPHASE						REG_FLD(6, 4)
   #define DITHER_6_FLD_ROUND_EN					REG_FLD(1, 3)
   #define DITHER_6_FLD_RDITHER_EN					REG_FLD(1, 2)
   #define DITHER_6_FLD_LFSR_EN						REG_FLD(1, 1)
   #define DITHER_6_FLD_EDITHER_EN					REG_FLD(1, 0)
#define DISP_REG_WDMA_DITHER_7						(DISPSYS_WDMA0_BASE+0xE1C)
   #define DITHER_7_FLD_DRMOD_B						REG_FLD(2, 8)
   #define DITHER_7_FLD_DRMOD_G						REG_FLD(2, 4)
   #define DITHER_7_FLD_DRMOD_R						REG_FLD(2, 0)
#define DISP_REG_WDMA_DITHER_8						(DISPSYS_WDMA0_BASE+0xE20)
   #define DITHER_8_FLD_INK_DATA_R					REG_FLD(10, 16)
   #define DITHER_8_FLD_INK						REG_FLD(1, 0)
#define DISP_REG_WDMA_DITHER_9						(DISPSYS_WDMA0_BASE+0xE24)
   #define DITHER_9_FLD_INK_DATA_B					 REG_FLD(10, 16)
   #define DITHER_9_FLD_INK_DATA_G					REG_FLD(10, 0)
#define DISP_REG_WDMA_DITHER_10						(DISPSYS_WDMA0_BASE+0xE28)
   #define DITHER_10_FLD_FPHASE_BIT				REG_FLD(3, 8)
   #define DITHER_10_FLD_FPHASE_SEL				REG_FLD(2, 4)
   #define DITHER_10_FLD_FPHASE_CTRL				REG_FLD(2, 0)
#define DISP_REG_WDMA_DITHER_11						(DISPSYS_WDMA0_BASE+0xE2C)
   #define DITHER_11_FLD_SUB_B						REG_FLD(2, 12)
   #define DITHER_11_FLD_SUB_G						REG_FLD(2, 8)
   #define DITHER_11_FLD_SUB_R						REG_FLD(2, 4)
   #define DITHER_11_FLD_SUBPIX_EN					REG_FLD(1, 0)
#define DISP_REG_WDMA_DITHER_12						(DISPSYS_WDMA0_BASE+0xE30)
   #define DITHER_12_FLD_H_ACTIVE					REG_FLD(16, 16)
   #define DITHER_12_FLD_TABLE_EN					REG_FLD(2, 4)
   #define DITHER_12_FLD_LSB_OFF						REG_FLD(1, 0)
#define DISP_REG_WDMA_DITHER_13						(DISPSYS_WDMA0_BASE+0xE34)
   #define DITHER_13_FLD_RSHIFT_B						REG_FLD(3, 8)
   #define DITHER_13_FLD_RSHIFT_G					REG_FLD(3, 4)
   #define DITHER_13_FLD_RSHIFT_R						REG_FLD(3, 0)
#define DISP_REG_WDMA_DITHER_14						(DISPSYS_WDMA0_BASE+0xE38)
   #define DITHER_14_FLD_DEBUG_MODE				REG_FLD(2, 8)
   #define DITHER_14_FLD_DIFF_SHIFT				REG_FLD(3, 4)
   #define DITHER_14_FLD_TESTPIN_EN				REG_FLD(1, 0)
#define DISP_REG_WDMA_DITHER_15						(DISPSYS_WDMA0_BASE+0xE3C)
   #define DITHER_15_FLD_LSB_ERR_SHIFT_R				REG_FLD(3, 28)
   #define DITHER_15_FLD_OVFLW_BIT_R				REG_FLD(3, 24)
   #define DITHER_15_FLD_ADD_lSHIFT_R				REG_FLD(3, 20)
   #define DITHER_15_FLD_INPUT_RSHIFT_R				REG_FLD(3, 16)
   #define DITHER_15_FLD_NEW_BIT_MODE				REG_FLD(1, 0)
#define DISP_REG_WDMA_DITHER_16						(DISPSYS_WDMA0_BASE+0xE40)
   #define DITHER_16_FLD_LSB_ERR_SHIFT_B				REG_FLD(3, 28)
   #define DITHER_16_FLD_OVFLW_BIT_B				REG_FLD(3, 24)
   #define DITHER_16_FLD_ADD_lSHIFT_B				REG_FLD(3, 20)
   #define DITHER_16_FLD_INPUT_RSHIFT_B				REG_FLD(3, 16)
   #define DITHER_16_FLD_lSB_ERR_SHIFT_G				REG_FLD(3, 12)
   #define DITHER_16_FLD_OVFLW_BIT_G				REG_FLD(3, 8)
   #define DITHER_16_FLD_ADD_lSHIFT_G				REG_FLD(3, 4)
   #define DITHER_16_FLD_INPUT_RSHIFT_G				REG_FLD(3, 0)
#define DISP_REG_WDMA_DITHER_17						(DISPSYS_WDMA0_BASE+0xE44)
   #define DITHER_17_FLD_CRC_RDY						REG_FLD(1, 16)
   #define DITHER_17_FLD_CRC_OUT						REG_FLD(16, 0)

#endif 

#define DISP_REG_WDMA_DST_ADDR0						(DISPSYS_WDMA0_BASE+0xF00)
    #define DST_ADDR0_FLD_ADDRESS0					REG_FLD(32, 0)
#define DISP_REG_WDMA_DST_ADDR1						(DISPSYS_WDMA0_BASE+0xF04)
    #define DST_ADDR1_FLD_ADDRESS1					REG_FLD(32, 0)
#define DISP_REG_WDMA_DST_ADDR2						(DISPSYS_WDMA0_BASE+0xF08)
    #define DST_ADDR2_FLD_ADDRESS2					REG_FLD(32, 0)


/* ------------------------------------------------------------- */
/* OVL */
#define DISP_REG_OVL_STA						(0x000UL)
	#define STA_FLD_RUN							REG_FLD_MSB_LSB(0, 0)
	#define STA_FLD_RDMA0_IDLE					REG_FLD_MSB_LSB(1, 1)
	#define STA_FLD_RDMA1_IDLE					REG_FLD_MSB_LSB(2, 2)
	#define STA_FLD_RDMA2_IDLE					REG_FLD_MSB_LSB(3, 3)
	#define STA_FLD_RDMA3_IDLE					REG_FLD_MSB_LSB(4, 4)

#define DISP_REG_OVL_INTEN						(0x004UL)
	#define INTEN_FLD_REG_CMT_INTEN				REG_FLD_MSB_LSB(0, 0)
	#define INTEN_FLD_FME_CPL_INTEN				REG_FLD_MSB_LSB(1, 1)
	#define INTEN_FLD_FME_UND_INTEN				REG_FLD_MSB_LSB(2, 2)
	#define INTEN_FLD_FME_SWRST_DONE_INTEN		REG_FLD_MSB_LSB(3, 3)
	#define INTEN_FLD_FME_HWRST_DONE_INTEN		REG_FLD_MSB_LSB(4, 4)
	#define INTEN_FLD_RDMA0_EOF_ABNORMAL_INTEN	REG_FLD_MSB_LSB(5, 5)
	#define INTEN_FLD_RDMA1_EOF_ABNORMAL_INTEN	REG_FLD_MSB_LSB(6, 6)
	#define INTEN_FLD_RDMA2_EOF_ABNORMAL_INTEN	REG_FLD_MSB_LSB(7, 7)
	#define INTEN_FLD_RDMA3_EOF_ABNORMAL_INTEN	REG_FLD_MSB_LSB(8, 8)
	#define INTEN_FLD_RDMA0_SMI_UNDERFLOW_INTEN	REG_FLD_MSB_LSB(9, 9)
	#define INTEN_FLD_RDMA1_SMI_UNDERFLOW_INTEN	REG_FLD_MSB_LSB(10, 10)
	#define INTEN_FLD_RDMA2_SMI_UNDERFLOW_INTEN	REG_FLD_MSB_LSB(11, 11)
	#define INTEN_FLD_RDMA3_SMI_UNDERFLOW_INTEN	REG_FLD_MSB_LSB(12, 12)
	#define INTEN_FLD_ABNORMAL_SOF				REG_FLD_MSB_LSB(13, 13)
	#define INTEN_FLD_START_INTEN				REG_FLD_MSB_LSB(14, 14)

#define DISP_REG_OVL_INTSTA						(0x008UL)
	#define INTSTA_FLD_REG_CMT_INTSTA			REG_FLD_MSB_LSB(0, 0)
	#define INTSTA_FLD_FME_CPL_INTSTA			REG_FLD_MSB_LSB(1, 1)
	#define INTSTA_FLD_FME_UND_INTSTA			REG_FLD_MSB_LSB(2, 2)
	#define INTSTA_FLD_FME_SWRST_DONE_INTSTA	REG_FLD_MSB_LSB(3, 3)
	#define INTSTA_FLD_FME_HWRST_DONE_INTSTA	REG_FLD_MSB_LSB(4, 4)
	#define INTSTA_FLD_RDMA0_EOF_ABNORMAL_INTSTA	REG_FLD_MSB_LSB(5, 5)
	#define INTSTA_FLD_RDMA1_EOF_ABNORMAL_INTSTA	REG_FLD_MSB_LSB(6, 6)
	#define INTSTA_FLD_RDMA2_EOF_ABNORMAL_INTSTA	REG_FLD_MSB_LSB(7, 7)
	#define INTSTA_FLD_RDMA3_EOF_ABNORMAL_INTSTA	REG_FLD_MSB_LSB(8, 8)
	#define INTSTA_FLD_RDMA0_SMI_UNDERFLOW_INTSTA	REG_FLD_MSB_LSB(9, 9)
	#define INTSTA_FLD_RDMA1_SMI_UNDERFLOW_INTSTA	REG_FLD_MSB_LSB(10, 10)
	#define INTSTA_FLD_RDMA2_SMI_UNDERFLOW_INTSTA	REG_FLD_MSB_LSB(11, 11)
	#define INTSTA_FLD_RDMA3_SMI_UNDERFLOW_INTSTA	REG_FLD_MSB_LSB(12, 12)
	#define INTSTA_FLD_ABNORMAL_SOF				REG_FLD_MSB_LSB(13, 13)
	#define INTSTA_FLD_START_INTEN				REG_FLD_MSB_LSB(14, 14)

#define DISP_REG_OVL_EN							(0x00CUL)
	#define EN_FLD_OVL_EN						REG_FLD_MSB_LSB(0, 0)
	#define EN_FLD_OVL_CKON						REG_FLD_MSB_LSB(8, 8)
	#define EN_FLD_SMI_CKON						REG_FLD_MSB_LSB(9, 9)
	#define EN_FLD_IGNORE_ABN_SOF				REG_FLD_MSB_LSB(16, 16)
	#define EN_FLD_BLOCK_EXT_ULTRA				REG_FLD_MSB_LSB(18, 18)
	#define EN_FLD_BLOCK_EXT_PREULTRA			REG_FLD_MSB_LSB(19, 19)
	#define EN_FLD_RD_WRK_REG					REG_FLD_MSB_LSB(20, 20)
	#define EN_FLD_FORCE_COMMIT					REG_FLD_MSB_LSB(21, 21)
	#define EN_FLD_BYPASS_SHADOW				REG_FLD_MSB_LSB(22, 22)

#define DISP_REG_OVL_TRIG						(0x010UL)
	#define TRIG_FLD_SW_TRIG					REG_FLD_MSB_LSB(0, 0)
	#define TRIG_FLD_CRC_EN	    				REG_FLD_MSB_LSB(8, 8)
	#define TRIG_FLD_CRC_CLR					REG_FLD_MSB_LSB(9, 9)

#define DISP_REG_OVL_RST						(0x014UL)
	#define RST_FLD_OVL_RST	    				REG_FLD_MSB_LSB(0, 0)
	#define RST_FLD_OVL_SMI_RST    				REG_FLD_MSB_LSB(28, 28)
	#define RST_FLD_OVL_SMI_HARD_RST			REG_FLD_MSB_LSB(29, 29)
	#define RST_FLD_OVL_SMI_IOBUF_RST			REG_FLD_MSB_LSB(30, 30)
	#define RST_FLD_OVL_SMI_IOBUF_HARD_RST  	REG_FLD_MSB_LSB(31, 31)

#define DISP_REG_OVL_ROI_SIZE					(0x020UL)
	#define ROI_SIZE_FLD_ROI_W					REG_FLD_MSB_LSB(12, 0)
	#define ROI_SIZE_FLD_ROI_H					REG_FLD_MSB_LSB(28, 16)

#define DISP_REG_OVL_DATAPATH_CON				(0x024UL)
	#define DATAPATH_CON_FLD_LAYER_SMI_ID_EN	REG_FLD_MSB_LSB(0, 0)
	#define DATAPATH_CON_FLD_RANDOM_BGCLR_EN	REG_FLD_MSB_LSB(1, 1)
	#define DATAPATH_CON_FLD_BGCLR_IN_SEL		REG_FLD_MSB_LSB(2, 2)
	#define DATAPATH_CON_FLD_OUTPUT_NO_RND		REG_FLD_MSB_LSB(3, 3)
	#define DATAPATH_CON_FLD_L0_GPU_MODE		REG_FLD_MSB_LSB(8, 8)
	#define DATAPATH_CON_FLD_L1_GPU_MODE		REG_FLD_MSB_LSB(9, 9)
	#define DATAPATH_CON_FLD_L2_GPU_MODE		REG_FLD_MSB_LSB(10, 10)
	#define DATAPATH_CON_FLD_L3_GPU_MODE		REG_FLD_MSB_LSB(11, 11)
	#define DATAPATH_CON_FLD_ADOBE_MODE			REG_FLD_MSB_LSB(12, 12)
	#define DATAPATH_CON_FLD_ADOBE_LAYER		REG_FLD_MSB_LSB(14, 13)
	#define DATAPATH_CON_FLD_OVL_GAMMA_OUT		REG_FLD_MSB_LSB(15, 15)
	#define DATAPATH_CON_FLD_PQ_OUT_SEL			REG_FLD_MSB_LSB(17, 16)
	#define DATAPATH_CON_FLD_RDMA0_OUT_SEL		REG_FLD_MSB_LSB(20, 20)
	#define DATAPATH_CON_FLD_RDMA1_OUT_SEL		REG_FLD_MSB_LSB(21, 21)
	#define DATAPATH_CON_FLD_RDMA2_OUT_SEL		REG_FLD_MSB_LSB(22, 22)
	#define DATAPATH_CON_FLD_RDMA3_OUT_SEL		REG_FLD_MSB_LSB(23, 23)
	#define DATAPATH_CON_FLD_GCLAST_EN			REG_FLD_MSB_LSB(24, 24)
	#define DATAPATH_CON_FLD_OUTPUT_CLAMP		REG_FLD_MSB_LSB(26, 26)
	#define DATAPATH_CON_FLD_OUTPUT_INTERLACE	REG_FLD_MSB_LSB(27, 27)

#define DISP_REG_OVL_ROI_BGCLR					(0x028UL)
	#define ROI_BGCLR_FLD_BLUE					REG_FLD_MSB_LSB(7, 0)
	#define ROI_BGCLR_FLD_GREEN					REG_FLD_MSB_LSB(18, 8)
	#define ROI_BGCLR_FLD_RED					REG_FLD_MSB_LSB(23, 16)
	#define ROI_BGCLR_FLD_ALPHA					REG_FLD_MSB_LSB(31, 24)

#define DISP_REG_OVL_SRC_CON					(0x02CUL)
	#define SRC_CON_FLD_L0_EN					REG_FLD_MSB_LSB(0, 0)
	#define SRC_CON_FLD_L1_EN					REG_FLD_MSB_LSB(1, 1)
	#define SRC_CON_FLD_L2_EN					REG_FLD_MSB_LSB(2, 2)
	#define SRC_CON_FLD_L3_EN					REG_FLD_MSB_LSB(3, 3)
	#define SRC_CON_FLD_LC_EN					REG_FLD_MSB_LSB(4, 4)
	#define SRC_CON_FLD_FORCE_RELAY_MODE		REG_FLD_MSB_LSB(8, 8)
	#define SRC_CON_FLD_RELAY_MODE_EN			REG_FLD_MSB_LSB(9, 9)

#define DISP_REG_OVL_L0_CON						(0x030UL)
	#define L_CON_FLD_APHA						REG_FLD_MSB_LSB(7, 0)
	#define L_CON_FLD_AEN						REG_FLD_MSB_LSB(8, 8)
	#define L_CON_FLD_VIRTICAL_FLIP				REG_FLD_MSB_LSB(9, 9)
	#define L_CON_FLD_HORI_FLIP					REG_FLD_MSB_LSB(10, 10)
	#define L_CON_FLD_EXT_MTX_EN				REG_FLD_MSB_LSB(11, 11)
	#define L_CON_FLD_CFMT						REG_FLD_MSB_LSB(15, 12)
	#define L_CON_FLD_MTX						REG_FLD_MSB_LSB(19, 16)
	#define L_CON_FLD_EN_3D						REG_FLD_MSB_LSB(20, 20)
	#define L_CON_FLD_EN_LANDSCAPE				REG_FLD_MSB_LSB(21, 21)
	#define L_CON_FLD_EN_R_FIRST				REG_FLD_MSB_LSB(22, 22)
	#define L_CON_FLD_CLRFMT_MAN				REG_FLD_MSB_LSB(23, 23)
	#define L_CON_FLD_BTSW						REG_FLD_MSB_LSB(24, 24)
	#define L_CON_FLD_RGB_SWAP					REG_FLD_MSB_LSB(25, 25)
	#define L_CON_FLD_MTX_AUTO_DIS				REG_FLD_MSB_LSB(26, 26)
	#define L_CON_FLD_MTX_EN					REG_FLD_MSB_LSB(27, 27)
	#define L_CON_FLD_LSRC						REG_FLD_MSB_LSB(29, 28)
	#define L_CON_FLD_SKEN						REG_FLD_MSB_LSB(30, 30)
	#define L_CON_FLD_DKEN						REG_FLD_MSB_LSB(31, 31)

#define DISP_REG_OVL_L0_SRCKEY					(0x034UL)
#define DISP_REG_OVL_L0_SRC_SIZE				(0x038UL)
	#define L_SRC_SIZE_FLD_WIDTH				REG_FLD_MSB_LSB(12, 0)
	#define L_SRC_SIZE_FLD_HEIGHT				REG_FLD_MSB_LSB(28, 16)
#define DISP_REG_OVL_L0_OFFSET					(0x03CUL)
	#define L_OFFSET_FLD_XOFF					REG_FLD_MSB_LSB(12, 0)
	#define L_OFFSET_FLD_YOFF					REG_FLD_MSB_LSB(28, 16)
#define DISP_REG_OVL_L0_ADDR					(0xf40UL)
#define DISP_REG_OVL_L0_PITCH					(0x044UL)
	#define L_PITCH_FLD_SRC_PITCH				REG_FLD_MSB_LSB(15, 0)
	#define L_PITCH_FLD_SUR_ALFA				REG_FLD_MSB_LSB(31, 16)
	#define L_PITCH_FLD_SA_SEL					REG_FLD_MSB_LSB(17, 16)
	#define L_PITCH_FLD_SRGB_SEL				REG_FLD_MSB_LSB(19, 18)
	#define L_PITCH_FLD_DA_SEL					REG_FLD_MSB_LSB(21, 20)
	#define L_PITCH_FLD_DRGB_SEL				REG_FLD_MSB_LSB(23, 22)
	#define L_PITCH_FLD_SA_SEL_EXT				REG_FLD_MSB_LSB(24, 24)
	#define L_PITCH_FLD_SRGB_SEL_EXT			REG_FLD_MSB_LSB(25, 25)
	#define L_PITCH_FLD_DA_SEL_EXT				REG_FLD_MSB_LSB(26, 26)
	#define L_PITCH_FLD_DRGB_SEL_EXT			REG_FLD_MSB_LSB(27, 27)
	#define L_PITCH_FLD_CONST_BLD				REG_FLD_MSB_LSB(28, 28)
	#define L_PITCH_FLD_SRGB_SEL_EXT2			REG_FLD_MSB_LSB(29, 29)
	#define L_PITCH_FLD_BLEND_RND_SHT			REG_FLD_MSB_LSB(30, 30)
	#define L_PITCH_FLD_SURFL_EN				REG_FLD_MSB_LSB(31, 31)

#define DISP_REG_OVL_L0_TILE					(0x048UL)
	#define L_TILE_FLD_HEIGHT	    			REG_FLD_MSB_LSB(19, 0)
	#define L_TILE_FLD_WIDTH_SEL    			REG_FLD_MSB_LSB(20, 20)
	#define L_TILE_FLD_TILE_EN      			REG_FLD_MSB_LSB(21, 21)
	#define L_TILE_FLD_TILE_HORI_BLOCK_NUM		REG_FLD_MSB_LSB(31, 24)

#define DISP_REG_OVL_L0_CLIP					(0x04CUL)
	#define OVL_L_CLIP_FLD_LEFT					REG_FLD_MSB_LSB(7, 0)
	#define OVL_L_CLIP_FLD_RIGHT				REG_FLD_MSB_LSB(15, 8)
	#define OVL_L_CLIP_FLD_TOP					REG_FLD_MSB_LSB(23, 16)
	#define OVL_L_CLIP_FLD_BOTTOM				REG_FLD_MSB_LSB(31, 24)

#define DISP_REG_OVL_L1_CON						(0x050UL)
#define DISP_REG_OVL_L1_SRCKEY					(0x054UL)
#define DISP_REG_OVL_L1_SRC_SIZE				(0x058UL)
#define DISP_REG_OVL_L1_OFFSET					(0x05CUL)
#define DISP_REG_OVL_L1_ADDR					(0xf60UL)
#define DISP_REG_OVL_L1_PITCH					(0x064UL)
#define DISP_REG_OVL_L1_TILE					(0x068UL)
#define DISP_REG_OVL_L1_CLIP					(0x06CUL)
#define DISP_REG_OVL_L2_CON						(0x070UL)
#define DISP_REG_OVL_L2_SRCKEY					(0x074UL)
#define DISP_REG_OVL_L2_SRC_SIZE				(0x078UL)
#define DISP_REG_OVL_L2_OFFSET					(0x07CUL)
#define DISP_REG_OVL_L2_ADDR					(0xf80UL)
#define DISP_REG_OVL_L2_PITCH					(0x084UL)
#define DISP_REG_OVL_L2_TILE					(0x088UL)
#define DISP_REG_OVL_L2_CLIP					(0x08CUL)
#define DISP_REG_OVL_L3_CON						(0x090UL)
#define DISP_REG_OVL_L3_SRCKEY					(0x094UL)
#define DISP_REG_OVL_L3_SRC_SIZE				(0x098UL)
#define DISP_REG_OVL_L3_OFFSET					(0x09CUL)
#define DISP_REG_OVL_L3_ADDR					(0xfA0UL)
#define DISP_REG_OVL_L3_PITCH					(0x0A4UL)
#define DISP_REG_OVL_L3_TILE					(0x0A8UL)
#define DISP_REG_OVL_L3_CLIP					(0x0ACUL)
#define DISP_REG_OVL_RDMA0_CTRL					(0x0C0UL)
	#define RDMA0_CTRL_FLD_RDMA_EN				REG_FLD_MSB_LSB(0, 0)
	#define RDMA0_CTRL_FLD_RDMA_INTERLACE		REG_FLD_MSB_LSB(0, 0)
	#define RDMA0_CTRL_FLD_RMDA_FIFO_USED_SZ		REG_FLD_MSB_LSB(27, 16)

#define DISP_REG_OVL_RDMA0_MEM_GMC_SETTING				(0x0C8UL)
	#define FLD_OVL_RDMA_MEM_GMC_ULTRA_THRESHOLD			REG_FLD_MSB_LSB(9, 0)
	#define FLD_OVL_RDMA_MEM_GMC_PRE_ULTRA_THRESHOLD		REG_FLD_MSB_LSB(25, 16)
	#define FLD_OVL_RDMA_MEM_GMC_ULTRA_THRESHOLD_HIGH_OFS		REG_FLD_MSB_LSB(28, 28)
	#define FLD_OVL_RDMA_MEM_GMC_PRE_ULTRA_THRESHOLD_HIGH_OFS	REG_FLD_MSB_LSB(31, 31)

#define DISP_REG_OVL_RDMA0_MEM_SLOW_CON				(0x0CCUL)
	#define FLD_SLOW_EN		                    	REG_FLD_MSB_LSB(0, 0)
	#define FLD_SLOW_CNT	                    	REG_FLD_MSB_LSB(16, 31)

#define DISP_REG_OVL_RDMA0_FIFO_CTRL				(0x0D0UL)
	#define FLD_OVL_RDMA_FIFO_THRD					REG_FLD_MSB_LSB(9, 0)
	#define FLD_OVL_RDMA_FIFO_SIZE					REG_FLD_MSB_LSB(27, 16)
	#define FLD_OVL_RDMA_FIFO_UND_EN				REG_FLD_MSB_LSB(31, 31)

#define DISP_REG_OVL_RDMA1_CTRL						(0x0E0UL)
#define DISP_REG_OVL_RDMA1_MEM_GMC_SETTING			(0x0E8UL)
#define DISP_REG_OVL_RDMA1_MEM_SLOW_CON				(0x0ECUL)
#define DISP_REG_OVL_RDMA1_FIFO_CTRL				(0x0F0UL)
#define DISP_REG_OVL_RDMA2_CTRL						(0x100UL)
#define DISP_REG_OVL_RDMA2_MEM_GMC_SETTING			(0x108UL)
#define DISP_REG_OVL_RDMA2_MEM_SLOW_CON				(0x10CUL)
#define DISP_REG_OVL_RDMA2_FIFO_CTRL				(0x110UL)
#define DISP_REG_OVL_RDMA3_CTRL						(0x120UL)
#define DISP_REG_OVL_RDMA3_MEM_GMC_SETTING			(0x128UL)
#define DISP_REG_OVL_RDMA3_MEM_SLOW_CON				(0x12CUL)
#define DISP_REG_OVL_RDMA3_FIFO_CTRL				(0x130UL)

#define DISP_REG_OVL_L0_Y2R_PARA_R0					(0x134UL)
	#define PARA_FLD_C_CF_RMY  	    				REG_FLD_MSB_LSB(12, 0)
	#define PARA_FLD_C_CF_RMU    					REG_FLD_MSB_LSB(28, 16)

#define DISP_REG_OVL_L0_Y2R_PARA_R1					(0x138UL)
	#define PARA_FLD_C_CF_RMV  	    				REG_FLD_MSB_LSB(12, 0)

#define DISP_REG_OVL_L0_Y2R_PARA_G0					(0x13CUL)
	#define PARA_FLD_C_CF_GMY     					REG_FLD_MSB_LSB(12, 0)
	#define PARA_FLD_C_CF_GMU  	    				REG_FLD_MSB_LSB(28, 16)

#define DISP_REG_OVL_L0_Y2R_PARA_G1					(0x140UL)
	#define PARA_FLD_C_CF_GMV  	    				REG_FLD_MSB_LSB(12, 0)

#define DISP_REG_OVL_L0_Y2R_PARA_B0					(0x144UL)
	#define PARA_FLD_C_CF_BMY     					REG_FLD_MSB_LSB(12, 0)
	#define PARA_FLD_C_CF_BMU  		    			REG_FLD_MSB_LSB(28, 16)

#define DISP_REG_OVL_L0_Y2R_PARA_B1					(0x148UL)
	#define PARA_FLD_C_CF_BMV  	    				REG_FLD_MSB_LSB(12, 0)

#define DISP_REG_OVL_L0_Y2R_PARA_YUV_A_0			(0x14CUL)
	#define PARA_FLD_C_CF_YA     					REG_FLD_MSB_LSB(8, 0)
	#define PARA_FLD_C_CF_UA  				    	REG_FLD_MSB_LSB(24, 16)

#define DISP_REG_OVL_L0_Y2R_PARA_YUV_A_1			(0x150UL)
	#define PARA_FLD_C_CF_VA     					REG_FLD_MSB_LSB(8, 0)

#define DISP_REG_OVL_L0_Y2R_PARA_RGB_A_0			(0x154UL)
	#define PARA_FLD_C_CF_RA     					REG_FLD_MSB_LSB(8, 0)
	#define PARA_FLD_C_CF_GA  				    	REG_FLD_MSB_LSB(24, 16)

#define DISP_REG_OVL_L0_Y2R_PARA_RGB_A_1			(0x158UL)
	#define PARA_FLD_C_CF_BA     					REG_FLD_MSB_LSB(8, 0)

#define DISP_REG_OVL_L1_Y2R_PARA_R0					(0x15CUL)
#define DISP_REG_OVL_L1_Y2R_PARA_R1					(0x160UL)
#define DISP_REG_OVL_L1_Y2R_PARA_G0					(0x164UL)
#define DISP_REG_OVL_L1_Y2R_PARA_G1					(0x168UL)
#define DISP_REG_OVL_L1_Y2R_PARA_B0					(0x16CUL)
#define DISP_REG_OVL_L1_Y2R_PARA_B1					(0x170UL)
#define DISP_REG_OVL_L1_Y2R_PARA_YUV_A_0			        (0x174UL)
#define DISP_REG_OVL_L1_Y2R_PARA_YUV_A_1			        (0x178UL)
#define DISP_REG_OVL_L1_Y2R_PARA_RGB_A_0			        (0x17CUL)
#define DISP_REG_OVL_L1_Y2R_PARA_RGB_A_1			        (0x180UL)
#define DISP_REG_OVL_L2_Y2R_PARA_R0					(0x184UL)
#define DISP_REG_OVL_L2_Y2R_PARA_R1					(0x188UL)
#define DISP_REG_OVL_L2_Y2R_PARA_G0					(0x18CUL)
#define DISP_REG_OVL_L2_Y2R_PARA_G1					(0x190UL)
#define DISP_REG_OVL_L2_Y2R_PARA_B0					(0x194UL)
#define DISP_REG_OVL_L2_Y2R_PARA_B1					(0x198UL)
#define DISP_REG_OVL_L2_Y2R_PARA_YUV_A_0			        (0x19CUL)
#define DISP_REG_OVL_L2_Y2R_PARA_YUV_A_1			        (0x1A0UL)
#define DISP_REG_OVL_L2_Y2R_PARA_RGB_A_0			        (0x1A4UL)
#define DISP_REG_OVL_L2_Y2R_PARA_RGB_A_1			        (0x1A8UL)
#define DISP_REG_OVL_L3_Y2R_PARA_R0					(0x1ACUL)
#define DISP_REG_OVL_L3_Y2R_PARA_R1					(0x1B0UL)
#define DISP_REG_OVL_L3_Y2R_PARA_G0					(0x1B4UL)
#define DISP_REG_OVL_L3_Y2R_PARA_G1					(0x1B8UL)
#define DISP_REG_OVL_L3_Y2R_PARA_B0					(0x1BCUL)
#define DISP_REG_OVL_L3_Y2R_PARA_B1					(0x1C0UL)
#define DISP_REG_OVL_L3_Y2R_PARA_YUV_A_0			        (0x1C4UL)
#define DISP_REG_OVL_L3_Y2R_PARA_YUV_A_1			        (0x1C8UL)
#define DISP_REG_OVL_L3_Y2R_PARA_RGB_A_0				(0x1CCUL)
#define DISP_REG_OVL_L3_Y2R_PARA_RGB_A_1			        (0x1D0UL)
#define DISP_REG_OVL_DEBUG_MON_SEL					(0x1D4UL)
	#define FLD_DBG_MON_SEL                     		REG_FLD_MSB_LSB(3, 0)

#define DISP_REG_OVL_BLD_EXT        				(0x1DCUL)
	#define FLD_L0_MINUS_BLD                     		REG_FLD_MSB_LSB(0, 0)
	#define FLD_L1_MINUS_BLD                     		REG_FLD_MSB_LSB(1, 1)
	#define FLD_L2_MINUS_BLD                     		REG_FLD_MSB_LSB(2, 2)
	#define FLD_L3_MINUS_BLD                     		REG_FLD_MSB_LSB(3, 3)
	#define FLD_LC_MINUS_BLD                     		REG_FLD_MSB_LSB(4, 4)
	#define FLD_EL0_MINUS_BLD                     		REG_FLD_MSB_LSB(5, 5)
	#define FLD_EL1_MINUS_BLD                     		REG_FLD_MSB_LSB(6, 6)
	#define FLD_EL2_MINUS_BLD                     		REG_FLD_MSB_LSB(7, 7)

#define DISP_REG_OVL_RDMA0_MEM_GMC_S2				(0x1E0UL)
	#define FLD_OVL_RDMA_MEM_GMC2_ISSUE_REQ_THRES		REG_FLD_MSB_LSB(11, 0)
	#define FLD_OVL_RDMA_MEM_GMC2_ISSUE_REQ_THRES_URG	REG_FLD_MSB_LSB(27, 16)
	#define FLD_OVL_RDMA_MEM_GMC2_REQ_THRES_PREULTRA	REG_FLD_MSB_LSB(28, 28)
	#define FLD_OVL_RDMA_MEM_GMC2_REQ_THRES_ULTRA		REG_FLD_MSB_LSB(29, 29)
	#define FLD_OVL_RDMA_MEM_GMC2_FORCE_REQ_THRES		REG_FLD_MSB_LSB(30, 30)

#define DISP_REG_OVL_RDMA1_MEM_GMC_S2				(0x1E4UL)
#define DISP_REG_OVL_RDMA2_MEM_GMC_S2				(0x1E8UL)
#define DISP_REG_OVL_RDMA3_MEM_GMC_S2				(0x1ECUL)
#define DISP_REG_OVL_RDMA_BURST_CON0				(0x1F0UL)
	#define FLD_BURST9A_32B                 		REG_FLD_MSB_LSB(2, 0)
	#define FLD_BURST10A_32B                 		REG_FLD_MSB_LSB(6, 4)
	#define FLD_BURST11A_32B                 		REG_FLD_MSB_LSB(10, 8)
	#define FLD_BURST12A_32B                 		REG_FLD_MSB_LSB(14, 12)
	#define FLD_BURST13A_32B                 		REG_FLD_MSB_LSB(18, 16)
	#define FLD_BURST14A_32B                 		REG_FLD_MSB_LSB(22, 20)
	#define FLD_BURST15A_32B                 		REG_FLD_MSB_LSB(26, 24)
	#define FLD_BURST_128B_BOUND               		REG_FLD_MSB_LSB(28, 28)

#define DISP_REG_OVL_RDMA_BURST_CON1				(0x1F4UL)
#define DISP_REG_OVL_RDMA_GREQ_NUM					(0x1F8UL)
	#define FLD_OVL_RDMA_GREQ_LAYER0_GREQ_NUM	REG_FLD_MSB_LSB(3, 0)
	#define FLD_OVL_RDMA_GREQ_LAYER1_GREQ_NUM	REG_FLD_MSB_LSB(7, 4)
	#define FLD_OVL_RDMA_GREQ_LAYER2_GREQ_NUM	REG_FLD_MSB_LSB(11, 8)
	#define FLD_OVL_RDMA_GREQ_LAYER3_GREQ_NUM	REG_FLD_MSB_LSB(15, 12)
	#define FLD_OVL_RDMA_GREQ_OSTD_GREQ_NUM		REG_FLD_MSB_LSB(23, 16)
	#define FLD_OVL_RDMA_GREQ_GREQ_DIS_CNT		REG_FLD_MSB_LSB(26, 24)
	#define FLD_OVL_RDMA_GREQ_STOP_EN		REG_FLD_MSB_LSB(27, 27)
	#define FLD_OVL_RDMA_GREQ_GRP_END_STOP			REG_FLD_MSB_LSB(28, 28)
	#define FLD_OVL_RDMA_GREQ_GRP_BRK_STOP		REG_FLD_MSB_LSB(29, 29)
	#define FLD_OVL_RDMA_GREQ_IOBUF_FLUSH_PREULTRA	REG_FLD_MSB_LSB(30, 30)
	#define FLD_OVL_RDMA_GREQ_IOBUF_FLUSH_ULTRA	REG_FLD_MSB_LSB(31, 31)

#define DISP_REG_OVL_RDMA_GREQ_URG_NUM					(0x1FCUL)
	#define FLD_OVL_RDMA_GREQ_LAYER0_GREQ_URG_NUM		REG_FLD_MSB_LSB(3, 0)
	#define FLD_OVL_RDMA_GREQ_LAYER1_GREQ_URG_NUM		REG_FLD_MSB_LSB(7, 4)
	#define FLD_OVL_RDMA_GREQ_LAYER2_GREQ_URG_NUM		REG_FLD_MSB_LSB(11, 8)
	#define FLD_OVL_RDMA_GREQ_LAYER3_GREQ_URG_NUM		REG_FLD_MSB_LSB(15, 12)
	#define FLD_OVL_RDMA_GREQ_ARG_GREQ_URG_TH		REG_FLD_MSB_LSB(25, 16)
	#define FLD_OVL_RDMA_GREQ_ARG_URG_BIAS			REG_FLD_MSB_LSB(28, 28)
	#define FLD_OVL_RDMA_GREQ_NUM_SHT_VAL			REG_FLD_MSB_LSB(29, 29)
	#define FLD_OVL_RDMA_GREQ_NUM_SHT				REG_FLD_MSB_LSB(31, 30)

#define DISP_REG_OVL_DUMMY_REG						(0x200UL)
#define DISP_REG_OVL_GDRDY_PRD						(0x208UL)
	#define FLD_GDRDY_PRD           				REG_FLD_MSB_LSB(23, 0)

#define DISP_REG_OVL_RDMA_ULTRA_SRC					(0x20CUL)
	#define FLD_OVL_RDMA_PREULTRA_BUF_SRC		REG_FLD_MSB_LSB(1, 0)
	#define FLD_OVL_RDMA_PREULTRA_SMI_SRC		REG_FLD_MSB_LSB(3, 2)
	#define FLD_OVL_RDMA_PREULTRA_ROI_END_SRC	REG_FLD_MSB_LSB(5, 4)
	#define FLD_OVL_RDMA_PREULTRA_RDMA_SRC		REG_FLD_MSB_LSB(7, 6)
	#define FLD_OVL_RDMA_ULTRA_BUF_SRC		REG_FLD_MSB_LSB(9, 8)
	#define FLD_OVL_RDMA_ULTRA_SMI_SRC		REG_FLD_MSB_LSB(11, 10)
	#define FLD_OVL_RDMA_ULTRA_ROI_END_SRC		REG_FLD_MSB_LSB(13, 12)
	#define FLD_OVL_RDMA_ULTRA_RDMA_SRC		REG_FLD_MSB_LSB(15, 14)

#define DISP_REG_OVL_RDMAn_BUF_LOW(layer)			(0x210UL + ((layer)<<2))
	#define FLD_OVL_RDMA_BUF_LOW_ULTRA_TH		REG_FLD_MSB_LSB(11, 0)
	#define FLD_OVL_RDMA_BUF_LOW_PREULTRA_TH	REG_FLD_MSB_LSB(23, 12)

#define DISP_REG_OVL_RDMA0_BUF_LOW			(0x210UL)
#define DISP_REG_OVL_RDMA1_BUF_LOW			(0x214UL)
#define DISP_REG_OVL_RDMA2_BUF_LOW			(0x218UL)
#define DISP_REG_OVL_RDMA3_BUF_LOW			(0x21cUL)
#define DISP_REG_OVL_RDMAn_BUF_HIGH(layer)			(0x220UL + ((layer)<<2))
	#define FLD_OVL_RDMA_BUF_HIGH_PREULTRA_TH	REG_FLD_MSB_LSB(23, 12)
	#define FLD_OVL_RDMA_BUF_HIGH_PREULTRA_DIS	REG_FLD_MSB_LSB(31, 31)

#define DISP_REG_OVL_RDMA0_BUF_HIGH					(0x220UL)
#define DISP_REG_OVL_RDMA1_BUF_HIGH					(0x224UL)
#define DISP_REG_OVL_RDMA2_BUF_HIGH					(0x228UL)
#define DISP_REG_OVL_RDMA3_BUF_HIGH					(0x22CUL)

#define DISP_REG_OVL_SMI_DBG						(0x230UL)
	#define FLD_SMI_FSM	                        	REG_FLD_MSB_LSB(9, 0)

#define DISP_REG_OVL_GREQ_LAYER_CNT					(0x234UL)
	#define FLD_LAYER0_GREQ_CNT                    	REG_FLD_MSB_LSB(5, 0)
	#define FLD_LAYER1_GREQ_CNT                    	REG_FLD_MSB_LSB(13, 8)
	#define FLD_LAYER2_GREQ_CNT                    	REG_FLD_MSB_LSB(21, 16)
	#define FLD_LAYER3_GREQ_CNT                    	REG_FLD_MSB_LSB(29, 24)

#define DISP_REG_OVL_GDRDY_PRD_NUM					(0x238UL)
	#define FLD_GDRDY_PRD_NUM                     	REG_FLD_MSB_LSB(23, 0)

#define DISP_REG_OVL_FLOW_CTRL_DBG					(0x240UL)
	#define FLD_FSM_STATE	                       	REG_FLD_MSB_LSB(9, 0)
	#define FLD_ADDCON_IDLE	                       	REG_FLD_MSB_LSB(10, 10)
	#define FLD_BLEND_IDLE	                       	REG_FLD_MSB_LSB(11, 11)
	#define FLD_OVL_OUT_VALID                     	REG_FLD_MSB_LSB(12, 12)
	#define FLD_OVL_OUT_READY                     	REG_FLD_MSB_LSB(13, 13)
	#define FLD_OUT_IDLE                           	REG_FLD_MSB_LSB(15, 15)
	#define FLD_RDMA3_IDLE                         	REG_FLD_MSB_LSB(16, 16)
	#define FLD_RDMA2_IDLE                         	REG_FLD_MSB_LSB(17, 17)
	#define FLD_RDMA1_IDLE                         	REG_FLD_MSB_LSB(18, 18)
	#define FLD_RDMA0_IDLE                         	REG_FLD_MSB_LSB(19, 19)
	#define FLD_RST                              	REG_FLD_MSB_LSB(20, 20)
	#define FLD_TRIG                              	REG_FLD_MSB_LSB(21, 21)
	#define FLD_FRAME_HWRST_DONE                   	REG_FLD_MSB_LSB(23, 23)
	#define FLD_FRAME_SWRST_DONE                   	REG_FLD_MSB_LSB(24, 24)
	#define FLD_FRAME_UNDER_RUN                   	REG_FLD_MSB_LSB(25, 25)
	#define FLD_FRAME_DONE                        	REG_FLD_MSB_LSB(26, 26)
	#define FLD_OVL_RUNNING                        	REG_FLD_MSB_LSB(27, 27)
	#define FLD_OVL_START                        	REG_FLD_MSB_LSB(28, 28)
	#define FLD_OVL_CLR                         	REG_FLD_MSB_LSB(29, 29)
	#define FLD_REG_UPDATE                        	REG_FLD_MSB_LSB(30, 30)
	#define FLD_OVL_UPD_REG                        	REG_FLD_MSB_LSB(31, 31)

#define DISP_REG_OVL_ADDCON_DBG						(0x244UL)
	#define ADDCON_DBG_FLD_ROI_X					REG_FLD_MSB_LSB(12, 0)
	#define ADDCON_DBG_FLD_L0_WIN_HIT				REG_FLD_MSB_LSB(14, 14)
	#define ADDCON_DBG_FLD_L1_WIN_HIT				REG_FLD_MSB_LSB(15, 15)
	#define ADDCON_DBG_FLD_ROI_Y					REG_FLD_MSB_LSB(28, 16)
	#define ADDCON_DBG_FLD_L2_WIN_HIT				REG_FLD_MSB_LSB(30, 30)
	#define ADDCON_DBG_FLD_L3_WIN_HIT				REG_FLD_MSB_LSB(31, 31)

#define DISP_REG_OVL_RDMA0_DBG						(0x24CUL)
	#define RDMA0_DBG_FLD_RDMA0_WRAM_RST_CS			REG_FLD_MSB_LSB(2, 0)
	#define RDMA0_DBG_FLD_RDMA0_LAYER_GREQ          REG_FLD_MSB_LSB(3, 3)
	#define RDMA0_DBG_FLD_RDMA0_OUT_DATA			REG_FLD_MSB_LSB(27 4)
	#define RDMA0_DBG_FLD_RDMA0_OUT_READY			REG_FLD_MSB_LSB(28, 28)
	#define RDMA0_DBG_FLD_RDMA0_OUT_VALID			REG_FLD_MSB_LSB(29, 29)
	#define RDMA0_DBG_FLD_RDMA0_SMI_BUSY			REG_FLD_MSB_LSB(30, 30)
	#define RDMA0_DBG_FLD_RDMA0_SMI_GREQ			REG_FLD_MSB_LSB(31, 31)

#define DISP_REG_OVL_RDMA1_DBG						(0x250UL)
#define DISP_REG_OVL_RDMA2_DBG						(0x254UL)
#define DISP_REG_OVL_RDMA3_DBG						(0x258UL)
#define DISP_REG_OVL_L0_CLR							(0x25cUL)
	#define CLR_FLD_BLUE                   				REG_FLD_MSB_LSB(7, 0)
	#define CLR_FLD_GREEN                  				REG_FLD_MSB_LSB(15, 8)
	#define CLR_FLD_RED                  				REG_FLD_MSB_LSB(23, 16)
	#define CLR_FLD_ALPHA                  				REG_FLD_MSB_LSB(31, 24)

#define DISP_REG_OVL_L1_CLR							(0x260UL)
#define DISP_REG_OVL_L2_CLR							(0x264UL)
#define DISP_REG_OVL_L3_CLR							(0x268UL)
#define DISP_REG_OVL_LC_CLR							(0x26cUL)
#define DISP_REG_OVL_CRC							(0x270UL)
#define DISP_REG_OVL_LC_CON							(0x280UL)
	#define CON_FLD_ALPHA                  				REG_FLD_MSB_LSB(7, 0)
	#define CON_FLD_ALPHA_EN                  			REG_FLD_MSB_LSB(8, 8)
	#define CON_FLD_EN_3D                     			REG_FLD_MSB_LSB(20, 20)
	#define CON_FLD_LAND_SCAPE                 			REG_FLD_MSB_LSB(21, 21)
	#define CON_FLD_R_FIST                     			REG_FLD_MSB_LSB(22, 22)
	#define CON_FLD_LAYER_SRC                  			REG_FLD_MSB_LSB(29, 28)
	#define CON_FLD_SRCKEY_EN                  			REG_FLD_MSB_LSB(30, 30)
	#define CON_FLD_DSTKEY_EN                  			REG_FLD_MSB_LSB(31, 31)

#define DISP_REG_OVL_LC_SRCKEY						(0x284UL)
#define DISP_REG_OVL_LC_SRC_SIZE					(0x288UL)
	#define LC_SRC_SIZE_FLD_WIDTH                  		REG_FLD_MSB_LSB(12, 0)
	#define LC_SRC_SIZE_FLD_HEIGHT                 		REG_FLD_MSB_LSB(28, 16)

#define DISP_REG_OVL_LC_OFFSET						(0x28cUL)
	#define LC_OFFSET_FLD_XOFF                  		REG_FLD_MSB_LSB(11, 0)
	#define LC_OFFSET_FLD_YOFF                   		REG_FLD_MSB_LSB(27, 16)

#define DISP_REG_OVL_LC_SRC_SEL						(0x290UL)
	#define FLD_CONST_LAYER_SEL                  		REG_FLD_MSB_LSB(2, 0)
	#define FLD_LC_SA_SEL                          		REG_FLD_MSB_LSB(17, 16)
	#define FLD_LC_SRGB_SEL                        		REG_FLD_MSB_LSB(19, 18)
	#define FLD_LC_DA_SEL                        		REG_FLD_MSB_LSB(21, 20)
	#define FLD_LC_DRGB_SEL                        		REG_FLD_MSB_LSB(23, 22)
	#define FLD_LC_SA_SEL_EXT                     		REG_FLD_MSB_LSB(24, 24)
	#define FLD_LC_SRGB_SEL_EXT                    		REG_FLD_MSB_LSB(25, 25)
	#define FLD_LC_DA_SEL_EXT                     		REG_FLD_MSB_LSB(26, 26)
	#define FLD_LC_DRGB_SEL_EXT                    		REG_FLD_MSB_LSB(27, 27)
	#define FLD_LC_CONST_BLD                    		REG_FLD_MSB_LSB(28, 28)
	#define FLD_LC_SRGB_SEL_EXT2                   		REG_FLD_MSB_LSB(29, 29)
	#define FLD_LC_BLEND_RND_SHT                   		REG_FLD_MSB_LSB(30, 30)
	#define FLD_LC_SURFL_EN                       		REG_FLD_MSB_LSB(31, 31)

#define DISP_REG_OVL_BANK_CON						(0x29cUL)
	#define FLD_OVL_BANK_CON                       		REG_FLD_MSB_LSB(4, 0)

#define DISP_REG_OVL_FUNC_DCM0						(0x2a0UL)
#define DISP_REG_OVL_FUNC_DCM1						(0x2a4UL)
#define DISP_REG_OVL_DVFS_L0_ROI					(0x2B0UL)
	#define FLD_DVFS_ROI_TB                       		REG_FLD_MSB_LSB(12, 0)
	#define FLD_DVFS_ROI_BB                       		REG_FLD_MSB_LSB(28, 16)

#define DISP_REG_OVL_DVFS_L1_ROI					(0x2B4UL)
#define DISP_REG_OVL_DVFS_L2_ROI					(0x2B8UL)
#define DISP_REG_OVL_DVFS_L3_ROI					(0x2BCUL)
#define DISP_REG_OVL_DVFS_EL0_ROI					(0x2C0UL)
#define DISP_REG_OVL_DVFS_EL1_ROI					(0x2C4UL)
#define DISP_REG_OVL_DVFS_EL2_ROI					(0x2C8UL)
#define DISP_REG_OVL_DATAPATH_EXT_CON				(0x324UL)
	#define EXT_CON_FLD_EL0_EN                    		REG_FLD_MSB_LSB(0, 0)
	#define EXT_CON_FLD_EL1_EN                    		REG_FLD_MSB_LSB(1, 1)
	#define EXT_CON_FLD_EL2_EN                    		REG_FLD_MSB_LSB(2, 2)
	#define EXT_CON_FLD_EL0_GPU_MODE               		REG_FLD_MSB_LSB(8, 8)
	#define EXT_CON_FLD_EL1_GPU_MODE               		REG_FLD_MSB_LSB(9, 9)
	#define EXT_CON_FLD_EL2_GPU_MODE               		REG_FLD_MSB_LSB(10, 10)
	#define EXT_CON_FLD_EL0_LAYER_SEL              		REG_FLD_MSB_LSB(18, 16)
	#define EXT_CON_FLD_EL1_LAYER_SEL              		REG_FLD_MSB_LSB(22, 20)
	#define EXT_CON_FLD_EL2_LAYER_SEL              		REG_FLD_MSB_LSB(26, 24)

#define DISP_REG_OVL_EL0_CON						(0x330UL)
#define DISP_REG_OVL_EL0_SRCKEY						(0x334UL)
#define DISP_REG_OVL_EL0_SRC_SIZE					(0x338UL)
#define DISP_REG_OVL_EL0_OFFSET						(0x33CUL)
#define DISP_REG_OVL_EL0_ADDR						(0xFB0UL)
#define DISP_REG_OVL_EL0_PITCH						(0x344UL)
#define DISP_REG_OVL_EL0_TILE						(0x348UL)
#define DISP_REG_OVL_EL0_CLIP						(0x34CUL)
#define DISP_REG_OVL_EL1_CON						(0x350UL)
#define DISP_REG_OVL_EL1_SRCKEY						(0x354UL)
#define DISP_REG_OVL_EL1_SRC_SIZE					(0x358UL)
#define DISP_REG_OVL_EL1_OFFSET						(0x35CUL)
#define DISP_REG_OVL_EL1_ADDR						(0xFB4UL)
#define DISP_REG_OVL_EL1_PITCH						(0x364UL)
#define DISP_REG_OVL_EL1_TILE						(0x368UL)
#define DISP_REG_OVL_EL1_CLIP						(0x36CUL)
#define DISP_REG_OVL_EL2_CON						(0x370UL)
#define DISP_REG_OVL_EL2_SRCKEY						(0x374UL)
#define DISP_REG_OVL_EL2_SRC_SIZE					(0x378UL)
#define DISP_REG_OVL_EL2_OFFSET						(0x37CUL)
#define DISP_REG_OVL_EL2_ADDR						(0xFB8UL)
#define DISP_REG_OVL_EL2_PITCH						(0x384UL)
#define DISP_REG_OVL_EL2_TILE						(0x388UL)
#define DISP_REG_OVL_EL2_CLIP						(0x38CUL)
#define DISP_REG_OVL_EL0_CLEAR						(0x390UL)
#define DISP_REG_OVL_EL1_CLEAR						(0x394UL)
#define DISP_REG_OVL_EL2_CLEAR						(0x398UL)
#define DISP_REG_OVL_SECURE							(0xFC0UL)
	#define FLD_L0_SECURE                     		REG_FLD_MSB_LSB(0, 0)
	#define FLD_L1_SECURE                     		REG_FLD_MSB_LSB(1, 1)
	#define FLD_L2_SECURE                     		REG_FLD_MSB_LSB(2, 2)
	#define FLD_L3_SECURE                     		REG_FLD_MSB_LSB(3, 3)

#endif

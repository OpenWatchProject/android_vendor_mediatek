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

#define LOG_TAG "DSI"

#include "ddp_drv.h"
#include "ddp_dsi.h"
#include "ddp_log.h"

#include "ddp_reg.h"
#include "display_tui.h"

#define DSI_OUTREG32(cmdq, addr, val) DISP_REG_SET(cmdq, addr, val)
#define DSI_BACKUPREG32(cmdq, hSlot, idx, addr) DISP_REG_BACKUP(cmdq, hSlot, idx, addr)
#define DSI_POLLREG32(cmdq, addr, mask, value) DISP_REG_CMDQ_POLLING(cmdq, addr, value, mask)
#define DSI_INREG32(type, addr) INREG32(addr)
#define DSI_READREG32(type, dst, src) mt_reg_sync_writel(INREG32(src), dst)

#define BIT_TO_VALUE(TYPE, bit)  \
do { \
	TYPE r;\
	*(unsigned long *)(&r) = ((unsigned int)0x00000000);\
	r.bit = ~(r.bit);\
	r;\
} while (0)

#define DSI_MASKREG32(cmdq, REG, MASK, VALUE)	DISP_REG_MASK((cmdq), (REG), (VALUE), (MASK))

#define DSI_OUTREGBIT(cmdq, TYPE, REG, bit, value)  \
do {\
	TYPE r;\
	TYPE v;\
	if (cmdq) {\
		*(unsigned int *)(&r) = ((unsigned int)0x00000000); \
		r.bit = ~(r.bit);  \
		*(unsigned int *)(&v) = ((unsigned int)0x00000000); \
		v.bit = value; \
		DISP_REG_MASK(cmdq, &REG, AS_UINT32(&v), AS_UINT32(&r)); \
	} else { \
		mt_reg_sync_writel(INREG32(&REG), &r); \
		r.bit = (value); \
		DISP_REG_SET(cmdq, &REG, INREG32(&r)); \
	} \
} while (0)

#define DSI_MODULE_BEGIN(x)	(x == DISP_MODULE_DSIDUAL?0:DSI_MODULE_to_ID(x))
#define DSI_MODULE_END(x)		(x == DISP_MODULE_DSIDUAL?1:DSI_MODULE_to_ID(x))
#define DSI_MODULE_to_ID(x)	(x == DISP_MODULE_DSI0?0:1)
#define DIFF_CLK_LANE_LP 0X10

struct DSI_REGS *DSI_REG[2];
struct DSI_PHY_REGS *DSI_PHY_REG[2];
struct DSI_CMDQ_REGS *DSI_CMDQ_REG[2];
struct DSI_VM_CMDQ_REGS *DSI_VM_CMD_REG[2];

static const char *_dsi_cmd_mode_parse_state(unsigned int state)
{
	switch (state) {
	case 0x0001:
		return "idle";
	case 0x0002:
		return "Reading command queue for header";
	case 0x0004:
		return "Sending type-0 command";
	case 0x0008:
		return "Waiting frame data from RDMA for type-1 command";
	case 0x0010:
		return "Sending type-1 command";
	case 0x0020:
		return "Sending type-2 command";
	case 0x0040:
		return "Reading command queue for type-2 data";
	case 0x0080:
		return "Sending type-3 command";
	case 0x0100:
		return "Sending BTA";
	case 0x0200:
		return "Waiting RX-read data";
	case 0x0400:
		return "Waiting SW RACK for RX-read data";
	case 0x0800:
		return "Waiting TE";
	case 0x1000:
		return "Get TE";
	case 0x2000:
		return "Waiting SW RACK for TE";
	case 0x4000:
		return "Waiting external TE";
	case 0x8000:
		return "Get external TE";
	default:
		return "unknown";
	}
	return "unknown";
}

static const char *dsi_mode_spy(LCM_DSI_MODE_CON mode)
{
	switch (mode) {
	case CMD_MODE:
		return "CMD_MODE";
	case SYNC_PULSE_VDO_MODE:
		return "SYNC_PULSE_VDO_MODE";
	case SYNC_EVENT_VDO_MODE:
		return "SYNC_EVENT_VDO_MODE";
	case BURST_VDO_MODE:
		return "BURST_VDO_MODE";
	default:
		return "unknown";
	}
}

unsigned int dsi_phy_get_clk(enum DISP_MODULE_ENUM module)
{
	int i = 0;
	int j = 0;

	unsigned int pcw = (DSI_PHY_REG[i]->MIPITX_DSI_PLL_CON0.RG_DSI0_PLL_SDM_PCW >> 24) & 0xFF;
	unsigned int prediv = 1;
	unsigned int posdiv = (1 << (DSI_PHY_REG[i]->MIPITX_DSI_PLL_CON1.RG_DSI0_PLL_POSDIV));


	DDPMSG("%s, pcw: %d, prediv: %d, posdiv: %d\n", __func__, pcw,
		prediv, posdiv);
	j = prediv * posdiv;
	if (j > 0)
		return 26 * pcw / j;

	return 0;
}

enum DSI_STATUS DSI_DumpRegisters(enum DISP_MODULE_ENUM module, int level)
{
	UINT32 i = 0;
	UINT32 k = 0;

	DDPDUMP("== DISP DSI REGS ==\n");
	if (level >= 0) {
		for (i = DSI_MODULE_BEGIN(module); i <= DSI_MODULE_END(module); i++) {
			unsigned int DSI_DBG8_Status;
			unsigned int DSI_DBG9_Status;
			unsigned long dsi_base_addr = (unsigned long)DSI_REG[i];

			if (DSI_REG[0]->DSI_MODE_CTRL.MODE == CMD_MODE) {
				unsigned int DSI_DBG6_Status = (INREG32(dsi_base_addr + 0x160)) & 0xffff;

				DDPDUMP("DSI%d state6(cmd mode):%s\n",
					i, _dsi_cmd_mode_parse_state(DSI_DBG6_Status));
			} else {
				unsigned int DSI_DBG7_Status = (INREG32(dsi_base_addr + 0x164)) & 0xff;

				DDPDUMP("DSI%d state7(vdo mode):%s\n",
					i, _dsi_vdo_mode_parse_state(DSI_DBG7_Status));
			}
			DSI_DBG8_Status = (INREG32(dsi_base_addr + 0x168)) & 0x3fff;
			DDPDUMP("DSI%d state8 WORD_COUNTER(cmd mode):%s\n",
				i, _dsi_cmd_mode_parse_state(DSI_DBG8_Status));
			DSI_DBG9_Status = (INREG32(dsi_base_addr + 0x16C)) & 0x3fffff;
			DDPDUMP("DSI%d state9 LINE_COUNTER(cmd mode):%s\n",
				i, _dsi_cmd_mode_parse_state(DSI_DBG9_Status));
		}
	}
	if (level >= 1) {
		for (i = DSI_MODULE_BEGIN(module); i <= DSI_MODULE_END(module); i++) {
			unsigned long dsi_base_addr = (unsigned long)DSI_REG[i];
			unsigned long mipi_base_addr = (unsigned long)DSI_PHY_REG[i];

			DDPDUMP("== DSI%d REGS ==\n", i);
			for (k = 0; k < sizeof(struct DSI_REGS); k += 16) {
				DDPDUMP("0x%04x: 0x%08x 0x%08x 0x%08x 0x%08x\n", k,
					INREG32(dsi_base_addr + k),
					INREG32(dsi_base_addr + k + 0x4),
					INREG32(dsi_base_addr + k + 0x8),
					INREG32(dsi_base_addr + k + 0xc));
			}

			DDPDUMP("- DSI%d CMD REGS -\n", i);
			for (k = 0; k < 32; k += 16) { /* only dump first 32 bytes cmd */
				DDPDUMP("0x%04x: 0x%08x 0x%08x 0x%08x 0x%08x\n", k,
					INREG32((dsi_base_addr + 0x200 + k)),
					INREG32((dsi_base_addr + 0x200 + k + 0x4)),
					INREG32((dsi_base_addr + 0x200 + k + 0x8)),
					INREG32((dsi_base_addr + 0x200 + k + 0xc)));
			}

			DDPDUMP("== DSI_PHY%d REGS ==\n", i);
			for (k = 0; k < sizeof(struct DSI_PHY_REGS); k += 16) {
				DDPDUMP("0x%04x: 0x%08x 0x%08x 0x%08x 0x%08x\n", k,
					INREG32((mipi_base_addr + k)),
					INREG32((mipi_base_addr + k + 0x4)),
					INREG32((mipi_base_addr + k + 0x8)),
					INREG32((mipi_base_addr + k + 0xc)));
			}
		}
	}

	return DSI_STATUS_OK;
}

void dsi_analysis(enum DISP_MODULE_ENUM module)
{
	int i = 0;

	DDPDUMP("== DISP DSI ANALYSIS ==\n");
	for (i = DSI_MODULE_BEGIN(module); i <= DSI_MODULE_END(module); i++) {
		DDPDUMP("MIPITX Clock: %d\n", dsi_phy_get_clk(module));
		DDPDUMP("DSI%d Start:%x, Busy:%d, DSI_DUAL_EN:%d, MODE:%s, High Speed:%d, FSM State:%s\n",
			i, DSI_REG[i]->DSI_START.DSI_START, DSI_REG[i]->DSI_INTSTA.BUSY,
			DSI_REG[i]->DSI_COM_CTRL.DSI_DUAL_EN, dsi_mode_spy(DSI_REG[i]->DSI_MODE_CTRL.MODE),
			DSI_REG[i]->DSI_PHY_LCCON.LC_HS_TX_EN,
			_dsi_cmd_mode_parse_state(DSI_REG[i]->DSI_STATE_DBG6.CMTRL_STATE));

		DDPDUMP("DSI%d IRQ,RD_RDY:%d, CMD_DONE:%d, SLEEPOUT_DONE:%d, TE_RDY:%d, VM_CMD_DONE:%d, VM_DONE:%d\n",
			i, DSI_REG[i]->DSI_INTSTA.RD_RDY, DSI_REG[i]->DSI_INTSTA.CMD_DONE,
			DSI_REG[i]->DSI_INTSTA.SLEEPOUT_DONE, DSI_REG[i]->DSI_INTSTA.TE_RDY,
			DSI_REG[i]->DSI_INTSTA.VM_CMD_DONE, DSI_REG[i]->DSI_INTSTA.VM_DONE);

		DDPDUMP("DSI%d Lane Num:%d, Ext_TE_EN:%d, Ext_TE_Edge:%d, HSTX_CKLP_EN:%d\n", i,
			DSI_REG[i]->DSI_TXRX_CTRL.LANE_NUM,
			DSI_REG[i]->DSI_TXRX_CTRL.EXT_TE_EN,
			DSI_REG[i]->DSI_TXRX_CTRL.EXT_TE_EDGE,
			DSI_REG[i]->DSI_TXRX_CTRL.HSTX_CKLP_EN);

		DDPDUMP("DSI%d LFR En:%d, LFR MODE:%d, LFR TYPE:%d, LFR SKIP NUMBER:%d\n", i,
			DSI_REG[i]->DSI_LFR_CON.LFR_EN,
			DSI_REG[i]->DSI_LFR_CON.LFR_MODE,
			DSI_REG[i]->DSI_LFR_CON.LFR_TYPE, DSI_REG[i]->DSI_LFR_CON.LFR_SKIP_NUM);
	}
}


static bool waitRDDone;
static bool wait_vm_cmd_done;
static bool wait_sleep_out_done;
static int s_isDsiPowerOn;
static int dsi_currect_mode;
static int dsi_force_config;
static int dsi0_te_enable = 1;
static const LCM_UTIL_FUNCS lcm_utils_dsi0;


int ddp_dsi_dump(enum DISP_MODULE_ENUM module, int level)
{
	if(level >=0)
	{
		dsi_analysis(module);
        DSI_DumpRegisters(module, level);
	}
	else if(level >=1)
	{
		DSI_DumpRegisters(module, level);
	}
}


enum DSI_STATUS DSI_Start(enum DISP_MODULE_ENUM module, cmdqRecHandle cmdq)
{
	int i = 0;

	if (module != DISP_MODULE_DSIDUAL) {
		for (i = DSI_MODULE_BEGIN(module); i <= DSI_MODULE_END(module); i++) {
			DSI_OUTREGBIT(cmdq, struct DSI_START_REG, DSI_REG[i]->DSI_START, DSI_START, 0);
			DSI_OUTREGBIT(cmdq, struct DSI_START_REG, DSI_REG[i]->DSI_START, DSI_START, 1);
		}
	} else {
		/* TODO: do we need this? */
		DSI_OUTREGBIT(cmdq, struct DSI_START_REG, DSI_REG[0]->DSI_START, DSI_START, 0);
		DSI_OUTREGBIT(cmdq, struct DSI_START_REG, DSI_REG[0]->DSI_START, DSI_START, 1);
	}

	return DSI_STATUS_OK;
}


static enum DSI_STATUS DSI_Reset(enum DISP_MODULE_ENUM module, cmdqRecHandle cmdq)
{
	int i = 0;
	unsigned int irq_en[2];
	/* DSI_RESET Protect: backup & disable dsi interrupt */
	for (i = DSI_MODULE_BEGIN(module); i <= DSI_MODULE_END(module); i++) {
		irq_en[i] = AS_UINT32(&DSI_REG[i]->DSI_INTEN);
		DSI_OUTREG32(NULL, &DSI_REG[i]->DSI_INTEN, 0);
		DDPMSG("\nDSI_RESET backup dsi%d irq:0x%08x ", i, irq_en[i]);
	}

	/* do reset */
	for (i = DSI_MODULE_BEGIN(module); i <= DSI_MODULE_END(module); i++) {
		DSI_OUTREGBIT(cmdq, struct DSI_COM_CTRL_REG, DSI_REG[i]->DSI_COM_CTRL, DSI_RESET, 1);
		DSI_OUTREGBIT(cmdq, struct DSI_COM_CTRL_REG, DSI_REG[i]->DSI_COM_CTRL, DSI_RESET, 0);
	}

	/* DSI_RESET Protect: restore dsi interrupt */
	for (i = DSI_MODULE_BEGIN(module); i <= DSI_MODULE_END(module); i++) {
		DSI_OUTREG32(NULL, &DSI_REG[i]->DSI_INTEN, irq_en[i]);
		DDPMSG("\nDSI_RESET restore dsi%d irq:0x%08x ", i,
			AS_UINT32(&DSI_REG[i]->DSI_INTEN));
	}
	return DSI_STATUS_OK;
}

static int _dsi_is_video_mode(enum DISP_MODULE_ENUM module)
{
	int i = 0;

	for (i = DSI_MODULE_BEGIN(module); i <= DSI_MODULE_END(module); i++) {
		if (DSI_REG[i]->DSI_MODE_CTRL.MODE == CMD_MODE)
			return 0;
		else
			return 1;
	}
	return 0;
}

void DSI_WaitForNotBusy(enum DISP_MODULE_ENUM module, cmdqRecHandle cmdq)
{
	int i = 0;
	unsigned int count = 0;
	unsigned int tmp = 0;
	int ret = 0;
	if(cmdq)
	{
		DSI_POLLREG32(cmdq, &DSI_REG[i]->DSI_INTSTA, 0x80000000, 0x0);
		return;
	}


	/*...dsi video is always in busy state...*/
	if (_dsi_is_video_mode(module))
	{
		return ;
	}

	for(i = DSI_MODULE_BEGIN(module); i <= DSI_MODULE_END(module); i++)
	{
		if(cmdq) {
			DSI_POLLREG32(cmdq, &DSI_REG[i]->DSI_INTSTA, 0x80000000, 0x0);
		} else {

#ifdef DISP_TUI_OPTION_LCM_BUSY_POLLING
			ret = disp_tui_poll(&DSI_REG[i]->DSI_INTSTA, 0x80000000, 0x0, -1);
#else
			/* use irq */
#endif
		}

        if (0 != ret)
        {
	        DDPERR("dsi wait not busy timeout\n");
            DSI_DumpRegisters(module, 1);
	        DSI_Reset(module, NULL);
        }

	}
}

int ddp_dsi_is_busy(enum DISP_MODULE_ENUM module)
{
	int i;
	unsigned int is_busy = 0;
	for(i = DSI_MODULE_BEGIN(module); i <= DSI_MODULE_END(module);i++)
	{
		is_busy |= DSI_REG[i]->DSI_INTSTA.BUSY;
	}
	return is_busy;
}

void DSI_set_cmdq(enum DISP_MODULE_ENUM module, cmdqRecHandle cmdq, unsigned int *pdata,
		  unsigned int queue_size, unsigned char force_update)
{
	/* DISPFUNC(); */

	int j = 0;
	int i = 0;

	/* DISPCHECK("DSI_set_cmdq, module=%s, cmdq=0x%08x\n", ddp_get_module_name(module), cmdq); */

	for (i = DSI_MODULE_BEGIN(module); i <= DSI_MODULE_END(module); i++) {
		if (0 != DSI_REG[i]->DSI_MODE_CTRL.MODE) {
			;
			/* video mode, do nothing*/
		} else {
			ASSERT(queue_size <= 32);
			DSI_WaitForNotBusy(module, cmdq);
#ifdef ENABLE_DSI_ERROR_REPORT
			if ((pdata[0] & 1)) {
				memcpy(_dsi_cmd_queue, pdata, queue_size * 4);
				_dsi_cmd_queue[queue_size++] = 0x4;
				pdata = (unsigned int *)_dsi_cmd_queue;
			} else {
				pdata[0] |= 4;
			}
#endif
			for (j = 0; j < queue_size; j++) {
				DSI_OUTREG32(cmdq, &DSI_CMDQ_REG[i]->data[j],
					     AS_UINT32((pdata + j)));
			}

			DSI_OUTREG32(cmdq, &DSI_REG[i]->DSI_CMDQ_SIZE, queue_size);

			if (force_update) {
				DSI_Start(module, cmdq);
				DSI_WaitForNotBusy(module, cmdq);
			}
		}
	}
}


#define MIPITX_INREG32(addr) \
({ \
	unsigned int val = 0; val = INREG32(addr);\
	if (dsi_reg_op_debug) \
		DDPMSG("[mipitx/inreg]%p=0x%08x\n", (void *)addr, val); \
	val; })

#define MIPITX_OUTREG32(addr, val) {\
		if (dsi_reg_op_debug) {	\
			DDPMSG("[mipitx/reg]%p=0x%08x\n", (void *)addr, val);\
		} \
		mt_reg_sync_writel(val, addr);\
	}

#define MIPITX_OUTREGBIT(TYPE, REG, bit, value)  \
	{\
		do {	\
			TYPE r;\
			mt_reg_sync_writel(INREG32(&REG), &r);	  \
			r.bit = value;	  \
			MIPITX_OUTREG32(&REG, AS_UINT32(&r));	  \
			} while (0);\
	}

#define MIPITX_MASKREG32(x, y, z)  MIPITX_OUTREG32(x, (MIPITX_INREG32(x)&~(y))|(z))

typedef struct {
	unsigned int lcm_width;
	unsigned int lcm_height;
	cmdqRecHandle *handle;
	bool enable;
	struct DSI_REGS regBackup;
	unsigned int cmdq_size;
	LCM_DSI_PARAMS dsi_params;
}
t_dsi_context;
t_dsi_context _dsi_context[DSI_INTERFACE_NUM];

int ddp_dsi_trigger(enum DISP_MODULE_ENUM module, cmdqRecHandle cmdq)
{
	int i = 0;
	unsigned int data_array[16];
	DDPMSG("%s, start!\n", (__func__));

	if (_dsi_context[i].dsi_params.mode == CMD_MODE) {
		data_array[0] = 0x002c3909;
		DSI_set_cmdq(module, cmdq, data_array, 1, 0);

		if (module == DISP_MODULE_DSIDUAL) {
			/*
			 * DSI1 is only used for triggering video data; thus pull up DSI_DUAL_EN.
			 * Otherwise, pull down DSI_DUAL_EN after triggering video data is done.
			 */
			DSI_OUTREGBIT(cmdq, struct DSI_START_REG, DSI_REG[0]->DSI_START, DSI_START, 0);
			DSI_OUTREGBIT(cmdq, struct DSI_COM_CTRL_REG, DSI_REG[1]->DSI_COM_CTRL, DSI_DUAL_EN, 1);
		}
	}
	DSI_Start(module, cmdq);

	if ((_dsi_context[i].dsi_params.mode == CMD_MODE) && (module == DISP_MODULE_DSIDUAL)) {

		DSI_OUTREGBIT(cmdq, struct DSI_COM_CTRL_REG, DSI_REG[1]->DSI_COM_CTRL, DSI_DUAL_EN, 0);
	}
	DDPMSG("%s, done!\n", (__func__));

	return 0;
}

enum DISP_MODULE_ENUM ddp_dsi_get_module()
{
	if(DSI_REG[1]->DSI_COM_CTRL.DSI_DUAL_EN)
		return DISP_MODULE_DSIDUAL;
	else
		return DISP_MODULE_DSI0;
}

enum DDP_MODE ddp_dsi_get_mode(enum DISP_MODULE_ENUM module)
{
	if(_dsi_is_video_mode(module))
		return DDP_VIDEO_MODE;
	else
		return DDP_CMD_MODE;
}

int ddp_dsi_reg_init(void)
{
	enum DSI_STATUS ret = DSI_STATUS_OK;
	int i = 0;
	DISPFUNC();
	DSI_REG[0] = (struct DSI_REGS *)DISPSYS_DSI0_BASE;
	DSI_PHY_REG[0] = (struct DSI_PHY_REGS *)DDP_REG_BASE_MIPITX0;
	DSI_CMDQ_REG[0] = (struct DSI_CMDQ_REGS *)(DISPSYS_DSI0_BASE+0x200);
	DSI_REG[1] = (struct DSI_REGS *)0;
	DSI_PHY_REG[1] = (struct DSI_PHY_REGS *)0;
	DSI_CMDQ_REG[1] = (struct DSI_CMDQ_REGS *)0;
	DSI_VM_CMD_REG[0] = (struct DSI_VM_CMDQ_REGS *)(DISPSYS_DSI0_BASE + 0x134);

	DSI_VM_CMD_REG[1] = (struct DSI_VM_CMDQ_REGS *)(0);

	DDPMSG("DSI0 Base Address:0x%x, PHY:0x%x\n", (unsigned long)DSI_REG[0], (unsigned long)DSI_PHY_REG[0]);
	DDPMSG("DSI1 Base Address:0x%x, PHY:0x%x\n", (unsigned long)DSI_REG[1], (unsigned long)DSI_PHY_REG[1]);

	return DSI_STATUS_OK;
}

const char *_dsi_vdo_mode_parse_state(unsigned int state)
{
	switch (state) {
	case 0x0001:
		return "Video mode idle";
	case 0x0002:
		return "Sync start packet";
	case 0x0004:
		return "Hsync active";
	case 0x0008:
		return "Sync end packet";
	case 0x0010:
		return "Hsync back porch";
	case 0x0020:
		return "Video data period";
	case 0x0040:
		return "Hsync front porch";
	case 0x0080:
		return "BLLP";
	case 0x0100:
		return "--";
	case 0x0200:
		return "Mix mode using command mode transmission";
	case 0x0400:
		return "Command transmission in BLLP";
	default:
		return "unknown";
	}

	return "unknown";
}

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

#define AS_UINT32(reg) DISP_REG_GET(reg)

#define DSI_POLLREG32(cmdq, addr,mask,value)  \
	do{\
		cmdqRecPoll(cmdq, disp_addr_convert((unsigned long)addr), value, mask);\
	}while(0);

#define DSI_OUTREG32(cmdq, addr, val) DISP_REG_SET(cmdq, addr,val)

#define BIT_TO_VALUE(TYPE,bit)  \
	do { \
		TYPE r;\
		*(unsigned int*)(&r) = ((unsigned int)0x00000000);	  \
		r.bit = ~(r.bit);\
		r;\
                } while (0);\

#define DSI_MASKREG32(cmdq, REG, MASK, VALUE)  	DISP_REG_MASK((cmdq), (REG), (VALUE), (MASK));

#define DSI_OUTREGBIT(cmdq, TYPE,REG,bit,value)  \
	do {\
		TYPE r;\
		TYPE v;\
		if(cmdq)		{*(unsigned int*)(&r) = ((unsigned int)0x00000000);r.bit = ~(r.bit);*(unsigned int*)(&v) = ((unsigned int)0x00000000);v.bit = value;DISP_REG_MASK(cmdq, &REG, AS_UINT32(&v), AS_UINT32(&r));}		\
		else			{r = *((TYPE*)&INREG32(&REG)); r.bit = (value);		DISP_REG_SET(cmdq, &REG, AS_UINT32(&r));}				\
	} while (0);

#define DSI_MODULE_BEGIN(x)	(x == DISP_MODULE_DSIDUAL?0:DSI_MODULE_to_ID(x))
#define DSI_MODULE_END(x)		(x == DISP_MODULE_DSIDUAL?1:DSI_MODULE_to_ID(x))
#define DSI_MODULE_to_ID(x)	(x == DISP_MODULE_DSI0?0:1)
#define DIFF_CLK_LANE_LP 0X10

static PDSI_REGS DSI_REG[2];
static PDSI_PHY_REGS DSI_PHY_REG[2];
static PDSI_CMDQ_REGS DSI_CMDQ_REG[2];
static PDSI_VM_CMDQ_REGS DSI_VM_CMD_REG[2];

const char* _dsi_cmd_mode_parse_state(unsigned int state)
{
	switch(state)
	{
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
			return "Reading command queue for data";
		case 0x0080:
			return "Sending type-3 command";
		case 0x0100:
			return "Sending BTA";
		case 0x0200:
			return "Waiting RX-read data ";
		case 0x0400:
			return "Waiting SW RACK for RX-read data";
		case 0x0800:
			return "Waiting TE";
		case 0x1000:
			return "Get TE ";
		case 0x2000:
			return "Waiting SW RACK for TE";
		case 0x4000:
			return "Waiting external TE";
		default:
			return "unknown";
	}
}

static const char *dsi_mode_spy(LCM_DSI_MODE_CON mode)
{
	switch (mode)
	{
		case CMD_MODE:	 			return "CMD_MODE";
		case SYNC_PULSE_VDO_MODE: 	return "SYNC_PULSE_VDO_MODE";
		case SYNC_EVENT_VDO_MODE: 	return "SYNC_EVENT_VDO_MODE";
		case BURST_VDO_MODE: 		return "BURST_VDO_MODE";
		default:						return "unknown";
	}
}


unsigned int dsi_phy_get_clk(DISP_MODULE_ENUM module)
{
	int i = 0;
	unsigned int pcw = DSI_PHY_REG[i]->MIPITX_DSI_PLL_CON2.RG_DSI0_MPPLL_SDM_PCW_H;
	unsigned int prediv = (1<<(DSI_PHY_REG[i]->MIPITX_DSI_PLL_CON0.RG_DSI0_MPPLL_PREDIV));
	unsigned int posdiv = (1<<(DSI_PHY_REG[i]->MIPITX_DSI_PLL_CON0.RG_DSI0_MPPLL_POSDIV));
	unsigned int txdiv0 = (1<<(DSI_PHY_REG[i]->MIPITX_DSI_PLL_CON0.RG_DSI0_MPPLL_TXDIV0));
	unsigned int txdiv1 = (1<<(DSI_PHY_REG[i]->MIPITX_DSI_PLL_CON0.RG_DSI0_MPPLL_TXDIV1));

	DDPMSG("%s, pcw: %d, prediv: %d, posdiv: %d, txdiv0: %d, txdiv1: %d\n", __func__, pcw, prediv, posdiv, txdiv0, txdiv1);

	for(i = DSI_MODULE_BEGIN(module);i <= DSI_MODULE_END(module);i++)
	{
		return 26 * pcw/prediv/4/txdiv0/txdiv1;
	}
}


DSI_STATUS DSI_DumpRegisters(DISP_MODULE_ENUM module, int level)
{
	UINT32 i;

	if(level >= 0)
	{
		if(module == DISP_MODULE_DSI0 || module == DISP_MODULE_DSIDUAL)
		{
			unsigned int DSI_DBG6_Status = (INREG32(DDP_REG_BASE_DSI0+0x160))&0xffff;
			DDPDUMP("DSI0 state:%s\n", _dsi_cmd_mode_parse_state(DSI_DBG6_Status));
			DDPDUMP("DSI Mode: lane num: transfer count: status:\n");
		}
	}
	if(level >= 1)
	{
		if(module == DISP_MODULE_DSI0 || module == DISP_MODULE_DSIDUAL)
		{
			unsigned int DSI_DBG6_Status = (INREG32(DDP_REG_BASE_DSI0+0x160))&0xffff;

			DDPDUMP("---------- Start dump DSI0 registers ----------\n");

			for (i = 0; i < sizeof(DSI_REGS); i += 16)
			{
				DDPDUMP("DSI+%04x : 0x%08x  0x%08x  0x%08x  0x%08x\n", i, INREG32(DDP_REG_BASE_DSI0 + i), INREG32(DDP_REG_BASE_DSI0 + i + 0x4), INREG32(DDP_REG_BASE_DSI0 + i + 0x8), INREG32(DDP_REG_BASE_DSI0 + i + 0xc));
			}

			for (i = 0; i < sizeof(DSI_CMDQ_REGS); i += 16)
			{
				DDPDUMP("DSI_CMD+%04x : 0x%08x  0x%08x  0x%08x  0x%08x\n", i, INREG32((DDP_REG_BASE_DSI0+0x200+i)), INREG32((DDP_REG_BASE_DSI0+0x200+i+0x4)), INREG32((DDP_REG_BASE_DSI0+0x200+i+0x8)), INREG32((DDP_REG_BASE_DSI0+0x200+i+0xc)));
			}

	    	for (i = 0; i < sizeof(DSI_PHY_REGS); i += 16)
		    {
		       	DDPDUMP("DSI_PHY+%04x : 0x%08x    0x%08x  0x%08x  0x%08x\n", i, INREG32((DISPSYS_DSI0_BASE+i)), INREG32((DISPSYS_BASE_MIPITX0+i+0x4)), INREG32((DISPSYS_BASE_MIPITX0+i+0x8)), INREG32((DISPSYS_BASE_MIPITX0+i+0xc)));
		    }
		}
	}

	return DSI_STATUS_OK;
}


void dsi_analysis(DISP_MODULE_ENUM module)
{
	int i = 0;
	DDPDUMP("==DISP DSI ANALYSIS==\n");
	for(i = DSI_MODULE_BEGIN(module);i <= DSI_MODULE_END(module);i++)
	{
		DDPDUMP("MIPITX Clock: %d\n", dsi_phy_get_clk(module));
		DDPDUMP("DSI%d Start:%x, Busy:%d, DSI_DUAL_EN:%d, MODE:%s, High Speed:%d, FSM State:%s\n", i,
			DSI_REG[i]->DSI_START.DSI_START,
			DSI_REG[i]->DSI_INTSTA.BUSY,
			DSI_REG[i]->DSI_COM_CTRL.DSI_DUAL_EN,
			dsi_mode_spy(DSI_REG[i]->DSI_MODE_CTRL.MODE),
			DSI_REG[i]->DSI_PHY_LCCON.LC_HS_TX_EN,
			_dsi_cmd_mode_parse_state(DSI_REG[i]->DSI_STATE_DBG6.CMTRL_STATE));

		DDPDUMP("DSI%d IRQ,RD_RDY:%d, CMD_DONE:%d, SLEEPOUT_DONE:%d, TE_RDY:%d, VM_CMD_DONE:%d, VM_DONE:%d\n", i,
			DSI_REG[i]->DSI_INTSTA.RD_RDY,
			DSI_REG[i]->DSI_INTSTA.CMD_DONE,
			DSI_REG[i]->DSI_INTSTA.SLEEPOUT_DONE,
			DSI_REG[i]->DSI_INTSTA.TE_RDY,
			DSI_REG[i]->DSI_INTSTA.VM_CMD_DONE,
			DSI_REG[i]->DSI_INTSTA.VM_DONE);

		DDPDUMP("DSI%d Lane Num:%d, Ext_TE_EN:%d, Ext_TE_Edge:%d, HSTX_CKLP_EN:%d\n", i,
			DSI_REG[i]->DSI_TXRX_CTRL.LANE_NUM,
			DSI_REG[i]->DSI_TXRX_CTRL.EXT_TE_EN,
			DSI_REG[i]->DSI_TXRX_CTRL.EXT_TE_EDGE,
			DSI_REG[i]->DSI_TXRX_CTRL.HSTX_CKLP_EN);
	}
}
int ddp_dsi_dump(DISP_MODULE_ENUM module, int level)
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


DSI_STATUS DSI_Start(DISP_MODULE_ENUM module, cmdqRecHandle cmdq)
{
	int i = 0;

	DSI_OUTREGBIT(cmdq, DSI_START_REG,DSI_REG[0]->DSI_START,DSI_START,0);
	DSI_OUTREGBIT(cmdq, DSI_START_REG,DSI_REG[0]->DSI_START,DSI_START,1);
	return DSI_STATUS_OK;
}


static DSI_STATUS DSI_Reset(DISP_MODULE_ENUM module, cmdqRecHandle cmdq)
{
	int i = 0;

	for(i = DSI_MODULE_BEGIN(module);i <= DSI_MODULE_END(module);i++)
	{
		DSI_OUTREGBIT(cmdq, DSI_COM_CTRL_REG,DSI_REG[i]->DSI_COM_CTRL,DSI_RESET,1);
		DSI_OUTREGBIT(cmdq, DSI_COM_CTRL_REG,DSI_REG[i]->DSI_COM_CTRL,DSI_RESET,0);
	}

    	return DSI_STATUS_OK;
}

static int _dsi_is_video_mode(DISP_MODULE_ENUM module)
{
	int i = 0;
	DSI_MODE_CTRL_REG tmpreg;

	for(i = DSI_MODULE_BEGIN(module);i <= DSI_MODULE_END(module);i++)
	{
		if(DSI_REG[i]->DSI_MODE_CTRL.MODE == CMD_MODE)
			return 0;
		else
			return 1;
	}
}

void DSI_WaitForNotBusy(DISP_MODULE_ENUM module, cmdqRecHandle cmdq)
{
	int i = 0;
	unsigned int count = 0;
	unsigned int tmp = 0;
	//static const long WAIT_TIMEOUT = 2 * HZ;	// 2 sec
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

int ddp_dsi_is_busy(DISP_MODULE_ENUM module)
{
	int i;
	unsigned int is_busy = 0;
	for(i = DSI_MODULE_BEGIN(module); i <= DSI_MODULE_END(module);i++)
	{
		is_busy |= DSI_REG[i]->DSI_INTSTA.BUSY;
	}
	return is_busy;
}

void DSI_set_cmdq(DISP_MODULE_ENUM module, cmdqRecHandle cmdq, unsigned int *pdata, unsigned int queue_size, unsigned char force_update)
{
	int j = 0;
	int i = 0;

	//DDPMSG("DSI_set_cmdq, module=%s, cmdq=0x%08x\n", ddp_get_module_name(module), cmdq);

	for(i = DSI_MODULE_BEGIN(module);i <= DSI_MODULE_END(module);i++)
	{
		if (0 != DSI_REG[i]->DSI_MODE_CTRL.MODE)
		{
			;/* video mode, do nothing*/
		}
		else
		{
			ASSERT(queue_size<=32);
			DSI_WaitForNotBusy(module, cmdq);
#ifdef ENABLE_DSI_ERROR_REPORT
			if ((pdata[0] & 1))
			{
				memcpy(_dsi_cmd_queue, pdata, queue_size*4);
				_dsi_cmd_queue[queue_size++] = 0x4;
				pdata = (unsigned int*) _dsi_cmd_queue;
			}
			else
			{
				pdata[0] |= 4;
			}
#endif

			for(j=0; j<queue_size;j++)
			{
				DSI_OUTREG32(cmdq, &DSI_CMDQ_REG[i]->data[j], AS_UINT32((pdata+j)));
			}

			DSI_OUTREG32(cmdq, &DSI_REG[i]->DSI_CMDQ_SIZE, queue_size);

			if(force_update)
		   {
				DSI_Start(module, cmdq);
		        DSI_WaitForNotBusy(module, cmdq);
		   }
		}
	}
}

int ddp_dsi_trigger(DISP_MODULE_ENUM module, cmdqRecHandle cmdq, DDP_MODE mode)
{
	int i = 0;
	unsigned int data_array[16];

	if(mode == DDP_CMD_MODE)
	{
		data_array[0]= 0x002c3909;
		DSI_set_cmdq(module, cmdq, data_array, 1, 0);
	}

	DSI_Start(module, cmdq);

	return 0;
}

DISP_MODULE_ENUM ddp_dsi_get_module()
{
	if(DSI_REG[0]->DSI_COM_CTRL.DSI_DUAL_EN)
		return DISP_MODULE_DSIDUAL;
	else
		return DISP_MODULE_DSI0;
}


DDP_MODE ddp_dsi_get_mode(DISP_MODULE_ENUM module)
{
	if(_dsi_is_video_mode(module))
		return DDP_VIDEO_MODE;
	else
		return DDP_CMD_MODE;
}

#if 0
int ddp_dsi_wait_te(DISP_MODULE_ENUM module, cmdqRecHandle cmdq_handle, DDP_MODE mode)
{
		// need waiting te
		if(module == DISP_MODULE_DSI0)
		{
			if(dsi0_te_enable == 0)
				return 0;

			ret = cmdqRecClearEventToken(cmdq_handle, CMDQ_EVENT_MDP_DSI0_TE_SOF);
			ret = cmdqRecWait(cmdq_handle, CMDQ_EVENT_MDP_DSI0_TE_SOF);
		}
		else if(module == DISP_MODULE_DSI1)
		{
			if(dsi1_te_enable == 0)
				return 0;

			ret = cmdqRecClearEventToken(cmdq_handle, CMDQ_EVENT_MDP_DSI1_TE_SOF);
			ret = cmdqRecWait(cmdq_handle, CMDQ_EVENT_MDP_DSI1_TE_SOF);
		}
		else if(module == DISP_MODULE_DSIDUAL)
		{
			if(dsidual_te_enable == 0)
				return 0;

			// TODO: dsi 8 lane do not use te????
			//ret = cmdqRecWait(cmdq_trigger_handle, CMDQ_EVENT_MDP_DSI0_TE_SOF);
			if(_dsi_context[0].swap_port || _dsi_context[1].swap_port)
			{
				ret = cmdqRecClearEventToken(cmdq_handle, CMDQ_EVENT_MDP_DSI1_TE_SOF);
				ret = cmdqRecWait(cmdq_handle, CMDQ_EVENT_MDP_DSI1_TE_SOF);
			}else{
				ret = cmdqRecClearEventToken(cmdq_handle, CMDQ_EVENT_MDP_DSI0_TE_SOF);
				ret = cmdqRecWait(cmdq_handle, CMDQ_EVENT_MDP_DSI0_TE_SOF);
			}
		}
		else
		{
			DISPERR("wrong module: %s\n", ddp_get_module_name(module));
			return -1;
		}
	}
}

#endif


int ddp_dsi_reg_init(DISP_MODULE_ENUM module, cmdqRecHandle cmdq)
{
	DSI_STATUS ret = DSI_STATUS_OK;
	int i = 0;
	DISPFUNC();

	DSI_REG[0] = DISPSYS_DSI0_BASE;
	DSI_PHY_REG[0] = DISPSYS_BASE_MIPITX0;
	DSI_CMDQ_REG[0] = DISPSYS_DSI0_BASE+0x200;
	DSI_REG[1] = DISPSYS_DSI1_BASE;
	DSI_PHY_REG[1] = DDP_REG_BASE_MIPITX1;
	DSI_CMDQ_REG[1] = DISPSYS_DSI1_BASE+0x200;
	DSI_VM_CMD_REG[0] = (PDSI_VM_CMDQ_REGS)(DISPSYS_DSI0_BASE + 0x134);
	DSI_VM_CMD_REG[1] = (PDSI_VM_CMDQ_REGS)(DDP_REG_BASE_DSI1 + 0x134);

	DDPMSG("DSI0 Base Address:0x%x, PHY:0x%x\n", (unsigned long)DSI_REG[0], (unsigned long)DSI_PHY_REG[0]);
	DDPMSG("DSI1 Base Address:0x%x, PHY:0x%x\n", (unsigned long)DSI_REG[1], (unsigned long)DSI_PHY_REG[1]);

	return DSI_STATUS_OK;
}

void DSI_manual_enter_HS(cmdqRecHandle cmdq)
{
	DSI_OUTREGBIT(cmdq, DSI_PHY_LCCON_REG, DSI_REG[0]->DSI_PHY_LCCON, LC_HS_TX_EN, 1);
}

void DSI_sw_clk_trail_cmdq(int module_idx,cmdqRecHandle cmdq)
{
	///init DSI clk lane software control
	DISP_REG_SET(cmdq, &DSI_REG[module_idx]->DSI_PHY_LCCON, 0x00000001);
	DISP_REG_SET(cmdq, &DSI_PHY_REG[module_idx]->MIPITX_DSI_SW_CTRL_CON0, 0x00000031);
	DISP_REG_SET(cmdq, &DSI_PHY_REG[module_idx]->MIPITX_DSI_SW_CTRL_CON1, 0x0F0F0F0F);
	DISP_REG_SET(cmdq, &DSI_PHY_REG[module_idx]->MIPITX_DSI_SW_CTRL_EN, 0x00000001);

	///controll DSI clk trail duration
	//force clk lane keep low
	DISP_REG_SET(cmdq, &DSI_PHY_REG[module_idx]->MIPITX_DSI_SW_CTRL_CON0, 0x00000071);
	//pull clk lane to LP state
	DISP_REG_SET(cmdq, &DSI_PHY_REG[module_idx]->MIPITX_DSI_SW_CTRL_CON0, 0x0000000F);

	///give back DSI clk lane control
	DISP_REG_SET(cmdq, &DSI_REG[module_idx]->DSI_PHY_LCCON, 0x00000000);
	DISP_REG_CMDQ_POLLING(cmdq, &DSI_REG[module_idx]->DSI_STATE_DBG0, 0x00010000, 0x00010000);
	DISP_REG_SET(cmdq, &DSI_PHY_REG[module_idx]->MIPITX_DSI_SW_CTRL_EN, 0x00000000);

}

CLK_MODE ddp_dsi_get_clk_lane_mode(void)
{
	int dsi_state_dbg0;

	dsi_state_dbg0 = DISP_REG_GET(&DSI_REG[0]->DSI_STATE_DBG0);
	DDPMSG("dsi_state_dbg0: 0x%x\n", dsi_state_dbg0);
	if (dsi_state_dbg0 & 0x00010000)
		return LP_MODE;
	else
		return HS_MODE;
}

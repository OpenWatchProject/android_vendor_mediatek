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

#include "drStd.h"
#include "DrApi/DrApi.h"
#include "drError.h"
#include "drTuiCommon.h"
#include "mt_typedefs.h"

#include "ddp_log.h"
#include "cmdq_sec_record.h"
#include "ddp_hal.h"
#include "ddp_rdma.h"
#include "DpDataType.h"
#include "ddp_reg.h"
#include "lcm_drv.h"
#include "ddp_dsi.h"
#include "ddp_path.h"

#include "drTuiHal.h"

#include "dapc.h"
#define TAG "[display]"
#include "log.h"

int disp_reg_init();

struct _reg_backup {
	void * va;
	unsigned int val;
};

static struct _reg_backup regs_backup[30];
static int sw_clk_trail_on;

int disp_tui_dump_backup_regs()
{
	return 0;
}

int disp_tui_init_backup_regs()
{
	int i;
	for(i=0; i<ARRAY_SIZE(regs_backup); i++) {
		regs_backup[i].va = NULL;
		regs_backup[i].val = 0;
	}
	return 0;
}

int disp_tui_reg_backup(void* addr)
{
	int i, backup = -1;
	for(i=0; i<ARRAY_SIZE(regs_backup); i++) {
		if((regs_backup[i].va == 0) && (backup == -1)){
			backup = i;
		}
		if(regs_backup[i].va == addr) {
			DDPAEE("reg backup again! reg=0x%x\n", addr);
			return -1;
		}
	}

	if(backup == -1) {
		DDPAEE("reg backup mem is full ! \n");
		disp_tui_dump_backup_regs();
	}
	regs_backup[backup].va = addr;
	regs_backup[backup].val = DISP_REG_GET(addr);
	return 0;
}

int disp_tui_reg_restore(cmdqRecHandle cmdq_handle)
{
	int i;
	DDPDBG("begin to restore disp regs===========>\n");
	for(i=0; i<ARRAY_SIZE(regs_backup); i++) {
		if(regs_backup[i].va){
			DDPDBG("0x%x = 0x%x\n", disp_addr_convert((unsigned long)regs_backup[i].va),
									regs_backup[i].val);
			DISP_REG_SET(cmdq_handle, regs_backup[i].va, regs_backup[i].val);
			regs_backup[i].va = NULL;
			regs_backup[i].val = 0;
		}
	}
	DDPDBG("restore disp regs done ===========<\n");

	return 0;
}

static void* larb0_base = NULL;
int disp_m4u_reg_init()
{
	if(larb0_base) {
		DDPMSG("disp m4u has been inited\n");
		return 0;
	}

	drApiResult_t ret;

	ret = drApiMapPhysicalBuffer(LARB0_BASE, 0x1000,
			MAP_HARDWARE, &larb0_base);

	if(ret != DRAPI_OK)
		DDPAEE("%s fail: pa=0x%x, size=0x%x, flag=0x%x, ret=%d(0x%x)\n",
			__func__, LARB0_BASE, 0x1000, MAP_HARDWARE, ret, ret);

    DDPMSG("M4U reg_map, map_addr=%p,reg_pa=0x%x\n", larb0_base, LARB0_BASE);

	return 0;
}

int disp_m4u_config_port(cmdqRecHandle cmdq_handle, int port, int mmu_en, int sec)
{
	unsigned int regval;
	DISP_REG_MASK_EXT(cmdq_handle, larb0_base+SMI_LARB_MMU_EN, LARB0_BASE+SMI_LARB_MMU_EN,
			F_SMI_MMU_EN(port, !!mmu_en), F_SMI_MMU_EN(port, 1));

	DISP_REG_MASK_EXT(cmdq_handle, larb0_base+SMI_LARB_SEC_EN, LARB0_BASE+SMI_LARB_SEC_EN,
			F_SMI_SEC_EN(port, !!sec), F_SMI_SEC_EN(port, 1));

	DDPMSG("%s: port=%d, mmu_en=%d, sec=%d  regval=0x%x,0x%X\n", __func__, port, mmu_en, sec,
		DISP_REG_GET(larb0_base+SMI_LARB_MMU_EN), DISP_REG_GET(larb0_base+SMI_LARB_SEC_EN));

	return 0;
}


/*==========================================================*/



/*====================================================================*/
#if 0

static uint64_t disp_fb_pa = 0x99c00000;
static int lcm_width = 1080;
static int lcm_height = 1920;
static int fb_cnt = 1;
static DP_COLOR_ENUM fb_format = eRGBA8888;

static int fb_Bpp = 0;
static int fb_total_size = 0;
static int fb_pitch = 0;
static void * disp_fb_va = NULL;


void* disp_tui_map_fb(uint64_t fb_pa, size_t size)
{
	void* fb_va;
	drApiResult_t ret;
	uint32_t attr = MAP_READABLE | MAP_WRITABLE | MAP_UNCACHED;

	ret = drApiMapPhysicalBuffer(fb_pa, size,
			attr, &fb_va);

	if(ret != DRAPI_OK) {
		DDPAEE("map fb fail: pa=0x%x, size=0x%x, flag=0x%x, ret=%d(0x%x)\n",
			(unsigned int)fb_pa, size, attr, ret, ret);
		return NULL;
	}
	DDPMSG("map fb done: pa=0x%x, size=0x%x, flag=0x%x, va=0x%x, ret=%d(0x%x)\n",
		(unsigned int)fb_pa, size, attr, fb_va, ret, ret);

	return fb_va;
}


static int disp_test_draw_fb()
{
	disp_tui_draw_buffer(disp_fb_va,
			lcm_width, lcm_height, fb_pitch, fb_Bpp);
	return 0;
}


static int disp_tui_init_fb_contex()
{
	fb_Bpp = DP_COLOR_BITS_PER_PIXEL(fb_format)/8;
	fb_pitch = lcm_width * fb_Bpp;
	fb_total_size = fb_pitch * lcm_height * fb_cnt;
	disp_fb_va = disp_tui_map_fb(disp_fb_pa, fb_total_size);

	disp_test_draw_fb();
	return 0;
}
#endif

int disp_tui_draw_buffer(uint8_t *va, int w, int h, int pitch, int Bpp)
{
	int x, y;
	uint8_t *buf = va;

	for(y=0; y<h; y++) {
		for(x=0; x<w; x++) {
			if(Bpp>0)
				(buf + x*Bpp + y*pitch)[0] = (unsigned char)y;
			if(Bpp>1)
				(buf + x*Bpp + y*pitch)[1] = (unsigned char)~x;
			if(Bpp>2)
				(buf + x*Bpp + y*pitch)[2] = (unsigned char)x;
		}
	}
	return 0;
}

/*====================================================================*/
static int disp_tui_cmdq_reset(cmdqRecHandle cmdq_handle)
{
	if(cmdq_handle)
		return cmdqRecReset(cmdq_handle);
	else
		return 0;
}
static int disp_tui_cmdq_flush(cmdqRecHandle cmdq_handle)
{
	int ret;
	if(!cmdq_handle)
		return 0;

#if 0
	DDPDUMP("===== dump cmd before flush ======>\n");
	cmdqRecDumpCommand(cmdq_handle);
	DDPDUMP("===== dump cmd done ======>\n");
#endif

	ret = cmdqRecFlush(cmdq_handle);
	if(ret)
		DDPERR("error to flush cmdq handle\n");
	disp_tui_cmdq_reset(cmdq_handle);
	return 0;
}


/*====================================================================*/

static int gMutex_id = 0;
static int gScenario = DDP_SCENARIO_PRIMARY_RDMA0_COLOR0_DISP;
static int gRdma_module = DISP_MODULE_RDMA0;

/* inited in disp_tui_init() */
static int gDsi_module = DISP_MODULE_DSI0;
static DDP_MODE gDdp_mode = DDP_CMD_MODE;

static cmdqRecHandle gCmdq_handle;
#define DISP_TUI_ENABLE_CMDQ

int disp_tui_analyse()
{
	int i;
    int * modules = ddp_get_scenario_list(gScenario);
    int module_num = ddp_get_module_num(gScenario);
    DISP_LOG_V("check status on scenario %s\n",
                ddp_get_scenario_name(gScenario));

    ddp_check_path(gScenario);
    ddp_check_mutex(gMutex_id, gScenario, gDdp_mode);

    for( i=0; i< module_num;i++)
    {
		int module_name;
        module_name = modules[i];

       ddp_dump_analysis(module_name);
       ddp_dump_reg(module_name);
    }
    ddp_dump_analysis(DISP_MODULE_CONFIG);
    ddp_dump_reg(DISP_MODULE_CONFIG);

    ddp_dump_analysis(DISP_MODULE_MUTEX);
    ddp_dump_reg(DISP_MODULE_MUTEX);

	return 0;
}



static int update_intferface_module(DISP_MODULE_ENUM dsi_module)
{
	/*update interface module, it may be: dsi0/dsi1/dsi_dual*/
	ddp_set_dst_module(DDP_SCENARIO_PRIMARY_DISP, dsi_module);
	ddp_set_dst_module(DDP_SCENARIO_PRIMARY_RDMA0_COLOR0_DISP, dsi_module);
	ddp_set_dst_module(DDP_SCENARIO_PRIMARY_RDMA0_DISP, dsi_module);
	ddp_set_dst_module(DDP_SCENARIO_PRIMARY_BYPASS_RDMA, dsi_module);

	ddp_set_dst_module(DDP_SCENARIO_PRIMARY_ALL, dsi_module);

	return 0;
}


int _disp_tui_trigger(cmdqRecHandle cmdq_handle, int mutex_id, DDP_MODE mode, DISP_MODULE_ENUM dsi_module)
{
	ddp_mutex_enable_l(mutex_id, cmdq_handle);
	ddp_dsi_trigger(dsi_module, cmdq_handle, mode);
	return 0;
}

int disp_tui_trigger(cmdqRecHandle cmdq_handle)
{
	if(gDdp_mode == DDP_VIDEO_MODE && ddp_dsi_is_busy(gDsi_module))
		return 0;
	if(cmdq_handle) {
		/* see notes before disp_tui_wait_frame_done() */
		cmdqRecClearEventToken(cmdq_handle, rdma_get_EOF_cmdq_event(gRdma_module));
	}
	_disp_tui_trigger(cmdq_handle, gMutex_id, gDdp_mode, gDsi_module);

    return 0;
}


/* cmdq event usage:
 * for cmd mode, we use RDMA_EOF event to indicate frame done
 * basically, RDMA_EOF should be set when RDMA is not running, so:
 * 1. set EOF when enter tui (for init)
 * 2. wait frame done uses cmdqRecWaitNoClear() (and don't clear it before wait)
 * 3. clear RDMA_EOF before trigger (it will be set automatically when rdma done)
 *
 * for video mode, it's simple because RDMA_EOF happens every frame.
 * we have to clear it before wait for next frame done (clear_before_wait)
 */
int disp_tui_wait_frame_done(cmdqRecHandle cmdq_handle)
{
	if(cmdq_handle) {
		int clear_before_wait = (gDdp_mode == DDP_CMD_MODE) ? 0 : 1;
		rdma_wait_frame_done_by_cmdq(gRdma_module, cmdq_handle, clear_before_wait, 0);
	} else {
		if(gDdp_mode == DDP_CMD_MODE) {
			DSI_WaitForNotBusy(gDsi_module, NULL);
		} else {
			#if DISP_RDMA_INTERRUPT_ENABLE
			rdma_wait_frame_done_by_interrupt(gRdma_module);
			#else
			rdma_wait_frame_done_by_polling(gRdma_module);
			#endif
		}
	}

	return 0;
}

DP_COLOR_ENUM disp_tui_get_color_fmt(tuiFbPixelType_t type)
{
	switch(type) {

	case TUI_HAL_FB_TYPE_RGB:
		return eRGB888;
	case TUI_HAL_FB_TYPE_BGR:
		return eBGR888;
	case TUI_HAL_FB_TYPE_RGBA:
		return eRGBA8888;
    case TUI_HAL_FB_TYPE_BGRA:
		return eBGRA8888;
	default:
		DDPAEE("invalid fb type 0x%x\n", type);
		return 0;
	}
	return 0;
}

int disp_tui_pan_display(tuiFbInfo_t *pFbInfo)
{
	cmdqRecHandle cmdq_handle = gCmdq_handle;

	disp_tui_cmdq_reset(cmdq_handle);
	disp_tui_wait_frame_done(cmdq_handle);

	rdma_config(gRdma_module,
		RDMA_MODE_MEMORY,
		pFbInfo->fbPhysAddr,
		disp_tui_get_color_fmt(pFbInfo->type),
		pFbInfo->lineLength,
		pFbInfo->xRes,
		pFbInfo->yRes,
		0,	0, cmdq_handle);

	/* config rdma port: PA & SEC */
	disp_m4u_config_port(cmdq_handle, M4U_PORT_DISP_RDMA0, 0, 1);

	if (sw_clk_trail_on)
		DSI_manual_enter_HS(cmdq_handle);

	disp_tui_trigger(cmdq_handle);

	if (sw_clk_trail_on) {
		rdma_wait_frame_done_by_cmdq(gRdma_module, cmdq_handle, 0, 0);
		DSI_WaitForNotBusy(DISP_MODULE_DSI0,cmdq_handle);
		DSI_sw_clk_trail_cmdq(0, cmdq_handle);
	}

	disp_tui_cmdq_flush(cmdq_handle);

	return 0;
}

unsigned int rdma_irq_backup;


int disp_tui_init()
{
	static int is_inited = 0;
	int ret;

	if(is_inited) {
		DDPDBG("%s has inited\n", __FUNCTION__);
		return 0;
	}
	is_inited = 1;

    disp_reg_init();
	disp_m4u_reg_init();
	start_devapc();
	cmdqRecInitialize();

	gDsi_module = ddp_dsi_get_module();

	update_intferface_module(gDsi_module);

	gDdp_mode = ddp_dsi_get_mode(gDsi_module);

	DDPDBG("%s, dsi_module=%s, mode=%s\n", ddp_get_module_name(gDsi_module),
		gDdp_mode==DDP_CMD_MODE ? "cmd" : "vdo");
//	DISP_REG_CMDQ_POLLING(cmdq, &DSI_REG[module_idx]->DSI_STATE_DBG0, 0x00010000, 0x00010000);

#ifdef DISP_TUI_ENABLE_CMDQ
	ret = cmdqRecCreate(CMDQ_SCENARIO_PRIMARY_DISP, &gCmdq_handle);
	if(ret)
		DDPAEE("error to create cmdq handle ! ret=%d\n", ret);
#endif

	//disp_tui_init_fb_contex();

	return ret;
}

#define DAPC_MMSYS_CONFIG_INDEX 77
#define DAPC_MMSYS_RDMA0_INDEX  91

static int protect_display_engines()
{
	/*protect display engines */
	/* permission control: 0-no protect; 1-sec RW; 2-sec RW & non-sec RD; 3-no access; */

	set_module_apc(DAPC_MMSYS_CONFIG_INDEX, 0, 2);
	set_module_apc(DAPC_MMSYS_RDMA0_INDEX, 0, 2);
	return 0;
}

static int unprotect_display_engines()
{
	/* unprotect display engines */
	set_module_apc(DAPC_MMSYS_CONFIG_INDEX, 0, 0);
	set_module_apc(DAPC_MMSYS_RDMA0_INDEX, 0, 0);
	return 0;
}


int disp_tui_enter()
{
	DDPMSG("%s\n", __func__);

	int ret;
	/*disable rdma irq before protect it */
	rdma_irq_backup = rdma_disable_irq_backup(gRdma_module, NULL);

#if DISP_RDMA_INTERRUPT_ENABLE
	rdma_irq_attach(gRdma_module);
#endif
	protect_display_engines();

	/* check path */
	ret = ddp_check_path_strict(gScenario);
	if(ret) {
		DDPERR("error to enter tui because path is incorrect !!\n");
		disp_tui_analyse();
		goto err1;
	}
	/* check path can't covor rdma mode */
	ret = rdma_is_mem_mode(gRdma_module);
	if(!ret) {
		DDPERR("error to enter tui because %s is not mem mode\n",
			ddp_get_module_name(gRdma_module));
		goto err1;
	}
	/* protect port config registers */
	rdma_reg_backup(gRdma_module);

	if(gDdp_mode == DDP_CMD_MODE && gCmdq_handle) {
		/* see notes before disp_tui_wait_frame_done() */
		DSI_WaitForNotBusy(gDsi_module, NULL);
		cmdqTzCoreSetEvent(rdma_get_EOF_cmdq_event(gRdma_module));
	}

	disp_tui_analyse();
	if (gDdp_mode == DDP_CMD_MODE && ddp_dsi_get_clk_lane_mode() == LP_MODE)
		sw_clk_trail_on = 1;
	else
		sw_clk_trail_on = 0;

	DDPMSG("%s done sw_clk_trail_on:%d\n", __func__, sw_clk_trail_on);
	return 0;


err1:

	unprotect_display_engines();
#if DISP_RDMA_INTERRUPT_ENABLE
	rdma_irq_dettach(gRdma_module);
#endif
	rdma_irq_restore(gRdma_module, NULL, rdma_irq_backup);

	return -1;
}

int disp_tui_leave()
{
	cmdqRecHandle cmdq_handle = gCmdq_handle;

	DDPMSG("%s start\n", __func__);

	disp_tui_cmdq_reset(cmdq_handle);
	/* clear fb and trigger black screen Robin*/
	disp_tui_wait_frame_done(cmdq_handle);

	/*restore display registers*/
	disp_tui_reg_restore(cmdq_handle);
	/* config rdma port */
	disp_m4u_config_port(cmdq_handle, M4U_PORT_DISP_RDMA0, 1, 0);

	DDPDBG("rdma_eof is %d\n", cmdqTzCoreGetEvent(rdma_get_EOF_cmdq_event(gRdma_module)));

	disp_tui_cmdq_flush(cmdq_handle);

	DDPMSG("%s wait cmdq done\n", __func__);

	unprotect_display_engines();

#if DISP_RDMA_INTERRUPT_ENABLE
	rdma_irq_dettach(gRdma_module);
#endif
	/* restore rdma irq */
	rdma_irq_restore(gRdma_module, NULL, rdma_irq_backup);

	DDPMSG("%s done\n", __func__);
	return 0;
}

void disp_tui_delay(unsigned int cnt)
{
	volatile unsigned int i = 0, k;
	DDPDBG("begin delay %u\n", cnt);
	for(i=0; i<cnt; i++)
		k = i;
	DDPDBG("finish delay %u\n", cnt);
}

void test_display(void)
{
    DDPMSG("test_display+\n");

	int i;

	disp_tui_init();
	DDPMSG("tui reg init done\n");

	disp_tui_enter();

	disp_tui_analyse();

	disp_tui_leave();

    DDPMSG("test_display-\n");
}


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

#define TAG "[DISP_TUI]"

#include "drStd.h"
#include "DrApi/DrApi.h"
#include "drError.h"
#include "drTuiCommon.h"
#include "mt_typedefs.h"

#include "cmdq_sec_record.h"
#include "ddp_hal.h"
#include "ddp_rdma.h"
#include "DpDataType.h"

#include "ddp_reg.h"
#include "lcm_drv.h"
#include "ddp_dsi.h"
#include "ddp_path.h"
#include "ddp_ovl.h"
#include "ddp_info.h"
#include "ddp_color_format.h"

#include "dapc.h"
#include "log.h"
#include "ddp_drv.h"

#include "display_tui.h"
#include "tlDapcIndex.h"
#include "tui_m4u.h"
struct _reg_backup {
	void * va;
	unsigned int val;
};

static struct _reg_backup regs_backup[30];
static enum DISP_TUI_VERSION gDisp_version;
static unsigned long last_addr;

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
	DDPMSG("begin to restore disp regs===========>\n");
	for(i=0; i<ARRAY_SIZE(regs_backup); i++) {
		if(regs_backup[i].va){
			DDPMSG("0x%x = 0x%x\n", disp_addr_convert((unsigned long)regs_backup[i].va),
									regs_backup[i].val);
			DISP_REG_SET(cmdq_handle, regs_backup[i].va, regs_backup[i].val);
			regs_backup[i].va = NULL;
			regs_backup[i].val = 0;
		}
	}
	DDPMSG("restore disp regs done ===========<\n");

	return 0;
}

/*=========================SMI/M4U ======================*/
#define LARB0_BASE 0x14017000
#define LARB1_BASE 0x15021000

#define SMI_LARB_SEC_CON_OFFSET(port) (0xf80 + (((port))*4))

enum _M4U_LARB0_PORT
{
	M4U_LARB0_PORT_DISP_OVL0,
	M4U_LARB0_PORT_DISP_2L_OVL0_LARB0,
	M4U_LARB0_PORT_DISP_2L_OVL1,
	M4U_LARB0_PORT_DISP_RDMA0,
	M4U_LARB0_PORT_DISP_RDMA1,
	M4U_LARB0_PORT_DISP_WDMA0,
	M4U_LARB0_PORT_MDP_RDMA0,
	M4U_LARB0_PORT_MDP_WROT0,
	M4U_LARB0_PORT_MDP_WDMA0,
	M4U_LARB0_PORT_DISP_FAKEi0,
};

/*=======================================================*/



static void* larb0_base = NULL;
static void* larb1_base = NULL;

int disp_m4u_init()
{
	drApiResult_t ret;

	tui_m4u_Init();
	if(larb0_base) {
		DDPMSG("disp m4u has been inited\n");
		return 0;
	}

	ret = drApiMapPhysicalBuffer(LARB0_BASE, 0x1000,
			MAP_HARDWARE, &larb0_base);

	if(ret != DRAPI_OK)
		DDPAEE("%s fail: pa=0x%x, size=0x%x, flag=0x%x, ret=%d(0x%x)\n",
			__func__, LARB0_BASE, 0x1000, MAP_HARDWARE, ret, ret);

    DDPMSG("M4U reg_map, map_addr=%p,reg_pa=0x%x\n", larb0_base, LARB0_BASE);
#if 0
	if(larb1_base) {
		DDPMSG("disp m4u has been inited\n");
		return 0;
	}

	ret = drApiMapPhysicalBuffer(LARB1_BASE, 0x1000,
			MAP_HARDWARE, &larb1_base);

	if(ret != DRAPI_OK)
		DDPAEE("%s fail: pa=0x%x, size=0x%x, flag=0x%x, ret=%d(0x%x)\n",
			__func__, LARB1_BASE, 0x1000, MAP_HARDWARE, ret, ret);

    DDPMSG("M4U reg_map, map_addr=%p,reg_pa=0x%x\n", larb1_base, LARB1_BASE);
#endif
	return 0;
}

int disp_m4u_config_port(cmdqRecHandle cmdq_handle, int port, int mmu_en, int sec)
{
	unsigned int regval, offset;

	if (gDisp_version == SINGLE_WINDOWS_TUI) {
		offset = SMI_LARB_SEC_CON_OFFSET(port);
		DISP_REG_MASK_EXT(cmdq_handle, larb0_base + offset,
			LARB0_BASE + offset, mmu_en, 0x1);
		DISP_REG_MASK_EXT(cmdq_handle, larb0_base + offset,
			LARB0_BASE + offset, sec << 1, 0x2);

		DDPMSG("%s: port=%d, mmu_en=%d, sec=%d  regval=0x%x\n",
			__func__, port, mmu_en, sec, DISP_REG_GET(larb0_base + offset));
	} else if (gDisp_version == MULTI_WINDOWS_TUI) {
		offset = SMI_LARB_SEC_CON_OFFSET(port);
		DISP_REG_MASK_EXT(cmdq_handle, larb0_base + offset,
			LARB1_BASE + offset, mmu_en, 0x1);
		DISP_REG_MASK_EXT(cmdq_handle, larb0_base + offset,
			LARB1_BASE + offset, sec << 1, 0x2);

		DDPMSG("%s: port=%d, mmu_en=%d, sec=%d  regval=0x%x\n",
			__func__, port, mmu_en, sec, DISP_REG_GET(larb0_base + offset));
	}
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
#endif

/*====================================================================*/
static int disp_tui_cmdq_reset(cmdqRecHandle cmdq_handle)
{
	/*DDPMSG("%s, cmdq_handle =%d\n", (__func__), cmdq_handle);*/
	if(cmdq_handle)
		return cmdqRecReset(cmdq_handle);
	else
		return 0;
}
static int disp_tui_cmdq_flush(cmdqRecHandle cmdq_handle)
{
	int ret;
	/*DDPMSG("%s, start!\n", (__func__));*/

	if(!cmdq_handle)
		return 0;

#if 0
	DDPMSG("===== dump cmd before flush ======>\n");
	cmdqRecDumpCommand(cmdq_handle);
	DDPMSG("===== dump cmd done ======>\n");
#endif

	ret = cmdqRecFlush(cmdq_handle);
	if(ret)
		DDPMSG("error to flush cmdq handle\n");
	disp_tui_cmdq_reset(cmdq_handle);
	/*DDPMSG("%s, done!\n", (__func__));*/
	return 0;
}


/*====================================================================*/

static int gMutex_id = 0;
static int gScenario = DDP_SCENARIO_PRIMARY_RDMA0_COLOR0_DISP;
static int gDisp_module = DISP_MODULE_RDMA0;

/* inited in disp_tui_init() */
static int gDsi_module = DISP_MODULE_DSI0;
static enum DDP_MODE gDdp_mode = DDP_CMD_MODE;

static cmdqRecHandle gCmdq_handle;
#define DISP_TUI_ENABLE_CMDQ

int disp_tui_analyse()
{
	int i;
    int * modules = ddp_get_scenario_list(gScenario);
    int module_num = ddp_get_module_num(gScenario);
	DDPDBG("%s start!\n", __func__);
    DISP_LOG_I("check status on scenario %s\n",
                ddp_get_scenario_name(gScenario));

    ddp_check_path(gScenario);
    ddp_check_mutex(gMutex_id, gScenario, gDdp_mode);
#if 0
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
#endif

	DDPDBG("%s done!\n", __func__);

	return 0;
}



static int update_intferface_module(enum DISP_MODULE_ENUM dsi_module)
{
	/*update interface module, it may be: dsi0/dsi1/dsi_dual*/
	ddp_set_dst_module(DDP_SCENARIO_PRIMARY_DISP, dsi_module);
	ddp_set_dst_module(DDP_SCENARIO_PRIMARY_RDMA0_COLOR0_DISP, dsi_module);
	ddp_set_dst_module(DDP_SCENARIO_PRIMARY_RDMA0_DISP, dsi_module);
	ddp_set_dst_module(DDP_SCENARIO_PRIMARY_ALL, dsi_module);

	return 0;
}


int _disp_tui_trigger(cmdqRecHandle cmdq_handle, int mutex_id, enum DDP_MODE mode,
	enum DISP_MODULE_ENUM dsi_module)
{
	/*DDPMSG("%s, start!\n", (__func__));*/

	/*ddp_mutex_enable_l(mutex_id, cmdq_handle);*/
	ddp_mutex_enable(mutex_id, gScenario, cmdq_handle);
	ddp_dsi_trigger(dsi_module, cmdq_handle);
	return 0;
}

int disp_tui_trigger(cmdqRecHandle cmdq_handle)
{
	/*DDPMSG("%s, start!\n", (__func__));*/

	if(gDdp_mode == DDP_VIDEO_MODE && ddp_dsi_is_busy(gDsi_module))
		return 0;

	_disp_tui_trigger(cmdq_handle, gMutex_id, gDdp_mode, gDsi_module);
	/*DDPMSG("%s, done!\n", (__func__));*/

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
	/*DDPMSG("%s, dxs cmdq_handle =%d\n", (__func__), cmdq_handle);*/

	if (cmdq_handle) {
		int clear_before_wait = (gDdp_mode == DDP_CMD_MODE) ? 0 : 1;
		rdma_wait_frame_done_by_cmdq(DISP_MODULE_RDMA0, cmdq_handle, clear_before_wait, 0);
	} else {
		if (gDdp_mode == DDP_CMD_MODE) {
			DSI_WaitForNotBusy(gDsi_module, NULL);
		} else {
			#if DISP_RDMA_INTERRUPT_ENABLE
			rdma_wait_frame_done_by_interrupt(DISP_MODULE_RDMA0);
			#else
			rdma_wait_frame_done_by_polling(DISP_MODULE_RDMA0);
			#endif
		}
	}

	return 0;
}

enum UNIFIED_COLOR_FMT disp_tui_get_color_fmt(tuiFbPixelType_t type)
{
	/*DDPMSG("%s, start0!\n", (__func__));*/

	switch (type) {

	case TUI_HAL_FB_TYPE_RGB:
		return UFMT_RGB888;
	case TUI_HAL_FB_TYPE_BGR:
		return UFMT_BGR888;
	case TUI_HAL_FB_TYPE_RGBA:
		return UFMT_RGBA8888;
    case TUI_HAL_FB_TYPE_BGRA:
		return UFMT_BGRA8888;
	default:
		DDPAEE("invalid fb type 0x%x\n", type);
		return 0;
	}
	return 0;
}

extern void dump_mva_graph(void);
extern void dump_pagetable_in_use(void);

static int _disp_tui_pan_disp_single(const tuiFbInfoV2_t *pFbInfo)
{
	cmdqRecHandle cmdq_handle = gCmdq_handle;
	struct rdma_bg_ctrl_t rdma_bg_ctrl;
	unsigned int bpp= 0;
	unsigned long mva = 0;

	DDPMSG("%s+\n", (__func__));
	disp_tui_cmdq_reset(cmdq_handle);

	disp_tui_wait_frame_done(cmdq_handle);

	if (gDdp_mode == DDP_CMD_MODE) /* need to clear rdma eof*/
		cmdqRecClearEventToken(cmdq_handle, rdma_get_EOF_cmdq_event(gDisp_module));

	rdma_bg_ctrl.left = 0;
	rdma_bg_ctrl.top  = 0;
	rdma_bg_ctrl.right= 0;
	rdma_bg_ctrl.bottom= 0;

	/*DDPMSG("%s, UFMT_GET_bpp start0!\n", (__func__));*/

	bpp = UFMT_GET_bpp( disp_tui_get_color_fmt(pFbInfo->type) ) / 8;
	/*DDPMSG("%s, dtui_m4u_alloc_mva: pa(%08x%08x)!\n",
	(__func__), VALUE_64(pFbInfo->fbPhysAddr));*/
	disp_m4u_config_port(NULL, M4U_LARB0_PORT_DISP_RDMA0, 1, 1);
	mva = tui_m4u_alloc_mva(M4U_PORT_DISP_RDMA0, pFbInfo->fbPhysAddr, pFbInfo->stride * pFbInfo->yRes);
	/*DDPMSG("%s, dtui_m4u_alloc_mva: mva(%08x%08x)!\n",
	(__func__), VALUE_64(mva));*/
	/*DDPMSG("%s, UFMT_GET_bpp done0!\n", (__func__));*/
	/*DDPMSG("%s, rdma_config()start0!\n", (__func__));*/

	rdma_config(gDisp_module,
		RDMA_MODE_MEMORY,
		mva,
		disp_tui_get_color_fmt(pFbInfo->type),
		pFbInfo->stride,
		pFbInfo->xRes,
		pFbInfo->yRes,
		0,	0, 0, &rdma_bg_ctrl, cmdq_handle, NULL ,bpp);
	/*DDPMSG("%s, rdma_config()done0!\n", (__func__));*/
	/* config rdma port: PA & SEC */
	/*DDPMSG("%s, disp_m4u_config_port()start0!\n", (__func__));*/
	dump_mva_graph();
	dump_pagetable_in_use();

	disp_tui_trigger(cmdq_handle);

	disp_tui_cmdq_flush(cmdq_handle);
	DDPMSG("%s-\n", (__func__));

	return 0;
}

static int is_pConfig_inited;
struct disp_ddp_path_config *pConfig = NULL;

static int _convert_disp_input_to_ovl(struct OVL_CONFIG_STRUCT *ovl_cfg, const tuiFbInfoV2_t *pFbInfo)
{
	int ret = 0;
	unsigned long mva = 0;

	DDPDBG("%s, start!\n", (__func__));
	ovl_cfg->layer = 0;
	ovl_cfg->isDirty = 1;
	ovl_cfg->buff_idx = -1;
	ovl_cfg->layer_en = 1;

	/* if layer is disable, we just needs config above params. */
	if (!ovl_cfg->layer_en)
		return 0;

	mva = tui_m4u_alloc_mva(M4U_PORT_DISP_2L_OVL0_LARB0, pFbInfo->fbPhysAddr, pFbInfo->stride * pFbInfo->yRes);
	ovl_cfg->fmt = disp_tui_get_color_fmt(pFbInfo->type);
	ovl_cfg->addr = mva;
	ovl_cfg->vaddr = NULL;
	ovl_cfg->src_x = 0;
	ovl_cfg->src_y = 0;
	ovl_cfg->src_w = pFbInfo->xRes;
	ovl_cfg->src_h = pFbInfo->yRes;
	ovl_cfg->src_pitch = pFbInfo->stride;
	ovl_cfg->dst_x = 0;
	ovl_cfg->dst_y = 0;

	/* dst W/H should <= src W/H */
	ovl_cfg->dst_w = pFbInfo->xRes;
	ovl_cfg->dst_h = pFbInfo->yRes;
	ovl_cfg->keyEn = 0;
	ovl_cfg->key = 0;
	ovl_cfg->aen = 0;
	ovl_cfg->sur_aen = 0;
	ovl_cfg->alpha = 0xFF;
	ovl_cfg->src_alpha = 0;
	ovl_cfg->dst_alpha = 0;
	ovl_cfg->yuv_range = 0;
	ovl_cfg->security = DISP_SECURE_BUFFER;
	ovl_cfg->source = OVL_LAYER_SOURCE_MEM;
	ovl_cfg->ext_sel_layer = -1;
	/*DDPDBG("%s, done!\n", (__func__));*/

	return ret;
}

void _cmdq_insert_wait_frame_done_token_mira(void *handle)
{
	if(gDdp_mode == DDP_VIDEO_MODE)
		cmdqRecWaitNoClear(handle, CMDQ_EVENT_MUTEX0_STREAM_EOF);
	else
		cmdqRecWaitNoClear(handle, CMDQ_SYNC_TOKEN_STREAM_EOF);
}

extern void ovl_dump_reg(enum DISP_MODULE_ENUM module);
extern void dump_mva_graph();
extern void dump_pagetable_in_use(void);

static int _disp_tui_pan_disp_multi(const tuiFbInfoV2_t *pFbInfo)
{
	cmdqRecHandle cmdq_handle = gCmdq_handle;
	struct OVL_CONFIG_STRUCT *ovl_cfg;

	/*DDPMSG("%s, start!\n", (__func__));*/
	disp_tui_cmdq_reset(cmdq_handle);
	if(cmdq_handle) {
		/* see notes before disp_tui_wait_frame_done() */
		_cmdq_insert_wait_frame_done_token_mira(cmdq_handle);

	}

	if (!is_pConfig_inited) {
		/*Allocate a block of memory from the heap for the pConfig struct.*/
		pConfig = drApiMalloc(sizeof(*pConfig),0);
		if (pConfig == NULL) {
			DDPERR("pConfig allocated FAILED!!\n");
			ASSERT(0);
			return -1;
		}
		is_pConfig_inited = 1;
	}

	ovl_cfg = &(pConfig->ovl_config[0]);
	_convert_disp_input_to_ovl(ovl_cfg, pFbInfo);

	pConfig->dst_w = X_RES;
	pConfig->dst_h = Y_RES;
	pConfig->dst_dirty = 1;
	pConfig->ovl_dirty = 1;
	pConfig->ovl_layer_scanned = 0;
	ovl_config_l(DISP_MODULE_OVL0_2L, pConfig, cmdq_handle);
	disp_m4u_config_port(cmdq_handle, M4U_LARB0_PORT_DISP_2L_OVL0_LARB0, 1, 1);
	dump_mva_graph();
	dump_pagetable_in_use();

	cmdqRecSetEventToken(cmdq_handle, CMDQ_SYNC_TOKEN_CONFIG_DIRTY);
	disp_tui_cmdq_flush(cmdq_handle);

	if (last_addr)
		tui_m4u_free(last_addr, pFbInfo->stride * pFbInfo->yRes);
	last_addr = ovl_cfg->addr;

	DDPMSG("%s, done!\n", (__func__));
}

int disp_tui_pan_display(const tuiFbInfoV2_t *pFbInfo)
{
	if (gDisp_version == SINGLE_WINDOWS_TUI)
		return _disp_tui_pan_disp_single(pFbInfo);
	else
		return _disp_tui_pan_disp_multi(pFbInfo);
}

unsigned int irq_backup;

int disp_tui_dsi_get_module(void)
{
#if 0
	int ufoe_mout = DISP_REG_GET(DISP_REG_CONFIG_DISP_UFOE_MOUT_EN);
	int dsi0_selin = DISP_REG_GET(DISP_REG_CONFIG_DISP_DSI0_SEL_IN);

	if (((ufoe_mout & 0x2) == 0x2) && (dsi0_selin == 0x1))
		return DISP_MODULE_DSIDUAL;
	else
#endif
		return DISP_MODULE_DSI0;
}

int disp_tui_init()
{
	static int is_inited = 0;
	int ret;

	if (is_inited) {
		DDPMSG("%s has inited\n", __FUNCTION__);
		return 0;
	}
	is_inited = 1;

	disp_reg_init();
	gDisp_version = disp_get_version();
	disp_m4u_init();
	//start_devapc();
	cmdqRecInitialize();

	gDsi_module = disp_tui_dsi_get_module();
	if (gDisp_version == SINGLE_WINDOWS_TUI)
		gDisp_module = DISP_MODULE_RDMA0;
	else if (gDisp_version == MULTI_WINDOWS_TUI)
		gDisp_module = DISP_MODULE_OVL0_2L;
	update_intferface_module(gDsi_module);

	gDdp_mode = ddp_dsi_get_mode(gDsi_module);

	DDPMSG("%s, dsi_module=%s, mode=%s\n", (__func__), ddp_get_module_name(gDsi_module),
		gDdp_mode==DDP_CMD_MODE ? "cmd" : "vdo");

	/*DISP_REG_CMDQ_POLLING(cmdq, &DSI_REG[module_idx]->DSI_STATE_DBG0, 0x00010000, 0x00010000);*/

#ifdef DISP_TUI_ENABLE_CMDQ
	ret = cmdqRecCreate(CMDQ_SCENARIO_PRIMARY_DISP, &gCmdq_handle);
	if(ret)
		DDPAEE("error to create cmdq handle ! ret=%d\n", ret);
#endif
	last_addr = 0;

	/*disp_tui_init_fb_contex();*/

	return ret;
}

static int proect_mmsys_salve_0()
{
	/* for whitney*/
	extern unsigned char *devapc_ao_mm_vaddr;
	extern int has_started;
	int ret = 0;
	unsigned int mmsys_slave_0 = 0x2;

	if (has_started) {
		unsigned int mmsys_slave = *(volatile unsigned int*)DEVAPC0_MM_SECURE_EN_0;
		*(volatile unsigned int*)DEVAPC0_MM_SECURE_EN_0 = (mmsys_slave & 0xFFFFFFFD) | mmsys_slave_0;
	} else {
		ret = -1;
		DDPERR("tui fail to protect mmsys slave 0!\n");
	}
	return ret;
}

static int unproect_mmsys_salve_0()
{
	/* for whitney*/
	extern unsigned char *devapc_ao_mm_vaddr;
	extern int has_started;
	int ret = 0;
	unsigned int mmsys_slave_0 = 0x0;
	if (has_started) {
		unsigned int mmsys_slave = *(volatile unsigned int*)DEVAPC0_MM_SECURE_EN_0;
		*(volatile unsigned int*)DEVAPC0_MM_SECURE_EN_0 = (mmsys_slave & 0xFFFFFFFD) | mmsys_slave_0;
	} else {
		ret = -1;
		DDPERR("tui fail to unprotect mmsys slave 0!\n");
	}
	return ret;
}

static int protect_display_engines()
{
	/*protect display engines */
	/* permission control: 0-no protect; 1-sec RW; 2-sec RW & non-sec RD; 3-no access; */

	/* Important Note: ALWAYS USE THE DEVICE APC MODULE INDEX IN "tlDapcIndex.h" HEADER */
	/*set_module_apc(DAPC_INDEX_MMSYS_CONFIG, DOMAIN_AP, SECURE_RW_AND_NON_SECURE_R_ONLY);
	*/
	if (proect_mmsys_salve_0())
		return -1;
	if (gDisp_version == SINGLE_WINDOWS_TUI)
		set_module_apc(DAPC_INDEX_DISP_RDMA0, DOMAIN_AP, SECURE_RW_AND_NON_SECURE_R_ONLY);
	else if (gDisp_version == MULTI_WINDOWS_TUI)
		set_module_apc(DAPC_INDEX_DISP_OVL0_2L, DOMAIN_AP, SECURE_RW_AND_NON_SECURE_R_ONLY);
	DDPDBG("%s done!\n", __func__);
	return 0;
}

static int unprotect_display_engines()
{
	/* unprotect display engines */

	/* Important Note: ALWAYS USE THE DEVICE APC MODULE INDEX IN "tlDapcIndex.h" HEADER */
	/*set_module_apc(DAPC_INDEX_MMSYS_CONFIG, DOMAIN_AP, NO_PROTECTION);*/
	if (unproect_mmsys_salve_0())
		return -1;
	if (gDisp_version == SINGLE_WINDOWS_TUI)
		set_module_apc(DAPC_INDEX_DISP_RDMA0, DOMAIN_AP, NO_PROTECTION);
	else if (gDisp_version == MULTI_WINDOWS_TUI)
		set_module_apc(DAPC_INDEX_DISP_OVL0_2L, DOMAIN_AP, NO_PROTECTION);
	DDPDBG("%s done!\n", __func__);
	return 0;
}


int disp_tui_enter()
{
	int ret;

	DDPMSG("%s\n", __func__);

	if (gDisp_version == SINGLE_WINDOWS_TUI) {
		/*disable rdma irq before protect it */
		irq_backup = rdma_disable_irq_backup(gDisp_module, NULL);

#if DISP_RDMA_INTERRUPT_ENABLE
		rdma_irq_attach(gDisp_module);
#endif
		gScenario = DDP_SCENARIO_PRIMARY_RDMA0_COLOR0_DISP;
	} else {
		irq_backup = ovl_disable_irq_backup(gDisp_module, NULL);
		gScenario = DDP_SCENARIO_PRIMARY_DISP;
	}
	ret = protect_display_engines();
	if(ret) {
		DDPERR("error to enter tui because fail to protect engines!!\n");
		goto err1;
	}

	/* check path */
	ret = ddp_check_path_strict(gScenario);
	if(ret) {
		DDPERR("error to enter tui because path is incorrect !!\n");
		disp_tui_analyse();
		goto err1;
	}

	if (gDisp_version == SINGLE_WINDOWS_TUI) {
		/* check path can't covor rdma mode */
		ret = rdma_is_mem_mode(gDisp_module);
		if (!ret) {
			DDPERR("error to enter tui because %s is not mem mode\n",
				ddp_get_module_name(gDisp_module));
			goto err1;
		}
		/* protect port config registers */
		rdma_reg_backup(gDisp_module);
		/*ddp_mutex_reg_backup(0);*/
	}
	if (gDdp_mode == DDP_CMD_MODE && gCmdq_handle) {
		/* see notes before disp_tui_wait_frame_done() */
		DSI_WaitForNotBusy(gDsi_module, NULL);
	}

	disp_tui_analyse();
	DDPMSG("%s -\n", __func__);
	return 0;

err1:

	unprotect_display_engines();
	if (gDisp_version == SINGLE_WINDOWS_TUI) {
#if DISP_RDMA_INTERRUPT_ENABLE
		rdma_irq_dettach(gDisp_module);
#endif
		rdma_irq_restore(gDisp_module, NULL, irq_backup);
	} else {
		ovl_irq_restore(gDisp_module, NULL, irq_backup);
	}

	return -1;
}

static int remove_tui_layer(void)
{
	cmdqRecHandle cmdq_handle = gCmdq_handle;
	struct OVL_CONFIG_STRUCT *ovl_cfg;

	/*DDPMSG("%s, start!\n", (__func__));*/
	disp_tui_cmdq_reset(cmdq_handle);
	if(cmdq_handle) {
		/* see notes before disp_tui_wait_frame_done() */
		_cmdq_insert_wait_frame_done_token_mira(cmdq_handle);

	}

	if (!is_pConfig_inited) {
		/*Allocate a block of memory from the heap for the pConfig struct.*/
		pConfig = drApiMalloc(sizeof(*pConfig),0);
		if (pConfig == NULL) {
			DDPERR("pConfig allocated FAILED!!\n");
			ASSERT(0);
			return -1;
		}
		is_pConfig_inited = 1;
	}

	ovl_cfg = &(pConfig->ovl_config[0]);
	ovl_cfg->layer = 0;
	ovl_cfg->isDirty = 1;
	ovl_cfg->buff_idx = -1;
	ovl_cfg->layer_en = 0;
	pConfig->dst_w = X_RES;
	pConfig->dst_h = Y_RES;
	pConfig->dst_dirty = 1;
	pConfig->ovl_dirty = 1;
	pConfig->ovl_layer_scanned = 0;
	ovl_config_l(DISP_MODULE_OVL0_2L, pConfig, cmdq_handle);
	disp_m4u_config_port(cmdq_handle, M4U_LARB0_PORT_DISP_2L_OVL0_LARB0, 1, 1);

	cmdqRecSetEventToken(cmdq_handle, CMDQ_SYNC_TOKEN_CONFIG_DIRTY);
	disp_tui_cmdq_flush(cmdq_handle);

	DDPMSG("%s, done!\n", (__func__));
}

int disp_tui_leave()
{
	cmdqRecHandle cmdq_handle = gCmdq_handle;

	DDPMSG("%s start\n", __func__);

	disp_tui_cmdq_reset(cmdq_handle);

	if (gDisp_version == SINGLE_WINDOWS_TUI) {
		/* clear fb and trigger black screen Robin*/
		disp_tui_wait_frame_done(cmdq_handle);
	} else if (gDisp_version == MULTI_WINDOWS_TUI) {
		remove_tui_layer();
		_cmdq_insert_wait_frame_done_token_mira(cmdq_handle);
	}
	/*restore display registers*/
	disp_tui_reg_restore(cmdq_handle);

	if (gDisp_version == SINGLE_WINDOWS_TUI) {
		/* config rdma port */
		disp_m4u_config_port(cmdq_handle, M4U_LARB0_PORT_DISP_RDMA0, 1, 0);
	} else if (gDisp_version == MULTI_WINDOWS_TUI) {
		disp_m4u_config_port(cmdq_handle, M4U_LARB0_PORT_DISP_2L_OVL0_LARB0, 1, 0);
	}

	DDPMSG("rdma_eof is %d\n",
		cmdqTzCoreGetEvent(rdma_get_EOF_cmdq_event(DISP_MODULE_RDMA0)));

	disp_tui_cmdq_flush(cmdq_handle);

	DDPMSG("%s wait cmdq done\n", __func__);

	unprotect_display_engines();

	if (gDisp_version == SINGLE_WINDOWS_TUI) {
#if DISP_RDMA_INTERRUPT_ENABLE
		rdma_irq_dettach(gDisp_module);
#endif
		/* restore rdma irq */
		rdma_irq_restore(gDisp_module, NULL, irq_backup);
	} else if (gDisp_version == MULTI_WINDOWS_TUI) {
		if (is_pConfig_inited && pConfig!=NULL) {
			/*Free the memory block allocated for pConfig struct*/
			drApiFree(pConfig);
			pConfig = NULL;
			is_pConfig_inited = 0;
			DDPMSG("%s drApiFree pConfig done, pConfig=0x%x\n", __func__, pConfig);
		}
		ovl_irq_restore(gDisp_module, NULL, irq_backup);
	}

	tui_m4u_deinit();

	DDPMSG("%s done\n", __func__);
	return 0;
}


int disp_tui_poll(void* addr, unsigned int mask, unsigned int value, long timeout)
{
	volatile long i = 0;
	volatile unsigned int reg_val;
	int ret = 0;

	while(1) {
		reg_val = DISP_REG_GET(addr);

		/*DDPMSG("poll%d 0x%p = 0x%x, msk=0x%x, val=0x%x\n", i, addr, reg_val, mask, value);*/

		if ((reg_val & mask) == value) {
			ret = 0;
			break;
		}
		if (timeout != -1 && i > timeout) {
			ret = -1;
			break;
		}

		i++;
	}
	return ret;
}

static inline int disp_tui_poll_nz(void* addr, unsigned int mask, long timeout)
{
	volatile long i = 0;
	volatile unsigned int reg_val;
	int ret = 0;

	while (1) {
		reg_val = DISP_REG_GET(addr);

		/*DDPMSG("poll%d 0x%p = 0x%x, msk=0x%x, val=0x%x\n", i, addr, reg_val, mask, value);*/

		if ((reg_val & mask)) {
			ret = 0;
			break;
		}
		if (timeout != -1 && i > timeout) {
			ret = -1;
			break;
		}

		i++;
	}
	return ret;
}


void disp_tui_delay(unsigned int cnt)
{
	volatile unsigned int i = 0, k;

	DDPMSG("begin delay %u\n", cnt);
	for(i=0; i<cnt; i++)
		k = i;
	DDPMSG("finish delay %u\n", cnt);
}

void test_display(void)
{
	DDPMSG("test_display+");
	disp_tui_init();
	DDPMSG("tui reg init done\n");

	disp_tui_enter();

	disp_tui_analyse();

	disp_tui_leave();
	DDPMSG("test_display-");
}


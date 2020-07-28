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
#include "tlDapcIndex.h"
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

#include "dapc.h"
#include "log.h"
#include "ddp_drv.h"

#include "display_tui.h"
#include "tui_m4u.h"

#define TAG "[display]"

struct _reg_backup {
	void * va;
	unsigned int val;
};

static struct _reg_backup regs_backup[30];

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
			/*DDPDBG("0x%x = 0x%x\n", disp_addr_convert((unsigned long)regs_backup[i].va),
									regs_backup[i].val);*/
			DISP_REG_SET(cmdq_handle, regs_backup[i].va, regs_backup[i].val);
			regs_backup[i].va = NULL;
			regs_backup[i].val = 0;
		}
	}
	DDPDBG("restore disp regs done ===========<\n");

	return 0;
}

/*=========================SMI/M4U ======================*/
#define LARB0_BASE 	ddp_reg_pa_base[DISP_REG_SMI_LARB0] //0x14026000

/* ================================================================ */
/* SMI larb */
/* common macro definitions */
#define F_VAL(val, msb, lsb) (((val)&((1<<(msb-lsb+1))-1))<<lsb)
#define F_MSK(msb, lsb)     F_VAL(0xffffffff, msb, lsb)
#define F_BIT_SET(bit)	  (1<<(bit))
#define F_BIT_VAL(val, bit)  ((!!(val))<<(bit))
#define F_MSK_SHIFT(regval, msb, lsb) (((regval)&F_MSK(msb, lsb))>>lsb)

/* ================================================================ */

#define SMI_LARB_SEC_CONx(larb_port)	(0xf80 + ((larb_port)<<2))
	#define F_SMI_SEC_MMU_EN(en)	F_BIT_VAL(en, 0)
	#define F_SMI_SEC_EN(sec)	    F_BIT_VAL(sec, 1)
	#define F_SMI_DOMN(domain)	    F_VAL(domain, 6, 4)
#define SMI_SEC_MMU_EN_FLD			REG_FLD(1, 0)
#define SMI_SEC_EN_FLD				REG_FLD(1, 1)	

enum _M4U_PORT
{
	DISP_M4U_PORT_OVL0 =0,
	DISP_M4U_PORT_2L_OVL0_LARB0 =1,
	DISP_M4U_PORT_RDMA0 =2,
	DISP_M4U_PORT_WDMA0 =3,
	MDP_M4U_PORT_RDMA0 =4,
	MDP_M4U_PORT_WDMA0 =5,
	MDP_M4U_PORT_WROT0 =6,
	DISP_M4U_PORT_OVL1=45,
	DISP_M4U_PORT_2L_OVL1=46,
	DISP_M4U_PORT_RDMA1=47,
	DISP_M4U_PORT_RDMA2=48,
	DISP_M4U_PORT_WDMA1=49,
	DISP_M4U_PORT_2L_OVL0_LARB4=52,
	MDP_M4U_PORT_RDMA1=53,
	MDP_M4U_PORT_WROT1=54,
};

/*=======================================================*/

static void* larb0_base = NULL;
int disp_m4u_init()
{
	larb0_base = dispsys_reg[DISP_REG_SMI_LARB0];
	tui_m4u_Init();
#if 0
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
#endif
	DDPMSG("M4U reg_map, map_addr=%p,reg_pa=0x%x\n", larb0_base, LARB0_BASE);

	return 0;
}

int disp_m4u_config_port(cmdqRecHandle cmdq_handle, int larb_port, int m4u_en, int sec)
{
	unsigned int regval;

	DISP_REG_MASK_EXT(cmdq_handle, larb0_base+SMI_LARB_SEC_CONx(larb_port), LARB0_BASE+SMI_LARB_SEC_CONx(larb_port),
			F_SMI_SEC_MMU_EN(!!(m4u_en)), F_SMI_SEC_MMU_EN(1));

	DISP_REG_MASK_EXT(cmdq_handle, larb0_base+SMI_LARB_SEC_CONx(larb_port), LARB0_BASE+SMI_LARB_SEC_CONx(larb_port),
			F_SMI_SEC_EN(!!(sec)), F_SMI_SEC_EN(1));

	DDPMSG("%s: port=%d, mmu_en=%d, sec=%d, regval=0x%x\n", __func__, larb_port, m4u_en, sec, DISP_REG_GET(larb0_base+SMI_LARB_SEC_CONx(larb_port)));

	return 0;
}

/*====================================================================*/
static int disp_tui_cmdq_reset(cmdqRecHandle cmdq_handle)
{
	/*DDPMSG("%s, dxs cmdq_handle =%d\n", (__func__), cmdq_handle);*/
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
	DDPMSG("===== dump cmd before flush ======>\n");
	cmdqRecDumpCommand(cmdq_handle);
	DDPMSG("===== dump cmd done ======>\n");
#endif

	ret = cmdqRecFlush(cmdq_handle);
	if(ret)
		DDPERR("error to flush cmdq handle!!\n");

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

    ddp_check_path(gScenario);
    ddp_check_mutex(gMutex_id, gScenario, gDdp_mode);
#if 1
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
	/*DDPMSG("%s, dxs start!\n", (__func__));*/

	ddp_mutex_enable(mutex_id, gScenario, cmdq_handle);
	ddp_dsi_trigger(dsi_module, cmdq_handle);
#if 1
/*<<+ force_commit, bypass_shadow */
		if (DISP_OPT_SHADOW_MODE != 0) {

		ddp_mutex_get(mutex_id, cmdq_handle);
		/* polling internal mutex is taken by sw */
		/*DISP_REG_CMDQ_POLLING(handle, DISP_REG_CONFIG_MUTEX_GET(handle->hwmutexid),
				REG_FLD_VAL(GET_FLD_INT_MUTEX0_EN, 1), REG_FLD_MASK(GET_FLD_INT_MUTEX0_EN));*/
		ddp_mutex_release(mutex_id, cmdq_handle);
		/* polling internal mutex is released */
		/*DISP_REG_CMDQ_POLLING(handle, DISP_REG_CONFIG_MUTEX_GET(handle->hwmutexid),
						REG_FLD_VAL(GET_FLD_INT_MUTEX0_EN, 1), REG_FLD_MASK(GET_FLD_INT_MUTEX0_EN));*/
		}
/* force_commit, bypass_shadow ->> */
#endif
	return 0;
}

int disp_tui_wait_lcm_te(cmdqRecHandle cmdq_handle)
{
	int ret = 0;
	/*DDPMSG("%s, start!\n", (__func__));*/
	if ((gDdp_mode == DDP_CMD_MODE) && cmdq_handle) {

		ret |= cmdqRecClearEventToken(cmdq_handle, CMDQ_EVENT_DSI_TE);
		ret |= cmdqRecWait(cmdq_handle, CMDQ_EVENT_DSI_TE);
	}
	
	/*DDPMSG("%s, done!\n", (__func__));*/
	return ret;
}

int disp_tui_trigger(cmdqRecHandle cmdq_handle)
{
	/*DDPMSG("%s, start!\n", (__func__));*/

	if(gDdp_mode == DDP_VIDEO_MODE && ddp_dsi_is_busy(gDsi_module))
		return 0;
	if(cmdq_handle) {
		/* see notes before disp_tui_wait_frame_done() */
		cmdqRecClearEventToken(cmdq_handle, rdma_get_EOF_cmdq_event(gRdma_module));
	}
	disp_tui_wait_lcm_te(cmdq_handle);
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

enum UNIFIED_COLOR_FMT disp_tui_get_color_fmt(tuiFbPixelType_t type)
{
	/*DDPDBG("%s, tuiFbPixelType_t type=%d!\n", (__func__), type);*/

	switch(type) {

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

int disp_tui_pan_display(const tuiFbInfo_t *pFbInfo)
{
	cmdqRecHandle cmdq_handle = gCmdq_handle;
	struct rdma_bg_ctrl_t rdma_bg_ctrl;
	unsigned int bpp;
	unsigned long mva = 0;

	DDPMSG("%s+\n", __func__);
	disp_tui_cmdq_reset(cmdq_handle);
	cmdqRecClearEventToken(cmdq_handle, rdma_get_EOF_cmdq_event(gRdma_module));

	rdma_bg_ctrl.left = 0;
	rdma_bg_ctrl.top  = 0;
	rdma_bg_ctrl.right= 0;
	rdma_bg_ctrl.bottom= 0;

	bpp = UFMT_GET_bpp( disp_tui_get_color_fmt(pFbInfo->type) ) / 8;
	mva = tui_m4u_alloc_mva(DISP_M4U_PORT_RDMA0, pFbInfo->fbPhysAddr, pFbInfo->lineLength * pFbInfo->yRes);

	rdma_config(gRdma_module,
		RDMA_MODE_MEMORY,
		mva,
		disp_tui_get_color_fmt(pFbInfo->type),
		pFbInfo->lineLength,
		pFbInfo->xRes,
		pFbInfo->yRes,
		0, DISP_SECURE_BUFFER, 0, &rdma_bg_ctrl, cmdq_handle, NULL ,bpp);
	/*DDPMSG("%s, dxs rdma_config()done0!\n", (__func__));*/
	/* config rdma port: PA & SEC */
	/*DDPMSG("%s, dxs disp_m4u_config_port()start0!\n", (__func__));*/
	disp_m4u_config_port(cmdq_handle, DISP_M4U_PORT_RDMA0, 1, 1);
	dump_mva_graph();
	dump_pagetable_in_use();

	disp_tui_trigger(cmdq_handle);
	disp_tui_wait_frame_done(cmdq_handle);
	disp_tui_cmdq_flush(cmdq_handle);
	DDPMSG("%s-\n", __func__);

	return 0;
}

unsigned int rdma_irq_backup;

int disp_tui_dsi_get_module(void)
{
	int ufoe_mout = DISP_REG_GET(DISP_REG_CONFIG_DISP_UFOE_MOUT_EN);
	int dsi0_selin = DISP_REG_GET(DISP_REG_CONFIG_DSI0_SEL_IN);

	if (((ufoe_mout & 0x2) == 0x2) && (dsi0_selin == 0x1))
		return DISP_MODULE_DSIDUAL;
	else
		return DISP_MODULE_DSI0;
}

int disp_tui_init()
{
	static int is_inited = 0;
	int ret;

	if(is_inited) {
		disp_m4u_init();
		DDPDBG("%s has inited\n", __FUNCTION__);
		return 0;
	}
	is_inited = 1;

    disp_reg_init();
	disp_m4u_init();
	start_devapc();
	cmdqRecInitialize();

	gDsi_module = disp_tui_dsi_get_module();

	update_intferface_module(gDsi_module);

	gDdp_mode = ddp_dsi_get_mode(gDsi_module);

	DDPDBG("%s, dsi_module=%s, mode=%s\n", (__func__), ddp_get_module_name(gDsi_module),
		gDdp_mode==DDP_CMD_MODE ? "cmd" : "vdo");

	/*DISP_REG_CMDQ_POLLING(cmdq, &DSI_REG[module_idx]->DSI_STATE_DBG0, 0x00010000, 0x00010000);*/

#ifdef DISP_TUI_ENABLE_CMDQ
	ret = cmdqRecCreate(CMDQ_SCENARIO_PRIMARY_DISP, &gCmdq_handle);
	if(ret)
		DDPAEE("error to create cmdq handle ! ret=%d\n", ret);
	else
		DDPDBG("tui gCmdq_handle has been inited!\n", __FUNCTION__);
#endif

	/*disp_tui_init_fb_contex();*/

	return ret;
}

/*DAPC_INDEX_MMSYS_RDMA0  138 from tlDapcIndex.h*/
#define DAPC_INDEX_MMSYS_RDMA0 DAPC_INDEX_DISP_RDMA0

extern unsigned char *devapc_ao_vaddr;
/*map_DAPC_Register() has been done, if the dapc has started*/
extern int has_started;

static int protect_mmsys_salve_0()
{
	int ret = 0;
	unsigned int mmsys_slave_0 = 0x2;
	unsigned char *devapc_ao_mm_vaddr_2nd;

	devapc_ao_mm_vaddr_2nd = devapc_ao_vaddr + 0x0E00;
	if (has_started) {
		/*set the mmsys dapc 2nd protect*/
		DISP_REG_MASK(NULL, devapc_ao_mm_vaddr_2nd, mmsys_slave_0, 0x3);
	} else {
		ret = -1;
		DDPERR("tui fail to protect mmsys slave 0!\n");
	}
	return ret;
}

static int unprotect_mmsys_salve_0()
{
	int ret = 0;
	unsigned int mmsys_slave_0 = 0x0;
	unsigned char *devapc_ao_mm_vaddr_2nd;

	devapc_ao_mm_vaddr_2nd = devapc_ao_vaddr + 0x0E00;
	if (has_started) {
		/*set the mmsys dapc 2nd protect*/
		DISP_REG_MASK(NULL, devapc_ao_mm_vaddr_2nd, mmsys_slave_0, 0x3);
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

	/*set_module_apc(DAPC_INDEX_MMSYS_CONFIG, 0, 2);*/
	if (protect_mmsys_salve_0())
		return -1;
	set_module_apc(DAPC_INDEX_MMSYS_RDMA0, 0, 2);
	DDPDBG("%s done!\n", __func__);
	return 0;
}

static int unprotect_display_engines()
{
	/* unprotect display engines */
	/*set_module_apc(DAPC_INDEX_MMSYS_CONFIG, 0, 0);*/
	if (unprotect_mmsys_salve_0())
		return -1;
	set_module_apc(DAPC_INDEX_MMSYS_RDMA0, 0, 0);
	DDPDBG("%s done!\n", __func__);
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
	/* check path can't covor rdma mode */
	ret = rdma_is_mem_mode(gRdma_module);
	if(!ret) {
		DDPERR("error to enter tui because %s is not mem mode\n",
			ddp_get_module_name(gRdma_module));
		goto err1;
	}
	/* protect port config registers */
	rdma_reg_backup(gRdma_module);
	/*ddp_mutex_reg_backup(0);*/

	if(gDdp_mode == DDP_CMD_MODE && gCmdq_handle) {
		/* see notes before disp_tui_wait_frame_done() */
		DSI_WaitForNotBusy(gDsi_module, NULL);
		cmdqTzCoreSetEvent(rdma_get_EOF_cmdq_event(gRdma_module));
	}

	disp_tui_analyse();
	DDPMSG("%s, done!\n", (__func__));
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
	int ret;
	cmdqRecHandle cmdq_handle = gCmdq_handle;

	DDPMSG("%s start\n", __func__);

	disp_tui_cmdq_reset(cmdq_handle);
	/* clear fb and trigger black screen Robin*/
	disp_tui_wait_frame_done(cmdq_handle);

	/*restore display registers*/
	disp_tui_reg_restore(cmdq_handle);
	/* config rdma port */
	disp_m4u_config_port(cmdq_handle, DISP_M4U_PORT_RDMA0, 1, 0);

	//DDPMSG("%s rdma_eof is %d\n", __func__, cmdqTzCoreGetEvent(rdma_get_EOF_cmdq_event(gRdma_module)));

	disp_tui_cmdq_flush(cmdq_handle);

	ret = unprotect_display_engines();
	if(ret)
		DDPERR("error to lave tui because fail to unprotect engines!!\n");

#if DISP_RDMA_INTERRUPT_ENABLE
	rdma_irq_dettach(gRdma_module);
#endif
	/* restore rdma irq */
	rdma_irq_restore(gRdma_module, NULL, rdma_irq_backup);

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

		if((reg_val & mask) == value) {
			ret = 0;
			break;
		}
		if(timeout != -1 && i > timeout) {
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

	while(1) {
		reg_val = DISP_REG_GET(addr);

		/*DDPMSG("poll%d 0x%p = 0x%x, msk=0x%x, val=0x%x\n", i, addr, reg_val, mask, value);*/

		if((reg_val & mask)) {
			ret = 0;
			break;
		}
		if(timeout != -1 && i > timeout) {
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
	/*DDPMSG("begin delay %u\n", cnt);*/
	for(i=0; i<cnt; i++)
		k = i;
	/*DDPMSG("finish delay %u\n", cnt);*/
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


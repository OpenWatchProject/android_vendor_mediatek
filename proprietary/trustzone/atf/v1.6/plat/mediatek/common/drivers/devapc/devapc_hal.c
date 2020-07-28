/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <debug.h>
#include <devapc.h>
#include <devapc_hal.h>

/* TEE module's APC setting */
static struct devapc_set_callbacks devapc_tee_cb[] = {
	{ DEVAPC_TEE_MODULE_REQ_CAMERA_ISP, devapc_tee_camera_isp },
	{ DEVAPC_TEE_MODULE_REQ_IMGSENSOR, devapc_tee_imgsensor },
	{ DEVAPC_TEE_MODULE_REQ_VDEC, devapc_tee_vdec },
	{ DEVAPC_TEE_MODULE_REQ_VENC, NULL },
	{ DEVAPC_TEE_MODULE_REQ_M4U, devapc_tee_m4u },
	{ DEVAPC_TEE_MODULE_REQ_I2C, devapc_tee_i2c },
	{ DEVAPC_TEE_MODULE_REQ_SPI, devapc_tee_spi },
	{ DEVAPC_TEE_MODULE_REQ_DISP, devapc_tee_disp }

};

static struct devapc_set_callbacks devapc_tee_mast_cb[] = {
	{ DEVAPC_MASTER_REQ_SPI, devapc_tee_mast_spi },
};

/* GZ module's APC setting */
static struct devapc_set_callbacks devapc_hyp_cb[] = {
	{ DEVAPC_HYP_MODULE_REQ_APU, devapc_hyp_apu },
};

static struct devapc_set_callbacks devapc_hyp_mast_cb[] = {
	{ DEVAPC_MASTER_REQ_SPI, devapc_hyp_mast_spi },
};

/*
 * devapc_tee_set_protect - set module's APC which requested from TEE
 *
 * use case: protect CAMERA_ISP/IMGSENSOR/VDEC/VENC/M4U/I2C/SPI/DISP
 */
static uint32_t devapc_tee_set_protect(enum DEVAPC_TEE_MODULE_REQ_TYPE module,
		enum DEVAPC_PROTECT_ON_OFF onoff, uint32_t param)
{
	INFO("[DEVAPC] %s\n", __func__);

	for (int i = 0; i < DEVAPC_TEE_MODULE_REQ_NUM; i++) {
		if (devapc_tee_cb[i].req_type == module)
			return devapc_tee_cb[i].devapc_set_cb(onoff, param);
	}

	return DEVAPC_ERR_REQ_TYPE_NOT_SUPPORTED;
}

/*
 * devapc_tee_set_master_dom - set master domain which requested from TEE
 *
 * use case: SPI
 */
static uint32_t devapc_tee_set_master_trans(enum DEVAPC_MASTER_REQ_TYPE module,
		enum DEVAPC_PROTECT_ON_OFF onoff, uint32_t param)
{
	INFO("[DEVAPC] %s\n", __func__);

	for (int i = 0; i < DEVAPC_MASTER_REQ_NUM; i++) {
		if (devapc_tee_mast_cb[i].req_type == module)
			return devapc_tee_mast_cb[i].
				devapc_set_cb(onoff, param);
	}

	return DEVAPC_ERR_REQ_TYPE_NOT_SUPPORTED;
}

/*
 * devapc_hyp_set_protect - set module's APC which requested from hypervisor
 *
 * use case: protect APUSYS
 */
static uint32_t devapc_hyp_set_protect(enum DEVAPC_HYP_MODULE_REQ_TYPE module,
		enum DEVAPC_PROTECT_ON_OFF onoff, uint32_t param)
{
	INFO("[DEVAPC] %s\n", __func__);

	for (int i = 0; i < DEVAPC_HYP_MODULE_REQ_NUM; i++) {
		if (devapc_hyp_cb[i].req_type == module)
			return devapc_hyp_cb[i].devapc_set_cb(onoff, param);
	}

	return DEVAPC_ERR_REQ_TYPE_NOT_SUPPORTED;
}

/*
 * devapc_hyp_set_master_dom - set master domain which requested from hypervisor
 *
 * use case: SPI
 */
static uint32_t devapc_hyp_set_master_dom(enum DEVAPC_MASTER_REQ_TYPE module,
		enum DEVAPC_PROTECT_ON_OFF onoff, uint32_t param)
{
	INFO("[DEVAPC] %s\n", __func__);

	for (int i = 0; i < DEVAPC_MASTER_REQ_NUM; i++) {
		if (devapc_hyp_mast_cb[i].req_type == module)
			return devapc_hyp_mast_cb[i].
				devapc_set_cb(onoff, param);
	}

	return DEVAPC_ERR_REQ_TYPE_NOT_SUPPORTED;
}

uint64_t sip_tee_devapc_hal_req(uint32_t cmd,
		uint32_t x1, uint32_t x2, uint32_t x3)
{
	INFO("[DEVAPC] cmd:0x%x, x1:0x%x, x2:0x%x, x3:0x%x\n",
			cmd, x1, x2, x3);

	if (cmd == SIP_APC_MODULE_SET)
		return devapc_tee_set_protect(x1, x2, x3);
	else if (cmd == SIP_APC_MASTER_SET)
		return devapc_tee_set_master_trans(x1, x2, x3);

	ERROR("[DEVAPC] Not supported cmd:0x%x!\n", cmd);
	return DEVAPC_ERR_INVALID_CMD;
}

uint64_t sip_hyp_devapc_hal_req(uint32_t cmd,
		uint32_t x1, uint32_t x2, uint32_t x3)
{
	INFO("[DEVAPC] cmd:0x%x, x1:0x%x, x2:0x%x, x3:0x%x\n",
			cmd, x1, x2, x3);

	if (cmd == SIP_APC_MODULE_SET)
		return devapc_hyp_set_protect(x1, x2, x3);
	else if (cmd == SIP_APC_MASTER_SET)
		return devapc_hyp_set_master_dom(x1, x2, x3);

	ERROR("[DEVAPC] Not supported cmd:0x%x!\n", cmd);
	return DEVAPC_ERR_INVALID_CMD;
}


/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __DEVAPC_HAL_H__
#define __DEVAPC_HAL_H__

enum DEVAPC_TEE_MODULE_REQ_TYPE {
	DEVAPC_TEE_MODULE_REQ_CAMERA_ISP = 0,
	DEVAPC_TEE_MODULE_REQ_IMGSENSOR,
	DEVAPC_TEE_MODULE_REQ_VDEC,
	DEVAPC_TEE_MODULE_REQ_VENC,
	DEVAPC_TEE_MODULE_REQ_M4U,
	DEVAPC_TEE_MODULE_REQ_I2C,
	DEVAPC_TEE_MODULE_REQ_SPI,
	DEVAPC_TEE_MODULE_REQ_DISP,
	DEVAPC_TEE_MODULE_REQ_NUM,
};

enum DEVAPC_HYP_MODULE_REQ_TYPE {
	DEVAPC_HYP_MODULE_REQ_APU,
	DEVAPC_HYP_MODULE_REQ_NUM,
};

enum DEVAPC_MASTER_REQ_TYPE {
	DEVAPC_MASTER_REQ_SPI,
	DEVAPC_MASTER_REQ_NUM,
};

enum DEVAPC_PROTECT_ON_OFF {
	DEVAPC_PROTECT_DISABLE = 0,
	DEVAPC_PROTECT_ENABLE = 1,
};

struct devapc_set_callbacks {
	uint32_t req_type;
	uint32_t (*devapc_set_cb)(enum DEVAPC_PROTECT_ON_OFF, uint32_t);
};

uint64_t sip_tee_devapc_hal_req(uint32_t cmd,
		uint32_t x1, uint32_t x2, uint32_t x3);
uint64_t sip_hyp_devapc_hal_req(uint32_t cmd,
		uint32_t x1, uint32_t x2, uint32_t x3);

#endif /* __DEVAPC_HAL_H__ */


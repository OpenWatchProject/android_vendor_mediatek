/*
 * Copyright (c) 2019 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */

#define MTK_SIP_TEE_APC_MODULE_SET_AARCH32      0x82000040
#define MTK_SIP_TEE_APC_MODULE_SET_AARCH64      0xC2000040
#define MTK_SIP_TEE_APC_MM2ND_SET_AARCH32       0x82000041
#define MTK_SIP_TEE_APC_MM2ND_SET_AARCH64       0xC2000041
#define MTK_SIP_TEE_APC_MASTER_SET_AARCH32      0x82000042
#define MTK_SIP_TEE_APC_MASTER_SET_AARCH64      0xC2000042
#define MTK_SIP_TEE_HAL_APC_SET_AARCH32         0x82000043
#define MTK_SIP_TEE_HAL_APC_SET_AARCH64         0xC2000043
#define MTK_SIP_TEE_HAL_MASTER_TRANS_AARCH32    0x82000044
#define MTK_SIP_TEE_HAL_MASTER_TRANS_AARCH64    0xC2000044


static inline uint32_t __set_devapc_protect(enum DEVAPC_MODULE_REQ_TYPE module,
		enum DEVAPC_PROTECT_ON_OFF onoff, uint32_t param)
{
	uint32_t api_ret, smc_ret;

	api_ret = msee_smc_call(MTK_SIP_TEE_HAL_APC_SET_AARCH32, module,
				onoff, param, &smc_ret);

	if (api_ret) {
		SECDRV_REL_PRINT("%s: SMC API failed, ret=0x%x\n", __func__,
				api_ret);
		return DEVAPC_ERROR_SMC_CALL_API_FAIL;

	} else if (smc_ret) {
		SECDRV_REL_PRINT("%s: SMC Set failed, ret=0x%x\n", __func__,
				smc_ret);
		return DEVAPC_ERROR_SMC_CALL_RESULT_FAIL;
	}

	SECDRV_DBG_PRINT("SMC passed!\n");
	return DEVAPC_API_OK;

}

static inline uint32_t __set_devapc_master_trans(enum DEVAPC_MODULE_REQ_TYPE module,
		enum DEVAPC_PROTECT_ON_OFF onoff, uint32_t param)
{
	uint32_t api_ret, smc_ret;

	api_ret = msee_smc_call(MTK_SIP_TEE_HAL_MASTER_TRANS_AARCH32, module,
				onoff, param, &smc_ret);

	if (api_ret) {
		SECDRV_REL_PRINT("%s: SMC API failed, ret=0x%x\n", __func__,
				api_ret);
		return DEVAPC_ERROR_SMC_CALL_API_FAIL;

	} else if (smc_ret) {
		SECDRV_REL_PRINT("%s: SMC Set failed, ret=0x%x\n", __func__,
				smc_ret);
		return DEVAPC_ERROR_SMC_CALL_RESULT_FAIL;
	}


	SECDRV_DBG_PRINT("SMC passed!\n");
	return DEVAPC_API_OK;
}

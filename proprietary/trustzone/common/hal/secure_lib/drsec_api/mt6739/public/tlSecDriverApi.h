/*
 * Copyright (c) 2013 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */

/*
 * @file   tlSecDriverApi.h
 * @brief  Contains trustlet API definitions
 *
 */

#ifndef __TL_M_SEC_DRIVER_API_H__
#define __TL_M_SEC_DRIVER_API_H__

#include "tlApisec.h"


/*
 * Error code for Device APC
 */
#ifdef SMC_SUPPORT
#define DEVAPC_API_OK                       0x00000000
#define DEVAPC_ERROR_SMC_CALL_API_FAIL      0x10000000
#define DEVAPC_ERROR_SMC_CALL_RESULT_FAIL   0x20000000
#define DEVAPC_ERROR_BOUNDARY_CHECK_FAIL    0x30000000
#else
#define E_DAPC_SUCCESS                      0X00000000
#define E_DAPC_MODULE_DOMAIN_WRONG_INDEX    0X10000000
#define E_DAPC_MODULE_WRONG_PERMISSION      0X20000000
#define E_DAPC_SLAVE_INDEX_EXCEED_LIMIT     0X30000000
#define E_DAPC_SLAVE_TYPE_NOT_AVAILABLE     0X40000000
#define E_DAPC_MASTER_WRONG_TRANS_TYPE      0X50000000
#define E_DAPC_MASTER_INDEX_EXCEED_LIMIT    0X60000000
#define E_DAPC_MASTER_TYPE_NOT_AVAILABLE    0X70000000
#endif /* SMC_SUPPORT */

int32_t tlset_mm2nd_apc(unsigned int module, unsigned int security_type);
int32_t tlset_module_apc(unsigned int module, unsigned int domain_num, unsigned int permission_control);
int32_t tlset_master_transaction(unsigned int master_index, unsigned int transaction_type);

int32_t tlset_devapc_protect(enum DEVAPC_MODULE_REQ_TYPE module,
		enum DEVAPC_PROTECT_ON_OFF onoff, uint32_t param);
int32_t tlset_devapc_master_trans(enum DEVAPC_MODULE_REQ_TYPE module,
		enum DEVAPC_PROTECT_ON_OFF onoff, uint32_t param);

#endif // __TL_M_SEC_DRIVER_API_H__

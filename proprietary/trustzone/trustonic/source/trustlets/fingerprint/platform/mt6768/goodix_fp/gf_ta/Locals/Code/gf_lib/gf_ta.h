/*
* Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
* All Rights Reserved.
*/


#ifndef __GF_TA_H__
#define __GF_TA_H__

#include <gf_error.h>
#include "gf_type_define.h"
#ifdef __cplusplus
extern "C" {
#endif

gf_error_t gf_ta_create_entry_point(void);

void gf_ta_destroy_entry_point(void);

gf_error_t gf_ta_invoke_cmd_entry_point(uint32_t cmd_id, void *buffer, uint32_t buffer_len);

gf_error_t gf_ta_invoke_user_cmd_entry_point(uint32_t cmd_id, void *buffer);

//alipay interface
/**
 * @brief
 *  get last authenticated finger id
 *  @param[in|out] auth_id
 *  @param[in|out] auth_id len
 *  @return
 *      GF_ERROR_BAD_PARAMS:auth_id is NULL
 *      GF_SUCCESS:success
 *      GF_ERROR_NOT_MATCH:last authenticate can not found match finger
 *      GF_ERROR_TIMEOUT:last authenticate found match finger,but timeout
 */
gf_error_t gf_ta_get_last_identify_id(uint8_t *pIdBuf, uint32_t *pBufLen);

gf_error_t gf_ta_get_id_list(uint8_t *pIdBuf, uint32_t *pBufLen);

gf_error_t gf_get_authenticator_version(uint32_t *pVersion);

gf_error_t gf_ta_get_hmac_key(void);


#ifdef __cplusplus
}
#endif

#endif //__GF_TA_H__

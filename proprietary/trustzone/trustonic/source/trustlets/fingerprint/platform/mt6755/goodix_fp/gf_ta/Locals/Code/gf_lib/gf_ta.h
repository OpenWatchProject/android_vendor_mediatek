#ifndef __GF_TA_H__
#define __GF_TA_H__

#include <gf_error.h>

#ifdef __cplusplus
extern "C" {
#endif

gf_error_t gf_ta_create_entry_point(void);
void gf_ta_destroy_entry_point(void);
gf_error_t gf_ta_invoke_cmd_entry_point(void *buffer);

//alipay interface
/**
 * @brief
 *  get last authenticated finger id
 *  @param[in|out] auth_id
 *  @return
 *      GF_ERROR_BAD_PARAMS:auth_id is NULL
 *      GF_SUCCESS:success
 *      GF_ERROR_NOT_MATCH:last authenticate can not found match finger
 *      GF_ERROR_TIMEOUT:last authenticate found match finger,but timeout
 */
gf_error_t gf_ta_get_last_identify_id(uint32_t *id);

#ifdef __cplusplus
}
#endif

#endif //__GF_TA_H__

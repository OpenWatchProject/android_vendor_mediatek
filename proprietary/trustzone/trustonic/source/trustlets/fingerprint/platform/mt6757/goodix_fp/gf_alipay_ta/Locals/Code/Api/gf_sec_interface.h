#ifndef _GF_SEC_INTERFACE_H_
#define _GF_SEC_INTERFACE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <tee_internal_api.h>

TEE_Result gf_alipay_api_get_auth_id(int32_t* finger_id);

#ifdef __cplusplus
}
#endif

#endif //_GF_SEC_INTERFACE_H_

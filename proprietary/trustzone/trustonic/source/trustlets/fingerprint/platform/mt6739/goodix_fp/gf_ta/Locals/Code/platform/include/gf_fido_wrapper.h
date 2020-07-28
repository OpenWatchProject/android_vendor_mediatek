/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */

#ifndef __GF_FIDO_WRAPPER_H__
#define __GF_FIDO_WRAPPER_H__

#include "gf_log.h"
#include "gf_type_define.h"


#ifdef __cplusplus
extern "C" {
#endif

gf_error_t gf_fido_get_attestation_private_key(uint8_t* pAttKeyPrv);
gf_error_t gf_fido_buffer_is_non_secure_mem(const uint8_t* pBuffer, uint32_t nLen);
gf_error_t gf_fido_get_wrapped_len(uint32_t nLen);
gf_error_t gf_fido_wrap_object_to_other_ta(const uint8_t* pInBuf, uint32_t nInLen, uint8_t* pOutBuf,
        uint32_t* pnOutLen);
gf_error_t gf_fido_unwrap_object_to_other_ta(uint8_t* pDestTA, const uint8_t* pInBuf,
        uint32_t nInLen, uint8_t* pOutBuf, uint32_t* pnOutLen);
gf_error_t gf_fido_wrap_object_to_self(const uint8_t* pInBuf, uint32_t  nInLen, uint8_t* pOutBuf,
        uint32_t* pnOutLen);
gf_error_t gf_fido_unwrap_object_to_self(const uint8_t* pInBuf, uint32_t  nInLen, uint8_t* pOutBuf,
        uint32_t* pnOutLen);
gf_error_t gf_fido_generate_random_data(uint32_t nRandDataLen, uint8_t* pRandData);
gf_error_t gf_fido_get_last_wrap_key(uint8_t *buf, uint32_t *buf_len);

#ifdef __cplusplus
}
#endif


#endif  // __GF_FIDO_WRAPPER_H__

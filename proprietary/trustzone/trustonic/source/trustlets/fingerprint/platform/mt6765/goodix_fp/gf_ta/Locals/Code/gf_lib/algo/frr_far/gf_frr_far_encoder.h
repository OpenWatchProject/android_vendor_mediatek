/*
* Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
* All Rights Reserved.
*/


#ifndef __GF_FRR_FAR_ENCODER_H__
#define __GF_FRR_FAR_ENCODER_H__

#include "gf_error.h"
#include "gf_type_define.h"
#include "gf_common.h"

#ifdef __cplusplus
extern "C" {
#endif
void encode_buf_crc32(uint8_t* buf, uint32_t buf_len);
gf_error_t check_buf_crc32(uint8_t* buf, uint32_t len);

gf_error_t gf_frr_far_get_fingerTempInfo_size(uint32_t *len);
gf_error_t gf_frr_far_decode_fingertempinfo(uint8_t *buf, uint32_t len, void** finger_temp_info);
gf_error_t gf_frr_far_encode_fingertempinfo(uint8_t *buf, uint32_t buf_len);

gf_error_t gf_frr_far_get_finger_list_size(uint32_t *len);
gf_error_t gf_frr_far_decode_finger_list(uint8_t *buf, uint32_t buf_len);
gf_error_t gf_frr_far_encode_finger_list(uint8_t *buf, uint32_t buf_len);

void print_finger_list(void);
#ifdef __cplusplus
}
#endif

#endif // __GF_FRR_FAR_ENCODER_H__

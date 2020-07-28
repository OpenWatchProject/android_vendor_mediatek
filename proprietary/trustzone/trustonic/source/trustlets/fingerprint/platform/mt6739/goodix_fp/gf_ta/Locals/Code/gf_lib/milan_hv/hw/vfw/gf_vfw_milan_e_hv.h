/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#ifndef __GF_VFW_MILAN_E_HV_H__
#define __GF_VFW_MILAN_E_HV_H__

#include "gf_error.h"

#ifdef __cplusplus
extern "C" {
#endif

uint8_t vfw_milan_e_hv_get_otp_data_crc(uint8_t* otp_buf);
gf_error_t vfw_milan_e_hv_set_general_reg(void);
gf_error_t vfw_milan_e_hv_update_dac_to_regs(uint16_t dac_groups[]);

#ifdef __cplusplus
}
#endif

#endif  // __GF_VFW_MILAN_E_HV_H__

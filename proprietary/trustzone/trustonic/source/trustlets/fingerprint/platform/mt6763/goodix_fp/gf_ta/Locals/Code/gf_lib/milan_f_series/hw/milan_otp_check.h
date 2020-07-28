/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#ifndef __MILAN_OTP_CHECK_H__
#define __MILAN_OTP_CHECK_H__

#include "milan_common.h"

#ifdef __cplusplus
extern "C" {
#endif

gf_error_t milan_efglfn_calculate_otp_crc(uint8_t* otp_buf);
gf_error_t milan_kjh_calculate_otp_crc(uint8_t* otp_buf);
gf_error_t milan_efglfn_check_ft_test_info(uint8_t* otp_buf);
gf_error_t milan_kjh_check_ft_test_info(uint8_t* otp_buf);

#ifdef __cplusplus
}
#endif
#endif  // __MILAN_OTP_CHECK_H__


/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#ifndef __GF_TA_TEST_CHIP_SERIES_H__
#define __GF_TA_TEST_CHIP_SERIES_H__

#include "gf_common.h"
#include "gf_error.h"

#ifdef __cplusplus
extern "C" {
#endif

void gf_ta_test_chip_series_init_global_variable(void);
gf_error_t gf_ta_test_performance(gf_cmd_header_t *cmd);
gf_error_t gf_ta_test_spi(gf_test_spi_t *cmd);
gf_error_t gf_ta_test_interrupt_pin(gf_cmd_header_t *cmd);
gf_error_t gf_ta_test_pre_get_version(gf_cmd_header_t *cmd);
gf_error_t gf_ta_test_get_version(gf_test_get_version_t *cmd);
gf_error_t gf_ta_test_pixel_open_step1(gf_cmd_header_t *cmd);
gf_error_t gf_ta_test_pixel_open_finish(gf_cmd_header_t *cmd);
gf_error_t gf_ta_test_bad_point(gf_cmd_header_t *cmd);

gf_error_t gf_ta_test_chip_series_invoke_cmd_entry_point(uint32_t cmd_id, void *buffer);
gf_error_t gf_ta_test_chip_series_driver_cmd(gf_test_driver_cmd_t *cmd);

#ifdef __cplusplus
}
#endif

#endif  // __GF_TA_TEST_CHIP_SERIES_H__

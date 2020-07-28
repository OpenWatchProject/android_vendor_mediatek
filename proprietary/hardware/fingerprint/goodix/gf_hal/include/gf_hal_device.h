/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#ifndef __GF_HAL_DEVICE_H__
#define __GF_HAL_DEVICE_H__

#include "gf_error.h"
#include "gf_common.h"

#ifdef __cplusplus
extern "C" {
#endif

extern gf_error_t gf_hal_device_open(void);
extern void gf_hal_device_close(void);

extern gf_error_t gf_hal_device_enable(void);
extern gf_error_t gf_hal_device_disable(void);

extern gf_error_t gf_hal_enable_power(void);
extern gf_error_t gf_hal_disable_power(void);
extern gf_error_t gf_hal_device_remove(void);
extern gf_error_t gf_hal_enable_irq(void);
extern gf_error_t gf_hal_disable_irq(void);

extern gf_error_t gf_hal_reset_chip(void);
extern gf_error_t gf_hal_get_fw_info(uint8_t *buf);
extern gf_error_t gf_hal_control_spi_clock(uint8_t enable);
extern gf_error_t gf_hal_send_key_event(gf_key_code_t code, gf_key_status_t status);
extern gf_error_t gf_hal_send_nav_event(gf_nav_code_t code);
extern gf_error_t gf_hal_enter_sleep_mode(void);

extern gf_error_t gf_hal_chip_info(gf_ioc_chip_info info);


#ifdef __cplusplus
}
#endif

#endif  // __GF_HAL_DEVICE_H__

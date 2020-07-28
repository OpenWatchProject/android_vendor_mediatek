/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */

#ifndef __GF_MILAN_H__
#define __GF_MILAN_H__

#ifdef __cplusplus
extern "C" {
#endif

uint8_t milan_otp_crc8(uint8_t *ucPtr, uint8_t ucLen);

extern gf_error_t gf_milan_chip_init(uint8_t *otp_info, uint32_t *otp_info_len);

extern gf_error_t gf_milan_pre_get_image(void);
extern gf_error_t gf_milan_get_image(uint8_t *origin_data, uint32_t *origin_data_len);
extern gf_error_t gf_milan_post_get_image(uint16_t *raw_data, uint32_t *raw_data_len);

extern gf_error_t gf_milan_pre_get_nav_image(void);
extern gf_error_t gf_milan_get_nav_image(uint8_t *origin_data, uint32_t *origin_data_len);
extern gf_error_t gf_milan_post_get_nav_image(uint16_t *raw_data, uint32_t *raw_data_len,
        uint8_t *frame_num, uint8_t *over_flag);

extern gf_error_t gf_milan_pre_get_nav_base(void);
extern gf_error_t gf_milan_get_nav_base(uint8_t *origin_data, uint32_t *origin_data_len);
extern gf_error_t gf_milan_post_get_nav_base(uint16_t *raw_data, uint32_t *raw_data_len);

extern gf_error_t gf_milan_nav_complete(void);

extern gf_error_t gf_milan_get_spi_speed(uint32_t irq_type, uint32_t *speed);

extern gf_error_t gf_milan_start_sensor_broken_check(void);
extern gf_error_t gf_milan_stop_sensor_broken_check(void);
extern gf_error_t gf_milan_update_dac_for_sensor_broken_check(uint32_t is_set_default_dac,
        uint32_t is_trigger_image);
extern gf_error_t gf_milan_update_dac_for_sensor_pixel_open_test(uint32_t is_set_default_dac,
        uint32_t is_trigger_image);

extern gf_error_t gf_milan_switch_mode(void);

extern gf_error_t gf_milan_update_fdt_base(uint16_t *buf, uint32_t buf_len);
extern gf_error_t gf_milan_detect_finger_up(void);
extern gf_error_t gf_milan_get_fdt_base(uint16_t **buf, uint32_t *length);
extern gf_error_t gf_milan_get_fdt_threshold(uint16_t* threshold);
extern gf_error_t gf_milan_set_data_base_status(uint8_t is_data_base_ok);
extern gf_error_t gf_milan_host_irq_set(void);

extern gf_error_t gf_milan_test_sensor_validity(void);

#ifdef __cplusplus
}
#endif

#endif /* __GF_MILAN_H__ */


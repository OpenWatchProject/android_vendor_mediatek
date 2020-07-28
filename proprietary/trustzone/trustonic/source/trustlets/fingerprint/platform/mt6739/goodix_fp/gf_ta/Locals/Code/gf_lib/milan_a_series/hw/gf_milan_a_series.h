/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#ifndef __GF_MILAN_A_SERIES_H__
#define __GF_MILAN_A_SERIES_H__

// milan-a chip_id consists with 16bit, in which last 4bit is changeable
// so that we use first 12bit to present milan-a, chip_id >> 4 makes this chip_id
#define MILAN_A_SERIES_SIZE_CONFIG 256

gf_error_t gf_milan_a_series_notify_fw_adc_reading_end(void);

gf_error_t gf_milan_a_series_notify_fw_adc_reading_begin(void);

extern gf_error_t gf_milan_a_series_chip_init(uint8_t *download_fw_flag,
        uint8_t *download_cfg_flag);

extern gf_error_t gf_milan_a_series_get_fw_version(uint8_t *buf, uint32_t buf_len);

extern gf_error_t gf_milan_a_series_download_fw(uint8_t *reset_flag);
extern gf_error_t gf_milan_a_series_download_cfg(gf_config_type_t config_type);
extern gf_error_t gf_milan_a_series_test_download_fw(uint8_t *fw_data, uint32_t fw_data_len);
extern gf_error_t gf_milan_a_series_test_download_cfg(uint8_t *cfg_data, uint32_t cfg_data_len);
extern gf_error_t gf_milan_a_series_test_download_fwcfg(uint8_t *fw_data, uint32_t fw_data_len,
        uint8_t *cfg_data, uint32_t cfg_data_len);
extern gf_error_t gf_milan_a_series_test_read_fw(uint8_t *fw_data, uint32_t *length);
extern gf_error_t gf_milan_a_series_test_read_cfg(uint8_t *cfg_data, uint32_t *length);

gf_error_t gf_milan_a_series_esd_check(uint8_t *result);

extern gf_error_t gf_milan_a_series_pre_get_image(void);
extern gf_error_t gf_milan_a_series_get_image(uint8_t *origin_data, uint32_t *origin_data_len);
extern gf_error_t gf_milan_a_series_post_get_image(uint16_t *raw_data, uint32_t *raw_data_len);

extern gf_error_t gf_milan_a_series_get_nav_image(uint8_t *origin_data, uint32_t *origin_data_len);
extern gf_error_t gf_milan_a_series_post_get_nav_image(uint16_t *raw_data, uint32_t *raw_data_len,
        uint8_t *frame_num, uint8_t *over_flag);

extern gf_error_t gf_milan_a_series_get_nav_base(uint8_t *origin_data, uint32_t *origin_data_len);
extern gf_error_t gf_milan_a_series_post_get_nav_base(uint16_t *raw_data, uint32_t *raw_data_len);

extern gf_error_t gf_milan_a_series_get_spi_speed(uint32_t irq_type, uint32_t *speed);

extern gf_error_t gf_milan_a_series_update_dac_for_sensor_pixel_open_test(
        uint32_t is_set_default_dac, uint32_t is_trigger_image);

extern gf_error_t gf_milan_a_series_get_gsc(uint8_t *untouch_data, uint32_t *untouch_data_len,
        uint8_t *touch_data, uint32_t *touch_data_len, uint16_t *gsc_base);
extern gf_error_t gf_milan_a_series_get_hbd(uint8_t *hbd_data, uint32_t *hbd_data_len);
extern gf_error_t gf_milan_a_series_get_hbd_base(uint16_t *base);

extern gf_error_t gf_milan_a_series_get_sensor_status(uint8_t *touch, uint8_t *temperature);
extern gf_error_t gf_milan_a_series_update_temprature_dac(void);
extern gf_error_t gf_milan_a_series_update_dac_offset_by_mean(int32_t mean_value);
extern gf_error_t gf_milan_a_series_judge_finger_stable(uint32_t sleep_time);
extern gf_error_t gf_milan_a_series_set_config_for_stable_acquire_image(uint8_t flag);
extern gf_error_t gf_milan_a_series_update_cfg_for_reissue_key_down(void);
extern gf_error_t gf_milan_a_series_enable_reissue_key_down(gf_mode_t mode);

#endif  // __GF_MILAN_A_SERIES_H__


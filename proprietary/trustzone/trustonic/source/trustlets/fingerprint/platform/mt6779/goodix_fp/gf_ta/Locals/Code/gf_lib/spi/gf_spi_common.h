/*
* Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
* All Rights Reserved.
*/

/**
 * @file   gf_spi_common.h
 * @brief  GF secure SPI driver defined
 *
 * <Copyright goes here>
 */

#ifndef __TA_GF_SPI_COMMON_H__
#define __TA_GF_SPI_COMMON_H__

#include "gf_error.h"
#include "gf_common.h"
#include "gf_config.h"
#include "gf_spi.h"

typedef struct {
    gf_error_t (*get_chip_id)(uint8_t *value, uint32_t buf_len);
    gf_error_t (*get_fw_version)(uint8_t *buf, uint32_t buf_len);
    gf_error_t (*get_vendor_id)(uint8_t *buf, uint32_t buf_len);
    gf_error_t (*get_production_date)(uint8_t *buf, uint32_t buf_len);
    gf_error_t (*get_sensor_id)(uint8_t *buf, uint32_t buf_len);
    gf_error_t (*get_sensor_otp_info)(uint8_t *buf, uint32_t buf_len);
    gf_error_t (*get_product_info)(void);
    gf_error_t (*write_product_info)(uint8_t product_cfg_idx);

    gf_error_t (*chip_init)(uint8_t *download_fw_flag, uint8_t *download_cfg_flag);
    gf_error_t (*erase_fw)(void);
    gf_error_t (*download_fw)(uint8_t *reset_flag);
    gf_error_t (*download_cfg)(gf_config_type_t config_type);
    gf_error_t (*fw_cfg_sanity_check)(void);
    gf_error_t (*chip_config_init)(void);
    gf_error_t (*init_finished)(void);

    gf_error_t (*get_image)(uint16_t **buf, uint32_t *length, uint8_t **origin_buf, uint32_t *origin_length);
    gf_error_t (*get_nav_image)(uint16_t **buf, uint32_t *length, uint8_t *frame_num,
            uint8_t *over_flag);
    gf_error_t (*get_nav_base)(uint16_t **buf, uint32_t *length);
    gf_error_t (*nav_complete)(void);
    gf_error_t (*esd_check)(uint8_t *result);

    gf_error_t (*data_sample_suspend)(void);
    gf_error_t (*data_sample_resume)(void);

    gf_error_t (*set_mode)(gf_mode_t mode);
    gf_error_t (*get_mode)(gf_mode_t *mode);

    gf_error_t (*start_sensor_broken_check)(void);
    gf_error_t (*stop_sensor_broken_check)(void);
    gf_error_t (*update_dac_for_sensor_broken_check)(uint32_t is_set_default_dac,
            uint32_t is_trigger_image);
    gf_error_t (*update_dac_for_sensor_pixel_open_test)(uint32_t is_set_default_dac,
            uint32_t is_trigger_image);

    gf_error_t (*get_irq_type)(uint32_t *irq_type, gf_mode_t current_mode);
    gf_error_t (*clear_irq)(uint32_t irq_type);

    gf_error_t (*switch_mode)(void);

    gf_error_t (*aes_set)(void);

    gf_error_t (*update_fdt_keybase)(uint16_t *buf, uint32_t buf_len);

    gf_error_t (*get_orientation)(uint16_t *orientation, uint16_t *facing);

    gf_error_t (*get_gsc)(uint8_t **buf, uint32_t *length);
    gf_error_t (*get_hbd)(uint8_t **buf, uint32_t *length);
    gf_error_t (*get_hbd_base)(uint16_t *base);

    gf_error_t (*test_download_fw_cfg)(uint8_t *fw_cfg_data, uint32_t fw_cfg_data_len, gf_download_fw_cfg_status_t fw_cfg_status);

    gf_error_t (*detect_finger_up)(void);

    gf_error_t (*get_fdt_base)(uint16_t **buf, uint32_t *length);
    gf_error_t (*get_fdt_threshold)(uint16_t* threshold);
    gf_error_t (*update_fdt_down_up_base)(void);
    gf_error_t (*check_sensor)(void);

    gf_error_t (*set_data_base_status)(uint8_t is_data_base_ok);
    gf_error_t (*get_sensor_status)(uint8_t *touch, uint8_t *temperature);
    gf_error_t (*get_key_status)(uint8_t *touch);
    gf_error_t (*get_irq_status_for_spi_clk)(uint32_t *irq_type, gf_mode_t current_mode);

    //driver test api
    gf_error_t (*spi_write_byte)(uint16_t address, uint8_t value);
    gf_error_t (*spi_read_byte)(uint16_t address, uint8_t *value);
    gf_error_t (*set_ignore_irq_type)(uint8_t irq_type_mask_offset);

    gf_error_t (*update_temprature_dac)(void);
    gf_error_t (*update_dac_offset_by_mean)(int32_t mean_value);

    gf_error_t (*select_chip_otp)(gf_detect_sensor_t* cmd);
    gf_error_t (*detect_sensor)(void);
     //milan_a_series judge finger stable
    gf_error_t (*judge_finger_stable)(uint32_t);

    gf_error_t (*set_config_for_stable_acquire_image)(uint8_t flag);

    gf_error_t (*update_cfg_for_reissue_key_down)(void);
    gf_error_t (*enable_reissue_key_down)(gf_mode_t mode);
    //milan_a_series print pixel key value
    gf_error_t (*print_pixel_key_value)(void);

} gf_spi_function_t;

extern uint16_t *g_gf_raw_data;
extern uint16_t *g_gf_raw_data_tmp;

/* common function for Oswego single and multi chip */
extern gf_error_t gf_spi_raw_data_split(uint16_t *dest, uint8_t *raw_data);
extern gf_error_t gf_spi_nav_raw_data_split(uint16_t *dest, uint8_t *raw_data, uint8_t frame_num);
extern gf_error_t gf_spi_dump_raw_data(uint16_t *addr, int len);
extern uint16_t gf_spi_cal_checksum(uint8_t *data, uint32_t len);
extern uint16_t gf_spi_cal_crc(uint8_t *data, uint16_t len);

extern gf_error_t gf_spi_function_init(gf_spi_function_t * spi_function);
extern gf_error_t gf_spi_function_customize(gf_spi_function_t *spi_function);
extern gf_error_t gf_spi_create(void);
extern gf_error_t gf_spi_destroy(void);
extern void endian_exchange(uint8_t* buf, int32_t len);

#endif  /* __TA_GF_SPI_COMMON_H__ */


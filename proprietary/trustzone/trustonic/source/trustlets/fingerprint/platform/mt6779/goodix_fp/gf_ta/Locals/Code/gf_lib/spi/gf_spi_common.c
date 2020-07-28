/*
* Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
* All Rights Reserved.
*/


/**
 * @file  gf_spi_common.c
 * @brief  GF secure SPI abstract layer driver
 *
 * <Copyright goes here>
 */

#include "gf_tee_internal_api.h"
#include "gf_spi_common.h"
#include "gf_sensor.h"
#include "cpl_memory.h"

#include "GF11_Rawdata_InvCipher.h"

#define LOG_TAG "[gf_spi_common] "

#define READ_FRAME_NUM  5

uint16_t *g_gf_raw_data = NULL;
uint16_t *g_gf_raw_data_tmp = NULL; //tmp buffer

void endian_exchange(uint8_t* buf, int32_t len) {
    int32_t i;
    uint8_t buf_tmp;
    int32_t size = len / 2;

    for (i = 0; i < size; i++) {
        buf_tmp = buf[2 * i + 1];
        buf[2 * i + 1] = buf[2 * i];
        buf[2 * i] = buf_tmp;
    }
}

gf_error_t gf_spi_create(void) {
    gf_error_t err = GF_SUCCESS;

    do {
        if (NULL == g_gf_raw_data) {
            g_gf_raw_data = (uint16_t *) CPL_MEM_MALLOC(READ_FRAME_NUM * g_sensor.row * g_sensor.col * sizeof(uint16_t));
            if (NULL == g_gf_raw_data) {
                GF_LOGE(LOG_TAG "gf_spi_create g_gf_raw_data out of memory");
                err = GF_ERROR_OUT_OF_MEMORY;
                break;
            }
        }

        if (NULL == g_gf_raw_data_tmp) {
            g_gf_raw_data_tmp = (uint16_t *) CPL_MEM_MALLOC(
                    READ_FRAME_NUM *g_sensor.row * g_sensor.col * sizeof(uint16_t));
            if (NULL == g_gf_raw_data_tmp) {
                GF_LOGE(LOG_TAG "gf_spi_create g_gf_raw_data_tmp out of memory");
                err = GF_ERROR_OUT_OF_MEMORY;
                break;
            }
        }
    } while (0);

    return err;
}

gf_error_t gf_spi_destroy(void) {
    gf_error_t err = GF_SUCCESS;

    if (g_gf_raw_data != NULL) {
        CPL_MEM_FREE(g_gf_raw_data);
        g_gf_raw_data = NULL;
    }

    if (g_gf_raw_data_tmp != NULL) {
        CPL_MEM_FREE(g_gf_raw_data_tmp);
        g_gf_raw_data_tmp = NULL;
    }

    return err;
}

uint16_t gf_spi_cal_checksum(uint8_t *data, uint32_t len) {
    uint16_t checksum = 0;
    uint32_t i = 0;
    for (i = 0; i < len; i++) {
        checksum += data[i];
    }
    return checksum;
}

gf_error_t gf_spi_raw_data_split(uint16_t *out, uint8_t *raw_data) {
    gf_error_t ret = GF_SUCCESS;
    uint8_t *src = raw_data;
    uint16_t *dst = g_gf_raw_data_tmp;
    uint8_t tmp[3] = { 0 };
    uint32_t i = 0;
    uint32_t j = 0;

    if (g_sensor.raw_data_is_encrypt) {
        /* length should be multiple of 12  */
        GF11_Rawdata_AES_InvCipher(raw_data, (g_sensor.sensor_raw_data_len + 11) / 12 * 12, 0);
    }

    for (j = 0; j < g_sensor.sensor_raw_data_len; j += g_sensor.line_aligned_len) {
        for (i = 0; i < g_sensor.line_len; i += 3) {
            tmp[0] = *src++;
            tmp[1] = *src++;
            tmp[2] = *src++;

            *dst++ = (tmp[0] << 4) + ((tmp[1] & 0xF0) >> 4);
            *dst++ = ((tmp[1] & 0x0F) << 8) + tmp[2];
        }
        src += (g_sensor.line_aligned_len - g_sensor.line_len);
    }

    for (i = 0; i < g_sensor.row; i++) {
        for (j = 0; j < g_sensor.col; j++) {
            *out++ = g_gf_raw_data_tmp[j * g_sensor.row + i];
        }
    }

    return ret;
}

gf_error_t gf_spi_nav_raw_data_split(uint16_t *out, uint8_t *raw_data, uint8_t frame_num) {
    gf_error_t ret = GF_SUCCESS;

    uint8_t *src = raw_data;
    uint16_t *dst = g_gf_raw_data_tmp;
    uint8_t tmp[3] = { 0 };
    uint32_t i = 0;
    uint32_t j = 0;
    uint32_t n = 0;
    uint32_t len = 0;
    uint16_t offset = 0;

    len = g_sensor.nav_frame_len * frame_num;

    for (j = 0; j < len; j += g_sensor.line_aligned_len) {
        for (i = 0; i < g_sensor.line_len; i += 3) {
            tmp[0] = *src++;
            tmp[1] = *src++;
            tmp[2] = *src++;

            *dst++ = (tmp[0] << 4) + ((tmp[1] & 0xF0) >> 4);
            *dst++ = ((tmp[1] & 0x0F) << 8) + tmp[2];
        }
        src += (g_sensor.line_aligned_len - g_sensor.line_len);
    }

    for (n = 0; n < frame_num; n++) {
        offset = g_sensor.row * g_sensor.nav_col * n;

        for (i = 0; i < g_sensor.row; i++) {
            for (j = 0; j < g_sensor.nav_col; j++) {
                *out++ = g_gf_raw_data_tmp[j * g_sensor.row + i + offset];
            }
        }
    }
    return ret;
}


/* check chip version, MP/ECO... */
static gf_error_t gf_spi_get_chip_id(uint8_t *buf, uint32_t buf_len) {
    UNUSED_VAR(buf);
    UNUSED_VAR(buf_len);
    return GF_SUCCESS;
}

static gf_error_t gf_spi_get_fw_version(uint8_t *buf, uint32_t buf_len) {
    UNUSED_VAR(buf);
    UNUSED_VAR(buf_len);
    return GF_SUCCESS;
}

static gf_error_t gf_spi_get_vendor_id(uint8_t *buf, uint32_t buf_len) {
    UNUSED_VAR(buf);
    UNUSED_VAR(buf_len);
    return GF_SUCCESS;
}

static gf_error_t gf_spi_get_production_date(uint8_t *buf, uint32_t buf_len) {
    UNUSED_VAR(buf);
    UNUSED_VAR(buf_len);
    return GF_SUCCESS;
}

static gf_error_t gf_spi_get_sensor_id(uint8_t *buf, uint32_t buf_len) {
    UNUSED_VAR(buf);
    UNUSED_VAR(buf_len);
    return GF_SUCCESS;
}

static gf_error_t gf_spi_get_sensor_otp_info(uint8_t *buf, uint32_t buf_len) {
    UNUSED_VAR(buf);
    UNUSED_VAR(buf_len);
    return GF_SUCCESS;
}

static gf_error_t gf_spi_get_product_info(void) {
    return GF_SUCCESS;
}

static gf_error_t gf_spi_write_product_info(uint8_t product_cfg_idx) {
    UNUSED_VAR(product_cfg_idx);
    return GF_SUCCESS;
}

static gf_error_t gf_spi_chip_init(uint8_t *download_fw_flag, uint8_t *download_cfg_flag) {
    UNUSED_VAR(download_fw_flag);
    UNUSED_VAR(download_cfg_flag);
    return GF_SUCCESS;
}

static gf_error_t gf_spi_erase_fw(void) {
    return GF_SUCCESS;
}

static gf_error_t gf_spi_download_fw(uint8_t *reset_flag) {
    UNUSED_VAR(reset_flag);
    return GF_SUCCESS;
}

static gf_error_t gf_spi_download_cfg(gf_config_type_t config_type) {
    UNUSED_VAR(config_type);
    return GF_SUCCESS;
}

static gf_error_t gf_spi_fw_cfg_sanity_check(void) {
    return GF_SUCCESS;
}

static gf_error_t gf_spi_chip_config_init(void) {
    return GF_SUCCESS;
}

static gf_error_t gf_spi_init_finished(void){
    return GF_SUCCESS;
}

static gf_error_t gf_spi_get_image(uint16_t **buf, uint32_t *length, uint8_t **origin_buf, uint32_t *origin_length) {
    UNUSED_VAR(buf);
    UNUSED_VAR(length);
    UNUSED_VAR(origin_buf);
    UNUSED_VAR(origin_length);
    return GF_SUCCESS;
}

static gf_error_t gf_spi_get_nav_image(uint16_t **buf, uint32_t *length, uint8_t *frame_num, uint8_t *over_flag) {
    UNUSED_VAR(buf);
    UNUSED_VAR(length);
    UNUSED_VAR(frame_num);
    UNUSED_VAR(over_flag);
    return GF_SUCCESS;
}

static gf_error_t gf_spi_get_nav_base(uint16_t **buf, uint32_t *length) {
    UNUSED_VAR(buf);
    UNUSED_VAR(length);
    return GF_SUCCESS;
}

static gf_error_t gf_spi_nav_complete(void) {
    return GF_SUCCESS;
}

static gf_error_t gf_spi_esd_check(uint8_t *result) {
    UNUSED_VAR(result);
    return GF_SUCCESS;
}

/*Suspend to sample finger-print data*/
static gf_error_t gf_spi_data_sample_suspend(void) {
    return GF_SUCCESS;
}

/*Resume to sample finger-print data*/
static gf_error_t gf_spi_data_sample_resume(void) {
    return GF_SUCCESS;
}

static gf_error_t gf_spi_set_mode(gf_mode_t mode) {
    UNUSED_VAR(mode);
    return GF_SUCCESS;
}

static gf_error_t gf_spi_get_mode(gf_mode_t *mode) {
    UNUSED_VAR(mode);
    return GF_SUCCESS;
}

static gf_error_t gf_spi_start_sensor_broken_check(void) {
    return GF_SUCCESS;
}

static gf_error_t gf_spi_stop_sensor_broken_check(void) {
    return GF_SUCCESS;
}

static gf_error_t gf_spi_update_dac_for_sensor_broken_check(uint32_t is_set_default_dac, uint32_t is_trigger_image) {
    UNUSED_VAR(is_set_default_dac);
    UNUSED_VAR(is_trigger_image);
    return GF_SUCCESS;
}

static gf_error_t gf_spi_update_dac_for_sensor_pixel_open_test(uint32_t is_set_default_dac, uint32_t is_trigger_image) {
    UNUSED_VAR(is_set_default_dac);
    UNUSED_VAR(is_trigger_image);
    return GF_SUCCESS;
}

static gf_error_t gf_spi_get_irq_type(uint32_t *irq_type, gf_mode_t current_mode) {
    UNUSED_VAR(irq_type);
    UNUSED_VAR(current_mode);
    return GF_SUCCESS;
}

static gf_error_t gf_spi_clear_irq(uint32_t irq_type) {
    UNUSED_VAR(irq_type);
    return GF_SUCCESS;
}

static gf_error_t gf_spi_switch_mode(void) {
    return GF_SUCCESS;
}

static gf_error_t gf_spi_aes_set(void) {
    return GF_SUCCESS;
}

static gf_error_t gf_spi_update_fdt_keybase(uint16_t *buf, uint32_t buf_len) {
    UNUSED_VAR(buf);
    UNUSED_VAR(buf_len);
    return GF_SUCCESS;
}

static gf_error_t gf_spi_get_orientation(uint16_t *orientation, uint16_t *facing) {
    UNUSED_VAR(orientation);
    UNUSED_VAR(facing);
    return GF_SUCCESS;
}

static gf_error_t gf_spi_get_gsc(uint8_t **buf, uint32_t *length) {
    UNUSED_VAR(buf);
    UNUSED_VAR(length);
    return GF_SUCCESS;
}

static gf_error_t gf_spi_get_hbd(uint8_t **buf, uint32_t *length) {
    UNUSED_VAR(buf);
    UNUSED_VAR(length);
    return GF_SUCCESS;
}

static gf_error_t gf_spi_get_hbd_base(uint16_t *base) {
    UNUSED_VAR(base);
    return GF_SUCCESS;
}

static gf_error_t gf_spi_test_download_fw_cfg(uint8_t *fw_cfg_data, uint32_t fw_cfg_data_len,  gf_download_fw_cfg_status_t fw_cfg_status) {
    UNUSED_VAR(fw_cfg_data);
    UNUSED_VAR(fw_cfg_data_len);
    UNUSED_VAR(fw_cfg_status);
    return GF_SUCCESS;
}

static gf_error_t gf_spi_detect_finger_up(void) {
    return GF_SUCCESS;
}

static gf_error_t gf_spi_get_fdt_base(uint16_t **buf, uint32_t *length) {
    UNUSED_VAR(buf);
    UNUSED_VAR(length);
    return GF_SUCCESS;
}

static gf_error_t gf_spi_get_fdt_threshold(uint16_t* threshold) {
    UNUSED_VAR(threshold);
    return GF_SUCCESS;
}

static gf_error_t gf_spi_update_fdt_down_up_base(void) {
    return GF_SUCCESS;
}

static gf_error_t gf_spi_check_sensor(void) {
    return GF_SUCCESS;
}

static gf_error_t gf_spi_set_data_base_status(uint8_t is_data_base_ok) {
    UNUSED_VAR(is_data_base_ok);
    return GF_SUCCESS;
}

static gf_error_t gf_spi_get_sensor_status(uint8_t *touch, uint8_t *temperature) {
    UNUSED_VAR(touch);
    UNUSED_VAR(temperature);
    return GF_SUCCESS;
}

static gf_error_t gf_spi_get_key_status(uint8_t *touch) {
    UNUSED_VAR(touch);
    return GF_SUCCESS;
}

static gf_error_t gf_spi_get_irq_status_for_spi_clk(uint32_t *irq_type, gf_mode_t current_mode) {
    UNUSED_VAR(irq_type);
    UNUSED_VAR(current_mode);
    return GF_SUCCESS;
}

static gf_error_t gf_spi_write_byte(uint16_t address, uint8_t value) {
    UNUSED_VAR(address);
    UNUSED_VAR(value);
    return GF_SUCCESS;
}

static gf_error_t gf_spi_read_byte(uint16_t address, uint8_t *value) {
    UNUSED_VAR(address);
    UNUSED_VAR(value);
    return GF_SUCCESS;
}

static gf_error_t gf_spi_set_ignore_irq_type(uint8_t irq_type_mask_offset) {
    UNUSED_VAR(irq_type_mask_offset);
    return GF_SUCCESS;
}

static gf_error_t gf_spi_update_dac_offset_by_mean(int32_t mean_value) {
    UNUSED_VAR(mean_value);
    return GF_SUCCESS;
}

static gf_error_t gf_spi_select_chip_otp(gf_detect_sensor_t* cmd) {
    UNUSED_VAR(cmd);
    return GF_SUCCESS;
}

static gf_error_t gf_spi_detect_sensor(void) {
    return GF_SUCCESS;
}

static gf_error_t gf_spi_judge_finger_stable(uint32_t sleep_time) {
    UNUSED_VAR(sleep_time);
    return GF_SUCCESS;
}

static gf_error_t gf_update_cfg_for_reissue_key_down(void) {
    return GF_SUCCESS;
}

static gf_error_t gf_enable_reissue_key_down(gf_mode_t mode) {
    UNUSED_VAR(mode);
    return GF_SUCCESS;
}

static gf_error_t gf_spi_update_temprature_dac(void) {
    return GF_SUCCESS;
}
static gf_error_t gf_set_config_for_stable_acquire_image(uint8_t flag) {
    UNUSED_VAR(flag);
    return GF_SUCCESS;
}

static gf_error_t gf_spi_print_pixel_key_value(void) {
    return GF_SUCCESS;
}

gf_error_t gf_spi_function_init(gf_spi_function_t *spi_function) {
    gf_error_t err = GF_SUCCESS;

    do {
        if (NULL == spi_function) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] invalid param", __func__);
            break;
        }

        spi_function->get_chip_id = gf_spi_get_chip_id;
        spi_function->get_fw_version = gf_spi_get_fw_version;
        spi_function->get_vendor_id = gf_spi_get_vendor_id;
        spi_function->get_production_date = gf_spi_get_production_date;
        spi_function->get_sensor_id = gf_spi_get_sensor_id;
        spi_function->get_sensor_otp_info = gf_spi_get_sensor_otp_info;
        spi_function->get_product_info = gf_spi_get_product_info;
        spi_function->write_product_info = gf_spi_write_product_info;

        spi_function->chip_init = gf_spi_chip_init;
        spi_function->erase_fw = gf_spi_erase_fw;
        spi_function->download_fw = gf_spi_download_fw;
        spi_function->download_cfg = gf_spi_download_cfg;
        spi_function->fw_cfg_sanity_check = gf_spi_fw_cfg_sanity_check;
        spi_function->chip_config_init = gf_spi_chip_config_init;
        spi_function->init_finished = gf_spi_init_finished;

        spi_function->get_image = gf_spi_get_image;
        spi_function->get_nav_image = gf_spi_get_nav_image;
        spi_function->get_nav_base = gf_spi_get_nav_base;
        spi_function->nav_complete = gf_spi_nav_complete;
        spi_function->esd_check = gf_spi_esd_check;

        spi_function->data_sample_suspend = gf_spi_data_sample_suspend;
        spi_function->data_sample_resume = gf_spi_data_sample_resume;

        spi_function->set_mode = gf_spi_set_mode;
        spi_function->get_mode = gf_spi_get_mode;

        spi_function->start_sensor_broken_check = gf_spi_start_sensor_broken_check;
        spi_function->stop_sensor_broken_check = gf_spi_stop_sensor_broken_check;
        spi_function->update_dac_for_sensor_broken_check = gf_spi_update_dac_for_sensor_broken_check;
        spi_function->update_dac_for_sensor_pixel_open_test = gf_spi_update_dac_for_sensor_pixel_open_test;

        spi_function->get_irq_type = gf_spi_get_irq_type;
        spi_function->clear_irq = gf_spi_clear_irq;

        spi_function->switch_mode = gf_spi_switch_mode;

        spi_function->aes_set = gf_spi_aes_set;

        spi_function->update_fdt_keybase = gf_spi_update_fdt_keybase;

        spi_function->get_orientation = gf_spi_get_orientation;

        spi_function->get_gsc = gf_spi_get_gsc;
        spi_function->get_hbd = gf_spi_get_hbd;
        spi_function->get_hbd_base = gf_spi_get_hbd_base;

        spi_function->test_download_fw_cfg = gf_spi_test_download_fw_cfg;

        spi_function->detect_finger_up = gf_spi_detect_finger_up;

        spi_function->get_fdt_base = gf_spi_get_fdt_base;
        spi_function->get_fdt_threshold = gf_spi_get_fdt_threshold;
        spi_function->update_fdt_down_up_base = gf_spi_update_fdt_down_up_base;
        spi_function->check_sensor = gf_spi_check_sensor;

        spi_function->set_data_base_status = gf_spi_set_data_base_status;
        spi_function->get_sensor_status = gf_spi_get_sensor_status;
        spi_function->get_key_status = gf_spi_get_key_status;
        spi_function->get_irq_status_for_spi_clk = gf_spi_get_irq_status_for_spi_clk;

        spi_function->spi_write_byte = gf_spi_write_byte;
        spi_function->spi_read_byte = gf_spi_read_byte;
        spi_function->set_ignore_irq_type = gf_spi_set_ignore_irq_type;

        spi_function->update_dac_offset_by_mean = gf_spi_update_dac_offset_by_mean;

        spi_function->select_chip_otp = gf_spi_select_chip_otp;
        spi_function->detect_sensor = gf_spi_detect_sensor;
        spi_function->judge_finger_stable = gf_spi_judge_finger_stable;
        spi_function->update_temprature_dac= gf_spi_update_temprature_dac;
        spi_function->set_config_for_stable_acquire_image = gf_set_config_for_stable_acquire_image;
        spi_function->update_cfg_for_reissue_key_down = gf_update_cfg_for_reissue_key_down;
        spi_function->enable_reissue_key_down = gf_enable_reissue_key_down;
        spi_function->print_pixel_key_value = gf_spi_print_pixel_key_value;
        /* customize spi functions */
        err = gf_spi_function_customize(spi_function);
        if (err != GF_SUCCESS) {
            GF_LOGE(LOG_TAG "[%s] gf_spi_function_customize() returns %d", __func__, err);
            break;
        }
    } while (0);

    return err;
}

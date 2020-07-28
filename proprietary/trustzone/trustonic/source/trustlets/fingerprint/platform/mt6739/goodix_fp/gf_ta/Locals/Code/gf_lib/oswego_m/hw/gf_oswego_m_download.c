/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */

#include "gf_error.h"
#include "gf_hw_common.h"
#include "gf_oswego_m_bus.h"
#include "gf_oswego_m_download.h"
#include "gf_fw_cfg.h"
#include "cpl_string.h"
#include "gf_tee_internal_api.h"
#include "cpl_memory.h"

#define GF_OSWEGO_M_CONFIG_VERSION_ADDR     (0x8047)
#define GF_OSWEGO_M_CFG_DOWNLOAD_FLAG_REG   (0x813F)

#define LOG_TAG "[gf_oswego_m_download]"

static gf_error_t gf_oswego_m_download_init_complete_check(void) {
    gf_error_t err = GF_SUCCESS;
    uint8_t reg_value = 0;
    uint32_t i = 0;
    FUNC_ENTER();
    for (; i < GF_SPI_COMPLETE_CHECK_RETRY_COUNT; i++) {
        gf_oswego_m_secspi_read_byte(0x5094, &reg_value);
        if (0xAA == reg_value) {
            break;
        }

        gf_sleep(10);
    }

    if (GF_SPI_COMPLETE_CHECK_RETRY_COUNT == i) {
        GF_LOGE(LOG_TAG "[%s] timeout", __func__);
        err = GF_ERROR_SPI_FW_DOWNLOAD_FAILED;
    }

    GF_LOGI(LOG_TAG "[%s] retry_count=%u, reg_value=0x%02X", __func__, i, reg_value);
    FUNC_EXIT(err);
    return err;
}

static gf_error_t gf_oswego_m_download_burn_complete_check(void) {
    gf_error_t err = GF_SUCCESS;
    uint8_t reg_value = 0;
    uint32_t i = 0;
    FUNC_ENTER();
    for (; i < GF_SPI_COMPLETE_CHECK_RETRY_COUNT; i++) {
        gf_oswego_m_secspi_read_byte(0x5094, &reg_value);
        if (0x00 == reg_value) {
            break;
        }

        gf_sleep(10);
    }
    if (GF_SPI_COMPLETE_CHECK_RETRY_COUNT == i) {
        GF_LOGE(LOG_TAG "[%s] timeout", __func__);
        err = GF_ERROR_SPI_FW_DOWNLOAD_FAILED;
    }

    GF_LOGI(LOG_TAG "[%s] retry_count=%u, reg_value=0x%02X", __func__, i, reg_value);
    FUNC_EXIT(err);
    return err;
}

static gf_error_t gf_oswego_m_download_9p_init(void) {
    gf_error_t err = GF_SUCCESS;
    uint8_t reg = 0;
    FUNC_ENTER();
    do {
        /* 7. */
        gf_oswego_m_secspi_write_byte(0x5081, 0x00);

        /* 8. Hold SS51 and DSP */
        gf_oswego_m_secspi_write_byte(0x4180, 0x0c);
        gf_oswego_m_secspi_read_byte(0x4180, &reg);
        if (reg != 0x0c) {
            err = GF_ERROR_SPI_FW_DOWNLOAD_FAILED;
            break;
        }

        /* 9. Enable DSP and MCU */
        gf_oswego_m_secspi_write_byte(0x4010, 0x00);
    } while (0);
    FUNC_EXIT(err);
    return err;
}

static gf_error_t gf_oswego_m_download_9p_check_version(void) {
    gf_error_t err = GF_SUCCESS;
    uint32_t i = 0;

    uint8_t version[4] = { 0 };
    FUNC_ENTER();
    for (; i < GF_SPI_COMPLETE_CHECK_RETRY_COUNT; i++) {
        gf_oswego_m_secspi_read_bytes(0x4220, version, 4);
        /* 9P MP chip version is 0x00900802*/
        if (0x00 == version[3] && 0x90 == version[2] && 0x08 == version[1]) {
            break;
        }

        gf_sleep(10);
    }

    if (GF_SPI_COMPLETE_CHECK_RETRY_COUNT == i) {
        GF_LOGE(LOG_TAG "[%s] timeout, 9p version=0x%02X, 0x%02X, 0x%02X, 0x%02X.",
                            __func__, version[3], version[2], version[1], version[0]);
        err = GF_ERROR_SPI_FW_DOWNLOAD_FAILED;
    }
    FUNC_EXIT(err);
    return err;
}

static gf_error_t gf_oswego_m_download_9p_dsp_isp(uint8_t *buf, uint16_t len) {
    gf_error_t err = GF_SUCCESS;
    uint8_t *tmp = buf;
    uint32_t retry_times = 0;
    uint32_t i = 0;
    uint8_t write_buf[256] = { 0 };
    uint8_t read_buf[256] = { 0 };
    FUNC_ENTER();
    do {
        /* 1. Close watch-dog,  clear cache enable(write 0 to 0x40B0/0x404B)*/
        gf_oswego_m_secspi_write_byte(0x40B0, 0x00);
        gf_oswego_m_secspi_write_byte(0x404B, 0x00);

        /* 2. Confg to boot from SRAM(write 0x2 to 0x4190)*/
        gf_oswego_m_secspi_write_byte(0x4190, 0x02);

        /* 3. software reset(write 0x1 to 0x4184)*/
        gf_oswego_m_secspi_write_byte(0x4184, 0x01);

        /* 4. Enable bank3(write 0x3 to 0x4048)*/
        gf_oswego_m_secspi_write_byte(0x4048, 0x03);

        /* 5. Enable accessing code(write 0x1 to 0x404c)*/
        gf_oswego_m_secspi_write_byte(0x404C, 0x01);

        /* 6. Burn the DSP ISP code to the area begin from 0xC000*/
        for (i = 0; i < len;) {
            cpl_memcpy(&write_buf, tmp, 256);
            gf_oswego_m_secspi_write_bytes(0xC000 + i, write_buf, 256);

            gf_oswego_m_secspi_read_bytes(0xC000 + i, read_buf, 256);
            if (0 == cpl_memcmp(tmp, read_buf, 256)) {
                i += 256;
                tmp += 256;
                retry_times = 0;
            } else {
                GF_LOGI(LOG_TAG "[%s] retry_times=%u", __func__, retry_times);
                retry_times++;
                if (retry_times > GF_SPI_COMPLETE_CHECK_RETRY_COUNT) {
                    err = GF_ERROR_SPI_FW_DOWNLOAD_FAILED;
                    break;
                }
            }
        }

        if (err != GF_SUCCESS) {
            break;
        }

        /* 7. Set scramble(write 0x0 to 0x4218)*/
        gf_oswego_m_secspi_write_byte(0x4218, 0x00);

        /* 8. Disable accessing code (writ 0 to 0x404C)*/
        gf_oswego_m_secspi_write_byte(0x404C, 0x00);
    } while (0);

    FUNC_EXIT(err);
    return err;
}

static gf_error_t gf_oswego_m_download_patch_block0(uint8_t *buf, uint16_t len) {
    gf_error_t err = GF_SUCCESS;
    uint8_t *tmp = buf;
    uint32_t retry_times = 0;
    uint32_t i = 0;
    uint8_t write_buf[256] = { 0 };
    uint8_t read_buf[256] = { 0 };
    uint8_t *rx_buf = NULL;
    FUNC_ENTER();

    do {
        /* 1. Hold SS51 and DSP(write 0x0C to 0x4180)*/
        gf_oswego_m_secspi_write_byte(0x4180, 0x0C);
        /* 2. Clean control flag(write 0x0 to 0x5094)*/
        gf_oswego_m_secspi_write_byte(0x5094, 0x00);
        /* 3. Enbale bank4(write 0x4 to 0x4048)*/
        gf_oswego_m_secspi_write_byte(0x4048, 0x04);
        /* 4. Enable accessing code(write 0x1 to 0x404d)*/
        gf_oswego_m_secspi_write_byte(0x404D, 0x01);

        /* 5. Hold SS51, release DSP(write 0x4 to 0x4180).
         * Waiting the initialization to complete(0x5094==0xAA)*/
        gf_oswego_m_secspi_write_byte(0x4180, 0x04);

        /* 6. Check 0X5094 0xAA*/
        err = gf_oswego_m_download_init_complete_check();
        if (err != GF_SUCCESS) {
            break;
        }

        /* 7. Burn the first 8KB of the firmware to the area begin from 0xC000.*/
        for (i = 0; i < len;) {
            cpl_memcpy(write_buf, tmp, 256);
            gf_oswego_m_secspi_write_bytes(0xC000 + i, write_buf, 256);

            gf_oswego_m_secspi_read_bytes(0xC000 + i, read_buf, 256);
            if (0 == cpl_memcmp(tmp, read_buf, 256)) {
                i += 256;
                tmp += 256;
                retry_times = 0;
            } else {
                GF_LOGI(LOG_TAG "[%s] retry_times=%u", __func__, retry_times);
                retry_times++;
                if (retry_times > GF_SPI_COMPLETE_CHECK_RETRY_COUNT) {
                    err = GF_ERROR_SPI_FW_DOWNLOAD_FAILED;
                    break;
                }
            }
        }
        if (err != GF_SUCCESS) {
            break;
        }

        /* 8. Send the command(move code[write 0x01 to 0x5094]). Waiting DSP move the code.*/
        gf_oswego_m_secspi_write_byte(0x5094, 0x01);
        err = gf_oswego_m_download_burn_complete_check();
        if (err != GF_SUCCESS) {
            break;
        }

        /* 9. Select bank4(write 0x4 to 0x4048)*/
        gf_oswego_m_secspi_write_byte(0x4048, 0x04);
        /* 10. Enable accessing code(write 0x1 to 0x404D)*/
        gf_oswego_m_secspi_write_byte(0x404D, 0x01);

        /* 11. Read 8KB data from 0xC000, check whether burn successfully.*/
        rx_buf = (uint8_t *) CPL_MEM_MALLOC(len);
        if (NULL == rx_buf) {
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        gf_oswego_m_secspi_read_bytes(0xC000, rx_buf, len);

        if (cpl_memcmp(buf, rx_buf, len)) {
            GF_LOGE(LOG_TAG "[%s] Data read back is not the same as the write one", __func__);
            err = GF_ERROR_SPI_FW_DOWNLOAD_FAILED;
            break;
        }

        /* 12. Disable accessing code(write 0x00 to 0x404D)*/
        gf_oswego_m_secspi_write_byte(0x404D, 0x00);
    } while (0);

    if (rx_buf != NULL) {
        CPL_MEM_FREE(rx_buf);
    }
    FUNC_EXIT(err);
    return err;
}

static gf_error_t gf_oswego_m_download_patch_block1(uint8_t *buf, uint16_t len) {
    gf_error_t err = GF_SUCCESS;
    uint8_t *tmp = buf;
    uint32_t retry_times = 0;
    uint8_t write_buf[256] = { 0 };
    uint8_t read_buf[256] = { 0 };
    uint8_t *rx_buf = NULL;
    uint32_t i = 0;
    FUNC_ENTER();
    do {
        /* 13. Hold SS51 and DSP(write 0x0C to 0x4180)*/
        gf_oswego_m_secspi_write_byte(0x4180, 0x0C);
        /* 14. Enbale bank5(write 0x5 to 0x4048)*/
        gf_oswego_m_secspi_write_byte(0x4048, 0x05);
        /* 15. Enable accessing code(write 0x1 to 0x404d)*/
        gf_oswego_m_secspi_write_byte(0x404E, 0x01);

        /* 16. Hold SS51, release DSP(write 0x4 to 0x4180).
         * Waiting the initialization to complete(0x5094==0xAA)*/
        gf_oswego_m_secspi_write_byte(0x4180, 0x04);
        /* 17. Check 0x5094*/
        err = gf_oswego_m_download_init_complete_check();
        if (err != GF_SUCCESS) {
            break;
        }

        /* 18. Burn the second 8KB of the firmware to the area begin from 0xC000.*/
        for (i = 0; i < len;) {
            cpl_memcpy(write_buf, tmp, 256);
            gf_oswego_m_secspi_write_bytes(0xC000 + i, write_buf, 256);

            gf_oswego_m_secspi_read_bytes(0xC000 + i, read_buf, 256);
            if (0 == cpl_memcmp(tmp, read_buf, 256)) {
                i += 256;
                tmp += 256;
                retry_times = 0;
            } else {
                GF_LOGI(LOG_TAG "[%s] retry_times=%u", __func__, retry_times);
                retry_times++;
                if (retry_times > GF_SPI_COMPLETE_CHECK_RETRY_COUNT) {
                    err = GF_ERROR_SPI_FW_DOWNLOAD_FAILED;
                    break;
                }
            }
        }
        if (err != GF_SUCCESS) {
            break;
        }

        /* 19. Send the command(move code[write 0x02 to 0x5094]).
         *  Waiting DSP move the code.
         *  */
        gf_oswego_m_secspi_write_byte(0x5094, 0x02);
        err = gf_oswego_m_download_burn_complete_check();
        if (err != GF_SUCCESS) {
            break;
        }

        /* 20. Select bank5(write 0x5 to 0x4048)*/
        gf_oswego_m_secspi_write_byte(0x4048, 0x05);
        /* 21. Enable accessing code(write 0x1 to 0x404E)*/
        gf_oswego_m_secspi_write_byte(0x404E, 0x01);

        /* 22. Read 8KB data from 0xC000, check whether burn successfully.*/
        rx_buf = (uint8_t *) CPL_MEM_MALLOC(len);
        if (NULL == rx_buf) {
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        gf_oswego_m_secspi_read_bytes(0xC000, rx_buf, len);
        if (cpl_memcmp(buf, rx_buf, len)) {
            GF_LOGE(LOG_TAG "[%s] Data read back is not the same as the write one", __func__);
            err = GF_ERROR_SPI_FW_DOWNLOAD_FAILED;
            break;
        }

        /* 23. Disable accessing code(write 0x00 to 0x404E)*/
        gf_oswego_m_secspi_write_byte(0x404E, 0x00);
    } while (0);

    if (rx_buf != NULL) {
        CPL_MEM_FREE(rx_buf);
    }
    FUNC_EXIT(err);
    return err;
}

static gf_error_t gf_oswego_m_download_9p_dsp_code(uint8_t *buf, uint16_t len) {
    gf_error_t err = GF_SUCCESS;
    uint8_t *tmp = buf;
    uint8_t *rx_buf = NULL;
    uint32_t retry_times = 0;
    uint8_t write_buf[256] = { 0 };
    uint8_t read_buf[256] = { 0 };
    uint32_t i = 0;
    FUNC_ENTER();
    do {
        /* 1. Select bank3(write 0x3 to 0x4048)*/
        gf_oswego_m_secspi_write_byte(0x4048, 0x03);
        /* 2. Hold SS51 and DSP(write 0x0C to 0x4180)*/
        gf_oswego_m_secspi_write_byte(0x4180, 0x0C);
        /* 3. Setting scramble(write 0x0 to 0x4218)*/
        gf_oswego_m_secspi_write_byte(0x4218, 0x00);

        /* 4. Hold SS51, release DSP(write 0x4 to 0x4180).
         * Waiting the initialization to complete(0x5094==0xAA)*/
        gf_oswego_m_secspi_write_byte(0x4180, 0x04);
        /* 5. Check 0x5094 */
        err = gf_oswego_m_download_init_complete_check();
        if (err != GF_SUCCESS) {
            break;
        }

        /* 6. Hold SS51, release DSP(write 0x4 to 0x4180).
         * Waiting the initialization to complete(0x5094==0xAA)*/

        for (i = 0; i < len;) {
            cpl_memcpy(write_buf, tmp, 256);
            gf_oswego_m_secspi_write_bytes(0x9000 + i, write_buf, 256);

            gf_oswego_m_secspi_read_bytes(0x9000 + i, read_buf, 256);
            if (0 == cpl_memcmp(tmp, read_buf, 256)) {
                i += 256;
                tmp += 256;
                retry_times = 0;
            } else {
                GF_LOGI(LOG_TAG "[%s] retry_times=%u", __func__, retry_times);
                retry_times++;
                if (retry_times > GF_SPI_COMPLETE_CHECK_RETRY_COUNT) {
                    err = GF_ERROR_SPI_FW_DOWNLOAD_FAILED;
                    break;
                }
            }
        }
        if (err != GF_SUCCESS) {
            break;
        }

        /* 7. Send the command(move code[write 0x05 to 0x5094]). Waiting DSP move the code.*/
        gf_oswego_m_secspi_write_byte(0x5094, 0x05);
        err = gf_oswego_m_download_burn_complete_check();
        if (err != GF_SUCCESS) {
            break;
        }

        /* 8. Read 4KB data from 0x9000 to do verification.*/
        rx_buf = (uint8_t *) CPL_MEM_MALLOC(len);
        if (NULL == rx_buf) {
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        gf_oswego_m_secspi_read_bytes(0x9000, rx_buf, len);
        if (cpl_memcmp(buf, rx_buf, len)) {
            GF_LOGE(LOG_TAG "[%s] Data read back is not the same as the write one", __func__);
            err = GF_ERROR_SPI_FW_DOWNLOAD_FAILED;
            break;
        }
    } while (0);

    if (rx_buf != NULL) {
        CPL_MEM_FREE(rx_buf);
    }
    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_oswego_m_download_fw_from_file(uint8_t *fw_data) {
    gf_error_t err = GF_SUCCESS;
    uint8_t *buf = fw_data + 14;
    FUNC_ENTER();
    do {
        if (NULL == fw_data) {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        GF_LOGI(LOG_TAG "[%s] check 9p version", __func__);
        /* step0: confirm 9P chip version*/
        err = gf_oswego_m_download_9p_check_version();
        if (err != GF_SUCCESS) {
            break;
        }

        GF_LOGI(LOG_TAG "[%s] download 9p init code", __func__);
        err = gf_oswego_m_download_9p_init();
        if (err != GF_SUCCESS) {
            break;
        }

        GF_LOGI(LOG_TAG "[%s] download dsp isp code", __func__);
        /*Step1: download dsp_isp. 4KB[38K~42K]*/
        err = gf_oswego_m_download_9p_dsp_isp(buf + 38 * 1024, 4096);
        if (err != GF_SUCCESS) {
            break;
        }

        GF_LOGI(LOG_TAG "[%s] download DSP CODE", __func__);
        /* Step2: download dsp code. 4KB[32K~36K] */
        err = gf_oswego_m_download_9p_dsp_code(buf + 32 * 1024, 4096);
        if (err != GF_SUCCESS) {
            break;
        }

        GF_LOGI(LOG_TAG "[%s] download patch code part0", __func__);
        /* Step3: download patch code part0. 8KB[0~8K] */
        err = gf_oswego_m_download_patch_block0(buf, 8192);
        if (err != GF_SUCCESS) {
            break;
        }

        GF_LOGI(LOG_TAG "[%s] download patch code part1", __func__);
        /* Step4: download patch code part1. 8KB[8~16K] */
        err = gf_oswego_m_download_patch_block1(buf + 8192, 8192);
    } while (0);

    FUNC_EXIT(err);
    return err;
}

static gf_error_t gf_oswego_m_download_cfg_complete_check(void) {
    gf_error_t err = GF_SUCCESS;
    uint8_t reg_value = 0;
    uint32_t i = 0;
    FUNC_ENTER();
    for (; i < GF_SPI_COMPLETE_CHECK_RETRY_COUNT; i++) {
        gf_oswego_m_secspi_read_byte(0x8011, &reg_value);
        if (0xAA == reg_value || 0x55 == reg_value) {
            break;
        }

        gf_sleep(10);
    }

    if (GF_SPI_COMPLETE_CHECK_RETRY_COUNT == i) {
        GF_LOGE(LOG_TAG "[%s] timeout", __func__);
        // err = GF_ERROR_SPI_CFG_DOWNLOAD_FAILED;
    }

    if (reg_value == 0x55) {
        GF_LOGI("[%s] finished. Same cfg, retry=%u, reg_value=0x%02X", __func__, i, reg_value);
    } else if (reg_value == 0xAA) {
        GF_LOGI("[%s] finished. Update to new cfg, retry=%u, reg_value=0x%02X",
                __func__, i, reg_value);
    }
    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_oswego_m_download_cfg_from_file(uint8_t *cfg_data, gf_config_type_t config_type) {
    gf_error_t err = GF_SUCCESS;
    uint8_t *tx_buf = NULL;
    FUNC_ENTER();
    do {
        uint8_t cfg_checksum = 0;
        /* reset cfg download finished flag */
        gf_oswego_m_secspi_write_byte(0x8011, 0);

        GF_LOGI(LOG_TAG "[%s] cfg version=0x%02X", __func__, cfg_data[0]);
        tx_buf = (uint8_t *) CPL_MEM_MALLOC(GF_OSWEGO_M_CFG_LENGTH);
        if (NULL == tx_buf) {
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }
        cpl_memcpy(tx_buf, cfg_data, GF_OSWEGO_M_CFG_LENGTH);

        cfg_checksum = 0 - (uint8_t) gf_oswego_m_get_checksum(tx_buf, GF_OSWEGO_M_CFG_LENGTH - 2);
        if (tx_buf[GF_OSWEGO_M_CFG_LENGTH - 2] != cfg_checksum) {
            GF_LOGI(LOG_TAG "[%s] cfg checksum error, in file=0x%02X, cal checksum=0x%02X",
                    __func__, tx_buf[GF_OSWEGO_M_CFG_LENGTH -2], cfg_checksum);
            tx_buf[GF_OSWEGO_M_CFG_LENGTH - 2] = cfg_checksum;
        }

        gf_oswego_m_secspi_write_bytes(GF_OSWEGO_M_CONFIG_VERSION_ADDR, tx_buf,
                GF_OSWEGO_M_CFG_LENGTH);

        if (config_type == CONFIG_NORMAL) {
            err = gf_oswego_m_download_cfg_complete_check();
        }

        if (err != GF_SUCCESS) {
            break;
        }
    } while (0);

    if (tx_buf != NULL) {
        CPL_MEM_FREE(tx_buf);
    }

    FUNC_EXIT(err);
    return err;
}

uint16_t gf_oswego_m_get_checksum(uint8_t *data, uint32_t len) {
    uint16_t checksum = 0;
    uint32_t i = 0;
    for (i = 0; i < len; i++) {
        checksum += data[i];
    }
    return checksum;
}

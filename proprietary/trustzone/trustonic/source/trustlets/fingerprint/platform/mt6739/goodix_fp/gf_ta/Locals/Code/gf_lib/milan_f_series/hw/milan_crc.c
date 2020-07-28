/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#include <string.h>

#include "gf_tee_internal_api.h"

#include "gf_common.h"
#include "gf_error.h"

#include "milan_crc.h"
#include "milan_regs.h"

#define LOG_TAG "[milan_crc]"

#define MILAN_F_IMAGE_CRC_LEN (108*88*3/2)
#define MILAN_F_NAV_CRC_LEN   (108*24*3/2)
#define CRC_32_POLYNOMIALS  0x04C11DB7

uint32_t g_crc32_from_host = 0xFFFFFFFF;  // CRC32 Value calculate by Host
uint32_t g_crc32_from_chip = 0;  // CRC32 Value general by Chip

static uint32_t DTable_CRC32[256] = { 0 };  // CRC32 Direct table

void milan_crc32_table_init(void) {
    uint32_t i32 = 0, j32 = 0;;
    VOID_FUNC_ENTER();

    for (i32 = 0; i32 < 256; i32++) {
        uint32_t nAccum32 = 0;
        uint32_t nData32 = (uint32_t) (i32 << 24);
        for (j32 = 0; j32 < 8; j32++) {
            if ((nData32 ^ nAccum32) & 0x80000000) {
                nAccum32 = (nAccum32 << 1) ^ CRC_32_POLYNOMIALS;
            } else {
                nAccum32 <<= 1;
            }
            nData32 <<= 1;
        }
        DTable_CRC32[i32] = nAccum32;
    }
    VOID_FUNC_EXIT();
}

static uint32_t milan_crc32_process(uint8_t *pchMsg, uint16_t wDataLen) {
    uint32_t dwCRC = 0xFFFFFFFF;
    VOID_FUNC_ENTER();

    while (wDataLen--) {
        uint8_t chChar = *pchMsg++;
        dwCRC = DTable_CRC32[((dwCRC >> 24) ^ chChar) & 0xff] ^ (dwCRC << 8);
    }

    GF_LOGD(LOG_TAG "[%s] dwCRC=0x%08X", __func__, dwCRC);
    VOID_FUNC_EXIT();
    return dwCRC;
}

// data_len: don't contain the check sum for crc32
gf_error_t milan_crc32_check(uint8_t *pchMsg, uint32_t data_len) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        g_crc32_from_host = milan_crc32_process(pchMsg, data_len);
        g_crc32_from_chip = (pchMsg[data_len + 2] << 24) + (pchMsg[data_len + 3] << 16)
                + (pchMsg[data_len] << 8) + (pchMsg[data_len + 1]);

        if (g_crc32_from_chip == g_crc32_from_host) {
            GF_LOGD(LOG_TAG "[%s] crc check success", __func__);
            err = GF_SUCCESS;
        } else {
            GF_LOGE(LOG_TAG "[%s] crc check failed, g_crc32_from_host=0x%08X, "
                    "g_crc32_from_chip=0x%08X", __func__, g_crc32_from_host, g_crc32_from_chip);
            err = GF_ERROR_SPI_RAW_DATA_CRC_FAILED;
        }
    } while (0);
    FUNC_EXIT(err);
    return err;
}

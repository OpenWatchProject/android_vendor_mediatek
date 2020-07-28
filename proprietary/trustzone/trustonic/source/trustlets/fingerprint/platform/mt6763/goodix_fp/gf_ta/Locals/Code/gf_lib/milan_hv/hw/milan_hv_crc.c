/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#include "milan_hv_crc.h"
#include "gf_log.h"

#define LOG_TAG "[milan_hv_crc]"

#define CRC_32_POLYNOMIALS  0x04C11DB7

static uint32_t g_crc32_table[256];  // CRC32 table

void gf_milan_hv_crc32_init_table(void) {
    uint32_t i, j;
    VOID_FUNC_ENTER();

    for (i = 0; i < 256; i++) {
        uint32_t data = (uint32_t) (i << 24);
        uint32_t accum = 0;
        for (j = 0; j < 8; j++) {
            if ((data ^ accum) & 0x80000000) {
                accum = (accum << 1) ^ CRC_32_POLYNOMIALS;
            } else {
                accum <<= 1;
            }
            data <<= 1;
        }
        g_crc32_table[i] = accum;
    }

    VOID_FUNC_EXIT();
}

uint32_t gf_milan_hv_get_crc32(uint8_t *ptr, uint16_t len) {
    uint32_t crc = 0xFFFFFFFF;

    while (len--) {
        uint8_t temp_data = *ptr++;
        crc = g_crc32_table[((crc >> 24) ^ temp_data) & 0xff] ^ (crc << 8);
    }

    GF_LOGD(LOG_TAG "[%s] crc32=0x%08X", __func__, crc);
    return crc;
}

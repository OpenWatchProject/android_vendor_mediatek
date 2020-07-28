/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */

#include "gf_tee_internal_api.h"
#include "gf_hw_common.h"
#include "cpl_memory.h"

#define LOG_TAG "[gf_hw_common]"

void gf_hw_endian_exchange(uint8_t* buf, int32_t len) {
    int32_t i = 0;
    int32_t size = len & 0xFFFFFFFE;

    for (i = 0; i < size; i += 2) {
        uint8_t buf_tmp = buf[i + 1];
        buf[i + 1] = buf[i];
        buf[i] = buf_tmp;
    }
}

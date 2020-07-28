/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */

#ifndef __MILAN_CRC_H__
#define __MILAN_CRC_H__

#include "milan_common.h"

extern uint32_t g_crc32_from_host;  // CRC32 Value calculate by Host
extern uint32_t g_crc32_from_chip;  // CRC32 Value general by Chip

#ifdef __cplusplus
extern "C" {
#endif

void milan_crc32_table_init(void);
gf_error_t milan_crc32_check(uint8_t *pchMsg, uint32_t data_len);

#ifdef __cplusplus
}
#endif

#endif  // __MILAN_CRC_H__

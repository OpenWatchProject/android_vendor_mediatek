/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#ifndef __MILAN_HV_CRC_H__
#define __MILAN_HV_CRC_H__

#include "milan_hv_common.h"

#ifdef __cplusplus
extern "C" {
#endif

void gf_milan_hv_crc32_init_table(void);
uint32_t gf_milan_hv_get_crc32(uint8_t *ptr, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif  // __MILAN_HV_CRC_H__

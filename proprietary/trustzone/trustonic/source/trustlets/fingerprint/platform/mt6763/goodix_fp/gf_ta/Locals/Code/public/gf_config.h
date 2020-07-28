/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#ifndef __GF_CONFIG_H__
#define __GF_CONFIG_H__

#include "gf_type_define.h"

#ifdef __cplusplus
extern "C" {
#endif

extern gf_config_t g_config;

void gf_config_init(void);
void gf_config_init_by_chip_type(gf_chip_type_t chip_type);

#ifdef __cplusplus
}
#endif

#endif  // __GF_CONFIG_H__

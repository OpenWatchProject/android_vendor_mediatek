/*
* Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
* All Rights Reserved.
*/

#ifndef __GF_ALGO_CRC32_H__
#define __GF_ALGO_CRC32_H__

#include <cpl_stdio.h>

typedef struct {
    uint32_t crc;
} gf_crc32_context;

#ifdef __cplusplus
extern "C" {
#endif

void gf_algo_crc32_init(gf_crc32_context *ctx);

void gf_algo_crc32_update(gf_crc32_context *ctx, const unsigned char *data, int32_t len);

void gf_algo_crc32_final(gf_crc32_context *ctx, unsigned char *md);

#ifdef __cplusplus
}
#endif

#endif //__GF_ALGO_CRC32_H__

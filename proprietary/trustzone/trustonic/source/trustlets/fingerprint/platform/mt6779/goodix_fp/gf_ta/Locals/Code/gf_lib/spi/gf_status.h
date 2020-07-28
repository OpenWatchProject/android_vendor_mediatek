/*
* Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
* All Rights Reserved.
*/

#ifndef __GF_STATUS_H
#define __GF_STATUS_H

#include <stdint.h>

typedef struct _gf_algo_cfg_t {
    uint8_t isEnableEncrypt; // 0: disable firmware data encrypt 1: enable
    uint8_t isFloating;
    uint8_t isPixelCancel;
    uint8_t isCoating;
    uint32_t selectBmpThreshold;
} gf_algo_cfg_t;

typedef enum _gf_decode_ret_t {
    GF_DECODE_SUCESS, //
    GF_DECODE_ERROR_CFG_LEN
} gf_decode_ret_t;

//success return 0, otherwise return error code
gf_decode_ret_t gf_decodeAlgoCfgFromFwCfg(uint8_t *cfg, uint32_t cfg_len, gf_algo_cfg_t *algoCfg);

#endif

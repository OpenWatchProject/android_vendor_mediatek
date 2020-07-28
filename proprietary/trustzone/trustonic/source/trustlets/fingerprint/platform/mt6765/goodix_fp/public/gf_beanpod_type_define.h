/*
* Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
* All Rights Reserved.
*/

#ifndef __GF_BEANPOD_TYPE_DEFINE_H__
#define __GF_BEANPOD_TYPE_DEFINE_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// for beanpod tee
typedef struct {
    uint32_t head1;
    uint32_t head2;
    uint32_t head3;
    uint32_t len;
} gf_beanpod_header_t;

typedef struct {
    uint32_t cmd_id;
    uint32_t reverse[3];
} gf_beanpod_cmd_id_t;

#ifdef __cplusplus
}
#endif

#endif // __GF_TYPE_BEANPOD_DEFINE_H__

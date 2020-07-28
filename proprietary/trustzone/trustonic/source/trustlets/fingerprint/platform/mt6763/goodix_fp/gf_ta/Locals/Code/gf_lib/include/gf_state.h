/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#ifndef __GF_STATE_H__
#define __GF_STATE_H__

#include "gf_common.h"
#include "gf_error.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t priority;
    gf_mode_t mode;
} gf_operation_info_t;

/**
 * [gf_get_mode description]
 * @param  operation [description]
 * @return           [description]
 */
gf_mode_t gf_state_get_mode(gf_operation_type_t operation);

/**
 * [gf_state_is_authenticate description]
 * @param  operation [description]
 * @return           [description]
 */
uint32_t gf_state_is_authenticate(gf_operation_type_t operation);

/**
 * [gf_state_is_test_operation description]
 * @param  operation [description]
 * @return           [description]
 */
uint32_t gf_state_is_test_operation(gf_operation_type_t operation);

/**
 * [gf_state_is_base_operation description]
 * @param  operation [description]
 * @return           [description]
 */
uint32_t gf_state_is_base_operation(gf_operation_type_t operation);

/**
 * [gf_state_get_priority_of_operation description]
 * @param  operation [description]
 * @return           [description]
 */
uint32_t gf_state_get_priority_of_operation(gf_operation_type_t operation);

/**
 * [gf_state_set_mode description]
 * @param  operation  [description]
 * @param  reset_flag [description]
 * @return            [description]
 */
void gf_state_set_mode(gf_operation_type_t operation, uint8_t *reset_flag);
#ifdef __cplusplus
}
#endif

#endif  // __GF_STATE_H__

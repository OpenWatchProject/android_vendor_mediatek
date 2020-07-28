/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#ifndef __GF_STATE_COMMON_H__
#define __GF_STATE_COMMON_H__

#include <stdint.h>
#include "gf_common.h"
#include "gf_error.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * [dump the all operations in the stack ]
 */
void gf_state_dump(void);

/**
 *gf_state_init_global_variable Description: initialize the global variable
 */
void gf_state_init_global_variable(void);

/**
 * [gf_state_get_next_operation : get an operation from g_operation_array use to execute!]
 * @return  [the operation will be execute ]
 */
gf_operation_type_t gf_state_get_next_operation(void);

/**
 * [gf_state_get_default_operation]
 * @return  [the operation that define in config]
 */
gf_operation_type_t gf_state_get_default_operation(void);

/**
 *  @return  [the operation that be executed currently]
 */
gf_operation_type_t gf_state_get_current_operation(void);

/**
 *  @return  [the mode that be executed currently]
 */
gf_mode_t gf_state_get_current_mode(void);

/**
 * [if operation need pairs ,when down irq happens ,call this function]
 */
void gf_state_require_down_and_up_in_pairs_begin(void);

/**
 * [if operation need pairs ,when up irq happens ,call this function,means,pairs successful]
 */
void gf_state_require_down_and_up_in_pairs_end(uint8_t *reset_flag);

/**
 * [if current operation (update base or need pairs)can not be interrupted,when the operation
 * down ,call this function to get operation from operation array and execute it]
 * case 1: base update completed
 * case 2: down_and_up pairs completed
 * @param  reset_flag [1:need reset the chip,0:don't need resest the chip]
 * @return            [spi_set_mode error]
 */
gf_error_t gf_state_operation_done(uint8_t *reset_flag);

/**
 * [gf_state_cancel_operation]
 * @param  reset_flag [1:need reset the chip,0:don't need resest the chip]
 * @return            [spi_set_mode error]
 */

gf_error_t gf_state_cancel_operation(uint8_t *reset_flag);

/**
 * [gf_state_cancel_prior_test_operation]
 * @param  reset_flag [1:need reset the chip,0:don't need resest the chip]
 * @return            [spi_set_mode error]
 */
gf_error_t gf_state_cancel_prior_test_operation(uint8_t *reset_flag);

/**
 * [gf_state_cancel_test_operation]
 * @param  reset_flag [1:need reset the chip,0:don't need resest the chip]
 * @return            [spi_set_mode error]
 */
gf_error_t gf_state_cancel_test_operation(uint8_t *reset_flag);

/**
 * @param  dest  [the dest need copy to]
 * @param  size  [the size need copy]
 * @return            [spi_set_mode error]
 */
gf_error_t gf_state_get_operation_array(void *dest, unsigned int size);

/**
 * @param  operation  [the operation that will be executed]
 * @param  reset_flag [1:need reset the chip,0:don't need resest the chip]
 * @return            [spi_set_mode error]
 */
gf_error_t gf_state_execute_operation(gf_operation_type_t operation, uint8_t *reset_flag);

#ifdef __cplusplus
}
#endif

#endif  // __GF_STATE_COMMON_H__

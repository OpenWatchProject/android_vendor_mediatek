/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#include "gf_state_common.h"
#include "gf_common.h"
#include "gf_config.h"
#include "gf_log.h"
#include "cpl_string.h"
#include "gf_hw_common.h"
#include "gf_tee_internal_api.h"
#include "gf_state.h"

#define LOG_TAG "[gf_state_common]"

/**/
gf_operation_type_t g_operation_array[MAX_OPERATION_ARRAY_SIZE] = {
OPERATION_INVAILD, /*0*/
OPERATION_INVAILD, /*1*/
OPERATION_INVAILD, /*2*/
OPERATION_INVAILD, /*3*/
OPERATION_INVAILD, /*4*/
OPERATION_INVAILD, /*5*/
OPERATION_INVAILD, /*6*/
OPERATION_INVAILD, /*7*/
OPERATION_INVAILD, /*8*/
OPERATION_INVAILD  /*9*/
};
/**/
static gf_operation_type_t g_current_operation = OPERATION_INVAILD;

/*
 *value 0: don't need pairs or pairs end
 *      1: begin pairs and wait pairs end!
 */
static uint32_t g_down_and_up_pairs_flag = 0;

/*the threshold value of update base time*/
#define BASE_FRAME_TIMEOUT_MS 10000000  // 10 seconds

/*the tag of update base begin*/
static uint64_t g_update_base_start_time = 0;

extern uint8_t g_last_authenticate_result;

extern void gf_ta_cancel_operation(void);

static void state_save_operation(gf_operation_type_t operation);

static gf_error_t state_set_current_operation(gf_operation_type_t operation, uint8_t *reset_flag);

static gf_operation_type_t state_pick_next_operation(void);

/**
 *gf_state_init_global_variable Description:initialize the global variable
 */
void gf_state_init_global_variable(void) {
    int32_t i = 0;
    VOID_FUNC_ENTER();

    for (; i < MAX_OPERATION_ARRAY_SIZE; i++) {
        g_operation_array[i] = OPERATION_INVAILD;
    }

    g_current_operation = OPERATION_INVAILD;
    g_down_and_up_pairs_flag = 0;
    g_update_base_start_time = 0;

    VOID_FUNC_EXIT();
}

/**
 * [state_save_operation description :the priority of operation equals g_operation_array's index ]
 * @param operation :the operation that will be stored in g_operation_array
 * @return
 */
static void state_save_operation(gf_operation_type_t operation) {
    int32_t priority = PRIORITY_UNDEFINE;

    VOID_FUNC_ENTER();

    do {
        priority = gf_state_get_priority_of_operation(operation);

        if (PRIORITY_UNDEFINE == priority) {
            break;
        }

        g_operation_array[priority] = operation;
    } while (0);

    VOID_FUNC_EXIT();
}

/**
 * [state_set_current_operation]
 * @param  operation  [the operation that will be executed]
 * @param  reset_flag [1:need reset the chip,0:don't need resest the chip]
 * @return
 */
static gf_error_t state_set_current_operation(gf_operation_type_t operation, uint8_t *reset_flag) {
    gf_error_t err = GF_SUCCESS;
    gf_operation_type_t cur_operation = OPERATION_NONE;
    gf_mode_t cur_mode = MODE_NONE;
    gf_mode_t tmp_mode = MODE_NONE;

    FUNC_ENTER();

    do {
        tmp_mode = gf_state_get_mode(operation);
        if (tmp_mode == MODE_NONE) {
            break;
        }

        cur_operation = gf_state_get_current_operation();
        cur_mode = gf_state_get_mode(cur_operation);

        // if the mode that will be switched to equals current mode ,we don't need switch mode
        if (tmp_mode != cur_mode) {
            gf_state_set_mode(operation, reset_flag);

            // if the operation is update base ,we should record the begin time ,and if update time
            // exceeds the defined threshold value,stop it.
            if (gf_state_is_base_operation(operation) > 0) {
                gf_get_timestamp(&g_update_base_start_time);
                gf_hw_data_sample_resume();

            } else if (MODE_DEBUG == tmp_mode) {
                gf_hw_data_sample_resume();
            }
        }
    } while (0);

    g_current_operation = operation;
    if (gf_state_is_authenticate(g_current_operation) > 0) {
        g_last_authenticate_result = 0;
    }

    FUNC_EXIT(err);
    return err;
}


/**
 * [state_pick_next_operation : get a operation from g_operation_array use to execute!]
 * @return  [the operation will be execute ]
 */
static gf_operation_type_t state_pick_next_operation(void) {
    int32_t priority = MAX_OPERATION_ARRAY_SIZE - 1;
    gf_operation_type_t operation = OPERATION_INVAILD;

    VOID_FUNC_ENTER();

    do {
        operation = g_operation_array[priority];

        if (OPERATION_INVAILD != operation) {
            g_operation_array[priority] = OPERATION_INVAILD;
            break;
        }

        priority--;
    } while (priority >= 0);

    // if g_operation_array has no operation,execute default operation
    if (OPERATION_INVAILD == operation) {
        GF_LOGD(LOG_TAG "[%s] g_operation_array is empty,execute default operation!", __func__);
        operation = gf_state_get_default_operation();
    }

    VOID_FUNC_EXIT();
    return operation;
}

/**
 * [dump the all operations in the operation array ]
 */
void gf_state_dump(void) {
    int32_t index = 0;

    VOID_FUNC_ENTER();

    while (index < MAX_OPERATION_ARRAY_SIZE) {
        if (OPERATION_INVAILD != g_operation_array[index]) {
            GF_LOGD(LOG_TAG "[%s] operation=%s, priority=%d, mode=%s", __func__,
                    gf_stroperation(g_operation_array[index]), index,
                    gf_strmode(gf_state_get_mode(g_operation_array[index])));
        }
        index++;
    }

    VOID_FUNC_EXIT();
}

/**
 * [gf_state_get_next_operation : get a operation from g_operation_array use to execute!]
 * @return  [the operation will be execute ]
 */
gf_operation_type_t gf_state_get_next_operation(void) {
    int32_t priority = MAX_OPERATION_ARRAY_SIZE - 1;
    gf_operation_type_t operation = OPERATION_INVAILD;

    VOID_FUNC_ENTER();

    do {
        operation = g_operation_array[priority];

        if (OPERATION_INVAILD != operation) {
            break;
        }

        priority--;
    } while (priority >= 0);

    // if g_operation_array has no operation,execute default operation
    if (OPERATION_INVAILD == operation) {
        GF_LOGD(LOG_TAG "[%s] g_operation_array is empty,execute default operation!", __func__);
        operation = gf_state_get_default_operation();
    }

    VOID_FUNC_EXIT();
    return operation;
}

/**
 * [gf_state_get_default_operation]
 * @return  [the operation that define in config]
 */
gf_operation_type_t gf_state_get_default_operation(void) {
    gf_operation_type_t operation = OPERATION_NONE;
    VOID_FUNC_ENTER();

    if (g_config.support_nav_mode > GF_NAV_MODE_NONE) {
        operation = OPERATION_NAV;
    } else if (g_config.support_key_mode > 0) {
        operation = OPERATION_HOME_KEY;
    }

    VOID_FUNC_EXIT();
    return operation;
}

/**
 *  @return  [the operation that be executed currently]
 */
gf_operation_type_t gf_state_get_current_operation(void) {
    VOID_FUNC_ENTER();

    GF_LOGD(LOG_TAG "[%s] g_current_operation=%s", __func__, gf_stroperation(g_current_operation));

    VOID_FUNC_EXIT();

    return g_current_operation;
}

/**
 *  @return  [the mode that be executed currently]
 */
gf_mode_t gf_state_get_current_mode(void) {
    gf_mode_t current_mode = MODE_MAX;
    VOID_FUNC_ENTER();

    current_mode = gf_state_get_mode(gf_state_get_current_operation());

    VOID_FUNC_EXIT();

    return current_mode;
}

/**
 * [if operation need pairs ,when down irq happens ,call this function]
 */
void gf_state_require_down_and_up_in_pairs_begin(void) {
    VOID_FUNC_ENTER();

    g_down_and_up_pairs_flag = 1;

    state_save_operation(gf_state_get_current_operation());

    VOID_FUNC_EXIT();
}

/**
 * [if operation need pairs ,when up irq happens ,call this function,means,pairs successful]
 */
void gf_state_require_down_and_up_in_pairs_end(uint8_t *reset_flag) {
    VOID_FUNC_ENTER();
    if (g_down_and_up_pairs_flag > 0) {
        gf_state_operation_done(reset_flag);
    }

    VOID_FUNC_EXIT();
}

/**
 * [if current operation (update base or need pairs)can not be interrupted,when the operation
 * down ,call this function to get operation from operation array and execute it]
 * case 1: base update completed
 * case 2: down_and_up pairs completed
 * @param  reset_flag [1:need reset the chip,0:don't need resest the chip]
 * @return            [spi_set_mode error]
 */
gf_error_t gf_state_operation_done(uint8_t *reset_flag) {
    gf_operation_type_t operation = OPERATION_NONE;
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    g_down_and_up_pairs_flag = 0;
    operation = state_pick_next_operation();

    GF_LOGD(LOG_TAG "[%s] next operation=%s", __func__, gf_stroperation(operation));

    err = state_set_current_operation(operation, reset_flag);

    FUNC_EXIT(err);

    return err;
}

/**
 * [gf_state_cancel_operation]
 * @param  reset_flag [1:need reset the chip,0:don't need resest the chip]
 * @return            [spi_set_mode error]
 */

gf_error_t gf_state_cancel_operation(uint8_t *reset_flag) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do {
        if (g_down_and_up_pairs_flag > 0) {
            g_operation_array[PRIORITY_AUTHENTICATE] = OPERATION_INVAILD;
            GF_LOGD(LOG_TAG "[%s] can't cancel current operation, down an up not in pairs!",
                    __func__);
            break;
        }

        if (gf_state_get_priority_of_operation(gf_state_get_current_operation())
                != PRIORITY_AUTHENTICATE) {
            g_operation_array[PRIORITY_AUTHENTICATE] = OPERATION_INVAILD;
        } else {
            gf_ta_cancel_operation();
            err = gf_state_operation_done(reset_flag);
        }
    } while (0);

    FUNC_EXIT(err);
    return err;
}

/**
 * [gf_state_cancel_prior_test_operation]
 * @param  reset_flag [1:need reset the chip,0:don't need resest the chip]
 * @return            [spi_set_mode error]
 */
gf_error_t gf_state_cancel_prior_test_operation(uint8_t *reset_flag) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    if (gf_state_get_priority_of_operation(g_current_operation) != PRIORITY_PRIOR_TEST) {
        g_operation_array[PRIORITY_PRIOR_TEST] = OPERATION_INVAILD;
    } else {
        gf_ta_cancel_operation();
        err = gf_state_operation_done(reset_flag);
    }

    FUNC_EXIT(err);
    return err;
}

/**
 * [gf_state_cancel_test_operation]
 * @param  reset_flag [1:need reset the chip,0:don't need resest the chip]
 * @return            [spi_set_mode error]
 */
gf_error_t gf_state_cancel_test_operation(uint8_t *reset_flag) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do {
        if (g_down_and_up_pairs_flag > 0) {
            g_operation_array[PRIORITY_TEST] = OPERATION_INVAILD;
            GF_LOGD(LOG_TAG "[%s] can't cancel current operation, down an up not in pairs!",
                    __func__);
            break;
        }

        if (gf_state_get_priority_of_operation(g_current_operation) != PRIORITY_TEST) {
            g_operation_array[PRIORITY_TEST] = OPERATION_INVAILD;
        } else {
            gf_ta_cancel_operation();
            err = gf_state_operation_done(reset_flag);
        }
    } while (0);

    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_state_get_operation_array(void *dest, unsigned int size) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do {
        if (dest == NULL || size == 0) {
            GF_LOGE(LOG_TAG "[%s] invalid parameter", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }
        if (size >= sizeof(g_operation_array)) {
            cpl_memcpy(dest, g_operation_array, sizeof(g_operation_array));
        } else {
            cpl_memcpy(dest, g_operation_array, size);
        }
    } while (0);

    FUNC_EXIT(err);
    return err;
}

/**
 * @param  operation  [the operation that will be executed]
 * @param  reset_flag [1:need reset the chip,0:don't need resest the chip]
 * @return            [spi_set_mode error]
 */
gf_error_t gf_state_execute_operation(gf_operation_type_t operation, uint8_t *reset_flag) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do {
        gf_operation_type_t cur_operation;
        int32_t cmp_result = 0;
        // when execute the operation ,we should check the down and
        // up pairs,if now has not pairs ,save it to array.
        if ((g_down_and_up_pairs_flag > 0) &&
                (OPERATION_BROKEN_CHECK_DEFAULT != operation) &&
                (OPERATION_BROKEN_CHECK_NEGATIVE != operation) &&
                (OPERATION_BROKEN_CHECK_POSITIVE != operation)) {
            // flag >0 means finger not up ,this time can not excute update base!
            if (gf_state_is_base_operation(operation) > 0) {
                gf_state_require_down_and_up_in_pairs_end(reset_flag);
            } else {
                state_save_operation(operation);
                // if it meets test operation, down and up in pairs end,
                // response test operation immediately.
                if (gf_state_is_test_operation(operation) > 0) {
                    gf_state_require_down_and_up_in_pairs_end(reset_flag);
                }
                break;
            }
        }

        cur_operation = gf_state_get_current_operation();

        if (OPERATION_INVAILD == cur_operation) {
            err = state_set_current_operation(operation, reset_flag);
            break;
        }

        //  <0: the priority of left_operation lower than right_operation
        // ==0: the priority of left_operation equals right_operation
        //  >0: the priority of left_operation higher than right_operation
        cmp_result = gf_state_get_priority_of_operation(cur_operation)
                       - gf_state_get_priority_of_operation(operation);

        if (cmp_result < 0) {
            GF_LOGD(LOG_TAG "[%s] save operation=%s ", __func__, gf_stroperation(cur_operation));
            state_save_operation(cur_operation);
            err = state_set_current_operation(operation, reset_flag);
        } else if (0 == cmp_result) {
            err = state_set_current_operation(operation, reset_flag);
        } else {
            if (gf_state_is_base_operation(cur_operation) > 0) {
                uint64_t current_time = 0;
                if (gf_state_is_test_operation(operation) > 0) {
                    err = state_set_current_operation(operation, reset_flag);
                    break;
                }

                gf_get_timestamp(&current_time);

                if (current_time - g_update_base_start_time > BASE_FRAME_TIMEOUT_MS) {
                    err = state_set_current_operation(operation, reset_flag);
                    break;
                }
            }

            GF_LOGD(LOG_TAG "[%s] save operation=%s ", __func__, gf_stroperation(operation));
            state_save_operation(operation);
        }
    } while (0);

    GF_LOGD(LOG_TAG "[%s] g_current_operation=%s",
            __func__, gf_stroperation(gf_state_get_current_operation()));
    FUNC_EXIT(err);

    return err;
}

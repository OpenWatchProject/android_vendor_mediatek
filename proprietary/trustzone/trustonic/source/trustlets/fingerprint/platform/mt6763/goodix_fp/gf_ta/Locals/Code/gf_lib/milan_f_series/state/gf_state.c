/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#include "gf_state.h"
#include <stdint.h>
#include "gf_common.h"
#include "gf_error.h"
#include "gf_tee_internal_api.h"
#include "gf_log.h"
#include "gf_hw_common.h"

#define LOG_TAG "[gf_state]"

/**
 **the priority of operation equal array index       **
 **<----  |__0__| | (test op)    ----> **
 **<----  |__1__| | (google op)  ----> **
 **<----  |__2__| | (special op) ----> **
 **<----  |__3__| V (update base)----> **
 */

const gf_operation_info_t g_operation_info[] = {
{ 1, MODE_IMAGE },                      /* OPERATION_ENROLL = 0*/
{ 1, MODE_IMAGE },                      /* OPERATION_AUTHENTICATE_IMAGE 1*/
{ 1, MODE_FF },                         /* OPERATION_AUTHENTICATE_FF, 2*/
{ 1, MODE_SLEEP },                      /* OPERATION_AUTHENTICATE_SLEEP, 3*/
{ 0, MODE_IMAGE },                      /* OPERATION_AUTHENTICATE_FIDO, 4*/
{ 3, MODE_FINGER_BASE },                /* OPERATION_FINGER_BASE, 5*/
{ 0, MODE_NAV },                        /* OPERATION_NAV, 6*/
{ 3, MODE_NAV_BASE },                   /* OPERATION_NAV_BASE, 7*/
{ 3, MODE_FINGER_BASE },                /* OPERATION_CHECK_FINGER_LONG_PRESS, 8*/
{ 0, MODE_KEY },                        /* OPERATION_HOME_KEY, 9*/
{ 0, MODE_FF },                         /* OPERATION_POWER_KEY, 10*/
{ 0, MODE_KEY },                        /* OPERATION_CAMERA_KEY, 11*/
{ 0, MODE_HBD },                        /* OPERATION_HEARTBEAT_KEY, 12*/
{ 0, MODE_IMAGE },                      /* OPERATION_TEST_IMAGE_MODE, 13*/
{ 0, MODE_DEBUG },                      /* OPERATION_TEST_DEBUG_MODE, 14*/
{ 0, MODE_FF },                         /* OPERATION_TEST_FF_MODE, 15*/
{ 0, MODE_KEY },                        /* OPERATION_TEST_KEY_MODE, 16*/
{ 0, MODE_NAV },                        /* OPERATION_TEST_NAV_MODE, 17*/
{ 0, MODE_NAV_BASE },                   /* OPERATION_TEST_NAV_BASE_MODE, 18*/
{ 0, MODE_FINGER_BASE },                /* OPERATION_TEST_FINGER_BASE_MODE, 19*/
{ 0, MODE_IDLE },                       /* OPERATION_TEST_IDLE_MODE, 20*/
{ 0, MODE_SLEEP },                      /* OPERATION_TEST_SLEEP_MODE, 21*/
{ 0, MODE_HBD_DEBUG },                  /* OPERATION_TEST_HBD_DEBUG_MODE, 22*/
{ 0, MODE_HBD },                        /* OPERATION_TEST_HBD_MODE, 23*/
{ 0, MODE_DEBUG },                      /* OPERATION_TEST_PIXEL_OPEN_STEP1, 24*/
{ 0, MODE_DEBUG },                      /* OPERATION_TEST_PIXEL_OPEN_STEP2, 25*/
{ 0, MODE_TEST_BAD_POINT_FINGER_BASE }, /* OPERATION_TEST_BAD_POINT_RECODE_BASE, 26*/
{ 0, MODE_IMAGE },                      /* OPERATION_TEST_BAD_POINT, 27*/
{ 0, MODE_IMAGE },                      /* OPERATION_TEST_PERFORMANCE, 28*/
{ 0, MODE_FINGER_BASE },                /* OPERATION_TEST_SPI_PERFORMANCE, 29*/
{ 0, MODE_DEBUG },                      /* OPERATION_TEST_SPI_TRANSFER, 30*/
{ 0, MODE_FINGER_BASE },                /* OPERATION_TEST_FRR_FAR_RECORD_BASE_FRAME, 31*/
{ 0, MODE_IMAGE },                      /* OPERATION_TEST_FRR_FAR_RECORD_ENROLL, 32*/
{ 0, MODE_IMAGE },                      /* OPERATION_TEST_FRR_FAR_RECORD_AUTHENTICATE, 33*/
{ 0, MODE_IMAGE },                      /* OPERATION_TEST_UNTRUSTED_ENROLL, 34*/
{ 0, MODE_IMAGE },                      /* OPERATION_TEST_UNTRUSTED_AUTHENTICATE, 35*/
{ 0, MODE_KEY },                        /* OPERATION_TEST_CHECK_FINGER_EVENT, 36*/
{ 0, MODE_HBD_DEBUG },                  /* OPERATION_TEST_BIO_CALIBRATION, 37*/
{ 0, MODE_HBD_DEBUG },                  /* OPERATION_TEST_HBD_CALIBRATION, 38*/
{ 0, MODE_DEBUG },                      /* OPERATION_TEST_REAL_TIME_DATA, 39*/
{ 0, MODE_IMAGE },                      /* OPERATION_TEST_BMP_DATA, 40*/
{ 2, MODE_IDLE },                       /* OPERATION_TEST_SENSOR_VALIDITY, 41*/
{ 0, MODE_NONE },                       /* OPERATION_TEST_RESET_PIN, 42*/
{ 0, MODE_IDLE},                        /* OPERATION_TEST_INTERRUPT_PIN, 43*/
{ 2, MODE_IDLE },                       /* OPERATION_TEST_PRE_SPI 44 */
{ 1, MODE_SLEEP },                      /* OPERATION_SCREEN_OFF_SLEEP, 45*/
{ 0, MODE_FINGER_BASE },                /* OPERATION_TEST_DATA_NOISE, 46*/
{ 0, MODE_IMAGE },                      /* OPERATION_TEST_RAWDATA_SATURATED, 47*/
{ 0, MODE_DEBUG },                      /* OPERATION_TEST_SENSOR_FINE_STEP1, 48*/
{ 0, MODE_FINGER_BASE },                /* OPERATION_TEST_SENSOR_FINE_STEP2, 49*/
{ 2, MODE_BROKEN_CHECK_DEFAULT },       /* OPERATION_BROKEN_CHECK_DEFAULT, 50*/
{ 2, MODE_BROKEN_CHECK_NEGATIVE },      /* OPERATION_BROKEN_CHECK_NEGATIVE, 51*/
{ 2, MODE_BROKEN_CHECK_POSITIVE },      /* OPERATION_BROKEN_CHECK_POSITIVE, 52*/
{ 0, MODE_SLEEP },                      /* OPERATION_NONE, 53*/
// in case of index out of array bounds
{ PRIORITY_UNDEFINE, MODE_NONE }        /* OPERATION_INVAILD, 54*/
};


/**
 * [gf_get_mode description]
 * @param  operation [description]
 * @return           [description]
 */
gf_mode_t gf_state_get_mode(gf_operation_type_t operation) {
    return g_operation_info[operation].mode;
}

/**
 * [state_get_priority_of_operation description]
 * @param  operation [description]
 * @return           [description]
 */
uint32_t gf_state_get_priority_of_operation(gf_operation_type_t operation) {
    return g_operation_info[operation].priority;
}

/**
 * [gf_state_is_base_operation description]
 * @param  operation [description]
 * @return           [description]
 */
uint32_t gf_state_is_base_operation(gf_operation_type_t operation) {
    uint32_t is_base = 0;

    switch (operation) {
        case OPERATION_CHECK_FINGER_LONG_PRESS:
        case OPERATION_NAV_BASE:
        case OPERATION_FINGER_BASE:
            is_base = 1;
            break;
        default:
            is_base = 0;
            break;
    }
    return is_base;
}

/**
 * [gf_state_is_test_operation description]
 * @return  [description]
 */
uint32_t gf_state_is_test_operation(gf_operation_type_t operation) {
    uint32_t is_test = 0;

    switch (operation) {
        case OPERATION_TEST_IMAGE_MODE:
        case OPERATION_TEST_DEBUG_MODE:
        case OPERATION_TEST_FF_MODE:
        case OPERATION_TEST_KEY_MODE:
        case OPERATION_TEST_NAV_MODE:
        case OPERATION_TEST_NAV_BASE_MODE:
        case OPERATION_TEST_FINGER_BASE_MODE:
        case OPERATION_TEST_IDLE_MODE:
        case OPERATION_TEST_SLEEP_MODE:
        case OPERATION_TEST_HBD_DEBUG_MODE:
        case OPERATION_TEST_HBD_MODE:
        case OPERATION_TEST_PIXEL_OPEN_STEP1:
        case OPERATION_TEST_PIXEL_OPEN_STEP2:
        case OPERATION_TEST_BAD_POINT:
        case OPERATION_TEST_PERFORMANCE:
        case OPERATION_TEST_SPI_PERFORMANCE:
        case OPERATION_TEST_SPI_TRANSFER:
        case OPERATION_TEST_FRR_FAR_RECORD_BASE_FRAME:
        case OPERATION_TEST_FRR_FAR_RECORD_ENROLL:
        case OPERATION_TEST_FRR_FAR_RECORD_AUTHENTICATE:
        case OPERATION_TEST_UNTRUSTED_ENROLL:
        case OPERATION_TEST_UNTRUSTED_AUTHENTICATE:
        case OPERATION_TEST_CHECK_FINGER_EVENT:
        case OPERATION_TEST_BIO_CALIBRATION:
        case OPERATION_TEST_HBD_CALIBRATION:
        case OPERATION_TEST_REAL_TIME_DATA:
        case OPERATION_TEST_BMP_DATA:
        case OPERATION_TEST_SENSOR_VALIDITY:
        case OPERATION_TEST_RESET_PIN:
        case OPERATION_TEST_SENSOR_FINE_STEP1:
        case OPERATION_TEST_SENSOR_FINE_STEP2:
        case OPERATION_TEST_INTERRUPT_PIN:
        case OPERATION_TEST_PRE_SPI:
        case OPERATION_TEST_DATA_NOISE:
        case OPERATION_TEST_RAWDATA_SATURATED:
            is_test = 1;
            break;
        default:
            is_test = 0;
            break;
    }
    return is_test;
}

/**
 * [gf_state_is_authenticate_operation description]
 * @return  [description]
 */
uint32_t gf_state_is_authenticate(gf_operation_type_t operation) {
    uint32_t is_authenticate = 0;

    switch (operation) {
        case OPERATION_AUTHENTICATE_IMAGE:
        case OPERATION_AUTHENTICATE_FF:
        case OPERATION_AUTHENTICATE_SLEEP:
        case OPERATION_AUTHENTICATE_FIDO:
            is_authenticate = 1;
            break;
        default:
            is_authenticate = 0;
            break;
    }
    return is_authenticate;
}

/**
 * [gf_state_set_mode]
 * @param  operation  [the operation that will be executed]
 * @param  reset_flag [1:need reset the chip,0:don't need resest the chip]
 * @return
 */
void gf_state_set_mode(gf_operation_type_t operation, uint8_t *reset_flag) {
    gf_mode_t mode = MODE_NONE;
    gf_error_t err = GF_SUCCESS;

    VOID_FUNC_ENTER();

    do {
        mode = gf_state_get_mode(operation);
        if (MODE_NONE == mode) {
            GF_LOGD(LOG_TAG "[%s] mode is MODE_NONE, nothing to do", __func__);
            break;
        }

        err = gf_hw_set_mode(mode);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] set mode failed, mode=%s", __func__, gf_strmode(mode));
            if (NULL != reset_flag) {
                *reset_flag = 1;
            }
            break;
        }

        GF_LOGD(LOG_TAG "[%s] set mode success, mode=%s", __func__, gf_strmode(mode));
    } while (0);

    VOID_FUNC_EXIT();
}

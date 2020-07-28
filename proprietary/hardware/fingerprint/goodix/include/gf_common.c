/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */

#include"gf_common.h"
#ifdef __HAL__
#include<string.h>
#define GFSTRCPY(A, B) strcpy((A), (B))  // NOLINT
#define GFSTRCAT(A, B) strcat((A), (B))  // NOLINT
#else
#include"cpl_string.h"
#define GFSTRCPY(A, B) cpl_strcpy((int8_t*)(A), (const int8_t*)(B))
#define GFSTRCAT(A, B) cpl_strcat((int8_t*)(A), (const int8_t*)(B))
#endif

static char g_strirq_buf[1024] = { 0 };

typedef struct {
    gf_mode_t mode;
    const char* strmode;
} gf_strmode_t;

typedef struct {
    gf_cmd_id_t cmd_id;
    const char* strcmd;
} gf_strcmd_t;

typedef struct {
    gf_operation_type_t opera;
    const char* stropera;
} gf_stroperation_t;

typedef struct {
    gf_nav_code_t nav;
    const char* strnav;
} gf_strnav_t;

typedef struct {
    gf_key_code_t key;
    const char* strkey;
} gf_strkey_t;

gf_strmode_t mode_table[] = {
    { MODE_IMAGE, "MODE_IMAGE" },
    { MODE_KEY, "MODE_KEY" },
    { MODE_SLEEP, "MODE_SLEEP" },
    { MODE_FF, "MODE_FF" },
    { MODE_NAV, "MODE_NAV" },
    { MODE_NAV_BASE, "MODE_NAV_BASE" },
    { MODE_DEBUG, "MODE_DEBUG" },
    { MODE_FINGER_BASE, "MODE_FINGER_BASE" },
    { MODE_IDLE, "MODE_IDLE" },
    { MODE_HBD, "MODE_HBD" },
    { MODE_HBD_DEBUG, "MODE_HBD_DEBUG" },
    { MODE_IMAGE_CONTINUE, "MODE_IMAGE_CONTINUE" },
    { MODE_BROKEN_CHECK_DEFAULT, "MODE_BROKEN_CHECK_DEFAULT" },
    { MODE_BROKEN_CHECK_NEGATIVE, "MODE_BROKEN_CHECK_NEGATIVE" },
    { MODE_BROKEN_CHECK_POSITIVE, "MODE_BROKEN_CHECK_POSITIVE" },
    { MODE_TEST_BAD_POINT, "MODE_TEST_BAD_POINT" },
    { MODE_TEST_PIXEL_OPEN_DEFAULT, "MODE_TEST_PIXEL_OPEN_DEFAULT" },
    { MODE_TEST_PIXEL_OPEN_POSITIVE, "MODE_TEST_PIXEL_OPEN_POSITIVE" },
    { MODE_TEST_BAD_POINT_FINGER_BASE, "MODE_TEST_BAD_POINT_FINGER_BASE"},
    { MODE_MAX, "MODE_MAX" },
    { MODE_NONE, "INVALID_MODE" }
};

gf_strcmd_t cmd_id_table[] = {
    { GF_CMD_DETECT_SENSOR, "GF_CMD_DETECT_SENSOR" },
    { GF_CMD_INIT, "GF_CMD_INIT" },
    { GF_CMD_EXIT, "GF_CMD_EXIT" },
    { GF_CMD_DOWNLOAD_FW, "GF_CMD_DOWNLOAD_FW" },
    { GF_CMD_DOWNLOAD_CFG, "GF_CMD_DOWNLOAD_CFG" },
    { GF_CMD_INIT_FINISHED, "GF_CMD_INIT_FINISHED" },
    { GF_CMD_PRE_ENROLL, "GF_CMD_PRE_ENROLL" },
    { GF_CMD_ENROLL, "GF_CMD_ENROLL" },
    { GF_CMD_POST_ENROLL, "GF_CMD_POST_ENROLL" },
    { GF_CMD_CANCEL, "GF_CMD_CANCEL" },
    { GF_CMD_AUTHENTICATE, "GF_CMD_AUTHENTICATE" },
    { GF_CMD_GET_AUTH_ID, "GF_CMD_GET_AUTH_ID" },
    { GF_CMD_SAVE, "GF_CMD_SAVE" },
    { GF_CMD_REMOVE, "GF_CMD_REMOVE" },
    { GF_CMD_SET_ACTIVE_GROUP, "GF_CMD_SET_ACTIVE_GROUP" },
    { GF_CMD_ENUMERATE, "GF_CMD_ENUMERATE" },
    { GF_CMD_IRQ, "GF_CMD_IRQ" },
    { GF_CMD_SCREEN_ON, "GF_CMD_SCREEN_ON" },
    { GF_CMD_SCREEN_OFF, "GF_CMD_SCREEN_OFF" },
    { GF_CMD_ESD_CHECK, "GF_CMD_ESD_CHECK" },
    { GF_CMD_SET_SAFE_CLASS, "GF_CMD_SET_SAFE_CLASS" },
    { GF_CMD_CAMERA_CAPTURE, "GF_CMD_CAMERA_CAPTURE" },
    { GF_CMD_ENABLE_FINGERPRINT_MODULE, "GF_CMD_ENABLE_FINGERPRINT_MODULE" },
    { GF_CMD_ENABLE_FF_FEATURE, "GF_CMD_ENABLE_FF_FEATURE" },
    { GF_CMD_TEST_BAD_POINT, "GF_CMD_TEST_BAD_POINT" },
    { GF_CMD_TEST_PIXEL_OPEN, "GF_CMD_TEST_PIXEL_OPEN" },
    { GF_CMD_TEST_PIXEL_OPEN_STEP1, "GF_CMD_TEST_PIXEL_OPEN_STEP1" },
    { GF_CMD_TEST_PIXEL_OPEN_FINISH, "GF_CMD_TEST_PIXEL_OPEN_FINISH" },
    { GF_CMD_TEST_PERFORMANCE, "GF_CMD_TEST_PERFORMANCE" },
    { GF_CMD_TEST_SPI_PERFORMANCE, "GF_CMD_TEST_SPI_PERFORMANCE" },
    { GF_CMD_TEST_SPI_TRANSFER, "GF_CMD_TEST_SPI_TRANSFER" },
    { GF_CMD_TEST_PRE_SPI, "GF_CMD_TEST_PRE_SPI" },
    { GF_CMD_TEST_SPI, "GF_CMD_TEST_SPI" },
    { GF_CMD_TEST_SPI_RW, "GF_CMD_TEST_SPI_RW" },
    { GF_CMD_TEST_PRE_GET_VERSION, "GF_CMD_TEST_PRE_GET_VERSION" },
    { GF_CMD_TEST_GET_VERSION, "GF_CMD_TEST_GET_VERSION" },
    { GF_CMD_TEST_FRR_FAR_INIT, "GF_CMD_TEST_FRR_FAR_INIT" },
    { GF_CMD_TEST_FRR_FAR_RECORD_BASE_FRAME, "GF_CMD_TEST_FRR_FAR_RECORD_BASE_FRAME" },
    { GF_CMD_TEST_FRR_FAR_RECORD_ENROLL, "GF_CMD_TEST_FRR_FAR_RECORD_ENROLL" },
    { GF_CMD_TEST_FRR_FAR_RECORD_AUTHENTICATE, "GF_CMD_TEST_FRR_FAR_RECORD_AUTHENTICATE" },
    { GF_CMD_TEST_FRR_FAR_RECORD_AUTHENTICATE_FINISH,
            "GF_CMD_TEST_FRR_FAR_RECORD_AUTHENTICATE_FINISH" },
    { GF_CMD_TEST_FRR_FAR_PLAY_BASE_FRAME, "GF_CMD_TEST_FRR_FAR_PLAY_BASE_FRAME" },
    { GF_CMD_TEST_FRR_FAR_PLAY_ENROLL, "GF_CMD_TEST_FRR_FAR_PLAY_ENROLL" },
    { GF_CMD_TEST_FRR_FAR_PLAY_AUTHENTICATE, "GF_CMD_TEST_FRR_FAR_PLAY_AUTHENTICATE" },
    { GF_CMD_TEST_FRR_FAR_ENROLL_FINISH, "GF_CMD_TEST_FRR_FAR_ENROLL_FINISH" },
    { GF_CMD_TEST_FRR_FAR_CANCEL, "GF_CMD_TEST_FRR_FAR_CANCEL" },
    { GF_CMD_TEST_RESET_PIN1, "GF_CMD_TEST_RESET_PIN1" },
    { GF_CMD_TEST_RESET_PIN2, "GF_CMD_TEST_RESET_PIN2" },
    { GF_CMD_TEST_SENSOR_FINE, "GF_CMD_TEST_SENSOR_FINE" },
    { GF_CMD_TEST_SENSOR_FINE_FINISH, "GF_CMD_TEST_SENSOR_FINE_FINISH" },
    { GF_CMD_TEST_INTERRUPT_PIN, "GF_CMD_TEST_INTERRUPT_PIN" },
    { GF_CMD_TEST_DOWNLOAD_FW, "GF_CMD_TEST_DOWNLOAD_FW" },
    { GF_CMD_TEST_DOWNLOAD_CFG, "GF_CMD_TEST_DOWNLOAD_CFG" },
    { GF_CMD_TEST_DOWNLOAD_FWCFG, "GF_CMD_TEST_DOWNLOAD_FWCFG" },
    { GF_CMD_TEST_RESET_FWCFG, "GF_CMD_TEST_RESET_FWCFG" },
    { GF_CMD_TEST_SENSOR_VALIDITY, "GF_CMD_TEST_SENSOR_VALIDITY" },
    { GF_CMD_TEST_SET_CONFIG, "GF_CMD_TEST_SET_CONFIG" },
    { GF_CMD_TEST_DRIVER_CMD, "GF_CMD_TEST_DRIVER_CMD" },
    { GF_CMD_TEST_UNTRUSTED_ENROLL, "GF_CMD_TEST_UNTRUSTED_ENROLL" },
    { GF_CMD_TEST_UNTRUSTED_AUTHENTICATE, "GF_CMD_TEST_UNTRUSTED_AUTHENTICATE" },
    { GF_CMD_TEST_DELETE_UNTRUSTED_ENROLLED_FINGER,
            "GF_CMD_TEST_DELETE_UNTRUSTED_ENROLLED_FINGER" },
    { GF_CMD_TEST_CHECK_FINGER_EVENT, "GF_CMD_TEST_CHECK_FINGER_EVENT" },
    { GF_CMD_TEST_BIO_CALIBRATION, "GF_CMD_TEST_BIO_CALIBRATION" },
    { GF_CMD_TEST_HBD_CALIBRATION, "GF_CMD_TEST_HBD_CALIBRATION" },
    { GF_CMD_TEST_CANCEL, "GF_CMD_TEST_CANCEL" },
    { GF_CMD_TEST_REAL_TIME_DATA, "GF_CMD_TEST_REAL_TIME_DATA" },
    { GF_CMD_TEST_BMP_DATA, "GF_CMD_TEST_BMP_DATA" },
    { GF_CMD_TEST_READ_CFG, "GF_CMD_TEST_READ_CFG" },
    { GF_CMD_TEST_READ_FW, "GF_CMD_TEST_READ_FW" },
    { GF_CMD_NAVIGATE, "GF_CMD_NAVIGATE" },
    { GF_CMD_DETECT_NAV_EVENT, "GF_CMD_DETECT_NAV_EVENT" },
    { GF_CMD_NAVIGATE_COMPLETE, "GF_CMD_NAVIGATE_COMPLETE" },
    { GF_CMD_DUMP_NAV_DATA, "GF_CMD_DUMP_NAV_DATA" },
    { GF_CMD_CHECK_FINGER_LONG_PRESS, "GF_CMD_CHECK_FINGER_LONG_PRESS" },
    { GF_CMD_FDT_DOWN_TIMEOUT, "GF_CMD_FDT_DOWN_TIMEOUT" },
    { GF_CMD_START_HBD, "GF_CMD_START_HBD" },
    { GF_CMD_AUTHENTICATE_FIDO, "GF_CMD_AUTHENTICATE_FIDO" },
    { GF_CMD_DUMP_TEMPLATE, "GF_CMD_DUMP_TEMPLATE" },
    { GF_CMD_DUMP_DATA, "GF_CMD_DUMP_DATA" },
    { GF_CMD_DUMP_ORIGIN_DATA, "GF_CMD_DUMP_ORIGIN_DATA" },
    { GF_CMD_TEST_PRIOR_CANCEL, "GF_CMD_TEST_PRIOR_CANCEL" },
    { GF_CMD_TEST_NOISE, "GF_CMD_TEST_NOISE" },
    { GF_CMD_TEST_RAWDATA_SATURATED, "GF_CMD_TEST_RAWDATA_SATURATED" },
    { GF_CMD_UPDATE_STITCH, "GF_CMD_UPDATE_STITCH" },
    { GF_CMD_AUTHENTICATE_STUDY, "GF_CMD_AUTHENTICATE_STUDY"},
    { GF_CMD_MAX, "INVALID_CMD_ID" }
};

gf_stroperation_t operation_table[] = {
    { OPERATION_ENROLL, "OPERATION_ENROLL" },
    { OPERATION_AUTHENTICATE_IMAGE, "OPERATION_AUTHENTICATE_IMAGE" },
    { OPERATION_AUTHENTICATE_FF, "OPERATION_AUTHENTICATE_FF" },
    { OPERATION_AUTHENTICATE_SLEEP, "OPERATION_AUTHENTICATE_SLEEP" },
    { OPERATION_AUTHENTICATE_FIDO, "OPERATION_AUTHENTICATE_FIDO" },
    { OPERATION_FINGER_BASE, "OPERATION_FINGER_BASE" },
    { OPERATION_NAV, "OPERATION_NAV" },
    { OPERATION_NAV_BASE, "OPERATION_NAV_BASE" },
    { OPERATION_CHECK_FINGER_LONG_PRESS, "OPERATION_CHECK_FINGER_LONG_PRESS" },
    { OPERATION_HOME_KEY, "OPERATION_HOME_KEY" },
    { OPERATION_POWER_KEY, "OPERATION_POWER_KEY" },
    { OPERATION_CAMERA_KEY, "OPERATION_CAMERA_KEY" },
    { OPERATION_HEARTBEAT_KEY, "OPERATION_HEARTBEAT_KEY" },
    { OPERATION_TEST_IMAGE_MODE, "OPERATION_TEST_IMAGE_MODE" },
    { OPERATION_TEST_DEBUG_MODE, "OPERATION_TEST_DEBUG_MODE" },
    { OPERATION_TEST_FF_MODE, "OPERATION_TEST_FF_MODE" },
    { OPERATION_TEST_KEY_MODE, "OPERATION_TEST_KEY_MODE" },
    { OPERATION_TEST_NAV_MODE, "OPERATION_TEST_NAV_MODE" },
    { OPERATION_TEST_NAV_BASE_MODE, "OPERATION_TEST_NAV_BASE_MODE" },
    { OPERATION_TEST_FINGER_BASE_MODE, "OPERATION_TEST_FINGER_BASE_MODE" },
    { OPERATION_TEST_IDLE_MODE, "OPERATION_TEST_IDLE_MODE" },
    { OPERATION_TEST_SLEEP_MODE, "OPERATION_TEST_SLEEP_MODE" },
    { OPERATION_TEST_HBD_DEBUG_MODE, "OPERATION_TEST_HBD_DEBUG_MODE" },
    { OPERATION_TEST_HBD_MODE, "OPERATION_TEST_HBD_MODE" },
    { OPERATION_TEST_PIXEL_OPEN_STEP1, "OPERATION_TEST_PIXEL_OPEN_STEP1" },
    { OPERATION_TEST_PIXEL_OPEN_STEP2, "OPERATION_TEST_PIXEL_OPEN_STEP2" },
    { OPERATION_TEST_BAD_POINT_RECODE_BASE, "OPERATION_TEST_BAD_POINT_RECODE_BASE"},
    { OPERATION_TEST_BAD_POINT, "OPERATION_TEST_BAD_POINT" },
    { OPERATION_TEST_PERFORMANCE, "OPERATION_TEST_PERFORMANCE" },
    { OPERATION_TEST_SPI_PERFORMANCE, "OPERATION_TEST_SPI_PERFORMANCE" },
    { OPERATION_TEST_SPI_TRANSFER, "OPERATION_TEST_SPI_TRANSFER" },
    { OPERATION_TEST_FRR_FAR_RECORD_BASE_FRAME, "OPERATION_TEST_FRR_FAR_RECORD_BASE_FRAME" },
    { OPERATION_TEST_FRR_FAR_RECORD_ENROLL, "OPERATION_TEST_FRR_FAR_RECORD_ENROLL" },
    { OPERATION_TEST_FRR_FAR_RECORD_AUTHENTICATE, "OPERATION_TEST_FRR_FAR_RECORD_AUTHENTICATE" },
    { OPERATION_TEST_UNTRUSTED_ENROLL, "OPERATION_TEST_UNTRUSTED_ENROLL" },
    { OPERATION_TEST_UNTRUSTED_AUTHENTICATE, "OPERATION_TEST_UNTRUSTED_AUTHENTICATE" },
    { OPERATION_TEST_CHECK_FINGER_EVENT, "OPERATION_TEST_CHECK_FINGER_EVENT" },
    { OPERATION_TEST_BIO_CALIBRATION, "OPERATION_TEST_BIO_CALIBRATION" },
    { OPERATION_TEST_HBD_CALIBRATION, "OPERATION_TEST_HBD_CALIBRATION" },
    { OPERATION_TEST_REAL_TIME_DATA, "OPERATION_TEST_REAL_TIME_DATA" },
    { OPERATION_TEST_BMP_DATA, "OPERATION_TEST_BMP_DATA" },
    { OPERATION_TEST_SENSOR_VALIDITY, "OPERATION_TEST_SENSOR_VALIDITY" },
    { OPERATION_TEST_RESET_PIN, "OPERATION_TEST_RESET_PIN" },
    { OPERATION_TEST_INTERRUPT_PIN, "OPERATION_TEST_INTERRUPT_PIN" },
    { OPERATION_TEST_PRE_SPI, "OPERATION_TEST_PRE_SPI" },
    { OPERATION_SCREEN_OFF_SLEEP, "OPERATION_SCREEN_OFF_SLEEP" },
    { OPERATION_TEST_DATA_NOISE, "OPERATION_TEST_DATA_NOISE" },
    { OPERATION_TEST_RAWDATA_SATURATED, "OPERATION_TEST_RAWDATA_SATURATED" },
    { OPERATION_TEST_SENSOR_FINE_STEP1, "OPERATION_TEST_SENSOR_FINE_STEP1" },
    { OPERATION_TEST_SENSOR_FINE_STEP2, "OPERATION_TEST_SENSOR_FINE_STEP2" },
    { OPERATION_BROKEN_CHECK_DEFAULT, "OPERATION_BROKEN_CHECK_DEFAULT" },
    { OPERATION_BROKEN_CHECK_NEGATIVE, "OPERATION_BROKEN_CHECK_NEGATIVE" },
    { OPERATION_BROKEN_CHECK_POSITIVE, "OPERATION_BROKEN_CHECK_POSITIVE" },
    { OPERATION_NONE, "OPERATION_NONE" },
    { OPERATION_INVAILD, "OPERATION_INVAILD" },
    { OPERATION_MAX, "INVALID_OPERATION" }
};

gf_strnav_t nav_table[] = {
    { GF_NAV_NONE, "GF_NAV_NONE" },
    { GF_NAV_FINGER_UP, "GF_NAV_FINGER_UP" },
    { GF_NAV_FINGER_DOWN, "GF_NAV_FINGER_DOWN" },
    { GF_NAV_UP, "GF_NAV_UP" },
    { GF_NAV_DOWN, "GF_NAV_DOWN" },
    { GF_NAV_LEFT, "GF_NAV_LEFT" },
    { GF_NAV_RIGHT, "GF_NAV_RIGHT" },
    { GF_NAV_CLICK, "GF_NAV_CLICK" },
    { GF_NAV_HEAVY, "GF_NAV_HEAVY" },
    { GF_NAV_LONG_PRESS, "GF_NAV_LONG_PRESS" },
    { GF_NAV_DOUBLE_CLICK, "GF_NAV_DOUBLE_CLICK" },
    { GF_NAV_MAX, "INVALID_NAV_CODE" }
};

gf_strkey_t key_table[] = {
    { GF_KEY_NONE, "GF_KEY_NONE" },
    { GF_KEY_HOME, "GF_KEY_HOME" },
    { GF_KEY_POWER, "GF_KEY_POWER" },
    { GF_KEY_MENU, "GF_KEY_MENU" },
    { GF_KEY_BACK, "GF_KEY_BACK" },
    { GF_KEY_CAMERA, "GF_KEY_CAMERA" },
    { GF_KEY_MAX, "INVALID_KEY_CODE" }
};

const char* gf_strmode(gf_mode_t mode) {
    uint32_t idx = 0;
    uint32_t len = sizeof(mode_table) / sizeof(gf_strmode_t);

    // mode is consequent[ 0~max ,then 1000]
    do {
        if ((mode > MODE_MAX) || (mode - MODE_IMAGE >= len)) {
            idx = len - 1;
            break;
        }

        if (mode == mode_table[mode - MODE_IMAGE].mode) {
            idx = mode - MODE_IMAGE;
            break;
        }

        while (idx < len) {
            if (mode == mode_table[idx].mode) {
                break;
            }

            idx++;
        }

        if (idx == len) {
            idx = len - 1;
        }
    } while (0);

    return mode_table[idx].strmode;
}

const char* gf_strcmd(gf_cmd_id_t cmd_id) {
    uint32_t idx = 0;
    uint32_t len = sizeof(cmd_id_table) / sizeof(gf_strcmd_t);

    // cmd_id is consequent [1000,1001,...,max]
    do {
        if ((cmd_id < GF_CMD_DETECT_SENSOR) || (cmd_id > GF_CMD_MAX)
                || cmd_id - GF_CMD_DETECT_SENSOR >= len) {
            idx = len - 1;
            break;
        }

        if (cmd_id == cmd_id_table[cmd_id - GF_CMD_DETECT_SENSOR].cmd_id) {
            idx = cmd_id - GF_CMD_DETECT_SENSOR;
            break;
        }

        while (idx < len) {
            if (cmd_id == cmd_id_table[idx].cmd_id) {
                break;
            }

            idx++;
        }

        if (idx == len) {
            idx = len - 1;
        }
    } while (0);

    return cmd_id_table[idx].strcmd;
}

const char* gf_stroperation(gf_operation_type_t opera) {
    uint32_t idx = 0;
    uint32_t len = sizeof(operation_table) / sizeof(gf_stroperation_t);

    // operation is consequent [0,1,,...,max]
    do {
        if ((opera > OPERATION_MAX) || (opera - OPERATION_ENROLL >= len)) {
            idx = len - 1;
            break;
        }

        if (opera == operation_table[opera - OPERATION_ENROLL].opera) {
            idx = opera - OPERATION_ENROLL;
            break;
        }

        while (idx < len) {
            if (opera == operation_table[idx].opera) {
                break;
            }

            idx++;
        }

        if (idx == len) {
            idx = len - 1;
        }
    } while (0);

    return operation_table[idx].stropera;
}

const char* gf_strnav(gf_nav_code_t nav) {
    uint32_t idx = 0;
    uint32_t len = sizeof(nav_table) / sizeof(gf_strnav_t);

    // nav code is consequent [0,1,,...,max]
    do {
        if ((nav > GF_NAV_MAX) || (nav - GF_NAV_NONE >= len)) {
            idx = len - 1;
            break;
        }

        if (nav == nav_table[nav - GF_NAV_NONE].nav) {
            idx = nav - GF_NAV_NONE;
            break;
        }

        while (idx < len) {
            if (nav == nav_table[idx].nav) {
                break;
            }

            idx++;
        }

        if (idx == len) {
            idx = len - 1;
        }
    } while (0);
    return nav_table[idx].strnav;
}

const char* gf_strkey(gf_key_code_t key) {
    uint32_t idx = 0;
    uint32_t len = sizeof(key_table) / sizeof(gf_strkey_t);

    // key code is consequent [0,1,,...,max]
    do {
        if ((key > GF_KEY_MAX) || (key - GF_KEY_NONE >= len)) {
            idx = len - 1;
            break;
        }

        if (key == key_table[key - GF_KEY_NONE].key) {
            idx = key - GF_KEY_NONE;
            break;
        }

        while (idx < len) {
            if (key == key_table[idx].key) {
                break;
            }

            idx++;
        }

        if (idx == len) {
            idx = len - 1;
        }
    } while (0);

    return key_table[idx].strkey;
}

const char* gf_strirq(uint32_t irq_type) {
    uint32_t irq_count = 0;
    uint32_t irq_copy = irq_type;

    // attention to use this function in multiple threads, because of g_strirq_buf
    while (irq_copy) {
        if (irq_copy & 1) {
            irq_count++;
        }
        irq_copy >>= 1;
    }

    if ((irq_count > 5) || (irq_count == 0)) {
        GFSTRCPY(g_strirq_buf, "INVALID_IRQ_TYPE");
    } else {
        g_strirq_buf[0] = '\0';

        if (irq_type & GF_IRQ_FINGER_DOWN_MASK) {
            GFSTRCAT(g_strirq_buf, "[GF_IRQ_FINGER_DOWN_MASK]");
        }

        if (irq_type & GF_IRQ_FINGER_UP_MASK) {
            GFSTRCAT(g_strirq_buf, "[GF_IRQ_FINGER_UP_MASK]");
        }

        if (irq_type & GF_IRQ_MENUKEY_DOWN_MASK) {
            GFSTRCAT(g_strirq_buf, "[GF_IRQ_MENUKEY_DOWN_MASK]");
        }

        if (irq_type & GF_IRQ_MENUKEY_UP_MASK) {
            GFSTRCAT(g_strirq_buf, "[GF_IRQ_MENUKEY_UP_MASK]");
        }

        if (irq_type & GF_IRQ_BACKKEY_DOWN_MASK) {
            GFSTRCAT(g_strirq_buf, "[GF_IRQ_BACKKEY_DOWN_MASK]");
        }

        if (irq_type & GF_IRQ_BACKKEY_UP_MASK) {
            GFSTRCAT(g_strirq_buf, "[GF_IRQ_BACKKEY_UP_MASK]");
        }

        if (irq_type & GF_IRQ_IMAGE_MASK) {
            GFSTRCAT(g_strirq_buf, "[GF_IRQ_IMAGE_MASK]");
        }

        if (irq_type & GF_IRQ_RESET_MASK) {
            GFSTRCAT(g_strirq_buf, "[GF_IRQ_RESET_MASK]");
        }

        if (irq_type & GF_IRQ_TMR_IRQ_MNT_MASK) {
            GFSTRCAT(g_strirq_buf, "[GF_IRQ_TMR_IRQ_MNT_MASK]");
        }

        if (irq_type & GF_IRQ_ONE_FRAME_DONE_MASK) {
            GFSTRCAT(g_strirq_buf, "[GF_IRQ_ONE_FRAME_DONE_MASK]");
        }

        if (irq_type & GF_IRQ_ESD_IRQ_MASK) {
            GFSTRCAT(g_strirq_buf, "[GF_IRQ_ESD_IRQ_MASK]");
        }

        if (irq_type & GF_IRQ_ADC_FIFO_FULL_MASK) {
            GFSTRCAT(g_strirq_buf, "[GF_IRQ_ADC_FIFO_FULL_MASK]");
        }

        if (irq_type & GF_IRQ_ADC_FIFO_HALF_MASK) {
            GFSTRCAT(g_strirq_buf, "[GF_IRQ_ADC_FIFO_HALF_MASK]");
        }

        if (irq_type & GF_IRQ_FDT_REVERSE_MASK) {
            GFSTRCAT(g_strirq_buf, "[GF_IRQ_FDT_REVERSE_MASK]");
        }

        if (irq_type & GF_IRQ_NAV_MASK) {
            GFSTRCAT(g_strirq_buf, "[GF_IRQ_NAV_MASK]");
        }

        if (irq_type & GF_IRQ_GSC_MASK) {
            GFSTRCAT(g_strirq_buf, "[GF_IRQ_GSC_MASK]");
        }

        if (irq_type & GF_IRQ_HBD_MASK) {
            GFSTRCAT(g_strirq_buf, "[GF_IRQ_HBD_MASK]");
        }

        if (irq_type & GF_IRQ_FW_ERR_MASK) {
            GFSTRCAT(g_strirq_buf, "[GF_IRQ_FW_ERR_MASK]");
        }

        if (irq_type & GF_IRQ_CFG_ERR_MASK) {
            GFSTRCAT(g_strirq_buf, "[GF_IRQ_CFG_ERR_MASK]");
        }

        if (irq_type & GF_IRQ_ESD_ERR_MASK) {
            GFSTRCAT(g_strirq_buf, "[GF_IRQ_ESD_ERR_MASK]");
        }

        if (irq_type & GF_IRQ_NAV_LEFT_MASK) {
            GFSTRCAT(g_strirq_buf, "[GF_IRQ_NAV_LEFT_MASK]");
        }

        if (irq_type & GF_IRQ_NAV_RIGHT_MASK) {
            GFSTRCAT(g_strirq_buf, "[GF_IRQ_NAV_RIGHT_MASK]");
        }

        if (irq_type & GF_IRQ_NAV_UP_MASK) {
            GFSTRCAT(g_strirq_buf, "[GF_IRQ_NAV_UP_MASK]");
        }

        if (irq_type & GF_IRQ_NAV_DOWN_MASK) {
            GFSTRCAT(g_strirq_buf, "[GF_IRQ_NAV_DOWN_MASK]");
        }

        if (irq_type & GF_IRQ_PRESS_LIGHT_MASK) {
            GFSTRCAT(g_strirq_buf, "[GF_IRQ_PRESS_LIGHT_MASK]");
        }

        if (irq_type & GF_IRQ_PRESS_HEAVY_MASK) {
            GFSTRCAT(g_strirq_buf, "[GF_IRQ_PRESS_HEAVY_MASK]");
        }

        if (irq_type & GF_IRQ_UPDATE_BASE_MASK) {
            GFSTRCAT(g_strirq_buf, "[GF_IRQ_UPDATE_BASE_MASK]");
        }

        if (irq_type & GF_IRQ_TEMPERATURE_CHANGE_MASK) {
            GFSTRCAT(g_strirq_buf, "[GF_IRQ_TEMPERATURE_CHANGE_MASK]");
        }
    }

    return g_strirq_buf;
}

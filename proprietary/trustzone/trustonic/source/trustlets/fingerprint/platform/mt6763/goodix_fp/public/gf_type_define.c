/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */

#include"gf_type_define.h"

typedef struct {
    gf_nav_mode_t navmode;
    const char* strnavmode;
} gf_strnavmode_t;

typedef struct {
    gf_chip_type_t chip;
    const char* strchip;
} gf_strchiptype_t;

typedef struct {
    gf_cmd_test_id_t testcmdid;
    const char* strtestcmd;
} gf_strtestcmd_t;

gf_strnavmode_t navmode_table[] = {
    { GF_NAV_MODE_NONE, "GF_NAV_MODE_NONE" },
    { GF_NAV_MODE_X, "GF_NAV_MODE_X" },
    { GF_NAV_MODE_Y, "GF_NAV_MODE_Y" },
    { GF_NAV_MODE_Z, "GF_NAV_MODE_Z" },
    { GF_NAV_MODE_XY, "GF_NAV_MODE_XY" },
    { GF_NAV_MODE_XZ, "GF_NAV_MODE_XZ" },
    { GF_NAV_MODE_YZ, "GF_NAV_MODE_YZ" },
    { GF_NAV_MODE_XYZ, "GF_NAV_MODE_XYZ" },
    { GF_NAV_MODE_MAX, "GF_NAV_MODE_MAX" }
};

gf_strchiptype_t chip_table[] = {
    { GF_CHIP_316M, "GF_CHIP_316M" },
    { GF_CHIP_318M, "GF_CHIP_318M" },
    { GF_CHIP_3118M, "GF_CHIP_3118M" },
    { GF_CHIP_516M, "GF_CHIP_516M" },
    { GF_CHIP_518M, "GF_CHIP_518M" },
    { GF_CHIP_5118M, "GF_CHIP_5118M" },
    { GF_CHIP_816M, "GF_CHIP_816M" },
    { GF_CHIP_3266, "GF_CHIP_3266" },
    { GF_CHIP_3208, "GF_CHIP_3208" },
    { GF_CHIP_3268, "GF_CHIP_3268" },
    { GF_CHIP_3228, "GF_CHIP_3228" },
    { GF_CHIP_3288, "GF_CHIP_3288" },
    { GF_CHIP_3206, "GF_CHIP_3206" },
    { GF_CHIP_3226, "GF_CHIP_3226" },
    { GF_CHIP_3258, "GF_CHIP_3258" },
    { GF_CHIP_5206, "GF_CHIP_5206" },
    { GF_CHIP_5216, "GF_CHIP_5216" },
    { GF_CHIP_5208, "GF_CHIP_5208" },
    { GF_CHIP_5218, "GF_CHIP_5218" },
    { GF_CHIP_8206, "GF_CHIP_8206" },
    { GF_CHIP_5266, "GF_CHIP_5266" },
    { GF_CHIP_5288, "GF_CHIP_5288" },
    { GF_CHIP_6226, "GF_CHIP_6226" },
    { GF_CHIP_UNKNOWN, "GF_CHIP_UNKNOWN" }
};

gf_strtestcmd_t  testcmd_table[] = {
    { CMD_TEST_ENUMERATE,    "CMD_TEST_ENUMERATE" },
    { CMD_TEST_DRIVER,       "CMD_TEST_DRIVER" },
    { CMD_TEST_PIXEL_OPEN,   "CMD_TEST_PIXEL_OPEN" },
    { CMD_TEST_BAD_POINT,    "CMD_TEST_BAD_POINT" },
    { CMD_TEST_SENSOR_FINE,  "CMD_TEST_SENSOR_FINE" },
    { CMD_TEST_PERFORMANCE,  "CMD_TEST_PERFORMANCE" },
    { CMD_TEST_SPI_PERFORMANCE,  "CMD_TEST_SPI_PERFORMANCE" },
    { CMD_TEST_SPI_TRANSFER, "CMD_TEST_SPI_TRANSFER" },
    { CMD_TEST_SPI,          "CMD_TEST_SPI" },
    { CMD_TEST_GET_VERSION,  "CMD_TEST_GET_VERSION" },
    { CMD_TEST_FRR_FAR_GET_CHIP_TYPE,       "CMD_TEST_FRR_FAR_GET_CHIP_TYPE" },
    { CMD_TEST_FRR_FAR_INIT,                "CMD_TEST_FRR_FAR_INIT" },
    { CMD_TEST_FRR_FAR_RECORD_BASE_FRAME,   "CMD_TEST_FRR_FAR_RECORD_BASE_FRAME" },
    { CMD_TEST_FRR_FAR_RECORD_ENROLL,       "CMD_TEST_FRR_FAR_RECORD_ENROLL" },
    { CMD_TEST_FRR_FAR_RECORD_AUTHENTICATE, "CMD_TEST_FRR_FAR_RECORD_AUTHENTICATE" },
    { CMD_TEST_FRR_FAR_RECORD_AUTHENTICATE_FINISH, "CMD_TEST_FRR_FAR_RECORD_AUTHENTICATE_FINISH" },
    { CMD_TEST_FRR_FAR_PLAY_BASE_FRAME,     "CMD_TEST_FRR_FAR_PLAY_BASE_FRAME" },
    { CMD_TEST_FRR_FAR_PLAY_ENROLL,         "CMD_TEST_FRR_FAR_PLAY_ENROLL" },
    { CMD_TEST_FRR_FAR_PLAY_AUTHENTICATE,   "CMD_TEST_FRR_FAR_PLAY_AUTHENTICATE" },
    { CMD_TEST_FRR_FAR_ENROLL_FINISH,       "CMD_TEST_FRR_FAR_ENROLL_FINISH" },
    { CMD_TEST_CANCEL_FRR_FAR, "CMD_TEST_CANCEL_FRR_FAR" },
    { CMD_TEST_RESET_PIN,      "CMD_TEST_RESET_PIN" },
    { CMD_TEST_INTERRUPT_PIN,  "CMD_TEST_INTERRUPT_PIN" },
    { CMD_TEST_CANCEL,         "CMD_TEST_CANCEL" },
    { CMD_TEST_GET_CONFIG,     "CMD_TEST_GET_CONFIG" },
    { CMD_TEST_SET_CONFIG,     "CMD_TEST_SET_CONFIG" },
    { CMD_TEST_DOWNLOAD_FW,    "CMD_TEST_DOWNLOAD_FW" },
    { CMD_TEST_DOWNLOAD_CFG,   "CMD_TEST_DOWNLOAD_CFG" },
    { CMD_TEST_DOWNLOAD_FWCFG, "CMD_TEST_DOWNLOAD_FWCFG" },
    { CMD_TEST_RESET_FWCFG,    "CMD_TEST_RESET_FWCFG" },
    { CMD_TEST_SENSOR_VALIDITY, "CMD_TEST_SENSOR_VALIDITY" },
    { CMD_TEST_RESET_CHIP,     "CMD_TEST_RESET_CHIP" },
    { CMD_TEST_UNTRUSTED_ENROLL, "CMD_TEST_UNTRUSTED_ENROLL" },
    { CMD_TEST_UNTRUSTED_AUTHENTICATE, "CMD_TEST_UNTRUSTED_AUTHENTICATE" },
    { CMD_TEST_DELETE_UNTRUSTED_ENROLLED_FINGER, "CMD_TEST_DELETE_UNTRUSTED_ENROLLED_FINGER" },
    { CMD_TEST_CHECK_FINGER_EVENT, "CMD_TEST_CHECK_FINGER_EVENT" },
    { CMD_TEST_BIO_CALIBRATION,    "CMD_TEST_BIO_CALIBRATION" },
    { CMD_TEST_HBD_CALIBRATION,    "CMD_TEST_HBD_CALIBRATION" },
    { CMD_TEST_SPI_RW,             "CMD_TEST_SPI_RW" },
    { CMD_TEST_REAL_TIME_DATA,     "CMD_TEST_REAL_TIME_DATA" },
    { CMD_TEST_READ_CFG,           "CMD_TEST_READ_CFG" },
    { CMD_TEST_READ_FW,            "CMD_TEST_READ_FW" },
    { CMD_TEST_FRR_DATABASE_ACCESS, "CMD_TEST_FRR_DATABASE_ACCESS" },
    { CMD_TEST_PRIOR_CANCEL,       "CMD_TEST_PRIOR_CANCEL" },
    { CMD_TEST_NOISE,              "CMD_TEST_NOISE" },
    { CMD_TEST_RAWDATA_SATURATED,  "CMD_TEST_RAWDATA_SATURATED" },
    { CMD_TEST_BMP_DATA,           "CMD_TEST_BMP_DATA" },
    { CMD_TEST_MEMMGR_SET_CONFIG,  "CMD_TEST_MEMMGR_SET_CONFIG" },
    { CMD_TEST_MEMMGR_GET_CONFIG,  "CMD_TEST_MEMMGR_GET_CONFIG" },
    { CMD_TEST_MEMMGR_GET_INFO,    "CMD_TEST_MEMMGR_GET_INFO" },
    { CMD_TEST_MEMMGR_DUMP_POOL,   "CMD_TEST_MEMMGR_DUMP_POOL" },
    { CMD_TEST_UNKNOWN, "CMD_TEST_UNKNOWN" }
};

const char* gf_strnavmode(gf_nav_mode_t navmode) {
    // nav_mode enum is not consequent
    uint32_t idx = 0;
    uint32_t len = sizeof(navmode_table) / sizeof(gf_strnavmode_t);

    do {
        if (navmode > GF_NAV_MODE_MAX) {
            idx = len - 1;
            break;
        }

        while (idx < len) {
            if (navmode == navmode_table[idx].navmode) {
                break;
            }

            idx++;
        }

        if (idx == len) {
            idx = len - 1;
        }
    } while (0);

    return navmode_table[idx].strnavmode;
}

const char* gf_strchiptype(gf_chip_type_t chip) {
    uint32_t idx = 0;
    uint32_t len = sizeof(chip_table) / sizeof(gf_strchiptype_t);

    do {
        if ((chip > GF_CHIP_UNKNOWN) || (chip - GF_CHIP_316M >= len)) {
            idx = len - 1;
            break;
        }

        if (chip == chip_table[chip - GF_CHIP_316M].chip) {
            idx = chip - GF_CHIP_316M;
            break;
        }

        while (idx < len) {
            if (chip == chip_table[idx].chip) {
                break;
            }

            idx++;
        }

        if (idx == len) {
            idx = len - 1;
        }
    } while (0);

    return chip_table[idx].strchip;
}

const char* gf_strtestcmd(gf_cmd_test_id_t test_cmd_id) {
    uint32_t idx = 0;

    do {
        if (test_cmd_id > CMD_TEST_UNKNOWN || test_cmd_id < CMD_TEST_ENUMERATE) {
            idx = CMD_TEST_UNKNOWN;
        } else {
            idx = test_cmd_id;
        }
    } while (0);

    return testcmd_table[idx].strtestcmd;
}

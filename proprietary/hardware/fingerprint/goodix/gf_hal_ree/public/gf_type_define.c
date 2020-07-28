/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
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
    { GF_CHIP_3208FN, "GF_CHIP_3208FN" },
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
    { GF_CHIP_5288, "GF_CHIP_5288" },
    { GF_CHIP_6226, "GF_CHIP_6226" },
    { GF_CHIP_UNKNOWN, "GF_CHIP_UNKNOWN" }
};

const char* gf_strnavmode(gf_nav_mode_t navmode){
    //nav_mode enum is not consequent
    uint32_t idx = 0;
    uint32_t len = sizeof(navmode_table) / sizeof(gf_strnavmode_t);

    do {
        if ((navmode < GF_NAV_MODE_NONE) || (navmode > GF_NAV_MODE_MAX)) {
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

const char* gf_strchiptype(gf_chip_type_t chip){
    uint32_t idx = 0;
    uint32_t len = sizeof(chip_table) / sizeof(gf_strchiptype_t);

    do {
        if ((chip < GF_CHIP_316M) || (chip > GF_CHIP_UNKNOWN) || (chip - GF_CHIP_316M >= len)) {
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

/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */

#include "gf_milan_platform_chip_params.h"
#include "gf_log.h"
#include "milan_regs.h"

#define LOG_TAG "[gf_milan_platform_chip_params]"

// continue mode , qualcomm read 4 frames in nav
#define READ_FRAMS_FOR_NAV_IN_CONTINUE 4

typedef enum {
    MILAN_G_NAV_CONFIG_REG_CTRL8_VALUE = 0x0310,  // MILAN_REG_PIXEL_CTRL8 = 0x0062, /* RW, 2 byte*/
    MILAN_G_NAV_CONFIG_REG_CTRL9_VALUE = 0x0030  // MILAN_REG_PIXEL_CTRL9 = 0x0064, /* RW, 2 byte*/
} milan_g_nav_config;

typedef enum {
    MILAN_E_NAV_CONFIG_REG_CTRL8_VALUE = 0x0310,  // MILAN_REG_PIXEL_CTRL8 = 0x0062, /* RW, 2 byte*/
    MILAN_E_NAV_CONFIG_REG_CTRL9_VALUE = 0x0030  // MILAN_REG_PIXEL_CTRL9 = 0x0064, /* RW, 2 byte*/
} milan_e_nav_config;

typedef enum {
    MILAN_F_NAV_CONFIG_READ_FRAMS_FOR_NAV_IN_CONTINUE = 4
} milan_f_nav_config;

typedef enum {
    MILAN_H_NAV_CONFIG_READ_FRAMS_FOR_NAV_IN_CONTINUE = 4
} milan_h_nav_config;

gf_error_t gf_milan_platform_init_chip_params(milan_chip_handle_t* handle,
        uint32_t* read_frame_num_for_nav_in_continue) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        switch (handle->chip_id >> 8) {
        case MILAN_E_CHIP_ID: {
            MILAN_UPDATE_CONFIG_ARRAY_BY_WORD(handle->config->nav,
                    MILAN_REG_PIXEL_CTRL8, MILAN_E_NAV_CONFIG_REG_CTRL8_VALUE);
            MILAN_UPDATE_CONFIG_ARRAY_BY_WORD(handle->config->nav,
                    MILAN_REG_PIXEL_CTRL9, MILAN_E_NAV_CONFIG_REG_CTRL9_VALUE);

            MILAN_UPDATE_CONFIG_ARRAY_BY_WORD(handle->config->nav_base,
                    MILAN_REG_PIXEL_CTRL8, MILAN_E_NAV_CONFIG_REG_CTRL8_VALUE);
            MILAN_UPDATE_CONFIG_ARRAY_BY_WORD(handle->config->nav_base,
                    MILAN_REG_PIXEL_CTRL9, MILAN_E_NAV_CONFIG_REG_CTRL9_VALUE);
        }
            break;
        case MILAN_F_CHIP_ID: {
            *read_frame_num_for_nav_in_continue = MILAN_F_NAV_CONFIG_READ_FRAMS_FOR_NAV_IN_CONTINUE;
        }
            break;
        case MILAN_G_CHIP_ID: {
            MILAN_UPDATE_CONFIG_ARRAY_BY_WORD(handle->config->nav,
                    MILAN_REG_PIXEL_CTRL8, MILAN_G_NAV_CONFIG_REG_CTRL8_VALUE);
            MILAN_UPDATE_CONFIG_ARRAY_BY_WORD(handle->config->nav,
                    MILAN_REG_PIXEL_CTRL9, MILAN_G_NAV_CONFIG_REG_CTRL9_VALUE);

            MILAN_UPDATE_CONFIG_ARRAY_BY_WORD(handle->config->nav_base,
                    MILAN_REG_PIXEL_CTRL8, MILAN_G_NAV_CONFIG_REG_CTRL8_VALUE);
            MILAN_UPDATE_CONFIG_ARRAY_BY_WORD(handle->config->nav_base,
                    MILAN_REG_PIXEL_CTRL9, MILAN_G_NAV_CONFIG_REG_CTRL9_VALUE);
        }
            break;
        case MILAN_H_CHIP_ID: {
            *read_frame_num_for_nav_in_continue = MILAN_H_NAV_CONFIG_READ_FRAMS_FOR_NAV_IN_CONTINUE;
        }
            break;
        default: {
            GF_LOGE(LOG_TAG "[%s]unknown chip id", __func__);
        }
            break;
        }
    } while (0);

    FUNC_EXIT(err);
    return err;
}


/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#include "gf_irq_common.h"

#include <stdint.h>

#include "gf_state_common.h"
#include "gf_common.h"
#include "gf_tee_internal_api.h"
#include "gf_log.h"
#include "gf_algo.h"
#include "cpl_string.h"
#include "gf_hw_common.h"
#include "gf_state.h"

#define LOG_TAG "[gf_irq_common]"

extern gf_config_t g_config;
extern gf_image_data_t *g_image_data;
extern uint32_t g_max_frame_num;

void gf_irq_common_reset(uint32_t irq_type) {
    gf_mode_t cur_mode = MODE_NONE;

    VOID_FUNC_ENTER();

    GF_LOGD(LOG_TAG "[%s] irq_type=%s, irq_type=0x%08x", __func__, gf_strirq(irq_type), irq_type);

    UNUSED_VAR(irq_type);

    if (g_config.require_down_and_up_in_pairs_for_image_mode > 0
            || g_config.require_down_and_up_in_pairs_for_ff_mode > 0
            || g_config.require_down_and_up_in_pairs_for_nav_mode > 0
            || g_config.require_down_and_up_in_pairs_for_key_mode > 0) {
        uint8_t reset_flag = 0;
        gf_state_require_down_and_up_in_pairs_end(&reset_flag);
    }
    cur_mode = gf_state_get_current_mode();

    if (MODE_NONE != cur_mode) {
        gf_hw_set_mode(cur_mode);
    }

    VOID_FUNC_EXIT();
}

gf_error_t irq_test_data_noise(gf_irq_t *irq) {
    gf_error_t err = GF_SUCCESS;
    static uint32_t cur_frame_num = 0;
    FUNC_ENTER();

    do {
        if (NULL == irq) {
            GF_LOGE(LOG_TAG "[%s] bad paramter, irq is NULL.", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        // GF_LOGD(LOG_TAG "[%s] irq_type=%s, irq_type_code=0x%08X", __func__,
        //        gf_strirq(irq->irq_type), irq->irq_type);

        if (irq->irq_type & GF_IRQ_IMAGE_MASK) {
            cur_frame_num++;
            //  GF_LOGD(LOG_TAG "[%s] g_cur_frame_num = %d g_max_frame_num = %d", __func__,
            //        g_cur_frame_num, g_max_frame_num);

            cpl_memcpy(irq->test_data_noise.raw_data, g_image_data->raw_data,
                       g_image_data->raw_data_len * sizeof(uint16_t));
            irq->test_data_noise.frame_num = cur_frame_num;

            if (cur_frame_num >= g_max_frame_num) {
                cur_frame_num = 0;
            }

            gf_hw_data_sample_resume();
        }
    } while (0);

    FUNC_EXIT(err);
    return err;
}

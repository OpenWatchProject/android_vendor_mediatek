/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#include "cpl_string.h"
#include "gf_fw_cfg.h"

#if defined(GF316M)
#include "gf316m_fw_cfg.h"
#elif defined(GF318M)
#include "gf318m_fw_cfg.h"
#elif defined(GF3118M)
#include "gf3118m_fw_cfg.h"
#elif defined(GF516M)
#include "gf516m_fw_cfg.h"
#elif defined(GF518M)
#include "gf518m_fw_cfg.h"
#elif defined(GF5118M)
#include "gf5118m_fw_cfg.h"
#elif defined(GF816M)
#include "gf816m_fw_cfg.h"
#endif

gf_oswego_m_fw_cfg_t g_oswego_m_fw_cfg = { { 0 } };

void gf_fw_cfg_init(void) {
    cpl_memset((void*) &g_oswego_m_fw_cfg, 0, sizeof(gf_oswego_m_fw_cfg_t));

    g_oswego_m_fw_cfg.fw_data = (uint8_t *)g_fw_data;
    g_oswego_m_fw_cfg.fw_data_len = sizeof(g_fw_data);

    {
        uint32_t i = 0;
        for (i = 0; i < OSWEGO_M_VENDOR_CFG_COUNT_MAX; i++) {
            g_oswego_m_fw_cfg.cfg_data_by_vendor_id[i] =
                    (gf_oswego_m_cfg_by_vendor_id_t *) & g_cfg_data_by_vendor_id[i];
        }

        for (i = 0; i < OSWEGO_M_PRODUCT_CFG_COUNT_MAX; i++) {
            g_oswego_m_fw_cfg.cfg_data_by_product_idx[i] =
                    (gf_oswego_m_cfg_by_product_idx_t *) & g_cfg_data_by_product_idx[i];
        }
    }

    g_oswego_m_fw_cfg.cfg_data_pixel_test = (uint8_t *)g_pixel_test_cfg_data;
    g_oswego_m_fw_cfg.cfg_data_len = GF_OSWEGO_M_CFG_LENGTH;
}


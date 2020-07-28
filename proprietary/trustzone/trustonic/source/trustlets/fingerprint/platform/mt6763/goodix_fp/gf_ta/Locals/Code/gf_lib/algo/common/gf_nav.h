/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#ifndef __GF_NAV_H__
#define __GF_NAV_H__

#include "gf_error.h"
#include "gf_common.h"

typedef void (*gf_nav_on_event)(gf_nav_code_t nav_code, uint8_t over_flag);

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \initialize the global variable in nav mode
 */
void gf_nav_init_global_variable(void);

/**
 * \brief Initialize the navigation module
 *
 * \param chip_series[in]  The current chip series
 *
 * \param chip_type[in]  The current chip type
 *
 * \param listener[in]     The callback of navigation result.
 *
 * \return If success, return #GF_SUCCESS otherwise return error code,eg:
 *  \li \c #GF_ERROR_BAD_PARAMS
 */
gf_error_t gf_nav_init(gf_chip_type_t chip_type,
        uint32_t nav_row, uint32_t nav_col, uint32_t nav_gap);

/**
 * \brief Destroy the navigation module.
 */
void gf_nav_destroy(void);

/**
 * \brief Set the navigation mode supported by algorithm.
 *
 * \param mode[in]  0: all direction 1: horizontal direction 2: vertical direction
 *
 * \return If success, return #GF_SUCCESS otherwise return error code,eg:
 *  \li \c #GF_ERROR_BAD_PARAMS
 */
gf_error_t gf_nav_set_mode(gf_nav_mode_t mode);

/**
 * \brief Set the navigation config supported by algorithm.
 *
 * \param config
 *
 * \return If success, return #GF_SUCCESS otherwise return error code,eg:
 *  \li \c #GF_ERROR_BAD_PARAMS
 */
gf_error_t gf_nav_set_config(gf_nav_config_t config);

/**
 * \brief Set the base data needed by algorithm
 *
 * \param buf[in]      The buffer pointer of base data.
 *
 * \param buf_len[in]  The buffer length of base data.
 *
 * \return If success, return #GF_SUCCESS otherwise return error code,eg:
 *  \li \c #GF_ERROR_BAD_PARAMS
 */
gf_error_t gf_nav_set_base(uint16_t *buf, uint32_t buf_len);

/**
 * \brief Get the navigation base raw data.
 *
 * \param buf[out]      The buffer pointer of base data.
 *
 * \param buf_len[out]  The buffer length of base data.
 *
 * \return If success, return #GF_SUCCESS otherwise return error code,eg:
 *  \li \c #GF_ERROR_BAD_PARAMS
 */
gf_error_t gf_nav_get_base(uint16_t **buf, uint32_t *buf_len);

/**
 * \brief Get the base set virable.
 *
 * \param status[out]   0: base not set, 1: base is set.
 *
 * \return If success, return #GF_SUCCESS otherwise return error code,eg:
 *  \li \c #GF_ERROR_BAD_PARAMS
 */
gf_error_t gf_nav_get_base_state(uint8_t *status);

/**
 * \brief Post a navigation unit to handling thread.
 *
 * \param buf[in]       the buffer of navigation data
 *
 * \param buf_len[in]   the length of navigation data
 *
 * \param is_over[in]   the flag of last frame of navigtaion data.
 *
 * \param frame_num[in] the count of frame of one buffer(oswego_m only)
 *
 * \return If success, return #GF_SUCCESS otherwise return error code,eg:
 *  \li \c #GF_ERROR_BAD_PARAMS
 */
gf_error_t gf_nav_detect(uint16_t *buf, uint32_t buf_len, uint8_t is_over, uint8_t frame_num,
        gf_nav_code_t *nav_code);

void gf_nav_stop(void);

#ifdef __cplusplus
}
#endif

#endif  // __GF_NAV_H__


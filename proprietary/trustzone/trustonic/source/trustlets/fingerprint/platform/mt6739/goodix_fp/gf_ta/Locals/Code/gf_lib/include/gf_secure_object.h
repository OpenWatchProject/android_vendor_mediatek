/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#ifndef __GF_SECURE_OBJECT_H__
#define __GF_SECURE_OBJECT_H__

#include "gf_error.h"

#define GF_CALIBRATION_OBJECT_ID        "gf_calibration.so"
#define GF_CALIBRATION_OBJECT_ID_BAK    "gf_calibration_bak.so"

#define GF_NAV_BASE_OBJECT_ID           "gf_nav_base.so"
#define GF_NAV_BASE_OBJECT_ID_BAK       "gf_nav_base_bak.so"

#define GF_FDT_BASE_OBJECT_ID           "gf_fdt_base.so"
#define GF_FDT_BASE_OBJECT_ID_BAK       "gf_fdt_base_bak.so"

#define GF_FINGER_OBJECT_ID             "finger_%d_%d.so"
#define GF_FINGER_OBJECT_ID_BAK         "finger_%d_%d_bak.so"

#define GF_AUTH_TOKEN_OBJECT_ID             "auth_token.so"
#define GF_AUTH_TOKEN_OBJECT_ID_BY_GROUP    "auth_token_%d.so"

#define GF_OTP_INFO_SECURE_ID           "gf_otp_info.so"

#define GF_FINGER_OBJECT_ID_MAX_LEN     64

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Read the specified secure object's content.
 *
 * \param name[in]  The secure object's identifier. Note: can not be NULL
 *
 * \param data[out] Point to the beginning address of the secure object's content. It is allocated by
 *                  this API, and should be free by caller.
 *
 * \param data_len[out] The length of the data.
 *
 * \return If success, return #GF_SUCCESS, and allocate memory for secure object's content, and the caller
 *  should release the memory(#data). otherwise return error code, eg:
 *  \li \c #GF_ERROR_BAD_PARAMS
 *  \li \c #GF_ERROR_OPEN_SECURE_OBJECT_FAILED
 *  \li \c #GF_ERROR_OUT_OF_MEMORY
 *  \li \c #GF_ERROR_READ_SECURE_OBJECT_FAILED
 */
gf_error_t gf_so_load_persistent_object(int8_t *name, uint8_t **data, uint32_t *data_len);

/**
 * \brief Save data into persistent object.
 *
 * \param name[in] The identifier of secure object.
 *
 * \param data[in]  The beginning address of data.
 *
 * \param data_len[in] The length of data.
 *
 * \return If success,return #GF_SUCCESS,otherwise return error code, eg:
 *  \li \c #GF_ERROR_BAD_PARAMS
 *  \li \c #GF_ERROR_OPEN_SECURE_OBJECT_FAILED
 *  \li \c #GF_ERROR_WRITE_SECURE_OBJECT_FAILED
 */
gf_error_t gf_so_save_persistent_object(int8_t *name, uint8_t *data, uint32_t data_len);

/**
 * \brief Delete the specified persistent object from secure storage.
 *
 * \param name[in] The identifier of persistent object
 *
 * \return If success, return #GF_SUCCESS, otherwise return error code, eg:
 *  \li \c #GF_ERROR_BAD_PARAMS
 */
gf_error_t gf_so_delete_persistent_object(int8_t *name);


gf_error_t gf_so_get_auth_token_pdu(uint8_t *buf, uint32_t buf_len, uint64_t secure_user_id,
        uint64_t authenticator_id);
gf_error_t gf_so_load_auth_token(uint64_t *secure_user_id, uint64_t *authenticator_id,
        uint8_t *buf, uint32_t buf_len);
gf_error_t gf_so_get_auth_token_len(uint32_t *len);

#ifdef __cplusplus
}
#endif

#endif  // __GF_SECURE_OBJECT_H__

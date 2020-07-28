/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#ifndef __GF_TEE_STORAGE_H__
#define __GF_TEE_STORAGE_H__

#include "gf_error.h"

typedef void* GF_TEE_HANDLE;
#define GF_TEE_HANDLE_NULL  (0)

/**
 * may:
 *  SO_MODE_READ
 *  SO_MODE_WRITE
 *  SO_MODE_READ | SO_MODE_WRITE
 */
typedef enum gf_tee_mode {
    SO_MODE_READ = 1,  // < Opens for reading, If the so does not exist or can not be found,
                      // < then #gf_tee_open_object call fails.
    SO_MODE_WRITE = 2,  // < Opens an empty secure object for writing, If the given secure object
                        // < exists, its contents are destroyed.
} gf_tee_mode_t;

#ifdef __cplusplus
extern "C" {
#endif

#define GF_LOG_BUFFER_MAX_LEN   2048    ///< The max bytes of log buffer

/**
 * \brief Check whether the specified secure object exists.
 *
 * \param object_id[in] The secure object's identifier.Note:can not be empty or NULL, and must
 * be end with '\0'.
 *
 * \return An integer value of #gf_error_t, If secure object exist, then return #GF_SUCCESS,
 * otherwise, return error code, eg:
 *  \li \c #GF_SUCCESS
 *  \li \c #GF_ERROR_BAD_PARAMS
 *  \li \c #GF_ERROR_SECURE_OBJECT_NOT_EXIST
 */
gf_error_t gf_tee_object_exist(uint8_t *object_id);

/**
 * \brief Opens a handle on an existing secure object. It returns a handle that
 * can be used to access the so's attributes and data stream.
 *
 * \param object_id[in]
 * The object identifier. Note that this buffer can not be empty string, and must
 * be end with '\0'.
 *
 * \param mode[in] Kind of access that's enabled.
 *
 * \param so_handle[out]
 * A pointer to the handle, which contains the opened handle upon successful completion.
 * If this function fails for any reason, the value pointed to by object is set to NULL.
 * When the object handle is no longer required, it MUST be closed using a call to the
 * #gf_tee_close_object.
 *
 * \return
 *  \li \c #GF_SUCCESS
 *  \li \c #GF_ERROR_BAD_PARAMS
 *  \li \c #GF_ERROR_OPEN_SECURE_OBJECT_FAILED
 */
gf_error_t gf_tee_open_object(uint8_t *object_id, gf_tee_mode_t mode, GF_TEE_HANDLE *so_handle);

/**
 * \brief Get the data stream's size of a secure object.
 *
 * \param so_handle[in]
 * The secure object handle.
 *
 * \param object_size[out]
 * The the data stream's size of a secure object.
 *
 * \return
 *  \li \c #GF_SUCCESS
 *  \li \c #GF_ERROR_BAD_PARAMS
 */
gf_error_t gf_tee_get_object_size(GF_TEE_HANDLE so_handle, uint32_t *object_size);

/**
 * \brief Attempts to read size bytes from the data stream associated with the so_handle
 * secure object into the buffer pointed to by buffer.
 *
 * \param so_handle[in]
 * The secure object handle.
 *
 * \param buffer[out]
 * A pointer to the memory which, upon successful completion, contains the bytes read.
 *
 * \param size[in]
 * The number of bytes to read
 *
 * \return
 * \li \c #GF_SUCCESS
 * \li \c #GF_ERROR_BAD_PARAMS
 * \li \c #GF_ERROR_READ_SECURE_OBJECT_FAILED
 */
gf_error_t gf_tee_read_object_data(GF_TEE_HANDLE so_handle, uint8_t *buffer, uint32_t size);

/**
 * \brief Writes size bytes from the buffer pointed to by buffer to the data stream associated
 * with the open object handle so_handle.
 *
 * \param so_handle[in]
 * The secure object handle.
 *
 * \param buffer[in]
 * The buffer containing the data to be written.
 *
 * \param size[in]
 * The number of bytes to write.
 *
 * \return
 * \li \c #GF_SUCESS
 * \li \c #GF_ERROR_BAD_PARAMS
 * \li \c #GF_ERROR_WRITE_SECURE_OBJECT_FAILED
 */

#if (defined __TEE_NUTLET) || (defined __TEE_TRUSTKERNEL)
gf_error_t gf_tee_write_object_data(uint8_t *object_id, GF_TEE_HANDLE *so_handle, uint8_t *buffer,
        uint32_t size);
#else
gf_error_t gf_tee_write_object_data(GF_TEE_HANDLE so_handle, uint8_t *buffer, uint32_t size);
#endif

/**
 * \brief Delete the specified secure object if it exist. If it isn't exist, then nothing
 to do.
 *
 * \param object_id[in]
 * The secure object identifier.
 *
 * \return
 * \li \c #GF_SUCCESS
 * \li \c #GF_ERROR_BAD_PARAMS
 */
gf_error_t gf_tee_delete_object(uint8_t *object_id);

/**
 * \brief Closes an opened object handle.
 *
 * \param so_handle[in]
 * Handle on the secure object to close. If set to NULL, does nothing.
 */
void gf_tee_close_object(GF_TEE_HANDLE so_handle);

#ifdef __cplusplus
}
#endif

#endif  // __GF_TEE_STORAGE_H__

/*
 * Copyright (c) 2013-2016 TRUSTONIC LIMITED
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the TRUSTONIC LIMITED nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __TLTEEKEYMASTER_API_H__
#define __TLTEEKEYMASTER_API_H__

#include "tci.h"
#ifdef SOTER
#include "mtk_keymaster_defs.h"
#else
#include "hardware/keymaster_defs.h"
#endif



/**
 * Command ID's
 */
#define CMD_ID_TEE_ADD_RNG_ENTROPY         0x01
#define CMD_ID_TEE_GENERATE_KEY            0x02
#define CMD_ID_TEE_GET_KEY_CHARACTERISTICS 0x03
#define CMD_ID_TEE_IMPORT_KEY              0x04
#define CMD_ID_TEE_EXPORT_KEY              0x05
#define CMD_ID_TEE_BEGIN                   0x06
#define CMD_ID_TEE_UPDATE                  0x07
#define CMD_ID_TEE_FINISH                  0x08
#define CMD_ID_TEE_ABORT                   0x09
#ifdef SOTER
/* Additional SOTER COMMAND */
#define CMD_ID_TEE_VERIFY_ATTK             0x51
#define CMD_ID_TEE_GENERATE_ATTK           0x52
#define CMD_ID_TEE_EXPORT_ATTK             0x53
#define CMD_ID_TEE_SOTER_EXPORT            0x54
#define CMD_ID_TEE_GET_DEVICEID            0x55
#endif

// for internal use:
#define CMD_ID_TEE_GET_KEY_INFO          0x0101
#define CMD_ID_TEE_GET_OPERATION_INFO    0x0102
/* ... add more command ids when needed */

/*
+ KEY FORMATS

We define here the serialized key formats used in the code.

All uint32_t are serialized in little-endian format.

++ Key Blob

This is the 'key_material' part of a keymaster_key_blob_t:

- key_blob = IV (16 bytes) | ciphertext | tag (16 bytes)

where 'ciphertext' is the AES-GCM encryption of the key material (defined next),
'IV' is the initialization vector and 'tag' is the authentication tag.

++ Key Material

This comprises:

- key_material = params_len (4 bytes) | params | key_data

where 'params_len' is a uint32_t representing the length of 'params', 'params'
is the serialized representation of the Keymaster key parameters (see
serialization.h for documentation of this), and 'key_data' is defined next.

++ Key Data

This comprises:

- key_data = key_type | key_size | core_key_data

where 'key_type' is a uint32_t corresponding to a keymaster_algorithm_t,
'key_size' is a uint32_t representing the key size in bits (the exact meaning
of 'key size' being key-type-dependent), and 'core_key_data' is defined next.

++ Core Key Data

This comprises:

- core_key_data = key_metadata | raw_key_data

where 'key_metadata' and 'raw_key_data' are defined below.

++ Key Metadata

This is key-type-dependent.

For AES and HMAC keys, it is empty.

For RSA keys, it comprises:

- rsa_metadata = keysize(bits) | n_len | e_len | d_len | p_len | q_len | dp_len | dq_len | qinv_len

where all lengths are uint32_t and (except for keysize) measured in bytes.

For exported RSA public keys, it comprises:

- rsa_pub_metadata = keysize(bits) | n_len | e_len

For EC keys, it comprises:

- ec_metadata = curve | x_len | y_len | d_len

where these are all uint32_t, 'curve' represents curve type and lengths are in
bytes.

For exported EC public keys, it comprises:

- ec_pub_metadata = curve | x_len | y_len

++ Raw Key Data

This is key-type-dependent.

For AES and HMAC keys, it comprises the key bytes.

For RSA keys, it comprises:

- n | e | d | p | q | dp | dq | qinv

with lengths as specified in the key metadata, and all numbers big-endian.

For exported RSA public keys, it comprises:

- n | e

For EC keys, it comprises:

- x | y | d

with lengths as specified in the key metadata, and all numbers big-endian.

For exported EC public keys, it comprises:

- x | y
*/

#define KM_RSA_METADATA_SIZE (9*4)
#define KM_EC_METADATA_SIZE (4*4)

typedef struct {
    uint32_t data; /**< secure address */
    uint32_t data_length; /**< data length */
} data_blob_t;

/**
 * Command message.
 *
 * @param len Length of the data to process.
 * @param data Data to be processed
 */
typedef struct __attribute__((packed)) {
    tciCommandHeader_t header; /**< Command header */
    uint32_t len; /**< Length of data to process */
} command_t;


/**
 * Response structure
 */
typedef struct __attribute__((packed)) {
    tciResponseHeader_t header; /**< Response header */
    uint32_t len;
} response_t;


/**
 * add_rng_entropy data structure
 */
typedef struct __attribute__((packed)) {
    data_blob_t rng_data; /**< [in] random data to be mixed in */
} add_rng_entropy_t;


/**
 * generate_key data structure
 */
typedef struct __attribute__((packed)) {
    data_blob_t params; /**< [in] serialized */
    data_blob_t key_blob; /**< [out] keymaster_key_blob_t */
    data_blob_t characteristics; /**< [out] serialized */
} generate_key_t;


/**
 * get_key_characteristics data structure
 */
typedef struct __attribute__((packed)) {
    data_blob_t key_blob; /**< [in] keymaster_key_blob_t */
    data_blob_t client_id; /**< [in] */
    data_blob_t app_data; /**< [in] */
    data_blob_t characteristics; /**< [out] serialized */
} get_key_characteristics_t;


/**
 * import_key data structure
 */
typedef struct __attribute__((packed)) {
    data_blob_t params; /**< [in] serialized */
    data_blob_t key_data; /**< [in] km_key_data */
    data_blob_t key_blob; /**< [out] keymaster_key_blob_t */
    data_blob_t characteristics; /**< [out] serialized */
} import_key_t;


/**
 * export_key data structure
 */
typedef struct __attribute__((packed)) {
#ifdef SOTER
    keymaster_key_format_t export_format; /**< [in] keymaster_key_format_t */
#endif
    data_blob_t key_blob; /**< [in] keymaster_key_blob_t */
    data_blob_t client_id; /**< [in] */
    data_blob_t app_data; /**< [in] */
    data_blob_t key_data; /**< [out] km_key_data */
} export_key_t;


/**
 * begin data structure
 */
typedef struct __attribute__((packed)) {
    keymaster_operation_handle_t handle; /**< [out] */
    keymaster_purpose_t purpose; /**< [in] */
    data_blob_t params; /**< [in] serialized */
    data_blob_t key_blob; /**< [in] keymaster_key_blob_t */
    data_blob_t out_params; /**< [out] serialized */
} begin_t;


/**
 * update data structure
 */
typedef struct __attribute__((packed)) {
    keymaster_operation_handle_t handle; /**< [in] */
    data_blob_t params; /**< [in] serialized */
    data_blob_t input; /**< [in] */
    uint32_t input_consumed; /**< [out] */
    data_blob_t output; /**< [out] */
} update_t;


/**
 * finish data structure
 */
typedef struct __attribute__((packed)) {
    keymaster_operation_handle_t handle; /**< [in] */
    data_blob_t params; /**< [in] serialized */
    data_blob_t signature; /**< [in] */
    data_blob_t output; /**< [out] */
} finish_t;


/**
 * abort structure
 */
typedef struct __attribute__((packed)) {
    keymaster_operation_handle_t handle; /**< [in] */
} abort_t;


/**
 * get_key_info data structure
 */
typedef struct __attribute__((packed)) {
    data_blob_t key_blob; /**< [in] keymaster_key_blob_t */
    data_blob_t client_id; /**< [in] KM_TAG_APPLICATION_ID */
    data_blob_t app_data; /**< [in] KM_TAG_APPLICATION_DATA */
    uint32_t key_type; /**< [out] (keymaster_algorithm_t) */
    uint32_t key_size; /**< [out] bits */
} get_key_info_t;


/**
 * get_operation_info data structure
 */
typedef struct __attribute__((packed)) {
    keymaster_operation_handle_t handle; /**< [in] */
    uint32_t algorithm; /**< [out] key type of operation (keymaster_algorithm_t) */
    uint32_t data_length; /**< [out] upper bound for length in bytes of output from finish() */
} get_operation_info_t;


#ifdef SOTER
/**
 * verify_attk_key_pair data structure
 */
typedef struct {
    uint32_t data; /**< verify result */
} verify_attk_key_pair_t;
/**
 * generate_attk_key_t data structure
 */
typedef struct {
    data_blob_t params;     /**< [in] serialized */
    uint32_t copy_num;      /**< [in] copy number of ATTK */
    uint32_t key_blob_len;  /**< [in] key_material_len */
} generate_attk_key_t;
/**
 * export_soter_key data structure
 */
typedef struct {
    data_blob_t key_pair_blob; /**< [in] key pai keymaster_key_blob_t */
    data_blob_t key_blob; /**< [in] x509 pub key keymaster_key_blob_t */
    data_blob_t client_id; /**< [in] */
    data_blob_t app_data; /**< [in] */
    data_blob_t key_data; /**< [out] export soter json data */
} export_soter_key_t;
/**
 * get_device_id_t data structure
 */
typedef struct {
    data_blob_t suid;       /**< [out] suid */
} get_device_id_t;
#endif

/**
 * TCI message data.
 */
typedef struct __attribute__((packed)) {
    union {
        command_t command;
        response_t response;
    };

    union {
        add_rng_entropy_t   add_rng_entropy;
        generate_key_t      generate_key;
        get_key_characteristics_t get_key_characteristics;
        import_key_t        import_key;
        export_key_t        export_key;
        begin_t             begin;
        update_t            update;
        finish_t            finish;
        abort_t             abort;
        get_key_info_t      get_key_info;
        get_operation_info_t get_operation_info;
#ifdef SOTER
        verify_attk_key_pair_t  verify_attk_key_pair;
        generate_attk_key_t     generate_attk_key;
        export_soter_key_t      export_soter_key;
        get_device_id_t         get_device_id;
#endif
    };
} tciMessage_t, *tciMessage_ptr;


/**
 * Overall TCI structure.
 */
typedef struct __attribute__((packed)) {
    tciMessage_t message; /**< TCI message */
} tci_t;


/**
 * Trustlet UUID
 */
#define TEE_KEYMASTER_M_TA_UUID { { 7, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x4D } }


#endif // __TLTEEKEYMASTER_API_H__

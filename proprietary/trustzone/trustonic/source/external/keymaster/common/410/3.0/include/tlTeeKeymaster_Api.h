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

#include <api_keymaster_defs.h>

#include "keymaster_version.h"



/**
 * Command IDs
 */
#define CMD_MASK_ID                  0x0000ffffu
#define CMD_MASK_VERSION             0x001f0000u
#define CMD_SHIFT_VERSION                    16
#define CMD_MASK_RESERVED            0xffe00000u

// #define CMD_VERSION_TEE_KEYMASTER2   (2 << CMD_SHIFT_VERSION)
#define CMD_VERSION_TEE_KEYMASTER3   (3 << CMD_SHIFT_VERSION)

#if defined(KEYMASTER_WANTED_VERSION) && KEYMASTER_WANTED_VERSION <= 1
#define KEYMASTER_IMPLICIT_CMD_BITS        0
#elif KEYMASTER_WANTED_VERSION == 3
#define KEYMASTER_IMPLICIT_CMD_BITS CMD_VERSION_TEE_KEYMASTER3
#else
#error "KEYMASTER_WANTED_VERSION unknown or unset"
#endif

#define CMD_ID_TEE_ADD_RNG_ENTROPY         (0x01 | KEYMASTER_IMPLICIT_CMD_BITS)
#define CMD_ID_TEE_GENERATE_KEY            (0x02 | KEYMASTER_IMPLICIT_CMD_BITS)
#define CMD_ID_TEE_GET_KEY_CHARACTERISTICS (0x03 | KEYMASTER_IMPLICIT_CMD_BITS)
#define CMD_ID_TEE_IMPORT_KEY              (0x04 | KEYMASTER_IMPLICIT_CMD_BITS)
#define CMD_ID_TEE_EXPORT_KEY              (0x05 | KEYMASTER_IMPLICIT_CMD_BITS)
#define CMD_ID_TEE_BEGIN                   (0x06 | KEYMASTER_IMPLICIT_CMD_BITS)
#define CMD_ID_TEE_UPDATE                  (0x07 | KEYMASTER_IMPLICIT_CMD_BITS)
#define CMD_ID_TEE_FINISH                  (0x08 | KEYMASTER_IMPLICIT_CMD_BITS)
#define CMD_ID_TEE_ABORT                   (0x09 | KEYMASTER_IMPLICIT_CMD_BITS)
#define CMD_ID_TEE_CONFIGURE               (0x0a | KEYMASTER_IMPLICIT_CMD_BITS)
#define CMD_ID_TEE_UPGRADE_KEY             (0x0b | KEYMASTER_IMPLICIT_CMD_BITS)
#define CMD_ID_TEE_ATTEST_KEY              (0x0c | KEYMASTER_IMPLICIT_CMD_BITS)
#define CMD_ID_TEE_GET_HARDWARE_FEATURES   (0x0d | KEYMASTER_IMPLICIT_CMD_BITS)
#define CMD_ID_TEE_DESTROY_ATTESTATION_IDS (0x0e | KEYMASTER_IMPLICIT_CMD_BITS)

// for internal use by the Keymaster TLC:
#define CMD_ID_TEE_GET_KEY_INFO           (0x0101 | KEYMASTER_IMPLICIT_CMD_BITS)
#define CMD_ID_TEE_GET_OPERATION_INFO     (0x0102 | KEYMASTER_IMPLICIT_CMD_BITS)
#define CMD_ID_TEE_GET_VERSION            (0x0103 | KEYMASTER_IMPLICIT_CMD_BITS)
#define CMD_ID_TEE_SET_DEBUG_LIES         (0x0104 | KEYMASTER_IMPLICIT_CMD_BITS)
#define CMD_ID_TEE_GET_VERIFIED_BOOT_DATA (0x0105 | KEYMASTER_IMPLICIT_CMD_BITS)

// for use by the key provisioning agent:
#define CMD_ID_TEE_SET_ATTESTATION_DATA   (0x0201 | KEYMASTER_IMPLICIT_CMD_BITS)
#if !RPMB_SUPPORT
#define CMD_ID_TEE_LOAD_ATTESTATION_DATA  (0x0202 | KEYMASTER_IMPLICIT_CMD_BITS)
#endif
#define CMD_ID_TEE_CHECK_ATTESTATION_DATA (0x0203 | KEYMASTER_IMPLICIT_CMD_BITS)

/* ... add more command IDs when needed */

/*
+ KEY FORMATS

We define here the serialized key formats used in the code.

All uint32_t are serialized in little-endian format.

++ Key Blob

This is the 'key_material' part of a keymaster_key_blob_t, and contains an
IV, an payload encrypted using AES-GCM, and a tag.  There are now multiple
versions of this, and the details are complicated.  For the full picture, see
`util/keymaster_util.c'.

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

/* Versions and compatibility.
 *
 * This TA is used to support several versions of the TCI protocol.  Clients
 * running old protocols don't allocate enough TCI buffer to accommodate
 * extensions in later protocol versions.
 *
 * When you want to add new structures, arguments, or return values, here's how
 * you do it.
 *
 *   1. Allocate a new API version constant at the top of this file.  We'll call
 *      this new version `N' in what follows.
 *
 *   2. If you're editing the structure for an existing command, then find the
 *      latest version of the current structure, and copy it, calling the new
 *      version `CMD_vN_t'.  Then modify this version.  It's best to limit your
 *      edits to adding new members at the end of the structure if you can: see
 *      steps 4 and 7 below.  (This copying is not a maintenance disaster
 *      because nobody will ever edit the old structures.)
 *
 *   3. If you're adding new commands, just create new structures named
 *      `CMD_vN_t'.
 *
 *   4. Find the latest version of the `tciMessage_vM_t' structure, and copy it,
 *      naming the new one `tciMessage_vN'.  Modify this copied structure
 *      appropriately to refer to your new command structures added in steps 2
 *      and 3.  If you only added members to the ends of structures, then you
 *      can just switch the appropriate member's type to the new one.  If you
 *      rearranged things more drastically, then you'll have to keep the old one
 *      around: name it `CMD_vM'.  The structure MUST NOT get smaller with
 *      increasing version numbers.
 *
 *   5. Edit the `typedef' of `tciMessage_t' to refer to `tciMessage_vN_t' which
 *      you made in step 4.
 *
 *   6. Dig up `tlMain.c' and add a new entry to the table at the top so that it
 *      can check a TCI buffer size it's been given and set its version number
 *      cap appropriately.
 *
 *   7. Modify the handler functions in `tlCryptoHandler.c'.  If you only added
 *      new commands, and/or new members to the ends of existing structures,
 *      then this will be fairly straightforward.  Entirely new commands just
 *      need to check that they've been invoked with a sufficient version (if
 *      you don't do this then the TCI buffer might be too short and you'll
 *      overrun it).  If you added new members to an existing structure, then
 *      you can most likely add a piece to the end of the argument-reading code
 *      guarded by a version check which either reads the new arguments or picks
 *      some suitable defaults.  If you did something more complicated then
 *      you'll need to cope with that the hard way.
 */

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
} add_rng_entropy_v0_t;


/**
 * generate_key data structure
 */
typedef struct __attribute__((packed)) {
    data_blob_t params; /**< [in] serialized */
    data_blob_t key_blob; /**< [out] keymaster_key_blob_t */
    data_blob_t characteristics; /**< [out] serialized */
} generate_key_v0_t;


/**
 * get_key_characteristics data structure
 */
typedef struct __attribute__((packed)) {
    data_blob_t key_blob; /**< [in] keymaster_key_blob_t */
    data_blob_t client_id; /**< [in] */
    data_blob_t app_data; /**< [in] */
    data_blob_t characteristics; /**< [out] serialized */
} get_key_characteristics_v0_t;


/**
 * import_key data structure
 */
typedef struct __attribute__((packed)) {
    data_blob_t params; /**< [in] serialized */
    data_blob_t key_data; /**< [in] km_key_data */
    data_blob_t key_blob; /**< [out] keymaster_key_blob_t */
    data_blob_t characteristics; /**< [out] serialized */
} import_key_v0_t;


/**
 * export_key data structure
 */
typedef struct __attribute__((packed)) {
    data_blob_t key_blob; /**< [in] keymaster_key_blob_t */
    data_blob_t client_id; /**< [in] */
    data_blob_t app_data; /**< [in] */
    data_blob_t key_data; /**< [out] km_key_data */
} export_key_v0_t;


/**
 * upgrade_key data structure
 */
typedef struct __attribute__((packed)) {
    data_blob_t key_to_upgrade; /**< [in] keymaster_key_blob_t */
    data_blob_t upgrade_params; /**< [in] keymaster_key_param_set_t */
    data_blob_t upgraded_key; /**< [out] keymaster_key_blob_t */
} upgrade_key_v3_t;


/**
 * Maximum number of concurrent Keymaster operations
 *
 * This is relied on by both sides of the interface.  If it increases a lot,
 * or the limit disappears entirely, then both ends will need to be
 * rethought.
 */
#define MAX_SESSION_NUM 16

/**
 * begin data structure
 */
typedef struct __attribute__((packed)) {
    keymaster_operation_handle_t handle; /**< [out] */
    keymaster_purpose_t purpose; /**< [in] */
    data_blob_t params; /**< [in] serialized */
    data_blob_t key_blob; /**< [in] keymaster_key_blob_t */
    data_blob_t out_params; /**< [out] serialized */
} begin_v0_t;

typedef struct __attribute__((packed)) {
    keymaster_operation_handle_t handle; /**< [out] */
    keymaster_purpose_t purpose; /**< [in] */
    data_blob_t params; /**< [in] serialized */
    data_blob_t key_blob; /**< [in] keymaster_key_blob_t */
    data_blob_t out_params; /**< [out] serialized */
    // v3...
    uint32_t algorithm; /**< [out] key type of operation (keymaster_algorithm_t) */
    uint32_t final_length; /**< [out] upper bound for extra length in bytes of output from finish() */
} begin_v3_t;


/**
 * update data structure
 */
typedef struct __attribute__((packed)) {
    keymaster_operation_handle_t handle; /**< [in] */
    data_blob_t params; /**< [in] serialized */
    data_blob_t input; /**< [in] */
    uint32_t input_consumed; /**< [out] */
    data_blob_t output; /**< [out] */
} update_v0_t;


/**
 * finish data structure
 */
typedef struct __attribute__((packed)) {
    keymaster_operation_handle_t handle; /**< [in] */
    data_blob_t params; /**< [in] serialized */
    data_blob_t signature; /**< [in] */
    data_blob_t output; /**< [out] */
} finish_v0_t;

typedef struct __attribute__((packed)) {
    keymaster_operation_handle_t handle; /**< [in] */
    data_blob_t params; /**< [in] serialized */
    data_blob_t signature; /**< [in] */
    data_blob_t output; /**< [out] */
    // v3...
    data_blob_t input; /**< [in] */
} finish_v3_t;


/**
 * abort structure
 */
typedef struct __attribute__((packed)) {
    keymaster_operation_handle_t handle; /**< [in] */
} abort_v0_t;


/**
 * configure structure
 */
typedef struct __attribute__((packed)) {
    data_blob_t params; /**< [in] serialized */
} configure_v3_t;


/**
 * attest_key structure
 */
typedef struct __attribute__((packed)) {
    data_blob_t key_to_attest; /**< [in] keymaster_key_blob_t */
    data_blob_t attest_params; /**< [in] serialized */
    data_blob_t cert_chain; /**< [out] serialized */
} attest_key_v3_t;


/**
 * get_hardware_features structure
 */
typedef struct __attribute__((packed)) {
    bool is_secure; /**< [out] keys are stored in secure hardware and never leave it */
    bool supports_elliptic_curve; /**< [out] hardware supports ECC with P224, P256, P384 and P521 */
    bool supports_symmetric_cryptography; /**< [out] hardware supports AES and HMAC */
    bool supports_attestation; /**< [out] hardware supports generation of public key attestation certificates */
} get_hardware_features_v3_t;


/**
 * get_key_info data structure
 */
typedef struct __attribute__((packed)) {
    data_blob_t key_blob; /**< [in] keymaster_key_blob_t */
    data_blob_t client_id; /**< [in] KM_TAG_APPLICATION_ID */
    data_blob_t app_data; /**< [in] KM_TAG_APPLICATION_DATA */
    uint32_t key_type; /**< [out] (keymaster_algorithm_t) */
    uint32_t key_size; /**< [out] bits */
} get_key_info_v0_t;


/**
 * get_operation_info data structure
 */
typedef struct __attribute__((packed)) {
    keymaster_operation_handle_t handle; /**< [in] */
    uint32_t algorithm; /**< [out] key type of operation (keymaster_algorithm_t) */
    uint32_t data_length; /**< [out] upper bound for length in bytes of output from finish() */
} get_operation_info_v0_t;


/**
 * get_version data structure
 */
typedef struct __attribute__((packed)) {
    keymaster_version_t version; /**< [out] version number  */
} get_version_t;


/**
 * set_debug_lies structure
 *
 * This command is not recognized in a non-debug build of the TA.
 */
typedef struct __attribute__((packed)) {
    uint32_t os_version; /**< [in] */
    uint32_t os_patchlevel; /**< [in] */
} set_debug_lies_v3_t;

/**
 * set_attestation_data data structure
 */
typedef struct __attribute__((packed)) {
    data_blob_t data; /**< [in] attestation data (format is OEM-specific) */
#if !RPMB_SUPPORT
    data_blob_t blob; /**< [out] encrypted attestation data (Secure Object) */
#endif
} set_attestation_data_v3_t;

#if !RPMB_SUPPORT
/**
 * load_attestation_data data structure
 */
typedef struct __attribute__((packed)) {
    data_blob_t data; /**< [in] attestation data (format is OEM-specific) */
} load_attestation_data_v3_t;
#endif

/**
 * check_attestation_data data structure
 */
typedef struct __attribute__((packed)) {
    data_blob_t data; /**< [in] attestation data (format is OEM-specific) */
#if !RPMB_SUPPORT
    data_blob_t blob; /**< [in] encrypted attestation data (Secure Object) */
#endif
} check_attestation_data_v3_t;

/**
 * TCI message data.
 */
typedef struct __attribute__((packed)) {
    union {
        command_t command;
        response_t response;
    };

    union {
        add_rng_entropy_v0_t   add_rng_entropy;
        generate_key_v0_t      generate_key;
        get_key_characteristics_v0_t get_key_characteristics;
        import_key_v0_t        import_key;
        export_key_v0_t        export_key;
        begin_v0_t             begin;
        update_v0_t            update;
        finish_v0_t            finish;
        abort_v0_t             abort;
        get_key_info_v0_t      get_key_info;
        get_operation_info_v0_t get_operation_info;
        get_version_t          get_version;
    };
} tciMessage_v0_t;

typedef struct __attribute__((packed)) {
    union {
        command_t command;
        response_t response;
    };

    union {
        add_rng_entropy_v0_t   add_rng_entropy;
        generate_key_v0_t      generate_key;
        get_key_characteristics_v0_t get_key_characteristics;
        import_key_v0_t        import_key;
        export_key_v0_t        export_key;
        begin_v3_t             begin;
        update_v0_t            update;
        finish_v3_t            finish;
        abort_v0_t             abort;
        configure_v3_t         configure;
        upgrade_key_v3_t       upgrade_key;
        attest_key_v3_t        attest_key;
        get_hardware_features_v3_t get_hardware_features;
        get_key_info_v0_t      get_key_info;
        get_operation_info_v0_t get_operation_info;
        get_version_t          get_version;
        set_debug_lies_v3_t    set_debug_lies;
        set_attestation_data_v3_t set_attestation_data;
#if !RPMB_SUPPORT
        load_attestation_data_v3_t load_attestation_data;
#endif
        check_attestation_data_v3_t check_attestation_data;
    };
} tciMessage_v3_t;

/* Define the TCI message format for the requested version.  Here, version 0 is
 * special, and means to use the latest version; this is usually used by the TA,
 * which implements all supported versions.
 */
#if KEYMASTER_WANTED_VERSION == 1
  typedef tciMessage_v0_t tciMessage_t, *tciMessage_ptr;
#elif KEYMASTER_WANTED_VERSION == 3 || KEYMASTER_WANTED_VERSION == 0
  typedef tciMessage_v3_t tciMessage_t, *tciMessage_ptr;
#endif

/**
 * Overall TCI structure.
 */
typedef struct __attribute__((packed)) {
    tciMessage_t message; /**< TCI message */
} tci_t;


/**
 * Trustlet UUID
 */
#define TEE_KEYMASTER_TA_UUID { { 7, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x4D } }
#define TEE_KEYMASTER_M_TA_UUID TEE_KEYMASTER_TA_UUID
#define TEE_KEYMASTER_N_TA_UUID TEE_KEYMASTER_TA_UUID


#endif // __TLTEEKEYMASTER_API_H__

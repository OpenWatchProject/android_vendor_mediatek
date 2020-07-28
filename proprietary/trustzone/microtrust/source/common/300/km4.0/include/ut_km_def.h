/*
 * Copyright (c) 2015-2017 MICROTRUST Incorporated
 * All rights reserved
 *
 * This file and software is confidential and proprietary to MICROTRUST Inc.
 * Unauthorized copying of this file and software is strictly prohibited.
 * You MUST NOT disclose this file and software unless you get a license
 * agreement from MICROTRUST Incorporated.
 */

#ifndef __UT_KEYMASTER_DEF_H__
#define __UT_KEYMASTER_DEF_H__
/**
 * Command ID's
 */
#define CMD_ID_TEE_RSA_GEN_KEY_PAIR 1
#define CMD_ID_TEE_RSA_SIGN 2
#define CMD_ID_TEE_RSA_VERIFY 3
#define CMD_ID_TEE_KEY_IMPORT 4
#define CMD_ID_TEE_GET_PUB_KEY 5
#define CMD_ID_TEE_DSA_GEN_KEY_PAIR 6
#define CMD_ID_TEE_DSA_SIGN 7
#define CMD_ID_TEE_DSA_VERIFY 8
#define CMD_ID_TEE_ECDSA_GEN_KEY_PAIR 9
#define CMD_ID_TEE_ECDSA_SIGN 10
#define CMD_ID_TEE_ECDSA_VERIFY 11
#define CMD_ID_TEE_GET_KEY_INFO 12

#define CMD_ID_TEE_GENERATE_KEY 13
#define CMD_ID_TEE_GET_SUPPORTED_ALGO 14
#define CMD_ID_TEE_GET_SUPPORTED_BLOCK 15
#define CMD_ID_TEE_GET_SUPPORTED_PADDING 16
#define CMD_ID_TEE_GET_SUPPORTED_DIGESTS 17
#define CMD_ID_TEE_GET_SUPPORTED_IMPORT_FORMATS 18
#define CMD_ID_TEE_GET_SUPPORTED_EXPORT_FORMATS 19
#define CMD_ID_TEE_GET_KEY_CHARACTERISTICS 20
#define CMD_ID_TEE_IMPORT_KEY 21
#define CMD_ID_TEE_EXPORT_KEY 22
#define CMD_ID_TEE_DELETE_KEY 23
#define CMD_ID_TEE_DELETE_ALL_KEYS 24
#define CMD_ID_TEE_BEGIN 25
#define CMD_ID_TEE_UPDATE 26
#define CMD_ID_TEE_FINISH 27
#define CMD_ID_TEE_ABORT 28
#define CMD_ID_GET_HMAC_KEY 29

/*for gatekeeper*/
#define CMD_ID_TEE_GENERATE_AUTOTOKEN 30
#define CMD_ID_TEE_GENERATE_PASSWORD_HANDLE 31
/*... add more command ids when needed */

#define CMD_ID_TEE_ADD_RNG_ENTROPY 201
#define CMD_ID_TEE_AGREE_KEY 202
#define CMD_ID_TEE_ATTEST_KEY 203
#define CMD_ID_TEE_UPGRADE_KEY 204
#define CMD_ID_TEE_CONFIGURE 205


#define CMD_ID_TEE_GENERATE_ATTK 501
#define CMD_ID_TEE_VERIFY_ATTK 502
#define CMD_ID_TEE_EXPORT_ATTK 503
#define CMD_ID_TEE_GET_DEVICE_ID 504
#define CMD_ID_TEE_GET_WECHAT_INFO 505

#define CMD_ID_TEE_REE_IMPORT_KEYBOX 601
#define CMD_ID_TEE_REE_CHECK_KEYBOX 602
/*... add more curves when needed */

#define KM_MAGIC 0xaf015838

typedef uint64_t secure_id_t;
typedef uint64_t salt_t;
typedef unsigned char uint8_t;


typedef enum {
    CMD_CA_IMPORT_KEYBOX = 0,
    CMD_CA_VERIFY_KEYBOX,
    CMD_CA_GENERATE_ATTK,
    CMD_CA_VERIFY_ATTK,
    CMD_CA_GET_UUID,
    CMD_CA_EXPORT_ATTK,
    CMD_CA_GET_WECHAT_INFO,


    CMD_TA_GET_KEYBOX = 100,
    CMD_TA_GET_KEYBOX_STATUS,

    CMD_TA_GET_UUID = 200,
    CMD_TA_GET_PRI_ATTK,

    CMD_CA_TEST = 1000,

} kb_cmd_t;
/**
 * Command message.
 *
 * @param len Length of the data to process.
 * @param data Data to be processed
 */
typedef struct { uint32_t commandId; } command_t;
/**
 * Response structure
 */
typedef struct { int32_t error; } response_t;

typedef struct {
    uint32_t in_params_length;
    uint32_t all_block_length;
    uint32_t hw_params_length;
    uint32_t sw_params_length;
    uint32_t key_material_size;
    uint32_t android_version;
    uint32_t km_version;
    uint32_t reserve[2];
    uint8_t name[12];
} generate_key_t;

typedef struct { uint32_t algorithms_length; } get_supported_algo_t;

typedef struct {
    uint32_t algo;
    uint32_t purpose;
    uint32_t modes_length;
} get_supported_block_t;

typedef struct {
    uint32_t algo;
    uint32_t purpose;
    uint32_t modes_length;
} get_supported_padding_t;

typedef struct {
    uint32_t algo;
    uint32_t purpose;
    uint32_t digests_length;
} get_supported_digests_t;

typedef struct {
    uint32_t algo;
    uint32_t formats_length;
} get_import_formats_t;

typedef struct {
    uint32_t algo;
    uint32_t formats_length;
} get_export_formats_t;

typedef struct {
    uint32_t key_material_size;
    uint32_t client_id_data_length;
    uint32_t app_data_length;
    uint32_t hw_length;
    uint32_t sw_length;
    uint32_t android_version;
    uint32_t km_version;
    uint32_t reserve[2];
    uint8_t name[12];
} get_key_character_t;

typedef struct {
    uint32_t params_length;
    uint32_t key_format;
    uint32_t key_data_length;
    uint32_t key_material_size;
    uint32_t hw_length;
    uint32_t sw_length;
    uint32_t android_version;
    uint32_t km_version;
    uint32_t reserve[2];
    uint8_t name[12];
} import_key_t;

typedef struct {
    uint32_t export_format;
    uint32_t export_data_length;
    uint32_t app_data_length;
    uint32_t client_id_data_length;
    uint32_t key_material_size;
    uint32_t android_version;
    uint32_t km_version;
    uint32_t reserve[2];
    uint8_t name[12];
} export_key_t;

typedef struct {
    uint32_t key_length;
    uint32_t android_version;
    uint32_t km_version;
    uint32_t reserve[2];
    uint8_t name[12];
} delete_key_t;

typedef struct {
    uint32_t purpose;
    uint32_t key_size;
    uint32_t in_params_count;
    uint32_t out_params_count;
    uint64_t operation_handle;
    uint32_t android_version;
    uint32_t km_version;
    uint32_t reserve[2];
    uint8_t name[12];
} begin_t;

typedef struct {
    uint64_t operation_handle;
    uint32_t in_params_count;
    uint32_t input_size;
    uint32_t input_consumed;
    uint32_t out_params_count;
    uint32_t output_size;
    uint32_t android_version;
    uint32_t km_version;
    uint32_t reserve[2];
    uint8_t name[12];
} update_t;

typedef struct {
    uint64_t operation_handle;
    uint32_t in_params_count;
    uint32_t signature_size;
    uint32_t out_params_count;
    uint32_t output_size;
	uint32_t input_size;
    uint32_t android_version;
    uint32_t km_version;
    uint32_t reserve[2];
    uint8_t name[12];
} finisth_t;

typedef struct {
    uint64_t operation_handle;
    uint32_t android_version;
    uint32_t km_version;
    uint32_t reserve[2];
    uint8_t name[12];
} abort_t;

typedef struct { uint32_t key_len; } get_hmac_key_t;

typedef struct __attribute__((__packed__)) {
    uint8_t version;
    secure_id_t user_id;
    uint64_t flags;
    salt_t salt;
    uint32_t password_length;
    uint8_t password[20];  // more than 17
    uint8_t signature[32];
} for_signature_password_t;

typedef struct __attribute__((__packed__)) {
    uint8_t version;  // Current version is 0
    uint64_t challenge;
    uint64_t user_id;             // secure user ID, not Android user ID
    uint64_t authenticator_id;    // secure authenticator ID
    uint32_t authenticator_type;  // hw_authenticator_type_t, in network order
    uint64_t timestamp;           // in network order
    uint8_t hmac[32];
} for_generate_authtoken_t;


typedef struct{
	uint32_t key_material_size;
    uint32_t android_version;
    uint32_t km_version;
    uint32_t reserve[2];
    uint8_t name[12];
}configure_t;

typedef struct{
	uint32_t key_material_size;
	uint32_t in_params_length;
	uint32_t out_entry_count;
	uint32_t datalen[4];
    uint32_t android_version;
    uint32_t km_version;
    uint32_t reserve[2];
    uint8_t name[12];
}attest_key_t;

typedef struct{
	uint32_t key_material_size;
	uint32_t in_params_length;
	uint32_t out_keyname_length;
    uint32_t android_version;
    uint32_t km_version;
    uint32_t reserve[2];
    uint8_t name[12];
}upgrade_key_t;

typedef struct{
	uint32_t attest_keybox_len;
}import_attest_keybox_t;

typedef struct {
  uint32_t google_key_staus;
  uint32_t attk_status;
  uint32_t oemkey_status;
  uint32_t ifaa_key_status;
  uint32_t reserved0;
  uint32_t reserver1;
  uint32_t reserver2;
  uint32_t reserver3;
} key_status_t;


typedef struct {
  uint8_t copy_num;
  uint32_t device_len;
} generate_attk_t;

typedef struct { uint32_t idlen; } get_device_id_t;

typedef struct { uint32_t attklen; } export_attk_t;

typedef struct {
	uint32_t kmversion;
	uint32_t wechatversion;
	uint32_t supportwechat;
	uint32_t reserved;
} get_wechat_info_t;


/**
 * TCI message data.
 */
typedef struct {
    command_t command;
    response_t response;
    // if we get this param is 1, then we need to parse keymaster_key_param_t.block
    // also if we send a data, we need to check, if the keymaster_key_param_t data has a block type
    // param
    // if yes, we set this param 1, then add the buffer according to the rule you make,
    unsigned keymaster_key_param_t_has_block;
    union {
        generate_key_t generate_key;
        get_supported_algo_t get_supported_algo;
        get_supported_block_t get_supported_block;
        get_supported_padding_t get_supported_padding;
        get_supported_digests_t get_supported_digests;
        get_import_formats_t get_import_format;
        get_export_formats_t get_export_format;
        get_key_character_t get_key_character;
        delete_key_t delete_key;
        import_key_t import_key;
        export_key_t export_key;
        begin_t begin;
        update_t update;
        finisth_t finisth;
        abort_t abort;
        get_hmac_key_t get_hmac_key;
        for_signature_password_t for_signature_password;
        for_generate_authtoken_t for_generate_authtoken;
        configure_t configure;
        attest_key_t attest_key;
        upgrade_key_t upgrade_key;
        import_attest_keybox_t import_attest_keybox;
        generate_attk_t generate_attk;
        get_device_id_t get_device_id;
        export_attk_t export_attk;
        get_wechat_info_t wechat_info;
    };
} ut_message_t;

#endif  // __UT_KEYMASTER_DEF_H__

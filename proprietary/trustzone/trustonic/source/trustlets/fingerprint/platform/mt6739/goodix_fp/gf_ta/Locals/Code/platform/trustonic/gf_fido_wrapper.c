/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */

#include <taStd.h>
#include <tee_internal_api.h>
#include "get_key.h"
#include "gf_fido_wrapper.h"

#define LOG_TAG "[gf_fido_wrapper]"

#define FIDO_DEBUG 0

#define UUID_SIZE        16
#define PLAIN_DATA_LEN_SIZE  4
#define WRAP_LEN(len)    (((16/* UUID_SIZE */ + 4/* PLAIN_DATA_LEN_SIZE */ + len) / 16 + 1) * 16)
#define WRAPPING_KEY_LEN 16
#define AES_BLOCK_SIZE   16

/** Symmetric key structure. */
typedef struct {
    uint8_t *key; /**< Pointer to the key. */
    uint32_t len; /**< Byte length of the key. */
} SymKey_t;

static unsigned char TA_UUID[] = { 0x05, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00 };

static SymKey_t wrapping_key = { NULL, 0 };

/**
 *  Function fido_create_aeskey_128:
 *  Description:
 *      Create a AES key, key size is defined by WRAPPING_KEY_LEN (128 bits now)
 *
 */
static TEE_Result fido_create_aeskey_128(OUT TEE_ObjectHandle *phSecretKey) {
    uint32_t err = TEE_SUCCESS;
    TEE_Attribute attrs[1];
    TEE_ObjectHandle ohKey = TEE_HANDLE_NULL;
    uint8_t wrapping_key_data[WRAPPING_KEY_LEN] = {0};

    FUNC_ENTER();

    do {
        if (NULL == phSecretKey) {
            GF_LOGE(LOG_TAG "[%s] invalid parameter:phSecretKey is NULL.\n", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        err = get_key(wrapping_key_data, WRAPPING_KEY_LEN);
        if (TEE_SUCCESS != err) {
            GF_LOGE(LOG_TAG "%s get_key result FAIL (%x)", __func__, err);
            break;
        }
        wrapping_key.key = wrapping_key_data;
        wrapping_key.len = WRAPPING_KEY_LEN;

#if FIDO_DEBUG
        {
            uint32_t i = 0;
            GF_LOGD(LOG_TAG "[%s] wrapping_key_data len:%d.\n", __func__, WRAPPING_KEY_LEN);
            for (i = 0; i < WRAPPING_KEY_LEN; i++) {
                GF_LOGD(LOG_TAG "[%s] wrapping_key_data[%d]:0x%x.\n",
                        __func__, i, *(wrapping_key_data + i));
            }
        }
#endif
        TEE_InitRefAttribute(&attrs[0], TEE_ATTR_SECRET_VALUE,
                           (void *)wrapping_key.key, wrapping_key.len);
        err = TEE_AllocateTransientObject(TEE_TYPE_AES, wrapping_key.len * 8, &ohKey);
        if (err != TEE_SUCCESS) {
            GF_LOGE(LOG_TAG"TEE_AllocateTransientObject returned (0x%x).", err);
            break;
        }

        err = TEE_PopulateTransientObject(ohKey, attrs,
                                            sizeof(attrs) / sizeof(attrs[0]));
        if (err != TEE_SUCCESS) {
            GF_LOGE(LOG_TAG"TEE_PopulateTransientObject returned (0x%x).", err);
            TEE_FreeTransientObject(ohKey);
            break;
        }

        *phSecretKey = ohKey;
    } while (0);

    FUNC_EXIT(err);
    return err;
}

/**
 *  Function fido_encrypt_aes:
 *  Description:
 *           Perform an AES encryption on data.
 */
static TEE_Result fido_encrypt_aes(
    uint8_t *ciphertext,
    uint32_t *ciphertext_len,
    const uint8_t *plaintext,
    uint32_t plaintext_len,
    IN TEE_ObjectHandle hSecretKey) {

    TEE_Result err;
    TEE_OperationHandle operationHandle = 0;
    TEE_ObjectInfo KeyInfo = {0};
    FUNC_ENTER();

    do {
        TEE_GetObjectInfo(hSecretKey, &KeyInfo);
        GF_LOGD(LOG_TAG "%s: maxObjectSize = (%d).\n", __func__, KeyInfo.maxObjectSize);

        err = TEE_AllocateOperation(&operationHandle, TEE_ALG_AES_CBC_NOPAD ,
                TEE_MODE_ENCRYPT, KeyInfo.maxObjectSize);
        if (err != TEE_SUCCESS) {
            GF_LOGE(LOG_TAG "TEE_AllocateOperation() failed (%08x).", err);
            break;
        }

        err = TEE_SetOperationKey(operationHandle, hSecretKey);
        if (err != TEE_SUCCESS) {
            GF_LOGE(LOG_TAG"TEE_SetOperationKey() failed (%08x).", err);
            break;
        }

        TEE_CipherInit(operationHandle, NULL, 0);

        err = TEE_CipherDoFinal(
            operationHandle, (void *)plaintext,
            plaintext_len, ciphertext, (size_t *)ciphertext_len);
        if (err != TEE_SUCCESS) {
            GF_LOGE(LOG_TAG"TEE_CipherDoFinal() failed (%08x).", err);
            break;
        }
    } while (0);

    if (operationHandle != NULL) {
        TEE_FreeOperation(operationHandle);
    }

    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_fido_get_attestation_private_key(uint8_t* pAttKeyPrv) {
    UNUSED_VAR(pAttKeyPrv);
    return GF_SUCCESS;
}

gf_error_t gf_fido_buffer_is_non_secure_mem(const uint8_t* pBuffer, uint32_t nLen) {
    UNUSED_VAR(pBuffer);
    UNUSED_VAR(nLen);
    return GF_SUCCESS;
}

gf_error_t gf_fido_get_wrapped_len(uint32_t nLen) {
    UNUSED_VAR(nLen);
    return GF_SUCCESS;
}

/*
 *wrap object to other ta, add header front the data and encrypt in AES
 *the header is /--- uuid(16 bytes) --/-- plaindata len(4 bytes) --/
 *so the plain data before encrypt:/--- uuid(16 bytes) --/-- plaindata len(4 bytes) --/--
 *so plain data(plaindata len bytes) --/
 */
gf_error_t gf_fido_wrap_object_to_other_ta(const uint8_t* pInBuf, uint32_t nInLen, uint8_t* pOutBuf,
    uint32_t* pnOutLen) {
    gf_error_t err = GF_SUCCESS;
    uint8_t *wrap_buf = NULL;

    FUNC_ENTER();

    do {
        uint32_t wrap_len = 0;
        uint8_t *temp = NULL;
        TEE_ObjectHandle hSecretKeyAes = TEE_HANDLE_NULL;
        if (NULL == pInBuf) {
            GF_LOGE(LOG_TAG "[%s] invalid parameter:pInBuf is NULL.\n", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        if (NULL == pOutBuf) {
            GF_LOGE(LOG_TAG "[%s] invalid parameter:pOutBuf is NULL.\n", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        if (NULL == pOutBuf) {
            GF_LOGE(LOG_TAG "[%s] invalid parameter:pOutBuf is NULL.\n", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        wrap_len = WRAP_LEN(nInLen);
        GF_LOGD(LOG_TAG "[%s] len:%d, wrap_len:%d.\n", __func__, nInLen, wrap_len);
        if (wrap_len > *pnOutLen) {
            GF_LOGE(LOG_TAG "[%s] *pnOutLen:%d. is too short!\n", __func__, *pnOutLen);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        wrap_buf = TEE_Malloc(wrap_len, 0);
        if (NULL == wrap_buf) {
            GF_LOGE(LOG_TAG "[%s] out of memory when malloc wrap_buf.\n", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }
        // 1. wrap uuid
        temp = wrap_buf;
        memcpy(temp, TA_UUID, UUID_SIZE);
        // 2. wrap data len
        temp = temp + UUID_SIZE;
        memcpy(temp, &nInLen, PLAIN_DATA_LEN_SIZE);
        // 3. wrap data
        temp = temp + PLAIN_DATA_LEN_SIZE;
        memcpy(temp, pInBuf, nInLen);

#if FIDO_DEBUG
        {
            uint32_t i = 0;
            GF_LOGD(LOG_TAG "[%s] wrap data len:%d.\n", __func__, wrap_len);
            for (i = 0; i < wrap_len; i++) {
                GF_LOGD(LOG_TAG "[%s] wrap data[%d]:0x%x.\n", __func__, i, *(wrap_buf + i));
            }
        }
#endif
        err = fido_create_aeskey_128(&hSecretKeyAes);
        if (err != TEE_SUCCESS) {
            GF_LOGE(LOG_TAG "[%s] fido_create_aeskey_128 failed, ret:0x%x\n", __func__, err);
            break;
        }

        err = fido_encrypt_aes(pOutBuf, pnOutLen, wrap_buf, wrap_len, hSecretKeyAes);
        if (err != TEE_SUCCESS) {
            GF_LOGE(LOG_TAG "[%s] fido_encrypt_aes failed, ret:0x%x\n", __func__, err);
            break;
        }
#if FIDO_DEBUG
        {
            uint32_t i = 0;
            GF_LOGD(LOG_TAG "[%s] encrypt data len:%d.\n", __func__, *pnOutLen);
            for (i = 0; i < *pnOutLen; i++) {
                GF_LOGD(LOG_TAG "[%s] encrypt data[%d]:0x%x.\n", __func__, i, *(pOutBuf + i));
            }
        }
#endif
    } while (0);

    if (wrap_buf != NULL) {
        TEE_Free(wrap_buf);
    }

    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_fido_unwrap_object_to_other_ta(uint8_t* pDestTA, const uint8_t* pInBuf,
        uint32_t nInLen, uint8_t* pOutBuf, uint32_t* pnOutLen) {
    UNUSED_VAR(pDestTA);
    UNUSED_VAR(pInBuf);
    UNUSED_VAR(nInLen);
    UNUSED_VAR(pOutBuf);
    UNUSED_VAR(pnOutLen);
    return GF_SUCCESS;
}

gf_error_t gf_fido_wrap_object_to_self(const uint8_t* pInBuf, uint32_t nInLen, uint8_t* pOutBuf,
        uint32_t* pnOutLen) {
    UNUSED_VAR(pInBuf);
    UNUSED_VAR(nInLen);
    UNUSED_VAR(pOutBuf);
    UNUSED_VAR(pnOutLen);
    return GF_SUCCESS;
}

gf_error_t gf_fido_unwrap_object_to_self(const uint8_t* pInBuf, uint32_t nInLen, uint8_t* pOutBuf,
        uint32_t* pnOutLen) {
    UNUSED_VAR(pInBuf);
    UNUSED_VAR(nInLen);
    UNUSED_VAR(pOutBuf);
    UNUSED_VAR(pnOutLen);
    return GF_SUCCESS;
}

gf_error_t gf_fido_generate_random_data(uint32_t nRandDataLen, uint8_t* pRandData) {
    UNUSED_VAR(nRandDataLen);
    UNUSED_VAR(pRandData);
    return GF_SUCCESS;
}

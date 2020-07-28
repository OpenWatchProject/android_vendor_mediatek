/*
 * Copyright (C) 2010 MediaTek Inc.
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <errno.h>
#include <string.h>
#include <stdint.h>

#include <tlTeeKeymaster_log.h>
#include <tlTeeKeymaster_Api.h>
#include <tlcTeeKeymaster_if.h>

#include <hardware/hardware.h>
#include <hardware/keymaster0.h>

#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/rsa.h>
#include <openssl/err.h>
#include <openssl/x509.h>

#include <UniquePtr.h>

// For debugging
// #define LOG_NDEBUG 0

#include <cutils/log.h>

#define RSA_KEY_SO_SIZE 4096

struct BIGNUM_Delete {
    void operator()(BIGNUM* p) const {
        BN_free(p);
    }
};
typedef UniquePtr<BIGNUM, BIGNUM_Delete> Unique_BIGNUM;

struct EVP_PKEY_Delete {
    void operator()(EVP_PKEY* p) const {
        EVP_PKEY_free(p);
    }
};
typedef UniquePtr<EVP_PKEY, EVP_PKEY_Delete> Unique_EVP_PKEY;

struct PKCS8_PRIV_KEY_INFO_Delete {
    void operator()(PKCS8_PRIV_KEY_INFO* p) const {
        PKCS8_PRIV_KEY_INFO_free(p);
    }
};
typedef UniquePtr<PKCS8_PRIV_KEY_INFO, PKCS8_PRIV_KEY_INFO_Delete> Unique_PKCS8_PRIV_KEY_INFO;

struct DSA_Delete {
    void operator()(DSA* p) const {
        DSA_free(p);
    }
};
typedef UniquePtr<DSA, DSA_Delete> Unique_DSA;

struct EC_KEY_Delete {
    void operator()(EC_KEY* p) const {
        EC_KEY_free(p);
    }
};
typedef UniquePtr<EC_KEY, EC_KEY_Delete> Unique_EC_KEY;

struct EC_GROUP_Delete {
    void operator()(EC_GROUP* p) const {
        EC_GROUP_free(p);
    }
};
typedef UniquePtr<EC_GROUP, EC_GROUP_Delete> Unique_EC_GROUP;

struct RSA_Delete {
    void operator()(RSA* p) const {
        RSA_free(p);
    }
};
typedef UniquePtr<RSA, RSA_Delete> Unique_RSA;

struct Malloc_Free {
    void operator()(void* p) const {
        free(p);
    }
};

typedef UniquePtr<keymaster0_device_t> Unique_keymaster0_device_t;

/**
 * Many OpenSSL APIs take ownership of an argument on success but
 * don't free the argument on failure. This means we need to tell our
 * scoped pointers when we've transferred ownership, without
 * triggering a warning by not using the result of release().
 */
template <typename T, typename Delete_T>
inline void release_because_ownership_transferred(UniquePtr<T, Delete_T>& p) {
    T* val __attribute__((unused)) = p.release();
}

/*
 * Checks this thread's OpenSSL error queue and logs if
 * necessary.
 */
static void logOpenSSLError(const char* location) {
    int error = ERR_get_error();

    if (error != 0) {
        char message[256];
        ERR_error_string_n(error, message, sizeof(message));
        ALOGE("OpenSSL error in %s %d: %s", location, error, message);
    }

    ERR_clear_error();
    ERR_remove_state(0);
}


__attribute__((visibility("default"))) int mt_km_generate_keypair(
    const keymaster0_device_t*, const keymaster_keypair_t key_type, const void* key_params,
    uint8_t** keyBlob, size_t* keyBlobLength) {
    Unique_EVP_PKEY pkey(EVP_PKEY_new());
    teeResult_t ret = TEE_ERR_FAIL;

    ALOGD("mt_km_generate_keypair");

    if (pkey.get() == NULL) {
        logOpenSSLError("mt_km_generate_keypair");
        return -1;
    }

    if (key_type != TYPE_RSA) {
        ALOGE("RSA key type only.");
        return -1;
    }

    if (key_params == NULL) {
        ALOGW("key_params == null");
        return -1;
    }

    const keymaster_rsa_keygen_params_t* rsa_params =
        (const keymaster_rsa_keygen_params_t*)key_params;

    if (rsa_params->modulus_size == 512 || rsa_params->modulus_size == 1024 ||
        rsa_params->modulus_size == 2048 || rsa_params->modulus_size == 3072 || 
        rsa_params->modulus_size == 4096 ) {
        // Key size is supported
    } else {
        // Key size is not supported
        ALOGE("Not supported key size: %d", rsa_params->modulus_size);
        return -1;
    }

    UniquePtr<uint8_t> rsa_key(static_cast<uint8_t*>(malloc(RSA_KEY_SO_SIZE)));
    if (rsa_key.get() == NULL) {
        ALOGE("Could not allocate memory rsa_key");
        return -1;
    }

    *keyBlobLength = 0;
    ret = TEE_RSAGenerateKeyPair(TEE_KEYPAIR_RSACRT, rsa_key.get(), RSA_KEY_SO_SIZE,
				rsa_params->modulus_size, (uint32_t)rsa_params->public_exponent,
				(uint32_t*)keyBlobLength);
    if (ret != TEE_ERR_NONE) {
        ALOGE("TEE_RSAGenerateKeyPair() is failed: %d", ret);
        return -1;
    }

   *keyBlob = rsa_key.release();

    return 0;
}

__attribute__((visibility("default"))) int mt_km_import_keypair(const keymaster0_device_t*,
                                                                  const uint8_t* key,
                                                                  const size_t key_length,
                                                                  uint8_t** key_blob,
                                                                  size_t* key_blob_length) {
    uint8_t *mod = NULL;
    uint8_t *pub_exp = NULL;
    uint8_t *pri_exp = NULL;
    uint8_t *rsa_key_ptr = NULL;
    uint32_t offset = 0;
    uint32_t modlen = 0;
    uint32_t pub_exp_len = 0;
    uint32_t pri_exp_len = 0;
    uint32_t rsa_size = 0; 
    uint32_t rsa_key_len = 0;
    uint32_t rsa_key_so_len = 0;
    RSA *rsa_ptr = NULL;
    teeKeyMeta_t keymeta;
    teeResult_t ret = TEE_ERR_FAIL;
    
    ALOGD("mt_km_import_keypair");

    if (key == NULL) {
        ALOGW("input key == NULL");
        return -1;
    } else if (key_blob == NULL || key_blob_length == NULL) {
        ALOGW("output key blob or length == NULL");
        return -1;
    }

    Unique_PKCS8_PRIV_KEY_INFO pkcs8(d2i_PKCS8_PRIV_KEY_INFO(NULL, &key, key_length));
    if (pkcs8.get() == NULL) {
        logOpenSSLError("mt_km_import_keypair");
        return -1;
    }

    /* assign to EVP */
    Unique_EVP_PKEY pkey(EVP_PKCS82PKEY(pkcs8.get()));
    if (pkey.get() == NULL) {
        logOpenSSLError("mt_km_import_keypair");
        return -1;
    }
    release_because_ownership_transferred(pkcs8);

    /* Convert EVP_PKEY to RSA */
    Unique_RSA rsa(EVP_PKEY_get1_RSA(pkey.get()));
    if (rsa.get() == NULL) {
        logOpenSSLError("Convert EVP_PKEY to RSA failed!");
        return -1;
    }

    UniquePtr<uint8_t, Malloc_Free> rsa_key(static_cast<uint8_t*>(malloc(RSA_KEY_SO_SIZE)));
    if (rsa_key.get() == NULL) {
        ALOGE("Could not allocate memory for rsa_key");
        return -1;
    }

    UniquePtr<uint8_t, Malloc_Free> rsa_key_so(static_cast<uint8_t*>(malloc(RSA_KEY_SO_SIZE)));
    if (rsa_key_so.get() == NULL) {
        ALOGE("Could not allocate memory for rsa_key_so");
        return -1;
    }

    rsa_ptr = rsa.get();
    rsa_size = RSA_size(rsa_ptr);
    UniquePtr<uint8_t, Malloc_Free> buffer(static_cast<uint8_t*>(malloc(rsa_size*3)));
    if (buffer.get() == NULL) {
        ALOGE("Could not allocate memory for temp buffer");
        return -1;
    }

    mod = buffer.get();
    pub_exp = buffer.get() + rsa_size; 
    pri_exp = buffer.get() + rsa_size*2;

    modlen = BN_bn2bin(rsa_ptr->n, mod);
    pub_exp_len = BN_bn2bin(rsa_ptr->e, pub_exp);
    pri_exp_len = BN_bn2bin(rsa_ptr->d, pri_exp);

    memset(&keymeta, 0, sizeof(teeKeyMeta_t));

    if(modlen != 64 && modlen != 128 && modlen != 256) {
        ALOGE("Not supported key size = %d bits", modlen << 3);
        return -1;
    }
    keymeta.keytype = TEE_KEYTYPE_RSA;
    keymeta.rsakey.keysize = modlen << 3;
    keymeta.rsakey.lenpubmod = modlen;
    keymeta.rsakey.lenpubexp = pub_exp_len;
    keymeta.rsakey.type = TEE_KEYPAIR_RSA;
    keymeta.rsakey.rsapriv.lenpriexp = pri_exp_len;

    rsa_key_ptr = rsa_key.get();
    memcpy(rsa_key_ptr, &keymeta, sizeof(teeKeyMeta_t));
    offset += sizeof(teeKeyMeta_t);
    memcpy(rsa_key_ptr+offset, mod, modlen);
    offset += modlen;
    memcpy(rsa_key_ptr+offset, pub_exp, pub_exp_len);
    offset += pub_exp_len;
    memcpy(rsa_key_ptr+offset, pri_exp, pri_exp_len);

    rsa_key_len = sizeof(teeKeyMeta_t) +
                    modlen +
                    pub_exp_len +
                    pri_exp_len;
    rsa_key_so_len = RSA_KEY_SO_SIZE;

    ret = TEE_KeyImport(
            rsa_key_ptr,
            rsa_key_len,
            rsa_key_so.get(),
            &rsa_key_so_len);
    if (TEE_ERR_NONE != ret)
    {
        ALOGE("TEE_KeyImport failed: %d\n", ret);
        return -1;
    }

    *key_blob = rsa_key_so.release();
    *key_blob_length = rsa_key_so_len;

    return 0;
}

__attribute__((visibility("default"))) int mt_km_get_keypair_public(
    const struct keymaster0_device*, const uint8_t* key_blob, const size_t key_blob_length,
    uint8_t** x509_data, size_t* x509_data_length) {
    uint8_t *mod = NULL;
    uint8_t *pub_exp = NULL;
    uint32_t modlen = 0;
    uint32_t pub_exp_len = 0;
    uint32_t pub_key_len = 0;
    uint32_t rsa_size = 0;
    RSA *rsa_ptr = NULL;
    teeResult_t ret = TEE_ERR_FAIL;
    teePubKeyMeta_t *pub_meta = NULL;

    ALOGD("mt_km_get_keypair_public");

    if (x509_data == NULL || x509_data_length == NULL) {
        ALOGW("output public key buffer == NULL");
        return -1;
    }

    UniquePtr<uint8_t, Malloc_Free> pub_key(static_cast<uint8_t*>(malloc(1024)));
    if (pub_key.get() == NULL) {
        ALOGE("Could not allocate memory for pub_key");
        return -1;
    }
    memset(pub_key.get(), 0, 1024);
    pub_key_len = 1024;

    pub_meta = (teePubKeyMeta_t *)pub_key.get();
    ret = TEE_GetPubKey(key_blob, key_blob_length, pub_key.get(), &pub_key_len);
    if (TEE_ERR_NONE != ret) {
        ALOGE("TEE_GetPubKey failed: %d\n", ret);
        return -1;
    }

    rsa_size = pub_meta->rsakey.lenpubmod; 
    UniquePtr<uint8_t, Malloc_Free> buffer(static_cast<uint8_t*>(malloc(rsa_size*2)));
    if (buffer.get() == NULL) {
        ALOGE("Could not allocate memory for temp buffer");
        return -1;
    }

    mod = buffer.get();
    pub_exp = buffer.get() + rsa_size;

    modlen = pub_meta->rsakey.lenpubmod;
    pub_exp_len = pub_meta->rsakey.lenpubexp;
    memcpy(mod, pub_key.get() + sizeof(teePubKeyMeta_t), modlen);
    memcpy(pub_exp, pub_key.get() + sizeof(teePubKeyMeta_t) + modlen, 
        pub_exp_len);

    UniquePtr<BIGNUM, BIGNUM_Delete> bn_mod(BN_new());
    if (bn_mod.get() == NULL) {
        ALOGE("Could not allocate memory for bn_mod");
        return -1;
    }
    UniquePtr<BIGNUM, BIGNUM_Delete> bn_pub_exp(BN_new());
    if (bn_pub_exp.get() == NULL) {
        ALOGE("Could not allocate memory for bn_exp");
        return -1;
    }

    BN_bin2bn(mod, modlen, bn_mod.get());
    BN_bin2bn(pub_exp, pub_exp_len, bn_pub_exp.get());

    Unique_RSA rsa(RSA_new());
    if (rsa.get() == NULL) {
        logOpenSSLError("Could not allocate memory for rsa");
        return -1;
    }

    rsa_ptr = rsa.get();
    rsa_ptr->n = bn_mod.release();
    rsa_ptr->e = bn_pub_exp.release();

    Unique_EVP_PKEY pkey(EVP_PKEY_new());
    if (pkey.get() == NULL) {
        logOpenSSLError("Could not allocate pkey");
        return -1;
    }
    if (EVP_PKEY_assign_RSA(pkey.get(), rsa_ptr) == 0) {
        logOpenSSLError("Could not assign rsa to pkey");
        return -1;
    }
    release_because_ownership_transferred(rsa);

    int len = i2d_PUBKEY(pkey.get(), NULL);
    if (len <= 0) {
        logOpenSSLError("mt_km_get_keypair_public");
        return -1;
    }

    UniquePtr<uint8_t, Malloc_Free> key(static_cast<uint8_t*>(malloc(len)));
    if (key.get() == NULL) {
        ALOGE("Could not allocate memory for public key data");
        return -1;
    }

    unsigned char* tmp = reinterpret_cast<unsigned char*>(key.get());
    if (i2d_PUBKEY(pkey.get(), &tmp) != len) {
        logOpenSSLError("mt_km_get_keypair_public");
        return -1;
    }

    ALOGV("Length of x509 data is %d", len);
    *x509_data_length = len;
    *x509_data = key.release();

    return 0;
}

__attribute__((visibility("default"))) int mt_km_sign_data(
    const keymaster0_device_t*, const void* params, const uint8_t* keyBlob,
    const size_t keyBlobLength, const uint8_t* data, const size_t dataLength, uint8_t** signedData,
    size_t* signedDataLength) {
    teeResult_t ret = TEE_ERR_FAIL;
    uint8_t *msg = NULL;

    ALOGD("mt_km_sign_data");

    if (data == NULL) {
        ALOGW("input data to sign == NULL");
        return -1;
    } else if (signedData == NULL || signedDataLength == NULL) {
        ALOGW("output signature buffer == NULL");
        return -1;
    }

    const keymaster_rsa_sign_params_t* sign_params =
        reinterpret_cast<const keymaster_rsa_sign_params_t*>(params);
    if (sign_params->digest_type != DIGEST_NONE) {
        ALOGW("Cannot handle digest type %d", sign_params->digest_type);
        return -1;
    } else if (sign_params->padding_type != PADDING_NONE) {
        ALOGW("Cannot handle padding type %d", sign_params->padding_type);
        return -1;
    }

    UniquePtr<uint8_t> signed_data_result(static_cast<uint8_t*>(malloc(RSA_KEY_SO_SIZE)));
    if (signed_data_result.get() == NULL) {
        ALOGE("Could not allocate memory for signed_data_result");
        return -1;
    }
    memset(signed_data_result.get(), 0, RSA_KEY_SO_SIZE);
    *signedDataLength = RSA_KEY_SO_SIZE; 

    msg = (uint8_t*)malloc(dataLength);
    if (msg == NULL) {
        ALOGE("Could not allocate memory for msg");
        return -1;
    }
    memcpy(msg, data, dataLength);
    // sign_rsa
    ret = TEE_RSASign(keyBlob, keyBlobLength, (const uint8_t *)msg, dataLength, signed_data_result.get(),
			(uint32_t*)signedDataLength, TEE_RSA_NODIGEST_NOPADDING);
    if (msg)
        free(msg);
    if (ret != TEE_ERR_NONE) {
        ALOGE("TEE_RSASign() is failed: %d", ret);
        return -1;
    }

    *signedData = signed_data_result.release();

    return 0;
}

__attribute__((visibility("default"))) int mt_km_verify_data(
    const keymaster0_device_t*, const void* params, const uint8_t* keyBlob,
    const size_t keyBlobLength, const uint8_t* signedData, const size_t signedDataLength,
    const uint8_t* signature, const size_t signatureLength) {
    bool verified_result = false;
    uint8_t *signed_data_buf = NULL; 
    uint8_t *signature_buf = NULL;
    teeResult_t ret = TEE_ERR_FAIL;

    ALOGD("mt_km_verify_data");

    if (signedData == NULL || signature == NULL) {
        ALOGW("data or signature buffers == NULL");
        return -1;
    }

    const keymaster_rsa_sign_params_t* sign_params =
        reinterpret_cast<const keymaster_rsa_sign_params_t*>(params);
    if (sign_params->digest_type != DIGEST_NONE) {
        ALOGW("Cannot handle digest type %d", sign_params->digest_type);
        return -1;
    } else if (sign_params->padding_type != PADDING_NONE) {
        ALOGW("Cannot handle padding type %d", sign_params->padding_type);
        return -1;
    } else if (signatureLength != signedDataLength) {
        ALOGW("signed data length must be signature length");
        return -1;
    }

    signed_data_buf = (uint8_t*)malloc(signedDataLength); 
    signature_buf = (uint8_t*)malloc(signatureLength);
    if(signed_data_buf == NULL || signature_buf == NULL) {
        ALOGE("Could not allocate memory for RSA verify");
        goto error;
    }
    memcpy(signed_data_buf, signedData, signedDataLength);
    memcpy(signature_buf, signature, signatureLength);
    ret = TEE_RSAVerify(keyBlob, keyBlobLength,
            (const uint8_t*)signed_data_buf, signedDataLength,
            (const uint8_t *)signature_buf, signatureLength,
            TEE_RSA_NODIGEST_NOPADDING, &verified_result);
    if (ret != TEE_ERR_NONE) {
        ALOGE("TEE_RSAVerify() is failed: %d", ret);
        goto error;
    }
error:
    if(signed_data_buf)
        free(signed_data_buf);
    if(signature_buf)
        free(signature_buf);

    if(verified_result) {
        // ALOGD("verify_rsa passed");
        return 0;
    } else {
        // ALOGD("verify_rsa failed");
        return -1;
    }
}

/* Close an opened OpenSSL instance */
static int mt_km_close(hw_device_t* dev) {
    delete dev;
    return 0;
}

static int mt_km_open(const hw_module_t* module, const char* name, hw_device_t** device) {
    if (strcmp(name, KEYSTORE_KEYMASTER) != 0)
        return -EINVAL;

    Unique_keymaster0_device_t dev(new keymaster0_device_t);
    if (dev.get() == NULL)
        return -ENOMEM;

    dev->common.tag = HARDWARE_DEVICE_TAG;
    dev->common.version = 1;
    dev->common.module = (struct hw_module_t*)module;
    dev->common.close = mt_km_close;

    dev->flags = 0;

    dev->generate_keypair = mt_km_generate_keypair;
    dev->import_keypair = mt_km_import_keypair;
    dev->get_keypair_public = mt_km_get_keypair_public;
    dev->delete_keypair = NULL;
    dev->delete_all = NULL;
    dev->sign_data = mt_km_sign_data;
    dev->verify_data = mt_km_verify_data;

    ERR_load_crypto_strings();
    ERR_load_BIO_strings();

    *device = reinterpret_cast<hw_device_t*>(dev.release());

    return 0;
}

static struct hw_module_methods_t keystore_module_methods = {
    .open = mt_km_open,
};

struct keystore_module HAL_MODULE_INFO_SYM
__attribute__ ((visibility ("default"))) = {
    common: {
        tag: HARDWARE_MODULE_TAG,
        version_major: 1,
        version_minor: 0,
        id: KEYSTORE_HARDWARE_MODULE_ID,
        name: "Keymaster MT t-base HAL",
        author: "The MTK Keymaster Source Project",
        methods: &keystore_module_methods,
        dso: 0,
        reserved: {},
    },
};

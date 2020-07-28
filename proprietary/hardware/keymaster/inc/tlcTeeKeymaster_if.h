/*
 * Copyright (c) 2013-2014 TRUSTONIC LIMITED
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

#ifndef __TLCTEEKEYMASTERIF_H__
#define __TLCTEEKEYMASTERIF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>


/**
 * Key sizes
 */
#define TEE_RSA_KEY_SIZE_512   512
#define TEE_RSA_KEY_SIZE_1024  1024
#define TEE_RSA_KEY_SIZE_2048  2048
#define TEE_RSA_KEY_SIZE_3072  3072
#define TEE_RSA_KEY_SIZE_4096  4096


/* error codes */
typedef enum
{
    TEE_ERR_NONE             = 0,
    TEE_ERR_FAIL             = 1,
    TEE_ERR_INVALID_BUFFER   = 2,
    TEE_ERR_BUFFER_TOO_SMALL = 3,
    TEE_ERR_NOT_IMPLEMENTED  = 4,
    TEE_ERR_SESSION          = 5,
    TEE_ERR_MC_DEVICE        = 6,
    TEE_ERR_NOTIFICATION     = 7,
    TEE_ERR_MEMORY           = 8,
    TEE_ERR_MAP              = 9,
    TEE_ERR_UNMAP            = 10,
    TEE_ERR_INVALID_INPUT    = 11
    /* more can be added as required */
} teeResult_t;


/* Key types */
typedef enum
{
    TEE_KEYTYPE_RSA          = 1,
    TEE_KEYTYPE_DSA          = 2,
    TEE_KEYTYPE_ECDSA        = 3,
    /* more can be added as required */
} teeKeyType_t;


/* RSA key pair types */
typedef enum {
    TEE_KEYPAIR_RSA       = 1,   /**< RSA public and RSA private key. */
    TEE_KEYPAIR_RSACRT    = 2    /**< RSA public and RSA CRT private key. */
} teeRsaKeyPairType_t;


/* Supported RSA signature algorithms */
typedef enum
{
    /* RSA */
    TEE_RSA_NODIGEST_NOPADDING    = 1, /**< No digest and padding */
} teeRsaSigAlg_t;


typedef enum {
  TEE_ECC_CURVE_NIST_P192 = 1,
  TEE_ECC_CURVE_NIST_P224 = 2,
  TEE_ECC_CURVE_NIST_P256 = 3,
  TEE_ECC_CURVE_NIST_P384 = 4,
  TEE_ECC_CURVE_NIST_P521 = 5,
} teeEccCurveType_t;


/**
 * RSA private key metadata (Private modulus and exponent lengths)
 */
typedef struct {
    uint32_t     lenpriexp;     /**< Private key exponent length */
} teeRsaPrivKeyMeta_t;


/**
 * RSA CRT private key metadata (Private modulus and exponent lengths)
 */
typedef struct {
    uint32_t     lenp;          /**< Prime p length */
    uint32_t     lenq;          /**< Prime q length */
    uint32_t     lendp;         /**< DP length */
    uint32_t     lendq;         /**< DQ length */
    uint32_t     lenqinv;       /**< QP length */
} teeRsaCrtPrivKeyMeta_t;


/**
 * RSA key metadata (public key hash, key size, modulus/exponent lengths, etc..)
 */
typedef struct {
    uint32_t     type;        /**< RSA key type (RSA or RSA CRT) */
    uint32_t     keysize;       /**< Key size, e.g. 1024, 2048 */
    uint32_t     lenpubmod;   /**< Public key modulus length */
    uint32_t     lenpubexp;   /**< Public key exponent length */
    union {
        teeRsaPrivKeyMeta_t    rsapriv;       /**< RSA private key */
        teeRsaCrtPrivKeyMeta_t rsacrtpriv; /**< RSA CRT private key */
    };
} teeRsaKeyMeta_t;


/**
 * DSA key metadata (p, q, g, x and y lengths)
 */
typedef struct {
    uint32_t     pLen;        /**< q length */
    uint32_t     qLen;        /**< q length */
    uint32_t     gLen;        /**< Generator length */
    uint32_t     xLen;        /**< x length */
    uint32_t     yLen;        /**< y length */
} teeDsaKeyMeta_t;


/**
 * ECDSA key metadata (x, y and private key lengths)
 */
typedef struct {
    uint32_t  curve;       /**< Curve type */
    uint32_t  curveLen;    /**< Curve length */
} teeEcdsaKeyMeta_t;


/**
 * Key metadata
 */
typedef struct {
    uint32_t     keytype;       /**< Key type, e.g. RSA */
    union {
        teeRsaKeyMeta_t    rsakey;    /**< RSA key */
        teeDsaKeyMeta_t    dsakey;    /**< DSA key */
        teeEcdsaKeyMeta_t  ecdsakey;  /**< ECDSA key */
    };
} teeKeyMeta_t;


/**
 * RSA public key metadata
 */
typedef struct {
    uint32_t     lenpubmod;   /**< Public key modulus length */
    uint32_t     lenpubexp;   /**< Public key exponent length */
} teeRsaPubKeyMeta_t;


/**
 * DSA public key metadata (p, q, g and y lengths)
 */
typedef struct {
    uint32_t     pLen;        /**< q length */
    uint32_t     qLen;        /**< q length */
    uint32_t     gLen;        /**< Generator length */
    uint32_t     yLen;        /**< y length */
} teeDsaPubKeyMeta_t;


/**
 * ECDSA public key metadata
 */
typedef struct {
    uint32_t  curve;        /**< Curve e.g. P192, P224,..,P521*/
} teeEcdsaPubKeyMeta_t;


/**
 * Public key metadata
 */
typedef struct {
    uint32_t     keytype;       /**< Key type, e.g. RSA */
    union {
        teeRsaPubKeyMeta_t   rsakey;    /**< RSA public key */
        teeDsaPubKeyMeta_t   dsakey;    /**< DSA public key */
        teeEcdsaPubKeyMeta_t ecdsakey;  /**< ECDSA public key */
    };
} teePubKeyMeta_t;


/**
 * DSA parameters
 */
typedef struct {
    uint8_t*      p;     /**< Prime p */
    uint8_t*      q;     /**< Prime q */
    uint8_t*      g;     /**< Generator */
    uint32_t      pLen;  /**< Prime p length */
    uint32_t      qLen;  /**< Prime q length */
    uint32_t      gLen;  /**< Generator length */
    uint32_t      xLen;  /**< x length */
    uint32_t      yLen;  /**< y length */
} teeDsaParams_t;


/**
 * ECDSA public key
 */
typedef struct {
    uint8_t*      x;     /**< Pointer to x coordinate */
    uint8_t*      y;     /**< Pointer to y coordinate */
    uint32_t      xLen;  /**< x length */
    uint32_t      yLen;  /**< y length */
} teeEcdsaPublicKey_t;


/**
 * ECDSA private key
 */
typedef struct {
    uint8_t*      data;   /**< Pointer to private key */
    uint32_t      len;    /**< Private key length */
} teeEcdsaPrivateKey_t;


/**
 * TEE_RSAGenerateKeyPair
 *
 * Generates RSA key pair and returns key pair data as wrapped object
 *
 * @param  keyType        [in]  Key pair type. RSA or RSACRT
 * @param  keyData        [in]  Pointer to the key data buffer
 * @param  keyDataLength  [in]  Key data buffer length
 * @param  keySize        [in]  Key size
 * @param  exponent       [in]  Exponent number
 * @param  soLen          [out] Key data secure object length
 */
teeResult_t TEE_RSAGenerateKeyPair(
    teeRsaKeyPairType_t keyType,
    uint8_t*            keyData,
    uint32_t            keyDataLength,
    uint32_t            keySize,
    uint32_t            exponent,
    uint32_t*           soLen);


/**
 * TEE_RSASign
 *
 * Signs given plain data and returns signature data
 *
 * @param  keyData          [in]  Pointer to key data buffer
 * @param  keyDataLength    [in]  Key data buffer length
 * @param  plainData        [in]  Pointer to plain data to be signed
 * @param  plainDataLength  [in]  Plain data length
 * @param  signatureData    [out] Pointer to signature data
 * @param  signatureDataLength  [out] Signature data length
 * @param  algorithm        [in]  RSA signature algorithm
 */
teeResult_t TEE_RSASign(
    const uint8_t*  keyData,
    const uint32_t  keyDataLength,
    const uint8_t*  plainData,
    const uint32_t  plainDataLength,
    uint8_t*        signatureData,
    uint32_t*       signatureDataLength,
    teeRsaSigAlg_t  algorithm);


/**
 * TEE_RSAVerify
 *
 * Verifies given data with RSA public key and return status
 *
 * @param  keyData          [in]  Pointer to key data buffer
 * @param  keyDataLength    [in]  Key data buffer length
 * @param  plainData        [in]  Pointer to plain data to be signed
 * @param  plainDataLength  [in]  Plain data length
 * @param  signatureData    [in]  Pointer to signed data
 * @param  signatureData    [in]  Plain  data length
 * @param  algorithm        [in]  RSA signature algorithm
 * @param  validity         [out] Signature validity
 */
teeResult_t TEE_RSAVerify(
    const uint8_t*  keyData,
    const uint32_t  keyDataLength,
    const uint8_t*  plainData,
    const uint32_t  plainDataLength,
    const uint8_t*  signatureData,
    const uint32_t  signatureDataLength,
    teeRsaSigAlg_t  algorithm,
    bool            *validity);


/**
 * TEE_KeyImport
 *
 * Imports key data and returns key data as secure object
 *
 * Key data needs to be in the following format
 *
 * RSA key data:
 * |--key metadata--|--public modulus--|--public exponent--|--private exponent--|
 *
 * RSA CRT key data:
 * |--key metadata--|--public modulus--|--public exponent--|--P--|--Q--|--DP--|--DQ--|--Qinv--|
 *
 * Where:
 * P:     secret prime factor
 * Q:     secret prime factor
 * DP:    d mod (p-1)
 * DQ:    d mod (q-1)
 * Qinv:  q^-1 mod p
 *
 * DSA key data:
 * |-- Key metadata --|--p--|--q--|--g--|--y--|--private key data (x)--|
 *
 * Where:
 * p:     prime (modulus)
 * q:     sub prime
 * g:     generator
 * y:     public y
 * x:     private x
 *
 * ECDSA key data:
 * |-- Key metadata --|--x--|--y--|--private key data (d)--|
 *
 * Where:
 * x:     affine coordinate x
 * y:     affine coordinate y
 * d:     private key
 *
 * @param  keyData          [in]  Pointer to key data
 * @param  keyDataLength    [in]  Key data length
 * @param  soData           [out] Pointer to wrapped key data
 * @param  soDataLength     [out] Wrapped key data length
 */
teeResult_t TEE_KeyImport(
    const uint8_t*  keyData,
    const uint32_t  keyDataLength,
    uint8_t*        soData,
    uint32_t*       soDataLength);


/** * TEE_GetPubKey
 *
 * Retrieves public key data from wrapped key data
 *
 *
 * RSA public key data:
 * |--public key metadata--|--public modulus--|--public exponent--|
 *
 * DSA public key data:
 * |-- public key metadata --|--p--|--q--|--g--|--y--|
 *
 * ECDSA public key data:
 * |-- public key metadata --|--x--|--y--|
 *
 *
 * @param  keyData          [in]  Pointer to key data
 * @param  keyDataLength    [in]  Key data length
 * @param  pubKeyData       [out] Pointer to public key data
 * @param  pubKeyDataLength [out] Public key data length
 */
teeResult_t TEE_GetPubKey(
    const uint8_t*  keyData,
    const uint32_t  keyDataLength,
    uint8_t*        pubKeyData,
    uint32_t*       pubKeyDataLength);


/**
 * TEE_DSAGenerateKeyPair
 *
 * Generates DSA key pair and returns key pair data as wrapped object
 *
 * @param  keyData        [in]  Pointer to the key data buffer
 * @param  keyDataLength  [in]  Key data buffer length
 * @param  params         [in]  DSA parameters
 * @param  soLen          [out] Key data secure object length
 */
teeResult_t TEE_DSAGenerateKeyPair(
    uint8_t*            keyData,
    uint32_t            keyDataLength,
    teeDsaParams_t      *params,
    uint32_t*           soLen);


/**
 * TEE_DSASign
 *
 * Signs given plain data and returns signature data
 *
 * @param  keyData          [in]  Pointer to key data buffer
 * @param  keyDataLength    [in]  Key data buffer length
 * @param  digest           [in]  Digest data to be signed
 * @param  digestLength     [in]  Digest data length
 * @param  signatureData    [out] Pointer to signature data
 * @param  signatureDataLength  [out] Signature data length
 */
teeResult_t TEE_DSASign(
    const uint8_t*  keyData,
    const uint32_t  keyDataLength,
    const uint8_t*  digest,
    const uint32_t  digestLength,
    uint8_t*        signatureData,
    uint32_t*       signatureDataLength);


/**
 * TEE_DSAVerify
 *
 * Verifies given data with DSA public key and return status
 *
 * @param  keyData          [in]  Pointer to key data buffer
 * @param  keyDataLength    [in]  Key data buffer length
 * @param  plainData        [in]  Pointer to plain data to be signed
 * @param  plainDataLength  [in]  Plain data length
 * @param  signatureData    [in]  Pointer to signed data
 * @param  signatureData    [in]  Plain  data length
 * @param  validity         [out] Signature validity
 */
teeResult_t TEE_DSAVerify(
    const uint8_t*  keyData,
    const uint32_t  keyDataLength,
    const uint8_t*  plainData,
    const uint32_t  plainDataLength,
    const uint8_t*  signatureData,
    const uint32_t  signatureDataLength,
    bool            *validity);


/**
 * TEE_ECDSAGenerateKeyPair
 *
 * Generates ECDSA key pair and returns key pair data as wrapped object
 *
 * @param  keyData        [in]  Pointer to the key data buffer
 * @param  keyDataLength  [in]  Key data buffer length
 * @param  curveType      [in]  Curve type
 * @param  soLen          [out] Key data secure object length
 */
teeResult_t TEE_ECDSAGenerateKeyPair(
    uint8_t*            keyData,
    uint32_t            keyDataLength,
    teeEccCurveType_t   curveType,
    uint32_t*           soLen);


/**
 * TEE_ECDSASign
 *
 * Signs given plain data and returns signature data
 *
 * @param  keyData          [in]  Pointer to key data buffer
 * @param  keyDataLength    [in]  Key data buffer length
 * @param  digest           [in]  Digest data to be signed
 * @param  digestLength     [in]  Digest data length
 * @param  signatureData    [out] Pointer to signature data
 * @param  signatureDataLength  [out] Signature data length
 */
teeResult_t TEE_ECDSASign(
    const uint8_t*  keyData,
    const uint32_t  keyDataLength,
    const uint8_t*  digest,
    const uint32_t  digestLength,
    uint8_t*        signatureData,
    uint32_t*       signatureDataLength);


/**
 * TEE_ECDSAVerify
 *
 * Verifies given data with ECDSA public key and return status
 *
 * @param  keyData          [in]  Pointer to key data buffer
 * @param  keyDataLength    [in]  Key data buffer length
 * @param  digest           [in]  Pointer to digest data to be verified
 * @param  digestLen        [in]  Digest data length
 * @param  signatureData    [in]  Pointer to signed data
 * @param  signatureData    [in]  Plain  data length
 * @param  validity         [out] Signature validity
 */
teeResult_t TEE_ECDSAVerify(
    const uint8_t*  keyData,
    const uint32_t  keyDataLength,
    const uint8_t*  digest,
    const uint32_t  digestLen,
    const uint8_t*  signatureData,
    const uint32_t  signatureDataLength,
    bool            *validity); 


/**
 * TEE_GetKeyInfo
 *
 * Retrieves key information (type, length,..) from key data blob and populates key metadata
 * accordingly
 *
 * @param  keyBlob          [in]  Pointer to key blob data
 * @param  keyBlobLength    [in]  Key data buffer length
 * @param  metadata         [out] Pointer to digest data to be verified
 */
teeResult_t TEE_GetKeyInfo(
    const uint8_t*  keyBlob,
    const uint32_t  keyBlobLength,
    teeKeyMeta_t*   metadata);


#ifdef __cplusplus
}
#endif

#endif // __TLCTEEKEYMASTERIF_H__

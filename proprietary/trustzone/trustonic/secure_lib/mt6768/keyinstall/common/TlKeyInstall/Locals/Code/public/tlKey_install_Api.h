 /*
 * Copyright (c) 2013 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */

#ifndef TL_KEY_INSTALL_API_H_
#define TL_KEY_INSTALL_API_H_

#include "tci.h"
//#include "TlApi/TlApi.h"    //not for tlApiCrSession_t, CryptoResult, CRYPTO_SUCESS


#define DEBUG   1
//#define DUMP_HEX_RESULT 1
/**
 * Command ID's for communication Trustlet Connector -> Trustlet.
 */

#define TZCMD_DRMKEY_QUERY           	1
#define TZCMD_DRMKEY_GEN_EKKB_PUB		2
#define TZCMD_DRMKEY_GEN_KB_EKKB_EKC	3
#define TZCMD_DRMKEY_GEN_REENC_EKKB		4
#define TZCMD_DRMKEY_INIT_ENV    		5
#define TZCMD_DRMKEY_INSTALL           	6
//#define CMD_SAMPLE_SHA256	7	/**< SHA256 */
#define TZCMD_DRMKEY_DRCRYPT            8
#define TZCMD_DRMKEY_GET_PLAINTEXT      9
#define TZCMD_DRMKEY_INSTALL_PLAINTEXT  10

/**
 * Return codes
 */
#define RET_ERR_INVALID_BUFFER	RET_CUSTOM_START + 1
#define RET_ERR_ALGORITHM		RET_CUSTOM_START + 3
#define RET_ERR_INIT		    RET_CUSTOM_START + 4
#define RET_ERR_KEY_INSTALL	    RET_CUSTOM_START + 5
#define RET_ERR_KEY_BLOCK_SIG_VERIFY_FAIL	    RET_CUSTOM_START + 6

#define AES128_BLOCK_SIZE 16

typedef enum {
    CRYPTO_SUCESS,
    CRYPTO_ERROR_ECB_INIT_FAIL,
    CRYPTO_ERROR_ECB_DO_FAIL,
    CRYPTO_ERROR_ECB_ENCRYPT_FAIL,
    CRYPTO_ERROR_GENERATE_SUBKEY_FAIL,
    CRYPTO_ERROR_CBC_ENCRYPT_FAIL,
    CRYPTO_ERROR_CBC_DECRYPT_FAIL,
    CRYPTO_ERROR_RSA_PKCS1_DECRYPT_FAIL,
    CRYPTO_ERROR_CTR_FAIL
} CryptoResult;

/**
 * Termination codes
 */
#define EXIT_ERROR	((uint32_t)(-1))


/**
 * SHA256 command message.
 */
typedef struct {
    tciCommandHeader_t header;	/**< Command header. */
    uint32_t keyid;
    uint32_t keytype;
    uint32_t Buffer1;               /* input buffer, mapInfo.sVirtualAddr */
    uint32_t Buffer2;               /* output buffer, deckey_mapInfo.sVirtualAddr */
    uint32_t srcLen;                /* Length of ekkb input data. */
    uint32_t outLen;                /* Length of the decrypted data. */
    uint32_t srcOffs;               /* The offset of the source data within the bulk buffer. */
    uint64_t decrypt_mem_handle;    /* the handle of sec mem (to be freed) */
    uint32_t keypair_index;         /* SONY request: use different set of Input_Ekkb_pub and InputPkb */
} cmdKi_t;

/**
 * Response structure Trustlet -> Trustlet Connector.
 */
typedef struct {
    tciResponseHeader_t header;	/* Response header. */
    uint32_t outLen;    /* The length of the output buffer */
    uint32_t count;
} rspKi_t;

/**
 * TCI message data.
 */
typedef union {
    tciCommandHeader_t	commandHeader;
    tciResponseHeader_t	responseHeader;
    cmdKi_t     cmdKi;//cmdSha256;
    rspKi_t     rspKi;//rspSha256;
} tciMessage_t;

/**
 * Overall TCI structure.
 */
#define TCI_PAYLOAD_LEN 1024
typedef struct {
	tciMessage_t message;	/**< TCI message */
} tci_t;

//for OAEP decode

#if !defined(KEY_INSTALL_NWD)
#define AES128_BLOCK_SIZE 16
#define AES128_KEY_SIZE 16
#define AES128_IV_SIZE 16
#define RAS_PRIVATE_KEY_LENGTH 256
#define SHA256_DIGEST_LENGTH 32
#define SHA_DIGEST_LENGTH 20

#define HASH_SHA1_SIZE 20

#define TEMP_BUF_SIZE (15 * 1024)

#define STORE32H(x, y)                                                                     \
      { (y)[0] = (unsigned char)(((x)>>24)&255); (y)[1] = (unsigned char)(((x)>>16)&255);   \
        (y)[2] = (unsigned char)(((x)>>8)&255); (y)[3] = (unsigned char)((x)&255); }


#define LOG_VERBOSE 4
#define LOG_DEBUG 3
#define LOG_INFORMATION 2
#define LOG_WARNING 1
#define LOG_ERROR 0

#define TEE_MDW_LOG_LEVEL LOG_INFORMATION

#define TEE_KI_LOGE(...)                                   \
    do {                                                    \
        if(TEE_MDW_LOG_LEVEL >= LOG_ERROR) {                \
            tlDbgPrintf("TRUSTONIC_TEE_KI[E]: ");          \
            tlApiLogPrintf(__VA_ARGS__);                    \
            tlDbgPrintf(EOL);                               \
        }                                                   \
    } while(FALSE)

#define TEE_KI_LOGI(...)                                   \
    do {                                                    \
        if(TEE_MDW_LOG_LEVEL >= LOG_INFORMATION) {                \
            tlDbgPrintf("TRUSTONIC_TEE_KI[I]: ");          \
            tlApiLogPrintf(__VA_ARGS__);                    \
            tlDbgPrintf(EOL);                               \
        }                                                   \
    } while(FALSE)




CryptoResult crypto_aes128_ecb_encrypt( const uint8_t* inputBuf,
		const size_t inputBufLen, uint8_t* outputBuf,
		size_t outputBufLen, uint8_t* key);

/*
CryptoResult crypto_aes128_cmac(uint8_t* key, uint8_t* input,
	int inputLen, uint8_t *mac);
*/

bool crypto_aes128_cbc_decrypt(const uint8_t* key, const uint8_t* iv, const uint8_t* src,
                                size_t src_len, uint8_t* dst, size_t* dst_len);

bool crypto_aes128_cbc_encrypt(const uint8_t* key, const uint8_t* iv, const uint8_t* src,
                                size_t src_len, uint8_t* dst, size_t* dst_len);

CryptoResult crypto_aes128_ctr_decrypt(const uint8_t* key, const uint8_t* iv, const uint8_t* src,
                                size_t src_len, uint8_t* dst, size_t* dst_len);

CryptoResult crypto_aes128_ctr_decrypt_with_offset(uint8_t* key, uint8_t* iv, const uint8_t* src,
                                size_t src_len, uint8_t* dst, size_t* dst_len,
                                uint8_t* ecount_buf, size_t *num);

void crypto_ctr128_inc(uint8_t* counter);

tlApiRsaKey_t crypto_mock_rsa_key();

tlApiRsaKey_t crypto_mock_verify_rsa_key();

bool cryoto_message_digest_sha1(const uint8_t* message,
            size_t message_length, uint8_t* hash, size_t* hash_length);

bool cryoto_sign_hmac_sha256(const uint8_t* mac_key, const uint8_t* message,
            size_t message_length, uint8_t* signature, size_t* signature_length);

bool cryoto_sign_verify_hmac_sha256(const uint8_t* mac_key, const uint8_t* message,
            size_t message_length, const uint8_t* signature, size_t signature_length);

bool crypto_generate_rsa_signature(const tlApiRsaKey_t *rsa_key, const uint8_t* message,
            size_t message_length, uint8_t* signature, size_t* signature_length);

bool crypto_rsa_pkcs1_oaep_decrypt(const tlApiRsaKey_t *rsa_key, const uint8_t* src,
                                size_t src_len, uint8_t* dst, size_t* dst_len);

bool crypto_pkcs_1_oaep_decode(const uint8_t *msg,    uint32_t msglen,
                       const uint8_t *lparam, uint32_t lparamlen,
                       uint32_t modulus_bitlen,
                       uint8_t *out,    uint32_t *outlen,
                       int           *res);

bool crypto_rsa_decrypt_nopad(const tlApiRsaKey_t *rsa_key, const uint8_t* src,
                                size_t src_len, uint8_t* dst, size_t* dst_len);

//======================
/* header */

#define HASH_LENGTH 20
#define BLOCK_LENGTH 64

union _buffer {
	uint8_t b[BLOCK_LENGTH];
	uint32_t w[BLOCK_LENGTH/4];
};

union _state {
	uint8_t b[HASH_LENGTH];
	uint32_t w[HASH_LENGTH/4];
};

typedef struct sha1nfo {
	union _buffer buffer;
	uint8_t bufferOffset;
	union _state state;
	uint32_t byteCount;
	uint8_t keyBuffer[BLOCK_LENGTH];
	uint8_t innerHash[HASH_LENGTH];
} sha1nfo;

/* public API - prototypes - TODO: doxygen*/

/**
 */
void sha1_init(sha1nfo *s);
/**
 */
void sha1_writebyte(sha1nfo *s, uint8_t data);
/**
 */
void sha1_write(sha1nfo *s, const char *data, size_t len);
//=========

int pkcs_1_pss_decode_sha1
(
const unsigned char* msghash,
unsigned long        msghashlen,
const unsigned char* sig,
unsigned long        siglen,
unsigned long        saltlen,
unsigned long        modulus_bitlen,
int*                 res
);

int crypto_rsa_pkcs1_pss_decrypt( const tlApiRsaKey_t *rsa_key,
                                    const unsigned char* msghash,
                                    unsigned long        msghashlen,
                                    const unsigned char* sig,
                                    unsigned long        siglen,
                                    unsigned long        saltlen
                                    );
#endif //#if !defined(KEY_INSTALL_NWD)

#endif // TL_KEY_INSTALL_API_H_

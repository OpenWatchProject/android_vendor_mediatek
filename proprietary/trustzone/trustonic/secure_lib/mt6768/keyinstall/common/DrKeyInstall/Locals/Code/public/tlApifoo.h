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

#ifndef __TLAPIKEY_INSTALL_H__
#define __TLAPIKEY_INSTALL_H__

#include "TlApi/TlApiCommon.h"
#include "TlApi/TlApiError.h"

/* Marshaled function parameters.
 * structs and union of marshaling parameters via TlApi.
 *
 * @note The structs can NEVER be packed !
 * @note The structs can NOT used via sizeof(..) !
 */

 /*
  * Function id definitions
  */
#define FID_DR_OPEN_SESSION     1
#define FID_DR_CLOSE_SESSION    2
#define FID_DR_INIT_DATA        3
#define FID_DR_EXECUTE          4
#define FID_DRV_ADD_FOO         5
#define FID_DRV_SUB_FOO         6

#define KEY_INSTALL_MEM_COPY_DECRYPT_KEY 11  // sample code define command end is 6

/* .. add more when needed */

/* Marshaled function parameters.
 * structs and union of marshaling parameters via TlApi.
 *
 * @note The structs can NEVER be packed !
 * @note The structs can NOT used via sizeof(..) !
 */
#if 0
typedef struct {
    uint32_t    commandId;
	uint32_t    x;
	uint32_t    y;
    uint32_t    result;
} tlApiFoo_t, *tlApiFoo_ptr;
#endif

typedef struct {
    // for decrypt mem buffer
    uint64_t decrypt_mem_handle;  // outoput target (PA of secure handle)
    uint64_t decrypt_mem_pa;
    uint64_t decrypt_mem_length;
    uint32_t decrypt_key_length;
    uint64_t decrypt_key_va;
} tlApiDecryptMemData_t, *tlApiDecryptMemData_ptr;
//} tlApiH264VdecShareData_t, *tlApiH264VdecShareData_ptr;


/** Encode cleartext with rot13.
 *
 * @param encodeData Pointer to the tlApiEncodeRot13_t structure
 * @return TLAPI_OK if data has successfully been encrypted.
 */
#if 0
_TLAPI_EXTERN_C tlApiResult_t tlApiAddFoo(tlApiFoo_ptr FooData);
#endif

//_TLAPI_EXTERN_C drApiResult_t CopyDecryptKeyBuf2Secmem(uint32_t mem_target_pa, uint32_t *mem_target_length, uint8_t *decrypt_buf, uint32_t decrypted_key_len);
_TLAPI_EXTERN_C tlApiResult_t tlApiKeyInstallCopy_decrypt_key(uint32_t sid, tlApiDecryptMemData_ptr decryptData);
_TLAPI_EXTERN_C tlApiResult_t tlApiKeyInstallUnMemMap_decrypt_key(uint32_t sid, tlApiDecryptMemData_ptr decryptData);


#endif // __TLAPIKEY_INSTALL_H__


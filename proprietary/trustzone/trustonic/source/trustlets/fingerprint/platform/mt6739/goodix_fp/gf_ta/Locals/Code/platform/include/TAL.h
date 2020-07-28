/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */

/**
 * @file TAL.h
 * @brief Declares TEE Abstraction Layer (TAL) interface functions.
 * @version 0.1
 */
#ifndef __TAL_H__
#define __TAL_H__


/** No error */

#define TAL_ERR_SUCCESS     0

/** General error */
#define TAL_ERR_GENERAL     1

/** Not supported functionality */
#define TAL_ERR_UNSUPPORTED     2

/** Invalid parameter */
#define TAL_ERR_INVALIDPARAM      3

/** Buffer size is small to store the data */
#define TAL_ERR_SMALLBUFFER     4

/** File I/O error. */
#define TAL_ERR_FILE_IO     5

/** Maximum length of Trustlet ID */
#define TAL_MAX_TA_ID_LEN     256

/** Result code from TAL API */
typedef unsigned int tal_result_t;

/** Unsigned Integer */
typedef unsigned int tal_uint_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Gets the attestation private key.
 *
 * @param [out]   pAttKeyPrv   pointer to the buffer that receives the 32 byte key.
 *
 * @return   TAL_ERR_SUCCESS if no errors occurred.
 */
tal_result_t TAL_GetAttestationPrivateKey(void* pAttKeyPrv);


 /**

 * Checks whether a buffer of nLen bytes pointed to by pBuffer
 * is entirely in non secure memory area.
 *
 * @param [in]   pBuffer   a pointer to the buffer to be checked.
 * @param [in]   nLen   the length of the buffer to be checked.
 *
 * @return   TAL_ERR_SUCCESS if the buffer is in non secure memory area.
 */
tal_result_t TAL_BufferIsNonSecureMemory(const void* pBuffer,
                                         tal_uint_t  nLen);

/**
 * Gets wrapped object length based on the object length.
 *
 * @param [in]   nLen the length of the object to be wrapped.
 *
 * @return   wrapped object length in bytes.
 */
tal_uint_t TAL_WrappedLen(tal_uint_t nLen);

/**
 * Wraps an object for another trusted application (TA).
 *
 * @param [in]   pDestTA specifies a destination TA for which the object is wrapped.
 * @param [in]   pInBuf pointer to the buffer that containes the object to be wrapped.
 * @param [in]   nInLen the length of the buffer pointed to by pInBuf.
 * @param [in]   pOutBuf   pointer to the buffer that receives wrapped object.
 * @param [in,out]   pnOutLen [in] the length of the buffer pointed to by pOutBuf,
 *                   [out] the length of wrapped object.
 *
 * @return   TAL_ERR_SUCCESS if no errors occurred.
 */
tal_result_t TAL_WrapObjectToOtherTA(const void* pDestTA,
                                     const void* pInBuf,
                                     tal_uint_t  nInLen,
                                     void*       pOutBuf,
                                     tal_uint_t* pnOutLen);

/**
 * Unwraps an object from another trusted application (TA).
 *
 * @param [out]   pSourceTA pointer to the buffer of TAL_MAX_TA_ID_LEN bytes where
 *                a Source TA ID from which the object is received will be returned.
 * @param [in]    pInBuf pointer to the buffer that containes the object to be unwrapped.
 * @param [in]    nInLen the length of the buffer pointed to by pInBuf.
 * @param [in]    pOutBuf  pointer to the buffer that receives unwrapped object.
 * @param [in,out]  pnOutLen [in] the length of the buffer pointed to by pOutBuf,
 *                 [out] the length of unwrapped object.
 *
 * @return   TAL_ERR_SUCCESS if no errors occurred.
 */
tal_result_t TAL_UnwrapObjectFromOtherTA(void* pSourceTA,
                                         const void* pInBuf,
                                         tal_uint_t  nInLen,
                                         void*       pOutBuf,
                                         tal_uint_t* pnOutLen);

/**
 * Wraps an object to be used (unwrapped) later by the same trusted application.
 *
 * @param [in]    pInBuf pointer to the buffer that containes the object to be wrapped.
 * @param [in]    nInLen the length of the buffer pointed to by pInBuf.
 * @param [in]    pOutBuf pointer to the buffer that receives wrapped object.
 * @param [in,out]   pnOutLen [in] the length of the buffer pointed to by pOutBuf,
 *                   [out] the length of wrapped object.
 *
 * @return   TAL_ERR_SUCCESS if no errors occurred.
 */
tal_result_t TAL_WrapObjectToSelf(const void* pInBuf,
                                  tal_uint_t  nInLen,
                                  void*       pOutBuf,
                                  tal_uint_t* pnOutLen);

/**
 * Unwraps an object from the same trusted application.
 *
 * @param [in]   pInBuf pointer to the buffer that containes the object to be unwrapped.
 * @param [in]   nInLen the length of the buffer pointed to by pInBuf.
 * @param [in]   pOutBuf   pointer to the buffer that receives unwrapped object.
 * @param [in,out]   pnOutLen [in] the length of the buffer pointed to by pOutBuf,
 *                   [out] the length of unwrapped object.
 *
 * @return  TAL_ERR_SUCCESS if no errors occurred.
 */
tal_result_t TAL_UnwrapObjectFromSelf(const void* pInBuf,
                                      tal_uint_t  nInLen,
                                      void*       pOutBuf,
                                      tal_uint_t* pnOutLen);

/**
 * Generates random data.
 *
 * @param [in]   nRandDataLen desired length of random data.
 * @param [out]   pRandData pointer to the buffer that receives randomly generated data.
 *
 * @return   TAL_ERR_SUCCESS if no errors occurred.
 */
tal_result_t TAL_GetRandom(tal_uint_t nRandDataLen,
                           void*      pRandData);

/**
 * Logs formatted Debug message.
 *
 * @param [in]   fmt format control string.
 */
void TAL_DbgLog(const char* fmt, ...);

/**
 * Logs formatted message.
 *
 * @param [in]   fmt format control string.
 */
void TAL_ErrLog(const char* fmt, ...);


#ifdef __cplusplus
}
#endif

#endif /* __TAL_H__ */


/*
* Copyright (c) 2014 - 2016 MediaTek Inc.
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files
* (the "Software"), to deal in the Software without restriction,
* including without limitation the rights to use, copy, modify, merge,
* publish, distribute, sublicense, and/or sell copies of the Software,
* and to permit persons to whom the Software is furnished to do so,
* subject to the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef __TLDRIVERAPI_H__
#define __TLDRIVERAPI_H__

#include <tee_internal_api.h>
#include <tee_type.h>

typedef enum {
    RDA_ID = 0,
    WIDEVINE_ID,
    SOTER_ATTK,
    SOTER_COUNTER,
    GATEKEEPER_ID,
    KEYMASTER_ID,
    UNDEFINED6_ID,
    UNDEFINED7_ID
} RPMB_USER_ID;

/* Can not be larger than MAX_DR_SESSIONS defined in drv_session_mgmt.h */
#define RPMB_MAX_USER_NUM 8
/*
 * Open session to the driver with given data
 *
 * @return  session id
 */
_TLAPI_EXTERN_C uint32_t TEE_RpmbOpenSession( uint32_t uid );


/*
 * Close session
 *
 * @param sid  session id
 *
 * @return  TLAPI_OK upon success or specific error
 */
_TLAPI_EXTERN_C TEE_Result TEE_RpmbCloseSession( uint32_t sid );

/*
 * Executes command
 *
 * @param sid        session id
 * @param commandId  command id
 *
 * @return  TLAPI_OK upon success or specific error
 */
//_TLAPI_EXTERN_C tlApiResult_t tlApiExecute(
//        uint32_t sid,
//        tlApiRpmb_ptr RpmbData);
_TLAPI_EXTERN_C TEE_Result TEE_RpmbReadDatabyOffset(
    uint32_t sid,
    uint32_t offset,
    uint8_t *buf,
    uint32_t bufSize,
    int *result);

_TLAPI_EXTERN_C TEE_Result TEE_RpmbWriteDatabyOffset(
    uint32_t sid,
    uint32_t offset,
    uint8_t *buf,
    uint32_t bufSize,
    int *result);

_TLAPI_EXTERN_C TEE_Result TEE_RpmbReadData(
    uint32_t sid,
    uint8_t *buf,
    uint32_t bufSize,
    int *result);

_TLAPI_EXTERN_C TEE_Result TEE_RpmbWriteData(
    uint32_t sid,
    uint8_t *buf,
    uint32_t bufSize,
    int *result);

#if (CFG_RPMB_KEY_PROGRAMED_IN_KERNEL == 1)
_TLAPI_EXTERN_C TEE_Result TEE_RpmbProgramKey(
    uint32_t sid,
    int *result);
#endif

#endif // __TLDRIVERAPI_H__

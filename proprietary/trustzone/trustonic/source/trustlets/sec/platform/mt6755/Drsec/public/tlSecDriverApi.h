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

/*
 * @file   tlDriverApi.h
 * @brief  Contains trustlet API definitions
 *
 */

#ifndef __TL_M_SEC_DRIVER_API_H__
#define __TL_M_SEC_DRIVER_API_H__

//#if defined(TRUSTEDAPP)
//#include "taStd.h"
//#else
#include "tlStd.h"
//#endif
#include "TlApi/TlApiError.h"
#include "tlApisec.h"

/*
 * Open session to the driver with given data
 *
 * @return  session id
 */
_TLAPI_EXTERN_C uint32_t tlSecApiOpenSession( void );


/*
 * Close session
 *
 * @param sid  session id
 *
 * @return  TLAPI_OK upon success or specific error
 */
_TLAPI_EXTERN_C tlApiResult_t tlSecApiCloseSession( uint32_t sid );


/*
 * Executes command
 *
 * @param sid        session id
 * @param commandId  command id
 *
 * @return  TLAPI_OK upon success or specific error
 */
_TLAPI_EXTERN_C tlApiResult_t tlSecApiExecute(
        uint32_t sid,
        tlApiDapc_ptr FooData);

_TLAPI_EXTERN_C int32_t exNwdApi_HACC_ProcessData( const void *pSrc, uint32_t dataSize, 
    const void *pSeed, uint32_t seedSize, uint32_t nwd_user, uint32_t nwd_req_dir);

_TLAPI_EXTERN_C int32_t tlset_module_apc(unsigned int module, unsigned int domain_num , unsigned int permission_control);

_TLAPI_EXTERN_C int32_t tlset_master_transaction(unsigned int master_index ,unsigned int transaction_type);

_TLAPI_EXTERN_C int32_t tlget_devinfo(unsigned int index , uint32_t * result);

#endif // __TL_M_SEC_DRIVER_API_H__

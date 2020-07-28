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

#ifndef __TLDRIVERAPI_H__
#define __TLDRIVERAPI_H__

#include "tlStd.h"
#include "TlApi/TlApiError.h"

#include "tlApifoo.h"


/*
 * Open session to the driver with given data
 *
 * @return  session id
 */
_TLAPI_EXTERN_C uint32_t tlApiOpenSession( void );


/*
 * Close session
 *
 * @param sid  session id
 *
 * @return  TLAPI_OK upon success or specific error
 */
_TLAPI_EXTERN_C tlApiResult_t tlApiCloseSession( uint32_t sid );


/*
 * Executes command
 *
 * @param sid        session id
 * @param commandId  command id
 *
 * @return  TLAPI_OK upon success or specific error
 */
_TLAPI_EXTERN_C tlApiResult_t tlApiExecute(
        uint32_t sid,
        tlApiFoo_ptr FooData);


/* tlApi function to call driver via IPC.
 * Function should be called only from customer specific TlApi library. 
 * Sends a MSG_RQ message via IPC to a MobiCore driver.
 *
 * @param driverID The driver to send the IPC to.
 * @param pMarParam MPointer to marshaling parameters to send to the driver.
 *
 * @return TLAPI_OK
 * @return E_TLAPI_COM_ERROR in case of an IPC error.
 */
_TLAPI_EXTERN_C tlApiResult_t tlApi_callDriver(
        uint32_t driver_ID,
        void* pMarParam);
        
#endif // __TLDRIVERAPI_H__

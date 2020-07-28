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

#ifndef __TLDRIVERSPIAPI_H__
#define __TLDRIVERSPIAPI_H__

#include "tlStd.h"
#include "TlApi/TlApiCommon.h"
#include "TlApi/TlApiError.h"

#include "drtlspi_api.h"
#include "spi.h"


/* Marshaled function parameters.
 * structs and union of marshaling parameters via TlApi.
 *
 * @note The structs can NEVER be packed !
 * @note The structs can NOT used via sizeof(..) !
 */
 
 /*
  * Function id definitions
  */
/*
#define FID_DR_OPEN_SESSION     1
#define FID_DR_CLOSE_SESSION    2
#define FID_DR_INIT_DATA        3
#define FID_DR_SPI_EXECUTE      4
#define FID_DR_SPI_HW_CONFIG    5
#define FID_DR_SPI_DEBUG        6
*/
/* .. add more when needed */



/*
 * Open session to the driver with given data
 *
 * @return  session id
 */
_TLAPI_EXTERN_C uint32_t tlSpiOpenSession( void );


/*
 * Close session
 *
 * @param sid  session id
 *
 * @return  TLAPI_OK upon success or specific error
 */
_TLAPI_EXTERN_C tlApiResult_t tlSpiCloseSession( uint32_t sid );


/*
 * Executes command
 *
 * @param sid        session id
 * @param commandId  command id
 *
 * @return  TLAPI_OK upon success or specific error
 */
_TLAPI_EXTERN_C tlApiResult_t tlSpiExecute(
        uint32_t sid,
        spi_transfer_ptr spiData);


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

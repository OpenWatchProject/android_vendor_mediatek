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
 * @file   drApiMarshal.h
 * @brief  Contains marshalling definitions and data structures
 *
 */
 
#ifndef __DRAPIMARSHAL_H__
#define __DRAPIMARSHAL_H__

#include "DrApi/DrApiCommon.h"
#include "TlApi/TlApiError.h"

#include "drtlspi_api.h"
#include "spi.h"

/*
 * Driver ID. Update accordingly
 */
#define SPI_DRV_ID DRIVER_ID

/* Invalid session id. Returned in case of an error. */
#define DR_SID_INVALID      0xffffffff
 
/** Encode cleartext with rot13.
 *
 * @param encodeData Pointer to the tlApiEncodeRot13_t structure 
 * @return TLAPI_OK if data has successfully been encrypted.
 */

/** Union of marshaling parameters. */
/* If adding any function, add the marshaling structure here */
typedef struct 
{
    /**< Function identifier. */
	uint32_t     functionId;
	struct
    {    
		tlApiResult_t retVal;
		struct spi_transfer  *spi_data;
        /* untyped parameter list (expends union to 8 entries) */
	} payload;
    uint32_t     sid;           /** Session identifier */
} drMarshalingParam_t, *drMarshalingParam_ptr;

#endif // __DRAPIMARSHAL_H__


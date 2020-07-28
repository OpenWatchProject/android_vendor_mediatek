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
 
#ifndef __DRTLSPI_API_H__
#define __DRTLSPI_API_H__

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


/* Marshaled function parameters.
 * structs and union of marshaling parameters via TlApi.
 *
 * @note The structs can NEVER be packed !
 * @note The structs can NOT used via sizeof(..) !
 */
//#define QUERY_MAX_LEN 8

#include <spi.h>

_TLAPI_EXTERN_C tlApiResult_t drSpiSend0(const void *tx_buf, void	*rx_buf, uint32_t len, struct mt_chip_conf *chip_conf, uint32_t flag);
_TLAPI_EXTERN_C tlApiResult_t drSpiHwConfig(struct mt_chip_conf *chip_conf);
_TLAPI_EXTERN_C tlApiResult_t drSpiDebug(void);
_TLAPI_EXTERN_C tlApiResult_t drSpiTest(void);

#endif // __DRTLSPI_API_H__


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
 
#ifndef __TL_M_TPLAY_API_H__
#define __TL_M_TPLAY_API_H__

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
#define FID_DR_SPI_EXECUTE      4
#define FID_DR_SPI_HW_CONFIG    5
#define FID_DR_SPI_DEBUG        6
#define FID_DR_SPI_TEST         7

typedef struct {
    uint32_t    commandId;
	uint32_t    index;
    uint32_t    result;
    uint32_t    module_index;
    uint32_t    domain_num;
    uint32_t    permission_control;
    
} tlApiSpi_t, *tlApiSpi_ptr;

#endif // __TLAPIDAPC_H__


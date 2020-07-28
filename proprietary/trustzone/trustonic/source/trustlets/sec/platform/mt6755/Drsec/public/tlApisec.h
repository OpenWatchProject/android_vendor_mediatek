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
 
#ifndef __TL_M_SECDRV_API_H__
#define __TL_M_SECDRV_API_H__

#if !defined(TRUSTEDAPP)
#include "TlApi/TlApiCommon.h"
#endif
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
#define FID_DR_DAPC_OPEN_SESSION            6   
#define FID_DR_DAPC_CLOSE_SESSION           7
#define FID_DR_DAPC_INIT_DATA               8   /* this will not be used */
#define FID_DR_DAPC_EXECUTE                 9
#define FID_DRV_DEVINFO_GET                 10
#define FID_DRV_DEVAPC_SET                  11
#define FID_DRV_HACC_NWD_PROCESS            16  /* for NWD HACC */

typedef enum {
    MEM_TA_VIRTUAL = 1,
    MEM_PHYSICAL = 2,
    MEM_NULL = 3,
    MEM_HANDLE_PHY_MAPPING = 5,
    MEM_RESERVED = 0x7FFFFFFF  /* force enum to use 32 bits */
} EX_MEM_t;

/* .. add more when needed */

/* Marshaled function parameters.
 * structs and union of marshaling parameters via TlApi.
 *
 * @note The structs can NEVER be packed !
 * @note The structs can NOT used via sizeof(..) !
 */

typedef struct{
    uint32_t    module_index;
    uint32_t    domain_num;
    uint32_t    permission_control;
}dapc_para;

typedef struct{
	uint32_t    index;
    uint32_t    result;
}devinfo_para;


typedef struct {
    uint32_t    commandId;
	uint32_t    index;
    uint32_t    result;
    uint32_t    module_index;
    uint32_t    domain_num;
    uint32_t    permission_control;
    
} tlApiDapc_t, *tlApiDapc_ptr;

/** Encode cleartext with rot13.
 *
 * @param encodeData Pointer to the tlApiEncodeRot13_t structure 
 * @return TLAPI_OK if data has successfully been encrypted.
 */

_TLAPI_EXTERN_C tlApiResult_t tlApiAddDapc(tlApiDapc_ptr DapcData);

#endif // __TLAPIDAPC_H__


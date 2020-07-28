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
 * @file   drError.h
 * @brief  Error id definitions
 *
 */

#ifndef __DR_M_SECDRV_ERROR_H__
#define __DR_M_SECDRV_ERROR_H__

#define E_DRAPI_DRV_ROT13 0x40001

/*
 * Driver fatal error codes.
 */
typedef enum {
    E_DR_OK               = 0, /**< Success */
    E_DR_IPC              = 1, /**< IPC error */
    E_DR_INTERNAL         = 2, /**< Internal error */
    E_DR_INVALID_SRC_MEM_TYPE = 3, /** HACC invalide source memory type */
    E_DR_INVALID_DEST_MEM_TYPE = 4, /** HACC invalide dest memory type */
    E_DR_INVALID_IV_SIZE = 5, /** HACC invalide IV size */
    E_DR_INVALID_SESSION_DATA = 6, /** HACC invalide session data */
    E_DR_WRONG_KEY_LENGTH = 7,  /** HACC wrong key length */
    E_DR_PHYSICAL_MEM_MAP_FAIL = 8,  /** HACC map physical memory fail */
    E_DR_DATA_SIZE_PACKGE_TOO_LARGE = 9, /** HACC data package size too large (more that 128KB) */
    E_DR_PHYSICAL_MEM_UNMAP_FAIL = 10,  /** HACC unmap physical memory fail */
    E_DR_PHYSICAL_ADDRESS_NOT_ALIGN = 11,  /** HACC physcial address not align */
    E_DR_HACC_NWD_REQUEST_FAIL = 12,  /** HACC operation for NWD failed */
    E_DR_SESSION_OPEN_FAIL  = 13, /** HACC open session failed */
    E_DR_SESSION_CLOSE_FAIL  = 14, /** HACC close session failed */
    E_DR_NO_RESPONSE_FAIL  = 15, /** HACC driver is not responsed */
    E_DR_INVALID_SECURE_MEMORY_BOUNDARY  = 17, /** HACC invalide source physical address boundary */
    /* ... add more error codes when required */
    E_DR_RESERVED = 0x7FFFFFFF  /* force enum to use 32 bits */
} drError_t;

#define SEC_OK                                  0x0000
#define ERR_HACC_MODE_INVALID                   0x6001
#define ERR_HACC_KEY_INVALID                    0x6002
#define ERR_HACC_DATA_UNALIGNED                 0x6003
#define ERR_HACC_SEED_LEN_ERROR                 0x6004
#define ERR_HACC_HW_WRAP_KEY_NOT_INIT           0x6007
#define ERR_HACC_UNKOWN_USER                    0x600C

#endif // __DRERROR_H__


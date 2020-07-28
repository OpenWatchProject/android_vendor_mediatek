/*
 * Copyright (c) 2016-2018 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */

/**
 * @file   VerifiedBoot_types.h
 * @brief  Contains data structures related to Verified Boot feature
 *
 */
#ifndef __VERIFIEDBOOT_TYPES_H__
#define __VERIFIEDBOOT_TYPES_H__

#define KM_CHECK_VERSION_ERR_UNDEF        (0)
#define KM_CHECK_VERSION_OK               (1)
#define KM_CHECK_OS_VERSION_FAIL          (2)
#define KM_CHECK_OS_PATCH_LEVEL_FAIL      (3)
#define KM_CHECK_VENDOR_PATCH_LEVEL_FAIL  (4)

/**
 * Number of bytes used to store device state.
 */
#define KM_DEVICE_STATE_SIZE 32

typedef enum {
    VB_VERSION_1_0 = 0x0,
    VB_VERSION_UNDEFINED = 0xFFFF
} VerifiedBoot_Version_t;

typedef enum {
    verified_boot_state_green  = 0,
    verified_boot_state_yellow = 1,
    verified_boot_state_orange = 2,
    verified_boot_state_red    = 3,
} verified_boot_state_t;

typedef struct {
    int8_t                isEqual;
    uint32_t              os_version;
    uint32_t              os_patchlevel;
    uint32_t              vendor_patchlevel;
    uint32_t              boot_patchlevel;
} VersionInfo_t;

/**
 * os_version and os_patchlevel fields are not provided by the bootloader in Android 9
 * and are kept for compatibility with older Android versions
 * device_locked whether device is locked
 * boot_state VerifiedBootState
 * verified_boot_key 32-byte hash of boot verification key
 */
typedef struct {
    uint32_t              os_version;
    uint32_t              os_patchlevel;
    uint32_t              vendor_patchlevel;
    uint32_t              boot_patchlevel;
    verified_boot_state_t boot_state;
    bool                  device_locked;
    uint8_t               verified_boot_key[32];
    uint8_t               vbmeta_digest[32];
} VerifiedBoot_Data_V1_t;

typedef struct {
    VerifiedBoot_Version_t     version;
    union {
        VerifiedBoot_Data_V1_t data_v1;
    } data;
} VerifiedBoot_Data_t;

#endif

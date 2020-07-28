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
 * @file   TlApiVerifiedBoot.h
 * @brief  Contains API interface for verified boot
 *
 */
#ifndef __TLAPIVERIFIEDBOOT_H__
#define __TLAPIVERIFIEDBOOT_H__

/**
 * Populate a data structure with data provided by the bootloader.
 */
_TLAPI_EXTERN_C tlApiResult_t tlApiGetVerifiedBootData(VerifiedBoot_Data_t *data_p);

_TLAPI_EXTERN_C tlApiResult_t tlApiVersionEqBoot(VersionInfo_t *NWd);

_TLAPI_EXTERN_C tlApiResult_t tlApiGetDeviceState(uint8_t device_state[KM_DEVICE_STATE_SIZE]);

#endif

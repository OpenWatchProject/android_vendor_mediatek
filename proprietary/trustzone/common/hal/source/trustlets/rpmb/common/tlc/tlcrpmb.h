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

#ifndef TLCRPMB_H_
#define TLCRPMB_H_

#if 1
#include <tee_client_types.h>
#include <tee_client_error.h>

TEEC_Result caOpen(void);

TEEC_Result caRpmbRead(
    const void* plainText, uint32_t plainTextLength,
    uint32_t* result);

TEEC_Result caRpmbWrite(
    const void* plainText, uint32_t plainTextLength,
    uint32_t* result);

#ifdef CFG_RPMB_KEY_PROGRAMED_IN_KERNEL
TEEC_Result caRpmbSetKey(uint32_t *result);
#endif

TEEC_Result caRpmbReadByOffset(
    const void* plainText, uint32_t plainTextLength,
    uint32_t offset, uint32_t userID, uint32_t* result);

TEEC_Result caRpmbWriteByOffset(
    const void* plainText, uint32_t plainTextLength,
    uint32_t offset, uint32_t userID, uint32_t* result);

void caClose(void);

#else
#include "MobiCoreDriverApi.h"

mcResult_t tlcOpen(void);

//mcResult_t add(uint32_t *num1, uint32_t *num2, uint32_t *result);
//mcResult_t minus(uint32_t *num1, uint32_t *num2, uint32_t *result);

mcResult_t read(uint32_t *buf, uint32_t size, uint32_t *result);
mcResult_t write(uint32_t *buf, uint32_t size, uint32_t *result);

void tlcClose(void);
#endif



#endif // TLCRPMB_H_

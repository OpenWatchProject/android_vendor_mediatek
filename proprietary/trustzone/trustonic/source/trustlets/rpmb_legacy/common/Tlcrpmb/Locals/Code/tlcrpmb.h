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

#include "MobiCoreDriverApi.h"

mcResult_t tlcOpen(void);

//mcResult_t add(uint32_t *num1, uint32_t *num2, uint32_t *result);
//mcResult_t minus(uint32_t *num1, uint32_t *num2, uint32_t *result);

mcResult_t read(uint32_t *buf, uint32_t size, uint32_t *result);
mcResult_t write(uint32_t *buf, uint32_t size, uint32_t *result);
#ifdef CFG_RPMB_KEY_PROGRAMED_IN_KERNEL
mcResult_t setkey(uint32_t *result);
#endif


void tlcClose(void);

#endif // TLCRPMB_H_

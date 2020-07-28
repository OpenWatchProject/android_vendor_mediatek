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
 * @file   TlApiGatekeeper.h
 * @brief  Contains API interface for Gatekeeper storage
 *
 */
#ifndef __TLAPIGATEKEEPER_H__
#define __TLAPIGATEKEEPER_H__

_TLAPI_EXTERN_C tlApiResult_t tlApiWriteTransientFailureRecord(uint32_t uid, const tee_failure_record_t* const failure_record);
_TLAPI_EXTERN_C tlApiResult_t tlApiReadTransientFailureRecord(uint32_t uid, const tee_failure_record_t* const failure_record);

#endif

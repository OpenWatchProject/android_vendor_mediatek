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
 * @file   Gatekeeper_types.h
 * @brief  Contains data structures related to Gatekeeper
 * Note: Some are Re-definition to avoid major rework
 */
#ifndef __GATEKEEPER_TYPES_H__
#define __GATEKEEPER_TYPES_H__

/*
 * Defines structure of a failure record kept in the driver
 */
typedef struct {
    uint64_t secure_user_id;
    uint64_t last_checked_timestamp;
    uint32_t uid;
    uint16_t counter;
    uint16_t version;
} Android_FailureRecord_t;

typedef struct __attribute__((packed))  { // original definition: gatekeeper.h
    uint64_t secure_user_id;
    uint64_t last_checked_timestamp;
    uint16_t failure_counter;
    uint16_t version;
} tee_failure_record_t;

#endif

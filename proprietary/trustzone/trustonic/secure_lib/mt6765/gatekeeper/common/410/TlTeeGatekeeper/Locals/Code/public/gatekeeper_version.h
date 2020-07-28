/*
 * Copyright (c) 2016 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */

#ifndef __GATEKEEPER_VERSION_H__
#define __GATEKEEPER_VERSION_H__


/**
 * Trustlet Version
 */
#define GATEKEEPER_VERSION_MAJOR 0x0100 /* Google Gatekeeper specification V1.0 */
#define GATEKEEPER_VERSION_MINOR 0x0102 /* Trustonic implementation v1.2 */

typedef struct {
    uint32_t major; /* (defined by Google Gatekeeper API version) */
    uint32_t minor; /* (defined by Trustonic implementation version) */
} gatekeeper_version_t;
#endif


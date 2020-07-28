/*
 * Copyright (c) 2015-2016 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */

#ifndef __gatekeeper_def_H__
#define __gatekeeper_def_H__

/**
 * Trustlet UUID
 */
#define TEE_GATEKEEPER_TL_UUID { { 7, 6, 0x10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } }

/**
 * Command ID's
 */
#define CMD_ID_ENROLL       1
#define CMD_ID_VERIFY       2
#define CMD_ID_DELETE_USER  3
#define CMD_ID_GET_VERSION  4

#endif /* __gatekeeper_def_H__ */

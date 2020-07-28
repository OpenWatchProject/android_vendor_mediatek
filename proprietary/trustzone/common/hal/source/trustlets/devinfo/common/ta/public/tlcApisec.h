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

#ifndef __TLSEC_H__
#define __TLSEC_H__

/*
 * Command ID's for communication CA -> TA.
 */
#define CMD_DEVINFO_GET     1

/*
 * Trustlet UUID.
 */
//05130000 0000 0000 00 00 00 00 00 00 00 00
#define TL_SEC_UUID { 0x05130000, 0x0000, 0x0000, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }}

#endif // __TLSEC_H__

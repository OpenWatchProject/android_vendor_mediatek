/*
 * Copyright (c) 2015-2017 MICROTRUST Incorporated
 * All rights reserved
 *
 * This file and software is confidential and proprietary to MICROTRUST Inc.
 * Unauthorized copying of this file and software is strictly prohibited.
 * You MUST NOT disclose this file and software unless you get a license
 * agreement from MICROTRUST Incorporated.
 */

#ifndef __UT_KEYMASTER_IOCTL_H__
#define __UT_KEYMASTER_IOCTL_H__

__BEGIN_DECLS

#include <stdbool.h>
#include <stdint.h>

#define TEEI_IOC_MAGIC 'T'
#define CMD_KM_MEM_CLEAR _IO(TEEI_IOC_MAGIC, 0x1)
#define CMD_KM_MEM_SEND _IO(TEEI_IOC_MAGIC, 0x2)

extern unsigned char* keymaster_buffer;

int TEE_InvokeCommand(void);
int TEE_Alloc_KM_Buffer_And_Reset();
void TEE_Free_KM_Buffer(void);

__END_DECLS

#endif //__UT_KEYMASTER_IOCTL_H__

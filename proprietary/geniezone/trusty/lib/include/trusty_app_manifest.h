/*
 * Copyright (c) 2013, Google, Inc. All rights reserved
 * Copyright (c) 2012-2013, NVIDIA CORPORATION. All rights reserved
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef __LIB_TRUSTY_APP_MANIFEST_H
#define __LIB_TRUSTY_APP_MANIFEST_H

#include <sys/types.h>
#include <err.h>
#include "trusty_uuid.h"

/*
 * Layout of .trusty_app.manifest section in the trusted application is the
 * required UUID followed by an arbitrary number of configuration options.
 */
typedef struct trusty_app_manifest {
	uuid_t uuid;
	uint32_t config_options[];
} trusty_app_manifest_t;

enum {
	TRUSTY_APP_CONFIG_LAST_ENTRY = 0,
	TRUSTY_APP_CONFIG_KEY_MIN_STACK_SIZE	= 1,
	TRUSTY_APP_CONFIG_KEY_MIN_HEAP_SIZE	= 2,
	TRUSTY_APP_CONFIG_KEY_MAP_MEM		= 3,
	TRUSTY_APP_CONFIG_KEY_MGMT_FLAGS	= 4,
	TRUSTY_APP_CONFIG_KEY_START_PORT	= 5,
	TRUSTY_APP_CONFIG_KEY_THREAD_PRIORITY,
	TRUSTY_APP_CONFIG_KEY_ENABLE_NONSECURE_IPC,
	TRUSTY_APP_CONFIG_KEY_ENABLE_SECURE_IPC,
	TRUSTY_APP_CONFIG_KEY_MAX_IPC_MSG_SIZE,
	TRUSTY_APP_CONFIG_KEY_IRQ,
	TRUSTY_APP_CONFIG_THREAD_NUM,
	TRUSTY_APP_CONFIG_THREAD_STACK_SIZE,
	TRUSTY_APP_CONFIG_SDSP1_FW_ADDR,
	TRUSTY_APP_CONFIG_SDSP1_FW_SIZE,
	TRUSTY_APP_CONFIG_SDSP2_FW_ADDR,
	TRUSTY_APP_CONFIG_SDSP2_FW_SIZE,
#if WITH_HAPP_THIN_EL1_SUPPORT
	TRUSTY_APP_CONFIG_ENABLE_THIN_EL1,
#endif
};
enum trusty_app_mgmt_flags {
	TRUSTY_APP_MGMT_FLAGS_NONE			= 0x0,
	/* Restart the application on exit */
	TRUSTY_APP_MGMT_FLAGS_RESTART_ON_EXIT		= 0x1,
	/* Don't start the application at boot */
	TRUSTY_APP_MGMT_FLAGS_DEFERRED_START		= 0x2,
};

enum {
	APP_THREAD_PRIORITY_LOW		= 4,
	APP_THREAD_PRIORITY_DEFAULT	= 16,
	APP_THREAD_PRIORITY_HIGH	= 20,
};

#define TRUSTY_APP_CONFIG_THREAD_STACK_SIZE(sz) \
	TRUSTY_APP_CONFIG_THREAD_STACK_SIZE, sz

#define TRUSTY_APP_CONFIG_THREAD_NUM(sz) \
	TRUSTY_APP_CONFIG_THREAD_NUM, sz

#define TRUSTY_APP_CONFIG_SDSP1_FW_ADDR(sz) \
	TRUSTY_APP_CONFIG_SDSP1_FW_ADDR, sz

#define TRUSTY_APP_CONFIG_SDSP1_FW_SIZE(sz) \
	TRUSTY_APP_CONFIG_SDSP1_FW_SIZE, sz

#define TRUSTY_APP_CONFIG_SDSP2_FW_ADDR(sz) \
	TRUSTY_APP_CONFIG_SDSP2_FW_ADDR, sz

#define TRUSTY_APP_CONFIG_SDSP2_FW_SIZE(sz) \
	TRUSTY_APP_CONFIG_SDSP2_FW_SIZE, sz

#define TRUSTY_APP_CONFIG_MIN_STACK_SIZE(sz) \
	TRUSTY_APP_CONFIG_KEY_MIN_STACK_SIZE, sz

#define TRUSTY_APP_CONFIG_MIN_HEAP_SIZE(sz) \
	TRUSTY_APP_CONFIG_KEY_MIN_HEAP_SIZE, sz

#define TRUSTY_APP_CONFIG_MAP_MEM(id,off,sz) \
	TRUSTY_APP_CONFIG_KEY_MAP_MEM, id, off, sz

#define TRUSTY_APP_CONFIG_MGMT_FLAGS(mgmt_flags) \
	TRUSTY_APP_CONFIG_KEY_MGMT_FLAGS, mgmt_flags

#define TRUSTY_APP_CONFIG_THREAD_PRIORITY(pri) \
	TRUSTY_APP_CONFIG_KEY_THREAD_PRIORITY, pri

#define TRUSTY_APP_CONFIG_ENABLE_IPC(nonsecure, secure) \
	TRUSTY_APP_CONFIG_KEY_ENABLE_NONSECURE_IPC, nonsecure, \
	TRUSTY_APP_CONFIG_KEY_ENABLE_SECURE_IPC, secure

#define TRUSTY_APP_CONFIG_MAX_IPC_MSG_SIZE(sz) \
	TRUSTY_APP_CONFIG_KEY_MAX_IPC_MSG_SIZE, sz

#define TRUSTY_APP_CONFIG_IRQ(irq) \
	TRUSTY_APP_CONFIG_KEY_IRQ, irq

#if WITH_HAPP_THIN_EL1_SUPPORT
#define TRUSTY_APP_CONFIG_ENABLE_THIN_EL1(enable) \
	TRUSTY_APP_CONFIG_ENABLE_THIN_EL1, enable
#endif

#define TRUSTY_APP_CONFIG_END() \
	TRUSTY_APP_CONFIG_LAST_ENTRY, 0

/* manifest section attributes */
#define TRUSTY_APP_MANIFEST_ATTRS \
	__attribute((aligned(4))) __attribute((section(".trusty_app.manifest")))

static inline int app_get_config_by_key(uint32_t *config_blob, uint32_t key, uint32_t *value)
{
	uint32_t i;

	for (i = 0; config_blob[i] != TRUSTY_APP_CONFIG_LAST_ENTRY; i+=2)
		if (key == config_blob[i]) {
			*value = config_blob[i+1];
			return NO_ERROR;
		}

	return ERR_NOT_FOUND;
}

#endif


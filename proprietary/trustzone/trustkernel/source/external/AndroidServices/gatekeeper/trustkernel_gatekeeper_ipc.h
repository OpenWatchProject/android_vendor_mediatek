/*
 * Copyright (C) 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TRUSTKERNEL_GATEKEEPER_IPC_H
#define TRUSTKERNEL_GATEKEEPER_IPC_H

#include <tee_client_api.h>
#include <password_handle.h>

namespace gatekeeper {
	static const TEEC_UUID UUID_TrustKernelGateKeeper = {
		0x02662e8e, 0xe126, 0x11e5, { 0xb8, 0x6d, 0x9a, 0x79, 0xf0, 0x6e, 0x94, 0x78 } };
}

#define ALLOCATE_SHARED_MEMORY(ctx, sharedMemory, \
		                   sharedMemorySize, \
		                   memoryType) \
	do { \
		(sharedMemory).size = (sharedMemorySize); \
		(sharedMemory).flags = (memoryType); \
		res = AllocateSharedMemory(&(ctx), &(sharedMemory)); \
		if (res != TEEC_SUCCESS) { \
			goto exit; \
		} \
		memset((sharedMemory).buffer, 0, (sharedMemorySize)); \
	} while (0)

#define ALLOCATE_AND_FILL_SHARED_MEMORY(ctx, sharedMemory, \
		                    sharedMemorySize, \
		                    memoryType, copySize, data) \
	do { \
		(sharedMemory).size = (sharedMemorySize); \
		(sharedMemory).flags = (memoryType); \
		res = TEEC_AllocateSharedMemory((ctx), &(sharedMemory)); \
		if (res != TEEC_SUCCESS) { \
			goto exit; \
		} \
		if (data != NULL) { \
			memcpy((sharedMemory)->buffer, (data), (copySize)); \
		} \
	} while (0)

#define SET_SHARED_MEMORY_OPERATION_PARAMETER(param, \
		                          sharedMemoryOffset, \
		                          sharedMemory, \
		                          sharedMemorySize) \
	(param).memref.offset = (sharedMemoryOffset); \
    (param).memref.size = (sharedMemorySize); \
    (param).memref.parent = (sharedMemory);

#define CMD_ENROLL 0
#define CMD_VERIFY 1

#endif


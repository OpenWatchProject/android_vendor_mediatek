/*
 * Copyright (C) 2015 The Android Open Source Project
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

#pragma once

#include <trusty_std.h>

#include "block_device_tipc.h"
#include "crypt.h"
#include "ipc.h"

#define STORAGE_SESSION_MAGIC 0x53535343 // SSSC (Secure Storage Session Context)

/**
 * storage_proxy_session
 * @magic:        a sentinel value used for checking for data corruption.
 *                Initialized to STORAGE_SESSION_MAGIC.
 * @block_device: the file system state
 * @key:          storage encryption key
 * @proxy_ctx:    the context object on the proxy channel
 */
struct storage_session {
	uint32_t magic;
	struct block_device_tipc block_device;
	struct key key;

	struct ipc_channel_context proxy_ctx;
};


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

#include <stdint.h>

#include "ipc.h"
#include "transaction.h"

#define STORAGE_CLIENT_SESSION_MAGIC 0x53435343 // SCSC (Storage Client Session Context)

struct file_handle;

/*
 * Structure that tracks state associated with a session.
 */
struct storage_client_session {
	uint32_t magic;
	struct transaction tr;
	uuid_t uuid;
	struct file_handle **files;
	size_t files_count;

	struct ipc_channel_context context;
};

/*
 * Copyright (C) 2015-2016 The Android Open Source Project
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

#include <trusty_std.h>

#include <interface/storage/storage.h>

#include "block_cache.h"
#include "ipc.h"
#include "proxy.h"
#include "tipc_limits.h"

#define SS_ERR(args...)		fprintf(stderr, "ss: " args)

int main(void)
{
	struct ipc_port_context ctx = {
		.ops = {
			.on_connect = proxy_connect,
		}
	};

	block_cache_init();

	int rc = ipc_port_create(&ctx, STORAGE_DISK_PROXY_PORT, 1, STORAGE_MAX_BUFFER_SIZE,
				 IPC_PORT_ALLOW_TA_CONNECT | IPC_PORT_ALLOW_NS_CONNECT);

	if (rc < 0) {
		SS_ERR("fatal: unable to initialize proxy endpoint (%d)\n", rc);
		return rc;
	}

	ipc_loop();

	ipc_port_destroy(&ctx);
	return 0;
}

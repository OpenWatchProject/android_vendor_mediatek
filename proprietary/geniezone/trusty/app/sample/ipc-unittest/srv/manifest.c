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

#include <trusty_app_manifest.h>
#include <stddef.h>
#include <stdio.h>

#include <app/ipc_unittest/uuids.h>

trusty_app_manifest_t TRUSTY_APP_MANIFEST_ATTRS trusty_app_manifest =
{
	.uuid = IPC_UNITTEST_SRV_APP_UUID,

	/* optional configuration options here */
	{
		/* four pages for heap */
		TRUSTY_APP_CONFIG_MIN_HEAP_SIZE(4 * 4096),

		/* 2 pages for stack */
		TRUSTY_APP_CONFIG_MIN_STACK_SIZE(2 * 4096),
	},
};


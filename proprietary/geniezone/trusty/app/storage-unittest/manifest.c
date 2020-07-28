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

#include <trusty_app_manifest.h>
#include <stddef.h>
#include <stdio.h>

/* App UUID:   {1c1c3151-f04f-4012-bfd7-59ab76bc7912} */
#define STORAGE_UNITTEST_UUID  \
	{ 0x1c1c3151, 0xf04f, 0x4012, \
	{ 0xbf, 0xd7, 0x59, 0xab, 0x76, 0xbc, 0x79, 0x12 } }

trusty_app_manifest_t TRUSTY_APP_MANIFEST_ATTRS trusty_app_manifest =
{
	.uuid = STORAGE_UNITTEST_UUID,

	/* optional configuration options here */
	{
		/* 16 pages for heap */
		TRUSTY_APP_CONFIG_MIN_HEAP_SIZE(16 * 4096),

		/* 2 pages for stack */
		TRUSTY_APP_CONFIG_MIN_STACK_SIZE(2 * 4096),
	},
};




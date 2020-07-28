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

/* App UUID:   {ab742471-d6e6-4806-85f6-0555b024f4da} */
#define HWCRYPTO_UNITTEST_UUID \
	{ 0xab742471, 0xd6e6, 0x4806, \
	  { 0x85, 0xf6, 0x05, 0x55, 0xb0, 0x24, 0xf4, 0xda } }

trusty_app_manifest_t TRUSTY_APP_MANIFEST_ATTRS trusty_app_manifest =
{
	.uuid = HWCRYPTO_UNITTEST_UUID,

	/* optional configuration options here */
	{
		/* one page for heap */
		TRUSTY_APP_CONFIG_MIN_HEAP_SIZE(4096),

		/* one page for stack */
		TRUSTY_APP_CONFIG_MIN_STACK_SIZE(4096),
	},
};



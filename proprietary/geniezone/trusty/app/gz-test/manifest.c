/*
 * Copyright (C) 2012-2014 The Android Open Source Project
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

trusty_app_manifest_t TRUSTY_APP_MANIFEST_ATTRS trusty_app_manifest =
{
	/* d2c50855-6f2b-57ee-2db6-7c49944f30a0 */
	{ 0xd2c50855, 0x6f2b, 0x57ee,
	  { 0x2d, 0xb6, 0x7c, 0x49, 0x94, 0x4f, 0x30, 0xa0 } },

	/* optional configuration options here */
	{
		/* four pages for heap */
#ifndef MULTICORE_TEST
		TRUSTY_APP_CONFIG_MIN_HEAP_SIZE(4 * 4096),
#else
		TRUSTY_APP_CONFIG_MIN_HEAP_SIZE(72 *1024 * 1024),
#endif

		/* 2 pages for stack */
		TRUSTY_APP_CONFIG_MIN_STACK_SIZE(2 * 4096),

		TRUSTY_APP_CONFIG_MGMT_FLAGS(TRUSTY_APP_MGMT_FLAGS_NONE),

		TRUSTY_APP_CONFIG_THREAD_NUM(4),

	},
};

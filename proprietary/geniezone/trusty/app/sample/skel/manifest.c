/*
 * Copyright (C) 2012-2013 The Android Open Source Project
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
	/* UUID : {eca48f94-00aa-560e-8f8c-d94b50d484f3} */
	{ 0xeca48f94, 0x00aa, 0x560e,
	  { 0x8f, 0x8c, 0xd9, 0x4b, 0x50, 0xd4, 0x84, 0xf3 } },

	/* optional configuration options here */
	{
		/* four pages for heap */
		TRUSTY_APP_CONFIG_MIN_HEAP_SIZE(4 * 4096),

		/* request two I/O mappings */
		TRUSTY_APP_CONFIG_MAP_MEM(1, 0x70000000, 0x1000),
		TRUSTY_APP_CONFIG_MAP_MEM(2, 0x70000804, 0x4)
	},
};

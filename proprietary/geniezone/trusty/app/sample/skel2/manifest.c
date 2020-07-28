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
	/* UUID : {f7fc6e07-78d8-5efa-b4a9-ecf5e077fd63} */
	{ 0xf7fc6e07, 0x78d8, 0x5efa,
	  { 0xb4, 0xa9, 0xec, 0xf5, 0xe0, 0x77, 0xfd, 0x63 } },

	/* optional configuration options here */
	{
		/* four pages for heap */
		TRUSTY_APP_CONFIG_MIN_HEAP_SIZE(4 * 4096),

		/* request two I/O mappings */
		TRUSTY_APP_CONFIG_MAP_MEM(1, 0x70000000, 0x1000),
		TRUSTY_APP_CONFIG_MAP_MEM(2, 0x70000804, 0x4)
	},
};

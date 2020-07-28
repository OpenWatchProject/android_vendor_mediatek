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

#include <stddef.h>
#include <trusty_app_manifest.h>

trusty_app_manifest_t TRUSTY_APP_MANIFEST_ATTRS trusty_app_manifest =
{
	/* UUID : {7445ca05-35d5-4698-bb9a-0d840f9a5fcf} */
	{ 0x7445ca05, 0x35d5, 0x4698,
	  { 0xbb, 0x9a, 0x0d, 0x84, 0x0f, 0x9a, 0x5f, 0xcf } },

	/* optional configuration options here */
	{
		TRUSTY_APP_CONFIG_MIN_HEAP_SIZE(24 * 4096),
		TRUSTY_APP_CONFIG_MIN_STACK_SIZE(8 * 4096),
	},
};

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
#include <stdlib.h>

#ifdef SUPPORT_sDSP_0
#include "vpu_main_sDSP_0.c"
#endif
#ifdef SUPPORT_sDSP_1
#include "vpu_main_sDSP_1.c"
#endif

trusty_app_manifest_t TRUSTY_APP_MANIFEST_ATTRS trusty_app_manifest =
{
	/* 2ec89cac-c16e-11e8-a355-529269fb1459 */
	{ 0x2ec89cac, 0xc16e, 0x11e8,
	  { 0xa3, 0x55, 0x52, 0x92, 0x69, 0xfb, 0x14, 0x59 } },

	/* optional configuration options here */
	{
		/* four pages for heap */
		TRUSTY_APP_CONFIG_MIN_HEAP_SIZE(4 * 4096),

		/* 2 pages for stack */
		TRUSTY_APP_CONFIG_MIN_STACK_SIZE(1024 * 4),

		TRUSTY_APP_CONFIG_MGMT_FLAGS(TRUSTY_APP_MGMT_FLAGS_RESTART_ON_EXIT),

#ifdef SUPPORT_sDSP_0
		TRUSTY_APP_CONFIG_SDSP1_FW_ADDR((uint32_t)vpu_main_sDSP_0),
		TRUSTY_APP_CONFIG_SDSP1_FW_SIZE(sizeof(vpu_main_sDSP_0)),
#endif

#ifdef SUPPORT_sDSP_1
		TRUSTY_APP_CONFIG_SDSP2_FW_ADDR((uint32_t)vpu_main_sDSP_1),
		TRUSTY_APP_CONFIG_SDSP2_FW_SIZE(sizeof(vpu_main_sDSP_1)),
#endif
	},
};

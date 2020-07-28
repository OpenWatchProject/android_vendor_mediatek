/*
 * Copyright (C) 2016 The Android Open Source Project
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

#include "block_cache_priv.h"

trusty_app_manifest_t TRUSTY_APP_MANIFEST_ATTRS trusty_app_manifest =
{
    .uuid = { /* cea8706d-6cb4-49f3-b994-29e0e478bd29 */
        0xcea8706d, 0x6cb4, 0x49f3,
        { 0xb9, 0x94, 0x29, 0xe0, 0xe4, 0x78, 0xbd, 0x29 }
    },
    {
        TRUSTY_APP_CONFIG_MIN_STACK_SIZE(4 * 4096),
        TRUSTY_APP_CONFIG_MIN_HEAP_SIZE(8 * 4096 + BLOCK_CACHE_SIZE_BYTES),
        TRUSTY_APP_CONFIG_MGMT_FLAGS(TRUSTY_APP_MGMT_FLAGS_RESTART_ON_EXIT),
    },
};

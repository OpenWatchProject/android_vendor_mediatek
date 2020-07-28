/*
 * Copyright (C) 2011-2014 MediaTek Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __GRALLOC_MTK_DEFS_H__
#define __GRALLOC_MTK_DEFS_H__

#include <stdint.h>
#include <hardware/gralloc1.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * [Important]
 * Define the MTK specific enumeration values for gralloc usage in order to avoid "polute" AOSP file
 * (hardware/libhardware/include/hardware/gralloc1.h)
 * The enumeration value definition must not be conflict with the gralloc1.h in original AOSP file
 *
 * [For Secure Usages]
 * User should attach AOSP GRALLOC1_PRODUCER_USAGE_PROTECTED before using MTK secure usages
 */
enum {
    /// the following define the extended gralloc enumeration value of
    GRALLOC1_USAGE_HW_CAMERA_ZSL        = (GRALLOC1_PRODUCER_USAGE_CAMERA | GRALLOC1_CONSUMER_USAGE_CAMERA),
    GRALLOC1_USAGE_SECURE               = (GRALLOC1_PRODUCER_USAGE_PROTECTED | GRALLOC1_PRODUCER_USAGE_PRIVATE_0), /* MTK secure usage */
    GRALLOC1_USAGE_CAMERA_ORIENTATION   = GRALLOC1_PRODUCER_USAGE_PRIVATE_1,
    GRALLOC1_USAGE_CAMERA               = GRALLOC1_PRODUCER_USAGE_PRIVATE_2,
    GRALLOC1_USAGE_NULL_BUFFER          = GRALLOC1_PRODUCER_USAGE_PRIVATE_3,
    GRALLOC1_USAGE_NO_COMPRESS          = GRALLOC1_PRODUCER_USAGE_PRIVATE_8,
    GRALLOC1_USAGE_SMVR                 = GRALLOC1_PRODUCER_USAGE_PRIVATE_1,
    GRALLOC1_USAGE_2D_FR                = (GRALLOC1_PRODUCER_USAGE_PROTECTED | GRALLOC1_PRODUCER_USAGE_PRIVATE_2), /* MTK secure usage */
    GRALLOC1_USAGE_PROT                 = (GRALLOC1_PRODUCER_USAGE_PROTECTED | GRALLOC1_CONSUMER_USAGE_CAMERA | GRALLOC1_PRODUCER_USAGE_CAMERA), /* MTK secure usage */
    GRALLOC1_USAGE_PROT_CONSUMER        = (GRALLOC1_PRODUCER_USAGE_PROTECTED | GRALLOC1_CONSUMER_USAGE_CAMERA), /* MTK secure usage */
    GRALLOC1_USAGE_PROT_PRODUCER        = (GRALLOC1_PRODUCER_USAGE_PROTECTED | GRALLOC1_PRODUCER_USAGE_CAMERA), /* MTK secure usage */
};

#ifdef __cplusplus
}
#endif

#endif /* __GRALLOC_MTK_DEFS_H__ */

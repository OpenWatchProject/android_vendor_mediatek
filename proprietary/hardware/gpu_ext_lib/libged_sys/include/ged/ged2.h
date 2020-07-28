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

#ifndef __GED2_H__
#define __GED2_H__

#include <stdint.h>
#include "ged_type.h"

#include <vendor/mediatek/hardware/gpu/1.0/IGraphicExt.h>
using ::vendor::mediatek::hardware::gpu::V1_0::GED_HIDL_HANDLE;


GED_HIDL_HANDLE  ged_create_2(void);

void ged_destroy_2(GED_HIDL_HANDLE ged);



#endif

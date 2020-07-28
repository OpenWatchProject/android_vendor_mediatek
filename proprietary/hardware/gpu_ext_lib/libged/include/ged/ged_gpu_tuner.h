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

#ifndef __GED_GPU_TUNER_H__
#define __GED_GPU_TUNER_H__

#if defined (__cplusplus)
extern "C" {
#endif

#define GT_MAKE_BIT(start_bit, index) ( (index##u) << (start_bit) )
typedef enum {
	MTK_GPU_TUNER_ANISOTROPIC_DISABLE = GT_MAKE_BIT(0, 1),
	MTK_GPU_TUNER_TRILINEAR_DISABLE = GT_MAKE_BIT(1, 1),
}GPU_TUNER_FEATURE;


int ged_gpu_tuner_get_status(void *out);

#if defined (__cplusplus)
}
#endif

#endif


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

#include "ge_platform.h"

#include <system/graphics.h>
#include <graphics_mtk_defs.h>
#include <gralloc1_mtk_defs.h>

static int _d_plt_gralloc_extra_get_platform_format(int in_format, uint64_t usage)
{
    if (in_format == HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED)
    {
        int format = HAL_PIXEL_FORMAT_RGBA_8888;

        const uint64_t u_video = GRALLOC1_CONSUMER_USAGE_VIDEO_ENCODER;
        const uint64_t u_camera = GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_camera_texture = GRALLOC1_CONSUMER_USAGE_GPU_TEXTURE | GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_camera_composer = GRALLOC1_CONSUMER_USAGE_HWCOMPOSER | GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_camera_zsl      = GRALLOC1_CONSUMER_USAGE_CAMERA | GRALLOC1_PRODUCER_USAGE_CAMERA;

        if ((usage & u_video) == u_video)
            format = HAL_PIXEL_FORMAT_YV12;
        else if ((usage & u_camera_zsl) == u_camera_zsl)
            format = HAL_PIXEL_FORMAT_CAMERA_OPAQUE;
        else if ((usage & u_camera_texture) == u_camera_texture)
            format = HAL_PIXEL_FORMAT_YV12;
        else if ((usage & u_camera_composer) == u_camera_composer)
            format = HAL_PIXEL_FORMAT_YV12;
        else if ((usage & u_camera) == u_camera)
            format = HAL_PIXEL_FORMAT_YCbCr_422_I;

        return format;
    }

    if (in_format == HAL_PIXEL_FORMAT_YCbCr_420_888)
    {
        int format = HAL_PIXEL_FORMAT_YCbCr_420_888;

        const uint64_t u_video = GRALLOC1_CONSUMER_USAGE_VIDEO_ENCODER;
        const uint64_t u_camera = GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_camera_texture = GRALLOC1_CONSUMER_USAGE_GPU_TEXTURE | GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_camera_composer = GRALLOC1_CONSUMER_USAGE_HWCOMPOSER | GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_cpu_read = GRALLOC1_PRODUCER_USAGE_CPU_READ_OFTEN;
        const uint64_t u_cpu_write = GRALLOC1_PRODUCER_USAGE_CPU_WRITE_OFTEN;

        if ((usage & u_video) == u_video)
            format = HAL_PIXEL_FORMAT_YV12;
        if ((usage & u_camera_texture) == u_camera_texture)
            format = HAL_PIXEL_FORMAT_YV12;
        else if ((usage & u_camera_composer) == u_camera_composer)
            format = HAL_PIXEL_FORMAT_YV12;
        else if ( ((usage & u_camera) == u_camera) &&
                  (((usage & u_cpu_read) == u_cpu_read) || ((usage & u_cpu_write) == u_cpu_write)) )
        {
            format = HAL_PIXEL_FORMAT_YCrCb_420_SP;
        }
        else if ((usage & u_camera) == u_camera)
            format = HAL_PIXEL_FORMAT_YV12;

        return format;
    }

    return in_format;
}

ge_platform_fn ge_platform =
{
    .gralloc_extra_get_platform_format = &_d_plt_gralloc_extra_get_platform_format,
};

__attribute__((constructor))
    static void init_ge_platform() { ge_platform_wrap_init(&ge_platform); }

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

#define LOG_TAG "GE"
#include <cutils/log.h>

#include <system/graphics.h>
#include <graphics_mtk_defs.h>
#include <gralloc1_mtk_defs.h>
#include <cutils/properties.h>
#include <string.h>
#include "ge_platform.h"

#ifdef GE_PIXEL_FORMAT_IMPL_CUSTOMIZATION
#include GE_PIXEL_FORMAT_CUSTOM_HEADER
#else
#include "default.h"
#endif

static inline bool _has_usage(uint64_t usage_flag, uint64_t usage)
{
    return (usage & usage_flag) == usage_flag;
}

static int _plt_gralloc_extra_get_platform_format_mt6735(int in_format, uint64_t usage)
{
    if (in_format == HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED)
    {
        int format = HAL_PIXEL_FORMAT_RGBA_8888;

        const uint64_t u_video = GRALLOC1_CONSUMER_USAGE_VIDEO_ENCODER;
        const uint64_t u_camera_producer = GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_camera_texture = GRALLOC1_CONSUMER_USAGE_GPU_TEXTURE | GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_camera_composer = GRALLOC1_CONSUMER_USAGE_HWCOMPOSER | GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_cpu_read = GRALLOC1_PRODUCER_USAGE_CPU_READ_OFTEN;
        const uint64_t u_cpu_write = GRALLOC1_PRODUCER_USAGE_CPU_WRITE_OFTEN;

        if ((usage & u_video) == u_video)
            format = HAL_PIXEL_FORMAT_YV12;
        else if ((usage & u_camera_texture) == u_camera_texture)
            format = HAL_PIXEL_FORMAT_YV12;
        else if ((usage & u_camera_composer) == u_camera_composer)
            format = HAL_PIXEL_FORMAT_YV12;
        else if (((usage & u_camera_producer) == u_camera_producer) &&
            (((usage & u_cpu_read) == u_cpu_read) || ((usage & u_cpu_write) == u_cpu_write))){
            format = HAL_PIXEL_FORMAT_YV12;
        }
        else if ((usage & u_camera_producer) == u_camera_producer)
            format = HAL_PIXEL_FORMAT_YCbCr_422_I;

        return format;
    }

    if (in_format == HAL_PIXEL_FORMAT_YCbCr_420_888)
    {
        int format = HAL_PIXEL_FORMAT_YCbCr_420_888;

        const uint64_t u_video = GRALLOC1_CONSUMER_USAGE_VIDEO_ENCODER;
        const uint64_t u_camera_producer = GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_camera_texture = GRALLOC1_CONSUMER_USAGE_GPU_TEXTURE | GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_camera_composer = GRALLOC1_CONSUMER_USAGE_HWCOMPOSER | GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_cpu_read = GRALLOC1_PRODUCER_USAGE_CPU_READ_OFTEN;
        const uint64_t u_cpu_write = GRALLOC1_PRODUCER_USAGE_CPU_WRITE_OFTEN;

        if ((usage & u_camera_texture) == u_camera_texture)
            format = HAL_PIXEL_FORMAT_YV12;
        else if ((usage & u_camera_composer) == u_camera_composer)
            format = HAL_PIXEL_FORMAT_YV12;
        else if (((usage & u_camera_producer) == u_camera_producer) &&
            (((usage & u_cpu_read) == u_cpu_read) || ((usage & u_cpu_write) == u_cpu_write))){
            format = HAL_PIXEL_FORMAT_YV12;
        }
        else if ((usage & u_camera_producer) == u_camera_producer)
            format = HAL_PIXEL_FORMAT_YV12;
        else if ((usage & u_video) == u_video)
            format = HAL_PIXEL_FORMAT_YV12;

        return format;
    }

    return in_format;
}

static int _plt_gralloc_extra_get_platform_format_mt6739(int in_format, uint64_t usage)
{
    if (in_format == HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED)
    {
        int format = HAL_PIXEL_FORMAT_RGBA_8888;

        const uint64_t u_video = GRALLOC1_CONSUMER_USAGE_VIDEO_ENCODER;
        const uint64_t u_camera_producer = GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_camera_texture = GRALLOC1_CONSUMER_USAGE_GPU_TEXTURE | GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_camera_composer = GRALLOC1_CONSUMER_USAGE_HWCOMPOSER | GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_cpu_read = GRALLOC1_PRODUCER_USAGE_CPU_READ_OFTEN;
        const uint64_t u_cpu_write = GRALLOC1_PRODUCER_USAGE_CPU_WRITE_OFTEN;

        if ((usage & u_video) == u_video)
                format = HAL_PIXEL_FORMAT_YV12;
        else if ((usage & u_camera_texture) == u_camera_texture)
                format = HAL_PIXEL_FORMAT_YV12;
        else if ((usage & u_camera_composer) == u_camera_composer)
                format = HAL_PIXEL_FORMAT_YV12;
        else if (((usage & u_camera_producer) == u_camera_producer) &&
            (((usage & u_cpu_read) == u_cpu_read) || ((usage & u_cpu_write) == u_cpu_write))){
                format = HAL_PIXEL_FORMAT_YCrCb_420_SP;
        }
        else if ((usage & u_camera_producer) == u_camera_producer)
                format = HAL_PIXEL_FORMAT_YCbCr_422_I;

        return format;
    }

    if (in_format == HAL_PIXEL_FORMAT_YCbCr_420_888)
    {
        int format = HAL_PIXEL_FORMAT_YCbCr_420_888;

        const uint64_t u_video = GRALLOC1_CONSUMER_USAGE_VIDEO_ENCODER;
        const uint64_t u_camera_producer = GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_camera_texture = GRALLOC1_CONSUMER_USAGE_GPU_TEXTURE | GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_camera_composer = GRALLOC1_CONSUMER_USAGE_HWCOMPOSER | GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_cpu_read = GRALLOC1_PRODUCER_USAGE_CPU_READ_OFTEN;
        const uint64_t u_cpu_write = GRALLOC1_PRODUCER_USAGE_CPU_WRITE_OFTEN;

        if ((usage & u_camera_texture) == u_camera_texture)
                format = HAL_PIXEL_FORMAT_YV12;
        else if ((usage & u_camera_composer) == u_camera_composer)
                format = HAL_PIXEL_FORMAT_YV12;
        else if (((usage & u_camera_producer) == u_camera_producer) &&
            (((usage & u_cpu_read) == u_cpu_read) || ((usage & u_cpu_write) == u_cpu_write))){
                format = HAL_PIXEL_FORMAT_YCrCb_420_SP;
        }
        else if ((usage & u_camera_producer) == u_camera_producer)
                format = HAL_PIXEL_FORMAT_YV12;

        return format;
    }

    return in_format;
}

static int _plt_gralloc_extra_get_platform_format_mt6755(int in_format, uint64_t usage)
{
    if (in_format == HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED)
    {
        int format = HAL_PIXEL_FORMAT_RGBA_8888;

        const uint64_t u_video = GRALLOC1_CONSUMER_USAGE_VIDEO_ENCODER;
        const uint64_t u_camera_producer = GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_camera_texture = GRALLOC1_CONSUMER_USAGE_GPU_TEXTURE | GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_camera_composer = GRALLOC1_CONSUMER_USAGE_HWCOMPOSER | GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_cpu_read = GRALLOC1_PRODUCER_USAGE_CPU_READ_OFTEN;
        const uint64_t u_cpu_write = GRALLOC1_PRODUCER_USAGE_CPU_WRITE_OFTEN;

        if ((usage & u_video) == u_video)
            format = HAL_PIXEL_FORMAT_YV12;
        else if ((usage & u_camera_texture) == u_camera_texture)
            format = HAL_PIXEL_FORMAT_YV12;
        else if ((usage & u_camera_composer) == u_camera_composer)
            format = HAL_PIXEL_FORMAT_YV12;
        else if (((usage & u_camera_producer) == u_camera_producer) &&
            (((usage & u_cpu_read) == u_cpu_read) || ((usage & u_cpu_write) == u_cpu_write))){
            format = HAL_PIXEL_FORMAT_YCrCb_420_SP;
        }
        else if ((usage & u_camera_producer) == u_camera_producer)
            format = HAL_PIXEL_FORMAT_YCbCr_422_I;

        return format;
    }

    if (in_format == HAL_PIXEL_FORMAT_YCbCr_420_888)
    {
        int format = HAL_PIXEL_FORMAT_YCbCr_420_888;

        const uint64_t u_video = GRALLOC1_CONSUMER_USAGE_VIDEO_ENCODER;
        const uint64_t u_camera_producer = GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_camera_texture = GRALLOC1_CONSUMER_USAGE_GPU_TEXTURE | GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_camera_composer = GRALLOC1_CONSUMER_USAGE_HWCOMPOSER | GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_cpu_read = GRALLOC1_PRODUCER_USAGE_CPU_READ_OFTEN;
        const uint64_t u_cpu_write = GRALLOC1_PRODUCER_USAGE_CPU_WRITE_OFTEN;

        if ((usage & u_camera_texture) == u_camera_texture)
            format = HAL_PIXEL_FORMAT_YV12;
        else if (((usage & u_camera_producer) == u_camera_producer) &&
            (((usage & u_cpu_read) == u_cpu_read) || ((usage & u_cpu_write) == u_cpu_write))){
            format = HAL_PIXEL_FORMAT_YCrCb_420_SP;
        }
        else if ((usage & u_camera_composer) == u_camera_composer)
            format = HAL_PIXEL_FORMAT_YV12;
        else if ((usage & u_camera_producer) == u_camera_producer)
            format = HAL_PIXEL_FORMAT_YV12;

        return format;
    }

    return in_format;
}

static int _plt_gralloc_extra_get_platform_format_mt6761(int in_format, uint64_t usage)
{
    if (in_format == HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED)
    {
        int format = HAL_PIXEL_FORMAT_RGBA_8888;
        const uint64_t u_video = GRALLOC1_CONSUMER_USAGE_VIDEO_ENCODER;
        const uint64_t u_camera_producer = GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_camera_texture = GRALLOC1_CONSUMER_USAGE_GPU_TEXTURE | GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_camera_composer = GRALLOC1_CONSUMER_USAGE_HWCOMPOSER | GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_camera_zsl      = GRALLOC1_CONSUMER_USAGE_CAMERA | GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_cpu_read = GRALLOC1_PRODUCER_USAGE_CPU_READ_OFTEN;
        const uint64_t u_cpu_write = GRALLOC1_PRODUCER_USAGE_CPU_WRITE_OFTEN;

        if ((usage & u_video) == u_video)
            format = HAL_PIXEL_FORMAT_VIDEO;
        else if ((usage & u_camera_composer) == u_camera_composer)
            format = HAL_PIXEL_FORMAT_CAM_COMPO;
        else if ((usage & u_camera_texture) == u_camera_texture)
            format = HAL_PIXEL_FORMAT_CAM_TEX;
        else if (((usage & u_camera_producer) == u_camera_producer) &&
            (((usage & u_cpu_read) == u_cpu_read) || ((usage & u_cpu_write) == u_cpu_write))){
            format = HAL_PIXEL_FORMAT_YCrCb_420_SP;
        }
        else if ((usage & u_camera_zsl) == u_camera_zsl)
            format = HAL_PIXEL_FORMAT_CAM_ZS1;
        else if ((usage & u_camera_producer) == u_camera_producer)
            format = HAL_PIXEL_FORMAT_CAM;

        return format;
    }

    if (in_format == HAL_PIXEL_FORMAT_YCbCr_420_888)
    {
        int format = HAL_PIXEL_FORMAT_YCbCr_420_888;
        const uint64_t u_video = GRALLOC1_CONSUMER_USAGE_VIDEO_ENCODER;
        const uint64_t u_camera_producer = GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_camera_texture = GRALLOC1_CONSUMER_USAGE_GPU_TEXTURE | GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_camera_composer = GRALLOC1_CONSUMER_USAGE_HWCOMPOSER | GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_cpu_read = GRALLOC1_PRODUCER_USAGE_CPU_READ_OFTEN;
        const uint64_t u_cpu_write = GRALLOC1_PRODUCER_USAGE_CPU_WRITE_OFTEN;

        if ((usage & u_camera_texture) == u_camera_texture)
            format = HAL_PIXEL_FORMAT_YV12;
        else if ((usage & u_camera_composer) == u_camera_composer)
            format = HAL_PIXEL_FORMAT_YV12;
        else if (((usage & u_camera_producer) == u_camera_producer) &&
            (((usage & u_cpu_read) == u_cpu_read) || ((usage & u_cpu_write) == u_cpu_write))){
            format = HAL_PIXEL_FORMAT_YCrCb_420_SP;
        }
        else if ((usage & u_camera_producer) == u_camera_producer)
            format = HAL_PIXEL_FORMAT_YV12;

        return format;
    }

    return in_format;
}

static int _plt_gralloc_extra_get_platform_format_mt6763(int in_format, uint64_t usage)
{
    if (in_format == HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED)
    {
        int format = HAL_PIXEL_FORMAT_RGBA_8888;
        const uint64_t u_video = GRALLOC1_CONSUMER_USAGE_VIDEO_ENCODER;
        const uint64_t u_camera_producer = GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_camera_texture = GRALLOC1_CONSUMER_USAGE_GPU_TEXTURE | GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_camera_composer = GRALLOC1_CONSUMER_USAGE_HWCOMPOSER | GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_camera_zsl      = GRALLOC1_CONSUMER_USAGE_CAMERA | GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_cpu_read = GRALLOC1_PRODUCER_USAGE_CPU_READ_OFTEN;
        const uint64_t u_cpu_write = GRALLOC1_PRODUCER_USAGE_CPU_WRITE_OFTEN;

        if ((usage & u_video) == u_video)
            format = HAL_PIXEL_FORMAT_YV12;
        else if ((usage & u_camera_composer) == u_camera_composer)
            format = HAL_PIXEL_FORMAT_YV12;
        else if ((usage & u_camera_texture) == u_camera_texture)
            format = HAL_PIXEL_FORMAT_YV12;
        else if (((usage & u_camera_producer) == u_camera_producer) &&
            (((usage & u_cpu_read) == u_cpu_read) || ((usage & u_cpu_write) == u_cpu_write))){
            format = HAL_PIXEL_FORMAT_YCrCb_420_SP;
        }
        else if ((usage & u_camera_zsl) == u_camera_zsl)
            format = HAL_PIXEL_FORMAT_CAMERA_OPAQUE;
        else if ((usage & u_camera_producer) == u_camera_producer)
            format = HAL_PIXEL_FORMAT_YCbCr_422_I;
        return format;
    }
    if (in_format == HAL_PIXEL_FORMAT_YCbCr_420_888)
    {
        int format = HAL_PIXEL_FORMAT_YCbCr_420_888;
        const uint64_t u_video = GRALLOC1_CONSUMER_USAGE_VIDEO_ENCODER;
        const uint64_t u_camera_producer = GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_camera_texture = GRALLOC1_CONSUMER_USAGE_GPU_TEXTURE | GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_camera_composer = GRALLOC1_CONSUMER_USAGE_HWCOMPOSER | GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_cpu_read = GRALLOC1_PRODUCER_USAGE_CPU_READ_OFTEN;
        const uint64_t u_cpu_write = GRALLOC1_PRODUCER_USAGE_CPU_WRITE_OFTEN;

        if ((usage & u_camera_texture) == u_camera_texture)
            format = HAL_PIXEL_FORMAT_YV12;
        else if ((usage & u_camera_composer) == u_camera_composer)
            format = HAL_PIXEL_FORMAT_YV12;
        else if (((usage & u_camera_producer) == u_camera_producer) &&
            (((usage & u_cpu_read) == u_cpu_read) || ((usage & u_cpu_write) == u_cpu_write))){
            format = HAL_PIXEL_FORMAT_YCrCb_420_SP;
        }
        else if ((usage & u_camera_producer) == u_camera_producer)
            format = HAL_PIXEL_FORMAT_YV12;
        return format;
    }
    return in_format;
}

int _plt_gralloc_extra_get_platform_format_mt6765(int in_format, uint64_t usage)
{
    if (in_format == HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED)
    {
        int format = HAL_PIXEL_FORMAT_RGBA_8888;
        const uint64_t u_video = GRALLOC1_CONSUMER_USAGE_VIDEO_ENCODER;
        const uint64_t u_camera_producer = GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_camera_texture = GRALLOC1_CONSUMER_USAGE_GPU_TEXTURE | GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_camera_composer = GRALLOC1_CONSUMER_USAGE_HWCOMPOSER | GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_camera_zsl      = GRALLOC1_CONSUMER_USAGE_CAMERA | GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_cpu_read = GRALLOC1_PRODUCER_USAGE_CPU_READ_OFTEN;
        const uint64_t u_cpu_write = GRALLOC1_PRODUCER_USAGE_CPU_WRITE_OFTEN;
        const uint64_t u_protected = GRALLOC1_PRODUCER_USAGE_PROTECTED;

        if (_has_usage(u_protected | u_camera_producer, usage))
            format = HAL_PIXEL_FORMAT_NV12;
        else if ((usage & u_video) == u_video)
            format = HAL_PIXEL_FORMAT_VIDEO;
        else if ((usage & u_camera_composer) == u_camera_composer)
            format = HAL_PIXEL_FORMAT_CAM_COMPO;
        else if ((usage & u_camera_texture) == u_camera_texture)
            format = HAL_PIXEL_FORMAT_CAM_TEX;
        else if (((usage & u_camera_producer) == u_camera_producer) &&
            (((usage & u_cpu_read) == u_cpu_read) || ((usage & u_cpu_write) == u_cpu_write))){
            format = HAL_PIXEL_FORMAT_YCrCb_420_SP;
        }
        else if ((usage & u_camera_zsl) == u_camera_zsl)
            format = HAL_PIXEL_FORMAT_CAM_ZS1;
        else if ((usage & u_camera_producer) == u_camera_producer)
            format = HAL_PIXEL_FORMAT_CAM;

        return format;
    }

    if (in_format == HAL_PIXEL_FORMAT_YCbCr_420_888)
    {
        int format = HAL_PIXEL_FORMAT_YCbCr_420_888;
        const uint64_t u_video = GRALLOC1_CONSUMER_USAGE_VIDEO_ENCODER;
        const uint64_t u_camera_producer = GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_camera_texture = GRALLOC1_CONSUMER_USAGE_GPU_TEXTURE | GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_camera_composer = GRALLOC1_CONSUMER_USAGE_HWCOMPOSER | GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_cpu_read = GRALLOC1_PRODUCER_USAGE_CPU_READ_OFTEN;
        const uint64_t u_cpu_write = GRALLOC1_PRODUCER_USAGE_CPU_WRITE_OFTEN;

        if ((usage & u_camera_texture) == u_camera_texture)
            format = HAL_PIXEL_FORMAT_YV12;
        else if ((usage & u_camera_composer) == u_camera_composer)
            format = HAL_PIXEL_FORMAT_YV12;
        else if (((usage & u_camera_producer) == u_camera_producer) &&
            (((usage & u_cpu_read) == u_cpu_read) || ((usage & u_cpu_write) == u_cpu_write))){
            format = HAL_PIXEL_FORMAT_YCrCb_420_SP;
        }
        else if ((usage & u_camera_producer) == u_camera_producer)
            format = HAL_PIXEL_FORMAT_YV12;

        return format;
    }

    return in_format;
}

static int _plt_gralloc_extra_get_platform_format_mt6768(int in_format, uint64_t usage)
{
    if (in_format == HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED)
    {
        int format = HAL_PIXEL_FORMAT_RGBA_8888;

        const uint64_t u_video = GRALLOC1_CONSUMER_USAGE_VIDEO_ENCODER;
        const uint64_t u_camera_producer = GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_camera_texture = GRALLOC1_CONSUMER_USAGE_GPU_TEXTURE | GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_camera_composer = GRALLOC1_CONSUMER_USAGE_HWCOMPOSER | GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_cpu_read = GRALLOC1_PRODUCER_USAGE_CPU_READ_OFTEN;
        const uint64_t u_cpu_write = GRALLOC1_PRODUCER_USAGE_CPU_WRITE_OFTEN;
        const uint64_t u_protected = GRALLOC1_PRODUCER_USAGE_PROTECTED;

        if (_has_usage(u_protected | u_camera_producer, usage))
            format = HAL_PIXEL_FORMAT_NV12;
        else if ((usage & u_video) == u_video)
            format = HAL_PIXEL_FORMAT_YV12;
        else if ((usage & u_camera_texture) == u_camera_texture)
            format = HAL_PIXEL_FORMAT_YV12;
        else if ((usage & u_camera_composer) == u_camera_composer)
            format = HAL_PIXEL_FORMAT_YV12;
        else if (((usage & u_camera_producer) == u_camera_producer) &&
            (((usage & u_cpu_read) == u_cpu_read) || ((usage & u_cpu_write) == u_cpu_write))){
            format = HAL_PIXEL_FORMAT_YCrCb_420_SP;
        }
        else if ((usage & u_camera_producer) == u_camera_producer)
            format = HAL_PIXEL_FORMAT_YV12;

        return format;
    }

    if (in_format == HAL_PIXEL_FORMAT_YCbCr_420_888)
    {
        int format = HAL_PIXEL_FORMAT_YCbCr_420_888;

        const uint64_t u_video = GRALLOC1_CONSUMER_USAGE_VIDEO_ENCODER;
        const uint64_t u_camera_producer = GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_camera_texture = GRALLOC1_CONSUMER_USAGE_GPU_TEXTURE | GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_camera_composer = GRALLOC1_CONSUMER_USAGE_HWCOMPOSER | GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_cpu_read = GRALLOC1_PRODUCER_USAGE_CPU_READ_OFTEN;
        const uint64_t u_cpu_write = GRALLOC1_PRODUCER_USAGE_CPU_WRITE_OFTEN;
        const uint64_t u_video_decoder = GRALLOC1_PRODUCER_USAGE_VIDEO_DECODER | GRALLOC1_CONSUMER_USAGE_HWCOMPOSER;

        if ((usage & u_camera_texture) == u_camera_texture)
            format = HAL_PIXEL_FORMAT_YV12;
        else if ((usage & u_camera_composer) == u_camera_composer)
            format = HAL_PIXEL_FORMAT_YV12;
        else if (((usage & u_camera_producer) == u_camera_producer) &&
            (((usage & u_cpu_read) == u_cpu_read) || ((usage & u_cpu_write) == u_cpu_write))){
            format = HAL_PIXEL_FORMAT_YCrCb_420_SP;
        }
        else if ((usage & u_camera_producer) == u_camera_producer)
            format = HAL_PIXEL_FORMAT_YV12;
        else if ((usage & u_video_decoder) == u_video_decoder)
            format = HAL_PIXEL_FORMAT_YV12;

        return format;
    }

    return in_format;
}

static int _plt_gralloc_extra_get_platform_format_mt6771(int in_format, uint64_t usage)
{
    if (in_format == HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED)
    {
        int format = HAL_PIXEL_FORMAT_RGBA_8888;

        const uint64_t u_video = GRALLOC1_CONSUMER_USAGE_VIDEO_ENCODER;
        const uint64_t u_camera_producer = GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_camera_texture = GRALLOC1_CONSUMER_USAGE_GPU_TEXTURE | GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_camera_composer = GRALLOC1_CONSUMER_USAGE_HWCOMPOSER | GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_cpu_read = GRALLOC1_PRODUCER_USAGE_CPU_READ_OFTEN;
        const uint64_t u_cpu_write = GRALLOC1_PRODUCER_USAGE_CPU_WRITE_OFTEN;
        const uint64_t u_protected = GRALLOC1_PRODUCER_USAGE_PROTECTED;

        if (_has_usage(u_protected | u_camera_producer, usage))
            format = HAL_PIXEL_FORMAT_NV12;
        else if ((usage & u_video) == u_video)
            format = HAL_PIXEL_FORMAT_YV12;
        else if ((usage & u_camera_texture) == u_camera_texture)
            format = HAL_PIXEL_FORMAT_YV12;
        else if ((usage & u_camera_composer) == u_camera_composer)
            format = HAL_PIXEL_FORMAT_YV12;
        else if (((usage & u_camera_producer) == u_camera_producer) &&
            (((usage & u_cpu_read) == u_cpu_read) || ((usage & u_cpu_write) == u_cpu_write))){
            format = HAL_PIXEL_FORMAT_YCrCb_420_SP;
        }
        else if ((usage & u_camera_producer) == u_camera_producer)
            format = HAL_PIXEL_FORMAT_YV12;

        return format;
    }

    if (in_format == HAL_PIXEL_FORMAT_YCbCr_420_888)
    {
        int format = HAL_PIXEL_FORMAT_YCbCr_420_888;

        const uint64_t u_video = GRALLOC1_CONSUMER_USAGE_VIDEO_ENCODER;
        const uint64_t u_camera_producer = GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_camera_texture = GRALLOC1_CONSUMER_USAGE_GPU_TEXTURE | GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_camera_composer = GRALLOC1_CONSUMER_USAGE_HWCOMPOSER | GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_cpu_read = GRALLOC1_PRODUCER_USAGE_CPU_READ_OFTEN;
        const uint64_t u_cpu_write = GRALLOC1_PRODUCER_USAGE_CPU_WRITE_OFTEN;

        if ((usage & u_camera_texture) == u_camera_texture)
            format = HAL_PIXEL_FORMAT_YV12;
        else if ((usage & u_camera_composer) == u_camera_composer)
            format = HAL_PIXEL_FORMAT_YV12;
        else if (((usage & u_camera_producer) == u_camera_producer) &&
            (((usage & u_cpu_read) == u_cpu_read) || ((usage & u_cpu_write) == u_cpu_write))){
            format = HAL_PIXEL_FORMAT_YCrCb_420_SP;
        }
        else if ((usage & u_camera_producer) == u_camera_producer)
            format = HAL_PIXEL_FORMAT_YV12;

        return format;
    }

    return in_format;
}

static int _plt_gralloc_extra_get_platform_format_mt6775(int in_format, uint64_t usage)
{
    if (in_format == HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED)
    {
        int format = HAL_PIXEL_FORMAT_YV12;

        const uint64_t u_video = GRALLOC1_CONSUMER_USAGE_VIDEO_ENCODER;
        const uint64_t u_camera_producer = GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_camera_texture = GRALLOC1_CONSUMER_USAGE_GPU_TEXTURE | GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_camera_composer = GRALLOC1_CONSUMER_USAGE_HWCOMPOSER | GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_cpu_read = GRALLOC1_PRODUCER_USAGE_CPU_READ_OFTEN;
        const uint64_t u_cpu_write = GRALLOC1_PRODUCER_USAGE_CPU_WRITE_OFTEN;

        if ((usage & u_video) == u_video)
            format = HAL_PIXEL_FORMAT_YV12;
        else if ((usage & u_camera_texture) == u_camera_texture)
            format = HAL_PIXEL_FORMAT_YV12;
        else if ((usage & u_camera_composer) == u_camera_composer)
            format = HAL_PIXEL_FORMAT_YV12;
        else if (((usage & u_camera_producer) == u_camera_producer) &&
            (((usage & u_cpu_read) == u_cpu_read) || ((usage & u_cpu_write) == u_cpu_write))){
            format = HAL_PIXEL_FORMAT_YCrCb_420_SP;
        }
        else if ((usage & u_camera_producer) == u_camera_producer)
            format = HAL_PIXEL_FORMAT_YV12;

        return format;
    }

    if (in_format == HAL_PIXEL_FORMAT_YCbCr_420_888)
    {
        int format = HAL_PIXEL_FORMAT_YCbCr_420_888;

        const uint64_t u_video = GRALLOC1_CONSUMER_USAGE_VIDEO_ENCODER;
        const uint64_t u_camera_producer = GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_camera_texture = GRALLOC1_CONSUMER_USAGE_GPU_TEXTURE | GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_camera_composer = GRALLOC1_CONSUMER_USAGE_HWCOMPOSER | GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_cpu_read = GRALLOC1_PRODUCER_USAGE_CPU_READ_OFTEN;
        const uint64_t u_cpu_write = GRALLOC1_PRODUCER_USAGE_CPU_WRITE_OFTEN;

        if ((usage & u_camera_texture) == u_camera_texture)
            format = HAL_PIXEL_FORMAT_YV12;
        else if ((usage & u_camera_composer) == u_camera_composer)
            format = HAL_PIXEL_FORMAT_YV12;
        else if (((usage & u_camera_producer) == u_camera_producer) &&
            (((usage & u_cpu_read) == u_cpu_read) || ((usage & u_cpu_write) == u_cpu_write))){
            format = HAL_PIXEL_FORMAT_YCrCb_420_SP;
        }
        else if ((usage & u_camera_producer) == u_camera_producer)
            format = HAL_PIXEL_FORMAT_YV12;

        return format;
    }

    return in_format;
}

static int _plt_gralloc_extra_get_platform_format_mt6779(int in_format, uint64_t usage)
{
    if (in_format == HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED)
    {
        int format = HAL_PIXEL_FORMAT_RGBA_8888;
        const uint64_t u_video = GRALLOC1_CONSUMER_USAGE_VIDEO_ENCODER;
        const uint64_t u_camera_producer = GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_camera_texture = GRALLOC1_CONSUMER_USAGE_GPU_TEXTURE | GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_camera_composer = GRALLOC1_CONSUMER_USAGE_HWCOMPOSER | GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_cpu_read = GRALLOC1_PRODUCER_USAGE_CPU_READ_OFTEN;
        const uint64_t u_cpu_write = GRALLOC1_PRODUCER_USAGE_CPU_WRITE_OFTEN;
        const uint64_t u_protected = GRALLOC1_PRODUCER_USAGE_PROTECTED;

        if (_has_usage(u_protected | u_camera_producer, usage))
            format = HAL_PIXEL_FORMAT_NV12;
        else if ((usage & u_video) == u_video)
            format = HAL_PIXEL_FORMAT_YV12;
        else if ((usage & u_camera_texture) == u_camera_texture)
            format = HAL_PIXEL_FORMAT_YV12;
        else if ((usage & u_camera_composer) == u_camera_composer)
            format = HAL_PIXEL_FORMAT_YV12;
        else if (((usage & u_camera_producer) == u_camera_producer) &&
            (((usage & u_cpu_read) == u_cpu_read) || ((usage & u_cpu_write) == u_cpu_write))){
            format = HAL_PIXEL_FORMAT_YCrCb_420_SP;
        }
        else if ((usage & u_camera_producer) == u_camera_producer)
            format = HAL_PIXEL_FORMAT_YV12;

        return format;
    }

    if (in_format == HAL_PIXEL_FORMAT_YCbCr_420_888)
    {
        int format = HAL_PIXEL_FORMAT_YCbCr_420_888;
        const uint64_t u_video = GRALLOC1_CONSUMER_USAGE_VIDEO_ENCODER;
        const uint64_t u_camera_producer = GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_camera_texture = GRALLOC1_CONSUMER_USAGE_GPU_TEXTURE | GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_camera_composer = GRALLOC1_CONSUMER_USAGE_HWCOMPOSER | GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_cpu_read = GRALLOC1_PRODUCER_USAGE_CPU_READ_OFTEN;
        const uint64_t u_cpu_write = GRALLOC1_PRODUCER_USAGE_CPU_WRITE_OFTEN;

        if ((usage & u_camera_texture) == u_camera_texture)
            format = HAL_PIXEL_FORMAT_YV12;
        else if ((usage & u_camera_composer) == u_camera_composer)
            format = HAL_PIXEL_FORMAT_YV12;
        else if (((usage & u_camera_producer) == u_camera_producer) &&
            (((usage & u_cpu_read) == u_cpu_read) || ((usage & u_cpu_write) == u_cpu_write))){
            format = HAL_PIXEL_FORMAT_YCrCb_420_SP;
        }
        else if ((usage & u_camera_producer) == u_camera_producer)
            format = HAL_PIXEL_FORMAT_YV12;

        return format;
    }

    return in_format;
}

static int _plt_gralloc_extra_get_platform_format_mt6785(int in_format, uint64_t usage)
{
    if (in_format == HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED)
    {
        int format = HAL_PIXEL_FORMAT_RGBA_8888;

        const uint64_t u_video = GRALLOC1_CONSUMER_USAGE_VIDEO_ENCODER;
        const uint64_t u_camera_producer = GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_camera_texture = GRALLOC1_CONSUMER_USAGE_GPU_TEXTURE | GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_camera_composer = GRALLOC1_CONSUMER_USAGE_HWCOMPOSER | GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_cpu_read = GRALLOC1_PRODUCER_USAGE_CPU_READ_OFTEN;
        const uint64_t u_cpu_write = GRALLOC1_PRODUCER_USAGE_CPU_WRITE_OFTEN;
        const uint64_t u_protected = GRALLOC1_PRODUCER_USAGE_PROTECTED;

        if (_has_usage(u_protected | u_camera_producer, usage))
            format = HAL_PIXEL_FORMAT_NV12;
        else if ((usage & u_video) == u_video)
            format = HAL_PIXEL_FORMAT_YV12;
        else if ((usage & u_camera_texture) == u_camera_texture)
            format = HAL_PIXEL_FORMAT_YCrCb_420_SP;
        else if ((usage & u_camera_composer) == u_camera_composer)
            format = HAL_PIXEL_FORMAT_YV12;
        else if (((usage & u_camera_producer) == u_camera_producer) &&
                (((usage & u_cpu_read) == u_cpu_read) || ((usage & u_cpu_write) == u_cpu_write))) {
            format = HAL_PIXEL_FORMAT_YCrCb_420_SP;
        } else if ((usage & u_camera_producer) == u_camera_producer)
            format = HAL_PIXEL_FORMAT_YV12;

        return format;
    }

    if (in_format == HAL_PIXEL_FORMAT_YCbCr_420_888)
    {
        int format = HAL_PIXEL_FORMAT_YCbCr_420_888;

        const uint64_t u_video = GRALLOC1_CONSUMER_USAGE_VIDEO_ENCODER;
        const uint64_t u_camera_producer = GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_camera_texture = GRALLOC1_CONSUMER_USAGE_GPU_TEXTURE | GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_camera_composer = GRALLOC1_CONSUMER_USAGE_HWCOMPOSER | GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_cpu_read = GRALLOC1_PRODUCER_USAGE_CPU_READ_OFTEN;
        const uint64_t u_cpu_write = GRALLOC1_PRODUCER_USAGE_CPU_WRITE_OFTEN;
        const uint64_t u_video_decoder = GRALLOC1_PRODUCER_USAGE_VIDEO_DECODER | GRALLOC1_CONSUMER_USAGE_HWCOMPOSER;

        if ((usage & u_video) == u_video)
            format = HAL_PIXEL_FORMAT_YV12;
        else if ((usage & u_camera_texture) == u_camera_texture)
            format = HAL_PIXEL_FORMAT_YCrCb_420_SP;
        else if ((usage & u_camera_composer) == u_camera_composer)
            format = HAL_PIXEL_FORMAT_YV12;
        else if (((usage & u_camera_producer) == u_camera_producer) &&
                (((usage & u_cpu_read) == u_cpu_read) || ((usage & u_cpu_write) == u_cpu_write))) {
            format = HAL_PIXEL_FORMAT_YCrCb_420_SP;
        } else if ((usage & u_camera_producer) == u_camera_producer)
            format = HAL_PIXEL_FORMAT_YV12;
        else if ((usage & u_video_decoder) == u_video_decoder)
            format = HAL_PIXEL_FORMAT_YV12;

        return format;
    }

    return in_format;
}

static int _plt_gralloc_extra_get_platform_format_mt8167(int in_format, uint64_t usage)
{
    if (in_format == HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED)
    {
        int format = HAL_PIXEL_FORMAT_RGBA_8888;

        const uint64_t u_video = GRALLOC1_CONSUMER_USAGE_VIDEO_ENCODER;
        const uint64_t u_camera_producer = GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_camera_texture = GRALLOC1_CONSUMER_USAGE_GPU_TEXTURE | GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_camera_composer = GRALLOC1_CONSUMER_USAGE_HWCOMPOSER | GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_cpu_read = GRALLOC1_PRODUCER_USAGE_CPU_READ_OFTEN;
        const uint64_t u_cpu_write = GRALLOC1_PRODUCER_USAGE_CPU_WRITE_OFTEN;

        if ((usage & u_video) == u_video)
            format = HAL_PIXEL_FORMAT_YV12;
        else if ((usage & u_camera_texture) == u_camera_texture)
            format = HAL_PIXEL_FORMAT_YV12;
        else if ((usage & u_camera_composer) == u_camera_composer)
            format = HAL_PIXEL_FORMAT_YV12;
        else if (((usage & u_camera_producer) == u_camera_producer) &&
            (((usage & u_cpu_read) == u_cpu_read) || ((usage & u_cpu_write) == u_cpu_write))) {
            format = HAL_PIXEL_FORMAT_YV12;
        }
        else if ((usage & u_camera_producer) == u_camera_producer)
            format = HAL_PIXEL_FORMAT_YCbCr_422_I;

        return format;
    }

    if (in_format == HAL_PIXEL_FORMAT_YCbCr_420_888)
    {
        int format = HAL_PIXEL_FORMAT_YCbCr_420_888;

        const uint64_t u_video = GRALLOC1_CONSUMER_USAGE_VIDEO_ENCODER;
        const uint64_t u_camera_producer = GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_camera_texture = GRALLOC1_CONSUMER_USAGE_GPU_TEXTURE | GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_camera_composer = GRALLOC1_CONSUMER_USAGE_HWCOMPOSER | GRALLOC1_PRODUCER_USAGE_CAMERA;
        const uint64_t u_cpu_read = GRALLOC1_PRODUCER_USAGE_CPU_READ_OFTEN;
        const uint64_t u_cpu_write = GRALLOC1_PRODUCER_USAGE_CPU_WRITE_OFTEN;

        if ((usage & u_camera_texture) == u_camera_texture)
            format = HAL_PIXEL_FORMAT_YV12;
        else if ((usage & u_camera_composer) == u_camera_composer)
            format = HAL_PIXEL_FORMAT_YV12;
        else if (((usage & u_camera_producer) == u_camera_producer) &&
            (((usage & u_cpu_read) == u_cpu_read) || ((usage & u_cpu_write) == u_cpu_write))) {
            format = HAL_PIXEL_FORMAT_YV12;
        }
        else if ((usage & u_camera_producer) == u_camera_producer)
            format = HAL_PIXEL_FORMAT_YV12;

        return format;
    }

    return in_format;
}

static int _plt_gralloc_extra_get_platform_format_mt6757(int in_format, uint64_t usage)
{
	if (in_format == HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED)
	{
		int format = HAL_PIXEL_FORMAT_RGBA_8888;

		const uint64_t u_video = GRALLOC1_CONSUMER_USAGE_VIDEO_ENCODER;
		const uint64_t u_camera = GRALLOC1_PRODUCER_USAGE_CAMERA;
		const uint64_t u_camera_texture = GRALLOC1_CONSUMER_USAGE_GPU_TEXTURE | GRALLOC1_PRODUCER_USAGE_CAMERA;
		const uint64_t u_camera_composer = GRALLOC1_CONSUMER_USAGE_HWCOMPOSER | GRALLOC1_PRODUCER_USAGE_CAMERA;

		if ((usage & u_video) == u_video)
			format = HAL_PIXEL_FORMAT_YV12;
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
		int format = HAL_PIXEL_FORMAT_YV12;

		const uint64_t u_video = GRALLOC1_CONSUMER_USAGE_VIDEO_ENCODER;
		const uint64_t u_camera = GRALLOC1_PRODUCER_USAGE_CAMERA;
		const uint64_t u_camera_texture = GRALLOC1_CONSUMER_USAGE_GPU_TEXTURE | GRALLOC1_PRODUCER_USAGE_CAMERA;
		const uint64_t u_camera_composer = GRALLOC1_CONSUMER_USAGE_HWCOMPOSER | GRALLOC1_PRODUCER_USAGE_CAMERA;

		if ((usage & u_camera_texture) == u_camera_texture)
			format = HAL_PIXEL_FORMAT_YV12;
		else if ((usage & u_camera_composer) == u_camera_composer)
			format = HAL_PIXEL_FORMAT_YV12;
		else if ((usage & u_camera) == u_camera)
			format = HAL_PIXEL_FORMAT_YV12;

		return format;
	}

	return in_format;
}


void ge_platform_wrap_init(ge_platform_fn *table)
{
    char propbuf[PROP_VALUE_MAX];
    char propbuf2[PROP_VALUE_MAX];
    property_get("ro.board.platform", propbuf, "");
    property_get("ro.boot.hardware", propbuf2, "");
    if (strstr(propbuf, "mt6735") != NULL || strstr(propbuf2, "mt6735") != NULL) {
        table->gralloc_extra_get_platform_format = &_plt_gralloc_extra_get_platform_format_mt6735;
    } else if (strstr(propbuf, "mt6739") != NULL || strstr(propbuf2, "mt6739") != NULL) {
        table->gralloc_extra_get_platform_format = &_plt_gralloc_extra_get_platform_format_mt6739;
    } else if (strstr(propbuf, "mt6755") != NULL || strstr(propbuf2, "mt6755") != NULL) {
        table->gralloc_extra_get_platform_format = &_plt_gralloc_extra_get_platform_format_mt6755;
    } else if (strstr(propbuf, "mt6761") != NULL || strstr(propbuf2, "mt6761") != NULL) {
        table->gralloc_extra_get_platform_format = &_plt_gralloc_extra_get_platform_format_mt6761;
    } else if (strstr(propbuf, "mt6763") != NULL || strstr(propbuf2, "mt6763") != NULL) {
        table->gralloc_extra_get_platform_format = &_plt_gralloc_extra_get_platform_format_mt6763;
    } else if (strstr(propbuf, "mt6765") != NULL || strstr(propbuf2, "mt6765") != NULL) {
        table->gralloc_extra_get_platform_format = &_plt_gralloc_extra_get_platform_format_mt6765;
    } else if (strstr(propbuf, "mt6768") != NULL || strstr(propbuf2, "mt6768") != NULL) {
        table->gralloc_extra_get_platform_format = &_plt_gralloc_extra_get_platform_format_mt6768;
    } else if (strstr(propbuf, "mt6771") != NULL || strstr(propbuf2, "mt6771") != NULL) {
        table->gralloc_extra_get_platform_format = &_plt_gralloc_extra_get_platform_format_mt6771;
    } else if (strstr(propbuf, "mt6775") != NULL || strstr(propbuf2, "mt6775") != NULL) {
        table->gralloc_extra_get_platform_format = &_plt_gralloc_extra_get_platform_format_mt6775;
    } else if (strstr(propbuf, "mt6779") != NULL || strstr(propbuf2, "mt6779") != NULL) {
        table->gralloc_extra_get_platform_format = &_plt_gralloc_extra_get_platform_format_mt6779;
    } else if (strstr(propbuf, "mt6785") != NULL || strstr(propbuf2, "mt6785") != NULL) {
        table->gralloc_extra_get_platform_format = &_plt_gralloc_extra_get_platform_format_mt6785;
    } else if (strstr(propbuf, "mt8167") != NULL || strstr(propbuf2, "mt8167") != NULL) {
        table->gralloc_extra_get_platform_format = &_plt_gralloc_extra_get_platform_format_mt8167;
    } else if (strstr(propbuf, "mt6757") != NULL || strstr(propbuf2, "mt6757") != NULL) {
        table->gralloc_extra_get_platform_format = &_plt_gralloc_extra_get_platform_format_mt6757;
    } else {
        ALOGE("gralloc_extra find no platform information");
    }
}

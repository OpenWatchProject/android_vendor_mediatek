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

#define LOG_TAG "GraphicBufferExtra_hal"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include <stdint.h>
#include <inttypes.h>
#include <errno.h>

#include <utils/Errors.h>
#include <utils/Log.h>
#include <utils/Trace.h>

#include <system/graphics.h>
#include <hardware/gralloc1.h>

#include <ui/gralloc_extra.h>
#include <gralloc1_mtk_defs.h>
#include <graphics_mtk_defs.h>

#include <ged/ged_ge.h>

#include "platform/ge_platform.h"

#ifndef AWAR
#define AWAR(fmt, args...) \
	__android_log_print(ANDROID_LOG_WARN, "[GrallocExtra-Warning]", "%s:%d " fmt, __func__, __LINE__, ##args)
#endif
#ifndef AINF
#define AINF(fmt, args...) __android_log_print(ANDROID_LOG_INFO, "[GrallocExtra]", fmt, ##args)
#endif
#ifndef AERR
#define AERR(fmt, args...) \
	__android_log_print(ANDROID_LOG_ERROR, "[GrallocExtra-ERROR]", "%s:%d " fmt, __func__, __LINE__, ##args)
#endif
#ifndef AERR_IF
#define AERR_IF(eq, fmt, args...) \
	if ((eq))                     \
	AERR(fmt, args)
#endif

extern "C" int ge_query(buffer_handle_t hnd, GEFD ge_fd, GRALLOC_EXTRA_ATTRIBUTE_QUERY attr, void *out);
extern "C" int ge_perform(buffer_handle_t hnd, GEFD ge_fd, GRALLOC_EXTRA_ATTRIBUTE_PERFORM attr, void *in);

extern "C" int gralloc_extra_query(
        buffer_handle_t handle, GRALLOC_EXTRA_ATTRIBUTE_QUERY attribute, void * out_pointer)
{
    GRALLOC_EXTRA_RESULT ret = GRALLOC_EXTRA_UNKNOWN;
    ge_handle_t *ge_hnd;

    if (!handle)
    {
        AERR("gralloc_extra_query with NULL handle: %p", handle);
        return -GRALLOC_EXTRA_INVALIDE_PARAMS;
    }

    ge_hnd = get_ge_handle(handle);

    if (!ge_hnd)
    {
        AERR("gralloc_extra_query with NULL ge_hnd");
        return GRALLOC_EXTRA_ERROR;
    }

    switch (attribute)
    {
        case GRALLOC_EXTRA_GET_ION_FD:
            *((int *)out_pointer) = ge_hnd->share_fd;
            ret = GRALLOC_EXTRA_OK;
            break;

        case GRALLOC_EXTRA_GET_WIDTH:
            *((int *)out_pointer) = ge_hnd->width;
            ret = GRALLOC_EXTRA_OK;
            break;

        case GRALLOC_EXTRA_GET_HEIGHT:
            *((int *)out_pointer) = ge_hnd->height;
            ret = GRALLOC_EXTRA_OK;
            break;

        case GRALLOC_EXTRA_GET_FORMAT:
            *((int *)out_pointer) = ge_hnd->format;
            ret = GRALLOC_EXTRA_OK;
            break;

        case GRALLOC_EXTRA_GET_STRIDE:
            *((int *)out_pointer) = ge_hnd->stride;
            ret = GRALLOC_EXTRA_OK;
            break;

        case GRALLOC_EXTRA_GET_VERTICAL_STRIDE:
            *((int *)out_pointer) = ge_hnd->vertical_stride;
            ret = GRALLOC_EXTRA_OK;
            break;

        case GRALLOC_EXTRA_GET_VERTICAL_2ND_STRIDE:
            *((int *)out_pointer) = ge_hnd->vertical_2nd_stride;
            ret = GRALLOC_EXTRA_OK;
            break;

        case GRALLOC_EXTRA_GET_BYTE_2ND_STRIDE:
            *((int *)out_pointer) = ge_hnd->byte_2nd_stride;
            ret = GRALLOC_EXTRA_OK;
            break;

        case GRALLOC_EXTRA_GET_ALLOC_SIZE:
            *((int *)out_pointer) = ge_hnd->alloc_size;
            ret = GRALLOC_EXTRA_OK;
            break;

        case GRALLOC_EXTRA_GET_SECURE_HANDLE:
            if ((ge_hnd->consumer_usage | ge_hnd->producer_usage) & GRALLOC1_PRODUCER_USAGE_PROTECTED)
            {
                *((uint32_t *)out_pointer) = ge_hnd->sec_handle;
                ret = GRALLOC_EXTRA_OK;
            }
            else
            {
                AINF("hnd=%p (consumer_usage %" PRIu64 " | producer_usage %" PRIu64 ") had no GRALLOC1_PRODUCER_USAGE_PROTECTED", \
                     ge_hnd, ge_hnd->consumer_usage, ge_hnd->producer_usage);
                ret = -GRALLOC_EXTRA_INVALIDE_PARAMS;
            }
            break;

        case GRALLOC_EXTRA_GET_USAGE:
            *((int *)out_pointer) = ge_hnd->consumer_usage | ge_hnd->producer_usage;
            ret = GRALLOC_EXTRA_OK;
            break;

        case GRALLOC_EXTRA_GET_CONSUMER_USAGE:
            *((uint64_t *)out_pointer) = ge_hnd->consumer_usage;
            ret = GRALLOC_EXTRA_OK;
            break;

        case GRALLOC_EXTRA_GET_PRODUCER_USAGE:
            *((uint64_t *)out_pointer) = ge_hnd->producer_usage;
            ret = GRALLOC_EXTRA_OK;
            break;

        case GRALLOC_EXTRA_GET_FB_MVA:
            ret = GRALLOC_EXTRA_NOT_SUPPORTED;
            break;

        default:
            if (ge_query(handle, ge_hnd->ge_fd, attribute, out_pointer))
            {
                AERR("unknown query enum: %d with ge_fd %d", attribute, ge_hnd->ge_fd);
                ret = -GRALLOC_EXTRA_INVALIDE_PARAMS;
            }
            else
            {
                ret = GRALLOC_EXTRA_OK;
            }
    }
    return ret;
}

extern "C" int gralloc_extra_perform(
        buffer_handle_t handle, GRALLOC_EXTRA_ATTRIBUTE_PERFORM attribute, void * in_pointer)
{
    GRALLOC_EXTRA_RESULT ret = GRALLOC_EXTRA_UNKNOWN;
    ge_handle_t *ge_hnd;

    if (!handle)
    {
        AERR("gralloc_extra_perform with NULL handle: %p", handle);
        return -GRALLOC_EXTRA_INVALIDE_PARAMS;
    }

    ge_hnd = get_ge_handle(handle);

    if (!ge_hnd)
    {
        AERR("bad handle! handle->version: %d != sizeof(native_handle_t): %d", \
            (int)handle->version, (int)sizeof(native_handle_t));
        AERR("gralloc_extra_perform with bad handle: %p and can't be transfer to ge_hnd", handle);
        return -GRALLOC_EXTRA_INVALIDE_PARAMS;
    }

    if (ge_perform(handle, ge_hnd->ge_fd, attribute, in_pointer))
    {
        AERR("unknown perform enum: %d with ge_fd %d", attribute, ge_hnd->ge_fd);
        ret = -GRALLOC_EXTRA_INVALIDE_PARAMS;
    }
    else
    {
        ret = GRALLOC_EXTRA_OK;
    }
    return ret;
}

static int __gralloc_extra_sf_set_int(int *data, int32_t mask, int32_t value)
{
    int32_t old_status;
    int32_t new_status;

    old_status = *data;
    new_status = ( old_status & (~mask) );
    new_status |= ( value & mask );

    /* Force modify the const member */
    *data = new_status;

    return GRALLOC_EXTRA_OK;
}

extern "C" int gralloc_extra_sf_set_status(gralloc_extra_ion_sf_info_t *sf_info, int32_t mask, int32_t value)
{
    return __gralloc_extra_sf_set_int(&sf_info->status, mask, value);
}

extern "C" int gralloc_extra_sf_set_status2(gralloc_extra_ion_sf_info_t *sf_info, int32_t mask, int32_t value)
{
    return __gralloc_extra_sf_set_int(&sf_info->status2, mask, value);
}

extern "C" int gralloc_extra_get_platform_format(int in_format, uint64_t usage)
{
    return ge_platform.gralloc_extra_get_platform_format(in_format, usage);
}

extern "C" int gralloc_extra_need_resident(uint64_t usage)
{
	return (usage & (0
			| GRALLOC1_USAGE_HW_CAMERA_ZSL
			)) == 0;
}

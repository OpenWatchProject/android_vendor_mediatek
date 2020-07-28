/*
 * Copyright (C) 2018-2019 MediaTek Inc.
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

#ifndef GPUD_INCLUDE_GPUD_GPUD_H_
#define GPUD_INCLUDE_GPUD_GPUD_H_

#include <sys/cdefs.h>
#include <sys/types.h>
#include <system/window.h>
#include <nativebase/nativebase.h>
#include <gpud/gpud_context.h>
#include <gpud/gpud_gl.h>
#include <gpud/gpud_vk.h>

__BEGIN_DECLS

/**
 * Enum describing the different counters used when dumping.
 */
typedef enum {
    GPUD_DUMP_COUNTER_AUXIMAGE_SRC,
    GPUD_DUMP_COUNTER_AUXIMAGE_DST,
    GPUD_DUMP_COUNTER_TEXIMAGE,
    GPUD_DUMP_COUNTER_EXTIMAGE,
    GPUD_DUMP_COUNTER_FRAMEBUFFER,
    GPUD_DUMP_COUNTER_SHADERSOURCE,
    GPUD_DUMP_COUNTER_WINDOWSYSTEM_FRAMEBUFFER,
    GPUD_DUMP_COUNTER_FRAMEBUFFERSURFACE,
    GPUD_DUMP_COUNTER_NUM,
} gpud_dump_counter;

typedef enum {
    MEDIA_UNKNOWN,
    MEDIA_VIDEO,
    MEDIA_PICTURE,
    MEDIA_ISP_PREVIEW,
    MEDIA_VIDEO_CODEC,
    MEDIA_ISP_CAPTURE
} DP_MEDIA_TYPE_ENUM;

typedef enum {
	PATH_NO_AUX = 0,
	PATH_AUX_VIDEO,
	PATH_AUX_UIPQ,
	PATH_AUX_INVALID,
} AUX_PATH;

typedef enum {
    AUX_CONVERSION_GET_BUFFER_INFO_FAIL = -7,
    AUX_CONVERSION_INVALIDE_DIMENSION,
    AUX_CONVERSION_SRC_FORMAT_NOT_SUPPORT,
    AUX_CONVERSION_CREATE_JOB_FAIL,
    AUX_CONVERSION_LOCK_SRC_BUFFER_FAIL,
    AUX_CONVERSION_DST_FORMAT_NOT_SUPPORT,
    AUX_CONVERSION_DST_NOT_ION_BUFFER,
    AUX_CONVERSION_SUCCESS = 0,
    AUX_CONVERSION_INVALIDATE_FAIL,
    AUX_CONVERSION_SKIP_PROTECT_BUFFER,
    AUX_CONVERSION_SKIP_NOT_DIRTY,
    AUX_CONVERSION_PATH_NO_AUX,
} AUX_CONVERSION_STATUS;

typedef enum {
    DP_PROFILE_BT601, //Limited range
    DP_PROFILE_BT709,
    DP_PROFILE_JPEG,
    DP_PROFILE_FULL_BT601 = DP_PROFILE_JPEG,
    DP_PROFILE_BT2020,     // not support for output
    DP_PROFILE_FULL_BT709, // not support for output
    DP_PROFILE_FULL_BT2020 // not support for output
} DP_PROFILE_ENUM;

void gpudGetProcessInfo(int *pid, char *pname, int *tid, int size);
void gpudGetDumpProcessName(char *pname, char *dump_pname);
const char *gpudGetInitTypeName(gpud_init_type type);
char *gpudReplaceSubString(const char *str, const char *old_substr, const char *new_substr);
void gpudValidateFileformat(char *str);
uint64_t gpudGetHashCode(char *str);
void gpudGlDumpTexImage(TexImageInfo image_info, GLvoid const *data);
void gpudGlDumpFramebuffer(FramebufferInfo framebuffer_info, GLvoid const *data);
void gpudAuxDumpAUXImage(AUXImageInfo image_info, const ANativeWindowBuffer *buffer);
void gpudAuxShowPQInfo(AUXDpPqParamInfo gpud_dppq_param);
void gpudAuxShowImageInfo(AUXImageInfo image_info, const ANativeWindowBuffer *buffer);
void gpudDumpANativeWindowBuffer(ANWBufferInfo buffer_info, const ANativeWindowBuffer *buffer);
void gpudGlDumpShaderSource(ShaderSourceInfo shadersource_info);
char *gpudAuxGetConversionStatusStr(int ConversionStatus);
char *gpudAuxGetAuxPathStr(int path);

__END_DECLS

#endif  // GPUD_INCLUDE_GPUD_GPUD_H_

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

#ifndef GPUD_INCLUDE_GPUD_GPUD_CONTEXT_H_
#define GPUD_INCLUDE_GPUD_GPUD_CONTEXT_H_

#include <sys/cdefs.h>
#include <sys/types.h>
#include <dpframework/mmdump.h>
#include <dpframework/mmdump_fmt.h>

__BEGIN_DECLS

typedef enum {
    GPUD_INIT_TYPE_UNKNOWN,
    GPUD_INIT_TYPE_GL,
    GPUD_INIT_TYPE_VK,
    GPUD_INIT_TYPE_CL,
    GPUD_INIT_TYPE_GPUD,
    GPUD_INIT_TYPE_NUM,
} gpud_init_type;

/**
 * @brief Identifies what kind of dumping is requested.
 */
typedef enum {
    GPUD_DUMP_SPECIFIER_TEXIMAGE_READ,
    GPUD_DUMP_SPECIFIER_TEXIMAGE_DRAW,
    GPUD_DUMP_SPECIFIER_AUXIMAGE_READ,
    GPUD_DUMP_SPECIFIER_AUXIMAGE_DRAW,
    GPUD_DUMP_SPECIFIER_GL_EXTIMAGE_READ,
    GPUD_DUMP_SPECIFIER_GL_EXTIMAGE_DRAW,
    GPUD_DUMP_SPECIFIER_VK_EXTIMAGE_READ,
    GPUD_DUMP_SPECIFIER_VK_EXTIMAGE_DRAW,
    GPUD_DUMP_SPECIFIER_GL_ANWBUFFER_READ,
    GPUD_DUMP_SPECIFIER_GL_ANWBUFFER_DRAW,
    GPUD_DUMP_SPECIFIER_VK_ANWBUFFER_READ,
    GPUD_DUMP_SPECIFIER_VK_ANWBUFFER_DRAW,
    GPUD_DUMP_SPECIFIER_SF_ANWBUFFER_READ,
    GPUD_DUMP_SPECIFIER_SF_ANWBUFFER_DRAW,
    GPUD_DUMP_SPECIFIER_GL_SHADER_SOURCE,
    GPUD_DUMP_SPECIFIER_VK_SHADER_SOURCE,
    GPUD_DUMP_SPECIFIER_NUM,
} gpud_dump_specifier;

typedef enum {
    GPUD_AUXIMAGE_REQUIREMENT_UNKNOWN,
    GPUD_AUXIMAGE_REQUIREMENT_VIDEO,
    GPUD_AUXIMAGE_REQUIREMENT_UIPQ,
    GPUD_AUXIMAGE_REQUIREMENT_NUM,
} gpud_auximage_requirement;

typedef enum {
    GPUD_AUXIMAGE_COLORSPACE_BT601,
    GPUD_AUXIMAGE_COLORSPACE_BT709,
    GPUD_AUXIMAGE_COLORSPACE_JPEG,
    GPUD_AUXIMAGE_COLORSPACE_FULL_BT601 = GPUD_AUXIMAGE_COLORSPACE_JPEG,
    GPUD_AUXIMAGE_COLORSPACE_BT2020,
    GPUD_AUXIMAGE_COLORSPACE_FULL_BT709,
    GPUD_AUXIMAGE_COLORSPACE_FULL_BT2020,
} gpud_auximage_colorspace;

typedef struct AUXDpPqParamInfo {
    int scenario;
    unsigned int enable;
    unsigned int video_paramTable;
    unsigned int video_id;
    unsigned int video_timeStamp;
    bool video_isHDR2SDR;
    unsigned int scltmEnable;
    unsigned int scltmPosition;
    unsigned int scltmOrientation;
    unsigned int scltmTableIndex;
} AUXDpPqParamInfo;

typedef struct AUXImageInfo {
    int specifier;
    int ion_fd;
    int format;
    int DPFormat;
    int width;
    int height;
    bool is_omx_align;
    int uv_align;
    int width_align;
    int height_align;
    int stride;
    int y_stride;
    int uv_stride;
    unsigned int size;
    unsigned int yuv_info;
    unsigned int yuv_info_orig;
} AUXImageInfo;

typedef struct TexImageInfo {
    bool compressed;
    unsigned int target;
    int level;
    int internalformat;
    int xoffset;
    int yoffset;
    int zoffset;
    int width;
    int height;
    int depth;
    unsigned int format;
    unsigned int type;
    size_t size;
} TexImageInfo;

typedef struct ANWBufferInfo {
    int specifier;
    unsigned int format;
    int width;
    int height;
    int stride;
    size_t size;
} ANWBufferInfo;

typedef struct FramebufferInfo {
    int read_binding;
    int draw_binding;
    unsigned int target;
    unsigned int format;
    unsigned int type;
    int x;
    int y;
    int width;
    int height;
    int stride;
    size_t size;
} FramebufferInfo;

typedef struct ShaderSourceInfo {
    unsigned int id;
    uint64_t hashcode;
    char *string;
    size_t size;
} ShaderSourceInfo;

typedef struct BufferExtraInfo {
    uint64_t id;
    int share_fd;
    int width;
    int height;
    int format;
    int stride;
    int vertical_stride;
    int alloc_size;
    bool is_compressed;
    int32_t sf_status;
    int32_t sf_status2;
    uint32_t timestamp;
    unsigned int yuv_info;
} BufferExtraInfo;

typedef struct gpud_gl_context {
    char vendor[64];
    char renderer[64];
    int viewport_x;  // valid range : > 0
    int viewport_y;  // valid range : > 0
    int viewport_width;  // valid range : > 0
    int viewport_height;  // valid range : > 0
    int scissor_x;  // valid range : > 0
    int scissor_y;  // valid range : > 0
    int scissor_width;  // valid range : > 0
    int scissor_height;  // valid range : > 0
    char polygonoffset_factor[64];
    char polygonoffset_units[64];
    char clear[64];
    float clear_color_r;  // valid range : 0.0 ~ 1.0
    float clear_color_g;  // valid range : 0.0 ~ 1.0
    float clear_color_b;  // valid range : 0.0 ~ 1.0
    float clear_color_a;  // valid range : 0.0 ~ 1.0
    float clear_depthf;  // valid range : 0.0 ~ 1.0
    char clear_stencil[64];
    bool state_error_dump;
    bool readpixels_dump;
    bool shadersource_hack;
    bool shadersource_dump;
    char shadersource_hack_oldsubstr[256];
    char shadersource_hack_newsubstr[256];
    char shadersource_hack_oldsubstr_2[256];
    char shadersource_hack_newsubstr_2[256];
    uint64_t shadersource_hack_hashcode;
} gpud_gl_context;

typedef struct gpud_vk_context {
    int viewport_x;
    int viewport_y;
    int viewport_width;
    int viewport_height;
    char instance_returncode[64];
    char physicaldevice_vendor_id[64];
    char physicaldevice_devicename[64];
} gpud_vk_context;

typedef struct gpud_fwrite_context {
    bool is_enabled;
    bool rgba1010102;
} gpud_fwrite_context;

typedef struct gpud_mmdump_context {
    bool is_debugging;
    void *handle;
    mmdump_func fnDump;
    mmdump2_func fnDump2;
} gpud_mmdump_context;

typedef struct gpud_mmpath_context {
    bool is_debugging;
} gpud_mmpath_context;

typedef struct gpud_context {
    bool is_debugging;
    int is_initialized;
    bool is_logging;
    bool is_AUX_logging;
    int pid;
    int tid;
    char pname[256];
    char dump_pname[256];
    char folder[64];
    char auximage_colorspace[64];
    int auximage_dstformat;  // valid range : > 0
    int auximage_supportformat;  // valid range : > 0
    bool auximage_dump;
    bool extimage_dump;
    bool teximage_dump;
    bool wsframebuffer_dump;
    bool wsframebuffer_log;
    gpud_gl_context gl;
    gpud_vk_context vk;
    gpud_fwrite_context fwrite;
    gpud_mmdump_context mmdump;
    gpud_mmpath_context mmpath;
} gpud_context;

extern gpud_context g_gpud_context;

__END_DECLS

#endif  // GPUD_INCLUDE_GPUD_GPUD_CONTEXT_H_

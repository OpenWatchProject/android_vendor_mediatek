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

#ifndef GPUD_INCLUDE_GPUD_GPUD_API_H_
#define GPUD_INCLUDE_GPUD_GPUD_API_H_

#include <sys/cdefs.h>
#include <sys/types.h>
#include <system/window.h>
#include <nativebase/nativebase.h>
#include <gpud/gpud_gl.h>
#include <gpud/gpud_vk.h>
#include <gpud/gpud_context.h>

__BEGIN_DECLS

#define GPUD_INIT_ENTRY(...)                                       \
    do {                                                           \
        if (!g_gpud_context.is_initialized) {                      \
            (gpudInitialize)(__VA_ARGS__);                         \
        } else if (g_gpud_context.is_initialized != __VA_ARGS__) { \
            (gpudInitialize)(__VA_ARGS__);                         \
        }                                                          \
    } while (0)

#define GPUD_API_ENTRY(name, ...)                  \
    do {                                           \
        if (!g_gpud_context.is_initialized) {      \
            (gpudInitialize)(GPUD_INIT_TYPE_GPUD); \
        }                                          \
        if (g_gpud_context.is_debugging) {         \
            (gpud##name)(__VA_ARGS__);             \
        }                                          \
    } while (0)

// --------------------------------------------------------------------------------------------------------------------

void gpudInitialize(gpud_init_type requirement);
bool gpudIsEnabled(gpud_init_type type);

// --------------------------------------------------------------------------------------------------------------------

void gpudAuxHackAUXImageYUVColorSpace(unsigned int *yuv_info);
void gpudAuxHackAUXImageDstFormat(int *format);
void gpudAuxHackAUXImageSupportFormat(int *format);
void gpudAuxShowConversionStatus(int ConversionStatus);
void gpudAuxShowAUXPath(char *condition, int path);
void gpudGlHackAUXImageRequirement(int *requirement);  // legcy
void gpudGlHackUseProgram(GLuint *program, GLsizei *count, GLuint *shaders);
void gpudGlHackViewport(GLint *x, GLint *y, GLsizei *width, GLsizei *height);
void gpudGlHackGetString(GLenum name, const GLubyte **string);
void gpudGlHackScissor(GLint *x, GLint *y, GLsizei *width, GLsizei *height);
void gpudGlHackPolygonOffset(GLfloat *factor, GLfloat *units);
void gpudGlHackClear(GLbitfield *mask);
void gpudGlHackClearColor(GLclampf *red, GLclampf *green, GLclampf *blue, GLclampf *alpha);
void gpudGlHackClearDepthf(GLclampf *depth);
void gpudGlHackClearStencil(GLint *s);
void gpudVkHackGetPhysicalDeviceProperties(const char* entry, GpudVkPhysicalDeviceProperties *properties);
void gpudVkHackGetPhysicalDeviceProperties2(const char* entry, GpudVkPhysicalDeviceProperties *properties);
void gpudVkHackGetPhysicalDeviceQueueFamilyProperties(const char* entry, uint32_t *property_count,
                                                      GpudVkQueueFamilyProperties *properties);
void gpudVkHackGetPhysicalDeviceQueueFamilyProperties2(const char* entry, uint32_t *property_count,
                                                       GpudVkQueueFamilyProperties *properties);
void gpudVkHackCreateDevice(GpudVkPhysicalDevice physical_device,
                            GpudVkDeviceCreateInfo *create_info, GpudVkDevice device);
void gpudVkHackCreateInstance(GpudVkInstanceCreateInfo *create_info, GpudVkInstance instance, GpudVkResult *res);

// --------------------------------------------------------------------------------------------------------------------

void gpudAuxDumpAUXImageSrc(AUXImageInfo src_image_info, const ANativeWindowBuffer *src_buffer, AUXDpPqParamInfo gpud_dppq_param);
void gpudAuxDumpAUXImageDst(AUXImageInfo dst_image_info, const ANativeWindowBuffer *dst_buffer);
void gpudGlDumpTexImage2D(GLenum target, GLint level, GLint internalformat,
                          GLsizei width, GLsizei height, GLint border,
                          GLenum format, GLenum type, GLvoid const *data);
void gpudGlDumpTexSubImage2D(GLenum target, GLint level, GLint xoffset,
                             GLint yoffset, GLsizei width, GLsizei height,
                             GLenum format, GLenum type, GLvoid const *data);
void gpudGlDumpCompressedTexImage2D(GLenum target, GLint level,
                                    GLenum internalformat, GLsizei width, GLsizei height,
                                    GLint border, GLsizei imageSize, GLvoid const *data);
void gpudGlDumpCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset,
                                       GLint yoffset, GLsizei width, GLsizei height,
                                       GLenum format, GLsizei imageSize, GLvoid const *data);
void gpudGlDumpTexImage3D(GLenum target, GLint level, GLint internalformat,
                          GLsizei width, GLsizei height, GLsizei depth,
                          GLint border, GLenum format, GLenum type, GLvoid const *data);
void gpudGlDumpTexSubImage3D(GLenum target, GLint level,
                             GLint xoffset, GLint yoffset, GLint zoffset,
                             GLsizei width, GLsizei height, GLsizei depth,
                             GLenum format, GLenum type, GLvoid const *data);
void gpudGlDumpCompressedTexImage3D(GLenum target, GLint level, GLenum internalformat,
                                    GLsizei width, GLsizei height, GLsizei depth,
                                    GLint border, GLsizei imageSize, GLvoid const *data);
void gpudGlDumpCompressedTexSubImage3D(GLenum target, GLint level,
                                       GLint xoffset, GLint yoffset, GLint zoffset,
                                       GLsizei width, GLsizei height, GLsizei depth,
                                       GLenum format, GLsizei imageSize, GLvoid const *data);
void gpudGlDumpCopyTexImage2D(GLenum target, GLint level,
                              GLenum internalformat, GLint x, GLint y,
                              GLsizei width, GLsizei height, GLint border);
void gpudGlDumpCopyTexSubImage2D(GLenum target, GLint level,
                                 GLint xoffset, GLint yoffset,
                                 GLint x, GLint y, GLsizei width, GLsizei height);
void gpudGlDumpCopyTexSubImage3D(GLenum target, GLint level,
                                 GLint xoffset, GLint yoffset, GLint zoffset,
                                 GLint x, GLint y, GLsizei width, GLsizei height);
void gpudGlDumpEGLImageTargetTexture2DOES(GLenum target, GLeglImageOES image,
                                          const ANativeWindowBuffer *buffer);
void gpudVkDumpAllocateMemory(int target, const ANativeWindowBuffer *buffer);
void gpudGlDumpReadPixels(GLint framebuffer, GLint x, GLint y, GLsizei width,
                          GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
void gpudGlDumpPresentFrame(ANativeWindow *win, ANativeWindowBuffer* buffer, int fence_fd);
void gpudVkDumpPresentFrame(ANativeWindow *win, ANativeWindowBuffer* buffer, int fence_fd);
void gpudSfDumpPresentFrame(ANativeWindow *win, ANativeWindowBuffer* buffer, int fence_fd);

void gpudGlDumpStateError(void *ctx, int error, const char *error_info,
                          const char *func, const char *file, int line);

// --------------------------------------------------------------------------------------------------------------------

void gpudGlHackOrDumpShaderSource(char **source, GLuint shader, GLsizei count,
                                  const GLchar *const *string, const GLint *length);

__END_DECLS

#endif  // GPUD_INCLUDE_GPUD_GPUD_API_H_

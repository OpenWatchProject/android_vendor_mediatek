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

#ifndef __GED_GLES_H__
#define __GED_GLES_H__

#include <stdint.h>
#include "ged_type.h"
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglplatform.h>

#if defined (__cplusplus)
extern "C" {
#endif

GED_GLES_HANDLE ged_gles_create(void);

void ged_gles_destroy(GED_GLES_HANDLE hGedGles);

GED_BOOL ged_gles_is_null_driver_enabled(GED_GLES_HANDLE hGedGles);

int ged_gles_ged_frame_id(GED_GLES_HANDLE hGedGles);

GED_ERROR ged_gles_glClear_CB(GED_GLES_HANDLE hGedGles);

GED_ERROR ged_gles_glDrawXXXX_CB(GED_GLES_HANDLE hGedGles);

GED_ERROR ged_gles_glBindFramebuffer_CB(GED_GLES_HANDLE hGedGles, GLenum target, GLuint framebuffer);

GED_ERROR ged_gles_eglSwapBuffers_CB(GED_GLES_HANDLE hGedGles, EGLSurface draw);

GED_ERROR ged_gles_GPUFrameCompletion_CB(GED_GLES_HANDLE hGedGles, EGLSurface draw, int32_t i32FrameID);

GED_ERROR ged_gles_eglQueryContext_CB(GED_GLES_HANDLE hGedGles);

#if defined (__cplusplus)
}
#endif

#endif


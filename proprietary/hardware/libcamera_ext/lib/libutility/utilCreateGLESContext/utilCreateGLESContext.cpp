/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#define LOG_TAG "utilCreateGLESContext"
#define MTK_LOG_ENABLE 1
#include <stdio.h>
//#include "MTKGPUContextErrCode.h"
#include "utilCreateGLESContext.h"

#include <stdio.h>
#include <string.h>
#if defined(__ANDROID__) || defined(ANDROID)
#include <android/log.h>
#define MY_LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#else // WIN32 or LINUX64
#define MY_LOGD printf
#endif

#ifndef SIM_MAIN
extern "C"
{
extern int drvb_f0(void);
}
#endif

CreateGLESContext::CreateGLESContext()
{

    MY_LOGD("CreateGLESContext Constructor\n");

    //gCount = 0;

    //MY_LOGD("AppGPUContext m_VideoFaceBeautyState = MTK_VIDEO_FACE_BEAUTY_STATE_STANDBY\n");
    //m_VideoFaceBeautyState = MTK_VIDEO_FACE_BEAUTY_STATE_STANDBY;

};

/*******************************************************************************
*
********************************************************************************/
CreateGLESContext::~CreateGLESContext()
{
    MY_LOGD("CreateGLESContext Deconstructor\n");


    //MY_LOGD("AppVideoFaceBeauty m_VideoFaceBeautyState = MTK_VIDEO_FACE_BEAUTY_STATE_STANDBY;\n");
    //m_VideoFaceBeautyState = MTK_VIDEO_FACE_BEAUTY_STATE_STANDBY;
};

MRESULT CreateGLESContext::GPUContextCreate(MTK_GPU_CONTEXT_VERSION_ENUM ContextVersion)
{
    MRESULT    RetCode = S_GPU_CONTEXT_OK ;

    MUINT32 gles_version;
    MUINT32 context_version;


      // init pbuffer context

    //-------------------------------------------------------------------------
    if( GLES2 == ContextVersion )
    {
        gles_version = EGL_OPENGL_ES2_BIT;
        context_version = 2;
    }
    else if( GLES3 == ContextVersion )
    {
        gles_version = EGL_OPENGL_ES3_BIT_KHR;
        context_version = 3;
    }
    else
    {
        MY_LOGD("GPUContextCreate wrong context version, %d", ContextVersion);
        return E_GPU_CONTEXT_WRONG_VERSION;
    }

    const EGLint config_attribs[] = {          // config surface pixel format
    //EGL_RED_SIZE, 8,
    //EGL_GREEN_SIZE, 8,
    //EGL_BLUE_SIZE, 8,
    EGL_RENDERABLE_TYPE, 
    //gles_version,
    static_cast<EGLint>(gles_version), //Clang build error fix
    //EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
    EGL_NONE
    };

    //const EGLint buffer_attribs[] = {          // for the dummy pbuffer
    //    EGL_RENDER_BUFFER, EGL_BACK_BUFFER,
    //    EGL_NONE
    //};
    const EGLint pbuffer_attribs[] = {          // for the dummy pbuffer
        EGL_WIDTH, 1,                           // just set to min size
        EGL_HEIGHT, 1,
        EGL_NONE
    };
    const EGLint context_attribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 
        //context_version,
        static_cast<EGLint>(context_version), //Clang build error fix
        EGL_NONE
    };

    eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (EGL_NO_DISPLAY == eglDisplay) {
        MY_LOGD("GPUContextCreate eglGetDisplay failed");
        return E_GPU_CONTEXT_EGL_DISPLAY_FAIL;
    }
    //EGLBoolean returnValue;
    EGLint     majorVersion;
    EGLint     minorVersion;
    eglInitialize(eglDisplay, &majorVersion, &minorVersion);

    EGLint num = 0;
    eglChooseConfig(eglDisplay, config_attribs, &eglConfig, 1, &num);  // get usable config
    if (0 == num) {
        MY_LOGD("GPUContextCreate eglChooseConfig failed");
        return E_GPU_CONTEXT_CHOOSE_CONFIG_FAIL;
    }

    eglSurface = eglCreatePbufferSurface(eglDisplay, eglConfig, pbuffer_attribs); //off screen buffer
    if (EGL_NO_SURFACE == eglSurface) {
       MY_LOGD("GPUContextCreate eglCreatePbufferSurface failed");
       return E_GPU_CONTEXT_PBUFFER_SURFACE_FAIL;
    }

    eglContext = eglCreateContext(eglDisplay, eglConfig, NULL, context_attribs);
    if (EGL_NO_CONTEXT == eglContext) {
       MY_LOGD("GPUContextCreate eglCreateContext failed");
        return E_GPU_CONTEXT_CREATE_CONTEXT_FAIL;
    }
    eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);


    MY_LOGD("GPUContextCreate    eglDisplay:%p", eglDisplay);
    MY_LOGD("GPUContextCreate    eglSurface:%p", eglSurface);
    MY_LOGD("GPUContextCreate    eglContext:%p", eglContext);

    return RetCode;
}

MRESULT CreateGLESContext::GPUContextDestroy(void)
{
    MRESULT    RetCode = S_GPU_CONTEXT_OK ;

    eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(eglDisplay, eglContext);
    eglDestroySurface(eglDisplay, eglSurface);  // release egl rendering staff
    eglTerminate(eglDisplay);

    return RetCode;
}

MRESULT CreateGLESContext::GPUContextGetDisplayHandle(void* handle)
{
    MRESULT    RetCode = S_GPU_CONTEXT_OK ;

    EGLDisplay* ptr = (EGLDisplay*) handle;

    *ptr = eglDisplay;

    return RetCode;
}
#ifndef _UTIL_EGLIMAGE_IO_
#define _UTIL_EGLIMAGE_IO_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#ifdef _MSC_VER
//#define EGL_EGLEXT_PROTOTYPES
#define EGLImageKHR void*
#else

//#include <ui/GraphicBuffer.h>
//using namespace android;
#include "android/hardware_buffer.h"

#endif

typedef enum UTIL_EGLIMAGE_COLOR_SPACE
{
    YUV_NOT_SET=1,/* set to BT601_REDUCE as default */
    YUV_BT601_NARROW,
    YUV_BT601_FULL,
    YUV_BT709_NARROW
}UTIL_EGLIMAGE_COLOR_SPACE;
typedef struct
{
    EGLDisplay    display;
    bool is_yuv; //1: yuv data, 0:rgb data
    bool source; //1:source image, 0:dst image
    unsigned int color_attachment; //for framebuffer
    EGLImageKHR img;
    GLuint textureID;
    GLuint fbo;
    GLuint rbo;
    UTIL_EGLIMAGE_COLOR_SPACE source_color_domain;
#ifdef _MSC_VER
    void* GB;
#else
    //sp<GraphicBuffer> GB;
    AHardwareBuffer* GB;
#endif
}
UTIL_EGLIMAGE_IO_TEXTURE_PARA_STRUCT;


void GetEGLImage(UTIL_EGLIMAGE_IO_TEXTURE_PARA_STRUCT* obj);
void DestroyEGLImage(UTIL_EGLIMAGE_IO_TEXTURE_PARA_STRUCT* obj);
void GetEGLImageTexture(UTIL_EGLIMAGE_IO_TEXTURE_PARA_STRUCT* obj);
void DestroyEGLImageTexture(UTIL_EGLIMAGE_IO_TEXTURE_PARA_STRUCT* obj);

#endif
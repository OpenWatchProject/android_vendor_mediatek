#define LOG_TAG "utilEGLImageIO"
#define MTK_LOG_ENABLE 1
#include "utilEGLImageIO.h"

//#ifndef _MSC_VER
//#include "ui/gralloc_extra.h"
//#endif
#if defined(__ANDROID__) || defined(ANDROID)
#include <android/log.h>
#define MY_LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#else // WIN32 or LINUX64
#define MY_LOGD printf
#endif

static void CheckError(int t)
{
    GLuint egl_err = eglGetError();
    GLuint gl_err = glGetError();
    MY_LOGD("check point %d\n",t);
    if(egl_err!=EGL_SUCCESS)
    {
        MY_LOGD("Error code: egl %d\n",egl_err);
    }
    if(gl_err!=GL_NO_ERROR)
    {
        MY_LOGD("Error code: gl %d\n",gl_err);
    }
}

static GLuint GenerateTexure(GLint mode)
{
    GLuint texture;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);    //09,05,06 Eddie

    // allocate a texture name
    glGenTextures( 1, &texture );

    // select our current texture
    glBindTexture( GL_TEXTURE_2D, texture );

    // when texture area is small, bilinear filter the closest MIP map
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mode );//NEAREST
    // when texture area is large, bilinear filter the first MIP map
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mode );//NEAREST

    // if wrap is true, the texture wraps over at the edges (repeat)
    //       ... false, the texture ends at the edges (clamp)
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

    return texture;
}

static GLuint GenerateTexureOES(GLint mode)
{
    GLuint texture;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);    //09,05,06 Eddie

    // allocate a texture name
    glGenTextures( 1, &texture );

    // select our current texture
    glBindTexture( GL_TEXTURE_EXTERNAL_OES, texture );

    // when texture area is small, bilinear filter the closest MIP map
    glTexParameterf( GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, mode );//NEAREST
    // when texture area is large, bilinear filter the first MIP map
    glTexParameterf( GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, mode );//NEAREST

    // if wrap is true, the texture wraps over at the edges (repeat)
    //       ... false, the texture ends at the edges (clamp)
    glTexParameteri( GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri( GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

    return texture;
}
void GetEGLImage(UTIL_EGLIMAGE_IO_TEXTURE_PARA_STRUCT* obj)
{
    static EGLClientBuffer clientBuffer;
               //declare EGLImage
           //sp<GraphicBuffer>* bufAddr;
            //input
                //InputTexStruct[p].TexBuffer = *((sp<GraphicBuffer>*) pInitInfo->InputGB + p*sizeof(sp<GraphicBuffer>));
                //bufAddr = (sp<GraphicBuffer>*) *((int*)pInitInfo->InputGB + p);
                //InputTexStruct[p].TexBuffer = *bufAddr;
#ifndef _MSC_VER
    //clientBuffer = (EGLClientBuffer)(obj->GB)->getNativeBuffer();
    clientBuffer  = eglGetNativeClientBufferANDROID(obj->GB);
    //obj->img = eglCreateImageKHR(obj->display, EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID, clientBuffer, 0);
    obj->img = eglCreateImageKHR(obj->display, EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID, clientBuffer, 0);

    //check input color space
    /*
    if(obj->source == true)
    {
     gralloc_extra_ion_sf_info_t info;
     typedef const native_handle_t* buffer_handle_t;
     buffer_handle_t buf = AHardwareBuffer_getNativeHandle(obj->GB);
     gralloc_extra_query(buf, GRALLOC_EXTRA_GET_IOCTL_ION_SF_INFO, &info);//(obj->GB)->handle
     
   switch (info.status & GRALLOC_EXTRA_MASK_YUV_COLORSPACE)
   {
            case GRALLOC_EXTRA_BIT_YUV_NOT_SET:  // set to BT601_REDUCE as default 
                    MY_LOGD("utilEGLImageIO: GRALLOC_EXTRA_BIT_YUV_NOT_SET");
                    obj->source_color_domain = YUV_NOT_SET;
                break;
            case GRALLOC_EXTRA_BIT_YUV_BT601_NARROW:
                MY_LOGD("utilEGLImageIO: GRALLOC_EXTRA_BIT_YUV_BT601_NARROW");
                obj->source_color_domain = YUV_BT601_NARROW;
                break;
            case GRALLOC_EXTRA_BIT_YUV_BT601_FULL:
                MY_LOGD("utilEGLImageIO: GRALLOC_EXTRA_BIT_YUV_BT601_FULL");
                obj->source_color_domain = YUV_BT601_FULL;
                break;
            case GRALLOC_EXTRA_BIT_YUV_BT709_NARROW:
                MY_LOGD("utilEGLImageIO: GRALLOC_EXTRA_BIT_YUV_BT709_NARROW");
                obj->source_color_domain = YUV_BT709_NARROW;
                break;
        }
    }
    */
    //

    /*
    if(obj->source == true)
        {
    char* buf = NULL;
    FILE* fp;
    fp = fopen("algo_input.bin","wb");
    status_t err = (obj->GB)->lock(GRALLOC_USAGE_SW_READ_OFTEN, (void**)(&buf));
    //memcpy(buf, gImageBuffer, warp_info.Width*warp_info.Height*3/2);
    fwrite(buf,4,((obj->GB)->getWidth())*((obj->GB)->getHeight()),fp);
    //MY_LOGD("core GB addr = %x\n", buf);
    err = (obj->GB)->unlock();
    }
    */
#endif

 CheckError(-1);
    //printf("InputTexStruct[p].img: %d\n",InputTexStruct[p].img);
    if (obj->img == EGL_NO_IMAGE_KHR)
    {
        MY_LOGD("create EGL Image fail\n");
        return;
    }
 }
/*
    Get EGLImage from graphic buffer
*/
void GetEGLImageTexture(UTIL_EGLIMAGE_IO_TEXTURE_PARA_STRUCT* obj)
{
    static EGLClientBuffer clientBuffer;
               //declare EGLImage
           //sp<GraphicBuffer>* bufAddr;
            //input
                //InputTexStruct[p].TexBuffer = *((sp<GraphicBuffer>*) pInitInfo->InputGB + p*sizeof(sp<GraphicBuffer>));
                //bufAddr = (sp<GraphicBuffer>*) *((int*)pInitInfo->InputGB + p);
                //InputTexStruct[p].TexBuffer = *bufAddr;
#ifndef _MSC_VER
    //clientBuffer = (EGLClientBuffer)(obj->GB)->getNativeBuffer();
    clientBuffer  = eglGetNativeClientBufferANDROID(obj->GB);
    //obj->img = eglCreateImageKHR(obj->display, EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID, clientBuffer, 0);
    obj->img = eglCreateImageKHR(obj->display, EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID, clientBuffer, 0);

    //check input color space
    /*
    if(obj->source == true)
    {
     gralloc_extra_ion_sf_info_t info;
     typedef const native_handle_t* buffer_handle_t;
     buffer_handle_t buf = AHardwareBuffer_getNativeHandle(obj->GB);
     gralloc_extra_query(buf, GRALLOC_EXTRA_GET_IOCTL_ION_SF_INFO, &info);//(obj->GB)->handle
     
   switch (info.status & GRALLOC_EXTRA_MASK_YUV_COLORSPACE)
   {
            case GRALLOC_EXTRA_BIT_YUV_NOT_SET:  // set to BT601_REDUCE as default 
                    MY_LOGD("utilEGLImageIO: GRALLOC_EXTRA_BIT_YUV_NOT_SET");
                    obj->source_color_domain = YUV_NOT_SET;
                break;
            case GRALLOC_EXTRA_BIT_YUV_BT601_NARROW:
                MY_LOGD("utilEGLImageIO: GRALLOC_EXTRA_BIT_YUV_BT601_NARROW");
                obj->source_color_domain = YUV_BT601_NARROW;
                break;
            case GRALLOC_EXTRA_BIT_YUV_BT601_FULL:
                MY_LOGD("utilEGLImageIO: GRALLOC_EXTRA_BIT_YUV_BT601_FULL");
                obj->source_color_domain = YUV_BT601_FULL;
                break;
            case GRALLOC_EXTRA_BIT_YUV_BT709_NARROW:
                MY_LOGD("utilEGLImageIO: GRALLOC_EXTRA_BIT_YUV_BT709_NARROW");
                obj->source_color_domain = YUV_BT709_NARROW;
                break;
        }
    }
    */
    //

    /*
    if(obj->source == true)
        {
    char* buf = NULL;
    FILE* fp;
    fp = fopen("algo_input.bin","wb");
    status_t err = (obj->GB)->lock(GRALLOC_USAGE_SW_READ_OFTEN, (void**)(&buf));
    //memcpy(buf, gImageBuffer, warp_info.Width*warp_info.Height*3/2);
    fwrite(buf,4,((obj->GB)->getWidth())*((obj->GB)->getHeight()),fp);
    //MY_LOGD("core GB addr = %x\n", buf);
    err = (obj->GB)->unlock();
    }
    */
#endif

 CheckError(-1);
    //printf("InputTexStruct[p].img: %d\n",InputTexStruct[p].img);
    if (obj->img == EGL_NO_IMAGE_KHR)
    {
        MY_LOGD("create EGL Image fail\n");
        return;
    }



#ifndef _MSC_VER
    if(obj->is_yuv == true)
    {
        obj->textureID = GenerateTexureOES(GL_LINEAR);
        glBindTexture(GL_TEXTURE_EXTERNAL_OES, obj->textureID);//GL_TEXTURE_EXTERNAL_OES
        glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, obj->img);    // bind eglImage to texture
        /*
        unsigned char* temp_buf = (unsigned char*) malloc(((obj->GB)->getWidth())*((obj->GB)->getHeight())*4);
        memset(temp_buf, 0x66, ((obj->GB)->getWidth())*((obj->GB)->getHeight())*4);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (obj->GB)->getWidth(), (obj->GB)->getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, temp_buf);
        free(temp_buf);
        */
        MY_LOGD("input yuv img\n");
        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        //MY_LOGD("GB width %d GB height %d GB stride %d\n",(EGLClientBuffer)(obj->GB)->getWidth(), (EGLClientBuffer)(obj->GB)->getHeight(), (EGLClientBuffer)(obj->GB)->getStride());
    }
    else
    {
        obj->textureID = GenerateTexure(GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, obj->textureID);//GL_TEXTURE_EXTERNAL_OES
        glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, obj->img);    // bind eglImage to texture
        MY_LOGD("input rgb img\n");
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        //MY_LOGD("GB width %d GB height %d GB stride %d\n",(EGLClientBuffer)(obj->GB)->getWidth(), (EGLClientBuffer)(obj->GB)->getHeight(), (EGLClientBuffer)(obj->GB)->getStride());
        //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1920, 1440, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    }

#endif
 CheckError(-2);


    //MY_LOGD("obj->textureID %d, obj->img %d\n",obj->textureID, obj->img);

    if(obj->source == false) //dst
    {
        glGenFramebuffers(1, &(obj->fbo));
        //MY_LOGD("obj->fbo %d, obj->textureID %d, obj->img %d, obj->color_attachment %d\n",obj->fbo,obj->textureID, obj->img,obj->color_attachment);
        glBindFramebuffer(GL_FRAMEBUFFER, obj->fbo);
      /*
      glGenRenderbuffers(1, &(obj->rbo));
        glBindRenderbuffer(GL_RENDERBUFFER, obj->rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8_OES, 1920, 1440);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, obj->rbo);
      */
      if(obj->is_yuv == true)
          glFramebufferTexture2D(GL_FRAMEBUFFER, obj->color_attachment, GL_TEXTURE_EXTERNAL_OES, obj->textureID, 0);
      else
          glFramebufferTexture2D(GL_FRAMEBUFFER, obj->color_attachment, GL_TEXTURE_2D, obj->textureID, 0);    //GL_COLOR_ATTACHMENT0


           CheckError(-3);
          GLenum status ;
          status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if(status != GL_FRAMEBUFFER_COMPLETE)
          MY_LOGD("framebuffer not complete: %d\n",status);
      CheckError(-4);
    }

        return;

}
/*
    Destroy EGLImage
*/
void DestroyEGLImageTexture(UTIL_EGLIMAGE_IO_TEXTURE_PARA_STRUCT* obj)
{
#ifndef _MSC_VER
    eglDestroyImageKHR(obj->display, obj->img);
#endif
    glDeleteTextures(1, &obj->textureID);

    if(obj->source == false)
    {
        glDeleteFramebuffers(1, &obj->fbo);
    }
}
void DestroyEGLImage(UTIL_EGLIMAGE_IO_TEXTURE_PARA_STRUCT* obj)
{
#ifndef _MSC_VER
    eglDestroyImageKHR(obj->display, obj->img);
#endif
}

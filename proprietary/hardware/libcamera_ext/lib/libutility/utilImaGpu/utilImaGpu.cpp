//-------------------------------------------------------------------------------
//-- Title      : The imaGPU library implementation
//
//-- Creator    : Eddie Tsao
//
//-- Version    : v0
//
//-- Description : imaGPU is a GLES2.0 base framework for image processing
//
//-- Add glPixelStorei(GL_UNPACK_ALIGNMENT, 1) for igLoadImage.
//-------------------------------------------------------------------------------
#define LOG_TAG "utilImaGpu"
#define MTK_LOG_ENABLE 1
#include "MTKUtilCommon.h"
#include "utilImaGpu.h"

#ifdef SIM_MAIN
#include <stdio.h>
#define LOGD printf
#else
#include <android/log.h>
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#endif

UTIL_ERRCODE_ENUM igLoadShaderString(GLuint *shd_handle, const char *filestr, GLenum type)
{
    UTIL_ERRCODE_ENUM result = UTIL_OK;
    GLint bShaderCompiled = 0;
    GLuint Shader = 0;

    // Create the shader object
    Shader = glCreateShader(type);

    // Load the source
    glShaderSource(Shader, 1, (const char**)&filestr, NULL);

    // Compile the source code
    glCompileShader(Shader);

    // error check
    glGetShaderiv(Shader, GL_COMPILE_STATUS, &bShaderCompiled);
    if ((glGetError() != GL_NO_ERROR) || !bShaderCompiled || !Shader)
    {
        result = UTIL_IMA_GPU_ERR_READING_SHADER_STRING;
        LOGD("[%s] Error Message: %s\n", LOG_TAG, UTIL_GET_ERRCODE_NAME(result));
        return result;
    }

    *shd_handle = Shader;
    return result;
}

UTIL_ERRCODE_ENUM igCreateProgram(GLuint *po, GLuint vs, GLuint fs, GLfloat *ref_buffer)
{
    UTIL_ERRCODE_ENUM result = UTIL_OK;
    GLuint ProgramObject = glCreateProgram();
    GLint bLinked = 0;

    // Attach the fragment and vertex shaders to it
    glAttachShader(ProgramObject, vs);
    glAttachShader(ProgramObject, fs);

    // Here needs to be implement the imaGPU shader framework
    // define VPE_TC
    // define VPE_SC
    // define VPE_SD
    // define VPG_REF

    // Bind the custom vertex attribute "myVertex" to location VERTEX_ARRAY
    glBindAttribLocation(ProgramObject, VERTEX_ARRAY, "vpg_tc");
    glBindAttribLocation(ProgramObject, TEXTURE_ARRAY, "vpg_sc");
    if (ref_buffer)
    {
        glBindAttribLocation(ProgramObject, REF_ARRAY, "vpg_in_ref");
    }

    // Link the program
    glLinkProgram(ProgramObject);

    // use program
    glUseProgram(ProgramObject);

    // erro check
    if (!ProgramObject)
    {
        result = UTIL_IMA_GPU_ERR_CREATING_PROGRAM_OBJECT;
        LOGD("[%s] Error Message: %s\n", LOG_TAG, UTIL_GET_ERRCODE_NAME(result));
        return result;
    }

    // link program
    glGetProgramiv(ProgramObject, GL_LINK_STATUS, &bLinked);
    if ((glGetError() != GL_NO_ERROR) || !bLinked)
    {
        result = UTIL_IMA_GPU_ERR_LINKING_PROGRAM_OBJECT;
        LOGD("[%s] Error Message: %s\n", LOG_TAG, UTIL_GET_ERRCODE_NAME(result));
        return result;
    }

    // store result
    *po = ProgramObject;

    return result;
}

UTIL_ERRCODE_ENUM igLoadImage( void * data, GLint mode, GLenum format, int width, int height)
{
    UTIL_ERRCODE_ENUM result = UTIL_OK;
    GLuint texture;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  //09,05,06 Eddie

    // allocate a texture name
    glGenTextures( 1, &texture );

    // select our current texture
    glBindTexture( GL_TEXTURE_2D, texture );

    // when texture area is small, bilinear filter the closest MIP map
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    // when texture area is large, bilinear filter the first MIP map
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    // if wrap is true, the texture wraps over at the edges (repeat)
    //       ... false, the texture ends at the edges (clamp)
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mode );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mode );

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

    return result;
}

UTIL_ERRCODE_ENUM igLoadImage(void * data, GLint mode, GLenum format, int width, int height, GLuint *plan)
{
    UTIL_ERRCODE_ENUM result = UTIL_OK;
    GLint pn = (format==IG_YUV420SP) ? 2 : 3;
    GLint cw = (format==IG_RGB) ? width : width>>1;
    GLint ch = (format!=IG_YUV422P && format!=IG_RGB) ? height>>1 : height;

    unsigned char *plan_data;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // allocate a texture name
    glGenTextures(pn, plan);

    //Create Luma plan
    plan_data = (unsigned char *)data;
    glBindTexture(GL_TEXTURE_2D, plan[0]);

    // when texture area is small, bilinear filter the closest MIP map
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    // when texture area is large, bilinear filter the first MIP map
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    // if wrap is true, the texture wraps over at the edges (repeat)
    //       ... false, the texture ends at the edges (clamp)
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mode );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mode );

    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, plan_data);

    if (pn==3)
    {
        //Create Cb plan
        plan_data = plan_data + width*height;
        glBindTexture(GL_TEXTURE_2D, plan[1]);

        // when texture area is small, bilinear filter the closest MIP map
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        // when texture area is large, bilinear filter the first MIP map
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

        // if wrap is true, the texture wraps over at the edges (repeat)
        //       ... false, the texture ends at the edges (clamp)
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mode );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mode );

        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, cw, ch, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, plan_data);

        //Create Cr plan
        plan_data = plan_data + cw*ch;
        glBindTexture(GL_TEXTURE_2D, plan[2]);

        // when texture area is small, bilinear filter the closest MIP map
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        // when texture area is large, bilinear filter the first MIP map
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

        // if wrap is true, the texture wraps over at the edges (repeat)
        //       ... false, the texture ends at the edges (clamp)
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mode );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mode );

        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, cw, ch, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, plan_data);
    }
    else
    {
        //Create Chroma plan
        plan_data = plan_data + width*height;
        glBindTexture(GL_TEXTURE_2D, plan[1]);

        // when texture area is small, bilinear filter the closest MIP map
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        // when texture area is large, bilinear filter the first MIP map
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

        // if wrap is true, the texture wraps over at the edges (repeat)
        //       ... false, the texture ends at the edges (clamp)
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mode );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mode );

        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, cw, ch, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, plan_data);
    }

    return result;
}

UTIL_ERRCODE_ENUM igCreateVPE(GLint *elenum, GLint *vpg_size, vtx_fmt *vtx_data, GLushort *ele_data, GLuint *buffers)
{
    UTIL_ERRCODE_ENUM result = UTIL_OK;
    int vpg_width  = vpg_size[0];
    int vpg_height = vpg_size[1];

    int i,j, index, start;
    GLfloat gridx = 2/GLfloat(vpg_width);
    GLfloat gridy = 2/GLfloat(vpg_height);
    GLfloat gridu = 1/GLfloat(vpg_width);
    GLfloat gridv = 1/GLfloat(vpg_height);

    /* Generate array and element buffers. */
    glGenBuffers(2, buffers);
    /* Bind the array and element buffers. */
    glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);

    //Prepare vtx data
    index = 2;
    vtx_data[0].x = -1.0;
    vtx_data[0].y = -1.0;
    vtx_data[0].u = 0.0;
    vtx_data[0].v = 0.0;
    vtx_data[1].x = -1.0;
    vtx_data[1].y = vtx_data[0].y + gridy;
    vtx_data[1].u = 0.0;
    vtx_data[1].v = vtx_data[0].v + gridv;
    for (i=0; i<vpg_height; i++)
    {
        for (j=0; j<vpg_width; j++)
        {
            vtx_data[index].x = vtx_data[index-2].x + gridx;
            vtx_data[index].y = vtx_data[index-2].y;
            vtx_data[index].u = vtx_data[index-2].u + gridu;
            vtx_data[index].v = vtx_data[index-2].v;
            index ++;
            vtx_data[index].x = vtx_data[index-2].x + gridx;
            vtx_data[index].y = vtx_data[index-2].y;
            vtx_data[index].u = vtx_data[index-2].u + gridu;
            vtx_data[index].v = vtx_data[index-2].v;
            index ++;
        }
        if (i+1 != vpg_height)
        {
            vtx_data[index].x =  -1.0;
            vtx_data[index].y = vtx_data[index-1].y;
            vtx_data[index].u = 0.0;
            vtx_data[index].v = vtx_data[index-1].v;
            index ++;
            vtx_data[index].x =  -1.0;
            vtx_data[index].y = vtx_data[index-1].y + gridy;
            vtx_data[index].u = 0.0;
            vtx_data[index].v = vtx_data[index-1].v + gridv;
            index ++;
        }
    }
    /* Upload the array buffer. */
    glBufferData(GL_ARRAY_BUFFER, sizeof(vtx_fmt)*(vpg_width+1)*2*vpg_height, vtx_data, GL_STATIC_DRAW);


    //Prepare element data
    index = 0;
    start = 0;
    for (i=0; i<vpg_height; i++)
    {
        for (j=0; j<vpg_width; j++)
        {
            ele_data[index] = start;
            ele_data[index+1] = start + 1;
            ele_data[index+2] = start + 2;

            ele_data[index+3] = ele_data[index+2];
            ele_data[index+4] = ele_data[index+1];
            ele_data[index+5] = start + 3;
            index += 6;
            start += 2;
        }
        start += 2;
    }
    /* Upload the array buffer. */
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort)*6*vpg_width*vpg_height, ele_data, GL_STATIC_DRAW);

    /* Enable vertex array attribute 0 (position). */
    glEnableVertexAttribArray(VERTEX_ARRAY);
    glVertexAttribPointer(VERTEX_ARRAY, 2, GL_FLOAT, GL_FALSE, sizeof(vtx_fmt), 0);

    /* Enable vertex array attribute 1 (texture coordiate). */
    glEnableVertexAttribArray(TEXTURE_ARRAY);
    glVertexAttribPointer(TEXTURE_ARRAY, 2, GL_FLOAT, GL_FALSE, sizeof(vtx_fmt), (GLvoid *)(sizeof(GLfloat)*2));

    *elenum =  6*vpg_width*vpg_height;
    return result;
}

UTIL_ERRCODE_ENUM igFreeShader(GLuint shader)
{
    UTIL_ERRCODE_ENUM result = UTIL_OK;

    if (shader == 0)
    {
        result = UTIL_IMA_GPU_ERR_NULL_SHADER_HANDLE;
        LOGD("[%s] Error Message: %s\n", LOG_TAG, UTIL_GET_ERRCODE_NAME(result));
        return result;
    }
    glDeleteShader(shader);

    return result;
}

UTIL_ERRCODE_ENUM igFreeProgram(GLuint program)
{
    UTIL_ERRCODE_ENUM result = UTIL_OK;

    if (program == 0)
    {
        result = UTIL_IMA_GPU_ERR_NULL_PROGRAM_HANDLE;
        LOGD("[%s] Error Message: %s\n", LOG_TAG, UTIL_GET_ERRCODE_NAME(result));
        return result;
    }
    glDeleteProgram(program);

    return result;
}

UTIL_ERRCODE_ENUM igFreeImage(GLuint n, const GLuint* image)
{
    UTIL_ERRCODE_ENUM result = UTIL_OK;

    for (GLuint i=0; i<n; i++)
    {
        if (image[i] == 0)
        {
            result = UTIL_IMA_GPU_ERR_NULL_TEXTURE_HANDLE;
            LOGD("[%s] Error Message: %s\n", LOG_TAG, UTIL_GET_ERRCODE_NAME(result));
            return result;
        }
    }
    glDeleteTextures(n, image);

    return result;
}

UTIL_ERRCODE_ENUM igFreeVPE(const GLuint* buffers)
{
    UTIL_ERRCODE_ENUM result = UTIL_OK;

    for (GLuint i=0; i<2; i++)
    {
        if (buffers[i] == 0)
        {
            result = UTIL_IMA_GPU_ERR_NULL_VERTEX_HANDLE;
            LOGD("[%s] Error Message: %s\n", LOG_TAG, UTIL_GET_ERRCODE_NAME(result));
            return result;
        }
    }
    glDeleteBuffers(2, buffers);

    return result;
}

UTIL_ERRCODE_ENUM igCreateBufferObject(int target_width, int target_height, GLuint &fbo, GLuint &rbo)
{
    UTIL_ERRCODE_ENUM result = UTIL_OK;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8_OES, target_width, target_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
    {
        result = UTIL_IMA_GPU_ERR_CREATING_BUFFER_OBJECT;
        LOGD("[%s] Error Message: %s\n", LOG_TAG, UTIL_GET_ERRCODE_NAME(result));
        return result;
    }
    else
    {
        return result;
    }
}

UTIL_ERRCODE_ENUM igFreeBufferObject(GLuint &fbo, GLuint &rbo)
{
    UTIL_ERRCODE_ENUM result = UTIL_OK;

    glDeleteFramebuffers(1, &fbo);
    glDeleteRenderbuffers(1, &rbo);

    return result;
}

UTIL_ERRCODE_ENUM igLoadShaderBinary(GLuint *shd_handle, const char *shd_bin, GLint shd_size, GLenum type)
{
    UTIL_ERRCODE_ENUM result = UTIL_OK;

    // create shader handle
    *shd_handle = glCreateShader(type);

    // get shader format
    GLint formats;
    glGetIntegerv(GL_SHADER_BINARY_FORMATS, &formats);

    // load shader binary
    glShaderBinary(1, shd_handle, formats, shd_bin, shd_size);
    if(glGetError() != GL_NO_ERROR || !(*shd_handle))
    {
        result = UTIL_IMA_GPU_ERR_READING_SHADER_BINARY;
        LOGD("[%s] Error Message: %s\n", LOG_TAG, UTIL_GET_ERRCODE_NAME(result));
        return result;
    }

    return result;
}

UTIL_ERRCODE_ENUM igStartRendering(GLuint program, const char *img_name, GLuint image, GLuint elenum, P_UTIL_CLIP_IMAGE_STRUCT src)
{
    UTIL_ERRCODE_ENUM result = UTIL_OK;
    if(img_name != NULL)
      LOGD("[%s] img_name: %s \n",LOG_TAG, img_name);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, image);
    glUniform1i(glGetUniformLocation(program, "SrcImg"), 0);
    glViewport(-src->clip_x, -src->clip_y, src->width, src->height);
    glDrawElements(GL_TRIANGLES, elenum, GL_UNSIGNED_SHORT, 0);
    glFlush();
    glReadPixels(0, 0, src->clip_width, src->clip_height, GL_RGBA, GL_UNSIGNED_BYTE, src->data);

    return result;
}


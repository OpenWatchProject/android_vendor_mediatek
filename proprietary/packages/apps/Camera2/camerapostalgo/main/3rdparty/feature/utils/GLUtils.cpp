#include "GLUtils.h"
#include "LogUtils.h"
#define LOG_TAG "FeatureUtils/GLUtils"

GLuint GLUtils::generateTexureOES(GLint mode)
{
    FUNCTION_IN;
    GLuint texture;

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

    FUNCTION_OUT;
    return texture;
}

void GLUtils::getEGLImageTexture(GLUTILS_GET_EGLIMAGE_TEXTURE_PARAMS *params)
{
    FUNCTION_IN;
    // create EGLImage
    static EGLClientBuffer clientBuffer;
    clientBuffer  = eglGetNativeClientBufferANDROID(params->graphicBuffer);
    params->eglImage= eglCreateImageKHR(params->eglDisplay, EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID, clientBuffer, 0);

    // bind EGLImage to texture
    params->textureID = generateTexureOES(GL_LINEAR);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, params->textureID); //GL_TEXTURE_EXTERNAL_OES
    glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, params->eglImage); // bind eglImage to texture

    if (params->isRenderTarget)
    {
        glGenFramebuffers(1, &(params->fbo));
        glBindFramebuffer(GL_FRAMEBUFFER, params->fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_EXTERNAL_OES, params->textureID, 0);
        glCheckFramebufferStatus(GL_FRAMEBUFFER);
    }
    FUNCTION_OUT;
}

void GLUtils::releaseEGLImageTexture(GLUTILS_GET_EGLIMAGE_TEXTURE_PARAMS *params)
{
    FUNCTION_IN;

    if (params->textureID) {
        GLuint texture[1];
        texture[0] = params->textureID;
        glDeleteTextures(1, texture);
    }

    if (params->eglImage) {
        eglDestroyImageKHR(params->eglDisplay, params->eglImage);
    }

    if (params->isRenderTarget) {
        GLuint fbo[1];
        fbo[0] = params->fbo;
        glDeleteFramebuffers(1, fbo);
    }

    FUNCTION_OUT;
}

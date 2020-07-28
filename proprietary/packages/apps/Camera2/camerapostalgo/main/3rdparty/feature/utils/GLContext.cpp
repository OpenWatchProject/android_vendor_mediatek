#include <android/native_window.h>
#include <vndk/hardware_buffer.h>
#include <strings.h>
#include <android/log.h>

#include "GLContext.h"
#include "LogUtils.h"
#define LOG_TAG "FeatureUtils/GLContext"

GLContext::GLContext() {
    _display = EGL_NO_DISPLAY;
    _surface = EGL_NO_SURFACE;
    _context = EGL_NO_CONTEXT;
}

GLContext::~GLContext(){
}

bool GLContext::initialize(bool isYV12){
    FUNCTION_IN;

    const EGLint attribs_yv12[] = {
        EGL_NATIVE_VISUAL_ID, AHARDWAREBUFFER_FORMAT_YV12,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_RED_SIZE, 8,
        EGL_NONE
    };

    const EGLint attribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_RED_SIZE, 8,
        EGL_NONE
    };

    EGLint context_attribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    const EGLint pbuffer_attribs[] = {          // for the dummy pbuffer
        EGL_WIDTH, 1,                           // just set to min size
        EGL_HEIGHT, 1,
        EGL_NONE
    };

    EGLDisplay display;
    EGLConfig config;
    EGLint numConfigs;
    EGLSurface surface;
    EGLContext context;

    if ((display = eglGetDisplay(EGL_DEFAULT_DISPLAY)) == EGL_NO_DISPLAY) {
        MY_LOGE("eglGetDisplay() returned error %d", eglGetError());
        return false;
    }
    if (!eglInitialize(display, 0, 0)) {
        MY_LOGE("eglInitialize() returned error %d", eglGetError());
        return false;
    }

    if (!eglChooseConfig(display, isYV12 ? attribs_yv12 : attribs, &config, 1, &numConfigs)) {
        MY_LOGE("eglChooseConfig() returned error %d", eglGetError());
        unInitialize();
        return false;
    }

    if (!(surface = eglCreatePbufferSurface(display, config, pbuffer_attribs))) { //off screen buffer
        MY_LOGE("eglCreatePbufferSurface() returned error %d", eglGetError());
        unInitialize();
        return false;
    }

    if (!(context = eglCreateContext(display, config, EGL_NO_CONTEXT, context_attribs))) {
        MY_LOGE("eglCreateContext() returned error %d", eglGetError());
        unInitialize();
        return false;
    }

    _display = display;
    _surface = surface;
    _context = context;
    FUNCTION_OUT;
    return true;
}

void GLContext::unInitialize() {
    FUNCTION_IN;

    eglMakeCurrent(_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(_display, _context);
    eglDestroySurface(_display, _surface);
    eglTerminate(_display);

    _display = EGL_NO_DISPLAY;
    _surface = EGL_NO_SURFACE;
    _context = EGL_NO_CONTEXT;

    FUNCTION_OUT;

    return;
}

bool GLContext::enable(){
    FUNCTION_IN;
    if (!eglMakeCurrent(_display, _surface, _surface, _context)) {
        MY_LOGE("eglMakeCurrent() return error %d", eglGetError());
        unInitialize();
        return false;
    }
    FUNCTION_OUT;
    return true;
}

EGLSyncKHR GLContext::createSyncKHR() {
    _sync = eglCreateSyncKHR(_display, EGL_SYNC_NATIVE_FENCE_ANDROID, NULL);
    if (_sync == EGL_NO_SYNC_KHR) {
        MY_LOGE("getSyncKHR return error %#x", eglGetError());
        return nullptr;
    }
    return _sync;
}

int GLContext::clientWaitSyncKHR() {
    EGLint result = eglClientWaitSyncKHR(_display, _sync, 0, EGL_FOREVER_KHR);
    if (result == EGL_FALSE) {
        MY_LOGE("getSyncKHR return error %#x", eglGetError());
        return -1;
    }
    return 1;
}

void GLContext::destroySyncKHR() {
    eglDestroySyncKHR(_display, _sync);
}

 int GLContext::createFenceFd(){
    int fenceFd = eglDupNativeFenceFDANDROID(_display, _sync);
    eglDestroySyncKHR(_display, _sync);
    if (fenceFd == EGL_NO_NATIVE_FENCE_FD_ANDROID) {
        MY_LOGE("createFenceFd return error %#x", eglGetError());
        return -1;
    }
    return fenceFd;
}

void GLContext::disable() {
    FUNCTION_IN;
    eglMakeCurrent(_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    FUNCTION_OUT;
}
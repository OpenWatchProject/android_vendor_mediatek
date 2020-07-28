//#include <string>
#include <android/log.h>
#include <string.h>
#include <stdarg.h>
#include "afaaa_log.h"

pthread_mutex_t mutexAfLog = PTHREAD_MUTEX_INITIALIZER;

void afBlog_print(AFBLog *blog, const char *tag, const char *fmt, ...)
{
    pthread_mutex_lock(&mutexAfLog);

    char* buf = blog->mBuf;
    size_t* totLen = &(blog->mLen);
    //
    va_list arg;
    va_start(arg, fmt);
    size_t const freeLen = AF_BUF_REAL_SIZE - *totLen;
    size_t len = vsnprintf(buf + *totLen, freeLen, fmt, arg);
    va_end(arg);
    //
    *totLen += len;
    if (*totLen > AF_BUF_REAL_SIZE) {
        __android_log_print(ANDROID_LOG_ERROR, tag,
            "too many log, may lost information %zu > %d",
            *totLen, AF_BUF_REAL_SIZE);
        *totLen = AF_BUF_REAL_SIZE;
    }
    //
    while(*totLen > AF_BUF_THRESHOLD){
        __android_log_print(ANDROID_LOG_DEBUG, tag, "%.*s", AF_BUF_THRESHOLD, buf);
        *totLen -= AF_BUF_THRESHOLD;
        memmove(buf, buf + AF_BUF_THRESHOLD, *totLen);
    }
    buf[*totLen] = '\0';

    pthread_mutex_unlock(&mutexAfLog);
}

void afBlog_flush(AFBLog *blog, const char *tag)
{
    pthread_mutex_lock(&mutexAfLog);

    if(blog->mLen != 0) {
        __android_log_print(ANDROID_LOG_DEBUG, tag, "%s", blog->mBuf);
        blog->mLen = 0;
    }

    pthread_mutex_unlock(&mutexAfLog);
}
